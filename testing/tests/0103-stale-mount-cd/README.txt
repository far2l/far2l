0103-stale-mount-cd — regression test: cd into a stalled folder must not
terminate or wedge far2l; it refuses with an error message and stays operable.

Dev-only repro tooling (never shipped in the product):

  mkdir -p /tmp/far2l-stall-shim
  gcc -shared -fPIC -O1 -o /tmp/far2l-stall-shim/stall.so stall_shim.c -ldl

The shim (LD_PRELOAD) simulates a stalled vboxsf mount on /tmp/far2l-stale/deep:
libc stat/open/chdir and the raw syscalls MountResponsive's probe child uses
(SYS_openat/getdents64/newfstatat with fd tracking) sleep forever for that dir,
and STALL_SHIM_MOUNTINFO serves a fake mountinfo mapping /tmp/far2l-stale to
fstype "vboxsf" so the production gate engages. Everything else passes through.

Prerequisite: the stalled target must exist before the run (create it as the
caller, never from inside the test - the harness carries LD_PRELOAD too):

  mkdir -p /tmp/far2l-stale/deep

Run (from the repo testing/ directory):

  LD_PRELOAD=/tmp/far2l-stall-shim/stall.so \
  STALL_SHIM_MOUNTINFO=/tmp/far2l-stall-shim/mountinfo.vboxsf \
  STALL_SHIM_LOG=/tmp/far2l-stall-shim/shim.log \
  ./far2l-smoke-run.sh <path-to-far2l> 0103-stale-mount-cd

The harness inherits the caller's environment into far2l (testing/src/main.go
passes os.Environ()). The test itself writes only the fake mountinfo file
(/tmp/far2l-stall-shim/mountinfo.vboxsf); /tmp/far2l-stale/deep must already
exist by then, as created by the caller per the prerequisite above. Ground
truth for the simulated defect:
.ai-factory/reports/2026-11-01-stale-folder-cd-gdb.txt
