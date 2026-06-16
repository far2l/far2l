mydir=WorkDir()
profile=mydir + "/profile"
left=mydir + "/left"
right=mydir + "/right"
MkdirsAll([profile, left, right], 0700)

StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", left, "-cd", right]);
ExpectString("left", 0, 0, -1, -1, 10000);
ExpectString("Help - FAR2L", 0, 0, -1, -1, 10000);
status = AppStatus();

TypeEscape()
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
// Cycle panel focus to ensure the file panel is active.
// Always runs — needed when OSC52 focus left focus in the wrong panel,
// and harmless (returns to file panel) when OSC52 was absent.
TypeVK(9);
Sleep(200);
TypeVK(9);
Sleep(200);
Sync(5000);
TypeDown()
TypeFKey(3)
ExpectString("left/viewme.txt", 0, 0, -1, -1, 10000)

Sync(10000)

TypePageDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, mydir + '/test1.txt')

TypePageDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, mydir + '/test2.txt')

TypeDown()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, mydir + '/test3.txt')

TypeHome()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, mydir + '/test4.txt')

TypeFKey(7)
ExpectString("═══ Search ═══", 0, 0, -1, -1, 10000)
TypeText("::setselectpos")
TypeEnter()

Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, mydir + '/test5.txt')

TypeUp()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, mydir + '/test6.txt')

TypeUp()
Sync(10000)
BoundedLinesMatchTextFile(0, 1, -1, status.Height - 2, mydir + '/test7.txt')

TypeEscape()

TypeFKey(10)
ExpectString("Do you want to quit FAR?", 0, 0, -1, -1, 10000)
TypeEnter()
ExpectAppExit(0, 10000)
0;
