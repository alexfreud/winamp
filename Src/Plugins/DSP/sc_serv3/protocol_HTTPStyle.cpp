#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_HTTPStyle.h"
#include "protocol_shoutcast1Client.h"
#include "protocol_shoutcast2Client.h"
#include "protocol_HTTPClient.h"
#include "protocol_flvClient.h"
#include "protocol_m4aClient.h"
#include "protocol_admincgi.h"
#include "base64.h"
#include "banList.h"
#include "ripList.h"
#include "adminList.h"
#include "webNet/urlUtils.h"
#include "file/fileUtils.h"
#include "aolxml/aolxml.h"
#include "services/stdServiceImpl.h"
#include "global.h"
#include "bandwidth.h"
#include "updater.h"
#include "metadata.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define HEAD_REQUEST (m_httpRequestInfo.m_request == HTTP_HEAD)

size_t gFF_fix = 0;

CacheMap_t m_xmlStatsCache, m_xmlStatisticsCache, m_jsonStatsCache,
		   m_jsonStatisticsCache, m_7Cache, m_PLSCache, m_M3UCache,
		   m_ASXCache, m_QTLCache, m_XSPFCache, m_xmlTracksCache,
		   m_xmlMetadataCache, m_jsonMetadataCache, m_jsonTracksCache,
		   m_xmlPlayedCache, m_jsonPlayedCache, m_htmlPlayedCache,
		   m_streamArtCache, m_playingArtCache, m_crossdomainCache;

AOL_namespace::mutex m_xmlStatsLock, m_xmlStatisticsLock, m_jsonStatsLock,
					 m_jsonStatisticsLock, m_7Lock, m_PLSLock, m_M3ULock,
					 m_ASXLock, m_QTLLock, m_XSPFLock, m_xmlTracksLock,
					 m_xmlMetadataLock, m_jsonMetadataLock, m_jsonTracksLock,
					 m_xmlPlayedLock, m_jsonPlayedLock, m_htmlPlayedLock,
					 m_streamArtLock, m_playingArtLock, m_crossdomainLock;

#ifdef _WIN32
typedef unsigned long in_addr_t;
#endif

const utf8 getStreamHeader(const streamData::streamID_t sid, const utf8& headerTitle)
{
	return "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
		   "<meta name=viewport content=\"width=device-width, initial-scale=1\">"
		   "<title>Shoutcast Server</title>"
		   "<link href=\"index.css\" rel=\"stylesheet\" type=\"text/css\">"
		   "<link href=\"images/favicon.ico\" rel=\"shortcut icon\" type=\"" +
		   gOptions.faviconFileMimeType() + "\"></head><body style=\"margin:0;\">"
		   "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr>"
		   "<td><div style=\"float:left;clear:both;\" class=\"logo\">Shoutcast " +
		   headerTitle + "</div></td><td style=\"text-align:right;vertical-align:bottom;padding-right:0.1em;\">"
		   "<a target=\"_blank\" title=\"Built: " __DATE__ "\" href=\"http://www.shoutcast.com\">"
		   "Shoutcast Server v" + addWBR(gOptions.getVersionBuildStrings() + "/" SERV_OSNAME) +
		   "</a></td></tr><tr><td class=\"thr\" align=\"center\" colspan=\"2\">"
		   "<div id=\"hdrbox\" class=\"tnl\" style=\"justify-content:space-around;display:flex;"
		   "flex-flow:row wrap;\"><div class=\"thr\"><a href=\"index.html?sid=" + tos(sid) +
		   "\">Status</a></div><div class=\"thr\">&nbsp;|&nbsp;</div>";
}

const utf8 getStreamMiddlePlayingHeader(const streamData::streamID_t sid)
{
	return "<div class=\"thr\"><a href=\"played.html?sid=" + tos(sid) +
		   "\">History <img border=\"0\" title=\"History\" "
		   "alt=\"History\" style=\"vertical-align:middle\" "
		   "src=\"images/history.png\"></a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>";
}

const utf8 getStreamMiddleListenHeader(const streamData::streamID_t sid)
{
	return "<div class=\"thr\"><a href=\"listen.pls?sid=" + tos(sid) +
		   "\">Listen <img border=\"0\" title=\"Listen to Stream\" "
		   "alt=\"Listen to Stream\" style=\"vertical-align:middle\" "
		   "src=\"images/listen.png\"></a></div>"
		   "<div class=\"thr\">&nbsp;|&nbsp;</div>";
}

const utf8 getStreamEndHeader(const streamData::streamID_t sid)
{
	return "<div class=\"thr\"><a target=\"_blank\" href=\"home.html?sid=" +
		   tos(sid) + "\">Website</a></div><div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi?sid=" + tos(sid) +
		   "\">Stream Login&nbsp;<img border=\"0\" title=\"Stream Login\nPassword Required\" "
		   "alt=\"Stream Login\nPassword Required\" style=\"vertical-align:middle\" "
		   "src=\"images/lock.png\"></a></div><div class=\"thr\">&nbsp;|&nbsp;</div>"
		   "<div class=\"thr\"><a href=\"admin.cgi\">Server Login&nbsp;"
		   "<img border=\"0\" title=\"Server Login\nPassword Required\" "
		   "alt=\"Server Login\nPassword Required\" style=\"vertical-align:middle\" "
		   "src=\"images/lock.png\"></a></div></div></td></tr></table>";
}

const bool getHideState(const streamData::streamID_t sid)
{
	const utf8 &hideStats = gOptions.getStreamHideStats(sid);
	if (!hideStats.empty())
	{
		return ((hideStats == "stats" || hideStats == "all")) && !(hideStats == "none");
	}
	return false;
}

#define DEBUG_LOG(...)      do { if (gOptions.httpStyleDebug()) DLOG(__VA_ARGS__); } while (0)
#define LOGNAME "[HTTPSTYLE] "

#define COMPRESS(header, body) if (m_compressed && compressData(body)) header += "Content-Encoding:gzip\r\n"

const utf8 getNewVersionMessage(const utf8& ending)
{
	utf8 body = "";
	// display update message where applicable
	updater::verInfo ver;
	if (updater::getNewVersion(ver))
	{
		body += "<table cellspacing=\"0\" cellpadding=\"5\" align=\"center\"><tr><td align=\"center\" class=\"infb\">"
				"<div align=\"center\" class=\"infh\"><b>New DNAS Version Available: " + ver.ver + "</b></div>"
				"Please <a href=\"admin.cgi\"><b>login</b></a> or see <a target=\"_blank\" href=\"" +
				aolxml::escapeXML(ver.log) + "\"><b>here</b></a> to find out more about the new DNAS version."
				+ (!ver.slimmsg.empty() ? "<br><br>" + ver.slimmsg : "") + "</tr></table>" + ending;
	}
	return body;
}

const utf8 getStreamPath(const size_t sid, const bool for_public)
{
	utf8 streamPath;
	config::streamConfig stream;
	if (gOptions.getStreamConfig(stream, sid))
	{
		// if not empty then set to the value otherwise leave as /stream/<sid>
		if (!stream.m_urlPath.empty())
		{
			// but ensure that a / is on the front if not manually specified
			if (stream.m_urlPath[0] == '/')
			{
				streamPath = stream.m_urlPath;
			}
			else
			{
				streamPath = "/" + stream.m_urlPath;
			}

			// this makes sure that if someone sets / or /stream/x/ then
			// if we're asked for a 'public' url that we append the ';'.
			if (for_public && (streamPath.rfind((utf8)"/") == streamPath.size() - 1))
			{
				streamPath += ";";
			}
		}
	}

	if (streamPath.empty())
	{
		if (!sid || (sid == DEFAULT_CLIENT_STREAM_ID))
		{
			streamPath = (!for_public ? "/" : "/;");
		}
		else
		{
			streamPath = "/stream/" + tos(sid) + (!for_public ? "/" : "/;");
		}
	}

	return streamPath;
}

protocol_HTTPStyle::protocol_HTTPStyle (microConnection &mc, const string &firstLine)    throw(exception) :
        runnable (mc), m_clientPort(mc.m_srcPort), m_clientHostName(mc.m_srcHostName), m_clientAddr(mc.m_srcAddress), m_clientLogString(dstAddrLogString(mc.m_srcHostName,mc.m_srcPort))
{
    m_protocols = mc.m_protocols;
    m_outBufferSize = 0;
    m_outBuffer = NULL;
    m_postRequest = 0;
    m_compressed = 0;
    m_postRequestLength = 0;
    m_state = &protocol_HTTPStyle::state_GetLine;
    m_nextState = &protocol_HTTPStyle::state_AnalyzeHTTPHeaders;

	// Parse the first line of the HTTP transaction into it's components
	// (GET/POST etc, url, query etc.) and now is done first so we can
	// abort the request asap if getting bad data instead of waiting to
	// process the headers and then look at the actual HTTP request...
	string request, url, protocolAndVersion;
	const int state = getHTTPRequestDetails(firstLine, request, url, protocolAndVersion);
	if (!request.empty())
	{
		if (request == "GET")
		{
			m_httpRequestInfo.m_request = HTTP_GET;
		}
		else if (request == "POST")
		{
			m_httpRequestInfo.m_request = HTTP_POST;
			m_postRequest = 1;
		}
		else if (request == "HEAD")
		{
			m_httpRequestInfo.m_request = HTTP_HEAD;
		}
		else
		{
			ELOG(m_clientLogString + "Badly formed HTTP request [" + firstLine + "]");
			sendMessageAndClose(MSG_HTTP405);
			return;
		}
	}
	else
	{
		ELOG(m_clientLogString + "Badly formed HTTP request [" + firstLine + "]");
		sendMessageAndClose(MSG_HTTP405);
		return;
	}

	if ((m_httpRequestInfo.m_request == HTTP_UNKNOWN) ||
		(state != 3) || url.empty() || protocolAndVersion.empty() ||
		(protocolAndVersion.find("/") == string::npos))
	{
		ELOG(m_clientLogString + "Badly formed HTTP request [" + firstLine + "]");
		sendMessageAndClose(MSG_HTTP400);
		return;
	}

	// check for query data and finish up url
	string::size_type pos = url.find("?");
	m_httpRequestInfo.m_url = urlUtils::unescapeString(url.substr(0, pos));

	// provide a stripped version to speed up some of the checks
	const utf8::size_type upos = m_httpRequestInfo.m_url.find(utf8("/"));
	m_url = (((upos == 0) && m_httpRequestInfo.m_url.size() > 1) ?
			 m_httpRequestInfo.m_url.substr(upos + 1) : m_httpRequestInfo.m_url);

	// this is so we can do Icecast title updates which use
	// a different path but will do a ?mode=updinfo request
	if (m_url == "admin/metadata")
	{
		m_url = "admin.cgi";
	}

	string queryData = "";
	if (pos != string::npos)
	{
		queryData = url.substr(pos+1);
		const vector<string> queryTokens = tokenizer(queryData,'&');
		utf8 lastToken;
		for (vector<string>::const_iterator i = queryTokens.begin(); i != queryTokens.end(); ++i)
		{
			// this is for a specific case when we get xml titles
			// and we need it to preserve the data instead of it
			// tokenising and leaving broken xml for &amp; cases
			if (!lastToken.empty() && ((*i).find("amp;") == 0))
			{
				m_httpRequestInfo.m_QueryParameters[lastToken] = m_httpRequestInfo.m_QueryParameters[lastToken] +
																 "&" + urlUtils::unescapeString((*i));
				// we don't want to process like normal so skip
				continue;
			}
			pos = (*i).find("=");
			if (pos == string::npos)
			{
				m_httpRequestInfo.m_QueryParameters[(lastToken = urlUtils::unescapeString(*i))] = "";
			}
			else
			{
				m_httpRequestInfo.m_QueryParameters[(lastToken = urlUtils::unescapeString((*i).substr(0, pos)))] = 
													urlUtils::unescapeString((*i).substr(pos + 1));
			}
		}
	}

	if (gOptions.httpStyleDebug())
	{
		DLOG(m_clientLogString + "HTTP Request [" + (m_httpRequestInfo.m_request == HTTP_GET ? "GET" :
													(m_httpRequestInfo.m_request == HTTP_POST ? "POST" :
													(HEAD_REQUEST ? "HEAD" : "UNKNOWN"))) + "]");
		DLOG(m_clientLogString + "HTTP Url [" + m_url + "]");
		if (!queryData.empty()) DLOG(m_clientLogString + "HTTP Query [" + queryData + "]");
		pos = protocolAndVersion.find("/");
		DLOG(m_clientLogString + "HTTP Protocol [" + protocolAndVersion.substr(0, pos) + "]");
		DLOG(m_clientLogString + "HTTP Version [" + protocolAndVersion.substr(pos + 1) + "]");
	}
}

protocol_HTTPStyle::~protocol_HTTPStyle() throw()
{
	socketOps::forgetTCPSocket(m_socket);
}

void protocol_HTTPStyle::timeSlice() throw(std::exception)
{
	(this->*m_state)();
}

void protocol_HTTPStyle::getPNGImage(const uniString::utf8 &png) throw()
{
	const utf8 &modified = mapGet(m_httpRequestInfo.m_HTTPHeaders, utf8("if-modified-since"),utf8("0"));
	const time_t curTime = ::time(NULL),
				 readTime = readRFCDate(modified),
				 diffTime = (curTime - readTime);

	// check if we need to provide a copy or if we can just do a '304 Not Modified' response
	if (!readTime || (diffTime > g_upTime) || (diffTime > 31536000))
	{
		utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:image/png\r\n"
					  "Access-Control-Allow-Origin:*\r\n"
					  "Cache-Control:private,max-age=31536000\r\n"
					  "Expires:" + getRFCDate(curTime + 31536000) + "\r\n"
					  "Last-Modified:" + getRFCDate(g_upTime) + "\r\n"
					  "Content-Length:" + tos(png.size()) + "\r\n\r\n";
		sendMessageAndClose(header + (!HEAD_REQUEST ? png : ""));
	}
	else
	{
		sendMessageAndClose("HTTP/1.0 304 Not Modified\r\n\r\n");
	}
}

const utf8 protocol_HTTPStyle::getClientIP(const bool streamPublic, const utf8 &publicIP) throw()
{
	// test for potentially invalid IPs or ones that will cause the playlist link generation to fail
	// attempting to use the server path provided by the YP if in public mode, otherwise uses 'host'
	DEBUG_LOG(m_clientLogString + "streamPublic: " + tos(streamPublic) + " " +
			  "publicIP: " + publicIP + " " + "hostIP: " + m_hostIP + " " +
			  "clientAddr: " + g_IPAddressForClients);
	return (!streamPublic || publicIP.empty() ?
			((g_IPAddressForClients.find(utf8("0.")) == 0 ||
			 // allow localhost / loopback connections through for admin access
			 ((!g_IPAddressForClients.empty() && g_IPAddressForClients.find(utf8("127.")) == 0)) ||
			  g_IPAddressForClients.empty()) && !m_hostIP.empty() ?
			  m_hostIP : (!m_hostIP.empty() ? m_hostIP : g_IPAddressForClients)) : publicIP);
}

const bool isCDNMaster(const streamData::streamID_t sid)
{
	if (!gOptions.cdn().empty())
	{
		// check if opt-in or opt-out
		bool isCDN = (gOptions.cdn() == "always" || gOptions.cdn() == "master");
		// then if manually specifed, we override
		if (gOptions.read_cdn_master(sid))
		{
			const int master = gOptions.cdn_master(sid);
			if (master != -1)
			{
				isCDN = (!!master);
			}
		}
		return isCDN;
	}
	return false;
}

const bool isCDNSlave(const streamData::streamID_t sid)
{
	if (!gOptions.cdn().empty())
	{
		// check if opt-in or opt-out
		// for 'master' we run opt-in
		bool isCDN = (gOptions.cdn() == "always");
		// then if manually specifed, we override
		if (gOptions.read_cdn_slave(sid))
		{
			const int slave = gOptions.cdn_slave(sid);
			if (slave != -1)
			{
				isCDN = (!!slave);
			}
		}
		return isCDN;
	}
	return false;
}

const bool protocol_HTTPStyle::isAdminAccessAllowed(const uniString::utf8 &hostIP, const uniString::utf8 &hostName) throw()
{
	int inAdminIp = g_adminList.find(hostIP),
					inAdminName = inAdminIp;

	DEBUG_LOG(m_clientLogString + "Pre-check admin access - hostIP: " +
			  hostIP + ", hostName: " + hostName + ", " + "inAdminIp: " +
			  tos(inAdminIp) + ", inAdminName: " + tos(inAdminName));

	// 1 if found, 0 if not, -1 if empty (assume allowed)
	if (inAdminIp == -1)
	{
		// abort and just pass through if there's no list
		return true;
	}

	// in a lot of cases it will work out that
	// hostIP and hostName are set as the same
	// so only try if there is a difference...
	if (!hostName.empty() && (hostIP != hostName))
	{
		inAdminName = g_adminList.find(hostName);
	}

	DEBUG_LOG(m_clientLogString + "Checking admin access - hostIP: " +
			  hostIP + ", hostName: " + hostName + ", " + "inAdminIp: " +
			  tos(inAdminIp) + ", inAdminName: " + tos(inAdminName));

	// if either check matches then we will allow the
	// request as the host might map to an allowed IP
	// etc. is only if both don't match then we block
	return (inAdminIp || inAdminName);
}

const bool protocol_HTTPStyle::isAccessAllowed(const streamData::streamID_t sid, const utf8 &hostAddr = "",
											   const bool showOutput = false) throw()
{
	if (isUserAgentOfficial(m_userAgentLowered))
	{
		return true;
	}

	const utf8& addr = (hostAddr.empty() ? m_clientAddr : hostAddr);
	const bool inBan = g_banList.find(addr,((gOptions.read_stream_banFile(sid) && !gOptions.stream_banFile(sid).empty()) ? sid : 0));
	const bool inRip = g_ripList.find(addr,((gOptions.read_stream_ripFile(sid) && !gOptions.stream_ripFile(sid).empty()) ? sid : 0));
	const bool isCDN = isCDNMaster(sid);
	bool ripOnly = (isCDN ? true : gOptions.stream_ripOnly(sid));

	if (!gOptions.read_stream_ripOnly(sid))
	{
		ripOnly = (isCDN ? true : gOptions.ripOnly());
	}

	if (gOptions.httpStyleDebug())
	{
		string host = m_clientAddr.hideAsString();
		if (gOptions.nameLookups())
		{
			host = m_clientAddr.hideAsString();
			string addr = host;
			u_short port = 0;
			socketOps::getpeername(this->m_socket, addr, port);

			if (socketOps::addressToHostName(addr, port, host))
			{
				host = addr;
			}

			DEBUG_LOG(m_clientLogString + "Checking access rights - addr: " + addr + ", " +
					  "host: " + host + ", " + "inBan: " + tos(inBan) + ", " + "inRip: " +
					  tos(inRip) + ", " + "isCDN: " + tos(isCDN) + ", " + "ripOnly: " + tos(ripOnly));
		}
		else
		{
			DEBUG_LOG(m_clientLogString + "Checking access rights - addr: " + addr + ", " +
					  "inBan: " + tos(inBan) + ", " + "inRip: " + tos(inRip) + ", " +
					  "isCDN: " + tos(isCDN) + ", " + "ripOnly: " + tos(ripOnly));
		}
	}

	// check here if we're ok to try to provide the stream to the client or not
	bool allowed = true;
	if ((ripOnly || inBan) && (!inRip))
	{
		allowed = false;
		if (ripOnly)
		{
			// allow localhost / loopback connections through for admin access
			if (!m_clientAddr.empty() && m_clientAddr.find(utf8("127.")) == 0)
			{
				allowed = true;
			}
			else
			{
				if (showOutput)
				{
					if (gOptions.nameLookups())
					{
						string hostName = m_clientAddr.hideAsString();
						string addr = hostName;
						u_short port = 0;
						socketOps::getpeername(this->m_socket, addr, port);

						if (socketOps::addressToHostName(addr, port, hostName))
						{
							hostName = addr;
						}

						if (hostName != addr)
						{
							WLOG("[" + hostName + " (" + m_clientAddr + ") sid=" + tos(sid) + "] Host not in reserved list - disconnecting.");
						}
						else
						{
							WLOG("[" + m_clientAddr + " sid=" + tos(sid) + "] IP not in reserved list - disconnecting.");
						}
					}
					else
					{
						WLOG("[" + m_clientAddr + " sid=" + tos(sid) + "] IP not in reserved list - disconnecting.");
					}

					m_result.schedule(1000);
				}
			}
		}
		else
		{
			// allow loopback address through for admin access
			if (!m_clientAddr.empty() && m_clientAddr.find(utf8("127.")) == 0)
			{
				allowed = true;
			}
			else
			{
				utf8::size_type pos;
				if (!m_referer.empty() && (pos = m_referer.rfind(utf8("/admin.cgi"))) != utf8::npos)
				{
					allowed = true;
				}

				if (showOutput && !(m_url == "index.css"))
				{
					allowed = false;
					if (gOptions.nameLookups())
					{
						string hostName = m_clientAddr.hideAsString();
						string addr = hostName;
						u_short port = 0;
						socketOps::getpeername(this->m_socket, addr, port);

						if (socketOps::addressToHostName(addr, port, hostName))
						{
							hostName = addr;
						}

						if (hostName != addr)
						{
							ILOG("[" + hostName + " (" + m_clientAddr + ") sid=" + tos(sid) + "] Host in banned list - disconnecting.");
						}
						else
						{
							ILOG("[" + m_clientAddr + " sid=" + tos(sid) + "] IP in banned list - disconnecting.");
						}
					}
					else
					{
						ILOG("[" + m_clientAddr + " sid=" + tos(sid) + "] IP in banned list - disconnecting.");
					}
					
					m_result.schedule(1000);
				}
			}
		}
	}
	return allowed;
}

const bool protocol_HTTPStyle::isViewingAllowed(const streamData::streamID_t sid, const utf8 &password,
												const bool no_stream, bool &adminOverride,
												const bool hide, bool &passworded) throw()
{
	adminOverride = false;
	// when the hidestats option is enabled, still need to allow it on the admin page
	if (m_referer.rfind(utf8("/admin.cgi")) != utf8::npos)
	{
		adminOverride = true;
	}

	// do a password check if we've got a hidden page or if a password param is present
	bool proceed = false;

	if ((hide == true) || (passworded == true))
	{
		passworded = false;
		if (!password.empty())
		{
			utf8 streamPassword;
			if (!no_stream)
			{
				streamPassword = gOptions.stream_password(sid);
				if (!gOptions.read_stream_password(sid) && streamPassword.empty())
				{
					streamPassword = gOptions.password();
				}
			}

			utf8 streamAdminPassword = gOptions.stream_adminPassword(sid);
			if (!gOptions.read_stream_adminPassword(sid) && streamAdminPassword.empty())
			{
				streamAdminPassword = gOptions.adminPassword();
			}

			passworded = ((!streamPassword.empty() && (password == streamPassword)) ||
						  (!streamAdminPassword.empty() && (password == streamAdminPassword)));

			if (hide)
			{
				proceed = passworded;
			}
		}
	}

	return proceed;
}

const bool protocol_HTTPStyle::findBaseStream(bool& no_sid, streamData::streamID_t& sid)
{
	// if no sid is specified, attempt to match to the only stream (v1 like behaviour)
	// before just attempting to provide the results for the default stream id (sid=1)
	if (no_sid)
	{
		bool htmlPage = false;
		streamData::streamID_t found_sid = streamData::getStreamIdFromPath(m_httpRequestInfo.m_url, htmlPage);
		if (found_sid > 0)
		{
			sid = found_sid;
			no_sid = false;
		}

		streamData::streamID_t lastSID = 0;
		if (!found_sid && streamData::totalActiveStreams(lastSID) == (streamData::streamID_t)DEFAULT_CLIENT_STREAM_ID)
		{
			sid = lastSID;
			return true;
		}
	}
	return false;
}

const bool protocol_HTTPStyle::getCachedResponse(cacheItem *item, AOL_namespace::mutex &lock, const int limit)
{
	utf8 response;
	if (GetFromCache(item, lock, m_lastActivityTime, !!m_compressed, HEAD_REQUEST, response, limit))
	{
		sendMessageAndClose(response);
		return true;
	}
	return false;
}

void protocol_HTTPStyle::sendCachedResponse(cacheItem *item, CacheMap_t &cache, AOL_namespace::mutex &lock,
											uniString::utf8 &header, uniString::utf8 &body,
											const streamData::streamID_t sid,
											const bool jsonp, const bool noCompress)
{
	if (sid && !noCompress)
	{
		COMPRESS(header, body);
	}
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";

	const utf8 &response = (header + body);
	if (!jsonp)
	{
		// if a callback is specified then is best to not cache
		AddorUpdateCache(item, m_lastActivityTime, !!m_compressed,
						 header, response, cache, lock, sid);
	}
	sendMessageAndClose((!HEAD_REQUEST ? response : header));
}

