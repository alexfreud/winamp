#include "./wndmgr/layout.h"

#include <tataki/export.h>
//<?#include "<class data="implementationheader"/>"
#include "apiinit.h" 
//?>

#include <api/memmgr/api_memmgr.h>

#define WASABI_SAYMYNAME
#include <wasabicfg.h>
#undef WASABI_SAYMYNAME

#include <api/service/servicei.h>
#include <api/locales/api_localesi.h>
#include <api/config/api_configi.h>
#include <api/config/items/cfgitem.h>
#ifdef WASABI_COMPILE_FONTS
#include <api/font/FontCreator.h>
#include <api/font/fontapi.h>
#endif
#include <api/skin/skinapi.h>
#include <api/skin/groupwndcreate.h>
#include <api/script/makiapi.h>
#include <api/script/objecttable.h>
#include <api/wnd/wndapi.h>
#include <api/wnd/wndclass/foreignwnd.h>
#include <api/wnd/wndclass/clickwnd.h>
#include <api/imgldr/imgldrapi.h>

#ifdef WASABI_COMPILE_FONTS
#ifdef WIN32
# include <api/font/win32/truetypefont_win32.h>
#elif defined(LINUX)
# include <api/font/linux/truetypefont_linux.h>
#else
# error port me
#endif // platform
#endif // fonts

#include <api/wndmgr/wndmgrapi.h>

#ifdef WASABI_COMPILE_MAKIDEBUG
#include <api/script/debugger/debugapi.h>
#endif
#ifdef WASABI_COMPILE_MAKIDEBUG
#include <api/core/api_core.h>
#endif
#ifdef WASABI_COMPILE_FILEREADER
#include <api/filereader/filereaderapi.h>
#ifdef WASABI_COMPILE_ZIPREADER
#include <api/filereader/zip/zipread.h>
#endif
#endif
#ifdef WASABI_COMPILE_LINUX
#include <api/linux/api_linuxi.h>
#endif
#ifdef WASABI_COMPILE_TEXTMODE
#if defined(WIN32)
#include <api/textmode/textmode_win32.h>
#elif defined(LINUX)
#include <api/textmode/textmode_ncurses.h>
#else // dummy
#include <api/textmode/api_textmodeI.h>
#endif
#endif


#if defined(WASABI_COMPILE_COMPONENTS) | defined(GEN_FF) // MULTIAPI-FIXME
#include <api/wac/compon.h>
#endif

#include <api/wndmgr/gc.h>

#include <api/application/api_application.h>
#ifndef WASABI_CUSTOM_MODULE_SVCMGR
DECLARE_MODULE_SVCMGR;
#endif

#ifdef _WIN32
extern HINSTANCE hInstance;
#endif

extern StringW g_resourcepath;
api_application *applicationApi = NULL;
timer_api *timerApi = NULL;

int ApiInit::init(OSMODULEHANDLE hinstance,  OSWINDOWHANDLE mainwnd, api_service *wa5_svc)
{
	Tataki::Init(wa5_svc);
	serviceApi = wa5_svc;
	initcount++;

#ifndef WA3COMPATIBILITY

	// ------------------------------------------------------------------------
	// Init callback system
	// ------------------------------------------------------------------------
	
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(syscbApiServiceGuid);
	if (sf) sysCallbackApi = reinterpret_cast<api_syscb *>(sf->getInterface());

	// ------------------------------------------------------------------------
	// Init application API
	// ------------------------------------------------------------------------
		
	sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) applicationApi = reinterpret_cast<api_application *>(sf->getInterface());

g_resourcepath = StringPathCombine(WASABI_API_APP->path_getAppPath(), WASABI_RESOURCES_SUBDIRECTORY);

	// ------------------------------------------------------------------------
	// Init centralized memory manager api
	// ------------------------------------------------------------------------
	
	sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) WASABI_API_MEMMGR = reinterpret_cast<api_memmgr *>(sf->getInterface());

