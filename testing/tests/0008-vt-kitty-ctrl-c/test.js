mydir=WorkDir()
profile=mydir + "/profile"
left=mydir + "/left"
right=mydir + "/right"
MkdirsAll([profile, left, right], 0700)
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", left, "-cd", right]);
ExpectString("left", 0, 0, -1, -1, 10000);
ExpectString("Help - FAR2L", 0, 0, -1, -1, 10000);

// Dismiss Help and OSC52 dialog
TypeEscape(10)
Sync(5000)
BeCalm()
var r = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic()
if (r.I < 1) {
    TypeEnter();
    Sleep(500);
    Sync(5000);
}

// Start an interactive bash session so the VT shell stays alive across
// multiple raw key injections (Ctrl+D, Ctrl+C).
TypeText("echo 'READY'; exec bash --norc --noprofile")
TypeEnter()
ExpectString("READY", 0, 0, -1, -1, 10000)
Sync(5000)
Sleep(1000)

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

// Exit interactive bash, dismiss VT output, exit far2l
TTYWriteRaw("exit\n")
Sync(5000)
Sleep(1000)
TypeEscape()
TypeText("exit far")
TypeEnter()
ExpectAppExit(0, 10000)
0;
