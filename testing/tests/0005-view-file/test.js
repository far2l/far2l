LoadJS("../common.js");
var dirs = SetupTestDirs();

StartTestApp(dirs.profile, dirs.left, dirs.right);
var status = AppStatus();
DismissHelpAndOSC52();

// Cycle panel focus to ensure the file panel is active.
// Always runs — needed when OSC52 focus left focus in the wrong panel,
// and harmless (returns to file panel) when OSC52 was absent.
TypeVK(9);
Sleep(200);
TypeVK(9);
Sleep(200);
Sync(5000);

// Navigate to viewme.txt and open it with F3
TypeDown()
TypeFKey(3)
ExpectString("left/viewme.txt", 0, 0, -1, -1, 10000)
Sync(10000)

// Scroll through the file and verify each page
TypePageDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test1.txt')

TypePageDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test2.txt')

TypeDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test3.txt')

TypeHome()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test4.txt')

// Search for ::setselectpos and verify the result
TypeFKey(7)
ExpectString("═══ Search ═══", 0, 0, -1, -1, 10000)
TypeText("::setselectpos")
TypeEnter()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test5.txt')

TypeUp()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test6.txt')

TypeUp()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test7.txt')

TypeEscape()

ExitFar2lWithConfirm()
