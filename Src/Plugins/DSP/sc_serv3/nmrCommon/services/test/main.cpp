#include "stdServiceImpl.h"

using namespace std;

#ifndef WIN32
#define DWORD int
#define TRUE true
#define FALSE false
#endif

static bool valToBool(const string &s) throw()
{
	if (s.empty()) return false;
	return (s[0] == '1' || s[0] == 't' || s[0] == 'T' || s[0] == 'y' || s[0] == 'Y');
}

class options
{
public:
	string	m_name;
	string	m_fileLog;
	bool	m_consoleLogging;

	vector<string> fromArgs(const vector<string> &args) throw()
	{
		vector<string> unused;

		for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
		{
			string::size_type colon_pos = (*i).find(":");
			if (colon_pos != string::npos)
			{
				string key	= (*i).substr(0,colon_pos);
				string value= (*i).substr(colon_pos+1);
				if (key == "name")			{ m_name = value; }
				else if (key == "clog")		{ m_consoleLogging = valToBool(value); }
				else if (key == "flog")		{ m_fileLog = value; }
				else
				{
					unused.push_back(*i);
				}
			}
			else
			{
				unused.push_back(*i);
			}
		}

		return unused;
	}

	string logText() const throw()
	{
		ostringstream o;
		o << endl;
		o << "name = " << m_name << endl;
		o << "file log = " << m_fileLog << endl;
		o << "console logging = " << m_consoleLogging << endl;
		return o.str();
	}
};

options gOptions;

class testService
{
private:
	bool m_done;
	int  m_goResult;

public:
	explicit testService(stdServiceBase &b) :
		m_done(false),
		m_goResult(0){}

	int go(stdServiceBase &b) throw(exception)
	{
		DLOG(__F__ "");

		m_goResult = 0;
		bool comInitialized = false;

		try
		{
			b.comInit();

			// event loop (until done)
			while (!m_done)
			{
				event	dummyEvent(TRUE);
				HANDLE evts[2];
				DWORD evtCount = 0;

				evts[evtCount++] = serviceMain::sStop;
//				evts[evtCount++] = m_webServer.getRequestQueueEvent();//m_webServer.getCommandSignal();

				//wait for a single from the webserver or console abort and take action
				DWORD waitResult = ::WaitForMultipleObjects(evtCount,evts,FALSE,250);

//				gProcessTable.update();
				switch(waitResult)
				{
					case WAIT_OBJECT_0:
					{
						ILOG("Ctrl+C application termination");
						m_done = true;
						break;
					}
//					case WAIT_OBJECT_0+1:
//					{
//						DLOG("Got a web event");
//						handleWebEvent();
//						break;
//					}
					default:
					{
//						if (m_perfmon)
//						{
//							time_t ttt = time(NULL);
//							m_perfmon->updateEPOCH(ttt);
//							m_perfmon->updateAppCounter(gProcessTable.countRunning());
//						}

						//DLOG("PM: Got a periodic event");
						// periodic event
//						if (gProcessTable.isIdle())
//						{
//							if (m_QuitWhenIdle)
//							{
//								m_done = true;
//								m_goResult = 0;
//							}
//							else if (m_HUPWhenIdle)
//							{
//								m_done = true;
//								m_goResult = HUP_SIGNAL;
//							}
//						}
						break;
					}
				}
			}
			b.comUninit();
			comInitialized = false;
			::SetEvent(serviceMain::sStop);
		}
		catch(...)
		{
			ELOG(__F__ " Caught an exception");
			if (comInitialized)
			{
				b.comUninit();
			}
			::SetEvent(serviceMain::sStop);
			throw;
		}
		return m_goResult;
	}
};

int sm_main(const vector<string> &args) throw()
{
#ifdef WIN32
	stdServiceWin32<testService,options> s("testService");
#else
	stdServiceUnix<testService,options> s;
#endif
	return s.sm_main(args);
}