// look at the HTTP intro line and headers and decide what to do next		
void protocol_HTTPStyle::state_DetermineAction() throw(exception)
{
	utf8 relayHostIP, relayHostName, XFF;

	// figure out what to do next based on HTTP intro line and headers
	// dump http headers to log
	for (map<utf8,utf8>::const_iterator i = m_httpRequestInfo.m_HTTPHeaders.begin(); i != m_httpRequestInfo.m_HTTPHeaders.end(); ++i)
	{
		DEBUG_LOG(m_clientLogString + "HTTP Header [" + (*i).first + ":" + (*i).second + "]");
		// grab the ip used for accessing to determine
		// the ip to return in the listen action when
		// destip has not been specified or is invalid
		if ((*i).first == "host")
		{
			m_hostIP = (*i).second;
			if (!m_hostIP.empty())
			{
				// strip off the port from the ip though
				// could just use it but want to ensure
				// the correct port is used for it all
				utf8::size_type pos = utf8::npos;
				if ((pos = m_hostIP.find(utf8(":"))) != utf8::npos)
				{
					m_hostIP.resize(pos);
				}
			}
		}

		// used specificially with the fire buuilds to be able to work around
		// issues with the VIPs and what is checked against the reserved list
		// -this gets the IP and also will resolve the hostname as applicable
		else if ((*i).first == "icy-host")
		{
			relayHostIP = (*i).second;
			if (gOptions.nameLookups())
			{
				u_short port = 0;
				string addr, hostName;
				socketOps::getpeername(m_socket, addr, port);
				hostName = relayHostIP.hideAsString();
				if (socketOps::addressToHostName(addr, port, hostName))
				{
					hostName = relayHostIP.hideAsString();
				}
				relayHostName = hostName;
			}
		}

		else if ((*i).first == "x-forwarded-for")
		{
			//utf8 blah = "129.78.138.66";//", 129.78.64.103";
			// this can be in the format of client
			// or client, proxy1, proxy2, etc. and
			// it can be open to spoofing so we'll
			// see what complaints that will arise
			if (!(*i).second.empty())
			{
				utf8::size_type pos = utf8::npos;
				if ((pos = (*i).second.find(utf8(","))) != utf8::npos)
				{
					utf8 tempAddr = (*i).second.substr(0, pos);
					if (!tempAddr.empty())
					{
						tempAddr = stripWhitespace(tempAddr);
						in_addr_t ip =  inet_addr((const char *)tempAddr.c_str());
						if (ip != INADDR_NONE)
						{
							XFF = tempAddr;
						}
					}
				}
				else
				{
					utf8 tempAddr = stripWhitespace((*i).second);
					if (!tempAddr.empty())
					{
						in_addr_t ip =  inet_addr((const char *)tempAddr.c_str());
						if (ip != INADDR_NONE)
						{
							XFF = tempAddr;
						}
					}
				}
			}
		}

		else if ((*i).first == "referer")
		{
			m_referer = (*i).second;
		}

		else if ((*i).first == "accept-encoding")
		{
			utf8 encoding = (*i).second;
			if (!encoding.empty())
			{
				vector<utf8> encodingTokens = tokenizer(stripWhitespace(encoding), ',');
				for (vector<utf8>::const_iterator i = encodingTokens.begin(); i != encodingTokens.end(); ++i)
				{
					utf8 value = stripWhitespace(*i);
					if (value == "gzip")
					{
						m_httpRequestInfo.m_AcceptEncoding |= ACCEPT_GZIP;
						m_compressed = 1;
					}
					else if (value == "deflate")
					{
						m_httpRequestInfo.m_AcceptEncoding |= ACCEPT_DEFLATE;
					}
				}
			}
		}
	}

	// attempt to work out the streamid to use, keeping a track of things, etc
	//
	// for b195+ we recognise the sid from the password if
	// its not provided as a specific parameter on the url
	streamData::streamID_t sid = mapGet(m_httpRequestInfo.m_QueryParameters, "sid", -1), realSID = sid;
	// with means in b72+ to do this based on the streampath instead of by sid
	utf8 sp = mapGet(m_httpRequestInfo.m_QueryParameters, "sp", (utf8)"");
	bool hasMount = false;
	if (sp.empty())
	{
		// this allows us to support icecast based title updates and on
		// reflection would have been the better naming to use vs 'sp'.
		sp = mapGet(m_httpRequestInfo.m_QueryParameters, "mount", (utf8)"");
		hasMount = !sp.empty();
	}
	if (!sp.empty())
	{
		bool htmlPage = false;
		// make sure there is a / on the front so that we're
		// going to be able to search for a match correctly.
		if (sp.find(utf8("/")) != 0)
		{
			sp = "/" + sp;
		}
		streamData::streamID_t found_sid = streamData::getStreamIdFromPath(sp, htmlPage);
		if (found_sid > 0)
		{
			realSID = sid = found_sid;
		}
	}

	bool no_sid = ((int)sid <= 0);
	// check that we've got a valid sid, otherwise force assume it's sid=1 i.e. helps for just /listen.pls
	sid = ((sid >= DEFAULT_CLIENT_STREAM_ID) && !(no_sid && ((int)sid <= -1)) ? sid : DEFAULT_CLIENT_STREAM_ID);

	// this will better handle user agents which could have high bit code points
	// so we attempt to convert the string to a hopefully valid utf8 encoded string
	string agent = string(mapGet(m_httpRequestInfo.m_HTTPHeaders, "user-agent", (utf8)"").toANSI());
	m_userAgentLowered = toLower((m_userAgent = asciiToUtf8(agent)));

	if (isUserAgentRelay(m_userAgentLowered))
	{
		bool allowRelay = gOptions.stream_allowRelay(sid);
		if (!gOptions.read_stream_allowRelay(sid))
		{
			allowRelay = gOptions.allowRelay();
		}

		if (!allowRelay && !(m_url == "admin.cgi"))
		{
			ILOG(m_clientLogString + "Relay not allowed: `" + m_userAgent + "'.");
			sendMessageAndClose(MSG_HTTP403);
			return;
		}
	}

	if ((m_userAgentLowered.find(utf8("rip")) != utf8::npos) ||
		(m_userAgentLowered.find(utf8("copy")) != utf8::npos))
	{
		if (!(m_url == "admin.cgi"))
		{
			ILOG(m_clientLogString + "Stream savers not allowed.");
			sendMessageAndClose(MSG_HTTP403);
			return;
		}
	}

	utf8::size_type ipos = m_url.find(utf8("images/"));
	if ((ipos == 0) || (m_url == "favicon.ico"))
	{
		utf8 url = (ipos == 0 ? m_url.substr(7) : m_url);
		if (url == "favicon.ico")
		{
			const utf8 &modified = mapGet(m_httpRequestInfo.m_HTTPHeaders, utf8("if-modified-since"), utf8("0"));
			const time_t curTime = ::time(NULL),
						 readTime = readRFCDate(modified),
						 diffTime = (curTime - readTime);

			// check if we need to provide a copy or if we can just do a '304 Not Modified' response
			if (!readTime || (diffTime > gOptions.m_favIconTime) || (diffTime > 31536000))
			{

				const int g_favIconSize = 1150;
				const uniString::utf8::value_type g_favIcon[] = {
					"\x00\x00\x01\x00\x01\x00\x10\x10\x00\x00\x01\x00\x20\x00\x68\x04"
					"\x00\x00\x16\x00\x00\x00\x28\x00\x00\x00\x10\x00\x00\x00\x20\x00"
					"\x00\x00\x01\x00\x20\x00\x00\x00\x00\x00\x40\x04\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x14\x9a\xfd\x2f\x13\x95\xfe\x54\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x18\x98\xfd\x3d\x16\x98\xfd\xff\x15\x97"
					"\xfd\xab\x11\x95\xfd\x17\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x4c\xba\xfc\x02\x31\xa5\xfc\xa2\x1a\x9a"
					"\xfd\xff\x16\x99\xfd\xea\x13\x95\xfd\x5a\x07\x7a\xfe\x01\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44\xb4\xfb\x05\x46\xb2"
					"\xfc\xb2\x28\xa0\xfb\xff\x16\x98\xfd\xff\x15\x98\xfd\xb0\x11\x93"
					"\xfd\x1b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x46\xb5"
					"\xfd\x09\x4b\xb7\xfd\xbf\x3a\xaa\xfb\xff\x1a\x98\xfc\xff\x16\x98"
					"\xfd\xed\x14\x98\xfe\x60\x08\x81\xfe\x01\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x13\x98"
					"\xfd\x21\x1e\x9e\xfd\x82\x41\xb1\xfd\xfa\x49\xb4\xfc\xff\x28\x9e"
					"\xfa\xff\x16\x97\xfd\xff\x15\x98\xfd\xb6\x10\x95\xfd\x1d\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x15\x98"
					"\xfd\x48\x16\x98\xfd\xff\x19\x99\xfd\xff\x42\xb0\xfc\xff\x4d\xb7"
					"\xfc\xff\x3a\xa9\xfa\xff\x1a\x97\xfb\xff\x14\x98\xf9\x8f\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x26\x99"
					"\xf9\x17\x3a\xa8\xfa\xaf\x3d\xaa\xfa\xfe\x40\xad\xfb\xff\x3d\xac"
					"\xfb\xff\x4d\xb7\xfc\xff\x49\xb4\xfc\xf6\x24\x97\xf5\x6c\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x4a\xb5\xfd\x48\x4c\xb7\xfc\xd7\x1f\x97"
					"\xf9\xff\x36\xa7\xfa\xff\x19\x9a\xfd\xc8\x0e\x92\xfe\x0c\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x2d\xac\xfd\x07\x1e\x98"
					"\xfa\xd6\x4c\xb6\xfc\xf1\x27\x9d\xfa\xff\x15\x97\xfd\xbe\x10\x9a"
					"\xfd\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x12\x90"
					"\xfa\x8d\x4a\xb5\xfc\x1b\x4a\xb6\xfc\xa4\x29\xa0\xfb\xfd\x15\x97"
					"\xfd\xb6\x10\x94\xfd\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11\x8e"
					"\xf9\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x49\xb6\xfc\x3d\x27\xa1"
					"\xfc\xce\x15\x97\xfd\x25\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x2d\x2d\x2d\x02\x2e\x2e\x2e\x08\x0f\x8c"
					"\xfa\x4a\x2a\x2a\x2a\x09\x00\x00\x00\x00\x00\x00\x00\x00\x45\xb4"
					"\xf8\x04\x13\x92\xf7\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x36\x36\x36\x29\x2f\x2f\x2f\x2d\x0e\x7f"
					"\xe7\x2e\x31\x31\x31\x38\x33\x33\x33\x21\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x35\x35\x35\x33\x31\x31\x31\x50\x23\x55"
					"\x85\x38\x32\x32\x32\x4d\x33\x33\x33\x21\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x2b\x2b\x2b\x02\x34\x34\x34\x4d\x35\x35"
					"\x35\x60\x32\x32\x32\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcf\xff"
					"\x00\x00\xc3\xff\x00\x00\xc0\xff\x00\x00\xe0\x7f\x00\x00\xf0\x1f"
					"\x00\x00\xf0\x0f\x00\x00\xf0\x0f\x00\x00\xf0\x0f\x00\x00\xfc\x0f"
					"\x00\x00\xfe\x07\x00\x00\xff\x03\x00\x00\xff\x63\x00\x00\xfc\x33"
					"\x00\x00\xfc\x1f\x00\x00\xfc\x1f\x00\x00\xfc\x3f\x00\x00"
				};

				const int g_favIconSizeGZ = 498;
				const uniString::utf8::value_type g_favIconGZ[] = {
					"\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03\x63\x60\x60\x04\x42\x01"
					"\x01\x06\x20\xa9\xc0\x90\xc1\xc2\xc0\x20\xc6\xc0\xc0\xa0\x01\xc4"
					"\x40\x21\xa0\x08\x44\x1c\x04\x1c\x58\x18\x70\x02\x91\x59\x7f\xf5"
					"\x85\xa7\xfe\x0b\xc1\xad\x02\x3f\x90\x98\xf1\xd7\x56\x6c\xc6\xdf"
					"\xff\xa2\xd3\xff\xae\x16\x9c\xfa\x57\x9c\x54\xfd\x3e\xbb\xfe\x30"
					"\x19\x2e\xfd\xb3\x48\x6a\xd6\xdf\xff\x62\x33\xff\xbe\x12\x9e\xfa"
					"\x37\x8a\xbd\xea\x1f\x23\x29\x66\xb8\x6c\xf9\xcd\xea\xb6\xe9\xcf"
					"\x26\x8d\x05\xbf\xff\x83\xdd\x32\xe3\xef\x06\xc1\xc9\x7f\xa5\x49"
					"\x31\xc3\x6d\xeb\x5f\x4e\xef\xed\x7f\xf7\x5b\xad\xfa\xfd\x5f\x6a"
					"\xc6\x1f\x90\x39\x6f\x45\x66\xfc\x4b\xe0\x68\x24\xce\x2d\xc2\x33"
					"\xfe\x2a\xca\xcd\xfb\xdb\xe4\xb8\xf1\xef\x2f\xcf\x2d\x7f\xfe\x6b"
					"\xcc\xfb\xf5\x5f\x6c\x3a\xd8\x2d\xdb\x04\xa6\xfe\x95\x25\xa4\x1f"
					"\xa8\xce\x03\xe4\x76\xc9\x99\x7f\xff\x3b\x6d\xf8\xf3\xdf\x77\xfb"
					"\x9f\xff\x56\x2b\x7f\xfd\x97\x9a\xfe\xfb\xbf\xc8\x8c\x9f\xfd\x84"
					"\xf4\xab\xcd\xfc\x29\x6e\xb5\xe2\xd7\x7a\xdb\x55\xbf\xfe\x39\xac"
					"\xfd\xfd\xdf\x76\xcd\x6f\xb0\x19\x40\xb7\x7c\x53\x99\xfe\x35\x87"
					"\x18\x3f\x80\x80\xd7\xd6\xbf\x1e\x3e\xdb\xff\x5c\x97\x9f\xfe\xf3"
					"\xbf\xd9\xf2\x5f\xff\x25\x67\xfd\x3d\xc1\x37\xe9\x1f\x0f\xb1\xfa"
					"\x41\x40\x77\xcd\x5f\x76\xb9\x19\xbf\xae\xf9\x6c\xfb\xf3\x51\x7d"
					"\xee\x2f\x50\xda\xd8\x27\x30\xeb\x2f\x17\x29\x66\x08\x4d\xf8\xd5"
					"\xeb\xb5\xf5\x8f\xb4\xd7\xb6\x3f\x4b\x34\x17\xfc\xfe\x0b\x34\x63"
					"\x9b\xc0\x94\xbf\x6c\xc4\xea\x17\xec\xfb\x09\xf7\xb3\xe7\xb6\x3f"
					"\xb6\xea\x0b\xff\x9c\x03\x9a\xa1\x4a\xd0\xed\xba\xba\x4c\x7a\x7a"
					"\x7a\x1c\xfc\x3d\xbf\xbc\xb4\xb4\xb4\x38\x61\xe2\xae\x5b\x7e\xb0"
					"\x08\x4f\xfa\x4e\xd0\x7e\x33\x33\x33\x4d\x7d\x7d\x7d\x5d\xbe\xfa"
					"\xe7\x7a\x86\x86\x86\x16\xc6\xc6\xc6\x8a\xc4\xba\x19\x04\x4c\x4d"
					"\x4d\x8d\x81\xfa\x02\x94\x43\x5b\x2d\x8c\x8c\x8c\x7c\x49\xd5\xaf"
					"\xad\xad\xcd\x64\x62\x62\xe2\x0b\x34\x27\x01\xa8\xdf\x01\x97\xba"
					"\xf3\xff\x19\x18\x0e\x03\xf1\x01\x20\x7e\x50\xcf\xc0\xf0\x41\x1e"
					"\x88\xf9\x11\xf8\x0f\x10\xff\x63\x67\x60\xf8\xcf\x0c\xc4\xc9\x40"
					"\xbe\x31\x10\xcb\x43\xb1\x3d\x03\x03\x00\x34\x69\x3c\x26\x7e\x04"
					"\x00\x00"
				};

				const time_t modTime = (!gOptions.m_favIconTime ? (gOptions.m_favIconTime = curTime) : gOptions.m_favIconTime);
				utf8 header = "HTTP/1.0 200 OK\r\n"
							  "Access-Control-Allow-Origin:*\r\n"
							  "Content-Type:" + gOptions.faviconFileMimeType() + "\r\n"
							  "Content-Length:" + tos((m_compressed ? g_favIconSizeGZ : g_favIconSize)) + "\r\n"
							  + (m_compressed ? "Content-Encoding:gzip\r\n" : "") +
							  "Cache-Control:private,max-age=31536000\r\n"
							  "Expires:" + getRFCDate(::time(NULL) + 31536000) + "\r\n"
							  "Last-Modified:" + getRFCDate(0) + "\r\n\r\n",
				     body = (m_compressed ? utf8(g_favIconGZ,g_favIconSizeGZ) : utf8(g_favIcon,g_favIconSize));

				if(gOptions.faviconFile() == "")
				{
				}
				else
				{
					if (gOptions.m_faviconBody.empty())
					{
						body = loadLocalFile(fileUtil::getFullFilePath(gOptions.faviconFile()));
						if (!body.empty())
						{
							gOptions.m_faviconBodyGZ = gOptions.m_faviconBody = body;

							gOptions.m_faviconHeader = "HTTP/1.0 200 OK\r\n"
													   "Content-Type:" + gOptions.faviconFileMimeType() + "\r\n"
													   "Content-Length:" + tos(body.size()) + "\r\n"
													   "Cache-Control:private,max-age=31536000\r\n"
													   "Expires:" + getRFCDate(curTime + 31536000) + "\r\n"
													   "Last-Modified:" + getRFCDate(modTime) + "\r\n\r\n";

							if (compressData(gOptions.m_faviconBodyGZ))
							{
								gOptions.m_faviconHeaderGZ = "HTTP/1.0 200 OK\r\n"
															 "Content-Type:" + gOptions.faviconFileMimeType() + "\r\n"
															 "Content-Length:" + tos(gOptions.m_faviconBodyGZ.size()) + "\r\n"
															 "Cache-Control:private,max-age=31536000\r\n"
															 "Content-Encoding:gzip\r\n"
															 "Expires:" + getRFCDate(curTime + 31536000) + "\r\n"
															 "Last-Modified:" + getRFCDate(modTime) + "\r\n\r\n";
							}
							else
							{
								gOptions.m_faviconHeaderGZ.clear();
							}
						}
						else
						{
							body = MSG_HTTP404;
						}
					}

					if (!gOptions.m_faviconBody.empty())
					{
						// make sure there is a gzipped version available to send if signalled as supported
						if (m_compressed && !gOptions.m_faviconBodyGZ.empty())
						{
							body = gOptions.m_faviconBodyGZ;
							header = gOptions.m_faviconHeaderGZ;
						}
						else
						{
							body = gOptions.m_faviconBody;
							header = gOptions.m_faviconHeader;
						}
					}
				}

				sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
			}
			else
			{
				sendMessageAndClose("HTTP/1.0 304 Not Modified\r\n\r\n");
			}
			return;
		}
		else if (url == "listen.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x16\x00\x00\x00\x0D\x08\x04\x00\x00\x00\x07\xAC\x56"
							"\xE8\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC3\x00\x00\x0E"
							"\xC3\x01\xC7\x6F\xA8\x64\x00\x00\x01\x2A\x49\x44\x41\x54\x28\xCF"
							"\x63\xF8\xC7\x88\x05\xBA\x82\x21\xF3\x3F\xAD\x5F\x13\x9E\x4F\x7D"
							"\x33\xF9\x9F\xF2\x3F\x1E\x90\x38\x03\x16\xA5\xE9\xFF\xFE\xEF\xFA"
							"\xFF\xEF\xFF\x3F\x96\x4F\x3A\xC7\xF6\xA4\xBD\xBB\x76\xF7\xE3\xAD"
							"\x7F\x1A\x20\xE5\x98\x8A\x3B\xFF\xFD\x9F\xF1\x9F\x01\xA4\xD8\xF0"
							"\xBD\xF1\xB9\x5D\x1E\x9F\x95\x7E\x5E\xBC\xF7\x76\xDD\x3F\x41\x54"
							"\xC5\xAB\xC1\xF0\x7F\x19\x50\x29\x48\xF1\xB7\xA5\xBB\xB6\x3E\xDF"
							"\xF2\xEA\x76\xE4\xCF\xA0\x5F\x6F\x3F\xFE\xD3\xFE\xC7\x8F\xAC\xF8"
							"\x7F\x08\x18\x32\x40\x15\x9F\x5F\xEF\xF5\xD4\xF3\xE9\x8B\x3D\xD7"
							"\x3F\xF1\xFF\xBB\xF8\xE9\x6E\x1E\x9A\x62\x06\x24\xF8\xF7\xFF\xCB"
							"\x15\x4F\xF6\x58\xBC\x9A\x7D\xEE\xF5\x47\xD7\xDF\x53\xBF\x3D\x6A"
							"\xC0\xAB\xB8\x5E\xED\xED\xD1\xDC\x87\xA5\xF7\x5F\xEE\xF3\x79\x56"
							"\x74\x8B\x80\xE2\x57\x5D\x4F\x8E\x1B\x7E\x58\x78\xE5\xF5\x17\xE7"
							"\x3F\x33\x7E\x60\x28\x46\x75\xF3\x8B\xEE\xCC\x47\xC1\xCF\x5F\x1D"
							"\x3B\xF3\x95\xE7\xDF\xA5\xAF\x77\x73\x51\x15\xA3\x85\xC6\x9F\xD6"
							"\xB3\x2B\x5E\xEE\x7E\x71\xD1\xED\x4F\xEE\xAF\xF7\x5F\xD0\x43\x03"
							"\x3D\x9C\x15\x7F\x68\x9F\x3B\x68\xF7\x51\xEB\xEB\x8D\xCB\xAF\xA7"
							"\xFC\xE3\xC7\x1F\x83\xCC\x9F\x74\x4E\x1C\xCE\x7C\x71\xE3\xF2\xA7"
							"\xF3\xFF\x14\xB0\xC7\x20\x6A\xDA\xE8\x7A\xD6\xFF\xAA\x0F\xA8\x94"
							"\x0B\x24\x0E\x00\x69\xB1\x67\x35\x8E\x65\x16\x39\x00\x00\x00\x00"
							"\x49\x45\x4E\x44\xAE\x42\x60\x82", 376);
			getPNGImage(png);
		}
		else if (url == "history.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x0D\x00\x00\x00\x0F\x08\x04\x00\x00\x00\x95\x2A\x8D"
							"\xFC\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0B\x12\x00\x00\x0B"
							"\x12\x01\xD2\xDD\x7E\xFC\x00\x00\x00\xA5\x49\x44\x41\x54\x18\xD3"
							"\x6D\x90\xA1\x12\xC3\x20\x0C\x86\x79\x84\x49\xEC\xE4\x64\x65\xED"
							"\x24\x72\x72\x76\xB2\xB2\x72\xB6\xB2\xB2\x12\x5B\x89\x9C\x9C\xED"
							"\x23\x20\x79\x95\x7F\x3F\xA1\x39\x22\xC6\x77\x90\xE3\xBE\x4B\x02"
							"\x71\x01\x16\x8F\x19\x4E\x57\xE0\xA5\x73\xC3\xD8\x65\xC0\x62\x18"
							"\x28\xEE\x2A\x03\x36\xC3\x20\x78\x55\xBB\x21\x09\x93\xAA\x0F\x59"
							"\xF1\x34\x31\xA8\x3A\x58\xE8\xC5\x5D\x63\x64\xCE\x17\x0F\x55\x91"
							"\x05\x22\xB2\xC4\x84\x0B\xCF\xB5\xF7\x9A\xB8\xB3\xC4\xC4\x27\x78"
							"\x16\x2D\x10\x95\x4D\xDE\x8E\xAB\x30\x57\x59\x7B\xB5\x6E\x35\x26"
							"\x7E\xBA\x71\xE0\x7C\x61\xA7\xFD\xEC\xDD\xB2\x2C\x1B\x07\x35\xB2"
							"\x7C\xE9\x93\xD4\x75\x70\x4C\xCB\x3F\xE1\x5C\x61\x0F\x15\x3F\x46"
							"\x11\xE5\xA0\x02\x5A\xBC\xC7\x00\x00\x00\x00\x49\x45\x4E\x44\xAE"
							"\x42\x60\x82", 243);
			getPNGImage(png);
		}
		else if (url == "lock.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x08\x00\x00\x00\x0B\x08\x04\x00\x00\x00\xE8\x92\x04"
							"\xAE\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC3\x00\x00\x0E"
							"\xC3\x01\xC7\x6F\xA8\x64\x00\x00\x00\x81\x49\x44\x41\x54\x08\xD7"
							"\x8D\xC8\x3D\x0E\x82\x40\x10\x86\xE1\xA1\xB1\xF2\xAF\x30\xA1\xE0"
							"\x4A\xD6\x34\x9E\x80\xD6\x0A\xAD\x37\x14\x34\x44\xAF\xA0\xD7\xD2"
							"\x1B\xD0\xC1\xEE\xB7\xCC\x30\xC3\x96\xD8\x99\xA7\x79\xF3\x92\x66"
							"\x52\xA2\xF7\x09\x7A\x29\x35\xA3\xB9\x80\x7F\xC7\x5C\x73\x7D\x01"
							"\xC3\x5C\x10\x57\xDF\x70\xB0\x2B\xDF\xA6\xA3\x7E\x46\xAE\x88\xDD"
							"\x73\x3A\x0B\x3C\x86\x0B\x3F\xC0\x8E\x10\x6B\xB9\x0B\x80\xD0\x4A"
							"\x2D\x88\xD4\xD9\xD6\x76\x76\x4A\xF6\xA9\x3A\x23\x67\x9B\x15\xF7"
							"\xCF\x68\x7E\x46\x63\x14\x6C\x5C\x09\xB6\x00\x7C\x27\x7A\xE3\x33"
							"\xC6\x13\x8C\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 207);
			getPNGImage(png);
		}
		else if (url == "streamart.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x0D\x00\x00\x00\x0E\x08\x04\x00\x00\x00\x5E\x76\x5E"
							"\x59\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\x77\x49\x44\x41\x54\x18\xD3"
							"\x75\x90\x3B\x01\xC0\x20\x0C\x44\x4F\x02\x16\x90\x50\x0B\xC8\xA8"
							"\x85\x4A\xE8\xDA\x11\x09\x58\x40\x42\x57\x64\xC4\xCA\xEB\x40\x7F"
							"\x14\x9A\x6C\x79\xB9\x24\x17\xA9\x0B\x23\x30\xA3\x51\x04\x84\x7E"
							"\x20\x62\xC1\xFE\x50\x46\x92\xE1\x99\x48\x18\x85\x42\x22\x52\x55"
							"\x92\x0A\x1A\xA7\x64\x43\xE0\xB9\x67\x7F\x33\x57\xE4\x3A\xB0\xF1"
							"\xF2\xE1\x09\x37\x88\xCF\xD9\x19\x91\xB8\xB6\x2E\x34\x8F\x71\xD7"
							"\x6C\xAD\xEC\xAD\xD5\x48\x38\x0B\x5B\xAB\xAA\xB0\x76\x97\xD7\x83"
							"\x0E\xF0\x9F\x7B\xB8\x7B\x95\x61\x07\x00\x00\x00\x00\x49\x45\x4E"
							"\x44\xAE\x42\x60\x82", 197);
			getPNGImage(png);
		}
		else if (url == "playingart.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x0D\x00\x00\x00\x0E\x08\x04\x00\x00\x00\x5E\x76\x5E"
							"\x59\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\x77\x49\x44\x41\x54\x18\xD3"
							"\x75\x90\x41\x15\x80\x20\x10\x44\x27\x02\x15\x88\x60\x05\x62\x58"
							"\x81\x08\x5E\x3D\x12\x81\x0A\x46\xF0\x4A\x0C\xAA\x7C\x0F\x08\x28"
							"\xE2\xCE\x69\xF7\xBF\x79\xB3\xBB\x92\x56\x1C\x19\x7D\x6B\x45\x08"
							"\x37\x43\x19\x8F\xD0\x0C\x49\xC7\x3F\x2A\xAE\x40\x24\x91\xC8\x44"
							"\x16\xEC\x9D\xCE\x4C\x09\x49\x76\x8A\x32\x3D\x6B\x54\xA9\xFD\x03"
							"\x4C\x5F\x2B\xB4\xA1\xC3\x8E\x77\xFA\x96\x10\x11\xC7\x13\x9D\x6C"
							"\xD4\x3B\xCD\xFB\x71\x9E\xFD\x6E\x1D\x81\xE1\x61\x89\xE2\xAE\xE0"
							"\x02\xE8\xCB\x7B\xB8\xAB\x17\xF7\x85\x00\x00\x00\x00\x49\x45\x4E"
							"\x44\xAE\x42\x60\x82", 197);
			getPNGImage(png);
		}
		else if (url == "noadavail.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC1\x00\x00\x0E"
							"\xC1\x01\xB8\x91\x6B\xED\x00\x00\x02\x66\x49\x44\x41\x54\x38\xCB"
							"\x63\xF8\xFF\xFF\x3F\x03\x0A\x66\x60\xD0\x00\xE2\x66\x20\xDE\x0C"
							"\xC4\xAF\x81\xF8\x31\x10\xAF\x07\xE2\x1A\xB0\x1C\x9A\x7A\x74\xCD"
							"\x39\x40\xFC\x19\x2C\x8C\x1D\x7F\x07\xE2\x02\xEC\x06\x30\x30\x6C"
							"\x87\x29\xFC\xCA\xC0\xB0\x6E\x29\xBF\x40\xC7\x82\xBE\xCA\x9B\xAB"
							"\xA6\x57\x3F\x9C\x2F\x21\x32\xF9\x33\x03\xC3\x62\x24\x83\x40\x6A"
							"\x59\x10\x06\x40\x6C\x06\x73\xFF\x30\x30\xBC\x2D\xF3\x70\x4A\xCF"
							"\x2E\x4E\x9B\xB3\xFE\xF0\x82\x77\xDB\x4E\xCE\xFB\x94\x53\x9B\xBC"
							"\x36\x30\xDA\x2B\xF6\x14\x1F\x4F\x18\x50\xCD\x73\xA8\xDA\x12\x88"
							"\x01\x10\x3F\x83\x9D\xFD\x9B\x81\xE1\x3A\x88\x7E\xA8\xAD\xF2\x7A"
							"\xDA\xB2\xCE\x5B\xBB\xCF\x2C\xF9\x7A\xF0\xCA\xC2\x5F\x53\xD6\x36"
							"\xBF\xEA\x98\x5B\x71\x38\x3C\xCD\x2F\xEE\x36\x2B\x8B\x0F\x92\x77"
							"\x34\x18\xA0\x01\x06\x12\x58\x3E\x5B\x5E\xDE\xE9\x17\x2B\xCB\x4F"
							"\x10\x7F\x8F\x87\xE5\xD1\x45\xBB\xFA\x1F\x2E\x39\xD8\xF8\x21\xAD"
							"\x21\xEE\x40\xF5\xB4\xEC\x4B\xA1\x39\xBE\x13\x54\x0D\x55\x95\x80"
							"\xF2\xD3\xA1\x7A\x6A\x18\xA0\xA1\x0D\x72\x7A\xA0\x95\xAD\x95\xC6"
							"\x4E\x37\xAB\x43\x20\xFE\x5F\x46\xC6\x7F\x3B\x0B\xFD\xDE\x77\x6C"
							"\x8C\x7F\xA1\xED\xA8\x5F\xE6\x91\xE4\x98\x68\xE1\x67\x16\x2C\xAB"
							"\x25\x2F\x06\x94\x87\xB9\x62\x3D\x03\x34\xAA\x40\x1C\x19\x19\x45"
							"\x79\x7E\xC7\x70\xFB\xA4\x43\xD6\xCA\x30\xB1\xFF\xFB\xE2\x8C\xFE"
							"\xC4\x77\x07\x5F\x30\x0D\xB3\x6C\x10\x51\x92\x52\xE7\xE4\xE3\x66"
							"\x06\x8A\x8B\x40\xE5\x1F\xA3\x18\xA0\xA0\xA5\xCC\x24\xAD\x26\xC7"
							"\x2B\xAC\x2C\x6B\xB3\x5C\x51\xFC\x10\xCC\x90\x93\x1E\x1A\xBF\xAB"
							"\x16\x47\xBF\x0A\x28\x72\xE9\x53\x31\x51\x96\x43\x37\x00\xEE\x05"
							"\x03\x3B\x23\x59\xBF\x0C\xDF\xE4\x90\xC2\xC0\x34\x2D\x4B\x6D\xD3"
							"\xCD\xE9\xEE\x0F\xFF\x30\x31\x82\x0D\xB9\xAC\x2F\xF5\x25\xAD\x37"
							"\xF0\x90\x55\x98\xA9\x19\x30\xB0\x3D\x91\xBD\x00\x0B\xC4\xD9\x56"
							"\x1E\x56\x86\x89\x0D\xD1\x1B\x73\xA7\x26\x5D\x0B\x2F\x0F\x68\x6D"
							"\x5C\x59\x76\x77\x6E\xB1\xC3\xC7\x9F\xCC\x4C\xBF\x41\x6A\xEE\x49"
							"\xF1\xBF\x8C\xB7\x56\xB5\x00\xB2\xFB\x91\x03\x51\x03\x1A\x25\xFF"
							"\xCF\x71\x73\x06\x7B\xA5\xBA\x96\x24\x74\x84\x9C\x4C\xEB\x8B\xB9"
							"\xD1\xB0\xB6\xE8\x6B\xD9\xCA\xA4\x9F\x0D\xF9\x2E\xCF\x3F\x71\xB1"
							"\xFD\x02\xA9\xF9\xC5\xC0\x70\x15\x48\xFF\x46\x44\x23\x24\x21\x15"
							"\x40\x4D\x7C\xBE\x97\x8F\x2B\xDA\x22\xD0\x34\xCE\x3D\xCD\x79\x7E"
							"\xE6\xEC\xF8\x77\x59\x73\xE2\x3E\x7A\xE6\xBA\xAE\x2D\xB4\x51\x2B"
							"\x04\x3A\xFD\x09\x50\xCD\x3F\xD4\x84\x84\x25\x29\x7F\x02\x7A\x67"
							"\x36\x1F\x77\x5B\x52\x89\xFB\xF5\xF4\x4A\xEF\xFB\x93\xC5\x04\xBA"
							"\x3F\x30\x32\xCC\x80\xDA\x0C\x52\xB3\x1B\x35\x29\x43\x0C\x60\x01"
							"\x9B\x0A\xF5\x0E\x9E\xCC\x54\x02\xD3\x8C\x99\x1B\xC9\xC8\xCE\x00"
							"\x29\x75\xE7\x5B\x81\xE0\xCE\xEC\x00\x00\x00\x00\x49\x45\x4E\x44"
							"\xAE\x42\x60\x82", 692);
			getPNGImage(png);
		}
		else if (url == "adavail.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC2\x00\x00\x0E"
							"\xC2\x01\x15\x28\x4A\x80\x00\x00\x02\xF6\x49\x44\x41\x54\x38\xCB"
							"\x63\x60\xC0\x00\x8C\x2C\xEC\x6C\x1C\xEA\x2A\xCA\x1A\x39\x33\x57"
							"\x4D\x7C\xBA\x74\xEB\xE4\x37\xBA\xC6\xBA\x8D\x6C\xEC\x1C\x5A\x20"
							"\x49\x06\xFC\x80\x91\x95\x95\x85\x4D\x41\x48\x40\xD4\xC7\xDC\xDC"
							"\xBA\x77\xF5\xE1\x45\xDF\xB7\x9D\x5C\xF8\xC3\x3D\xD4\x7D\xA3\xA0"
							"\xA0\x70\x10\x0B\x2B\x9B\x12\x50\x11\x13\x56\xAD\x4C\x8C\x4C\x3C"
							"\xFC\xBC\x42\xF6\xF6\x0E\x8E\x5D\xB5\x2D\x15\xE7\x96\xEF\x98\xFD"
							"\xF1\xFC\xBD\x8D\xFF\x2E\x3E\x5C\xFB\x6F\xD9\xFE\x49\x3F\xCA\xDB"
							"\xF2\xAE\x5A\x39\x59\x4D\xE4\xE4\xE6\xD6\x01\x2A\x67\xC5\xB0\x9A"
							"\x9D\x95\x43\xC5\xC1\xD1\xA9\x65\xDD\xEE\x45\x1F\x4E\xDE\xDA\xF8"
							"\x77\xEA\xEA\xB6\xF7\xFB\x2E\x2C\xFB\x7B\xF2\xF6\xAA\x7F\xF3\x77"
							"\xF5\xFD\x3A\x7A\x6B\xF9\xBF\xC5\xBB\x26\x7C\xD6\x37\xD7\x2F\x62"
							"\x62\x62\x92\x44\xF3\x0E\x23\xB3\x88\x90\xB8\x5B\x65\x6D\xD9\xE1"
							"\x2B\x8F\x77\xFD\xEF\x9D\x53\xF3\x40\xCD\x56\xA7\x7E\xE3\xF1\x85"
							"\x5F\x76\x5E\x9A\xF6\xCB\x21\xC1\xFD\x58\xCB\x82\x92\xB7\xEB\xCE"
							"\xF6\xFC\x0F\x4E\xF5\xD9\x06\x74\x85\x39\x9A\x57\x18\x99\xA5\xA5"
							"\xE4\xC3\xBA\x27\xB7\xDC\xB8\xF8\x68\xFB\xFF\xEC\xAA\x94\xF3\x9C"
							"\x7C\xDC\x6A\x93\xB7\xD6\xBC\x99\xBA\x27\xF9\xBB\x94\x86\x42\xA3"
							"\xBA\xBD\x7A\x7D\x68\xB9\xCF\x6D\xC7\x24\x97\x73\xFC\xA2\xC2\xBE"
							"\x40\x4D\xCC\x28\x41\xC0\xCF\x27\x68\x97\x92\x9B\xB4\x7B\xFD\xB9"
							"\x89\xFF\xDB\xD7\xA7\x7E\x4D\xEF\xCD\xBE\x30\x75\x4F\xFD\xAF\xEE"
							"\x9D\xB1\xBF\x14\xCC\xB5\x26\xB2\xB0\xB3\xF2\x73\x0B\xF1\x6A\x81"
							"\x30\x33\x0B\x8B\x00\x66\xF8\x33\x32\x71\x8B\xCA\x4A\xC7\x07\x56"
							"\xFA\xDE\x6D\xD9\x96\xF4\x7F\xE6\xA1\xFC\xFF\x13\xF7\xE6\xFD\xEF"
							"\xD8\x99\xF2\xAF\x74\x69\xC2\x77\xBF\x1A\xBF\xDB\x52\x3A\x2A\x4D"
							"\xCC\x2C\xAC\x32\x78\x23\x12\xA8\x40\x41\x44\x5D\xB1\xC1\x26\xD5"
							"\xF9\x7C\xE3\xC6\x8C\x3F\xAD\xDB\x52\xFF\x35\x6D\x49\xFD\xD3\xBF"
							"\x2F\xFD\x5F\xDD\xC6\x94\xFF\xC6\x5E\xC6\xF3\x59\xD9\xD8\xD5\x31"
							"\xD2\x04\x1B\x3B\xBB\x98\xA0\x94\x88\x81\xB0\x8C\x98\x11\x3B\x17"
							"\x87\x30\x2B\x1B\xA7\x5E\xE7\x86\xEA\x8F\x4D\x1B\x93\x7E\x9B\xC5"
							"\x3A\x9E\x8A\x68\x0F\xBC\xD5\xB1\x2B\xFD\x5F\xCE\xE2\xB8\xBF\x92"
							"\xCA\xD2\x09\x28\x61\xC0\xC8\xC8\xC8\xAA\xA4\xAE\x96\x97\x33\x29"
							"\xFF\x7D\xC9\x9C\xA2\x2F\xDA\x96\xBA\xB9\x40\x41\xB6\xA6\xD5\xA5"
							"\x6F\xEA\xD6\x26\x7E\x17\x93\x97\x2E\xD4\xB4\x56\x49\x6C\xD8\x9C"
							"\xFA\xB7\x70\x65\xD2\x7F\x69\x55\x4C\x03\x58\x94\x54\x94\x53\xAB"
							"\xE6\x15\xBC\x9E\xB0\xBF\xFC\x3F\x30\xB4\x0F\x2B\xEA\x69\x64\x76"
							"\x6C\xA8\xFF\x5C\xB5\x36\xF1\xA7\xA6\x9B\xF1\x4A\xCF\x3C\xE7\x0D"
							"\x0D\x9B\xD3\xFF\xC5\x74\x86\xBE\x14\x96\x12\x0B\x41\x8F\x05\x46"
							"\x4E\x4E\x2E\x6D\x33\x6F\xCB\x59\x79\x0B\x93\x7E\x54\x6D\x48\xF9"
							"\x5F\x3C\x35\xEB\x45\xE7\xD6\x9A\xBF\x15\x6B\x92\xFF\xA5\x4C\x8B"
							"\xFD\x52\xB1\x2E\xED\x7F\xEE\x82\xC4\x5F\xDA\x2E\x7A\x3D\xAC\x6C"
							"\x6C\x8A\x18\x61\x00\x8C\x05\x5E\x1E\x3E\x7E\x47\x2D\x1B\x9D\xBE"
							"\x80\x52\xEF\x9B\x79\xF3\x53\x7E\xD4\x6D\xCC\xFE\x57\xBB\x01\xE8"
							"\x6F\xA0\xC6\x60\x60\xEC\x68\xD9\xEA\xF4\x73\x70\x71\x81\x32\x15"
							"\x1B\xCE\xCC\xC4\xC2\xCA\xAA\x0C\x34\xC8\x43\xD1\x40\xB5\x2F\x61"
							"\x7A\xF4\xF7\xB4\x99\x71\xDF\x0D\xBD\xCD\xD6\xF3\xF2\xF3\x7B\x03"
							"\xE5\x14\x88\xC8\x91\xC0\x9C\xC2\xC6\xAE\x29\x26\x2F\x99\x15\xDA"
							"\x1A\xF4\x34\xB6\x27\xFC\xB5\xBC\x8E\x72\x2D\x34\xEA\x30\x00\x00"
							"\xBB\x07\x15\x4B\xC6\x05\x9A\xAA\x00\x00\x00\x00\x49\x45\x4E\x44"
							"\xAE\x42\x60\x82", 836);
			getPNGImage(png);
		}
		else if (url == "adplayed.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC1\x00\x00\x0E"
							"\xC1\x01\xB8\x91\x6B\xED\x00\x00\x02\x50\x49\x44\x41\x54\x38\xCB"
							"\x63\x60\xA0\x22\x60\x04\x02\x39\x31\x71\x89\xB2\xA9\x4B\x3A\x5E"
							"\x29\xA8\x2A\xF4\x02\xF9\xF2\x20\x71\x62\x34\xB3\x00\x81\x86\x9A"
							"\x86\x6A\x59\x62\x56\xCC\xC1\x3D\xE7\x96\xFE\xCA\xAD\x4B\xBA\xA6"
							"\xA6\xA5\x5C\xCD\xCC\xCC\x4C\xD8\x10\x46\x06\x46\x31\x6D\x1D\xAD"
							"\xD6\x65\x9B\xA7\xBD\xE9\x5F\xDC\xF8\xFE\xF0\xD5\x15\x7F\xE7\xED"
							"\xE8\xF9\xB9\x60\x73\xD7\x3B\x79\x65\xD9\x76\x31\x31\xF1\x3A\xBC"
							"\xAE\x62\x61\x61\x35\xCA\x2F\xC9\x3C\x3B\x67\x55\xE7\x6B\x31\x05"
							"\xF1\xD2\x95\x87\xFA\xBF\x19\xB8\x19\xEF\xCB\x6B\x8D\x7B\xED\x13"
							"\xEE\x7E\x0F\xE8\xAA\xD3\x78\x5D\x05\x34\xC0\x38\xA7\x34\xE5\x42"
							"\xEF\xB2\xF2\xB7\x7A\x76\xFA\xF5\xDD\x9B\xB2\xBE\x0A\xCB\x89\x4E"
							"\x37\xB1\x36\xDE\xB4\x7C\xDB\x8C\xAF\x40\x57\x7D\x84\xB9\x6A\xE1"
							"\x96\xEE\x77\x4A\xEA\xF2\xE5\x40\x6D\xFC\xC8\x8E\xE0\x95\x56\x96"
							"\xAE\x2D\x9C\x1D\xF5\xA9\x67\x4B\xF6\xF7\x9E\x9D\x19\x7F\x83\x4B"
							"\xDD\xDE\x94\xD5\x17\xBC\x99\xB3\xB2\xF3\x9D\xA8\xBC\x78\x19\xC8"
							"\x55\x86\xEE\xA6\x7B\xEB\x66\x66\x3F\x77\x8F\x70\x38\xC4\xC4\xC4"
							"\xA4\x81\x6C\x00\x13\xD0\x6F\xDC\xAC\x5C\x9C\xFE\x3A\x1E\xFA\xDB"
							"\x1B\x37\xA4\xFE\x4E\x68\xF1\xFB\xD6\x3C\xA5\xEC\xDF\xC4\x35\x55"
							"\xBF\xF4\xEC\xF5\x27\x80\x5C\x25\xA2\x20\xDE\x2B\x2A\x2F\x1C\x2F"
							"\xA3\x2D\x5B\x0B\x34\x40\x01\xAE\x9B\x95\x8D\x4D\xD7\xC4\xDD\xB4"
							"\x5F\x56\x4D\x36\x91\x95\x9D\x55\xA9\x66\x59\xFA\x67\x59\x3D\xC5"
							"\x65\xCE\xB1\x36\x0F\xCB\x16\xC7\xFF\xEF\xDD\x92\xF5\x13\xE4\xAA"
							"\xE4\x9E\xA0\xE7\xAA\x36\xEA\x73\x98\x98\x99\x35\x51\xC2\x80\x93"
							"\x8B\xD3\xBD\x70\x6A\xC6\xDB\xA8\x2A\xBF\x4B\x02\xA2\x42\x71\xD5"
							"\xCB\xB3\xBF\xCA\xE8\x29\x2C\xF1\xCA\xB6\x3B\xE9\x96\xE1\xF4\x59"
							"\xD7\xCB\xF0\x04\xC8\x55\xC5\x0B\x63\xBE\x34\x6E\x48\xFA\x65\x1E"
							"\x60\xB8\x93\x91\x89\x49\x11\x39\x05\x49\xA9\x18\xAB\x4C\xCF\x9C"
							"\x11\xF9\x31\xAB\x3B\xF1\x75\xFD\xFA\xDC\xBF\xA1\x75\x7E\xDF\x13"
							"\x3B\x63\x7F\xEA\x3B\xEB\x6F\x07\xBA\xCA\x08\xE4\x2A\x39\x43\xA5"
							"\x85\x01\x45\x8E\x47\x0A\x96\xC7\xFF\x65\xE3\x64\x77\x42\x89\x49"
							"\x66\x16\x66\x2D\x29\x35\xC9\x2A\x87\x58\x9B\xA3\x79\x0B\x13\x7E"
							"\xD9\x84\x59\x3E\x2C\x9E\x9E\xF9\x2D\xB2\xCA\xEF\x3A\xD0\x55\xF1"
							"\x10\x57\x29\x2E\xF6\x2D\x70\x3C\x1E\xDB\x19\xFC\x8E\x95\x8D\xD5"
							"\x1A\x5B\x52\x96\xE5\x15\xE6\x2B\x09\x6E\xF0\x7B\x25\x2C\x23\x3A"
							"\x15\xE8\xAA\x45\xC8\xAE\x4A\x9A\x14\xF9\x35\x7D\x46\xD4\x27\x39"
							"\x5D\xD9\x2E\xA0\x7A\x61\x82\xC9\x1B\xDD\x55\xEE\xD9\xCE\xD7\xA4"
							"\xD5\xA5\xAA\x99\x98\x99\xE4\x40\x31\x47\x6C\x06\x83\xBB\x0A\x98"
							"\x1E\x3A\x81\x7C\x19\x6A\xE6\x60\x06\x00\x9A\x6E\xEF\x92\x09\x7E"
							"\x22\xAD\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 670);
			getPNGImage(png);
		}
		else if (url == "v1.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x01\x81\x49\x44\x41\x54\x38\xCB"
							"\xA5\x93\x41\x4B\x02\x51\x14\x85\xFD\x29\xFD\x8A\x16\x2D\x5A\xB4"
							"\x68\xD1\x22\x08\x5A\xB4\x94\xA0\x7F\x20\xB4\x6C\x11\x11\xC1\x14"
							"\x42\x84\x60\x44\x66\xB5\x08\x22\x08\xC1\xC1\xC4\xAC\x4D\x61\x46"
							"\x12\x8E\x9A\x49\x9A\x59\x56\x9A\xCE\x28\x64\x24\xC4\x69\xCE\x90"
							"\xC3\x3C\x67\x5C\x75\xE1\x83\xC7\xB9\xE7\x1E\xDE\x7B\xF3\xC6\xE5"
							"\xEA\x2B\x49\x92\xDC\x3A\x71\x1D\xF4\x41\xCD\xED\x1A\x54\x7A\x73"
							"\x48\x27\xE5\xF7\xFB\xA1\x28\x0A\x5A\xAD\x16\x7A\xC5\x35\x35\xF6"
							"\xE8\xA1\xD7\x69\x58\x95\x65\xD9\x18\x78\x6C\x76\x11\xBC\x56\xB1"
							"\x18\x7D\x37\xE0\x9A\x1A\x8B\x1E\x7A\x85\x10\xA6\xF6\x86\x03\xC9"
							"\x26\x66\x76\xCB\x8E\xB0\x67\x09\x49\x99\x67\xE6\xD6\x58\xDB\x89"
							"\x06\xA6\x03\x25\x93\x83\x94\x8A\xD3\xFB\xB6\xA0\xD1\xC3\xFA\x3B"
							"\x8E\x9B\x01\x71\x9E\xAF\xD8\xF8\xC6\xD4\xD6\x83\x49\x48\xD1\xD0"
							"\xE9\xFE\x20\xA6\x07\x58\x75\x42\x2F\x67\x38\xCB\x00\xE3\x92\x36"
							"\x2F\xEA\x98\xF4\x17\x4C\x66\xF7\x8B\x08\x67\x34\x44\xEF\x34\x41"
							"\x27\xF4\x72\x86\xB3\x46\x00\x6B\xFE\xB8\x82\x09\x5F\x5E\x60\x2F"
							"\xF9\x81\x93\x9C\x66\xD3\xE9\x65\x09\x01\x9E\xA3\x32\xC6\xD7\x73"
							"\x02\xC1\x44\x1D\x91\xAC\x6A\xD3\xE9\x15\x02\xB8\x9D\x8D\xF3\x57"
							"\x8C\x79\x33\x02\x81\xCB\x1A\x64\xA5\x69\xD3\xE9\xB5\x1E\xC1\xB8"
							"\xC4\x42\xED\x0B\xA3\xAB\x69\x13\xCF\x61\x09\x57\xA5\x36\xB2\xD5"
							"\x4F\x2C\x84\xCA\x42\x8F\x5E\xEB\x25\x9A\x9F\xD1\x1B\x7B\xC1\xC8"
							"\xCA\xAD\xC1\xB2\xFC\x04\x39\xDD\x44\x38\xDD\x80\xEF\xAC\x6A\xEA"
							"\xF4\x08\x9F\xB1\xFF\x21\xAD\x45\x9F\x31\xBC\x74\xE3\x08\x7B\xB6"
							"\x87\xE4\xF4\x94\xF3\x6F\x1D\x48\x91\x0A\xE6\x76\xF2\x06\x5C\x53"
							"\x1B\xF8\x94\xFF\xFD\x33\xFD\xE7\x77\xFE\x05\xEF\x5F\x9A\xB1\x51"
							"\x9D\x7F\x55\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 463);
			getPNGImage(png);
		}
		else if (url == "v2.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x01\xA6\x49\x44\x41\x54\x38\xCB"
							"\xA5\x93\xBD\x2F\x83\x51\x14\xC6\xFB\xA7\xF8\x2B\x0C\x06\x83\xC1"
							"\x60\x90\x48\x0C\x6C\x5D\x98\x8C\x84\x58\x31\x54\x5E\xD5\x44\x9A"
							"\x26\x15\x14\x11\xB1\xF8\x88\x44\x95\x52\x22\x48\x51\x8D\xB4\x48"
							"\x35\xF4\x4B\x5B\x6D\xD3\x56\x69\x4A\x0D\x7D\xBC\xCF\x8D\xDE\xF4"
							"\xAD\x9A\x9C\xE4\x97\xF7\xDE\x73\x9E\x73\x72\xCF\x7D\xCF\xD5\xE9"
							"\xEA\x4C\x51\x14\xBD\x8A\x4B\x05\x75\xD0\xA7\xD7\xFD\x65\x6A\xB0"
							"\x49\xC5\x6B\xB5\x5A\xE1\xF7\xFB\x51\x28\x14\x50\x35\xAE\xE9\x63"
							"\x8C\x1A\x6A\x1B\x25\xE7\xED\x76\xBB\x48\x08\xE7\xBE\xB0\x74\x95"
							"\xC7\xD8\x7E\x4A\xC0\x35\x7D\x34\x6A\xA8\xD5\x14\x61\xD5\x6A\xB2"
							"\xED\x32\x87\x9E\xE5\x48\x43\x18\xAB\x29\xE2\x95\x3D\xF3\x68\xB4"
							"\x05\x77\x16\xDD\xB6\x90\xE0\xF0\xE1\x4D\xF8\x2A\x95\x0A\x7C\x89"
							"\x92\xF4\x53\x43\xFB\x69\x47\xCF\x02\x2E\xF6\xF7\x94\x2D\xA3\x6B"
							"\xEE\x51\x70\x1E\x2A\xC2\x7C\x92\x86\x7E\x25\x2C\x18\x77\x24\x11"
							"\xCC\x7C\xCA\x38\xB5\xCC\x61\x2E\x0B\x88\x4B\x9A\x3D\xCB\xA0\xD3"
							"\x1A\x14\xF4\xAF\x86\xD0\x6B\x7B\x94\xFB\xD1\xED\x18\x92\x85\xB2"
							"\xDC\x53\xCB\x1C\xE6\x8A\x02\xB4\xE1\xAD\x18\x3A\x2C\x01\x0D\x8A"
							"\x33\x01\x4F\xB4\x88\xDD\xBB\x57\x0C\x6D\x44\xA5\x9F\x5A\x9A\xA6"
							"\xC0\xE0\x7A\x04\xED\x33\xF7\x92\xE9\x83\x04\x96\xDC\x19\x18\xF6"
							"\xE2\x18\xD9\xD4\xC6\xA8\xD5\x14\xE0\x71\xCC\xC7\x49\xB4\x99\x6E"
							"\x05\x46\x67\x1C\xF3\xA7\x29\x0C\xAC\x3D\x49\x5F\x2D\xD4\xD6\xB6"
							"\x20\x2E\x31\x98\xFE\x40\xEB\x94\x4F\xE0\x89\xBC\xC3\x79\x9F\x87"
							"\xF9\x28\xA9\x92\x10\x4C\x3A\x9E\x65\x9C\xDA\xDA\x4B\x94\xBF\xD1"
							"\x74\x10\x47\x8B\xE1\x06\x16\x35\x61\xC7\x97\x85\xDD\x97\x13\x5F"
							"\xB2\x7A\x91\x16\x31\x6A\x34\xBF\xB1\x7E\x90\x8C\xFB\xCF\x68\x9E"
							"\xB8\x6E\x08\x63\xBF\x06\xA9\xD1\x28\x07\x5E\x4A\x50\x1C\x31\xF4"
							"\x2D\x06\x04\x5C\xD3\xF7\xE7\x28\xFF\xFB\x31\xFD\xE7\x39\x7F\x03"
							"\x7B\x97\xA2\x69\xD2\xB0\x90\x85\x00\x00\x00\x00\x49\x45\x4E\x44"
							"\xAE\x42\x60\x82", 500);
			getPNGImage(png);
		}
		else if (url == "relay.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x01\xAB\x49\x44\x41\x54\x38\xCB"
							"\xA5\x93\xCB\x4B\x02\x51\x14\xC6\xFD\x53\xFA\x2B\x5A\xB4\x68\xD1"
							"\xA2\x45\x8B\x20\x68\x51\x3B\x5B\x04\xAD\x5A\x15\x41\xDB\x1E\x14"
							"\x58\x08\x11\x81\x15\xBD\x89\x36\x45\x44\x98\x45\x58\x10\x45\x0F"
							"\x33\x6B\x7A\x88\x54\xEA\x68\x9A\xA2\x26\x9A\x59\x0B\xBF\xE6\x3B"
							"\xE4\xA0\x36\xAE\x3C\xF0\x63\xCE\xE3\x3B\x87\x7B\xEF\xDC\x6B\x32"
							"\x55\x98\xC5\x62\x31\x6B\x38\x35\x50\x01\x73\x66\x53\x35\xD3\x8A"
							"\x75\x1A\x6E\x9B\xCD\x06\x45\x51\x90\x4E\xA7\x51\x34\xFA\xCC\xB1"
							"\x46\x0D\xB5\x46\xCD\x29\xBB\xDD\x2E\x0D\xFE\xE4\x0F\x96\xAF\x52"
							"\x18\x3A\x78\x17\xE8\x33\x47\xA3\x86\xDA\xB2\x21\x9C\x5A\x6C\x5E"
							"\xBC\x4C\xA2\x63\x25\x60\x08\x6B\x25\x43\xDC\xFA\x9E\xB9\x34\xDA"
							"\xC2\x79\x02\xED\x8B\xAF\x50\x22\x39\x14\x0A\x05\xC9\xF1\x7B\xF7"
							"\x96\x93\x3C\xA1\x86\xF6\xB7\x1D\x33\x07\x38\xB9\xBF\x97\xC4\x37"
							"\xDA\xE6\x9F\x05\x36\xCC\x9D\xC5\x61\x5E\xF3\x0B\xC3\x8E\x08\x7C"
							"\xF1\xBC\x5E\xA7\x96\x3D\xEC\xE5\x00\x39\xA4\xD9\xD3\x38\x5A\x6D"
							"\x3E\xE1\x36\xFC\x09\xAB\x33\xAA\xC7\x83\x3B\x2A\x22\xE9\x6F\x3D"
							"\xA6\x96\x3D\xEC\x95\x01\xB4\x81\x6D\x15\x2D\x33\x5E\xC1\x13\xFA"
							"\x84\x3F\x91\x87\x2B\x98\x15\xF6\x1E\x3E\xD0\xBF\x15\xD4\xEB\xD4"
							"\xD2\xCA\x06\xF4\x6D\x06\xD0\x3C\xF5\x28\xDC\xA8\x59\xEC\x2A\x29"
							"\x8C\xEF\x87\xE5\xCB\xB8\x58\x23\xD4\x96\x0D\xE0\x72\xA6\x8F\x23"
							"\x68\xB2\xDE\x0B\xD7\xC1\x0C\xC6\x1C\x21\xF1\xBB\x96\xB5\x25\x9F"
							"\x44\x85\x62\x9D\xDA\xD2\x2D\xC8\x21\xFA\x62\x5F\x68\x9C\xB8\x13"
							"\x5C\x81\x0C\x46\xED\xAA\x1E\xF7\x6E\x3C\x23\x9C\xCA\xEB\x31\xB5"
							"\xA5\x87\xA8\xFF\x46\xEB\x61\x18\x0D\xE3\x1E\xCC\x1C\xBD\xA1\x67"
							"\xD5\x27\x3E\xE9\x9C\x7D\xC2\xFA\x45\x4C\x7C\x6A\xCA\x7E\x63\xE5"
							"\x45\x9A\x3C\x08\xA1\x7E\xE4\xDA\x10\xD6\xFE\x5D\x24\xA3\xAB\xEC"
							"\x8D\xE6\x60\x71\xA8\xE8\x5E\xF2\x0A\xF4\x99\xAB\x7A\x95\x6B\x7E"
							"\x4C\xB5\x3C\xE7\x5F\xCE\xEF\xA6\xA0\xA0\x37\x1F\x5A\x00\x00\x00"
							"\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 505);
			getPNGImage(png);
		}
		else if (url == "wa.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x02\x0D\x49\x44\x41\x54\x38\xCB"
							"\x8D\x93\x4F\x68\x92\x71\x18\xC7\x35\xA3\xFF\x86\x5B\x5B\x32\x61"
							"\x0D\xC2\xEE\xC5\x08\x22\x0F\x8D\xB5\x0E\x12\x81\x04\xEE\xE0\x41"
							"\x21\xA2\xD8\x24\x2A\x56\xB1\x82\xC4\x0A\x06\xC5\xD0\x75\xD8\xA2"
							"\x4D\x66\x10\x48\xB0\xB6\x5D\x94\x1D\xB2\x74\x68\x2D\xDD\x41\x93"
							"\x2E\xC1\xCA\x2D\x8D\x6D\x81\x98\x22\x25\x82\x7C\xF2\x7D\xB7\x44"
							"\x97\x6E\xFB\xC0\x73\x7A\xDF\xCF\xF3\xFE\x9E\xEF\xEF\x79\x25\x92"
							"\xCD\x61\xBD\x2A\x51\x94\xCA\x2E\xD9\x06\x2C\x7C\x5D\x20\x10\x0C"
							"\x54\x36\xD1\xE9\xF5\xFA\xB4\xF1\x62\x27\x5B\xCA\x8B\x4B\x8B\x08"
							"\x24\x93\x49\xDC\x6E\x37\x6A\xB5\x3A\xEE\x9D\x70\x92\x9B\x3C\x8F"
							"\xF5\xB2\x62\xD3\x06\x24\x12\x09\xFE\xD1\x77\xAB\x0F\xAB\xD5\xCA"
							"\xCF\x99\x07\xA4\x1F\xB7\x10\x19\x6B\x46\xD9\x28\x9B\xDE\x52\xF6"
							"\x78\x3C\x18\x8D\x46\xE2\x9F\x67\x29\x4C\x74\xB1\x7A\x4D\x46\xCA"
							"\x21\xE7\xA8\x72\x47\x7A\x3D\x87\xFA\xB2\xD9\x6C\xC6\xE9\x74\xC2"
							"\x97\x61\x0A\xE3\x2D\xAC\xF6\x4A\x49\x0D\xEE\xE4\xF8\x11\x29\x2E"
							"\x97\xAB\x56\xB0\x6B\x33\x0B\xB3\x5A\x2C\x16\xD2\x3F\x62\x10\xBC"
							"\x00\x6F\x4E\xF2\xC7\x75\x82\xE2\x5B\x3D\xF1\x29\x3D\x5E\x9B\x96"
							"\xAB\xDD\x67\x09\x85\x43\x55\x4D\xC4\xB4\x05\x6E\x9B\xB4\x64\x46"
							"\x3A\xC9\x3C\x3C\x44\xC6\x56\x12\x7F\x2D\x81\xBF\x1D\x66\x0E\x93"
							"\x1B\xDA\x4F\x7E\x40\xCA\x3D\xD3\x69\xF1\x5D\x9F\xCF\x57\x6E\x22"
							"\x5E\x95\xC0\xDD\x9E\x8E\xD2\x13\x15\x84\xBA\xE0\xF7\x77\x98\x3F"
							"\x47\xE1\x75\x33\xB9\xC1\x3D\x14\x47\x77\xC1\xF8\x6E\x9E\x3D\xB9"
							"\xB3\xD6\xC0\xEF\xAB\x3E\xC5\xDC\xC7\x39\x9E\xDA\x1E\x71\xE3\xD2"
							"\x29\xC8\x97\xB2\xF8\xA4\x25\x37\xDA\x40\x7E\x64\x1F\xBC\x92\xE3"
							"\xBB\xBF\x17\xDB\x40\xBF\x28\x87\xE7\xC3\xB5\x73\x30\x18\x0C\xA5"
							"\xF9\xDF\x93\x77\xB7\x93\x1D\x92\x53\x9C\x6A\x24\xEE\x50\x70\xBD"
							"\xFB\x18\xC1\x60\x50\x94\xA3\xD1\x68\x4D\x59\xA1\xD1\x68\x22\xA9"
							"\x6F\x01\xB2\x63\x6D\xE4\x5D\x4D\xF0\x4E\x85\xBD\xF7\x20\x2F\x5F"
							"\x3C\x2F\xEF\x44\x2C\x16\xAB\x29\x4B\x04\x79\xE5\xC3\x34\x59\x47"
							"\x1B\xC5\xD9\x56\x7C\xC3\x4A\x7A\x4C\x67\xF0\xFB\xFD\x65\xB9\xDE"
							"\x97\x45\xFA\x75\x6A\xF2\x93\x4A\xD2\xDE\x56\xAE\xE8\x0E\x08\x8B"
							"\x42\x28\x14\x62\x79\x65\x59\x94\x37\x5E\xDB\x7F\x08\xBB\x6D\xBF"
							"\xD9\x80\xAA\x49\xE6\xAC\xD8\x32\xF1\xAA\x36\xA6\x5D\x8F\x48\xA9"
							"\x3A\xB6\xF9\x3B\x57\xF1\x17\x81\x38\xA3\x74\x72\x00\xB9\xAF\x00"
							"\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 603);
			getPNGImage(png);
		}
		else if (url == "curl.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\x4E\x49\x44\x41\x54\x38\xCB"
							"\x63\x60\xA0\x0F\x68\xF8\x7F\xE0\xC0\x81\xFF\x98\x6C\xDA\x18\xD0"
							"\xF0\x1F\x53\x11\xD1\x06\x40\x24\x61\x98\x2C\x03\x28\xF2\x02\x44"
							"\x02\xBB\x17\x90\x31\xDE\x30\xC0\xE6\x05\x64\x31\x4C\xEF\x51\x3B"
							"\xBE\x09\xC7\x02\x59\xF1\x4D\x4F\x03\x06\xAF\x17\x88\x4A\x07\xC8"
							"\x71\x8C\x8D\x8D\x2D\x1D\x00\x00\x6B\x22\xE2\xA4\xD5\xBE\x1F\x4C"
							"\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 156);
			getPNGImage(png);
		}
		else if (url == "radionomy.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\xAB\x49\x44\x41\x54\x38\xCB"
							"\x63\x60\x00\x02\xB1\xAE\x33\xAC\x40\x5C\x0A\xC4\x37\x81\xF8\x1F"
							"\x10\xFF\xC7\x81\xFF\x41\xD5\x80\xD4\xB2\x32\x20\x69\xDE\x81\x47"
							"\x13\x2E\xBC\x03\x6C\x08\xD4\x34\x90\xC0\x3B\x20\x0E\x03\x62\x2E"
							"\x06\x1C\x00\x24\x07\x55\xF3\x0E\xAA\xA7\x94\x01\xEA\x24\x10\x27"
							"\x8C\x81\x48\x00\x35\x04\xA4\xE7\x26\x03\x92\x9F\xF1\xD9\xCC\x8A"
							"\xC5\x25\xE0\x30\x61\x80\xF9\x09\x8B\xA6\xC9\x40\x7C\x0B\x88\x7B"
							"\x81\xF8\x2F\x10\x37\xA0\xC9\x43\xF4\xE1\x31\x60\x03\x54\xEE\x2E"
							"\x10\xD7\x03\xB1\x13\xB9\x06\xD8\xE3\xF0\x16\xD1\x06\x88\x50\x6A"
							"\x80\x00\xCD\x0D\xC0\x1A\x8D\x40\x7E\x22\x10\x4F\x05\x62\x36\x1C"
							"\x09\x0A\x1E\x8D\x14\x27\x24\x8A\x93\x32\xB9\x99\x69\x27\x7A\x8E"
							"\x24\x2B\x3B\x03\x00\xC3\xBC\x4D\xD5\xF9\xC1\xBC\x7C\x00\x00\x00"
							"\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 249);
			getPNGImage(png);
		}
		else if (url == "chrome.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x02\xE8\x49\x44\x41\x54\x78\x01\x65\x93\xDD\x6B\xD6"
							"\x65\x18\xC7\x3F\xD7\xFD\xB2\xFD\x9E\x47\x63\xB3\x74\x4D\xDB\xF2"
							"\x99\x21\xC5\x93\x16\x19\x61\x27\xD1\x36\xF1\xA0\x37\x09\x02\xE7"
							"\xEA\xC4\x88\x75\x24\x74\xE2\x69\x41\x45\x7F\xC1\x3C\x1B\x42\x45"
							"\x50\x9E\x14\x41\x74\x12\x54\xD0\x91\x06\xE9\x81\x33\x59\xE5\x66"
							"\x9B\x3A\x6D\xDB\xE3\xDA\xCB\xF3\x7B\xBB\xEF\xAB\x1F\x13\x65\xD5"
							"\x07\x3E\x47\xF7\x75\x7F\xBF\x07\xF7\x7D\x09\xFF\xE1\xAF\x63\x2F"
							"\x8A\x49\x3A\xFB\x4D\xAD\xE3\x29\x31\x66\xB7\x46\x25\xA6\xD9\xD5"
							"\xB8\x96\x9E\x0F\xED\x6C\x16\x50\x36\xE1\xE6\x5F\x19\xE6\x2E\xB1"
							"\x9D\xF7\xC6\xB5\x6C\x0C\x18\x01\x06\xC4\x99\x44\x43\x24\xA6\x79"
							"\x1A\xB3\x62\xBA\x3A\x3F\x13\xD7\xF3\x09\x60\xFE\x5E\x40\xB9\xB8"
							"\x0A\x80\xA6\x45\x53\x6A\xFE\x14\x30\xA8\x31\x4A\x35\x8C\x18\x41"
							"\xA3\xA2\x45\xA8\x57\x01\x8F\x57\x41\xEF\x57\x0E\x69\xBB\x38\x01"
							"\x5C\x02\x70\xA1\xB5\xC6\xD9\x47\x7D\xEF\xD3\xBF\x15\xE3\x3E\xC6"
							"\xA1\x6A\x98\xE8\xEF\x5C\x46\x75\x43\x2D\xA9\x0C\x68\x16\x24\xE6"
							"\xE5\x90\xE6\x61\x3C\xA6\xF2\x06\x30\xEF\xCA\x56\xE4\xAB\xD1\x1D"
							"\x63\xEE\xB3\x5B\xC3\x07\x26\x33\xA4\xA3\x04\x03\x72\xFF\x36\x42"
							"\x7F\x3F\x78\xB0\xE9\x34\xE4\xB7\x37\xC2\xAC\x46\xC4\xE8\xB0\xEB"
							"\x29\xC7\x80\x0F\xDD\x3B\xA7\x1B\x0F\x47\x2F\x47\x7F\x38\xD4\x45"
							"\xDF\xD4\x75\xB6\xB7\x03\x3C\xF9\x04\x53\xAF\xBD\xC9\xCC\x03\x0D"
							"\x30\xC2\x9E\xCE\x3F\xD8\xEF\xC6\xD9\xC2\x39\xAC\x57\xAC\x8B\x18"
							"\xA3\x47\xC5\xF1\x89\xBC\xF4\xCD\x0B\x47\xC4\xF2\xB9\x51\xAD\x1F"
							"\xFA\xF4\x1A\x43\x17\x1D\x7F\x9E\xFC\x88\xAF\x69\x40\x54\x00\x04"
							"\x61\xA4\x39\xC5\xF3\xF7\x9D\x80\xB0\x00\x22\x68\x94\x75\x8D\x76"
							"\xD4\x10\xB4\x81\x92\x44\x67\xF8\x79\xB8\x9B\xB9\x67\xF6\x72\xD6"
							"\xEE\x62\xEE\xD6\x0A\x8B\x7F\xB7\x59\xAA\x9C\x5D\x58\xE1\xA7\xB9"
							"\x06\xA5\x6F\x02\x01\x94\x4A\x49\x62\x30\x0D\x17\x03\x18\x5B\x19"
							"\x95\x56\x5F\x8D\x8B\x87\x7B\x58\x49\x0B\x16\x56\x53\x12\x6F\x01"
							"\x48\x8B\xC0\xF2\x9A\xB2\x19\x8D\x86\x50\x5A\x9C\x16\x3A\xA3\x86"
							"\x54\xAD\xD4\xC5\x18\x26\x77\x2E\xB1\x2F\x2C\x53\x4E\x15\xDC\x5C"
							"\x2D\x01\xA8\x77\x78\x0E\xEE\xBC\x8E\x2F\x2F\xA3\x58\x54\x85\x50"
							"\xBA\x34\xE4\x7E\xC6\xC5\x54\x2F\x00\x57\xC4\xCA\x3E\xE3\x85\xC5"
							"\x6C\x89\x1B\x5B\xBF\xE5\xC8\xC1\xE7\xB8\xF4\xBB\x05\x60\xFF\xC0"
							"\x2A\xCF\xEE\x38\x0D\xE5\x4D\xC0\x12\x4B\x47\x91\xFA\x2B\x45\xE6"
							"\x2F\xC8\xE2\x77\x7D\x1C\xBB\xD6\x7C\xD7\xD6\xE5\x03\x5B\x37\x88"
							"\x03\x50\xB6\x75\x74\xD3\xE3\x76\x91\xA5\xB0\x27\xFC\xC2\xDB\xBD"
							"\xE7\x48\x0C\x54\xCD\x64\x6B\x09\x59\xBB\xF3\xBD\x8D\x67\x8C\x51"
							"\x28\xDB\x71\x22\x06\x19\x24\x32\x6C\x6B\x82\x78\xA1\x95\xDD\x66"
							"\x21\x6D\x51\xCF\x32\xDE\xEA\xF9\x95\x44\x84\xAA\x91\x6C\x3D\x21"
							"\x5D\xED\xFC\xBE\xC8\xDD\x04\x80\xCC\x7E\xF9\x08\x00\xAF\xCF\x35"
							"\x9A\xD6\xCB\x29\x57\x33\x83\xAE\x26\x62\xBC\x10\x81\x97\x6B\xF3"
							"\x1C\xEF\x9A\x23\x14\x96\x74\xBD\x43\xF3\xB6\xFF\xB1\xCC\xED\xBD"
							"\xAF\x2C\xD3\x5F\xEC\xE5\x2E\xA3\xB3\x7D\xBD\xD6\xC9\x58\x15\x34"
							"\x22\x4E\x06\x1E\xF2\x45\x72\xB2\xFB\x06\x0F\x4A\x91\xE6\x85\x9D"
							"\x0E\x85\x39\x53\x96\xE6\x5F\xCB\x24\x97\x3F\x7E\x8C\xCD\xBC\x3A"
							"\xB9\x5D\xB6\x74\xB9\x3E\xB5\x72\xE0\x70\x7D\x7D\xF7\xF1\xAD\xCB"
							"\xC4\x28\x57\x43\x90\xF3\x2B\x2D\xF7\xBF\x75\xFE\x07\xC1\xB8\x73"
							"\x8C\xE9\x85\xD9\x6A\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60"
							"\x82", 801);
			getPNGImage(png);
		}
		else if (url == "firefox.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x03\x3C\x49\x44\x41\x54\x38\x11\x05\xC1\x4B\x68\x5C"
							"\x55\x00\x80\xE1\xFF\xDC\x73\xE7\x95\x8C\x79\x4D\x26\x0E\x69\x4C"
							"\x82\x8D\x49\x93\x2A\x41\x25\x89\x31\xE8\x46\x45\x22\x6E\x04\x11"
							"\x45\x51\xD0\x8D\x2B\x17\x82\x10\x50\xF7\x2E\x45\x5D\xB8\x11\x5C"
							"\xB6\x14\x2A\x5A\xAA\x3B\x43\x85\x28\xA8\x2D\x5A\x34\xE6\xD1\x4C"
							"\x26\xCE\x8C\x93\xC7\xDC\xCC\xEB\xCE\xDC\xD7\x39\xF7\xF8\x7D\x02"
							"\x00\x00\x00\xE0\xAB\xBD\x50\x96\x4E\xBD\x59\x3F\x8A\x57\x0D\x3C"
							"\x08\x18\x01\x07\x29\xDB\xFA\x25\x9B\xCF\xDE\xDB\xB8\x24\x35\x00"
							"\x00\x80\x00\xA8\x7F\x36\x67\xE5\xA2\x5D\xF1\xFE\xFC\xF1\x78\xAB"
							"\xE3\xBF\xD7\x16\xF6\xCB\x76\x52\x5E\xC8\x24\xAC\x84\x40\x10\x46"
							"\x2A\x4A\xC4\xAA\xBC\x38\x96\xB8\x92\x33\x9D\x2F\xE7\x9E\x9A\xAD"
							"\x2C\x83\x01\x90\x00\xEF\x2E\x0C\x3E\xBA\x15\xAE\xBF\xBE\xE9\xCE"
							"\x6E\x9C\x38\xCD\xD7\x72\xF6\xF6\x50\x26\xD1\x92\x75\x9D\xC7\xD3"
							"\x31\x75\x3F\x94\xAD\x86\x3B\x9C\xE9\x36\xD6\x96\x46\x1B\xCF\x27"
							"\x9E\xCC\xBB\x1F\x17\xB6\xFE\xFE\xF4\x36\xB1\x04\x78\x65\x7C\xEC"
							"\x4D\xD1\xF3\x3F\x99\x32\x3B\x53\x2B\xD9\x9F\xC5\xCA\xC8\x6F\xDC"
							"\x74\x57\x29\x47\x03\xB4\x83\x80\x66\xC7\x23\xAA\xB7\xB1\x3A\x81"
							"\x58\x9B\x93\x63\x13\xF9\xF0\x99\xB0\x20\xF7\x3E\xBF\xB2\xBB\x6D"
							"\x1B\x73\x59\xFC\xF9\x86\x9E\x1E\xC9\x34\xC4\x14\x0D\x92\x96\xA1"
							"\x56\x1B\xE4\x89\xC6\x2D\xF6\xFA\x2B\xDC\xB5\x2F\x23\x23\x8D\xE8"
							"\xFA\x94\xDB\x5D\xF6\xCA\x43\xAC\x3C\x9D\x1C\x08\xC7\xE5\xDB\xE6"
							"\x9C\x1F\xE4\x5B\xCD\xDC\xBC\x34\xE6\x43\x23\xC4\x08\xB6\x05\x29"
							"\x8B\x84\x0E\x99\x0C\xE0\x54\x8D\x51\x6E\x0F\x60\x39\x2D\x8C\x73"
							"\x4E\xE8\x76\xE9\x35\x2B\x3C\xB7\x74\x4E\x7F\xA6\x9A\x08\xEB\x07"
							"\x37\xEC\xC8\x63\x45\x79\x4C\x8A\x84\xC1\x28\xD0\xAE\x40\x0F\xDB"
							"\x5C\xF5\xE6\xB9\xD5\x9A\xC0\x68\x07\xD5\x73\x51\x6E\x93\x38\xF4"
							"\x99\x5A\x2C\xA0\xA3\x16\xBD\x60\xBA\xB0\x73\xFD\xE2\xBA\xED\xB9"
							"\x0C\x11\x62\x27\x00\xE5\x41\xE3\x3F\xF8\x46\x2F\xF0\x63\xD4\x4F"
							"\x37\x3E\x86\x58\x83\xD7\x80\xA0\x83\xF6\xBA\x3C\x94\x77\x18\xEE"
							"\x3F\xA1\x78\x98\x4D\xEF\xB7\x06\xA7\xEC\xC0\x23\x24\x12\xB1\x25"
							"\x8C\x15\x6B\x43\x9F\x84\x4B\x69\x87\xED\xB3\x32\xD9\xAC\x66\x3A"
							"\xDF\xE5\x76\xA9\x9F\xC3\x33\x8F\xA4\xE9\x72\x9F\xEF\xA0\x3A\x4D"
							"\x32\x7E\xA4\x17\x26\xED\xA2\x1D\xF8\xFC\x6A\x42\xEA\x16\x14\xD2"
							"\x7D\x60\xA7\x0C\x6B\xE9\x0A\x0F\x4F\x9E\x12\xE9\x98\x83\x76\x92"
							"\xFD\x68\x02\xD5\x4B\xF1\xC8\xCC\x39\x8F\x3F\x70\x42\xF7\xD8\xC7"
							"\x2A\xC9\x5A\xAE\x19\xFF\x64\x1B\x6D\xFE\xF1\x3C\x71\xC7\xC4\xE2"
							"\x05\x69\x19\x92\x02\x02\x0D\xBF\x57\x24\x9B\xD5\x01\xEE\x76\xB3"
							"\x04\x96\x26\x97\x6A\xB0\x3E\xDF\x24\xD5\xD1\x9C\x15\xD3\x54\xFF"
							"\x92\x55\x59\x8B\x4B\xF2\xEB\x03\x27\x7A\xF5\xFE\x9C\x1F\x45\xBC"
							"\x88\x10\x49\x29\x20\x29\x21\x3F\x18\x93\xEE\x83\xA1\x74\xC8\x6A"
							"\xC1\xE5\x9D\xC7\xDA\x2C\x8F\x87\x84\x65\xC1\xBF\xBB\x36\xC5\x43"
							"\x79\xFD\xA5\xDC\xD1\x77\x12\xE0\xD9\xDC\x60\x31\x0E\xC4\x68\xAC"
							"\xC4\x92\x41\x08\x61\x04\x69\x11\x73\x31\x1B\xB2\x9C\xF7\x59\x1C"
							"\x0D\xC8\xA5\x34\xAA\x09\x5E\xDD\xC4\x61\xCB\x7C\x9F\x15\xEA\xA3"
							"\x99\x1B\xB5\xA6\x04\xF8\xF6\xB4\x61\x9C\x40\xED\x0F\x18\x7B\x2C"
							"\xA9\xAC\x59\xB4\xB0\xB5\x16\x28\x25\x88\x43\x81\xF2\xC1\xEB\x08"
							"\x4A\x27\xC2\xEC\x54\xC3\x9B\xD7\x8A\xCE\x07\x1B\x7F\x54\x8A\x00"
							"\x12\x00\xA0\x14\x04\xC1\x1D\xB7\xBB\xAD\x22\x4E\x2D\x65\xA5\x82"
							"\x00\xDB\xF7\x31\xAD\x9E\xF1\x1C\x37\x3E\xAF\x36\xF5\xFE\xD6\x71"
							"\x70\xED\xEA\x51\xFD\x8B\x4D\xA7\x75\x0F\x50\x00\x02\x00\x00\x00"
							"\xB0\x81\xEC\x98\x4C\x14\x66\xD2\xE9\x89\x0B\xA9\x64\x2E\x30\x71"
							"\x5C\x57\xCA\xA9\x06\x61\xED\x28\x0C\x6A\x80\x0B\x28\x00\x80\xFF"
							"\x01\xD5\xC5\xB8\x35\x61\xCD\xB2\x5B\x00\x00\x00\x00\x49\x45\x4E"
							"\x44\xAE\x42\x60\x82", 885);
			getPNGImage(png);
		}
		else if (url == "safari.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x03\x2E\x49\x44\x41\x54\x38\x11\x75\xC1\x4B\x4C\x1C"
							"\x75\x00\x07\xE0\xDF\xBC\x67\xFE\xF3\xDA\x61\xE9\xB6\xC0\x2E\x2C"
							"\x94\xA6\x65\x23\xC5\x08\x41\xD3\x18\xAD\x35\x31\x69\x6F\x26\x90"
							"\xE8\x49\xAE\xE8\xD5\x0B\x37\xCA\xCD\x8B\x37\x3D\x35\x69\xDA\x98"
							"\x18\x83\x89\x26\x35\x8A\x89\x31\x5A\x63\x02\xB5\x07\xB7\x40\x28"
							"\xB0\xB0\x0F\xB6\xFB\x90\xD9\x07\x3B\xCC\xEC\xEE\xCC\xEC\x8E\x7B"
							"\xE0\x40\x7C\x7C\x1F\x85\xFF\xB0\xB4\xB4\x34\x23\x8A\xE2\x94\xEF"
							"\xFB\x1B\x2B\x2B\x2B\xDB\x14\x45\x05\xF8\x1F\x14\xFE\x21\xB5\xBF"
							"\xAF\x1E\xE5\xF3\x1F\x48\x92\xF4\x11\x91\xA4\x12\x21\xA4\x1C\x00"
							"\x39\xDB\xB6\x1F\xA7\x52\xA9\xC7\x73\x73\x73\x0D\x9C\x43\xE1\x9C"
							"\x64\x32\x39\x2B\x8A\xC2\x72\x26\x9D\xBE\x99\x3D\xCA\x4B\xB6\xD3"
							"\x42\x10\x00\x22\xCF\x42\x53\xE5\x56\x37\x08\x7E\x3D\x3C\x38\x5C"
							"\x5E\xBE\x7B\x77\x1D\x67\x18\x9C\x59\x5D\x5D\x7D\x93\xE7\xB9\x07"
							"\x4F\x9E\x3C\x7D\x75\xEF\xA8\xCE\xB1\x72\x1C\x46\x78\x1C\x10\x86"
							"\xD0\xEC\x88\x28\x16\x4B\x6C\xCD\x7C\x31\x6E\x1E\x1F\xDF\x1A\xBF"
							"\x7C\x79\x73\x6B\x6B\x2B\x83\x1E\x06\x3D\x0B\x0B\x0B\xD1\xD1\x78"
							"\xFC\x5E\x2E\xFF\x62\xB2\xD0\x54\x40\x5F\xB8\x0E\x85\x84\xA1\x13"
							"\x02\xC7\xE7\x01\x36\x04\x86\xD1\xE0\x75\x02\x04\xFE\x89\x61\x59"
							"\x8D\x97\x66\xA6\xA7\xD7\xFE\x4C\x26\x4F\x68\xF4\xF0\x3C\xFF\x5E"
							"\xB9\x5C\x9A\x35\x3D\x05\x26\x1F\xC3\x5F\xCD\x0E\x52\x0D\x07\x0D"
							"\x06\xC8\x35\x5B\x28\x17\x0E\xD0\xDA\xFB\x1D\x05\xAF\x0F\x6A\x6C"
							"\x0A\x7D\x46\x68\x5A\x51\xD5\xF7\xD1\x43\x87\xFB\xFA\xB4\x4E\xC7"
							"\xBF\x43\xB1\x02\x32\x6D\x03\x45\x1B\xA8\x77\x00\x4F\xE0\xE0\x0B"
							"\x02\x94\xC6\x21\xA2\x7F\xDC\xC7\x73\x57\x41\xC5\x67\xD1\xA0\xC2"
							"\x18\x88\xC5\x21\xCB\xE4\xCE\xE2\xE2\xA2\x4E\x0F\x0F\x0F\x5F\x92"
							"\x09\x19\x65\x24\x03\x99\x3A\x8D\x74\xC9\x41\xBA\xEE\xC3\xF2\x19"
							"\xFC\xF4\xF3\x53\x24\x7E\xFB\x0C\xFC\xF8\xCB\x88\xCC\xDC\x80\x04"
							"\xC0\x69\x71\x30\x22\x31\x70\x1C\x17\x0F\x82\x60\x90\x95\x15\x85"
							"\x84\x42\x21\xA1\x4B\xB3\xE0\xDA\x5D\x0C\xD0\x2C\x22\x9A\x88\xC4"
							"\x25\x82\xB1\xA1\x18\xB6\xC9\x3C\x6A\x57\xDF\xC0\x81\x45\x23\xE5"
							"\x11\x04\x39\x07\xD7\x07\x55\xB8\xED\xB6\x90\xCF\xE7\x09\xED\x79"
							"\x9E\xA5\x6A\x9A\xAD\x4B\x0C\xF4\xB0\x06\x4F\x31\xB0\x55\xED\x80"
							"\xF7\x4E\x91\x6D\x74\x51\x9F\x78\x1B\x0D\x51\x87\x2B\x89\x90\x06"
							"\xFB\x61\x0C\x18\x30\x14\x0E\xA6\x69\xDA\x99\x74\xDA\xA2\x37\x36"
							"\x36\x4A\x15\xB3\xB2\x23\x50\x1E\x86\x46\x14\x1C\xC9\x32\xC6\x06"
							"\x78\x24\xCD\x36\x56\x8B\x1A\x44\xA2\x62\xBF\xE2\x83\x95\x45\x30"
							"\x82\x80\x5B\x93\xFD\xF0\x4F\x2B\x48\x67\xB2\xCF\x77\xF7\xF6\x0A"
							"\x0C\x00\x4F\xD5\x34\x89\x61\xE8\xDB\xB7\x6F\x4C\x31\x27\xBC\x82"
							"\x1A\x58\x64\x69\x03\x3E\x2B\xC1\x2A\x05\x70\xC1\x81\x0E\xF1\x18"
							"\x95\x80\xF9\x58\x13\xDF\x7E\xF5\x85\xF7\xEC\xD9\xE6\xA7\xA6\x69"
							"\xAE\x33\xE8\xA9\x55\xAB\xB9\x53\xDB\x9E\x10\xE0\x5F\x9B\x7B\x7D"
							"\x12\x6D\xA2\xC3\x62\x05\x04\x1C\x07\xC2\xF2\x08\xE9\x2C\x66\x2E"
							"\x06\x98\xBF\xE8\xE0\x87\x2F\xEF\x63\x6D\xED\xC7\xEF\x8B\xC5\xE2"
							"\x27\xAE\xEB\x3A\x0C\x7A\x9A\x3D\xED\x56\x6B\x33\x93\xCD\x5E\x3B"
							"\x35\x0B\xF1\x77\xAE\xF4\x53\x37\x87\x55\xBC\x72\x81\xC1\xEC\x60"
							"\x17\x6F\x45\x1C\x44\xCC\x1D\x7C\xFD\xF0\x5E\xF0\xE8\xD1\x77\xBF"
							"\x94\xCB\xE5\x8F\x2D\xCB\xCA\xA0\x87\xC2\x39\xB2\x2C\xC7\x75\x5D"
							"\xFF\x30\x16\x8D\xBE\x9B\x48\x4C\x44\x47\x46\x46\x04\x8E\xE7\x71"
							"\x6C\x9A\xEE\xCE\xEE\x6E\x3E\xB5\x9F\xFA\xA6\x54\x28\x7C\x6E\xDB"
							"\x76\x1A\x67\x28\xFC\x9B\x20\x49\x52\x42\xD3\xF5\xD7\x54\x4D\x1B"
							"\xE3\x38\x0E\xAE\xEB\xA6\x6B\xD5\xEA\x7A\xB5\x52\xD9\x06\xD0\xC6"
							"\x39\x7F\x03\x12\xDF\x5D\xB7\xD4\xC5\x56\x33\x00\x00\x00\x00\x49"
							"\x45\x4E\x44\xAE\x42\x60\x82", 871);
			getPNGImage(png);
		}
		else if (url == "ie.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x03\x50\x49\x44\x41\x54\x38\x11\x8D\xC1\x7B\x68\x1B"
							"\x75\x00\x07\xF0\xEF\xEF\xEE\x72\x49\x2E\x97\x47\xD3\x24\xC6\x3E"
							"\xB2\xDA\x9A\x59\xD9\xDC\x2B\x48\xB7\x76\xCC\xC1\xA8\x3A\x10\xB6"
							"\x15\x57\x14\xDC\x40\x10\x59\xE9\x98\xFF\xC9\x90\xE1\x98\x22\x88"
							"\x22\x28\x43\x68\x51\x51\xD8\x10\xCA\x5A\x51\xD9\xF0\xB1\x39\xD8"
							"\xAB\xDB\x1C\x76\x63\xF6\x61\xB3\xD4\x35\x49\x93\xA6\x79\xF4\xD2"
							"\xBC\x93\xBB\xDF\xFD\x0C\xAC\xC2\x10\x84\x7D\x3E\x04\x8F\x88\x95"
							"\x86\xAD\x48\xFE\xEE\x67\xCA\xC2\x1A\x54\x8A\x45\xA6\xD3\x5B\x7C"
							"\xCF\x75\x85\xE0\x7F\xF4\x9D\xBE\xDD\xD0\xD8\xE2\xEE\x5C\x67\x59"
							"\x6C\x3E\xE8\x3E\xEB\xB3\x96\xA6\x7A\xD5\x42\xCA\x66\xD2\xB2\x77"
							"\x41\xB9\x02\xA8\xBE\xC8\x74\x3A\x2C\xE0\x3F\xF6\x7F\x75\x4D\x7A"
							"\x3A\xD0\xDE\xDF\xE0\x34\xBF\xE9\x37\x46\xD6\x6F\x2C\xFE\x2C\xD1"
							"\x6C\x90\xBF\x24\xEC\xC4\x38\xDF\x35\x3D\x95\x90\x7E\x1D\x19\xE5"
							"\x7F\x64\xAF\xF7\xED\x04\x83\x8F\xC7\x43\xFA\xBF\xBE\x2E\x6F\xEE"
							"\x79\xEA\x7D\x5F\xB3\xF9\x44\xB7\x69\xA2\x2D\xB0\xFC\x85\x71\xA5"
							"\x54\xE1\xC6\x9B\x8E\x60\xDE\xFB\x12\x54\xC9\xEB\xD6\x78\xE9\x45"
							"\xA7\x97\x16\x7B\xE9\xD8\x5D\x59\xAC\x96\x38\x3C\xA4\x73\x4B\xC7"
							"\xA1\x26\x97\xE1\xF0\x36\xF6\x9B\xD1\x1F\x3D\x89\xD9\x94\x18\x39"
							"\x6B\x1A\xF8\x36\xA8\xAE\x3D\x4D\xAB\x6A\xD8\x21\x73\xF0\x7A\x4C"
							"\x76\xD7\x63\xD6\x13\x47\x73\xEF\x6D\x03\x66\x63\x1C\x56\xF5\x8F"
							"\x05\xFD\x4E\x1B\x06\xBA\x70\xD1\xD0\x1A\xFB\x06\x93\x19\xF7\xC4"
							"\x68\xBA\xB7\xEF\xAD\x0F\xD6\xBD\x76\xAC\x49\x3A\xF0\xF7\x64\x6C"
							"\x7F\xAD\x54\x9D\xB6\x5A\x78\x38\x1D\xA2\x23\x6A\xDE\xF8\x6A\xF3"
							"\x4F\x17\x64\x0E\xAB\x7C\x0D\xDA\x8E\xAD\x86\x9B\x4F\xB4\x2D\x9D"
							"\xC2\x9F\xEC\x59\xFD\x07\xD3\xE0\x97\x27\xCF\x3D\x3E\xB5\xB7\xFB"
							"\xAA\xBC\x67\xF8\x8A\x75\xEC\xB3\x0B\x93\xD9\x64\xEE\x8C\x81\x07"
							"\x64\x89\x87\x68\x16\xB7\x58\x9B\xDC\x9B\x09\xEA\x82\xF3\xE7\xF9"
							"\x12\x4D\x0F\xB5\x6B\xBF\xBC\x71\xBF\xE4\xC1\xCD\x27\xDF\xD1\x0B"
							"\x35\xE3\x2C\x61\x2C\x07\x80\xE0\x01\x46\x38\xE2\x12\x2D\xC6\x8E"
							"\x44\x46\xC3\xED\x99\x3C\x0D\xCD\x24\x0F\x0B\xA8\x6B\xF7\xC4\x37"
							"\xA8\xE9\x5B\xDB\x23\x4B\x2A\x2E\xFB\x06\xC0\x89\x76\x70\xB4\x6A"
							"\x63\x8C\x88\x00\x08\xFE\x45\xA0\x97\x0B\xD5\x60\xA5\xA8\x42\xAB"
							"\xD4\x74\x5D\xD3\xCA\x02\xD5\xCE\x9B\xB8\xC2\xD5\x5D\xB9\xE4\x5C"
							"\xCB\x77\xB5\x43\x28\x72\xAD\xF0\xA8\x1A\x8D\xCE\xA5\x8E\x45\xEE"
							"\x25\xCF\x99\x24\x51\xC0\x03\xAC\x94\x2F\xF3\x4C\x87\x50\xAE\xE8"
							"\x48\x66\x35\xA8\x4A\x25\x23\x40\xCF\x78\xA0\x4C\x06\xE2\x69\xEE"
							"\xAF\x2B\x5A\xA0\xAD\xB3\xA4\xBA\x74\x9B\x68\x70\x79\x6D\xCF\x87"
							"\xEE\x44\xC6\xCE\xBC\x12\xC8\xA3\xEE\xE0\xE8\x84\x63\xD3\xD6\x8E"
							"\xA3\x94\x13\x36\xCC\xC7\x6B\x58\x09\x65\x43\x6A\x22\x74\x5C\x40"
							"\x79\xCE\xC9\x94\xC5\xE6\x16\x5A\xF8\x38\xB5\x82\x1D\x2E\x45\x1D"
							"\x6C\xB0\xF1\x70\x38\x2C\x2F\xF7\xEC\x7E\xC6\xBC\x69\x7A\xE9\x7B"
							"\x10\x08\xB2\x43\xDA\x27\xCA\xE6\x17\x32\x79\x5D\x28\x43\x67\x35"
							"\x9D\x1C\x8F\x7E\xB2\x5B\x11\x48\x2E\x2C\xA1\x5C\x86\x5D\x54\xEE"
							"\xC4\x62\xA5\x1B\x2E\x07\xB7\x5E\x96\xF8\xE7\x78\x4E\x34\xC8\x76"
							"\xCB\x5E\xC9\x2E\xED\x41\x1D\x65\x84\xAC\x14\x28\xE2\xC9\x0A\xCB"
							"\x2C\xE5\x47\xB2\xD1\xC4\xE7\xA8\xE3\xA0\x23\x0D\x5D\x50\x19\x2C"
							"\xBE\xF8\xDB\x6B\x23\x8D\x89\x1B\x83\xD9\xE8\xC2\x48\x78\xA1\xA8"
							"\xC4\x53\x35\x64\x72\x94\xA4\x73\x94\xC4\x92\x55\x16\x5E\x28\xC6"
							"\x16\x23\xCA\x87\xA9\xFB\xF1\x23\xA1\x77\xB7\x2F\xA3\x8E\x50\x76"
							"\x8A\x27\xE3\x43\xFB\x50\xC9\x77\x03\xEA\x1F\x84\x54\xEF\x5D\x4C"
							"\xFA\xC3\x1F\xB1\x4F\xD7\xD8\x1B\x2D\x5D\x26\x93\xA1\x95\x81\xD1"
							"\x72\x51\x9D\x5B\x4E\xE5\xAE\xCD\x8E\xCF\x04\x63\x43\xFD\x14\xAB"
							"\x08\xEA\xF4\xE0\x01\x82\xF8\x94\x15\xB4\xC2\x03\x2C\xCF\xED\x9A"
							"\xD1\xF0\x88\xFE\x01\xAA\x49\x94\xA9\xE6\x22\x1B\x06\x00\x00\x00"
							"\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 905);
			getPNGImage(png);
		}
		else if (url == "vlc.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x02\x20\x49\x44\x41\x54\x38\xCB"
							"\x9D\x93\x4F\x48\x53\x71\x00\xC7\x3F\xBF\xF7\xDE\xDE\xDE\xB6\xE7"
							"\xDE\x66\x30\x65\xFE\xEB\xCF\x92\x4A\x4A\xEC\x3F\x2E\x0B\xA2\xEC"
							"\x10\x8C\x52\x30\xA3\x4B\x97\xEA\x10\x81\x78\x52\x28\x08\x3A\xD5"
							"\xA9\x82\xB2\x82\x4E\x46\xC7\x0C\x22\x21\xE8\xCF\x41\xA8\x40\xC2"
							"\x29\xE4\x44\x49\x62\xA6\x92\x6B\xBA\x5A\xC6\xB6\xB7\xED\x75\xC9"
							"\x2E\x6B\xB6\xFA\x5E\xBF\x5F\xBE\x7C\xBE\x87\x2F\xAC\xA2\xF0\x39"
							"\xED\x62\xF8\x8C\x72\x99\xFF\xD1\x83\x0E\x6F\xF5\x7C\x17\x56\xAC"
							"\x1B\xAB\xFF\x58\xD9\xBA\x62\x39\xA9\x98\x91\xAF\x0D\x1E\x92\xAA"
							"\xB6\x62\xF9\x1B\xB0\xD6\xEF\x6F\x2D\x96\x53\x8A\x19\xD6\x9E\xB3"
							"\xA1\x58\x53\x23\x19\x33\x4B\x7A\x64\x2C\x04\x4F\xEF\x96\x4C\x70"
							"\xF5\x46\x9F\xEE\xF6\x94\x1F\xB0\x84\x82\x90\x6C\xB8\x74\x77\xF0"
							"\xDA\xCD\x3E\x4F\xC9\x04\x6B\xC2\xF7\x9A\xAB\x8C\xD6\x72\xBB\xDA"
							"\x84\x2A\x43\xF5\xEC\xB0\x91\x1C\x7E\xD1\x02\x3C\x29\xA9\xC0\x58"
							"\x8C\x84\xFC\xC3\x23\xB8\xC3\xA0\x1A\x60\xC4\x60\x76\xC1\x1E\xFA"
							"\x53\x41\xC1\x84\xDB\x27\xEB\x65\xB7\x95\x3A\x22\x2B\x60\x01\x79"
							"\x40\x75\x42\x59\x3E\x7D\xF8\xCE\xA9\x4D\xB6\xBF\x16\x24\x76\x75"
							"\x35\xE4\xB6\xB7\x05\xB2\xC6\x5A\x72\x36\x9D\xAC\xAC\x93\x31\x6A"
							"\x31\x1B\x8F\xD6\x2D\xED\xBC\xB0\x6D\xD5\x09\x1F\xEE\xB7\x7B\x07"
							"\x13\x5F\xDA\x53\xCD\xDD\x7C\x2A\x77\xA2\xE6\x52\x08\x49\x90\x34"
							"\x05\xC9\xCF\x09\xE4\xC8\x9B\xB6\x81\xF3\x5B\xA6\x8F\xDF\x1A\x5F"
							"\x2A\x20\x88\x3E\x3C\xBD\x4F\x7B\x3D\x10\x35\x16\xA7\x2F\xD9\x1C"
							"\x2E\x72\x42\x25\x6D\xF7\x92\x51\xBD\xE4\x25\x0D\x45\x73\xA0\x2E"
							"\x7C\xEC\xF5\x4D\x8C\x47\x9F\xF7\xEC\x3D\x58\x40\xA0\xC5\x47\x3B"
							"\x62\xCB\x79\xDD\x33\xFA\x08\x9F\x4F\x42\xDA\x10\xC4\x72\xFA\x90"
							"\x64\x81\x2B\x39\x4F\xCD\xC4\x10\xE2\xFD\x63\xF1\x23\x8B\xEE\xCB"
							"\xC4\x3A\x81\x97\x00\x02\x60\xEE\xD5\x15\xD9\xF5\xEE\xFA\x44\x7C"
							"\x2A\x1E\x98\x99\x81\x4A\x0F\x38\x35\x50\x14\x81\xCD\x29\x90\x45"
							"\x9E\x6F\x71\x98\x9C\x83\xAA\x6A\xA8\xDC\x6C\x44\xCD\x96\xDE\x40"
							"\x45\x73\x8F\xA9\x00\x38\x48\x34\xDA\x95\x78\xC0\x5F\x0F\x5E\x0F"
							"\xA4\x92\x20\xB2\x20\x59\x16\x92\x69\x21\x34\x28\xAB\x84\xDD\x35"
							"\xE0\xA8\x00\xA1\x7D\xAD\x35\xB5\xE5\x1D\xC0\x5B\x05\x40\xCF\x0C"
							"\x75\x2A\x1B\xFD\x00\xD8\xEB\x80\xEF\x2B\x87\xF8\xC5\xB8\x32\xD4"
							"\x05\xE8\x80\x0C\x4A\x6A\xF0\xC4\xEF\x82\x67\xFD\x63\x93\xDE\x6C"
							"\xEA\x9F\xDE\x9A\x76\x2F\x46\x00\x7E\x02\x6B\x20\xAD\x2C\x4A\xE3"
							"\xCD\x0E\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 622);
			getPNGImage(png);
		}
		else if (url == "fb2k.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x04\x00\x00\x00\xB5\xFA\x37"
							"\xEA\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x01\x4C\x49\x44\x41\x54\x28\xCF"
							"\x6D\x91\x3D\x28\xC4\x71\x1C\xC6\x9F\xDF\xEF\xDC\xFD\x4F\xE9\xF2"
							"\x9A\xAB\x43\x28\x49\xC9\x62\xF0\x32\xB8\x52\x5E\xCA\x6D\xC2\x60"
							"\x10\x16\x59\x44\xD8\x2E\x65\xB1\x5B\xA4\x2C\x22\xDB\x95\x41\x57"
							"\x36\x2F\x25\x79\x49\x62\x21\x13\x65\x40\x4E\x49\x9D\xB7\xFB\x18"
							"\x9C\xEB\x0E\x9F\xF1\xFB\xF4\x3C\xDF\x7A\x1E\x29\x89\xF1\xE7\x76"
							"\xC9\x25\x49\x72\xF9\xDA\x6D\xA9\x32\x31\x39\x9D\x8B\x87\x77\xEE"
							"\x26\x79\xE5\x64\xD5\xEF\xDC\x76\xAF\x18\x5F\xBA\xEE\xF1\x04\x6F"
							"\x5E\x60\x66\xDF\xD6\xD9\xDA\xC9\x6D\xB8\x8B\x7B\x3B\xE4\xA4\xEC"
							"\xA6\x28\xB4\x0A\x00\xC7\xB1\xA3\xC7\x04\x00\x7D\x11\xE3\x97\x49"
							"\xFA\x5D\x0D\x27\x31\x7E\x71\xF9\x9C\xD5\x92\xCC\x30\x79\x15\x61"
							"\xFE\xA1\x66\xCE\x14\x7C\x3F\x08\xF4\xAD\x87\x68\xE6\x3C\x25\x5D"
							"\x10\xA4\x95\xE1\x4D\x5B\x26\x23\x19\x5B\x36\xB6\xDD\x86\xC8\xE7"
							"\x0A\x80\x6B\x8A\x11\xCD\x84\x0F\x6C\xA5\xAC\x64\x8C\xBF\x27\xB2"
							"\x86\x10\x21\x00\x7A\x11\x62\x89\xC1\xA8\x09\xC8\x48\x32\x39\x45"
							"\x23\x1F\x89\x11\x84\x8B\x77\x12\x64\x23\x06\xF8\x24\x30\xF1\xD3"
							"\x85\x35\x25\x53\x5B\xB0\x47\x14\x80\x4D\x76\x81\xD9\x7D\x5B\x9E"
							"\x6C\x56\x92\xDB\xDD\x78\x70\x0F\xB0\xC0\x3C\x00\x67\x31\xA7\x45"
							"\x9E\xF4\x2E\x9D\xC2\xFE\x87\xF8\x06\x8B\x2C\x13\xE1\xE9\xAD\x78"
							"\x48\xDE\x5F\x6B\xC8\xA9\x1C\x3B\x79\x07\x38\xFD\xA8\x9E\xFE\x2B"
							"\x4B\x92\xBB\x7C\xF4\x21\x1E\x7B\xAD\x1A\xCF\x0C\x4F\xC7\xFA\x3A"
							"\x72\x43\xB2\xE9\xA7\x2F\x4C\x12\xCB\xB6\x7D\xA8\x6B\x13\x00\x00"
							"\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 410);
			getPNGImage(png);
		}
		else if (url == "wmp.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x02\x55\x49\x44\x41\x54\x38\xCB"
							"\xD5\x93\x4B\x48\x94\x51\x14\xC7\xFF\xF7\x7E\xF7\x7B\xCC\xCB\xC7"
							"\x8C\xAF\xD1\x31\xED\x81\xD1\xA6\x20\x22\x06\xAD\x45\x12\x6D\x82"
							"\xA2\x08\x24\x42\xDA\xB5\x6F\x51\x50\xFB\x68\x23\x84\xBB\x96\x41"
							"\x9B\xB6\x25\x84\xDA\x26\x8A\x94\x28\xA5\xA8\xA6\x30\x35\x23\xD4"
							"\xC6\x79\xE9\x38\xE3\x7C\x8F\x7B\xEF\x69\x61\x46\xE3\xBA\x4D\x67"
							"\x73\x76\x3F\x7E\xE7\x7F\xCE\x01\xFE\xFB\x62\x00\x70\xFD\xF1\x7C"
							"\x6F\x22\xE6\x1C\x06\x10\xE3\x0C\x61\x00\xA6\x25\xB8\x30\x18\xB3"
							"\x0C\x0E\xD3\x36\x85\xE0\x06\x17\x9B\x1B\x1B\x99\x8A\x62\x8B\xC2"
							"\x09\x7D\xB8\x7D\xA2\xC3\x05\x00\x01\x00\xC9\x78\xE4\xF2\xD1\x7D"
							"\xED\xA9\xB2\x1B\x9C\xB1\x75\xCD\x4A\xAC\xBF\x8F\xDB\xBA\xC6\xCA"
							"\xD1\x3E\x7F\x2B\xDA\xED\x81\x73\xF2\x03\x65\x69\xA5\x0A\x51\x2D"
							"\xB3\x39\x1F\x77\x00\x4C\xFC\x01\x70\x06\x80\x48\x8B\xD5\x99\xC6"
							"\xFE\xC5\x91\x44\x48\x6F\x71\x88\x30\x60\x37\x44\x7E\xB4\x9C\xF6"
							"\xBE\xF5\x5C\x2C\x49\xCD\x51\xF1\x14\x79\x41\x40\x9F\x4A\xA4\x77"
							"\x46\x10\x00\x50\xA8\x2A\x35\x97\x2D\xAD\x5F\x98\xB9\x11\x0F\x1F"
							"\x1F\xE6\x3C\xD4\x04\xF9\x72\x14\xE4\xCE\x21\x55\xCC\xDA\x55\xB3"
							"\x3D\x5A\xE9\x18\xA8\x19\x8C\x18\xDF\x95\x01\x07\x00\x5F\x6A\x74"
							"\x2F\x4F\x1C\x6B\x74\x0B\x06\x8B\x26\x21\x8E\x0C\xC1\xBA\xF2\x08"
							"\xBC\x2B\x0D\xBD\xB6\x84\xD4\xE7\x87\x61\x59\xAD\x30\x02\x49\xCE"
							"\xB6\x73\xAB\x03\x30\x06\x24\xCA\xF3\x9D\x14\x10\xA0\xB7\xED\x78"
							"\xF3\x1E\x58\x43\xF7\x61\x0E\xDE\x84\xB3\xFA\x91\xBB\x9B\x65\x26"
							"\x95\x96\xBE\xD2\xFE\xDF\x00\xF1\xBB\x13\x31\x2B\xD8\x06\x80\x76"
							"\xB6\x03\xE9\x41\xE5\x97\xA1\x03\xA0\x2A\x49\x06\x80\xD2\x80\x4C"
							"\x36\xDA\xBC\xCE\x80\x08\xC8\xB7\xA7\x17\xC8\x27\x90\xD2\x20\x22"
							"\xC8\xAF\xD3\xD8\x1A\xBD\x84\xE0\xF9\x03\x94\x22\x07\x24\x8F\x34"
							"\xF8\x96\x30\xE0\x08\x03\x51\xDB\x60\x75\x06\x04\x60\x35\xD9\x3F"
							"\x9B\x6D\x4D\x9F\x4B\xCE\x3E\x75\xD4\x97\x69\x04\x6F\xC7\x00\x29"
							"\x01\x26\xF0\xEE\xD0\xD5\x22\x0F\x37\x48\xF2\xD6\xA1\x88\x21\xBB"
							"\xE1\xE9\x7A\x03\x30\xE6\x93\x21\x27\x07\x46\x16\x7F\x52\x53\x21"
							"\x78\x3D\x06\x72\x03\xF8\x91\x94\x9E\x1A\xBC\x9B\x5B\xD9\x7B\x2A"
							"\xE7\x4A\x42\xC5\x0D\x58\xAE\xB4\x69\x8C\xDF\xBB\xE5\x00\x08\x01"
							"\x60\x02\x00\xB2\x0B\x99\x29\x77\x65\x2E\xCD\x80\xA9\xEF\x2D\xC3"
							"\xA9\xCE\xF3\xD7\xE2\xB6\x72\xA9\x64\x36\x0B\x32\x9D\x30\xAB\x78"
							"\x96\x94\x81\x59\xCC\x17\xEC\x62\x6E\x2D\x56\x5A\xCA\x30\x00\x26"
							"\x80\x1A\xDB\x7D\xDB\xA9\x64\x17\x3F\x78\xF2\x6C\xBC\xB5\xB7\xAF"
							"\x27\x96\x68\x6B\x73\xA2\x0D\x2D\xCC\x74\x3A\x88\xF1\xFD\x6F\x9E"
							"\x3D\x79\x91\x79\x35\x3E\x59\x5E\x9E\xCF\xFF\xB3\x67\xFA\x05\xB0"
							"\xB4\x12\x06\x04\xED\x6B\x1E\x00\x00\x00\x00\x49\x45\x4E\x44\xAE"
							"\x42\x60\x82", 675);
			getPNGImage(png);
		}
		else if (url == "icecast.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x02\xC7\x49\x44\x41\x54\x38\xCB"
							"\x95\x92\x6B\x48\x93\x61\x14\xC7\x5F\x4B\x53\xBA\x78\x63\x66\x89"
							"\x48\x89\x15\x95\xB7\xC4\xB4\x54\x4C\xA4\xCC\x10\xDC\xE5\xD5\xCD"
							"\xCD\x4B\xD2\x74\x73\x7B\xA7\x73\xCB\xCB\x34\x35\xDC\xE6\x6D\xF3"
							"\x92\xBE\x86\x69\x5E\xD2\xA6\xA9\x65\x98\x64\xA0\x29\xD2\x97\x0C"
							"\xF1\x43\xA0\xF5\x41\x41\xBA\x48\x44\xA9\x88\xFA\xC5\x20\xFE\xBD"
							"\x5A\x94\x5D\x2C\xFB\xC1\x03\x0F\x87\x73\xFE\xE7\xFF\x3C\xE7\x10"
							"\xC4\x37\xEC\xEC\x1D\x59\xD6\x36\xF6\xE6\xC4\x06\x12\x94\xBD\xB6"
							"\x11\x31\xA5\x07\x36\xC6\x76\xED\xB6\x71\x26\x36\xC3\xD6\xCE\xC1"
							"\x92\xC5\x72\xDA\xB9\x76\x17\x52\x77\xEC\xE5\xC5\xD3\x63\x6C\x71"
							"\xCB\x4B\x6F\x7F\xAE\x0B\x6B\xAF\xD3\x0E\xE6\x58\x12\xFF\xC2\x9E"
							"\xB5\xDF\x2B\x30\x4C\xE1\x46\x95\xCE\x4C\xA8\x6A\x57\x20\x50\x0D"
							"\x22\x22\x8E\x9E\x3E\xE4\xEA\x11\x4F\x6C\x05\x21\xD5\xE9\x1C\x97"
							"\xD6\xF3\x5C\xAE\x7F\x01\x35\xBD\x82\xE8\xD4\x5E\x84\x73\x34\x88"
							"\x4C\x6C\x78\xE5\x1B\x28\xF4\xF8\x6B\xF1\xC5\x8C\x01\x57\x85\xE1"
							"\xCD\x8C\x9A\xE9\x9C\xAC\x19\x42\x6E\xC5\x38\xA2\x04\x99\x88\x52"
							"\xF4\x22\x2A\xB5\x0F\x91\x97\x9A\xDF\x05\x9F\x4F\xF5\xF9\x63\xB1"
							"\x88\xEA\x3C\x4C\x95\xBD\x7E\xAB\xAA\x59\x84\xC6\x30\x8A\xB2\x0A"
							"\x13\x8A\x75\x85\x28\xD1\xEA\xD0\x7D\x6F\x18\x42\x49\x0D\x48\x46"
							"\x84\x9D\xD4\xFA\xE1\x64\x50\xAC\xDF\x2F\x9D\x07\xBD\x52\x8D\xB3"
							"\xEF\xD5\xF4\x32\xA4\x97\xDB\x61\x2C\xD2\xE2\x6E\x67\x07\x5A\xAF"
							"\x57\x43\x2D\x15\xA3\xDD\x74\x1B\x55\xE5\xD5\x50\x66\xD1\x8C\x9B"
							"\x07\x8C\x48\xDB\x62\xD0\x39\x2A\x74\xBD\x38\x4E\xD5\xEF\xAB\x30"
							"\xCE\x7E\x5C\xB3\x4D\x15\x8E\x21\x2F\x27\x07\x49\x7C\x36\x2A\x4B"
							"\xAA\xD0\xD6\xDC\x08\x89\x58\x8A\xDA\xEA\x6B\x68\x6F\xAA\x47\x7E"
							"\x86\x06\x22\x49\x2D\x48\x45\x1F\xB8\x92\x8E\xE5\x80\xB3\xB2\x30"
							"\x82\x2F\xEF\xEE\x11\x65\x3E\x81\x48\x3D\x00\x95\xBA\x10\xC3\x43"
							"\x8F\x31\x36\xFA\x14\x94\xF2\x0A\x0C\xBA\x22\x94\xE8\xB5\x28\xC8"
							"\xD1\xA0\xAD\xE5\x16\x5A\x1B\x6E\x40\x9F\x97\x8B\x70\x6E\x2E\x42"
							"\x48\x03\x4E\x5F\xC8\x7A\x46\xB0\x13\x6A\xF7\x90\x29\x5D\x43\x9A"
							"\x02\x1A\x0B\xF3\xF3\x98\x9B\x9B\x83\x20\x5A\x80\xF8\x58\x11\x74"
							"\x45\x34\x04\x31\x72\xA4\x48\xE5\x50\x29\xD3\xC1\xE1\x70\xB0\xBC"
							"\xBC\x82\xD2\xB2\x4A\x78\x9C\x8A\x1F\x77\x3B\x7A\xC6\x71\xFD\x19"
							"\x64\x22\x6D\x55\x90\x9D\xBD\x60\xA2\x8D\x28\xD7\x5E\x45\x86\x2A"
							"\x1D\x89\x62\x35\x63\xD3\x04\x9F\x10\x39\xBC\x03\xE2\x50\xA4\x2B"
							"\x85\x97\xA7\xDF\xA7\x7A\x83\xEE\x73\x53\x4D\x15\xB8\x5C\x7E\xE8"
							"\xF7\x4F\x64\xB6\xCF\x6A\x64\x64\x64\x61\x75\x75\x15\x75\x75\x75"
							"\x98\x9C\x9C\x84\x48\xAC\x07\x4F\xD6\xFD\x55\x20\x58\x8E\xE3\x3E"
							"\x1C\x3C\xEC\x7F\x34\x31\x35\x35\x35\xB4\xB4\xB4\x04\x83\xB1\x3C"
							"\xED\xB7\x51\x5A\x5B\xDB\x99\x85\xF3\xF2\x6F\xF2\xA5\x8D\x88\xA2"
							"\xEE\xFF\x24\xE0\x1D\x2C\xC3\x31\xF7\xD0\x1E\x07\x6B\x5B\x4B\x26"
							"\xCF\x62\xD3\x65\xE2\x0A\xF5\xE6\xA4\xA4\xAD\x9E\x64\xC6\xF5\x43"
							"\x40\x06\x4F\xFF\x18\xD3\x3E\x67\x77\x0B\x62\xAB\x90\xC9\xAD\x95"
							"\xBC\x94\xAE\x75\x81\x23\x3E\x3C\xDA\xCD\xE5\xE0\x76\xE2\x7F\x60"
							"\x9C\x98\x45\xF0\x8B\xCB\x4E\xF8\x91\x2D\x4C\xE7\x6D\x9B\xE5\x7D"
							"\x01\xA7\x4F\x5F\xE6\xC8\x50\xBB\x4A\x00\x00\x00\x00\x49\x45\x4E"
							"\x44\xAE\x42\x60\x82", 789);
			getPNGImage(png);
		}
		else if (url == "html5.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x01\x9F\x49\x44\x41\x54\x38\xCB"
							"\x9D\x93\xCF\x4B\x02\x41\x14\xC7\xF7\x8F\xF0\xE8\x39\xED\x26\x1D"
							"\x82\x08\xAA\x83\xD0\x29\x22\x82\x2C\x28\xD1\x48\xA8\x4B\x25\x88"
							"\xD4\x29\x41\x88\xF5\x10\x74\x10\xBA\xF4\xC3\x2E\x21\x44\x10\x1D"
							"\xCB\x4E\x92\xE0\xC9\xE8\x90\x1E\x3A\xA5\xEE\xB8\xAE\xAB\x64\x98"
							"\x85\xD9\x6B\xE7\x35\xBB\x8C\xAB\x16\xF4\xE0\x0B\xBB\xCC\x77\x3F"
							"\x33\xF3\x7D\x6F\x85\xC2\xD4\x80\x45\x13\xFC\x53\x16\x81\xD6\x6F"
							"\xA6\xDA\xB2\xBD\x4B\xEA\x92\x1D\xD7\x04\xBD\xB4\x97\x3C\x7E\xE0"
							"\x1A\x82\xA2\x77\x8C\xD3\x38\xBC\x6E\x39\x3B\x54\x5F\x1F\x06\xD5"
							"\x8D\x80\x3C\x0F\xC8\x50\x00\x89\xEE\x80\xA2\x28\x86\x54\x55\x05"
							"\x73\x35\xE3\xBB\x50\x59\x44\x40\x86\x07\x24\x28\x40\x12\x37\xF1"
							"\xC3\xF2\x53\x0E\xE4\xE4\x35\x28\x77\x37\xD0\xCA\xA5\x3B\xD4\xD8"
							"\xF7\x81\xB2\x60\xA3\x80\x04\x0F\x88\x21\x20\xE0\x42\x40\xE9\xF2"
							"\xD4\xC8\xE0\xC5\x37\xD8\xA5\xF2\x3C\x02\x62\x3C\x40\xA4\xE6\xE2"
							"\xEA\xE4\xCF\x09\xB2\x0F\x40\x0E\x45\x28\x1D\x45\xE0\xFD\x2A\xDA"
							"\x21\x9A\x81\x3C\x87\x00\x91\x07\x04\x70\x47\xF7\xE8\x9F\x19\xD4"
							"\xFD\x23\x40\x66\x11\x10\xE0\x01\x1E\xFD\xC8\x34\x07\x5D\x24\xE2"
							"\x87\xB7\x93\x6D\xD4\x57\xB3\x81\x00\x7A\x05\x69\x06\xBD\x1E\x1E"
							"\xE0\xD4\x01\x72\xEA\x16\xAF\x60\xCE\x80\x56\xBB\x4A\xF0\xB9\x38"
							"\x8D\x6B\x4E\x1E\xE0\x30\x00\xF7\x69\x3C\xBE\xB4\x17\xC4\x2B\xD5"
							"\x83\x13\xD0\x3C\x0B\x23\xE0\xF3\x39\x8B\x00\xE6\x75\xF0\x00\xAB"
							"\x01\xA0\xED\xEB\x93\x41\xEB\x31\x85\x93\xC8\xBC\x56\x81\x2F\x63"
							"\x7C\xB5\x69\x94\xC2\x6B\x50\xBA\x38\x86\x8A\x36\x0F\x6D\xA5\x00"
							"\x1F\xC9\x73\x68\x1C\x6C\x60\x07\xAA\x5E\xD3\x18\x73\x00\xD2\xEB"
							"\x5F\xE0\xFB\x4F\x77\x67\x33\x40\x7A\x01\x42\x9A\xB2\x66\x00\x9D"
							"\x7B\xD9\x65\xD3\x93\x07\xE6\x09\x09\xFD\x8A\xE5\xB1\xA2\x29\xAE"
							"\xA9\xC6\x14\x67\xAD\xB6\x9A\xFD\xDF\x6D\xBF\x3E\xC4\xFD\x38\xE7"
							"\x3F\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 493);
			getPNGImage(png);
		}
		else if (url == "flash.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x01\x15\x49\x44\x41\x54\x38\xCB"
							"\x95\x93\x41\x8A\xC2\x30\x14\x40\x7F\x3D\xC1\x78\x84\x7A\x04\x0F"
							"\x20\x88\x5A\xAB\xAD\x8A\xB8\x1A\x10\xC6\xEE\xDD\x09\x6E\x5C\xBA"
							"\x70\xA3\xCC\xC2\xBD\x17\xF0\x04\x2E\xBD\xC0\xE0\x09\xE6\x06\x95"
							"\x42\xA1\x50\x28\xFD\x93\x84\x64\xF8\x69\xD3\xA2\x81\xB7\x69\xF3"
							"\xDE\x4F\x03\x05\xDF\xB2\x70\xC2\x98\x02\xE0\x8C\x30\x95\xF0\x77"
							"\x7C\x8F\xC7\x18\x37\x1A\x82\x91\xC4\x65\x40\x6D\xC0\xAA\x0E\xB8"
							"\xA6\x80\x06\x91\x45\xC0\x30\x5D\x0B\x28\x94\x48\xE5\xFD\x7C\x8E"
							"\xBF\x8F\x07\xF2\x65\x0C\xD4\xF1\x1D\x04\x48\x17\x95\x87\x3C\xC0"
							"\x8F\xE6\x15\x24\x4F\x12\xB4\x5A\x98\xE7\x39\xC6\xCF\x27\x9E\x56"
							"\xAB\x92\x2C\x02\xEA\x62\x3C\x82\x7A\x76\xBB\x5C\x30\xCB\x32\xDC"
							"\x76\xBB\x46\xD9\xE1\x81\x11\x11\x28\x5F\xB6\x8D\x69\x9A\x6A\x50"
							"\x59\x0B\x98\xB8\x1E\x8F\x98\x24\x89\x06\x15\x15\xE0\x92\xA3\x71"
							"\x16\xCD\x26\x9E\xD7\x6B\x8C\xE3\x58\xB0\xE9\x74\x4A\x53\x15\x03"
							"\x53\x20\x8A\xA2\x7F\x0E\xCB\x65\xA5\xA8\x80\x61\xE1\xBB\xC2\x30"
							"\xC4\x9F\xFB\x1D\x77\xBE\x5F\x12\x8B\xB2\x08\x38\x86\x8B\x71\x5E"
							"\x10\xFB\x12\x70\xDE\x90\x74\xD9\x12\xC0\xA0\x66\x63\xD5\x54\x2E"
							"\xF6\x24\x20\x97\xCD\xF8\x7C\x03\xF6\xBF\x41\x9B\xF1\xF1\x07\x5A"
							"\x4C\x80\x8C\x91\x5C\x9B\x60\x00\x00\x00\x00\x49\x45\x4E\x44\xAE"
							"\x42\x60\x82", 355);
			getPNGImage(png);
		}
		else if (url == "rtb.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\xA9\x49\x44\x41\x54\x38\xCB"
							"\xAD\x53\xC1\x0D\xC4\x20\x0C\xCB\x28\x1D\x25\x4B\xF4\xDF\x51\xB2"
							"\xD9\x8D\x76\x95\x53\x19\xB8\xD6\x04\xA9\x3A\x24\x4B\xB4\xC2\xC6"
							"\x4E\x82\x99\x5A\x11\x91\x70\xFF\x26\xF8\xBD\x5C\x11\x9E\x04\x33"
							"\x8D\x4B\xCC\x35\x79\xDF\x3F\xB6\x6D\xFD\x20\xF7\x77\xE0\x3F\xCE"
							"\x3E\x6E\x1E\x09\xD8\x47\xD4\x22\x3F\x4E\x94\x6D\xE6\xAF\xE2\xB4"
							"\x82\x29\xDB\xC7\x51\x0B\x00\xC9\xBD\xF2\x74\xDB\x20\x81\x8C\x7D"
							"\x15\x03\x48\xEE\x78\x0B\x0E\xF7\xD6\xAD\x45\x92\x3B\xB3\x49\x47"
							"\x04\x85\x47\xB1\xE4\x32\xC2\x4A\x84\x35\x19\x1D\x25\x97\x45\xAC"
							"\xAA\x0D\xF2\x3D\x66\x2B\xE2\xAC\x8D\xAB\x76\xB6\x36\xAA\x41\x52"
							"\x31\x9E\x83\xE6\xF3\x51\x56\x22\x74\x21\x47\xF9\x2F\x8F\xE9\xE5"
							"\x73\x3E\x01\x35\x60\x01\xA4\x93\x4C\xC0\x7E\x00\x00\x00\x00\x49"
							"\x45\x4E\x44\xAE\x42\x60\x82", 247);
			getPNGImage(png);
		}
		else if (url == "ps.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x04\x00\x00\x00\xB5\xFA\x37"
							"\xEA\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\xE2\x49\x44\x41\x54\x28\xCF"
							"\xA5\xCE\xBF\x2B\x04\x70\x00\xC6\xE1\xEF\x24\xC2\x9D\x23\x52\x26"
							"\xD9\x0D\x97\xED\x22\x31\x28\x93\x01\x65\xB0\xF8\x03\xAE\x8C\xCA"
							"\x20\x75\x83\x48\xE1\xE4\x72\x49\x91\xB2\xB2\x18\x64\x40\x16\x19"
							"\xAE\x48\x91\xF2\xF3\xBA\x52\x2C\x06\x0A\x3D\x46\x91\x53\xF2\xCE"
							"\x4F\x6F\x9F\x10\xFE\xB2\x9C\x82\x5F\x41\x9F\x61\xEB\xEE\x8B\xA3"
							"\x84\x98\x3A\x8D\x92\xC5\x9E\xDA\x55\x19\xB3\xA5\x5C\xB3\x27\x3F"
							"\x82\x88\x05\xE7\x6A\x55\xD8\x2D\x06\xD2\x4E\xD5\xAA\xB1\xE8\xC7"
							"\x86\x88\x79\xC7\x62\xCA\xEC\x7D\x07\x2F\x2E\x25\x44\xA4\x3D\x18"
							"\x90\xF2\xFE\x09\xDE\xDC\x59\x12\xD7\xA3\x5B\xA5\x19\x37\x46\x4D"
							"\xCB\x98\x95\xF3\x2A\xE4\xF5\x6A\xD0\x22\xEB\x44\x87\xA8\x94\x21"
							"\x5D\x56\x1D\x19\x54\x22\x29\x6C\x8B\x6A\xB2\x2C\x6B\x44\xA7\x88"
							"\x09\x05\x67\x56\xB4\xAA\xD7\xEF\x42\x08\xE1\xDA\x81\x39\xE3\x76"
							"\xB4\x29\x95\x32\xA9\x5A\x5C\x46\xDE\xAD\xC7\xAF\xA9\x57\xA6\xAC"
							"\xD9\x70\xE8\x59\x08\x21\xEC\xDB\x14\xFE\xBD\x0F\x28\x4D\xB1\x81"
							"\xCE\x50\xD3\x95\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 304);
			getPNGImage(png);
		}
		else if (url == "mplayer.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\x9C\x00\x00\x0E"
							"\xC4\x01\x75\xF6\x84\x81\x00\x00\x01\xBE\x49\x44\x41\x54\x38\xCB"
							"\xB5\x93\x4D\x6B\x13\x41\x18\xC7\x97\x7C\x0A\x6F\x82\x37\xBF\x80"
							"\x57\x3F\x81\x47\xD3\x83\xD4\x80\xF5\xE2\xCB\x21\xA8\xE8\x36\xC4"
							"\x22\xE9\xC1\x1C\x12\x85\xE2\x41\x44\x6D\x11\xC1\x43\x63\x5A\xA5"
							"\x9A\x88\x11\x8B\x07\xA1\x07\x6F\xBE\x20\xD9\x45\x21\x31\x11\xEB"
							"\xEE\xEC\xEC\xEE\x64\x13\x4C\xF7\xE7\xCE\x56\xEA\x45\xD1\x06\xFD"
							"\xC3\x33\xC3\xC0\xCC\xFF\x79\x66\xE6\xF7\x18\xC6\xFF\xD4\xCB\x0D"
							"\x9F\xA5\xBB\x82\xB9\x79\x87\xE9\xE3\x9B\x1C\x38\xF8\x99\x3D\xFB"
							"\xFA\xEC\xDD\xDF\xE3\x71\xD3\x67\x67\xA3\xA5\x14\x4A\x0D\x79\xFD"
							"\x36\x62\xE5\x61\x40\x75\xC1\xE7\xDC\xAC\xC7\x19\x53\xFE\x36\xA6"
							"\x8E\x6E\x6E\x1B\x5C\xF0\x24\xEF\xC7\x5B\xEC\x56\xC3\xD1\x38\x49"
							"\x3A\xC0\x38\x24\x3C\x86\x4C\x26\x31\x48\x0C\x66\x3C\x9F\x49\xF5"
							"\x2A\x0C\x31\x2E\x25\x57\xD0\x3A\x3B\xEB\x73\xF5\x5A\xC8\x8D\xDB"
							"\x8A\xC3\xD3\x82\xC5\x3B\x16\xF5\x7A\x9D\x76\xBB\x4D\xA7\xD3\xA1"
							"\xD7\xEB\x21\xA5\xA4\xDB\xED\x62\x59\x16\x51\x14\xB1\x1A\x2A\x8C"
							"\x5B\x72\xBB\x82\x13\x79\xC9\x91\x63\x82\x99\x93\x5E\x3A\x2F\x2E"
							"\x3D\x21\x9F\xCF\xA7\x51\xA9\x54\x28\x14\x0A\x14\x8B\x45\x6A\xB5"
							"\x1A\xA6\x69\xE2\x38\x0E\x57\x82\xF0\x1F\x18\xAC\x27\x83\x56\xA8"
							"\x62\x46\xA3\x18\xE1\xC5\xA8\x41\xCC\x20\x79\x20\xD7\x75\xE9\xF7"
							"\xFB\xB4\x5A\x2D\x1A\x8D\x46\xBA\x56\xC9\x97\xEB\xF2\xB5\x4E\x27"
							"\xC9\x0D\xFB\x87\xC1\xDF\xE8\xAB\xB3\xC5\xEA\x5A\xC4\x9B\x77\xDF"
							"\xD2\xF5\x94\x9F\x54\xA0\x33\xC5\x71\xFC\xC7\xC3\x1F\x3E\x8E\xB9"
							"\x38\xEF\x72\xBF\xBE\xC1\xF2\x4A\xC4\xBD\xE5\x88\x53\x09\x02\x29"
							"\x4C\xD5\x05\xC1\x83\xB5\x90\x47\xCD\x20\x8D\xE6\xD3\x80\x67\xCF"
							"\xE5\x4E\xAC\xBF\x90\x98\x73\x2E\xAE\x08\xC9\x64\x32\xE4\x72\x39"
							"\xCE\x97\xBE\xFC\x44\x59\x73\xAD\xF9\xD6\x9C\x6B\xDE\x35\xF7\x9A"
							"\x7F\xDD\x07\xBA\x1F\xBA\x9F\x02\xAE\xDF\x74\xB0\x6C\x9F\x6C\x36"
							"\x8B\x6D\xDB\x94\x2E\x0B\x76\xDD\x5C\xA5\xB2\xA0\x5C\xF5\x29\x95"
							"\x3D\x26\xEA\xCE\x5F\x19\x7C\x07\xD7\xA8\xAB\xC5\x53\x48\xD2\x2A"
							"\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 524);
			getPNGImage(png);
		}
		else if (url == "apple.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x04\x00\x00\x00\xB5\xFA\x37"
							"\xEA\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\xAD\x49\x44\x41\x54\x28\xCF"
							"\x63\x60\xC0\x02\x1E\xFD\x67\xC0\x0D\x2E\xFD\x8F\xFA\x5F\x86\x4F"
							"\x41\xD4\xFF\xA0\xFF\xAB\xFE\xE3\xD1\xEF\x85\x5D\xFA\xD5\xFF\x26"
							"\xA0\xCE\xB0\xFF\x9B\xFE\x9F\x03\x2A\x48\xFB\x9F\xF5\x7F\x1F\xAA"
							"\xB2\xAC\xFF\x6E\x40\xBB\x93\xC0\x30\x0B\x0A\x91\xA4\xCF\x81\xA5"
							"\xB3\xFE\x17\x01\x61\x1E\x18\x16\xFD\xBF\x86\xAC\x60\x11\xD0\xE6"
							"\x24\xA8\x14\x04\x4E\x42\xB5\x60\xDA\x7F\x3F\xB0\xBD\x08\xD8\x86"
							"\xAA\xE0\x10\x50\x41\x12\x8A\x82\xB4\xFF\xA7\xFE\xA3\x84\x9C\xDB"
							"\xFF\x38\xA0\x20\x32\x8C\xFB\xBF\x0B\x59\x49\x1B\x50\x20\x09\x0D"
							"\xDE\x42\x56\x70\x0D\x18\x0A\x51\x40\x45\x08\xB8\x0C\x3D\xB8\xB6"
							"\x01\x15\x44\xFD\xAF\x02\x06\x18\xC8\xF8\xAA\xFF\x58\xE3\xF0\xDC"
							"\x7F\x58\xB8\x30\x10\x0F\x00\x5E\x29\xAA\xB1\xD7\x27\xEE\x1B\x00"
							"\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 251);
			getPNGImage(png);
		}
		else if (url == "synology.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x04\x00\x00\x00\xB5\xFA\x37"
							"\xEA\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\xAA\x49\x44\x41\x54\x28\xCF"
							"\xB5\xD0\xCD\x4A\x02\x01\x18\x85\xE1\xEF\xFE\xAF\xC0\xBD\x84\x18"
							"\x18\x22\xA1\x09\x35\x43\x19\x3A\x88\x3F\x19\x14\x49\x24\x41\xC5"
							"\x40\xA6\x1B\x41\x21\x62\x9E\x16\x6D\x6A\xD1\xB4\xC9\xB3\x7E\x36"
							"\xE7\x8D\xF8\x7B\x4A\xF7\x2F\xA0\x90\xDB\x94\x81\xB1\xB6\xBA\x87"
							"\xDF\x41\x6A\xE7\xC9\xC8\x1C\x2B\xD7\x16\x66\xEE\x14\xE6\xFA\x76"
							"\x22\xE2\x4A\xCD\xB9\xB5\x23\x5B\x03\xC7\x1A\x6E\x34\x25\x4E\x4C"
							"\xF4\x45\xC4\xBB\xA5\x44\x22\x33\x96\xBA\x75\x86\x54\xC5\x0B\x86"
							"\x22\xE2\xC2\xDA\x54\xD7\xAB\xAA\xCC\xB3\x0E\x4E\x1D\xC8\x2C\xF5"
							"\x44\xC4\xBD\xA6\x8E\x37\x1C\x5A\x58\x19\x62\xE4\xD1\xA5\x96\xFC"
							"\xAB\x43\x01\x26\x1A\xB6\x3F\x1E\x14\xDF\x43\x7D\x98\xCA\xF7\x95"
							"\xBA\x1C\x7C\x02\xE6\xEF\xB0\xFE\x1C\xF5\x08\xB2\x00\x00\x00\x00"
							"\x49\x45\x4E\x44\xAE\x42\x60\x82", 248);
			getPNGImage(png);
		}
		else if (url == "roku.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x04\x00\x00\x00\xB5\xFA\x37"
							"\xEA\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\x64\x49\x44\x41\x54\x28\xCF"
							"\x63\x60\xA0\x0A\x68\xF8\xCF\x80\x04\x03\xFE\x2F\xF8\x8F\x43\x81"
							"\x03\x5C\x51\xC3\x7F\xEC\x26\x00\x81\x16\x98\xA5\x80\x5B\xC1\x01"
							"\x28\x1B\xC5\x1A\x64\x05\x0B\xA0\xEC\x03\xD8\x14\x2C\x00\xDA\xAD"
							"\x00\x75\x28\x1E\x5F\x60\xB8\x00\x53\x41\x16\x4E\x05\x50\x3F\x30"
							"\xE0\x56\xC0\xC0\x00\x71\x83\x01\x6E\x6F\x02\x01\x98\x1D\x8A\xEA"
							"\x0B\x07\x30\x84\xF0\xB2\xA0\x3C\x8C\x00\x27\x13\x00\x00\x9C\xD5"
							"\x79\xC4\xEC\xC2\x17\x58\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42"
							"\x60\x82", 178);
			getPNGImage(png);
		}
		else if (url == "itunes.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0B\x13\x00\x00\x0B"
							"\x13\x01\x00\x9A\x9C\x18\x00\x00\x03\x87\x49\x44\x41\x54\x38\xCB"
							"\x4D\x93\x4B\x68\x5C\x65\x1C\x47\x7F\xFF\xEF\x3E\xE6\x95\x9B\x99"
							"\x49\x9A\xF4\x26\xCD\x98\x4C\x93\x4E\x1E\x6D\x93\x4A\x5D\xB4\x2A"
							"\x49\xB5\x69\xD0\x2A\x05\x17\x62\x0B\x85\x16\x34\x98\xB8\x51\x17"
							"\x52\x15\x25\x62\xC1\x4D\x21\xB5\xB8\xA9\xD9\xA8\xC1\x85\x94\x08"
							"\x52\x04\x51\x3A\xA5\xA6\x25\x09\x18\x43\x8A\xAF\xD8\x94\x32\x49"
							"\x9A\x99\x49\xEA\xCC\x9D\xB9\xF3\xBE\x8F\xEF\x73\x13\xC1\xB3\x3F"
							"\x67\x77\x08\x00\x88\x88\x84\x10\x00\x20\xA2\xD1\x68\x74\xE4\xC4"
							"\xC8\xB9\xC7\x3A\x3A\x87\xEA\x83\xE1\x56\x22\x62\x05\x33\x97\x5C"
							"\x4B\xDC\x9F\x8D\xC7\xE3\xD3\xAB\xAB\xAB\xAB\x00\xC0\x18\x63\x9C"
							"\x73\x4E\x3B\xB2\x00\x80\xF1\xB1\xB1\x77\x9F\x38\x72\xEC\x22\xF3"
							"\x86\x64\xBF\x16\x86\xEA\xF5\x03\x20\xD4\x6A\x35\x54\x8A\x26\x78"
							"\x25\xCB\x7F\x5B\x9A\xFD\xE4\xD3\x2B\x57\x3E\xFC\x2F\x42\x00\x08"
							"\x80\x98\x98\x98\x98\x6A\x8E\xEC\x1F\xAD\xB1\x3A\xEC\x6E\xEB\xB0"
							"\x55\x2D\x4C\xDE\x80\x46\x8A\x22\x13\x83\x10\x76\xB5\xCC\x93\x89"
							"\x84\x62\xE7\xD3\x28\x6C\xFD\x39\x73\xE1\xC2\x3B\x2F\x03\x80\x04"
							"\x00\xA3\xA3\xAF\xBD\xD5\xD2\x31\xF0\x7E\xD2\x84\xAD\xE9\xED\x54"
							"\x91\x7C\x32\x17\x9C\x6D\xAF\xAF\xB0\x07\x77\xE7\xE9\xAF\x5F\x7E"
							"\x66\xBF\xCF\xC7\xA5\x6A\x29\x2B\x78\xB0\xC3\x12\xAE\xE8\x1F\xE8"
							"\x8D\xA8\xF3\x0B\x0B\x37\x65\x5D\xD7\xF5\xFD\x03\x47\x3E\x9E\xFB"
							"\x23\x0D\x7D\xDF\x01\x29\x61\x82\x85\x79\x09\x73\x5F\x7F\x86\xBC"
							"\x61\xC0\xB5\x1D\x08\xCE\xE1\x58\x55\xEC\xEE\xEC\xA5\xFE\xD8\x33"
							"\xEA\xF2\xE2\x12\x9E\xEC\x3D\xF4\x5E\x77\x77\xF7\xB4\x74\xE6\xCC"
							"\xE9\xF1\xED\x52\xDD\x8B\xC9\xB2\xC7\xA9\x78\x1B\xA5\x6A\x40\x07"
							"\xC1\xC5\xFD\xF8\x35\x54\xCD\x02\x98\x2C\xC1\x13\x0C\x41\x0D\x68"
							"\x20\xC5\x0F\xA7\xE5\x71\x4A\xA6\x32\x76\x29\x97\x97\x0E\x76\xEE"
							"\x2A\xC9\xAD\x7B\xA2\xCF\xFE\xB4\x5C\x40\x35\x14\x21\xB3\xE6\x85"
							"\x4F\x04\xC1\x6D\x07\x6A\x43\x03\xDA\x4E\x9E\x85\xDA\xBA\x0F\x4E"
							"\xA8\x1D\x52\x31\x8B\xD4\x37\x97\xB0\x59\x91\x91\xE5\x75\xCC\x58"
							"\x4B\xE2\xD4\xE1\xC8\x90\x6C\xB9\x6A\xDB\xCA\x46\x0E\x1E\x45\x25"
							"\xC5\x0E\xC0\x2F\x87\x21\x39\x29\x68\x91\xBD\xA8\x3B\x79\x0A\x8F"
							"\x92\x40\xCE\x06\x84\x53\x86\x6D\x39\x28\x59\x2A\x52\x25\x99\x0A"
							"\x1B\x65\x94\x0E\xD6\xB5\xCA\xB6\x23\x60\x94\x3D\x40\x49\x05\xAB"
							"\x79\xE0\xE5\x1E\xF8\x5D\x0E\x3B\x6B\xC0\x63\x00\x39\x47\x40\x09"
							"\x10\x90\xC9\x21\x9D\x33\x61\x14\x81\x94\xE1\x02\x26\xC1\xB2\x38"
							"\x98\x6B\x15\x37\x43\xC1\x26\x00\x3E\xC1\xFC\x0D\xD0\x75\xA0\xB1"
							"\x1E\x10\x12\x83\xAD\x01\x3E\x96\x47\x76\xF2\x4D\xDC\xFB\xE0\x2C"
							"\xDC\x5A\x0D\x92\xE2\x03\x1C\x8F\xF0\x06\x1A\x41\xBC\x9A\x62\xDB"
							"\xA9\xBF\x6F\xC7\xF6\x34\x00\x65\x45\xC4\xFC\x15\xB4\x5F\xFF\x08"
							"\xC5\x2F\x2E\xC2\x1B\xD0\x00\x0D\x70\x96\x6E\xC0\xF8\xF1\x5B\x48"
							"\x66\x06\xF5\x5A\x3D\xB8\xA5\x02\x59\x9B\x77\xB6\xEC\x42\x3E\xF3"
							"\x60\x4E\xCA\x65\xD3\x1B\xCF\x0F\x8F\xBC\x3E\xBF\xA1\x7B\xFA\xA5"
							"\x59\x2E\x96\x67\x88\x9B\x65\x04\x7B\x0E\xE0\xD1\xDE\x21\xD8\x52"
							"\x13\x9A\xBD\x04\x2D\x12\x43\xBE\x6F\x14\x9B\xEB\x24\xEC\x95\x75"
							"\xE9\xF4\xB0\x8A\x5B\x3F\x5C\x7D\x43\x32\x0B\x95\x5C\xA4\xC9\x15"
							"\x7D\x87\x86\x8F\xFF\x9A\xD5\xDC\xCE\xA8\x82\x40\x57\x0F\xFD\x73"
							"\xF4\x3C\x36\x4A\x41\x64\x9D\x00\xCC\xAE\x41\xA4\x9B\x8E\xE3\x61"
							"\xCA\x27\xCA\x0B\xF7\xEC\xE7\x8E\x06\x24\x35\x77\xED\x72\xFC\xE6"
							"\xEC\x57\x84\x1D\xC6\xC7\xC6\xBE\x34\xDA\xDF\x3E\xF7\x7D\x36\x04"
							"\xD1\x15\x76\x2C\x3F\x23\xE1\x0A\x12\x42\xC0\xCD\x39\x02\x89\x82"
							"\xC0\x6A\x5A\x7E\x21\x56\x45\x8C\xCD\x5C\xBF\x3C\x79\xE9\x25\x00"
							"\x5C\x22\x22\x06\x40\x2C\x2E\x2E\x7E\xD7\x17\x5C\xA3\x63\xB1\xB6"
							"\xA7\x4A\x5B\xB2\x62\xAE\x57\x59\x6D\xDB\x22\xB1\x55\x25\x2D\x53"
							"\x64\x07\xA5\x3C\x7B\xA5\x67\xDD\xA5\xB5\xAB\x93\x9F\x4F\x4D\xBD"
							"\x0A\x40\x10\x11\xA3\x9D\x9D\x99\x10\x82\x03\x40\x44\x0F\xF6\x0D"
							"\x0E\x9E\x38\xDF\x10\x39\xFC\xB4\xF0\x36\xB7\x00\x8C\xA8\xB6\xBD"
							"\x65\x6C\x2E\xDD\xB9\x73\xFB\xC6\x74\xE2\x61\xE6\xEE\xFF\x9D\x7F"
							"\x01\x0D\x6F\xA1\x6E\xFB\xAE\xB6\x18\x00\x00\x00\x00\x49\x45\x4E"
							"\x44\xAE\x42\x60\x82", 981);
			getPNGImage(png);
		}
		else if (url == "warn.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x06\x00\x00\x00\x1F\xF3\xFF"
							"\x61\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x01\x88\x49\x44\x41\x54\x38\xCB"
							"\x9D\x93\xCD\x4A\x02\x51\x18\x86\xA7\x24\xA2\x02\x47\xAC\x4C\x8C"
							"\x68\x51\x37\x90\xAB\xDA\x55\x77\x90\x17\x50\x0B\x6F\xC1\xBA\x81"
							"\x7E\xD6\x25\xD4\xA2\x85\x9B\xA0\x56\x6D\x2A\xC2\x28\xA2\xA0\xB0"
							"\x06\x8C\x74\x61\x90\x41\x18\x92\x3F\xD8\x8F\x24\xF8\x33\xE3\xBC"
							"\x7D\xE7\x8C\x33\x5A\x69\x69\x07\x1E\xCE\x7B\xDE\x73\xBE\xF7\xFB"
							"\x66\x31\x82\xD0\x60\x25\xFD\xD6\xF1\xA2\x34\x08\x06\xD3\x42\xAB"
							"\x8B\x15\x96\xDE\x56\x88\x65\x1E\xD2\x52\x31\xEB\x98\xBF\x72\x00"
							"\xD8\xE6\xE4\x03\x8E\xD6\xA6\xC8\x5D\xD8\x79\x77\x3D\x80\x4D\xC1"
							"\xBC\xA6\xBB\xBF\x9F\xD9\x00\xC5\xC7\x0A\x38\x2A\xE9\xEC\xB9\xAD"
							"\xB9\x29\x32\x27\x7D\x28\xA6\x17\xA0\x16\xBC\xD5\x80\xC2\x1A\x79"
							"\xF3\xC8\x9C\xF4\xE3\xCF\xEE\x49\x7F\x2F\xD4\x8F\x45\x62\xA9\x1A"
							"\x40\x9A\x79\x74\xFF\xFB\x14\xF1\x3D\x0B\x0A\xB1\x39\x94\x5F\x3C"
							"\x50\x09\x23\x80\x74\xF9\xD5\x83\x62\x6C\x16\xF1\x7D\x0B\x1A\x76"
							"\x7F\xDC\x35\xA3\x9C\x70\x6B\x24\xDD\x46\x80\x7E\x66\xB0\x37\x75"
							"\xA7\x88\xEE\xF4\x20\x17\x9A\x80\xF2\xE4\x82\x12\x9B\x21\x5C\xD5"
							"\x00\xD2\xDC\x27\xB2\xB7\x93\xB8\xA7\xB7\x3F\xBA\x47\xB6\xBA\x20"
							"\x47\xA7\xA0\x30\x1E\x18\xD3\x46\x80\xA2\xFB\x51\xCD\x67\x6F\xBF"
							"\x4C\x11\xF2\x75\x22\x7D\x3A\x04\x39\x32\x56\x83\x13\xF2\x9D\xF3"
							"\x9B\xA7\xF9\x89\xE3\x61\x84\xA9\xC6\xE8\x7E\xB3\xD9\x81\x52\x68"
							"\x84\x18\x85\x4C\xBB\x1C\x26\x48\xEB\x13\x70\x4F\xF7\xC3\x9A\x0E"
							"\x52\x0D\x9F\x42\x5A\x37\x21\x75\x64\x45\x29\x38\x40\xD8\xF9\x2E"
							"\x57\x76\x23\x80\xCE\x72\xCD\x3D\x23\x71\x28\x42\xDA\x30\x41\x08"
							"\x78\xDB\x91\xF2\x9B\x51\x92\xC4\x0A\x96\x1A\x5D\x87\x6B\x91\x7E"
							"\x2E\x11\xCF\x07\xDD\x08\x78\xDB\xC0\x3F\xE1\x72\x55\xC0\x7F\x60"
							"\xB5\x9F\x32\xEB\xC2\x92\x1E\xFA\xFF\x9A\x00\x00\x00\x00\x49\x45"
							"\x4E\x44\xAE\x42\x60\x82", 470);
			getPNGImage(png);
		}
		else if (url == "xff.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x10\x00\x00\x00\x10\x08\x04\x00\x00\x00\xB5\xFA\x37"
							"\xEA\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC4\x00\x00\x0E"
							"\xC4\x01\x95\x2B\x0E\x1B\x00\x00\x00\xCC\x49\x44\x41\x54\x28\xCF"
							"\x63\x60\x08\x67\x68\x64\x68\x40\x83\xAD\x40\x28\xCC\x00\x05\xB5"
							"\xDF\xFF\xA3\x83\xE3\xFF\x4D\xFE\x6B\xFD\xCF\xEA\xC1\xA9\xE0\xF0"
							"\xFF\xD8\xFF\xEF\xFF\x5B\xFD\x77\x9E\x84\x43\xC1\xC1\xFF\x12\x40"
							"\x69\xAB\xFF\x5C\xFF\x19\x8A\xB0\x2A\x40\x00\xA0\x5B\x88\x52\xD0"
							"\xFC\x7F\xED\xFF\x9B\xFF\xE3\xFF\xBF\xFE\x1F\x0C\x86\x57\xFF\xA7"
							"\x81\xE9\xED\x30\x05\x67\xFE\x8B\x01\x6D\xEC\xFB\xFF\xF6\x3F\xF7"
							"\xFF\x3B\x40\xF8\xFD\xBF\xCC\xFF\x03\x40\xFA\x03\xC2\x8A\x98\xFF"
							"\x72\xFF\xFF\x00\x15\xB0\xFE\x8F\xFE\x9F\x0A\xE4\xCB\xFC\xF7\x07"
							"\xB2\xDE\xC0\x14\x3C\x07\x9A\x20\x05\x34\x10\x64\xC2\x95\xFF\xD7"
							"\xC1\x0A\x76\x02\x59\xBF\x60\x0A\xC2\xFE\x97\xFF\xDF\xF1\x5F\xF1"
							"\xFF\xA3\xFF\xBC\x50\xC7\xC9\xFC\xBF\x8F\xEC\xC8\x03\xFF\xBF\x81"
							"\xC3\xEF\x19\x30\x88\x20\xE0\xD8\xFF\xEF\x24\x79\x93\x42\x05\x9E"
							"\xE0\xC8\xC5\x05\xE3\x00\x1F\xF1\x7A\x19\x4B\x38\x03\x56\x00\x00"
							"\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 282);
			getPNGImage(png);
		}
		else
		{
			sendMessageAndClose(MSG_HTTP404);
		}
		return;
	}
	else if (m_url == "index.css")
	{
		const utf8 &modified = mapGet(m_httpRequestInfo.m_HTTPHeaders, utf8("if-modified-since"), utf8("0"));
		const time_t curTime = ::time(NULL),
					 readTime = readRFCDate(modified),
					 diffTime = (curTime - readTime);

		// check if we need to provide a copy or if we can just do a '304 Not Modified' response
		if (!readTime || (diffTime > gOptions.m_styleCustomHeaderTime) || (diffTime > 31536000))
		{
			const time_t modTime = (!gOptions.m_styleCustomHeaderTime ? (gOptions.m_styleCustomHeaderTime = curTime) : gOptions.m_styleCustomHeaderTime);
			utf8 body,
				 header = "HTTP/1.0 200 OK\r\n"
						  "Content-Type:text/css\r\n"
						  "Vary:Accept-Encoding\r\n"
						  "Cache-Control:private,max-age=31536000\r\n"
						  + utf8(m_compressed ? "Content-Encoding:gzip\r\n" : "") +
						  "Expires:" + getRFCDate(curTime + 31536000) + "\r\n"
						  "Last-Modified:" + getRFCDate(modTime) + "\r\n",
				 // v2 DNAS style
				 g_styleV2Str = "a:visited,a:link{color:#2762AE;text-decoration:none;}"
								"a:hover{text-decoration:underline;}"
								".logo,.titlespan,.tsp,.inp,.tll,b.w,.infh{color:#DEAC2F;}"
								".logo{font-weight:bold;font-size:2.25em;letter-spacing:-0.0625em;padding-left:0.1em;}"
								"textarea,input,select,body,pre,table,b.i{color:#636363;font-family:arial,helvetica;font-size:small;}"
								"textarea,input,select{background-color:#F0F0F0;border:1px solid #CCCCCC;}"
								".ls,.ls td,.en,.en td,.ent,fieldset,.infb{border-collapse:collapse;border:1px solid #CCCCCC;}"
								".ls a{display: block;}"
								".tsp{background:#4C4C4C;}"
								".inp{background:#636363;}"
								".tll,.infh,.thr{background:#F0F0F0;}"
								"div.thr{padding:0.4em;display:inline-block;}"
								".tnl{color:#636363;font-weight:bold;text-decoration:none;}"
								".submit{color:white;background:#2350A5;border:1px solid #CCCCCC;}"
								"span.default{overflow:auto;border:1px solid #CCCCCC;color:#636363;display:block;}"
								".titlespan{padding:3px 0 2px 0;display:block;}"
								"input:disabled,font.t{color:#CCCCCC;}"
								"hr{border:0;background-color:#CCCCCC;height:1px;margin:0 -15px;}"
								"b.e{color:red;}"
								"b.d{color:green;}"
								"b.u{color:blue;}"
								".infh{position:relative;top:-15px;margin:0 -15px;padding:2px 0px;border-bottom:1px solid #CCCCCC;}"
								".infb{display:inline;float:left;margin:0px 15px;padding:15px 15px 10px;}",
				 // v1 DNAS style
				 g_styleV1Str = "a:visited,a:link,b.u{color:blue;text-decoration:none;}"
								"a:hover{text-decoration:underline;}"
								".logo{color:red;font-weight:bold;font-size:2.25em;letter-spacing:-0.0625em;padding-left:0.1em;}"
								"textarea,input,select,body,pre,b.i{color:#eeeeee;background:black;font-family:arial,helvetica;font-size:small;}"
								".ls,.en,.ent,fieldset,.infb{border-collapse:collapse;}"
								"textarea,input,select,.ent,.ls td,.en td,.infb,span.default{border:1px solid #CCCCCC;}"
								".inp,.infh{background:#000080;}"
								".ls a{display: block;}"
								".tll{background:black;}"
								".tsp{background:#000025;}"
								"div.thr{padding:0.4em;display:inline-block;}"
								".thr{background:#DDDDDD;}"
								".tnl{color:black;font-weight:bold;text-decoration:none;}"
								"span.default{overflow:auto;color:white;display:block;}"
								".titlespan{color:white;padding:3px 0 2px 0;display:block;}"
								"input:disabled,font.t{color:#CCCCCC;}"
								"hr{border:0;background-color:#CCCCCC;height:1px;margin:0 -15px;}"
								"b.e{color:red;}"
								"b.d{color:green;}"
								"b.w{color:yellow;}"
								".infh{position:relative;top:-15px;margin:0 -15px;padding:2px 0px;border-bottom:1px solid #CCCCCC;}"
								".infb{display:inline;float:left;margin:0px 15px 10px 0px;padding:15px 15px 10px;}";

			if (gOptions.m_styleCustomStr.empty())
			{
				bool compress = !!m_compressed;
				const bool v2 = (gOptions.adminCSSFile() == "v2");
				if (!v2 && !(gOptions.adminCSSFile() == "v1"))
				{
					body = gOptions.getIndexCSS(compress);
				}

				// fallback to v2 DNAS style
				if (body.empty())
				{
					// force things to the default style if not found so we're not re-trying all the time
					gOptions.m_styleCustomStrGZ = gOptions.m_styleCustomStr = body = (v2 ? g_styleV2Str : g_styleV1Str);
					compress = true;
				}

				gOptions.m_styleCustomHeader = "HTTP/1.0 200 OK\r\n"
											   "Content-Type:text/css\r\n"
											   "Vary:Accept-Encoding\r\n"
											   "Cache-Control:private,max-age=31536000\r\n"
											   "Expires:" + getRFCDate(curTime + 31536000) + "\r\n"
											   "Last-Modified:" + getRFCDate(modTime) + "\r\n";

				if (compress && !gOptions.m_styleCustomStrGZ.empty() && compressData(gOptions.m_styleCustomStrGZ))
				{
					gOptions.m_styleCustomHeaderGZ = gOptions.m_styleCustomHeader + "Content-Encoding:gzip\r\n";
				}
				else
				{
					gOptions.m_styleCustomStrGZ.clear();
				}
			}

			if (!gOptions.m_styleCustomStr.empty())
			{
				// make sure there is a gzipped version available to send if signalled as supported
				if (m_compressed && !gOptions.m_styleCustomStrGZ.empty())
				{
					body = gOptions.m_styleCustomStrGZ;
					header = gOptions.m_styleCustomHeaderGZ;
				}
				else
				{
					body = gOptions.m_styleCustomStr;
					header = gOptions.m_styleCustomHeader;
				}
				header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
			}

			sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
		}
		else
		{
			sendMessageAndClose("HTTP/1.0 304 Not Modified\r\n\r\n");
		}
		return;
	}

	// now we do most of the processing depending on the mode the http instance has been started as
	const utf8 &type = toLower(stripWhitespace(mapGet(m_httpRequestInfo.m_QueryParameters, "type", (utf8)"")));
	const int webSetupAllowed = (m_protocols & P_WEB_SETUP);
	const utf8 host = (relayHostName.empty() ? m_clientHostName : relayHostName),
			   addr = (relayHostIP.empty() ? m_clientAddr : relayHostIP);

	if (m_protocols & P_WEB)
	{
		bool auth_check = false;
		// make sure we unescape this so we cope with some weird clients
		utf8 password = urlUtils::unescapeString(mapGet(m_httpRequestInfo.m_QueryParameters, "pass", (utf8)"").hideAsString());
		if (password.empty())
		{
			// as we need the password for some of the public pages
			// we'll convert any 'authorization' headers into their
			// 'pass' parameter equivalent and then do all checking
			// based on the password which will be cleaner overall.
			const utf8 &auth = mapGet(m_httpRequestInfo.m_HTTPHeaders, "authorization", (utf8)"");
			const vector<utf8> &vauth = tokenizer(auth, (utf8::value_type)' ');
			// format " Basic xxxxxxxxx"
			const vector<__uint8> va = base64::decode((vauth.size() < 2 ? "" : vauth[1]).hideAsString().c_str());
			password.insert(password.end(), va.begin(), va.end());
			auth_check = true;
		}
		else
		{
			// we also support base64 encoded passwords
			// so we need to check for them and decode
			// before passing on to be fully checked.
			if (password.find(utf8("YWRtaW46")) == 0)
			{
				vector<__uint8> va = base64::decode(password.hideAsString().c_str());
				password.clear();
				password.insert(password.end(), va.begin(), va.end());
				auth_check = true;
			}
		}

		// look at the password and check for the multi-1.x style support,
		// extracting as needed which is likely if using the same password
		// parameters for the title update handling that should be sent
		utf8 dj_name;	// throw-away
		int alt_sid = -1;
		if (extractPassword(password, dj_name, alt_sid))
		{
			if (alt_sid != -1)
			{
				realSID = sid = alt_sid;
			}

			// if this is from a converted 'authorization' header then we
			// need to ensure that the user is what we allow i.e. 'admin'
			// changed b611 as Icecast sources send different values so
			// for those cases we'll have to allow it through so it works
			if (auth_check && !hasMount && !(dj_name == "admin"))
			{
				password.clear();
			}
		}

		if (m_url == "admin.cgi")
		{
			// added in 2.4.8+ the ability to restrict access to the admin
			// sections based on the IP / host reported by the connection.
			// this is a user request as a means to help restrict access
			// to these pages and (slightly) improve security (https would
			// be better but for the time being, it's this and passwords).
			if (isAdminAccessAllowed(addr, host) == false)
			{
				// try to redirect to something (if configured)
				// before failing and provide a 403 response...
				const utf8 redirectUrl = gOptions.getStreamRedirectURL((no_sid ? 0 : sid), false, false, !!m_compressed, true);
				sendMessageAndClose((!redirectUrl.empty() ? redirectUrl : MSG_HTTP403));
				return;
			}

			// we need a password for all of the admin.cgi methods
			// so no point in creating it if we know it's missing
			// and return WWW-Authenticate quicker than if we did
			// the checks inside of protocol_admincgi like before.
			if (!password.empty())
			{
				const socketOps::tSOCKET s = m_socket;
				m_socket = socketOps::cINVALID_SOCKET;

				const bool zero_sid = (((int)realSID == -1) ? true : (realSID == 0));
				threadedRunner::scheduleRunnable(new protocol_admincgi(s, sid, no_sid, zero_sid, m_clientLogString,
																	   password, m_referer, m_hostIP, m_userAgent,
																	   m_httpRequestInfo));
				m_result.done();
			}
			else
			{
				sendMessageAndClose(MSG_AUTHFAILURE401 + utf8(!HEAD_REQUEST ?
									"<html><head>Unauthorized<title>Shoutcast "
									"Administrator</title></head></html>" : ""));
			}
			return;
		}
		else if (m_url == "index.html")
		{
			// if no sid is specified, attempt to match to the only stream (v1 like behaviour)
			// before just attempting to provide the results for the default stream id (sid=1)
			if (no_sid)
			{
				streamData::streamID_t lastSID = 0;
				if (streamData::totalActiveStreams(lastSID) != (streamData::streamID_t)DEFAULT_CLIENT_STREAM_ID)
				{
					path_root_summary(XFF);
				}
				else
				{
					if (realSID == 0)
					{
						path_root_summary(XFF, true);
					}
					else
					{
						sendMessageAndClose(redirect("index.html?sid=" + tos(lastSID), !!m_compressed));
					}
				}
			}
			else
			{
				path_root(sid, XFF);
			}
			return;
		}
		else if ((m_url == "7.html") || (m_url == "7"))
		{
			findBaseStream(no_sid, sid);

			bool adminOverride = false, hide = getHideState((no_sid ? 0 : sid)), passworded = false,
				 proceed = isViewingAllowed(sid, password, false, adminOverride, hide, passworded);

			if ((hide == true) && (adminOverride == false) && (proceed == false))
			{
				path_redirect_url(sid, no_sid, true);
			}
			else
			{
				if (isAccessAllowed(sid, XFF) == true)
				{
					cacheItem *item = m_7Cache[sid];
					if (getCachedResponse(item, m_7Lock))
					{
						return;
					}

					streamData::streamInfo info;
					streamData::extraInfo extra;
					streamData::getStreamInfo(sid, info, extra);

					stats::statsData_t data;
					stats::getStats(sid, data);

					const int maxUsers = ((info.m_streamMaxUser > 0) && (info.m_streamMaxUser < gOptions.maxUser()) ? info.m_streamMaxUser : gOptions.maxUser());
					// 7.html format is CURRENTLISTENERS STREAMSTATUS PEAKLISTENERS MAXLISTENERS UNIQUELISTENERS BITRATE SONGTITLE 
					utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:text/html;charset=utf-8\r\n"
								  "Cache-Control:no-cache\r\nConnection:close\r\n",
						 body = "<html><body>" + tos(data.connectedListeners) + "," +
								(extra.isConnected ? "1" : "0") + "," + tos(data.peakListeners) +
								"," + (maxUsers > 0 ? tos(maxUsers) : "UNLIMITED") + "," +
								tos(data.uniqueListeners) + "," + tos(info.m_streamBitrate) +
								"," + aolxml::escapeXML(info.m_currentSong) + getfooterStr();

					sendCachedResponse(item, m_7Cache, m_7Lock, header, body, sid);
				}
				else
				{
					sendMessageAndClose(MSG_HTTP403);
				}
			}
			return;
		}
		else if (m_url == "stats")
		{
			findBaseStream(no_sid, sid);

			bool adminOverride = false, hide = getHideState((no_sid ? 0 : sid)), passworded = true,
				 proceed = isViewingAllowed(sid, password, false, adminOverride, hide, passworded);

			if ((hide == true) && (adminOverride == false) && (proceed == false))
			{
				path_redirect_url(sid, no_sid, true);
			}
			else
			{
				if (isAccessAllowed(sid, XFF) == true)
				{
					const bool json = mapGet(m_httpRequestInfo.m_QueryParameters, "json", (bool)false);
					if (json)
					{
						const utf8 &callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
						path_stats_json(sid, passworded, callback);
					}
					else
					{
						path_stats_xml(sid, passworded);
					}
				}
				else
				{
					sendMessageAndClose(MSG_HTTP403);
				}
			}
			return;
		}
		else if (m_url == "statistics")
		{
			bool adminOverride = false, hide = getHideState((no_sid ? 0 : sid)), passworded = true,
				 proceed = isViewingAllowed(sid, password, true, adminOverride, hide, passworded);

			if ((hide == true) && (adminOverride == false) && (proceed == false))
			{
				path_redirect_url(sid, no_sid, true);
			}
			else
			{
				if (isAccessAllowed(sid, XFF) == true)
				{
					const bool json = mapGet(m_httpRequestInfo.m_QueryParameters, "json", (bool)false);
					if (json)
					{
						const utf8 &callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
						path_statistics_json(sid, passworded, callback);
					}
					else
					{
						path_statistics_xml(sid, passworded);
					}
				}
				else
				{
					sendMessageAndClose(MSG_HTTP403);
				}
			}
			return;
		}
		else if (m_url == "robots.txt")
		{
			utf8 header = "HTTP/1.0 200 OK\r\n"
						  "Content-Type:text/plain\r\n"
						  "Connection:close\r\nContent-Length:24\r\n\r\n",
				 body = "User-agent:*\r\nDisallow:/";

			if (gOptions.robotstxtFile() == "")
			{
			}
			else
			{
				if (gOptions.m_robotsTxtBody.empty())
				{
					body = loadLocalFile(fileUtil::getFullFilePath(gOptions.robotstxtFile()));
					if (!body.empty())
					{
						gOptions.m_robotsTxtBodyGZ = gOptions.m_robotsTxtBody = body;
						gOptions.m_robotsTxtHeader = "HTTP/1.0 200 OK\r\n"
													 "Content-Type:text/plain\r\n"
													 "Connection:close\r\n"
													 "Content-Length:" + tos(body.size()) + "\r\n\r\n";

						if (compressData(gOptions.m_robotsTxtBodyGZ))
						{
							gOptions.m_robotsTxtHeaderGZ = "HTTP/1.0 200 OK\r\n"
														   "Content-Type:text/plain\r\n"
														   "Connection:close\r\n"
														   "Content-Length:" + tos(gOptions.m_robotsTxtBodyGZ.size()) + "\r\n"
														   "Content-Encoding:gzip\r\n\r\n";
						}
						else
						{
							gOptions.m_robotsTxtBodyGZ.clear();
						}
					}
					else
					{
						body = MSG_HTTP404;
					}
				}

				if (!gOptions.m_robotsTxtBody.empty())
				{
					// make sure there is a gzipped version available to send if signalled as supported
					if (m_compressed && !gOptions.m_robotsTxtBodyGZ.empty())
					{
						body = gOptions.m_robotsTxtBodyGZ;
						header = gOptions.m_robotsTxtHeaderGZ;
					}
					else
					{
						body = gOptions.m_robotsTxtBody;
						header = gOptions.m_robotsTxtHeader;
					}
				}
			}

			sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
			return;
		}
		else if (m_url == "crossdomain.xml")
		{
			path_crossdomain();
			return;
		}
		else if ((m_url == "played.html") || (m_url == "played"))
		{
			// if no sid is specified, attempt to match to the only stream (v1 like behaviour)
			// before just attempting to provide the results for the default stream id (sid=1)
			if (no_sid && findBaseStream(no_sid, sid))
			{
				sendMessageAndClose(redirect((!gOptions.getSongHistorySize(sid) ? "index.html?sid=" : "played.html?sid=") + tos(sid), !!m_compressed));
				return;
			}
			else
			{
				if (!no_sid && !gOptions.getSongHistorySize(sid))
				{
					sendMessageAndClose(redirect("index.html?sid=" + tos(sid), !!m_compressed));
					return;
				}
			}

			const bool json = (type == "json"), xml = (type == "xml");
			const utf8 &hideStats = gOptions.getStreamHideStats((no_sid ? 0 : sid));
			bool hide = (hideStats == "all") && !(hideStats == "none"), adminOverride = false, passworded = false,
				 proceed = isViewingAllowed(sid, password, false, adminOverride, hide, passworded);

			// only do a password check if we've got a hidden page
			if ((hide == true) && !password.empty())
			{
				if (proceed && !(json || xml))
				{
					proceed = false;
				}
			}

			// if xml or json is requested then we see if we
			// should provide it publically or use redirect
			if (no_sid || ((hide == true) && (proceed == false)))
			{
				path_redirect_url(sid, no_sid, false);
			}
			else
			{
				if (!json && !xml)
				{
					path_played_html(sid, XFF);
				}
				else
				{
					if (json)
					{
						const utf8 &callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
						path_played_json(sid, callback, proceed, XFF);
					}
					else
					{
						path_played_xml(sid, proceed, XFF);
					}
				}
			}
			return;
		}
		else if ((m_url == "home.html") || (m_url == "home"))
		{
			if (no_sid)
			{
				path_root_summary(XFF);
			}
			else
			{
				path_home(sid);
			}
			return;
		}
		else if ((m_url == "currentsong") || (m_url == "nextsong"))
		{
			const utf8 &hideStats = gOptions.getStreamHideStats((no_sid ? 0 : sid));
			bool adminOverride = false, hide = (hideStats == "all") && !(hideStats == "none"),
				 passworded = false, proceed = isViewingAllowed(sid, password, false, adminOverride, hide, passworded);

			if ((hide == true) && (adminOverride == false) && (proceed == false))
			{
				path_redirect_url(sid, no_sid, true);
			}
			else
			{
				path_track(sid, (m_url == "nextsong"));
			}
			return;
		}
		else if (m_url == "nextsongs")
		{
			const utf8 &hideStats = gOptions.getStreamHideStats((no_sid ? 0 : sid));
			bool adminOverride = false, hide = (hideStats == "all") && !(hideStats == "none"),
				 passworded = false, proceed = isViewingAllowed(sid, password, false, adminOverride, hide, passworded);

			if ((hide == true) && (adminOverride == false) && (proceed == false))
			{
				path_redirect_url(sid, no_sid, true);
			}
			else
			{
				const bool json = mapGet(m_httpRequestInfo.m_QueryParameters, "json", (bool)false);
				if (json)
				{
					const utf8 &callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
					path_tracks_json(sid, callback);
				}
				else
				{
					path_tracks_xml(sid);
				}
			}
			return;
		}
		else if (m_url == "currentmetadata")
		{
			const utf8 &hideStats = gOptions.getStreamHideStats((no_sid ? 0 : sid));
			bool adminOverride = false, hide = (hideStats == "all") && !(hideStats == "none"),
				 passworded = false, proceed = isViewingAllowed(sid, password, false, adminOverride, hide, passworded);

			if (no_sid || ((hide == true) && (adminOverride == false) && (proceed == false)))
			{
				path_redirect_url(sid, no_sid, true);
			}
			else
			{
				const bool json = mapGet(m_httpRequestInfo.m_QueryParameters, "json", (bool)false);
				if (json)
				{
					const utf8 &callback = mapGet(m_httpRequestInfo.m_QueryParameters, "callback", (utf8)"");
					path_current_metadata_json(sid, callback);
				}
				else
				{
					path_current_metadata_xml(sid);
				}
			}
			return;
		}
		else if ((m_url == "streamart") || (m_url == "playingart"))
		{
			const utf8 &hideStats = gOptions.getStreamHideStats((no_sid ? 0 : sid));
			bool adminOverride = false, hide = (hideStats == "all") && !(hideStats == "none"),
				 passworded = false, proceed = isViewingAllowed(sid, password, false, adminOverride, hide, passworded);

			if (no_sid || ((hide == true) && (adminOverride == false) && (proceed == false)))
			{
				path_redirect_url(sid, no_sid, true);
			}
			else
			{
				path_art(sid, (m_url == "playingart"));
			}
			return;
		}
		else if ((m_url == "listen.pls") || (m_url == "listen"))
		{
			if (isAccessAllowed(sid, XFF) == true)
			{
				cacheItem *item = m_PLSCache[sid];
				if (getCachedResponse(item, m_PLSLock, 5))
				{
					return;
				}

				streamData::streamInfo info;
				streamData::extraInfo extra;
				streamData::getStreamInfo(sid, info, extra);

				int index = 1;
				utf8 backupServer = "";
				if (!info.m_backupServer.empty())
				{
					++index;
					backupServer = "File"+tos(index)+"=" + info.m_backupServer +
								   (!info.m_streamName.empty() ? "\nTitle"+tos(index)+"=" + info.m_streamName : "") +
								   "\nLength"+tos(index)+"=-1\n";

					if (!info.m_backupServersList.empty())
					{
						for (vector<utf8>::const_iterator b = info.m_backupServersList.begin(); b != info.m_backupServersList.end(); ++b)
						{
							++index;
							backupServer += "File"+tos(index)+"=" + (*b) +
											(!info.m_streamName.empty() ? "\nTitle"+tos(index)+"=" + info.m_streamName : "") +
											"\nLength"+tos(index)+"=-1\n";
						}
					}
				}

				utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:audio/x-scpls\r\nConnection:close\r\n",
					 body = "[playlist]\nNumberOfEntries=" + tos(index) +
							"\nFile1=http://" + getClientIP(info.m_streamPublic, info.m_publicIP) + ":" +
							tos(g_portForClients) + getStreamPath(sid, true) +
							((streamData::getStreamContentType(sid) == "video/nsv") ? ";stream.nsv\n" : "\n") +
							(!info.m_streamName.empty() ? "Title1=" + info.m_streamName + "\n" : "") + "Length1=-1\n" +
							backupServer + "Version=2";

				sendCachedResponse(item, m_PLSCache, m_PLSLock, header, body, sid);
			}
			else
			{
				sendMessageAndClose(MSG_HTTP403);
			}
			return;
		}
		else if (m_url == "listen.m3u")
		{
			if (isAccessAllowed(sid, XFF) == true)
			{
				cacheItem *item = m_M3UCache[sid];
				if (getCachedResponse(item, m_M3ULock, 5))
				{
					return;
				}

				streamData::streamInfo info;
				streamData::extraInfo extra;
				streamData::getStreamInfo(sid, info, extra);

				utf8 backupServer = "";
				if (!info.m_backupServer.empty())
				{
					backupServer = "\n#EXTINF:-1," + info.m_streamName + "\n" + info.m_backupServer;
					if (!info.m_backupServersList.empty())
					{
						for (vector<utf8>::const_iterator b = info.m_backupServersList.begin(); b != info.m_backupServersList.end(); ++b)
						{
							backupServer += "\n#EXTINF:-1," + info.m_streamName + "\n" + (*b);
						}
					}
				}

				utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:audio/x-mpegurl\r\nConnection:close\r\n",
					 body = "#EXTM3U\n"
							"#EXTINF:-1," + info.m_streamName + "\n"
							"http://" + getClientIP(info.m_streamPublic, info.m_publicIP) +
							":" + tos(g_portForClients) + getStreamPath(sid, true) +
							(streamData::getStreamContentType(sid) == "video/nsv" ? ";stream.nsv" : "") +
							backupServer;

				sendCachedResponse(item, m_M3UCache, m_M3ULock, header, body, sid);
			}
			else
			{
				sendMessageAndClose(MSG_HTTP403);
			}
			return;
		}
		else if (m_url == "listen.asx")
		{
			if (isAccessAllowed(sid, XFF) == true)
			{
				cacheItem *item = m_ASXCache[sid];
				if (getCachedResponse(item, m_ASXLock, 5))
				{
					return;
				}

				streamData::streamInfo info;
				streamData::extraInfo extra;
				streamData::getStreamInfo(sid, info, extra);

				utf8 backupServer = "";
				if (!info.m_backupServer.empty())
				{
					backupServer = "\n<entry>\n" +
								   (!info.m_streamName.empty() ? "<title>" + aolxml::escapeXML(info.m_streamName) + "</title>\n" : "") +
								   "<ref href=\"" + info.m_backupServer + "\"/>\n</entry>";
					if (!info.m_backupServersList.empty())
					{
						for (vector<utf8>::const_iterator b = info.m_backupServersList.begin(); b != info.m_backupServersList.end(); ++b)
						{
							backupServer += "\n<entry>\n" +
											(!info.m_streamName.empty() ? "<title>" + aolxml::escapeXML(info.m_streamName) + "</title>\n" : "") +
											"<ref href=\"" + (*b) + "\"/>\n</entry>";
						}
					}
					backupServer += "\n";
				}

				utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:video/x-ms-asf\r\nConnection:close\r\n",
					 body = "<asx version=\"3.0\">\n" +
							(!info.m_streamName.empty() ? "<title>" + aolxml::escapeXML(info.m_streamName) + "</title>\n" : "") +
							"<entry>\n" +
							(!info.m_streamName.empty() ? "<title>" + aolxml::escapeXML(info.m_streamName) + "</title>\n" : "") +
							"<ref href=\"http://" + getClientIP(info.m_streamPublic, info.m_publicIP) +
							":" + tos(g_portForClients) + getStreamPath(sid, true) +
							(streamData::getStreamContentType(sid) == "video/nsv" ? ";stream.nsv" : "") +
							"\"/>\n</entry>" + backupServer + "</asx>";

				sendCachedResponse(item, m_ASXCache, m_ASXLock, header, body, sid);
			}
			else
			{
				sendMessageAndClose(MSG_HTTP403);
			}
			return;
		}
		else if (m_url == "listen.qtl")
		{
			if (isAccessAllowed(sid, XFF) == true)
			{
				cacheItem *item = m_QTLCache[sid];
				if (getCachedResponse(item, m_QTLLock, 5))
				{
					return;
				}

				streamData::streamInfo info;
				streamData::extraInfo extra;
				streamData::getStreamInfo(sid, info, extra);

				utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:application/x-quicktimeplayer\r\nConnection:close\r\n",
					 body = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<?quicktime type=\"application/x-quicktime-media-link\"?>"
							"<embed autoplay=\"true\" controller=\"true\" quitwhendone=\"false\" loop=\"false\" " +
							(!info.m_streamName.empty() ? "moviename=\"" + aolxml::escapeXML(info.m_streamName) + "\" " : "") +
							"src=\"http://" + getClientIP(info.m_streamPublic, info.m_publicIP) + ":" +
							tos(g_portForClients) + "/listen.pls?sid=" + tos(sid) + "\"/>";

				sendCachedResponse(item, m_QTLCache, m_QTLLock, header, body, sid);
			}
			else
			{
				sendMessageAndClose(MSG_HTTP403);
			}
			return;
		}
		else if (m_url == "listen.xspf")
		{
			if (isAccessAllowed(sid, XFF) == true)
			{
				cacheItem *item = m_XSPFCache[sid];
				if (getCachedResponse(item, m_XSPFLock, 5))
				{
					return;
				}

				streamData::streamInfo info;
				streamData::extraInfo extra;
				streamData::getStreamInfo(sid, info, extra);

				utf8 backupServer = "";
				if (!info.m_backupServer.empty())
				{
					backupServer = "\n<track><location>" + info.m_backupServer + "</location>\n" +
								   (!info.m_streamName.empty() ? "<title>" + aolxml::escapeXML(info.m_streamName) + "</title>\n" : "") +
								   "</track>";
					if (!info.m_backupServersList.empty())
					{
						for (vector<utf8>::const_iterator b = info.m_backupServersList.begin(); b != info.m_backupServersList.end(); ++b)
						{
							backupServer += "\n<track><location>" + (*b) + "</location>\n" +
											(!info.m_streamName.empty() ? "<title>" + aolxml::escapeXML(info.m_streamName) + "</title>\n" : "") +
											"</track>";
						}
					}
					backupServer += "\n";
				}

				utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:application/xspf+xml\r\nConnection:close\r\n",
					 body = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
							"<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">" +
							(!info.m_streamName.empty() ? "<title>" + aolxml::escapeXML(info.m_streamName) + "</title>" : "") +
							"<info>" + aolxml::escapeXML(info.m_streamURL) + "</info>"
							"<trackList>"
							"<track><location>http://" + getClientIP(info.m_streamPublic, info.m_publicIP) +
							":" + tos(g_portForClients) + getStreamPath(sid, true) +
							(streamData::getStreamContentType(sid) == "video/nsv" ? ";stream.nsv" : "") +
							"</location>" +
							(!info.m_streamName.empty() ? "<title>" + aolxml::escapeXML(info.m_streamName) + "</title>" : "") +
							"</track>" + backupServer + "</trackList></playlist>";

				sendCachedResponse(item, m_XSPFCache, m_XSPFLock, header, body, sid);
			}
			else
			{
				sendMessageAndClose(MSG_HTTP403);
			}
			return;
		}
		else if (m_url == "shoutcast.swf")
		{
			path_shoutcastswf();
			return;
		}
#ifdef CONFIG_BUILDER
		else if ((m_url == "builder") || (m_url == "setup"))
#else
		else if (m_url == "setup")
#endif
		{
			sendMessageAndClose(redirect("index.html", !!m_compressed));
			return;
		}
	}
	else if (webSetupAllowed)
	{
		if ((m_url == "runserver") && (m_httpRequestInfo.m_request == HTTP_POST))
		{
			sendMessageAndClose("HTTP/1.0 200 OK\r\nContent-Length:0\r\n\r\n");
			setkill(2);
			return;
		}
		else if ((m_url == "exit") && (m_httpRequestInfo.m_request == HTTP_POST))
		{
			sendMessageAndClose("HTTP/1.0 200 OK\r\nContent-Length:0\r\n\r\n");
			setkill(1);
			return;
		}
		else if ((m_url == "config") && (m_httpRequestInfo.m_request == HTTP_POST))
		{
			utf8 conf_file = gStartupDirectory + "sc_serv.conf";
			ILOG("[SETUP] Saving settings to `" + conf_file + "'");

			config setupOptions;
			setupOptions.load(conf_file, false);

			const vector<string> queryTokens = tokenizer(m_httpRequestInfo.m_PostLine.toANSI(true),'&');
			for (vector<string>::const_iterator i = queryTokens.begin(); i != queryTokens.end(); ++i)
			{
				const utf8 entry = urlUtils::unescapeString(*i);
				const string::size_type pos = entry.find(utf8("="));
				if (pos == string::npos)
				{
					setupOptions.setOption(entry, (utf8)"");
				}
				else
				{
					const utf8 name = entry.substr(0, pos);
					const utf8 value = entry.substr(pos + 1);
					setupOptions.setOption(name, value);
				}
			}

			if (setupOptions.rewriteConfigurationFile(true, false, true))
			{
				ILOG("[SETUP] Saved settings to `" + conf_file + "'");
			}
			else
			{
				ILOG("[SETUP] Unable to save settings to `" + conf_file + "'. Please resolve the error(s) above.");
			}

			sendMessageAndClose("HTTP/1.0 200 OK\r\nContent-Length:0\r\n\r\n");
			return;
		}
		else if (m_url == "configs")
		{
			const utf8 &query = stripWhitespace(mapGet(m_httpRequestInfo.m_QueryParameters, "query", (utf8)""));
			utf8 fn = "sc_serv.conf";
			utf8 body = "";

			const vector<string> queryTokens = tokenizer(query.toANSI(true),'&');
			for (vector<string>::const_iterator i = queryTokens.begin(); i != queryTokens.end(); ++i)
			{
				const utf8 entry = urlUtils::unescapeString(*i);
				const string::size_type pos = entry.find(utf8("="));
				if (pos != string::npos)
				{
					const utf8 name = entry.substr(0,pos);
					const utf8 value = entry.substr(pos+1);
					if (name == "fn")
					{
						fn = value;
					}
					else if (name == "body")
					{
						body = value;
					}
				}
			}

			utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:text/plain\r\n"
						  "Content-Disposition:attachment;filename=\""+fn+"\"\r\n";
			COMPRESS(header, body);
			header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
			sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
			return;
		}
#ifdef CONFIG_BUILDER
		else if (m_url == "collapse.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x0D\x00\x00\x00\x0D\x08\x06\x00\x00\x00\x72\xEB\xE4"
							"\x7C\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0B\x13\x00\x00\x0B"
							"\x13\x01\x00\x9A\x9C\x18\x00\x00\x00\xA3\x49\x44\x41\x54\x28\xCF"
							"\xA5\x92\x3D\x0A\xC4\x20\x10\x46\x73\xA1\xDC\x2A\x37\xF3\x20\x2E"
							"\xD8\xD8\x69\x65\x3A\x71\x2B\x1B\x1B\x4B\x21\x32\xEB\x27\xC9\xE0"
							"\xA6\x48\xF6\x47\x78\x22\xE3\xFB\x06\x15\xA7\x65\x59\xE6\xC6\xA3"
							"\xB1\x35\xE8\x82\x6D\xF7\xE6\xA9\x4D\x4A\x4A\x49\xB5\x56\xBA\x1A"
							"\xD8\x87\x07\x1F\xA1\x67\x4A\x89\x3E\x19\xF0\xE0\xFF\x1E\x8A\x31"
							"\x52\x29\xE5\x0D\xDC\xE3\x5C\x83\xC7\xA1\x10\x02\xE5\x9C\x99\xF1"
							"\x01\xC6\x3A\x3C\x0E\xAD\xEB\xDA\xBB\xEC\x9D\x3A\xE7\x35\x80\xC7"
							"\x21\x63\x0C\x79\xEF\x59\xC2\xFA\x60\xAC\xC1\xE3\x90\xD6\x9A\x9C"
							"\x73\xB7\xC0\xE3\x90\x52\x8A\xAC\xB5\xB7\xC0\xFB\x2B\xA4\x84\x10"
							"\xFD\xBC\x57\x01\xEC\xC3\x3B\x7E\xC4\xD7\x7F\xEF\x05\xFC\x4E\x08"
							"\xFC\x0F\xA9\x38\xAE\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60"
							"\x82", 241);
			getPNGImage(png);
			return;
		}
		else if (m_url == "expand.png")
		{
			const utf8 png ("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
							"\x00\x00\x00\x0D\x00\x00\x00\x0D\x08\x06\x00\x00\x00\x72\xEB\xE4"
							"\x7C\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0B\x13\x00\x00\x0B"
							"\x13\x01\x00\x9A\x9C\x18\x00\x00\x00\xCC\x49\x44\x41\x54\x28\xCF"
							"\x95\x92\x41\x0A\x83\x30\x10\x45\xBD\x90\xB7\xF2\x66\xDE\xC2\x55"
							"\x21\x1B\xC1\x85\x6E\xD4\x85\xA0\xA6\x0D\xBA\x51\x50\x77\x42\x65"
							"\x9A\x1F\x3A\x31\x6D\x51\x6A\xE0\xC9\x38\xF9\x2F\x31\x12\x2F\x08"
							"\x02\x5F\x73\xD3\x3C\x35\x74\xC2\xF3\x9D\xF3\x3D\xFD\x10\x61\x18"
							"\xD2\x3C\xCF\xB4\x6D\x1B\x1D\x8D\x65\x59\x08\x39\xE4\x21\x49\xA5"
							"\x14\xFD\x33\x86\x61\x80\x24\x8D\x84\x97\xAB\xD2\xBD\xEF\x7B\x9A"
							"\xA6\xE9\x83\x75\x5D\x7F\x7A\x5D\xD7\x59\xE9\xD1\xB6\x2D\x8D\xE3"
							"\x68\x0F\x8D\x9A\x71\x7B\x4D\xD3\xEC\x52\x59\x96\x84\xDD\x00\x87"
							"\xBE\x6B\x80\x9C\x95\xD2\x34\xA5\xBA\xAE\x49\x4A\x69\x70\x7F\x35"
							"\xF7\x30\x9F\x65\xD9\x2E\xC5\x71\x4C\x45\x51\x18\xAA\xAA\x32\x40"
							"\xE0\x9A\xE7\x92\x24\xB1\x92\x12\x42\x98\x55\x98\x3C\xCF\x2D\x6E"
							"\x1F\x8B\x1F\x4A\x47\xB8\x92\x88\xA2\x88\x70\xAE\x33\xF0\x99\xC8"
							"\xF1\x8D\xB8\x7C\xF7\x5E\x5E\xA8\x08\x66\xFF\x94\x63\xFE\x00\x00"
							"\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82", 282);
			getPNGImage(png);
			return;
		}
		else if (m_url == "builder")
		{
			uniFile::filenameType fn = gStartupDirectory + "config_builder/config_builder.html";
			size_t fileSize = uniFile::fileSize(fn);
			uniFile::filenameType fn_js = gStartupDirectory + "config_builder/config_builder.js";
			size_t fileSizeJS = uniFile::fileSize(fn_js);

			utf8 header = MSG_NO_CLOSE_200,
				 body = "<!DOCTYPE html><html><head>"
						"<meta charset=\"utf-8\">"
						"<meta name=viewport content=\"width=device-width, initial-scale=1\">"\
						"<title>Shoutcast Configuration Builder</title>"
						"<link href=\"index.css\" rel=\"stylesheet\" type=\"text/css\">"
						"<link href=\"images/favicon.ico\" rel=\"shortcut icon\" type=\"" + gOptions.faviconFileMimeType() + "\">"
						"<link href=\"config_builder.css\" rel=\"stylesheet\" type=\"text/css\">"
						// skip config_builder.js if the main setup.html cannot be found to load
						+ (fileSize > 0 ? utf8("<script type=\"text/javascript\" src=\"config_builder.js\"></script>") : "") +
						"</head>"
						"<body style=\"margin:0;\">"
						"<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr>"
						"<td><div class=\"logo\">Shoutcast Configuration Builder</div></td></tr><tr>"
						"<td style=\"text-align:right;vertical-align:bottom;padding-right:0.1em;\">"
						"<a target=\"_blank\" title=\"Built: "__DATE__"\" href=\"http://www.shoutcast.com\">Shoutcast Server v" +
						addWBR(gOptions.getVersionBuildStrings() + "/" SERV_OSNAME) + "</a>"
						"</td></tr><tr><td class=\"thr\" align=\"left\"><table width=\"100%\" "
						"border=\"0\" cellpadding=\"5\" cellspacing=\"0\"><tr class=\"tnl\">"
						"<td>&nbsp;<a target=\"_blank\" href=\"http://forums.shoutcast.com/forumdisplay.php?f=140\">Help</a>&nbsp;&nbsp;|&nbsp;&nbsp;"
						" <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_Broadcaster\">Documentation</a></td></tr></table></td></tr></table><br>";

			// only process if the html and js files can be found
			if (fileSize && fileSizeJS)
			{
				body += loadLocalFile(fn);
			}
			else
			{
				body += "<br><table cellpadding=\"5\" cellspacing=\"0\" border=\"0\" width=\"100%\">"
						"<tr><td align=\"center\" id=\"counter\">"
						"<br>There was an error finding the files required for running the configuration builder.<br>"
						"Check that there is a 'config_builder' folder in the same folder as the server.<br><br>"
						"<a href=\"/setup\">Click here</a> to try loading the configuration builder files again. If this message<br>"
						"remains, you will need to seek assistance via the <a target=\"_blank\" "
						"href=\"http://forums.shoutcast.com/forumdisplay.php?f=140\">help forum</a>.<br><br>"
						"<input type=\"button\" value=\"Exit\" onclick=\"exit()\" class=\"submit\"></td></tr></table></tr></table></td></tr></table>"

						"<script type=\"text/javascript\">"
						"function $(id){return document.getElementById(id);}"
						"function exit(){"
						"$('counter').innerHTML = \"<span><b>Server Stopped</b></span><br><br>The server has now been stopped."
						"<br>Please fix the issue shown on the prior page and retry running setup if required.\";"
						"if(window.XMLHttpRequest){"
						"xmlhttp=new XMLHttpRequest();"
						"}else{"
						"xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");"
						"}"
						"xmlhttp.open(\"POST\",\"exit\",true);"
						"xmlhttp.setRequestHeader(\"Content-length\",4);"
						"xmlhttp.send(\"exit\");"
						"}</script>" + getfooterStr();
			}

			COMPRESS(header, body);
			header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
			sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
			return;
		}
