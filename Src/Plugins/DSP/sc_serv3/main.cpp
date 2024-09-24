#ifdef _WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <conio.h>
#else
#include <sys/resource.h>
#include <termios.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "services/stdServiceImpl.h"
#include "file/fileUtils.h"
#include "global.h"
#include "threadedRunner.h"
#include "protocol_relay.h"
#include "w3cLog.h"
#include "yp2.h"
#include "updater.h"
#include "auth.h"
#include "streamData.h"
#include "adminList.h"
#include "banList.h"
#include "ripList.h"
#include "agentList.h"
#include "cpucount.h"
#include "stats.h"
#include "bandwidth.h"
#include "cache.h"

#ifdef _WIN32
#define _WSPIAPI_H_
#endif
//#include <GeoIP.h>

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define LOGNAME "[MAIN] "

#ifdef _WIN32
#include <Mswsock.h>
static void win32SocketSetup() throw(runtime_error)
{
	WSADATA wsaData = {0};
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		throw runtime_error("Could not find usable Winsock DLL");
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		WSACleanup();
		throw runtime_error("Could not find appropriate winsock dll version");
	}
	/* The WinSock DLL is acceptable. Proceed. */

	// if we can detect WSAPoll(..) then we'll use that over
	// select which means Vista+ should be similar to Linux.
	HINSTANCE ws2_32 = GetModuleHandle(TEXT("WS2_32.DLL"));
	if (ws2_32 != NULL)
	{
		typedef INT (WSAAPI *LPFN_WSAPOLL)(LPWSAPOLLFD fdarray, ULONG nfds, INT timeout);
		extern LPFN_WSAPOLL fnWSAPoll;
		fnWSAPoll = (LPFN_WSAPOLL)GetProcAddress(ws2_32, "WSAPoll");
	}
}

static void win32SocketCleanup() throw()
{
	WSACleanup();
}
#else
int _kbhit(void)
{
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	int ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	return (ch != EOF);
}
#endif

void scheduleRelay(const config::streamConfig &info) throw()
{
	// only attempt to run if not indicated as having moved
	if(gOptions.stream_movedUrl(info.m_streamID).empty())
	{
		threadedRunner::scheduleRunnable(new protocol_relay(info));
	}
}

