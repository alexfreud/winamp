#ifndef __API_INIT_H
#define __API_INIT_H

#include <bfc/platform/platform.h>
#include <api/skin/widgets.h>
#include <bfc/string/bfcstring.h>
#include <api/syscb/api_syscbi.h>
#include <api/service/service.h>

class api_configI;
class SkinApi;
class api_makiI;
class MemMgrApi;
class TimerApi;
class WndApi;
class ForeignWnd;
class ClickWnd;

class ImgLdrApi;
#ifdef WASABI_COMPILE_FONTS
class FontApi;
#endif
#ifdef WASABI_COMPILE_WNDMGR
class WndMgrApi;
#endif
#ifdef WASABI_COMPILE_MAKIDEBUG
class MakiDebuggerApi;
#endif
#ifdef WASABI_COMPILE_FILEREADER
class FileReaderApi;
#endif
#ifdef WASABI_COMPILE_LOCALES
class api_localesI;
#endif
#ifndef WA3COMPATIBILITY
#ifdef WASABI_COMPILE_MEDIACORE
class Core;
#endif
#endif



#include <api/syscb/api_syscb.h>




#ifdef WASABI_COMPILE_MEDIACORE

#include <api/core/api_core.h>

class CoreApiService : public waServiceBase<api_core, CoreApiService> {
public:
  CoreApiService() : waServiceBase<api_core, CoreApiService>(coreApiServiceGuid) {}
  static const char *getServiceName() { return "Core API"; }
  virtual api_core *getService() { return coreApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#endif // WASABI_COMPILE_MEDIACORE

#ifdef WASABI_COMPILE_FILEREADER

#include <api/filereader/api_filereader.h>

class FileReaderApiService : public waServiceBase<api_fileReader, FileReaderApiService> {
public:
  FileReaderApiService() : waServiceBase<api_fileReader, FileReaderApiService>(fileReaderApiServiceGuid) {}
  static const char *getServiceName() { return "FileReader API"; }
  virtual api_fileReader *getService() { return fileApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#endif // WASABI_COMPILE_FILEREADER

#ifdef WASABI_COMPILE_CONFIG

#include <api/config/api_config.h>

class ConfigApiService : public waServiceBase<api_config, ConfigApiService> {
public:
  ConfigApiService() : waServiceBase<api_config, ConfigApiService>(configApiServiceGuid) {}
  static const char *getServiceName() { return "Config API"; }
  virtual api_config *getService() { return configApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#endif // WASABI_COMPILE_CONFIG

#ifdef WASABI_COMPILE_IMGLDR

#include <api/imgldr/api_imgldr.h>

class ImgLdrApiService: public waServiceBase<imgldr_api, ImgLdrApiService> {
public:
  ImgLdrApiService() : waServiceBase<imgldr_api, ImgLdrApiService>(imgLdrApiServiceGuid) {}
  static const char *getServiceName() { return "Image Loading API"; }
  virtual imgldr_api *getService() { return imgLoaderApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#endif // WASABI_COMPILE_IMGLDR

#ifdef WASABI_COMPILE_FONTS

#include <api/font/api_font.h>

class FontApiService : public waServiceBase<api_font, FontApiService> {
public:
  FontApiService() : waServiceBase<api_font, FontApiService>(fontApiServiceGuid) {}
  static const char *getServiceName() { return "Font Rendering API"; }
  virtual api_font *getService() { return fontApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#endif // WASABI_COMPILE_FONT


#include <api/locales/api_locales.h>

class LocalesApiService : public waServiceBase<api_locales, LocalesApiService> {
public:
  LocalesApiService() : waServiceBase<api_locales, LocalesApiService>(localesApiServiceGuid) {}
  static const char *getServiceName() { return "Locales API"; }
  virtual api_locales *getService() { return localesApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};



#include <api/wnd/api_wnd.h>

class WndApiService : public waServiceBase<wnd_api, WndApiService> {
public:
  WndApiService() : waServiceBase<wnd_api, WndApiService>(wndApiServiceGuid) {}
  static const char *getServiceName() { return "Windowing API"; }
  virtual wnd_api *getService() { return wndApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};


#include <api/wndmgr/api_wndmgr.h>

class WndMgrApiService : public waServiceBase<wndmgr_api, WndMgrApiService> {
public:
  WndMgrApiService() : waServiceBase<wndmgr_api, WndMgrApiService>(wndMgrApiServiceGuid) {}
  static const char *getServiceName() { return "Window Manager API"; }
  virtual wndmgr_api *getService() { return wndManagerApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#include <api/skin/api_skin.h>

class SkinApiService : public waServiceBase<api_skin, SkinApiService> {
public:
  SkinApiService() : waServiceBase<api_skin, SkinApiService>(skinApiServiceGuid) {}
  static const char *getServiceName() { return "Skinning API"; }
  virtual api_skin *getService() { return skinApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#include <api/script/api_maki.h>

class MakiApiService : public waServiceBase<api_maki, MakiApiService> {
public:
  MakiApiService() : waServiceBase<api_maki, MakiApiService>(makiApiServiceGuid) {}
  static const char *getServiceName() { return "MAKI Scripting API"; }
  virtual api_maki *getService() { return makiApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#ifdef WASABI_COMPILE_MAKIDEBUG

#include <api/script/debugger/api_makidebug.h>

class MakiDebugApiService : public waServiceBase<api_makiDebugger, MakiDebugApiService> {
public:
  MakiDebugApiService() : waServiceBase<api_makiDebugger, MakiDebugApiService>(makiDebugApiServiceGuid) {}
  static const char *getServiceName() { return "MAKI Debugger API"; }
  virtual api_makiDebugger *getService() { return debugApi; }
  static FOURCC getServiceType() { return WaSvc::UNIQUE; }
};

#endif // WASABI_COMPILE_SKIN

class ApiInit 
{
public:
  static int init(OSMODULEHANDLE hinstance, OSWINDOWHANDLE mainwnd, api_service *wa5_svc); // 0 == error, 1 == success
  static int shutdown(); // 0 == error, 1 == success
  static int getInitCount(); // if you do init / shutdown more than once in the session, this may be useful to invalid some caches

private:
  #ifdef WASABI_COMPILE_SKIN
  #ifdef WASABI_COMPILE_COMPONENTS
  static waServiceFactoryI * groupcreate;
  #endif //WASABI_COMPILE_COMPONENTS
  #endif //WASABI_COMPILE_SKIN

  static waServiceFactoryI * win32Font;
  static FontApiService *fontapisvc;

public:
  static Widgets * widgets;
private:
  static SkinApiService *skinapisvc;

  static api_configI * cfg;
  static ConfigApiService *cfgapisvc;
  static ImgLdrApi * img;
  static ImgLdrApiService *imgldrapisvc;

  #ifdef WASABI_COMPILE_FONTS
  static FontApi * font;
  #endif

  #ifdef WASABI_COMPILE_FILEREADER
  static FileReaderApi * fr;
  #ifdef WASABI_COMPILE_ZIPREADER
  static waServiceFactoryI * zipRead;
  #endif
  static FileReaderApiService *frapisvc;
  #endif

  static api_localesI *loc;
  static LocalesApiService *localesapisvc;

  static WndApi * wnd;
  static ForeignWnd * mainWnd;
  static ClickWnd * default_mainWnd;
  static WndApiService *wndapisvc;

  static WndMgrApi * wndmgr;
  static WndMgrApiService *wndmgrapisvc;


  static api_makiI * maki;
  static MakiApiService *makiapisvc;

  static SkinApi * skin;

  #ifdef WASABI_COMPILE_MAKIDEBUG
  static MakiDebuggerApi * debug;
  static MakiDebugApiService *makidebugapisvc;
  #endif

  #ifdef WASABI_COMPILE_MEDIACORE
  static CoreApiService *coreapisvc;
  #endif

  static int initcount;
};

#endif

