LoadJS("../common.js");
var dirs = SetupTestDirs();

StartTestApp(dirs.profile, dirs.left, dirs.right);
DismissHelpAndOSC52();

// Start an interactive bash session so the VT shell stays alive across
// multiple raw key injections (Ctrl+D, Ctrl+C).
StartBashShell();

// Enable kitty keyboard protocol (CSI = 1 u)
TTYWriteRaw("printf '\\033[=1u' > /dev/tty\n")
Sync(5000)
Sleep(500)

// Test 1: Ctrl+D as raw 0x04 exits cat
TTYWriteRaw("cat; echo KITTYEOF\n")
Sleep(500)
ToggleLCtrl(true)
TypeVK(0x44)
ToggleLCtrl(false)
ExpectString("KITTYEOF", 0, 0, -1, -1, 10000)

// Test 2: Ctrl+C as raw 0x03 kills cat with SIGINT
TTYWriteRaw("cat; echo CAT_INTERRUPTED\n")
Sleep(500)
ToggleLCtrl(true)
TypeVK(0x43)
ToggleLCtrl(false)
ExpectString("CAT_INTERRUPTED", 0, 0, -1, -1, 10000)

ExitBashAndFar2l()