utf8 getLogFile(utf8 fileName)
{
#ifdef _WIN32
	// this will fill in the default log path as required
	wchar_t s_defaultFileName[MAX_PATH] = {0};
	ExpandEnvironmentStringsW(DEFAULT_LOGW, s_defaultFileName, MAX_PATH);
	utf8 m_defaultFilename(utf32(s_defaultFileName).toUtf8()), m_fileName = fileName;

	HANDLE m_file = ::CreateFileW(m_fileName.toWString().c_str(),GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (m_file == INVALID_HANDLE_VALUE)
	{
		m_file = ::CreateFileW(m_defaultFilename.toWString().c_str(),GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if (m_file == INVALID_HANDLE_VALUE)
		{
			uniFile::filenameType fallbackFilename("%temp%\\sc_serv_" + tos(getpid()) + ".log");
			wchar_t s_fallbackFileName[MAX_PATH] = {0};
			ExpandEnvironmentStringsW(fallbackFilename.toWString().c_str(), s_fallbackFileName, MAX_PATH);
			utf8 m_fallbackFilename(utf32(s_fallbackFileName).toUtf8());

			m_file = ::CreateFileW(m_fallbackFilename.toWString().c_str(),GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if (m_file != INVALID_HANDLE_VALUE)
			{
				forgetHandleInvalid(m_file);
			}
			else
			{
				m_fileName = m_fallbackFilename;
			}
		}
		else
		{
			m_fileName = m_defaultFilename;
		}
	}
#else
	utf8 m_defaultFilename = gOptions.logFile_Default(), m_fileName = fileName;

	int m_file = ::open(m_fileName.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (m_file == -1)
	{
		m_file = ::open(m_defaultFilename.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (m_file == -1)
		{
			uniFile::filenameType fallbackFilename("/tmp/sc_serv_" + tos(getpid()) + ".log");
			m_file = ::open(fallbackFilename.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			if (m_file == -1)
			{
				throw runtime_error("Logger could not open the log file \"" + m_fileName.hideAsString() + "\" for writing [" + errMessage().hideAsString() + "]. Check the directory exists and another instance is not already running.");
			}
			else
			{
				m_fileName = fallbackFilename;
			}
		}
		else
		{
			m_fileName = m_defaultFilename;
		}
	}
#endif
	return m_fileName;
}

// event triggers for main loop
static vector<HANDLE>	s_ControlEvents;

class sc_serv2_service
{
	// log messages that have been stuffed in config objects deferred list.
	// this only need be called at startup, to display messages that were created during
	// system initialization, but could not be displayed because the loggers didn't yet exist.
	static void logDeferredMessages()
	{
		const vector<utf8> &deferred_error_log_messages(gOptions.deferredErrorLogMessages());
		for (vector<utf8>::const_iterator i = deferred_error_log_messages.begin(); i != deferred_error_log_messages.end(); ++i)
		{
			ELOG((*i).hideAsString());
		}

		const vector<utf8> &deferred_warn_log_messages(gOptions.deferredWarnLogMessages());
		for (vector<utf8>::const_iterator i = deferred_warn_log_messages.begin(); i != deferred_warn_log_messages.end(); ++i)
		{
			WLOG((*i).hideAsString());
		}

		gOptions.clearDeferredErrorLogMessages();
		gOptions.clearDeferredWarnLogMessages();
	}

public:
	static void addCustomLogElements() {}
	sc_serv2_service() {}

	// main application loop
	static int go(stdServiceBase &base) throw()
	{
		g_upTime = ::time(NULL);
		g_userAgent = g_userAgentBase + gOptions.getVersionBuildStrings();
		#ifdef _WIN32
		bool win32_socket_cleanup_required(false);
		#endif
		int mainResult = 0; // result of main loop
		utf8 pidFn;
		vector<threadedRunner*>	runners;

		try
		{
runserver:
#ifdef CONFIG_BUILDER
			bool builderMode = (gOptions.confFile() == "builder" || gOptions.confFile() == "-b");
			bool setupMode = (gOptions.confFile() == "setup" || gOptions.confFile() == "-s" || builderMode);
#else
			bool setupMode = (gOptions.confFile() == "setup" || gOptions.confFile() == "-s");
#endif
			bool gotConfig = (!gOptions.confFile().empty() && uniFile::fileExists(gOptions.confFile()));
			// look for sc_serv.conf or sc_serv.ini to emulate v1 DNAS behaviour as
			// too many people cannot get on with having to specify a config file :(
			if (gotConfig == false && setupMode == false)
			{
				utf8 currentLogFile = gOptions.logFile();

				#ifdef _WIN32
				vector<wstring> fileList = fileUtil::directoryFileList(gStartupDirectory.toWString() + L"sc_serv.ini", L"", true, true);
				vector<wstring> fileListConf = fileUtil::directoryFileList(gStartupDirectory.toWString() + L"sc_serv.conf", L"", true, true);
				#else
				vector<string> fileList = fileUtil::directoryFileList(gStartupDirectory.hideAsString() + "sc_serv.ini", "");
				vector<string> fileListConf = fileUtil::directoryFileList(gStartupDirectory.hideAsString() + "sc_serv.conf", "");
				#endif

				if (!fileList.empty())
				{
					fileList.insert(fileList.end(), fileListConf.begin(),fileListConf.end());
				}
				else
				{
					fileList = fileListConf;
				}

				if (!fileList.empty())
				{
					#ifdef _WIN32
					utf32 u32file(fileList[0]);
					utf8 u8f(u32file.toUtf8());
					gotConfig = gOptions.load(u8f);
					#else
					gotConfig = gOptions.load(fileList[0]);
					#endif
				}

				// if these do not match then we need to update the log file being used
				// as otherwise it reports the wrong thing on the admin pages, etc
				if (gOptions.logFile() != currentLogFile)
				{
					base.startNormalLog(false);
				}
			}

			if (isPostSetup() == false)
			{
				if (gOptions.screenLog())
				{
					base.startScreenLog();
				}
				// during initial startup some messages are produced but cannot be logged because
				// the loggers do not yet exist. These are in the deferred list. At this point they
				// can be safely logged
				logDeferredMessages();

				gOptions.m_certPath = fileUtil::getFullFilePath(gStartupDirectory + "cacert.pem");
				if (!uniFile::fileExists(gOptions.m_certPath))
				{
					WLOG(LOGNAME "Cannot find `" + gOptions.m_certPath + "'");
					WLOG(LOGNAME "Without `cacert.pem' the DNAS may not be able to contact the Directory");
					WLOG(LOGNAME "The latest can be downloaded from `http://curl.haxx.se/ca/cacert.pem'" + eol());
				}

				#ifdef _WIN32
				ILOG("*********************" +
					 string(!sDaemon ? "***************************" : "<<RUNNING_IN_SERVICE_MODE>>") +
					 "*********************");
				#else
				ILOG("*********************" +
					 string(!sDaemon ? "***************************" : "<<RUNNING_IN__DAEMON_MODE>>") +
					 "*********************");
				#endif
				ILOG("**        Shoutcast Distributed Network Audio Server (DNAS)        **");
				ILOG("**    Copyright (C) 2014-2023 Radionomy SA, All Rights Reserved    **");
				if (gotConfig == false && setupMode == false)
				{
					ILOG("**      Use \"sc_serv [filename]\" to specify a config file       **");
				}
				ILOG("*********************************************************************");
				utf8 version = gOptions.getVersionBuildStrings();
#if defined(_DEBUG) || defined(DEBUG)
				version += "[DBUG]";
#endif
				ILOG(LOGNAME "Shoutcast DNAS/" SERV_OSNAME " v" + version + " (" __DATE__ ")");
				ILOG(LOGNAME "PID: " + tos(getpid()));
			}
			if (gotConfig)
			{
				ILOG(LOGNAME "Saving log output to `" + fileUtil::getFullFilePath(gOptions.realLogFile()) + "'");
				ILOG(LOGNAME "Automatic log rotation " + (!gOptions.rotateInterval() ? utf8("disabled") : utf8("interval: ") + timeString(gOptions.rotateInterval())));
				ILOG(LOGNAME "Loaded config from `" + fileUtil::getFullFilePath(gOptions.confFile()) + "'");
			}
			else
			{
				uniFile::filenameType oldLogFile = gOptions.logFile();
				// if we get to this state then to make things easier, we attempt to offer
				// some possible configuration files to attempt to load as the config file
				int mode = (setupMode ? 2 : gOptions.promptConfigFile());
				if (mode <= 0)
				{
					if (mode == -1)
					{
						ELOG(LOGNAME "Aborting as no valid config files could be found.");
						throwEx<runtime_error>(LOGNAME "Try running setup mode to create a valid config file.");
					}
					else if (mode == -2)
					{
						throwEx<runtime_error>(LOGNAME "Aborting at user request.");
					}
					else
					{
						if (!gOptions.confFile().empty() && !uniFile::fileExists(gOptions.confFile()))
						{
							throwEx<runtime_error>(LOGNAME "Passed config file does not exist (check the file path exists)");
						}
						else
						{
							throwEx<runtime_error>(LOGNAME "No config file passed");
						}
					}
				}
				else if (mode == 1)
				{
					// if these do not match then we need to update the log file being used
					utf8 newLogFile = getLogFile(gOptions.logFile());
					if (newLogFile != oldLogFile)
					{
						uniString::utf8 file = newLogFile.substr(0,gOptions.logFile().rfind(fileUtil::getFilePathDelimiter())).c_str();
						if ((file == newLogFile) || fileUtil::directoryExists(fileUtil::onlyPath(newLogFile)))
						{
							#ifdef _WIN32
							// see if we can create the file, if not then just keep using the temp folder for the log file as we know it's ok
							HANDLE m_file = ::CreateFileW(newLogFile.toWString().c_str(),GENERIC_WRITE,
														  FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
							if (m_file != INVALID_HANDLE_VALUE)
							{
								gOptions.setOption(utf8("reallogfile"), newLogFile);
								forgetHandleInvalid(m_file);
								base.startNormalLog(true);
							}
							// make sure it's showing the correct log file
							else
							{
								// this will fill in the default log path as required
								wchar_t m_fileName[MAX_PATH] = {0};
								ExpandEnvironmentStringsW(DEFAULT_LOGW, m_fileName, MAX_PATH);
								utf8 log(utf32(m_fileName).toUtf8());
								gOptions.setOption(utf8("logfile"), log);
								gOptions.setOption(utf8("reallogfile"), log);
								ILOG(LOGNAME "Logger keeping log file as `" + fileUtil::getFullFilePath(log) + "'");
								ILOG(LOGNAME "Check you have write permissions for the folder(s) set in the config file.");
							}
							#else
							// see if we can create the file, if not then just keep using the temp folder for the log file as we know it's ok
							int m_file = ::open(newLogFile.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
							if (m_file != -1)
							{
								::close(m_file);
								gOptions.setOption(utf8("reallogfile"), newLogFile);
								base.startNormalLog(true);
							}
							// make sure it's showing the correct log file
							else
							{
								// this will fill in the default log path as required
								utf8 log(DEFAULT_LOG);
								gOptions.setOption(utf8("logfile"), log);
								gOptions.setOption(utf8("reallogfile"), log);
								ILOG(LOGNAME "Logger keeping log file as `" + fileUtil::getFullFilePath(log) + "'");
								ILOG(LOGNAME "Check you have write permissions for the folder(s) set in the config file.");
							}
							#endif
						}
						else
						{
							throwEx<runtime_error>(LOGNAME "Log file path does not exist (check the folder path exists)");
						}
					}

					ILOG(LOGNAME "Saving log output to `" + fileUtil::getFullFilePath(gOptions.realLogFile()) + "'");
					ILOG(LOGNAME "Automatic log rotation " + (!gOptions.rotateInterval() ? utf8("disabled") :
						 utf8("interval: ") + timeString(gOptions.rotateInterval())));
					ILOG(LOGNAME "Loaded config from `" + fileUtil::getFullFilePath(gOptions.confFile()) + "'");
				}
				else if (mode == 2)
				{
					if (setupMode)
					{
#ifdef CONFIG_BUILDER
						if (!builderMode)
						{
#endif
							ILOG(LOGNAME "Entering setup mode - Open 127.0.0.1:8000/setup in a");
							ILOG(LOGNAME "browser on the same machine the DNAS was started on");
#ifdef CONFIG_BUILDER
						}
						else
						{
							ILOG(LOGNAME "Entering builder mode - Open 127.0.0.1:8000/builder in");
							ILOG(LOGNAME "a browser on the same machine the DNAS was started on");
						}
#endif
						ILOG(LOGNAME "if the browser does not automatically open the page.");
						ILOG(LOGNAME "If working remotely then replace 127.0.0.1 with the");
						ILOG(LOGNAME "IP / address of the remote system to use this mode.");
					}

					// attempt to open the setup / builder page on the system's browser i.e. kiss option
					#ifdef _WIN32
#ifdef CONFIG_BUILDER
					if (!builderMode)
					{
#endif
						::system("start http://127.0.0.1:8000/setup");
#ifdef CONFIG_BUILDER
					}
					else
					{
						::system("start http://127.0.0.1:8000/builder");
					}
#endif
					#else
#ifdef CONFIG_BUILDER
					if (!builderMode)
					{
#endif
						::system("(if(which xdg-open > /dev/null)then(xdg-open http://127.0.0.1:8000/setup)elif(which gnome-open > /dev/null)then(gnome-open http://127.0.0.1:8000/setup)fi)&>/dev/null");
#ifdef CONFIG_BUILDER
					}
					else
					{
						::system("(if(which xdg-open > /dev/null)then(xdg-open http://127.0.0.1:8000/builder)elif(which gnome-open > /dev/null)then(gnome-open http://127.0.0.1:8000/builder)fi)&>/dev/null");
					}
#endif
					#endif

					#ifdef _WIN32
					win32SocketSetup();
					win32_socket_cleanup_required = true;
					#endif

					threadedRunner *tr = new threadedRunner;
					runners.push_back(tr);
					tr->start();

					threadedRunner::scheduleRunnable(new microServer("", gOptions.portBase(),
													 (microServer::AllowableProtocols_t)(P_WEB_SETUP),
													  microServer::L_MISC));

					s_ControlEvents.push_back(serviceMain::sStop);
					while (!iskilled())
					{
						#if defined(_WIN32) && defined(_WIN64)
						if (WaitForMultipleObjects((DWORD)s_ControlEvents.size(), &(s_ControlEvents[0]), 0, 1000) == WAIT_OBJECT_0)
						#else
						if (WaitForMultipleObjects(s_ControlEvents.size(), &(s_ControlEvents[0]), 0, 1000) == WAIT_OBJECT_0)
						#endif
						{
							// stop signal
							setkill(1);
							break;
						}
					}

					if (iskilled() == 2)
					{
						ILOG(LOGNAME "Stopping setup mode. Preparing for broadcasting...");
					}
					else
					{
						ILOG(LOGNAME "Stopping setup mode. Shutting down...");
					}

					for (vector<threadedRunner*>::const_iterator i = runners.begin(); i != runners.end(); ++i)
					{
						(*i)->stop();
						(*i)->join();
						delete (*i);
					}
					runners.clear();

					#ifdef _WIN32
					if (win32_socket_cleanup_required)
					{
						win32SocketCleanup();
					}
					#endif

					s_ControlEvents.clear();

					// if we get here, we effectively restart the server post-setup
					if (iskilled() == 2)
					{
						#ifdef _WIN32
						gOptions.load(gStartupDirectory + "sc_serv.conf");
						#else
						gOptions.load(gStartupDirectory.hideAsString() + "sc_serv.conf");
						#endif
						setPostSetup(true);
						setkill(0);
						goto runserver;
					}
					setkill(0);
					return mainResult;
				}
			}

			// just make sure that we can do this else we need to abort
			if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
			{
				throwEx<runtime_error>(LOGNAME "Unable to load libcurl & / or its dependencies - cannot continue.");
			}

			config::streams_t streams;
			gOptions.getStreamConfigs(streams);
			// the old sc_serv had two sets of passwords, mimic this
			if (gOptions.adminPassword().empty())
			{
				WLOG(gOptions.logSectionName() + "A dedicated `adminpassword' should be specified in the configuration.");
				WLOG(gOptions.logSectionName() + "Legacy handling has been enabled to map `adminpassword' to `password'.");
				WLOG(gOptions.logSectionName() + "This is not deemed safe and `adminpassword' should be directly set.");
				gOptions.setOption(utf8("adminpassword"), gOptions.password());
			}
			else
			{
				// otherwise if explicitly set as the same then we abort
				if (gOptions.adminPassword() == gOptions.password())
				{
					throwEx<runtime_error>(gOptions.logSectionName() + "You must specify different passwords for `adminpassword' and `password'.");
				}
			}


			// abort no matter what as without passwords we can be at risk
			if (gOptions.adminPassword().empty() && gOptions.password().empty())
			{
				throwEx<runtime_error>(gOptions.logSectionName() + "You must specify a password for `adminpassword' and `password'.");
			}

			if (!streams.empty())
			{
				if (gOptions.setupPasswords(streams))
				{
					// if there was any error on the passwords then we need to abort
					// so it can be fixed. important if using multi-stream hosting!
					throwEx<runtime_error>(gOptions.logSectionName() + "Check the stream configurations above or ensure `adminpassword' and `password' have been set.");
				}
			}
			else
			{
				// if there are no stream configs and no main password
				// then we'll also need to abort as that's not allowed
				if (gOptions.password().empty())
				{
					throwEx<runtime_error>(gOptions.logSectionName() + "You must specify a password for `password'.");
				}
			}


			if (!gOptions.cdn().empty())
			{
				ILOG(LOGNAME "CDN " + utf8(gOptions.cdn() == "on" ? "opt-in" : "opt-out") + " mode enabled -> ensure all stream(s) are properly configured");
			}

			const int cpu_count = gOptions.getCPUCount();	// check options
			ILOG(LOGNAME "Calculated CPU count is " + tos(cpucount()) + " -> " + (cpu_count == cpucount() ? "using " +
				 utf8(cpu_count > 1 ? "all" : "the") + " available CPU" + (cpu_count > 1 ? "s" : "") :
				 tos(cpu_count) + " CPU" + (cpu_count > 1 ? "s" : "") + " specified to be used"));
#ifndef _WIN32
			rlimit rlim = {0};
			if(!getrlimit(RLIMIT_NOFILE, &rlim))
			{
				ILOG(LOGNAME "Limited to " + tos(rlim.rlim_cur) + " file descriptors [relates to ulimit -n]");
			}
#endif
			// calculate defaults for yp
			// if 'publicport' is not the same as 'portbase'
			// then we use it instead of the normal portbase
			g_portForClients = ((gOptions.publicPort() != -1 && (gOptions.publicPort() != gOptions.portBase())) ? gOptions.publicPort() : gOptions.portBase());

			// before anything else starts up we'll start this as needed
			metrics::metrics_apply(gOptions);
			auth::init();

            // load any SSL certificates
            threadedRunner::SSL_init();

			#ifdef _WIN32
			win32SocketSetup();
			win32_socket_cleanup_required = true;
			#endif

			ILOG(LOGNAME "Starting " + tos(cpu_count) + " network thread" + (cpu_count > 1 ? "s" : ""));

			for (int x = 0; x < cpu_count; ++x)
			{
				threadedRunner *tr = new threadedRunner;
				runners.push_back(tr);
				tr->start();
			}

			constructMessageResponses();

			// w3c logging
			if (gOptions.w3cEnable())
			{
				w3cLog::open(gOptions.w3cLog());
			}

			// load geoIP database if able to be found
			/*{
				static utf8 dir = gStartupDirectory;
				char path[1024] = {0};
				GeoIP_setup_custom_directory(strncpy(path, (char*)dir.hideAsString().c_str(), sizeof(path)));
				GeoIP * gi = GeoIP_new(GEOIP_MEMORY_CACHE);
				if(gi != NULL)
				{
					ILOG(LOGNAME "Loaded GeoIP database - Ban and Reserve by country is enabled");
					const char* returnedCountry,
							  * ips[] = {"58.218.199.227","77.76.181.71","87.104.93.85","195.238.117.56",
										 "114.244.60.232","108.52.34.116","91.121.164.186","78.46.75.50",
										 "62.75.139.39","172.17.200.143","205.188.215.228","82.30.80.183"
					};
					for (int ip = 0; ip < sizeof(ips)/sizeof(ips[0]); ip++)
					{
						utf8 details;
						returnedCountry = GeoIP_country_code_by_addr(gi, ips[ip]);
						details = (returnedCountry ? returnedCountry : utf8("UNKNOWN"));
						returnedCountry = GeoIP_country_name_by_addr(gi, ips[ip]);
						details += " (" + (returnedCountry ? returnedCountry : utf8("UNKNOWN")) + ")";
						WLOG(details);
					}

					GeoIP_delete(gi);
				}
			}*/

			// load up stream branding artwork
			gOptions.m_artworkBody[0] = loadLocalFile(fileUtil::getFullFilePath(gOptions.artworkFile()), LOGNAME, 523872/*32 x (16384 - 6 - 6 - 1)*/);

			// load up ban file
			g_banList.load(gOptions.banFile(),0);

			// load up rip file
			g_ripList.load(gOptions.ripFile(),0);

			// load up admin access file
			g_adminList.load(gOptions.adminFile());

			// load up agent file
			g_agentList.load(gOptions.agentFile(),0);

			// per-stream options
			gOptions.getStreamConfigs(streams);
			for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
			{
				// w3c logging
				if (gOptions.read_stream_w3cLog((*i).first))
				{
					w3cLog::open(gOptions.stream_w3cLog((*i).first),(*i).first);
				}

				// load up ban file
				if (gOptions.read_stream_banFile((*i).first))
				{
					g_banList.load(gOptions.stream_banFile((*i).first),(*i).first);
				}

				// load up rip file
				if (gOptions.read_stream_ripFile((*i).first))
				{
					g_ripList.load(gOptions.stream_ripFile((*i).first),(*i).first);
				}

				// load up agent file
				if (gOptions.read_stream_agentFile((*i).first))
				{
					g_agentList.load(gOptions.stream_agentFile((*i).first),(*i).first);
				}

				// load up stream branding artwork
				if (gOptions.read_stream_artworkFile((*i).first))
				{
					gOptions.m_artworkBody[(*i).first] = loadLocalFile(fileUtil::getFullFilePath(gOptions.stream_artworkFile((*i).first)), LOGNAME, 523872/*32 x (16384 - 6 - 6 - 1)*/);
				}
			}

			utf8 srcBindAddr = stripHTTPprefix(stripWhitespace(gOptions.srcIP()));
			utf8 destBindAddr = metrics::metrics_verifyDestIP(gOptions, false);

			if (g_IPAddressForClients.empty())
			{
				char s[MAXHOSTNAMELEN] = {0}; // paranoia
				if (!::gethostname(s, MAXHOSTNAMELEN - 1))
				{
					// changed to not throw (build 43) as this will still run correctly
					// and has caused a number of users to go back to v1 unnecessarily.
					g_IPAddressForClients = socketOps::hostNameToAddress(s, g_portForClients);
				}
			}
			else
			{
				utf8 addr = socketOps::hostNameToAddress(destBindAddr.hideAsString().c_str());
				if (!addr.empty())
				{
					destBindAddr = addr;
				}
				/*hostent *host;
				if ((host = ::gethostbyname((const char *)destBindAddr.c_str())) != NULL){
					destBindAddr = (inet_ntoa(*( (struct in_addr *)host->h_addr)));
				}*/
			}

			if (!srcBindAddr.empty())
			{
				utf8 addr = socketOps::hostNameToAddress(srcBindAddr.hideAsString().c_str());
				if (!addr.empty())
				{
					srcBindAddr = addr;
				}
				/*hostent *host;
				if ((host = ::gethostbyname((const char *)srcBindAddr.c_str())) != NULL){
					srcBindAddr = (inet_ntoa(*( (struct in_addr *)host->h_addr)));
				}*/
			}

			// for legacy sources (with optional portbase override / disable)
			g_legacyPort = ((gOptions.portLegacy() != gOptions.portBase() + 1) && (gOptions.portLegacy() != -1) ?
							gOptions.portLegacy() : gOptions.portBase() + 1);

			// if src and dst are same we configure ourselves a bit differently
			if ((srcBindAddr == destBindAddr) ||
				(inet_addr((const char *)srcBindAddr.c_str()) == inet_addr((const char *)destBindAddr.c_str())))
			{
				// for clients and sources
				threadedRunner::scheduleRunnable(new microServer(srcBindAddr.hideAsString(), gOptions.portBase(),
												 (microServer::AllowableProtocols_t)(P_SHOUTCAST1CLIENT | P_SHOUTCAST2CLIENT |
																					 P_SHOUTCAST1SOURCE | P_SHOUTCAST2SOURCE | P_WEB),
												  (microServer::ListenTypes_t)(microServer::L_SOURCE | microServer::L_CLIENT)));
			}
			else
			{
				// changed in b71 as it's possible to specify a destip which does not correctly bind but the
				// value of destip could be correct and able to work in all other areas e.g. via listen.pls
				// so if there is an issue with the destip specific bind, then attempt a destip==srcip bind
				microServer *r = 0;
				try
				{
					// for clients
					// changed in b79
					r = new microServer(destBindAddr.hideAsString(), gOptions.portBase(),
										(microServer::AllowableProtocols_t)(P_SHOUTCAST1CLIENT | P_SHOUTCAST2CLIENT | P_SHOUTCAST1SOURCE | P_WEB),
										 (microServer::ListenTypes_t)(microServer::L_SOURCE | microServer::L_CLIENT));
					threadedRunner::scheduleRunnable(r);

					// for sources
					threadedRunner::scheduleRunnable(new microServer(srcBindAddr.hideAsString(), gOptions.portBase(),
													 (microServer::AllowableProtocols_t)(P_SHOUTCAST1SOURCE | P_SHOUTCAST2SOURCE | P_WEB),
													  microServer::L_SOURCE));
				}
				catch(const exception &ex)
				{
					if (r == 0)
					{
						WLOG(ex.what());
					}

					try
					{
						// for clients and sources
						threadedRunner::scheduleRunnable(new microServer(srcBindAddr.hideAsString(), gOptions.portBase(),
														 (microServer::AllowableProtocols_t)(P_SHOUTCAST1CLIENT | P_SHOUTCAST2CLIENT |
																							 P_SHOUTCAST1SOURCE | P_SHOUTCAST2SOURCE | P_WEB),
														  (microServer::ListenTypes_t)(microServer::L_SOURCE | microServer::L_CLIENT)));
					}
					catch(const exception &exx)
					{
						// changed in b79
						// if we get to here, if we've been able to bind at least on the destip
						// but the srcip bind fails then attempt to allow v2 sources on destip.
						if (r != 0)
						{
							r->updateProtocols((microServer::AllowableProtocols_t)(P_SHOUTCAST1CLIENT | P_SHOUTCAST2CLIENT |
																				   P_SHOUTCAST1SOURCE | P_SHOUTCAST2SOURCE | P_WEB),
												(microServer::ListenTypes_t)(microServer::L_SOURCE | microServer::L_SOURCE2), gOptions.portBase());
						}
						else
						{
							throwEx<runtime_error>(exx.what());
						}
					}
				}
			}

			if (g_legacyPort > 0)
			{
				// for v1 sources
				threadedRunner::scheduleRunnable(new microServer(srcBindAddr.hideAsString(), g_legacyPort,
																 (microServer::AllowableProtocols_t)(P_SHOUTCAST1SOURCE),
																  microServer::L_SOURCE));
			}
			else
			{
				ILOG("[MICROSERVER] Legacy v1 source support not enabled");
			}

			// for flash policy file server
			if (gOptions.flashPolicyServerPort() != -1)
			{
				threadedRunner::scheduleRunnable(new microServer(srcBindAddr.hideAsString(), gOptions.flashPolicyServerPort(),
																(microServer::AllowableProtocols_t)(P_FLASHPOLICYFILE), microServer::L_FLASH));
			}
			else
			{
				ILOG("[MICROSERVER] Flash policy file server not enabled");
			}

			// and finally look at adding any extra client ports e.g. so you can run on 80
			// and 8000 and anything else for clients that cannot connect to the main port
			if (!gOptions.alternatePorts().empty())
			{
				vector<utf8> tokens = tokenizer(gOptions.alternatePorts(), ',');
				for (size_t tok = 0; tok < tokens.size(); tok++)
				{
					u_short port = (u_short)atoi(tokens[tok].hideAsString().c_str());
					// make sure we're only allowing a valid port number and it isn't already been registered
					if (port && (port != gOptions.portBase()) && (port != g_portForClients))
					{
						microServer *r = 0;
						try
						{
							r = new microServer(srcBindAddr.hideAsString(), port,
												(microServer::AllowableProtocols_t)(P_SHOUTCAST1CLIENT | P_SHOUTCAST2CLIENT),
												(microServer::ListenTypes_t)(microServer::L_CLIENT_ALT));
							threadedRunner::scheduleRunnable(r);
							gOptions.m_usedAlternatePorts += "," + tokens[tok];
						}
						catch(const exception &ex)
						{
							if (r == 0)
							{
								WLOG(ex.what());
								WLOG("[MICROSERVER] Alternate client connections on port " + tokens[tok] + " will not work");
							}
						}
					}
					else
					{
						if (port != gOptions.portBase() && port != g_portForClients)
						{
							WLOG("[MICROSERVER] Skipping `" + tokens[tok] + "' as it is an invalid alternate client port");
						}
						else
						{
							WLOG("[MICROSERVER] Skipping alternate port " + tokens[tok] + " as it is already a used port");
						}
					}
				}
			}

			if (gOptions.pidFile().empty() || (gOptions.pidFile() == "sc_serv_$.pid"))
			{
				pidFn = gStartupDirectory + "sc_serv_" + tos(gOptions.portBase()) + ".pid";
			}
			else if (!gOptions.pidFile().empty())
			{
				pidFn = gOptions.pidFile();
			}

			if (!pidFn.empty())
			{
				FILE *f = uniFile::fopen(pidFn, "wb");
				if (f)
				{
					try 
					{
						utf8 s(tos(getpid()) + eol());
						if (fwrite(s.c_str(),1,s.size(),f) != s.size())
						{
							ELOG(LOGNAME "I/O error writing PID file `" + fileUtil::getFullFilePath(pidFn) + "'");
						}
					}
					catch(...)
					{
						if (f)
						{
							::fclose(f);
						}
						ELOG(LOGNAME "Error writing to PID file `" + fileUtil::getFullFilePath(pidFn) + "'");
					}
					if (f)
					{
						::fclose(f);
					}
				}
				else
				{
					ELOG(LOGNAME "Could not open PID file `" + fileUtil::getFullFilePath(pidFn) +
						 "' for writing (" + errMessage().hideAsString() + ")");
				}
			}

			// schedule relays
			if (!gOptions.startInactive())
			{
				vector<config::streamConfig> relayList(gOptions.getRelayList());
				if (!relayList.empty())
				{
					for_each(relayList.begin(), relayList.end(), scheduleRelay);
				}
			}

			//threadedRunner::scheduleRunnable(new yp2);
			threadedRunner::scheduleRunnable(new updater);

			s_ControlEvents.push_back(serviceMain::sStop);
#ifndef _WIN32
			s_ControlEvents.push_back(serviceMain::sWINCH);
			s_ControlEvents.push_back(serviceMain::sHUP);
			s_ControlEvents.push_back(serviceMain::sUSR1);
			s_ControlEvents.push_back(serviceMain::sUSR2);
#endif

#ifdef OPEN_PORT_CHECKER
			//BOOL CheckPortTCP(short int dwPort , char*ipAddressStr)  
			{  
				struct sockaddr_in client;
				int sock;

				client.sin_family = AF_INET;
				client.sin_port = htons(gOptions.portBase()-1000);
				//client.sin_addr.s_addr = inet_addr(g_IPAddressForClients/*destBindAddr*/.hideAsString().c_str()/*"127.0.0.1");
				client.sin_addr.s_addr = inet_addr("192.168.2.3"/*"127.0.0.1"*/);

				sock = (int) socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				ELOG(g_IPAddressForClients + ":" + tos(gOptions.portBase()) + /*destBindAddr*/ + " " + tos(connect(sock, (struct sockaddr *) &client,sizeof(client)) == 0));
			}
#endif

			while (!iskilled())
			{
				metrics::metrics_wakeup();

				#if defined(_WIN32) && defined(_WIN64)
				int waitResult = WaitForMultipleObjects((DWORD)s_ControlEvents.size(),&(s_ControlEvents[0]),0,1000);
				#else
				int waitResult = WaitForMultipleObjects(s_ControlEvents.size(),&(s_ControlEvents[0]),0,1000);
				#endif
				switch (waitResult)
				{
					// stop signal
					case WAIT_OBJECT_0:
					{
						setkill(true);
						// remove the pid file on successful exit
						if (!pidFn.empty() && uniFile::fileExists(pidFn))
						{
							uniFile::unlink(pidFn);
						}
						break;
					}
#ifdef _WIN32
					case WAIT_OBJECT_0+1: // admin server
					break;

					case WAIT_OBJECT_0+2: // main msg loop
					break;
#else
					case WAIT_OBJECT_0+1: // winch
					{
						// reserved, banned and blocked user agent list(s) reload
						reloadBanLists();
						reloadRipLists();
						reloadAdminAccessList();
						reloadAgentLists();
						break;
					}
					case WAIT_OBJECT_0+2: // hup
					{
						ILOG(LOGNAME "Rotating log and W3C files [PID: " + tos(getpid()) + "]");
						ROTATE;
						printUpdateMessage();
						rotatew3cFiles("w3c");
						ILOG(LOGNAME "Rotated log and W3C files [PID: " + tos(getpid()) + "]");
						break;
					}
					case WAIT_OBJECT_0+3: // usr1
					{
						// config reload
						reloadConfig(false);
						break;
					}
					case WAIT_OBJECT_0+4: // usr2
					{
						// forced config reload
						reloadConfig(true);
						break;
					}
					case WAIT_OBJECT_0+5: // admin server
					break;

					case WAIT_OBJECT_0+6: // main msg loop
					break;
#endif
					default:
					{
					}
				}
			}

			ILOG(LOGNAME "Exiting threads...");
			if (gOptions.configRewrite())
			{
				gOptions.rewriteConfigurationFile((gOptions.configRewrite() == 1), true);
			}
		}
		catch(const exception &ex)
		{
			ELOG(ex.what());
			mainResult = -2;
		}
		catch(...)
		{
			ELOG(LOGNAME "Unknown exception caught");
			mainResult = -1;
		}

		if (stats::getTotalUniqueListeners() > 0)
		{
			ILOG(LOGNAME "Terminating listeners...");
			stats::kickAllClients(0, true);
		}

		// Stop all sources and wait up to ten seconds for everything to clear out
		const streamData::streamIDs_t streamIds = streamData::getStreamIds(true);
		if (!streamIds.empty())
		{
			ILOG(LOGNAME "Terminating sources...");
			for (streamData::streamIDs_t::const_iterator i = streamIds.begin(); i != streamIds.end(); ++i)
			{
				// kick source off system
				streamData::killStreamSource((*i));
			}
		}

		if (yp2::requestsInQueue())
		{
			ILOG(LOGNAME "Running Directory cleanup...");
			// now wait for YP to clear out
			for (int x = 0; (x < 100); ++x)
			{
				if (yp2::requestsInQueue())
				{
					safe_sleep(0, 10000);
				}
				else
				{
					break;
				}
			}
		}

		for (vector<threadedRunner*>::const_iterator i = runners.begin(); i != runners.end(); ++i)
		{
			(*i)->stop();
			(*i)->join();
			delete (*i);
		}

		runners.clear();
		metrics::metrics_wakeup(true);
		metrics::metrics_stop();
		auth::cleanup();

		// general files to save
		if (gOptions.saveBanListOnExit())
		{
			g_banList.save(gOptions.banFile(),0);
		}

		if (gOptions.saveRipListOnExit())
		{
			g_ripList.save(gOptions.ripFile(),0);
		}

		if (gOptions.saveAgentListOnExit())
		{
			g_agentList.save(gOptions.agentFile(),0);
		}

		if (gOptions.w3cEnable())
		{
			w3cLog::close(0);
		}

		// per-stream files to save
		config::streams_t streams;
		gOptions.getStreamConfigs(streams);
		for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
		{
			if (gOptions.saveBanListOnExit())
			{
				if (gOptions.read_stream_banFile((*i).first) && !gOptions.stream_banFile((*i).first).empty())
				{
					g_banList.save(gOptions.stream_banFile((*i).first),(*i).first);
				}
			}

			if (gOptions.saveRipListOnExit())
			{
				if (gOptions.read_stream_ripFile((*i).first) && !gOptions.stream_ripFile((*i).first).empty())
				{
					g_ripList.save(gOptions.stream_ripFile((*i).first),(*i).first);
				}
			}

			if (gOptions.saveAgentListOnExit())
			{
				if (gOptions.read_stream_agentFile((*i).first) && !gOptions.stream_agentFile((*i).first).empty())
				{
					g_agentList.save(gOptions.stream_agentFile((*i).first),(*i).first);
				}
			}

			if (gOptions.read_stream_w3cLog((*i).first))
			{
				w3cLog::close((*i).first);
			}
		}

		#ifdef _WIN32
		if (win32_socket_cleanup_required)
		{
			win32SocketCleanup();
		}
		#endif

		s_ControlEvents.clear();
		DeleteAllCaches();

		stats::getFinalStats();
		bandWidth::getFinalAmounts();
		utf8 t = timeString(::time(NULL) - g_upTime);
		ILOG(LOGNAME + (t.empty() ? "Shutdown" : "Shutdown after " + t + " running") + eol());

		curl_global_cleanup();

/*#ifdef _WIN32
		{
			wchar_t buf[MAX_PATH] = L"\"";
			STARTUPINFO si = {sizeof(si), };
			PROCESS_INFORMATION pi;
			GetModuleFileName(NULL, buf + 1, sizeof(buf) - 1);
			wcsncat(buf, L"\"", MAX_PATH);
			CreateProcess(NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		}
#else
#endif*/

		// if we're in 'user' mode then if there was an exception
		// we'll not immediately abort and instead keep things on
		// display so it's easier for noobs, etc to see the error
		if (!sDaemon && gOptions.screenLog() && (mainResult < 0))
		{
			ILOG(LOGNAME "Press any key to continue . . .");
			while(!_kbhit()) {}
			#ifdef _WIN32
			// do this so we consume the input which
			// as we already have a custom _kbhit()
			// on non-Windows already does it for us
			_getch();
			#endif
		}
		return mainResult;
	}
};

// create the appropriate handler for the app/daemon/service framework
int sm_main(const vector<utf8> &args) throw()
{
#ifdef _WIN32
	stdServiceWin32<sc_serv2_service> s("Shoutcast DNAS");
#else
	stdServiceUnix<sc_serv2_service> s;
#endif
	return s.sm_main(args);
}