#endif
		else if (m_url == "setup")
		{
			uniFile::filenameType fn = gStartupDirectory + "setup/setup.html";
			size_t fileSize = uniFile::fileSize(fn);
			uniFile::filenameType fn_js = gStartupDirectory + "setup/setup.js";
			size_t fileSizeJS = uniFile::fileSize(fn_js);

			utf8 header = MSG_NO_CLOSE_200,
				 body = "<!DOCTYPE html><html><head>"
						"<meta charset=\"utf-8\">"
						"<meta name=viewport content=\"width=device-width, initial-scale=1\">"\
						"<title>Shoutcast DNAS Setup</title>"
						"<link href=\"../index.css\" rel=\"stylesheet\" type=\"text/css\">"
						"<link href=\"../images/favicon.ico\" rel=\"shortcut icon\" type=\"" + gOptions.faviconFileMimeType() + "\">"
						// skip setup.js if the main setup.html cannot be found to load
						+ (fileSize > 0 ? utf8("<script type=\"text/javascript\" src=\"setup.js\"></script>") : "") +
						"</head><body style=\"margin:0;\">"
						"<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr>"
						"<td><div class=\"logo\">Shoutcast DNAS Setup</div></td>"
						"<td style=\"text-align:right;vertical-align:bottom;padding-right:0.1em;\">"
						"<a target=\"_blank\" title=\"Built: " __DATE__ "\" "
						"href=\"http://www.shoutcast.com\">Shoutcast Server v" +
						addWBR(gOptions.getVersionBuildStrings() + "/" SERV_OSNAME) + "</a>"
						"</td></tr><tr><td class=\"thr\" align=\"left\" colspan=\"2\">"
						"<table width=\"100%\" border=\"0\" cellpadding=\"5\" cellspacing=\"0\">"
						"<tr class=\"tnl\"><td>&nbsp;<a target=\"_blank\" "
						"href=\"http://forums.shoutcast.com/forumdisplay.php?f=140\">Help</a>&nbsp;&nbsp;|&nbsp;&nbsp;"
						" <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_Broadcaster\">Documentation</a></td></tr></table></td></tr></table>";

			// only process if the html and js files can be found
			if (fileSize && fileSizeJS)
			{
				body += loadLocalFile(fn);
			}
			else
			{
				body += "<br><table cellpadding=\"5\" cellspacing=\"0\" border=\"0\" width=\"100%\">"
						"<tr><td align=\"center\" id=\"counter\">"
						"<br>There was an error finding the files required for running setup.<br>"
						"Check there is a 'setup' folder in the same folder as the server.<br><br>"
						"<a href=\"/setup\">Click here</a> to try loading the setup files again. If this message<br>"
						"remains, you will need to seek assistance via the <a target=\"_blank\" "
						"href=\"http://forums.shoutcast.com/forumdisplay.php?f=140\">help forum</a>.<br><br>"
						"<input type=\"button\" value=\"Exit\" onclick=\"exit()\" class=\"submit\"></td></tr></table></tr></table></td></tr></table>"

						"<script type=\"text/javascript\">"
						"function $(id){return document.getElementById(id);}"
						"function exit(){"
						"$('counter').innerHTML = \"<span><b>Server Stopped</b></span><br><br>The server has now been stopped."
						"<br>Please fix the issue shown on the prior page and retry running setup if required.\";"
						"if(window.XMLHttpRequest){"
						"xmlhttp=new XMLHttpRequest();"
						"}else{"
						"xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");"
						"}"
						"xmlhttp.open(\"POST\",\"exit\",true);"
						"xmlhttp.setRequestHeader(\"Content-length\",4);"
						"xmlhttp.send(\"exit\");"
						"}</script>" + getfooterStr();
			}

			COMPRESS(header, body);
			header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
			sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
			return;
		}
		else
		{
			utf8::size_type pos;
			if (((pos = m_referer.rfind(utf8("setup"))) != utf8::npos) || (m_url == "setup.txt"))
			{
				uniFile::filenameType fn = gStartupDirectory + "setup" + m_httpRequestInfo.m_url;
				utf8 header = "HTTP/1.0 200 OK\r\n"
							  "Cache-Control:no-cache\r\n"
							  "Access-Control-Allow-Origin:*\r\n"
							  "Connection:close\r\n",
					 body = loadLocalFile(fn);

				if (!body.empty())
				{
					utf8 mime = "text/plain";
					if ((pos = m_httpRequestInfo.m_url.rfind(utf8(".js"))) != utf8::npos)
					{
						mime = "text/javascript";
					}
					header += "Content-Type:" + mime + "\r\n";
				}

				COMPRESS(header, body);
				header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
				sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
				return;
			}
#ifdef CONFIG_BUILDER
			else if (((pos = m_referer.rfind(utf8("/builder"))) != utf8::npos) || (m_url == "/config_builder.txt"))
			{
				uniFile::filenameType fn = gStartupDirectory + "config_builder" + m_httpRequestInfo.m_url;
				utf8 header = "HTTP/1.0 200 OK\r\n"
							  "Cache-Control:no-cache\r\n"
							  "Access-Control-Allow-Origin:*\r\n"
							  "Connection:close\r\n",
					 body = loadLocalFile(fn);

				if (!body.empty())
				{
					utf8 mime = "text/plain";
					if ((pos = m_url.rfind(utf8(".js"))) != utf8::npos)
					{
						mime = "text/javascript";
					}
					if ((pos = m_url.rfind(utf8(".css"))) != utf8::npos)
					{
						mime = "text/css";
					}
					if ((pos = m_url.rfind(utf8(".png"))) != utf8::npos)
					{
						mime = "image/png";
					}
					header += "Content-Type:" + mime + "\r\n";
				}

				COMPRESS(header, body);
				header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
				sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
				return;
			}
#endif
		}
	}

	// if we've gotten to here, try to look for the request coming from a browser
	// and provide the related html page or fallback to serving up stream content
	if ((m_httpRequestInfo.m_url == "/") &&
		// added in 2.4.8+ so we can force through based on the 'type' parameter
		// which allows for url/?type=.flv to work correctly whilst we also allow
		// for setting url/?type=html to force showing of the html pages provided
		(type.empty() || (type == "html")) &&
	   ((m_userAgentLowered.find(utf8("mozilla")) != utf8::npos) ||
	    (m_userAgentLowered.find(utf8("opera")) != utf8::npos)))
	{
		if (webSetupAllowed)
		{
			// if in setup mode then force everything to the /setup page
			sendMessageAndClose(redirect("setup", !!m_compressed));
		}
		else
		{
			// if enabled then we need to block access to this page as it's not allowed to be public
			path_redirect_url(sid, no_sid, false);
		}
		return;
	}


	// check here if we're ok to try to provide the stream to the client or not
	// i.e. it's not a banned or not in the reserved ip list when that is enabled
	bool htmlPage = false;
	const streamData::streamID_t found_sid = streamData::getStreamIdFromPath(m_httpRequestInfo.m_url, htmlPage);
	if (found_sid > 0)
	{
		sid = found_sid;
	}
    if (m_ssl)
    {
        streamData::streamInfo info;
        streamData::extraInfo extra;
        if (streamData::getStreamInfo (sid, info, extra))
        {
            if (info.m_allowSSL == 0)
            {
                sendMessageAndClose(MSG_HTTP403);
                return;
            }
        }
    }


	// bit convoluted but it allows us to check 'icy-host' and then the
	// client's address since it came up in testing the CDN mode that
	// it didn't allow through connections which were configured by IP
	// only instead of DNS - was a bug in a few builds where this checked
	// for the 'host' value instead of the actual client address (oops)
	// 2.4.8+ - this also looks that 'XFF' header value if provided
	const bool validXFF = !metrics::metrics_verifyDestIP(gOptions, true, XFF).empty();
	if (!validXFF || (isAccessAllowed(sid, XFF) == false))
	{
		if (isAccessAllowed(sid, addr) == false)
		{
			if (isAccessAllowed(sid, host, true) == false)
			{
				sendMessageAndClose(MSG_HTTP403);
				return;
			}
		}
	}

	// now process things and redirect to the relevant page found
	// though if it's the direct url and it's from the SC/YP tester
	// then we really need to let it through as true stream access

	// changed in 2.4.8+ to follow the 'type' parameter so it will
	// be able to force a specific usage type as required be that
	// sc1, sc2, flv, http, m4a or just figure it out from headers
	const bool uvox_agent = ((m_userAgentLowered.find(utf8("ultravox/2.1")) != utf8::npos) &&
							 !(!type.empty() && ((type == ".flv") || (type == "flv") || (type == ".fla") ||
												 (type == "fla") || (type == "http") || (type == "sc1")))) ||
												 (!type.empty() && (type == "sc2"));
	const bool officialAgent = isUserAgentOfficial(m_userAgentLowered);
	if ((htmlPage && !officialAgent && !uvox_agent) || (!type.empty() && (type == "html")))
	{
		sendMessageAndClose(redirect("index.html?sid=" + tos(found_sid), !!m_compressed));
		return;
	}

	// try to redirect directly to the appropriate page based on the number of active streams
	streamData::streamID_t lastSID = 0;
	if (!found_sid && (streamData::totalActiveStreams(lastSID) == (streamData::streamID_t)DEFAULT_CLIENT_STREAM_ID))
	{
		sid = lastSID;
	}

	protocol_shoutcastClient *client = 0;

	// Force v1 connection for Winamp 5.5x clients which ident with ultravox/2.1 as they do not support title updates
	if ((m_protocols & P_SHOUTCAST2CLIENT) && uvox_agent &&
		(m_userAgentLowered.find(utf8("winampmpeg/5.5")) == utf8::npos))
	{	// go for Ultravox 2.1
        client = new protocol_shoutcast2Client (*this, sid, host, addr, XFF, mapGet(m_httpRequestInfo.m_HTTPHeaders, "cdn-slave", false));
	}

	if (!client && (m_protocols & P_SHOUTCAST1CLIENT))
	{
		if (!type.empty() && ((type == ".flv") || (type == "flv") || (type == ".fla") || (type == "fla")))
		{
			client = new protocol_flvClient (*this, sid, host, addr, XFF); // flv
		}
		/*else if ((type == ".m4a") || (type == ".mp4"))
		{
			client = new protocol_m4aClient(s, sid, host, addr, m_clientPort, m_userAgent, XFF, m_referer, HEAD_REQUEST); // m4a
		}*/
		else
		{
			// from 2.4.8+, we now treat 1.x listener connections differently based on the
			// requirement to send 1.x-style metadata or not, which if not we can go for a
			// stipped down handler which will better frame-sync the output for reliabilty
			// which is trickier to do with the metadata handling, hence the split handler
			const bool sendMetadata = mapGet(m_httpRequestInfo.m_HTTPHeaders, "icy-metadata", false);

			// and this is used to look for what is essentially a Shoutcast player request
			// and if we see icy=http then we force providing the HTTP listener client and
			// not the SC 1.x listener client as we know the player won't use that data...
			const utf8 &http = toLower(stripWhitespace(mapGet(m_httpRequestInfo.m_QueryParameters, "icy", (utf8)"")));
			const bool forceHTTP = ((!http.empty() && (http == "http")) || (!type.empty() && (type == "http")));

			if ((gOptions.getBackupLoop(sid) == 1 ? false : (!forceHTTP ? sendMetadata : false)) ||
				(!type.empty() && (type == "sc1")))
			{
				client = new protocol_shoutcast1Client (*this, sid, host, addr, XFF); // shoutcast 1 with metadata
			}
			else
			{
				client = new protocol_HTTPClient (*this, sid, host, addr, XFF);	// http (equivalent of shoutcast 1 without metadata)
			}
		}
	}

	if (client)
	{
        client->m_queryParams = encodeVariables (m_httpRequestInfo.m_QueryParameters);
		// to prevent some of the stats / logs getting skewed
		// then we filter out the SHOUTcast site 'test' users
		// and we only action for GET requests, not HEAD, etc
		if (!officialAgent && (m_httpRequestInfo.m_request == HTTP_GET) &&
			// changed in #668 to skip doing auth on local clients
			// as it will never provide an applicable advert group
			// unless we're detecting a remote XFF value to use...
			(isRemoteAddress(addr) || isRemoteAddress(XFF)))
		{
			DEBUG_LOG(m_clientLogString + "Starting client via auth.");
			client->authForStream();
		}
		else
		{
			DEBUG_LOG(m_clientLogString + "Starting client directly.");
			threadedRunner::scheduleRunnable(client);
		}
	}

	m_result.done();
}

