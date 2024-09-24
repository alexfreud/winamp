#include "serviceMain.h"
#include <string>
#include <string.h>
#include <curl/curl.h>
#include "expat.h"
#include "stl/stringUtils.h"
#include "services/stdServiceImpl.h"
#include "../../versions.h"
#include "../../global.h"
#ifdef _WIN32
#include "win32/rezFuncs.h"
#include <crtdbg.h>
#include <shlwapi.h>
#else
#ifdef PLATFORM_LINUX
#include "../stacktrace/StackTrace.h"
#endif
#include <libgen.h>
#include "unixversion.h"
#include <pthread.h>
#include <sys/param.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE_CC__
#include <mach-o/dyld.h>
#include "file/fileUtils.h"
#endif
#endif

using namespace std;
using namespace uniString;
using namespace stringUtil;

#ifndef _WIN32
#define TRUE true
#define FALSE false
#endif

// are we running as a daemon or service
bool sDaemon = false;

event serviceMain::sStop(TRUE);
#ifndef _WIN32
event serviceMain::sWINCH(FALSE);
event serviceMain::sHUP(FALSE);
event serviceMain::sUSR1(FALSE);
event serviceMain::sUSR2(FALSE);
#endif

#ifdef _WIN32
static SERVICE_STATUS_HANDLE ssh=NULL; // does not have to be closed

BOOL WINAPI _console_handler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL+C signal.
		case CTRL_CLOSE_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_C_EVENT:
		{
			_SetEvent(serviceMain::sStop);
			return TRUE;
		}
		default:
		{
			return FALSE;
		}
	}
}

#else
#ifdef PLATFORM_LINUX
static void custom_signal_handler(int signum)
{
	signal(signum, SIG_DFL);
	// http://sourceforge.net/p/stacktrace/code/HEAD/tree/
	// ensure we building with -rdynamic so this works ok

	static char buf[256] = {0};
	static time_t last_ttt;

	struct tm ttm;
	time_t ttt = ::time(NULL);
	if (ttt != last_ttt)
	{
		last_ttt = ttt;
		::strftime(buf, 255, "%Y-%m-%d %H:%M:%S", ::localtime_r(&ttt, &ttm));
	}

	FILE *fp = freopen(("/tmp/sc_serv_segfault_" + tos(getpid()) + ".log").c_str(), "w", stderr);
	fprintf(stderr, "Shoutcast DNAS/" SERV_OSNAME " v%s (" __DATE__ ")\n"
			"An unrecoverable error (%d) has occurred @ %s.\n\n",
			gOptions.getVersionBuildStrings().hideAsString().c_str(), signum, buf);
	stacktrace::displayCurrentStackTrace(/*-1U, 0*/);
	fflush(fp);
	kill(getpid(), signum);
}
#endif
unsigned sigWatcher()
{
	while (::WaitForSingleObject(serviceMain::sStop,0) != WAIT_OBJECT_0)
	{
		sigset_t catchset;
		sigemptyset(&catchset);
		sigaddset(&catchset,SIGPIPE);
		sigaddset(&catchset,SIGTERM);
		sigaddset(&catchset,SIGHUP);
		sigaddset(&catchset,SIGINT);
		sigaddset(&catchset,SIGQUIT);
		sigaddset(&catchset,SIGTSTP); // ^Z allow this
		sigaddset(&catchset,SIGCHLD);
		sigaddset(&catchset,SIGWINCH);
		sigaddset(&catchset,SIGUSR1);
		sigaddset(&catchset,SIGUSR2);

		//struct timespec ts;
		//ts.tv_sec = 1;
		//ts.tv_nsec = 0;
		//apple is missing sigtimed wait.
		int err = 0;
		if (::sigwait(&catchset,&err))
		{
			err = SIGTERM;
		}

		switch (err)
		{
			case SIGTERM: case SIGINT: case SIGQUIT: case SIGTSTP:
			{
				_SetEvent(serviceMain::sStop);
				break;
			}
			case SIGHUP:
			{
				_SetEvent(serviceMain::sHUP);
				break;
			}
			case SIGWINCH:
			{
				_SetEvent(serviceMain::sWINCH);
				break;
			}
			case SIGUSR1:
			{
				_SetEvent(serviceMain::sUSR1);
				break;
			}
			case SIGUSR2:
			{
				_SetEvent(serviceMain::sUSR2);
				break;
			}
		}
	}
	return 0;
}

