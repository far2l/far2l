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

namespace openwith::mac_os_api
{
	std::optional<std::string> ResolveFileUTI(const std::string& filepath)
	{
		BEGIN_AUTORELEASE_POOL(pool)
			NSString *ns_filepath = [NSString stringWithUTF8String:filepath.c_str()];
			NSURL *file_url = [NSURL fileURLWithPath:ns_filepath];
			if (!file_url) {
				return std::nullopt;
			}

			NSString *uti = nil;
			NSError *error = nil;
			BOOL success = [file_url getResourceValue:&uti forKey:NSURLTypeIdentifierKey error:&error];
			if (!success || !uti) {
				return std::nullopt;
			}
			return std::string([uti UTF8String]);
		END_AUTORELEASE_POOL
	}


	std::optional<std::string> GetDefaultBundlePath(const std::string& filepath)
	{
		BEGIN_AUTORELEASE_POOL(pool)
			NSString *ns_filepath = [NSString stringWithUTF8String:filepath.c_str()];
			NSURL *file_url = [NSURL fileURLWithPath:ns_filepath];
			if (!file_url) {
				return std::nullopt;
			}

			NSURL* default_app_url = [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:file_url];
			if (default_app_url && [default_app_url path]) {
				return std::string([[default_app_url path] UTF8String]);
			}
		END_AUTORELEASE_POOL
		return std::nullopt;
	}


	std::vector<std::string> GetCompatibleBundlePaths(const std::string& filepath)
	{
		std::vector<std::string> result;
		BEGIN_AUTORELEASE_POOL(pool)
			NSString *ns_filepath = [NSString stringWithUTF8String:filepath.c_str()];
			NSURL *file_url = [NSURL fileURLWithPath:ns_filepath];
			if (!file_url) {
				return result;
			}

			NSArray *all_app_urls;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
			all_app_urls = [[NSWorkspace sharedWorkspace] URLsForApplicationsToOpenURL:file_url];
#else
			// On macOS < 11, URLsForApplicationsToOpenURL is unavailable, so we fallback to a single-element array
			// containing only the default app.
			NSURL* default_app_url = [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:file_url];
#if __has_feature(objc_array_literals)
			all_app_urls = default_app_url ? @[default_app_url] : @[];
#else
			if (default_app_url) {
				all_app_urls = [NSArray arrayWithObject:default_app_url];
			} else {
				all_app_urls = [NSArray array];
			}
#endif
#endif

			for (NSUInteger i = 0; i < [all_app_urls count]; i++) {
				NSURL *app_url = [all_app_urls objectAtIndex:i];
				if (app_url && [app_url path]) {
					result.push_back([[app_url path] UTF8String]);
				}
			}
		END_AUTORELEASE_POOL
		return result;
	}


	std::optional<std::unordered_map<std::string, std::string>> ParseAppBundleMetadata(const std::string& bundle_path, const std::vector<std::string>& keys)
	{
		std::optional<std::unordered_map<std::string, std::string>> result;
		BEGIN_AUTORELEASE_POOL(pool)
			NSString *ns_path = [NSString stringWithUTF8String:bundle_path.c_str()];
			NSURL *app_url = [NSURL fileURLWithPath:ns_path];
			if (!app_url) {
				return result;
			}

			NSBundle *bundle = [NSBundle bundleWithURL:app_url];
			if (!bundle) {
				return result;
			}

			NSDictionary *info_dict = [bundle infoDictionary];
			if (!info_dict) {
				return result;
			}

			std::unordered_map<std::string, std::string> res_map;
			for (const auto& key : keys) {
				NSString *ns_key = [NSString stringWithUTF8String:key.c_str()];
				NSString *ns_value = [info_dict objectForKey:ns_key];
				if (ns_value && [ns_value isKindOfClass:[NSString class]]) {
					res_map[key] = [ns_value UTF8String];
				}
			}
			result = std::move(res_map);
		END_AUTORELEASE_POOL
		return result;
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

} // namespace openwith::mac_os_api

#endif // __APPLE__
