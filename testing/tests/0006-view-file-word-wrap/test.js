LoadJS("../common.js");
var dirs = SetupTestDirs();

StartTestApp(dirs.profile, dirs.left, dirs.right, null, true, [95, 24]);
var status = AppStatus();
DismissHelpAndOSC52();

// Navigate to viewme.txt and open it with F3, then toggle word wrap (Shift+F2)
TypeDown()
TypeFKey(3)
ExpectString("left/viewme.txt", 0, 0, -1, -1, 10000)
Sync(10000)
ToggleShift(true)
TypeFKey(2)
ToggleShift(false)

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

// Go back to top, search for VMenu::SetUserData and verify
TypeHome()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test4.txt')

TypeFKey(7)
ExpectString("═══ Search ═══", 0, 0, -1, -1, 10000)
TypeText("VMenu::SetUserData")
TypeEnter()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test8.txt')

TypeUp()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test9.txt')

TypeUp()
TypeUp()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test10.txt')

TypeDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test11.txt')

TypeDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test12.txt')

TypeDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test13.txt')

TypePageDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test14.txt')

TypePageUp()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, dirs.mydir + '/test15.txt')

TypeEscape()

ExitFar2lWithConfirm()