// We need access to this for Apple, because we don't have sigtimedwait(). This means there
// circumstances when we need access to this thread so we can signal it and force sigwait() to exit
Tthread<pointer_to_thread_function> gSigWatcherThread(sigWatcher);

#endif

#ifdef _WIN32
int run_install(const vector<uniString::utf8> &args) throw();
int run_uninstall(const vector<uniString::utf8> &args) throw();
int run_run(const vector<uniString::utf8> &args) throw(std::exception);
int run_daemon(const vector<uniString::utf8> &args) throw(std::exception);
#else
int run_run(const vector<uniString::utf8> &args, const vector<uniString::utf8> &arg) throw(std::exception);
int run_daemon(const vector<uniString::utf8> &args, const vector<uniString::utf8> &arg) throw(std::exception);

int blockSignals() throw()
{
	return thread_CORE::standard_signal_block();
}
#endif


uniString::utf8 getVersionBuildStrings() throw()
{
#ifdef _WIN32
	static utf8 version = "";
	if (version.empty())
	{
		getVersionInfo(version);
	}
#else
	static utf8 version = "";
	if (version.empty())
	{
		for (int x = 0; x < VENT; ++x)
		{
			if (x) version += ".";
			version += tos(PRODUCTVERSION[x]);
		}
	}
#endif
#ifdef LICENCE_FREE
    version += " no-licence-check";
#endif
	return version;
}

