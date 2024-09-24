#ifdef _WIN32
#include <winsock2.h>
#endif
#include <assert.h>
#include "cpucount.h"
#include "yp2.h"
#include "updater.h"
#include "stl/stringUtils.h"
#include "file/fileUtils.h"
#include "aolxml/aolxml.h"
#include "bandwidth.h"
#include "streamData.h"
#include "stats.h"
#include "metadata.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

static AOL_namespace::mutex g_YP2Lock;
static yp2 *gYP2 = 0;
extern int auth_enabled;

// for production we don't need to prettify the xml for the YP
#ifdef XML_DEBUG
#define EOL "\n"
#else
#define EOL ""
#endif

#define DEBUG_LOG(...) do { if (gOptions.yp2Debug()) DLOG(__VA_ARGS__); } while (0)

#define ADD_CMD 0
#define REM_CMD 1
#define UPD_CMD 2

#define REQUEST_KEY			m_userData_u
#define REQUEST_CMD			m_userData_i
#define REQUEST_USR			m_userData_p

uniString::utf8 yp2::logString(const yp2::yp2SessionKey &key) throw() { return "key=" + tos(key); }

yp2::stationInfo::stationInfo() throw() : m_updateFrequency(gOptions.ypReportInterval()), m_advertMode(0)
{
    m_allowSSL = -1;
    m_allowAllFormats = -1;
    m_allowMaxBitrate = 0;
    m_allowBackupURL = -1;
}


///////////////////////////////////////////////////////////////////////////////////////////
void yp2::updateYPBandWidthSent(webClient::request r) throw()
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

	bandWidth::updateAmount(bandWidth::YP_SENT, total);
}

size_t yp2::requestsInQueue() throw()
{
	stackLock sml(g_YP2Lock);

	return (gYP2 ? gYP2->queueEntries() : 0);
}

yp2::yp2() throw() : webClient(YP2_LOGNAME)
{
	stackLock sml(g_YP2Lock);

	gYP2 = this;
	m_addFailed = false;
	m_ignoreAdd = false;
	m_errorCode = 200;
	m_errorMsg = "";
	m_errorMsgExtra = "";
	m_sessionKeyCounter = 1;
}

yp2::~yp2() throw() 
{
	stackLock sml(g_YP2Lock);

	gYP2 = 0;
}

void yp2::getAddUpdateBody(webClient::request& r, const ypInfo& info, const bool update) throw()
{
	r.m_method = webClient::request::POST;
	r.m_addr = gOptions.ypAddr();
	r.m_port = gOptions.ypPort();
	r.m_path = gOptions.ypPath();
	r.m_nonBlocking = 1;
	r.m_contentType = "text/xml;charset=utf-8";
	r.REQUEST_KEY = info.key;
	r.REQUEST_CMD = (!update ? ADD_CMD : UPD_CMD);
	r.REQUEST_USR = reinterpret_cast<void*>(info.streamPublic);
	r.m_sid = (!info.sid ? DEFAULT_CLIENT_STREAM_ID : info.sid);
	r.m_XFF = metrics::metrics_verifyDestIP(gOptions);

	utf8 id;
	if (update)
	{
		yp2_server_state &yp2ss = m_serverMap[info.key];
		id = ("<id>" + yp2ss.m_serverID.escapeXML() + "</id>");
	}

	const utf8 xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" EOL\
					 "<yp version=\"2\">" EOL\
					 "<cmd op=\"" + utf8(!update ? "add" : "update") + "\" seq=\"1\">" EOL\
					 "<tstamp>" + tos(::time(NULL)) + "</tstamp>" EOL\
					 "<url>" EOL\
					 "<port>" + tos(g_portForClients) + "</port>" EOL\
					 "<path>" + getStreamPath(info.sid).escapeXML() + "</path>" EOL\
					 "<sid>" + tos(r.m_sid) + "</sid>" EOL\
					 "<alt>" + gOptions.m_usedAlternatePorts + "</alt>" EOL\
					 "</url>" EOL\
					 + ((!info.streamPublic && !gOptions.cdn().empty()) ? "<cdn>1</cdn>" : "") + EOL\
					 "<bitrate>" + tos(info.bitrate) + "</bitrate>" EOL\
					 "<samplerate>" + tos(info.samplerate) + "</samplerate>" EOL\
					 "<vbr>" + tos(info.vbr) + "</vbr>" EOL\
					 "<mimetype>" + info.mimeType.escapeXML() + "</mimetype>" EOL\
					 + (update ? id : "") + EOL\
					 "<sa>" + tos(info.streamArtwork) + "</sa>" EOL\
					 "<pa>" + tos(info.playingArtwork) + "</pa>" EOL\
					 "<peak>" + tos(info.peakClientConnections) + "</peak>" EOL\
					 "<maxclients>" + (info.maxClientConnections > 0 ? tos(info.maxClientConnections) : "unlimited") + "</maxclients>" EOL\
					 "<authhash>" + info.authhash.escapeXML() + "</authhash>" EOL\
					 "<dj>" + info.sourceUser.escapeXML() + "</dj>" EOL\
					 "<source>" + (!info.sourceIdent.empty() ? info.sourceIdent.escapeXML() : "Legacy / Unknown") + "</source>" EOL\
					 "<dnas>" + gOptions.getVersionBuildStrings().c_str() + "/" SERV_OSNAME"</dnas>" EOL\
					 "<cpu>" + tos(gOptions.getCPUCount()) + "/" + tos(cpucount()) + "</cpu>" EOL\
					 + (!info.relayURL.empty() ? "<relayurl>" + info.relayURL.escapeXML() + "</relayurl>" : "") + EOL\
					 + (update ? "<stats>" EOL\
					 "<listeners>" + tos(info.numListeners) + "</listeners>" EOL\
					 "<uniquelisteners>" + tos(info.numUniqueListeners) + "</uniquelisteners>" EOL\
					 "<avglistentime>" + tos(info.avgUserListenTime) + "</avglistentime>" EOL\
					 "<newsessions>" + tos(info.numberOfClientsConnectedMoreThanFiveMinutes) + "</newsessions>" EOL\
					 "<connects>" + tos(info.numberOfClientConnectsSinceLastUpdate) + "</connects>" EOL\
					 "</stats>" : "") + EOL\
					 + (!info.songMetadataForYP2.empty() ? METADATA + EOL +
						info.songMetadataForYP2 + EOL + E_METADATA : "") + EOL\
					 "</cmd>" EOL"</yp>";

	r.m_content.insert(r.m_content.end(), xml.begin(), xml.end());
	updateYPBandWidthSent(r);

	if (!update)
	{
		yp2_server_state &yp2ss = m_serverMap[info.key];
		DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " add for " + logString(info.key) +
				  " [" + tos(yp2ss.m_addRetries) +  "]" + eol() + xml);

		++yp2ss.m_addRetries;
		yp2ss.m_addState = ADD_PENDING;
		yp2ss.m_lastOperationTime = ::time(NULL);

		queueRequest((yp2ss.m_lastRequest = r));
	}
	else
	{
		DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " " + logString(info.key) + eol() + xml);
		queueRequest(r);
	}
}

