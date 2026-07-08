#pragma once

#if defined(__APPLE__)

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace openwith::mac_os_api
{
	// Возвращает UTI файла, либо std::nullopt, если файл недоступен или произошла ошибка.
	std::optional<std::string> ResolveFileUTI(const std::string& filepath);

	// Обращается к Launch Services и возвращает путь (ID) приложения по умолчанию.
	std::optional<std::string> GetDefaultBundlePath(const std::string& filepath);

	// Обращается к Launch Services и возвращает пути (ID) всех совместимых приложений.
	std::vector<std::string> GetCompatibleBundlePaths(const std::string& filepath);

	// Извлекает данные из бандла (Info.plist) по указанному пути для заданного списка ключей.
	std::optional<std::unordered_map<std::string, std::string>> ParseAppBundleMetadata(const std::string& bundle_path, const std::vector<std::string>& keys);

	// Конвертирует системный UTI в стандартный MIME-тип.
	std::string ConvertUTIToMime(const std::string& uti);
}

#endif // __APPLE__
