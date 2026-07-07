#pragma once

#if defined(__APPLE__)

#include "MacOSAppProvider.hpp"
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace openwith::mac_bridge
{
	struct AppSupportList
	{
		std::string default_app_id;
		std::vector<std::string> compatible_app_ids;
	};

	// Возвращает пару {UTI, is_accessible} для переданного пути файла.
	std::pair<std::string, bool> ResolveFileUTI(const std::string& filepath);

	// Обращается к Launch Services и возвращает пути (ID) совместимых приложений.
	AppSupportList FetchCompatibleAppIds(const std::string& filepath);

	// Извлекает данные из бандла (Info.plist) по указанному пути.
	std::optional<openwith::AppBundleMetadata> ParseAppBundleMetadata(const std::string& app_id);

	// Конвертирует системный UTI в стандартный MIME-тип.
	std::string ConvertUTIToMime(const std::string& uti);
}

#endif // __APPLE__
