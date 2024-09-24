// the glue!
#include <precomp.h>
#include <wasabicfg.h>
#include <bfc/assert.h>
#include "wac.h"

#include <api/service/servicei.h>
#include <api/memmgr/api_memmgr.h>

#ifndef STATIC_COMPONENT
extern WAComponentClient *the;

#ifndef WASABINOMAINAPI
ComponentAPI *api;	// we do NOT delete this on shutdown!!!
#endif

OSMODULEHANDLE g_modulehandle;
#endif	//STATIC_COMPONENT

WAComponentClient::WAComponentClient(const wchar_t *name) :
  CfgItemI(name) {
  OSModuleHandle = NULL;
}

const wchar_t *WAComponentClient::getName() {
  return cfgitem_getName();
}

#ifdef WASABINOMAINAPI
void WAComponentClient::registerServices(api_service *_serviceapi)
{
  serviceApi = _serviceapi;

  waServiceFactory *sf = serviceApi->service_getServiceByGuid(applicationApiServiceGuid);
  if (sf) applicationApi = reinterpret_cast<api_application*>(sf->getInterface());

#ifdef WASABI_COMPILE_SYSCB
  sf = serviceApi->service_getServiceByGuid(syscbApiServiceGuid);
  if (sf) sysCallbackApi = reinterpret_cast<api_syscb *>(sf->getInterface());
#endif

  sf = serviceApi->service_getServiceByGuid(memMgrApiServiceGuid);
  if (sf) memmgrApi = reinterpret_cast<api_memmgr *>(sf->getInterface());

#ifdef WASABI_COMPILE_CONFIG
  sf = serviceApi->service_getServiceByGuid(configApiServiceGuid);
  if (sf) configApi = reinterpret_cast<api_config *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_IMGLDR
  sf = serviceApi->service_getServiceByGuid(imgLdrApiServiceGuid);
  if (sf) imgLoaderApi = reinterpret_cast<imgldr_api  *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_FONTS
  sf = serviceApi->service_getServiceByGuid(fontApiServiceGuid);
  if (sf) fontApi = reinterpret_cast<api_font *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_FILEREADER
  sf = serviceApi->service_getServiceByGuid(fileReaderApiServiceGuid);
  if (sf) fileApi = reinterpret_cast<api_fileReader *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_LOCALES
  sf = serviceApi->service_getServiceByGuid(localesApiServiceGuid);
  if (sf) localesApi = reinterpret_cast<api_locales *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_TIMERS
  sf = serviceApi->service_getServiceByGuid(timerApiServiceGuid);
  if (sf) timerApi = reinterpret_cast<timer_api *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_WND
  sf = serviceApi->service_getServiceByGuid(wndApiServiceGuid);
  if (sf) wndApi = reinterpret_cast<wnd_api *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_WNDMGR
  sf = serviceApi->service_getServiceByGuid(wndMgrApiServiceGuid);
  if (sf) wndManagerApi = reinterpret_cast<wndmgr_api *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_COMPONENTS
#endif

#ifdef WASABI_COMPILE_METADB
#endif

#ifdef WASABI_COMPILE_SCRIPT
  sf = serviceApi->service_getServiceByGuid(makiApiServiceGuid);
  if (sf) makiApi = reinterpret_cast<api_maki *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_SKIN
  sf = serviceApi->service_getServiceByGuid(skinApiServiceGuid);
  if (sf) skinApi = reinterpret_cast<api_skin *>(sf->getInterface());
#endif

#ifdef WASABI_COMPILE_MAKIDEBUG
  sf = serviceApi->service_getServiceByGuid(makiDebugApiServiceGuid);
  if (sf) debugApi = reinterpret_cast<api_makiDebugger *>(sf->getInterface());
#endif

#else
void WAComponentClient::registerServices(ComponentAPI *_api) {
  // save API *
  api = _api;
#endif

  // propagate name from component to cfgitemi
  if (cfgitem_getName() == NULL) CfgItemI::setName(WaComponent::getName());

  // register resources
  foreach(resources)
    resources.getfor()->onRegisterServices();
  endfor

  // our CfgItem GUID defaults to our component GUID
  cfgitem_setGUID(getGUID());

  // by default, peg us under "Installed Components" in config
  // {99CFD75C-1CA7-49e5-B8C0-7D78AA443C10}
  const GUID installed_guid = 
  { 0x99cfd75c, 0x1ca7, 0x49e5, { 0xb8, 0xc0, 0x7d, 0x78, 0xaa, 0x44, 0x3c, 0x10 } };
  if (cfgitem_getParentGuid() == INVALID_GUID)
    setParentGuid(installed_guid);

  WASABI_API_CONFIG->config_registerCfgItem(this);

  // allow overridden component class to do stuff
  onRegisterServices();
}

int WAComponentClient::RegisterServicesSafeModeOk()
{
	return 1;
}

void WAComponentClient::deregisterServices() {
  foreach(resources)
    resources.getfor()->onDeregisterServices();
  endfor
  WASABI_API_CONFIG->config_deregisterCfgItem(this);
}

CfgItem *WAComponentClient::getCfgInterface(int n) {
  if (n == 0) return this;
  return NULL;
}

class RegService : public LoadableResource {
public:
  RegService(waServiceFactoryI *sf) : factory(sf) {}

  virtual void onRegisterServices() {
    WASABI_API_SVC->service_register(factory);
  }
  virtual void onDeregisterServices() {
    WASABI_API_SVC->service_deregister(factory);
  }

private:
  waServiceFactoryI *factory;
};

void WAComponentClient::registerService(waServiceFactoryI* service) {
  registerResource(new RegService(service));
}

class SkinPart : public LoadableResource {
public:
  SkinPart(const wchar_t *name, int rel) : filename(name), id(-1), relative(rel) {}

  void beforeLoadingSkinElements() {
    ASSERT(!filename.isempty());
    StringW fn(filename);
    switch (relative) {
      case RSF_RELATIVETOWAC:
        fn.prepend(the->getComponentPath());// api->path_getComponentPath());
      break;
      case RSF_RELATIVETOTHEME: {
        wchar_t theme[WA_MAX_PATH]=L"";
        WASABI_API_CONFIG->getStringPrivate(L"theme", theme, WA_MAX_PATH, L"default");
				StringW path = WASABI_API_APP->path_getAppPath();
				path.AppendPath(L"themes");
				path.AppendPath(theme);
				fn = StringPathCombine(path, fn);
      }
      break;
    }
    id = WASABI_API_SKIN->loadSkinFile(fn);
  }
  virtual void onDeregisterServices() {
    if (id >= 0 && WASABI_API_SKIN) WASABI_API_SKIN->unloadSkinPart(id);
    id = -1;
  }

private:
  StringW filename;
  int id;
  int relative;
};

void WAComponentClient::registerSkinFile(const wchar_t *filename, int relative) 
{
  registerResource(new SkinPart(filename, relative));
}

void WAComponentClient::registerResource(LoadableResource *res) {
  ASSERT(!resources.haveItem(res));
  resources.addItem(res);
  if (postregisterservices) res->onRegisterServices(); 
}

void WAComponentClient::internal_onDestroy() {
  onDestroy();
  foreach(resources)
    if (resources.getfor()->deleteOnShutdown()) delete resources.getfor();
  endfor
  resources.removeAll();
}

int WAComponentClient::internal_onNotify(int cmd, int param1, int param2, int param3, int param4) {
  switch (cmd) {
    case WAC_NOTIFY_BEFORELOADINGSKINELEMENTS:
      foreach(resources)
        resources.getfor()->beforeLoadingSkinElements();
      endfor
      break;
    case WAC_NOTIFY_SKINLOADED:
      onSkinLoaded();
      break;
  }
  return onNotify(cmd, param1, param2, param3, param4);
}

#ifndef STATIC_COMPONENT
extern "C" {

__declspec(dllexport) UINT WAC_getVersion(void) {
  return WA_COMPONENT_VERSION;
}

__declspec(dllexport) int WAC_init(OSMODULEHANDLE moduleHandle) {
  the->setOSModuleHandle(moduleHandle);
  if (g_modulehandle == NULL) g_modulehandle = moduleHandle;
  return TRUE;
}

__declspec(dllexport) WaComponent *WAC_enumComponent(int n) {
  return the;
}

} // end extern "C"
#endif

#define CBCLASS WaComponentI
START_DISPATCH;
  CB(GETNAME, getName);
  CB(GETGUID, getGUID);
#ifndef WASABINOMAINAPI
  VCB(REGISTERSERVICES, registerServices);
#else
  VCB(REGISTERSERVICES2, registerServices);
#endif
  CB(15, RegisterServicesSafeModeOk)
  VCB(DEREGISTERSERVICES, deregisterServices);
  VCB(ONCREATE, onCreate);
  VCB(ONDESTROY, internal_onDestroy);
  CB(CREATEWINDOW, createWindow);
  CB(ONNOTIFY, internal_onNotify);
  CB(ONCOMMAND, onCommand);
  CB(GETCFGINTERFACE, getCfgInterface);
  VCB(SETOSMODULEHANDLE, setOSModuleHandle);
  CB(GETOSMODULEHANDLE, getOSModuleHandle);
  VCB(SETPATH, setComponentPath);
  CB(GETPATH, getComponentPath);
END_DISPATCH;
#undef CBCLASS

#ifdef WASABINOMAINAPI

api_application *applicationApi = NULL;
api_service *serviceApi = NULL;
api_syscb *sysCallbackApi = NULL;


api_memmgr *memmgrApi = NULL;

#ifdef WASABI_COMPILE_CONFIG
api_config *configApi = NULL;
#endif

#ifdef WASABI_COMPILE_IMGLDR
imgldr_api *imgLoaderApi = NULL;
#endif

#ifdef WASABI_COMPILE_FONTS
api_font *fontApi = NULL;
#endif


#ifdef WASABI_COMPILE_FILEREADER
api_fileReader *fileApi = NULL;
#endif

#ifdef WASABI_COMPILE_LOCALES
api_locales *localesApi = NULL;
#endif

#ifdef WASABI_COMPILE_TIMERS
timer_api *timerApi = NULL;
#endif

#ifdef WASABI_COMPILE_WND
wnd_api *wndApi = NULL;
#endif

#ifdef WASABI_COMPILE_WNDMGR
wndmgr_api *wndManagerApi = NULL;
#endif

#ifdef WASABI_COMPILE_COMPONENTS
#endif

#ifdef WASABI_COMPILE_METADB
#endif

#ifdef WASABI_COMPILE_SCRIPT
api_maki *makiApi = NULL;
#endif

#ifdef WASABI_COMPILE_SKIN
api_skin *skinApi;
#endif

#ifdef WASABI_COMPILE_MAKIDEBUG
api_makiDebugger *debugApi = NULL;
#endif

#ifdef WASABI_COMPILE_MEDIACORE
api_core *coreApi = NULL;
#endif

#endif // ifdef WASABINOMAINAPI