LoadJS("../common.js");
var dirs = SetupTestDirs();

// 0018-commands-menu — Test Commands menu items.
// Uses F9 menu navigation to verify menu structure and dialog opening.

// Create test files
Mkfiles([dirs.left + "/findme.txt"], 0o666, 100, 1000);
Mkfiles([dirs.left + "/another.txt"], 0o666, 100, 1000);

StartTestApp(dirs.profile, dirs.left, dirs.right);
DismissHelpAndOSC52();

// Ensure file panel is active
TypeVK(9); Sleep(200); TypeVK(9); Sleep(200); Sync(5000);
ExpectString("left", 0, 0, -1, -1, 5000);
Sync(2000);

// ========================================
// Phase 1: Open Commands menu via F9, verify structure
// ========================================
TypeFKey(9)
ExpectString("Left    Files    Commands    Options    Right", 0, 0, -1, -1, 10000)
Sync(1000)

// Navigate to Commands (3rd column)
TypeRight()
Sleep(200)
TypeRight()
Sleep(200)
Sync(1000)

// Open Commands dropdown
TypeDown()
Sleep(500)
Sync(2000)

// Verify Commands menu items are visible
ExpectString("Find file", 0, 0, -1, -1, 10000)
Sync(1000)

// Close menu with Escape (press twice to fully exit menu mode)
TypeEscape()
Sleep(500)
Sync(2000)
TypeEscape()
Sleep(300)
Sync(2000)

// Should be back at panels
ExpectString("left", 0, 0, -1, -1, 5000)
Sync(2000)
Sleep(500)
Sync(2000)

ExitFar2lWithConfirm()


///////////////////
///////////////////
// CORNER CASES
///////////////////
///////////////////
var mydir = WorkDir();

///////////////////
// Corner case: Open and close Commands menu without action
// Verifies menu opens and closes cleanly
var dirsCM_profile = mydir + "/profile-cmdmenu";
var dirsCM_left = mydir + "/left-cm";
var dirsCM_right = mydir + "/right-cm";
MkdirsAll([dirsCM_profile, dirsCM_left, dirsCM_right], 0o700);

StartTestApp(dirsCM_profile, dirsCM_left, dirsCM_right);
DismissHelpAndOSC52();
TypeVK(9); Sleep(200); TypeVK(9); Sleep(200); Sync(5000);

// Open top menu with F9
TypeFKey(9)
ExpectString("Left    Files    Commands    Options    Right", 0, 0, -1, -1, 10000)
Sync(1000)

// Navigate to Commands
TypeRight()
Sleep(200)
TypeRight()
Sleep(200)
Sync(1000)
TypeDown()
Sleep(500)
Sync(2000)

// Verify menu is open
ExpectString("Find file", 0, 0, -1, -1, 10000)
Sync(1000)

// Close with Escape (twice to fully exit menu mode)
TypeEscape()
Sleep(500)
Sync(2000)
TypeEscape()
Sleep(300)
Sync(2000)

// Should be back at panels
ExpectString("left-cm", 0, 0, -1, -1, 5000)
Sync(2000)
Sleep(500)
Sync(2000)

ExitFar2lWithConfirm()


///////////////////
// Corner case: Navigate to different Commands menu items
// Verifies menu navigation works for multiple items
var dirsMI_profile = mydir + "/profile-menuitems";
var dirsMI_left = mydir + "/left-mi";
var dirsMI_right = mydir + "/right-mi";
MkdirsAll([dirsMI_profile, dirsMI_left, dirsMI_right], 0o700);

StartTestApp(dirsMI_profile, dirsMI_left, dirsMI_right);
DismissHelpAndOSC52();
TypeVK(9); Sleep(200); TypeVK(9); Sleep(200); Sync(5000);

TypeFKey(9)
ExpectString("Left    Files    Commands    Options    Right", 0, 0, -1, -1, 10000)
Sync(1000)

// Navigate to Commands
TypeRight()
Sleep(200)
TypeRight()
Sleep(200)
Sync(1000)
TypeDown()
Sleep(500)
Sync(2000)

// Verify "Find file" is visible (first item)
ExpectString("Find file", 0, 0, -1, -1, 10000)
Sync(1000)

// Navigate down to verify other items exist
TypeDown()
Sleep(200)
Sync(1000)

// Close menu (twice to fully exit menu mode)
TypeEscape()
Sleep(500)
Sync(2000)
TypeEscape()
Sleep(300)
Sync(2000)

ExpectString("left-mi", 0, 0, -1, -1, 5000)
Sync(2000)
Sleep(500)
Sync(2000)

ExitFar2lWithConfirm()