void protocol_HTTPStyle::state_GetLine() throw(exception)
{
	if (getHTTPStyleHeaderLine(DEFAULT_CLIENT_STREAM_ID, m_lineBuffer, m_clientLogString, m_postRequest == 2 ? m_postRequestLength : 0))
	{
		m_state = m_nextState;
	}
}

// parse header line into key/value components and load into table
void protocol_HTTPStyle::state_AnalyzeHTTPHeaders() throw(exception)
{
	m_lastActivityTime = ::time(NULL);

	if ((int)m_httpRequestInfo.m_HTTPHeaders.size() >= gOptions.maxHeaderLineCount())
	{
		ELOG(m_clientLogString + "Max HTTP header lines exceeded");
		sendMessageAndClose(MSG_HTTP400);
		return;
	}

	// since a POST will get a null line, we need to check and handle it as appropriate when stripping whitespace
	if (!m_lineBuffer.empty() && ((m_postRequest == 0) || 
	   ((m_postRequest == 1) && (m_lineBuffer[0] != '\n') && (m_lineBuffer[0] != '\r'))))
	{
		m_lineBuffer = stripWhitespace(m_lineBuffer);
	}

	if (m_lineBuffer.empty())
	{
		m_state = &protocol_HTTPStyle::state_DetermineAction;
		m_result.run();
	}
	else if ((m_postRequest == 1) && ((m_lineBuffer[0] == '\n') || (m_lineBuffer[0] == '\r')))
	{
		m_postRequest = 2;
		m_nextState = &protocol_HTTPStyle::state_AnalyzeHTTPHeaders;
		m_state = &protocol_HTTPStyle::state_GetLine;
		m_result.read();
		m_result.timeoutSID();
		m_lineBuffer.clear();
	}
	else
	{
		// find the colon that divides header lines into key/value fields
		const utf8::size_type pos = m_lineBuffer.find(utf8(":"));
		// however in some cases (mainly found in the fire builds), the http headers may be split
		// in a manner not expected so we have to look at the last key and append if there is one
		// recorded which appears to be safe to do from testing vs killing the connection as was.
		bool appended = false;
		if ((pos == utf8::npos) && !m_postRequest)
		{
			if (!m_lastKey.empty())
			{
				utf8 oldValue = m_httpRequestInfo.m_HTTPHeaders[m_lastKey];
				m_httpRequestInfo.m_HTTPHeaders[m_lastKey] = oldValue + stripWhitespace(m_lineBuffer.substr(pos+1));
				DEBUG_LOG(m_clientLogString + "Appending HTTP header string to previous key [" + m_lineBuffer + "]");
				appended = true;
			}
			else
			{
				DEBUG_LOG(m_clientLogString + "Bad HTTP header string [" + m_lineBuffer + "]");
			}
		}

		if (m_postRequest < 2)
		{
			// make sure not to re-add if we're doing a header append (see above)
			if (appended == false)
			{
				utf8 key = toLower(stripWhitespace(m_lineBuffer.substr(0,pos)));
				m_lastKey = key;
				// allow empty values. (for urls and what-not)
				if (key.empty())
				{
					ELOG(m_clientLogString + "Connection rejected. Bad HTTP header string [" + m_lineBuffer + "]");
					sendMessageAndClose(MSG_HTTP400);
					return;
				}
                utf8 val = stripWhitespace(m_lineBuffer.substr(pos+1));
				m_httpRequestInfo.m_HTTPHeaders[key] = val;
                if (toLower(key) == "content-length")
                {
                    m_postRequestLength = atoi ((char*)&val[0]);
                    if (m_postRequestLength < 0 || m_postRequestLength > 5000000)
                    {
                        ELOG(m_clientLogString + "Connection rejected. Content length too large [" + m_lineBuffer + "]");
                        sendMessageAndClose(MSG_HTTP400);
                        return;
                    }
                }
			}
		}
		else
		{
			m_state = &protocol_HTTPStyle::state_DetermineAction;
			m_result.run();
			// copy the POST response
			m_httpRequestInfo.m_PostLine = m_lineBuffer;
			m_lineBuffer.clear();
			return;
		}

		m_nextState = &protocol_HTTPStyle::state_AnalyzeHTTPHeaders;
		m_state = &protocol_HTTPStyle::state_GetLine;
		m_result.schedule();
		m_result.timeoutSID();
		m_lineBuffer.clear();
	}
}

