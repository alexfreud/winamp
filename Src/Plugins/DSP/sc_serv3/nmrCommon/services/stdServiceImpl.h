#pragma once
#ifndef stdServiceImpl_H_
#define stdServiceImpl_H_

#include <vector>
#include <string>
#include <stdexcept>
#include <stdio.h>

#include "serviceMain.h"
#include "logger.h"
#include "../../config.h"

#ifdef _WIN32
#define __F__ __FUNCTION__
#else
#define __F__ string(__PRETTY_FUNCTION__) + 
#endif

#define LIBRARY_LOG_TAG "<***> "

extern AOL_logger::stdLog_t *gLog;
extern uniFile::filenameType gStartupDirectory;
extern uniFile::filenameType gStartupPath;
extern config gOptions;

// are we running as a daemon or service
extern bool sDaemon;

#define ELOG(...)       do { if (gLog) gLog->postMessage(AOL_logger::message::makeError(__VA_ARGS__)); } while (0)
#define WLOG(...)       do { if (gLog) gLog->postMessage(AOL_logger::message::makeWarning(__VA_ARGS__)); } while (0)
#define ILOG(...)       do { if (gLog) gLog->postMessage(AOL_logger::message::makeInfo(__VA_ARGS__)); } while (0)
#define DLOG(...)       do { if (gLog) gLog->postMessage(AOL_logger::message::makeDebug(__VA_ARGS__)); } while (0)
#define ULOG(...)       do { if (gLog) gLog->postMessage(AOL_logger::message::makeUpdate(__VA_ARGS__)); } while (0)
#define ROTATE          do { if (gLog) gLog->postMessage(AOL_logger::message::makeRotate()); } while (0)

#define HUP_SIGNAL 45

class stdServiceBase
{
protected:
	virtual ~stdServiceBase() throw(){}
	virtual void addPanicLogElements() throw(std::exception) = 0;
	virtual void addConsoleLogElements() throw(std::exception) = 0;
	virtual void addNormalLogElements(bool partial) throw(std::exception) = 0;
	virtual void preloop() throw(std::exception) = 0;
	virtual int loop(const std::vector<uniString::utf8> &args) throw(std::exception) = 0;

	void base_preloop() throw(std::exception)
	{
		comInit();
	}

	virtual void postloop() throw();

	static void preflight(const std::vector<uniString::utf8> &args) throw(std::exception)
	{
		// get cmd line settings
		const std::vector<uniString::utf8> leftover(gOptions.fromArgs(args));
		if (!leftover.empty())
		{
			uniString::utf8 s;
			for (std::vector<uniString::utf8>::const_iterator i = leftover.begin(); i != leftover.end(); ++i)
			{
				s += (*i) + " ";
			}
			throw std::runtime_error(std::string(LIBRARY_LOG_TAG) + "Bad cmd line parameters: `" + s.hideAsString() + "'");
		}
	}

	template<typename APP>
	int base_loop(const std::vector<uniString::utf8> &args) throw(std::exception)
	{
		int result = -1;
		bool done(false);
		while (!done)
		{	
			_ResetEvent(serviceMain::sStop);

			try
			{
				preflight(args);
				startNormalLog(false);
				result = APP().go(*this);
			}
			catch(const std::exception &err)
			{
				printf("%s", (std::string(LIBRARY_LOG_TAG) + "Exception in main: " + err.what()).c_str());
				done = true;
			}
			catch(...)
			{
				printf("%s", (std::string(LIBRARY_LOG_TAG) + "Unknown exception in main()").c_str());
				done = true;
			}

			stopLog();
			if (result != HUP_SIGNAL)
			{
				done = true;
			}
		}	
		return result;
	}

public:
	virtual void comInit() throw(std::exception) = 0;
	virtual void comUninit() throw() = 0;
	virtual void stopLog() throw();
	virtual void startNormalLog(bool partial) throw(std::runtime_error);
	virtual void startScreenLog() throw();
	virtual void startPanicLog() throw();
	virtual void panic(const uniString::utf8 &errM) throw() = 0;
	int sm_main(const std::vector<uniString::utf8> &args) throw();
};

#ifdef _WIN32
// stuff that doesn't require templates
class stdServiceWin32generic: public stdServiceBase
{
public:
	virtual void comInit() throw(std::exception);
	virtual void comUninit() throw();
	virtual void panic(const uniString::utf8 &errM) throw();
};

template<typename APP>
class stdServiceWin32: public stdServiceWin32generic
{
protected:
	uniString::utf8 m_serviceName;

	virtual void addPanicLogElements() throw(std::exception)
	{
		gLog->addElement(new AOL_logger::systemLogger_element(m_serviceName, gStartupPath, AOL_logger::systemLogger_element::panicConfiguration()));
	}

	virtual void addConsoleLogElements() throw(std::exception)
	{
		_ASSERTE(gLog);

		AOL_logger::consoleLogger_element *c = 0;

		try
		{
			if (gOptions.getConsoleLogging())
			{
				gLog->addElement(c = new AOL_logger::consoleLogger_element());
				c = 0;
			}
		}
		catch(...)
		{
			forget(c);
			throw;
		}

		// handler must be set after console is created
		if (!sDaemon && (gOptions.getConsoleLogging()))
		{
			::SetConsoleCtrlHandler(_console_handler,TRUE);
		}
	}

