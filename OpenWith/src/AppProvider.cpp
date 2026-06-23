#include "AppProvider.hpp"

#include "XDGBasedAppProvider.hpp"
#include "MacOSAppProvider.hpp"

std::unique_ptr<AppProvider> AppProvider::CreateAppProvider(TMsgGetter msg_getter)
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	return std::make_unique<XDGBasedAppProvider>(msg_getter);
#elif defined(__APPLE__)
	return std::make_unique<MacOSAppProvider>(msg_getter);
#else
	return nullptr;
#endif
}


AppProvider* AppProvider::GetOrInitialize(TMsgGetter msg_getter)
{
	static std::unique_ptr<AppProvider> s_provider = [msg_getter]() -> std::unique_ptr<AppProvider> {
		if (!msg_getter) {
			fprintf(stderr, "OpenWith: AppProvider::GetOrInitialize() called with nullptr!\n");
			return nullptr;
		}
		return CreateAppProvider(msg_getter);
	}();

	return s_provider.get();
}


void AppProvider::Initialize(TMsgGetter msg_getter)
{
	if (!msg_getter) {
		fprintf(stderr, "OpenWith: AppProvider::Initialize() called with nullptr!\n");
	}
	GetOrInitialize(msg_getter);
}


AppProvider* AppProvider::GetInstance()
{
	return GetOrInitialize(nullptr);
}