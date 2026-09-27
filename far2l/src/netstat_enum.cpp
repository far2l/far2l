#include "netstat_enum.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cctype>
#include <unordered_map>
#include <algorithm>
#include <cstdio>

#if defined(__linux__)
#include <dirent.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#elif defined(__APPLE__)
#include <libproc.h>
#include <sys/proc_info.h>
#include <sys/sysctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <arpa/inet.h>
#include <unistd.h>
#if defined(__FreeBSD__)
#include <sys/user.h>
#endif
#endif

#include <WideMB.h>

namespace NetstatFar2l {

const char* TcpStateToString(TcpState state) {
    switch (state) {
        case TcpState::ESTABLISHED: return "ESTABLISHED";
        case TcpState::SYN_SENT:    return "SYN_SENT";
        case TcpState::SYN_RECV:    return "SYN_RECV";
        case TcpState::FIN_WAIT1:   return "FIN_WAIT1";
        case TcpState::FIN_WAIT2:   return "FIN_WAIT2";
        case TcpState::TIME_WAIT:   return "TIME_WAIT";
        case TcpState::CLOSE:       return "CLOSE";
        case TcpState::CLOSE_WAIT:  return "CLOSE_WAIT";
        case TcpState::LAST_ACK:    return "LAST_ACK";
        case TcpState::LISTEN:      return "LISTEN";
        case TcpState::CLOSING:     return "CLOSING";
        default:                    return "UNKNOWN";
    }
}

TcpState HexToTcpState(uint8_t hex_state) {
    if (hex_state >= 1 && hex_state <= 11) {
        return static_cast<TcpState>(hex_state);
    }
    return TcpState::UNKNOWN;
}

std::string ConnectionEntry::GetLocalEndpoint() const {
    if (local_addr.find(':') != std::string::npos && local_addr.front() != '[') {
        return "[" + local_addr + "]:" + std::to_string(local_port);
    }
    return local_addr + ":" + std::to_string(local_port);
}

std::string ConnectionEntry::GetRemoteEndpoint() const {
    if (remote_port == 0 && (remote_addr == "0.0.0.0" || remote_addr == "::" || remote_addr == "*")) {
        return "*:*";
    }
    if (remote_addr.find(':') != std::string::npos && remote_addr.front() != '[') {
        return "[" + remote_addr + "]:" + std::to_string(remote_port);
    }
    return remote_addr + ":" + std::to_string(remote_port);
}

std::string ConnectionEntry::GetProcessDisplay() const {
    if (pid <= 0 && process_name.empty()) {
        return "-";
    }
    if (!process_name.empty() && pid > 0) {
        return std::to_string(pid) + "/" + process_name;
    }
    if (pid > 0) {
        return std::to_string(pid);
    }
    return process_name;
}

static bool MatchesFilter(TcpState state, SocketFilter filter) {
    switch (filter) {
        case SocketFilter::OPEN_AND_CONNECTED:
            return state == TcpState::LISTEN || state == TcpState::ESTABLISHED;
        case SocketFilter::LISTEN_ONLY:
            return state == TcpState::LISTEN;
        case SocketFilter::ESTABLISHED_ONLY:
            return state == TcpState::ESTABLISHED;
        case SocketFilter::ALL_STATES:
            return true;
    }
    return true;
}

// -----------------------------------------------------------------------------
// LINUX IMPLEMENTATION (/proc/net/tcp, /proc/net/tcp6, /proc/[pid]/fd)
// -----------------------------------------------------------------------------
#if defined(__linux__)

// Convert 8-character hex string from /proc/net/tcp to IPv4 string
static std::string ParseIpv4Address(const std::string& hex_addr) {
    if (hex_addr.length() < 8) return "0.0.0.0";
    uint32_t raw = 0;
    if (std::sscanf(hex_addr.c_str(), "%X", &raw) != 1) {
        return "0.0.0.0";
    }
    struct in_addr addr;
    addr.s_addr = raw; // /proc/net/tcp stores in host-endian representation of network bytes
    char buf[INET_ADDRSTRLEN] = {0};
    if (inet_ntop(AF_INET, &addr, buf, sizeof(buf))) {
        return std::string(buf);
    }
    return "0.0.0.0";
}

// Convert 32-character hex string from /proc/net/tcp6 to IPv6 string
static std::string ParseIpv6Address(const std::string& hex_addr) {
    if (hex_addr.length() < 32) return "::";
    uint32_t words[4] = {0, 0, 0, 0};
    if (std::sscanf(hex_addr.c_str(), "%08X%08X%08X%08X",
                    &words[0], &words[1], &words[2], &words[3]) != 4) {
        return "::";
    }

    struct in6_addr in6;
    for (int i = 0; i < 4; ++i) {
        // Words in /proc/net/tcp6 are 32-bit integers in host order
        uint32_t w = words[i];
        std::memcpy(&in6.s6_addr[i * 4], &w, 4);
    }

    // Check for IPv4-mapped IPv6 (::ffff:x.x.x.x)
    if (words[0] == 0 && words[1] == 0 && 
        words[2] == 0xFFFF0000) {
        struct in_addr in4;
        in4.s_addr = words[3];
        char buf[INET_ADDRSTRLEN] = {0};
        if (inet_ntop(AF_INET, &in4, buf, sizeof(buf))) {
            return std::string("::ffff:") + buf;
        }
    }

    char buf[INET6_ADDRSTRLEN] = {0};
    if (inet_ntop(AF_INET6, &in6, buf, sizeof(buf))) {
        return std::string(buf);
    }
    return "::";
}

// Build a mapping of socket inode -> ProcessInfo by scanning /proc/[pid]/fd/*
static std::unordered_map<uint64_t, ProcessInfo> BuildInodeProcessMap() {
    std::unordered_map<uint64_t, ProcessInfo> inode_map;
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) {
        return inode_map;
    }

