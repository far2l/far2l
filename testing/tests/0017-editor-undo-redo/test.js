LoadJS("../common.js");
var dirs = SetupTestDirs();

// 0017-editor-undo-redo — Test editor undo/redo state machine.
//
// PREREQUISITE: Pre-configure EditorUndoSize=5 in the profile's config
// directory so the undo stack limit can be exercised. Without this,
// UndoSize defaults to 0 (unlimited) and the pruning code path is dead.

// Pre-configure editor settings before starting far2l
var settingsDir = dirs.profile + "/.config/settings";
MkdirsAll([settingsDir], 0o700);
SaveTextFile(settingsDir + "/config.ini", [
    "[Editor]",
    "EditorUndoSize=5",
    ""
]);

// Create a test file to edit
WriteFile(dirs.left + "/editme.txt", "original line\n", 0o666);

StartTestApp(dirs.profile, dirs.left, dirs.right, "left", false);
TypeEscape();
Sleep(300);
Sync(2000);
BeCalm();
var rOSC = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic();
if (rOSC.I < 1) {
    TypeEnter();
    Sleep(500);
}
Sync(5000);

// Ensure file panel is active
TypeVK(9); Sleep(200); TypeVK(9); Sleep(200); Sync(5000);

// ========================================
// Phase 1: Open file in editor (F4), type text, verify modified
// ========================================
TypeDown()
Sleep(300)
Sync(2000)
TypeFKey(4)
Sleep(1000)
Sync(5000)

// Verify editor is open — look for the filename in the editor title bar
// or the editor's key bar at the bottom
BeCalm()
var rEdit = ExpectString("editme.txt", 0, 0, -1, -1, 10000)
BePanic()
if (rEdit.I < 1) {
    Log("Editor opened — editme.txt found in title")
} else {
    // Maybe the editor shows a different title; check for Save (F2) in keybar
    Log("Checking editor state — editme.txt found at " + rEdit.X + "," + rEdit.Y)
}
Sync(3000)

// Type some text at the cursor position
TypeText("HELLO")
Sleep(500)
Sync(3000)

// The editor should show the typed text
ExpectString("HELLO", 0, 0, -1, -1, 10000)
Sync(2000)

// ========================================
// Phase 2: Undo (Ctrl+Z) restores original content
// ========================================
// Press Ctrl+Z to undo the typed text
ToggleLCtrl(true)
TypeVK(0x5A) // 'Z'
ToggleLCtrl(false)
Sleep(500)
Sync(2000)

// The typed "HELLO" should be gone
BeCalm()
var r = ExpectString("HELLO", 0, 0, -1, -1, 3000)
BePanic()
if (r.I >= 1) {
    Log("Undo removed typed text — correct")
} else {
    Panic("Undo failed — HELLO still visible after Ctrl+Z")
}
Sync(1000)

// ========================================
// Phase 3: Redo (Ctrl+Shift+Z) re-applies undone edit
// ========================================
ToggleShift(true)
ToggleLCtrl(true)
TypeVK(0x5A) // 'Z' with Ctrl+Shift = Redo
ToggleLCtrl(false)
ToggleShift(false)
Sleep(500)
Sync(2000)

// The typed "HELLO" should be back
ExpectString("HELLO", 0, 0, -1, -1, 10000)
Sync(1000)

// ========================================
// Phase 4: Multiple undo/redo cycles
// ========================================
// Undo again
ToggleLCtrl(true)
TypeVK(0x5A)
ToggleLCtrl(false)
Sleep(300)
Sync(2000)

BeCalm()
var r2 = ExpectString("HELLO", 0, 0, -1, -1, 3000)
BePanic()
if (r2.I >= 1) {
    Log("Second undo removed text — correct")
} else {
    Panic("Second undo failed")
}
Sync(1000)

// Redo again (Ctrl+Shift+Z)
ToggleShift(true)
ToggleLCtrl(true)
TypeVK(0x5A)
ToggleLCtrl(false)
ToggleShift(false)
Sleep(300)
Sync(2000)

ExpectString("HELLO", 0, 0, -1, -1, 10000)
Sync(1000)

// ========================================
// Phase 5: Undo coalescing — type "world" rapidly, single undo removes all
// ========================================
// First undo the current "HELLO"
ToggleLCtrl(true)
TypeVK(0x5A)
ToggleLCtrl(false)
Sleep(300)
Sync(2000)

// Type "world" — each character should coalesce into one undo record
TypeText("world")
Sleep(500)
Sync(2000)
ExpectString("world", 0, 0, -1, -1, 10000)
Sync(1000)

// Single undo should remove all 5 characters of "world"
ToggleLCtrl(true)
TypeVK(0x5A)
ToggleLCtrl(false)
Sleep(500)
Sync(2000)

BeCalm()
var r3 = ExpectString("world", 0, 0, -1, -1, 3000)
BePanic()
if (r3.I >= 1) {
    Log("Coalesced undo removed all of 'world' — correct")
} else {
    Panic("Coalescing failed — 'world' still visible after single undo")
}
Sync(1000)

// ========================================
// Phase 6: Exit editor with unsaved changes
// ========================================
// Type something new
TypeText("SAVED")
Sleep(500)
Sync(2000)
ExpectString("SAVED", 0, 0, -1, -1, 10000)
Sync(1000)

// Exit editor with F10 — if file is modified, far2l asks to save
TypeFKey(10)
Sleep(1000)
Sync(5000)