/*
	It is called before each update to confirm that the connection to yp is still good.
	Normally this wouldn't really be necessary, but if yp goes down, we want to re-establish
	without having to restart sc_serv
*/
yp2::yp2SessionKey yp2::pvt_add(ypInfo& info) throw(exception)
{
	if (info.key == INVALID_SESSION_KEY)
	{
		info.key = m_sessionKeyCounter;
		while (++m_sessionKeyCounter == INVALID_SESSION_KEY) {;}
	}

	stackLock sml(m_serverMapLock);

	if (gOptions.yp2Debug())
	{
		// either is okay since this is called continuouslly
		serverMap_t::const_iterator i = m_serverMap.find(info.key);
		if (i == m_serverMap.end())
		{
			DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " No entry for " + logString(info.key));
		}
	}

	yp2_server_state &yp2ss = m_serverMap[info.key];
	yp2ss.m_authHash = info.authhash;
	yp2ss.m_sessionKey = info.key;
	yp2ss.m_streamID = info.sid;

	assert(yp2ss.m_addState != ADD_REMOVEISSUED);

	bool must_delay_add = false;
	if (yp2ss.m_addState == ADD_FAILED)
	{
		must_delay_add = true;

		static time_t ADD_RETRY_INTERVAL(gOptions.ypTimeout());
		time_t t = ::time(NULL);
		if (t < yp2ss.m_lastOperationTime) // rollover compensation
		{
			yp2ss.m_lastOperationTime = t;
		}
		if (t - yp2ss.m_lastOperationTime >= ADD_RETRY_INTERVAL)
		{
			must_delay_add = false;
		}
	}

	// if state is NONE (just created) or failed, we try again
	if (((yp2ss.m_addState == ADD_NONE) || (yp2ss.m_addState == ADD_FAILED) ||
		 (yp2ss.m_addState == UPDATE_FAILED)) && (!must_delay_add))
	{	
		webClient::request r;
		getAddUpdateBody(r, info, false);
		return info.key;
	}
	return INVALID_SESSION_KEY;
}

/*
	remove() is called when a stream is destroyed, and this can lead to a number of cases
	that interact with add

	1) ADD_NONE - no problem here. Nothing has been done, nothing needs to be done. just remove table entry
	2) ADD_FAILED - again no problem. remove table entry
	3) ADD_SUCCEDED - again no problem. Send remove to yp(). Response function will remove table entry
	4) ADD_PENDING - This is trouble. We cannot queue a remove() because we don't have the
			serverID yet. We also don't know what will happen with the pending add.
			To deal with this we transition to the ADD_REMOVEISSUED state.
			If add() succeeds and finds this state, we assume that
			remove() occured, and then we post the actual remove to yp. If the subsequent add fails, it must
			remove the table entry
*/	
void yp2::pvt_queue_remove(const ypInfo& info) throw()
{
	if (info.key != INVALID_SESSION_KEY)
	{
		webClient::request r;
		r.m_method = webClient::request::POST;
		r.m_addr = gOptions.ypAddr();
		r.m_port = gOptions.ypPort();
		r.m_path = gOptions.ypPath();
		r.m_nonBlocking = 0;
		r.m_contentType = "text/xml;charset=utf-8";
		r.REQUEST_KEY = info.key;
		r.REQUEST_CMD = REM_CMD;
		r.REQUEST_USR = reinterpret_cast<void*>(info.streamPublic);
		r.m_sid = (!info.sid ? DEFAULT_CLIENT_STREAM_ID : info.sid);

		yp2_server_state &yp2ss = m_serverMap[info.key];
		assert(yp2ss.m_sessionKey == info.key);

		utf8 cdn = ((!info.streamPublic && !gOptions.cdn().empty()) ? "<cdn>1</cdn>" EOL : "");
		utf8 xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" EOL\
				   "<yp version=\"2\">" EOL\
				   "<cmd op=\"remove\" seq=\"1\">" EOL\
				   "<tstamp>" + tos(::time(NULL)) + "</tstamp>" EOL\
				   "<url>" EOL\
				   "<port>" + tos(g_portForClients) + "</port>" EOL\
				   "<path>" + getStreamPath(info.sid).escapeXML() + "</path>" EOL\
				   "<sid>" + tos(r.m_sid) + "</sid>" EOL\
				   "<alt>" + gOptions.m_usedAlternatePorts + "</alt>" EOL\
				   "</url>" EOL\
				   + cdn +\
				   "<id>" + yp2ss.m_serverID.escapeXML() + "</id>" EOL\
				   + (info.peakClientConnections != (size_t)-1 ? "<peak>" + tos(info.peakClientConnections) + "</peak>" : "") + EOL\
				   + (info.maxClientConnections != (size_t)-1 ? "<maxclients>" +
					 (info.maxClientConnections > 0 ? tos(info.maxClientConnections) : "unlimited") + "</maxclients>" : "") + EOL\
				   "<authhash>" + info.authhash.escapeXML() + "</authhash>" EOL\
				   "</cmd>" EOL\
				   "</yp>";

		DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " " + logString(info.key) + eol() + xml);

		r.m_content.insert(r.m_content.end(),xml.begin(),xml.end());

		updateYPBandWidthSent(r);
		queueRequest(r);
	}
}

