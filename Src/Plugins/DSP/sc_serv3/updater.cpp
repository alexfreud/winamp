#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "updater.h"
#include "file/fileUtils.h"
#include "stl/stringUtils.h"
#include "aolxml/aolxml.h"
#include "bandwidth.h"
#include "services/stdServiceImpl.h"
#include "./versions.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

static AOL_namespace::mutex g_UpdaterLock;
static updater *g_Updater = 0;

#define DEBUG_LOG(...)  do { if (gOptions.yp2Debug()) DLOG(__VA_ARGS__); } while (0)

///////////////////////////////////////////////////////////////////////////////////////////

void updater::updaterBandWidthSent(webClient::request r) throw()
{
	// this effectively works out the size in the same manner
	// as webclient::toRequest(..) does to build the request.
	size_t total = r.m_content.size() + r.m_contentType.size() +
				   // 'POST' or 'GET' with '?' on the front
				   (r.m_method == webClient::request::POST ? 4 : 3) +
				   64 + r.m_addr.size() + g_userAgent.size() +
				   (!r.m_content.empty() ? r.m_content.size() : 0);

	for (httpHeaderMap_t::const_iterator i = r.m_queryVariables.begin(); i != r.m_queryVariables.end(); ++i)
	{
		if (i != r.m_queryVariables.begin())
		{
			++total;
		}
		total += urlUtils::escapeURI_RFC3986((*i).first).size();
		if (!(*i).second.empty())
		{
			total += 1 + urlUtils::escapeURI_RFC3986((*i).second).size();
		}
	}

	if (!r.m_XFF.empty())
	{
		total += 18 + r.m_XFF.size();
	}

	if ((r.m_method == webClient::request::POST) && r.m_content.empty())
	{
		total += 48;
	}
	else if (!r.m_contentType.empty())
	{
		total += 15 + r.m_contentType.size();
	}

	bandWidth::updateAmount(bandWidth::PRIVATE_WEB, total);
}

void updater::updaterBandWidthReceived(const response r) throw()
{
	size_t total = r.m_body.size();

	for (httpHeaderMap_t::const_iterator i = r.m_headers.begin(); i != r.m_headers.end(); ++i)
	{
		total += (*i).first.size() + 1 + (*i).second.size() + eol().size();
	}

	bandWidth::updateAmount(bandWidth::PRIVATE_WEB, total);
}

size_t updater::requestsInQueue() throw()
{
	stackLock sml(g_UpdaterLock);

	return (g_Updater ? g_Updater->queueEntries() : 0);
}

updater::updater() throw() : webClient(UPDATER_LOGNAME)
{
	stackLock sml(g_UpdaterLock);

	g_Updater = this;
	m_running = false;
}

updater::~updater() throw() 
{
	stackLock sml(g_UpdaterLock);

	g_Updater = 0;
}

/////////// response handling //////////////
// handle response. retry_exceptions do just that. otherwise retry occurs in yptimeout seconds

void updater::response_updater(const request & /*q*/, const response &r) throw(exception)
{
	FILE *f = uniFile::fopen(g_Updater->m_verInfo.fn, "wb");
	if (f)
	{
		if (fwrite(&(r.m_body[0]),1,r.m_body.size(),f) == r.m_body.size())
		{
			ILOG(UPDATER_LOGNAME "Downloaded update to `" + g_Updater->m_verInfo.fn + "' [" + tos(r.m_body.size()) + " bytes]");
			g_Updater->m_verInfo.downloaded = 1;
			g_Updater->m_verInfo.needsUpdating = 0;
		}
		else
		{
			ILOG(UPDATER_LOGNAME "Error saving the update to `" + g_Updater->m_verInfo.fn + "' [" + tos(r.m_body.size()) + " bytes]");
			g_Updater->m_verInfo.fn.clear();
			g_Updater->m_verInfo.needsUpdating = 1;
		}
		::fclose(f);
	}
	else
	{
		FILE *f = uniFile::fopen(g_Updater->m_verInfo.fn_alt, "wb");
		if (f)
		{
			if (fwrite(&(r.m_body[0]),1,r.m_body.size(),f) == r.m_body.size())
			{
				g_Updater->m_verInfo.fn = g_Updater->m_verInfo.fn_alt;
				ILOG(UPDATER_LOGNAME "Downloaded update to `" + g_Updater->m_verInfo.fn + "' [" + tos(r.m_body.size()) + " bytes]");
				g_Updater->m_verInfo.downloaded = 1;
				g_Updater->m_verInfo.needsUpdating = 0;
			}
			else
			{
				ILOG(UPDATER_LOGNAME "Error saving the update to `" + g_Updater->m_verInfo.fn_alt + "' [" + tos(r.m_body.size()) + " bytes]");
				g_Updater->m_verInfo.fn.clear();
				g_Updater->m_verInfo.fn_alt.clear();
				g_Updater->m_verInfo.needsUpdating = 1;
			}
			::fclose(f);
		}
		else
		{
			ELOG(UPDATER_LOGNAME "Error creating file: " + errMessage());
			g_Updater->m_verInfo.fn.clear();
			g_Updater->m_verInfo.fn_alt.clear();
			g_Updater->m_verInfo.needsUpdating = 1;
		}
	}
}

