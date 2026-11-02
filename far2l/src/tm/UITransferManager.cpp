#include "headers.hpp"

#include "UITransferManager.hpp"
#include "LocalFileSystem.hpp"
#include "OsdPanel.hpp"

#include "lang.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "farcolors.hpp"
#include "config.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <fcntl.h>

#include "DialogBuilder.hpp"
#include "formats.hpp"
#include "dialog.hpp"

#include <utils.h>

/*     │❓/run/me...cus_mechaicus.svg  ███░░░░░░░░░ 19%│
       │📄   1/   1 📁   1/   1 💾 18M/ 75M 📶4.34 M/s │        */
void UITransferManager::onProgress(const TransferProgress& jobProgress, const TransferProgress& prog) {
	int row = jobToRow[jobProgress.job_id];

	std::wstring icon = L"⚙️";
	if (jobProgress.is_completed) icon = L"🌿";
	if (jobProgress.is_scanning_active) icon = L"🍃";
	if (jobProgress.active_questions_count > 0) icon = L"❓";
	if (jobProgress.is_paused_for_io_error) icon = L"‼️";

	// first line: icon, current file, progress meter
	osd->UpdateField(row, 0, 1, icon);
	int lineW = osd->GetMaxWidth() - 2 - 2 - 16; /* │⚙️ ...  ███░░░░░░░░░ 00%│ */
	osd->UpdateField(row, 1, lineW, TruncatePathMiddle(jobProgress.current_target_item, lineW));
	osd->UpdateField(row, 2 + lineW, 16, OsdPanel::BuildProgressBarString(jobProgress.bytes_transferred, 0, jobProgress.total_bytes, 16, true));

	// second line: counters
	if (jobProgress.is_paused_for_io_error) 
		osd->UpdateLine(row + 1, jobProgress.io_error_reason);
	else {
		osd->UpdateField(row + 1, 2, 4, BuildSizeString(jobProgress.files_transferred, 4));
        osd->UpdateField(row + 1, 7, 4, BuildSizeString(jobProgress.total_files, 4));
		osd->UpdateField(row + 1, 13, 4, BuildSizeString(jobProgress.dirs_transferred, 4));
        osd->UpdateField(row + 1, 18, 4, BuildSizeString(jobProgress.total_dirs, 4));
		osd->UpdateField(row + 1, 24, 4, BuildSizeString(jobProgress.bytes_transferred, 4));
        osd->UpdateField(row + 1, 29, 4, BuildSizeString(jobProgress.total_bytes, 4));
        osd->UpdateField(row + 1, 35, 4, BuildRealString(jobProgress.current_speed_mb_s, 4));
	}

	if (jobProgress.is_completed) { 
		osd->CompleteLine(row);
		osd->CompleteLine(row + 1);
		jobToRow.erase(jobProgress.job_id);
	}
}

void UITransferManager::onFolderScanProgress(const FolderScanProgress& scan) {
	int row = jobToRow[scan.job_id];

	std::wstring icon = L"🔍";
	if (scan.is_completed || scan.state == ScanState::COMPLETED) icon = L"🌿";
	if (scan.state == ScanState::CANCELLED || scan.state == ScanState::FAILED) icon = L"‼️";

	// first line: icon, current file, progress meter
	osd->UpdateField(row, 0, 1, icon);

	// second line: counters
    osd->UpdateField(row + 1, 7, 4, BuildSizeString(scan.scanned_files, 4));
    osd->UpdateField(row + 1, 18, 4, BuildSizeString(scan.scanned_folders, 4));
    osd->UpdateField(row + 1, 29, 4, BuildSizeString(scan.scanned_bytes, 4));
}

