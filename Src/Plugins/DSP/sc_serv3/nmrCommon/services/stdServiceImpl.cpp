#include "stdServiceImpl.h"
#include "file/fileUtils.h"
#include "stl/stringUtils.h"

using namespace std;
using namespace stringUtil;
using namespace uniString;

AOL_logger::stdLog_t *gLog = 0;
uniFile::filenameType gStartupDirectory;
uniFile::filenameType gStartupPath;

// shutdown the logger
void stdServiceBase::stopLog() throw()
{
	if (!gLog) return;
	//ILOG(LIBRARY_LOG_TAG "Logger shutdown");
	gLog->postMessage(AOL_logger::message::makeDone()); // logger shutdown message
	gLog->join(); // wait for logger to stop
	forget(gLog);
}

// start log in minimal panic mode
void stdServiceBase::startPanicLog() throw()
{
	stopLog();
	try
	{
		gLog = new AOL_logger::stdLog_t;
		addPanicLogElements();
		gLog->start();
	}
	catch(...){}
}

// start log in normal mode
void stdServiceBase::startNormalLog(bool partial) throw(std::runtime_error)
{
	if (partial == false)
	{
		stopLog();
	}
	try
	{
		if (partial == false)
		{
			gLog = new AOL_logger::stdLog_t;
		}
		addNormalLogElements(partial);
		gLog->start();
		if (partial == false)
		{
			//ILOG(LIBRARY_LOG_TAG "Logger startup");
		}
		else
		{
			ILOG("[MAIN] Logger updating log file to use");
		}
	}
	catch(...)
	{
		forget(gLog);
		throw;
	}
}


// start log in normal mode
void stdServiceBase::startScreenLog() throw()
{
	try
	{
		addConsoleLogElements();
	}
	catch(...){}
}

void stdServiceBase::postloop() throw()
{		
	comUninit();
	_SetEvent(serviceMain::sStop);
}

///// main entry point to primary flow of control (after daemon nonsense, Win32 service nonsense, etc).
int stdServiceBase::sm_main(const vector<utf8> &args) throw()
{
	int result = -1;
	try
	{
		preloop(/* args */); // moved to preflight so logger can be reconfigured on internal restart
		result = loop(args); // will only throw during preflight
	}
	catch(const exception &err)
	{
		panic(err.what());
	}
	catch(...)
	{
		panic("Unknown exception");
	}
	postloop();
	return result;
}

#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////////
//////////////////////// win32 specific ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void stdServiceWin32generic::comInit() throw(exception)
{
	if (FAILED(CoInitialize(NULL)))
	{
		throw runtime_error("CoInitialize failure");
	}
}

void stdServiceWin32generic::comUninit() throw()
{
	CoUninitialize();
}

// panic occurs when an error happens before the loggers
// are brought up.
void stdServiceWin32generic::panic(const utf8 &errM) throw()
{
	wstring err_m(errM.toWString());
	if (!sDaemon)
	{
		::MessageBox(0,err_m.c_str(),L"SHOUTcast DNAS Error",MB_OK|MB_ICONEXCLAMATION);
	}

	// try to build the event logger
	if (FAILED(CoInitialize(NULL)))
	{
		return;
	}

	try
	{
		startPanicLog();
		ELOG(LIBRARY_LOG_TAG + errM);
		stopLog();
	}
	catch(...){}
	CoUninitialize();
}

#else
//////////////////////////////////////////////////////////////////////////////
//////////////////////// unix specific ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void stdServiceUnixgeneric::panic(const utf8 &errM) throw()
{
	try
	{
		startPanicLog();
		ELOG(LIBRARY_LOG_TAG + errM);
		stopLog();
	}
	catch(...){}
}

#endif