    struct dirent* entry = nullptr;
    while ((entry = readdir(proc_dir)) != nullptr) {
        // Only inspect directories that are numeric (PIDs)
        if (entry->d_type != DT_DIR && entry->d_type != DT_UNKNOWN) {
            continue;
        }
        if (!std::isdigit(entry->d_name[0])) {
            continue;
        }

        pid_t pid = std::atoi(entry->d_name);
        if (pid <= 0) continue;

        std::string pid_str = entry->d_name;
        std::string fd_dir_path = "/proc/" + pid_str + "/fd";
        DIR* fd_dir = opendir(fd_dir_path.c_str());
        if (!fd_dir) {
            // Permission denied or process exited
            continue;
        }

        // We lazily read process name only if at least one socket is found
        bool process_info_read = false;
        ProcessInfo pinfo;
        pinfo.pid = pid;

        struct dirent* fd_entry = nullptr;
        while ((fd_entry = readdir(fd_dir)) != nullptr) {
            if (fd_entry->d_name[0] == '.') continue;

            std::string link_path = fd_dir_path + "/" + fd_entry->d_name;
            char target[256];
            ssize_t len = readlink(link_path.c_str(), target, sizeof(target) - 1);
            if (len <= 0) continue;
            target[len] = '\0';

            uint64_t inode = 0;
            // Check for "socket:[12345]"
            if (std::strncmp(target, "socket:[", 8) == 0) {
                inode = std::strtoull(target + 8, nullptr, 10);
            } else if (std::strncmp(target, "[0000]:", 7) == 0) {
                inode = std::strtoull(target + 7, nullptr, 10);
            }

            if (inode > 0) {
                if (!process_info_read) {
                    // Read comm (short process name)
                    std::string comm_path = "/proc/" + pid_str + "/comm";
                    std::ifstream comm_file(comm_path);
                    if (comm_file.is_open()) {
                        std::getline(comm_file, pinfo.name);
                        // Trim newline or carriage return
                        while (!pinfo.name.empty() && 
                               (pinfo.name.back() == '\n' || pinfo.name.back() == '\r')) {
                            pinfo.name.pop_back();
                        }
                    }
                    // Read cmdline
                    std::string cmd_path = "/proc/" + pid_str + "/cmdline";
                    std::ifstream cmd_file(cmd_path, std::ios::binary);
                    if (cmd_file.is_open()) {
                        std::string cmd;
                        char ch;
                        while (cmd_file.get(ch)) {
                            pinfo.cmdline += (ch == '\0' ? ' ' : ch);
                        }
                        if (!pinfo.cmdline.empty() && pinfo.cmdline.back() == ' ') {
                            pinfo.cmdline.pop_back();
                        }
                    }
                    if (pinfo.name.empty()) {
                        pinfo.name = pid_str;
                    }
                    process_info_read = true;
                }
                inode_map[inode] = pinfo;
            }
        }
        closedir(fd_dir);
    }
    closedir(proc_dir);
    return inode_map;
}