#ifdef _WIN32
int do__main(vector<uniString::utf8> &args) throw(std::exception)
#else
int do__main(vector<uniString::utf8> &args, vector<uniString::utf8> &arg) throw(std::exception)
#endif
{
	if (!args.empty())
	{
		utf8 s = toLower(args.front());

		if (s == "--version" || s == "-v")
		{
			XML_Expat_Version expat = XML_ExpatVersionInfo();
			printf("%s", utf8("Shoutcast DNAS/" SERV_OSNAME " v" + getVersionBuildStrings() +
							  " (" __DATE__ ") " + utf8(curl_version()) + " expat/" +
							  tos(expat.major) + "." + tos(expat.minor) + "." + tos(expat.micro) +
//#ifdef _WIN32
//							  " pthread-win32/" PTW32_VERSION_STR "-mod" +
//#endif
							  eol()).toANSI().c_str());
			return 0;
		}

		if (s == "--help" || s == "/?")
		{
			#ifdef _WIN32
			#define SC_SERV_END ".exe"
			#else
			#define SC_SERV_END ""
			#endif

			printf("%s", utf8("*********************************************************************" + eol()).toANSI().c_str());
			printf("%s", utf8("**        Shoutcast Distributed Network Audio Server (DNAS)        **" + eol()).toANSI().c_str());
			printf("%s", utf8("**    Copyright (C) 2014-2023 Radionomy SA, All Rights Reserved    **" + eol()).toANSI().c_str());
			printf("%s", utf8("*********************************************************************" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "Usage: sc_serv" SC_SERV_END " [OPTION] [PARAMETERS]... [conf]" + eol()).toANSI().c_str());

			printf("%s", utf8(eol() + "\t[conf] - File path to the configuration file (this can be" +
							  eol() + "\t\t relative or absolute) [optional]" + eol()).toANSI().c_str());

			printf("%s", utf8(eol() + "\t\t If not specified then sc_serv.conf / sc_serv.ini" +
							  eol() + "\t\t in the same folder will be automatically loaded." + eol()).toANSI().c_str());

			printf("%s", utf8(eol() + "Options:").toANSI().c_str());

			printf("%s", utf8(eol() + "\t-s, setup\t\tRun the DNAS in setup mode for" + eol() +
									  "\t\t\t\tcreating a basic configuration" + eol()).toANSI().c_str());
#ifdef CONFIG_BUILDER
			printf("%s", utf8(eol() + "\t-b, builder\t\tRun the DNAS in builder mode for" + eol() +
									  "\t\t\t\tcreating an advanced configuration" + eol()).toANSI().c_str());
#endif
			printf("%s", utf8(eol() + "\t-v, --version\t\tDisplay version information" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "\t/?, --help\t\tDisplay this information" + eol()).toANSI().c_str());

			#ifdef _WIN32
			printf("%s", utf8(eol() + eol() + "Service Options:" + eol()).toANSI().c_str());
			
			printf("%s", utf8(eol() + "\tinstall [servicename] [username] [password] [conf]" + eol()).toANSI().c_str());

			printf("%s", utf8(eol() + "\t\tservicename - Unique name for the service install" + eol() +
									  "\t\t\t      if the default is not appropriate or" + eol() +
									  "\t\t\t      needing multiple services [optional]" + eol() +
									  "\t\t\t      Default is: \"Shoutcast DNAS Service\"" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "\t\tusername - User under which to run the service as" + eol() +
									  "\t\t\t   or '0' for the local system [optional]" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "\t\tpassword - Password for user or '0' for the local" + eol() +
									  "\t\t\t   system or with no password [optional]" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "\t\tconf - File path to the configuration file either" + eol() +
									  "\t\t       as a full or relative path [optional]" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "\texample:" + eol() + "\t\tsc_serv.exe install" + eol() +
									  "\t\tor" + eol() + "\t\tsc_serv.exe install sc_serv" + eol() +
									  "\t\tor" + eol() + "\t\tsc_serv.exe install sc_serv 0 0 sc_serv.conf" +
									  eol()).toANSI().c_str());

			printf("%s", utf8(eol() + eol() + "\tuninstall [servicename]" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "\t\tservicename - Name used to install the service or" + eol() +
									  "\t\t\t      leave blank for default [optional]" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "\texample:" + eol() + "\t\tsc_serv.exe uninstall" + eol() +
									  "\t\tor" + eol() + "\t\tsc_serv.exe uninstall sc_serv" + eol() + eol()).toANSI().c_str());
			#else
			#define SC_SERV_FILE "sc_serv"
			printf("%s", utf8(eol() + "Daemon Options:" + eol()).toANSI().c_str());
			printf("%s", utf8(eol() + "\tdaemon [conf]\t\tRun the DNAS in daemon mode " + eol()).toANSI().c_str());
			#endif
			XML_Expat_Version expat = XML_ExpatVersionInfo();
			printf("%s", utf8(eol() + eol() + "Built with: " + utf8(curl_version()) +
							  " expat/" + tos(expat.major) + "." + tos(expat.minor) + "." +
							  tos(expat.micro) + eol()).toANSI().c_str());
			return 0;
		}

		if (s == "install")
		{
			#ifdef _WIN32
			args.erase(args.begin());
			return run_install(args);
			#else
			cerr << "install is not supported on this platform" << endl;
			return -1;
			#endif
		}

		if (s == "uninstall")
		{
						#ifdef _WIN32
			args.erase(args.begin());
			return run_uninstall(args);
			#else
			cerr << "uninstall is not supported on this platform" << endl;
			return -1;
			#endif
		}

		if (s == "daemon")
		{
			args.erase(args.begin());
			#ifdef _WIN32
			return run_daemon(args);
			#else
			return run_daemon(args, arg);
			#endif
		}
	}

#ifdef _WIN32
	return run_run(args);
#else
	return run_run(args, arg);
#endif
}

int main(int argc, char* argv[]) throw(std::exception)
{
	int result = 0;
	try
	{
		vector<utf8> args;

		// convert args to vector of strings
		for (int x = 1; x < argc; ++x)
		{
			args.push_back(argv[x]);
		}

		// grab the calling program param as
		// will be needed to ensure relative
		// path handling will work correctly
#ifdef _WIN32
		result = do__main(args);
#else
		vector<utf8> arg;
		arg.push_back(argv[0]);
		result = do__main(args, arg);
#endif
	}
	catch(const std::exception &err)
	{
		printf("%s", (std::string(LIBRARY_LOG_TAG) + "Exception in main: " + err.what()).c_str());
	}
	return result;
}

/////////////////////////////////////////////////////

