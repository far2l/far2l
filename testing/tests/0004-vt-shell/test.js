mydir=WorkDir()
profile=mydir + "/profile"
left=mydir + "/left-fgdfgfd"
right=mydir + "/right"
MkdirsAll([profile, left, right], 0700)
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", left, "-cd", right]);
ExpectString("left-fgdfgfd", 0, 0, -1, -1, 10000);
ExpectString("Help - FAR2L", 0, 0, -1, -1, 10000);
TypeEscape(10)
Sync(5000)
// Dismiss OSC52 clipboard dialog if present (first start only)
BeCalm()
var r = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic()
if (r.I < 1) {
    TypeEnter();
    Sleep(500);
}
TypeText("echo 'VT' 'Shell' 'smoke' 'test'; false")
TypeEnter()
ExpectString("VT Shell smoke test", 0, 0, -1, -1, 10000)
ExpectString("~~~~~~~~~~~~~~~~~~~", 0, 0, -1, -1, 10000)

// Dismiss "Press any key" prompt from the smoke test's 'false' command
TypeEscape()
Sleep(500)

// Start an interactive bash session so the PTY stays open for TTYWriteRaw.
TypeText("echo 'SHELL_READY'; exec bash --norc --noprofile")
TypeEnter()
ExpectString("SHELL_READY", 0, 0, -1, -1, 10000)
Sync(5000)
Sleep(1000)

// Verify basic TTYWriteRaw injects a command that bash reads and executes.
TTYWriteRaw("echo 'RAW_WRITE_OK'\n")
ExpectString("RAW_WRITE_OK", 0, 0, -1, -1, 10000)

// Exit interactive bash, dismiss VT output, exit far2l
TTYWriteRaw("exit\n")
Sync(5000)
Sleep(1000)
TypeEscape()
TypeText("exit far")
TypeEnter()
ExpectAppExit(0, 10000)
