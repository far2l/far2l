LoadJS("../common.js");
var dirs = SetupTestDirs();

// Pre-configure one active macro. Phases 1-2 test EmptySequence and
// InvalidKey error paths. Phase 3 deletes the macro then re-adds it
// with the same key to exercise the add-over-deleted-slot path
// (macrobrowser.cpp:6896-6904).
var macrosIni = dirs.profile + "/.config/settings";
MkdirsAll([macrosIni], 0o700);
SaveTextFile(macrosIni + "/key_macros.ini", [
    "[KeyMacros]",
    "MacroVersion=1",
    "",
    "[KeyMacros/Common/F2]",
    "Sequence=F3",
    "Description=Will be recycled",
    ""
]);

// Start far2l. Pre-existing .config/settings suppresses first-run Help,
// but the OSC52 clipboard dialog may still appear on first start.
StartTestApp(dirs.profile, dirs.left, dirs.right, "left", false);
TypeEscape();
Sleep(300);
Sync(2000);
BeCalm();
var r = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic();
if (r.I < 1) {
    TypeEnter();
    Sleep(500);
}
Sync(5000);

// ========================================
// Helpers
// ========================================
function OpenMacroBrowser() {
    TypeFKey(9);
    ExpectString("Left    Files    Commands    Options    Right", 0, 0, -1, -1, 10000);
    Sync(1000);
    TypeRight(); Sleep(200);
    TypeRight(); Sleep(200);
    Sync(1000);
    TypeDown(); Sleep(500); Sync(2000);
    TypeEnd(); Sleep(200); Sync(1000);
    TypeUp(); Sleep(200); Sync(1000);
    ExpectString("Macro Browser", 0, 0, -1, -1, 5000);
    TypeEnter(); Sleep(500); Sync(5000);
    ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
    Sync(2000);
}

function CloseMacroBrowser() {
    TypeEscape(); Sleep(300); Sync(2000);
    ExpectString("left", 0, 0, -1, -1, 10000);
    Sync(1000);
}

function GotoFirstMacro() {
    TypeDown(); Sleep(200); Sync(1000);
}

// Walk the VMenu by Down+Enter to identify which macro is selected,
// then cancel back to the list. When target description is found,
// press Del instead of Enter and confirm the delete.
function FindAndDeleteMacro(desc) {
    for (var attempt = 0; attempt < 30; attempt++) {
        TypeDown();
        Sleep(100);
        Sync(500);

        // Open Edit dialog to check description
        TypeEnter();
        Sleep(500);
        Sync(2000);

        BeCalm();
        var found = ExpectString(desc, 0, 0, -1, -1, 2000);
        BePanic();

        if (found.I < 1) {
            // Found — cancel Edit, then Del
            TypeEscape(); Sleep(300); Sync(1000);
            TypeDel(); Sleep(500); Sync(3000);
            ExpectString("Mark macro as deleted", 0, 0, -1, -1, 10000);
            Sync(1000);
            TypeEnter(); Sleep(500); Sync(3000);
            ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
            Sync(2000);
            return true;
        }
        TypeEscape(); Sleep(300); Sync(1000);
    }
    return false;
}

// ========================================
// Phase 1: EmptySequence — empty sequence field
// ========================================
OpenMacroBrowser();
Sync(2000);
TypeIns(); Sleep(500); Sync(3000);
ExpectString("New Macro", 0, 0, -1, -1, 10000);
Sync(1000);

TypeVK(9); Sleep(200); Sync(1000);   // Tab → Key
TypeText("F9"); Sleep(200); Sync(1000);

TypeVK(9); Sleep(100); Sync(500);    // Tab → AssignKey button
TypeVK(9); Sleep(100); Sync(500);    // Tab → Description
TypeVK(9); Sleep(200); Sync(1000);   // Tab → Sequence (leave empty)

ToggleLCtrl(true);
TypeEnter();
ToggleLCtrl(false);
Sleep(500);
Sync(3000);

ExpectString("Empty macro sequence field", 0, 0, -1, -1, 10000);
Sync(1000);
TypeEnter(); Sleep(300); Sync(2000);          // Dismiss error
ExpectString("New Macro", 0, 0, -1, -1, 10000); // Dialog still open
Sync(1000);
TypeEscape(); Sleep(300); Sync(2000);          // Cancel
ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
Sync(1000);
CloseMacroBrowser();

// ========================================
// Phase 2: InvalidKey — garbage key name
// ========================================
OpenMacroBrowser();
Sync(2000);
TypeIns(); Sleep(500); Sync(3000);
ExpectString("New Macro", 0, 0, -1, -1, 10000);
Sync(1000);

TypeVK(9); Sleep(200); Sync(1000);
TypeText("ZZZZZZZ"); Sleep(200); Sync(1000);

TypeVK(9); Sleep(100); Sync(500);
TypeVK(9); Sleep(100); Sync(500);
TypeText("Bad key macro"); Sleep(200); Sync(1000);

TypeVK(9); Sleep(200); Sync(1000);
TypeText("F3"); Sleep(200); Sync(1000);

ToggleLCtrl(true);
TypeEnter();
ToggleLCtrl(false);
Sleep(500);
Sync(3000);

ExpectString("Invalid Key", 0, 0, -1, -1, 10000);
Sync(1000);
TypeEnter(); Sleep(300); Sync(2000);
ExpectString("New Macro", 0, 0, -1, -1, 10000);
Sync(1000);
TypeEscape(); Sleep(300); Sync(2000);
ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
Sync(1000);
CloseMacroBrowser();

// ========================================
// Phase 3: Add over deleted-entry slot
// ========================================
// Delete F2 via the MacroBrowser, then re-add a macro with the same
// key F2 in the same area. MacroReplaceAdd detects the deleted
// duplicate at macrobrowser.cpp:6896-6904 and reuses the slot instead
// of returning DuplicateAreaKey.

OpenMacroBrowser();
Sync(2000);

var deleted = FindAndDeleteMacro("Will be recycled");
if (!deleted) {
    Panic("Could not find 'Will be recycled' macro to delete");
}

// Verify description is gone (freed by MacroDelete)
BeCalm();
var descCheck = ExpectString("Will be recycled", 0, 0, -1, -1, 2000);
BePanic();
if (descCheck.I < 1) {
    Panic("Description still visible after deletion");
}
CloseMacroBrowser();

// Re-add with same key F2 — should reuse deleted slot
OpenMacroBrowser();
Sync(2000);
TypeIns(); Sleep(500); Sync(3000);
ExpectString("New Macro", 0, 0, -1, -1, 10000);
Sync(1000);

TypeVK(9); Sleep(200); Sync(1000);
TypeText("F2"); Sleep(200); Sync(1000);

TypeVK(9); Sleep(100); Sync(500);
TypeVK(9); Sleep(100); Sync(500);
TypeText("Recycled slot macro"); Sleep(200); Sync(1000);

TypeVK(9); Sleep(200); Sync(1000);
TypeText("F4"); Sleep(200); Sync(1000);

ToggleLCtrl(true);
TypeEnter();
ToggleLCtrl(false);
Sleep(500);
Sync(5000);

// Should succeed (not DuplicateAreaKey) — reuses deleted F2 slot
ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
Sync(2000);
ExpectString("Recycled slot macro", 0, 0, -1, -1, 10000);
Sync(1000);

CloseMacroBrowser();
ExitFar2lWithConfirm();