void yp2::pvt_remove(ypInfo& info) throw(exception)
{
	assert(info.key != INVALID_SESSION_KEY);

	stackLock sml(m_serverMapLock);

	serverMap_t::iterator i = m_serverMap.find(info.key);
	if (i == m_serverMap.end())
	{
		throwEx<runtime_error>("Internal error. " + logString(info.key) +
							   " does not exist in server map.");
	}

	yp2_server_state &yp2ss = (*i).second;

	assert(yp2ss.m_authHash == info.authhash);
	assert(yp2ss.m_sessionKey == info.key);

	metrics::metrics_stream_down (yp2ss.m_streamID, yp2ss.m_stationInfo.m_radionomyID,
		yp2ss.m_serverID, yp2ss.m_stationInfo.m_publicIP, info.streamStartTime);

	switch (yp2ss.m_addState)
	{
		case ADD_NONE:
		case ADD_FAILED:
		case UPDATE_FAILED:
		{
			DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " erasing map entry for key=" + tos(info.key));
			m_serverMap.erase(info.key);
			break;
		}

		case ADD_PENDING:
		{
			yp2ss.m_addState = ADD_REMOVEISSUED;
			break;
		}

		case ADD_SUCCEEDED:
		{
			if (yp2ss.m_serverID.empty())
			{
				throwEx<runtime_error>("Internal error. serverID for " +
									   logString(info.key) + " is empty.");
			}

			DEBUG_LOG(YP2_LOGNAME "Sending remove for " + (*i).second.logString());
			pvt_queue_remove(info);
			break;
		}

		case ADD_REMOVEISSUED:
		default:
		{
			throwEx<runtime_error>(YP2_LOGNAME + string(__FUNCTION__) + " Internal error. Bad state");
			break;
		}
	}
}

// only send an update if everything is ok (ADD_SUCCEEDED)
yp2::updateResult yp2::pvt_update(const ypInfo& info) throw(exception)
{
	assert(info.key != INVALID_SESSION_KEY);

	stackLock sml(m_serverMapLock);

	serverMap_t::const_iterator i = m_serverMap.find(info.key);
	if (i == m_serverMap.end())
	{
		throwEx<runtime_error>("Internal error. " + logString(info.key) + " does not exist in server map.");
	}

	yp2::updateResult result;
	if ((*i).second.m_addState == ADD_SUCCEEDED)
	{
		if ((*i).second.m_serverID.empty())
		{
			throwEx<runtime_error>("Internal error. serverID for " + logString(info.key) + " is empty.");
		}

		webClient::request r;
		getAddUpdateBody(r, info, true);
		result.m_requestQueued = 1;
	}
	else
	{
		// tinkered this after build 10 to prevent a mass of debug spam when we have an
		// add fail so we will keep a track of the previous state and if it's different
		assert(m_serverMap.find(info.key) != m_serverMap.end());
		yp2_server_state &yp2ss = m_serverMap[info.key];
		assert(yp2ss.m_sessionKey == info.key);

		static time_t ADD_RETRY_INTERVAL(gOptions.ypTimeout());
		time_t t = ::time(NULL);
		if (t - yp2ss.m_lastOperationTime >= ADD_RETRY_INTERVAL)
		{
			DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " add still pending for " + logString(info.key) +
					  " [" + tos((int)(*i).second.m_addState) + "] [" + tos((*i).second.m_addRetries)+  "]");

			webClient::request r = yp2ss.m_lastRequest;
			yp2ss.m_lastOperationTime = t;
			++yp2ss.m_addRetries;

			r.m_addr = gOptions.ypAddr();
			r.m_port = gOptions.ypPort();
			r.m_path = gOptions.ypPath();

			r.m_XFF = metrics::metrics_verifyDestIP(gOptions);

			yp2ss.m_addState = ADD_PENDING;
			yp2ss.m_lastOperationTime = ::time(NULL);

			updateYPBandWidthSent(r);
			queueRequest((yp2ss.m_lastRequest = r));
			result.m_requestQueued = 1;
			return result;
		}
	}

	result.m_maxYPInterval = (*i).second.m_stationInfo.m_updateFrequency;

	return result;
}

yp2::addState_t yp2::pvt_addStatus(yp2SessionKey key, int &addFailIgnore, int &errorCode) throw()
{
	stackLock sml(m_serverMapLock);

	serverMap_t::const_iterator i = m_serverMap.find(key);
	bool updateFail = false;
	// if the YP cannot be resolved then allow clients to connect by faking a success
	if (i != m_serverMap.end() && ((*i).second.m_addState == ADD_SUCCEEDED || (*i).second.m_addState == UPDATE_FAILED))
	{
		updateFail = ((*i).second.m_addState == UPDATE_FAILED);
	}
	// allow clients if there was a YP failure or it's down for maintenance
	addFailIgnore = (updateFail ? 2 : m_addFailed || (m_errorCode == YP_MAINTENANCE_CODE));
	errorCode = m_errorCode;

	return (i != m_serverMap.end() ? (*i).second.m_addState : ADD_NONE);
}

/////////// response handling //////////////
// handle response. retry_exceptions do just that. otherwise retry occurs in yptimeout seconds

// what kind of response we get is unimportant, but we will log it if it's an error
void yp2::response_remove(const request &q, const response &r) throw(exception)
{
	yp2SessionKey key = q.REQUEST_KEY;

	serverMap_t::const_iterator i = m_serverMap.find(key);
	if (i == m_serverMap.end())
	{
		throwEx<runtime_error>("Internal error in " + string(__FUNCTION__) +
							   " serverMap entry missing for " + logString(key));
	}

	const yp2_server_state &yp2ss = (*i).second;
	size_t streamID = yp2ss.m_streamID;

	DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " Erasing serverMap entry for " + logString(key));
	assert(m_serverMap.find(key) != m_serverMap.end());
	m_serverMap.erase(key);

	aolxml::node *n = 0;

	try
	{
		n = aolxml::node::parse(&(r.m_body[0]),r.m_body.size());

		int code = aolxml::subNodeText(n, "/yp/resp/status/statuscode", 200);
		utf8 msg = aolxml::subNodeText(n, "/yp/resp/status/statustext", (utf8)"");
		if (code != 200)
		{
			throwEx<runtime_error>("Remove error. " + logString(key) +
								   " code=" + tos(code) + " text=" + msg);
		}
		else
		{
			int error = aolxml::subNodeText(n, "/yp/resp/error/code", -1);
			if (error != -1)
			{
				ELOG(YP2_LOGNAME "De-listing stream #" + tos(streamID) + " failed. YP2 error code is " +
								 tos(error) + " [" + aolxml::subNodeText(n, "/yp/resp/error/message", (utf8)"") + "]");
				utf8 detailed = aolxml::subNodeText(n, "/yp/resp/error/messagedetail", (utf8)"");
				if (!detailed.empty())
				{
					ELOG(YP2_LOGNAME + detailed);
				}
			}
			else
			{
				if (q.REQUEST_USR)
				{
					ILOG(YP2_LOGNAME "Stream #" + tos(streamID) + " has been removed from the Shoutcast Directory.");
				}
			}
		}
	}
	catch(const exception &ex)
	{
		ELOG(string(YP2_LOGNAME) + ex.what());
	}

	forget(n);
}

