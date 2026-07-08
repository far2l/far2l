#if defined(__APPLE__)

#include "MacOSAppProvider.hpp"
#include "LaunchServicesBridge.hpp"
#include "common.hpp"
#include "lng.hpp"
#include "KeyFileHelper.h"
#include "WideMB.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>


namespace openwith
{
	constexpr const char* INI_SECTION_MAC_PROVIDER = "Settings.MAC";

	MacOSAppProvider::MacOSAppProvider()
	{
		_platform_settings_definitions = {
			{ "ShowUtiInsteadOfMime", MsgID::ShowUtiInsteadOfMime, &MacOSAppProvider::_show_uti_instead_of_mime, false, false},
			{ "RespectSystemRanking", MsgID::RespectSystemRanking, &MacOSAppProvider::_respect_system_ranking,   true,  true},
			{ "SortAlphabetically",   MsgID::SortAlphabetically,   &MacOSAppProvider::_sort_alphabetically,      false, true}
		};

		for (const auto& def : _platform_settings_definitions) {
			_key_wide_to_member_map[StrMB2Wide(def.internal_key)] = def.member_variable;
			this->*(def.member_variable) = def.default_value;
		}
	}


	void MacOSAppProvider::LoadPlatformSettings(const KeyFileReadHelper &key_reader)
	{
		for (const auto& def : _platform_settings_definitions) {
			this->*(def.member_variable) = key_reader.GetInt(INI_SECTION_MAC_PROVIDER, def.internal_key, def.default_value) != 0;
		}
	}


	void MacOSAppProvider::SavePlatformSettings(KeyFileHelper& key_writer)
	{
		for (const auto& def : _platform_settings_definitions) {
			key_writer.SetInt(INI_SECTION_MAC_PROVIDER, def.internal_key, this->*(def.member_variable));
		}
	}


	std::vector<ProviderSetting> MacOSAppProvider::GetPlatformSettings()
	{
		std::vector<ProviderSetting> settings;
		settings.reserve(_platform_settings_definitions.size());
		constexpr bool is_disabled = false;

		for (const auto& def : _platform_settings_definitions) {
			settings.push_back({StrMB2Wide(def.internal_key), GetMsg(def.display_name_id), this->*(def.member_variable),
								is_disabled, def.affects_candidates});
		}
		return settings;
	}


	void MacOSAppProvider::SetPlatformSettings(const std::vector<ProviderSetting>& settings)
	{
		for (const auto& s : settings) {
			if (auto it = _key_wide_to_member_map.find(s.internal_key); it != _key_wide_to_member_map.end()) {
				this->*(it->second) = s.value;
			}
		}
	}


	std::vector<CandidateContextLocation> MacOSAppProvider::GetCandidateContextLocations(const CandidateInfo& candidate)
	{
		return {{GetMsg(MsgID::GoToBundle), candidate.id}};
	}


	void MacOSAppProvider::ClearLastQueryCaches()
	{
		_last_uti_profiles.clear();
		_app_bundle_metadata_cache.clear();
		_uti_to_apps_cache.clear();
	}


