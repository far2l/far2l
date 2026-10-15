#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <sys/types.h>

namespace NetstatFar2l {

// Standard TCP States matching RFC 793 and Linux /proc/net/tcp
enum class TcpState : uint8_t {
    UNKNOWN     = 0,
    ESTABLISHED = 1,
    SYN_SENT    = 2,
    SYN_RECV    = 3,
    FIN_WAIT1   = 4,
    FIN_WAIT2   = 5,
    TIME_WAIT   = 6,
    CLOSE       = 7,
    CLOSE_WAIT  = 8,
    LAST_ACK    = 9,
    LISTEN      = 10,
    CLOSING     = 11
};

// Filter options for enumeration
enum class SocketFilter {
    OPEN_AND_CONNECTED, // Listening + Established (User request default)
    LISTEN_ONLY,        // Only Listening (Open sockets)
    ESTABLISHED_ONLY,   // Only Established (Connected sockets)
    ALL_STATES          // All TCP socket states
};

// Information about a process that owns a socket
struct ProcessInfo {
    pid_t pid = -1;
    std::string name;     // Short process name (e.g. "node", "far2l")
    std::string cmdline;  // Full command line if available
};

// A single network connection entry
struct ConnectionEntry {
    std::string protocol;    // "tcp" or "tcp6"
    std::string local_addr;  // e.g. "0.0.0.0", "127.0.0.1", "::", "::1"
    uint16_t local_port = 0;
    std::string remote_addr; // e.g. "0.0.0.0", "192.168.1.10", "*", "::"
    uint16_t remote_port = 0;
    TcpState state = TcpState::UNKNOWN;
    std::string state_name;  // "LISTEN", "ESTABLISHED", etc.
    uint64_t inode = 0;      // Socket inode number
    uint32_t uid = 0;        // Socket owner user ID
    
    // Process discovery
    pid_t pid = -1;          // Process ID (-1 if unprivileged or unknown)
    std::string process_name;// e.g. "sshd", "far2l"
    std::string cmdline;     // Full command line
    
    // Convenient helpers
    std::string GetLocalEndpoint() const;
    std::string GetRemoteEndpoint() const;
    std::string GetProcessDisplay() const; // e.g. "44/node" or "-"
};

// Column width specifications tailored for far2l VMenu dialogs
struct Far2lColumnSizes {
    // Widths are tuned for terminal displays.
    // In far2l, a standard terminal window is often 80 to 120 columns wide.
    // Box-drawing characters (│, ║, |) or spaces can be used as separators.
    
    // Normal format (total row width ~ 84 chars with single separators, 89 with spaces):
    int proto_width   = 5;   // "tcp", "tcp6"
    int state_width   = 11;  // "LISTEN", "ESTABLISHED"
    int local_width   = 22;  // "127.0.0.1:3000", "[::1]:8080"
    int remote_width  = 22;  // "192.168.1.50:443", "*:*"
    int process_width = 18;  // "1234/far2l", "44/node"
    
    // Compact format (fits cleanly inside standard 80-column terminal dialog with frame):
    // 5 + 1 + 11 + 1 + 21 + 1 + 21 + 1 + 16 = 78 chars
    static Far2lColumnSizes Standard() {
        return Far2lColumnSizes{5, 11, 23, 23, 18};
    }
    
    static Far2lColumnSizes Compact() {
        return Far2lColumnSizes{5, 11, 21, 21, 16};
    }
    
    static Far2lColumnSizes Wide() {
        return Far2lColumnSizes{6, 12, 28, 28, 24};
    }
};

// Core enumeration API
// Zero external dependencies. Reads /proc/net/tcp & /proc/net/tcp6 and /proc/[pid]/fd on Linux,
// sysctl/libproc on macOS, sysctl on BSD.
std::vector<ConnectionEntry> EnumerateConnections(SocketFilter filter = SocketFilter::OPEN_AND_CONNECTED);

// Format helpers specifically designed for far2l VMenu items and text tables
std::string FormatVMenuHeader(const Far2lColumnSizes& sizes = Far2lColumnSizes::Standard(), bool use_box_chars = true);
std::string FormatVMenuSeparator(const Far2lColumnSizes& sizes = Far2lColumnSizes::Standard(), bool use_box_chars = true);
std::string FormatVMenuRow(const ConnectionEntry& entry, const Far2lColumnSizes& sizes = Far2lColumnSizes::Standard(), bool use_box_chars = true);

// Direct printf table output to stdout
void PrintConnections(const std::vector<ConnectionEntry>& entries, 
                      const Far2lColumnSizes& sizes = Far2lColumnSizes::Standard(),
                      bool use_box_chars = true);

std::vector<std::wstring> FormatConnections(const std::vector<ConnectionEntry>& entries, 
                      const Far2lColumnSizes& sizes = Far2lColumnSizes::Standard(),
                      bool use_box_chars = true);

// Utility functions
const char* TcpStateToString(TcpState state);
TcpState HexToTcpState(uint8_t hex_state);

} // namespace NetstatFar2l