void protocol_HTTPStyle::state_Close() throw(exception)
{
	m_result.done();
}

// send buffer text
void protocol_HTTPStyle::state_Send() throw(exception)
{
	if (sendDataBuffer(DEFAULT_CLIENT_STREAM_ID, m_outBuffer, m_outBufferSize, m_clientLogString))
	{
		m_state = m_nextState;
	}
}

void protocol_HTTPStyle::sendMessageAndClose(const utf8 &msg) throw()
{
	m_outMsg = msg;
	m_outBuffer = m_outMsg.c_str();
	bandWidth::updateAmount(bandWidth::PUBLIC_WEB, (m_outBufferSize = (int)m_outMsg.size()));
	m_state = &protocol_HTTPStyle::state_Send;
	m_nextState = &protocol_HTTPStyle::state_Close;
	m_result.write();
	m_result.run();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

utf8 protocol_HTTPStyle::getPlayedBody(const streamData::streamID_t sid)
{
	utf8 body = "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\">"
				"<tr><td><b>Played @</b></td><td><b>Song Title</b></td></tr>";

	streamData::streamHistory_t songHistory;
	streamData::getStreamSongHistory(sid, songHistory);

	if (songHistory.empty())
	{
		body += "<tr><td>No Songs Played</td></tr>";
	}
	else
	{
		for (streamData::streamHistory_t::const_iterator i = songHistory.begin(); i != songHistory.end(); ++i)
		{
			char buf[1024] = {0};
			struct tm lt;
			time_t t = (*i).m_when;
			::localtime_s(&lt, &t);
			snprintf(buf, sizeof(buf) - 1, "%02d:%02d:%02d", lt.tm_hour, lt.tm_min, lt.tm_sec);
			body += "<tr><td>" + utf8(buf) + "</td><td>" + getCurrentSong((*i).m_title) + "</td>" +
					(i == songHistory.begin() ? (streamData::isSourceConnected(sid) ?
					"<td style=\"padding: 0 1em;\"><b>Current Song</b></td>" : "") : "") + "</tr>";
		}
	}

	body += "</table>";
	return body;
}

// played history page
void protocol_HTTPStyle::path_played_html(const streamData::streamID_t sid, const utf8 &XFF) throw()
{
	cacheItem *item = m_htmlPlayedCache[sid];
	if (getCachedResponse(item, m_htmlPlayedLock))
	{
		return;
	}

	utf8 header = MSG_NO_CLOSE_200,
		 body = getStreamHeader(sid, "Stream History");

	if (gOptions.getSongHistorySize(sid))
	{
		body += getStreamMiddlePlayingHeader(sid);
	}

	if (isAccessAllowed(sid, XFF) == true)
	{
		body += getStreamMiddleListenHeader(sid);
	}

	body += getStreamEndHeader(sid) + getPlayedBody(sid) + getIEFlexFix() + getfooterStr();

	sendCachedResponse(item, m_htmlPlayedCache, m_htmlPlayedLock, header, body, sid);
}

utf8 protocol_HTTPStyle::getPlayedJSON(const streamData::streamID_t sid, utf8 &header,
									   const utf8 &callback, const bool allowed) throw()
{
	const bool jsonp = !callback.empty();
	header = "HTTP/1.0 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n";
	utf8 body = (jsonp ? callback + "(" : "") + "[";

	if (allowed)
	{
		streamData::streamHistory_t songHistory;
		streamData::getStreamSongHistory(sid, songHistory);

		bool first = true;
		for (streamData::streamHistory_t::const_iterator i = songHistory.begin(); i != songHistory.end(); ++i)
		{
			body += (!first ? utf8(",") : "") + "{\"playedat\":" +
					tos((*i).m_when) + "," "\"title\":\"" + escapeJSON((*i).m_title) + "\""
					",\"metadata\":" + getCurrentJSONMetadataBody((*i).m_metadata) + "}";
			first = false;
		}
	}

	return (body + "]" + (jsonp ? utf8(")") : ""));
}

void protocol_HTTPStyle::path_played_json(const streamData::streamID_t sid, const utf8 &callback,
										  const bool password, const utf8 &XFF) throw()
{
	cacheItem *item = m_jsonPlayedCache[sid];
	if (getCachedResponse(item, m_jsonPlayedLock))
	{
		return;
	}

	const bool jsonp = !callback.empty();
	utf8 header, body = getPlayedJSON(sid, header, callback, (password || (isAccessAllowed(sid, XFF) == true)));
	sendCachedResponse(item, m_jsonPlayedCache, m_jsonPlayedLock, header, body, sid, jsonp);
}

utf8 protocol_HTTPStyle::getPlayedXML(const streamData::streamID_t sid, utf8 &header, const bool allowed) throw()
{
	header = "HTTP/1.0 200 OK\r\nContent-Type:text/xml\r\n";
	utf8 body = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>"
				"<SHOUTCASTSERVER><SONGHISTORY>";

	if (allowed)
	{
		streamData::streamHistory_t songHistory;
		streamData::getStreamSongHistory(sid, songHistory);

		for (streamData::streamHistory_t::const_iterator i = songHistory.begin(); i != songHistory.end(); ++i)
		{
			body += "<SONG><PLAYEDAT>" + tos((*i).m_when) + "</PLAYEDAT>"
					"<TITLE>" + aolxml::escapeXML((*i).m_title) + "</TITLE>" +
					getCurrentXMLMetadataBody(true, (*i).m_metadata) + "</SONG>";
		}
	}

	return (body + "</SONGHISTORY></SHOUTCASTSERVER>");
}

void protocol_HTTPStyle::path_played_xml(const streamData::streamID_t sid,
										 const bool password, const utf8 &XFF) throw()
{
	cacheItem *item = m_xmlPlayedCache[sid];
	if (getCachedResponse(item, m_xmlPlayedLock))
	{
		return;
	}

	utf8 header, body = getPlayedXML(sid, header, (password || (isAccessAllowed(sid, XFF) == true)));
	sendCachedResponse(item, m_xmlPlayedCache, m_xmlPlayedLock, header, body, sid);
}

// flash x-domain file
void protocol_HTTPStyle::path_crossdomain() throw()
{
	cacheItem *item = m_crossdomainCache[0];
	if (getCachedResponse(item, m_crossdomainLock, 86400))	// 1 day
	{
		return;
	}

	// return either the current song or the next song to be played (if known)
	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:text/x-cross-domain-policy\r\n",
		 body = gOptions.getCrossDomainFile(!!m_compressed);

	// make sure there is a gzipped version available to send if signalled as supported
	if (m_compressed && !gOptions.m_crossdomainStrGZ.empty())
	{
		header += "Content-Encoding:gzip\r\n";
		body = gOptions.m_crossdomainStrGZ;
	}

	sendCachedResponse(item, m_crossdomainCache, m_crossdomainLock, header, body);
}

// flash swf file (assumes shoutcast.swf for ease of implementation / consistency)
void protocol_HTTPStyle::path_shoutcastswf() throw()
{
	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:application/x-shockwave-flash\r\n",
		 body = gOptions.getShoutcastSWF(!!m_compressed);

	// make sure there is a gzipped version available to send if signalled as supported
	if (m_compressed && !gOptions.m_shoutcastSWFStrGZ.empty())
	{
		header += "Content-Encoding:gzip\r\n";
		body = gOptions.m_crossdomainStrGZ;
	}

	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
}

void protocol_HTTPStyle::path_redirect_url(const streamData::streamID_t sid, const bool no_sid, const bool isStats) throw()
{
	streamData::streamInfo info;
	streamData::extraInfo extra;
	streamData::getStreamInfo(sid, info, extra);

	const utf8 redirectUrl = gOptions.getStreamRedirectURL((no_sid ? 0 : sid), isStats, !info.m_streamURL.empty(), !!m_compressed);
	if (redirectUrl.empty())
	{
		// try to redirect directly to the appropriate page based on the number of active streams
		streamData::streamID_t lastSID = 0;
		if (streamData::totalActiveStreams(lastSID) == (streamData::streamID_t)DEFAULT_CLIENT_STREAM_ID)
		{
			sendMessageAndClose(redirect("index.html?sid=" + tos(lastSID), !!m_compressed));
		}
		else
		{
			sendMessageAndClose(redirect("index.html", !!m_compressed));
		}
	}
	else
	{
		sendMessageAndClose(redirectUrl);
	}
}

void protocol_HTTPStyle::path_root_summary(const utf8 &XFF, const bool force) throw()
{
	// if enabled then we need to block access to this page as it's not allowed to be public
	if (gOptions.getStreamHideStats(0) == "all")
	{
		path_redirect_url(0, true, false);
	}
	else
	{
		utf8 header = MSG_NO_CLOSE_200,
			 body = "<!DOCTYPE html><html><head>"
					"<meta charset=\"utf-8\">"
					"<meta name=viewport content=\"width=device-width, initial-scale=1\">"\
					"<title>Shoutcast Server</title>"
					"<link href=\"index.css\" rel=\"stylesheet\" type=\"text/css\">"
					"<link href=\"images/favicon.ico\" rel=\"shortcut icon\" type=\"" + gOptions.faviconFileMimeType() + "\">"
					"</head><body style=\"margin:0;\">"
					"<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr>"
					"<td><div class=\"logo\">Shoutcast Streams Status</div></td>"
					"<td style=\"text-align:right;vertical-align:bottom;padding-right:0.1em;\">"
					"<a target=\"_blank\" title=\"Built: " __DATE__ "\" "
					"href=\"http://www.shoutcast.com\">Shoutcast Server v" +
					addWBR(gOptions.getVersionBuildStrings() + "/" SERV_OSNAME) + "</a>"
					"</td></tr><tr><td class=\"thr\" align=\"center\" colspan=\"2\">"
					"<table width=\"100%\" border=\"0\" cellpadding=\"5\" cellspacing=\"0\"><tr class=\"tnl\">"
					"<td align=\"left\">&nbsp;<a target=\"_blank\" href=\"http://forums.shoutcast.com/forumdisplay.php?f=140\">Help</a>&nbsp;&nbsp;|&nbsp;&nbsp;"
					" <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_Broadcaster\">Documentation</a></td>"
					"<td align=\"right\"><a href=\"admin.cgi\">Server Login&nbsp;"
					"<img border=\"0\" title=\"Server Login\nPassword Required\" alt=\"Server Login\nPassword Required"
					"\" style=\"vertical-align:middle\" src=\"images/lock.png\">"
					"</a>&nbsp;&nbsp;</td></tr></table></td></tr></table>";

		utf8 streamBody = "";
		size_t total = 0, known = streamData::totalStreams();

		if (known > 0)
		{
			--known;	// just makes it easier to know when to insert the <hr> element

			size_t inc = 0;
			streamData::streamID_t sid = 0;
			do
			{
				streamData::streamInfo info;
				streamData::extraInfo extra;
				if (streamData::getStreamInfo((sid = streamData::enumStreams(inc)), info, extra))
				{
					// increment our stream total now that we know we have one
					++total;

					utf8 tooltip = streamData::getContentType(info) + " @ " +
								   (info.m_streamBitrate > 0 ? tos(info.m_streamBitrate) :
								   "unknown") + " kbps" + (info.m_vbr ? " (VBR)" : "") +
								   ", " + sampleRateStr(info.m_streamSampleRate);

					streamBody += "<tr><td class=\"tnl\">"
								  "<a href=\"index.html?sid=" + tos(sid) + "\" title=\"" + tooltip + "\">Stream #" + tos(sid) + "</a> "
								  "<a href=\"admin.cgi?sid=" + tos(sid) + "\">(Stream Login)</a>";

					if (isAccessAllowed(sid, XFF) == true)
					{
						streamBody += " <a href=\"listen.pls?sid=" + tos(sid) + "\">"
									  "<img border=\"0\" title=\"Listen to Stream\" alt=\"Listen to Stream"
									  "\" style=\"vertical-align:middle\" src=\"images/listen.png\"></a>";

						streamBody += " <a href=\"played.html?sid=" + tos(sid) + "\">"
									  "<img border=\"0\" title=\"History\" alt=\"History"
									  "\" style=\"vertical-align:middle\" src=\"images/history.png\"></a>";
					}

					streamBody += "</td></tr><tr><td>" + (info.m_streamPublic && extra.ypConnected ?
								  "<a target=\"_blank\" href=\"http://directory.shoutcast.com/Search?query=" +
								  urlUtils::escapeURI_RFC3986(info.m_streamName) +
								  "\">" + aolxml::escapeXML(info.m_streamName) + "</a>" :
								  (info.m_streamURL.empty() ? aolxml::escapeXML(info.m_streamName) :
								  urlLink(info.m_streamURL, info.m_streamName))) + "</td></tr>"
								  "<tr><td>" + tooltip + "</td></tr>";

					if (!getHideState(sid))
					{
						stats::statsData_t data;
						stats::getStats(sid, data);

						const int maxUsers = ((info.m_streamMaxUser > 0) && (info.m_streamMaxUser < gOptions.maxUser()) ? info.m_streamMaxUser : gOptions.maxUser());
						streamBody += "<tr valign=\"top\"><td>" + tos(data.connectedListeners) +
									  (maxUsers > 0 ? " of " + tos(maxUsers) : "") + " listeners" +
									  (!maxUsers ? " (unlimited)" : "") +
									  (data.connectedListeners != data.uniqueListeners ?
									  (" (" + tos(data.uniqueListeners) + " unique)") : "") + "</td></tr>";
					}
					
					if (!info.m_currentSong.empty())
					{
						streamBody += "<tr valign=\"top\"><td>Playing Now: "
									  "<b><a href=\"currentsong?sid=" + tos(sid) + "\">" +
									  getCurrentSong(info.m_currentSong) + "</a></b></td></tr>";

						// only show if we have a valid current song
						if (!info.m_comingSoon.empty())
						{
							streamBody += "<tr valign=\"top\"><td>Playing Next: <b><a href=\"nextsong?sid=" +
										  tos(sid) + "\">" + aolxml::escapeXML(info.m_comingSoon) + "</a></b></td></tr>";
						}
					}

					if (!info.m_contentType.empty() && (info.m_uvoxDataType == MP3_DATA))
					{
						streamBody += streamData::getHTML5Player(sid);
					}

					if (inc < known)
					{
						// this allows us to only add between
						// blocks and not below the very last
						streamBody += "<tr><td><hr></td></tr>";
					}
				}
				++inc;
			}
			while (sid);
		}

		// if only 1 stream is active then behave like a v1 DNAS and show that stream's summary page
		if (total == 1 && !force)
		{
			sendMessageAndClose(redirect("index.html", !!m_compressed));
		}
		else
		{
			body += "<table cellpadding=\"5\" cellspacing=\"0\" border=\"0\" width=\"100%\">"
					"<tr><td class=\"tsp\" align=\"center\">"
					"Available Streams: " + tos(total) + "</td></tr></table><br>" +
					getNewVersionMessage() +
					"<table cellpadding=\"5\" cellspacing=\"0\" border=\"0\" "
					"width=\"100%\" style=\"padding:0 1em;\" align=\"center\">"
					// if there are any streams then we can add them in now we have the real total and finish off the page
					"<tbody align=\"center\">" + streamBody + "</tbody></table>" +
					getHTML5Remover() + getfooterStr();

			COMPRESS(header, body);
			header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
			sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
		}
	}
}

// main admin page
void protocol_HTTPStyle::path_root(const streamData::streamID_t sid, const utf8 &XFF) throw()
{
	// if enabled then we need to block access to this page as it's not allowed to be public
	if (gOptions.getStreamHideStats(sid) == "all")
	{
		path_redirect_url(sid, false, false);
	}
	else
	{
		utf8 header = MSG_NO_CLOSE_200,
			 body = getStreamHeader(sid, "Stream Status");

		if (gOptions.getSongHistorySize(sid))
		{
			body += getStreamMiddlePlayingHeader(sid);
		}

		if (isAccessAllowed(sid, XFF) == true)
		{
			body += getStreamMiddleListenHeader(sid);
		}

		body += getStreamEndHeader(sid) + "<table cellpadding=\"5\" "
				"cellspacing=\"0\" border=\"0\" width=\"100%\"><tr>"
				"<td class=\"tsp\" align=\"center\">Current Stream "
				"Information</td></tr></table><br>" + getNewVersionMessage();

		stats::statsData_t data;
		stats::getStats(sid, data);

		streamData::streamInfo info;
		streamData::extraInfo extra;
		if (streamData::getStreamInfo(sid, info, extra))
		{
			const int maxUsers = ((info.m_streamMaxUser > 0) && (info.m_streamMaxUser < gOptions.maxUser()) ? info.m_streamMaxUser : gOptions.maxUser());
			const bool isListable = streamData::isAllowedType(info.m_uvoxDataType);
			body +=	"<table cellpadding=\"2\" cellspacing=\"0\" border=\"0\" align=\"center\" style=\"padding-left:1em;\"><tr valign=\"top\">"
					"<td>Listing Status: </td>"
					"<td><b>Stream is currently up " +
					(info.m_streamPublic && isListable ? (!extra.ypConnected ? "" : utf8("and public")) : utf8("and private (not listed)")) +
					(info.m_streamPublic && isListable ? (!extra.ypConnected ? 
						(extra.ypErrorCode == -1 ? "but unable to access the Directory.<br>Listeners are allowed and the stream will act like it is private until resolved." :
						(extra.ypErrorCode == YP_MAINTENANCE_CODE ? "but received a Directory maintenance notification.<br>Listeners are allowed to connect and the stream will act like it is private." :
						(!info.m_authHash.empty() ?
						 (extra.ypErrorCode != YP_NOT_VISIBLE ? "but is waiting on a Directory response." : "but is not visible on the internet "
																"(YP error code: <a target=\"_blank\" href=\"http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2#YP_Server_Errors\">" +
																tos(extra.ypErrorCode) + "</a>).<br>Listeners are allowed and the stream will act like it is private until resolved.<br><br>"
																"Resolving this issue will allow the stream to be listed in the Shoutcast Directory.") :
													"but requires <a href=\"admin.cgi?sid=" + tos(sid) + "&amp;mode=register\">registration</a> in the Shoutcast Directory.<br>"
													"Listeners are allowed and the stream will act like it is private until resolved."))) : "") : "") + "</b></td></tr>"
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

			body += "<tr valign=\"top\"><td>Stream Name: </td><td><b>" +
					(info.m_streamPublic && extra.ypConnected ?
					 "<a target=\"_blank\" href=\"http://directory.shoutcast.com/Search?query=" +
					 urlUtils::escapeURI_RFC3986(info.m_streamName) + "\">" +
					 aolxml::escapeXML(info.m_streamName) + "</a>" :
					 aolxml::escapeXML(info.m_streamName)) + "</b></td></tr>";

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

			if (!info.m_contentType.empty() && (info.m_uvoxDataType == MP3_DATA))
			{
				body += streamData::getHTML5Player(sid);
			}

			body += "</table>";
		}
		else
		{
			body += "<table cellpadding=\"2\" cellspacing=\"0\" border=\"0\" "
					"align=\"center\"><tr valign=\"top\"><td>Stream Status: </td><td><b>";
			const utf8 movedUrl = gOptions.stream_movedUrl(sid);
			if (movedUrl.empty())
			{
				const int maxUsers = ((info.m_streamMaxUser > 0) && (info.m_streamMaxUser < gOptions.maxUser()) ? info.m_streamMaxUser : gOptions.maxUser());
				body += "Stream is currently down" + (data.connectedListeners > 0 ?
						" with " + tos(data.connectedListeners) + (maxUsers > 0 ? " of " +
						tos(maxUsers) : "") + " listeners" + (!maxUsers ? " (unlimited)" : "") +
						(data.connectedListeners != data.uniqueListeners ? (" (" +
						tos(data.uniqueListeners) + " unique)") : "") : ".") + "<br>There is no "
						"source connected or no stream is configured for stream #" + tos(sid);
			}
			else
			{
				body += "Stream has been moved to " + urlLink(movedUrl);
			}
			body += "</b></td></tr>";

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

			body += "</table>";
		}

		body += getIEFlexFix() + getHTML5Remover() + getfooterStr();

		COMPRESS(header, body);
		header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
		sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
	}
}

// redirect to stream URL
void protocol_HTTPStyle::path_home(const streamData::streamID_t sid) throw()
{
	streamData::streamInfo info;
	streamData::extraInfo extra;
	streamData::getStreamInfo(sid, info, extra);

	utf8 streamUrl = (!info.m_streamURL.empty() ?
					  ((info.m_streamURL.find(utf8("://")) == utf8::npos ? "http://" : "") +
					   info.m_streamURL) : "http://www.shoutcast.com");

	sendMessageAndClose(redirect(streamUrl, !!m_compressed));
}

void protocol_HTTPStyle::path_track(const streamData::streamID_t sid, const int mode) throw()
{
	// this is a non-compressed response as most normal titles aren't
	// long enough or contain enough data sans gzip header and footer
	// to make it worth trying to create a gzipped response for this.
	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:text/plain;charset=utf-8\r\n", currentSong, comingSoon;
	std::vector<uniString::utf8> nextSongs;
	if (streamData::getStreamNextSongs(sid, currentSong, comingSoon, nextSongs))
	{
		// return either the current song or the next song to be played (if known)
		utf8 body = (!mode ? (currentSong.empty() ? "" : currentSong) :
					 (!currentSong.empty() ? comingSoon : ""));
		header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
		sendMessageAndClose(header + (!HEAD_REQUEST ? body : ""));
	}
	else
	{
		sendMessageAndClose(header + "\r\n");
	}
}

void protocol_HTTPStyle::path_tracks_json(const streamData::streamID_t sid, const utf8 &callback) throw()
{
	cacheItem *item = m_jsonTracksCache[sid];
	if (getCachedResponse(item, m_jsonTracksLock))
	{
		return;
	}

	const bool jsonp = !callback.empty();
	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n",
		 body = (jsonp ? callback + "(" : "") + "[", currentSong, comingSoon;
	std::vector<uniString::utf8> nextSongs;
	if (streamData::getStreamNextSongs(sid, currentSong, comingSoon, nextSongs))
	{
		int index = 1;
		for (std::vector<uniString::utf8>::const_iterator i = nextSongs.begin(); i != nextSongs.end(); ++i, index++)
		{
			body += (i != nextSongs.begin() ? utf8(",") : "") +
					"{\"title\":\"" + escapeJSON((*i).empty() ? "" : (*i)) + "\"}";
		}
	}

	body += utf8("]") + (jsonp ? ")" : "");

	sendCachedResponse(item, m_jsonTracksCache, m_jsonTracksLock, header, body, sid, jsonp);
}

void protocol_HTTPStyle::path_tracks_xml(const streamData::streamID_t sid) throw()
{
	cacheItem *item = m_xmlTracksCache[sid];
	if (getCachedResponse(item, m_xmlTracksLock))
	{
		return;
	}

	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:text/xml\r\n",
		 body = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>"
				"<SHOUTCASTSERVER><NEXTSONGS>", currentSong, comingSoon;
	std::vector<uniString::utf8> nextSongs;
	if (streamData::getStreamNextSongs(sid, currentSong, comingSoon, nextSongs))
	{
		int index = 1;
		for (std::vector<uniString::utf8>::const_iterator i = nextSongs.begin(); i != nextSongs.end(); ++i, index++)
		{
			body += "<TITLE seq=\"" + tos(index) + "\">" + aolxml::escapeXML(*i) + "</TITLE>";
		}
	}

	body += "</NEXTSONGS></SHOUTCASTSERVER>";

	sendCachedResponse(item, m_xmlTracksCache, m_xmlTracksLock, header, body, sid);
}

uniString::utf8 protocol_HTTPStyle::getCurrentXMLMetadataBody(const bool mode, const utf8 &metadata)
{
	utf8 body = (!mode ? "<SONGMETADATA>" : "<METADATA>");

	if (!metadata.empty())
	{
		// strip out the metadata between the <metadata /> part
		// so it'll instead go in our own <songmetadata /> part
		const utf8::size_type pos1 = metadata.find(METADATA), // 10 chars
							  pos2 = metadata.find(E_METADATA); // 11 chars
		if ((pos1 != utf8::npos) && (pos2 != utf8::npos))
		{
			body += stripWhitespace(metadata.substr(pos1 + 10, pos2 - pos1 - 10));
		}
	}

	return (body + (!mode ? "</SONGMETADATA>" : "</METADATA>"));
}

void protocol_HTTPStyle::path_current_metadata_xml(const streamData::streamID_t sid) throw()
{
	cacheItem *item = m_xmlMetadataCache[sid];
	if (getCachedResponse(item, m_xmlMetadataLock))
	{
		return;
	}

	streamData::streamHistory_t songHistory;
	streamData::getStreamSongHistory(sid, songHistory);

	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:text/xml\r\n",
		 body = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>"
		 "<SHOUTCASTSERVER>" + getCurrentXMLMetadataBody(false,
		 (!songHistory.empty() ? songHistory[0].m_metadata : "")) +
		 "</SHOUTCASTSERVER>";

	sendCachedResponse(item, m_xmlMetadataCache, m_xmlMetadataLock, header, body, sid);
}

uniString::utf8 protocol_HTTPStyle::getCurrentJSONMetadataBody(const utf8 &metadata)
{
	utf8 body = "{";

	if (!metadata.empty())
	{
		// strip out the metadata between the <metadata /> part
		// so it'll instead go in our own <songmetadata /> part
		const utf8::size_type pos1 = metadata.find(METADATA), // 10 chars
							  pos2 = metadata.find(E_METADATA); // 11 chars
		if ((pos1 != utf8::npos) && (pos2 != utf8::npos))
		{
			// take the xml metadata, create a new xml block from it
			const utf8 meta = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?><meta>" +
							  stripWhitespace(metadata.substr(pos1 + 10, pos2 - pos1 - 10)) + "</meta>";

			// then split down into node+text to be able to output
			aolxml::node *n = 0;
			try
			{
				n = aolxml::node::parse(meta);
				if (n)
				{
					for (aolxml::node::const_childIterator_t i = n->childrenBegin(); i != n->childrenEnd(); ++i)
					{
						const utf8 node = toLower((*i)->name());
						body += (i != n->childrenBegin() ? utf8(",") : "") +
								"\"" + escapeJSON(node) +
								"\":" + ((*i)->childrenEmpty() ? "\"" + escapeJSON((*i)->pcdata()) + "\"" : "");
						// if we've got a child block it should be <extension/> and so parse out
						if (!(*i)->childrenEmpty())
						{
							body += "[";
							for (aolxml::node::const_childIterator_t c = (*i)->childrenBegin(); c != (*i)->childrenEnd(); ++c)
							{
								const utf8 nodec = toLower((*c)->name()) + (*c)->findAttributeString("seq");
								body += (c != (*i)->childrenBegin() ? utf8(",") : "") +
										"{\"" + escapeJSON(nodec) +
										"\":" + ((*c)->childrenEmpty() ? "\"" + escapeJSON((*c)->pcdata()) + "\"" : "") + "}";
							}
							body += "]";
						}
					}
				}
			}
			catch(const exception &)
			{
			}
			forget(n);
		}
	}

	return (body + "}");
}

void protocol_HTTPStyle::path_current_metadata_json(const streamData::streamID_t sid, const utf8 &callback) throw()
{
	cacheItem *item = m_jsonMetadataCache[sid];
	if (getCachedResponse(item, m_jsonMetadataLock))
	{
		return;
	}

	streamData::streamHistory_t songHistory;
	streamData::getStreamSongHistory(sid, songHistory);

	const bool jsonp = !callback.empty();
	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n",
		 body = (jsonp ? callback + "(" : "") +
				getCurrentJSONMetadataBody((!songHistory.empty() ? songHistory[0].m_metadata : "")) +
				(jsonp ? ")" : "");

	sendCachedResponse(item, m_jsonMetadataCache, m_jsonMetadataLock, header, body, sid, jsonp);
}

utf8 protocol_HTTPStyle::getStatsXMLBody(const streamData::streamID_t sid, const bool single,
										 const bool proceed, const socketOps::tSOCKET m_socket,
										 stats::statsData_t& data, const bool no_copy)
{
	streamData::streamInfo info;
	streamData::extraInfo extra;
	streamData::getStreamInfo(sid, info, extra);

	if (!no_copy)
	{
		stats::getStats(sid, data);
	}

	size_t maxUsers = gOptions.maxUser();

	config::streamConfig stream;
	if (gOptions.getStreamConfig(stream, sid))
	{
		maxUsers = ((stream.m_maxStreamUser > 0) && (stream.m_maxStreamUser < gOptions.maxUser()) ? stream.m_maxStreamUser : gOptions.maxUser());
	}

	utf8 body = "<CURRENTLISTENERS>" + tos(data.connectedListeners) + "</CURRENTLISTENERS>"
				"<PEAKLISTENERS>" + tos(data.peakListeners) + "</PEAKLISTENERS>"
				"<MAXLISTENERS>" + (maxUsers > 0 ? tos(maxUsers) : "UNLIMITED") + "</MAXLISTENERS>"
				"<UNIQUELISTENERS>" + tos(data.uniqueListeners) + "</UNIQUELISTENERS>"
				"<AVERAGETIME>" + tos(data.avgUserListenTime) + "</AVERAGETIME>";

	for (int n = 0; n < 5; n++)
	{
		const utf8 num = (n ? tos(n + 1) : "");
		body += "<SERVERGENRE" + num + ">" + aolxml::escapeXML(info.m_streamGenre[n]) + "</SERVERGENRE" + num + ">";
	}

	body += "<SERVERURL>" + aolxml::escapeXML(info.m_streamURL) + "</SERVERURL>"
			"<SERVERTITLE>" + aolxml::escapeXML(info.m_streamName) + "</SERVERTITLE>"
			"<SONGTITLE>" + aolxml::escapeXML(info.m_currentSong.empty() ? "" : info.m_currentSong) + "</SONGTITLE>";

	if (!info.m_comingSoon.empty())
	{
		body += "<NEXTTITLE>" + aolxml::escapeXML(info.m_comingSoon) + "</NEXTTITLE>";
	}

	if (!info.m_streamUser.empty())
	{
		body += "<DJ>" + aolxml::escapeXML(info.m_streamUser) + "</DJ>";
	}

	if (!info.m_currentURL.empty())
	{
		body += "<SONGURL>" + aolxml::escapeXML(info.m_currentURL) + "</SONGURL>";
	}

	body += "<STREAMHITS>" + tos(data.totalStreamHits) + "</STREAMHITS>"
			"<STREAMSTATUS>" + (extra.isConnected ? "1" : "0") + "</STREAMSTATUS>"
			"<BACKUPSTATUS>" + (extra.isBackup ? "1" : "0") + "</BACKUPSTATUS>"
			"<STREAMLISTED>" + (extra.ypConnected ? "1" : "0") + "</STREAMLISTED>";
	if (!extra.ypConnected)
	{
		body += "<STREAMLISTEDERROR>" + tos(extra.ypErrorCode) + "</STREAMLISTEDERROR>";
	}

	// if a source is connected and we have a valid password then output this
	if (proceed && extra.isConnected)
	{
		// strip down the source address for display output to an appropriate output based on settings
		utf8 srcAddr = (extra.isBackup ? info.m_backupURL : (extra.isRelay ? info.m_relayURL : info.m_srcAddr));
		if (gOptions.nameLookups())
		{
			if (!extra.isBackup && !extra.isRelay)
			{
				u_short port = 0;
				string addr, hostName;
				socketOps::getpeername(m_socket, addr, port);

				string src = (extra.isBackup ? info.m_backupURL : (extra.isRelay ? info.m_relayURL : info.m_srcAddr)).hideAsString();
				hostName = src;
				if (!socketOps::addressToHostName(addr, port, hostName))
				{
					srcAddr = hostName + " (" + (src) + ")";
				}
			}
		}

		body += "<STREAMSOURCE>" + aolxml::escapeXML(srcAddr) + "</STREAMSOURCE>";

		if (!info.m_backupURL.empty())
		{
			body += "<STREAMBACKUP>" + aolxml::escapeXML(info.m_backupURL) + "</STREAMBACKUP>";
		}
	}

	body += "<STREAMPATH>" + aolxml::escapeXML(getStreamPath(sid)) + "</STREAMPATH>" +
			(extra.isConnected ? ("<STREAMUPTIME>" + tos(::time(NULL) - streamData::getStreamUptime(sid)) + "</STREAMUPTIME>") : "") +
			"<BITRATE>" + tos(info.m_streamBitrate) + "</BITRATE>"
			"<SAMPLERATE>" + tos(info.m_streamSampleRate) + "</SAMPLERATE>" +
			(info.m_vbr ? "<VBR>1</VBR>" : "") +
			"<CONTENT>" + aolxml::escapeXML(info.m_contentType) + "</CONTENT>";

	if (single)
	{
		body += "<VERSION>" + aolxml::escapeXML(gOptions.getVersionBuildStrings()) + " (" SERV_OSNAME ")</VERSION>";
	}

	return body;
}

void protocol_HTTPStyle::path_stats_xml(streamData::streamID_t sid, bool proceed) throw()
{
	cacheItem *item = m_xmlStatsCache[sid];
	if (getCachedResponse(item, m_xmlStatsLock))
	{
		return;
	}

	stats::statsData_t data;
	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:text/xml\r\n",
		 body = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>"
				"<SHOUTCASTSERVER>" + getStatsXMLBody(sid, true, proceed, m_socket, data) + "</SHOUTCASTSERVER>";

	sendCachedResponse(item, m_xmlStatsCache, m_xmlStatsLock, header, body, sid);
}

void protocol_HTTPStyle::path_statistics_xml(streamData::streamID_t sid, bool proceed) throw()
{
	cacheItem *item = m_xmlStatisticsCache[sid];
	if (getCachedResponse(item, m_xmlStatisticsLock))
	{
		return;
	}

	// this will generate a DNAS wide statistics report making it clearer on what is / isn't going on (requested by WaveStreaming)
	size_t totalConnectedListeners = 0,
		   totalPeakListeners = 0,
		   totalMoved = 0;
	time_t totalAvgUserListenTime = 0;

	streamData::streamIDs_t streamIds = streamData::getStreamIds(2);
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		if ((*i).second.m_streamID)
		{
			streamIds.insert((*i).second.m_streamID);
		}
	}

	utf8 block = "";
	for (streamData::streamIDs_t::const_iterator i = streamIds.begin(); i != streamIds.end(); ++i)
	{
		const utf8 movedUrl = gOptions.stream_movedUrl((*i));
		if (movedUrl.empty())
		{
			stats::statsData_t data;
			stats::getStats((*i), data);

			// increment the system wide totals
			totalConnectedListeners += data.connectedListeners;
			totalPeakListeners += data.peakListeners;
			totalAvgUserListenTime += data.avgUserListenTime;

			block += "<STREAM id=\"" + tos((*i)) + "\">" + getStatsXMLBody((*i), false, proceed, m_socket, data, true) + "</STREAM>";
		}
		else
		{
			++totalMoved;
		}
	}

	const int maxUsers = gOptions.maxUser();
	streamData::streamID_t lastSID = 0;
	const utf8 main = "<STREAMSTATS>"
					  "<TOTALSTREAMS>" + tos(streamIds.size() - totalMoved) + "</TOTALSTREAMS>"
					  "<ACTIVESTREAMS>" + tos(streamData::totalActiveStreams(lastSID)) + "</ACTIVESTREAMS>"
					  "<CURRENTLISTENERS>" + tos(totalConnectedListeners) + "</CURRENTLISTENERS>"
					  "<PEAKLISTENERS>" + tos(totalPeakListeners) + "</PEAKLISTENERS>"
					  "<MAXLISTENERS>" + (maxUsers > 0 ? tos(maxUsers) : "UNLIMITED") + "</MAXLISTENERS>"
					  "<UNIQUELISTENERS>" + tos(stats::getTotalUniqueListeners()) + "</UNIQUELISTENERS>"
					  "<AVERAGETIME>" + tos(totalConnectedListeners > 0 ?
					  (totalAvgUserListenTime / totalConnectedListeners) : 0) + "</AVERAGETIME>"
					  "<VERSION>" + aolxml::escapeXML(gOptions.getVersionBuildStrings()) + " (" SERV_OSNAME ")</VERSION>";

	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:text/xml\r\n",
		 body = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>"
				"<SHOUTCASTSERVER>" + main + block + "</STREAMSTATS></SHOUTCASTSERVER>";

	sendCachedResponse(item, m_xmlStatisticsCache, m_xmlStatisticsLock, header, body, sid);
}

