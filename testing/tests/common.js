// common.js — Shared test functions for far2l smoke tests.
// Load with: LoadJS("../common.js")

// SetupTestDirs creates the standard profile/left/right directory layout.
// leftName/rightName default to "left"/"right" if omitted.
// Returns {mydir, profile, left, right}.
function SetupTestDirs(leftName, rightName) {
    leftName = leftName || "left";
    rightName = rightName || "right";
    var mydir = WorkDir();
    var profile = mydir + "/profile";
    var left = mydir + "/" + leftName;
    var right = mydir + "/" + rightName;
    MkdirsAll([profile, left, right], 0o700);
    return {mydir: mydir, profile: profile, left: left, right: right};
}

// FreshTestDirs creates a fresh set of profile/left/right directories
// under the test workdir with the given suffix. Reduces duplication in
// corner-case test sections that each need their own far2l instance.
// Returns {mydir, profile, left, right}.
function FreshTestDirs(suffix, leftName, rightName) {
    leftName = leftName || ("left-" + suffix);
    rightName = rightName || ("right-" + suffix);
    var mydir = WorkDir();
    var profile = mydir + "/profile-" + suffix;
    var left = mydir + "/" + leftName;
    var right = mydir + "/" + rightName;
    MkdirsAll([profile, left, right], 0o700);
    return {mydir: mydir, profile: profile, left: left, right: right};
}

// StartTestApp launches far2l with standard flags and expects the left panel
// label. leftLabel is the text shown in the left panel title bar (defaults
// to "left"). Pass null to skip the panel label check. If expectHelp is true
// (default), also expects the Help window. Optional size = [cols, rows] for
// non-default terminal dimensions.
function StartTestApp(profile, left, right, leftLabel, expectHelp, size) {
    if (leftLabel === undefined) leftLabel = "left";
    if (expectHelp === undefined) expectHelp = true;
    var args = ["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", left, "-cd", right];
    if (size) {
        StartAppWithSize(args, size[0], size[1]);
    } else {
        StartApp(args);
    }
    if (leftLabel) {
        ExpectString(leftLabel, 0, 0, -1, -1, 10000);
    }
    if (expectHelp) {
        ExpectString("Help - FAR2L", 0, 0, -1, -1, 10000);
    }
}

// DismissHelpAndOSC52 closes the Help window and, on first start,
// dismisses the OSC52 clipboard dialog if it appears.
function DismissHelpAndOSC52() {
    TypeEscape(10);
    Sync(5000);
    BeCalm();
    var r = ExpectString("OSC52", 0, 0, -1, -1, 2000);
    BePanic();
    if (r.I < 1) {
        TypeEnter();
        Sleep(500);
    }
}

// ExitFar2lWithConfirm presses F10 and confirms the exit dialog.
function ExitFar2lWithConfirm() {
    TypeFKey(10);
    ExpectString("Do you want to quit FAR?", 0, 0, -1, -1, 10000);
    TypeEnter();
    ExpectAppExit(0, 10000);
}

// StartBashShell opens an interactive bash session inside far2l's VT shell.
// Used by tests that need TTYWriteRaw to inject commands.
function StartBashShell() {
    TypeText("echo 'READY'; exec bash --norc --noprofile");
    TypeEnter();
    ExpectString("READY", 0, 0, -1, -1, 10000);
    Sync(5000);
    Sleep(1000);
}

// ExitBashAndFar2l exits the interactive bash session, then exits far2l.
function ExitBashAndFar2l() {
    TTYWriteRaw("exit\n");
    Sync(5000);
    Sleep(1000);
    TypeEscape();
    TypeText("exit far");
    TypeEnter();
    ExpectAppExit(0, 10000);
}
