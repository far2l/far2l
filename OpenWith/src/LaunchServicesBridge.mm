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

#include "LaunchServicesBridge.hpp"

namespace
{
#ifndef __clang__

	// GCC/MRC: @autoreleasepool does not drain on C++ exceptions.
	// This RAII wrapper ensures the pool is always drained.
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

} // namespace

namespace openwith::mac_bridge
{
	std::pair<std::string, bool> ResolveFileUTI(const std::string& filepath)
	{
		BEGIN_AUTORELEASE_POOL(pool)
			NSString *ns_filepath = [NSString stringWithUTF8String:filepath.c_str()];
			NSURL *file_url = [NSURL fileURLWithPath:ns_filepath];
			if (!file_url) {
				return { "", false };
			}

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
		END_AUTORELEASE_POOL
	}


	AppSupportList FetchCompatibleAppIds(const std::string& filepath)
	{
		AppSupportList result;
		BEGIN_AUTORELEASE_POOL(pool)
			NSString *ns_filepath = [NSString stringWithUTF8String:filepath.c_str()];
			NSURL *file_url = [NSURL fileURLWithPath:ns_filepath];
			if (!file_url) {
				return result;
			}

			// 1. Узнаем дефолтное приложение
			NSURL* default_app_url = [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:file_url];
			if (default_app_url && [default_app_url path]) {
				result.default_app_id = std::string([[default_app_url path] UTF8String]);
			}

			NSArray *all_app_urls;

			// 2. Получаем все поддерживаемые приложения (с учетом обратной совместимости)
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
				if (app_url && [app_url path]) {
					result.compatible_app_ids.push_back([[app_url path] UTF8String]);
				}
			}
		END_AUTORELEASE_POOL
		return result;
	}


	std::optional<openwith::AppBundleMetadata> ParseAppBundleMetadata(const std::string& app_id)
	{
		BEGIN_AUTORELEASE_POOL(pool)
			NSString *ns_path = [NSString stringWithUTF8String:app_id.c_str()];
			NSURL *app_url = [NSURL fileURLWithPath:ns_path];
			if (!app_url) {
				return std::nullopt;
			}

			NSBundle *bundle = [NSBundle bundleWithURL:app_url];
			if (!bundle) {
				return std::nullopt;
			}

			NSDictionary *info_dict = [bundle infoDictionary];
			NSString *bundle_name = [info_dict objectForKey:@"CFBundleDisplayName"] ?: [info_dict objectForKey:@"CFBundleName"];
			NSString *bundle_short_version = [info_dict objectForKey:@"CFBundleShortVersionString"];
			NSString *bundle_version = [info_dict objectForKey:@"CFBundleVersion"];
			NSString *bundle_executable = [info_dict objectForKey:@"CFBundleExecutable"];

			openwith::AppBundleMetadata metadata;
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

			return metadata;
		END_AUTORELEASE_POOL
	}


	std::string ConvertUTIToMime(const std::string& uti_str)
	{
		std::string out_mime_str;
		BEGIN_AUTORELEASE_POOL(pool)
			NSString *uti = [NSString stringWithUTF8String:uti_str.c_str()];
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
		END_AUTORELEASE_POOL
		return out_mime_str;
	}

} // namespace openwith::mac_bridge

#endif // __APPLE__
