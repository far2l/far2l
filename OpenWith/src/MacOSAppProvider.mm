#if defined(__APPLE__)

#include <AvailabilityMacros.h>

// If the compiler (e.g., GNU GCC) does not support Clang feature checking macros,
// safely define them to 0 to fallback to classic Objective-C.
#ifndef __has_feature
	#define __has_feature(x) 0
#endif
#ifndef __has_extension
	#define __has_extension __has_feature
#endif

#import <Cocoa/Cocoa.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
	#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#endif

#include "MacOSAppProvider.hpp"
#include "common.hpp"
#include "lng.hpp"
#include "KeyFileHelper.h"
#include "utils.h"
#include "WideMB.h"
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

namespace
{
#ifndef __clang__

	// GCC/MRC: @autoreleasepool does not drain on C++ exceptions.
	// This RAII wrapper ensures the pool
	// is always drained, even when OperationCancelledException propagates through the loop body.

	struct AutoreleasePoolGuard
	{
		NSAutoreleasePool *pool;
		AutoreleasePoolGuard() : pool([[NSAutoreleasePool alloc] init]) {}
		~AutoreleasePoolGuard() { [pool drain]; }
		AutoreleasePoolGuard(const AutoreleasePoolGuard&) = delete;
		AutoreleasePoolGuard& operator=(const AutoreleasePoolGuard&) = delete;
	};
#endif

#ifdef __clang__
	#define BEGIN_AUTORELEASE_POOL(name) @autoreleasepool {
	#define END_AUTORELEASE_POOL         }
#else
	#define BEGIN_AUTORELEASE_POOL(name) { AutoreleasePoolGuard name;
	#define END_AUTORELEASE_POOL         }
#endif

	std::string NSURLToPath(NSURL *url)
	{
		if (!url) {
			return {};
		}
		return std::string([[url path] UTF8String]);
	}