// surround by quotes
#ifdef _WIN32
static wstring quote(const wstring &s) throw()
{
	return wstring(L"\"") + s + wstring(L"\"");
}

int run_install(const vector<utf8> &args) throw()
{
	int result = -1;

	SC_HANDLE schSCManager = 0;
	SC_HANDLE schService = 0;

	try
	{
		wstring serviceName = (!args.empty() ? args[0].toWString().c_str() : L"Shoutcast DNAS Service");
		wstring account = (args.size() > 2 ? (args[1] == "0" ? L"" : args[1].toWString()) : L"");
		wstring password = (args.size() > 3 ? (args[2] == "0" ? L"" : args[2].toWString()) : L"");
			
		const size_t SIZ(2048);
		wchar_t nameBuffer[SIZ+2] = {0};
		::GetModuleFileNameW(0,nameBuffer,SIZ);

		schSCManager = ::OpenSCManagerW(NULL,	// local machine 
										NULL,	// ServicesActive database 
										SC_MANAGER_ALL_ACCESS);  // full access rights 

		if (schSCManager == NULL)
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
			{
				throw runtime_error("Aborting service install due to a lack of required permissions.\n\n"
									"Ensure you are using an Administrator Command Prompt or that you have administrator access to be able to install a service.");
			}
			else
			{
				static char error[512];
				snprintf(error, sizeof(error),
						 "Aborting service install due to OpenSCManager(..) failure.\n\nError code: %d\n[%s]",
						 GetLastError(), errMessage().c_str());
				throw runtime_error(error);
			}
		}

		wstring cmdString = quote(nameBuffer) + L" " + quote(L"daemon") + L" " + quote(serviceName);
		if (args.size() > 4) cmdString += L" " + quote(args[3].toWString());

		schService = ::CreateServiceW( 
				schSCManager,              // SCManager database 
				serviceName.c_str(),       // name of service 
				serviceName.c_str(),       // service name to display 
				SERVICE_ALL_ACCESS,        // desired access 
				SERVICE_WIN32_OWN_PROCESS, // service type 
				SERVICE_DEMAND_START,      // start type 
				SERVICE_ERROR_NORMAL,      // error control type 
				cmdString.c_str(),
				NULL,                      // no load ordering group 
				NULL,                      // no tag identifier 
				NULL,                      // no dependencies 
				(account == L"" ? NULL : account.c_str()),                      // LocalSystem account 
				(password == L"" ? NULL : password.c_str()));                   // no password 

		if (schService == NULL) 
		{
			int err = GetLastError();
			static char error[512];
			snprintf(error, sizeof(error),
					 "Aborting service install due to CreateService(..) failure.\n\nError code: %d\n[%s]%s",
					 err, errMessage().c_str(),
					 (err == ERROR_SERVICE_EXISTS ? "\n\nCheck the service name has not already been used." :
					  (err == ERROR_SERVICE_MARKED_FOR_DELETE ? "\n\nCheck the previous service instance has been completely stopped." : "")));
			throw runtime_error(error);
		}
		else
		{
			SERVICE_DESCRIPTION schServiceDesc;
			schServiceDesc.lpDescription = L"Shoutcast DNAS Server (sc_serv) v2";
			ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &schServiceDesc);
		}
		::CloseServiceHandle(schService); 
		::CloseServiceHandle(schSCManager);

		result = 0;
	}
	catch(const exception &ex)
	{
		::MessageBox(0,tows(ex.what()).c_str(),L"Shoutcast DNAS Error",MB_OK|MB_ICONEXCLAMATION);
		if (schService)
			::CloseServiceHandle(schService); 
		if (schSCManager)
			::CloseServiceHandle(schSCManager);
	}
	catch(...)
	{
		::MessageBox(0,L"Unknown exception",L"Shoutcast DNAS Error",MB_OK);
		if (schService)
			::CloseServiceHandle(schService); 
		if (schSCManager)
			::CloseServiceHandle(schSCManager);
	}

	return result;
}

/////////////////////////////////////////////////////