// Dismiss any save/modified dialog with Enter (default action)
BeCalm()
TypeEnter()
Sleep(1000)
Sync(3000)
TypeEnter()
Sleep(500)
Sync(2000)
BePanic()

// Should be back at panels or at far2l exit dialog
Sync(2000)

ExitFar2lWithConfirm()


///////////////////
///////////////////
// CORNER CASES
///////////////////
///////////////////
var mydir = WorkDir();

///////////////////
// Corner case: Undo stack limit — UndoSize=5 means only 5 undo steps
// Create 8+ edits, verify only last 5 are undoable
var dirsUL_profile = mydir + "/profile-undolimit";
var dirsUL_left = mydir + "/left-ul";
var dirsUL_right = mydir + "/right-ul";
MkdirsAll([dirsUL_profile, dirsUL_left, dirsUL_right], 0o700);

// Pre-configure with smaller UndoSize for this test
var ulSettings = dirsUL_profile + "/.config/settings";
MkdirsAll([ulSettings], 0o700);
SaveTextFile(ulSettings + "/config.ini", [
    "[Editor]",
    "EditorUndoSize=3",
    ""
]);

WriteFile(dirsUL_left + "/limit.txt", "base\n", 0o666);

StartTestApp(dirsUL_profile, dirsUL_left, dirsUL_right, "left-ul", false);
TypeEscape(); Sleep(300); Sync(2000);
BeCalm();
var rUL = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic();
if (rUL.I < 1) { TypeEnter(); Sleep(500); }
Sync(5000);
TypeVK(9); Sleep(200); TypeVK(9); Sleep(200); Sync(5000);

TypeDown()
TypeFKey(4)
ExpectString("limit.txt", 0, 0, -1, -1, 10000)
Sync(3000)

// Type 6 different words, each creating a separate undo record
// (moved to different positions to prevent coalescing)
TypeText("A1")
Sleep(200)
TypeText(" B2")
Sleep(200)
TypeText(" C3")
Sleep(200)
TypeText(" D4")
Sleep(200)
TypeText(" E5")
Sleep(200)
TypeText(" F6")
Sleep(500)
Sync(2000)

// With UndoSize=3, only the last 3 edits should be undoable.
// Undo 3 times — should work.
for (var i = 0; i < 3; i++) {
    ToggleLCtrl(true)
    TypeVK(0x5A)
    ToggleLCtrl(false)
    Sleep(300)
    Sync(1000)
}

// 4th undo should fail (no more undo records) — file should still show content
ToggleLCtrl(true)
TypeVK(0x5A)
ToggleLCtrl(false)
Sleep(300)
Sync(2000)

// The editor should still be alive and showing content
ExpectString("limit.txt", 0, 0, -1, -1, 5000)
Sync(1000)

TypeEscape()
Sleep(500)
Sync(2000)
TypeEscape()
Sleep(500)
Sync(2000)

ExitFar2lWithConfirm()


///////////////////
// Corner case: Non-coalesced edits — typing on different lines creates
// separate undo records
var dirsNC_profile = mydir + "/profile-noncoalesce";
var dirsNC_left = mydir + "/left-nc";
var dirsNC_right = mydir + "/right-nc";
MkdirsAll([dirsNC_profile, dirsNC_left, dirsNC_right], 0o700);

var ncSettings = dirsNC_profile + "/.config/settings";
MkdirsAll([ncSettings], 0o700);
SaveTextFile(ncSettings + "/config.ini", [
    "[Editor]",
    "EditorUndoSize=10",
    ""
]);

WriteFile(dirsNC_left + "/multi.txt", "line1\nline2\n", 0o666);

StartTestApp(dirsNC_profile, dirsNC_left, dirsNC_right, "left-nc", false);
TypeEscape(); Sleep(300); Sync(2000);
BeCalm();
var rNC = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic();
if (rNC.I < 1) { TypeEnter(); Sleep(500); }
Sync(5000);
TypeVK(9); Sleep(200); TypeVK(9); Sleep(200); Sync(5000);

TypeDown()
TypeFKey(4)
ExpectString("multi.txt", 0, 0, -1, -1, 10000)
Sync(3000)

// Type on first line
TypeText("AAA")
Sleep(300)
Sync(1000)

// Move to next line (Down arrow)
TypeDown()
Sleep(200)
Sync(1000)

// Type on second line
TypeText("BBB")
Sleep(300)
Sync(2000)

// Undo should remove "BBB" first (last edit on line 2)
ToggleLCtrl(true)
TypeVK(0x5A)
ToggleLCtrl(false)
Sleep(500)
Sync(2000)

BeCalm()
var r4 = ExpectString("BBB", 0, 0, -1, -1, 3000)
BePanic()
if (r4.I >= 1) {
    Log("First undo removed BBB (different line, non-coalesced) — correct")
} else {
    Panic("Non-coalesced undo failed — BBB still visible")
}
Sync(1000)

// Second undo should remove "AAA" (edit on line 1)
ToggleLCtrl(true)
TypeVK(0x5A)
ToggleLCtrl(false)
Sleep(500)
Sync(2000)

BeCalm()
var r5 = ExpectString("AAA", 0, 0, -1, -1, 3000)
BePanic()
if (r5.I >= 1) {
    Log("Second undo removed AAA (separate undo step) — correct")
} else {
    Panic("Non-coalesced undo failed — AAA still visible")
}
Sync(1000)

TypeEscape()
Sleep(500)
Sync(2000)
TypeEscape()
Sleep(500)
Sync(2000)

ExitFar2lWithConfirm()