	const MacOSAppProvider::AppBundleMetadata* MacOSAppProvider::GetOrParseMetadata(const std::string& bundle_path)
	{
		if (bundle_path.empty()) {
			return nullptr;
		}

		auto [it, inserted] = _app_bundle_metadata_cache.try_emplace(bundle_path, std::nullopt);
		std::optional<AppBundleMetadata>& metadata_opt = it->second;
		if (!inserted) {
			return metadata_opt.has_value() ? &(*metadata_opt) : nullptr;
		}

		static constexpr std::pair<const char*, std::string AppBundleMetadata::*> plist_map[] = {
			{"CFBundleDisplayName",        &AppBundleMetadata::bundle_display_name},
			{"CFBundleName",               &AppBundleMetadata::bundle_name},
			{"CFBundleShortVersionString", &AppBundleMetadata::bundle_short_version_string},
			{"CFBundleVersion",            &AppBundleMetadata::bundle_version},
			{"CFBundleExecutable",         &AppBundleMetadata::bundle_executable},
			{"CFBundleIdentifier",         &AppBundleMetadata::bundle_identifier}
		};

		static const std::vector<std::string> keys = [] {
				std::vector<std::string> v;
				v.reserve(std::size(plist_map));
				for (const auto& item : plist_map) {
					v.emplace_back(item.first);
				}
				return v;
			}();

		if (auto plist_opt = openwith::launch_services::ParseAppBundleMetadata(bundle_path, keys)) {
			const auto& plist_data = *plist_opt;

			AppBundleMetadata metadata;
			metadata.bundle_path = bundle_path;

			for (const auto& [key, member_ptr] : plist_map) {
				if (auto k = plist_data.find(key); k != plist_data.end()) {
					metadata.*member_ptr = k->second;
				}
			}

			metadata.name = std::string(GetFirstNonEmpty({
				metadata.bundle_display_name,
				metadata.bundle_name,
				metadata.bundle_executable,
				bundle_path
			}));

			metadata.version = std::string(GetFirstNonEmpty({
				metadata.bundle_short_version_string,
				metadata.bundle_version
			}));

			metadata_opt.emplace(std::move(metadata));
			return &(*metadata_opt);
		}

		return nullptr;
	}


	const MacOSAppProvider::AppListForUti& MacOSAppProvider::FetchCompatibleApps(const std::string& filepath, const std::string& uti)
	{
		auto [it, inserted] = _uti_to_apps_cache.try_emplace(uti);
		if (!inserted) {
			return it->second;
		}

		AppListForUti& new_cache_entry = it->second;

		auto default_bundle_path_opt = openwith::launch_services::GetDefaultBundlePath(filepath);
		auto compatible_bundle_paths = openwith::launch_services::GetCompatibleBundlePaths(filepath);

		if (default_bundle_path_opt) {
			new_cache_entry.default_app_metadata = GetOrParseMetadata(*default_bundle_path_opt);
		}

		for (const auto& bundle_path : compatible_bundle_paths) {
			if (const auto* meta = GetOrParseMetadata(bundle_path)) {
				new_cache_entry.compatible_apps_metadata.push_back(meta);
			}
		}

		return new_cache_entry;
	}