int run_uninstall(const vector<utf8> &args) throw()
{
	int result = -1;

	SC_HANDLE schSCManager = 0;
	SC_HANDLE service = 0;
	try
	{
		schSCManager = ::OpenSCManagerW(NULL,	// local machine 
										NULL,	// ServicesActive database 
										SC_MANAGER_ALL_ACCESS);  // full access rights 

		if (schSCManager == NULL)
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
			{
				throw runtime_error("Aborting service uninstall due to a lack of required permissions.\n\n"
									"Ensure you are using an Administrator Command Prompt or that you have administrator access to be able to uninstall a service.");
			}
			else
			{
				static char error[512];
				snprintf(error, sizeof(error),
						 "Aborting service uninstall due to OpenSCManager(..) failure.\n\nError code: %d\n[%s]",
						 GetLastError(), errMessage().c_str());
				throw runtime_error(error);
			}
		}

		service = ::OpenServiceW(schSCManager, (!args.empty() ? args[0].toWString().c_str() : L"Shoutcast DNAS Service"), DELETE);
		if (!service)
		{
			int err = GetLastError();
			static char error[512];
			snprintf(error, sizeof(error),
					 "Aborting service uninstall due to OpenService(..) failure.\n\nError code: %d\n[%s]%s",
					 err, errMessage().c_str(),
					 (err == ERROR_SERVICE_DOES_NOT_EXIST ? "\n\nCheck the service has not already been uninstalled and the\n"
															"service name matches what was used to register the service." : ""));
			throw runtime_error(error);
		}

		if (!::DeleteService(service))
		{
			int err = GetLastError();
			static char error[512];
			snprintf(error, sizeof(error),
					 "Aborting service uninstall due to DeleteService(..) failure.\n\nError code: %d\n[%s]%s",
					 err, errMessage().c_str(),
					 (err == ERROR_SERVICE_DOES_NOT_EXIST ? "\n\nCheck the service has not already been removed and that the\n"
															"service name matches what was used to register the service." :
					  (err == ERROR_SERVICE_MARKED_FOR_DELETE ? "\n\nCheck the previous service instance has been completely stopped." : "")));
			throw runtime_error(error);
		}

		::CloseServiceHandle(service);
		::CloseServiceHandle(schSCManager);

		result = 0;
	}
	catch(const exception &ex)
	{
		::MessageBox(0,tows(ex.what()).c_str(),L"Shoutcast DNAS Error",MB_OK|MB_ICONEXCLAMATION);
		if (service)
		{
			::CloseServiceHandle(service); 
		}
		if (schSCManager)
		{
			::CloseServiceHandle(schSCManager);
		}
	}
	catch(...)
	{
		if (service)
			::CloseServiceHandle(service);
		if (schSCManager)
			::CloseServiceHandle(schSCManager);
	}

	return result;
}
#endif

/////////////////////////////////////////////////////
static vector<utf8> gServiceArgs;

#ifdef _WIN32

//  Wraps SetServiceStatus.
void SetTheServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
						DWORD dwCheckPoint,   DWORD dwWaitHint)
{
	SERVICE_STATUS ss;  // Current status of the service.

	// Disable control requests until the service is started.
	if (dwCurrentState == SERVICE_START_PENDING)
		ss.dwControlsAccepted = 0;
	else
		ss.dwControlsAccepted =
				SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
				// Other flags include SERVICE_ACCEPT_PAUSE_CONTINUE
				// and SERVICE_ACCEPT_SHUTDOWN.

	// Initialize ss structure.
	ss.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
	ss.dwServiceSpecificExitCode = 0;
	ss.dwCurrentState            = dwCurrentState;
	ss.dwWin32ExitCode           = dwWin32ExitCode;
	ss.dwCheckPoint              = dwCheckPoint;
	ss.dwWaitHint                = dwWaitHint;

	// Send status of the service to the Service Controller.
	::SetServiceStatus(ssh, &ss);
}