static FarLangMsg answerToText(QuestionAnswer qa) {
	switch(qa) {
    case QuestionAnswer::OVERWRITE:     		return Msg::TransferQuestionOverwrite;
    case QuestionAnswer::OVERWRITE_ALL:    		return Msg::TransferQuestionOverwrite;
    case QuestionAnswer::APPEND:        		return Msg::TransferQuestionAppend;
    case QuestionAnswer::RESUME:        		return Msg::TransferQuestionResume;
    case QuestionAnswer::SKIP:          		return Msg::TransferQuestionSkip;
    case QuestionAnswer::SKIP_ALL:         		return Msg::TransferQuestionSkip;
    case QuestionAnswer::RENAME:        		return Msg::TransferQuestionRename;
    case QuestionAnswer::AUTO_RENAME:      		return Msg::TransferQuestionRename;
    case QuestionAnswer::PLACE_INTO:    		return Msg::TransferQuestionOverwrite;
    case QuestionAnswer::RETRY: 				return Msg::TransferQuestionRetry;
    case QuestionAnswer::OVERWRITE_IF_NEWER:    return Msg::TransferQuestionOverwriteIfNewer;
    case QuestionAnswer::OVERWRITE_IF_DIFFERENT_SIZE:    return Msg::TransferQuestionOverwriteIfSize;
    case QuestionAnswer::OVERWRITE_IF_NEWER_OR_DIFFERENT_SIZE:    return Msg::TransferQuestionOverwriteIfNewerOrSize;
    case QuestionAnswer::IGNORE: 				return Msg::TransferQuestionIgnore;
    case QuestionAnswer::IGNORE_ALL:			return Msg::TransferQuestionIgnore;
    	
    case QuestionAnswer::CANCEL:
	case QuestionAnswer::PROMPT:
	default:
		return Msg::TransferQuestionCancel;
	}
}

/*
static template<typename... Args> void addButtons(std::vector<QuestionAnswer>& v, Args... args) {
    static_assert((std::is_same_v<Args, QuestionAnswer> && ...), "All arguments must be QuestionAnswer");
    (v.push_back(args), ...);
} */

static void addButtons(std::vector<QuestionAnswer>& v, std::initializer_list<QuestionAnswer> buttons) {
    v.insert(v.end(), buttons);
}

static std::vector<DialogBuilder::ItemReference> addButtonsLineToUI(DialogBuilder& builder, std::vector<QuestionAnswer> v, int DlgPrefWidth) {
	FarLangMsg texts[128];
	int textsCount = (int)v.size();

	for(size_t i = 0; i < v.size(); ++i) 
		texts[i] = answerToText(v[i]);
	return builder.AddButtonsLine(texts, textsCount, 0, DlgPrefWidth);
}

static FarLangMsg questionToText(QuestionType type) {
	switch(type) {
	case QuestionType::NO_SPACE_LEFT: 				return Msg::TransferQuestionNoSpaceLeft;
	case QuestionType::CRITICAL_IO_ERROR: 			return Msg::TransferQuestionCriticalIoError;
	case QuestionType::LINK_DESTINATION_EXISTS: 	return Msg::TransferQuestionLinkExists;
	case QuestionType::FOLDER_DESTINATION_IS_FILE:	return Msg::TransferQuestionFolderTargetIsFile;
	case QuestionType::FILE_DESTINATION_IS_FOLDER:  return Msg::TransferQuestionFileTargetIsFolder;
	case QuestionType::OVERWRITE_EXISTING:			return Msg::TransferQuestionTargetExists;
	case QuestionType::MOVE_READ_ONLY_SOURCE:		return Msg::TransferQuestionSourceReadOnly;
	case QuestionType::OVERWRITE_READ_ONLY_TARGET:	return Msg::TransferQuestionTargetReadOnly;
    case QuestionType::PERMISSION_DENIED:         	return Msg::TransferQuestionAccessDenied;
    case QuestionType::SAME_SOURCE_AND_TARGET:     	return Msg::TransferQuestionCopyItself;
    case QuestionType::CANNOT_CREATE_FOLDER:     	return Msg::TransferQuestionCannotCreateFolder;
    case QuestionType::CANNOT_REMOVE_SOURCE:     	return Msg::TransferQuestionCannotRemoveSource;
    case QuestionType::CANNOT_COPY_METADATA:     	return Msg::TransferQuestionCannotCopyMetadata;
    case QuestionType::BROKEN_SYMLINK:     			return Msg::TransferQuestionBrokenSymlink;
    case QuestionType::READ_ERROR:     				return Msg::TransferQuestionReadError;
    case QuestionType::WRITE_ERROR:     			return Msg::TransferQuestionWriteError;
    case QuestionType::IO_ERROR:     				return Msg::TransferQuestionIoError;
	default:										return Msg::TransferQuestionGeneralIoError;
	}
	return Msg::TransferQuestionGeneralIoError;
}