void updateDetail(aolxml::node *n, string path, utf8 &original)
{
	utf8 value = aolxml::subNodeText(n, path, (utf8)"");
	if (!value.empty())
	{
		original = value;
		DEBUG_LOG ("changing setting " + path + " to " + value);
	}
}

// we generally don't care about the result here (from a state machine standpoint), except that
// a bad serverID error means we should probably re-add ourselves. We rely on the fact that streamData
// will initiate an add before every touch if things have failed, so we enter a failed state		
void yp2::response_update(const request &q, const response &r) throw(exception)
{
	yp2SessionKey key = q.REQUEST_KEY;

	assert(m_serverMap.find(key) != m_serverMap.end());
	yp2_server_state &yp2ss = m_serverMap[key];
	assert(yp2ss.m_sessionKey == key);

	aolxml::node *n = 0;

	try
	{
		// we must check our state, to avoid going berserk if we've queued up a lot of updates
		// and they are failing. This will cause addState to transition to ADD_FAILED, which causes
		// "add" to be reposted and state to become ADD_PENDING. In this situation we don't want our
		// queued updates to fail because that will force the state to ADD_FAILED which will queue up
		// even more "adds"
		if (yp2ss.m_addState == yp2::ADD_SUCCEEDED)
		{
			m_errorCode = 200;
			m_errorMsg = "";
			m_errorMsgExtra = "";

			n = aolxml::node::parse(&(r.m_body[0]),r.m_body.size());
			int code = aolxml::subNodeText(n, "/yp/resp/status/statuscode", 200);
			utf8 msg = aolxml::subNodeText(n, "/yp/resp/status/statustext", (utf8)"");
			if (code != 200)
			{
				throwEx<runtime_error>("Update error. " + logString(key) +
									   " code=" + tos(code) + " msg=" + msg);
			}

			int error = aolxml::subNodeText(n, "/yp/resp/error/code", -1);
			if (error != -1)
			{
				ELOG(YP2_LOGNAME "Updating listing details for stream #" + tos(yp2ss.m_streamID) +
					 " failed. YP2 error code is " + tos(error) + " [" +
					 aolxml::subNodeText(n, "/yp/resp/error/message", (utf8)"") + "]");
				utf8 detailed = aolxml::subNodeText(n, "/yp/resp/error/messagedetail", (utf8)"");
				if (!detailed.empty())
				{
					ELOG(YP2_LOGNAME + detailed);
				}

				yp2ss.m_addState = UPDATE_FAILED;
			}
			else
			{
				updateDetail(n, "/yp/resp/id", yp2ss.m_serverID);
				updateDetail(n, "/yp/resp/stnid", yp2ss.m_stationID);

				yp2ss.m_stationInfo.m_updateFrequency = aolxml::subNodeText(n, "/yp/resp/updatefreq", yp2ss.m_stationInfo.m_updateFrequency);

				updateDetail(n, "/yp/resp/backupserver", yp2ss.m_stationInfo.m_backupServer);
				yp2ss.m_stationInfo.m_backupServersList.clear();
				aolxml::node::nodeList_t nodes = aolxml::node::findNodes(n,"/yp/resp/backupservers/server");
				if (!nodes.empty())
				{
					for (aolxml::node::nodeList_t::const_iterator b = nodes.begin(); b != nodes.end(); ++b)
					{
						yp2ss.m_stationInfo.m_backupServersList.push_back((*b)->pcdata());
					}
				}
				else
				{
					yp2ss.m_stationInfo.m_backupServersList.clear();
				}
				updateDetail(n, "/yp/resp/publicip", yp2ss.m_stationInfo.m_publicIP);
				updateDetail(n, "/yp/resp/station/name", yp2ss.m_stationInfo.m_streamTitle);
				updateDetail(n, "/yp/resp/station/genre", yp2ss.m_stationInfo.m_streamGenre[0]);
				updateDetail(n, "/yp/resp/station/genre2", yp2ss.m_stationInfo.m_streamGenre[1]);
				updateDetail(n, "/yp/resp/station/genre3", yp2ss.m_stationInfo.m_streamGenre[2]);
				updateDetail(n, "/yp/resp/station/genre4", yp2ss.m_stationInfo.m_streamGenre[3]);
				updateDetail(n, "/yp/resp/station/genre5", yp2ss.m_stationInfo.m_streamGenre[4]);
				updateDetail(n, "/yp/resp/station/logo", yp2ss.m_stationInfo.m_streamLogo);
				updateDetail(n, "/yp/resp/station/url", yp2ss.m_stationInfo.m_broadcasterURL);
				updateDetail(n, "/yp/resp/station/callsign", yp2ss.m_stationInfo.m_radionomyID);
				yp2ss.m_stationInfo.m_advertMode = aolxml::subNodeText(n, "/yp/resp/station/admode", yp2ss.m_stationInfo.m_advertMode);
                yp2ss.m_stationInfo.m_allowSSL = aolxml::subNodeText(n, "/yp/resp/station/allowssl", -1);
                yp2ss.m_stationInfo.m_allowMaxBitrate = aolxml::subNodeText(n, "/yp/resp/station/allowmaxbitrate", 0);
                yp2ss.m_stationInfo.m_allowBackupURL = aolxml::subNodeText(n, "/yp/resp/station/allowbackupurl", -1);
                yp2ss.m_stationInfo.m_allowAllFormats = aolxml::subNodeText(n, "/yp/resp/station/allowallformats", -1);

				if (q.REQUEST_USR)
				{
					ILOG(YP2_LOGNAME "Updating listing details for stream #" + tos(yp2ss.m_streamID) + " succeeded.");
				}

				// we get notified of updated DNAS via this on add and update (for long running instances)
				updater::verInfo m_verInfo;
				updater::getNewVersion(m_verInfo);
				m_verInfo.ver = aolxml::subNodeText(n, "/yp/resp/dnas/ver", m_verInfo.ver);
				m_verInfo.url = aolxml::subNodeText(n, "/yp/resp/dnas/url", m_verInfo.url);
				m_verInfo.log = aolxml::subNodeText(n, "/yp/resp/dnas/log", m_verInfo.log);
				m_verInfo.info = aolxml::subNodeText(n, "/yp/resp/dnas/info", m_verInfo.info);
				m_verInfo.message = aolxml::subNodeText(n, "/yp/resp/dnas/msg", m_verInfo.message);
				m_verInfo.slimmsg = aolxml::subNodeText(n, "/yp/resp/dnas/slim", m_verInfo.slimmsg);
				updater::setNewVersion(m_verInfo);

				// YP overridable urls for anything to do with metrics, adverts, etc
				updateDetail(n, "/yp/resp/meu", yp2ss.m_stationInfo.m_metrics_audience_url);
				updateDetail(n, "/yp/resp/adu", yp2ss.m_stationInfo.m_metrics_adverts_url);
				updateDetail(n, "/yp/resp/mer", yp2ss.m_stationInfo.m_metrics_reset_url);
				updateDetail(n, "/yp/resp/aut", yp2ss.m_stationInfo.m_metrics_auth_url);
				updateDetail(n, "/yp/resp/air", yp2ss.m_stationInfo.m_tunein_air_api_url);
				updateDetail(n, "/yp/resp/tsu", yp2ss.m_stationInfo.m_targetspot_url);

				// this will ensure that the info from the update
				// is set asap as this is needed for advert mode.
				streamData *sd = streamData::accessStream(yp2ss.m_streamID);
				if (sd)
				{
					sd->YP2_updateInfo(yp2ss.m_stationInfo);
					sd->releaseStream();
				}
			}
		}
	}
	catch (const exception &)
	{
		m_errorCode = aolxml::subNodeText(n, "/yp/resp/error/code", -1);
		if (m_errorCode != -1)
		{
			m_errorMsg = aolxml::subNodeText(n, "/yp/resp/error/message", (utf8)"");
			m_errorMsgExtra = aolxml::subNodeText(n, "/yp/resp/error/messagedetail", (utf8)"");
		}
		else
		{
			m_errorCode = aolxml::subNodeText(n, "/yp/resp/status/statuscode", 400);
			m_errorMsg = aolxml::subNodeText(n, "/yp/resp/status/statustext", utf8("Generic Error"));
			m_errorMsgExtra = "";
		}

		ELOG(YP2_LOGNAME "Updating listing details for stream #" + tos(yp2ss.m_streamID) + " failed. YP2 error code is " +
						 tos(m_errorCode) + " [" + m_errorMsg + "]");
		if (!m_errorMsgExtra.empty())
		{
			ELOG(YP2_LOGNAME + m_errorMsgExtra);
		}

		yp2ss.m_addState = UPDATE_FAILED;
	}

	forget(n);
}