static void WINAPI service_ctrl(DWORD dwCtrlCode)
{
	DWORD dwState = SERVICE_RUNNING;

	switch (dwCtrlCode)
	{
		case SERVICE_CONTROL_STOP:
			dwState = SERVICE_STOP_PENDING;
		break;

		case SERVICE_CONTROL_SHUTDOWN:
			dwState = SERVICE_STOP_PENDING;
		break;

		case SERVICE_CONTROL_INTERROGATE:
		break;

		default:
		break;
	}

	// Set the status of the service.
	SetTheServiceStatus(dwState, NO_ERROR, 0, 0);

	// Tell service_main thread to stop.
	if ((dwCtrlCode == SERVICE_CONTROL_STOP) ||
		(dwCtrlCode == SERVICE_CONTROL_SHUTDOWN))
	{
		_SetEvent(serviceMain::sStop);
	}
}

static void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	try
	{
		// Register the service ctrl handler.
		//if (dwArgc == 0)
		if (gServiceArgs.empty())
		{
			throw runtime_error("service_main - no name");
		}
		ssh = ::RegisterServiceCtrlHandler(lpszArgv[0],(LPHANDLER_FUNCTION)service_ctrl);
		if (!ssh)
		{
			throw runtime_error("RegisterServiceCtrlHandler returned NULL");
		}
		// The service has started.
		SetTheServiceStatus(SERVICE_RUNNING, 0, 0, 0);

		vector<utf8> args(gServiceArgs.begin() + 1,gServiceArgs.end());

		#ifdef _WIN32
		::SetConsoleCtrlHandler(_console_handler,TRUE);
		#else
		gSigWatcherThread.start();
		#endif

		sm_main(args);

		#ifndef _WIN32
		::pthread_kill(gSigWatcherThread,SIGTERM);
		gSigWatcherThread.join();
		#endif

		// Stop the service.
		OutputDebugString(TEXT("SetTheServiceStatus, SERVICE_STOPPED\n"));
		SetTheServiceStatus(SERVICE_STOPPED, NO_ERROR, 0, 0);
	}
	catch(...)
	{
		SetTheServiceStatus(SERVICE_STOPPED, ::GetLastError(), 0, 0);
	}
}

#endif

#ifdef _WIN32
int run_daemon(const vector<uniString::utf8> &args) throw(std::exception)
#else
int run_daemon(const vector<uniString::utf8> &args, const vector<uniString::utf8> &arg) throw(std::exception)
#endif
{
	sDaemon = true;

	gServiceArgs = args;
	_ResetEvent(serviceMain::sStop);
	
#ifdef _WIN32
	SERVICE_TABLE_ENTRY ste[] =
		{{TEXT("Shoutcast DNAS"),(LPSERVICE_MAIN_FUNCTION)service_main}, {NULL, NULL}};

	gStartupDirectory = utf32(getfwd()).toUtf8();

	if (!::StartServiceCtrlDispatcher(ste))
	{
		static TCHAR error[512];
		_snwprintf(error, sizeof(error),
				   TEXT("Error code for StartServiceCtrlDispatcher: %u [%hs].\n"),
				   GetLastError(), errMessage().c_str());
		MessageBox (NULL, error, NULL, MB_SERVICE_NOTIFICATION);
		return -1;
	}
	return 0;

#else
	pid_t pid;
	if ((pid = fork()) < 0)
	{
		return -1;
	}
	else if (pid != 0)
	{
		cout << "sc_serv2 going daemon with PID [" << tos(pid) << "]" << endl;
		exit(0); // parent goes away
	}
	// child continues
	setsid();
	gStartupDirectory = getfwd((const char*)&(arg[0]));

	blockSignals();
	gSigWatcherThread.start();

	int result = sm_main(args);

	::pthread_kill(gSigWatcherThread,SIGTERM);
	gSigWatcherThread.join();

	return result;
#endif
}

