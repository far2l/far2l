#if defined(__APPLE__)

#include "MacOSAppProvider.hpp"
#include "LaunchServicesBridge.hpp"
#include "common.hpp"
#include "lng.hpp"
#include "KeyFileHelper.h"
#include "utils.h"
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
		const bool is_disabled = false;

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


	const MacOSAppProvider::AppBundleMetadata* MacOSAppProvider::GetOrParseMetadata(const std::string& app_id)
	{
		if (app_id.empty()) {
			return nullptr;
		}

		auto [it, inserted] = _app_bundle_metadata_cache.try_emplace(app_id, std::nullopt);
		if (!inserted) {
			return it->second.has_value() ? &(*it->second) : nullptr;
		}

		std::vector<std::string> keys = {
			"CFBundleDisplayName",
			"CFBundleName",
			"CFBundleShortVersionString",
			"CFBundleVersion",
			"CFBundleExecutable"
		};

		if (auto plist_opt = openwith::mac_bridge::ParseAppBundleMetadata(app_id, keys)) {
			const auto& plist_data = *plist_opt;
			
			AppBundleMetadata metadata;
			metadata.id = app_id;

			std::string bundle_name;
			if (auto k = plist_data.find("CFBundleDisplayName"); k != plist_data.end()) {
				bundle_name = k->second;
			} else if (auto k = plist_data.find("CFBundleName"); k != plist_data.end()) {
				bundle_name = k->second;
			}

			if (!bundle_name.empty()) {
				metadata.name = bundle_name;
				metadata.has_display_name = true;
			} else {
				metadata.name = app_id;
				metadata.has_display_name = false;
			}

			if (auto k = plist_data.find("CFBundleShortVersionString"); k != plist_data.end()) {
				metadata.short_version = k->second;
				metadata.version_string = metadata.short_version;
			}
			if (auto k = plist_data.find("CFBundleVersion"); k != plist_data.end()) {
				metadata.build_version = k->second;
				if (metadata.version_string.empty()) {
					metadata.version_string = metadata.build_version;
				}
			}
			if (auto k = plist_data.find("CFBundleExecutable"); k != plist_data.end()) {
				metadata.executable_name = k->second;
			}

			it->second = std::move(metadata);
			return &(*it->second);
		}

		return nullptr;
	}


	const MacOSAppProvider::AppListForUti& MacOSAppProvider::FetchCompatibleApps(const std::string& filepath, const std::string& uti_std_str)
	{
		auto [it, inserted] = _uti_to_apps_cache.try_emplace(uti_std_str);
		if (!inserted) {
			return it->second;
		}

		AppListForUti& new_cache_entry = it->second;
		auto default_app_id_opt = openwith::mac_bridge::GetDefaultAppId(filepath);
		auto compatible_app_ids = openwith::mac_bridge::GetCompatibleAppIds(filepath);

		if (default_app_id_opt) {
			new_cache_entry.default_app_metadata = GetOrParseMetadata(*default_app_id_opt);
		}

		for (const auto& app_id : compatible_app_ids) {
			if (const auto* meta = GetOrParseMetadata(app_id)) {
				new_cache_entry.compatible_apps_metadata.push_back(meta);
			}
		}

		return new_cache_entry;
	}


	AppProvider::GetCandidatesResult MacOSAppProvider::GetAppCandidates(const std::vector<std::wstring>& filepaths, ProgressCallback progress, const std::atomic<bool>* cancel_flag)
	{
		_last_uti_profiles.clear();
		_app_bundle_metadata_cache.clear();
		_uti_to_apps_cache.clear();

		GetCandidatesResult result;
		if (filepaths.empty()) {
			return result;
		}

		OperationGuard guard(*this, std::move(progress), cancel_flag);

		try {
			std::unordered_map<std::string, RankedCandidate> candidates_pool;
			std::unordered_set<std::string_view> app_ids_seen_for_file;

			ReportProgress({GetMsg(MsgID::IdentifyingUTIsDiscoveringApps), GetMsg(MsgID::PleaseWait)});
			const size_t files_total = filepaths.size();
			size_t files_processed = 0;

			for (const auto& filepath : filepaths) {
				app_ids_seen_for_file.clear();

				wchar_t status_buf[256];
				swprintf(status_buf, std::size(status_buf), GetMsg(MsgID::ProcessingFiles), ++files_processed, files_total);
				ReportProgress({nullptr, status_buf});
				CheckCancellation();

				std::string filepath_str = StrWide2MB(filepath);
				auto uti_opt = openwith::mac_bridge::ResolveFileUTI(filepath_str);
				
				std::string uti_std_str = uti_opt.value_or("");
				bool accessible = uti_opt.has_value();

				_last_uti_profiles.insert({ uti_std_str, accessible });

				if (!accessible || uti_std_str.empty()) {
					continue;
				}

				const AppListForUti& app_list_for_uti = FetchCompatibleApps(filepath_str, uti_std_str);

				// ---------- Per-file scoring and accumulation ----------
				// The app_ids_seen_for_file set prevents scoring the same app twice within a single file
				// (deduplication against the default app potentially appearing in both lists).

				auto register_app = [&](const AppBundleMetadata* metadata, bool is_default, int rank_index, size_t list_size) {
					if (!metadata) return;

					if (!app_ids_seen_for_file.insert(metadata->id).second) {
						return;
					}
					auto [it, inserted] = candidates_pool.try_emplace(metadata->id);

					RankedCandidate& ranked_candidate = it->second;
					if (inserted) {
						ranked_candidate.metadata = metadata;
					}
					if (is_default) {
						ranked_candidate.default_count++;
					}

					double current_file_percentile = (list_size > 1) ? static_cast<double>(rank_index) / (list_size - 1) : 0.0;
					ranked_candidate.match_count++;

					ranked_candidate.mean_suitability_percentile += 
						(current_file_percentile - ranked_candidate.mean_suitability_percentile) / ranked_candidate.match_count;
				};

				std::string_view default_id = app_list_for_uti.default_app_metadata
					? std::string_view(app_list_for_uti.default_app_metadata->id)
					: std::string_view();

				// Iterate all compatible apps returned by Launch Services in their native suitability order.
				const auto& compatible_apps = app_list_for_uti.compatible_apps_metadata;

				for (size_t i = 0; i < compatible_apps.size(); ++i) {
					const auto* metadata = compatible_apps[i];

					const bool is_default = (!default_id.empty() && metadata->id == default_id);
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
			// match_count must equal the total number of files.

			ReportProgress({GetMsg(MsgID::FilteringSortingResults), GetMsg(MsgID::PleaseWait)});

			std::vector<RankedCandidate> ranked_finalists;

			for (auto& [app_id, ranked_candidate] : candidates_pool) {
				if (ranked_candidate.match_count == files_total) {
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
					if (a.default_count != b.default_count) {
						return a.default_count > b.default_count;
					}

					// If enabled, sort by Launch Services suitability (lower mean percentile is better)
					if (use_sys_rank) {
						 constexpr double epsilon = 1e-9;
						 const double diff = a.mean_suitability_percentile - b.mean_suitability_percentile;
						 if (std::abs(diff) > epsilon) {
							 return diff < 0.0; 
						 }
					}

					// Alphabetical tie-breaker when ranks are equal
					return a.metadata->name < b.metadata->name;

			});

			// ---------- Final list generation ----------
			// Convert internal structures to CandidateInfo output.
			// If multiple bundles share
			// the same display name, append the version string to disambiguate them in the UI.

			std::vector<CandidateInfo> out_candidates;
			if (!ranked_finalists.empty()) {
				out_candidates.reserve(ranked_finalists.size());

				std::unordered_map<std::string_view, int> app_name_frequency;
				for (const auto& ranked_finalist : ranked_finalists) {
					app_name_frequency[ranked_finalist.metadata->name]++;
				}

				for (const auto& ranked_finalist : ranked_finalists) {
					CandidateInfo out_candidate;
					out_candidate.id = StrMB2Wide(ranked_finalist.metadata->id);
					out_candidate.terminal = false;
					out_candidate.multi_file_aware = true;

					std::string display_name = ranked_finalist.metadata->name;

					if (app_name_frequency[ranked_finalist.metadata->name] > 1 && !ranked_finalist.metadata->version_string.empty()) {
						display_name += " (" + ranked_finalist.metadata->version_string + ")";

					}
					out_candidate.name = StrMB2Wide(display_name);
					out_candidates.push_back(out_candidate);
				}
			}

			result.candidates = std::move(out_candidates);

		} catch (const OperationCancelledException&) {
			result.was_cancelled = true;
			_last_uti_profiles.clear();
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
		std::vector<Field> details;

		std::string app_id = StrWide2MB(candidate.id);
		auto it = _app_bundle_metadata_cache.find(app_id);

		if (it == _app_bundle_metadata_cache.end() || !it->second.has_value()) {
			return details;
		}

		const auto& metadata = *it->second;

		if (metadata.has_display_name) {
			details.push_back({GetMsg(MsgID::AppName), StrMB2Wide(metadata.name)});
		}

		details.push_back({GetMsg(MsgID::FullPath), candidate.id});

		if (!metadata.executable_name.empty()) {
			details.push_back({GetMsg(MsgID::ExecutableFile), StrMB2Wide(metadata.executable_name)});
		}

		if (!metadata.short_version.empty()) {
			details.push_back({GetMsg(MsgID::Version), StrMB2Wide(metadata.short_version)});
		}

		if (!metadata.build_version.empty()) {
			details.push_back({GetMsg(MsgID::BundleVersion), StrMB2Wide(metadata.build_version)});
		}

		return details;
	}


	std::vector<std::wstring> MacOSAppProvider::GetFileTypes()
	{
		std::unordered_set<std::string> unique_profile_strings;
		unique_profile_strings.reserve(_last_uti_profiles.size());

		for (const auto& profile : _last_uti_profiles) {

			if (!profile.accessible) {
				unique_profile_strings.insert("(inaccessible)");
				continue;
			}

			if (_show_uti_instead_of_mime) {
				if (profile.uti.empty()) {
					unique_profile_strings.insert("(none)");

				} else {
					unique_profile_strings.insert("(" + profile.uti + ")");
				}
				continue;
			}

			// Convert UTI -> MIME type through the bridge.
			std::string out_mime_str = mac_bridge::ConvertUTIToMime(profile.uti);

			if (out_mime_str.empty()) {
				unique_profile_strings.insert("(none)");
			} else {
				unique_profile_strings.insert("(" + out_mime_str + ")");

			}
		}

		std::vector<std::wstring> result_vec;
		result_vec.reserve(unique_profile_strings.size());
		for (const auto& mime_str : unique_profile_strings) {
			result_vec.push_back(StrMB2Wide(mime_str));
		}
		return result_vec;
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