void updater::gotResponse(const request &q, const response &r) throw(exception)
{
	stackLock sml(m_serverMapLock);

	DEBUG_LOG(UPDATER_LOGNAME + string(__FUNCTION__) + eol() +
			  "Response body=[" + eol() + utf8(r.m_body.begin(),r.m_body.end()) + "]" +
			  eol() + "Response code=[" + tos(r.m_resultCode) + "]");

	updaterBandWidthReceived(r);
	response_updater(q, r);
	m_running = false;
}

void updater::gotFailure(const request &/*q*/) throw(std::exception)
{
	stackLock sml(g_UpdaterLock);

	DEBUG_LOG(UPDATER_LOGNAME + string(__FUNCTION__));
	m_running = false;
}

void updater::updateVersion() throw()
{
	if (g_Updater)
	{
		// make sure we've got it and also an url
		if (g_Updater->m_verInfo.needsUpdating && !g_Updater->m_verInfo.url.empty() &&
			!g_Updater->m_verInfo.fn.empty() && !g_Updater->m_verInfo.fn_alt.empty())
		{
			try
			{
				g_Updater->pvt_downloadUpdate();
			}
			catch(...)
			{
			}
		}
	}
}

void updater::pvt_downloadUpdate() throw(exception)
{
	if (!m_running)
	{
		m_running = true;
		ILOG(UPDATER_LOGNAME "Preparing to download update package...");

		// build request
		webClient::request r;
		r.m_method = webClient::request::GET;

		config::streamConfig::urlObj downloadUrl(g_Updater->m_verInfo.url.hideAsString());

		r.m_addr = downloadUrl.server();
		r.m_port = downloadUrl.port();
		r.m_path = downloadUrl.path();
		r.m_nonBlocking = 1;

		updaterBandWidthSent(r);
		queueRequest(r);
	}
}

bool updater::getNewVersion(verInfo &ver) throw()
{
	stackLock sml(g_UpdaterLock);

	if (g_Updater)
	{
		// double-check that we've got things correctly
		// before we provide the current update status.
		bool ret = g_Updater->setNewVersion(g_Updater->m_verInfo, true);
		ver = g_Updater->m_verInfo;
		return ret;
 	}
	return false;
}