utf8 protocol_HTTPStyle::getStatsJSONBody(const streamData::streamID_t sid, const bool single,
										  const bool proceed, const socketOps::tSOCKET m_socket,
										  stats::statsData_t& data, const bool no_copy)
{
	streamData::streamInfo info;
	streamData::extraInfo extra;
	streamData::getStreamInfo(sid, info, extra);

	if (!no_copy)
	{
		stats::getStats(sid, data);
	}

	size_t maxUsers = gOptions.maxUser();

	config::streamConfig stream;
	if (gOptions.getStreamConfig(stream, sid))
	{
		maxUsers = ((stream.m_maxStreamUser > 0) && (stream.m_maxStreamUser < gOptions.maxUser()) ? stream.m_maxStreamUser : gOptions.maxUser());
	}

	utf8 body = (!single ? "\"id\":" + tos(sid) + "," : "") +
				"\"currentlisteners\":" + tos(data.connectedListeners) + ","
				"\"peaklisteners\":" + tos(data.peakListeners) + ","
				"\"maxlisteners\":" + (maxUsers > 0 ? tos(maxUsers) : "\"unlimited\"") + ","
				"\"uniquelisteners\":" + tos(data.uniqueListeners) + ","
				"\"averagetime\":" + tos(data.avgUserListenTime) + ",";

	for (int n = 0; n < 5; n++)
	{
		const utf8 num = (n ? tos(n + 1) : "");
		body += "\"servergenre" + num + "\":\"" + aolxml::escapeXML(info.m_streamGenre[n]) + "\",";
	}

	body += "\"serverurl\":\"" + escapeJSON(info.m_streamURL) + "\","
			"\"servertitle\":\"" + escapeJSON(info.m_streamName) + "\","
			"\"songtitle\":\"" + escapeJSON(info.m_currentSong.empty() ? "" : info.m_currentSong) + "\",";

	if (!info.m_comingSoon.empty())
	{
		body += "\"nexttitle\":\"" + escapeJSON(info.m_comingSoon.empty() ? "" : info.m_comingSoon) + "\",";
	}

	if (!info.m_streamUser.empty())
	{
		body += "\"dj\":\"" + escapeJSON(info.m_streamUser) + "\",";
	}

	if (!info.m_streamUser.empty())
	{
		body += "\"songurl\":\"" + escapeJSON(info.m_currentURL) + "\",";
	}

	body += "\"streamhits\":" + tos(data.totalStreamHits) + ","
			"\"streamstatus\":" + (extra.isConnected ? "1" : "0") + ","
			"\"backupstatus\":" + (extra.isBackup ? "1" : "0") + ","
			"\"streamlisted\":" + (extra.ypConnected ? "1" : "0") + ",";
	if (!extra.ypConnected)
	{
		body += "\"streamlistederror\":" + tos(extra.ypErrorCode) + ",";
	}

	// if a source is connected and we have a valid password then output this
	if (proceed && extra.isConnected)
	{
		// strip down the source address for display output to an appropriate output based on settings
		utf8 srcAddr = (extra.isBackup ? info.m_backupURL : (extra.isRelay ? info.m_relayURL : info.m_srcAddr));
		if (gOptions.nameLookups())
		{
			if (!extra.isBackup && !extra.isRelay)
			{
				u_short port = 0;
				string addr, hostName;
				socketOps::getpeername(m_socket, addr, port);

				string src = (extra.isBackup ? info.m_backupURL : (extra.isRelay ? info.m_relayURL : info.m_srcAddr)).hideAsString();
				hostName = src;
				if (!socketOps::addressToHostName(addr, port, hostName))
				{
					srcAddr = hostName + " (" + (src) + ")";
				}
			}
		}

		body += "\"streamsource\":\"" + escapeJSON(srcAddr) + "\",";

		if (!info.m_backupURL.empty())
		{
			body += "\"streambackup\":\"" + escapeJSON(info.m_backupURL) + "\",";
		}
	}

	body += "\"streampath\":\"" + escapeJSON(getStreamPath(sid)) + "\"," +
			(extra.isConnected ? ("\"streamuptime\":" + tos(::time(NULL) - streamData::getStreamUptime(sid)) + ",") : "") +
			"\"bitrate\":\"" + tos(info.m_streamBitrate) + "\","
			"\"samplerate\":\"" + tos(info.m_streamSampleRate) + "\"," +
			(info.m_vbr ? "\"vbr\":\"1\"," : "") +
			"\"content\":\"" + escapeJSON(info.m_contentType) + "\"";

	if (single)
	{
		body += ",\"version\":\"" + escapeJSON(gOptions.getVersionBuildStrings()) + " (" SERV_OSNAME ")\"";
	}

	return body;
}

