LoadJS("../common.js");
var dirs = SetupTestDirs("left-fgdfgfd", "right");

///////////////////
// First start - skip Help window and OSC52 dialog, press F10 expecting exit confirmation dialog
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-fgdfgfd");
DismissHelpAndOSC52();
status = AppStatus();
ExitFar2lWithConfirm();

///////////////////
// Second start - there should no Help appeared automatically
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-fgdfgfd", false);
ExitFar2lWithConfirm();

// Now lets disable exit confirmation and save settings
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-fgdfgfd", false);

// disable exit confirmation
TypeFKey(9)
ExpectString("Left    Files    Commands    Options    Right", 0, 0, -1, -1, 10000);
TypeText("on")
ExpectString("══ Confirmations ══", 0, 0, -1, -1, 10000);
ToggleLAlt(true)
TypeText("x")
ToggleLAlt(false)
TypeEnter()
ExpectNoString("══ Confirmations ══", 0, 0, -1, -1, 10000);

// Shift+F9 to save settings
ToggleShift(true)
TypeFKey(9)
ToggleShift(false)
ExpectString("══ Save setup ══", 0, 0, -1, -1, 10000);
TypeEnter()
ExpectNoString("══ Save setup ══", 0, 0, -1, -1, 10000);

// exit without confirmation now
TypeFKey(10)
ExpectAppExit(0, 10000)

///////////////////
// Third run just start and exit expecting no confirmation as such settings were saved
StartTestApp(dirs.profile, dirs.left, dirs.right, "left-fgdfgfd", false);
// exit without confirmation again
TypeFKey(10)
ExpectAppExit(0, 10000)