	AppProvider::GetCandidatesResult MacOSAppProvider::GetAppCandidates(const std::vector<std::wstring>& filepaths, ProgressCallback progress, const std::atomic<bool>* cancel_flag)
	{
		ClearLastQueryCaches();
		if (filepaths.empty()) {
			return {};
		}

		OperationGuard guard(*this, std::move(progress), cancel_flag);
		GetCandidatesResult result;

		try {
			std::unordered_map<std::string, RankedCandidate> candidates_pool;
			std::unordered_set<std::string_view> bundle_paths_seen_for_file;

			ReportProgress({GetMsg(MsgID::IdentifyingUTIsDiscoveringApps), GetMsg(MsgID::PleaseWait)});
			const size_t files_total = filepaths.size();
			size_t files_processed = 0;

			for (const auto& filepath_wide : filepaths) {
				bundle_paths_seen_for_file.clear();

				wchar_t status_buf[256];
				swprintf(status_buf, std::size(status_buf), GetMsg(MsgID::ProcessingFiles), ++files_processed, files_total);
				ReportProgress({nullptr, status_buf});
				CheckCancellation();

				std::string filepath = StrWide2MB(filepath_wide);
				auto uti_opt = openwith::launch_services::ResolveFileUTI(filepath);

				static const std::string empty_string;
				const std::string& uti = uti_opt ? *uti_opt : empty_string;
				bool accessible = uti_opt.has_value();

				_last_uti_profiles.insert({ uti, accessible });

				if (!accessible || uti.empty()) {
					continue;
				}

				const AppListForUti& app_list_for_uti = FetchCompatibleApps(filepath, uti);

				// ---------- Per-file scoring and accumulation ----------
				// The bundle_paths_seen_for_file set prevents scoring the same app twice within a single file
				// (deduplication against the default app potentially appearing in both lists).

				auto register_app = [&](const AppBundleMetadata* metadata, bool is_default, int rank_index, size_t list_size) {
					if (!metadata || (metadata->bundle_path == filepath)|| (!bundle_paths_seen_for_file.insert(metadata->bundle_path).second)) {
						return;
					}
					auto [it, inserted] = candidates_pool.try_emplace(metadata->bundle_path);
					RankedCandidate& ranked_candidate = it->second;
					if (inserted) {
						ranked_candidate.metadata = metadata;
					}
					if (is_default) {
						ranked_candidate.default_handler_count++;
					}
					double current_file_suitability_rank = (list_size > 1) ? static_cast<double>(rank_index) / (list_size - 1) : 0.0;
					ranked_candidate.supported_files_count++;
					ranked_candidate.avg_suitability_rank +=
						(current_file_suitability_rank - ranked_candidate.avg_suitability_rank) / ranked_candidate.supported_files_count;
				};

				std::string_view default_bundle_path = app_list_for_uti.default_app_metadata
					? std::string_view(app_list_for_uti.default_app_metadata->bundle_path)
					: std::string_view();

				const auto& compatible_apps = app_list_for_uti.compatible_apps_metadata;

				for (size_t i = 0; i < compatible_apps.size(); ++i) {
					const auto* metadata = compatible_apps[i];
					const bool is_default = (!default_bundle_path.empty() && metadata->bundle_path == default_bundle_path);
					register_app(metadata, is_default, static_cast<int>(i), compatible_apps.size());
				}

				// Fallback: Ensure the default app is registered even if Launch Services omitted it
				// from URLsForApplicationsToOpenURL (e.g., on older macOS versions or legacy handlers).

				if (app_list_for_uti.default_app_metadata) {
					size_t effective_size = std::max(size_t(1), app_list_for_uti.compatible_apps_metadata.size());
					register_app(app_list_for_uti.default_app_metadata, true, 0, effective_size);
				}
			}

			// ---------- Filtering and sorting ----------
			// Only applications that can open every selected file survive the intersection.

			ReportProgress({GetMsg(MsgID::FilteringSortingResults), GetMsg(MsgID::PleaseWait)});

			std::vector<RankedCandidate> ranked_finalists;

			for (auto& [bundle_path, ranked_candidate] : candidates_pool) {
				if (ranked_candidate.supported_files_count == files_total) {
					ranked_finalists.push_back(std::move(ranked_candidate));
				}
			}

			std::sort(ranked_finalists.begin(), ranked_finalists.end(),
				[alphabetical = _sort_alphabetically,
				 use_sys_rank = _respect_system_ranking](const RankedCandidate& a, const RankedCandidate& b) {

					// If strict global alphabetical sorting is enabled, it overrides everything
					if (alphabetical) {
						return a.metadata->name < b.metadata->name;
					}

					// Default handlers for the selected files always take top priority
					if (a.default_handler_count != b.default_handler_count) {
						return a.default_handler_count > b.default_handler_count;
					}

					// If enabled, sort by Launch Services suitability (lower average rank is better)
					if (use_sys_rank) {
						 return a.avg_suitability_rank < b.avg_suitability_rank;
					}

					// Alphabetical tie-breaker when ranks are equal
					return a.metadata->name < b.metadata->name;

			});

			// ---------- Final list generation ----------
			// Convert internal structures to CandidateInfo output. If multiple bundles share
			// the same name, append the version to disambiguate them in the UI.

			std::vector<CandidateInfo> out_candidates;
			if (!ranked_finalists.empty()) {
				out_candidates.reserve(ranked_finalists.size());

				std::unordered_map<std::string_view, int> app_name_frequency;
				for (const auto& ranked_finalist : ranked_finalists) {
					app_name_frequency[ranked_finalist.metadata->name]++;
				}

				for (const auto& ranked_finalist : ranked_finalists) {
					CandidateInfo out_candidate;
					out_candidate.id = StrMB2Wide(ranked_finalist.metadata->bundle_path);
					out_candidate.terminal = false;
					out_candidate.multi_file_aware = true;
					std::string name_for_menu = ranked_finalist.metadata->name;
					if (app_name_frequency[ranked_finalist.metadata->name] > 1 && !ranked_finalist.metadata->version.empty()) {
						name_for_menu += " (" + ranked_finalist.metadata->version + ")";
					}
					out_candidate.name = StrMB2Wide(name_for_menu);
					out_candidates.push_back(out_candidate);
				}
			}

			result.candidates = std::move(out_candidates);

		} catch (const OperationCancelledException&) {
			ClearLastQueryCaches();
			result.was_cancelled = true;
		}
		return result;

	}