#ifdef WASABI_COMPILE_FILEREADER
	// ------------------------------------------------------------------------
	// Init file reader api and optional file readers
	// ------------------------------------------------------------------------
	fr = new FileReaderApi();
	fileApi = fr;
	WASABI_API_SVC->service_register(frapisvc = new FileReaderApiService());
	//WASABI_API_SVC->service_register(&diskReader);
	// set up custom file readers
#ifdef WASABI_COMPILE_ZIPREADER
	zipRead = new waServiceTSingle<svc_fileReader, ZipRead>;
#endif
#ifdef WASABI_COMPILE_ZIPREADER
	WASABI_API_SVC->service_register(zipRead);
#endif
#endif // WASABI_COMPILE_FILEREADER


	// ------------------------------------------------------------------------
	// Init config api and register optional config items
	// ------------------------------------------------------------------------
#ifdef WASABI_COMPILE_CONFIG
	cfg = new api_configI();
	configApi = cfg;
	WASABI_API_SVC->service_register(cfgapisvc = new ConfigApiService());
#endif

	// if no static service was ever declared, we need to init it so we don't hit null
	INIT_MODULE_SVCMGR();

	DebugStringW(L"Auto-initializing %d services (system pass)\n", staticServiceMgr->__m_modules2.getNumItems());
	MODULE_SVCMGR_ONINIT2();

#ifdef WASABI_COMPILE_IMGLDR
	// ------------------------------------------------------------------------
	// Init image loading api and optional image formats
	// ------------------------------------------------------------------------
	img = new ImgLdrApi();
	imgLoaderApi = img;
	WASABI_API_SVC->service_register(imgldrapisvc = new ImgLdrApiService());

#endif

#ifdef WASABI_COMPILE_FONTS
	// ------------------------------------------------------------------------
	// Init font rendering api
	// ------------------------------------------------------------------------
	font = new FontApi();
	fontApi = font;
	win32Font = new FontCreator<TrueTypeFont_Win32>;
	WASABI_API_SVC->service_register(win32Font);
	WASABI_API_SVC->service_register(fontapisvc = new FontApiService());
#endif


#ifdef WASABI_COMPILE_LOCALES
	// ------------------------------------------------------------------------
	// Init localization
	// ------------------------------------------------------------------------
	loc = new api_localesI();
	localesApi = loc;
	WASABI_API_SVC->service_register(localesapisvc = new LocalesApiService());
#endif

	// ------------------------------------------------------------------------
	// Init timer api
	// ------------------------------------------------------------------------
	sf = WASABI_API_SVC->service_getServiceByGuid(timerApiServiceGuid);
	if (sf) timerApi = reinterpret_cast<timer_api *>(sf->getInterface());


#ifdef WASABI_COMPILE_WND
	// ------------------------------------------------------------------------
	// Init window api
	// ------------------------------------------------------------------------
	wnd = new WndApi();
	wndApi = wnd;
#ifdef _WIN32
	if (mainwnd != NULL)
	{
		mainWnd = new ForeignWnd(mainwnd, hInstance);
		DebugStringW(L"main wnd handle is %x\n", mainWnd->gethWnd());
		wnd->main_setRootWnd(mainWnd);
	}
	else
	{
		default_mainWnd = new ClickWnd();
		default_mainWnd->init(hInstance, NULL, TRUE);
		wnd->main_setRootWnd(default_mainWnd);
	}
  #endif
	WASABI_API_SVC->service_register(wndapisvc = new WndApiService());
#endif

	// ------------------------------------------------------------------------
	// Init media core api
	// ------------------------------------------------------------------------
#ifdef WASABI_COMPILE_MEDIACORE
 #ifndef WA3COMPATIBILITY
 #ifdef WASABI_CUSTOMIMPL_MEDIACORE
	coreApi = createCustomCoreApi();
#else
 #error "not done yet"
 #endif
 #endif
	WASABI_API_SVC->service_register(coreapisvc = new CoreApiService());
#endif

#ifdef WASABI_COMPILE_WNDMGR
	// ------------------------------------------------------------------------
	// Init window manager api
	// ------------------------------------------------------------------------
	wndmgr = new WndMgrApi ();
	wndManagerApi = wndmgr;
	WASABI_API_SVC->service_register(wndmgrapisvc = new WndMgrApiService());
#endif



