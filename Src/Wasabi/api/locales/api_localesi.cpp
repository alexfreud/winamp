#include <precomp.h>
//<?#include "<class data="implementationheader"/>"
#include "api_localesi.h"
//?>

#include <api/locales/localesmgr.h>
#include <api/wnd/keyboard.h>

api_locales *localesApi = NULL;

api_localesI::api_localesI() {
	LocalesManager::init();
}

api_localesI::~api_localesI() {
	LocalesManager::deinit();
}

const wchar_t *api_localesI::locales_getTranslation(const wchar_t *str) 
{
	return LocalesManager::getTranslation(str);
}

void api_localesI::locales_addTranslation(const wchar_t *from, const wchar_t *to) {
	LocalesManager::addTranslation(from, to);
}

const wchar_t *api_localesI::locales_getBindFromAction(int action) 
{
	return LocalesManager::getBindFromAction(action);
}

/* // TODO: benski> maybe hook up to Winamp 5.5's new lang pack stuff
int api_localesI::locales_getNumEntries() 
{
	return LocalesManager::getNumLocales();
}

const wchar_t *api_localesI::locales_enumEntry(int n) 
{
	return LocalesManager::enumLoadableLocales(n);
}*/

void api_localesI::locales_registerAcceleratorSection(const wchar_t *name, ifc_window *wnd, int global) 
{
	Keyboard::registerAcceleratorSection(name, wnd, global);
}