	std::vector<std::wstring> MacOSAppProvider::ConstructLaunchCommands(const CandidateInfo& candidate, const std::vector<std::wstring>& filepaths)
	{
		if (candidate.id.empty() || filepaths.empty()) {
			return {};
		}
		// The 'open -a <app_path>' command tells the system to open files with a specific application.
		std::wstring cmd = L"open -a " + EscapeForShell(candidate.id);
		for (const auto& filepath : filepaths) {
			cmd += L" " + EscapeForShell(filepath);

		}
		return {cmd};
	}


	std::vector<Field> MacOSAppProvider::GetCandidateDetails(const CandidateInfo& candidate)
	{
		std::string bundle_path = StrWide2MB(candidate.id);
		auto it = _app_bundle_metadata_cache.find(bundle_path);
		if (it == _app_bundle_metadata_cache.end() || !it->second.has_value()) {
			return {};
		}
		const auto& metadata = *it->second;

		static constexpr std::pair<MsgID, std::string AppBundleMetadata::*> field_map[] = {
			{MsgID::Location,                 &AppBundleMetadata::bundle_path},
			{MsgID::BundleDisplayName,        &AppBundleMetadata::bundle_display_name},
			{MsgID::BundleName,               &AppBundleMetadata::bundle_name},
			{MsgID::BundleShortVersionString, &AppBundleMetadata::bundle_short_version_string},
			{MsgID::BundleVersion,            &AppBundleMetadata::bundle_version},
			{MsgID::BundleExecutable,         &AppBundleMetadata::bundle_executable},
			{MsgID::BundleIdentifier,         &AppBundleMetadata::bundle_identifier}
		};

		std::vector<Field> details;
		for (const auto& [msg_id, member_ptr] : field_map) {
			const std::string& val = metadata.*member_ptr;
			if (!val.empty()) {
				details.push_back({GetMsg(msg_id), StrMB2Wide(val)});
			}
		}

		return details;
	}


	std::vector<std::wstring> MacOSAppProvider::GetFileTypes()
	{
		std::unordered_set<std::string> unique_filetypes;
		unique_filetypes.reserve(_last_uti_profiles.size());

		for (const auto& profile : _last_uti_profiles) {

			if (!profile.accessible) {
				unique_filetypes.insert("(inaccessible)");
				continue;
			}

			if (_show_uti_instead_of_mime) {
				if (profile.uti.empty()) {
					unique_filetypes.insert("(none)");
				} else {
					unique_filetypes.insert("(" + profile.uti + ")");
				}
				continue;
			}

			std::string mimetype = openwith::launch_services::ConvertUTIToMime(profile.uti);

			if (mimetype.empty()) {
				unique_filetypes.insert("(none)");
			} else {
				unique_filetypes.insert("(" + mimetype + ")");
			}
		}

		std::vector<std::wstring> out_filetypes;
		out_filetypes.reserve(unique_filetypes.size());

		for (const auto& filetype : unique_filetypes) {
			out_filetypes.push_back(StrMB2Wide(filetype));
		}
		return out_filetypes;
	}


	std::string_view MacOSAppProvider::GetFirstNonEmpty(std::initializer_list<std::string_view> items)
	{
		for (const auto& item : items) {
			if (!item.empty()) {
				return item;
			}
		}
		return {};
	}


	std::wstring MacOSAppProvider::EscapeForShell(const std::wstring& arg)
	{
		std::wstring out;
		out.push_back(L'\'');

		for (wchar_t c : arg) {
			if (c == L'\'') {
				out.append(L"'\\''");
			} else {
				out.push_back(c);
			}
		}
		out.push_back(L'\'');
		return out;
	}

} // namespace openwith
#endif // __APPLE__
