#pragma once

#if defined(__APPLE__)

#include "MacOSAppProvider.hpp"
#include <optional>
#include <string>
#include <vector>

namespace openwith::mac_bridge
{
	// Возвращает UTI файла, либо std::nullopt, если файл недоступен или произошла ошибка.
	std::optional<std::string> ResolveFileUTI(const std::string& filepath);

	// Обращается к Launch Services и возвращает путь (ID) приложения по умолчанию.
	std::optional<std::string> GetDefaultAppId(const std::string& filepath);

	// Обращается к Launch Services и возвращает пути (ID) всех совместимых приложений.
	std::vector<std::string> GetCompatibleAppIds(const std::string& filepath);

	// Извлекает данные из бандла (Info.plist) по указанному пути.
	std::optional<openwith::AppBundleMetadata> ParseAppBundleMetadata(const std::string& app_id);

	// Конвертирует системный UTI в стандартный MIME-тип.
	std::string ConvertUTIToMime(const std::string& uti);
}

#endif // __APPLE__