/*
	We have a special case if state of ADD_REMOVEISSUED. The streamData object called
	yp2::remove() while the add was still pending. If we fail, then it doesn't matter, but if
	we succeed, we have to post the remove command to yp to make sure we clean up after ourselves
*/
void yp2::response_add(const request &q, const response &r) throw(exception)
{
	yp2SessionKey key = q.REQUEST_KEY;

	serverMap_t::iterator i = m_serverMap.find(key);
	if (i == m_serverMap.end())
	{
		throwEx<runtime_error>("Internal error in " + string(__FUNCTION__) +
							   " serverMap entry missing for " + logString(key));
	}

	yp2_server_state &yp2ss = (*i).second;
	aolxml::node *n = 0;

	try
	{
		n = aolxml::node::parse(&(r.m_body[0]),r.m_body.size());

		int code = aolxml::subNodeText(n, "/yp/resp/status/statuscode", 200);
		utf8 msg = aolxml::subNodeText(n, "/yp/resp/status/statustext", (utf8)"");

		if (code != 200)
		{
			throwEx<runtime_error>("Add error. " + logString(key) +
								   " code=" + tos(code) + " text=" + msg);
		}

		// success case. Again there are two scenarios. If the serverMap state is ADD_REMOVEISSUED
		// then remove() was called. We need to repost the remove since yp2::remove() won't actually
		// issue it when the add has not succeeded. Otherwise, life is normal
		if (yp2ss.m_addState != ADD_REMOVEISSUED)
		{
			aolxml::subNodeTextTHROW(n, "/yp/resp/id", yp2ss.m_serverID);
			aolxml::subNodeTextTHROW(n, "/yp/resp/stnid", yp2ss.m_stationID);

			yp2ss.m_stationInfo.m_updateFrequency = aolxml::subNodeText(n, "/yp/resp/updatefreq", yp2ss.m_stationInfo.m_updateFrequency);
			yp2ss.m_stationInfo.m_backupServer = aolxml::subNodeText(n, "/yp/resp/backupserver", (utf8)"");
			yp2ss.m_stationInfo.m_backupServersList.clear();
			aolxml::node::nodeList_t nodes = aolxml::node::findNodes(n, "/yp/resp/backupservers/server");
			if (!nodes.empty())
			{
				for (aolxml::node::nodeList_t::iterator b = nodes.begin(); b != nodes.end(); ++b)
				{
					yp2ss.m_stationInfo.m_backupServersList.push_back((*b)->pcdata());
				}
			}
			yp2ss.m_stationInfo.m_publicIP = aolxml::subNodeText(n, "/yp/resp/publicip", (utf8)"");
			aolxml::subNodeTextTHROW(n, "/yp/resp/station/name", yp2ss.m_stationInfo.m_streamTitle);
			aolxml::subNodeTextTHROW(n, "/yp/resp/station/genre", yp2ss.m_stationInfo.m_streamGenre[0]);

			for (int i = 1; i < 5; i++)
			{
				yp2ss.m_stationInfo.m_streamGenre[i] = aolxml::subNodeText(n, "/yp/resp/station/genre" + tos(i + 1), yp2ss.m_stationInfo.m_streamGenre[i]);
			}

			yp2ss.m_stationInfo.m_streamLogo = aolxml::subNodeText(n, "/yp/resp/station/logo", yp2ss.m_stationInfo.m_streamLogo);
			yp2ss.m_stationInfo.m_broadcasterURL = aolxml::subNodeText(n, "/yp/resp/station/url", (utf8)"");
			yp2ss.m_stationInfo.m_radionomyID = aolxml::subNodeText(n, "/yp/resp/station/callsign", (utf8)"");
			yp2ss.m_stationInfo.m_advertMode = aolxml::subNodeText(n, "/yp/resp/station/admode", yp2ss.m_stationInfo.m_advertMode);
            yp2ss.m_stationInfo.m_allowSSL = aolxml::subNodeText(n, "/yp/resp/station/allowssl", -1);
            yp2ss.m_stationInfo.m_allowMaxBitrate = aolxml::subNodeText(n, "/yp/resp/station/allowmaxbitrate", 0);
            yp2ss.m_stationInfo.m_allowBackupURL = aolxml::subNodeText(n, "/yp/resp/station/allowbackupurl", -1);
            yp2ss.m_stationInfo.m_allowAllFormats = aolxml::subNodeText(n, "/yp/resp/station/allowallformats", -1);

			int peak = aolxml::subNodeText(n, "/yp/resp/peak", -1);
			if (peak > 0)
			{
				stats::updatePeak(yp2ss.m_streamID, peak);
			}

			yp2ss.m_addRetries = 0;
			m_errorCode = 200;
			m_errorMsg = "";
			m_errorMsgExtra = "";

			yp2ss.m_addState = ADD_SUCCEEDED;
			if (q.REQUEST_USR)
			{
				ILOG(YP2_LOGNAME "Stream #" + tos(yp2ss.m_streamID) + " has been added to the Shoutcast Directory.");
			}

			// we get notified of updated DNAS via this on add and update (for long running instances)
			updater::verInfo m_verInfo;
			updater::getNewVersion(m_verInfo);
			m_verInfo.ver = aolxml::subNodeText(n, "/yp/resp/dnas/ver", m_verInfo.ver);
			m_verInfo.url = aolxml::subNodeText(n, "/yp/resp/dnas/url", m_verInfo.url);
			m_verInfo.log = aolxml::subNodeText(n, "/yp/resp/dnas/log", m_verInfo.log);
			m_verInfo.info = aolxml::subNodeText(n, "/yp/resp/dnas/info", m_verInfo.info);
			m_verInfo.message = aolxml::subNodeText(n, "/yp/resp/dnas/msg", m_verInfo.message);
			m_verInfo.slimmsg = aolxml::subNodeText(n, "/yp/resp/dnas/slim", m_verInfo.slimmsg);
			updater::setNewVersion(m_verInfo);

			// YP overridable urls for anything to do with metrics, adverts, etc
			yp2ss.m_stationInfo.m_metrics_audience_url = aolxml::subNodeText(n, "/yp/resp/meu", (utf8)METRICS_AUDIENCE_URL);
			yp2ss.m_stationInfo.m_metrics_adverts_url = aolxml::subNodeText(n, "/yp/resp/adu", (utf8)METRICS_ADVERTS_URL);
			yp2ss.m_stationInfo.m_metrics_reset_url = aolxml::subNodeText(n, "/yp/resp/mer", (utf8)METRICS_RESET_URL);
			yp2ss.m_stationInfo.m_metrics_auth_url = aolxml::subNodeText(n, "/yp/resp/aut", (utf8)DNAS_AUTH_URL);
			yp2ss.m_stationInfo.m_targetspot_url = aolxml::subNodeText(n, "/yp/resp/tsu", (utf8)TARGETSPOT_URL);

			// this will ensure that the info from the update
			// is set asap as this is needed for advert mode.
			// as well as only doing metrics if the stream is
			// still determined to be accessible at the time.
			streamData *sd = streamData::accessStream(yp2ss.m_streamID);
			if (sd)
			{
				metrics::metrics_stream_up (yp2ss.m_streamID, yp2ss.m_stationInfo.m_radionomyID,
					yp2ss.m_serverID, yp2ss.m_stationInfo.m_publicIP, sd->getStartTime());

				sd->YP2_updateInfo(yp2ss.m_stationInfo);
				sd->releaseStream();
			}

			// we use this to look at the existing listeners and if
			// any have m_group = -1 then we'll 'add' them to the
			// metrics since they will otherwise not be known about
			stats::catchPreAddClients(yp2ss.m_streamID);
		}
		else
		{
			// we're done but have to issue the actual remove command to YP
			if (!iskilled())
			{
				WLOG(YP2_LOGNAME "Remove called while add was pending. Re-issuing remove. " + logString(key));
			}
			yp2ss.m_serverID = aolxml::subNodeText(n, "/yp/resp/id", (utf8)"");

			ypInfo info(key, yp2ss.m_streamID, yp2ss.m_authHash);
			info.peakClientConnections = (size_t)-1;
			info.maxClientConnections = (size_t)-1;
			pvt_queue_remove(info);
		}
	}
	catch(const exception &ex)
	{
		bool missing = false;
		utf8 message = ex.what();
		// skip missing xml response entries to avoid user confusion
		if (message.rfind(utf8(" missing")) == utf8::npos)
		{
			ELOG(string(YP2_LOGNAME) + message);
		}
		else
		{
			missing = true;
		}

		// if failure occurs and the state was ADD_REMOVEISSUED, we have to delete the entry from the serverMap
		if (yp2ss.m_addState == ADD_REMOVEISSUED)
		{
			m_serverMap.erase(key);
			WLOG(YP2_LOGNAME "Remove called while add was pending. Deleting serverMap entry " + logString(key)); 
		}
		else
		{
			yp2ss.m_addState = ADD_FAILED;

			m_errorCode = aolxml::subNodeText(n, "/yp/resp/error/code", -1);
			if (m_errorCode != -1)
			{
				m_errorMsg = aolxml::subNodeText(n, "/yp/resp/error/message", (utf8)"");
				m_errorMsgExtra = aolxml::subNodeText(n, "/yp/resp/error/messagedetail", (utf8)"");
			}
			else
			{
				if (!missing)
				{
					m_errorCode = aolxml::subNodeText(n, "/yp/resp/status/statuscode", 400);
					m_errorMsg = aolxml::subNodeText(n, "/yp/resp/status/statustext", utf8("Generic Error"));
					m_errorMsgExtra = "";
				}
				else
				{
					m_errorCode = YP_AUTH_ISSUE_CODE;
					m_errorMsg = "Required authhash parameter is missing, please contact support to resolve this issue";
					m_errorMsgExtra = "";
				}
			}

			ELOG(YP2_LOGNAME "Stream #" + tos(yp2ss.m_streamID) + " connection attempt " +
							 (m_errorCode == YP_MAINTENANCE_CODE ? "ignored" : "failed") + ". YP2 error code is " +
							 tos(m_errorCode) + " [" + m_errorMsg + "]");
			if (!m_errorMsgExtra.empty())
			{
				ELOG(YP2_LOGNAME + m_errorMsgExtra);
			}
		}
	}

	forget(n);
}