// Parse a single proc net tcp line
static std::optional<ConnectionEntry> ParseProcNetTcpLine(const std::string& line, 
                                                         bool is_ipv6,
                                                         const std::unordered_map<uint64_t, ProcessInfo>& inode_map,
                                                         SocketFilter filter) {
    // Format example:
    // sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
    //  0: 0100007F:0050 00000000:0000 0A ...                                  0        0 12345
    std::istringstream iss(line);
    std::string slot_col, local_col, rem_col, st_col;
    if (!(iss >> slot_col >> local_col >> rem_col >> st_col)) {
        return std::nullopt;
    }

    // Skip header line ("sl local_address ...")
    if (slot_col.find(':') == std::string::npos) {
        return std::nullopt;
    }

    // Parse state
    unsigned int state_hex = 0;
    if (std::sscanf(st_col.c_str(), "%X", &state_hex) != 1) {
        return std::nullopt;
    }
    TcpState state = HexToTcpState(static_cast<uint8_t>(state_hex));
    if (!MatchesFilter(state, filter)) {
        return std::nullopt;
    }

    // Parse local address and port
    size_t local_colon = local_col.find(':');
    if (local_colon == std::string::npos) return std::nullopt;
    std::string local_hex = local_col.substr(0, local_colon);
    unsigned int local_p = 0;
    std::sscanf(local_col.c_str() + local_colon + 1, "%X", &local_p);

    // Parse remote address and port
    size_t rem_colon = rem_col.find(':');
    if (rem_colon == std::string::npos) return std::nullopt;
    std::string rem_hex = rem_col.substr(0, rem_colon);
    unsigned int rem_p = 0;
    std::sscanf(rem_col.c_str() + rem_colon + 1, "%X", &rem_p);

    // Skip queue/timer columns to read uid and inode
    std::string txrx, tr_tm, retrnsmt, uid_str, timeout_str;
    uint64_t inode = 0;
    uint32_t uid = 0;
    if (iss >> txrx >> tr_tm >> retrnsmt >> uid_str >> timeout_str >> inode) {
        uid = std::strtoul(uid_str.c_str(), nullptr, 10);
    }

    ConnectionEntry entry;
    entry.protocol = is_ipv6 ? "tcp6" : "tcp";
    entry.local_addr = is_ipv6 ? ParseIpv6Address(local_hex) : ParseIpv4Address(local_hex);
    entry.local_port = static_cast<uint16_t>(local_p);
    entry.remote_addr = is_ipv6 ? ParseIpv6Address(rem_hex) : ParseIpv4Address(rem_hex);
    entry.remote_port = static_cast<uint16_t>(rem_p);
    entry.state = state;
    entry.state_name = TcpStateToString(state);
    entry.inode = inode;
    entry.uid = uid;

    // Lookup process
    auto it = inode_map.find(inode);
    if (it != inode_map.end()) {
        entry.pid = it->second.pid;
        entry.process_name = it->second.name;
        entry.cmdline = it->second.cmdline;
    }

    return entry;
}

static void ParseProcNetFile(const std::string& filepath, bool is_ipv6,
                             const std::unordered_map<uint64_t, ProcessInfo>& inode_map,
                             SocketFilter filter,
                             std::vector<ConnectionEntry>& out_entries) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto entry = ParseProcNetTcpLine(line, is_ipv6, inode_map, filter);
        if (entry.has_value()) {
            out_entries.push_back(std::move(*entry));
        }
    }
}

std::vector<ConnectionEntry> EnumerateConnections(SocketFilter filter) {
    std::vector<ConnectionEntry> results;
    // 1. Build map of socket inode to process info
    auto inode_map = BuildInodeProcessMap();

    // 2. Parse IPv4 TCP sockets (/proc/net/tcp)
    ParseProcNetFile("/proc/net/tcp", false, inode_map, filter, results);

    // 3. Parse IPv6 TCP sockets (/proc/net/tcp6)
    ParseProcNetFile("/proc/net/tcp6", true, inode_map, filter, results);

    // Stable sort: first LISTEN, then ESTABLISHED, then by local port
    std::stable_sort(results.begin(), results.end(), [](const ConnectionEntry& a, const ConnectionEntry& b) {
        if (a.state != b.state) {
            // Put LISTEN first, then ESTABLISHED
            if (a.state == TcpState::LISTEN) return true;
            if (b.state == TcpState::LISTEN) return false;
            return static_cast<int>(a.state) < static_cast<int>(b.state);
        }
        if (a.local_port != b.local_port) {
            return a.local_port < b.local_port;
        }
        return a.remote_port < b.remote_port;
    });

    return results;
}

