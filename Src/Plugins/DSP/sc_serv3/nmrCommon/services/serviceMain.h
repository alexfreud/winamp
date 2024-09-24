#pragma once
#ifndef serviceMain_H_
#define serviceMain_H_

#ifdef _WIN32
#include <windows.h>
#endif
#include <vector>
#include <string>
#include "threading/thread.h"
#include "unicode/uniFile.h"
#include "../../versions.h"

/*
	Class to encapsulate command line or service based launching.
	convention is that the first parameter will determine the action
	taken, either "install" (install service) "uninstall" (uninstall service)
	"daemon" (run as windows service or unix daemon) or "run" (run as application).
	If the keyword is missing then "run" is assumed.
	
	if action is "install" then you must provide the following arguments
	
		servicename logon password .... application arguments .....
	
	for example
	
		pm.exe install procmgr myaccount mypassword param1 param2 param3
	
	To use the default account and password you use a zero
	
		pm.exe install procmgr 0 0 param1 param2
			
	if action is "uninstall" then you must provide the following arguments
	
		servicename
		
	This only deals with process services.
*/

class serviceMain
{
public:
	static event sStop;  // if this is signaled, the service must stop. You must monitor this variable
#ifndef _WIN32
	static event sHUP;	// other unix signals
	static event sWINCH;
	static event sUSR1;
	static event sUSR2;
#endif
};

extern int main(int argc, char* argv[]) throw(std::exception);

// you must define this guy:
extern int sm_main(const std::vector<uniString::utf8> &args) throw();

#ifdef _WIN32
BOOL WINAPI _console_handler(DWORD fdwCtrlType);
wchar_t* getfwd() throw();
#else
int blockSignals() throw();
char* getfwd(const char* argv) throw();
#endif

#ifndef _WIN32
// We need access to this for Apple, because we don't have sigtimedwait(). This means there
// circumstances when we need access to this thread so we can signal it and force sigwait() to exit

extern Tthread<pointer_to_thread_function> gSigWatcherThread;
#endif

#endif