//////////////////////////////////////////////////////
#ifdef _WIN32
wchar_t* getfwd() throw()
{
	// determine the actual location of ourselves and use as needed
	static wchar_t fwd[MAX_PATH];
	if (!fwd[0])
	{
		GetModuleFileNameW(NULL, fwd, ARRAYSIZE(fwd));

		// this is needed for the service mode
		// so simpler to set it here than later
		gStartupPath = utf32(fwd).toUtf8();

		PathRemoveFileSpecW(fwd);
		PathAddBackslashW(fwd);

		// this mirrors existing Windows handling despite
		// other 2.4.2 changes needing to be made for it.
		SetCurrentDirectory(fwd);
	}
	return fwd;
}
#else
char* getfwd(const char* argv) throw()
{
	// determine the actual location of ourselves and use as needed
	static char fwd[MAXPATHLEN + 1];
	if (!fwd[0])
	{
		// first attempt to use readlink(..) as per the platform build
		#if (defined PLATFORM_LINUX || defined PLATFORM_ARMv6 || defined PLATFORM_ARMv7)
		if(readlink("/proc/self/exe", fwd, sizeof(fwd) - 1) == -1)
		#endif
		#ifdef PLATFORM_BSD
		if(readlink("/proc/curproc/file", fwd, sizeof(fwd) - 1) == -1)
		#endif
		#ifdef __APPLE_CC__
		uint32_t fwdSize = sizeof(fwd);
		if(!_NSGetExecutablePath(fwd, &fwdSize))
		#endif
		{
			#ifdef __APPLE_CC__
			// for this, we get the full program path which can include symlinks
			// so this will adjust it all so as to get a clean path and then to
			// strip off the program file name (also included) so we match all of
			// the other OS versions of this method so it will work consistently.
			strncpy(fwd, fileUtil::onlyPath(fileUtil::getFullFilePath(string(fwd))).hideAsString().c_str(), sizeof(fwd) - 1);
			#endif
			
			// now look at argv for a / in it
			if (strchr(argv, '/'))
			{
				// if it starts with a / it's absolute so just use
				if (argv[0] == '/')
				{
					strncpy(fwd, argv, sizeof(fwd) - 1);
				}
				// otherwise attempt to append to the cwd
				// only risk is if the cwd changed onload
				else
				{
					if (getcwd(fwd, sizeof(fwd) - 1))
					{
						int len = sizeof(fwd) - strlen(fwd);
						strncat(fwd, argv, min(len - 1, (int)sizeof(fwd) - 1));
					}
					// and if that doesn't work then set
					// it as / and behave like older builds
					else
					{
						strncpy(fwd, "/", sizeof(fwd) - 1);
					}
				}
			}
		}
		else
		{
			char tmp[MAXPATHLEN + 1] = {0};
			strncpy(fwd, strncpy(tmp, dirname(fwd), sizeof(tmp) - 1), sizeof(fwd) - 1);
		}

		// must be slash terminated
		size_t fwd_len = strlen(fwd);
		if (fwd_len && (fwd_len < (sizeof(fwd) - 1)) && fwd[fwd_len - 1] != '/')
		{
			strncat(fwd, "/", sizeof(fwd) - 1);
		}
	}
	
	return fwd;
}
#endif

#ifdef _WIN32
int run_run(const vector<uniString::utf8> &args) throw(std::exception)
#else
int run_run(const vector<uniString::utf8> &args, const vector<uniString::utf8> &arg) throw(std::exception)
#endif
{
#ifdef _WIN32
	::SetConsoleCtrlHandler(_console_handler,TRUE);
	gStartupDirectory = utf32(getfwd()).toUtf8();
#else
	if (blockSignals())
	{
		cerr << "pthread_sigmask failed in run_run()" << endl; exit(-1);
	}
	gSigWatcherThread.start();

#ifdef PLATFORM_LINUX
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = custom_signal_handler;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);  
	sigaction(SIGFPE, &sa, NULL);
#endif

	gStartupDirectory = getfwd((const char*)&(arg[0]));
#endif

	int result = 0;
	try
	{
		result = sm_main(args);
	}
	catch(const std::runtime_error &err)
	{
		printf("%s", (std::string(LIBRARY_LOG_TAG) + "Exception in main: " + err.what()).c_str());
	}
	catch(const std::exception &err)
	{
		printf("%s", (std::string(LIBRARY_LOG_TAG) + "Exception in main: " + err.what()).c_str());
	}
	catch(...)
	{
		printf("%s", (std::string(LIBRARY_LOG_TAG) + "Unknown exception in main()").c_str());
	}

#ifndef _WIN32
	::pthread_kill(gSigWatcherThread,SIGTERM);
	gSigWatcherThread.join();
#endif

	return result;
}