#ifdef WASABI_COMPILE_SKIN
	// ------------------------------------------------------------------------
	// Init skin
	// ------------------------------------------------------------------------
	skin = new SkinApi();
	skinApi = skin;

#ifdef WASABI_COMPILE_COMPONENTS
	groupcreate = new waServiceTSingle<svc_windowCreate, GroupWndCreateSvc>;
	WASABI_API_SVC->service_register(groupcreate);
#endif

	WASABI_API_SVC->service_register(skinapisvc = new SkinApiService());

#endif

#ifdef WASABI_COMPILE_MAKIDEBUG
	// ------------------------------------------------------------------------
	// Init script debugger api
	// ------------------------------------------------------------------------
	debug = new MakiDebuggerApi();
	debugApi = debug;
	WASABI_API_SVC->service_register(makidebugapisvc = new MakiDebugApiService());
#endif

#ifdef WASABI_COMPILE_SKIN
	widgets = new Widgets();
#endif

#endif


	DebugStringW(L"Auto-initializing %d services (user pass)\n", staticServiceMgr->__m_modules.getNumItems());
	MODULE_SVCMGR_ONINIT();

#ifndef WA3COMPATIBILITY

#ifdef WASABI_COMPILE_SCRIPT
	// ------------------------------------------------------------------------
	// Init scripting api
	// ------------------------------------------------------------------------
	maki = new api_makiI();
	makiApi = maki;
	WASABI_API_SVC->service_register(makiapisvc = new MakiApiService());
	maki->init();

	

#ifdef WASABI_COMPILE_WNDMGR
	garbageCollector = new GarbageCollector();
#endif

	ObjectTable::start();
#endif

#endif //wa3compat

	return 1; // success
}

int ApiInit::shutdown()
{
#ifndef WA3COMPATIBILITY

	garbageCollector->gccb_onGarbageCollect();
	delete garbageCollector;

#ifdef WASABI_COMPILE_SKIN
	skin->preShutdown();
#endif

	MODULE_SVCMGR_ONSHUTDOWN();

#ifdef _WIN32
	ComponentManager::unloadAll();
#else
#warning port me?
#endif

	WASABI_API_SVC->service_deregister(skinapisvc);
	delete skin;
	skinApi = NULL;
	
	delete skinapisvc;
	skinapisvc = 0;

#ifdef WASABI_COMPILE_SCRIPT
	ObjectTable::shutdown();
#endif

#ifdef WASABI_COMPILE_SKIN
	delete widgets;
	widgets = NULL;
#endif
	WASABI_API_SVC->service_deregister(makiapisvc);
	delete maki;
	makiApi = NULL;
	
	delete makiapisvc;
	makiapisvc = 0;

#ifdef WASABI_COMPILE_COMPONENTS
	WASABI_API_SVC->service_deregister(groupcreate);
	delete groupcreate;
	groupcreate = 0;
#endif

	WASABI_API_SVC->service_deregister(win32Font);
	delete win32Font;
	win32Font = 0;

#ifdef WASABI_COMPILE_MEDIACORE
 #ifndef WA3COMPATIBILITY
 #ifdef WASABI_CUSTOMIMPL_MEDIACORE
	destroyCustomCoreApi(coreApi);
#else
 #error "not done yet"
 #endif
 #endif

	WASABI_API_SVC->service_deregister(coreapisvc);
	delete coreapisvc;
	coreapisvc = 0;
#endif



	WASABI_API_SVC->service_deregister(wndmgrapisvc);
	delete wndmgr;
	wndManagerApi = NULL;

	
	delete wndmgrapisvc;
	wndmgrapisvc = 0;


	delete loc;
	localesApi = NULL;

	WASABI_API_SVC->service_deregister(localesapisvc);
	delete localesapisvc;
	localesapisvc = 0;


#ifdef WASABI_COMPILE_FILEREADER
#ifdef WASABI_COMPILE_ZIPREADER
	WASABI_API_SVC->service_deregister(zipRead);
	delete zipRead;
#endif


	delete fr;
	fileApi = NULL;

	WASABI_API_SVC->service_deregister(frapisvc);
	delete frapisvc;
	frapisvc = 0;
#endif

#endif

	delete mainWnd;
	mainWnd = NULL;
	delete default_mainWnd;
	default_mainWnd = NULL;
	WASABI_API_SVC->service_deregister(wndapisvc);
	delete wnd;
	wndApi = NULL;
	
	delete wndapisvc;
	wndapisvc = 0;

#ifdef WASABI_COMPILE_FONTS
	WASABI_API_SVC->service_deregister(fontapisvc);
	delete font;
	fontApi = NULL;

	delete fontapisvc;
	fontapisvc = 0;
#endif

#ifndef WA3COMPATIBILITY


	delete img;
	imgLoaderApi = NULL;

	WASABI_API_SVC->service_deregister(imgldrapisvc);
	delete imgldrapisvc;
	imgldrapisvc = 0;

#ifdef WASABI_COMPILE_CONFIG
	WASABI_API_SVC->service_deregister(cfgapisvc);
	delete cfg;
	configApi = NULL;
	
	delete cfgapisvc;
	cfgapisvc = 0;
#endif

	MODULE_SVCMGR_ONSHUTDOWN2();

	WASABI_API_MEMMGR = NULL;

applicationApi=NULL;
#endif //wa3compatibility

#ifdef WASABI_COMPILE_MAKIDEBUG
	delete debug;
	debugApi = NULL;

	WASABI_API_SVC->service_deregister(makidebugapisvc);
	delete makidebugapisvc;
	makidebugapisvc = 0;
#endif

	if (alphaMgr)
	delete alphaMgr;
	alphaMgr=0;

	// TODO: releaseInterface
	timerApi = NULL;

	Tataki::Quit();
	sysCallbackApi = NULL;

	return 1; // success
}

