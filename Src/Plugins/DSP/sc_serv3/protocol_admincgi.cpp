#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_shoutcastClient.h"
#include "protocol_admincgi.h"
#include "protocol_HTTPStyle.h"
#include "protocol_relay.h"
#include "base64.h"
#include "banList.h"
#include "ripList.h"
#include "adminList.h"
#include "agentList.h"
#include "uvox2Common.h"
#include "w3cLog.h"
#include "yp2.h"
#include "updater.h"
#include "aolxml/aolxml.h"
#include "webNet/urlUtils.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"
#include "bandwidth.h"
#include "cpucount.h"

using namespace std;
using namespace stringUtil;
using namespace uniString;

time_t last_update_check = 0;
utf8 logId, logTailId, listenerId;

#define DEBUG_LOG(...)      do { if (gOptions.httpStyleDebug()) DLOG(__VA_ARGS__); } while (0)
#define LOG_NAME "ADMINCGI"
#define LOGNAME "[" LOG_NAME "] "

#define HEAD_REQUEST (m_httpRequestInfo.m_request == protocol_HTTPStyle::HTTP_HEAD)
#define SHRINK (m_httpRequestInfo.m_AcceptEncoding & protocol_HTTPStyle::ACCEPT_GZIP)
#define COMPRESS(header, body) if (SHRINK && compressData(body)) header += "Content-Encoding:gzip\r\n"

static bool sortUniqueClientDataByTime(const stats::uniqueClientData_t &a, const stats::uniqueClientData_t &b)
{
	return (a.m_connectTime < b.m_connectTime);
}

utf8 getStreamAdminHeader(const streamData::streamID_t sid, const utf8& headerTitle,
						  const int refreshRequired = 0, const bool style = false)
{
	return "<!DOCTYPE html><html><head>"
		   "<meta charset=\"utf-8\">"
		   "<meta name=viewport content=\"width=device-width, initial-scale=1\">"
		   "<title>Shoutcast Administrator</title>"
		   "<link href=\"index.css\" rel=\"stylesheet\" type=\"text/css\">"
		   "<link href=\"images/favicon.ico\" rel=\"shortcut icon\" type=\"" +
		   gOptions.faviconFileMimeType() + "\">" + (abs(refreshRequired) > 0 ?
		   "<meta http-equiv=\"refresh\"content=\"3; url=admin.cgi?sid=" + tos(sid) + "\">" : "") +

		   (style ? "<style type=\"text/css\">"
					"li img{vertical-align:bottom;}li{padding-bottom:0.5em;}"
					"</style>" : (utf8)"") +

		   "</head><body style=\"margin:0;\">"
		   "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr>"
		   "<td><div class=\"logo\">Shoutcast " + headerTitle + "</div></td>"
		   "<td style=\"text-align:right;vertical-align:bottom;padding-right:0.1em;\">"
		   "<div id=\"up\"></div><a target=\"_blank\" title=\"Built: " __DATE__"\" "
		   "href=\"http://www.shoutcast.com\">Shoutcast Server v" +
		   addWBR(gOptions.getVersionBuildStrings() + "/" SERV_OSNAME) + "</a></td>"
		   "</tr><tr><td class=\"thr\" align=\"center\" colspan=\"3\"><table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"
		   "<tr><td class=\"thr\" align=\"center\"><div id=\"hdrbox\" class=\"tnl\" "
		   "style=\"justify-content:space-around;display:flex;flex-flow:row wrap;\">"
		   "<div class=\"thr\"><a href=\"admin.cgi?sid=" + tos(sid) + "\">Status &amp; Listeners</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=history\">History&nbsp;"
		   "<img border=\"0\" title=\"History\" alt=\"History\" style=\"vertical-align:middle\" src=\"images/history.png\"></a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   + (!(gOptions.read_stream_adminPassword(sid) && !gOptions.stream_adminPassword(sid).empty()) ?
						"<div class=\"thr\"><a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=viewlog\">Log</a> "
						"(<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=viewlog&amp;viewlog=tail\">Tailing</a>"
						" | <a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=viewlog&amp;viewlog=save\">Save</a>)</div>"
						"<div class=\"thr\">&nbsp;|&nbsp;</div>" : "") +
		   /*+ utf8(info.m_radionomyID.empty() ? warningImage(false) + "&nbsp;&nbsp;<b>Please Register Your Authhash</b><br><br>" : "") +*/
		   "<div class=\"thr\"><a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Authhash</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=viewban\">Ban List</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=viewrip\">Reserved List</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=viewagent\">User Agent List</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"index.html?sid=" + tos(sid) + "\">Stream Logout</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi\">Server Login&nbsp;"
		   "<img border=\"0\" title=\"Server Login\nPassword Required\" alt=\"Server Login\nPassword Required"
		   "\" style=\"vertical-align:middle\" src=\"images/lock.png\"></a></div>"
		   "</div></td></tr></table></td></tr></table>";
}

utf8 getServerAdminHeader(const utf8& headerTitle, const int refreshRequired = 0,
						  const utf8& childPage = "", const int style = 0)
{
	return "<!DOCTYPE html><html><head>"
		   "<meta charset=\"utf-8\">"
		   "<meta name=viewport content=\"width=device-width, initial-scale=1\">"
		   "<title>Shoutcast Server Administrator</title>"
		   "<link href=\"index.css\" rel=\"stylesheet\" type=\"text/css\">"
		   "<link href=\"images/favicon.ico\" rel=\"shortcut icon\" type=\"" +
		   gOptions.faviconFileMimeType() + "\">" +

		   (abs(refreshRequired) > 0 ? "<meta http-equiv=\"refresh\"content=\"" +
		   tos(abs(refreshRequired)) + "; url=admin.cgi?sid=0" + childPage + "\">" : "") +

		   (style ? "<style type=\"text/css\">" +
		   (style == 1 ? ".s,.t,.st{border-style:solid;border-color:#CCCCCC;padding:0.2em 1em;text-align:center;}"
						 ".s,.t,.st{border-width:1px;}"
						 /* this fixes a FF quirk with some of the edges being hidden*/
						 ".infh{position:static;}" :
		   (style == 2 ? "li img{vertical-align:bottom;}li{padding-bottom:0.5em;}" : (utf8)"")) + "</style>" : (utf8)"") +

		   "</head><body style=\"margin:0;\">"
		   "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr>"
		   "<td><div class=\"logo\">Shoutcast " + headerTitle + "</div></td>"
		   "<td style=\"text-align:right;vertical-align:bottom;padding-right:0.1em;\">"
		   "<div id=\"up\"></div><a target=\"_blank\" title=\"Built: " __DATE__"\" "
		   "href=\"http://www.shoutcast.com\">Shoutcast Server v" +
		   addWBR(gOptions.getVersionBuildStrings() + "/" SERV_OSNAME) + "</a></td></tr>"
		   "<tr><td class=\"thr\" align=\"center\" colspan=\"3\">"
		   "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"
		   "<tr><td class=\"thr\" align=\"center\"><div id=\"hdrbox\" class=\"tnl\" "
		   "style=\"justify-content:space-around;display:flex;flex-flow:row wrap;\">"
		   "<div class=\"thr\"><a href=\"admin.cgi?mode=help\">Help &amp; Documentation</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?mode=bandwidth\">Bandwidth Usage</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?mode=viewlog&amp;server=" + randomId(logId) + "\">Log</a> "
		   "(<a href=\"admin.cgi?mode=viewlog&amp;server=" + logId + "&amp;viewlog=tail\">Tailing</a> | "
		   "<a href=\"admin.cgi?mode=viewlog&amp;server=" + logId + "&amp;viewlog=save\">Save</a>)</div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi\">Summary</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?mode=viewban\">Ban List</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?mode=viewrip\">Reserved List</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?mode=viewagent\">User Agent List</a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"index.html\">Server Logout&nbsp;</a></div>"
		   "</div></td></tr></table></td></tr></table>";
}

static utf8 formatSizeString(const __uint64 size)
{
	utf8::value_type buf[128] = {0};
	if (size < 1024)
	{
		snprintf((char *)buf, sizeof(buf), "%llu B", size);
	}
	else if(size < 1048576)
	{
		snprintf((char *)buf, sizeof(buf), "%.02f KiB", size/1024.0f);
	}
	else if(size < 1073741824)
	{
		snprintf((char *)buf, sizeof(buf), "%.02f MiB", size/1048576.0f);
	}
	else if(size < 1099511627776LL)
	{
		snprintf((char *)buf, sizeof(buf), "%.02f GiB", size/1073741824.0f);
	}
	else
	{
		snprintf((char *)buf, sizeof(buf), "%.02f TiB", size/1099511627776.0f);
	}
	return buf;
}

utf8 getCheckedDuration(const size_t time)
{
	if (time >= 60)
	{
		if (time < 3600)
		{
			size_t min = (time / 60);
			return (tos(min) + " minute" + (min != 1 ? "s" : "") + " ago");
		}
		else if (time < 86400)
		{
			size_t hour = (time / 3600);
			return (tos(hour) + " hour" + (hour != 1 ? "s" : "") + " ago");
		}
		else
		{
			size_t week = (time / 86400);
			return (tos(week) + " week" + (week != 1 ? "s" : "") + " ago");
		}
	}
	return "less than a minute ago";
}

utf8 niceURL(utf8 srcAddr)
{
	if (!srcAddr.empty())
	{
		utf8::size_type pos = srcAddr.find(utf8("://"));
		if (pos != utf8::npos && ((pos == 4) || (pos == 5)))
		{
			srcAddr = srcAddr.substr(pos + 3);
		}
		srcAddr = aolxml::escapeXML(srcAddr);

		// look for a /stream/x/ path and strip off the end / so the
		// link goes to the admin page instead of playing the stream
		if (!srcAddr.empty())
		{
			utf8::size_type pos2 = srcAddr.find(utf8("/stream/")),
							pos3 = srcAddr.rfind(utf8("/"));
			if ((pos2 != utf8::npos) &&
				((pos3 != utf8::npos) && (pos3 > pos2) &&
				(pos3 == srcAddr.size()-1)))
			{
				srcAddr = srcAddr.substr(0, pos3);
			}
		}
	}
	return srcAddr;
}

void restartRelay(const config::streamConfig &info) throw()
{
	bool noEntry = false;
	const int relayActive = (streamData::isRelayActive(info.m_streamID, noEntry) & 12);
	if (!relayActive || !noEntry)
	{
		threadedRunner::scheduleRunnable(new protocol_relay(info));
	}
}

void checkVersion(const time_t t)
{
	utf8 tempId;
	httpHeaderMap_t queryParameters;
	queryParameters["id"] = randomId(tempId);

	yp2::runAuthHashAction(tempId, yp2::VER_CHECK, "/yp2", queryParameters,
						   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"\
						   "<yp version=\"2\"><cmd op=\"version\" seq=\"1\">"\
						   "<dnas>" + gOptions.getVersionBuildStrings() + "/" SERV_OSNAME "</dnas></cmd></yp>");

	last_update_check = t;
}

utf8 getUptimeScript(const bool base = false, const bool stream = false, const time_t streamUptime = 0)
{
	// TODO need to consider improving this to better deal with leap years, etc
	// used to increment the uptime value on the server admin page
	return (!base ? "<script type=\"text/javascript\">"
			"function $(id){return document.getElementById(id);}" EL : (utf8)"") +
			"function pad(num){return(num<10?'0':'')+num;}" EL
			"var i=" + tos((::time(NULL) - g_upTime)) + ";" EL

			"function time(t,slim){" EL
			"var min=parseInt(t/60);" EL
			"var sec=t-parseInt(min*60);" EL
			"var hours=parseInt(min/60);" EL
			"min-=parseInt(hours*60);" EL

			"var r=\"\";" EL
			"var days=parseInt(hours/24);" EL
			"hours-=parseInt(days*24);" EL
			"var weeks=parseInt(days/7);" EL
			"days-=parseInt(weeks*7);" EL
			"var years=parseInt(weeks/52);" EL
			"weeks-=parseInt(years*52);" EL
			"if(years)r+=years+\" year\"+(years!=1?\"s\":\"\")+\" \";" EL
			"if(weeks)r+=weeks+\" week\"+(weeks!=1?\"s\":\"\")+\" \";" EL
			"if(days)r+=days+\" day\"+(days!=1?\"s\":\"\")+\" \";" EL
			"if(slim){" EL
			"r+=pad(hours)+\":\"+pad(min)+\":\"+pad(sec);" EL
			"}else{" EL
			"if(hours)r+=hours+\" hour\"+(hours!=1?\"s\":\"\")+\" \";" EL
			"if(min)r+=min+\" minute\"+(min!=1?\"s\":\"\")+\" \";" EL
			"if(sec)r+=sec+\" second\"+(sec!=1?\"s\":\"\");" EL
			"}" EL
			"return r;" EL
			"}" EL

			"function count(){i++;$('up').innerHTML=\"" + (sDaemon ?
#ifdef _WIN32
			"Service"
#else
			"Daemon"
#endif
			: "") + " Uptime: \"+(!i?\"Starting&hellip;\":time(i,1));}" EL
			"count();setInterval(count,1000);" EL +

			// used to increment the uptime value on the stream admin pages
			(stream ?
				"var is=" + tos(streamUptime) + ";" EL
				"function counts(){is++;$('up2').innerHTML=\"<b>\"+(!is?\"Starting&hellip;\":time(is,0))+\"<\\/b>\";}" EL
				"counts();setInterval(counts,1000);" EL : "") +

			(!base ? "</script>" : "");
}

const bool reloadConfig(const int force)
{
	bool m_reloadRefresh = false;

	ILOG(gOptions.logSectionName() + "Starting stream config reload from `" + fileUtil::getFullFilePath(gOptions.confFile()) + "'");
	config newOptions;
	newOptions.load(gOptions.confFile());
	vector<config::streamConfig> relays;

	// to ease testing, especially on remote systems, will
	// allow toggling of the debugging options for v2.1+
	ILOG(gOptions.logSectionName() + "Processing global configuration settings...");

	if (newOptions.cdn() != gOptions.cdn())
	{
		ILOG(gOptions.logSectionName() + "Changing CDN mode from " + (!gOptions.cdn().empty() ? gOptions.cdn() : "off") +
										 " to " + (!newOptions.cdn().empty() ? newOptions.cdn() : "off"));
		try
		{
			gOptions.setOption(utf8("cdn"),utf8(newOptions.cdn()));
		}
		catch(const exception &)
		{
		}
	}

	if (newOptions.maxUser() != gOptions.maxUser())
	{
		const int old_maxUser = gOptions.maxUser(),
				  new_maxUser = newOptions.maxUser();
		ILOG(gOptions.logSectionName() + "Changing server maxuser from " +
			 (old_maxUser > 0 ? tos(old_maxUser) : "unlimited") + " to " +
			 (new_maxUser > 0 ? tos(new_maxUser) : "unlimited"));
		gOptions.setOption(utf8("maxuser"),utf8(tos(newOptions.maxUser())));
	}

	if (newOptions.maxBitrate() != gOptions.maxBitrate())
	{
		ILOG(gOptions.logSectionName() + "Changing server maxbitrate from " +
			 (gOptions.maxBitrate() > 0 ? tos(gOptions.maxBitrate()) + "bps" : "unlimited") + " to " +
			 (newOptions.maxBitrate() > 0 ? tos(newOptions.maxBitrate()) + "bps" : "unlimited"));
		gOptions.setOption(utf8("maxbitrate"),utf8(tos(newOptions.maxBitrate())));
	}

	if (newOptions.minBitrate() != gOptions.minBitrate())
	{
		ILOG(gOptions.logSectionName() + "Changing server minbitrate from " +
			 (gOptions.minBitrate() > 0 ? tos(gOptions.minBitrate()) + "bps" : "unlimited") + " to " +
			 (newOptions.minBitrate() > 0 ? tos(newOptions.minBitrate()) + "bps" : "unlimited"));
		gOptions.setOption(utf8("minbitrate"),utf8(tos(newOptions.minBitrate())));
	}

	bool publicChanged = false;
	if (newOptions.publicServer() != gOptions.publicServer())
	{
		ILOG(gOptions.logSectionName() + "Changing state of publicserver - killing sources as applicable");
		gOptions.setOption(utf8("publicserver"),newOptions.publicServer());
		publicChanged = true;
	}

	// update the server-wide hiding and redirection options
	if (newOptions.hideStats() != gOptions.hideStats())
	{
		ILOG(gOptions.logSectionName() + "Changing 'hidestats'");
		gOptions.setOption(utf8("hidestats"),utf8(newOptions.hideStats()));
	}

	if (newOptions.redirectUrl() != gOptions.redirectUrl())
	{
		ILOG(gOptions.logSectionName() + "Changing 'redirecturl'");
		gOptions.setOption(utf8("redirecturl"),utf8(newOptions.redirectUrl()));
	}

	if (newOptions.ripOnly() != gOptions.ripOnly())
	{
		ILOG(gOptions.logSectionName() + "Changing 'riponly'");
		gOptions.setOption(utf8("riponly"),utf8(tos(newOptions.ripOnly())));
	}

	if (newOptions.blockEmptyUserAgent() != gOptions.blockEmptyUserAgent())
	{
		ILOG(gOptions.logSectionName() + "Changing 'blockemptyuseragent'");
		gOptions.setOption(utf8("blockemptyuseragent"),utf8(tos(newOptions.blockEmptyUserAgent())));
	}

	if (newOptions.metricsMaxQueue() != gOptions.metricsMaxQueue())
	{
		ILOG(gOptions.logSectionName() + "Changing 'metricsmaxqueue'");
		gOptions.setOption(utf8("metricsmaxqueue"),utf8(tos(newOptions.metricsMaxQueue())));
	}

	metrics::metrics_apply(newOptions);

	if (newOptions.relayReconnectTime() != gOptions.relayReconnectTime())
	{
		ILOG(gOptions.logSectionName() + "Changing 'relayreconnecttime'");
		gOptions.setOption(utf8("relayreconnecttime"),utf8(tos(newOptions.relayReconnectTime())));
	}

	if (newOptions.relayConnectRetries() != gOptions.relayConnectRetries())
	{
		ILOG(gOptions.logSectionName() + "Changing 'relayconnectretries'");
		gOptions.setOption(utf8("relayconnectretries"),utf8(tos(newOptions.relayConnectRetries())));
	}

	if (newOptions.backupLoop() != gOptions.backupLoop())
	{
		ILOG(gOptions.logSectionName() + "Changing 'backuploop'");
		gOptions.setOption(utf8("backuploop"),utf8(tos(newOptions.backupLoop())));
	}

	if (newOptions.songHistory() != gOptions.songHistory())
	{
		ILOG(gOptions.logSectionName() + "Changing 'songhistory'");
		gOptions.setOption(utf8("songhistory"),utf8(tos(newOptions.songHistory())));
	}

	if (newOptions.adminFile() != gOptions.adminFile())
	{
		ILOG(gOptions.logSectionName() + "Changing 'adminfile'");
		gOptions.setOption(utf8("adminfile"),newOptions.adminFile());
	}

	if (newOptions.clacks() != gOptions.clacks())
	{
		ILOG(gOptions.logSectionName() + "Changing 'clacks'");
		gOptions.setOption(utf8("clacks"),utf8(tos(newOptions.clacks())));
	}

	if (newOptions.startInactive() != gOptions.startInactive())
	{
		ILOG(gOptions.logSectionName() + "Changing 'startinactive'");
		gOptions.setOption(utf8("startinactive"),utf8(tos(newOptions.startInactive())));
	}

	if (newOptions.rateLimit() != gOptions.rateLimit())
	{
		ILOG(gOptions.logSectionName() + "Changing 'rateLimit'");
		gOptions.setOption(utf8("ratelimit"),utf8(tos(newOptions.rateLimit())));
	}

	if (newOptions.adTestFile() != gOptions.adTestFile())
	{
		ILOG(gOptions.logSectionName() + "Changing 'adtestfile'");
		gOptions.setOption(utf8("adtestfile"),utf8(newOptions.adTestFile()));
	}

	if (newOptions.adTestFile2() != gOptions.adTestFile2())
	{
		ILOG(gOptions.logSectionName() + "Changing 'adtestfile2'");
		gOptions.setOption(utf8("adtestfile2"),utf8(newOptions.adTestFile2()));
	}

	if (newOptions.adTestFile3() != gOptions.adTestFile3())
	{
		ILOG(gOptions.logSectionName() + "Changing 'adtestfile3'");
		gOptions.setOption(utf8("adtestfile3"),utf8(newOptions.adTestFile3()));
	}

	if (newOptions.adTestFile4() != gOptions.adTestFile4())
	{
		ILOG(gOptions.logSectionName() + "Changing 'adtestfile4'");
		gOptions.setOption(utf8("adtestfile4"),utf8(newOptions.adTestFile4()));
	}

	if (newOptions.adTestFileLoop() != gOptions.adTestFileLoop())
	{
		ILOG(gOptions.logSectionName() + "Changing 'adtestfileloop'");
		gOptions.setOption(utf8("adtestfileloop"),utf8(tos(newOptions.adTestFileLoop())));
	}

	if (newOptions.metaInterval() != gOptions.metaInterval())
	{
		ILOG(gOptions.logSectionName() + "Changing 'metainterval'");
		gOptions.setOption(utf8("metainterval"),utf8(tos(newOptions.metaInterval())));
	}

	if (newOptions.useXFF() != gOptions.useXFF())
	{
		ILOG(gOptions.logSectionName() + "Changing 'usexff'");
		gOptions.setOption(utf8("usexff"),utf8(tos(newOptions.useXFF())));
	}

	if (newOptions.forceShortSends() != gOptions.forceShortSends())
	{
		ILOG(gOptions.logSectionName() + "Changing 'forceshortsends'");
		gOptions.setOption(utf8("forceshortsends"),utf8(tos(newOptions.forceShortSends())));
	}

	if (newOptions.adminNoWrap() != gOptions.adminNoWrap())
	{
		ILOG(gOptions.logSectionName() + "Changing 'adminnowrap'");
		gOptions.setOption(utf8("adminnowrap"),utf8(tos(newOptions.adminNoWrap())));
	}

	if (newOptions.adminCSSFile() != gOptions.adminCSSFile())
	{
		ILOG(gOptions.logSectionName() + "Changing 'admincssfile'");
		gOptions.setOption(utf8("admincssfile"),utf8(newOptions.adminCSSFile()));
		gOptions.m_styleCustomStr.clear();
		gOptions.m_styleCustomStrGZ.clear();
		gOptions.m_styleCustomHeaderTime = 0;
	}

	if (newOptions.destIP() != gOptions.destIP())
	{
		utf8 destBindAddr = metrics::metrics_verifyDestIP(newOptions, false);
		ILOG(gOptions.logSectionName() + "Changing 'destip'");
		gOptions.setOption(utf8("destip"), destBindAddr);

		// if we're updating then attempt to behave like it was a new load
		g_IPAddressForClients = destBindAddr;

		if (g_IPAddressForClients.empty())
		{
			char s[MAXHOSTNAMELEN] = {0};
			if (!::gethostname(s,MAXHOSTNAMELEN - 1))
			{
				g_IPAddressForClients = socketOps::hostNameToAddress(s,g_portForClients);
			}
		}
	}

	if (newOptions.publicIP() != gOptions.publicIP())
	{
		utf8 publicAddr = stripWhitespace(newOptions.publicIP());
		publicAddr = stripHTTPprefix(publicAddr);
		ILOG(gOptions.logSectionName() + "Changing 'publicip'");
		gOptions.setOption(utf8("publicip"),publicAddr);
	}

	if (newOptions.autoDumpTime() != gOptions.autoDumpTime())
	{
		ILOG(gOptions.logSectionName() + "Changing 'autodumptime'");
		gOptions.setOption(utf8("autodumptime"),utf8(tos(newOptions.autoDumpTime())));
	}

	if (newOptions.nameLookups() != gOptions.nameLookups())
	{
		ILOG(gOptions.logSectionName() + "Changing 'namelookups'");
		gOptions.setOption(utf8("namelookups"),utf8(tos(newOptions.nameLookups())));
	}

	// update the YP details (can do on fly without any actual updates)
	bool ypChanged = false;
	bool https = ((gOptions.ypAddr() == DEFAULT_YP_ADDRESS) && uniFile::fileExists(gOptions.m_certPath));
	utf8 oldYP = (https ? "https://" : "http://") + gOptions.ypAddr() + ":" + tos(gOptions.ypPort()) + gOptions.ypPath();
	if (newOptions.ypAddr() != gOptions.ypAddr())
	{
		ypChanged = true;
		gOptions.setOption(utf8("ypaddr"),utf8(newOptions.ypAddr()));
	}
	if (newOptions.ypPort() != gOptions.ypPort())
	{
		ypChanged = true;
		gOptions.setOption(utf8("ypport"),utf8(tos(newOptions.ypPort())));
	}
	if (newOptions.ypPath() != gOptions.ypPath())
	{
		ypChanged = true;
		gOptions.setOption(utf8("yppath"),utf8(newOptions.ypPath()));
	}

	if (ypChanged)
	{
		bool https = ((newOptions.ypAddr() == DEFAULT_YP_ADDRESS) && uniFile::fileExists(gOptions.m_certPath));
		utf8 newYP = (https ? "https://" : "http://") + newOptions.ypAddr() + ":" + tos(newOptions.ypPort()) + newOptions.ypPath();
		ILOG(gOptions.logSectionName() + "Changing YP details from " + oldYP + " to " + newYP);
	}

	gOptions.setOption(utf8("yp2debug"),utf8(newOptions.yp2Debug()?"1":"0"));
	gOptions.setOption(utf8("shoutcastsourcedebug"),utf8(newOptions.shoutcastSourceDebug()?"1":"0"));
	gOptions.setOption(utf8("uvox2sourcedebug"),utf8(newOptions.uvox2SourceDebug()?"1":"0"));
	gOptions.setOption(utf8("httpsourcedebug"),utf8(newOptions.HTTPSourceDebug()?"1":"0"));
	gOptions.setOption(utf8("shoutcast1clientdebug"),utf8(newOptions.shoutcast1ClientDebug()?"1":"0"));
	gOptions.setOption(utf8("shoutcast2clientdebug"),utf8(newOptions.shoutcast2ClientDebug()?"1":"0"));
	gOptions.setOption(utf8("httpclientdebug"),utf8(newOptions.HTTPClientDebug()?"1":"0"));
	gOptions.setOption(utf8("flvclientdebug"),utf8(newOptions.flvClientDebug()?"1":"0"));
	gOptions.setOption(utf8("m4aclientdebug"),utf8(newOptions.m4aClientDebug()?"1":"0"));
	gOptions.setOption(utf8("relayshoutcastdebug"),utf8(newOptions.relayShoutcastDebug()?"1":"0"));
	gOptions.setOption(utf8("relayuvoxdebug"),utf8(newOptions.relayUvoxDebug()?"1":"0"));
	gOptions.setOption(utf8("relaydebug"),utf8(newOptions.relayDebug()?"1":"0"));
	gOptions.setOption(utf8("streamdatadebug"),utf8(newOptions.streamDataDebug()?"1":"0"));
	gOptions.setOption(utf8("httpstyledebug"),utf8(newOptions.httpStyleDebug()?"1":"0"));
	gOptions.setOption(utf8("statsdebug"),utf8(newOptions.statsDebug()?"1":"0"));
	gOptions.setOption(utf8("microserverdebug"),utf8(newOptions.microServerDebug()?"1":"0"));
	gOptions.setOption(utf8("threadrunnerdebug"),utf8(newOptions.threadRunnerDebug()?"1":"0"));
	gOptions.setOption(utf8("admetricsdebug"),utf8(newOptions.adMetricsDebug()?"1":"0"));
	gOptions.setOption(utf8("authdebug"),utf8(newOptions.authDebug()?"1":"0"));

	ILOG(gOptions.logSectionName() + "Processed global configuration settings.");

	// test for the source password having changed this will be
	// applied in general though per stream changes are likely
	// to have been set in the earlier checks before we got here
	if (newOptions.password() != gOptions.password())
	{
		gOptions.setOption(utf8("password"),newOptions.password());
		streamData *sd = streamData::accessStream(DEFAULT_SOURCE_STREAM);
		if (sd)
		{
			ILOG(gOptions.logSectionName() + "Killing all stream sources due to change of relay options.");
			sd->killAllSources();
			m_reloadRefresh = true;
			sd->releaseStream();
		}
	}

	config::streams_t new_streams, old_streams;
	newOptions.getStreamConfigs(new_streams, false);
	gOptions.getStreamConfigs(old_streams, false);

	config::streams_t::const_iterator iNSC = new_streams.begin(), iOSC = old_streams.begin();

	// if no configurations found then we can just remove everything
	if (new_streams.empty())
	{
		// kick the source and clients as required on a removal
		for (; iOSC != old_streams.end(); ++iOSC)
		{
			size_t streamID = (*iOSC).second.m_streamID;
			streamData *sd = streamData::accessStream(streamID);
			if (sd)
			{
				sd->killSource(streamID, sd);
				m_reloadRefresh = true;
			}
			gOptions.removeStreamConfig((*iOSC).second);
		}
	}

	// otherwise if the same or more then we can update / add as required
	else if (new_streams.size() >= old_streams.size())
	{
		// if no configs specified and we're starting to add new stream configs
		// then we really need to kick any existing sources otherwise it'll stay
		// with the current details which will prevent a YP connection with extra
		// checks in build 21+ to make sure it only works if there was a change.
		if ((new_streams.size() != old_streams.size()) &&
		    (old_streams.size() == DEFAULT_SOURCE_STREAM))
		{
			// kick the source and clients as required on a complete addition
			// (wouldn't have a valid config via authhash, etc so is sensible)
			streamData *sd = streamData::accessStream(DEFAULT_SOURCE_STREAM);
			if (sd)
			{
				ILOG(gOptions.logSectionName() + "Forcing stream source disconnect due to addition of stream config(s).");
				sd->killAllSources();
				m_reloadRefresh = true;
				sd->releaseStream();
			}
		}

		for (; iNSC != new_streams.end(); ++iNSC)
		{
			config::streams_t::const_iterator iOSC2 = old_streams.find((*iNSC).first);
			if (iOSC2 != old_streams.end())
			{
				// update required
				__uint64 updated = gOptions.updateStreamConfig(newOptions, (*iNSC).second);
				size_t streamID = (*iNSC).second.m_streamID;
				streamData *sd = streamData::accessStream(streamID);
				if (sd)
				{
					// otherwise do an in-place update as long as it is applicable
					bool isRelay = sd->isRelayStream(streamID);
					if (publicChanged || force ||
						(!force && (updated & RELAY_URL || updated & SOURCE_PWD ||
						 updated & PUBLIC_SRV || (updated & ALLOW_RELAY && isRelay) ||
						 (updated & ALLOW_PUBLIC_RELAY && isRelay) || updated & CIPHER_KEY ||
						  updated & INTRO_FILE || updated & BACKUP_FILE ||
						  updated & BACKUP_URL || updated & MOVED_URL)))
					{
						sd->killSource(streamID);
						m_reloadRefresh = true;

						if (!(*iNSC).second.m_relayUrl.url().empty() &&
						    !sd->isSourceConnected(streamID) &&
						    gOptions.stream_movedUrl(streamID).empty())
						{
							relays.push_back((*iNSC).second);
						}
					}
					// otherwise do an in-place update as long as it is applicable
					else
					{
						sd->streamUpdate(streamID, (*iNSC).second.m_authHash, (*iNSC).second.m_maxStreamUser,
										 (*iNSC).second.m_maxStreamBitrate, (*iNSC).second.m_minStreamBitrate);
					}

					// refresh the played history size as needed
					if (updated & SONG_HIST)
					{
						sd->updateSongHistorySize();
					}

					if (updated & ARTWORK_FILE)
					{
						if (gOptions.read_stream_artworkFile(streamID))
						{
							gOptions.m_artworkBody[streamID] = loadLocalFile(fileUtil::getFullFilePath(gOptions.stream_artworkFile(streamID)), LOGNAME, 523872/*32 x (16384 - 6 - 6 - 1)*/);
						}
						else
						{
							gOptions.m_artworkBody[streamID].clear();
							sd->clearCachedArtwork(0);
						}
					}
					sd->releaseStream();
				}
				// otherwise if no proper update or a relay was added / is known and not connected then bump it
				else
				{
					// force flag a source relay kick if this is called
					bool noEntry = false;
					if ((streamData::isRelayActive(streamID, noEntry) & 12))
					{
						// if there is a relay attempt and no new one
						// then we need to signal it to abort trying
						if ((*iNSC).second.m_relayUrl.url().empty())
						{
							streamData::setRelayActiveFlags (streamID, noEntry, 2);
						}
						// otherwise we need update the relay url
						// which the attempt is trying to join to
						// which is done by the relay handling.
					}

					if (((*iOSC2).second.m_relayUrl.url() != (*iNSC).second.m_relayUrl.url() ||
						(updated & MOVED_URL)) && !(*iNSC).second.m_relayUrl.url().empty())
					{
						relays.push_back((*iNSC).second);
						m_reloadRefresh = true;
					}
				}
			}
			else
			{
				// addition required
				// no need to do anything else as there shouldn't be anything connected on this at the time
				gOptions.addStreamConfig(newOptions, (*iNSC).second);
				m_reloadRefresh = true;

				// only attempt to start a relay url if added and a relay url exists
				if (!(*iNSC).second.m_relayUrl.url().empty())
				{
					relays.push_back((*iNSC).second);
				}
			}
		}
	}

	// otherwise if there are fewer stream configurations then we can update / remove as required
	else
	{
		for (; iOSC != old_streams.end(); ++iOSC)
		{
			config::streams_t::const_iterator iOSC2 = new_streams.find((*iOSC).first);
			if (iOSC2 != new_streams.end())
			{
				// update required
				__uint64 updated = gOptions.updateStreamConfig(newOptions, (*iOSC2).second);
				size_t streamID = (*iOSC2).second.m_streamID;
				streamData *sd = streamData::accessStream(streamID);
				if (sd)
				{
					// check what has been updated and kick the source as applicable
					bool isRelay = sd->isRelayStream(streamID);
					if (publicChanged || force ||
						(!force && (updated & RELAY_URL || updated & SOURCE_PWD ||
						 updated & PUBLIC_SRV || (updated & ALLOW_RELAY && isRelay) ||
						 (updated & ALLOW_PUBLIC_RELAY && isRelay) || updated & CIPHER_KEY ||
						  updated & INTRO_FILE || updated & BACKUP_FILE ||
						  updated & BACKUP_URL || updated & MOVED_URL)))
					{
						sd->killSource(streamID);
						m_reloadRefresh = true;

						if (!(*iOSC2).second.m_relayUrl.url().empty() &&
						    !sd->isSourceConnected(streamID) &&
						    gOptions.stream_movedUrl(streamID).empty())
						{
							relays.push_back((*iOSC2).second);
						}
					}
					// otherwise do an in-place update as long as it is applicable
					else
					{
						sd->streamUpdate(streamID, (*iOSC2).second.m_authHash, (*iOSC2).second.m_maxStreamUser,
										 (*iOSC2).second.m_maxStreamBitrate, (*iOSC2).second.m_minStreamBitrate);
					}

					// refresh the played history size as needed
					if (updated & SONG_HIST)
					{
						sd->updateSongHistorySize();
					}

					if (updated & ARTWORK_FILE)
					{
						if (gOptions.read_stream_artworkFile(streamID))
						{
							gOptions.m_artworkBody[streamID] = loadLocalFile(fileUtil::getFullFilePath(gOptions.stream_artworkFile(streamID)), LOGNAME, 523872/*32 x (16384 - 6 - 6 - 1)*/);
						}
						else
						{
							gOptions.m_artworkBody[streamID].clear();
							sd->clearCachedArtwork(0);
						}
					}
					sd->releaseStream();
				}
			}
			else
			{
				// kick the source and clients as required on a removal
				size_t streamID = (*iOSC).second.m_streamID;
				streamData *sd = streamData::accessStream(streamID);
				if (sd)
				{
					sd->killSource(streamID, sd);
					m_reloadRefresh = true;
				}
				// removal required
				gOptions.removeStreamConfig((*iOSC).second);
			}
		}
	}

	// test for the require stream configs having changed
	// only need to kick streams not known if enabled
	if (newOptions.requireStreamConfigs() != gOptions.requireStreamConfigs())
	{
		gOptions.setOption(utf8("requirestreamconfigs"),utf8(newOptions.requireStreamConfigs()?"1":"0"));
		if (newOptions.requireStreamConfigs())
		{
			size_t inc = 0;
			size_t sid;
			do
			{
				sid = streamData::enumStreams(inc);
				config::streams_t streams;
				gOptions.getStreamConfigs(streams, false);

				config::streams_t::const_iterator ics = streams.find(sid);
				if (ics == streams.end())
				{
					streamData *sd = streamData::accessStream(sid);
					if (sd)
					{
						ILOG(gOptions.logSectionName() + "Killing source for stream #" + tos(sid) + " as it is not in the known stream config(s).", LOG_NAME, sid);
						sd->killSource(sid, sd);
						m_reloadRefresh = true;
					}
				}
				++inc;
			}
			while (sid);
		}
	}

	// finally we refresh the passwords so they're correct after any changes
	config::streams_t streams;
	gOptions.getStreamConfigs(streams, false);
	gOptions.setupPasswords(streams);

	// if a force was done then we really need to restart any relays
	if (force)
	{
		// schedule relays
		vector<config::streamConfig> relayList(gOptions.getRelayList());
		if (!relayList.empty())
		{
			for_each(relayList.begin(),relayList.end(),restartRelay);
		}
	}
	// otherwise only attempt to restart any which were added, changed, etc
	else
	{
		if (!relays.empty())
		{
			for_each(relays.begin(),relays.end(),restartRelay);
		}
	}

	ILOG(gOptions.logSectionName() + "Completed stream config reload from `" + fileUtil::getFullFilePath(gOptions.confFile()));
	return m_reloadRefresh;
}

