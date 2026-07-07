#pragma once

#if defined(__APPLE__)

#include "AppProvider.hpp"
#include "common.hpp"
#include "lng.hpp"
#include <atomic>
#include <map>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class KeyFileReadHelper;
class KeyFileHelper;

namespace openwith
{
	struct AppBundleMetadata
	{
		std::string name;
		std::string id;
		std::string version_string;
		std::string short_version;
		std::string build_version;
		std::string executable_name;
		bool has_display_name = false;
	};


	class MacOSAppProvider : public AppProvider
	{
	public:
		MacOSAppProvider();
		GetCandidatesResult GetAppCandidates(const std::vector<std::wstring>& filepaths,  ProgressCallback progress = nullptr,
											 const std::atomic<bool>* cancel_flag = nullptr) override;
		std::vector<std::wstring> ConstructLaunchCommands(const CandidateInfo& candidate, const std::vector<std::wstring>& filepaths) override;
		std::vector<std::wstring> GetFileTypes() override;
		std::vector<Field> GetCandidateDetails(const CandidateInfo& candidate) override;

		std::vector<ProviderSetting> GetPlatformSettings() override;
		void SetPlatformSettings(const std::vector<ProviderSetting>& settings) override;
		void LoadPlatformSettings(const KeyFileReadHelper &key_reader) override;
		void SavePlatformSettings(KeyFileHelper& key_writer) override;


	private:

		struct MacFileProfile
		{
			std::string uti; // UTI, or an empty string if the file is inaccessible.
			bool accessible; // true if the file was accessible and its UTI was successfully resolved.

			bool operator==(const MacFileProfile& other) const
			{
				return accessible == other.accessible && uti == other.uti;
			}

			struct Hash
			{
				std::size_t operator()(const MacFileProfile& p) const noexcept
				{
					std::size_t h1 = std::hash<std::string>{}(p.uti);
					std::size_t h2 = std::hash<bool>{}(p.accessible);
					std::size_t seed = h1;
					seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
					return seed;
				}
			};
		};


		struct PlatformSettingDefinition
		{
			std::string internal_key;                 // persistent INI key and internal identifier
			MsgID display_name_id;                    // ID to fetch the localized UI label
			bool MacOSAppProvider::* member_variable; // pointer to the linked boolean class member
			bool default_value;                       // fallback value if missing in the INI file
			bool affects_candidates;                  // true if changing this setting affects the contents or order of the candidate list
		};


		static std::wstring EscapeForShell(const std::wstring& arg);

		std::map<std::wstring, bool MacOSAppProvider::*> _key_wide_to_member_map;
		std::vector<PlatformSettingDefinition> _platform_settings_definitions;
		std::unordered_set<MacFileProfile, MacFileProfile::Hash> _last_uti_profiles;
		std::unordered_map<std::string, AppBundleMetadata> _app_bundle_metadata_cache;

		bool _show_uti_instead_of_mime;
		bool _respect_system_ranking;
		bool _sort_alphabetically;
	};
} // namespace openwith

#endif
