// burnlib.cpp : Defines the entry point for the DLL application.
//

#include "main.h"

extern api_service *serviceManager;
api_language *WASABI_API_LNG_BURN = 0;
HINSTANCE dllInstance = NULL;
HMODULE hResource = NULL;
HWND winampWnd = NULL;

// must be first call before you start using library
void InitializeBurningLibrary(api_service *wasabiServiceManager, HINSTANCE _dllInstance, HWND _winampWnd)
{
	dllInstance = _dllInstance;
	serviceManager = wasabiServiceManager;
	winampWnd = _winampWnd;

	waServiceFactory *sf = serviceManager->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG_BURN = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	hResource = WASABI_API_LNG_BURN->StartLanguageSupport(LoadLibraryW(L"burnlib.dll"),burnlibLangGUID);
}