// -----------------------------------------------------------------------------
// MACOS IMPLEMENTATION (libproc + proc_pidfdinfo)
// -----------------------------------------------------------------------------
#elif defined(__APPLE__)

std::vector<ConnectionEntry> EnumerateConnections(SocketFilter filter) {
    std::vector<ConnectionEntry> results;

    // 1. Get all active PIDs
    int num_pids = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
    if (num_pids <= 0) return results;

    std::vector<pid_t> pids(num_pids * 2);
    int bytes = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), pids.size() * sizeof(pid_t));
    if (bytes <= 0) return results;
    pids.resize(bytes / sizeof(pid_t));

    for (pid_t pid : pids) {
        if (pid <= 0) continue;

        // Get process name
        char name_buf[PROC_PIDPATHINFO_MAXSIZE] = {0};
        proc_name(pid, name_buf, sizeof(name_buf));
        std::string proc_name_str = name_buf;

        // List all file descriptors for this PID
        int fd_bytes = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, nullptr, 0);
        if (fd_bytes <= 0) continue;

        std::vector<struct proc_fdinfo> fds(fd_bytes / sizeof(struct proc_fdinfo));
        fd_bytes = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, fds.data(), fds.size() * sizeof(struct proc_fdinfo));
        if (fd_bytes <= 0) continue;
        fds.resize(fd_bytes / sizeof(struct proc_fdinfo));

        for (const auto& fd : fds) {
            if (fd.proc_fdtype != PROX_FDTYPE_SOCKET) continue;

            struct socket_fdinfo sinfo;
            int sbytes = proc_pidfdinfo(pid, fd.proc_fd, PROC_PIDFDSOCKETINFO, &sinfo, sizeof(sinfo));
            if (sbytes <= 0) continue;

            // Check if socket is TCP
            if (sinfo.psi.psi_desc.so_type != SOCK_STREAM) continue;
            int family = sinfo.psi.psi_desc.so_family;
            if (family != AF_INET && family != AF_INET6) continue;

            // Map TCP state from TSI_S_* constants
            int tsi_state = sinfo.psi.psi_proto.pri_tcp.tcps_state;
            TcpState state = TcpState::UNKNOWN;
            if (tsi_state == 1) state = TcpState::ESTABLISHED;
            else if (tsi_state == 10 || tsi_state == 1) state = TcpState::LISTEN; // TSI_S_LISTEN is 10
            else if (tsi_state == 2) state = TcpState::SYN_SENT;
            else if (tsi_state == 3) state = TcpState::SYN_RECV;
            else if (tsi_state == 4) state = TcpState::FIN_WAIT1;
            else if (tsi_state == 5) state = TcpState::FIN_WAIT2;
            else if (tsi_state == 6) state = TcpState::TIME_WAIT;
            else if (tsi_state == 7) state = TcpState::CLOSE;
            else if (tsi_state == 8) state = TcpState::CLOSE_WAIT;
            else if (tsi_state == 9) state = TcpState::LAST_ACK;
            else if (tsi_state == 11) state = TcpState::CLOSING;

            if (!MatchesFilter(state, filter)) continue;

            ConnectionEntry entry;
            entry.pid = pid;
            entry.process_name = proc_name_str;
            entry.state = state;
            entry.state_name = TcpStateToString(state);

            if (family == AF_INET) {
                entry.protocol = "tcp";
                char lbuf[INET_ADDRSTRLEN] = {0};
                char rbuf[INET_ADDRSTRLEN] = {0};
                struct in_addr laddr = sinfo.psi.psi_proto.pri_in.insi_laddr;
                struct in_addr faddr = sinfo.psi.psi_proto.pri_in.insi_faddr;
                inet_ntop(AF_INET, &laddr, lbuf, sizeof(lbuf));
                inet_ntop(AF_INET, &faddr, rbuf, sizeof(rbuf));
                entry.local_addr = lbuf;
                entry.remote_addr = rbuf;
                entry.local_port = ntohs(sinfo.psi.psi_proto.pri_in.insi_lport);
                entry.remote_port = ntohs(sinfo.psi.psi_proto.pri_in.insi_fport);
            } else {
                entry.protocol = "tcp6";
                char lbuf[INET6_ADDRSTRLEN] = {0};
                char rbuf[INET6_ADDRSTRLEN] = {0};
                struct in6_addr laddr6 = sinfo.psi.psi_proto.pri_in6.insi6_laddr;
                struct in6_addr faddr6 = sinfo.psi.psi_proto.pri_in6.insi6_faddr;
                inet_ntop(AF_INET6, &laddr6, lbuf, sizeof(lbuf));
                inet_ntop(AF_INET6, &faddr6, rbuf, sizeof(rbuf));
                entry.local_addr = lbuf;
                entry.remote_addr = rbuf;
                entry.local_port = ntohs(sinfo.psi.psi_proto.pri_in6.insi6_lport);
                entry.remote_port = ntohs(sinfo.psi.psi_proto.pri_in6.insi6_fport);
            }

            results.push_back(std::move(entry));
        }
    }

    std::stable_sort(results.begin(), results.end(), [](const ConnectionEntry& a, const ConnectionEntry& b) {
        if (a.state != b.state) return a.state == TcpState::LISTEN;
        return a.local_port < b.local_port;
    });

    return results;
}