bool updater::setNewVersion(verInfo &ver, bool no_lock) throw()
{
	if (!no_lock)
	{
		stackLock sml(g_UpdaterLock);
	}

	if (g_Updater)
	{
		if (g_Updater->m_verInfo.ver != ver.ver ||
			g_Updater->m_verInfo.url != ver.url ||
			g_Updater->m_verInfo.log != ver.log)
		{
			g_Updater->m_verInfo = ver;

			if (!g_Updater->m_verInfo.ver.empty())
			{
				const std::vector<uniString::utf8> newVerStr = tokenizer(g_Updater->m_verInfo.ver,'.'),
												   curVerStr = tokenizer(gOptions.getVersionBuildStrings(),'.');
				int newVer[] = {newVerStr[0].toInt(), newVerStr[1].toInt(), newVerStr[2].toInt(), newVerStr[3].toInt()},
					curVer[] = {curVerStr[0].toInt(), curVerStr[1].toInt(), curVerStr[2].toInt(), curVerStr[3].toInt()};

				g_Updater->m_verInfo.needsUpdating = 0;

				// look to compare from major to minor parts of the version strings
				// 2.x.x.x vs 3.x.x.x
				if (newVer[0] > curVer[0])
				{
					g_Updater->m_verInfo.needsUpdating = 1;
				}
				// 2.0.x.x vs 2.2.x.x
				else if ((newVer[0] == curVer[0]) && (newVer[1] > curVer[1]))
				{
					g_Updater->m_verInfo.needsUpdating = 1;
				}
				// 2.0.0.x vs 2.0.1.x
				else if ((newVer[0] == curVer[0]) && (newVer[1] == curVer[1]) && (newVer[2] > curVer[2]))
				{
					g_Updater->m_verInfo.needsUpdating = 1;
				}
				// 2.0.0.29 vs 2.0.0.30
				else if ((newVer[0] == curVer[0]) && (newVer[1] == curVer[1]) && (newVer[2] == curVer[2]) && (newVer[3] > curVer[3]))
				{
					g_Updater->m_verInfo.needsUpdating = 1;
				}

				if (g_Updater->m_verInfo.needsUpdating)
				{
					#ifdef _WIN32
					g_Updater->m_verInfo.fn = gStartupDirectory + "sc_serv_update_" SERV_UPDATE_NAME "_" + tos(newVer[0]) + "_" + tos(newVer[1]) + "_" + tos(newVer[2]) + "_" + tos(newVer[3]) + ".exe";
					wchar_t m_fileName[MAX_PATH] = {0};
					uniString::utf32 u32("%temp%\\sc_serv_update_" SERV_UPDATE_NAME "_" + tos(newVer[0]) + "_" + tos(newVer[1]) + "_" + tos(newVer[2]) + "_" + tos(newVer[3]) + ".exe");
					std::wstring u16;
					u32.toUtf16(u16);
					ExpandEnvironmentStringsW(u16.c_str(), m_fileName, MAX_PATH);
					g_Updater->m_verInfo.fn_alt = utf32(m_fileName).toUtf8();
					#else
					g_Updater->m_verInfo.fn = gStartupDirectory + "sc_serv_update_" SERV_UPDATE_NAME"_" + tos(newVer[0]) + "_" + tos(newVer[1]) + "_" + tos(newVer[2]) + "_" + tos(newVer[3]) + ".tar.gz";
					g_Updater->m_verInfo.fn_alt = "/tmp/sc_serv_update_" SERV_UPDATE_NAME"_" + tos(newVer[0]) + "_" + tos(newVer[1]) + "_" + tos(newVer[2]) + "_" + tos(newVer[3]) + ".tar.gz";
					#endif

					g_Updater->m_verInfo.downloaded = uniFile::fileExists(g_Updater->m_verInfo.fn);
					// if the main file cannot be found, look for the alternate file
					if (!g_Updater->m_verInfo.downloaded)
					{
						g_Updater->m_verInfo.downloaded = uniFile::fileExists(g_Updater->m_verInfo.fn_alt);
						if (g_Updater->m_verInfo.downloaded)
						{
							g_Updater->m_verInfo.fn = g_Updater->m_verInfo.fn_alt;
						}
					}
				}

				ULOG(string(YP2_LOGNAME) + "A new DNAS version is now available: " + g_Updater->m_verInfo.ver);
				ULOG(string(YP2_LOGNAME) + "The suggested download for your setup is: " + g_Updater->m_verInfo.url);
				ULOG(string(YP2_LOGNAME) + "See " + g_Updater->m_verInfo.log + " for more information about this update and alternative download links");

				if (!g_Updater->m_verInfo.downloaded && !g_Updater->m_verInfo.fn.empty() && !g_Updater->m_verInfo.fn_alt.empty())
				{
					g_Updater->updateVersion();
				}
			}
		}
		else
		{
			std::vector<uniString::utf8> newVerStr = tokenizer(g_Updater->m_verInfo.ver,'.');
			if (newVerStr.size() == 4)
			{
				int newVer[] = {newVerStr[0].toInt(), newVerStr[1].toInt(), newVerStr[2].toInt(), newVerStr[3].toInt()};

				#ifdef _WIN32
				g_Updater->m_verInfo.fn = gStartupDirectory + "sc_serv_update_" SERV_UPDATE_NAME "_" + tos(newVer[0]) + "_" + tos(newVer[1]) + "_" + tos(newVer[2]) + "_" + tos(newVer[3]) + ".exe";
				wchar_t m_fileName[MAX_PATH] = {0};
				uniString::utf32 u32("%temp%\\sc_serv_update_" SERV_UPDATE_NAME "_" + tos(newVer[0]) + "_" + tos(newVer[1]) + "_" + tos(newVer[2]) + "_" + tos(newVer[3]) + ".exe");
				std::wstring u16;
				u32.toUtf16(u16);
				ExpandEnvironmentStringsW(u16.c_str(), m_fileName, MAX_PATH);
				g_Updater->m_verInfo.fn_alt = utf32(m_fileName).toUtf8();
				#else
				g_Updater->m_verInfo.fn = gStartupDirectory + "sc_serv_update_" SERV_UPDATE_NAME"_" + tos(newVer[0]) + "_" + tos(newVer[1]) + "_" + tos(newVer[2]) + "_" + tos(newVer[3]) + ".tar.gz";
				g_Updater->m_verInfo.fn_alt = "/tmp/sc_serv_update_" SERV_UPDATE_NAME"_" + tos(newVer[0]) + "_" + tos(newVer[1]) + "_" + tos(newVer[2]) + "_" + tos(newVer[3]) + ".tar.gz";
				#endif

				g_Updater->m_verInfo.downloaded = uniFile::fileExists(g_Updater->m_verInfo.fn);
				// if the main file cannot be found, look for the alternate file
				if (!g_Updater->m_verInfo.downloaded)
				{
					g_Updater->m_verInfo.downloaded = uniFile::fileExists(g_Updater->m_verInfo.fn_alt);
					if (g_Updater->m_verInfo.downloaded)
					{
						g_Updater->m_verInfo.fn = g_Updater->m_verInfo.fn_alt;
					}
				}
				g_Updater->m_verInfo.needsUpdating = 1;

				if (!g_Updater->m_verInfo.downloaded && !g_Updater->m_verInfo.fn.empty())
				{
					g_Updater->updateVersion();
				}
			}
		}

		return (!g_Updater->m_verInfo.ver.empty() && (g_Updater->m_verInfo.needsUpdating || (g_Updater->m_verInfo.downloaded && !g_Updater->m_verInfo.fn.empty())));
	}
	return false;
}