void reloadBanLists()
{
	// load up ban file
	int loaded = g_banList.load(gOptions.banFile(),0),
		count = loaded;

	// per-stream options
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		// load up ban file
		if (gOptions.read_stream_banFile((*i).first))
		{
			++count;
			if (g_banList.load(gOptions.stream_banFile((*i).first),(*i).first))
			{
				++loaded;
			}
		}
	}

	if (!count)
	{
		ILOG("[BAN] No banned lists reloaded.");
	}
	else
	{
		if (count == loaded)
		{
			ILOG("[BAN] Reloaded all banned list(s).");
		}
		else
		{
			ILOG("[BAN] Partially reloaded banned list(s) - check error messages above.");
		}
	}
}

void reloadRipLists()
{
	// load up rip file
	int loaded = g_ripList.load(gOptions.ripFile(),0),
		count = loaded;

	// per-stream options
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		// load up rip file
		if (gOptions.read_stream_ripFile((*i).first))
		{
			++count;
			if (g_ripList.load(gOptions.stream_ripFile((*i).first),(*i).first))
			{
				++loaded;
			}
		}
	}

	if (!count)
	{
		ILOG("[RIP] No reserved lists reloaded.");
	}
	else
	{
		if (count == loaded)
		{
			ILOG("[RIP] Reloaded all reserved list(s).");
		}
		else
		{
			ILOG("[RIP] Partially reloaded reserved list(s) - check error messages above.");
		}
	}
}

void reloadAdminAccessList()
{
	// load up admin access file
	g_adminList.load(gOptions.adminFile());
	ILOG("[ADMINCGI] Reloaded admin access list.");
}

void reloadAgentLists()
{
	// load up agent file
	int loaded = g_agentList.load(gOptions.agentFile(),0),
		count = loaded;

	// per-stream options
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		// load up agent file
		if (gOptions.read_stream_agentFile((*i).first))
		{
			++count;
			if (g_agentList.load(gOptions.stream_agentFile((*i).first),(*i).first))
			{
				++loaded;
			}
		}
	}

	if (!count)
	{
		ILOG("[AGENT] No user agent lists reloaded.");
	}
	else
	{
		if (count == loaded)
		{
			ILOG("[AGENT] Reloaded all user agent list(s).");
		}
		else
		{
			ILOG("[AGENT] Partially reloaded user agent list(s) - check error messages above.");
		}
	}
}

void printUpdateMessage()
{
	// display update message where applicable
	updater::verInfo ver;
	if (updater::getNewVersion(ver))
	{
		ULOG(string(YP2_LOGNAME) + "A new DNAS version is now available: " + ver.ver);
		ULOG(string(YP2_LOGNAME) + "The suggested download for your setup is: " + ver.url);
		ULOG(string(YP2_LOGNAME) + "See " + ver.log + " for more information about this update and alternative download links");
	}
}

utf8 warningImage(const bool right = true)
{
	return utf8(right ? "&nbsp;" : "") + "<img border=\"0\" src=\"images/warn.png\" style=\"float:" +
		   utf8(right ? "right" : "left") + "\" "
		   "alt=\"" + (right ? "Possible Stream Ripper" : "Please Register Your Authhash") +
		   "\" title=\"" + (right ? "Possible Stream Ripper" : "Please Register Your Authhash") + "\">";
}

utf8 baseImage(const utf8& image, const utf8& title, const bool left = false, const bool prefix = true)
{
	return (prefix ? "&nbsp;" : (utf8)"") + "<img border=\"0\" src=\"images/" +
		   (!image.empty() ? (image + ".png") : "favicon.ico") + "\" " +
		   (left ? "style=\"float:left\" " : "") + "alt=\"" + title + "\" title=\"" + title + "\">";
}

#define xffImage() baseImage("xff", "XFF", true)
#define mplayerImage() baseImage("mplayer", "MPlayer Client")
#define psImage() baseImage("ps", "PlayStation Client")
#define radioToolboxImage() baseImage("rtb", "Radio Toolbox Monitor / Player")
#define v1Image() baseImage("v1", "v1 Client")
#define v2Image() baseImage("v2", "v2 Client")
#define relayImage() baseImage("relay", "Relay Connection", false, false)
#define html5Image() baseImage("html5", "HTTP / HTML5 Client")
#define flashImage() baseImage("flash", "Flash Client")
#define icecastImage() baseImage("icecast", "Icecast Client / Relay")
#define vlcImage() baseImage("vlc", "VLC Client")
#define waImage() baseImage("wa", "Winamp Client")
#define scImage() baseImage("", "Shoutcast Client / Relay")
#define iOSImage() baseImage("", "iOS Client")
#define curlImage() baseImage("curl", "cURL / libcurl Based Client")
#define radionomyImage() baseImage("radionomy", "Radionomy Stats Collector")
#define fb2kImage() baseImage("fb2k", "Foobar2000 Client")
#define rokuImage() baseImage("roku", "Roku Streaming Player")
#define WiiMCImage() baseImage("v1", "Wii Media Centre Player")
#define synologyImage() baseImage("synology", "Audio Station (Synology) Player")
#define appleImage() baseImage("apple", "Apple Device / OS / Client")
#define iTunesImage() baseImage("itunes", "iTunes Client")
#define wmpImage() baseImage("wmp", "Windows Media Player Client")
#define chromeImage() baseImage("chrome", "Chrome Web Browser")
#define safariImage() baseImage("safari", "Safari Web Browser")
#define ieImage() baseImage("ie", "Internet Explorer Web Browser")
#define firefoxImage() baseImage("firefox", "Firefox Web Browser")

utf8 advertImage(const streamData::streamID_t sid, const int group, const size_t count)
{
	if (streamData::knownAdvertGroup(sid, group))
	{
		if (count > 0)
		{
			return baseImage("adplayed", (tos(count) + " Advert Breaks Have Been Played\nAdvert Group: " + tos(group)), false, false);
		}
		return baseImage("adavail", ("Waiting For First Advert Break To Play\nAdvert Group: " + tos(group)), false, false);
	}
	if (count > 0)
	{
		return baseImage("", (tos(count) + " Advert Breaks Have Been Played\nNo Applicable "
						 "Adverts Currently Available\nAdvert Group: " + tos(group)), false, false);
	}

	return baseImage("noadavail", ("No Applicable Adverts Available\nAdvert Group: " + tos(group)), false, false);
}

utf8 getClientImage(const streamData::source_t type)
{
	return ((type & streamData::RADIONOMY) ? radionomyImage() :
		   ((type & streamData::FLV) ? flashImage() :
		   ((type & streamData::CURL_TOOL) ? curlImage() :
		   ((type & streamData::HTTP) ? html5Image() :
		   //((type & streamData::M4A) ? m4aImage() :
		   ((type & streamData::SHOUTCAST2) ? ((type & streamData::RELAY) ? scImage() : waImage()) :
		   ((type & streamData::ICECAST) ? icecastImage() :
		   ((type & streamData::VLC) ? vlcImage() :
		   ((type & streamData::WINAMP) ? waImage() :
		   ((type & streamData::APPLE) ? appleImage() :
		   ((type & streamData::ITUNES) ? iTunesImage() :
		   ((type & streamData::WMP) ? wmpImage() :
		   ((type & streamData::ROKU) ? rokuImage() :
		   ((type & streamData::WIIMC) ? WiiMCImage() :
		   ((type & streamData::SYNOLOGY) ? synologyImage() :
		   ((type & streamData::CHROME) ? chromeImage() :
		   ((type & streamData::SAFARI) ? safariImage() :
		   ((type & streamData::IE) ? ieImage() :
		   ((type & streamData::FIREFOX) ? firefoxImage() :
		   ((type & streamData::MPLAYER) ? mplayerImage() :
		   ((type & streamData::PS) ? psImage() :
		   ((type & streamData::RADIO_TOOLBOX) ? radioToolboxImage() :
		   ((type & streamData::HTML5) ? html5Image() :
		   ((type & streamData::WARNING) ? warningImage() :
		   ((type & streamData::SC_IRADIO) ? iOSImage() :
		   ((type & streamData::FB2K) ? fb2kImage() : v1Image()
		   ))))))))))))))))))))))))) +
		   ((type & streamData::RELAY) ? relayImage() : "");
}

/*
	Handles all HTTP requests to admin.cgi
*/

protocol_admincgi::protocol_admincgi(const socketOps::tSOCKET s, const streamData::streamID_t sid, const bool no_sid,
									 const bool zero_sid, const utf8 &clientLogString,
									 const uniString::utf8 &password, const uniString::utf8 &referer,
									 const uniString::utf8 &hostIP, const uniString::utf8 &userAgent,
									 const protocol_HTTPStyle::HTTPRequestInfo &httpRequestInfo) throw(std::exception)
	: runnable(s), m_noSID(no_sid), m_zeroSID(zero_sid),
	  m_saveLogFile(0), m_clientLogString(clientLogString),
	  m_httpRequestInfo(httpRequestInfo), m_password(password),
	  m_referer(referer), m_hostIP(hostIP), m_userAgent(userAgent),
	  m_sid(sid), m_state(&protocol_admincgi::state_ConfirmPassword),
	  m_nextState(0), m_outBuffer(0), m_outBufferSize(0),
	  m_tailLogFile(false), lastChar(-1), inMsg(false),
	  first(false), m_logFile(0)
{
	memset(&m_stream, 0, sizeof(m_stream));

	// check for updates weekly from the last update
	// so we're taking into account manual checks...
	if ((m_lastActivityTime - last_update_check) > 604800)
	{
		checkVersion(m_lastActivityTime);
	}

	DEBUG_LOG(utf8(LOGNAME) + __FUNCTION__);
}

protocol_admincgi::~protocol_admincgi() throw()
{
	DEBUG_LOG(utf8(LOGNAME) + __FUNCTION__);

	socketOps::forgetTCPSocket(m_socket);
	if (m_logFile)
	{
		::fclose(m_logFile);
		m_logFile = 0;
	}
}

void protocol_admincgi::timeSlice() throw(exception)
{
	(this->*m_state)();
}

void protocol_admincgi::state_Close() throw(exception)
{
	m_result.done();
}

// send buffer text
void protocol_admincgi::state_Send() throw(exception)
{
	if (sendDataBuffer(DEFAULT_CLIENT_STREAM_ID, m_outBuffer, m_outBufferSize, m_clientLogString))
	{
		m_state = m_nextState;
	}
}

void protocol_admincgi::sendMessageAndClose(const utf8 &msg) throw()
{
	m_outMsg = msg;
	m_outBuffer = m_outMsg.c_str();
	bandWidth::updateAmount(bandWidth::PRIVATE_WEB, (m_outBufferSize = (int)m_outMsg.size()));
	m_state = &protocol_admincgi::state_Send;
	m_nextState = &protocol_admincgi::state_Close;
	m_result.write();
	m_result.run();
}

/// update the metatdata in the shoutcast ring buffer		
void protocol_admincgi::state_UpdateMetadata() throw(exception)
{
	DEBUG_LOG(utf8(LOGNAME) + __FUNCTION__);

	streamData::streamInfo info;
	streamData::extraInfo extra;
	streamData::getStreamInfo(m_sid, info, extra);

	utf8 titleMsg, urlMsg, djMsg, nextMsg;

	if (!m_updinfoSong.empty())
	{
		// use this as a way to try to ensure we've got a utf-8
		// encoded title to improve legacy source title support
		if (!m_updinfoSong.isValid())
		{
			m_updinfoSong = asciiToUtf8(m_updinfoSong.toANSI(true));
		}

		if (streamData::validateTitle(m_updinfoSong))
		{
			titleMsg = ("[ADMINCGI sid=" + tos(m_sid) + "] Title updated [" + m_updinfoSong + "]");
		}
		else
		{
			WLOG("Title update rejected - value not allowed: " + m_updinfoSong, LOG_NAME, m_sid);
			m_updinfoSong = info.m_currentSong;
		}
	}
	else
	{
		if (!info.m_currentSong.empty())
		{
			titleMsg = ("[ADMINCGI sid=" + tos(m_sid) + "] Title cleared");
		}
	}

	if (!m_updinfoURL.empty())
	{
		urlMsg = ("[ADMINCGI sid=" + tos(m_sid) + "] Song url updated [" + m_updinfoURL + "]");
	}
	else
	{
		if (!info.m_currentURL.empty())
		{
			// TODO if there's a custom image then indicate using it or do not show this
			urlMsg = ("[ADMINCGI sid=" + tos(m_sid) + "] Song url cleared");
		}
	}

	if (!m_updinfoDJ.empty())
	{
		djMsg = ("[ADMINCGI sid=" + tos(m_sid) + "] DJ name updated [" + m_updinfoDJ + "]");
	}
	else
	{
		if (!info.m_streamUser.empty())
		{
			djMsg = ("[ADMINCGI sid=" + tos(m_sid) + "] DJ name cleared");
		}
	}

	if (!m_updinfoNext.empty())
	{
		// use this as a way to try to ensure we've got a utf-8
		// encoded title to improve legacy source title support
		if (!m_updinfoNext.isValid())
		{
			m_updinfoNext = asciiToUtf8(m_updinfoNext.toANSI(true));
		}

		if (streamData::validateTitle(m_updinfoNext))
		{
			titleMsg = ("[ADMINCGI sid=" + tos(m_sid) + "] Next title updated [" + m_updinfoNext + "]");
		}
		else
		{
			WLOG("[ADMINCGI sid=" + tos(m_sid) + "] Next title update rejected - value not allowed: " + m_updinfoNext);
			m_updinfoNext = info.m_comingSoon;
		}
	}
	else
	{
		if (!info.m_comingSoon.empty())
		{
			nextMsg = ("[ADMINCGI sid=" + tos(m_sid) + "] Next title cleared");
		}
	}

	streamData *sd = streamData::accessStream(m_sid);
	if (sd)
	{
		utf8 sourceIdent = (!m_userAgent.empty() ? m_userAgent : utf8("Legacy / Unknown"));
		sd->updateSourceIdent(sourceIdent, sd->isRelayStream(m_sid));
		sd->addSc1MetadataAtCurrentPosition(LOGNAME, m_updinfoSong, m_updinfoURL, m_updinfoNext);
		sd->updateStreamUser(m_updinfoDJ);
		// this will call streamData::releaseStream(..)
		streamData::streamClientLost(LOGNAME, sd, m_sid);

		// we'll output any earlier generated messages now that we've sent
		// the details to be used as otherwise they were being reported as
		// ok which if there was an issue (e.g. bad sid#) was confusing
		if (!titleMsg.empty())
		{
			ILOG(titleMsg, LOG_NAME, m_sid);
		}
		if (!nextMsg.empty())
		{
			ILOG(nextMsg, LOG_NAME, m_sid);
		}
		if (!urlMsg.empty())
		{
			ILOG(urlMsg, LOG_NAME, m_sid);
		}
		if (!djMsg.empty())
		{
			ILOG(djMsg, LOG_NAME, m_sid);
		}
	}
	else
	{
		WLOG("[ADMINCGI sid=" + tos(m_sid) + "] Metadata update rejected as the stream does not exist", LOG_NAME, m_sid);
	}

	m_updinfoSong.clear();
	m_updinfoURL.clear();
	m_updinfoDJ.clear();

	sendMessageAndClose(MSG_200);
}

/// update the metatdata in the shoutcast ring buffer		
void protocol_admincgi::state_UpdateXMLMetadata() throw(exception)
{
	DEBUG_LOG(utf8(LOGNAME) + __FUNCTION__);

	if (!m_updinfoSong.empty())
	{
		ILOG("XML title update [" + m_updinfoSong + "]", LOG_NAME, m_sid);
		streamData *sd = streamData::accessStream(m_sid);
		if (sd)
		{
			vector<__uint8> assembledData;
			assembledData.insert(assembledData.end(), m_updinfoSong.begin(), m_updinfoSong.end());
			utf8 sourceIdent = (!m_userAgent.empty() ? m_userAgent : utf8("Legacy / Unknown"));
			sd->updateSourceIdent(sourceIdent);
			sd->addUvoxMetadataAtCurrentPosition(MSG_METADATA_XML_NEW, assembledData);
			// this will call streamData::releaseStream(..)
			streamData::streamClientLost(LOGNAME, sd, m_sid);
		}
		else
		{
			WLOG("XML title update rejected as stream #" + tos(m_sid) + " does not exist", LOG_NAME, m_sid);
		}
		m_updinfoSong.clear();
		m_updinfoURL.clear();
	}

	sendMessageAndClose(MSG_200);
}

