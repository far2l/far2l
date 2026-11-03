mydir=WorkDir()
profile=mydir + "/profile"
left=mydir + "/left-ok"
gone=mydir + "/left-gone"
MkdirsAll([profile, left, gone, profile + "/.config/settings"], 0700)

// Panel state is saved on quit only with AutoSavePanels enabled; without this
// seed the second run would never see the stale folder.
SaveTextFile(profile + "/.config/settings/config.ini", ["[System]", "AutoSavePanels=1"])

// First run: start with 'gone' as the left panel folder (-cd startup arg) and
// quit cleanly so the panel state records it, then delete it. The pre-seeded
// config makes this a non-first start: no help or OSC52 dialogs appear.
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", gone, "-cd", left]);
ExpectString("left-gone", 0, 0, -1, -1, 10000);
TypeFKey(10)
ExpectString("Do you want to quit FAR?", 0, 0, -1, -1, 10000)
TypeEnter()
ExpectAppExit(0, 10000)

// Run 2 only exercises the stale folder if the state above really persisted
if (LoadTextFile(profile + "/.config/settings/panel.ini").join("\n").indexOf("left-gone") < 0) {
    Panic("panel state does not record left-gone; AutoSavePanels seed failed");
}

// Delete the saved panel folder
RemoveAll(gone)
if (Exists(gone)) {
    Log("FAIL: gone dir still exists after RemoveAll");
    Panic("gone dir still exists after RemoveAll");
}

// Second run: start with the stale folder as the left panel's startup dir
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", gone, "-cd", left]);
Sleep(3000)
// far2l must still be alive and quit cleanly despite the stale -cd target
TypeFKey(10)
ExpectString("Do you want to quit FAR?", 0, 0, -1, -1, 10000)
TypeEnter()
ExpectAppExit(0, 10000)

0;
