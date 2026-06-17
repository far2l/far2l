LoadJS("../common.js");
var dirs = SetupTestDirs();

var left_sub1 = dirs.left + "/sub1";
var left_sub2 = dirs.left + "/sub2";
MkdirsAll([left_sub1, left_sub2], 0700);

var left_files = [dirs.left + "/file1", dirs.left + "/file2", dirs.left + "/file3"];
var left_sub_files = [dirs.left + "/sub1/aaa", dirs.left + "/sub1/bbb", dirs.left + "/sub1/ccc", dirs.left + "/sub2/ddd"];
Mkfiles(left_files, 0666, 0, 1024);
Mkfiles(left_sub_files, 0752, 10 * 1024 * 1024, 20 * 1024 * 1024);

var left_items = [dirs.left + "/file1", dirs.left + "/file2", dirs.left + "/file3", dirs.left + "/sub1", dirs.left + "/sub2"];
var right_items = [dirs.right + "/file1", dirs.right + "/file2", dirs.right + "/file3", dirs.right + "/sub1", dirs.right + "/sub2"];
var left_hash = HashPathes(left_items, true, true, true, true, true);

StartTestApp(dirs.profile, dirs.left, dirs.right);
var status = AppStatus();
DismissHelpAndOSC52();

// Select all 5 items in left panel and press F6 to move
TypeDown()
TypeIns()
TypeIns()
TypeIns()
TypeIns()
TypeIns()
TypeFKey(6)
ExpectString("════ Rename/Move ═════", 0, 0, -1, -1, 10000)
TypeEnter()

// Wait for move to complete
for (var i = 0; ; ++i) {
	Sleep(100)
	ExpectNoString("════ Rename/Move ═════", 0, 0, -1, -1, 10000)
	var right_hash = HashPathes(right_items, true, true, true, true, true)
	if (right_hash == left_hash) {
		break
	}
	if (i == 100) {
		Log("Lhash: " + left_hash)
		Log("Rhash: " + right_hash)
		Panic("Hashes mismatched")
	}
}

// Verify source files were moved (no longer exist)
if (CountExisting(left_items) != 0) {
	Panic("Some source files still existing!")
}

ExitFar2lWithConfirm()