void protocol_admincgi::state_ConfirmPassword() throw(std::exception)
{
	DEBUG_LOG(utf8(LOGNAME) + __FUNCTION__);

	// this will see if we've been refered from the base admin.cgi and will allow through
	// if the password / auth matches with the key admin account so the page is useable.
	utf8::size_type pos;
	if ((pos = m_referer.rfind(utf8("admin.cgi"))) != utf8::npos)
	{
		m_referer = m_referer.substr(pos);
	}

	const utf8& mode = mapGet(m_httpRequestInfo.m_QueryParameters, "mode", (utf8)"");
	const bool updinfo = (mode == "updinfo");
	const bool viewxml = (mode == "viewxml");
	const bool viewjson = (mode == "viewjson");
	const bool _register = (mode == "register");

	// on the root admin summary page we can only allow referer admin through if looking at the stats
	const bool adminRefer = (m_referer.find(utf8("admin.cgi")) == 0 || m_referer.find(utf8("admin.cgi?sid=0")) == 0);
	int okReferer = (adminRefer && (viewxml || viewjson || _register));

	// on the root admin summary page we can check if it's an authhash action and allow
	// through if things match up + also allow updinfo (not ideal but maintains legacy)
	if ((!okReferer && ((mode == "manualauthhash") || _register)) || updinfo)
	{
		if (!m_referer.empty())
		{
			okReferer = (m_referer.find(utf8("admin.cgi?sid=" + tos(m_sid) + "&mode=")) != utf8::npos);
		}
	}

	DEBUG_LOG(LOGNAME "Referrer status: " + tos(okReferer));

	// fixed in Build 18 to revert to the master passwords if there's nothing for the stream
	utf8 streamPassword = gOptions.stream_password(m_sid);
	if (!gOptions.read_stream_password(m_sid) && streamPassword.empty())
	{
		streamPassword = gOptions.password();
	}

	utf8 streamAdminPassword = gOptions.stream_adminPassword(m_sid);
	if (!gOptions.read_stream_adminPassword(m_sid) && streamAdminPassword.empty())
	{
		streamAdminPassword = gOptions.adminPassword();
	}

	// this is so we can allow source password connections access to the
	// mode=viewxml&page=1 (/stats) and mode=viewxml&page=4 (/played) so
	// we maintain compatibility with 1.x DNAS which only had these ones
	int page = 0;
	bool compat_mode = false;
	if (viewxml || viewjson)
	{
		page = mapGet(m_httpRequestInfo.m_QueryParameters, "page", (int)page);
		// this will act like /stats or / played (or both
		// if page=0)so as to better emulate the 1.x DNAS
		if ((page == 0) || (page == 1) || (page == 4))
		{
			compat_mode = true;
		}
	}

	bool proceed = (((!streamPassword.empty() && (updinfo || compat_mode) && (m_password == streamPassword)) ||
					(!streamAdminPassword.empty() && (m_password == streamAdminPassword)) ||
					(!gOptions.adminPassword().empty() && (m_password == gOptions.adminPassword()))));

	DEBUG_LOG(LOGNAME "Password status: " + tos(proceed));

	if (proceed)
	{
		const streamData::streamID_t this_sid = (m_zeroSID || m_noSID ? 0 : m_sid);
		const utf8& p1 = mapGet(m_httpRequestInfo.m_QueryParameters, mode, (utf8)"");
		DEBUG_LOG(LOGNAME "Requested mode: " + (!mode.empty() ? mode : "Not Specified"));
		if (updinfo)
		{
			m_updinfoSong = stripWhitespace(mapGet(m_httpRequestInfo.m_QueryParameters, "song", (utf8)""));
			m_updinfoURL = stripWhitespace(mapGet(m_httpRequestInfo.m_QueryParameters, "url", (utf8)""));
			m_updinfoDJ = stripWhitespace(mapGet(m_httpRequestInfo.m_QueryParameters, "dj", (utf8)""));
			m_updinfoNext = stripWhitespace(mapGet(m_httpRequestInfo.m_QueryParameters, "next", (utf8)""));
			m_state = ((m_updinfoSong.find(utf8("<?xml ")) != utf8::npos) ? &protocol_admincgi::state_UpdateXMLMetadata : &protocol_admincgi::state_UpdateMetadata);
			m_result.run();
		}
		else if (viewxml)
		{
			if (m_noSID)
			{
				sendMessageAndClose(redirect("index.html", SHRINK));
			}
			else
			{
				const bool iponly = mapGet(m_httpRequestInfo.m_QueryParameters, "iponly", (bool)false);
				const bool ipcount = mapGet(m_httpRequestInfo.m_QueryParameters, "ipcount", (bool)false);
				mode_viewxml(m_sid, page, iponly, ipcount);
			}
		}
		else if (viewjson)
		{
			if (m_noSID)
			{
				sendMessageAndClose(redirect("index.html", SHRINK));
			}
			else
			{
				const bool iponly = mapGet(m_httpRequestInfo.m_QueryParameters, "iponly", (bool)false);
				const bool ipcount = mapGet(m_httpRequestInfo.m_QueryParameters, "ipcount", (bool)false);
				const utf8 &callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
				mode_viewjson(m_sid, page, iponly, ipcount, callback);
			}
		}
		else if (_register)
		{
			if (m_noSID)
			{
				// do a sanity check so that we're only accessing this with the true adminpassword
				if (m_zeroSID)
				{
					mode_summary(0);
				}
				// not matching the master password so show the simple summary
				else
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
			}
			else
			{
				// see if connected otherwise block any access to the pages
				bool connected = false;
				if (p1 == "clear")
				{
					bool handled = false, idHandled = false;
					// if we get a clear then just remove from the config and reload the page
					if (gOptions.editConfigFileEntry(m_sid, gOptions.confFile(), "", "",
													 true, handled, idHandled, true) == false)
					{
						handled = false;
					}
					// now attempt to update internal states as appropriate if all went ok
					if (handled == true)
					{
						gOptions.setOption(utf8("streamauthhash_" + tos(m_sid)), (utf8)"");
						streamData *sd = streamData::accessStream(m_sid);
						if (sd)
						{
							sd->streamUpdate(m_sid, (utf8)"", sd->streamMaxUser(),
											 sd->streamMaxBitrate(), sd->streamMinBitrate());
							sd->releaseStream();
						}
					}
				}
				else
				{
					connected = true;

					// sanity checks to ensure that we're not re-adding when there is one, etc
					streamData::streamInfo info;
					streamData::extraInfo extra;
					if (!streamData::getStreamInfo(m_sid, info, extra))
					{
						info.m_authHash = gOptions.stream_authHash(m_sid);
					}

					if (connected == true || extra.isRelay)
					{
						mode_register(m_sid, info);
					}
				}

				if (connected == false)
				{
					sendMessageAndClose(redirect("admin.cgi?sid=" + tos(this_sid), SHRINK));
				}
			}
		}
		else if (mode == "listeners")
		{
			mode_listeners(this_sid);
		}
		else if (mode == "kicksrc")
		{
			if (m_noSID)
			{
				if (adminRefer)
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
			else
			{
				// kick source off system
				streamData::killStreamSource(m_sid);

				if (!m_referer.empty())
				{
					utf8 check = ("admin.cgi?sid=" + tos(m_sid));
					// if the referer is the server summary page then we need to go back to it
					sendMessageAndClose(redirect((m_referer == check ? check : "admin.cgi?sid=0"), SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
		}
		else if (mode == "kickdst" && (!p1.empty()))
		{
			if (m_noSID)
			{
				if (adminRefer)
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
			else
			{
				mode_kickdst(m_sid, p1);
			}
		}
		else if (mode == "viewban")
		{
			mode_viewban(this_sid);
		}
		else if (mode == "bandst" && (!p1.empty()))
		{
			const int mask = mapGet(m_httpRequestInfo.m_QueryParameters, "banmsk", (int)255);
			mode_ban(this_sid, p1, mask);
		}
		else if (mode == "banip")
		{
			const utf8 &ip1 = mapGet(m_httpRequestInfo.m_QueryParameters, "ip1", (utf8)""),
					   &ip2 = mapGet(m_httpRequestInfo.m_QueryParameters, "ip2", (utf8)""),
					   &ip3 = mapGet(m_httpRequestInfo.m_QueryParameters, "ip3", (utf8)""),
					   &ip4 = mapGet(m_httpRequestInfo.m_QueryParameters, "ip4", (utf8)""),
					   &mask = mapGet(m_httpRequestInfo.m_QueryParameters, "banmsk", (utf8)"");
			if (ip1.empty() || ip2.empty() || ip3.empty() || ip4.empty() || mask.empty())
			{
				if ((m_referer != utf8("admin.cgi?sid=" + tos(this_sid) + "&mode=viewban")) &&
					(m_referer != utf8("admin.cgi?mode=viewban")))
				{
					sendMessageAndClose(MSG_STD200 + utf8(!HEAD_REQUEST ? "<html><head>"
										"<title>Shoutcast Server</title></head><body>"
										"Invalid resource</body></html>" : ""));
				}
				else
				{
					if (!m_referer.empty())
					{
						sendMessageAndClose(redirect("admin.cgi?sid=" + tos(this_sid) + "&mode=viewban", SHRINK));
					}
					else
					{
						sendMessageAndClose(MSG_200);
					}
				}
			}
			else
			{
				mode_ban(this_sid, (ip1 + "." + ip2 + "." + ip3 + "." + ip4), strtol((const char *)mask.c_str(),0,10));
			}
		}
		else if (mode == "unbandst")
		{
			const utf8 &ip = mapGet(m_httpRequestInfo.m_QueryParameters, "bandst", (utf8)""),
					   &mask = mapGet(m_httpRequestInfo.m_QueryParameters, "banmsk", (utf8)"");
			if (ip.empty() || mask.empty())
			{
				if ((m_referer != utf8("admin.cgi?sid=" + tos(this_sid) + "&mode=viewban")) &&
					(m_referer != utf8("admin.cgi?mode=viewban")))
				{
					sendMessageAndClose(MSG_STD200 + utf8(!HEAD_REQUEST ? "<html><head>"
										"<title>Shoutcast Server</title></head><body>"
										"Invalid resource</body></html>" : ""));
				}
				else
				{
					if (!m_referer.empty())
					{
						sendMessageAndClose(redirect("admin.cgi?sid=" + tos(this_sid) + "&mode=viewban", SHRINK));
					}
					else
					{
						sendMessageAndClose(MSG_200);
					}
				}
			}
			else
			{
				mode_unban(this_sid, ip, strtol((const char *)mask.c_str(), 0, 10));
			}
		}
		else if (mode == "viewrip")
		{
			mode_viewrip(this_sid);
		}
		else if (mode == "ripip")
		{
			const utf8 &ip1 = mapGet(m_httpRequestInfo.m_QueryParameters, "ip1", (utf8)""),
					   &ip2 = mapGet(m_httpRequestInfo.m_QueryParameters, "ip2", (utf8)""),
					   &ip3 = mapGet(m_httpRequestInfo.m_QueryParameters, "ip3", (utf8)""),
					   &ip4 = mapGet(m_httpRequestInfo.m_QueryParameters, "ip4", (utf8)"");
			if (ip1.empty() || ip2.empty() || ip3.empty() || ip4.empty())
			{
				// see if we've got a host add attempt and handle as appropriately
				if (m_hostIP.empty())
				{
					if ((m_referer != utf8("admin.cgi?sid=" + tos(m_sid) + "&mode=viewrip")) &&
						(m_referer != utf8("admin.cgi?mode=viewrip")))
					{
						sendMessageAndClose(MSG_STD200 + utf8(!HEAD_REQUEST ? "<html><head>"
											"<title>Shoutcast Server</title></head><body>"
											"Invalid resource</body></html>" : ""));
					}
					else
					{
						if (!m_referer.empty())
						{
							sendMessageAndClose(redirect("admin.cgi?sid=" + tos(m_sid) + "&mode=viewrip", SHRINK));
						}
						else
						{
							sendMessageAndClose(MSG_200);
						}
					}
				}
				else
				{
					const utf8 &raw = mapGet(m_httpRequestInfo.m_QueryParameters, "ripdstraw", (utf8)"");
					mode_rip(this_sid, m_hostIP, raw);
				}
			}
			else
			{
				const utf8 &raw = mapGet(m_httpRequestInfo.m_QueryParameters, "ripdstraw", (utf8)"");
				mode_rip(this_sid, (ip1 + "." + ip2 + "." + ip3 + "." + ip4), raw);
			}
		}
		else if (mode == "unripdst")
		{
			const utf8 &ip = mapGet(m_httpRequestInfo.m_QueryParameters, "ripdst", (utf8)""),
					   &raw = mapGet(m_httpRequestInfo.m_QueryParameters, "ripdstraw", (utf8)"");
			if (!isAddress(ip))
			{
				if ((m_referer != utf8("admin.cgi?sid=" + tos(m_sid) + "&mode=viewrip")) &&
					(m_referer != utf8("admin.cgi?mode=viewrip")))
				{
					sendMessageAndClose(MSG_STD200 + utf8(!HEAD_REQUEST ? "<html><head>"
										"<title>Shoutcast Server</title></head><body>"
										"Invalid resource</body></html>" : ""));
				}
				else
				{
					if (!m_referer.empty())
					{
						sendMessageAndClose(redirect("admin.cgi?sid=" + tos(m_sid) + "&mode=viewrip", SHRINK));
					}
					else
					{
						sendMessageAndClose(MSG_200);
					}
				}
			}
			else
			{
				mode_unrip(this_sid, ip, raw);
			}
		}
		else if (mode == "ripdst" && (!p1.empty()))
		{
			const utf8 &raw = mapGet(m_httpRequestInfo.m_QueryParameters, "ripdstraw", (utf8)"");
			mode_rip(this_sid, p1, raw);
		}
		else if (mode == "viewagent")
		{
			mode_viewagent(this_sid);
		}
		else if (mode == "unagent")
		{
			const utf8 &agent = mapGet(m_httpRequestInfo.m_QueryParameters, "agent", (utf8)"");
			if (agent.empty())
			{
				if ((m_referer != utf8("admin.cgi?sid=" + tos(m_sid) + "&mode=viewagent")) &&
					(m_referer != utf8("admin.cgi?mode=viewagent")))
				{
					sendMessageAndClose(MSG_STD200 + utf8(!HEAD_REQUEST ? "<html><head>"
										"<title>Shoutcast Server</title></head><body>"
										"Invalid resource</body></html>" : ""));
				}
				else
				{
					sendMessageAndClose(redirect("admin.cgi?sid=" + tos(m_sid) + "&mode=viewagent", SHRINK));
				}
			}
			else
			{
				mode_unagent(this_sid, agent);
			}
		}
		else if (mode == "agent")
		{
			const utf8 &agent = mapGet(m_httpRequestInfo.m_QueryParameters, "agent", (utf8)"");
			mode_agent(this_sid, agent);
		}
		else if (mode == "resetxml")
		{
			if (m_noSID)
			{
				if (adminRefer)
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
			else
			{
				stats::resetStats(m_sid);
				if (!m_referer.empty())
				{
					sendMessageAndClose(redirect("admin.cgi?sid=" + tos(m_sid), SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
		}
		else if (mode == "clearcache")
		{
			// TODO consider clearing out the intro / backup files as ell as part of this...

			// clear out all cached resource copies
			gOptions.m_crossdomainStr.clear();
			gOptions.m_crossdomainStrGZ.clear();
			gOptions.m_shoutcastSWFStr.clear();
			gOptions.m_shoutcastSWFStrGZ.clear();
			gOptions.m_robotsTxtBody.clear();
			gOptions.m_robotsTxtBodyGZ.clear();

			gOptions.m_faviconBody.clear();
			gOptions.m_faviconBodyGZ.clear();
			gOptions.m_favIconTime = 0;

			gOptions.m_styleCustomStr.clear();
			gOptions.m_styleCustomStrGZ.clear();
			gOptions.m_styleCustomHeaderTime = 0;

			DeleteAllCaches();

			last_update_check = 0;

			ILOG(LOGNAME "Cleared resource cache(s).");
			if (adminRefer)
			{
				sendMessageAndClose(redirect("admin.cgi", SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else if (mode == "bannedlist")
		{
			reloadBanLists();
			if (adminRefer)
			{
				sendMessageAndClose(redirect("admin.cgi", SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else if (mode == "reservelist")
		{
			reloadRipLists();
			if (adminRefer)
			{
				sendMessageAndClose(redirect("admin.cgi", SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else if (mode == "adminlist")
		{
			reloadAdminAccessList();
			if (adminRefer)
			{
				sendMessageAndClose(redirect("admin.cgi", SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else if (mode == "useragentlist")
		{
			reloadAgentLists();
			if (adminRefer)
			{
				sendMessageAndClose(redirect("admin.cgi", SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else if (mode == "reload")
		{
			const int force = mapGet(m_httpRequestInfo.m_QueryParameters, "force", (int)0);
			if (adminRefer)
			{
				sendMessageAndClose(redirect((reloadConfig(force) == true ? "admin.cgi?sid=0&refresh=3" : "admin.cgi?sid=0"), SHRINK));
			}
			else
			{
				reloadConfig(force);
				sendMessageAndClose(MSG_200);
			}
		}
		else if (mode == "viewlog")
		{
			if (m_noSID)
			{
				const utf8 &server = mapGet(m_httpRequestInfo.m_QueryParameters, "server", (utf8)"");
				if (!server.empty() && ((server == logId) || (server == logTailId)))
				{
					mode_viewlog(m_sid, (p1 == "tail"), (p1 == "save"), true);
				}
				else
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
			}
			else
			{
				mode_viewlog(m_sid, (p1 == "tail"), (p1 == "save"), false);
			}
		}
		else if (mode == "history")
		{
			if (m_noSID)
			{
				if (adminRefer)
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
			else
			{
				const utf8 &type = mapGet(m_httpRequestInfo.m_QueryParameters, "type", (utf8)"");
				const bool json = (type == "json"), xml = (type == "xml");
				if (!json && !xml)
				{
					mode_history(m_sid);
				}
				else
				{
					utf8 header, body;
					if (json)
					{
						const utf8& callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
						body = protocol_HTTPStyle::getPlayedJSON(m_sid, header, callback, true);
					}
					else
					{
						body = protocol_HTTPStyle::getPlayedXML(m_sid, header, true);
					}
					COMPRESS(header, body);
					header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
					sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
				}
			}
		}
		else if (mode == "art")
		{
			if (m_noSID)
			{
				if (adminRefer)
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
			else
			{
				mode_art(m_sid, (p1 == "playing"));
			}
		}
		else if (mode == "manualauthhash")
		{
			bool handled = false;
			const utf8 &authhash = mapGet(m_httpRequestInfo.m_QueryParameters, "authhash", (utf8)"");
			// make sure that we're only allow valid values
			if (authhash.empty() || yp2::isValidAuthhash(authhash))
			{
				bool idHandled = false;
				if (gOptions.editConfigFileEntry(m_sid, gOptions.confFile(), authhash, "",
												 true, handled, idHandled, true) == false)
				{
					handled = false;
				}

				// changed in b69 to force the change through even if there is a config saving issue
				// as there are cases where the authhash is gone but only updating on success would
				// end up in the inability to start things over again e.g. with CentroCast v3 quirks
				gOptions.setOption(utf8("streamauthhash_" + tos(m_sid)), authhash);
			}

			streamData *sd = streamData::accessStream(m_sid);
			if (sd)
			{
				if (sd->isSourceConnected(m_sid))
				{
					utf8 oldAuthhash = sd->streamAuthhash();
					sd->streamUpdate(m_sid, authhash, sd->streamMaxUser(),
									 sd->streamMaxBitrate(), sd->streamMinBitrate());

					ILOG(gOptions.logSectionName() + "Changed authhash for stream #" +
						 tos(m_sid) + " to " + (authhash.empty() ? "empty" : authhash) +
						 " [was " + (oldAuthhash.empty() ? "empty" : oldAuthhash) + "]");
				}
				sd->releaseStream();
			}

			// now attempt to update internal states as appropriate if all went ok
			if (handled == true)
			{
				sendMessageAndClose("HTTP/1.1 200 OK\r\n"
									"Content-Type:text/plain\r\n"
									"Content-Length:5\r\n"
									"Cache-Control:no-cache\r\n"
									"Access-Control-Allow-Origin:*\r\n"
									"Connection:close\r\n\r\n200\r\n");
			}
			else
			{
				utf8 message = "Error saving changes to the configuration file.<br>"
							   "Check that you have write access and the<br>"
							   "specified configuration file still exists.<br><br>"
							   "The requested authhash change was applied.";
				utf8 header = "HTTP/1.1 667\r\n"
							  "Content-Type:text/plain\r\n"
							  "Access-Control-Allow-Origin:*\r\n"
							  "Connection:close\r\n";
				COMPRESS(header, message);
				header += "Content-Length:" + tos(message.size()) + "\r\n\r\n";
				sendMessageAndClose(header + (!HEAD_REQUEST ? message : ""));
			}
		}
		else if (mode == "rotate")
		{
			const utf8 &files = mapGet(m_httpRequestInfo.m_QueryParameters, "files", (utf8)""),
					   &rotateType = (!(files == "log" || files == "w3c") ? "all " : (files == "log" ? "": "W3C "));
			ILOG(LOGNAME "Rotating " + rotateType + "log file(s)");

			if ((files == "log") || (files == ""))
			{
				ROTATE;
				printUpdateMessage();
				if (m_logFile)
				{
					::fclose(m_logFile);
					m_logFile = 0;
				}
			}

			// and now rotate the w3c logs (going upto the configured number of old copies to be like the log rotate)
			rotatew3cFiles(files);

			ILOG(LOGNAME "Rotated " + rotateType + "log file(s) [PID: " + tos(getpid()) + "]");
			if (adminRefer)
			{
				sendMessageAndClose(redirect("admin.cgi", SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else if (mode == "startrelay")
		{
			if (m_noSID)
			{
				if (adminRefer)
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
			else
			{
				// only attempt a source relay connection if it is not signaled
				// as active or pending to prevent multiple attempts being run.
				bool noEntry = false, active = (streamData::isRelayActive(m_sid, noEntry) == 1);
				if (!active)
				{
					config::streamConfig stream;
					if (gOptions.getStreamConfig(stream, m_sid))
					{
						restartRelay(stream);
					}
				}

				if (!m_referer.empty())
				{
					utf8 check = ("admin.cgi?sid=" + tos(m_sid));
					// if the referer is the server summary page then we need to go back to it
					sendMessageAndClose(redirect((m_referer == check ? check : "admin.cgi?sid=0") +
										(!active ? "&refresh=3" : ""), SHRINK));
				}
				else
				{
					sendMessageAndClose(MSG_200);
				}
			}
		}
		else if (mode == "startrelays")
		{
			bool refresh = false;
			config::streams_t streams;
			gOptions.getStreamConfigs(streams);
			for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
			{
				if (!(*i).second.m_relayUrl.url().empty() && !streamData::isSourceConnected((*i).first))
				{
					// only attempt a source relay connection if it is not signaled
					// as active or pending to prevent multiple attempts being run.
					bool noEntry = false, active = (streamData::isRelayActive((*i).first, noEntry) == 1);
					if (!active)
					{
						ILOG(gOptions.logSectionName() + "Starting source for stream #" + tos((*i).first) + ".");
						restartRelay((*i).second);
						refresh = true;
					}
				}
			}

			if (adminRefer)
			{
				sendMessageAndClose(redirect("admin.cgi?sid=0" + (refresh ? "&refresh=3" : (utf8)""), SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else if (mode == "kicksources")
		{
			bool refresh = false;
			streamData::streamIDs_t streamIds = streamData::getStreamIds(true);
			if (!streamIds.empty())
			{
				for (streamData::streamIDs_t::const_iterator i = streamIds.begin(); i != streamIds.end(); ++i)
				{
					// kick source off system
					streamData::killStreamSource((*i));
					refresh = true;
				}
			}

			if (adminRefer)
			{
				sendMessageAndClose(redirect("admin.cgi?sid=0" + (refresh ? "&refresh=1" : (utf8)""), SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else if(mode == "bandwidth")
		{
			const utf8 &type = mapGet(m_httpRequestInfo.m_QueryParameters, "type", (utf8)"");
			const bool json = (type == "json"), xml = (type == "xml");

			if (!json && !xml)
			{
				const int refresh = mapGet(m_httpRequestInfo.m_QueryParameters, "refresh", 0);
				mode_bandwidth_html(refresh);
			}
			else
			{
				if (json)
				{
					const utf8 &callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
					mode_bandwidth_json(callback);
				}
				else
				{
					mode_bandwidth_xml();
				}
			}
		}
		else if (mode == "ypstatus")
		{
			const utf8 &type = mapGet(m_httpRequestInfo.m_QueryParameters, "type", (utf8)"");
			if ((type == "json"))
			{
				const utf8 &callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
				mode_ypstatus_json(callback);
			}
			else
			{
				mode_ypstatus_xml();
			}
		}
		else if (mode == "sources")
		{
			mode_sources(m_hostIP);
		}
		else if (mode == "adgroups")
		{
			mode_adgroups();
		}
		else if (mode == "debug")
		{
			const utf8 &option = mapGet(m_httpRequestInfo.m_QueryParameters, "option", (utf8)"");
			if (!option.empty() && !adminRefer)
			{
				// prevent direct access to being able to edit this
				// and force a redirection to the non-param version
				sendMessageAndClose(redirect("admin.cgi?mode=debug", SHRINK));
				return;
			}

			const utf8 &on_off = mapGet(m_httpRequestInfo.m_QueryParameters, "on", (utf8)"");
			mode_debug(option, (on_off == "true"), adminRefer);
		}
		else if (mode == "help")
		{
			mode_help();
		}
		else if (mode == "config")
		{
			mode_config();
		}
#if 0
		else if (mode == "logs")
		{
			mode_logs(result);
		}
#endif
		else if (mode == "version")
		{
			// only allow from the admin page to avoid possible spamming sttempts
			if (adminRefer)
			{
				checkVersion(m_lastActivityTime);
				sendMessageAndClose(redirect("admin.cgi?sid=0&refresh=-3", SHRINK));
			}
			else
			{
				sendMessageAndClose(MSG_200);
			}
		}
		else
		{
			if (m_noSID)
			{
				// do a sanity check so that we're only accessing this with the true adminpassword
				if (m_zeroSID)
				{
					mode_summary(mapGet(m_httpRequestInfo.m_QueryParameters, "refresh", 0));
				}
				// not matching the master password so show the simple summary
				else
				{
					sendMessageAndClose(redirect("index.html", SHRINK));
				}
			}
			else
			{
				const int refresh = mapGet(m_httpRequestInfo.m_QueryParameters, "refresh", 0);
				mode_none(m_sid, refresh);
			}
		}
	}
	else
	{
		sendMessageAndClose(MSG_AUTHFAILURE401 + utf8(!HEAD_REQUEST ?
							"<html><head>Unauthorized<title>Shoutcast "
							"Administrator</title></head></html>" : ""));
	}
}

void protocol_admincgi::state_SendFileHeader() throw(std::exception)
{
	if ((!m_saveLogFile && SHRINK) &&
		compressDataStart(m_logFileBodyPrefix, &m_stream, (Bytef*)"sc_serv.log\0", false))
	{
		m_logFileHeader += "Content-Encoding:gzip\r\n";
	}
	m_logFileHeader += "\r\n";

	m_outMsg = m_logFileHeader + (!m_saveLogFile ? tohex(m_logFileBodyPrefix.size()) + "\r\n" + m_logFileBodyPrefix + "\r\n" : "");
	m_outBuffer = m_outMsg.c_str();
	bandWidth::updateAmount(bandWidth::PRIVATE_WEB, (m_outBufferSize = (int)m_outMsg.size()));
	m_state = &protocol_admincgi::state_Send;
	m_nextState = &protocol_admincgi::state_SendFileContents;
	m_result.write();

	if (m_tailLogFile)
	{
		m_result.read(fileno(m_logFile));
	}
}

void protocol_admincgi::state_SendFileFooter() throw(std::exception)
{
	if (SHRINK)
	{
		compressDataCont(m_logFileBodyFooter, &m_stream);
	}
	m_logFileBodyFooter = tohex(m_logFileBodyFooter.size()) + "\r\n" + m_logFileBodyFooter + "\r\n";
	m_outBuffer = m_logFileBodyFooter.c_str();
	bandWidth::updateAmount(bandWidth::PRIVATE_WEB, (m_outBufferSize = (int)m_logFileBodyFooter.size()));
	m_state = &protocol_admincgi::state_Send;
	m_nextState = &protocol_admincgi::state_SendFileEnd;
	m_result.write();
}

void protocol_admincgi::state_SendFileEnd() throw(std::exception)
{
	strncpy((char*)&(m_logFileBuffer[0]), "0\r\n\r\n", 1024);
	m_outBuffer = &(m_logFileBuffer[0]);
	m_outBufferSize = 5;
	m_nextState = &protocol_admincgi::state_Close;
	bandWidth::updateAmount(bandWidth::PRIVATE_WEB, m_outBufferSize);
	compressDataEnd(&m_stream);

	m_state = &protocol_admincgi::state_Send;
	m_nextState = &protocol_admincgi::state_Close;
	m_result.write();
}

// this will only be receiving an already converted
// string so no need to do the commented part again
utf8 protocol_admincgi::escapeText(const std::vector<uniString::utf8::value_type> &s) throw()
{
	utf8 result;
	result.resize(0);
	bool inHead = false;
	int headCount = 0;
	const size_t amt = s.size();

	for (size_t x = 0; x < amt; ++x)
	{
		if ((s[x] == '*') && (((x + 1) < amt) && (s[x + 1] == '*')))
		{
			inHead = true;
		}
		else if (s[x] == '\n')
		{
			if (!inHead)
			{
				if ((((x + 1) < amt) && isdigit(s[x + 1])))
				{
					if (inMsg)
					{
						result += "</b>";
					}
					result += "\n";
					inMsg = false;
				}
				else
				{
					if (((x + 1) == amt))
					{
						if (inMsg)
						{
							result += "</b>";
						}
						result += "\n";
						inMsg = false;
					}
					else
					{
						result += "\n\t\t\t";
					}
				}
			}
			else
			{
				++headCount;
			}
		}
		else if ((s[x] == 'D') &&
				(((x > 0) && (s[x - 1] == '\t')) ||
				(!x && (lastChar == '\t'))))
		{
			result += "<b class=\"d\">D";
			inMsg = true;
		}
		else if ((s[x] == 'E') &&
				(((x > 0) && (s[x - 1] == '\t')) ||
				(!x && (lastChar == '\t'))))
		{
			result += "<b class=\"e\">E";
			inMsg = true;
		}
		else if ((s[x] == 'W') &&
				(((x > 0) && (s[x - 1] == '\t')) ||
				(!x && (lastChar == '\t'))))
		{
			result += "<b class=\"w\">W";
			inMsg = true;
		}
		else if ((s[x] == 'U') &&
				(((x > 0) && (s[x-1] == '\t')) ||
				(!x && (lastChar == '\t'))))
		{
			result += "<b class=\"u\">U";
			inMsg = true;
		}
		else if ((s[x] == 'I') &&
				(((x > 0) && (s[x-1] == '\t')) ||
				(!x && (lastChar == '\t'))))
		{
			if (!inHead)
			{
				result += "<b class=\"i\">I";
				inMsg = true;
			}
		}
		else
		{
			if (!inHead)
			{
				if (s[x] == '<')
				{
					result += "&lt;";
				}
				else if (s[x] == '>')
				{
					result += "&gt;";
				}
				else if (s[x] == '&')
				{
					result += "&amp;";
				}
				else if (s[x] == '\'')
				{
					result += "&apos;";
				}
				else if (s[x] == '"')
				{
					result += "&quot;";
				}
				else
				{
					result += s[x];
				}
			}
			else
			{
				if (inHead && (headCount > 3) && (s[x] == '\t'))
				{
					inHead = false;
					headCount = 0;
					x += 5;
				}
			}
		}
		if (!s[x])
		{
			break;
		}
	}

	lastChar = s[amt - 1];
	return result;
}

void protocol_admincgi::state_SendFileContents() throw(std::exception)
{
	m_logFileBuffer.clear();
	m_logFileBuffer.resize(SEND_SIZE);
	const size_t amt = fread(&(m_logFileBuffer[0]), 1, (SEND_SIZE - 1), m_logFile);
	if (amt > 0)
	{
		static utf8 out;
		out.clear();

		if (!m_saveLogFile)
		{
			m_logFileBuffer.resize(amt);
			out = escapeText(m_logFileBuffer);
		}
		else
		{
			out = utf8(&(m_logFileBuffer[0]), amt);
		}

		if (!out.empty())
		{
			if (m_saveLogFile || (SHRINK))
			{
				if (m_saveLogFile == 1)
				{
					if (compressDataStart(out, &m_stream, (Bytef*)m_logFileName.c_str()))
					{
						m_saveLogFile = 2;
					}
				}
				else
				{
					compressDataCont(out, &m_stream);
				}
			}
			out = tohex(out.size()) + "\r\n" + out + "\r\n";
			m_outBuffer = out.c_str();
			bandWidth::updateAmount(bandWidth::PRIVATE_WEB, (m_outBufferSize = (int)out.size()));
			m_nextState = &protocol_admincgi::state_SendFileContents;
		}
		else
		{
			m_nextState = (m_saveLogFile ? &protocol_admincgi::state_SendFileEnd : &protocol_admincgi::state_SendFileFooter);
		}

		m_state = &protocol_admincgi::state_Send;
		m_result.write();

		if (m_tailLogFile)
		{
			m_result.timeout(1);
			m_result.read(fileno(m_logFile));
		}
	}
	else if (ferror(m_logFile) || !m_logFile)
	{
		m_state = &protocol_admincgi::state_Close;
		m_result.run();
	}
	else if (feof(m_logFile) && (!m_tailLogFile))
	{
		if (m_saveLogFile)
		{
			static utf8 out;
			out.clear();
			compressDataFinish(out, &m_stream);
			out = tohex(out.size()) + "\r\n" + out + "\r\n";
			m_outBuffer = out.c_str();
			bandWidth::updateAmount(bandWidth::PRIVATE_WEB, (m_outBufferSize = (int)out.size()));

			m_state = &protocol_admincgi::state_Send;
			m_nextState = &protocol_admincgi::state_SendFileEnd;
		}
		else
		{
			m_state = &protocol_admincgi::state_SendFileFooter;
		}

		m_result.write();
		m_result.run();
	}
	else
	{
		m_result.timeout(10);
	}
}

// display log
void protocol_admincgi::mode_viewlog(const streamData::streamID_t sid, const bool tail, const bool save, const bool server) throw()
{
	if (!save)
	{
		utf8 headerTitle = utf8(utf8("Server Log") + (tail ? " (Tailing)":""));
		m_logFileHeader = MSG_NO_CLOSE_200;

		m_logFileBodyPrefix += (server ? getServerAdminHeader(headerTitle) :
										 getStreamAdminHeader(sid, headerTitle)) + getUptimeScript() +
								(tail ? "<div style=\"padding:1em;\"><b>Showing log output from the current log "
										"position and onwards. Click <a href=\"admin.cgi?mode=viewlog&amp;server=" +
										randomId(logTailId) + "\">here</a> to view the complete log output or <a "
										"href=\"admin.cgi?mode=viewlog&amp;server=" + logId + "&amp;viewlog=save\">here</a> "
										"to save the complete log output. When tailing the log output, there may be "
										"a delay before any new log output will appear and it may also stop updating if "
										"there is no log activity depending on configured timeouts. Reload the page if this "
										"should happens.</b></div>" : "") + getIEFlexFix() +
								"<pre id=\"log\" style=\"tab-size:16;-moz-tab-size:16;-o-tab-size:16;\"><font class=\"t\">";

		// doing to just make sure that the end of the output should be correct
		m_logFileBodyFooter = "</font>\n</pre>" + getfooterStr();
		m_tailLogFile = tail;

		if (server || !(gOptions.read_stream_adminPassword(sid) && !gOptions.stream_adminPassword(sid).empty()))
		{
			const utf8 &file = mapGet(m_httpRequestInfo.m_QueryParameters, "file", gOptions.realLogFile());

			m_logFile = uniFile::fopen(file, "r");
			if (m_logFile)
			{
				m_logFileName = fileUtil::stripPath(file);
				m_logFileName.push_back('\0');

				if (m_tailLogFile)
				{
					::fseek(m_logFile, 0, SEEK_END);
				}
			}
		}
		else
		{
			utf8 body = m_logFileBodyPrefix + "Viewing Not Allowed With Current Permissions</pre>" + m_logFileBodyFooter;
			COMPRESS(m_logFileHeader, body);
			m_logFileHeader += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
			sendMessageAndClose(m_logFileHeader + (!HEAD_REQUEST ? body : ""));
			return;
		}

		if (!m_logFile)
		{
			utf8 body = m_logFileBodyPrefix + "Log File Not Found (" +
						stripWhitespace(errMessage()) + ")</pre>" +
						m_logFileBodyFooter;
			COMPRESS(m_logFileHeader, body);
			m_logFileHeader += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
			sendMessageAndClose(m_logFileHeader + (!HEAD_REQUEST ? body : ""));
		}
		else
		{
			m_logFileHeader += "Transfer-Encoding:chunked\r\n";
			m_state = &protocol_admincgi::state_SendFileHeader;
			m_result.run();
		}
	}
	else
	{
		if (server || !(gOptions.read_stream_adminPassword(sid) && !gOptions.stream_adminPassword(sid).empty()))
		{
			const utf8 &file = mapGet(m_httpRequestInfo.m_QueryParameters, "file", gOptions.realLogFile());

			m_logFile = uniFile::fopen(file, "r");
			if (m_logFile)
			{
				m_logFileName = fileUtil::stripPath(file);
				m_logFileHeader = "HTTP/1.1 200 OK\r\n"
								  "Content-Type:application/x-gzip-compressed\r\n"
								  "Content-Disposition:attachment;filename=\"" +
								  m_logFileName + ".gz\"\r\n";
				m_saveLogFile = true;
			}
		}
		else
		{
			sendMessageAndClose(MSG_HTTP403);
			return;
		}

		if (!m_logFile)
		{
			sendMessageAndClose(MSG_HTTP404);
		}
		else
		{
			m_logFileHeader += "Transfer-Encoding:chunked\r\n";
			m_state = &protocol_admincgi::state_SendFileHeader;
			m_result.run();
		}
	}
}

// shown played history (as is also shown on the public pages if enabled)
void protocol_admincgi::mode_history(const streamData::streamID_t sid) throw()
{
	utf8 header = MSG_NO_CLOSE_200,
		 body = getStreamAdminHeader(sid, "Stream History") +
				"<table width=\"100%\" border=\"0\" cellpadding=\"0\" "
				"cellspacing=\"0\"><tr valign=\"top\"><td>" +
				protocol_HTTPStyle::getPlayedBody(sid) + "</table>" +
				getUptimeScript() + getIEFlexFix() + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

// remove an IP from the rip list
void protocol_admincgi::mode_unrip(const streamData::streamID_t sid, const utf8 &ripAddr, const utf8 &rawIpAddr) throw()
{
	utf8 msg;
	try
	{
		size_t stream_ID = ((gOptions.read_stream_ripFile(sid) && !gOptions.stream_ripFile(sid).empty()) ? sid : 0);
		if (isAddress(ripAddr))
		{
			bool ret = g_ripList.remove(ripAddr,stream_ID,false), usingRaw = false;
			if (!ret && !rawIpAddr.empty())
			{
				ret = g_ripList.remove(rawIpAddr,stream_ID,false);
				if (ret) usingRaw = true;
			}
			if (ret)
			{
				ILOG("[RIP] Removed `" + (!usingRaw ? ripAddr : rawIpAddr) + "' from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip list");

				if (gOptions.saveRipListOnExit())
				{
					if (stream_ID && gOptions.read_stream_ripFile(stream_ID) && !gOptions.stream_ripFile(stream_ID).empty())
					{
						g_ripList.save(gOptions.stream_ripFile(stream_ID),stream_ID);
					}
					else
					{
						g_ripList.save(gOptions.ripFile(),0);
					}
				}

				stats::updateRipClients(stream_ID, (!usingRaw ? ripAddr : rawIpAddr), false);
			}
			else
			{
				ILOG("[RIP] Unable to remove `" + ripAddr + "' from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip list");
			}
		}
		else
		{
			ILOG("[RIP] `" + ripAddr + "' is not a valid value. Skipping removing from the " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip list");
		}

		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer == "admin.cgi?sid=" + tos(sid) ? "" : "&mode=viewrip"), SHRINK) : MSG_200);
	}
	catch(const exception &ex)
	{
		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer != utf8("admin.cgi?sid=" + tos(sid) + "&mode=viewrip") ? "" : "&mode=viewrip"), SHRINK) : MSG_200);
		ELOG(ex.what());
	}

	sendMessageAndClose(msg);
}

// add an IP / hostname to the rip list
void protocol_admincgi::mode_rip(const streamData::streamID_t sid, const utf8 &ripAddr, const utf8 &rawIpAddr) throw()
{
	utf8 msg;
	try
	{
		const size_t stream_ID = ((gOptions.read_stream_ripFile(sid) && !gOptions.stream_ripFile(sid).empty()) ? sid : 0);
		if (isAddress(ripAddr))
		{
			if (!g_ripList.find(ripAddr,stream_ID))
			{
				bool added = g_ripList.add(ripAddr,stream_ID, true), usingRaw = false;
				if (!added && !rawIpAddr.empty())
				{
					if (g_ripList.add(rawIpAddr,stream_ID, false))
					{
						usingRaw = true;
					}
				}

				ILOG("[RIP] Added `" + (!usingRaw ? ripAddr : rawIpAddr) + "' to " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip list");

				if (gOptions.saveRipListOnExit())
				{
					if (stream_ID && gOptions.read_stream_ripFile(stream_ID) && !gOptions.stream_ripFile(stream_ID).empty())
					{
						g_ripList.save(gOptions.stream_ripFile(stream_ID),stream_ID);
					}
					else
					{
						g_ripList.save(gOptions.ripFile(),0);
					}
				}
				stats::updateRipClients(stream_ID, (!usingRaw ? ripAddr : rawIpAddr), true);
			}
			else
			{
				ILOG("[RIP] `" + ripAddr + "' already in the " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip list");
			}
		}
		else
		{
			ILOG("[RIP] `" + ripAddr + "' is not a valid value. Skipping adding to the " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip list");
		}

		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer == "admin.cgi?sid=" + tos(sid) ? "" : "&mode=viewrip"), SHRINK) : MSG_200);
	}
	catch(const exception &ex)
	{
		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer != utf8("admin.cgi?sid=" + tos(sid) + "&mode=viewrip") ? "" : "&mode=viewrip"), SHRINK) : MSG_200);
		ELOG(ex.what());
	}

	sendMessageAndClose(msg);
}

// show rip list
void protocol_admincgi::mode_viewrip(const streamData::streamID_t sid) throw()
{
	vector<ripList::rip_t> rip_list;
	g_ripList.get(rip_list,((gOptions.read_stream_ripFile(sid) && !gOptions.stream_ripFile(sid).empty()) ? sid : 0));

	utf8 header = MSG_NO_CLOSE_200,
		 headerTitle = (!sid ? "Server Reserved List" : "Stream Reserved List"),
		 body = (!sid ? getServerAdminHeader(headerTitle) : getStreamAdminHeader(sid, headerTitle)) +
				"<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr valign=\"top\"><td>";

	if (rip_list.empty())
	{
		body += "<b>&nbsp;No Reserved Entries</b><br>";
	}
	else
	{
		body += "<b>&nbsp;Reserved Entry List:</b><ol>";
		for (vector<ripList::rip_t>::const_iterator i = rip_list.begin(); i != rip_list.end(); ++i)
		{
			body += "<li><b>" + aolxml::escapeXML((*i).m_numericIP) + "</b>" +
					(!(*i).m_hostIP.empty() ? " (" + aolxml::escapeXML((*i).m_hostIP) + ")" : "") +
					" - <a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=unripdst&amp;ripdst=" +
					urlUtils::escapeURI_RFC3986((*i).m_numericIP) + "\">remove</a>";
		}
		body += "</ol>";
	}

	body +=
		"</td><td style=\"padding:0 1em 0 1em;\"><br>"
		"<table class=\"ent\" border=\"0\" cellspacing=\"0\" cellpadding=\"3\" align=\"right\">"
		"<tr class=\"ent\">"
		"<td class=\"inp\" align=\"center\">Reserve&nbsp;Connection Slot by IP</td>"
		"</tr>"
		"<form method=\"url\" action=\"admin.cgi\">"
		"<tr>"
		"<td><table cellspacing=\"0\" cellpadding=\"3\" border=\"0\">"
		"<tr>"
		"<td align=\"center\">Enter the IP address:<br><i>(example: 127.0.0.1)</i></td>"
		"</tr>"
		"<tr>"
		"<td align=\"center\">"
		"<input name=\"mode\" value=\"ripip\" type=\"hidden\">"
		"<input name=\"sid\" value=\"" + tos(sid) + "\" type=\"hidden\">"
		"<input name=\"ip1\" size=\"3\" maxlength=\"3\">."
		"<input name=\"ip2\" size=\"3\" maxlength=\"3\">."
		"<input name=\"ip3\" size=\"3\" maxlength=\"3\">."
		"<input name=\"ip4\" size=\"3\" maxlength=\"3\">"
		"</td>"
		"</tr>"
		"<tr>"
		"<td colspan=\"2\" align=\"center\"><input class=\"submit\" type=\"submit\" value=\"Reserve IP\">"
		"&nbsp;<input class=\"submit\" value=\"Clear\" type=\"Reset\"></td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</table>"
		"</td>"
		"<td style=\"padding: 0 1em 0 0;\"><br>"
		"<table class=\"ent\" border=\"0\" cellspacing=\"0\" cellpadding=\"3\" align=\"left\">"
		"<tr class=\"ent\">"
		"<td class=\"inp\" align=\"center\">Reserve Connection Slot by Host</td>"
		"</tr>"
		"<form method=\"url\" action=\"admin.cgi\">"
		"<tr>"
		"<td><table cellspacing=\"0\" cellpadding=\"3\" border=\"0\">"
		"<tr>"
		"<td align=\"center\">Enter the hostname:<br><i>(example: my.example.com)</i></td>"
		"</tr>"
		"<tr>"
		"<td align=\"center\">"
		"<input name=\"mode\" value=\"ripip\" type=\"hidden\">"
		"<input name=\"sid\" value=\"" + tos(sid) + "\" type=\"hidden\">"
		"<input name=\"ripdstraw\" size=\"30\" maxlength=\"256\">"
		"</td>"
		"</tr>"
		"<tr>"
		"<td colspan=\"2\" align=\"center\"><input class=\"submit\" type=\"submit\" value=\"Reserve Host\">"
		"&nbsp;<input class=\"submit\" value=\"Clear\" type=\"Reset\"></td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</form>"
		"</table>" +
		getUptimeScript() +
		getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

// show ban lists
void protocol_admincgi::mode_viewban(const streamData::streamID_t sid) throw()
{
	vector<banList::ban_t> ban_list;
	g_banList.get(ban_list,((gOptions.read_stream_banFile(sid) && !gOptions.stream_banFile(sid).empty()) ? sid : 0));

	utf8 header = MSG_NO_CLOSE_200,
		 headerTitle = (!sid ? "Server Ban List" : "Stream Ban List"),
		 body = (!sid ? getServerAdminHeader(headerTitle) : getStreamAdminHeader(sid, headerTitle)) +
				"<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr valign=\"top\"><td>";

	if (ban_list.empty())
	{
		body += "<b>&nbsp;No Banned Entries</b><br>";
	}
	else
	{
		body += "<b>&nbsp;Ban Entry List:</b><ol>";
		for (vector<banList::ban_t>::const_iterator i = ban_list.begin(); i != ban_list.end(); ++i)
		{
			body += "<li><b>" + aolxml::escapeXML((*i).m_numericIP) + "</b>" +
					(!(*i).m_comment.empty() ? " : <b>" + aolxml::escapeXML((*i).m_comment) + "</b>" : "") +
					" - " + ((*i).m_mask == 255 ? "Single&nbsp;IP" : "Subnet") + "&nbsp;ban&nbsp;-&nbsp;"
					"<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=unbandst&amp;bandst=" +
					urlUtils::escapeURI_RFC3986((*i).m_numericIP) + "&amp;banmsk=" + tos((*i).m_mask) + "\">remove</a>";
		}
		body += "</ol>";
	}

	body +=
		"</td><td style=\"padding:0 1em 0 1em;\"><br>"
		"<table class=\"ent\" border=\"0\" cellspacing=\"0\" cellpadding=\"3\" align=\"right\">"
		"<tr class=\"ent\">"
		"<td class=\"inp\" align=\"center\">Ban a Single IP</td>"
		"</tr>"
		"<form method=\"url\" action=\"admin.cgi\">"
		"<tr>"
		"<td><table cellspacing=\"0\" cellpadding=\"3\" border=\"0\">"
		"<tr>"
		"<td align=\"center\">Enter&nbsp;the&nbsp;IP&nbsp;address:<br><i>(example: 127.0.0.1)</i></td>"
		"</tr>"
		"<tr>"
		"<td align=\"center\">"
		"<input name=\"mode\" value=\"banip\" type=\"hidden\">"
		"<input name=\"sid\" value=\"" + tos(sid) + "\" type=\"hidden\">"
		"<input name=\"ip1\" size=\"3\" maxlength=\"3\">."
		"<input name=\"ip2\" size=\"3\" maxlength=\"3\">."
		"<input name=\"ip3\" size=\"3\" maxlength=\"3\">."
		"<input name=\"ip4\" size=\"3\" maxlength=\"3\">"
		"<input type=\"hidden\" name=\"banmsk\" value=\"255\"></td>"
		"</tr>"
		"<tr>"
		"<td colspan=\"2\" align=\"center\"><input class=\"submit\" type=\"submit\" value=\"Ban Single IP\">"
		"&nbsp;<input class=\"submit\" value=\"Clear\" type=\"Reset\"></td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</form>"
		"</table>"
		"</td>"
		"<td style=\"padding: 0 1em 0 0;\"><br>"
		"<table class=\"ent\" border=\"0\" cellspacing=\"0\" cellpadding=\"3\" align=\"left\">"
		"<tr class=\"ent\">"
		"<td class=\"inp\" align=\"center\">Ban an Entire Subnet</td>"
		"</tr>"
		"<form method=\"url\" action=\"admin.cgi\">"
		"<tr>"
		"<td><table cellspacing=\"0\" cellpadding=\"3\" border=\"0\">"
		"<tr>"
		"<td align=\"center\">Enter the Subnet address:<br><i>(example: 255.255.255)</i></td>"
		"</tr>"
		"<tr>"
		"<td align=\"center\">"
		"<input name=\"mode\" value=\"banip\" type=\"hidden\">"
		"<input name=\"sid\" value=\"" + tos(sid) + "\" type=\"hidden\">"
		"<input name=\"ip1\" size=\"1\" maxlength=\"3\">."
		"<input name=\"ip2\" size=\"1\" maxlength=\"3\">."
		"<input name=\"ip3\" size=\"1\" maxlength=\"3\">.0-255"
		"<input name=\"ip4\" value=\"0\" type=\"hidden\">"
		"<input type=\"hidden\" name=\"banmsk\" value=\"0\"></td>"
		"</tr>"
		"<tr>"
		"<td colspan=\"2\" align=\"center\"><input class=\"submit\" type=\"submit\" value=\"Ban Whole Subnet\">"
		"&nbsp;<input class=\"submit\" value=\"Clear\" type=\"Reset\"></td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</form>"
		"</table>" +
		getUptimeScript() +
		getIEFlexFix() +
		getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

// remove an IP and mask from the ban list
void protocol_admincgi::mode_unban(const streamData::streamID_t sid, const utf8 &banAddr, const int banMask) throw()
{
	utf8 msg;
	try
	{
		const size_t stream_ID = ((gOptions.read_stream_banFile(sid) && !gOptions.stream_banFile(sid).empty()) ? sid : 0);
		if (isAddress(banAddr))
		{
			if (g_banList.remove(banAddr,banMask,stream_ID,false))
			{
				ILOG("[BAN] Removed `" + banAddr + "/" + tos(banMask) + "' from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban list");

				if (gOptions.saveBanListOnExit())
				{
					if (stream_ID && gOptions.read_stream_banFile(stream_ID) && !gOptions.stream_banFile(stream_ID).empty())
					{
						g_banList.save(gOptions.stream_banFile(stream_ID),stream_ID);
					}
					else
					{
						g_banList.save(gOptions.banFile(),0);
					}
				}
			}
			else
			{
				ILOG("[BAN] Unable to remove `" + banAddr + "' from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban list");
			}
		}
		else
		{
			ILOG("[BAN] `" + banAddr + "' is not a valid value. Skipping removing from the " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban list");
		}

		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer == "admin.cgi?sid=" + tos(sid) ? "" : "&mode=viewban"), SHRINK) : MSG_200);
	}
	catch(const exception &ex)
	{
		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer != utf8("admin.cgi?sid=" + tos(sid) + "&mode=viewban") ? "" : "&mode=viewban"), SHRINK) : MSG_200);
		ELOG(ex.what());
	}

	sendMessageAndClose(msg);
}

// add an IP and mask to the ban list
void protocol_admincgi::mode_ban(const streamData::streamID_t sid, const utf8 &banAddrs, const int banMask) throw()
{
	utf8 msg;
	try
	{
		const size_t stream_ID = ((gOptions.read_stream_banFile(sid) && !gOptions.stream_banFile(sid).empty()) ? sid : 0);
		if (!g_banList.find(banAddrs,banMask,stream_ID))
		{
			g_banList.add(banAddrs,banMask,"",stream_ID);
			ILOG("[BAN] Added `" + banAddrs + "/" + tos(banMask) + "' to " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban list");

			if (gOptions.saveBanListOnExit())
			{
				if (stream_ID && gOptions.read_stream_banFile(stream_ID) && !gOptions.stream_banFile(stream_ID).empty())
				{
					g_banList.save(gOptions.stream_banFile(stream_ID),stream_ID);
				}
				else
				{
					g_banList.save(gOptions.banFile(),0);
				}
			}
		}
		else
		{
			ILOG("[BAN] `" + banAddrs + "' already in the " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban list");
		}

		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer == "admin.cgi?sid=" + tos(sid) ? "" : "&mode=viewban"), SHRINK) : MSG_200);
	}
	catch(const exception &ex)
	{
		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer != utf8("admin.cgi?sid=" + tos(sid) + "&mode=viewban") ? "" : "&mode=viewban"), SHRINK) : MSG_200);
		ELOG(ex.what());
	}

	// additionally when a ban happens then we attempt to kick the client(s) at the same time
	const utf8 &p1 = mapGet(m_httpRequestInfo.m_QueryParameters, "kickdst", (utf8)"");
	if (!p1.empty())
	{
		// split out multiple clients to kick (split by a ,)
		std::vector<uniString::utf8> addrs = tokenizer(p1, ',');
		for (vector<uniString::utf8>::const_iterator i = addrs.begin(); i != addrs.end(); ++i)
		{
			if ((*i).find(utf8(".")) == utf8::npos)
			{
				stats::kickClient(sid, atoi((*i).hideAsString().c_str()));
			}
			else
			{
				stats::kickClient(sid, (*i));
			}
		}
	}

	sendMessageAndClose(msg);
}

// remove an agent from the agent list
void protocol_admincgi::mode_unagent(const streamData::streamID_t sid, const utf8 &agent) throw()
{
	utf8 msg;
	try
	{
		const size_t stream_ID = ((gOptions.read_stream_agentFile(sid) && !gOptions.stream_agentFile(sid).empty()) ? sid : 0);
		bool ret = g_agentList.remove(agent,stream_ID,false);
		if (!ret && !agent.empty())
		{
			ret = g_agentList.remove(agent,stream_ID,false);
		}
		if (ret)
		{
			ILOG("[AGENT] Removed `" + agent + "' from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " user agent list");

			if (gOptions.saveAgentListOnExit())
			{
				if (stream_ID && gOptions.read_stream_agentFile(stream_ID) && !gOptions.stream_agentFile(stream_ID).empty())
				{
					g_agentList.save(gOptions.stream_agentFile(stream_ID),stream_ID);
				}
				else
				{
					g_agentList.save(gOptions.agentFile(),0);
				}
			}
		}
		else
		{
			ILOG("[AGENT] Unable to remove `" + agent + "' from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " user agent list");
		}

		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer == "admin.cgi?sid=" + tos(sid) ? "" : "&mode=viewagent"), SHRINK) : MSG_200);
	}
	catch(const exception &ex)
	{
		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer != utf8("admin.cgi?sid=" + tos(sid) + "&mode=viewagent") ? "" : "&mode=viewagent"), SHRINK) : MSG_200);
		ELOG(ex.what());
	}

	sendMessageAndClose(msg);
}

// add an IP / hostname to the user agent list
void protocol_admincgi::mode_agent(const streamData::streamID_t sid, const utf8 &agent) throw()
{
	utf8 msg;
	try
	{
		const size_t stream_ID = ((gOptions.read_stream_agentFile(sid) && !gOptions.stream_agentFile(sid).empty()) ? sid : 0);
		if (!g_agentList.find(agent,stream_ID))
		{
			if (g_agentList.add(agent, stream_ID, true))
			{
				ILOG("[AGENT] Added `" + agent + "' to " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " user agent list");					
			}

			stats::kickClientList_t kick_data;
			stats::getClientDataForKicking(stream_ID, kick_data);
			for (stats::kickClientList_t::const_iterator i = kick_data.begin(); i != kick_data.end(); ++i)
			{
				if (!(*i)->m_kicked && ((*i)->m_userAgent == agent))
				{
					stats::kickClient(sid, (*i)->m_unique);
				}
				delete (*i);
			}

			if (gOptions.saveAgentListOnExit())
			{
				if (stream_ID && gOptions.read_stream_agentFile(stream_ID) && !gOptions.stream_agentFile(stream_ID).empty())
				{
					g_agentList.save(gOptions.stream_agentFile(stream_ID),stream_ID);
				}
				else
				{
					g_agentList.save(gOptions.agentFile(),0);
				}
			}
		}
		else
		{
			ILOG("[AGENT] `" + agent + "' already in the " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " user agent list");
		}
		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer == "admin.cgi?sid=" + tos(sid) ? "" : "&mode=viewagent"), SHRINK) : MSG_200);
	}
	catch(const exception &ex)
	{
		msg = (!m_referer.empty() ? redirect("admin.cgi?sid=" + tos(sid) + (m_referer != utf8("admin.cgi?sid=" + tos(sid) + "&mode=viewagent") ? "" : "&mode=viewagent"), SHRINK) : MSG_200);
		ELOG(ex.what());
	}

	sendMessageAndClose(msg);
}

// show agent list
void protocol_admincgi::mode_viewagent(const streamData::streamID_t sid) throw()
{
	vector<agentList::agent_t> agent_list;
	g_agentList.get(agent_list,((gOptions.read_stream_agentFile(sid) && !gOptions.stream_agentFile(sid).empty()) ? sid : 0));

	utf8 header = MSG_NO_CLOSE_200,
		 headerTitle = (!sid ? "Server Blocked User Agent List" :
							   "Stream Blocked User Agent List"),
		 body = (!sid ? getServerAdminHeader(headerTitle, 0, "", 2) : getStreamAdminHeader(sid, headerTitle, 0, true)) +
				"<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr valign=\"top\"><td>";

	if (agent_list.empty())
	{
		body += "<b>&nbsp;No Blocked User Agents</b><br>";
	}
	else
	{
		body += "<b>&nbsp;Blocked User Agent List:</b><ol>";
		for (vector<agentList::agent_t>::const_iterator i = agent_list.begin(); i != agent_list.end(); ++i)
		{
			const streamData::source_t clientType = ((streamData::source_t)streamData::UNKNOWN);
			body += "<li>" + getClientImage(streamData::getClientType(clientType, stringUtil::toLower((*i).m_agent))) +
					" <b>" + aolxml::escapeXML((*i).m_agent) + "</b> - <a href=\"admin.cgi?sid=" + tos(sid) +
					"&amp;mode=unagent&amp;agent=" + urlUtils::escapeURI_RFC3986((*i).m_agent) + "\">remove</a>";
		}
		body += "</ol>";
	}

	body +=
		"</td>"
		"<td style=\"padding: 0 1em 0 0;\"><br>"
		"<table class=\"ent\" border=\"0\" cellspacing=\"0\" cellpadding=\"3\" align=\"left\">"
		"<tr class=\"ent\">"
		"<td class=\"inp\" align=\"center\">Block User Agent</td>"
		"</tr>"
		"<form method=\"url\" action=\"admin.cgi\">"
		"<tr>"
		"<td><table cellspacing=\"0\" cellpadding=\"3\" border=\"0\">"
		"<tr>"
		"<td align=\"center\">Enter the user agent:<br><i>(example: streamripper)</i></td>"
		"</tr>"
		"<tr>"
		"<td align=\"center\">"
		"<input name=\"mode\" value=\"agent\" type=\"hidden\">"
		"<input name=\"sid\" value=\"" + tos(sid) + "\" type=\"hidden\">"
		"<input name=\"agent\" size=\"30\" maxlength=\"256\">"
		"</td>"
		"</tr>"
		"<tr>"
		"<td colspan=\"2\" align=\"center\"><input class=\"submit\" type=\"submit\" value=\"Block User Agent\">"
		"&nbsp;<input class=\"submit\" value=\"Clear\" type=\"Reset\"></td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</form>"
		"</table>" +
		getUptimeScript() +
		getIEFlexFix() +
		getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

// kick client(s)
void protocol_admincgi::mode_kickdst(const streamData::streamID_t sid, const utf8 &kickAddrs) throw()
{
	bool refresh = false;
	if (kickAddrs == "all")
	{
		// check if this is set to kick all
		refresh = stats::kickAllClients(sid);
	}
	else if (kickAddrs == "duplicates")
	{
		// check if this is set to kick all
		// duplicates (doing oldest first).
		refresh = stats::kickDuplicateClients(sid);
	}
	else
	{
		// split out multiple clients to kick (split by a ,)
		std::vector<uniString::utf8> addrs = tokenizer(kickAddrs,',');
		for (vector<uniString::utf8>::const_iterator i = addrs.begin(); i != addrs.end(); ++i)
		{
			if ((*i).find(utf8(".")) == utf8::npos)
			{
				stats::kickClient(sid, atoi((*i).hideAsString().c_str()));
			}
			else
			{
				stats::kickClient(sid,(*i));
			}
		}
	}

	if (!m_referer.empty())
	{
		const utf8 check = ("admin.cgi?sid=" + tos(sid));
		// if the referer is the server summary page then we need to go back to it
		sendMessageAndClose(redirect((m_referer == check ? check : "admin.cgi?sid=0") + (refresh ? "&refresh=1" : ""), SHRINK));
	}
	else
	{
		sendMessageAndClose(MSG_200);
	}
}

void protocol_admincgi::mode_art(const streamData::streamID_t sid, const int mode) throw()
{
	utf8 header = "HTTP/1.1 200 OK\r\n", body;
	streamData *sd = streamData::accessStream(sid);
	if (sd)
	{
		vector<__uint8> sc21_albumart = (mode == 0 ? sd->streamAlbumArt() : sd->streamPlayingAlbumArt());
		if (!sc21_albumart.empty())
		{
			utf8 mimeType[] = {
				"image/jpeg",
				"image/png",
				"image/bmp",
				"image/gif"
			};
			const size_t mime = (mode == 0 ? sd->streamAlbumArtMime() : sd->streamPlayingAlbumArtMime());
			// if not in the valid range then don't report the mime type in the generated response
			if (mime < 4)
			{
				header += "Content-Type:" + mimeType[mime] + "\r\n";
			}
			body += utf8(&sc21_albumart[0],sc21_albumart.size());
		}
		sd->releaseStream();
	}
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_register(const streamData::streamID_t sid, const streamData::streamInfo &info) throw()
{
	// construct a temp id for this action
	utf8 tempId = "";
	randomId(tempId);

	bool loaded = false;
	streamData *sd = streamData::accessStream(sid);
	if (sd)
	{
		int addFailIgnore = 0, errorCode = 0;
		loaded = sd->YP2_addSuccessful(addFailIgnore, errorCode);
		sd->releaseStream();
	}

	utf8 header = "HTTP/1.1 200 OK\r\n"
				  "Content-Type:text/html;charset=utf-8\r\n"
				  "Cache-Control:no-cache\r\n",
		 authhash = (!info.m_authHash.empty() ? info.m_authHash.substr(0, 36) : (utf8)""),
		 body = getStreamAdminHeader(sid, "Stream Authhash") + "<br>"
				"<form id=\"processing\" method=\"GET\" autocomplete=\"off\" onsubmit=\"return validateForm();\">"

				"<div style=\"display:table;width:100%;\">"

				"<div style=\"padding:0 1em;display:inline-block;float:left;max-width:15em;\">"
				"<table class=\"ent\" cellpadding=\"15px\"><tr><td valign=\"top\">"
				"<div align=\"center\" class=\"infh\"><b>Information</b></div><div id=\"info\">"
				"This page allows you to enter or amend the authhash to be used for this stream."
				"<br><br><hr><br>Authhash information is now managed online. "
				"<a target=\"blank\" href=\"https://radiomanager.shoutcast.com\"><b>Login</b></a> "
				"to create an authhash or update the details of an existing authhash."

				"<br><br><hr><br>The same authhash should be used for all stream instances of a station "
				"(e.g. 128kbps MP3 and 64kbps AAC streams for 'Super Awesome Radio').<br><br>This also includes all DNAS "
				"providing the stream(s) for a station to ensure the correct listing of all stream instances."

				// TODO need to have a link to the account page...
				"<br><br><hr><br>If you remove an authhash by mistake then you can either recover it from your "
				"<a href=\"https://radiomanager.shoutcast.com\" target=\"_blank\"><b>Shoutcast account</b></a> or you will need "
				"to contact <a href=\"mailto:support@shoutcast.com?subject=Shoutcast%20Support\"><b>support</b></a> directly for "
				"assistance.</div></td></tr></table><br></div>"

				// TODO need to consider customising some of this ?? or just above ??
				"<div align=\"left\" id=\"page_info\" style=\"display:inline-block;max-width:25em;padding:0 1em;\">"
				"<div align=\"left\" id=\"header\"></div>"
				"<table id=\"details\" style=\"width:100%;\" align=\"left\" border=\"0\">"

				"<tr id=\"hide\" valign=\"top\"><td colspan=\"2\">" + utf8(!authhash.empty() &&
				loaded && (info.m_streamSampleRate > 0) && info.m_radionomyID.empty() ?
				warningImage(false) + "&nbsp;&nbsp;<b>Please Register Your Authhash</b><br><br>" : "") +

				"<div style=\"float:right;padding:0 1em;\">"
				"<input style=\"white-space:normal;width:6em;\" id=\"register\" class=\"submit\" type=\"button\" "
						"onclick=\"window.open('https://radiomanager.shoutcast.com','_blank','')\" value=\"" +

				(!authhash.empty() ? "Manage Authhash\"></div>"
									 "The authhash currently configured for this stream is <b>" + authhash + "</b><br><br><br>"
									 "To change the authhash, enter it below and click 'Save'." :
									 "Create Authhash\"></div>"
									 "An authhash needs to be created for this stream.<br><br><br>"
									 "If you have just created an authhash via your "
									 "<a target=\"blank\" href=\"https://radiomanager.shoutcast.com\"><b>account</b></a> "
									 "or have an existing one for the stream, please enter it below and click 'Save'.") +

				"<br><br></td></tr>"
				"<tr id=\"hide\">"
				"<td style=\"width:25%;\" align=\"right\">Authhash:</td>"
				"<td colspan=\"2\"><input type=\"text\" style=\"width:15em;\" name=\"authhash\" id=\"authhash\" maxlength=\"36\" value=\"" + authhash + "\"></td>"
				"</tr>"
				"<tr>"
				"<td id=\"status\" align=\"center\" colspan=\"3\"><br>"
				"<input id=\"submit\" class=\"submit\" type=\"submit\" value=\"Save\">&nbsp;&nbsp;"
				"<input id=\"clear\" class=\"submit\" value=\"Clear\" type=\"button\" "
				"onclick=\"if(confirm('Clear the authhash and save the change now?"
				"\\n\\nChoose Cancel to just clear the field.')){window.location='admin.cgi?sid=" + tos(sid) +
				"&amp;mode=register&amp;register=clear';}else{$('authhash').value = '';authhashChange();}\">&nbsp;&nbsp;"
				"<input class=\"submit\" value=\"Cancel\" type=\"button\" onclick=\"window.location='admin.cgi?sid=" + tos(sid) + "';\"></td>"
				"</tr>"
				"</table>"
				"</div>"
				"</div>"
				"</form>"

				"<script type=\"text/javascript\">"
				"var original = \"" + /*(((mode == 3) && (auth_enabled != 1)) ?*/ info.m_authHash.substr(0, 36) /*: (utf8)"")*/ + "\";" EL

				"var timeout, response;" EL
				"function $(id){return document.getElementById(id);}" EL
				"function trimString(str){return str.replace(/^\\s+|\\s+$/g,'');}" EL
				+ utf8(!authhash.empty() ?
					"function changeExisting(){" EL
					"if($('existing').value != \"select\"){" EL
					"$('authhash').value = $('existing').value;" EL
					"}" EL
					"authhashChange();" EL
					"}" EL
					: "")
				+

				"function getHTTP(){" EL
				"if(window.XDomainRequest){" EL
				"return new XDomainRequest();" EL
				"}else if(window.XMLHttpRequest){" EL
				"return new XMLHttpRequest();" EL
				"}else{" EL
				"return new ActiveXObject(\"Microsoft.XMLHTTP\");" EL
				"}" EL
				"}" EL

				"function isValidAuthhash(str){" EL
				"var regexp = /^[-a-fA-F\\d]+$/;" EL
				"if(str != ''){" EL
				"return regexp.test(str);" EL
				"}" EL
				"return true;" EL
				"}" EL

				"function authhashChange(){" EL
				"var str = trimString($('authhash').value);" EL
				"var valid = isValidAuthhash(str);" EL
				"$('authhash').style.borderColor = (!(str.length == 0 || str.length > 36 && valid)?\"red\":\"\");" EL
				"$('submit').disabled = (!((str.length == 0 || str.length == 36) && (original != $('authhash').value) && valid));" EL
				"}" EL

				"function validateForm(){" EL
				"while($('hide') != null){" EL
				"$('hide').style.display = \"none\";" EL
				"$('hide').removeAttribute(\"id\");" EL
				"}" EL
				"$('status').setAttribute(\"colspan\",\"3\");" EL
				"$('status').setAttribute(\"align\",\"left\");" EL
				"$('status').setAttribute(\"valign\",\"middle\");" EL
				"$('status').innerHTML = \"</td><td><b>Processing</b><br><br>This may take a while.</td>\";" EL

				"var f = $('processing');" EL
				"var params=\"\";" EL
				"for(var i = 0; i < f.elements.length; i++ ){" EL
				"if(f.elements[i].name != \"\"){" EL
				"if(f.elements[i].name != \"private\" || (f.elements[i].name == \"private\" && f.elements[i].value == \"1\")){" EL
				"params += (i != 0 ? \"&\" : \"\") + f.elements[i].name + \"=\" + encodeURIComponent(f.elements[i].value);" EL
				"}" EL
				"}" EL
				"}" EL
				"if(params==\"\"){" EL
				"$('status').setAttribute(\"colspan\",\"3\");" EL
				"$('status').setAttribute(\"align\",\"center\");" EL
				"$('status').setAttribute(\"valign\",\"middle\");" EL
				"$('status').innerHTML=\"Critical error in processing request."
				"<br><br><b><a href=\\\"admin.cgi?sid=" + tos(sid) + "\\\">Click here to return to the Server Summary.</a></b>\";"
				"return false;"
				"}" EL

				"var xmlhttp = getHTTP();" EL
				"xmlhttp.open(\"GET\",\"admin.cgi?sid=" + tos(sid) + "&pass=" + gOptions.adminPassword() +
				"&mode=manualauthhash&tempid=" + tempId + "&\"+params,true);" EL
				"if(window.XDomainRequest){" EL
				"xmlhttp.onerror=xmlhttp.onload=function(){" EL
				"var code = parseInt((xmlhttp.responseText!=\"\"?xmlhttp.responseText:\"200\"));" EL
				
				"if(code!=200){" EL
				"clearInterval(timeout);" EL
				"$('status').setAttribute(\"align\",\"center\");" EL
				"$('status').setAttribute(\"colspan\",\"2\");" EL
				"if(code==0){" EL
				"$('status').innerHTML = \"</td><td>"
				"<b><br>Error Code: \"+code+\".<br>Check the DNAS is running and there is a working network connection.<br>"
				"<br><a href=\\\"admin.cgi?sid=" + tos(sid) + "\\\">Click here to return to the Server Summary.</a></b>"
				"</td>\";" EL
				"}else{" EL
				"$('status').innerHTML = \"</td><td>"
				"<b><br>Error Code: \"+code+\".<br>\"+xmlhttp.responseText.substring(5,xmlhttp.responseText.length)+\"<br>"
				"<br><a href=\\\"admin.cgi?sid=" + tos(sid) + "\\\">Click here to return to the Server Summary.</a></b>"
				"</td>\";" EL
				"}" EL
				"}else{" EL
				"clearInterval(timeout);" EL
				"$('status').setAttribute(\"align\",\"center\");" EL
				"$('status').innerHTML = \"</td><td>"
				"Authhash was changed and saved to the configuration file. The stream will now be updated with the change.<br>"
				"<br><a href=\\\"admin.cgi?sid=" + tos(sid) + "\\\"><b>Click here to return to the stream summary.</b></a></td>\";" EL
				"}" EL

				"};" EL
				"}else{" EL
				"xmlhttp.onreadystatechange=function(){" EL
				"if(xmlhttp.readyState==4){" EL
				"var code = parseInt((xmlhttp.responseText!=\"\"?xmlhttp.responseText:\"200\"));" EL
				"if(code!=200){" EL
				"clearInterval(timeout);" EL
				"$('status').setAttribute(\"align\",\"center\");" EL
				"$('status').setAttribute(\"colspan\",\"2\");" EL
				"if(code==0){" EL
				"$('status').innerHTML = \"</td><td>"
				"<b><br>Error Code: \"+code+\".<br>Check the DNAS is running and there is a working network connection.<br>"
				"<br><a href=\\\"admin.cgi?sid=" + tos(sid) + "\\\">Click here to return to the Server Summary.</a></b>"
				"</td>\";" EL
				"}else{" EL
				"$('status').innerHTML = \"</td><td>"
				"<b><br>Error Code: \"+code+\".<br>\"+xmlhttp.responseText.substring(5,xmlhttp.responseText.length)+\"<br>"
				"<br><a href=\\\"admin.cgi?sid=" + tos(sid) + "\\\">Click here to return to the Server Summary.</a></b>"
				"</td>\";" EL
				"}" EL
				"}" EL
				"}else{" EL
				"clearInterval(timeout);" EL
				"$('status').setAttribute(\"align\",\"center\");" EL
				"$('status').innerHTML = \"</td><td>"
				"Authhash was changed and saved to the configuration file. The stream will now be updated with the change.<br>"
				"<br><a href=\\\"admin.cgi?sid=" + tos(sid) + "\\\"><b>Click here to return to the stream summary.</b></a></td>\";" EL
				"}" EL
				"};" EL
				"}" EL
				"xmlhttp.send(null);" EL
				"timeout = setInterval(countDown,250);" EL
				"return false;" EL
				"}" EL

				"var counter=0;" EL
				"function countDown(){" EL
				"counter++;" EL
				"if(counter>=5){" EL
				"counter=0;" EL
				"}" EL
				"if(counter<5){" EL
				"$('status').setAttribute(\"colspan\",\"3\");" EL
				"$('status').setAttribute(\"align\",\"left\");" EL
				"$('status').setAttribute(\"valign\",\"middle\");" EL
				"$('status').innerHTML = \"<b>Processing\"+Array(counter).join(\".\")+\"</b><br><br>This may take a while.\";" EL
				"}" EL
				"}" EL

				"function runUrlGetError(){" EL
				"}" EL

				"function runUrlGet(urlString,callback){" EL
				"var xmlhttp = getHTTP();" EL
				"try{" EL
				"xmlhttp.open(\"GET\",(urlString==document.location?urlString:\"http://" +
					gOptions.ypAddr() + ":" + tos(gOptions.ypPort()) +
					((gOptions.ypPath() != utf8("/yp2")) ? "/yp" : "") +
				"/\"+urlString),true);" EL
				"if(window.XDomainRequest){" EL
				"xmlhttp.onload=callback;" EL
				"xmlhttp.onerror=runUrlGetError;" EL
				"}else{" EL
				"xmlhttp.onreadystatechange=callback;" EL
				"}" EL
				"response=xmlhttp;" EL
				"xmlhttp.send(null);" EL
				"}" EL
				"catch(e){" EL
				"}" EL
				"}" EL

				"function getAuthInfo(){" EL
				"if(response.readyState == null || response.readyState==4 && response.status==200){" EL
				"$('header').innerHTML = response.responseText;" EL
				"}" EL
				"}" EL

				"var registerOnWindowLoad = function(callback){" EL
				"if(window.addEventListener){" EL
				"window.addEventListener('load',callback,false);" EL
				"}else{" EL
				"window.attachEvent('onload',callback);" EL
				"}" EL
				"}" EL

				"registerOnWindowLoad(function(){" EL
				"runUrlGet(\"authinfo_" + (authhash.empty() ? "create" : "update") +
				"?v=" + urlUtils::escapeURI_RFC3986(gOptions.getVersionBuildStrings()) +
				"&os=" + urlUtils::escapeURI_RFC3986(SERV_OSNAME) + "\",getAuthInfo);" EL
				"if($('existing')!=null){" EL
				"$('existing').onkeyup=changeExisting;" EL
				"$('existing').onchange=changeExisting;" EL
				"}" EL
				"if($('authhash')!=null){" EL
				"$('authhash').onkeyup=authhashChange;" EL
				"$('authhash').onchange=authhashChange;" EL
				"}" EL
				"authhashChange();" EL
				"});" EL
				"</script>" +
				getUptimeScript() +
				getIEFlexFix() +
				getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_listeners(const streamData::streamID_t sid) throw()
{
	utf8 header = MSG_NO_CLOSE_200, body;
	// just to make sure we came from an appropriate page
	const utf8 &server = mapGet(m_httpRequestInfo.m_QueryParameters, "server", (utf8)""),
			   &check = ("admin.cgi?sid=" + tos(sid));
	const bool nowrap = mapGet(m_httpRequestInfo.m_QueryParameters, "nw", (bool)gOptions.adminNoWrap());
	const int fh = mapGet(m_httpRequestInfo.m_QueryParameters, "fh", (int)0);
	if ((sid > 0) && (m_referer.find(check) == 0) && !server.empty() && (listenerId == server))
	{
		stats::currentClientList_t client_data;
		stats::getClientDataForStream(sid, client_data);
		if (!client_data.empty())
		{
			streamData::streamInfo info;
			streamData::extraInfo extra;
			streamData::getStreamInfo(sid, info, extra);

			stats::statsData_t data;
			stats::getStats(sid, data);

			utf8 clientsBody = "<div style=\"overflow:auto;" + (!fh ? "max-height:500px;" : (utf8)"") + "\">"
							   "<table class=\"ls\" style=\"border:0;text-align:center;" +
							   (nowrap ? "white-space:nowrap;" : "") + "\" "
							   "cellpadding=\"5\" cellspacing=\"0\" width=\"100%\" align=\"center\">"
							   "<col width=\"15%\"><col width=\"50%\"><col width=\"20%\">"
							   "<tr><td colspan=\"10\" class=\"inp\">Current Listeners</td></tr>"
							   "<tr class=\"tll\"><td>Listener Address<br>(Host Address)</td>"
							   "<td>User Agent</td><td>Connected<br>Duration</td>" + utf8(!info.m_radionomyID.empty() ?
							   "<td>" + baseImage("adavail", "Advert Status", false, false) + "</td>" : "") +
							   (data.connectedListeners != data.uniqueListeners ? "<td>Kick<br>Client</td>" : (utf8)"") +
							   "<td>Kick<br>IP</td><td>Ban<br>IP</td><td>Ban<br>Subnet</td>"
							   "<td>Reserve<br>Listener</td><td>Block<br>User Agent</td></tr>";

			const time_t t = ::time(NULL);
			size_t rowCount = 0;
			// if we have non-unique clients then we need to process the list differently so as to group
			// them together and then re-sort the output by client listener duration on the first match
			// otherwise we revert the code back to the original un-grouped method to not waste resources
			if (data.connectedListeners != data.uniqueListeners)
			{
				map<utf8,stats::uniqueClientData_t> unique_clients;
				for (stats::currentClientList_t::const_iterator i = client_data.begin(); i != client_data.end(); ++i)
				{
					// if set to kicked then no need to show those (since they may re-appear if in-progress)
					if (!(*i)->m_kicked)
					{
						stats::uniqueClientData_t client;
						const bool localhost = ((*i)->m_ipAddr.find(utf8("127.")) == 0);

						// look for existing instances and append the new details to the existing details
						const map<utf8,stats::uniqueClientData_t>::const_iterator im = unique_clients.find((*i)->m_ipAddr);
						if (im != unique_clients.end())
						{
							client = (*im).second;
							client.m_userAgent += "</tr><tr" + ((*i)->m_ripClient || localhost ? utf8(" style=\"font-style:italic;\"") : "") + ">";
							client.m_unique += ",";
							++client.m_total;
						}
						else
						{
							client.m_connectTime = (*i)->m_startTime;
							client.m_ipAddr = (*i)->m_ipAddr;
							client.m_hostName = (*i)->m_hostName;
							client.m_XFF = (*i)->m_XFF;
							client.m_total = 1;
						}

						const int slave = ((*i)->m_clientType & streamData::SC_CDN_SLAVE);
						client.m_userAgent += "<td " + (!client.m_XFF.empty() ? "title=\"XFF: " + aolxml::escapeXML(client.m_XFF) +
											  "\" " : "") + "style=\"" + (!nowrap ? "" : "white-space:nowrap;") + "\"" +
											  utf8(slave ? " class=\"thr\"" : "") + ">" + (((*i)->m_clientType & streamData::RADIONOMY) ?
											  "Radionomy Stats Collector" : (!nowrap ? addWBR((*i)->m_userAgent) : "<div style=\"float:left;\">" +
											  aolxml::escapeXML((*i)->m_userAgent)) + "</div>") + " <div style=\"float:right;\">" +
											  getClientImage((*i)->m_clientType) + "</div></td>";
						if ((*i)->m_ripClient)
						{
							client.m_ripAddr = true;
						}

						const time_t connected = (t - (*i)->m_startTime);
						utf8 timer = timeString(connected, true), timerTip;
						if (timer.empty())
						{
							timer = "Starting...";
						}
						else
						{
							timerTip = timeString(connected);
						}

						client.m_userAgent += "<td" + utf8(slave ? " class=\"thr\"" : "") + " title=\"" +
											  timerTip + "\">" + aolxml::escapeXML(timer) + "</td>";
						client.m_unique = (*i)->m_ipAddr;
						if (!info.m_radionomyID.empty())
						{
							client.m_userAgent += "<td" + utf8(slave ? " class=\"thr\"" : "") + ">" + (((*i)->m_group > 0) ||
												  (*i)->m_triggers ? advertImage(sid, (*i)->m_group, (*i)->m_triggers) :
												  "<div title=\"Not Recognised For Adverts\">N/A</div>") + "</td>";
						}
						client.m_userAgent += "<td^" + utf8(slave ? " class=\"thr\"" : "") + "><a href=\"admin.cgi?sid=" +
											  tos(sid) + "&amp;mode=kickdst&amp;kickdst=" + tos((*i)->m_unique) + "\">Kick</a>" +
											  (client.m_total == 1 ? "</td^>" : "</td>") + "<td" + utf8(slave ? " class=\"thr\"" : "") +
											  ">" + ((*i)->m_userAgent.empty() || ((*i)->m_userAgent == EMPTY_AGENT) ? "N/A" :
											  "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=agent&amp;agent=" +
											  urlUtils::escapeURI_RFC3986((*i)->m_userAgent) + "\">Block</a>") + "</td>";

						unique_clients[(*i)->m_ipAddr] = client;
					}

					delete (*i);
				}

				// take the map and convert to a vector so we can then re-sort the list back into longest to least duration
				vector<stats::uniqueClientData_t> clients;
				for (map<utf8,stats::uniqueClientData_t>::const_iterator i = unique_clients.begin(); i != unique_clients.end(); ++i)
				{
					clients.push_back((*i).second);
				}

				std::sort(clients.begin(), clients.end(), sortUniqueClientDataByTime);

				// and now we dump the generated list
				for (vector<stats::uniqueClientData_t>::const_iterator i = clients.begin(); i != clients.end(); ++i)
				{
					const utf8 host = ((*i).m_hostName != (*i).m_ipAddr ? aolxml::escapeXML((*i).m_hostName) + " (" + (*i).m_ipAddr + ")" : (*i).m_ipAddr);
					const bool localhost = ((*i).m_ipAddr.find(utf8("127.")) == 0);

					// if we have a multiple client block then re-process so the relevant parts can
					// be listed individually with adjustment of some of the visual styles as needed
					utf8 multiBlock = (*i).m_userAgent;
					uniString::utf8::size_type tpos = multiBlock.find(utf8("<td^"));
					if (tpos != uniString::utf8::npos)
					{
						while (tpos != uniString::utf8::npos)
						{
							multiBlock.replace(tpos, 4, utf8(((*i).m_total > 1 ? "<td" : "<td colspan=\"2\"")));
							tpos = multiBlock.find(utf8("<td^"));
						}
					}

					tpos = multiBlock.find(utf8("</td^>"));
					if (tpos != uniString::utf8::npos)
					{
						const bool hasHostName = ((*i).m_hostName != (*i).m_ipAddr);
						utf8 endBlock = "<td" + ((*i).m_total > 1 ? " rowspan=\"" + tos((*i).m_total) + "\"" : (utf8)"") + ">" +
										(!localhost ? "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=bandst&amp;bandst=" +
										urlUtils::escapeURI_RFC3986((*i).m_ipAddr) + "&amp;banmsk=255" + "&amp;kickdst=" +
										(*i).m_unique + "\">Ban</a>" : "N/A") + "</td><td" + ((*i).m_total > 1 ? " rowspan=\"" +
										tos((*i).m_total) + "\"" : (utf8)"") + ">" + (!localhost ? "<a href=\"admin.cgi?sid=" +
										tos(sid) + "&amp;mode=bandst&amp;bandst=" + urlUtils::escapeURI_RFC3986((*i).m_ipAddr) +
										"&amp;banmsk=0" + "&amp;kickdst=" + (*i).m_unique + "\">Ban </a>" : "N/A") + "</td>"
										"<td" + ((*i).m_total > 1 ? " rowspan=\"" + tos((*i).m_total) + "\"" : (utf8)"") + ">" +
										(!localhost ? "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=" +
										((*i).m_ripAddr ? "unripdst" : "ripdst") + "&amp;ripdst=" +
										urlUtils::escapeURI_RFC3986((hasHostName ? (*i).m_hostName : (*i).m_ipAddr)) +
										(hasHostName ? "&amp;ripdstraw=" + urlUtils::escapeURI_RFC3986((*i).m_ipAddr) : "") +
										"\">" + ((*i).m_ripAddr ? "Remove" : "Add") + "</a>" : "N/A") + "</td>";

						multiBlock.replace(tpos, 6, utf8(((*i).m_total > 1 ?
										   "</td><td rowspan=\"" + tos((*i).m_total) +
										   "\"><a href=\"admin.cgi?sid=" + tos(sid) +
										   "&amp;mode=kickdst&amp;kickdst=" +
										   (*i).m_unique + "\">Kick</a></td>" +
										   endBlock : "</td>" + endBlock)));
					}

					rowCount += (*i).m_total;
					clientsBody +=	"<tr" + ((*i).m_ripAddr || localhost ? utf8(" style=\"font-style:italic;\"") : "") + ">"
									"<td" + ((*i).m_total > 1 ? " rowspan=\"" + tos((*i).m_total) + "\"" : "") + ">" +
									(gOptions.useXFF() && !(*i).m_XFF.empty() ? xffImage() + " " : "") + host + "</td>" + multiBlock;
				}
			}
			else
			{
				for (stats::currentClientList_t::const_iterator i = client_data.begin(); i != client_data.end(); ++i)
				{
					// if set to kicked then no need to show those (since they may re-appear if in-progress)
					if (!(*i)->m_kicked)
					{
						const time_t connected = (::time(NULL) - (*i)->m_startTime);
						utf8 timer = timeString(connected, true), timerTip;
						if (timer.empty())
						{
							timer = "Starting...";
						}
						else
						{
							timerTip = timeString(connected);
						}

						const utf8 host = ((*i)->m_hostName != (*i)->m_ipAddr ? aolxml::escapeXML((*i)->m_hostName) +
										  " (" + (*i)->m_ipAddr + ")" : (*i)->m_ipAddr);
						const bool localhost = ((*i)->m_ipAddr.find(utf8("127.")) == 0);
						const bool hasHostName = ((*i)->m_hostName != (*i)->m_ipAddr);

						++rowCount;
						const int slave = ((*i)->m_clientType & streamData::SC_CDN_SLAVE);
						clientsBody += "<tr" + ((*i)->m_ripClient || localhost ? utf8(" style=\"font-style:italic;\"") : "") + ">"
									   "<td " + (gOptions.useXFF() && !(*i)->m_XFF.empty() ? "title=\"XFF: " +
									   aolxml::escapeXML((*i)->m_XFF) + "\">" + xffImage() + " " : ">") + host + "</td><td " +
									   (!(*i)->m_XFF.empty() ? "title=\"XFF: " + aolxml::escapeXML((*i)->m_XFF) + "\" " : "") +
									   "style=\"" + (!nowrap ? "" : "white-space:nowrap;") + "\"" + utf8(slave ? " class=\"thr\"" :
									   "") + ">" + (((*i)->m_clientType & streamData::RADIONOMY) ? "Radionomy Stats Collector" :
									   (!nowrap ? addWBR((*i)->m_userAgent) : "<div style=\"float:left;\">" +
									   aolxml::escapeXML((*i)->m_userAgent)) + "</div>") + " <div style=\"float:right;\">" +
									   getClientImage((*i)->m_clientType) + "</div>" + "</td><td" + utf8(slave ? " class=\"thr\"" : "") +
									   " title=\"" + timerTip + "\">" + aolxml::escapeXML(timer) + "</td>";

						if (!info.m_radionomyID.empty())
						{
							clientsBody += "<td" + utf8(slave ? " class=\"thr\"" : "") + ">" + (((*i)->m_group > 0) ||
										   (*i)->m_triggers ? advertImage(sid, (*i)->m_group, (*i)->m_triggers) :
										   "<div title=\"Not Recognised For Adverts\">N/A</div>") + "</td>";
						}

						const utf8 unique = tos((*i)->m_unique);
						clientsBody += "<td" + utf8(slave ? " class=\"thr\"" : "") + "><a href=\"admin.cgi?sid=" +
									   tos(sid) + "&amp;mode=kickdst&amp;kickdst=" + unique + "\">Kick</a></td><td>" +
									   (!localhost ? "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=bandst&amp;bandst=" +
									   urlUtils::escapeURI_RFC3986((*i)->m_ipAddr) + "&amp;banmsk=255" + "&amp;kickdst=" + unique +
									   "\">Ban</a>" : "N/A") + "</td><td>" + (!localhost ? "<a href=\"admin.cgi?sid=" + tos(sid) +
									   "&amp;mode=bandst&amp;bandst=" + urlUtils::escapeURI_RFC3986((*i)->m_ipAddr) + "&amp;banmsk=0"
									   "&amp;kickdst=" + unique + "\">Ban </a>" : "N/A") + "</td><td>" + (!localhost ?
									   "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=" + ((*i)->m_ripClient ?
									   "unripdst" : "ripdst") + "&amp;ripdst=" + urlUtils::escapeURI_RFC3986((hasHostName ?
									   (*i)->m_hostName : (*i)->m_ipAddr)) + (hasHostName ? "&amp;ripdstraw=" +
									   urlUtils::escapeURI_RFC3986((*i)->m_ipAddr) : "") + "\">" + ((*i)->m_ripClient ?
									   "Remove" : "Add") + "</a>" : "N/A") + "</td><td" + utf8(slave ? " class=\"thr\"" : "") +
									   ">" + ((*i)->m_userAgent.empty() || ((*i)->m_userAgent == EMPTY_AGENT) ? "N/A" :
									   "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=agent&amp;agent=" +
									   urlUtils::escapeURI_RFC3986((*i)->m_userAgent) + "\">Block</a>") + "</td></tr>";
					}

					delete (*i);
				}
			}

			if (rowCount > 0)
			{
				if (rowCount > 1)
				{
					clientsBody +=	"<tr><td style=\"border:0;\"></td><td style=\"padding:0;\"><a href=\"admin.cgi?sid=" +
									tos(sid) + "&amp;mode=kickdst&amp;kickdst=duplicates\" title=\"Kick all duplicate "
									"listeners (based on oldest first by user-agent for the same address)\"><b>Kick Duplicates</b>"
									"</a></td><td>" + timeString(data.avgUserListenTime, true) + "</td><td style=\"padding:0;\" "
									"colspan=\"" + utf8(data.connectedListeners != data.uniqueListeners ? "4" : "3") +
									"\"><a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=kickdst&amp;kickdst=all\" "
									"title=\"Kick all currently connected listeners\"><b>Kick All</b></a></td></tr>";
				}

				// only output the table if we actually had clients to show
				body += clientsBody + "</table></div>";
			}
		}
	}
	else
	{
		body = "<div style=\"padding:1em;text-align:center;\"><b><img "
			   "border=\"0\" src=\"images/warn.png\"> The current "
			   "listener list could not be loaded. <img border=\"0\" "
			   "src=\"images/warn.png\"><br><a href=\"admin.cgi?sid=" +
			   tos(sid) + "&nw=" + tos(nowrap) + "&fh=" + tos(fh) +
			   "\">Click here to reload this page</a>. If this issue "
			   "<br>persists, try logging out and back in again.</b></div>";
	}

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

const utf8 protocol_admincgi::getClientIP(const bool streamPublic, const utf8 &publicIP) throw()
{
	// test for potentially invalid IPs or ones that will cause the playlist link generation to fail
	// attempting to use the server path provided by the YP if in public mode, otherwise uses 'host'
	return (!streamPublic || publicIP.empty() ?
			((g_IPAddressForClients.find(utf8("0.")) == 0 ||
			 // allow localhost / loopback connections through for admin access
			 ((!g_IPAddressForClients.empty() && g_IPAddressForClients.find(utf8("127.")) == 0)) ||
			  g_IPAddressForClients.empty()) && !m_hostIP.empty() ?
			  m_hostIP : (!m_hostIP.empty() ? m_hostIP : g_IPAddressForClients)) : publicIP);
}

void protocol_admincgi::mode_none(const streamData::streamID_t sid, const int refreshRequired) throw()
{
	utf8 header = MSG_NO_CLOSE_200,
		 body = getStreamAdminHeader(sid, "Stream Status &amp; Listeners", refreshRequired);

	time_t streamUptime = 0;
	bool hasListeners = false, isConnected = false;

	if (refreshRequired > 0)
	{
		body += "<br><table cellpadding=\"5\" cellspacing=\"0\" border=\"0\" width=\"100%\">"
				"<tr><td align=\"center\" id=\"counter\">"
				"<br>Waiting " + tos(refreshRequired) + " second" + ((refreshRequired > 1) ? "s" : (utf8)"") +
				" for any configuration changes to take effect.<br>"
				"If not automatically redirected or do not want to wait, "
				"<a href=\"admin.cgi?sid="+tos(sid)+"\">click here.</a>"
				"<br><br></td></tr></table></tr></table></td></tr></table>";
	}
	else
	{
		streamData::streamInfo info;
		streamData::extraInfo extra;
		streamData::getStreamInfo(sid, info, extra);

		stats::statsData_t data;
		stats::getStats(sid, data);

		isConnected = extra.isConnected;
		hasListeners = (data.connectedListeners > 0);

		// this is a placeholder for the listener details which we grab asynchronously for speed since #615
		body += "<div id=\"listeners\">" + (data.connectedListeners > 0 ?
				"<div style=\"padding:1em;text-align:center;\"><b>Loading current "
				"listener list...</b></div>" : (utf8)"") + "</div><table cellpadding=\"5\" "
				"cellspacing=\"0\" border=\"0\" width=\"100%\"><tr><td class=\"tsp\" "
				"align=\"center\">Current Stream Information</td></tr></table>";

		utf8 detailsBody = "";
		bool showing = false;
		if (!(gOptions.read_stream_adminPassword(sid) && !gOptions.stream_adminPassword(sid).empty()))
		{
			utf8 log = gOptions.realLogFile();
			utf8::size_type pos = log.rfind(fileUtil::getFilePathDelimiter());
			if ((pos != utf8::npos))
			{
				log = log.substr(pos + 1);
			}

			utf8 conf = gOptions.confFile();
			pos = conf.rfind(fileUtil::getFilePathDelimiter());
			if ((pos != utf8::npos))
			{
				conf = conf.substr(pos + 1);
			}

			// trim down the file paths shown to make things less cluttered on hosted setups
			// places the full path in the 'title' so it can still be found if required, etc
			detailsBody += "Log file: <b title=\"" +
						   aolxml::escapeXML(fileUtil::getFullFilePath(gOptions.realLogFile())) +
						   "\">" + aolxml::escapeXML(fileUtil::stripPath(log)) + "</b><br>"
						   "Configuration file: <b title=\"" +
						   aolxml::escapeXML(fileUtil::getFullFilePath(gOptions.confFile())) +
						   "\">" + aolxml::escapeXML(fileUtil::stripPath(conf)) + "</b><br>";

			showing = true;
		}

		utf8 introFile = gOptions.stream_introFile(sid);
		if (!gOptions.read_stream_introFile(sid))
		{
			introFile = gOptions.introFile();
		}

		utf8 backupFile = gOptions.stream_backupFile(sid);
		if (!gOptions.read_stream_backupFile(sid))
		{
			backupFile = gOptions.backupFile();
		}

		utf8 backupTitle = gOptions.stream_backupTitle(sid);
		if (!gOptions.read_stream_backupTitle(sid))
		{
			backupTitle = gOptions.backupTitle();
		}

		streamData *sd = streamData::accessStream(sid);
		detailsBody += utf8(showing ? "<br><hr><br>" : "") + "Intro file is <b title=\"" +
							((!introFile.empty() || (sd && sd->getIntroFile().gotData()) ? (!introFile.empty() ?
							fileUtil::getFullFilePath(introFile) : (utf8)"") : (utf8)"") + "\">" +
							aolxml::escapeXML((!introFile.empty() || (sd && sd->getIntroFile().gotData()) ? (!introFile.empty() ?
							fileUtil::stripPath(introFile) : "from source") : "empty")) + "</b><br>Backup file is <b title=\"" +
							((!backupFile.empty() || (sd && sd->getBackupFile().gotData())) ? (!backupFile.empty() ?
							fileUtil::getFullFilePath(backupFile) : (utf8)"") : (utf8)"") + "\">" +
							aolxml::escapeXML((!backupFile.empty() || (sd && sd->getBackupFile().gotData())) ?
							(!backupFile.empty() ? fileUtil::stripPath(backupFile) : "from source") : "empty")) + "</b><br>";

		if (!backupTitle.empty() && !backupFile.empty() && !extra.isConnected)
		{
			detailsBody += "Backup title is: <b>" + aolxml::escapeXML(backupTitle) + "</b><br>";
		}
					   
		detailsBody += "<br><hr><br>Idle timeouts are <b>" + tos(gOptions.getAutoDumpTime(sid)) + "s</b><br>";

		if (extra.isConnected)
		{
			detailsBody += "<br><hr><br>Source connection type: <b>" +
						   utf8(info.m_sourceType == streamData::SHOUTCAST1 ? "v1" :
						   (info.m_sourceType == streamData::SHOUTCAST2 ? "v2" : "HTTP")) +
						   (extra.isRelay ? " relay" + (extra.isBackup ? utf8("&nbsp;backup") : "") :
						   (extra.isBackup ? utf8("&nbsp;backup") : "")) + "</b><br><div "
						   "style=\"max-width:15em;\">Source user agent: <b>" +
						   addWBR((!info.m_sourceIdent.empty() ? info.m_sourceIdent :
						   "Legacy / Unknown")) + "</b>""</div>";
		}

		if (sd)
		{
			detailsBody += (extra.isConnected ? "<br><hr><br>" : "<br>");

			if (sd->streamAlbumArt().empty())
			{
				detailsBody += "Stream artwork <b>not available</b>";
			}
			else
			{
				detailsBody += "Stream artwork <b>available</b> [&nbsp;<a href=\"/streamart?sid=" + tos(sid) + "\">view</a>&nbsp;]</b>";
			}

			detailsBody += "<br>";

			if (sd->streamPlayingAlbumArt().empty())
			{
				detailsBody += "Playing artwork <b>not available</b>";
			}
			else
			{
				detailsBody += "Playing artwork <b>available</b> [&nbsp;<a href=\"/playingart?sid=" + tos(sid) + "\">view</a>&nbsp;]</b>";
			}

			if (!info.m_currentURL.empty() && (info.m_currentURL.find(utf8("DNAS/")) == utf8::npos))
			{
				detailsBody += "<br><br><hr><br>Song url from source [&nbsp;<a href=\"" +
							   utf8((info.m_currentURL.find(utf8("://")) == utf8::npos) &&
									(info.m_currentURL.find(utf8("&")) != 0) ? "//" : "") +
							   info.m_currentURL + "\">view</a>&nbsp;]<br>"
							   "<div style=\"max-width:15em;\"><b>Note:</b> "
							   "This may not be a valid url and is intended for internal use.</div>";
			}

			sd->releaseStream();
		}

		const utf8& message = streamData::getStreamMessage(sid);
		if (!message.empty())
		{
			detailsBody += "<tr><td style=\"border:0;padding:0;\"><br></td></tr>"
						   "<tr><td align=\"center\" valign=\"top\" "
						   "style=\"display:block;max-width:15em;padding-top:1px;\"><br>"
						   "<div align=\"center\" class=\"infh\">"
						   "<b>Official Message Received</b></div>" + message + "</td></tr>";
		}

		body += "<div style=\"padding:0 1em;\"><br></div>"
				"<table width=\"100%\" align=\"center\"><tr valign=\"top\">";

		const utf8 movedUrl = gOptions.stream_movedUrl(sid);
		if (movedUrl.empty())
		{
			body += "<td><table class=\"en\" cellpadding=\"15px\" "
					"style=\"border:0;margin-left:1em;\"><tr>"
					"<td valign=\"top\"><div align=\"center\" "
					"class=\"infh\"><b>Stream Details</b></div>" +
					detailsBody + "</td></tr></table></td>";
		}

		const int maxUsers = ((info.m_streamMaxUser > 0) && (info.m_streamMaxUser < gOptions.maxUser()) ? info.m_streamMaxUser : gOptions.maxUser());
		if (extra.isConnected)
		{
			const bool isListable = streamData::isAllowedType(info.m_uvoxDataType);
			utf8 listenLink = "<a href=\"listen.pls?sid=" + tos(sid) + "\"><img border=\"0\" title=\"Listen to Stream\" "
							  "alt=\"Listen to Stream\" style=\"vertical-align:middle\" src=\"images/listen.png\"></a>" +
								  (sd && !sd->radionomyID().empty() && sd->streamAdvertMode() ?
										  "<img border=\"0\" title=\"Active DNAS+ Stream\nMonetisation Enabled\" "
										  "alt=\"Active DNAS+ Stream\nMonetisation Enabled\" style=\"vertical-align:middle\" "
										  "src=\"images/adavail.png\">&nbsp;" : (utf8)"");

			body += "<td><table cellspacing=\"0\" cellpadding=\"2\" border=\"0\" style=\"padding-left:1em;\">"
					"<tr valign=\"top\"><td colspan=\"2\">" + getNewVersionMessage() + "<td></tr>"
					"<tr valign=\"top\"><td>Listing Status: </td><td><b>Stream is currently up " +
					(info.m_streamPublic && isListable ? (extra.ypConnected != 1 ? "" : utf8("and public") + listenLink) : utf8("and private (not listed)") + listenLink) +
					(info.m_streamPublic && isListable ? (extra.ypConnected != 1 ? (!yp2::isValidAuthhash(info.m_authHash) ?
															string(info.m_authHash.empty() ? "but requires <a href=\"admin.cgi?sid=" + tos(sid) +
																							 "&amp;mode=register\">registration</a> in the Shoutcast Directory.<br>" :
																							 "but not listed due to an invalid authhash.<br>") +

															(info.m_authHash.empty() ? "Listeners are allowed and the stream will act like it is private until resolved."
															"<br><br>To create an authhash you will need to <a href=\"admin.cgi?sid=" + tos(sid) +
															"&amp;mode=register\">register</a> the stream with us.<br>If you already have an existing authhash "
															"then you can enter it <a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">here</a>.<br><br>" :
															"Listeners are allowed and the stream will act like it is private until resolved.") :
															(extra.ypErrorCode == 200 ? "waiting on a Directory response." :
																(extra.ypErrorCode == YP_COMMS_FAILURE ? "unable to access the Directory.<br>Listeners are allowed and the stream will act like it is private until resolved." :
																(extra.ypErrorCode == YP_MAINTENANCE_CODE ? "received a Directory maintenance notification: <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2#YP_Server_Errors\">" + tos(extra.ypErrorCode) +
																											"</a><br>Listeners will be allowed though the stream will not be listed in the Directory." :
																 "received Directory error code: <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2#YP_Server_Errors\">" + tos(extra.ypErrorCode) + "</a><br>" +
																 (extra.ypConnected != 2 ? "" :
																 "during a listing update. The stream may no longer appear.") +
																 "<br>Check the server log and / or contact the server administrator.")))) : "") : "") + "</b></td></tr>"
					"<tr valign=\"top\"><td>Stream Status: </td>"
					"<td><b>Stream is up (" + streamData::getContentType(info) + " @ " +
					(info.m_streamBitrate > 0 ? tos(info.m_streamBitrate) : "unknown") +
					" kbps" + (info.m_vbr ? " (VBR)" : "") + ", " +
					sampleRateStr(info.m_streamSampleRate) + ") with " +
					tos(data.connectedListeners) + (maxUsers > 0 ? " of " +
					tos(maxUsers) : "") + " listeners" + (!maxUsers ? " (unlimited)" : "") +
					(data.connectedListeners != data.uniqueListeners ? (" (" +
					tos(data.uniqueListeners) + " unique)") : "") + "</b></td></tr>";

			if (data.peakListeners > 0)
			{
				body += "<tr valign=\"top\"><td>Listener Peak: </td><td><b>" +
						tos(data.peakListeners) + "</b></td></tr>";
			}

			const utf8 avgTime = timeString(data.avgUserListenTime);
			if (!avgTime.empty())
			{
				body +=	"<tr valign=\"top\"><td>Avg. Play Time: </td>"
						"<td><b>" + avgTime + "</b></td></tr>";
			}

			body +=	"<tr valign=\"top\"><td>Stream Name: </td><td><b>" +
					(info.m_streamPublic && extra.ypConnected ? "<a target=\"_blank\" href=\"http://directory.shoutcast.com/Search?query=" +
					 urlUtils::escapeURI_RFC3986(info.m_streamName) + "\">" + aolxml::escapeXML(info.m_streamName) + "</a>" :
					 aolxml::escapeXML(info.m_streamName)) + "</b></td></tr>" +

					(info.m_streamPublic && extra.ypConnected ? "<tr valign=\"top\"><td alt=\"Shoutcast Directory ID\" "
																"title=\"Shoutcast Directory ID\"><img border=\"0\" "
																"src=\"images/favicon.ico\" style=\"vertical-align:bottom\">"
																" ID: </td><td><b><a title=\"Shoutcast Directory ID\" href=\"http://" +
					 
					 gOptions.ypAddr().hideAsString() + ":" + tos(gOptions.ypPort()) + ((gOptions.ypPath() != utf8("/yp2")) ? "/yp" : "") +

					 "/sbin/tunein-station.pls?id="+info.m_stationID+"\">"+info.m_stationID+"</a></b></td></tr>" : "");

			if (!info.m_streamGenre[0].empty())
			{
				body += "<tr valign=\"top\"><td>Stream Genre(s): </td>"
						"<td><b>" + (info.m_streamPublic && extra.ypConnected ?
						"<a target=\"_blank\" href=\"http://directory.shoutcast.com/Genre?name=" +
						urlUtils::escapeURI_RFC3986(info.m_streamGenre[0]) + "\">" +
						aolxml::escapeXML(info.m_streamGenre[0]) + "</a>" :
						aolxml::escapeXML(info.m_streamGenre[0])) + "</b>";

				for (int i = 1; i < 5; i++)
				{
					if (!info.m_streamGenre[i].empty())
					{
						body += " , <b>" + (info.m_streamPublic && extra.ypConnected ? "<a target=\"_blank\" href=\"http://directory.shoutcast.com/Genre?name=" +
								 urlUtils::escapeURI_RFC3986(info.m_streamGenre[i]) + "\">" + aolxml::escapeXML(info.m_streamGenre[i]) + "</a>" :
								 aolxml::escapeXML(info.m_streamGenre[i])) + "</b>";
					}
				}

				body += "</td></tr>";
			}

			if (!info.m_streamUser.empty())
			{
				body += "<tr valign=\"top\"><td>Stream DJ: </td>"
						"<td><b>" + aolxml::escapeXML(info.m_streamUser) + "</b></td></tr>";
			}

			if (!info.m_streamURL.empty())
			{
				body += "<tr valign=\"top\"><td>Stream Website: </td>"
						"<td><b>" + urlLink(info.m_streamURL) + "</b></td></tr>";
			}

			if (!info.m_currentSong.empty())
			{
				body += "<tr valign=\"top\"><td>Playing Now: </td>"
						"<td><b><a href=\"currentsong?sid=" + tos(sid) + "\">" +
						getCurrentSong(info.m_currentSong) + "</a></b></td></tr>";

				// only show if we have a valid current song
				if (!info.m_comingSoon.empty())
				{
					body += "<tr valign=\"top\"><td>Playing Next: </td>"
							"<td><b><a href=\"nextsong?sid=" + tos(sid) + "\">" +
							aolxml::escapeXML(info.m_comingSoon) + "</a></b></td></tr>";
				}
			}

			// strip down the source address for display output to an appropriate output based on settings
			utf8 srcAddr = niceURL(extra.isBackup ? info.m_backupURL : (extra.isRelay ? info.m_relayURL : info.m_srcAddr));
			if (gOptions.nameLookups())
			{
				if (!extra.isBackup && !extra.isRelay)
				{
					u_short port = 0;
					string addr, hostName;
					socketOps::getpeername(m_socket, addr, port);

					string src = (extra.isBackup ? info.m_backupURL : (extra.isRelay ? info.m_relayURL : info.m_srcAddr)).hideAsString();
					hostName = src;
					if (!socketOps::addressToHostName(addr,port,hostName))
					{
						srcAddr = hostName + " (" + niceURL(src) + ")";
					}
				}
			}

			body += "<tr valign=\"top\"><td>Stream Source: </td>"
					"<td><b>" + (extra.isRelay || extra.isBackup ? urlLink(srcAddr) : srcAddr) + " " +
					(extra.isRelay ? "(relaying" + (extra.isBackup ? utf8(" backup") : "") + ") " : (extra.isBackup ? "(backup) " : "")) + "</b>"
					"[&nbsp;<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=kicksrc\">" + (extra.isRelay || extra.isBackup ? "stop" : "kick") +
					"</a>&nbsp;]</td></tr><tr valign=\"top\"><td>Stream Uptime: </td>"
					"<td id=\"up2\"><b>" + timeString((streamUptime = ::time(NULL) - streamData::getStreamUptime(sid))) + "</b></td></tr>";

			if (!info.m_contentType.empty() && (info.m_uvoxDataType == MP3_DATA))
			{
				body += streamData::getHTML5Player(sid);
			}
					
			body += "</table>";
		}
		else
		{
			body +=	"<td" + (movedUrl.empty() ? (utf8)"" : " align=\"center\"") + ">"
					"<table cellspacing=\"0\" cellpadding=\"2\" border=\"0\">"
					"<tr valign=\"top\"><td colspan=\"2\">" + getNewVersionMessage("<br>") + "<td></tr>"
					"<tr valign=\"top\"><td>Stream Status: </td><td><b>";

			if (movedUrl.empty())
			{
				body += "Stream is currently down" + (data.connectedListeners > 0 ?
						" with " + tos(data.connectedListeners) + (maxUsers > 0 ? " of " +
						tos(maxUsers) : "") + " listeners" + (!maxUsers ? " (unlimited)" : "") +
						(data.connectedListeners != data.uniqueListeners ? (" (" +
						tos(data.uniqueListeners) + " unique)") : "") : ".") + "<br>There is no "
						"source connected or no stream is configured for stream #" + tos(sid) + ".";
			}
			else
			{
				body += "Stream has been moved to " + urlLink(movedUrl) + "<br>No source connections will be allowed for this stream.";
			}
			body += "</b></td></tr>";

			if (data.peakListeners > 0)
			{
				body += "<tr valign=\"top\"><td>Listener Peak: </td><td><b>" +
						tos(data.peakListeners) + "</b></td></tr>";
			}

			utf8 avgTime = timeString(data.avgUserListenTime);
			if (!avgTime.empty())
			{
				body +=	"<tr valign=\"top\"><td>Avg. Play Time: </td>"
						"<td><b>" + avgTime + "</b></td></tr>";
			}

			// add in an option to restart a relay url...
			if (!gOptions.stream_relayURL(sid).empty() && movedUrl.empty())
			{
				// strip down the source address for display output
				utf8 srcAddr = niceURL(gOptions.stream_relayURL(sid));

				// make sure we're not exposing the option to try re-connecting to a pending source relay
				bool noEntry = false;
				if (!(streamData::isRelayActive(sid, noEntry) == 1))
				{
					body += "<tr><td><br>Start Relay:</td><td><br><b>" + urlLink(srcAddr) + "</b> "
							"[ <a href=\"admin.cgi?sid="+tos(sid)+"&amp;mode=startrelay\">start relay</a> ]</td></tr>";
				}
				else
				{
					body += "<tr><td><br>Starting Relay:</td><td><br><b>Connection pending to " +
							urlLink(srcAddr) + "</b> [ <a href=\"admin.cgi?sid=" + tos(sid) +
							"&amp;mode=kicksrc\">abort</a> ]</td></tr>";
				}
			}

			body += "</table>";
		}

		body += "</td></tr></table>";
	}

	// for a refresh, we'll show a countdown so it's obvious that something is happening
	if (refreshRequired)
	{
		body += "<script type=\"text/javascript\">"
				"function $(id){return document.getElementById(id);}" EL
				"var c = " + tos(abs(refreshRequired)) + ";" EL
				"function countDown(){" EL
				"c--;" EL
				"if(c > 0){" EL
				"$('counter').innerHTML = \"<br>Waiting \"+c+\" second\" + (c > 1 ? \"s\" : \"\") + \" for any configuration changes to take effect.<br>"
				"If not automatically redirected or do not want to wait, <a href=\\\"admin.cgi?sid="+tos(sid)+"\\\">click here.</a><br><br>\";" EL
				"}" EL
				"}" EL
				"setInterval(countDown,1000);" EL
				"</script>";
	}

	body += getUptimeScript(false, isConnected, streamUptime) + getIEFlexFix() +
			getHTML5Remover() + (!refreshRequired && hasListeners ?
			 getStreamListeners(sid, mapGet(m_httpRequestInfo.m_QueryParameters, "nw", (bool)gOptions.adminNoWrap()),
									 mapGet(m_httpRequestInfo.m_QueryParameters, "fh", (int)0)) : "") + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

utf8 getCDNMessage(const bool master, const bool slave)
{
	if (slave && !master)
	{
		return "Configured as a CDN slave<br>Authhash inheritance enabled from master";
	}
	else if (master && !slave)
	{
		return "Configured as a CDN master<br>Authhash inheritance enabled for slaves";
	}
	return "Configured as a CDN intermediary<br>Authhash inheritance enabled both ways";
}

utf8 getBadAuthhashMessage(const streamData::streamID_t sid, const utf8 &authHash)
{
	return "<div class=\"en\" style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">"
		   "<b>Invalid Authhash Detected</b>&nbsp;&nbsp;&nbsp;<input class=\"submit\" type=\"button\" "
		   "onclick=\"alert('An incorrect authhash is entered for this stream: " + authHash + "\\n\\n"
		   "Use the [ Clear ] option and re-enter a valid authhash or\\nregister your stream to be "
		   "listed in the Shoutcast directory.\\n\\nIf you think this is a valid authhash then please "
		   "contact\\nsupport including the authhash at support@shoutcast.com.')\" value=\"?\"><br><br>"
		   "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register&amp;register=clear\">Clear Authhash</a>"
		   "&nbsp; | &nbsp;<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Manage Authhash</a></div>";
}

void protocol_admincgi::mode_summary(const int refreshRequired) throw()
{
	utf8 header = MSG_NO_CLOSE_200,
		 streams = "",
		 body = getServerAdminHeader("Server Summary", refreshRequired);

	size_t totalListeners = 0,
		   totalPeakListeners = 0,
		   streamTotal = 0,
		   movedTotal = 0;
	map<size_t,uniString::utf8> streamBlocks;

	if (refreshRequired == 0)
	{
		size_t inc = 0, sid = DEFAULT_SOURCE_STREAM;
		do
		{
			utf8 streamBody = "";
			sid = streamData::enumStreams(inc);

			// check if we have an active source and valid sid before attempting to add
			if (sid >= DEFAULT_SOURCE_STREAM)
			{
				streamData::streamInfo info;
				streamData::extraInfo extra;
				if (streamData::getStreamInfo(sid, info, extra))
				{
					stats::statsData_t data;
					stats::getStats(sid, data);

					// increment our stream total now that we know we have one
					totalListeners += data.connectedListeners;
					totalPeakListeners += data.peakListeners;

					utf8 streamBody2 = "<tr><td align=\"center\">";
					const bool slave = isCDNSlave(sid);
					const bool master = isCDNMaster(sid);
					if (master || slave)
					{
						streamBody2 += "<b><div class=\"en\" style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">" +
									   getCDNMessage(master, slave) + "</div></b><br><br>";
					}

					const bool isListable = streamData::isAllowedType(info.m_uvoxDataType);
					if (!isListable)
					{
						streamBody2 += "<b><div class=\"en\" style=\"padding:1em;margin-bottom:1em;display:inline-block;border-radius:0.5em;\">" +
									   (info.m_uvoxDataType == OGG_DATA ? "OGG Vorbis based streams are not fully supported<br>and will not" :
																		  utf8("NSV based streams are no longer able<br>to ")) +
									   " be listed in the Shoutcast Directory.</div></b><br>";
					}
					/*else if (!info.m_streamPublic && gOptions.cdn().empty() && !slave && !master)
					{
						streamBody2 += "<b><div class=\"en\" style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">"
									   "An authhash is not required for private streams.</div></b><br><br>";
					}*/

					if (isListable)
					{
						if (info.m_authHash.empty())
						{
							streamBody2 += "<div class=\"en\" style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">"
										   "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Create Authhash</a>"
										   "&nbsp; | &nbsp;<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Manage Authhash</a></div>";
						}
						else
						{
							// check that the authhash is a valid length
							if (!yp2::isValidAuthhash(info.m_authHash))
							{
								streamBody2 += getBadAuthhashMessage(sid, info.m_authHash);
							}
							else
							{
								streamBody2 += "<div class=\"en\" style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">"
											   + utf8((master || slave || info.m_streamPublic) && (info.m_streamSampleRate > 0) &&
													  info.m_radionomyID.empty() && ((extra.ypErrorCode != YP_NOT_VISIBLE) &&
													  (extra.ypErrorCode != YP_AUTH_ISSUE_CODE) && (extra.ypErrorCode != -1)) ?
													  warningImage(false) + "&nbsp;&nbsp;<b>Please Register Your Authhash</b><br><br>" : "") +
													  "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Update Authhash</a>&nbsp; | "
													  "&nbsp;<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Manage Authhash</a></div>";
							}
						}
					}
					else
					{
						streamBody2 += "<div class=\"en\" style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">"
									   "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Manage Authhash</a></div>";
					}
					streamBody2 += "</td></tr>";

					streamData *sd = streamData::accessStream(sid);

					streamBody += "<tr><td align=\"center\" class=\"tnl\">" +
								  (sd && !sd->radionomyID().empty() && sd->streamAdvertMode() ?
										  "<img border=\"0\" title=\"Active DNAS+ Stream\nMonetisation Enabled\" "
										  "alt=\"Active DNAS+ Stream\nMonetisation Enabled\" style=\"vertical-align:middle\" "
										  "src=\"images/adavail.png\">&nbsp;" : (utf8)"") +
								  "<a href=\"index.html?sid=" + tos(sid) + "\">Stream #" + tos(sid) + "</a> "
								  "<a href=\"admin.cgi?sid=" + tos(sid) + "\">(Stream Login)</a>" +
								  " <a href=\"listen.pls?sid=" + tos(sid) + "\"><img border=\"0\" title=\"Listen to Stream\" "
								  "alt=\"Listen to Stream\" style=\"vertical-align:middle\" src=\"images/listen.png\"></a>" +
								  (sd && !sd->streamAlbumArt().empty() ? " <a href=\"/streamart?sid=" + tos(sid) + "\">"
												"<img border=\"0\" title=\"View Stream Artwork\" alt=\"View Stream Artwork\" "
												"style=\"vertical-align:middle;padding-left:0.5em;\" src=\"images/streamart.png\"></a>" : "") +
								  (sd && !sd->streamPlayingAlbumArt().empty() ? " <a href=\"/playingart?sid=" + tos(sid) + "\">"
												"<img border=\"0\" title=\"View Playing Artwork\" alt=\"View Playing Artwork\" "
												"style=\"vertical-align:middle;padding-left:0.5em;\" src=\"images/playingart.png\"></a>" : "") +

								  "</td></tr>" +
								  (!info.m_contentType.empty() && (info.m_uvoxDataType == MP3_DATA) ?
								  streamData::getHTML5Player(sid) : "") + streamBody2;

					utf8 content = streamData::getContentType(info);
					if (!info.m_streamUser.empty())
					{
						content = info.m_streamUser + " - " + content;
					}

					const int maxUsers = ((info.m_streamMaxUser > 0) && (info.m_streamMaxUser < gOptions.maxUser()) ? info.m_streamMaxUser : gOptions.maxUser());
					const utf8 listeners = (data.connectedListeners ? (tos(data.connectedListeners) +
										   (data.connectedListeners != data.uniqueListeners ?
										   (" (" + tos(data.uniqueListeners) + " unique)") : "")) : "0") +
										   (maxUsers > 0 ? " of " + tos(maxUsers) : " (unlimited)");

					const utf8 listenLink = "<a href=\"listen.pls?sid=" + tos(sid) + "\"><img border=\"0\" title=\"Listen to Stream\" "
											"alt=\"Listen to Stream\" style=\"vertical-align:middle\" src=\"images/listen.png\"></a>";

					streamBody += "<tr><td align=\"center\">" +
								  (info.m_streamPublic && extra.ypConnected ? "<a target=\"_blank\" href=\"http://directory.shoutcast.com/Search?query=" +
								  urlUtils::escapeURI_RFC3986(info.m_streamName) + "\">" + aolxml::escapeXML(info.m_streamName) + "</a>" :
								  aolxml::escapeXML(info.m_streamName)) + " (" + content + "&nbsp;@&nbsp;" +
								  (info.m_streamBitrate > 0 ? tos(info.m_streamBitrate) : "unknown") +
								  "&nbsp;kbps" + (info.m_vbr ? " (VBR)" : "") + ", " +
								  sampleRateStr(info.m_streamSampleRate) + ")</td></tr>" +

								  (!info.m_currentSong.empty() ? "<tr><td align=\"center\" style=\"padding-bottom:0;\">Playing: <b>"
																 "<a href=\"currentsong?sid=" + tos(sid) + "\">" +
																 aolxml::escapeXML(info.m_currentSong) + "</a></b></td></tr>" +
																 (!info.m_comingSoon.empty() ? "<tr><td align=\"center\" style=\"padding-top:0;\">Coming: <b>"
																  "<a href=\"currentsong?sid=" + tos(sid) + "\">" +
																  aolxml::escapeXML(info.m_comingSoon) + "</a></b></td></tr>" : "") : "") +

								  "<tr><td><table align=\"center\"><tr valign=\"top\"><td align=\"center\">"
								  "<div style=\"text-align:left;\">Listeners: <b>" + listeners + "</b>" +
								  (data.peakListeners > 0 ? "<br>Peak: <b>" + tos(data.peakListeners) + "</b>" : "") +
								  (data.connectedListeners > 0 ? " [&nbsp;<a href=\"admin.cgi?sid=" +
								  tos(sid) + "&amp;mode=kickdst&amp;kickdst=all\">kick all</a>&nbsp;]" : "") +
								  "</div></td><td>&nbsp;&nbsp;&nbsp;</td><td align=\"center\">Status: <b>" +
								  string(info.m_streamPublic && isListable ? (extra.ypConnected != 1 ? "" : string("Public</b><br>"

								  "<div title=\"Shoutcast Directory ID\" alt=\"Shoutcast Directory ID\">"
								  "<img border=\"0\" title=\"Shoutcast Directory ID\" alt=\"Shoutcast Directory ID\" style=\"vertical-align:bottom\" "
								  "src=\"images/favicon.ico\"> ID: <b><a title=\"Shoutcast Directory ID\" href=\"http://" +
								  gOptions.ypAddr().hideAsString() + ":" + tos(gOptions.ypPort()) + ((gOptions.ypPath() != utf8("/yp2")) ? "/yp" : "") +
								  "/sbin/tunein-station.pls?id=" + info.m_stationID.hideAsString() + "\">" + info.m_stationID.hideAsString() + "</a></b></div>")) +

								  (extra.ypConnected != 1 ? (!yp2::isValidAuthhash(info.m_authHash) ?
												   " Not Listed - " + string(info.m_authHash.empty() ? "Empty" : "Invalid") + " Authhash" :
												   (extra.ypErrorCode == 200 ? " Waiting on a Directory response" :
														(extra.ypErrorCode == -1 ? "Unable to access the Directory.<br>Check the server log for more details.<br>The stream will behave like it is private." :
														(extra.ypErrorCode == YP_MAINTENANCE_CODE ? "Directory is down for maintenance: <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2#YP_Server_Errors\">" +
																									tos(extra.ypErrorCode) + "</a><br>Listeners are allowed, stream will not be listed" :
														(extra.ypErrorCode == YP_AUTH_ISSUE_CODE ? " Please contact support as there is an issue with the authhash" : " Directory returned error code: <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2#YP_Server_Errors\">" + tos(extra.ypErrorCode) + "</a><br>" +
														(extra.ypConnected != 2 ? "" : "during a listing update. The stream may not<br>appear in the Directory due to the error. The<br> server will attempt to re-list the stream."))))))  : "") :
												   "Private") + "</td><td>&nbsp;&nbsp;&nbsp;</td>"
								  "<td title=\"Source User Agent: " + addWBR((!info.m_sourceIdent.empty() ?
								  info.m_sourceIdent : "Legacy / Unknown")) + "\">Source: <b>" +
								  utf8(info.m_sourceType == streamData::SHOUTCAST1 ? "v1" :
								  (info.m_sourceType == streamData::SHOUTCAST2 ? "v2" : "HTTP")) +
								  (extra.isRelay ? " relay" + (extra.isBackup ? utf8("&nbsp;backup") : "") :
								  (extra.isBackup ? "&nbsp;backup" : "")) + "</b> [&nbsp;<a href=\"admin.cgi?sid=" +
								  tos(sid) + "&amp;mode=kicksrc\">" + (extra.isRelay || extra.isBackup ? "stop" : "kick") +
								  "</a>&nbsp;]</td></tr></table></td></tr>"

								  "<tr><td align=\"center\">"
								  "<div style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\" class=\"en\">"
								  "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=viewxml\">Summary</a>&nbsp; | &nbsp;"

								  "<div style=\"display:inline-block;\"><a href=\"admin.cgi?sid=" +
								  tos(sid) + "&amp;mode=viewxml&amp;page=3\">Listeners</a> [ <a href=\"admin.cgi?sid=" +
								  tos(sid) + "&amp;mode=viewxml&amp;page=3&amp;ipcount=1\">Counts</a> ]</div>&nbsp; | &nbsp;"

								  "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=viewxml&amp;page=4\">History</a>&nbsp; | &nbsp;"
								  "<a href=\"currentmetadata?sid=" + tos(sid) + "\">Metadata</a>&nbsp; | &nbsp;"
								  "<a href=\"stats?sid=" + tos(sid) + "\">Statistics</a>&nbsp;"
								  "<a href=\"7?sid=" + tos(sid) + "\">&hellip;</a></div></td></tr>";

					if (sd)
					{
						sd->releaseStream();
					}

					const utf8& message = streamData::getStreamMessage(sid);
					if (!message.empty())
					{
						streamBody += "<tr><td align=\"center\" width=\"100%\"><table cellspacing=\"0\" cellpadding=\"15px;\" class=\"ent\" width=\"85%\">"
									  "<tr><td valign=\"top\" align=\"center\" style=\"border:1px;display:block;\"><br>"
									  "<div align=\"center\" class=\"infh\" style=\"margin-left:-15px;margin-right:-15px;margin-top:-14px;\">"
									  "<b>Official Message Received</b></div>" + message + "</td></tr></table></td></tr>";
					}

					streamBlocks[sid] = streamBody;
				}
			}
			++inc;
		}
		while (sid);

		// now we check through for any known but inactive relays and then get them listed as well
		vector<config::streamConfig> relayList(gOptions.getRelayList());
		if (!relayList.empty())
		{
			for (vector<config::streamConfig>::const_iterator i = relayList.begin(); i != relayList.end(); ++i)
			{
				sid = (*i).m_streamID;
				const bool exists = !(*i).m_relayUrl.url().empty();
				if (exists && !streamData::isSourceConnected(sid))
				{
					stats::statsData_t data;
					stats::getStats(sid, data);

					// increment our stream total now that we know we have one
					totalListeners += data.connectedListeners;
					totalPeakListeners += data.peakListeners;

					streamData::streamInfo info;
					streamData::extraInfo extra;
					streamData::getStreamInfo(sid, info, extra);
					utf8 listeners, content, streamBody2,
						 streamBody = "<tr align=\"center\" class=\"tnl\"><td>"
									  "<a href=\"index.html?sid=" + tos(sid) + "\">Stream #" + tos(sid) + "</a> "
									  "<a href=\"admin.cgi?sid=" + tos(sid) + "\">(Stream Login)</a></td></tr>";

					const utf8 movedUrl = gOptions.stream_movedUrl(sid);
					if (movedUrl.empty())
					{
						const bool slave = isCDNSlave(sid);
						const bool master = isCDNMaster(sid);
						if (master || slave)
						{
							streamBody2 = "<div class=\"en\" style=\"padding:1emx;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">" +
										  getCDNMessage(master, slave) + "</div><br><br>";
						}
						else
						{
							if ((*i).m_authHash.empty())
							{
								streamBody2 += "<div class=\"en\" style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">"
											   "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Create Authhash</a>"
											   "&nbsp; | &nbsp;<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Manage Authhash</a></div>";
							}
							else
							{
								// check that the authhash is a valid length
								if (!yp2::isValidAuthhash((*i).m_authHash))
								{
									streamBody2 += getBadAuthhashMessage(sid, info.m_authHash);
								}
								else
								{
									streamBody2 += "<div class=\"en\" style=\"padding:1em;margin-bottom:0.5em;display:inline-block;border-radius:0.5em;\">"
												   + utf8((master || slave || info.m_streamPublic) && (info.m_streamSampleRate > 0) &&
														  info.m_radionomyID.empty() && ((extra.ypErrorCode != YP_NOT_VISIBLE) &&
														  (extra.ypErrorCode != YP_AUTH_ISSUE_CODE) && (extra.ypErrorCode != -1)) ?
														  warningImage(false) + "&nbsp;&nbsp;<b>Please Register Your Authhash</b><br><br>" : "") +
														  "<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Update Authhash</a>&nbsp; | "
														  "&nbsp;<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">Manage Authhash</a></div>";
								}
							}

							if (!streamBody2.empty())
							{
								streamBody2 += "<br>";
							}
						}
					}
					else
					{
						streamBody2 = "<b>This stream is configured as having been moved or retired.<br>"
									  "No source connections will be allowed for this stream.<br><br>"
									  "All client connections received will be redirected to:<br>" +
									  urlLink(movedUrl) + "</b>";
						++movedTotal;
					}

					if (!streamBody2.empty())
					{
						streamBody += "<tr><td align=\"center\">" + streamBody2 + "</td></tr>";
					}

					// strip down the source address for display output
					const utf8 srcAddr = niceURL(gOptions.stream_relayURL(sid));
					if (movedUrl.empty())
					{
						bool noEntry = false;
						const bool isListable = streamData::isAllowedType(info.m_uvoxDataType);
						streamBody += "<tr><td><table align=\"center\"><tr valign=\"top\">"
									  "<td" + (!extra.isConnected ? " rowspan=\"2\"" : (utf8)"") +
									  " align=\"right\">Status: <b>" + string(info.m_streamPublic && isListable ? (extra.ypConnected != 1 ? "" : string("Public")) +
									  (extra.ypConnected != 1 ? (!yp2::isValidAuthhash(info.m_authHash) ?
													   " Not Listed - " + string(info.m_authHash.empty() ? "Empty" : "Invalid") + " Authhash" :
													   (extra.ypErrorCode == 200 ? " Waiting on a Directory response" :
															(extra.ypErrorCode == -1 ? "Unable to access the Directory.<br>Check the error server for more details.<br>The stream will behave like it is private." :
															(extra.ypErrorCode == YP_MAINTENANCE_CODE ? "Directory is down for maintenance: <a target=\"_blank\" "
																										"href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2#YP_Server_Errors\">" +
																										tos(extra.ypErrorCode) + "</a><br>Listeners are allowed, stream will not be listed" :
															(extra.ypErrorCode == YP_AUTH_ISSUE_CODE ? " Please contact support as there is an issue with the authhash" : " Directory returned "
																									   "error code: <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2#YP_Server_Errors\">" + tos(extra.ypErrorCode) + "</a><br>" +
															(extra.ypConnected != 2 ? "" : "during a listing update. The stream may not<br>appear in the Directory due to the error. The<br> server will attempt to re-list the stream."))))))  : "") :
													   "Private") + "</b></td><td" + (!extra.isConnected ? " rowspan=\"2\"" : (utf8)"") + ">&nbsp;&nbsp;&nbsp;</td>"
													   "<td title=\"Source User Agent: " + addWBR((!info.m_sourceIdent.empty() ? info.m_sourceIdent : "Legacy / Unknown")) + "\">"
													   "Source: <b>" + (!(streamData::isRelayActive(sid, noEntry) == 1) ?
															"Inactive relay</b> [&nbsp;<a href=\"admin.cgi?sid=" + tos(sid) +
															"&amp;mode=startrelay\">start relay</a>&nbsp;]<br>Using: <b>" + urlLink(srcAddr) + "</b>" :
															"Connection pending to " + urlLink(srcAddr) + "</b>"
															" [&nbsp;<a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=kicksrc\">abort</a>&nbsp;]") +

									  // if it's an inactive stream then we also want to show any remaining connections
									  "</td></tr>" + (data.connectedListeners > 0 ? "<tr valign=\"top\">"
									  "<td align=\"center\"><div style=\"text-align:left;padding-top:0.5em;\">Listeners: <b>" +
									  tos(data.connectedListeners) + "</b>" + (data.peakListeners > 0 ? " &nbsp;|&nbsp; Peak: <b>" +
									  tos(data.peakListeners) + "</b>" : "") + " [&nbsp;<a href=\"admin.cgi?sid=" + tos(sid) +
									  "&amp;mode=kickdst&amp;kickdst=all\">kick all</a>&nbsp;]" + "</div></td></tr>" : "") +									  
									  "</table></td></tr>";
					}
					streamBlocks[sid] = streamBody;
				}
			}
		}

		// now we check through for any known but in-active sources & then get them listed as well
		// new to build 70 but for fire builds makes it easier to see if a feed stream is inactive
		config::streams_t stream_configs;
		gOptions.getStreamConfigs(stream_configs);
		if (!stream_configs.empty())
		{
			for (config::streams_t::const_iterator i = stream_configs.begin(); i != stream_configs.end(); ++i)
			{
				if (streamBlocks.find((*i).first) == streamBlocks.end())
				{
					stats::statsData_t data;
					stats::getStats((*i).first, data);

					// increment our stream total now that we know we have one
					totalListeners += data.connectedListeners;
					totalPeakListeners += data.peakListeners;

					utf8 streamBody = "<tr><td align=\"center\" class=\"tnl\">"
									  "<a href=\"index.html?sid=" + tos((*i).first) + "\">Stream #" + tos((*i).first) + "</a> "
									  "<a href=\"admin.cgi?sid=" + tos((*i).first) + "\">(Stream Login)</a></td></tr>",
						 streamBody2;

					const utf8 movedUrl = gOptions.stream_movedUrl((*i).first);
					if (movedUrl.empty())
					{
						utf8 authhash = (*i).second.m_authHash;
						if (authhash.empty())
						{
							authhash = gOptions.stream_authHash((*i).first);
						}

						if (authhash.empty())
						{
							streamBody2 += "<div class=\"en\" style=\"padding:1em;margin-bottom:1em;display:inline-block;border-radius:0.5em;\">"
										   "<a href=\"admin.cgi?sid=" + tos((*i).first) + "&amp;mode=register\">Create Authhash</a>"
										   "&nbsp; | &nbsp;<a href=\"admin.cgi?sid=" + tos((*i).first) + "&amp;mode=register\">Manage Authhash</a></div>";
						}
						else
						{
							// check that the authhash is a valid length
							if (!yp2::isValidAuthhash(authhash))
							{
								streamBody2 += getBadAuthhashMessage((*i).first, authhash);
							}
							else
							{
								streamBody2 += "<div class=\"en\" style=\"padding:1em;margin-bottom:1em;display:inline-block;border-radius:0.5em;\">"
											   // TODO if the stream is not active then we don't know the admode
											   //	   status and so it's easier to not show the warning until
											   //	   we've got something that will allow us to check authhash.
											   //+ utf8(info.m_radionomyID.empty() ? warningImage(false) + "&nbsp;&nbsp;<b>Please Register Your Authhash</b><br><br>" : "") +
											   "<a href=\"admin.cgi?sid=" + tos((*i).first) + "&amp;mode=register\">Update Authhash</a>"
											   "&nbsp; |&nbsp;<a href=\"admin.cgi?sid=" + tos((*i).first) + "&amp;mode=register\">Manage Authhash</a></div>";
							}
						}

						if (!streamBody2.empty())
						{
							streamBody2 += "<br>";
						}
						streamBody2 += "<b>This stream is configured but has no source connected.</b>" +
									   (data.connectedListeners > 0 ? "<div style=\"text-align:center;padding-top:0.5em;\">Listeners: "
									   "<b>" + tos(data.connectedListeners) + "</b>" + (data.peakListeners > 0 ? " &nbsp;|&nbsp; "
									   "Peak: <b>" + tos(data.peakListeners) + "</b>" : "") + " [&nbsp;<a href=\"admin.cgi?sid=" +
									   tos(sid) + "&amp;mode=kickdst&amp;kickdst=all\">kick all</a>&nbsp;]" + "</div>" : "");
					}
					else
					{
						streamBody2 += "<b>This stream is configured as having been moved or retired.<br>"
									   "No source connections will be allowed for this stream.<br><br>"
									   "All client connections received will be redirected to:<br>" +
									   urlLink(movedUrl) + "</b>";
						++movedTotal;
					}

					if (!streamBody2.empty())
					{
						streamBody += "<tr><td align=\"center\">" + streamBody2 + "</td></tr>";
					}

					streamBlocks[(*i).first] = streamBody;
				}
			}
		}

		// this will now do a final check for any listeners which are on
		// an un-confiured stream but are being provided a 'backupfile'.
		const streamData::streamIDs_t activeIds = stats::getActiveStreamIds();
		if (!activeIds.empty())
		{
			for (streamData::streamIDs_t::const_iterator i = activeIds.begin(); i != activeIds.end(); ++i)
			{
				if (streamBlocks.find((*i)) == streamBlocks.end())
				{
					stats::statsData_t data;
					stats::getStats((*i), data);

					// increment our stream total now that we know we have one
					totalListeners += data.connectedListeners;
					totalPeakListeners += data.peakListeners;

					utf8 streamBody = "<tr><td align=\"center\" class=\"tnl\">"
									  "<a href=\"index.html?sid=" + tos((*i)) + "\">Stream #" + tos((*i)) + "</a> "
									  "<a href=\"admin.cgi?sid=" + tos((*i)) + "\">(Stream Login)</a></td></tr>",
						 streamBody2;
						streamBody2 += "<b>This stream is not configured and has no source connected.</b>" +
									   (data.connectedListeners > 0 ? "<div style=\"text-align:center;padding-top:0.5em;\">Listeners: "
									   "<b>" + tos(data.connectedListeners) + "</b>" + (data.peakListeners > 0 ? " &nbsp;|&nbsp; "
									   "Peak: <b>" + tos(data.peakListeners) + "</b>" : "") + " [&nbsp;<a href=\"admin.cgi?sid=" +
									   tos(sid) + "&amp;mode=kickdst&amp;kickdst=all\">kick all</a>&nbsp;]" + "</div>" : "");

					if (!streamBody2.empty())
					{
						streamBody += "<tr><td align=\"center\">" + streamBody2 + "</td></tr>";
					}

					streamBlocks[(*i)] = streamBody;
				}
			}
		}

		// now build up the output since if we've factored in inactive relay connections
		// then we could otherwise be showing the streams in an out of order manner
		for (map<size_t,uniString::utf8>::const_iterator i = streamBlocks.begin(); i != streamBlocks.end(); ++i)
		{
			if (!(*i).second.empty())
			{
				if (streamTotal > 0)
				{
					streams += "<tr><td align=\"center\"><br><hr><br></td></tr>";
				}
				streams += (*i).second;
				++streamTotal;
			}
		}

		// output a refresh option if there were no active streams found
		if (!streamTotal)
		{
			if (isPostSetup() && isPostSetup() <= 2)
			{
				streams += "<tr><td align=\"center\">"
						   "<br><div align=\"center\" class=\"infh\" style=\"border-bottom:0;top:-19px;\">"
						   "Setup Successfully Completed</div></td></tr><tr><td align=\"center\">"
						   "No stream sources are currently connected.<br>"
						   "To find newly connected sources, <a href=\"admin.cgi?sid=0\">click here.</a></td></tr>";
				setPostSetup(isPostSetup() + 1);
			}
			else
			{
				streams += "<tr><td align=\"center\">No stream sources are currently connected.<br>"
						   "To find newly connected sources, <a href=\"admin.cgi?sid=0\">click here.</a><br><br></td></tr>";
			}
		}
		else
		{
			setPostSetup(0);
		}
	}
	else if (refreshRequired > 0)
	{
		streams += "<tr><td align=\"center\" id=\"counter\">"
				   "Waiting " + tos(refreshRequired) + " second" + ((refreshRequired > 1) ? "s" : (utf8)"") +
				   " for any configuration changes to take effect.<br>"
				   "If not automatically redirected or do not want to wait, "
				   "<a href=\"admin.cgi?sid=0\">click here.</a><br><br></td></tr>";
	}
	else if (refreshRequired < 0)
	{
		streams += "<tr><td align=\"center\" id=\"counter\">"
				   "Waiting " + tos(abs(refreshRequired)) + " second" + ((abs(refreshRequired) > 1) ? "s" : (utf8)"") +
				   " whilst checking for any DNAS updates.<br>If not automatically redirected or do not want to wait, "
				   "<a href=\"admin.cgi?sid=0\">click here.</a><br><br></td></tr>";
	}

	utf8 log = gOptions.realLogFile();
	utf8::size_type pos = log.rfind(fileUtil::getFilePathDelimiter());
	if ((pos != utf8::npos))
	{
		log = log.substr(pos + 1);
	}

	utf8 conf = gOptions.confFile();
	pos = conf.rfind(fileUtil::getFilePathDelimiter());
	if ((pos != utf8::npos))
	{
		conf = conf.substr(pos + 1);
	}

	const bool requires = gOptions.requireStreamConfigs();
	body += "<table cellpadding=\"5\" cellspacing=\"0\" border=\"0\" width=\"100%\">"
			"<tr><td class=\"tsp\" align=\"center\">"
			"Available Streams: " + tos(streamTotal - movedTotal) +
			"&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;"
			"Server Listeners: " + tos(totalListeners) +
			"&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;"
			"Peak Server Listeners: " + tos(totalPeakListeners) +
			"&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;"
			"Unique Listeners: " + tos(stats::getTotalUniqueListeners()) +
			"</td></tr></table>"

			"<div style=\"padding:0 1em;\">"
			"<br><table width=\"100%\" align=\"center\"><tr valign=\"top\"><td>"

			"<table class=\"en\" cellpadding=\"15px\" style=\"border:0;display:block;padding-right:1em;\"><tr>"
			"<td valign=\"top\" style=\"max-width:20em;\">"
			"<div align=\"center\" class=\"infh\"><b>Server Management</b></div>"
			"Rotate Log File(s):"
			" [&nbsp;<a href=\"admin.cgi?mode=rotate\">All</a>&nbsp;]"
			"&nbsp; [&nbsp;<a href=\"admin.cgi?mode=rotate&amp;files=log\">Log</a>&nbsp;]"
			"&nbsp; [&nbsp;<a href=\"admin.cgi?mode=rotate&amp;files=w3c\">W3C</a>&nbsp;]"
			"<br><br>"

			// trim down the file paths shown to make things less cluttered on hosted setups
			// places the full path in the 'title' so it can still be found if required, etc
			"Log File: <b title=\"" +
			aolxml::escapeXML(fileUtil::getFullFilePath(gOptions.realLogFile())) +
			"\">" + aolxml::escapeXML(fileUtil::stripPath(log)) + "</b><br><br>"
			"Configuration File: <b title=\"" +
			aolxml::escapeXML(fileUtil::getFullFilePath(gOptions.confFile())) +
			"\">" + aolxml::escapeXML(fileUtil::stripPath(conf)) + "</b><br>"
			"[&nbsp;<a href=\"admin.cgi?mode=config\">View</a>&nbsp;]"
			"&nbsp; [&nbsp;<a href=\"admin.cgi?mode=reload\">Update</a>&nbsp;]"
			"&nbsp; [&nbsp;<a href=\"admin.cgi?mode=reload&amp;force=1\">Forced</a>&nbsp;]"

			"<br><br><hr><br>Source Connection Details:"
			" [&nbsp;<a href=\"admin.cgi?mode=sources\">View</a>&nbsp;]<br>" +

			(requires ? "<br><b>Note:</b> Only pre-configured streams are "
						"allowed to connect / be run on this server.<br>" : "") +

			"<br>Advert Group Details:"
			" [&nbsp;<a href=\"admin.cgi?mode=adgroups\">View</a>&nbsp;]<br>"

			"<br><hr><br>Stream Configuration(s):"
			" [&nbsp;<a href=\"admin.cgi?sid=1&amp;mode=viewjson&amp;page=6\">JSON</a>&nbsp;]"
			" &nbsp;[&nbsp;<a href=\"admin.cgi?sid=1&amp;mode=viewxml&amp;page=6\">XML</a>&nbsp;]<br>"

			"<br>View Stream Statistics:"
			" [&nbsp;<a href=\"statistics?json=1\">JSON</a>&nbsp;]"
			"&nbsp; [&nbsp;<a href=\"statistics\">XML</a>&nbsp;]<br>"

			"<br><hr><br>Manage Stream Source(s):<br>"
			"[&nbsp;<a href=\"admin.cgi?mode=startrelays\">Start Relays</a>&nbsp;]"
			"&nbsp; [&nbsp;<a href=\"admin.cgi?mode=kicksources\">Kick / Stop All</a>&nbsp;]<br>"

			"<br><hr><br>Reload Banned List(s):"
			" [&nbsp;<a href=\"admin.cgi?mode=bannedlist\">Update</a>&nbsp;]<br>"

			"<br>Reload Reserved List(s):"
			" [&nbsp;<a href=\"admin.cgi?mode=reservelist\">Update</a>&nbsp;]<br>"

			"<br>Reload User Agent List(s):"
			" [&nbsp;<a href=\"admin.cgi?mode=useragentlist\">Update</a>&nbsp;]<br>"

			"<br>Reload Admin Access List:"
			" [&nbsp;<a href=\"admin.cgi?mode=adminlist\">Update</a>&nbsp;]<br>"

			"<br>Clear Resource / Page Cache:"
			" [&nbsp;<a href=\"admin.cgi?mode=clearcache\">Clear</a>&nbsp;]<br>"

			"<br><hr><br>Debugging Options:<br>"
			"[&nbsp;<a href=\"admin.cgi?mode=debug&amp;option=all&amp;on=true\">Enable&nbsp;All</a>&nbsp;]"
			"&nbsp; [&nbsp;<a href=\"admin.cgi?mode=debug&amp;option=all&amp;on=false\">Disable&nbsp;All</a>&nbsp;]"
			"&nbsp; [&nbsp;<a href=\"admin.cgi?mode=debug\">Edit</a>&nbsp;]"

			"<br><br><hr><br>New DNAS Release: [&nbsp;<a href=\"admin.cgi?mode=version\">Check</a>&nbsp;]<br>"
			"Last checked: <b>" + getCheckedDuration((size_t)(m_lastActivityTime - last_update_check)) + "</b>"
			"</td></tr></table></td><td>";

	// display update message where applicable
	updater::verInfo ver;
	utf8 update_msg;
	if (updater::getNewVersion(ver))
	{
		update_msg += "<tr><td align=\"center\" class=\"tnl\">"
					  "<table cellspacing=\"0\" cellpadding=\"5\"><tr><td align=\"center\" class=\"infb\">"
					  "<div align=\"center\" class=\"infh\"><b>New DNAS Version Available</b></div>A new version of the DNAS is now available:"
					  " <b>" + ver.ver + "</b><br><br>" +
					  (!ver.url.empty() ?
					  ((ver.downloaded && !ver.fn.empty()) ? "The new version has been automatically downloaded to:<br><br><b>" + aolxml::escapeXML(ver.fn) + "</b><br><br>"
															 "Please install this update as soon as possible, thank you.<br><br>"
															 "Specific changes for this version can be found <a href=\"" + aolxml::escapeXML(ver.info) + "\"><b>here</b></a>."
														   :
															"Click <a href=\"" + aolxml::escapeXML(ver.url) + "\"><b>here</b></a> to download the new version of the DNAS.<br><br>"
															"Please install this update as soon as possible, thank you.<br><br>"
															"Specific changes for this version can be found <a href=\"" + aolxml::escapeXML(ver.info) + "\"><b>here</b></a>.") : "") +
					  (!ver.message.empty() ? "<br>" + ver.message :
					   (!ver.log.empty() ? "<br>For more details of the changes in this version see <a target=\"_blank\" href=\"" + aolxml::escapeXML(ver.log) + "\">" +
					  "<b>here</b></a>." : "")) + "</td></tr></table><br></td></tr>"
					  "<tr><td align=\"center\"><hr><br></td></tr>";
	}

	body += "<table cellspacing=\"0\" cellpadding=\"5\" border=\"0\">" +
			update_msg + streams + "</table></td></tr></table></div>" +
			"<script type=\"text/javascript\">"
			"function $(id){return document.getElementById(id);}" EL;

	// for a refresh, we'll show a countdown so it's obvious that something is happening
	if (refreshRequired > 0)
	{
		body += "var c = " + tos(refreshRequired) + ";" EL
				"function counter(){" EL
				"c--;" EL
				"if(c>0){" EL
				"$('counter').innerHTML=\"Waiting \"+c+\" second\" + (c > 1 ? \"s\" : \"\") + \" for any configuration changes to take effect.<br>"
				"If not automatically redirected or do not want to wait, <a href=\\\"admin.cgi\\\">click here.</a><br><br>\";" EL
				"}" EL
				"}" EL
				"setInterval(counter,1000);" EL;
	}
	else if (refreshRequired < 0)
	{
		body += "var c = " + tos(abs(refreshRequired)) + ";" EL
				"function counter(){" EL
				"c--;" EL
				"if(c>0){" EL
				"$('counter').innerHTML=\"Waiting \"+c+\" second\" + (c > 1 ? \"s\" : \"\") + \" whilst checking for any DNAS updates.<br>"
				"If not automatically redirected or do not want to wait, <a href=\\\"admin.cgi\\\">click here.</a><br><br>\";" EL
				"}" EL
				"}" EL
				"setInterval(counter,1000);" EL;
	}

	body += getUptimeScript(true) + "</script>" +
			getIEFlexFix() + getHTML5Remover() + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_bandwidth_html(const int refreshRequired) throw()
{
	const __uint64 all = bandWidth::getAmount(bandWidth::ALL),
				   sent = bandWidth::getAmount(bandWidth::ALL_SENT),
				   recv = bandWidth::getAmount(bandWidth::ALL_RECV);
	const time_t running = (::time(NULL) - g_upTime);

	utf8 header = MSG_NO_CLOSE_200,
		 body = getServerAdminHeader("Server Bandwidth Usage", refreshRequired, "&amp;mode=bandwidth&amp;refresh=" + tos(abs(refreshRequired)), 1) +
				"<table cellpadding=\"5\" cellspacing=\"0\" border=\"0\" width=\"100%\">"
				"<tr><td class=\"tsp\" align=\"center\">"
				"Total: <b>" + formatSizeString(all) + (all > 0 ? " (~" + formatSizeString((all / running) * 86400) + " / day)" : "") + "</b>"
				"&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;"
				"Sent: <b>" + formatSizeString(sent) + (sent > 0 ? " (~" + formatSizeString((sent / running) * 86400) + " / day)" : "") + "</b>"
				"&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;"
				"Received: <b>" + formatSizeString(recv) + (recv > 0 ? " (~" + formatSizeString((recv / running) * 86400) + " / day)" : "") + "</b>"
				"</td></tr></table><div><br>"

				"<table align=\"center\" class=\"en\" cellspacing=\"0\" cellpadding=\"3\" style=\"border:0;\">"

				"<tr class=\"infh\"><td class=\"t\">&nbsp;Total Client Data Sent</td>"
				"<td class=\"st\"><b>" + formatSizeString(bandWidth::getAmount(bandWidth::ALL_CLIENT_SENT)) + "</b></td></tr>"
				"<tr class=\"t\"><td>v2 Client(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::CLIENT_V2_SENT)) + "</td></tr>"
				"<tr class=\"t\"><td>v1 Client(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::CLIENT_V1_SENT)) + "</td></tr>"
				"<tr class=\"t\"><td>HTTP Client(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::CLIENT_HTTP_SENT)) + "</td></tr>"
				"<tr class=\"t\"><td>FLV Client(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::CLIENT_FLV_SENT)) + "</td></tr>"
#if 0
				"<tr class=\"t\"><td>M4A Client(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::CLIENT_M4A_SENT)) + "</td></tr>"
#endif
				"<tr><td style=\"border:0;\">&nbsp;</td></tr>"

				"<tr class=\"infh\"><td class=\"t\">&nbsp;Total Source Data Received</td>"
				"<td class=\"st\"><b>" + formatSizeString(bandWidth::getAmount(bandWidth::ALL_SOURCE_RECV)) + "</b></td></tr>"
				"<tr class=\"t\"><td>v2 Source(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::SOURCE_V2_RECV)) + "</td></tr>"
				"<tr class=\"t\"><td>v1 Source(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::SOURCE_V1_RECV)) + "</td></tr>"
				"<tr><td style=\"border:0;\">&nbsp;</td></tr>"

				"<tr class=\"infh\"><td class=\"t\">&nbsp;Total Source Data Sent</td>"
				"<td class=\"st\"><b>" + formatSizeString(bandWidth::getAmount(bandWidth::ALL_SOURCE_SENT)) + "</b></td></tr>"
				"<tr class=\"t\"><td>v2 Source(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::SOURCE_V2_SENT)) + "</td></tr>"
				"<tr class=\"t\"><td>v1 Source(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::SOURCE_V1_SENT)) + "</td></tr>"
				"<tr><td style=\"border:0;\">&nbsp;</td></tr>"

				"<tr class=\"infh\"><td class=\"t\">&nbsp;Total Relay Data Received</td>"
				"<td class=\"st\"><b>" + formatSizeString(bandWidth::getAmount(bandWidth::ALL_RELAY_RECV)) + "</b></td></tr>"
				"<tr class=\"t\"><td>Handshaking</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::RELAY_MISC_RECV)) + "</td></tr>"
				"<tr class=\"t\"><td>v2 Relay(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::RELAY_V2_RECV)) + "</td></tr>"
				"<tr class=\"t\"><td>v1 Relay(s)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::RELAY_V1_RECV)) + "</td></tr>"
				"<tr><td style=\"border:0;\">&nbsp;</td></tr>"

				"<tr class=\"infh\"><td class=\"t\">&nbsp;Total Web Page, XML and Resoures&nbsp;&nbsp;</td>"
				"<td class=\"st\"><b>" + formatSizeString(bandWidth::getAmount(bandWidth::ALL_WEB)) + "</b></td></tr>"
				"<tr class=\"t\"><td>Public (e.g. /stats or index.html)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::PUBLIC_WEB)) + "</td></tr>"
				"<tr class=\"t\"><td>Private (e.g. /admin.cgi pages)</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::PRIVATE_WEB)) + "</td></tr>"
				"<tr><td style=\"border:0;\">&nbsp;</td></tr>"

				"<tr class=\"infh\"><td class=\"t\">&nbsp;Total Other Data</td>"
				"<td class=\"st\"><b>" + formatSizeString(bandWidth::getAmount(bandWidth::ALL_OTHER)) + "</b></td></tr>"
				"<tr class=\"t\"><td>Flash Policy Server</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::FLASH_POLICY)) + "</td></tr>"
				"<tr class=\"t\"><td>v2 Relay(s) Sent</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::RELAY_V2_SENT)) + "</td></tr>"
				"<tr class=\"t\"><td>YP Sent</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::YP_SENT)) + "</td></tr>"
				"<tr class=\"t\"><td>YP Received</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::YP_RECV)) + "</td></tr>"
				"<tr class=\"t\"><td>Listener Authentication and Metrics</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::AUTH_AND_METRICS)) + "</td></tr>"
				"<tr class=\"t\"><td>Advert Retrieval</td>"
				"<td class=\"s\">" + formatSizeString(bandWidth::getAmount(bandWidth::ADVERTS)) + "</td></tr>"

				"</table></div>" + getUptimeScript() + getIEFlexFix() + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_bandwidth_xml() throw()
{
	utf8 header = "HTTP/1.1 200 OK\r\nContent-Type:text/xml\r\n",
		 body = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>"
				"<SHOUTCASTSERVER>"

				"<TOTAL>" + tos(bandWidth::getAmount(bandWidth::ALL)) + "</TOTAL>"
				"<SENT>" + tos(bandWidth::getAmount(bandWidth::ALL_SENT)) + "</SENT>"
				"<RECV>" + tos(bandWidth::getAmount(bandWidth::ALL_RECV)) + "</RECV>"
				"<TIME>" + tos((::time(NULL) - g_upTime)) + "</TIME>"
		
				"<CLIENTSENT>"
				"<TOTAL>" + tos(bandWidth::getAmount(bandWidth::ALL_CLIENT_SENT)) + "</TOTAL>"
				"<V2>" + tos(bandWidth::getAmount(bandWidth::CLIENT_V2_SENT)) + "</V2>"
				"<V1>" + tos(bandWidth::getAmount(bandWidth::CLIENT_V1_SENT)) + "</V1>"
				"<HTTP>" + tos(bandWidth::getAmount(bandWidth::CLIENT_HTTP_SENT)) + "</HTTP>"
				"<FLV>" + tos(bandWidth::getAmount(bandWidth::CLIENT_FLV_SENT)) + "</FLV>"
#if 0
				"<M4A>" + tos(bandWidth::getAmount(bandWidth::CLIENT_M4A_SENT)) + "</M4A>"
#endif
				"</CLIENTSENT>"

				"<SOURCERECV>"
				"<TOTAL>" + tos(bandWidth::getAmount(bandWidth::ALL_SOURCE_RECV)) + "</TOTAL>"
				"<V2>" + tos(bandWidth::getAmount(bandWidth::SOURCE_V2_RECV)) + "</V2>"
				"<V1>" + tos(bandWidth::getAmount(bandWidth::SOURCE_V1_RECV)) + "</V1>"
				"</SOURCERECV>"

				"<SOURCESENT>"
				"<TOTAL>" + tos(bandWidth::getAmount(bandWidth::ALL_SOURCE_SENT)) + "</TOTAL>"
				"<V2>" + tos(bandWidth::getAmount(bandWidth::SOURCE_V2_SENT)) + "</V2>"
				"<V1>" + tos(bandWidth::getAmount(bandWidth::SOURCE_V1_SENT)) + "</V1>"
				"</SOURCESENT>"

				"<RELAYRECV>"
				"<TOTAL>" + tos(bandWidth::getAmount(bandWidth::ALL_RELAY_RECV)) + "</TOTAL>"
				"<MISC>" + tos(bandWidth::getAmount(bandWidth::RELAY_MISC_RECV)) + "</MISC>"
				"<V2>" + tos(bandWidth::getAmount(bandWidth::RELAY_V2_RECV)) + "</V2>"
				"<V1>" + tos(bandWidth::getAmount(bandWidth::RELAY_V1_RECV)) + "</V1>"
				"</RELAYRECV>"

				"<WEBPAGES>"
				"<TOTAL>" + tos(bandWidth::getAmount(bandWidth::ALL_WEB)) + "</TOTAL>"
				"<PUBLIC>" + tos(bandWidth::getAmount(bandWidth::PUBLIC_WEB)) + "</PUBLIC>"
				"<PRIVATE>" + tos(bandWidth::getAmount(bandWidth::PRIVATE_WEB)) + "</PRIVATE>"
				"</WEBPAGES>"

				"<OTHER>"
				"<TOTAL>" + tos(bandWidth::getAmount(bandWidth::ALL_OTHER)) + "</TOTAL>"
				"<FLASH>" + tos(bandWidth::getAmount(bandWidth::FLASH_POLICY)) + "</FLASH>"
				"<RELAYSENTV2>" + tos(bandWidth::getAmount(bandWidth::RELAY_V2_SENT)) + "</RELAYSENTV2>"
				"<YPSENT>" + tos(bandWidth::getAmount(bandWidth::YP_SENT)) + "</YPSENT>"
				"<YPRECV>" + tos(bandWidth::getAmount(bandWidth::YP_RECV)) + "</YPRECV>"
				"<AUTH_METRICS>" + tos(bandWidth::getAmount(bandWidth::AUTH_AND_METRICS)) + "</AUTH_METRICS>"
				"<ADVERTS>" + tos(bandWidth::getAmount(bandWidth::ADVERTS)) + "</ADVERTS>"
				"</OTHER>"
				"</SHOUTCASTSERVER>";

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_bandwidth_json(const uniString::utf8& callback) throw()
{
	const bool jsonp = !callback.empty();
	utf8 header = "HTTP/1.1 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n",
		 body = (jsonp ? callback + "(" : "") +
				"{"
				"\"total\":" + tos(bandWidth::getAmount(bandWidth::ALL)) + ","
				"\"sent\":" + tos(bandWidth::getAmount(bandWidth::ALL_SENT)) + ","
				"\"recv\":" + tos(bandWidth::getAmount(bandWidth::ALL_RECV)) + ","
				"\"time\":" + tos((::time(NULL) - g_upTime)) + ","
				"\"clientsent\":{"
				"\"total\":" + tos(bandWidth::getAmount(bandWidth::ALL_CLIENT_SENT)) + ","
				"\"v2\":" + tos(bandWidth::getAmount(bandWidth::CLIENT_V2_SENT)) + ","
				"\"v1\":" + tos(bandWidth::getAmount(bandWidth::CLIENT_V1_SENT)) + ","
				"\"http\":" + tos(bandWidth::getAmount(bandWidth::CLIENT_HTTP_SENT)) + ","
				"\"flv\":" + tos(bandWidth::getAmount(bandWidth::CLIENT_FLV_SENT)) +
#if 0
				","
				"\"m4a\":" + tos(bandWidth::getAmount(bandWidth::CLIENT_M4A_SENT)) +
#endif
				"},\"sourcerecv\":{"
				"\"total\":" + tos(bandWidth::getAmount(bandWidth::ALL_SOURCE_RECV)) + ","
				"\"v2\":" + tos(bandWidth::getAmount(bandWidth::SOURCE_V2_RECV)) + ","
				"\"v1\":" + tos(bandWidth::getAmount(bandWidth::SOURCE_V1_RECV)) +
				"},\"sourcesent\":{"
				"\"total\":" + tos(bandWidth::getAmount(bandWidth::ALL_SOURCE_SENT)) + ","
				"\"v2\":" + tos(bandWidth::getAmount(bandWidth::SOURCE_V2_SENT)) + ","
				"\"v1\":" + tos(bandWidth::getAmount(bandWidth::SOURCE_V2_SENT)) +
				"},\"relayrecv\":{"
				"\"total\":" + tos(bandWidth::getAmount(bandWidth::ALL_SOURCE_RECV)) + ","
				"\"misc\":" + tos(bandWidth::getAmount(bandWidth::RELAY_MISC_RECV)) + ","
				"\"v2\":" + tos(bandWidth::getAmount(bandWidth::RELAY_V2_RECV)) + ","
				"\"v1\":" + tos(bandWidth::getAmount(bandWidth::RELAY_V1_RECV)) +
				"},\"webpages\":{"
				"\"total\":" + tos(bandWidth::getAmount(bandWidth::ALL_WEB)) + ","
				"\"public\":" + tos(bandWidth::getAmount(bandWidth::PUBLIC_WEB)) + ","
				"\"private\":" + tos(bandWidth::getAmount(bandWidth::PRIVATE_WEB)) +
				"},\"other\":{"
				"\"total\":" + tos(bandWidth::getAmount(bandWidth::ALL_OTHER)) + ","
				"\"flash\":" + tos(bandWidth::getAmount(bandWidth::FLASH_POLICY)) + ","
				"\"relaysentv2\":" + tos(bandWidth::getAmount(bandWidth::RELAY_V2_SENT)) + ","
				"\"ypsent\":" + tos(bandWidth::getAmount(bandWidth::YP_SENT)) + "," +
				"\"yprecv\":" + tos(bandWidth::getAmount(bandWidth::YP_RECV)) + "," +
				"\"auth_metrics\":" + tos(bandWidth::getAmount(bandWidth::AUTH_AND_METRICS)) + "," +
				"\"adverts\":" + tos(bandWidth::getAmount(bandWidth::ADVERTS)) +
				"}}" + (jsonp ? utf8(")") : "");

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_ypstatus_xml() throw()
{
	utf8 header = "HTTP/1.1 200 OK\r\nContent-Type:text/xml\r\n",
		 body = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>"
				"<SHOUTCASTSERVER>";

	streamData::streamIDs_t streamIds = streamData::getStreamIds();
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		if ((*i).second.m_streamID)
		{
			streamIds.insert((*i).second.m_streamID);
		}
	}

	for (streamData::streamIDs_t::const_iterator i = streamIds.begin(); i != streamIds.end(); ++i)
	{
		streamData::streamInfo info;
		streamData::extraInfo extra;
		streamData::getStreamInfo((*i), info, extra);

		const utf8 movedUrl = gOptions.stream_movedUrl((*i));
		if (movedUrl.empty())
		{
			body += "<STREAM id=\"" + tos((*i)) + "\">" +
					(!extra.isConnected ? "<STATUS>NOSOURCE</STATUS>" :
						(info.m_streamPublic ? (extra.ypConnected != 1 ? (!yp2::isValidAuthhash(info.m_authHash) ?
							(info.m_authHash.empty() ? "<STATUS>EMPTY_AUTHHASH</STATUS>" : "<STATUS>INVALID_AUTHHASH</STATUS>") :
								(extra.ypErrorCode == 200 ? "<STATUS>WAITING</STATUS>" :
									(extra.ypErrorCode == YP_COMMS_FAILURE ? "<STATUS>YP_NOT_FOUND</STATUS>" :
										(extra.ypErrorCode == YP_MAINTENANCE_CODE ? "<STATUS>YP_MAINTENANCE</STATUS>" :
											"<STATUS>ERROR</STATUS><CODE>" + tos(extra.ypErrorCode) + "</CODE>")))) :
												"<STATUS>PUBLIC</STATUS><STNID>" + info.m_stationID + "</STNID>") :
													"<STATUS>PRIVATE</STATUS>")) + "</STREAM>";
		}
		else
		{
			body += "<STREAM id=\"" + tos((*i)) + "\"><STATUS>MOVED</STATUS></STREAM>";
		}
	}

	body += "</SHOUTCASTSERVER>";
	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_ypstatus_json(const uniString::utf8& callback) throw()
{
	utf8 header = "HTTP/1.1 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n";

	streamData::streamIDs_t streamIds = streamData::getStreamIds();
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		if ((*i).second.m_streamID)
		{
			streamIds.insert((*i).second.m_streamID);
		}
	}

	const bool jsonp = !callback.empty();
	utf8 body = (jsonp ? callback + "(" : "") + "{" +
				(!streamIds.empty() ? "\"streams\":[" : "");

	bool read = false;
	for (streamData::streamIDs_t::const_iterator i = streamIds.begin(); i != streamIds.end(); ++i)
	{
		streamData::streamInfo info;
		streamData::extraInfo extra;
		streamData::getStreamInfo((*i), info, extra);

		const utf8 movedUrl = gOptions.stream_movedUrl((*i));
		if (movedUrl.empty())
		{
			body += (read ? utf8(",") : "") +
					"{"
					"\"id\":" + tos((*i)) + "," +
					(!extra.isConnected ? "\"status\":\"" + escapeJSON("nosource") + "\"" :
						(info.m_streamPublic ? (extra.ypConnected != 1 ? (!yp2::isValidAuthhash(info.m_authHash) ?
							(info.m_authHash.empty() ? "\"status\":\"" + escapeJSON("empty_authhash") + "\"" :
								"\"status\":\"" + escapeJSON("invalid_authhash") + "\"") :
								(extra.ypErrorCode == 200 ? "\"status\":\"" + escapeJSON("waiting") + "\"" :
									(extra.ypErrorCode == YP_COMMS_FAILURE ? "\"status\":\"" + escapeJSON("yp_not_found") + "\"" :
										(extra.ypErrorCode == YP_MAINTENANCE_CODE ? "\"status\":\"" + escapeJSON("yp_maintenance") + "\"" :
											"\"status\":\"" + escapeJSON("error") + "\",\"code\":" + tos(extra.ypErrorCode))))) :
												"\"status\":\"" + escapeJSON("public") + "\",\"stnid\":" + info.m_stationID) :
													"\"status\":\"" + escapeJSON("private") + "\"")) + "}";
		}
		else
		{
			body += (read ? utf8(",") : "") + "{\"id\":" + tos((*i)) + ","
					"\"status\":\"" + escapeJSON("moved") + "\"}";
		}
		read = true;
	}

	body += (!streamIds.empty() ? utf8("]") : "") + "}" + (jsonp ? utf8(")") : "");

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_viewxml(const streamData::streamID_t sid, int page,
									 const bool iponly, const bool ipcount) throw()
{
	// abort as there's nothing generated for this now
	if (page == 2)
	{
		sendMessageAndClose("HTTP/1.1 200 OK\r\nContent-Type:text/xml\r\n\r\n");
		return;
	}

	if ((page > 6) || (page < 0))
	{
		page = 0;
	}

	utf8 header = "HTTP/1.1 200 OK\r\nContent-Type:text/xml\r\n",
		 body = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?><SHOUTCASTSERVER>";

	if (page < 2)
	{
		stats::statsData_t data;
		body += protocol_HTTPStyle::getStatsXMLBody(sid, true, true, m_socket, data);
	}

	if ((page == 0) || (page == 3))
	{
		time_t t = ::time(NULL);
		body += "<LISTENERS>";

		stats::currentClientList_t client_data;
		stats::getClientDataForStream(sid, client_data);
		map<utf8, size_t> ip_counts;
		for (stats::currentClientList_t::const_iterator i = client_data.begin(); i != client_data.end(); ++i)
		{
			const utf8& host = ((*i)->m_hostName != (*i)->m_ipAddr ? (*i)->m_hostName : (*i)->m_ipAddr);

			if (!ipcount)
			{
				if (!iponly)
				{
					body += "<LISTENER>"
							"<HOSTNAME>" + aolxml::escapeXML(host) + "</HOSTNAME>"
							"<USERAGENT>" + aolxml::escapeXML((*i)->m_userAgent) + "</USERAGENT>"
							"<CONNECTTIME>" + tos(t - (*i)->m_startTime) + "</CONNECTTIME>"
							"<UID>" + tos((*i)->m_unique) + "</UID>"
							"<TYPE>" + tos((*i)->m_clientType) + "</TYPE>"
							"<REFERER>" + aolxml::escapeXML((*i)->m_referer) + "</REFERER>"
							"<XFF>" + aolxml::escapeXML((*i)->m_XFF) + "</XFF>"
							"<GRID>" + tos((*i)->m_group) + "</GRID>"
							"<TRIGGERS>" + tos((*i)->m_triggers) + "</TRIGGERS>"
							"</LISTENER>";
				}
				else
				{
					body += "<LISTENER>"
							"<HOSTNAME>" + aolxml::escapeXML(host) + "</HOSTNAME>"
							"</LISTENER>";
				}
			}
			else
			{
				++ip_counts[host];
			}

			delete (*i);
		}

		if (ipcount)
		{
			for (map<utf8, size_t>::const_iterator i = ip_counts.begin(); i != ip_counts.end(); ++i)
			{
				body += "<LISTENER>"
						"<HOSTNAME>" + aolxml::escapeXML((*i).first) + "</HOSTNAME>"
						"<TOTAL>" + tos((*i).second) + "</TOTAL>"
						"</LISTENER>";
			}
		}

		body += "</LISTENERS>";
	}

	if ((page == 0) || (page == 4) || (page == 5))
	{
		streamData::streamHistory_t songHistory;
		streamData::getStreamSongHistory(sid, songHistory);

		// only provide this as an option feature so as not to bloat the main xml stats unnecessarily
		if (!(page == 5))
		{
			body += "<SONGHISTORY>";
			for (streamData::streamHistory_t::const_iterator i = songHistory.begin(); i != songHistory.end(); ++i)
			{
				body += "<SONG><PLAYEDAT>" + tos((*i).m_when) + "</PLAYEDAT>"
						"<TITLE>" + aolxml::escapeXML((*i).m_title) + "</TITLE>" +
						protocol_HTTPStyle::getCurrentXMLMetadataBody(true, (*i).m_metadata) + "</SONG>";
			}
			body += "</SONGHISTORY>";
		}
		else
		{
			body += protocol_HTTPStyle::getCurrentXMLMetadataBody(false, (!songHistory.empty() ? songHistory[0].m_metadata : ""));
		}
	}

	if (page == 6)
	{
		const int maxUsers = gOptions.maxUser();
		body += "<STREAMCONFIGS>"
				"<REQUIRECONFIGS>" + tos(gOptions.requireStreamConfigs()) + "</REQUIRECONFIGS>"
				"<SERVERMAXLISTENERS>" + (maxUsers > 0 ? tos(maxUsers) : "UNLIMITED") + "</SERVERMAXLISTENERS>"
				"<SERVERMINBITRATE>" + tos(gOptions.minBitrate()) + "</SERVERMINBITRATE>"
				"<SERVERMAXBITRATE>" + tos(gOptions.maxBitrate()) + "</SERVERMAXBITRATE>"
				"<TOTALCONFIGS>";

		config::streams_t streams;
		gOptions.getStreamConfigs(streams);

		utf8 block = "";
		size_t moved = 0;
		for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
		{
			const utf8 movedUrl = gOptions.stream_movedUrl((*i).second.m_streamID);
			if (movedUrl.empty())
			{
				utf8 streamTag = ((*i).second.m_streamID > 1 ? "/stream/" + tos((*i).second.m_streamID) + "/" : "/");
				if (!(*i).second.m_urlPath.empty())
				{
					streamTag = (*i).second.m_urlPath;
				}

				const utf8& authhash = (*i).second.m_authHash;
				block += "<STREAMCONFIG id=\"" + tos((*i).second.m_streamID) + "\">"
						 "<STREAMAUTHHASH>" + aolxml::escapeXML(authhash.empty() ? "EMPTY" : !yp2::isValidAuthhash(authhash) ? "INVALID" : authhash) + "</STREAMAUTHHASH>"
						 "<STREAMPATH>" + aolxml::escapeXML(streamTag) + "</STREAMPATH>"
						 "<STREAMRELAYURL>" + aolxml::escapeXML((*i).second.m_relayUrl.url()) + "</STREAMRELAYURL>"
						 "<STREAMBACKUPURL>" + aolxml::escapeXML((*i).second.m_backupUrl.url()) + "</STREAMBACKUPURL>"
						 "<STREAMMAXLISTENERS>" + ((*i).second.m_maxStreamUser > 0 && (*i).second.m_maxStreamUser < gOptions.maxUser() ? tos((*i).second.m_maxStreamUser) : "SERVERMAXLISTENERS") + "</STREAMMAXLISTENERS>"
						 "<STREAMMINBITRATE>" + ((*i).second.m_minStreamBitrate > 0 ? tos((*i).second.m_minStreamBitrate) : "SERVERMINBITRATE") + "</STREAMMINBITRATE>"
						 "<STREAMMAXBITRATE>" + ((*i).second.m_maxStreamBitrate > 0 ? tos((*i).second.m_maxStreamBitrate) : "SERVERMAXBITRATE") + "</STREAMMAXBITRATE>"
						 "<STREAMPUBLIC>" + aolxml::escapeXML((*i).second.m_publicServer) + "</STREAMPUBLIC>"
						 "<STREAMALLOWRELAY>" + tos((*i).second.m_allowRelay) + "</STREAMALLOWRELAY>"
						 "<STREAMPUBLICRELAY>" + tos((*i).second.m_allowPublicRelay) + "</STREAMPUBLICRELAY>"
						 "</STREAMCONFIG>";
			}
			else
			{
				++moved;
			}
		}

		body += tos((streams.size() - moved)) + "</TOTALCONFIGS>" + block + "</STREAMCONFIGS>";
	}

	body += "</SHOUTCASTSERVER>";

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_viewjson(const streamData::streamID_t sid, int page,
									  const bool iponly, const bool ipcount,
									  const uniString::utf8& callback) throw()
{
	// abort as there's nothing generated for this now
	if (page == 2)
	{
		sendMessageAndClose("HTTP/1.1 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n\r\n");
		return;
	}

	if (page > 6 || page < 0)
	{
		page = 0;
	}

	const bool jsonp = !callback.empty();
	utf8 header = "HTTP/1.1 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n",
		 body = (jsonp ? callback + "(" : "") + (page < 2 ? "{" : "");

	if (page < 2)
	{
		stats::statsData_t data;
		body += protocol_HTTPStyle::getStatsJSONBody(sid, true, true, m_socket, data);
	}

	if ((page == 0) || (page == 3))
	{
		time_t t = ::time(NULL);
		body += (!page ? utf8(",\"listeners\":[") : "[");

		stats::currentClientList_t client_data;
		stats::getClientDataForStream(sid, client_data);
		map<utf8, size_t> ip_counts;
		for (stats::currentClientList_t::const_iterator i = client_data.begin(); i != client_data.end(); ++i)
		{
			const utf8& host = ((*i)->m_hostName != (*i)->m_ipAddr ? (*i)->m_hostName : (*i)->m_ipAddr);

			if (!ipcount)
			{
				if (!iponly)
				{
					body += (i != client_data.begin() ? utf8(",") : "") +
							"{"
							"\"hostname\":\"" + escapeJSON(host) + "\","
							"\"useragent\":\"" + escapeJSON((*i)->m_userAgent) + "\","
							"\"connecttime\":" + tos(t - (*i)->m_startTime) + ","
							"\"uid\":" + tos((*i)->m_unique) + ","
							"\"type\":" + tos((*i)->m_clientType) + ","
							"\"referer\":\"" + escapeJSON((*i)->m_referer) + "\","
							"\"xff\":\"" + escapeJSON((*i)->m_XFF) + "\","
							"\"grid\":" + tos((*i)->m_group) + ","
							"\"triggers\":" + tos((*i)->m_triggers) + ""
							"}";
				}
				else
				{
					body += (i != client_data.begin() ? utf8(",") : "") +
							"{"
							"\"hostname\":\"" + escapeJSON(host) + "\""
							"}";
				}
			}
			else
			{
				++ip_counts[host];
			}

			delete (*i);
		}

		if (ipcount)
		{
			for (map<utf8, size_t>::const_iterator i = ip_counts.begin(); i != ip_counts.end(); ++i)
			{
				body += (i != ip_counts.begin() ? utf8(",") : "") +
						"{"
						"\"hostname\":\"" + escapeJSON((*i).first) + "\","
						"\"total\":" + tos((*i).second) + ""
						"}";
			}
		}

		body += "]";
	}

	if ((page == 0) || (page == 4) || (page == 5))
	{
		streamData::streamHistory_t songHistory;
		streamData::getStreamSongHistory(sid, songHistory);

		// only provide this as an option feature so as not to bloat the main xml stats unnecessarily
		if (!(page == 5))
		{
			bool first = true;
			body += (!page ? utf8(",\"songs\":[") : "[");
			for (streamData::streamHistory_t::const_iterator i = songHistory.begin(); i != songHistory.end(); ++i)
			{
				body += (!first ? utf8(",") : "") + "{\"playedat\":" +
						tos((*i).m_when) + "," "\"title\":\"" +
						escapeJSON((*i).m_title) + "\",\"metadata\":" +
						protocol_HTTPStyle::getCurrentJSONMetadataBody((*i).m_metadata) + "}";
				first = false;
			}
			body += "]";
		}
		else
		{
			body += protocol_HTTPStyle::getCurrentJSONMetadataBody((!songHistory.empty() ? songHistory[0].m_metadata : ""));
		}
	}

	if (page == 6)
	{
		const int maxUsers = gOptions.maxUser();
		body += "{"
				"\"requireconfigs\":" + tos(gOptions.requireStreamConfigs()) + ","
				"\"maxlisteners\":" + (maxUsers > 0 ? tos(maxUsers) : "\"unlimited\"") + ","
				"\"minbitrate\":" + tos(gOptions.minBitrate()) + ","
				"\"maxbitrate\":" + tos(gOptions.maxBitrate()) + ",";

		config::streams_t streams;
		gOptions.getStreamConfigs(streams);

		bool read = false;
		utf8 block = "";
		size_t moved = 0;
		for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
		{
			const utf8 movedUrl = gOptions.stream_movedUrl((*i).second.m_streamID);
			if (movedUrl.empty())
			{
				utf8 streamTag = ((*i).second.m_streamID > 1 ? "/stream/" + tos((*i).second.m_streamID) + "/" : "/");
				if (!(*i).second.m_urlPath.empty())
				{
					streamTag = (*i).second.m_urlPath;
				}

				utf8 authhash = (*i).second.m_authHash;
				block += (read ? utf8(",") : "") +
						 "{"
						 "\"id\":" + tos((*i).second.m_streamID) + ","
						 "\"authhash\":\"" + escapeJSON(authhash.empty() ? "empty" : !yp2::isValidAuthhash(authhash) ? "invalid" : authhash) + "\","
						 "\"path\":\"" + escapeJSON(streamTag) + "\","
						 "\"relayurl\":\"" + escapeJSON((*i).second.m_relayUrl.url()) + "\","
						 "\"backupurl\":\"" + escapeJSON((*i).second.m_backupUrl.url()) + "\","
						 "\"maxlisteners\":\"" + escapeJSON(((*i).second.m_maxStreamUser > 0) && ((*i).second.m_maxStreamUser < gOptions.maxUser()) ? tos((*i).second.m_maxStreamUser) : escapeJSON("maxlisteners")) + "\","
						 "\"minbitrate\":\"" + escapeJSON((*i).second.m_minStreamBitrate > 0 ? tos((*i).second.m_minStreamBitrate) : escapeJSON("minbitrate")) + "\","
						 "\"maxbitrate\":\"" + escapeJSON((*i).second.m_maxStreamBitrate > 0 ? tos((*i).second.m_maxStreamBitrate) : escapeJSON("maxbitrate")) + "\","
						 "\"public\":\"" + escapeJSON((*i).second.m_publicServer) + "\","
						 "\"allowrelay\":\"" + tos((*i).second.m_allowRelay) + "\","
						 "\"publicrelay\":\"" + tos((*i).second.m_allowPublicRelay) + "\""
						 "}";
				read = true;
			}
			else
			{
				++moved;
			}
		}
		const size_t total = (streams.size() - moved);
		body +=	"\"total\":\"" + tos(total) + "\"" +
				(total > 0 ? ",\"streams\":[" : "") + block +
				(total > 0 ? utf8("]") : "") + "}";
	}

	body += (page < 2 ? utf8("}") : "") + (jsonp ? ")" : "");

	COMPRESS(header, body);
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_sources(const uniString::utf8& host) throw()
{
	utf8 header = MSG_NO_CLOSE_200,
		 body = getServerAdminHeader("Server Source Connection Details") +
				"<table cellpadding=\"5\" cellspacing=\"0\" border=\"0\" width=\"100%\">"
				"<tr><td class=\"tsp\" align=\"center\">" +
				utf8(gOptions.requireStreamConfigs() ? "Sources are only able to connect on the configured streams shown" :
													   "Sources are able to connect on any \"Stream ID\" (using the appropriate details)") +
				"</td></tr></table>"
				"<div><br>"

				"<div style=\"width:19.5em;margin: 0 0 1em 1em;\" class=\"infb\">"
				"<div align=\"center\" class=\"infh\"><b>Information</b></div>"
				"Here are the login details to enter in your chosen source software so it can then be used to connect to the server.<br><br>"
				"<b>Note:</b> Names of options in the source software may vary from those shown.<br><br>"

				"<b><font color=\"red\">*</font></b> This may not be correct and is a best guess from the current connection.<br><br><hr><br>"
				"The '<b>Legacy Password</b>' is for use with legacy sources (which are sources that are not able to directly specify the "
				"desired stream number).<br><br>This allows for use of any Shoutcast compatible source on any of the currently configured streams.<br><br>"
				"If connecting a legacy source to stream #1, the '<b>Password</b>' value can be used.<br><br><hr><br>"
				"The example JSON-P response uses callback=func which can be changed to a custom value as required for usage.<br><br><hr><br>"
				
				"For further information on connecting sources to the server, as well as ensuring that all of the required ports have been opened, "
				"see <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_Server_Source_Support\"><b>this wiki page</b></a>.<br><br></div>";

	config::streams_t streams;
	gOptions.getStreamConfigs(streams);

	utf8 ids = "";
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		ids += (i != streams.begin() ? ", ": "") + tos((*i).second.m_streamID);

		// skip over moved streams as they cannot be used, though we include the
		// streamid above (whether or not that'll confuse people i do not know).
		const utf8 movedUrl = gOptions.stream_movedUrl((*i).second.m_streamID);
		if (movedUrl.empty())
		{
			int maxbitrate = ((*i).second.m_maxStreamBitrate > 0 ? (*i).second.m_maxStreamBitrate : gOptions.maxBitrate()),
				minbitrate = ((*i).second.m_minStreamBitrate > 0 ? (*i).second.m_minStreamBitrate : gOptions.minBitrate());

			// test for being a relay which is active - if so then we need to not show
			// direct source login details as they will not be able to connect anyway
			bool noEntry = false, isRelay = (*i).second.m_relayUrl.isSet();
			if (isRelay && streamData::isRelayActive((*i).second.m_streamID, noEntry))
			{
				body += "<div class=\"infb\" style=\"width:19.5em;word-wrap:break-word;margin: 0 0 1em 1em;\">"
						"<div align=\"center\" class=\"infh\">"
						"<b>Stream #" + tos((*i).second.m_streamID) + "</b></div>"
						"This stream has been configured for use as a relay which is currently active.<br><br>"
						"Any source connections attempted will be blocked whilst the relay is active.<br><br>"
						"Password: <b>" + aolxml::escapeXML((*i).second.m_password) + "</b><br>"
						"Source: <b>" + urlLink(niceURL((*i).second.m_relayUrl.url())) + "</b><br>" +
						(!(*i).second.m_backupUrl.url().empty() ? "Backup Source: <b>" +
						urlLink(niceURL((*i).second.m_backupUrl.url())) + "</b><br>" : "");
			}
			// otherwise show direct source login details for specific as well as generic stream
			// logins (even if configured to be a relay as long as it is inactive at the time)
			else
			{
				body +=	"<div class=\"infb\" style=\"width:19.5em;word-wrap:break-word;margin: 0 0 1em 1em;\">"
						"<div align=\"center\" class=\"infh\"><b>Stream #" + tos((*i).second.m_streamID) + "</b></div>" +
						(isRelay ? "<b>Note:</b> Configured as a relay to use:<br><b>" +
								   niceURL((*i).second.m_relayUrl.url()) + "</b><br><br>" : "") +
						"Stream ID: <b>" + tos((*i).second.m_streamID) + "</b><br>"
						"Server Address: <b>" + (!host.empty() ? host : "<enter_server_address>") + "</b> <b>(<font color=\"red\">*</font>)</b><br>"
						"Source Port: <b>" + tos(gOptions.portBase()) + "</b><br><br>" +
						"Password: <b>" + aolxml::escapeXML((*i).second.m_password) + "</b><br>" +
						((*i).second.m_streamID > 1 && (g_legacyPort > 0) ? "Legacy Password: <b>" +
						aolxml::escapeXML((*i).second.m_password) + ":#" + tos((*i).second.m_streamID) + "</b><br><br>" : "<br>") +
						"Min Bitrate Allowed: <b>" + (!minbitrate ? "Unlimited" : tos(minbitrate / 1000) + " kbps") + "</b><br>"
						"Max Bitrate Allowed: <b>" + (!maxbitrate ? "Unlimited" : tos(maxbitrate / 1000) + " kbps") + "</b><br><br>"
						"Protocol Mode: <b>" + ((g_legacyPort > 0) ? "v2, v1" : "v2") + "</b><br>";
			}

			const utf8 address = "http://" + ((!host.empty() ? host : "<enter_server_address>") + ":" + tos(gOptions.portBase())),
					   params = "?sid=" + tos((*i).second.m_streamID) + "&amp;pass=" + aolxml::escapeXML((*i).second.m_password);
			body +=	"<br><hr><br>Listener Playlist(s):<br><b>"
					"<a href=\"" + address + "/listen.pls?sid=" + tos((*i).second.m_streamID) + "\">PLS</a>,&nbsp;&nbsp;"
					"<a href=\"" + address + "/listen.m3u?sid=" + tos((*i).second.m_streamID) + "\">M3U</a>,&nbsp;&nbsp;"
					"<a href=\"" + address + "/listen.asx?sid=" + tos((*i).second.m_streamID) + "\">ASX</a>,&nbsp;&nbsp;"
					"<a href=\"" + address + "/listen.xspf?sid=" + tos((*i).second.m_streamID) + "\">XSPF</a>,&nbsp;&nbsp;"
					"<a href=\"" + address + "/listen.qtl?sid=" + tos((*i).second.m_streamID) + "\">QTL</a></b>"

					"<br><br>Direct Stream URL(s):<br><b>"
					"<a title=\"Flash\" alt=\"Flash\" "
					"href=\"" + address + getStreamPath((*i).second.m_streamID) + "?type=.flv\">"
					"<img border=\"0\" style=\"vertical-align:bottom\" src=\"images/flash.png\"> FLV</a>,&nbsp;&nbsp;"
					"<a title=\"HTTP / HTML5\" alt=\"HTTP / HTML5\" "
					"href=\"" + address + getStreamPath((*i).second.m_streamID) + "?type=http\">"
					"<img border=\"0\" style=\"vertical-align:bottom\" src=\"images/html5.png\"> HTTP</a>,&nbsp;&nbsp;"
					"<a title=\"Shoutcast 1.x\" alt=\"Shoutcast 1.x\" "
					"href=\"" + address + getStreamPath((*i).second.m_streamID) + "?type=sc1\">"
					"<img border=\"0\" style=\"vertical-align:bottom\" src=\"images/favicon.ico\"> 1.x</a>,&nbsp;&nbsp;"
					"<a title=\"Shoutcast 2.x\" alt=\"Shoutcast 2.x\" "
					"href=\"" + address + getStreamPath((*i).second.m_streamID) + "?type=sc2\">"
					"<img border=\"0\" style=\"vertical-align:bottom\" src=\"images/favicon.ico\"> 2.x</a></b>"

					"<br><br>History:<br><b>"
					"<a href=\"" + address + "/played" + params + "&amp;type=xml\">XML</a>,&nbsp;&nbsp;"
					"<a href=\"" + address + "/played" + params + "&amp;type=json\">JSON</a>,&nbsp;&nbsp;"
					"<a href=\"" + address + "/played" + params + "&amp;type=json&amp;callback=func\">JSON-P</a></b>"

					"<br><br>Stream Statistics:<br><b>"
					"<a href=\"" + address + "/stats" + params + "\">XML</a>,&nbsp;&nbsp;"
					"<a href=\"" + address + "/stats" + params + "&amp;json=1\">JSON</a>,&nbsp;&nbsp;"
					"<a href=\"" + address + "/stats" + params + "&amp;json=1&amp;callback=func\">JSON-P</a></b><br></div>";
		}
	}

	if (!gOptions.requireStreamConfigs())
	{
		const int minbitrate = gOptions.minBitrate(), maxbitrate = gOptions.maxBitrate();
		body +=	"<div class=\"infb\" style=\"width:19.5em;word-wrap:break-word;margin: 0 0 1em 1em;\">"
				"<div align=\"center\" class=\"infh\"><b>All" + (!ids.empty() ? " Other" : (utf8)"") + " Streams</b></div>"
				"Stream ID: <b>&lt;any value" + (!ids.empty() ? " other than " + ids : "") + "&gt;</b><br>"
				"Server Address: <b>" + aolxml::escapeXML(!host.empty() ? host : "<enter_server_address>") + "</b> <b>(<font color=\"red\">*</font>)</b><br>"
				"Server Port: <b>" + tos(gOptions.portBase()) + "</b><br><br>" +
				"Password: <b>" + aolxml::escapeXML(gOptions.password()) + "</b><br>" +
				((g_legacyPort > 0) ? "Legacy Password: <b>" + aolxml::escapeXML(gOptions.password()) + ":#xx</b><br><br>" : "<br>") +
				"Min Bitrate Allowed: <b>" + (!minbitrate ? "Unlimited" : tos(minbitrate / 1000) + " kbps") + "</b><br>"
				"Max Bitrate Allowed: <b>" + (!maxbitrate ? "Unlimited" : tos(maxbitrate / 1000) + " kbps") + "</b><br><br>"
				"Protocol Mode: <b>" + ((g_legacyPort > 0) ? "v2, v1" : "v2") + "</b><br>";

		const utf8 address = "http://" + ((!host.empty() ? host : "<enter_server_address>") + ":" + tos(gOptions.portBase())),
				   params = "?sid=xx&amp;pass=" + aolxml::escapeXML(gOptions.password());
		body +=	"<br><hr><br>Listener Playlist(s):<br><b>"
				"<a href=\"" + address + "/listen.pls?sid=xx\">PLS</a>,&nbsp;&nbsp;"
				"<a href=\"" + address + "/listen.m3u?sid=xx\">M3U</a>,&nbsp;&nbsp;"
				"<a href=\"" + address + "/listen.asx?sid=xx\">ASX</a>,&nbsp;&nbsp;"
				"<a href=\"" + address + "/listen.xspf?sid=xx\">XSPF</a>,&nbsp;&nbsp;"
				"<a href=\"" + address + "/listen.qtl?sid=xx\">QTL</a></b>"

				"<br><br>History:<br><b>"
				"<a href=\"" + address + "/played" + params + "&amp;type=xml\">XML</a>,&nbsp;&nbsp;"
				"<a href=\"" + address + "/played" + params + "&amp;type=json\">JSON</a>,&nbsp;&nbsp;"
				"<a href=\"" + address + "/played" + params + "&amp;type=json&amp;callback=func\">JSON-P</a></b>"

				"<br><br>Stream Statistics:<br><b>"
				"<a href=\"" + address + "/stats" + params + "\">XML</a>,&nbsp;&nbsp;"
				"<a href=\"" + address + "/stats" + params + "&amp;json=1\">JSON</a>,&nbsp;&nbsp;"
				"<a href=\"" + address + "/stats" + params + "&amp;json=1&amp;callback=func\">JSON-P</a></b>"
				"<br><br><b>Note:</b> Replace the '<b>xx</b>' in '<b>sid=xx</b>' in all of the example links "
				"and in the '<b>Legacy Password</b>' with the stream number.<br></div>";
	}

	body +=	"</div>" + getUptimeScript() + getIEFlexFix() + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_adgroups() throw()
{
	utf8 header = MSG_NO_CLOSE_200,
		 body = getServerAdminHeader("Server Advert Group Details") +
				"<div style=\"padding-right:1em;\"><br>"
				"<table align=\"center\" width=\"100%\"><tr valign=\"top\">"
				"<td><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">"
				"<tr><td valign=\"top\">"
				"<div style=\"width:13em;\" class=\"infb\">"
				"<div align=\"center\" class=\"infh\"><b>Information</b></div>"
				"Here you can see which active stream(s) have adverts enabled and their status of obtaining the advert details."
				"<br><br>This can help to see if a listener should be receiving adverts or not by "
				"checking for the listener's group id with the group ids shown for the required stream.<br><br></div>"

				"</td><td valign=\"top\" width=\"100%\">"
				"<table class=\"en\" style=\"border:0;text-align:center;word-wrap:break-word;\" "
				"cellpadding=\"3\" cellspacing=\"0\" width=\"100%\" align=\"center\">";

	const streamData::streamIDs_t streamIds = streamData::getStreamIds();
	if (!streamIds.empty())
	{
		body += "<colgroup><col width=\"15%\"></colgroup>"
				"<tr class=\"tll\"><td>Stream #</td><td>Advert Group(s)</td></tr>";

		for (streamData::streamIDs_t::const_iterator i = streamIds.begin(); i != streamIds.end(); ++i)
		{
			streamData *sd = streamData::accessStream((*i));
			if (sd)
			{
				if (sd->isSourceConnected((*i)))
				{
					body += "<tr><td><b>" + tos((*i)) + "</b></td>"
							"<td style=\"max-width:0;text-align:left;\">" +
							sd->getAdvertGroup() + "</td></tr>";
				}
				sd->releaseStream();
			}
		}
	}
	else
	{
		body += "<tr><td style=\"border:0\">There are no active streams.</b></td></tr>";
	}
	body += "</table></td></tr></table></td></tr></table></div>" +
			getUptimeScript() + getIEFlexFix() + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_debug(const uniString::utf8& option, const int on_off, const bool adminRefer) throw()
{
	struct debug_options {
		utf8 option;
		utf8 desc;
		utf8 msg;
		bool value;
	};
	debug_options debug[] = {
		{"Listener Connections", "", ""},
		{"shoutcast1clientdebug", "Shoutcast 1.x Listener Connections", "Used to diagnose connection issues with listener connections using the legacy Shoutcast 1.x protocol"
																		"<br>(e.g. HTTP connections indicating they support icy-metadata).", gOptions.shoutcast1ClientDebug()},
		{"shoutcast2clientdebug", "Shoutcast 2.x Listener Connections", "Used to diagnose connection issues with listener connections using the Shoutcast 2.x protocol.", gOptions.shoutcast2ClientDebug()},
		{"httpclientdebug", "HTTP Listener Connections", "Used to diagnose connection issues with listener connections using the HTTP protocol.", gOptions.HTTPClientDebug()},
		{"flvclientdebug", "FLV Listener Connections", "Used to diagnose connection issues with listener connections using the FLV encapsulation.", gOptions.flvClientDebug()},
#if 0
		{"m4aclientdebug", "M4A Listener Connections", "Used to diagnose connection issues with listener connections using the M4A encapsulation.", gOptions.m4aClientDebug()},
#endif
		{"Direct Source Connections", "", ""},
		{"shoutcastsourcedebug", "Shoutcast 1.x Source Connections", "Used to diagnose connection issues with source connections using the legacy Shoutcast 1.x protocol.", gOptions.shoutcastSourceDebug()},
		{"uvox2sourcedebug", "Shoutcast 2.x Source Connections", "Used to diagnose connection issues with source connections using the Shoutcast 2.x protocol.", gOptions.uvox2SourceDebug()},
		{"httpsourcedebug", "HTTP Source Connections", "Used to diagnose connection issues with source connections using HTTP PUT protocol (e.g. Icecast sources).", gOptions.HTTPSourceDebug()},
		{"Relay Source Connections", "", ""},
		{"relayshoutcastdebug", "Shoutcast 1.x Relay Connections", "Used to diagnose connection issues with relay connections using the legacy Shoutcast 1.x protocol.", gOptions.relayShoutcastDebug()},
		{"relayuvoxdebug", "Shoutcast 2.x Relay Connections", "Used to diagnose connection issues with relay connections using the Shoutcast 2.x protocol.", gOptions.relayUvoxDebug()},
		{"relaydebug", "Common Relay Handling", "Used to diagnose issues with general relay handling when a relay connection begins (non-protcool specific).", gOptions.relayDebug()},
		{"HTTP Connections", "", ""},
		{"httpstyledebug", "HTTP Requests", "Used to inspect HTTP requests made to the server (e.g. listener or statistic requests).", gOptions.httpStyleDebug()},
		{"webclientdebug", "HTTP Connections", "Used to inspect and diagnose issues with HTTP connections issued by the server.", gOptions.webClientDebug()},
		{"Shoutcast Services", "", ""},
		{"admetricsdebug", "Advert & Metrics Handling", "Used to inspect and diagnose issues with the advert and metric activity for client connections.", gOptions.adMetricsDebug()},
		{"yp2debug", "YP / Directory Connections", "Used to diagnose failures to connect to the Directory servers or to be listed.", gOptions.yp2Debug()},
#if defined(_DEBUG) || defined(DEBUG)
		// this is not enabled as we don't really want to promote the existence of this
		{"authdebug", "Listener Auth Handling", "Used to diagnose issues with the client auth handling when clients connect to the stream.", gOptions.authDebug()},
#endif
		{"Miscellaneous", "", ""},
		{"streamdatadebug", "Common Stream Handling", "Used to diagnose issues with general streaming code (non-protcool specific).", gOptions.streamDataDebug()},
		{"statsdebug", "Client Statistics", "Used to inspect client statistics tracked by the server.", gOptions.statsDebug()},
		{"microserverdebug", "Common Server Activity", "Used to diagnose and track common server activity.", gOptions.microServerDebug()},
		{"threadrunnerdebug", "Thread Manager", "Used to diagnose and track the processing of threads by the thread manager.", gOptions.threadRunnerDebug()},
	};

	if (option.empty())
	{
		utf8 header = "HTTP/1.1 200 OK\r\n"
					  "Cache-Control:no-cache\r\n"
					  "Content-Type:text/html;charset=utf-8\r\n",
			 body = getServerAdminHeader("Server Debugging Options") +
					"<div><br>"
					"<table align=\"center\" width=\"100%\"><tr valign=\"top\">"
					"<td><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">"
					"<tr valign=\"top\"><td>"
					"<div style=\"max-width:15em;\" class=\"infb\">"
					"<div align=\"center\" class=\"infh\"><b>Information</b></div>"
					"Here you can find are all debugging options provided by the "
					"DNAS server.<br><br>Changing the debugging options will update "
					"the value saved in the DNAS server configuration file(s) as "
					"needed.<br><br>Any changes are applied immediately.<br><br><hr><br>"

					"<div align=\"left\"><b>All Options</b><br><br></div>"
					"<input class=\"submit\" type=\"button\" onclick=\"toggle('all','true',1);\" name=\"all\" id=\"all\" value=\"Enable\">"
					"&nbsp;&nbsp;&nbsp;<input class=\"submit\" type=\"button\" onclick=\"toggle('all','false',1);\" name=\"all\" id=\"all\" value=\"Disable\">"
					"<br><br><hr><br>"

					"<div align=\"left\"><b>Listener Options</b><br><br></div>"
					"<input class=\"submit\" type=\"button\" onclick=\"toggle('listener','true',2);\" name=\"listener\" id=\"listener\" value=\"Enable\">"
					"&nbsp;&nbsp;&nbsp;<input class=\"submit\" type=\"button\" onclick=\"toggle('listener','false',2);\" name=\"listener\" id=\"listener\" value=\"Disable\">"
					"<br><br><hr><br>"

					"<div align=\"left\"><b>Source Options</b><br><br></div>"
					"<input class=\"submit\" type=\"button\" onclick=\"toggle('source','true',3);\" name=\"source\" id=\"sources\" value=\"Enable\">"
					"&nbsp;&nbsp;&nbsp;<input class=\"submit\" type=\"button\" onclick=\"toggle('source','false',3);\" name=\"source\" id=\"source\" value=\"Disable\">"
					"<br><br><hr><br>"

					"<div align=\"left\"><b>Force Short Sends</b> <div id=\"forceshortsendsval\" "
					"style=\"display:inline-block\">[" + (gOptions.forceShortSends() ? "Enabled" :
					"Disabled") + "]</div><br><br>This inserts delays into sending stream(s)"
					"to replicate network limiting and bandwidth issues (Default: <b>Disabled</b>).<br><br></div>"
					"<input class=\"submit\" type=\"button\" onclick=\"toggle('forceshortsends','true',4);\" name=\"forceshortsends\" id=\"forceshortsends\" value=\"Enable\">"
					"&nbsp;&nbsp;&nbsp;<input class=\"submit\" type=\"button\" onclick=\"toggle('forceshortsends','false',4);\" name=\"forceshortsends\" id=\"forceshortsends\" value=\"Disable\">"
					"<br><br><hr><br>"

					"<div align=\"left\"><b>Rate Limiting</b> <div "
					"id=\"ratelimitval\" style=\"display:inline-block\">"
					"[" + (gOptions.rateLimit() ? "Enabled" : "Disabled") +
					"]</div><br><br>Controls the rate of output to even "
					"it out and prefer sending larger blocks in one go instead of "
					"doing smaller blocks more often (Default: <b>Enabled</b>).<br><br></div>"
					"<input class=\"submit\" type=\"button\" onclick=\"toggle('ratelimit','true',5);\" name=\"ratelimit\" id=\"ratelimit\" value=\"Enable\">"
					"&nbsp;&nbsp;&nbsp;<input class=\"submit\" type=\"button\" onclick=\"toggle('ratelimit','false',5);\" name=\"ratelimit\" id=\"ratelimit\" value=\"Disable\">"

					"</div><td>";

		for (size_t i = 0; i < sizeof(debug) / sizeof(debug[0]); i++)
		{
			if (!debug[i].desc.empty())
			{
				body += "<input autocomplete=\"off\" type=\"checkbox\" onclick=\"toggle('" +
						debug[i].option + "',$('" + debug[i].option +
						"').checked,0)\" name=\"" + debug[i].option +
						"\" id=\"" + debug[i].option + "\"" +
						(debug[i].value ? " checked=\"true\"" : " ") + " value=\"" +
						(debug[i].value ? "1" : "0") + "\">" "<label for=\"" +
						debug[i].option + "\" title=\"" + debug[i].option +
						"\" style=\"padding-left: 7px;\"><b>" + debug[i].desc + "</b></label>"
						"<div style=\"padding:5px 0 0 27px;\">" + debug[i].msg + "</div><br>";
			}
			else
			{
				body += (i ? "<br>" : (utf8)"") + "<b><u>" + debug[i].option + "</u>:</b><br><br>";
			}
		}

		body += "</td></tr></table></td></tr></table></div>"
				+ getUptimeScript() +
				"<script type=\"text/javascript\">" EL
				"function toggle(opt, on, mode) {" EL
				"if(window.XMLHttpRequest){" EL
				"xmlhttp=new XMLHttpRequest();" EL
				"}else{" EL
				"xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");" EL
				"}" EL
				"xmlhttp.open(\"GET\",\"admin.cgi?pass="+gOptions.adminPassword()+"&mode=debug&option=\"+opt+\"&on=\"+on,true);" EL
				"xmlhttp.send(null);" EL
				"if(mode == 1){" EL
				"var labels = document.getElementsByTagName('LAB EL');" EL
				"for (var i = 0; i < labels.length; i++) {" EL
				"if (labels[i].htmlFor != '') {" EL
				"$(labels[i].htmlFor).checked=(on=='true'?'true':'');"
				"}" EL
				"}" EL
				"}" EL
				"else if(mode == 2){" EL
				"var labels = ['shoutcast1clientdebug', 'shoutcast2clientdebug', 'httpclientdebug', 'flvclientdebug'/*, 'm4aclientdebug'*/];" EL
				"for (var i = 0; i < labels.length; i++) {" EL
				"$(labels[i]).checked=(on=='true'?'true':'');"
				"}" EL
				"}" EL
				"else if(mode == 3){" EL
				"var labels = ['shoutcastsourcedebug', 'uvox2sourcedebug', 'httpsourcedebug', 'relayshoutcastdebug', 'relayuvoxdebug', 'relaydebug'];" EL
				"for (var i = 0; i < labels.length; i++) {" EL
				"$(labels[i]).checked=(on=='true'?'true':'');"
				"}" EL
				"}" EL
				"else if(mode == 4){" EL
				"$('forceshortsendsval').innerHTML=(on=='true'?'[Enabled]':'[Disabled]');" EL
				"}" EL
				"else if(mode == 5){" EL
				"$('ratelimitval').innerHTML=(on=='true'?'[Enabled]':'[Disabled]');" EL
				"}" EL
				"}</script>" + getIEFlexFix() + getfooterStr();

		COMPRESS(header, body);
		header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
		sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
	}
	else
	{
		if (option == "all")
		{
			ILOG(LOGNAME + utf8(on_off ? "Enabling" : "Disabling") + " all debugging options");
			for (size_t i = 0; i < sizeof(debug) / sizeof(debug[0]); i++)
			{
				if (!debug[i].desc.empty())
				{
					// always set even if the saving fails so we've got something working
					try
					{
						gOptions.setOption(debug[i].option, utf8(tos(on_off)));
					}
					catch(const exception &)
					{
					}

					bool handled = false, idHandled = false;
					// if we get a clear then just remove from the config and reload the page
					if (gOptions.editConfigFileEntry(0, gOptions.confFile(), debug[i].option, tos(on_off),
													 true, handled, idHandled, true) == false)
					{
						ELOG(LOGNAME "Error saving debug option: `" + debug[i].option + "'");
					}
				}
			}
		}
		else if (option == "listener")
		{
			ILOG(LOGNAME + utf8(on_off ? "Enabling" : "Disabling") + " all listener debugging options");
			const utf8 options[] = {"shoutcast1clientdebug", "shoutcast2clientdebug", "httpclientdebug", "flvclientdebug"/*, "m4aclientdebug"*/};
			for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); i++)
			{
				// always set even if the saving fails so we've got something working
				try
				{
					gOptions.setOption(options[i], utf8(tos(on_off)));
				}
				catch(const exception &)
				{
				}

				bool handled = false, idHandled = false;
				// if we get a clear then just remove from the config and reload the page
				if (gOptions.editConfigFileEntry(0, gOptions.confFile(), options[i], tos(on_off),
												 true, handled, idHandled, true) == false)
				{
					ELOG(LOGNAME "Error saving debug option: `" + options[i] + "'");
				}
			}
		}
		else if (option == "source")
		{
			ILOG(LOGNAME + utf8(on_off ? "Enabling" : "Disabling") + " all source debugging options");
			const utf8 options[] = {"shoutcastsourcedebug", "uvox2sourcedebug", "httpsourcedebug",
									"relayshoutcastdebug", "relayuvoxdebug", "relaydebug"};
			for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); i++)
			{
				// always set even if the saving fails so we've got something working
				try
				{
					gOptions.setOption(options[i], utf8(tos(on_off)));
				}
				catch(const exception &)
				{
				}

				bool handled = false, idHandled = false;
				// if we get a clear then just remove from the config and reload the page
				if (gOptions.editConfigFileEntry(0, gOptions.confFile(), options[i], tos(on_off),
												 true, handled, idHandled, true) == false)
				{
					ELOG(LOGNAME "Error saving debug option: `" + options[i] + "'");
				}
			}
		}
		else
		{
			ILOG(LOGNAME + utf8(on_off ? "Enabling" : "Disabling") + " the `" + option + "' debugging options");

			// always set even if the saving fails so we've got something working
			gOptions.setOption(option, utf8(tos(on_off)));

			bool handled = false, idHandled = false;
			// if we get a clear then just remove from the config and reload the page
			if (gOptions.editConfigFileEntry(0, gOptions.confFile(), option, tos(on_off),
											 true, handled, idHandled, true) == false)
			{
				ELOG(LOGNAME "Error saving debug option: `" + option + "'");
			}
		}

		if (adminRefer)
		{
			sendMessageAndClose(redirect("admin.cgi", SHRINK));
		}
		else
		{
			sendMessageAndClose(MSG_200);
		}
	}
}

void protocol_admincgi::mode_help() throw()
{
	utf8 libs = "<u>Supporting Libraries</u>:<br><br>";
	std::vector<uniString::utf8> parts = tokenizer(utf8(curl_version()), ' ');
	for (vector<uniString::utf8>::const_iterator i = parts.begin(); i != parts.end(); ++i)
	{
		utf8::size_type pos = (*i).find('/');
		if (pos != utf8::npos)
		{
			libs += (*i).substr(0, pos) + utf8(": <b>") + (*i).substr(pos + 1) + utf8("</b>");
		}
		else
		{
			libs += "<b>" + (*i) + "</b>";
		}


		if ((*i).find((utf8)"libcurl") != utf8::npos)
		{
			libs += " (<a target=\"_blank\" href=\"http://curl.haxx.se/\"><b>site</b></a>)";
		}
		else if ((*i).find((utf8)"OpenSSL") != utf8::npos)
		{
			libs += " (<a target=\"_blank\" href=\"http://www.openssl.org/\"><b>site</b></a>)";
		}
		else if ((*i).find((utf8)"zlib") != utf8::npos)
		{
			libs += " (<a target=\"_blank\" href=\"http://www.zlib.net/\"><b>site</b></a>)";
		}
		libs += "<br>";
	}

	const int cpu_count = gOptions.getCPUCount();
	const utf8 cpu = "CPU Count: <b>" + tos(cpucount()) + "</b> -> " +
					 (cpu_count == cpucount() ? "using " + utf8(cpu_count > 1 ? "all" : "the") +
					 " available CPU" + (cpu_count > 1 ? "s" : "") : tos(cpu_count) +
					 " specified to be used") + "<br><br>";

	XML_Expat_Version expat = XML_ExpatVersionInfo();
	utf8 header = MSG_NO_CLOSE_200,
		 body = getServerAdminHeader("Server Help &amp; Documentation") +

				"<div><br>"

				"<div style=\"width:19em;margin: 0 0 1em 1em;\" class=\"infb\">"
				"<div align=\"center\" class=\"infh\"><b>Help</b></div>"
				"If you are having issues, you should first try to contact your "
				"hosting provider.<br><br>If running the DNAS server yourself (or if instructed "
				"to do so), then you can use our <a target=\"_blank\" "
				"href=\"http://forums.shoutcast.com/forumdisplay.php?f=140\"><b>forum</b></a> "
				"or you can contact <a href=\"mailto:support@shoutcast.com?subject=Shoutcast%20Support\">"
				"<b>Shoutcast support</b></a> directly (e.g. if the issue relates to an "
				"account / authhash issue).<br><br><b>Note:</b> If using the <a target=\"_blank\" "
				"href=\"http://forums.shoutcast.com/forumdisplay.php?f=140\"><b>forum</b></a>, "
				"do not post any information which could be used to compromise "
				"your account / authhash / etc (e.g. passwords and authhash(s)).<br><br></div>"

				"<div style=\"width:19em;margin: 0 0 1em 1em;\" class=\"infb\">"
				"<div align=\"center\" class=\"infh\"><b>Documentation</b></div>"
				"Supporting documentation for using the DNAS server from setup "
				"to getting statistics from the server are <a target=\"_blank\" "
				"href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_Broadcaster\"><b>online</b></a>.<br><br>"
				"Otherwise a local copy can usually be found on the host machine at:<br><br>"
				"<div style=\"word-break:break-all;\"><b>" + aolxml::escapeXML(gStartupDirectory) +
				"</b></div><br><b>Note:</b>  The local copy is usually correct for the version being "
				"used and the <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_Broadcaster\">"
				"<b>online</b></a> version will be for the most recent release.<br><br></div>"

				"<div style=\"width:19em;margin: 0 0 1em 1em;\" class=\"infb\">"
				"<div align=\"center\" class=\"infh\"><b>About</b></div>"
				"Information about the DNAS server and the supporting libraries currently used.<br><br>"
				"Version: <b>" + addWBR(gOptions.getVersionBuildStrings()) + "</b><br>"
				"Platform: <b>" + SERV_OSNAME "</b><br>"
				"Built: <b>" __DATE__"</b><br><br>"
				+ libs +	// libcurl, openssl, zlib
				"expat: <b>" + tos(expat.major) + "." + tos(expat.minor) + "." + tos(expat.micro) + "</b>"
				" (<a target=\"_blank\" href=\"http://www.libexpat.org/\"><b>site</b></a>)"
				//#ifdef _WIN32
				//"<br>pthread-win32: <b>" PTW32_VERSION_STR "-mod</b>"
				//" (<a target=\"_blank\" href=\"https://github.com/GerHobbelt/pthread-win32\"><b>site</b></a>)"
				//#endif
				"<br><br></div></div>"
				
				"<div style=\"width:19em;margin: 0 0 1em 1em;\" class=\"infb\"><div "
				"align=\"center\" class=\"infh\"><b>Diagnostics</b></div>" + cpu +
				"<u>Current thread &amp; runner usage</u>:<br><br>" +
				threadedRunner::getRunnabledetails() + "<br></div></div>"
				
				+ getUptimeScript() + getIEFlexFix() + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_admincgi::mode_config() throw()
{
	utf8 conf = gOptions.confFile();
	utf8::size_type pos = conf.rfind(fileUtil::getFilePathDelimiter());
	if ((pos != utf8::npos))
	{
		conf = conf.substr(pos + 1);
	}

	utf8 header = MSG_NO_CLOSE_200,
		 body = getServerAdminHeader("Server Configuration Settings") +
				"<div><div style=\"padding:1em;\"><b>This page shows the custom configuration settings "
				"that this DNAS server is currently using. (Settings which match DNAS server defaults "
				"may not be shown.)<br><br><u>Note #1:</u> To change these values, you will need to edit the "
				"<u title=\"" + aolxml::escapeXML(fileUtil::getFullFilePath(gOptions.confFile())) + "\">" +
				(!conf.empty() ? conf : "sc_serv.conf") + "</u> file on your server. See <a target=\"_blank\" "
				"href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2#Configuration_File\">here</a> "
				"for more information.<br><br><u>Note #2:</u> This is not the same as the actual configuration file "
				"(i.e. the structure of the configuration file(s) being used is not shown)</b></div>" +
				gOptions.dumpConfigFile() + "<br></div>" + getUptimeScript() + getIEFlexFix() + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

#if 0
void protocol_admincgi::mode_logs() throw()
{
	utf8 header = "HTTP/1.1 200 OK\r\n"
				  "Cache-Control:no-cache\r\n"
				  "Content-Type:text/html;charset=utf-8\r\n",
		 headerTitle = "Server Log Management", childPage = "";

	bool bwstyle = false;
	int refreshRequired = 0;
	utf8 body = s_serverAdminHeading;

	body += "<div style=\"padding:0 1em;\"><br>"
			"<table align=\"center\" width=\"100%\"><tr valign=\"top\">"
			"<td><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"padding-left:1em;\">"
			"<tr valign=\"top\"><td>"
			"<div style=\"width:215px;\" class=\"infb\">"
			"<div align=\"center\" class=\"infh\"><b>Information</b></div>"
			"Here you can find are all debugging options provided by the DNAS server.<br><br>Changing the debugging "
			"options will update the value saved in the DNAS server configuration file(s) as needed.<br><br>"
			"Any changes are applied immediately.<br><br></div>"

			"<td>";

	utf8 logfile = gOptions.realLogFile();

	body += "<div style=\"padding-bottom:5px;\">Current log file:</div>";
	body += "<div class=\"en\" style=\"padding-left:27px;padding:10px;margin-bottom:1em;"
			"display:inline-block;border-radius:5px;\"><b>" +
			aolxml::escapeXML(fileUtil::stripPath(logfile)) + "</b>"
			"&nbsp;&nbsp;&nbsp;<a href=\"admin.cgi?mode=viewlog&amp;server=1\">View</a>"
			"&nbsp;&nbsp;&nbsp;<a href=\"admin.cgi?mode=viewlog&amp;server=1&amp;viewlog=save\">Save</a>"
			"<br>Size: <b>" + formatSizeString(uniFile::fileSize(logfile)) + "</b></div><br>";

	utf8 rotated = fileUtil::stripSuffix(logfile) + "_*." + fileUtil::getSuffix(logfile);
	body += "<div style=\"padding-bottom:5px;\">Older log file(s):</div>";
	body += "<div>";
	#ifdef _WIN32
	vector<wstring> fileList = fileUtil::directoryFileList(utf8(fileUtil::getFullFilePath(rotated)).toWString(), L"", true, true);
	#else
	vector<string> fileList = fileUtil::directoryFileList(fileUtil::getFullFilePath(rotated), "");
	#endif
	if (!fileList.empty())
	{
		#ifdef _WIN32
		for (vector<wstring>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
		#else
		for (vector<string>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
		#endif
		{
			#ifdef _WIN32
			utf32 u32file(*i);
			utf8 u8f(u32file.toUtf8());
			body += "<div class=\"en\" style=\"padding-left:27px;padding:10px;margin-bottom:1em;"
					"display:inline-block;border-radius:5px;\"><b>" + fileUtil::stripPath(u8f) + "</b>"
					"<div style=\"float:right;\">&nbsp;&nbsp;&nbsp;<a href=\"admin.cgi?mode=viewlog&amp;server=1&amp;file=" +
					urlUtils::escapeURI_RFC3986(u8f) + "\">View</a>&nbsp;&nbsp;&nbsp;"
					"<a href=\"admin.cgi?mode=viewlog&amp;server=1&amp;viewlog=save&amp;file=" +
					urlUtils::escapeURI_RFC3986(u8f) + "\">Save</a></div><br>Size: <b>" +
					formatSizeString(uniFile::fileSize(u8f)) + "</b>"
					"<br>Last Modified: <b>" + getRFCDate(uniFile::fileTime(u8f)) + "</b></div><br>";
			#else
			body += "<div style=\"padding-left:27px;\"><b>" + fileUtil::stripPath(*i) + "</b>&nbsp;&nbsp;&nbsp;"
			"<a href=\"admin.cgi?mode=viewlog&amp;server=1&amp;viewlog=save&amp;file="+urlUtils::escapeURI_RFC3986(*i)+"\">Save</a></div>";
			#endif
		}
	}
	body += "</div>";
#if 0
	utf8 archived = fileUtil::stripSuffix(logfile) + "_*.gz";
	body += "<div style=\"padding:5px 0 0 27px;\"><b>Log file (archived):</b> "/* + aolxml::escapeXML(fileUtil::getFullFilePath(archived)) + */"</div><br>";
	body += "<div>";
	#ifdef _WIN32
	fileList = fileUtil::directoryFileList(utf8(fileUtil::getFullFilePath(archived)).toWString(), L"", true, true);
	#else
	fileList = fileUtil::directoryFileList(fileUtil::getFullFilePath(archived), "");
	#endif
	if (!fileList.empty())
	{
		#ifdef _WIN32
		for (vector<wstring>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
		#else
		for (vector<string>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
		#endif
		{
			#ifdef _WIN32
			utf32 u32file(*i);
			utf8 u8f(u32file.toUtf8());
			body += "<div style=\"padding-left:27px;\">" + fileUtil::stripPath(u8f) + "</div>";
			#else
			body += "<div style=\"padding-left:27px;\">" + fileUtil::stripPath(*i) + "</div>";
			#endif
		}
	}
	body += "</div>";

	body += "<hr>";

	utf8 w3cfile = gOptions.w3cLog();
	body += "<div style=\"padding:5px 0 0 27px;\">W3C file: " + aolxml::escapeXML(fileUtil::getFullFilePath(w3cfile)) + "</div><br>";
	utf8 archived = fileUtil::stripSuffix(w3cfile) + "_*.gz";
	body += "<div style=\"padding:5px 0 0 27px;\">W3C file (archived): " + aolxml::escapeXML(fileUtil::getFullFilePath(archived)) + "</div><br>";
	archived = fileUtil::stripSuffix(w3cfile) + "_*_w3c.gz";
	body += "<div style=\"padding:5px 0 0 27px;\">W3C file (archived 2): " + aolxml::escapeXML(fileUtil::getFullFilePath(archived)) + "</div><br>";
	body += "<div>";
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		// w3c logging
		if (gOptions.read_stream_w3cLog((*i).first))
		{
			body += "<div style=\"padding:5px 0 0 27px;\">Stream W3C file: " + aolxml::escapeXML(fileUtil::getFullFilePath(gOptions.stream_w3cLog((*i).first))) + "</div><br>";
		}
	}
	body += "</div>";
	body += "<hr>";
	body += "<div>";
	#ifdef _WIN32
	fileList = fileUtil::directoryFileList(gStartupDirectory.toWString() + L"sc_w3c*.log", L"", true, true);
	#else
	fileList = fileUtil::directoryFileList(gStartupDirectory.hideAsString() + "sc_w3c*.log", "");
	#endif
	if (!fileList.empty())
	{
		#ifdef _WIN32
		for (vector<wstring>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
		#else
		for (vector<string>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
		#endif
		{
			#ifdef _WIN32
			utf32 u32file(*i);
			utf8 u8f(u32file.toUtf8());
			body += "<div style=\"padding-left:27px;\">" + fileUtil::stripPath(u8f) + "</div>";
			#else
			body += "<div style=\"padding-left:27px;\">" + fileUtil::stripPath(*i) + "</div>";
			#endif
		}
	}
	body += "</div>";
#endif
	body += "</td></tr></table></td></tr></table></div>"
			+ getUptimeScript() +
			"<script type=\"text/javascript\">" EL
			"function toggle(opt, on, all) {" EL
			"if(window.XMLHttpRequest){" EL
			"xmlhttp=new XMLHttpRequest();" EL
			"}else{" EL
			"xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");" EL
			"}" EL
			"xmlhttp.open(\"GET\",\"admin.cgi?pass="+gOptions.adminPassword()+"&mode=debug&option=\"+opt+\"&on=\"+on,true);" EL
			"xmlhttp.send(null);" EL
			"if(all){" EL
			"var labels = document.getElementsByTagName('LAB EL');" EL
			"for (var i = 0; i < labels.length; i++) {" EL
			"if (labels[i].htmlFor != '') {" EL
			"$(labels[i].htmlFor).checked=(on=='true'?'true':'');"
			"}" EL
			"}" EL
			"}" EL
			"}</script>" + getfooterStr();

	COMPRESS(header, body);
	header += "Content-Length: " + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}
#endif