int ApiInit::getInitCount()
{
	return initcount;
}

#ifdef WASABI_COMPILE_SKIN
#ifdef WASABI_COMPILE_COMPONENTS
waServiceFactoryI * ApiInit::groupcreate = NULL;
#endif
#endif //WASABI_COMPILE_SKIN

#ifdef WASABI_COMPILE_FONTS
waServiceFactoryI * ApiInit::win32Font = NULL;
FontApiService *ApiInit::fontapisvc = NULL;
#endif //WASABI_COMPILE_FONTS

#ifdef WASABI_COMPILE_SKIN
Widgets * ApiInit::widgets = NULL;
SkinApiService *ApiInit::skinapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_CONFIG
api_configI * ApiInit::cfg = NULL;
ConfigApiService *ApiInit::cfgapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_IMGLDR
ImgLdrApi * ApiInit::img = NULL;
ImgLdrApiService *ApiInit::imgldrapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_FONTS
FontApi * ApiInit::font = NULL;
#endif

#ifdef WASABI_COMPILE_FILEREADER
FileReaderApi * ApiInit::fr = NULL;
#ifdef WASABI_COMPILE_ZIPREADER
waServiceFactoryI * ApiInit::zipRead = NULL;
#endif
FileReaderApiService *ApiInit::frapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_LOCALES
api_localesI * ApiInit::loc = NULL;
LocalesApiService *ApiInit::localesapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_WND
WndApi * ApiInit::wnd = NULL;
ForeignWnd * ApiInit::mainWnd = NULL;
ClickWnd * ApiInit::default_mainWnd = NULL;
WndApiService *ApiInit::wndapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_WNDMGR
WndMgrApi * ApiInit::wndmgr = NULL;
WndMgrApiService *ApiInit::wndmgrapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_SCRIPT
api_makiI * ApiInit::maki = NULL;
MakiApiService *ApiInit::makiapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_SKIN
SkinApi * ApiInit::skin = NULL;
#endif

#ifdef WASABI_COMPILE_MAKIDEBUG
MakiDebuggerApi * ApiInit::debug = NULL;
MakiDebugApiService *ApiInit::makidebugapisvc = NULL;
#endif

#ifdef WASABI_COMPILE_MEDIACORE
CoreApiService *ApiInit::coreapisvc = NULL;
#endif


int ApiInit::initcount = 0;

api_memmgr *memmgrApi = NULL;