// -----------------------------------------------------------------------------
// BSD IMPLEMENTATION (FreeBSD / OpenBSD / NetBSD via sysctl pcblist)
// -----------------------------------------------------------------------------
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)

std::vector<ConnectionEntry> EnumerateConnections(SocketFilter filter) {
    std::vector<ConnectionEntry> results;

    size_t len = 0;
    const char* mib_name = "net.inet.tcp.pcblist";
    if (sysctlbyname(mib_name, nullptr, &len, nullptr, 0) < 0 || len == 0) {
        return results;
    }

    std::vector<char> buf(len);
    if (sysctlbyname(mib_name, buf.data(), &len, nullptr, 0) < 0) {
        return results;
    }

#if defined(__FreeBSD__)
    // Iterate xinpgen / xtcpcb headers on FreeBSD
    struct xinpgen* xig = reinterpret_cast<struct xinpgen*>(buf.data());
    struct xinpgen* oxig = xig;

    for (xig = reinterpret_cast<struct xinpgen*>(reinterpret_cast<char*>(xig) + xig->xiph_len);
         xig->xiph_len > sizeof(struct xinpgen);
         xig = reinterpret_cast<struct xinpgen*>(reinterpret_cast<char*>(xig) + xig->xiph_len)) {
        
        struct xtcpcb* tp = reinterpret_cast<struct xtcpcb*>(xig);
        int t_state = tp->t_state;

        TcpState state = TcpState::UNKNOWN;
        if (t_state == TCPS_ESTABLISHED) state = TcpState::ESTABLISHED;
        else if (t_state == TCPS_LISTEN) state = TcpState::LISTEN;
        else if (t_state == TCPS_SYN_SENT) state = TcpState::SYN_SENT;
        else if (t_state == TCPS_SYN_RECEIVED) state = TcpState::SYN_RECV;
        else if (t_state == TCPS_FIN_WAIT_1) state = TcpState::FIN_WAIT1;
        else if (t_state == TCPS_FIN_WAIT_2) state = TcpState::FIN_WAIT2;
        else if (t_state == TCPS_TIME_WAIT) state = TcpState::TIME_WAIT;
        else if (t_state == TCPS_CLOSED) state = TcpState::CLOSE;
        else if (t_state == TCPS_CLOSE_WAIT) state = TcpState::CLOSE_WAIT;
        else if (t_state == TCPS_LAST_ACK) state = TcpState::LAST_ACK;
        else if (t_state == TCPS_CLOSING) state = TcpState::CLOSING;

        if (!MatchesFilter(state, filter)) continue;

        ConnectionEntry entry;
        entry.state = state;
        entry.state_name = TcpStateToString(state);

        if (tp->xt_inp.inp_vflag & INP_IPV6) {
            entry.protocol = "tcp6";
            char lbuf[INET6_ADDRSTRLEN] = {0};
            char rbuf[INET6_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET6, &tp->xt_inp.in6p_laddr, lbuf, sizeof(lbuf));
            inet_ntop(AF_INET6, &tp->xt_inp.in6p_faddr, rbuf, sizeof(rbuf));
            entry.local_addr = lbuf;
            entry.remote_addr = rbuf;
        } else {
            entry.protocol = "tcp";
            char lbuf[INET_ADDRSTRLEN] = {0};
            char rbuf[INET_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET, &tp->xt_inp.inp_laddr, lbuf, sizeof(lbuf));
            inet_ntop(AF_INET, &tp->xt_inp.inp_faddr, rbuf, sizeof(rbuf));
            entry.local_addr = lbuf;
            entry.remote_addr = rbuf;
        }
        entry.local_port = ntohs(tp->xt_inp.inp_lport);
        entry.remote_port = ntohs(tp->xt_inp.inp_fport);

        results.push_back(std::move(entry));
    }
#endif

    return results;
}