	std::pair<std::string, bool> ResolveUtiForFile(NSURL* file_url)
	{
		NSString *uti = nil;
		NSError *error = nil;
		BOOL success = [file_url getResourceValue:&uti forKey:NSURLTypeIdentifierKey error:&error];
		if (!success) {
			return { "", false };
		}
		if (!uti) {
			return { "", true };
		}
		return { std::string([uti UTF8String]), true };
	}
}


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


	AppProvider::GetCandidatesResult MacOSAppProvider::GetAppCandidates(const std::vector<std::wstring>& filepaths, ProgressCallback progress, const std::atomic<bool>* cancel_flag)
	{
		_last_uti_profiles.clear();

		GetCandidatesResult result;
		if (filepaths.empty()) {
			return result;
		}

		OperationGuard guard(*this, std::move(progress), cancel_flag);

		BEGIN_AUTORELEASE_POOL(outer_pool_guard)
			try {
				DiscoveryService discovery_service(_app_bundle_metadata_cache);
				std::unordered_map<std::string, RankedCandidate> candidates_pool;
				std::unordered_set<std::string> app_ids_seen_for_file;

				ReportProgress({GetMsg(MsgID::IdentifyingUTIsDiscoveringApps), GetMsg(MsgID::PleaseWait)});
				const size_t files_total = filepaths.size();
				size_t files_processed = 0;

				for (const auto& filepath : filepaths) {
					app_ids_seen_for_file.clear();

					wchar_t status_buf[256];
					swprintf(status_buf, std::size(status_buf), GetMsg(MsgID::ProcessingFiles), ++files_processed, files_total);
					ReportProgress({nullptr, status_buf});
					CheckCancellation();

					BEGIN_AUTORELEASE_POOL(inner_pool_guard)

						NSString *ns_filepath = [NSString stringWithUTF8String:StrWide2MB(filepath).c_str()];
						NSURL *file_url = [NSURL fileURLWithPath:ns_filepath];
						if (!file_url) {
							_last_uti_profiles.insert({ std::string(""), false });
							continue;
						}

						auto [uti_std_str, accessible] = ResolveUtiForFile(file_url);
						_last_uti_profiles.insert({ uti_std_str, accessible });

						if (!accessible || uti_std_str.empty()) {
							continue;
						}

						const AppListForUti& app_list_for_uti = discovery_service.FetchCompatibleApps(uti_std_str, file_url);

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

					END_AUTORELEASE_POOL
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
		END_AUTORELEASE_POOL

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
		if (it == _app_bundle_metadata_cache.end()) {
			return details;
		}

		const auto& metadata = it->second;

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

		BEGIN_AUTORELEASE_POOL(pool_guard)

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

				// Convert UTI -> MIME type.
				std::string out_mime_str;
				NSString *uti = [NSString stringWithUTF8String:profile.uti.c_str()];
				if (uti && [uti length] > 0) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 110000 // UTType is available on macOS 11.0+
					// Modern approach for macOS 11.0 and later, converting a UTI to a MIME type.
					UTType *type = [UTType typeWithIdentifier:uti];
					if (type) {
						if (NSString *ns_mime_str = type.preferredMIMEType) {
							if (const char *utf8_ptr = [ns_mime_str UTF8String]) {
								out_mime_str = utf8_ptr;
							}
						}
					}
#else
					// Legacy approach for older macOS versions.
#if __has_feature(objc_arc)
					CFStringRef cf_mime_tag = UTTypeCopyPreferredTagWithClass((__bridge CFStringRef)uti,
																		   kUTTagClassMIMEType);
					if (cf_mime_tag) {
						// Transfer ownership of the CFStringRef to ARC.
						NSString *ns_mime_str = (__bridge_transfer NSString *)cf_mime_tag;
						if (const char *utf8_ptr = [ns_mime_str UTF8String]) {
							out_mime_str = utf8_ptr;
						}
					}
#else // Manual Retain-Release (MRC) mode or GCC.
					CFStringRef cf_mime_tag = UTTypeCopyPreferredTagWithClass((CFStringRef)uti,
																		   kUTTagClassMIMEType);
					if (cf_mime_tag) {
						NSString *ns_mime_str = [(NSString *)cf_mime_tag autorelease];
						if (const char *utf8_ptr = [ns_mime_str UTF8String]) {
							out_mime_str = utf8_ptr;
						}
					}
#endif
#endif
				}

				if (out_mime_str.empty()) {
					unique_profile_strings.insert("(none)");
				} else {
					unique_profile_strings.insert("(" + out_mime_str + ")");
				}
			}
		END_AUTORELEASE_POOL

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


	class DiscoveryService
	{
	public:
		explicit DiscoveryService(std::unordered_map<std::string, AppBundleMetadata>& meta_cache)
			: _meta_cache(meta_cache)
		{
		}

		const AppListForUti& FetchCompatibleApps(const std::string& uti_std_str, NSURL* file_url)
		{
			// Check whether we have already queried Launch Services for this UTI.
			// If not, perform two queries:
			//   1. URLForApplicationToOpenURL: returns the default handler for this file.
			//   2. URLsForApplicationsToOpenURL: returns all apps that claim support.
			// On macOS < 11, URLsForApplicationsToOpenURL is unavailable, so we fallback to a single-element array
			// containing only the default app.
			auto cache_it = _uti_to_apps_cache.find(uti_std_str);
			if (cache_it != _uti_to_apps_cache.end()) {
				return cache_it->second;
			}

			AppListForUti new_cache_entry;

			NSURL* default_app_url = [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:file_url];
			if (default_app_url) {
				new_cache_entry.default_app_metadata = ParseAppBundleMetadata(default_app_url);
			}

			NSArray *all_app_urls;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
			all_app_urls = [[NSWorkspace sharedWorkspace] URLsForApplicationsToOpenURL:file_url];
#elif __has_feature(objc_array_literals)
			all_app_urls = default_app_url ? @[default_app_url] : @[];
#else
			if (default_app_url) {
				all_app_urls = [NSArray arrayWithObject:default_app_url];
			} else {
				all_app_urls = [NSArray array];
			}
#endif

			for (NSUInteger i = 0; i < [all_app_urls count]; i++) {
				NSURL *app_url = [all_app_urls objectAtIndex:i];
				if (const auto* meta = ParseAppBundleMetadata(app_url)) {
					new_cache_entry.compatible_apps_metadata.push_back(meta);
				}
			}

			return _uti_to_apps_cache.insert({uti_std_str, std::move(new_cache_entry)}).first->second;
		}

	private:
		const AppBundleMetadata* ParseAppBundleMetadata(NSURL *app_url)
		{
			std::string app_id = NSURLToPath(app_url);
			if (app_id.empty()) {
				return nullptr;
			}

			if (auto it = _meta_cache.find(app_id); it != _meta_cache.end()) {
				return &it->second;
			}

			NSBundle *bundle = [NSBundle bundleWithURL:app_url];
			NSDictionary *info_dict = [bundle infoDictionary];
			NSString *bundle_name = [info_dict objectForKey:@"CFBundleDisplayName"] ?: [info_dict objectForKey:@"CFBundleName"];
			NSString *bundle_short_version = [info_dict objectForKey:@"CFBundleShortVersionString"];
			NSString *bundle_version = [info_dict objectForKey:@"CFBundleVersion"];
			NSString *bundle_executable = [info_dict objectForKey:@"CFBundleExecutable"];

			AppBundleMetadata metadata;
			metadata.id = app_id;

			if (bundle_name) {
				metadata.name = [bundle_name UTF8String];
				metadata.has_display_name = true;
			} else {
				metadata.name = app_id;
				metadata.has_display_name = false;
			}

			if (bundle_short_version) {
				metadata.short_version = [bundle_short_version UTF8String];
				metadata.version_string = metadata.short_version;
			}
			if (bundle_version) {
				metadata.build_version = [bundle_version UTF8String];
				if (metadata.version_string.empty()) {
					metadata.version_string = metadata.build_version;
				}
			}
			if (bundle_executable) {
				metadata.executable_name = [bundle_executable UTF8String];
			}

			auto [it, inserted] = _meta_cache.insert({app_id, std::move(metadata)});
			const auto& new_metadata = it->second;
			return &new_metadata;
		}

		std::unordered_map<std::string, AppBundleMetadata>& _meta_cache;
		std::unordered_map<std::string, AppListForUti> _uti_to_apps_cache;
	};
} // namespace openwith
#endif // __APPLE__
