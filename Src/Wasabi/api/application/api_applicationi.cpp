#include <precomp.h> 
//<?#include "<class data="implementationheader"/>"
#include "api_applicationi.h" 
//?>

#include <wasabicfg.h>
#include <api/application/pathmgr.h>
#include <api/application/version.h>
#include <api/apiinit.h>

#ifdef WA3COMPATIBILITY
#include <runtime/main.h> // CUT!
#include <wndmgr/layout.h> // CUT!
#include <skin/skinparse.h>
#endif

StringW g_resourcepath;

// {3CBD4483-DC44-11d3-B608-000086340885}
static const GUID _baseGUID =
    { 0x3cbd4483, 0xdc44, 0x11d3, { 0xb6, 0x8, 0x0, 0x0, 0x86, 0x34, 0x8, 0x85 } };

api_application *applicationApi = NULL;
api_applicationI::api_applicationI(HINSTANCE instance, const wchar_t *_userPath) 
: appInstance(instance), mainthread(0), shuttingdown(0)
{
	userPath = _userPath;
#ifndef _WASABIRUNTIME

#ifdef WASABI_DIRS_FROMEXEPATH
	HINSTANCE hInst = GetModuleHandle(0);
#else
	HINSTANCE hInst = appInstance;
#endif

	StringW path;
	{
		wchar_t g[WA_MAX_PATH];
		GetModuleFileNameW(hInst, g, WA_MAX_PATH - 1);
		wchar_t *p = wcsrchr(g, '\\');
		if (p) *p = 0;
		path = g;
	}
#ifdef WIN32
	g_resourcepath = path;
#elif defined(LINUX)
	g_resourcepath = "/usr/local/share/";
	g_resourcepath += WasabiVersion::getAppName();
#endif

#ifdef LINUX
	//char *file = getenv( "WASABI_LOG_FILE" );
	//if (!ACCESS(file, 0)) UNLINK(file);
#endif

	apppath = path;

	g_resourcepath.AddBackslash();

#if defined(WASABI_COMPILE_SKIN) || defined(WASABI_COMPILE_IMGLDR)
	g_resourcepath.AppendPath(WASABI_RESOURCES_SUBDIRECTORY);
#endif

#endif

#ifdef WIN32
	HANDLE h = NULL;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &h, 0, FALSE, DUPLICATE_SAME_ACCESS);
	mainthread = h;  // 32-bit writes assumed atomic
#endif
#ifdef LINUX
	DebugString("portme: api main thread handle dup\n");
#endif
}

api_applicationI::~api_applicationI()
{
	g_resourcepath.purge();
}

const wchar_t *api_applicationI::main_getAppName()
{
	return WasabiVersion::getAppName();
}

const wchar_t *api_applicationI::main_getVersionString()
{
	return WasabiVersion::getVersionString();
}

unsigned int api_applicationI::main_getBuildNumber()
{
	return WasabiVersion::getBuildNumber();
}

GUID api_applicationI::main_getGUID()
{
	return guid;
}

HANDLE api_applicationI::main_getMainThreadHandle()
{
	if (mainthread == 0) return (HANDLE)0;
	HANDLE h = (HANDLE)0;
#ifdef WIN32
	DuplicateHandle(GetCurrentProcess(), mainthread, GetCurrentProcess(), &h, 0, FALSE, DUPLICATE_SAME_ACCESS);
#endif
#ifdef LINUX
	DebugString("portme: dup handle\n");
#endif
	return h;
}

HINSTANCE api_applicationI::main_gethInstance()
{
	return appInstance;
}

const wchar_t *api_applicationI::main_getCommandLine()
{
	return cmdLine;
}

void api_applicationI::main_shutdown(int deferred)
{
	int x = 1;
#ifdef WASABI_CHECK_CAN_EXIT
	WASABI_CHECK_CAN_EXIT(x)
#endif
	if (!x) return ;
	shuttingdown = 1;
#ifdef _WASABIRUNTIME
	Main::doAction(ACTION_CLOSE, deferred);
#endif
#ifdef WASABI_CUSTOM_QUIT
	WASABI_CUSTOM_QUITFN
#endif

}

void api_applicationI::main_cancelShutdown()
{
#ifdef _WASABIRUNTIME
	Main::cancelShutdown();
#endif
	shuttingdown = 0;
}

int api_applicationI::main_isShuttingDown()
{
	return shuttingdown;
}

const wchar_t *api_applicationI::path_getAppPath()
{
#ifdef _WASABIRUNTIME
	return Main::getMainAppPath();
#endif
	return apppath;
}

const wchar_t *api_applicationI::path_getUserSettingsPath()
{
	return userPath;
	//return PathMgr::getUserSettingsPath();
}

void api_applicationI::setHInstance(HINSTANCE instance)
{
	appInstance = instance;
};

void api_applicationI::setCommandLine(const wchar_t *cmdline)
{
	cmdLine = cmdline;
}

void api_applicationI::setGUID(GUID g)
{
	guid = g;
}

int api_applicationI::app_getInitCount()
{
	return ApiInit::getInitCount();
}


int api_applicationI::app_messageLoopStep()
{
#if defined(WASABI_COMPILE_TIMERS) || defined(WASABI_COMPILE_WND)
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage( &msg, NULL, 0, 0 )) return 0;
#ifdef WASABI_COMPILE_WND
		TranslateMessage( &msg );
#endif
		DispatchMessage( &msg );
	}
#endif
	return 1;
}

