// 0103-stale-mount-cd: far2l must refuse cd into a stalled folder with an
// error message and stay alive/operable (plan .ai-factory/PLAN.md; mechanism
// .ai-factory/reports/2026-11-01-stale-folder-cd-gdb.txt).
//
// Requires the stall-shim environment built from this directory (README.txt):
//   LD_PRELOAD=<abs>/stall.so STALL_SHIM_MOUNTINFO=<abs>/mountinfo.vboxsf
// Caller prerequisite: mkdir -p /tmp/far2l-stale/deep (the harness itself
// carries LD_PRELOAD and must never touch the magic dir).
// The shim stalls every lookup under /tmp/far2l-stale/deep (libc and the raw
// syscalls MountResponsive's probe child uses) and fakes /proc/self/mountinfo
// so the production MountFsType/NeedsMountProbe gate engages.

const magicdir = "/tmp/far2l-stale/deep"

// Self-contained fake mountinfo: "/" is ext4 (gates off), /tmp/far2l-stale is
// vboxsf (gates on). Written idempotently before the app starts.
SaveTextFile("/tmp/far2l-stall-shim/mountinfo.vboxsf", [
	"30 25 0:30 / / rw,relatime - ext4 /dev/root rw",
	"900 30 0:900 / /tmp/far2l-stale rw,relatime shared:900 - vboxsf stalled rw"])

mydir = WorkDir()
profile = mydir + "/profile"

// Run 1: startup -cd into the stalled directory. The gate refuses silently
// (pre-UI Message is guarded away) and far2l must boot operable at the
// fallback dir; if the gates hang, the help never arrives and we time out.
// Quitting through the F10 dialog proves the UI stayed interactive.
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", magicdir])
ExpectString("Help - FAR2L", 0, 0, -1, -1, 30000)
TypeEscape()
ExpectString("OSC52", 0, 0, -1, -1, 20000)
TypeEscape()
TypeFKey(10)
ExpectString("Do you want to quit FAR?", 0, 0, -1, -1, 10000)
TypeEnter()
ExpectAppExit(0, 10000)

Sleep(3000)	// let the harness reap run 1 before restarting

// Run 2 (same profile): panel navigation into the stalled tree. The panel
// starts at /tmp (responsive, ext4); navigating onto far2l-stale hits the
// FarChDir gate, whose probe stalls on the per-entry stat of the magic dir
// and refuses with the Ignore/Retry dialog. No VT session is involved, so
// the F10 quit works afterwards (0102-proven context).
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", "/tmp"])
ExpectString("/tmp ═", 0, 0, -1, -1, 30000)

DblClickWhereFound(ExpectString("far2l-stale"))	// navigate onto the stalled tree
ExpectString("timed out", 0, 0, -1, -1, 30000)
ExpectString("/tmp/far2l-stale", 0, 0, -1, -1, 5000)
TypeEnter()	// Ignore: keep the current folder

TypeFKey(10)
ExpectString("Do you want to quit FAR?", 0, 0, -1, -1, 10000)
TypeEnter()
ExpectAppExit(0, 10000)

0;
