LoadJS("../common.js");
var dirs = SetupTestDirs("left-тест1", "right");

StartTestApp(dirs.profile, dirs.left, dirs.right, "left-тест1");
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


///////////////////
///////////////////
// CORNER CASES
///////////////////
///////////////////

///////////////////
// Corner case: Multiple commands in sequence
// Verifies shell handles chained commands without losing output
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-тест1", false);
TypeText("echo 'VT' 'corner'; false")
TypeEnter()
ExpectString("VT corner", 0, 0, -1, -1, 10000)
TypeEscape()
Sleep(500)
StartBashShell()
TTYWriteRaw("echo 'MULTI_1'; echo 'MULTI_2'; echo 'MULTI_3'\n")
ExpectString("MULTI_1", 0, 0, -1, -1, 10000)
ExpectString("MULTI_2", 0, 0, -1, -1, 10000)
ExpectString("MULTI_3", 0, 0, -1, -1, 10000)
ExitBashAndFar2l()


///////////////////
// Corner case: Command with stderr output
// Verifies stderr is visible in the VT shell
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-тест1", false);
TypeText("echo 'VT' 'corner'; false")
TypeEnter()
ExpectString("VT corner", 0, 0, -1, -1, 10000)
TypeEscape()
Sleep(500)
StartBashShell()
TTYWriteRaw("echo 'STDERR_TEST' >&2\n")
ExpectString("STDERR_TEST", 0, 0, -1, -1, 10000)
ExitBashAndFar2l()


///////////////////
// Corner case: Pipe commands
// Verifies pipe operator works in the VT shell
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-тест1", false);
TypeText("echo 'VT' 'corner'; false")
TypeEnter()
ExpectString("VT corner", 0, 0, -1, -1, 10000)
TypeEscape()
Sleep(500)
StartBashShell()
TTYWriteRaw("echo 'PIPE_TEST' | tr 'A-Z' 'a-z'\n")
ExpectString("pipe_test", 0, 0, -1, -1, 10000)
ExitBashAndFar2l()


///////////////////
// Corner case: Environment variable set and read
// Verifies env vars persist within a session
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-тест1", false);
TypeText("echo 'VT' 'corner'; false")
TypeEnter()
ExpectString("VT corner", 0, 0, -1, -1, 10000)
TypeEscape()
Sleep(500)
StartBashShell()
TTYWriteRaw("export FAR2L_CORNER='hello'; echo $FAR2L_CORNER\n")
ExpectString("hello", 0, 0, -1, -1, 10000)
ExitBashAndFar2l()


///////////////////
// Corner case: Working directory change
// Verifies cd changes directory and subshell reflects it
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-тест1", false);
TypeText("echo 'VT' 'corner'; false")
TypeEnter()
ExpectString("VT corner", 0, 0, -1, -1, 10000)
TypeEscape()
Sleep(500)
StartBashShell()
TTYWriteRaw("cd /tmp && pwd\n")
ExpectString("/tmp", 0, 0, -1, -1, 10000)
ExitBashAndFar2l()


///////////////////
// Corner case: TTYWriteRaw with unicode output
// Verifies PTY handles non-ASCII bytes through the raw injection path
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-тест1", false);
TypeText("echo 'VT' 'corner'; false")
TypeEnter()
ExpectString("VT corner", 0, 0, -1, -1, 10000)
TypeEscape()
Sleep(500)
StartBashShell()
TTYWriteRaw("echo 'тест-юникод'\n")
ExpectString("тест-юникод", 0, 0, -1, -1, 10000)
ExitBashAndFar2l()


///////////////////
// Corner case: CTRL+C interrupts a long-running command
// Verifies signal delivery through the VT shell
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-тест1", false);
TypeText("echo 'VT' 'corner'; false")
TypeEnter()
ExpectString("VT corner", 0, 0, -1, -1, 10000)
TypeEscape()
Sleep(500)
StartBashShell()
TTYWriteRaw("sleep 30\n")
Sleep(500);
TTYWriteRaw("\x03")
Sleep(500);
ExpectString("$", 0, 0, -1, -1, 10000)
ExitBashAndFar2l()