/* 
Dialog UI:

(v1)

	<caution text>, possibly multi-line

	Souce:  /source/folder/to/foo/bar
	Source size: xxxx Modification time: xxxxxxxxxx

	Target: /destinatiuon/to/foo/bar
	Target size: xxxx Modification time: xxxxxxxxxx

	<details text>, possibly multi-line 

	[x] Apply the same option for further questions automatically

	[ Overwrite ] [ Skip ] [ Camcel ] - dynamic list from the specific questions.

(v2)
	<caution text>, possibly multi-line

	Souce:  /source/folder/to/foo/bar
	Target: /destinatiuon/to/foo/bar

	<details text>, possibly multi-line

	[x] Apply the same option for further questions automatically

	[ Retry ] [ Skip ] [ Camcel ] - dynamic list from the specific questions.
*/

QuestionAnswer UITransferManager::onQuestion(const TransferQuestion& q, bool& rememberForType)  {
	const int DlgPrefWidth = 70;
	bool isRemember = false;

	std::vector<QuestionAnswer> buttons;
	DialogBuilder builder(Msg::TransferQuestionTitle, L"TransferQuestionTitle");
	builder.SetDialogFlags(DMODE_WARNINGSTYLE);

	FarLangMsg labelID = questionToText(q.type);
	builder.AddWrappedText(labelID, DlgPrefWidth);
	builder.AddEmptyLine();

	std::wstring s_size, t_size, s_mtime, t_mtime;
	std::wstring s_path, t_path, s_msg;
	s_path = q.source_path;
	t_path = q.target_path;
	s_msg = q.message;

	if (q.type == QuestionType::OVERWRITE_EXISTING || q.type == QuestionType::OVERWRITE_READ_ONLY_TARGET) {
		s_size = BuildSizeString(q.source_size, 10);
		t_size = BuildSizeString(q.target_size, 10);
		s_mtime = BuildDateTime(q.source_mtime);
		t_mtime = BuildDateTime(q.target_mtime);

		builder.AddText(Msg::TransferQuestionSource);
		builder.AddText(TruncatePathMiddle(s_path, DlgPrefWidth).c_str());
		builder.AddText(Msg::TransferQuestionSourceSize, false);
		builder.AddText(s_size.c_str(), false);
		builder.AddText(Msg::TransferQuestionSourceMTime, false);
		builder.AddText(s_mtime.c_str());

		builder.AddText(Msg::TransferQuestionTarget, false);
		builder.AddText(TruncatePathMiddle(t_path, DlgPrefWidth).c_str(), false);
		builder.AddText(Msg::TransferQuestionSourceSize, false);
		builder.AddText(t_size.c_str(), false);
		builder.AddText(Msg::TransferQuestionSourceMTime, false);
		builder.AddText(t_mtime.c_str());
	}
	else {
		builder.AddText(Msg::TransferQuestionSource);
		builder.AddText(TruncatePathMiddle(s_path, DlgPrefWidth).c_str());
		builder.AddText(Msg::TransferQuestionTarget, false);
		builder.AddText(TruncatePathMiddle(t_path, DlgPrefWidth).c_str());
	}

	builder.AddEmptyLine();

	builder.AddText(Msg::TransferQuestionMessage);
	builder.AddWrappedText(s_msg.c_str(), DlgPrefWidth);

	/* checkbox */	
	builder.AddEmptyLine();
	builder.AddCheckbox(Msg::TransferQuestionRepeat, &isRemember);
	builder.AddEmptyLine();

    /* possible answers */
	if (q.type == QuestionType::NO_SPACE_LEFT || q.type == QuestionType::CRITICAL_IO_ERROR) 
		addButtons(buttons, { QuestionAnswer::RETRY, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::LINK_DESTINATION_EXISTS) 
		addButtons(buttons, { QuestionAnswer::OVERWRITE, QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::FOLDER_DESTINATION_IS_FILE) 
		addButtons(buttons, { QuestionAnswer::OVERWRITE, QuestionAnswer::RENAME, QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::FILE_DESTINATION_IS_FOLDER) 
		addButtons(buttons, { QuestionAnswer::PLACE_INTO, QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::OVERWRITE_READ_ONLY_TARGET)
		addButtons(buttons, { QuestionAnswer::OVERWRITE, QuestionAnswer::OVERWRITE_IF_NEWER, 
			QuestionAnswer::OVERWRITE_IF_DIFFERENT_SIZE, QuestionAnswer::OVERWRITE_IF_NEWER_OR_DIFFERENT_SIZE,
			QuestionAnswer::AUTO_RENAME, QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::OVERWRITE_EXISTING) 
		addButtons(buttons, { QuestionAnswer::OVERWRITE, QuestionAnswer::OVERWRITE_IF_NEWER, 
			QuestionAnswer::OVERWRITE_IF_DIFFERENT_SIZE, QuestionAnswer::OVERWRITE_IF_NEWER_OR_DIFFERENT_SIZE,
			QuestionAnswer::AUTO_RENAME, QuestionAnswer::APPEND, QuestionAnswer::RESUME, 
			QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::MOVE_READ_ONLY_SOURCE) 
		addButtons(buttons, { QuestionAnswer::OVERWRITE, QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::SAME_SOURCE_AND_TARGET || q.type == QuestionType::BROKEN_SYMLINK) 
		addButtons(buttons, { QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::CANNOT_REMOVE_SOURCE) 
		addButtons(buttons, { QuestionAnswer::RETRY, QuestionAnswer::IGNORE, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::CANNOT_COPY_METADATA) 
		addButtons(buttons, { QuestionAnswer::IGNORE, QuestionAnswer::RETRY, QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else if (q.type == QuestionType::READ_ERROR || q.type == QuestionType::WRITE_ERROR) 
		addButtons(buttons, { QuestionAnswer::RETRY, QuestionAnswer::SKIP, QuestionAnswer::CANCEL });
	else 
		addButtons(buttons, { QuestionAnswer::RETRY, QuestionAnswer::IGNORE, QuestionAnswer::SKIP, QuestionAnswer::CANCEL });

	rememberForType = false;

	auto buttonRefs = addButtonsLineToUI(builder, buttons, DlgPrefWidth);
	int firstID = buttonRefs[0].ID();
	int button = -1;
	if (builder.ShowDialog(&button)) {
		button -= firstID;
		rememberForType = isRemember;
		return buttons[button];
	}
	return QuestionAnswer::SKIP;
}

uint64_t UITransferManager::addTransferJob(const std::wstring& sourcePath, const std::wstring& destinationDir, TransferMode mode) {
	uint64_t job = TransferManager::addTransferJob(sourcePath, destinationDir, mode);

	// add new block in osd pane and then place it into map
	int row = osd->AddPath(L"⏱️", L"                ", sourcePath);
	osd->AddLine(L"📄   -/   0 📁   -/   0 💾   -/   0 📶   - M/s");
	jobToRow[job] = row;
	return job;
}
