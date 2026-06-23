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


void AppProvider::Initialize(TMsgGetter msg_getter)
{
	if (s_provider.has_value()) {
		fprintf(stderr, "OpenWith: AppProvider::Initialize() called more than once, ignoring.\n");
		return;
	}

	if (!msg_getter) {
		fprintf(stderr, "OpenWith: AppProvider::Initialize() called with nullptr!\n");
		return;
	}

	s_provider = CreateAppProvider(msg_getter);
}


AppProvider* AppProvider::GetInstance()
{
	if (!s_provider.has_value()) {
		fprintf(stderr, "OpenWith: AppProvider::GetInstance() called before Initialize()!\n");
		return nullptr;
	}
	return s_provider->get();
}
