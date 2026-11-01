#pragma once

#include <string>

// Filesystem type of the mount containing 'path' (longest mountpoint prefix
// match), or empty string. Parses /proc/self/mountinfo on Linux only; that
// never touches the target filesystem, so it is safe on a stalled mount.
std::string MountFsType(const std::string &path);

// True for VM shared folder filesystems: even a plain lookup can block for
// minutes when the host side stalls.
bool IsSharedFolderFs(const std::string &fstype);

// True for network filesystems: every lookup is a round trip to a server
// that may be gone or unreachable.
bool IsNetworkFs(const std::string &fstype);

// True for any filesystem whose lookups can block in kernel D state when the
// backing host/server stalls; use it to decide whether an operation on 'path'
// must be gated by MountResponsive().
bool NeedsMountProbe(const std::string &fstype);

// Returns false if probing 'path' (open, readdir, stat entries - the scan is
// self-limited to a fraction of timeout_ms so a large but alive directory is
// not rejected for being slow) does not complete within timeout_ms. The probe
// runs in a child that can never stall the caller and fails open if it cannot
// run. Verdicts are memoized per (path, timeout) for ~10 s with probe errno.
bool MountResponsive(const char *path, unsigned timeout_ms = 2000);