	// start log in normal mode (may throw)
	virtual void addNormalLogElements(bool partial) throw(std::exception)
	{
		_ASSERTE(gLog);

		AOL_logger::fileLogger_element *f = 0;
		AOL_logger::systemLogger_element *s = 0;

		try
		{
			APP::addCustomLogElements();// give app opportunity to add special elements
			if (!gOptions.getFileLog().empty())
			{
				if (partial == true)
				{
					gLog->postMessage(AOL_logger::message::makeDone()); // logger shutdown message
					gLog->join();
				}
				bool m_useDefault = false;
				gLog->addElement(f = new AOL_logger::fileLogger_element(gOptions.getFileLog(), gOptions.logFile_Default(),
																		m_useDefault, gOptions.logRotates(),
																		gOptions.logArchive(), gOptions.rotateInterval()));
				if (m_useDefault)
				{
					wchar_t m_defaultFileName[MAX_PATH] = {0};
					ExpandEnvironmentStringsW(DEFAULT_LOGW, m_defaultFileName, MAX_PATH);
					gOptions.setOption(uniString::utf8("logfile"), uniString::utf32(m_defaultFileName).toUtf8());
				}

				f = 0;
			}
			if ((partial == false) && sDaemon)
			{
				gLog->addElement(s = new AOL_logger::systemLogger_element(m_serviceName, gStartupPath,
																		  gOptions.getSystemLogConfigString()));
				s = 0;
			}
		}
		catch(...)
		{
			forget(f);
			forget(s);
			throw;
		}

		// handler must be set after console is created
		if ((partial == false) && !sDaemon && (gOptions.getConsoleLogging()))
		{
			::SetConsoleCtrlHandler(_console_handler,TRUE);
		}
	}

	virtual void preloop() throw(std::exception) { stdServiceBase::base_preloop(); }
	virtual int loop(const std::vector<uniString::utf8> &args) throw(std::exception) { return stdServiceBase::base_loop<APP>(args); }

public:
	explicit stdServiceWin32(const std::string &serviceName):m_serviceName(serviceName){}
};

#else

// stuff that doesn't require templates
class stdServiceUnixgeneric: public stdServiceBase
{
public:
	virtual void comInit() throw(std::exception){}
	virtual void comUninit() throw(){}
	virtual void panic(const uniString::utf8 &errM) throw();
};

template<typename APP>
class stdServiceUnix: public stdServiceUnixgeneric
{
protected:
	virtual void addPanicLogElements() throw(std::exception)
	{
		gLog->addElement(new AOL_logger::consoleLogger_element());
	}

	virtual void addConsoleLogElements() throw(std::exception)
	{
		assert(gLog);

		AOL_logger::consoleLogger_element *c = 0;

		try
		{
			if (gOptions.getConsoleLogging())
			{
				gLog->addElement(c = new AOL_logger::consoleLogger_element());
				c = 0;
			}
		}
		catch(...)
		{
			forget(c);
			throw;
		}
	}

	// start log in normal mode (may throw)
	virtual void addNormalLogElements(bool partial) throw(std::exception)
	{
		assert(gLog);

		AOL_logger::fileLogger_element *f = 0;

		try
		{
			APP::addCustomLogElements(); // give app an opporunity to add special loggers
			if (!gOptions.getFileLog().empty())
			{
				if (partial == true)
				{
					gLog->postMessage(AOL_logger::message::makeDone()); // logger shutdown message
					gLog->join();
				}
                size_t sid;
                size_t count = gOptions.count_stream_logFile();
                int i;

                for (i = 0; i < count; ++i)
                {
                    sid = 0;
                    uniString::utf8 fn = gOptions.fetchMulti (gOptions.stream_logFile_map(), i, "", &sid);
                    bool m_useDefault = false;
                    f = new AOL_logger::fileLogger_element (fn, fn, m_useDefault, gOptions.logRotates(),
                            gOptions.logArchive(), gOptions.rotateInterval(), sid);
                    gLog->addElement (f);
                }

				bool m_useDefault = false;
				gLog->addElement(f = new AOL_logger::fileLogger_element(gOptions.getFileLog(), gOptions.logFile_Default(),
																		m_useDefault, gOptions.logRotates(),
																		gOptions.logArchive(), gOptions.rotateInterval()));
				if (m_useDefault)
				{
					gOptions.setOption(uniString::utf8("logfile"),gOptions.logFile_Default());
				}
				f = 0;
			}
		}
		catch(...)
		{
			forget(f);
			throw;
		}
	}

	virtual void preloop() throw(std::exception) { stdServiceBase::base_preloop(); }
	virtual int loop(const std::vector<uniString::utf8> &args) throw(std::exception) { return stdServiceBase::base_loop<APP>(args); }

public:
	stdServiceUnix() throw(){}
};
#endif

#endif