void yp2::gotResponse(const request &q, const response &r) throw(exception)
{
	// if we've got a response then clear the pass-through flag
	gYP2->m_addFailed = false;

	DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " Request " + logString(q.REQUEST_KEY) +
			  " cmd=" + tos(q.REQUEST_CMD) + eol() + 
			  "Response body=[" + eol() + r.m_body + "]" +
			  eol() + "Response code=[" + tos(r.m_resultCode) + "]");

	bandWidth::updateAmount(bandWidth::YP_RECV, r.m_received);

	switch (q.REQUEST_CMD)
	{
		case ADD_CMD:
		{
			response_add(q, r);
			break;
		}
		case REM_CMD:
		{
			response_remove(q, r);
			break;
		}
		case UPD_CMD:
		{
			response_update(q, r);
			break;
		}
		case CHECK_AUTH:
		{
			// store the response in a sanitised format for when it is then queried
			authhashMap_t::iterator i = m_authhashMap.find(q.REQUEST_KEY);
			if (i != m_authhashMap.end())
			{
				(*i).second.m_response = r.m_body;
				(*i).second.m_status = r.m_resultCode;
				DEBUG_LOG(YP2_LOGNAME + (*i).second.m_response + " " + tos((*i).second.m_status));
			}
			break;
		}
		case VER_CHECK:
		{
			aolxml::node *n = 0;

			try
			{
				n = aolxml::node::parse(&(r.m_body[0]), r.m_body.size());

				int code = aolxml::subNodeText(n, "/yp/resp/status/statuscode", 200);
				utf8 msg = aolxml::subNodeText(n, "/yp/resp/status/statustext", (utf8)"");
				if (code != 200)
				{
					throwEx<runtime_error>("version check error. " + logString(q.REQUEST_KEY) +
										   " code=" + tos(code) + " text=" + msg);
				}
				else
				{
					// we get notified of updated DNAS via this on add and update (for long running instances)
					updater::verInfo m_verInfo;
					updater::getNewVersion(m_verInfo);
					m_verInfo.ver = aolxml::subNodeText(n, "/yp/resp/dnas/ver", m_verInfo.ver);
					m_verInfo.url = aolxml::subNodeText(n, "/yp/resp/dnas/url", m_verInfo.url);
					m_verInfo.log = aolxml::subNodeText(n, "/yp/resp/dnas/log", m_verInfo.log);
					m_verInfo.info = aolxml::subNodeText(n, "/yp/resp/dnas/info", m_verInfo.info);
					m_verInfo.message = aolxml::subNodeText(n, "/yp/resp/dnas/msg", m_verInfo.message);
					m_verInfo.slimmsg = aolxml::subNodeText(n, "/yp/resp/dnas/slim", m_verInfo.slimmsg);
					updater::setNewVersion(m_verInfo);
				}
			}
			catch(const exception &)
			{
			}

			forget(n);
			break;
			{
			}

			forget(n);
			break;
		}
		default:
		{
			ELOG(YP2_LOGNAME "Internal error. Unknown cmd value in response (" + tos(q.REQUEST_CMD) + ")");
			break;
		}
	}
}

