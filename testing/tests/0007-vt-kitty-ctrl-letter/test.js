mydir=WorkDir()
profile=mydir + "/profile"
left=mydir + "/left"
right=mydir + "/right"
MkdirsAll([profile, left, right], 0700)
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", left, "-cd", right]);
ExpectString("left", 0, 0, -1, -1, 10000);
ExpectString("Help - FAR2L", 0, 0, -1, -1, 10000);

// Enable kitty keyboard protocol (CSI = 1 u, i.e. ESC [ = 1 u), then run cat and wait for EOF.
// If Ctrl+D is sent as raw control byte (0x04), cat exits and the marker is printed.
// If Ctrl+D is sent as kitty sequence (\e[4;5u), cat never exits and the test times out.
TypeEscape(10)
Sync(5000)
// Dismiss OSC52 clipboard dialog if present (first start only)
BeCalm()
var r = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic()
if (r.I < 1) {
    TypeEnter();
    Sleep(500);
    Sync(5000);
}
TypeText("printf '\\033[=1u' > /dev/tty; cat; echo \"kittyeof\" | tr 'a-z' 'A-Z'")
TypeEnter()
Sleep(500)
ToggleLCtrl(true)
TypeVK(0x44)
ToggleLCtrl(false)
ExpectString("KITTYEOF", 0, 0, -1, -1, 10000)
Sync(5000)
// Wait for far2l to return to panel mode before typing exit command.
// Sync only drains the input queue, but VT shell teardown and panel
// redraw happen asynchronously and can race with TypeText.
ExpectString("left", 0, 0, -1, -1, 10000)
Sleep(500)
TypeText("exit far")
TypeEnter()
ExpectAppExit(0, 10000)
