LoadJS("../common.js");
var dirs = SetupTestDirs("left-fgdfgfd", "right");

StartTestApp(dirs.profile, dirs.left, dirs.right, "left-fgdfgfd");
DismissHelpAndOSC52();

// Run the built-in VT shell smoke test (echo + false triggers "Press any key")
TypeText("echo 'VT' 'Shell' 'smoke' 'test'; false")
TypeEnter()
ExpectString("VT Shell smoke test", 0, 0, -1, -1, 10000)
ExpectString("~~~~~~~~~~~~~~~~~~~", 0, 0, -1, -1, 10000)

// Dismiss "Press any key" prompt from the smoke test's 'false' command
TypeEscape()
Sleep(500)

// Start an interactive bash session so the PTY stays open for TTYWriteRaw
StartBashShell()

// Verify basic TTYWriteRaw injects a command that bash reads and executes
TTYWriteRaw("echo 'RAW_WRITE_OK'\n")
ExpectString("RAW_WRITE_OK", 0, 0, -1, -1, 10000)

// Exit interactive bash, dismiss VT output, exit far2l
ExitBashAndFar2l()