void yp2::gotFailure(const request &q) throw(exception)
{
	DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " Request " +
			  logString(q.REQUEST_KEY) + " cmd=" + tos(q.REQUEST_CMD));

	bandWidth::updateAmount(bandWidth::YP_RECV, q.m_received);

	switch (q.REQUEST_CMD)
	{
		case ADD_CMD:
		case REM_CMD:
		case UPD_CMD:
		{
			gYP2->m_errorCode = -1;
			gYP2->m_addFailed = true;
			break;
		}
		case CHECK_AUTH:
		{
			// we cannot assume we got anything back in the request if it failed
			// so if we've got a pending action then we need to get it cancelled
			// and direct people to the log for the cause as too messy otherwise
			authhashMap_t::iterator i = m_authhashMap.find(q.REQUEST_KEY);
			if (i != m_authhashMap.end())
			{
				(*i).second.m_response = utf8("Unable to process the request. Check the server logs for more details.");
				(*i).second.m_status = 669;
				DEBUG_LOG(YP2_LOGNAME + (*i).second.m_response + " " + tos((*i).second.m_status));
			}
			break;
		}
		default:
		{
			ELOG(YP2_LOGNAME "Internal error. Unknown cmd value in response (" + tos(q.REQUEST_CMD) + ")");
			break;
		}
	}
}