#else
// Fallback for unsupported platforms
std::vector<ConnectionEntry> EnumerateConnections(SocketFilter filter) {
    return {};
}
#endif

// -----------------------------------------------------------------------------
// FAR2L VMENU FORMATTING SPECIFICATION & UTILITIES
// -----------------------------------------------------------------------------
static std::string PadOrTruncate(const std::string& str, size_t width) {
    if (str.length() == width) {
        return str;
    }
    if (str.length() > width) {
        if (width > 3) {
            return str.substr(0, width - 2) + "..";
        }
        return str.substr(0, width);
    }
    return str + std::string(width - str.length(), ' ');
}

std::string FormatVMenuHeader(const Far2lColumnSizes& sizes, bool use_box_chars) {
    const std::string sep = use_box_chars ? "│" : "|";
    std::ostringstream oss;
    oss << PadOrTruncate("Proto", sizes.proto_width)   << " " << sep << " "
        << PadOrTruncate("State", sizes.state_width)   << " " << sep << " "
        << PadOrTruncate("Local Address", sizes.local_width) << " " << sep << " "
        << PadOrTruncate("Remote Address", sizes.remote_width) << " " << sep << " "
        << PadOrTruncate("PID/Program", sizes.process_width);
    return oss.str();
}

std::string FormatVMenuSeparator(const Far2lColumnSizes& sizes, bool use_box_chars) {
    const std::string h = use_box_chars ? "─" : "-";
    const std::string cross = use_box_chars ? "┼" : "+";
    
    auto repeat_str = [](const std::string& s, size_t count) {
        std::string res;
        for (size_t i = 0; i < count; ++i) res += s;
        return res;
    };

    std::ostringstream oss;
    oss << repeat_str(h, sizes.proto_width + 1) << cross
        << repeat_str(h, sizes.state_width + 2) << cross
        << repeat_str(h, sizes.local_width + 2) << cross
        << repeat_str(h, sizes.remote_width + 2) << cross
        << repeat_str(h, sizes.process_width + 1);
    return oss.str();
}

std::string FormatVMenuRow(const ConnectionEntry& entry, const Far2lColumnSizes& sizes, bool use_box_chars) {
    const std::string sep = use_box_chars ? "│" : "|";
    std::ostringstream oss;
    oss << PadOrTruncate(entry.protocol, sizes.proto_width) << " " << sep << " "
        << PadOrTruncate(entry.state_name, sizes.state_width) << " " << sep << " "
        << PadOrTruncate(entry.GetLocalEndpoint(), sizes.local_width) << " " << sep << " "
        << PadOrTruncate(entry.GetRemoteEndpoint(), sizes.remote_width) << " " << sep << " "
        << PadOrTruncate(entry.GetProcessDisplay(), sizes.process_width);
    return oss.str();
}

void PrintConnections(const std::vector<ConnectionEntry>& entries, 
                      const Far2lColumnSizes& sizes,
                      bool use_box_chars) {
    std::string header = FormatVMenuHeader(sizes, use_box_chars);
    std::string sep = FormatVMenuSeparator(sizes, use_box_chars);

    std::printf("%s\n", header.c_str());
    std::printf("%s\n", sep.c_str());

    for (const auto& entry : entries) {
        std::string row = FormatVMenuRow(entry, sizes, use_box_chars);
        std::printf("%s\n", row.c_str());
    }

    std::printf("%s\n", sep.c_str());
    std::printf("Total sockets: %zu\n", entries.size());
}

std::vector<std::wstring> FormatConnections(const std::vector<ConnectionEntry>& entries, 
                      const Far2lColumnSizes& sizes,
                      bool use_box_chars) {
	std::vector<std::wstring> v;
    for (const auto& entry : entries) {
        std::string row = FormatVMenuRow(entry, sizes, use_box_chars);
    	std::wstring ws;
    	StrMB2Wide(row, ws);
        v.push_back(ws);
    }
	return v;
}


} // namespace NetstatFar2l