void protocol_HTTPStyle::path_stats_json(const streamData::streamID_t sid, const bool proceed, const utf8 &callback) throw()
{
	const bool jsonp = !callback.empty();
	cacheItem *item = m_jsonStatsCache[sid];
	if (getCachedResponse(item, m_jsonStatsLock))
	{
		return;
	}

	stats::statsData_t data;
	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n",
		 body = (jsonp ? callback + "(" : "") + "{" +
				getStatsJSONBody(sid, true, proceed, m_socket, data) +
				"}" + (jsonp ? ")" : "");

	sendCachedResponse(item, m_jsonStatsCache, m_jsonStatsLock, header, body, sid, jsonp);
}

void protocol_HTTPStyle::path_statistics_json(const streamData::streamID_t sid, const bool proceed, const utf8 &callback) throw()
{
	const bool jsonp = !callback.empty();
	cacheItem *item = m_jsonStatisticsCache[sid];
	if (getCachedResponse(item, m_jsonStatisticsLock))
	{
		return;
	}

	// this will generate a DNAS wide statistics report making it clearer on what is / isn't going on (requested by WaveStreaming)
	size_t totalConnectedListeners = 0,
		   totalPeakListeners = 0,
		   totalMoved = 0;
	time_t totalAvgUserListenTime = 0;

	streamData::streamIDs_t streamIds = streamData::getStreamIds(2);
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);
	for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
	{
		if ((*i).second.m_streamID)
		{
			streamIds.insert((*i).second.m_streamID);
		}
	}

	utf8 block = "";
	bool read = false;
	for (streamData::streamIDs_t::const_iterator i = streamIds.begin(); i != streamIds.end(); ++i)
	{
		const utf8 movedUrl = gOptions.stream_movedUrl((*i));
		if (movedUrl.empty())
		{
			stats::statsData_t data;
			stats::getStats((*i), data);

			// increment the system wide totals
			totalConnectedListeners += data.connectedListeners;
			totalPeakListeners += data.peakListeners;
			totalAvgUserListenTime += data.avgUserListenTime;

			block += (read ? utf8(",") : "") + "{" + getStatsJSONBody((*i), false, proceed, m_socket, data, true) +  + "}";
			read = true;
		}
		else
		{
			++totalMoved;
		}
	}

	streamData::streamID_t lastSID = 0;
	const size_t total = streamIds.size() - totalMoved,
				 activeTotal = streamData::totalActiveStreams(lastSID),
				 maxUser = gOptions.maxUser();
	const utf8 main = "\"totalstreams\":" + tos(total) + ","
					  "\"activestreams\":" + tos(activeTotal) + ","
					  "\"currentlisteners\":" + tos(totalConnectedListeners) + ","
					  "\"peaklisteners\":" + tos(totalPeakListeners) + ","
					  "\"maxlisteners\":" + (maxUser > 0 ? tos(maxUser) : "\"unlimited\"") + ","
					  "\"uniquelisteners\":" + tos(stats::getTotalUniqueListeners()) + ","
					  "\"averagetime\":" + tos(totalConnectedListeners > 0 ? (totalAvgUserListenTime / totalConnectedListeners) : 0) + ","
					  "\"version\":\"" + escapeJSON(gOptions.getVersionBuildStrings()) + " (" SERV_OSNAME ")\"" +
					  (total > 0 ? ",\"streams\":[" : "");

	utf8 header = "HTTP/1.0 200 OK\r\nContent-Type:application/json;charset=utf-8\r\n",
		 body = (jsonp ? callback + "(" : "") +
				"{" + main + block + (total > 0 ? "]" : "") + "}" +
				(jsonp ? ")" : "");

	sendCachedResponse(item, m_jsonStatisticsCache, m_jsonStatisticsLock, header, body, sid, jsonp);
}

void protocol_HTTPStyle::path_art(const streamData::streamID_t sid, int mode) throw()
{
	cacheItem *item = (!mode ? m_streamArtCache[sid] : m_playingArtCache[sid]);
	if (getCachedResponse(item, (!mode ? m_streamArtLock : m_playingArtLock)))
	{
		return;
	}

	utf8 header = "HTTP/1.0 200 OK\r\n", body;
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

	sendCachedResponse(item, (!mode ? m_streamArtCache : m_playingArtCache),
					   (!mode ? m_streamArtLock : m_playingArtLock), header,
					   body, sid, false, false);
}