yp2::yp2SessionKey yp2::add(ypInfo& info, const int creating) throw()
{
	stackLock sml(g_YP2Lock);

	yp2SessionKey result = INVALID_SESSION_KEY;

	try
	{
		// abort asap if there is no authhash present
		if (info.authhash.empty())
		{
			if (gYP2 && !gYP2->m_ignoreAdd)
			{
				gYP2->m_ignoreAdd = true;
				throwEx<runtime_error>((creating == 2 ? "Manual authhash creation for stream #" +
									   tos(info.sid) + " is required. Aborting listing the "
									   "stream in the Shoutcast Directory." : "No authhash "
									   "specified for stream #" + tos(info.sid) + ". Aborting "
									   "listing the stream in the Shoutcast Directory. "));
			}
		}
		else if (!isValidAuthhash(info.authhash))
		{
			if (gYP2 && !gYP2->m_ignoreAdd)
			{
				gYP2->m_ignoreAdd = true;
				throwEx<runtime_error>("Invalid authhash specified for stream #" + tos(info.sid) +
									   ". Aborting listing the stream in the Shoutcast Directory.");
			}
		}
		else
		{
			if (gYP2)
			{
				gYP2->m_ignoreAdd = false;
			}
		}

		if (!iskilled() && gYP2 && !gYP2->m_ignoreAdd)
		{
			result = gYP2->pvt_add(info);
		}
	}
	catch(const exception &ex)
	{
		ELOG(string(YP2_LOGNAME) + ex.what());
	}
	return result;
}

void yp2::remove(ypInfo& info) throw()
{
	stackLock sml(g_YP2Lock);

	try
	{
		if (gYP2)
		{
			gYP2->pvt_remove(info);
		}
	}
	catch(const exception &ex)
	{
		ELOG(string(YP2_LOGNAME) + ex.what());
	}
}

yp2::updateResult yp2::update(ypInfo& info)
{
	stackLock sml(g_YP2Lock);

	yp2::updateResult result;
	result.m_maxYPInterval = gOptions.ypReportInterval();
	try
	{
		if (gYP2)
		{
			result = gYP2->pvt_update(info);
		}
	}
	catch(const exception &ex)
	{
		ELOG(string(YP2_LOGNAME) + ex.what());
	}
	return result;
}

yp2::addState_t yp2::addStatus(yp2SessionKey key, int &addFailIgnore, int &errorCode) throw()
{
	stackLock sml(g_YP2Lock);

	return (gYP2 ? gYP2->pvt_addStatus(key, addFailIgnore, errorCode) : ADD_NONE);
}

void yp2::pvt_runAuthHashAction(const uniString::utf8 &tempId, const int action,
							    const uniString::utf8 &urlPath,
							    const httpHeaderMap_t &queryParameters,
								const uniString::utf8 &content) throw()
{
	int actionId = tempId.toInt();
	yp2::authhashResult authhash;
	authhash.m_action = action;
	m_authhashMap[actionId] = authhash;

	webClient::request r;
	r.m_method = webClient::request::POST;
	r.m_addr = gOptions.ypAddr();
	r.m_port = gOptions.ypPort();
	// adjust the YP path if using non-standard urls so authhash ones work
	r.m_path = ((gOptions.ypPath() != utf8("/yp2")) ? "/yp" : "") + urlPath;
	r.m_queryVariables = queryParameters;
	r.REQUEST_KEY = actionId;
	r.REQUEST_CMD = action;

	if (!content.empty())
	{
		r.m_content.insert(r.m_content.end(), content.begin(), content.end());
	}

	DEBUG_LOG(YP2_LOGNAME + string(__FUNCTION__) + " Request " +
			  logString(r.REQUEST_KEY) + " cmd=" + tos(r.REQUEST_CMD));

	updateYPBandWidthSent(r);
	queueRequest(r);
}

void yp2::runAuthHashAction(const uniString::utf8 &tempId, const int action,
						    const uniString::utf8 &urlPath,
							const httpHeaderMap_t &queryParameters,
							const uniString::utf8 &content) throw()
{
	stackLock sml(g_YP2Lock);

	try
	{
		if (gYP2 && !queryParameters.empty() && !tempId.empty())
		{
			gYP2->pvt_runAuthHashAction(tempId, action, urlPath, queryParameters, content);
		}
	}
	catch(const exception &ex)
	{
		ELOG(string(YP2_LOGNAME) + ex.what());
	}
}

bool yp2::pvt_authHashActionStatus(const uniString::utf8& tempId, authhashResult &info, const bool remove) throw()
{
	if (!tempId.empty())
	{
		size_t actionId = tempId.toInt();
		authhashMap_t::const_iterator i = m_authhashMap.find(actionId);
		if (i != m_authhashMap.end())
		{
			// copy to the passed structure
			info = (*i).second;
			// and then remove from the map
			if(remove)
			{
				m_authhashMap.erase(actionId);
			}
			return true;
		}
	}
	return false;
}

bool yp2::authHashActionStatus(const uniString::utf8& tempId, authhashResult &info, const bool remove) throw()
{
	stackLock sml(g_YP2Lock);

	return (gYP2 ? gYP2->pvt_authHashActionStatus(tempId, info, remove) : false);
}

bool yp2::isValidAuthhash(uniString::utf8 authhash)
{
    if (authhash.empty() || (authhash.size() != 20 && authhash.size() != 36))
        return false;

    // check the authhash only contains
    int n = -1;
    const char *s = (char*)authhash.c_str();
    if (sscanf (s, "%*36[a-fA-F0-9-]%n", &n) == 0 && n == 36 && s[36] == '\0')      // match 36-char GUID only
        return true;
    if (sscanf (s, "%*20[a-zA-Z0-9]%n", &n) == 0 && n == 20 && s[20] == '\0')       // match 20-char Hash only
        return true;
    return false;
}

int yp2::pvt_getSrvID(yp2SessionKey key) throw()
{
	stackLock sml(m_serverMapLock);

	serverMap_t::const_iterator i = m_serverMap.find(key);
	return (i != m_serverMap.end() ? (*i).second.m_serverID.toInt() : 0);
}

int yp2::getSrvID(yp2SessionKey key) throw()
{
	stackLock sml(g_YP2Lock);

	return (gYP2 ? gYP2->pvt_getSrvID(key) : 0);
}

int yp2::pvt_getStnID(yp2SessionKey key) throw()
{
	stackLock sml(m_serverMapLock);

	serverMap_t::const_iterator i = m_serverMap.find(key);
	return (i != m_serverMap.end() ? (*i).second.m_stationID.toInt() : 0);
}

int yp2::getStnID(yp2SessionKey key) throw()
{
	stackLock sml(g_YP2Lock);

	return (gYP2 ? gYP2->pvt_getStnID(key) : 0);
}
