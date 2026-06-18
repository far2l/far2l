LoadJS("../common.js");
var dirs = SetupTestDirs();

// Pre-configure one macro so Edit-noop and Duplicate-rejection have a target.
// far2l stores macros in <profile>/.config/settings/key_macros.ini
var macrosIni = dirs.profile + "/.config/settings";
MkdirsAll([macrosIni], 0700);
SaveTextFile(macrosIni + "/key_macros.ini", [
    "[KeyMacros]",
    "MacroVersion=1",
    "",
    "[KeyMacros/Common/F2]",
    "Sequence=F3",
    "Description=Pre-existing macro",
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
// Helper: open Macro Browser via F9 menu
// ========================================
function OpenMacroBrowser() {
    TypeFKey(9);
    ExpectString("Left    Files    Commands    Options    Right", 0, 0, -1, -1, 10000);
    Sync(1000);
    TypeRight();
    Sleep(200);
    TypeRight();
    Sleep(200);
    Sync(1000);
    TypeDown();
    Sleep(500);
    Sync(2000);
    TypeEnd();
    Sleep(200);
    Sync(1000);
    TypeUp();
    Sleep(200);
    Sync(1000);
    ExpectString("Macro Browser", 0, 0, -1, -1, 5000);
    TypeEnter();
    Sleep(500);
    Sync(5000);
    ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
    Sync(2000);
}

// ========================================
// Helper: close Macro Browser with Escape
// ========================================
function CloseMacroBrowser() {
    TypeEscape();
    Sleep(300);
    Sync(2000);
    ExpectString("left", 0, 0, -1, -1, 10000);
    Sync(1000);
}

// ========================================
// Helper: navigate cursor to the first macro entry
// The VMenu list layout (focusable items only):
//   [0]  Total macros ...
//   [18] Common# 1: F2 - Pre-existing macro
//   [19] Common# 2: CtrlAltS - Memo Plugin
//   [21] Areas stat
//   [22] Macros stat
// One Down from [0] skips separators to [18].
// ========================================
function GotoFirstMacro() {
    TypeDown();
    Sleep(200);
    Sync(1000);
}

// ========================================
// Phase 1: MacroReplaceAdd — Add new macro
// ========================================
// Open Macro Browser, press Ins to add, fill dialog, OK, verify macro appears.
OpenMacroBrowser();
Sync(2000);

ExpectString("Pre-existing macro", 0, 0, -1, -1, 10000);
Sync(1000);

// Press Ins to open the "New Macro" edit dialog
TypeIns();
Sleep(500);
Sync(3000);

ExpectString("New Macro", 0, 0, -1, -1, 10000);
Sync(1000);

// Focus starts on the Area dropdown (Common by default).
// Tab to the Key edit field (TypeVK(9) = Tab).
TypeVK(9);
Sleep(200);
Sync(1000);
TypeText("F5");
Sleep(200);
Sync(1000);

// Tab past "Assign Key" button to Description field
TypeVK(9);
Sleep(100);
Sync(500);
TypeVK(9);
Sleep(100);
Sync(500);
TypeText("Added by smoke test");
Sleep(200);
Sync(1000);

// Tab to Sequence memo field
TypeVK(9);
Sleep(200);
Sync(1000);
TypeText("CtrlO");
Sleep(200);
Sync(1000);

// Press Ctrl+Enter to activate the OK button (DIF_DEFAULT).
// Plain Enter on a DI_MEMOEDIT inserts a newline instead of activating OK.
ToggleLCtrl(true);
TypeEnter();
ToggleLCtrl(false);
Sleep(500);
Sync(5000);

// Dialog should close and return to Macro Browser list.
ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
Sync(2000);
ExpectString("Added by smoke test", 0, 0, -1, -1, 10000);
Sync(1000);

CloseMacroBrowser();

// ========================================
// Phase 2: MacroReplaceAdd — Edit with NoChanges (noop)
// ========================================
// Open browser, navigate to the pre-existing macro, F4 to edit,
// OK without changing anything. MacroReplaceAdd returns NoChanges (8)
// which the dialog treats as success (return TRUE), so the dialog closes
// and the list remains unchanged.
OpenMacroBrowser();
Sync(2000);

// Navigate to the first macro entry (the pre-existing F2 macro)
GotoFirstMacro();

// Press F4 to edit the selected macro
TypeFKey(4);
Sleep(500);
Sync(3000);

ExpectString("Edit Macro", 0, 0, -1, -1, 10000);
Sync(1000);
ExpectString("Pre-existing macro", 0, 0, -1, -1, 10000);
Sync(1000);

// Press Ctrl+Enter to OK without changing anything — NoChanges path.
// Ctrl+Enter activates the DIF_DEFAULT button regardless of focus.
ToggleLCtrl(true);
TypeEnter();
ToggleLCtrl(false);
Sleep(500);
Sync(3000);

// Dialog should close (NoChanges returns TRUE => dialog exits with OK)
ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
Sync(2000);
ExpectString("Pre-existing macro", 0, 0, -1, -1, 10000);
Sync(1000);

CloseMacroBrowser();

// ========================================
// Phase 3: MacroReplaceAdd — Duplicate key rejection
// ========================================
// Open browser, Ins to add a new macro with the same key (F2) in the same
// area (Common) as the pre-existing macro. MacroReplaceAdd returns
// DuplicateAreaKey (9) and the dialog shows an error message.
OpenMacroBrowser();
Sync(2000);

// Press Ins to open the "New Macro" edit dialog
TypeIns();
Sleep(500);
Sync(3000);

ExpectString("New Macro", 0, 0, -1, -1, 10000);
Sync(1000);

// Tab to the Key field and type the duplicate key (F2, same as pre-existing)
TypeVK(9);
Sleep(200);
Sync(1000);
TypeText("F2");
Sleep(200);
Sync(1000);

// Tab past Assign Key button to Description
TypeVK(9);
Sleep(100);
Sync(500);
TypeVK(9);
Sleep(100);
Sync(500);
TypeText("Duplicate macro");
Sleep(200);
Sync(1000);

// Tab to Sequence field
TypeVK(9);
Sleep(200);
Sync(1000);
TypeText("F4");
Sleep(200);
Sync(1000);
// Press Ctrl+Enter to OK — should trigger DuplicateAreaKey error
ToggleLCtrl(true);
TypeEnter();
ToggleLCtrl(false);
Sleep(500);
Sync(3000);

// The error message should appear
ExpectString("already another macro", 0, 0, -1, -1, 10000);
Sync(1000);

// Dismiss the error dialog with Enter (Ok)
TypeEnter();
Sleep(300);
Sync(2000);

// We should still be in the Edit dialog (the error does not close it)
ExpectString("New Macro", 0, 0, -1, -1, 10000);
Sync(1000);

// Cancel the edit dialog
TypeEscape();
Sleep(300);
Sync(2000);

// Back in Macro Browser
ExpectString("Macro Browser", 0, 0, -1, -1, 10000);
Sync(1000);

CloseMacroBrowser();

ExitFar2lWithConfirm();
