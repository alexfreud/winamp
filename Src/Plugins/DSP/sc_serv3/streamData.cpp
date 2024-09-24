#ifdef _WIN32
#include <winsock2.h>
#include <string.h>
#define strncasecmp _strnicmp
#endif
#include <assert.h>
#include <list>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include "streamData.h"
#include "metadata.h"
#include "stats.h"
#include "aolxml/aolxml.h"
#include "stl/stringUtils.h"
#include "stl/stlx.h"
#include "global.h"
#include "protocol_shoutcastClient.h"
#include "bandwidth.h"
#include "uvox2Common.h"
#include "base64.h"
#include "file/fileUtils.h"
#include "ADTSHeader.h"
#include "MP3Header.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace stringUtil;
using namespace uniString;

#define DEBUG_LOG(...)          do { if (gOptions.streamDataDebug()) DLOG(__VA_ARGS__); } while(0)
#define AD_DEBUG_LOG(...)       do { if (gOptions.adMetricsDebug()) DLOG(__VA_ARGS__); } while(0)
#define ADLOGNAME               "ADVERT"
#define LOGNAME                 "STREAMDATA"

/////////// statics ///

AOL_namespace::mutex						streamData::g_streamMapLock;
set<streamData*>							streamData::g_streams;		// all the streams, including those that are "dying"
map<streamData::streamID_t, streamData*>	streamData::g_streamMap;
map<streamData::streamID_t, time_t>			streamData::g_streamUptime;
map<streamData*, int>						streamData::g_streamReferenceCounts;
map<streamData*, bool>						streamData::g_streamSourceIsConnected;	// is the source connected
AOL_namespace::mutex						streamData::g_sourceRelayMapLock;
map<streamData::streamID_t, int>			streamData::g_streamSourceRelayIsActive;	// is the source relay active e.g. connected or trying to connect or needs to die?
AOL_namespace::mutex						streamData::g_streamSongHistoryMapLock;
map<streamData::streamID_t, streamData::streamHistory_t>	streamData::g_streamSongHistoryMap;
static AOL_namespace::mutex					g_streamMessageMapLock;
static std::map<streamData::streamID_t, utf8>	g_streamMessageMap;

// global pool related
AOL_namespace::mutex                                streamData::adGroups::adContentLock;
list<streamData::specialFileData*>                  streamData::adGroups::adContentPending;
time_t                                              streamData::adGroups::m_nextDownloadRun;
streamData::adGroups::gpool                         streamData::adGroups::adData;

#ifdef LICENCE_FREE

int streamData::streamInfo::m_allowSSL_global = 1;
int streamData::streamInfo::m_allowAllFormats_global = 1;
int streamData::streamInfo::m_allowBackupURL_global = 1;
int streamData::streamInfo::m_allowMaxBitrate_global = 0;

#else
// default free version settings if no licence check response
//
int streamData::streamInfo::m_allowSSL_global = 1;
int streamData::streamInfo::m_allowAllFormats_global = 1;
int streamData::streamInfo::m_allowMaxBitrate_global = 0;
int streamData::streamInfo::m_allowBackupURL_global = 1;
#endif


// just make sure we drop the collected adverts, helps track memory leakage
streamData::adGroups::gpool::~gpool()
{
    for (; begin() != end(); )
    {
        delete begin()->second;
        erase (begin());
    }
}

#if defined(_DEBUG) || defined(DEBUG)
map<streamData::streamID_t, FILE*>			streamData::g_streamSaving;
#endif

static size_t handle_returned_header(void *ptr, size_t size, size_t nmemb, void *stream);
static size_t handle_returned_data(void *ptr, size_t size, size_t nmemb, void *stream);
int xferinfo(void *p, curl_off_t /*dltotal*/, curl_off_t /*dlnow*/, curl_off_t /*ultotal*/, curl_off_t /*ulnow*/);

class streamData::adGroups::adGroupsRetriever
{
public:
	stringstream	ss;
	CURL			*m_curl;
	utf8			post;
	streamData::streamID_t	m_sid;
	char*			m_curl_error;
    int				cleanup;
	int				loaded;
    string          last_uuid;

	adGroupsRetriever(const utf8 &url, const streamData::streamID_t sid) : m_curl(0), m_sid(sid), cleanup(0), loaded(0)
    {
		m_curl_error = new char[CURL_ERROR_SIZE];
		memset(m_curl_error, 0, CURL_ERROR_SIZE);

		streamData *sd = streamData::accessStream(sid);
		if (sd)
		{
			if (!sd->radionomyID().empty() && sd->streamAdvertMode())
			{
				m_curl = webClient::setupCurlDefaults(m_curl, ADLOGNAME, (url + "/?radionomyid=" + sd->radionomyID()), 0, 30L, sid);
				if (m_curl)
				{
                    const streamData::streamInfo &stream = sd->getInfo();

					curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, handle_returned_header);
					curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, this);
					curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, handle_returned_data);
					curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &ss);
					curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, &(m_curl_error[0]));

					// use progress/xfer functions to trap for the server kill case
					curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L);
					curl_easy_setopt(m_curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
					curl_easy_setopt(m_curl, CURLOPT_XFERINFODATA, (long)sid);

#ifdef CURLOPT_PASSWDFUNCTION
					curl_easy_setopt (m_curl, CURLOPT_PASSWDFUNCTION, my_getpass);
#endif

                    httpHeaderMap_t vars;
                    vars["tstamp"] = tos(::time(NULL));
                    vars["host"] = sd->streamPublicIP();
                    vars["path"] = getStreamPath(sid);
                    vars["radionomyid"] = vars["ref"] = sd->radionomyID();
                    vars["srvid"] = stream.m_serverID;
                    vars["bitrate"] = tos(sd->streamBitrate());
                    vars["codec"] = sd->streamContentType();
                    vars["server"] = "Shoutcast v" + gOptions.getVersionBuildStrings();
                    vars["port"] = tos(g_portForClients);

					post = encodeVariables(vars);
					curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, post.size());
					curl_easy_setopt(m_curl, CURLOPT_COPYPOSTFIELDS, post.c_str());
                    last_uuid = sd->advertGroups.lastUUID;
                    if (last_uuid == "")
                        AD_DEBUG_LOG ("sending no UUID", ADLOGNAME, sid);
                    else
                        AD_DEBUG_LOG ("sending UUID " + tos(last_uuid), ADLOGNAME, sid);
					cleanup = 0;
					loaded = 1;
				}
                else
                    WLOG ("Unable to create link for map", LOGNAME, sid);
			}
			// some clients may have already connected to the stream before
			// it had been publically listed, so for those we need to force
			// an update of the stream's information so we have the new lot
			else if (sd->m_streamInfo.m_streamPublic || !gOptions.cdn().empty())
			{
				int addFailIgnore = 0, errorCode = 0;
				if (sd->YP2_addSuccessful(addFailIgnore, errorCode))
				{
					loaded = 2;
				}
			}
			sd->releaseStream();
		}
    }

	~adGroupsRetriever()
	{
		if (m_curl)
		{
			// incase we're mid-processig then we don't
			// want to do this otherwise things go boom
			// instead we allow it to remain and it'll
			// be cleaned up once the abort is actioned
			if (loaded && cleanup)
			{
				curl_easy_cleanup(m_curl);
			}
			m_curl = 0;
		}

		forgetArray(m_curl_error);

		loaded = cleanup = 0;
	}
};


static size_t handle_returned_header(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int amount = (int)(size * nmemb);
    size_t remain = amount;
    const char *p = (const char*)ptr, *eol = (const char*)memchr (p, '\r', amount);
    class streamData::adGroups::adGroupsRetriever *feed = (streamData::adGroups::adGroupsRetriever*)stream;

    if (eol == NULL)
        return 0;

    remain = eol - p;
    if (remain > 13 && strncasecmp (p, "bluebox-uuid:", 13) == 0)
    {
        p += 13;
        size_t skip = strspn (p, " \t");
        remain -= (13 + skip);
        p += skip;
        feed->last_uuid = string (p, remain);
        DEBUG_LOG ("identified uuid as :" + tos (feed->last_uuid) + ":", ADLOGNAME);
    }
    bandWidth::updateAmount(bandWidth::ADVERTS, amount);
    return amount;
}

static size_t handle_returned_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    stringstream *ss = (stringstream*)stream;
    size_t length = size * nmemb;
    if (ss)
    {
        ss->write((const char*)ptr, length);
    }

    bandWidth::updateAmount(bandWidth::ADVERTS, length);
    return length;
}

int xferinfo(void *p, curl_off_t /*dltotal*/, curl_off_t /*dlnow*/, curl_off_t /*ultotal*/, curl_off_t /*ulnow*/)
{
    long sid = (long)p;
    const int killed = iskilled();

    if (killed || (sid >= 0 && !streamData::isSourceConnected((size_t)sid)))
    {
        WLOG("[ADVERT sid=" + tos(sid) + "] Aborting data transfer due to the " +
                (killed ? "server shutting down." : "stream source disconnecting."), ADLOGNAME, sid);
        return -1;
    }
    return 0;
}

// assumes the g_streamMapLock is set. Creates a stream based on the config data, installs it into
// the necessary tables and returns the result
streamData* streamData::_createNewStream(const streamSetup& setup) throw(exception)
{
	if (setup.m_sid > 0)
	{
		streamData *result = new streamData(setup);
		if (result)
		{
			g_streamMap[setup.m_sid] = result;
			g_streams.insert(result);
			g_streamUptime[setup.m_sid] = ::time(NULL);
			g_streamReferenceCounts[result] = 1;
			g_streamSourceIsConnected[result] = true;
			DEBUG_LOG(setup.m_logString + "Creating new stream for ID " + tos(setup.m_sid), LOGNAME, setup.m_sid);
			return result;
		}
		else
		{
			DEBUG_LOG(setup.m_logString + "Failed to create new stream for ID " + tos(setup.m_sid));
		}
	}
	return 0;
}

/*
	Create a type 1 (old shoutcast) stream. A number of things can happen here
	1) Sources are disabled - return NULL
	2) Stream does not exist - create a new one
	3) If the stream exists and a source is connected, return NULL
	4) If the stream exists, but the source is not connected, then check the stream configuration.
		a) Compatible, return the streamData
		b) Incompatible, boot clients, move stream to dead pool, create a new one
*/
streamData* streamData::createStream(const streamSetup& setup) throw(exception)
{
	streamData *result = 0;

	if (setup.m_sid > 0)
	{
		stackLock sml(g_streamMapLock);

		// does the stream exist
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(setup.m_sid);

		if (i == g_streamMap.end())
		{
			// case 2) The stream does not exist
			result = _createNewStream(setup);
		}
		else // cases 3 and 4
		{
			streamData *sd = (*i).second;
			// data structure integrity
			assert(g_streamSourceIsConnected.find(sd) != g_streamSourceIsConnected.end());
			assert(g_streams.find(sd) != g_streams.end());
			assert(g_streamReferenceCounts.find(sd) != g_streamReferenceCounts.end());
			assert(g_streamReferenceCounts[sd] > 0);

			if (!g_streamSourceIsConnected[sd])
			{
				// case 4 - stream exists and source is not connected
				if (sd->isSourceCompatible(setup))
				{
					// case 4a - streams exists and is compatible
					sd->sourceReconnect(setup);
					g_streamSourceIsConnected[sd] = true;
					++g_streamReferenceCounts[sd];
					result = sd;
				}
				else
				{
					if (!setup.m_backupURL.empty() && !sd->isBackupStream(setup.m_sid))
					{
						sd->sourceReconnect(setup);
						g_streamSourceIsConnected[sd] = true;
						++g_streamReferenceCounts[sd];
						result = sd;
					}
					// case 4b - source is incompatible. Move current stream to the dead pool and create a new one
					else
					{
						DEBUG_LOG(setup.m_logString + "Source has changed. Moving old stream " +
								  tos(setup.m_sid) + " to dead pool and creating a new one.", LOGNAME, setup.m_sid);
						// if you call die on a stream you MUST remove it from the g_streamMap
						_moveStreamToDeadPool(setup.m_sid);
						result = _createNewStream(setup);
					}
				}
			}
			// else, case 3 - source is connected. Do nothing and return default intializer of result (NULL)
			else
			{
				// if we have a backup running and a non-backup trying
				// to connect then we should allow it to connect to it
				if (!setup.m_backup && sd->isBackupStream(setup.m_sid))
				{
					// case 4xxx - source is incompatible. Move current stream to the dead pool and create a new one
					sd->setKill((sd->isBackupStream(setup.m_sid) ? 2 : 1));
					sd->sourceReconnect(setup);
					g_streamSourceIsConnected[sd] = true;
					++g_streamReferenceCounts[sd];
					result = sd;
				}
				// otherwise block the source connection from joining
				// as we should not override an existing true source.
			}
		}
	}
	if (result)
	    result->m_startTime = ::time(NULL);
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////	
// putting a stream, conceptually, into the dead pool requires a specific order of operations
// to avoid structure corruption. This order is different depending on the reference point
// so these calls cover the scenarios
void streamData::_moveStreamToDeadPool(streamData *sd) throw()
{
	if (sd)
	{
		g_streamMap.erase(sd->ID()); // do this before calling die, since die() clears objects stream ID()
		g_streamUptime.erase(sd->ID());
		sd->die();
	}
}

void streamData::removeRelayStatus(streamID_t ID)
{
	#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(__FUNCTION__, LOGNAME, ID);
	#endif
	stackLock sml(g_sourceRelayMapLock);

	map<streamID_t, int>::iterator r = g_streamSourceRelayIsActive.find(ID);
	if (r != g_streamSourceRelayIsActive.end())
	{
		g_streamSourceRelayIsActive.erase(r);
    }
}


void streamData::_moveStreamToDeadPool(map<streamID_t, streamData*>::iterator i) throw()
{
	// if you call die on a stream you MUST remove it from the g_streamMap
	(*i).second->die();

	g_streamUptime.erase((*i).second->m_ID);
	g_streamMap.erase(i);
}

bool streamData::isAllowedType(const int type)
{
	bool mp3;
	return isAllowedType(type, mp3);
}

bool streamData::isAllowedType(const int type, bool& mp3)
{
	mp3 = (type == MP3_DATA);
	return (mp3 || (type == AACP_DATA) || (type == AAC_LC_DATA));
}

void streamData::_moveStreamToDeadPool(streamData::streamID_t id) throw()
{
	if (id > 0)
	{
		map<streamID_t, streamData*>::iterator i = g_streamMap.find(id);
		if (i != g_streamMap.end())
		{
			_moveStreamToDeadPool(i);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

// you must hold the g_streamMapLock before calling this. increases the reference count
// on stream with indicated ID. returns stream object
streamData* streamData::_increaseReferenceCount(streamID_t id)
{
	streamData *result = 0;

	if (id > 0)
	{
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);

		if (i != g_streamMap.end())
		{
			assert((*i).second);
			map<streamData*, int>::iterator ic = g_streamReferenceCounts.find((*i).second);
			assert(ic != g_streamReferenceCounts.end());
			assert((*ic).second);
			++(*ic).second;
			result = (*i).second;
		}
	}
	return result;
}

// you must hold the g_streamMapLock before calling this. Decreases the reference count
// on the stream. Returns stream object if reference count has not reached zero
void streamData::_reduceReferenceCount(const utf8& logString, streamData *sd, const streamID_t id)
{
	if (sd && (id > 0))
	{
		map<streamData*, int>::iterator ic = g_streamReferenceCounts.find(sd);

		// integrity checks
		assert(ic != g_streamReferenceCounts.end());
		assert(g_streams.find(sd) != g_streams.end());
		assert(g_streamSourceIsConnected.find(sd) != g_streamSourceIsConnected.end());

		///////////////////
		if (ic != g_streamReferenceCounts.end())
		{
			if (--(*ic).second <= 0)
			{
				// clean it up
                if (id)
                {
                    g_streamMap.erase(id);
                    g_streamUptime.erase(id);
                }

                g_streamReferenceCounts.erase(ic);
                g_streamSourceIsConnected.erase(sd);
                g_streams.erase(sd);
                delete sd;
                sd = NULL;

                if (id)
                {
                    DEBUG_LOG (logString + "Cleaning up stream as no references remain", LOGNAME, id);
                }
			}
			else
			{
				DEBUG_LOG (logString + "Stream still has " + tos((*ic).second) + " reference" + ((*ic).second > 1 ? "s" : ""), LOGNAME, id);
			}
		}
	}
}

void streamData::streamSourceLost(const utf8& logString, streamData *sd, const streamID_t id)
{
	if (sd && (id > 0))
	{
		stackLock sml(g_streamMapLock);

		assert(g_streamSourceIsConnected[sd]);
		g_streamSourceIsConnected[sd] = false;
		_reduceReferenceCount(logString, sd, id);

		// if we are auto dumping our users, then move
		// the stream to the dead pool immediately
		bool autoDumpUsers = gOptions.stream_autoDumpUsers(id);
		if (!gOptions.read_stream_autoDumpUsers(id))
		{
			autoDumpUsers = gOptions.autoDumpUsers();
		}

		if (autoDumpUsers)
		{
			_moveStreamToDeadPool(id);
		}
	}

    // if there are still listeners but no sources
    // then (without this) the listeners will halt
    // this will force things to keep things alive
    threadedRunner::wakeup();
}

void streamData::streamClientLost(const utf8& logString, streamData *sd, const streamID_t id)
{
	if (sd && (id > 0))
	{
		stackLock sml(g_streamMapLock);

		_reduceReferenceCount(logString, sd, id);
	}
}

void streamData::streamUpdate(const streamID_t id, const uniString::utf8 &authHash,
							  const int streamMaxUser, const int streamMaxBitrate,
							  const int streamMinBitrate) throw()
{
	if (id > 0)
	{
		stackLock sml(g_streamMapLock);

		bool authChanged = (m_streamInfo.m_authHash != authHash);
		if (authChanged && !m_streamInfo.m_authHash.empty())
		{
			YP2_remove();
		}

		m_streamInfo.m_authHash = authHash;
		m_streamInfo.m_streamMaxBitrate = streamMaxBitrate;
		m_streamInfo.m_streamMinBitrate = streamMinBitrate;
		m_streamInfo.m_streamMaxUser = streamMaxUser;

		if (authChanged && !authHash.empty())
		{
			YP2_add();
		}
	}
}

void streamData::updateSourceIdent(uniString::utf8& sourceIdent, const bool relay) throw()
{
	// attempt to determine the version of the source connected
	if (!sourceIdent.empty())
	{
		utf8::size_type pos = sourceIdent.find(utf8("<BR>"));
		if (pos != utf8::npos)
		{
			sourceIdent = sourceIdent.substr(0, pos);
		}
		pos = sourceIdent.find(utf8("Ultravox/2.1 "));
		if (pos != utf8::npos && pos == 0)
		{
			sourceIdent = sourceIdent.substr(13);
		}

		if (!sourceIdent.empty() && ((!relay && m_streamInfo.m_sourceType != SHOUTCAST2) || relay))
		{
			if (m_streamInfo.m_sourceIdent.empty() ||
				(m_streamInfo.m_sourceIdent == utf8("Legacy / Unknown")))
			{
				m_streamInfo.m_sourceIdent = sourceIdent;
			}
		}
	}
}

// Client wants access to the stream. returns null if stream does not exist
// this one is only called by protocol_admincgi in order to allow metadata updates
// from sc_trans. Actual client connects should use the accessStream(streamID_t,bool)
// call, to allow consideration for whether the stream is is actually connected, or
// to allow rejection if yp2 has not returned stream information yet.
streamData* streamData::accessStream(streamID_t id) throw()
{
	#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(__FUNCTION__, LOGNAME, id);
	#endif

	if (id > 0)
	{
		try
		{
			if (g_streamMapLock.timedLock(3000))
			{
				streamData *sd = _increaseReferenceCount(id);
				g_streamMapLock.unlock();
				return sd;
			}
		}
		catch (const exception &ex)
		{
			WLOG(utf8("Failed to acquire lock(1): ") + ex.what(), LOGNAME, id);
		}
	}
	return 0;
}

streamData* streamData::accessStream(streamID_t id, bool &isSourceActive) throw()
{
	#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(utf8(__FUNCTION__) + "(2)", LOGNAME, id);
	#endif

	if (id > 0)
	{
		try
		{
			if (g_streamMapLock.timedLock(3000))
			{
				streamData *sd = _increaseReferenceCount(id);
				if (sd)
				{
					assert(g_streamSourceIsConnected.find(sd) != g_streamSourceIsConnected.end());
					isSourceActive = g_streamSourceIsConnected[sd];
				}
				g_streamMapLock.unlock();
				return sd;
			}
		}
		catch (const exception &ex)
		{
			WLOG(utf8("Failed to acquire lock(2): ") + ex.what(), LOGNAME, id);
		}
	}

	isSourceActive = false;
	return 0;
}

void streamData::releaseStream()
{
	#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(__FUNCTION__, LOGNAME, this->ID());
	#endif

	stackLock sml(g_streamMapLock);

	_reduceReferenceCount(utf8(), this, this->ID());
}
//////////

bool streamData::isSourceConnected(streamData::streamID_t id) throw()
{
	if ((id > 0) && g_streamMapLock.timedLock(3000))
	{
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);
		if (i == g_streamMap.end())
		{
			g_streamMapLock.unlock();
			return false;
		}

		const bool connected = g_streamSourceIsConnected[(*i).second];
		g_streamMapLock.unlock();
		return connected;
	}
	return false;
}

// is a relay source active either from trying to connect or connected
// (which also includes backup sources to prevent multiple spawning)
int streamData::isRelayActive(streamData::streamID_t id, bool &noEntry) throw()
{
	if (id > 0)
	{
		stackLock sml(g_sourceRelayMapLock);

		map<streamID_t,int>::const_iterator i = g_streamSourceRelayIsActive.find(id);
		if (i == g_streamSourceRelayIsActive.end())
		{
			noEntry = true;
			#if defined(_DEBUG) || defined(DEBUG)
			DEBUG_LOG("[STREAMDATA sid=" + tos(id) + "] isRelayActive: " + tos(id) + " ~0", LOGNAME, id);
			#endif
			return 0;
		}

		try
		{
			// only signal active if 1 since 2 is used to signal being stopped
			#if defined(_DEBUG) || defined(DEBUG)
			DEBUG_LOG("[STREAMDATA sid=" + tos(id) + "] isRelayActive: " + tos(id) + " " + tos((id == (*i).first) ? (*i).second : 0), LOGNAME, id);
			#endif
			return ((id == (*i).first) ? (*i).second : 0);
		}
		catch(...)
		{
		}
	}
	return 0;
}

int streamData::setRelayActiveFlags (streamData::streamID_t id, bool &noEntry, int flags, int mask)
{
    int state = 0;
    string msg;

    if (id <= 0 || (mask == 0 && flags <= 0))
        return -1;
    do
    {
        stackLock sml(g_sourceRelayMapLock);

        if (flags < 0)
        {
            map<streamID_t, int>::iterator r = g_streamSourceRelayIsActive.find (id);
            if (r != g_streamSourceRelayIsActive.end())
            {
                state = (*r).second;
                flags = state & mask;
                msg = " is " + tos (state) + ", flags " + tos(flags) + "/" + tos(mask);
                break; // it exists, so skip the setting (flags is nonsensical) and just possible log it
            }
            g_streamSourceRelayIsActive[id] = 0;    // create entry in map with 0 value;
            noEntry = true;
            return 0;
        }
        int &v = g_streamSourceRelayIsActive[id];

        if (mask == 0)
            mask = flags;
        flags &= mask;  // only focus on the requested bits.
        if ((v & mask) == flags)
        {
           state = v;
           break;  // already set as this, so return -1
        }
        msg = " was " + tos (v) + ", flags " + tos(flags) + "/" + tos(mask);
        v = (v & ~mask) | (flags & mask);
        state = v;
        flags = ~state; // invalidate the last test as we have done the change.

    } while (0);

#if defined(_DEBUG) || defined(DEBUG)
    DEBUG_LOG("[STREAMDATA sid=" + tos(id) + "] RelayActiveFlags: " + tos(id) + " " + tos(state) + msg, LOGNAME, id);
#endif
    noEntry = false;
    if ((state & mask) == flags)
        return -1;
    return state;
}

void streamData::setRelayActive(streamData::streamID_t id, int state) throw()
{
    int p;
    do
    {
        if (id <= 0)
            return;

        stackLock sml(g_sourceRelayMapLock);

        if (state < 0)
        {
            map<streamID_t, int>::iterator r = g_streamSourceRelayIsActive.find (id);
            if (r != g_streamSourceRelayIsActive.end())
            {
                p = state = (*r).second;
                break; // it exists, so skip setting it and just report it
            }
            state = 0;  // create entry in map with 0 value;
        }
        p = g_streamSourceRelayIsActive[id] = state;
    } while (0);

#if defined(_DEBUG) || defined(DEBUG)
    string msg = " was " + tos (p);
    DEBUG_LOG("[STREAMDATA sid=" + tos(id) + "] setRelayActive: " + tos(id) + " " + tos(state) + msg, LOGNAME, id);
#endif
}

///////////  are used in main() to facilitate an orderly shutdown that sends the
///////////  necessary remsrv to YP
// send die signal to all streams
void streamData::killAllSources() throw()
{
	stackLock sml(g_streamMapLock);

	// if you call die on a stream you must remove it from the g_streamMap
	for_each(g_streams.begin(),g_streams.end(),mem_fun(&streamData::die));
	g_streamMap.clear();
}

// total of all streamData objects, including those that are dying
size_t streamData::totalStreams() throw()
{
	if (g_streamMapLock.timedLock(3000))
	{
		const size_t total = g_streams.size();
		g_streamMapLock.unlock();
		return total;
	}

	return 0;
}

size_t streamData::totalActiveStreams(size_t &lastSID) throw()
{
	size_t total = 0;
	if (totalStreams() > 0)
	{
		size_t inc = 0;
		size_t sid = 0;
		do
		{
			streamInfo info;
			extraInfo extra;
			sid = enumStreams(inc);
			if (getStreamInfo(sid, info, extra))
			{
				++total;
				lastSID = sid;
			}
			++inc;
		}
		while (sid);
	}
	return total;
}

// enumerate the available number of streams actually present
size_t streamData::enumStreams(const size_t index) throw()
{
	stackLock sml(g_streamMapLock);

	if (index < g_streamMap.size())
	{
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.begin();
		size_t inc = 0;
		while (inc < index)
		{
			++inc;
			++i;
		}
		return (*i).second->ID();
	}
	return 0;
}

// enumerate the available number of streams to get the stream ids
streamData::streamIDs_t streamData::getStreamIds(const int mode) throw()
{
	streamIDs_t streamIds;
	if (totalStreams() > 0)
	{
		size_t inc = 0;
		size_t sid = 0;
		do
		{
			sid = streamData::enumStreams(inc);
			++inc;
			if (sid)
			{
				streamIds.insert(sid);
			}
		}
		while (sid);
	}

	if (mode)
	{
		// now we check through for any known but inactive relays
		// and then get them included for being kicked as well
		vector<config::streamConfig> relayList(gOptions.getRelayList());
		if (!relayList.empty())
		{
			for (vector<config::streamConfig>::const_iterator i = relayList.begin(); i != relayList.end(); ++i)
			{
				streamIds.insert((*i).m_streamID);
			}
		}

		if (mode == 2)
		{
			// this will now do a final check for any listeners which are on
			// an un-confiured stream but are being provided a 'backupfile'.
			streamData::streamIDs_t activeIds = stats::getActiveStreamIds();
			if (!activeIds.empty())
			{
				for (streamData::streamIDs_t::const_iterator i = activeIds.begin(); i != activeIds.end(); ++i)
				{
					streamIds.insert((*i));
				}
			}
		}
	}

	return streamIds;
}

void streamData::killStreamSource(const streamID_t id) throw()
{
	if (id > 0)
	{
		// handle relays / backups a bit differently from normal streams
		// since if it's not been able to connect or is backup mode then
		// the normal 'isSourceConnected' will not be able to kill it.
		if ((!gOptions.stream_relayURL(id).empty() && gOptions.stream_movedUrl(id).empty()) ||
			streamData::isRelayStream(id) || streamData::isBackupStream(id))
		{
			bool noEntry = false, active = ((streamData::isRelayActive(id, noEntry) & 12));
			if (active)
			{
				ILOG(gOptions.logSectionName() + "Kicking source for stream #" + tos(id) + ".", LOGNAME, id);

				// kick source off system
				streamData::killSource(id);
			}
		}
		else if (streamData::isSourceConnected(id))
		{
			ILOG(gOptions.logSectionName() + "Kicking source for stream #" + tos(id) + ".", LOGNAME, id);

			// kick source off system
			streamData::killSource(id);
		}
	}
}

// used by web interface to dump a specific source
void streamData::killSource(const streamID_t id, streamData *sd) throw()
{
	if ((id > 0) && g_streamMapLock.timedLock(3000))
	{
		if (sd)
		{
			g_streamSourceIsConnected[sd] = false;
			_reduceReferenceCount(utf8(), sd, sd->ID());
		}

		// force flag a source relay kick if this is called
		bool noEntry = false;
		if ((isRelayActive(id, noEntry) & 12))
		{
			setRelayActiveFlags (id, noEntry, 2);
		}
		_moveStreamToDeadPool(id);

		g_streamMapLock.unlock();
	}
}

// you must remove this streamData object from the g_streamMap immediately before or after calling
// this, otherwise you'll corrupt the static structures.
// Reason: die will cause the streamData to be removed from g_streams, but since die() causes ID() to return zero,
// it will not get removed from g_streamMap
void streamData::die() throw()
{
	if (!m_dead)
	{
		stackLock sml(m_stateLock);
		_YP2_remove();
		m_dead = 1;
	}
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
utf8 streamData::getStreamContentType(const streamID_t id) throw()
{
	if (id > 0)
	{
		stackLock sml(g_streamMapLock);

		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);
		return (i == g_streamMap.end() ? (utf8)"" : (*i).second->streamContentType());
	}
	return "";
}

const bool streamData::updateSourceSampleRate(unsigned int& samplerate, const unsigned int read_samplerate) throw()
{
	// we do some extra checking so we make sure we're got
	// consecutive frames that are at the same samplerate
	// otherwise we can end up with the checking spinning
	//DLOG ("samp " + tos(samplerate) + " read " + tos(read_samplerate) + " last" + tos(m_lastStreamSampleRate));
	if (read_samplerate > 0)
	{
		if ((m_lastStreamSampleRate == 0) ||
				(m_lastStreamSampleRate == read_samplerate))
		{
			// we've got something that matches
			m_streamInfo.m_streamSampleRate = samplerate = read_samplerate;
			m_lastStreamSampleRate = read_samplerate;
			return true;
		}
	}
	return false;
}

bool streamData::isRelayStream(const streamID_t id) throw()
{
	bool result = false;
	if (id > 0)
	{
		stackLock sml(g_streamMapLock);

		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);
		if (i != g_streamMap.end())
		{
			assert(g_streams.find((*i).second) != g_streams.end());
			result = !(*i).second->m_streamInfo.m_relayURL.empty();
		}
	}
	return result;
}

bool streamData::isBackupStream(const streamID_t id) throw()
{
	bool result = false;
	if (id > 0)
	{
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);
		if (i != g_streamMap.end())
		{
			assert(g_streams.find((*i).second) != g_streams.end());
			// check if there is a backup specified and also if we've been set as being a running backup
			result = (!(*i).second->m_streamInfo.m_backupURL.empty() && (*i).second->m_streamInfo.m_backup);
		}
	}
	return result;
}

bool streamData::getStreamInfo(const streamID_t id, streamInfo &info, extraInfo &extra) throw()
{
	if (id > 0)
	{
		extra.ypConnected = 0;

		stackLock sml(g_streamMapLock);
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);
		if (i == g_streamMap.end())
		{
			extra.ypErrorCode = 200;
			extra.isConnected = false;
			extra.isRelay = false;
			extra.isBackup = false;
		}
		else
		{
			assert(g_streams.find((*i).second) != g_streams.end());
			assert(g_streamSourceIsConnected.find((*i).second) != g_streamSourceIsConnected.end());

			info = (*i).second->m_streamInfo;
			extra.isConnected = g_streamSourceIsConnected[(*i).second];
			extra.isRelay = !(*i).second->m_streamInfo.m_relayURL.empty();
			extra.isBackup = (!(*i).second->m_streamInfo.m_backupURL.empty() && (*i).second->m_streamInfo.m_backup);

			extra.ypErrorCode = info.m_ypResponseCode;
			extra.ypConnected = ((*i).second->m_streamInfo.m_streamPublic && (info.m_ypResponseCode == 200));

		}
	}
	else
	{
		extra.ypConnected = 0;
		extra.ypErrorCode = 200;
		extra.isConnected = false;
		extra.isRelay = false;
		extra.isBackup = false;
	}

	return extra.isConnected;
}

void streamData::getStreamSongHistory(const streamID_t id, streamHistory_t& songHistory) throw()
{
	if (id > 0)
	{
		stackLock sml(g_streamSongHistoryMapLock);
		map<streamID_t, streamHistory_t>::const_iterator i = g_streamSongHistoryMap.find(id);
		if (i != g_streamSongHistoryMap.end())
		{
			songHistory = (*i).second;
		}
	}
}

bool streamData::getStreamNextSongs(const streamID_t id, uniString::utf8& currentSong,
									uniString::utf8& comingSoon,
									vector<uniString::utf8>& nextSongs) throw()
{
	bool isConnected = false;
	if (id > 0)
	{
		stackLock sml(g_streamMapLock);
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);
		if (i != g_streamMap.end())
		{
			assert(g_streams.find((*i).second) != g_streams.end());
			assert(g_streamSourceIsConnected.find((*i).second) != g_streamSourceIsConnected.end());
			isConnected = g_streamSourceIsConnected[(*i).second];
			currentSong = (*i).second->m_streamInfo.m_currentSong;
			comingSoon = (*i).second->m_streamInfo.m_comingSoon;
			nextSongs = (*i).second->m_streamInfo.m_nextSongs;
		}
	}
	return isConnected;
}

utf8 streamData::getContentType(const streamData::streamInfo &info) throw()
{
	utf8 content;
	if (info.m_uvoxDataType == MP3_DATA)
	{
		content = "MP3";
	}
	else if (info.m_uvoxDataType == AACP_DATA || info.m_uvoxDataType == AAC_LC_DATA)
	{
		content = "HE-AAC";
	}
	else if (info.m_uvoxDataType == OGG_DATA)
	{
		content = "OGG Vorbis";
	}
	else
	{
		content = aolxml::escapeXML(!info.m_contentType.empty() ? info.m_contentType : "unknown");
		if (content == "video/nsv")
		{
			content = "NSV";
		}
	}
	return content;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*********************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////

// file names for intro and backup files can have %d in them which should be replace by the bitrate
static uniFile::filenameType parseSpecialName(const uniFile::filenameType &rootName, const size_t bitrate) throw()
{
	uniFile::filenameType result = rootName;
	if (!result.empty())
	{
		uniFile::filenameType::size_type pos = result.find(uniFile::filenameType("%d"));
		if (pos != uniFile::filenameType::npos)
		{
			result.replace(pos,2,uniFile::filenameType(tos(bitrate)));
		}
	}
	return result;
}


int streamData::convertRawToUvox (vector<__uint8> &sc2buffer, const vector<__uint8> &buf,
        const int uvoxDataType, const int bitrate, const unsigned int samplerate) throw()
{
    __uint64 frameCount = 0;
    int last_size = 0, end = 0;
	const int len = (int)buf.size();
    const bool mp3 = (uvoxDataType == MP3_DATA);

    for (int i = 0; (i < (len - 8)) && !iskilled();)
    {
        unsigned int read_samplerate = 0;
        __uint8 asc_header[2] = {0};
        int read_bitrate = 0;
        const int found = (mp3 ? getMP3FrameInfo((const char*)&(buf[i]), &read_samplerate, &read_bitrate) :
                getADTSFrameInfo((const char*)&(buf[i]), &read_samplerate, asc_header));

        // need to find frames and that the input is the correct format!
        //
        // is a bit of a pain for AAC though as we've already done the
        // rough bitrate match when the advert / intro / backup was read
        // we'll just pass things through as though the bitrate is ok...
        if ((found > 0) && (mp3 && bitrate ? (read_bitrate == bitrate) : 1) &&
                (!mp3 || !samplerate || (samplerate == read_samplerate)))
        {
            if (!frameCount)
            {
                end = i;
            }

            i += (last_size = found);

            // only count valid full-size frames
            if (i <= len)
            {
                formMessage(&buf[end], last_size, uvoxDataType, sc2buffer);
                end += last_size;
                ++frameCount;
            }
            else
            {
                break;
            }
        }
        else
        {
            ++i;
        }
    }
    return frameCount;
}


// take raw file data and build sc1 and sc2 data buffers
void streamData::specialFileData::updateUvoxData (const int uvoxDataType,
        const int bitrate, const unsigned int samplerate) throw()
{
    const vector<__uint8> &buf = m_sc1Buffer;

    m_sc2Buffer.clear();
    if (buf.size() > 0) // can't take subscript of data[0] if siz==0
    {
        int frames = convertRawToUvox (m_sc2Buffer, buf, uvoxDataType, bitrate, samplerate);
        if (frames)
            m_samplerate = samplerate;
        m_missing = (frames ? false : true);
        m_lastUsed = ::time(NULL);
    }
}

// replace data with collection of uvox packets with given data type
void streamData::specialFileData::replaceData(const vector<__uint8> &data, const int uvoxDataType,
											  const int bitrate, const unsigned int samplerate) throw()
{
    stackLock sml(m_lock);

    m_sc1Buffer = data;

    updateUvoxData (uvoxDataType, bitrate, samplerate);
}


void streamData::specialFileData::release (specialFileData *f)
{
    AD_DEBUG_LOG ("[STREAMDATA] content release " + f->m_description + ", count was " + tos (f->m_refcount));
    if (f->m_refcount > 1)
    {
        --f->m_refcount;
        f->m_lastUsed = ::time (NULL);
        return;
    }
    delete f;
}


int streamData::specialFileData::verifyData (const utf8 &logString)
{
    // attempt to strip out any tags from the file
    // to increase client connection reliability

    size_t siz = m_sc1Buffer.size();
    // check for ID3v2.x tag
    if (siz > 3 &&
            (m_sc1Buffer[0] == 'I') &&
            (m_sc1Buffer[1] == 'D') &&
            (m_sc1Buffer[2] == '3'))
    {
        int id3len = make28BitValue(&m_sc1Buffer[6]) + 10;
        m_sc1Buffer.erase (m_sc1Buffer.begin(), m_sc1Buffer.begin()+id3len);
        siz = m_sc1Buffer.size();
    }

    // check for ID3v1.x tag
    if (siz > 128 &&
            (m_sc1Buffer[siz-128] == 'T') &&
            (m_sc1Buffer[siz-127] == 'A') &&
            (m_sc1Buffer[siz-126] == 'G'))
    {
        m_sc1Buffer.resize(siz -= 128);
    }

    // check for Lyrics3 tag
    if (siz > 20 &&
            (m_sc1Buffer[siz-9] == 'L') &&
            (m_sc1Buffer[siz-8] == 'Y') &&
            (m_sc1Buffer[siz-7] == 'R') &&
            (m_sc1Buffer[siz-6] == 'I') &&
            (m_sc1Buffer[siz-5] == 'C') &&
            (m_sc1Buffer[siz-4] == 'S') &&
            (m_sc1Buffer[siz-3] == '2') &&
            (m_sc1Buffer[siz-2] == '0') &&
            (m_sc1Buffer[siz-1] == '0'))
    {
        size_t length = 9;
        while (length < siz - 10)
        {
            if ((m_sc1Buffer[siz-length] == 'L') &&
                    (m_sc1Buffer[siz-length+1] == 'Y') &&
                    (m_sc1Buffer[siz-length+2] == 'R') &&
                    (m_sc1Buffer[siz-length+3] == 'I') &&
                    (m_sc1Buffer[siz-length+4] == 'C') &&
                    (m_sc1Buffer[siz-length+5] == 'S') &&
                    (m_sc1Buffer[siz-length+6] == 'B') &&
                    (m_sc1Buffer[siz-length+7] == 'E') &&
                    (m_sc1Buffer[siz-length+8] == 'G') &&
                    (m_sc1Buffer[siz-length+9] == 'I') &&
                    (m_sc1Buffer[siz-length+10] == 'N'))
            {
                m_sc1Buffer.resize(siz -= length);
                break;
			}
			++length;
		}
    }

    // check for Ape tag
    if (siz > 8)
    {
        size_t pos = 0;
        while (pos < siz - 8)
        {
            if (siz > 32 &&
                    (m_sc1Buffer[pos] == 'A') &&
                    (m_sc1Buffer[pos+1] == 'P') &&
                    (m_sc1Buffer[pos+2] == 'E') &&
                    (m_sc1Buffer[pos+3] == 'T') &&
                    (m_sc1Buffer[pos+4] == 'A') &&
                    (m_sc1Buffer[pos+5] == 'G') &&
                    (m_sc1Buffer[pos+6] == 'E') &&
                    (m_sc1Buffer[pos+7] == 'X'))
            {
                struct HeaderData
                {
                    __uint32 version;
                    __uint32 size;
                    __uint32 items;
                    __uint32 flags;
                };
                HeaderData *header = (HeaderData*)&(m_sc1Buffer[pos+8]);
                int size = header->size;
                int flags = header->flags;

                enum
                {
                    FLAG_HEADER_HAS_HEADER = (1 << 31),
                    FLAG_HEADER_NO_FOOTER = (1 << 30),
                    FLAG_HEADER_IS_HEADER = (1 << 29)
                };

                if (!!(flags & FLAG_HEADER_IS_HEADER) && !(flags & FLAG_HEADER_NO_FOOTER))
                {
                    size += 32;
                }
                if (!(flags & FLAG_HEADER_IS_HEADER) && !!(flags & FLAG_HEADER_HAS_HEADER))
                {
                    size += 32;
                }
                m_sc1Buffer.resize(siz -= size);
                break;
            }
            ++pos;
        }
    }
    int len = (int)m_sc1Buffer.size();
    if (len > 4)
    {
        parserInfo *parser = NULL;
        unsigned syncFrames = 0;

        getFrameInfo (parser, syncFrames, (unsigned char*)&(m_sc1Buffer[0]), len, 0);
        if (parser)
        {
            utf8 msg = logString;

            if (parser->m_reduce)
                m_sc1Buffer.resize (len - parser->m_reduce);
            msg += " Loaded ";
            msg += m_description;
            msg += ", ";
            msg += tos(m_sc1Buffer.size());
            msg += " bytes, ";
            msg += parser->m_description;
            if (parser->m_duration < 1)
            {
                msg += " (frames ";
                msg += tos(parser->m_frameCount);
                msg += ")";
            }
            m_bitrate = parser->m_bitrate;
            m_samplerate = parser->m_samplerate;
            m_duration = parser->m_duration;
            DEBUG_LOG (msg, LOGNAME);
            updateUvoxData (parser->getUvoxType(), parser->m_bitrate, parser->m_samplerate);
            delete parser;
            return 0;
        }
        if (m_description.empty() == false)
            WLOG (logString + " Trouble parsing " + m_description);

    }
    m_sc1Buffer.clear();
    return -1;
}


int streamData::cleanFileData(uniFile::filenameType fn, vector<__uint8> &buffer, size_t siz,
							  const int bitrate, const unsigned int samplerate,
							  const int /*uvoxDataType*/, const utf8& logString,
							  const utf8& /*description*/, unsigned int &read_samplerate)
{
	// attempt to strip out any tags from the file
	// to increase client connection reliability

	// check for ID3v2.x tag
	if (siz > 3 &&
	    (buffer[0] == 'I') &&
	    (buffer[1] == 'D') &&
	    (buffer[2] == '3'))
	{
        int id3len = make28BitValue(&buffer[6]) + 10;
        buffer.erase (buffer.begin(), buffer.begin()+id3len);
		siz = buffer.size();
	}

	// check for ID3v1.x tag
	if (siz > 128 &&
	    (buffer[siz-128] == 'T') &&
	    (buffer[siz-127] == 'A') &&
	    (buffer[siz-126] == 'G'))
	{
		buffer.resize(siz -= 128);
	}

	// check for Lyrics3 tag
	if (siz > 20 &&
	    (buffer[siz-9] == 'L') &&
	    (buffer[siz-8] == 'Y') &&
	    (buffer[siz-7] == 'R') &&
	    (buffer[siz-6] == 'I') &&
	    (buffer[siz-5] == 'C') &&
	    (buffer[siz-4] == 'S') &&
	    (buffer[siz-3] == '2') &&
	    (buffer[siz-2] == '0') &&
	    (buffer[siz-1] == '0'))
	{
		size_t length = 9;
		while (length < siz - 10)
		{
		   if ((buffer[siz-length] == 'L') &&
			   (buffer[siz-length+1] == 'Y') &&
			   (buffer[siz-length+2] == 'R') &&
			   (buffer[siz-length+3] == 'I') &&
			   (buffer[siz-length+4] == 'C') &&
			   (buffer[siz-length+5] == 'S') &&
			   (buffer[siz-length+6] == 'B') &&
			   (buffer[siz-length+7] == 'E') &&
			   (buffer[siz-length+8] == 'G') &&
			   (buffer[siz-length+9] == 'I') &&
			   (buffer[siz-length+10] == 'N'))
			{
				buffer.resize(siz -= length);
				break;
			}
			++length;
		}
	}

	// check for Ape tag
	if (siz > 8)
	{
		size_t pos = 0;
		while (pos < siz - 8)
		{
			if (siz > 32 &&
			    (buffer[pos] == 'A') &&
			    (buffer[pos+1] == 'P') &&
			    (buffer[pos+2] == 'E') &&
			    (buffer[pos+3] == 'T') &&
			    (buffer[pos+4] == 'A') &&
			    (buffer[pos+5] == 'G') &&
			    (buffer[pos+6] == 'E') &&
			    (buffer[pos+7] == 'X'))
			{
				struct HeaderData
				{
					__uint32 version;
					__uint32 size;
					__uint32 items;
					__uint32 flags;
				};
				HeaderData *header = (HeaderData*)&(buffer[pos+8]);
				int size = header->size;
				int flags = header->flags;

				enum
				{
					FLAG_HEADER_HAS_HEADER = (1 << 31),
					FLAG_HEADER_NO_FOOTER = (1 << 30),
					FLAG_HEADER_IS_HEADER = (1 << 29)
				};

				if (!!(flags & FLAG_HEADER_IS_HEADER) && !(flags & FLAG_HEADER_NO_FOOTER))
				{
					size += 32;
				}
				if (!(flags & FLAG_HEADER_IS_HEADER) && !!(flags & FLAG_HEADER_HAS_HEADER))
				{
					size += 32;
				}
				buffer.resize(siz -= size);
				break;
			}
			++pos;
		}
	}

	int read_bitrate = 0;
    do
    {
			int len = (int)buffer.size();//, start = 0, end = 0;
			if (len > 4)
			{
                parserInfo *parser = NULL;
                unsigned syncFrames = 0;
                getFrameInfo (parser, syncFrames, (unsigned char*)&(buffer[0]), len, 0);
                if (parser)
                {
                    read_bitrate = parser->m_bitrate;
                    read_samplerate = parser->m_samplerate;
                    int ok = true;
                    utf8 msg = logString;

                    if (((read_bitrate == bitrate) || !bitrate) &&
                            ((read_samplerate == samplerate) || !samplerate))
                        msg += " Loaded ";
                    else
                    {
                        msg += " Mismatched ";
                        ok = false;
                    }
                    msg += fn;
                    msg += ", ";
                    msg += tos(buffer.size());
                    msg += " bytes, ";
                    msg += parser->m_description;
                    if (parser->m_duration < 1)
                    {
                        msg += " (frames ";
                        msg += tos(parser->m_frameCount);
                        msg += ")";
                    }
                    delete parser;
                    if (ok)
                    {
                        DEBUG_LOG (msg, LOGNAME);
                        break;
                    }
                    WLOG (msg, LOGNAME);
                }
                else if (fn.empty() == false)
                    WLOG (logString + " Trouble parsing " + fn);
			}
            buffer.clear();

    } while (0);

	return read_bitrate;
}

int streamData::specialFileData::loadFromFile(const uniFile::filenameType &name, const int bitrate,
											  const int /*uvoxDataType*/, const unsigned int samplerate,
											  const utf8& logString) throw()
{
	stackLock sml(m_lock);

	m_sc1Buffer.clear();
	m_sc2Buffer.clear();
	vector<__uint8> &buffer = m_sc1Buffer;
	int read_bitrate = 0;

	uniFile::filenameType fn = parseSpecialName(name, bitrate);
	if (!fn.empty())
	{
        // KH, fopen etc can take some time if you are dealing with many files at once, in icecast
        // I used the lower level open/seek/tell calls in the end. library allocates buffers.
        //
		FILE *f = uniFile::fopen(fn,"rb");
		if (f)
		{
			if (!::fseek(f, 0, SEEK_END))
			{
				int maxSpecialFileSize = gOptions.maxSpecialFileSize();
				size_t siz = ::ftell(f);
				if ((siz > 0) && ((int)siz <= maxSpecialFileSize))
				{
					::fseek(f, 0, SEEK_SET);
					buffer.resize (siz);
					if (::fread(&(buffer[0]), 1, siz, f) != siz)
					{
						ELOG(logString + " Error reading " + m_description + " file `" + fn + "'");
					}
					else
                    {
                        unsigned int read_samplerate = 0;
                        size_t original_size = buffer.size();

                        verifyData ("");
                        read_bitrate = m_bitrate;
                        read_samplerate = m_samplerate;
						if (buffer.size() > 0)
						{
							ILOG(logString + " Loaded " + m_description + " file `" + fn + "' (" + tos(original_size) + " bytes"  +
												(original_size != buffer.size() ? " - processed down to " + tos(buffer.size()) + " bytes" : "") + ")");
						}
						else
						{
							WLOG(logString + "Skipped " + m_description + " file `" + fn +
								 "' as it is incompatible with the current stream format. Expected " +
								 tos(bitrate) + " kbps, got " + (read_bitrate > 0 ? tos(read_bitrate) : "unknown") +
								 " kbps. Expected " + sampleRateStr(samplerate) + ", got " + sampleRateStr(read_samplerate) + ".");
						}
					}
				}
				else
				{
					ELOG(logString + m_description + " " + fn + " has bad size (" + tos(siz) + ").");
				}
			}
			else
			{
				ELOG(logString + "Could not seek to end of " + m_description + " file `" + fn + "'");
			}
 			::fclose(f);
 		}
		else
		{
			ELOG(logString + "Could not open " + m_description + " file `" +
				 fn + "' (" + errMessage().hideAsString() + ")");
		}
	}
	return read_bitrate;
}

void streamData::_setupBuffers(const utf8& logString, const bool re_init) throw()
{
	// load intro and backup files
	size_t stream_ID = ID();
	utf8 introFile = gOptions.stream_introFile(stream_ID);
	if (!gOptions.read_stream_introFile(stream_ID))
	{
		introFile = gOptions.introFile();
	}

	m_introFile.loadFromFile(introFile, m_streamInfo.m_streamBitrate,
							 m_streamInfo.m_uvoxDataType,
							 m_streamInfo.m_streamSampleRate, logString);


	utf8 backupFile = gOptions.stream_backupFile(stream_ID);
	if (!gOptions.read_stream_backupFile(stream_ID))
	{
		backupFile = gOptions.backupFile();
	}

	m_backupFile.loadFromFile(backupFile, m_streamInfo.m_streamBitrate,
							  m_streamInfo.m_uvoxDataType,
							  m_streamInfo.m_streamSampleRate, logString);

#if 0
	utf8 adTestFile = gOptions.stream_adTestFile(stream_ID);
	if (!gOptions.read_stream_adTestFile(stream_ID))
	{
		adTestFile = gOptions.adTestFile();
	}

	m_adTestFile.loadFromFile(adTestFile, m_streamInfo.m_streamBitrate,
							  m_streamInfo.m_uvoxDataType,
							  m_streamInfo.m_streamSampleRate, logString);

	utf8 adTestFile2 = gOptions.stream_adTestFile2(stream_ID);
	if (!gOptions.read_stream_adTestFile2(stream_ID))
	{
		adTestFile2 = gOptions.adTestFile2();
	}

	m_adTestFile2.loadFromFile(adTestFile2, m_streamInfo.m_streamBitrate,
							   m_streamInfo.m_uvoxDataType,
							   m_streamInfo.m_streamSampleRate, logString);

	utf8 adTestFile3 = gOptions.stream_adTestFile3(stream_ID);
	if (!gOptions.read_stream_adTestFile3(stream_ID))
	{
		adTestFile3 = gOptions.adTestFile3();
	}

	m_adTestFile3.loadFromFile(adTestFile3, m_streamInfo.m_streamBitrate,
							   m_streamInfo.m_uvoxDataType,
							   m_streamInfo.m_streamSampleRate, logString);

	utf8 adTestFile4 = gOptions.stream_adTestFile4(stream_ID);
	if (!gOptions.read_stream_adTestFile4(stream_ID))
	{
		adTestFile4 = gOptions.adTestFile4();
	}

	m_adTestFile4.loadFromFile(adTestFile4, m_streamInfo.m_streamBitrate,
							   m_streamInfo.m_uvoxDataType,
							   m_streamInfo.m_streamSampleRate, logString);
#endif
	/////////////////////////////////////////////

	// determine configuration of ring buffer
	size_t requestedSize = gOptions.fixedBufferSize();

	if ((gOptions.bufferType() == 1) && (m_streamInfo.m_streamBitrate > 0))
	{
		DEBUG_LOG(logString + "Calculating buffer size from time (" +
							  tos(m_streamInfo.m_streamBitrate) + " kbps for " +
							  tos(gOptions.adaptiveBufferSize()) + "s)", LOGNAME, stream_ID);
		requestedSize = (size_t)((gOptions.adaptiveBufferSize() * m_streamInfo.m_streamBitrate * 1024) / 8);
	}

	size_t bufferHardLimit = gOptions.bufferHardLimit();
	requestedSize = min(requestedSize, bufferHardLimit);

	DEBUG_LOG(logString + "Requested fixed size of " + tos(requestedSize), LOGNAME, stream_ID);
	// make sure it's a power of two
	size_t powerOfTwo = 1;
	while (true)
	{
		size_t n = powerOfTwo * 2;
		if (n >= requestedSize)
		{
			requestedSize = n;
			break;
		}
		else	
		{
			powerOfTwo = n;
		}
	}

	if (re_init)
	{
		DEBUG_LOG(logString + "Re-initialising buffers", LOGNAME, stream_ID);
		m_sc1_ring_buffer.m_writePtr = m_sc21_ring_buffer.m_writePtr = 0;
	}

	DEBUG_LOG(logString + "Using buffer size of " + tos(requestedSize), LOGNAME, stream_ID);
	m_sc1_ring_buffer.m_data.resize(requestedSize);
	m_sc21_ring_buffer.m_data.resize(requestedSize);
	m_sc1_ring_buffer.m_writePtr = m_sc21_ring_buffer.m_writePtr = 0;
	m_sc1_ring_buffer.m_ptrMask = m_sc21_ring_buffer.m_ptrMask = (requestedSize - 1);
}

uniString::utf8 streamData::getYPStreamTitle() throw()
{
	// in order to determine if we should update the title in YP, we should
	// get the title based on where a client would be if he/she connected right now
	const sc1MetadataAndExtensionInfo md = getSc1Metadata(0xFFFFFFFF);
	if (!md.m_songTitle.empty())
	{
		const utf8::size_type pos1 = md.m_songTitle.find(utf8("StreamTitle='")),
							  pos2 = (pos1 == utf8::npos ? utf8::npos : md.m_songTitle.find(utf8("';"),pos1+13));

		utf8 metadata = stripWhitespace((pos1 == utf8::npos) || (pos2 == utf8::npos) ? (utf8)"" : md.m_songTitle.substr(pos1+13,pos2-pos1-13));
		// we use this to provide a nicer title to the YP when the advert update occurs
		int remove_size = 7;
		utf8::size_type pos = metadata.find(utf8("Advert:"));
		if (pos != 0)
		{
			pos = metadata.find(utf8("Advert!"));
		}

		if (!metadata.empty() && (pos == 0))
		{
			// got a first matching block
			metadata = metadata.replace(0, remove_size, (utf8)"");

			// look for an end block
			pos = metadata.find(utf8("Advert!"));
			if (pos == utf8::npos)
			{
				remove_size = 7;
				pos = metadata.find(utf8("Advert:"));
			}
			else
			{
				remove_size = 12;
			}

			if (pos != utf8::npos)
			{
				metadata = metadata.replace(pos, remove_size, (utf8)"");
			}

			metadata = stripWhitespace(metadata);
		}

		return metadata;
	}
	return (utf8)"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// YP2 Messaging ///////////////////////////////////////////////////////////////////////////////
void streamData::_YP2_add() throw() // yp2
{
	// only allow formats that we support
	if (isAllowedType(m_streamInfo.m_uvoxDataType) &&
		(m_streamInfo.m_avgBitrate > 0) &&
		(m_streamInfo.m_streamSampleRate > 0))
	{
		if ((!m_dead) && m_ID && m_streamInfo.m_streamPublic)
		{
#if 0
			// in order to determine if we should update the title in YP, we should
			// get the title based on where a client would be if he/she connected right now
			const sc1MetadataAndExtensionInfo md = getSc1Metadata(0xFFFFFFFF);
			const utf8 songTitle = getYPStreamTitle();

			yp2::ypInfo info(m_yp2SessionKey, m_ID, m_streamInfo.m_authHash,
							 m_streamInfo.m_vbr, m_streamInfo.m_streamPublic,
							 !m_sc21AlbumArtData[0].empty(), !m_sc21AlbumArtData[1].empty());

			info.bitrate = m_streamInfo.m_avgBitrate;
			info.samplerate = m_streamInfo.m_streamSampleRate;
			info.peakClientConnections = m_streamInfo.m_streamPeakUser;
			info.maxClientConnections = ((m_streamInfo.m_streamMaxUser > 0) &&
										 (m_streamInfo.m_streamMaxUser < gOptions.maxUser()) ?
										 m_streamInfo.m_streamMaxUser : gOptions.maxUser());
			info.mimeType = m_streamInfo.m_contentType;
			info.relayURL = m_streamInfo.m_relayURL;
			info.songMetadataForYP2 = (!md.m_songMetadataForYP2.empty() ? stripWhitespace(md.m_songMetadataForYP2) :
									   (!songTitle.empty() ? "<title seq=\"1\">" + aolxml::escapeXML(songTitle) + "</title>" : ""));
			info.sourceIdent = m_streamInfo.m_sourceIdent;
			info.sourceUser = m_streamInfo.m_streamUser;

			m_yp2SessionKey = yp2::add(info, m_creating);
#endif
			if (m_yp2SessionKey != yp2::INVALID_SESSION_KEY)
			{
				m_lastTouchTitle.clear();
			}

			if (!m_maxYPInterval)
			{
				m_maxYPInterval = gOptions.ypReportInterval();	
			}
		}
	}
}

void streamData::YP2_add() throw() // yp2
{
	stackLock sml(m_stateLock);

	_YP2_add();
}

// check status and update stream info
void streamData::YP2_updateInfo(const yp2::stationInfo &info) throw()
{
    stackLock sml(m_stateLock);
    int maxbitrate = info.m_allowMaxBitrate > 0 ? info.m_allowMaxBitrate : m_streamInfo.m_allowMaxBitrate_global;
    int allformats = info.m_allowAllFormats < 0 ? m_streamInfo.m_allowAllFormats_global : info.m_allowAllFormats;

    m_streamInfo.m_streamName = info.m_streamTitle;
    m_streamInfo.m_radionomyID = info.m_radionomyID;
#if 0
    for (int i = 0; i < 5; i++)
    {
        m_streamInfo.m_streamGenre[i] = info.m_streamGenre[i];
    }
    m_streamInfo.m_streamURL = info.m_broadcasterURL;
    m_streamInfo.m_backupServer = info.m_backupServer;
    m_streamInfo.m_backupServersList = info.m_backupServersList;
    m_streamInfo.m_publicIP = info.m_publicIP;
#endif
    if (m_streamInfo.m_ypResponseCode == 0)
        metrics::metrics_stream_up (m_ID, m_streamInfo.m_radionomyID, info.m_serverID, gOptions.publicIP(), getStartTime());

    m_streamInfo.m_advertMode = info.m_advertMode;
    m_streamInfo.m_ypResponseCode = info.m_responseCode;

    // use global default if per-stream settings not provided
    m_streamInfo.m_allowSSL = info.m_allowSSL < 0 ? m_streamInfo.m_allowSSL_global : info.m_allowSSL;
    m_streamInfo.m_allowAllFormats = allformats;
    m_streamInfo.m_allowMaxBitrate = maxbitrate;
    m_streamInfo.m_allowBackupURL = info.m_allowBackupURL < 0 ? m_streamInfo.m_allowBackupURL_global : info.m_allowBackupURL;
    m_streamInfo.m_stationID = info.m_stationID;
    m_streamInfo.m_serverID = info.m_serverID;
    advertGroups.setType (info.m_advertType);

    if (gOptions.adMetricsDebug())
    {
        string s = "using licence attribs, bitrate ";
        if (m_streamInfo.m_allowMaxBitrate)
            s += tos (m_streamInfo.m_allowMaxBitrate);
        else
            s += "unrestricted";
        s += ", fmts ";
        s += tos (m_streamInfo.m_allowAllFormats);
        s += ", ssl ";
        s += tos (m_streamInfo.m_allowSSL);
        s += ", backup ";
        s += tos (m_streamInfo.m_allowBackupURL);
        s += ", ad ";
        s += tos (m_streamInfo.m_advertMode);
        if (m_streamInfo.m_advertMode == 1)
        {
            s += ", ad mode ";
            s += info.m_advertType.hideAsString();
            if (advertGroups.m_type == ADVERT_MAP_PAUSE)  // maybe magic as well
            {
                size_t min_buf = m_streamInfo.m_streamBitrate * 1000 * 300 / 8;    // 5 minutes
                size_t powerOfTwo = 1;
                while (true)
                {
                    size_t n = powerOfTwo * 2;
                    if (n >= min_buf)
                    {
                        min_buf = n;
                        break;
                    }
                    else
                    {
                        powerOfTwo = n;
                    }
                }

                if (min_buf > m_sc1_ring_buffer.m_data.size())
                {
                    m_sc1_ring_buffer.m_data.resize (min_buf);
                    m_sc21_ring_buffer.m_data.resize(min_buf);
                    m_sc1_ring_buffer.m_ptrMask = m_sc21_ring_buffer.m_ptrMask = (min_buf - 1);
                    s += ", resized buffer to ";
                    s += tos (min_buf);
                }
            }
        }
        DLOG (s, LOGNAME, m_ID);
    }
}

bool streamData::YP2_addSuccessful(int &addFailIgnore, int &errorCode) throw()
{
	stackLock sml(m_stateLock);

	yp2::addState_t result = yp2::addStatus(m_yp2SessionKey, addFailIgnore, errorCode);
	return (!addFailIgnore ? (result == yp2::ADD_SUCCEEDED) : true);
}

void streamData::_YP2_remove() throw()
{
	if ((!m_dead) && m_ID && m_streamInfo.m_streamPublic)
	{
#if 0
		yp2::ypInfo info(m_yp2SessionKey, m_ID, m_streamInfo.m_authHash,
						 m_streamInfo.m_vbr, m_streamInfo.m_streamPublic);

		info.streamStartTime = m_startTime;
		info.peakClientConnections = m_streamInfo.m_streamPeakUser;
		info.maxClientConnections = ((m_streamInfo.m_streamMaxUser > 0) &&
									 (m_streamInfo.m_streamMaxUser < gOptions.maxUser()) ?
									 m_streamInfo.m_streamMaxUser : gOptions.maxUser());

		yp2::remove(info);
#endif
        pushMetricsYP();
        metrics::metrics_stream_down (m_ID, m_streamInfo.m_radionomyID,
                m_streamInfo.m_serverID, gOptions.publicIP(), m_startTime);
	}
}

void streamData::YP2_remove() throw()
{
	stackLock sml(m_stateLock);

	_YP2_remove();
}

#if 0
inline void streamData::YP2_update() throw()
{
        m_stateLock.lock();

	time_t t = ::time(NULL), elapsed = (t - m_lastYPTime);
	if (((m_streamInfo.m_streamPublic || !gOptions.cdn().empty()) && (!m_dead) && m_ID &&
						// make sure we're looking at min and max intervals to ensure we send updates
						// even when playing long mixes otherwise we can potentially drop off the YP.
						//
						// 2.4.8 we now have a fixed min-interval but if there's been no YP add yet
						// then we have a 2sec minimum which is a bit more response whilst allowing
						// for failed sources / relay connections to do things so we don't YP spam
						(elapsed >= (m_yp2SessionKey ? 10 : 2) || elapsed >= m_maxYPInterval)))
	{
		// make sure we're added
		if (m_yp2SessionKey == 0)
		{
			_YP2_add();
		}

		if (m_yp2SessionKey)
		{
			// in order to determine if we should update the title in YP, we should
			// get the title based on where a client would be if he/she connected right now
			m_stateLock.unlock();
			const uniString::utf8 songTitle = getYPStreamTitle();

			// its touch time
			if ((songTitle != m_lastTouchTitle) || (elapsed >= m_maxYPInterval))
			{
				yp2::ypInfo info(m_yp2SessionKey, m_ID, m_streamInfo.m_authHash,
								 m_streamInfo.m_vbr, m_streamInfo.m_streamPublic,
								 !m_sc21AlbumArtData[0].empty(), !m_sc21AlbumArtData[1].empty());

				stats::statsData_t data;
				stats::getStats(m_ID, data, true);

				info.numListeners = data.connectedListeners;							// li
				info.avgUserListenTime = data.avgUserListenTime;						// alt
				info.numberOfClientsConnectedMoreThanFiveMinutes = data.newSessions;	// cm
				info.numberOfClientConnectsSinceLastUpdate = data.newConnects;			// ht
				info.numUniqueListeners = data.uniqueListeners;

				const ringBufferAccess_t startPos = getClientStartPosition();
				const sc1MetadataAndExtensionInfo md = getSc1Metadata((!startPos ? 0xFFFFFFFF : startPos));

				//info.peakClientConnections = m_streamInfo.m_streamPeakUser;
				info.maxClientConnections = ((m_streamInfo.m_streamMaxUser > 0) &&
											 (m_streamInfo.m_streamMaxUser < gOptions.maxUser()) ?
											 m_streamInfo.m_streamMaxUser : gOptions.maxUser());
				info.songMetadataForYP2 = (!md.m_songMetadataForYP2.empty() ? stripWhitespace(md.m_songMetadataForYP2) :
										   "<title seq=\"1\">" + aolxml::escapeXML(songTitle) + "</title>");
				info.sourceIdent = m_streamInfo.m_sourceIdent;
				info.sourceUser = m_streamInfo.m_streamUser;
				info.bitrate = m_streamInfo.m_avgBitrate;
				info.samplerate = m_streamInfo.m_streamSampleRate;
				info.mimeType = m_streamInfo.m_contentType;
				info.relayURL = m_streamInfo.m_relayURL;

				m_stateLock.lock();
				yp2::updateResult ur = yp2::update(info);

				m_maxYPInterval = ur.m_maxYPInterval;
				if (ur.m_requestQueued)
				{
					m_lastYPTime = t;
					m_lastTouchTitle = songTitle;
				}
				m_stateLock.unlock();
			}
			return;
		}
	}
        m_stateLock.unlock();
}
#endif

int streamData::YP_SrvID(const streamID_t id) throw()
{
	if (id > 0)
	{
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);
		if (i == g_streamMap.end())
		{
			return 0;
		}
		return yp2::getSrvID((*i).second->m_yp2SessionKey);
	}
	return 0;
}

int streamData::YP_StnID(const streamID_t id) throw()
{
	if (id > 0)
	{
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(id);
		if (i == g_streamMap.end())
		{
			return 0;
		}
		return yp2::getStnID((*i).second->m_yp2SessionKey);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

streamData::streamData(const streamSetup& setup) throw()
	: m_ID(setup.m_sid), m_nextYPPush(::time(NULL)+10),
	  m_maxYPInterval(gOptions.ypReportInterval()),
	  m_yp2SessionKey(111), m_creating(0),
	  m_kill(0), m_dead(0), m_adTest(0), m_insertAdvert(false),
	  m_lastStreamSampleRate(setup.m_sampleRate),
	  m_lastStreamBitrate(0), m_syncFrameCount(0),
	  m_introFile("intro"), m_backupFile("backup"),
	  m_adTestFile("test advert"), m_adTestFile2("test advert"),
	  m_adTestFile3("test advert"), m_adTestFile4("test advert"),
      advertGroups(this), m_parser(NULL)
{
	sourceReconnect(setup);
	_setupBuffers(srcAddrLogString(setup.m_srcAddr, setup.m_srcPort, setup.m_sid));
}

streamData::~streamData() throw()
{
	DEBUG_LOG("[STREAMDATA sid=" + tos(m_ID) + "] " + __FUNCTION__, LOGNAME, m_ID);

	//YP2_remove();

	m_sc1_ring_buffer.m_writePtr = m_sc21_ring_buffer.m_writePtr = 0;
    delete m_parser;
}

// sc1 style. Return false if stream is already connected
void streamData::sourceReconnect(const streamSetup& setup) throw()
{
	stackLock sml(m_stateLock);

	const bool uvox = (setup.m_sourceType == SHOUTCAST2),
			   native = (setup.m_sourceType != HTTP);

	m_streamInfo.m_backup = setup.m_backup;
	m_streamInfo.m_streamUser = setup.m_streamUser;
	m_streamInfo.m_allowPublicRelay = setup.m_allowPublicRelay;
	m_streamInfo.m_streamMaxBitrate = setup.m_maxStreamBitrate;
	m_streamInfo.m_streamMinBitrate = setup.m_minStreamBitrate;
	m_streamInfo.m_streamMaxUser = setup.m_maxStreamUser;
	m_streamInfo.m_streamSampleRate = setup.m_sampleRate;
	m_streamInfo.m_authHash = setup.m_authHash;
	m_streamInfo.m_vbr = setup.m_vbr;

	m_streamInfo.m_srcAddr = setup.m_srcAddr;
	m_streamInfo.m_srcPort = setup.m_srcPort;

	m_streamInfo.m_relayURL = setup.m_relayURL;
	m_streamInfo.m_backupURL = setup.m_backupURL;

	if (!uvox)
	{
		m_streamInfo.m_streamName = mapGet(setup.m_headers, (native ? "icy-name" : "ice-name"), (utf8)"");
		m_streamInfo.m_streamGenre[0] = mapGet(setup.m_headers, (native ? "icy-genre" : "ice-genre"), utf8("Misc"));
	}
	else
	{
		m_streamInfo.m_streamName = setup.m_config.m_icyName;
		m_streamInfo.m_streamGenre[0] = setup.m_config.m_icyGenre;
	}

	for (int i = 1; i < 5; i++)
	{
		m_streamInfo.m_streamGenre[i].clear();
	}

	if (!uvox)
	{
		m_streamInfo.m_streamBitrate = getStreamBitrate(setup.m_headers);
		m_streamInfo.m_streamSampleRate = getStreamSamplerate(setup.m_headers);
		m_streamInfo.m_streamURL = mapGet(setup.m_headers, (native ? "icy-url" : "ice-url"), (utf8)"http://www.shoutcast.com");
		// sanity handling to map things to something which the YP2 will definitely like
		m_streamInfo.m_contentType = fixMimeType(mapGet(setup.m_headers, "content-type", utf8("audio/mpeg")));
		m_streamInfo.m_streamPublic = mapGet(setup.m_headers, (native ? "icy-pub" : "ice-pub"), false);
	}
	else
	{
		m_streamInfo.m_streamBitrate = (setup.m_config.m_avgBitrate / 1000);
		m_streamInfo.m_streamURL = setup.m_config.m_icyURL;
		m_streamInfo.m_contentType = setup.m_config.m_mimeType;
		m_streamInfo.m_streamPublic = (setup.m_config.m_icyPub ? true : false);
	}

	// see if there's a per-stream option else revert to master
	utf8 pub = toLower(gOptions.stream_publicServer(setup.m_sid));
	if (pub.empty())
	{
		pub = toLower(gOptions.publicServer());
	}
	if (pub == "always")
	{
		m_streamInfo.m_streamPublic = true;
	}
	else if (pub == "never")
	{
		m_streamInfo.m_streamPublic = false;
	}

	m_streamInfo.m_sourceType = setup.m_sourceType;

	if (!uvox)
	{
		m_streamInfo.m_avgBitrate = m_streamInfo.m_minBitrate =
		m_streamInfo.m_maxBitrate = (m_streamInfo.m_streamBitrate * 1000);
	}
	else
	{
		m_streamInfo.m_avgBitrate = setup.m_config.m_avgBitrate;
		m_streamInfo.m_minBitrate = m_streamInfo.m_maxBitrate = setup.m_config.m_maxBitrate;
	}

	if ((m_streamInfo.m_contentType == "audio/ogg" || m_streamInfo.m_contentType == "application/ogg"))
	{
		m_streamInfo.m_uvoxDataType = OGG_DATA;
		m_streamInfo.m_vbr = true;
	}
	else if ((m_streamInfo.m_contentType == "audio/aac") || (m_streamInfo.m_contentType == "audio/aacp"))
	{
		m_streamInfo.m_uvoxDataType = AACP_DATA;
	}
	else
	{
		m_streamInfo.m_uvoxDataType = MP3_DATA;
	}

	// uvox codes for intro and backup files will be wrong if contentType is empty. This is only a problem
	// if we are using vanilla uvox 2 which does not send a mime type (uvox 2.1 does, however)
	if (m_streamInfo.m_contentType.empty())
	{
		WLOG("Content type of stream " + tos(m_ID) + " is empty. Intro and backup files may not work.", LOGNAME, m_ID);
	}

	g_streamUptime[m_ID] = ::time(NULL);
    delete m_parser;
    m_parser = NULL;
}


MP3_FrameInfo *streamData::detectMP3 (unsigned int &failureThresh, const unsigned char *buf, unsigned int buflen, unsigned chk)
{
    unsigned loop = chk ? chk : 10000, remain = buflen, frames = 0;
    MP3_FrameInfo info, *parser = NULL;
    const unsigned char *p = buf;
    __uint64 samples = 0;

    if (buflen < 3000) // get sufficient data to check, saves failing part way through
        return NULL;
    int len = getMP3FrameInfo (p, buflen, info);
    // check a few frames to see about consistency
    while (len > 0)
    {
        //DLOG ("loop is " + tos(loop));
        len = info.verifyFrame (p, remain);
        if (len <= 0 || len > remain)
            break;
        samples += info.m_samples;
        frames++;
        p += len;
        remain -= len;
        loop--;
        // DLOG("Detecting frames, loop " + tos(loop) + ", remain " + tos(remain));
    }
    if (samples > 1000)
    {
        failureThresh += 100;
        parser = new MP3_FrameInfo (buf, buflen);
        parser->m_inUse = true;
        parser->m_description += info.getVersionName();
        parser->m_description += " layer ";
        parser->m_description += info.getLayerName();
        parser->m_description += (info.m_mono ? " mono" : " stereo");
        if (chk == 0 && loop && samples && info.m_samplerate)
        {
            parser->m_frameCount = frames;
            parser->m_duration = ((float)samples / info.m_samplerate);
            parser->m_description += " duration ";
            parser->m_description += tos (parser->m_duration);
            parser->m_description += "s";
        }
        if (remain && chk == 0 && loop)
        {
            parser->m_description += ", reduced size by ";
            parser->m_description += tos (remain);
            parser->m_reduce = remain;
        }
    }
    return parser;
}

static unsigned long getMSB4 (unsigned long v)
{
    unsigned long m = v;
    int c = 0;

    for (; m > 15; c++, m >>= 1)
        ;
    DEBUG_LOG ("bitrate estimate mark " + tos(m));
    if (m == 0xF)  // binary 1111 is not a normal bit pattern, adjust to 1000
        m = 0x10;
    if (m == 0xB)  // binary 1011 is not a normal bit pattern, adjust to 1100
        m = 0xC;
    return c ? m << c : m;
}

AAC_FrameInfo *streamData::detectAAC (unsigned int &failureThresh, const unsigned char *buf, unsigned int buflen, unsigned chk)
{
    unsigned int loop = chk ? chk : 10000, remain = buflen;
    int bytes = 0;
    float fcount = 0;
    AAC_FrameInfo info, *parser = NULL;
    const unsigned char *p = buf;

    if (buflen < 8000) // get sufficient data to check, saves failing part way through
        return 0;
    int len = getAACFrameInfo (p, buflen, info);

    // check a few frames to see about consistency
    while (len > 0)
    {
        len = info.verifyFrame (p, remain);
        if (len == 0 || len > remain) // short
            break;
        if (len < 0)    // failed to match
            break;
        p += len;
        remain -= len;
        bytes += (len - 7); // drop the adts frame header
        fcount += info.m_blocks;
        loop--;
        // DLOG("Detecting frames, loop " + tos(loop) + ", remain " + tos(remain));
    }
    if (bytes > 1000)
    {
        parser = new AAC_FrameInfo (buf, buflen);
        failureThresh += 100;
        parser->m_inUse = true;
        info.m_description += stringUtil::tos (info.m_samplerate);
        info.m_description += "hz";

        // increase the average a small amount, just to help rounding, we do tuncate later
        int r = (int)((bytes * 1.028 / fcount) * (parser->m_samplerate / 1000.0)) * 8;

        parser->m_bitrate = getMSB4 ((r/1024));
        parser->m_frameCount = fcount;

        parser->m_description += info.getVersionName();
        parser->m_description += ", ";
        parser->m_description += info.getAOT();
        parser->m_description += " (";
        parser->m_description += stringUtil::tos (info.m_aot);
        parser->m_description += "), ";
        parser->m_description += stringUtil::tos (info.m_samplerate);
        parser->m_description += "hz, estimated ";
        parser->m_description += tos (parser->m_bitrate);
        parser->m_description += " kbps";
        if (remain && chk == 0 && loop)
        {
            parser->m_description += ", reduced size by ";
            parser->m_description += tos (remain);
            parser->m_reduce = remain;
        }
    }
    return parser;
}


int streamData::getFrameInfo (parserInfo *&parser, unsigned &failureThresh, const unsigned char *buf, unsigned int len, unsigned chk)
{
    int ret = 0, loop = 100;
    do
    {
        loop--;
        if (parser && parser->m_inUse)
        {
            ret = parser->verifyFrame (buf, len);
            if (ret >= 0)
                break;
            delete parser;
            parser = NULL;
        }
        if (len < 3000)
            return 0;
        parser = detectMP3 (failureThresh, buf, len, chk);
        if (parser)
            continue;
        parser = detectAAC (failureThresh, buf, len, chk);
        if (parser)
            continue;
        return len < 12000 ? 0 : -1;
    } while (loop);

    // maybe ID3 check?
    if (failureThresh > 0)
        --failureThresh;
    return ret;
}


const bool streamData::syncToStream(short unsigned int& remainderSize, __uint8 *remainder,
									int amt, int& bitrate, const int type, const char *buf,
									const utf8& logString)
{
	unsigned int samplerate = m_streamInfo.m_streamSampleRate;
	bool mp3;
	if (streamData::isAllowedType(type, mp3))
	{
		int end = 0;

        //DLOG ("process block of " + tos (amt));
		for (int i = 0; (i < (amt - 8)) && !iskilled();)
		{
			unsigned int read_samplerate = 0;
			int read_bitrate = 0;
            const unsigned char* f = (const unsigned char *)&(buf[i]);
            int remain = amt - i;
            int restart = m_parser == NULL ? 1 : 0;

            int found = getFrameInfo (m_parser, m_syncFrameCount, f, remain);

            if (found > remain || found == 0)
            {
                end = i;
                // DLOG ("Need more data for " + tos(found) + " , remaining " + tos(remain));
                break;
            }
            // DLOG ("frame size return " + tos (found));

            if (m_parser)
            {
                if (restart)
                {
                    utf8 msg = logString;

                    msg += "stream detected ";
                    msg += m_parser->m_description;
                    ILOG (msg, LOGNAME, ID());
                }
                read_samplerate = m_parser->m_samplerate;
                read_bitrate = m_parser->m_bitrate;
            }

            // DLOG ("FME: frame size is " + tos (found));
            if (found > 0)
            {
                if (m_streamInfo.m_streamSampleRate == 0 && read_samplerate)
                {
                    m_streamInfo.m_streamSampleRate = (samplerate = read_samplerate);
                }
                if (m_streamInfo.m_streamBitrate == 0)
                {
                    // update everything as we'll have no bitrate if we're calling this...
                    if (read_bitrate == 0 && bitrate > 0)
                        m_streamInfo.m_streamBitrate = bitrate;
                    else
                        m_streamInfo.m_streamBitrate = (bitrate = read_bitrate);
                    m_streamInfo.m_avgBitrate = m_streamInfo.m_minBitrate =
                        m_streamInfo.m_maxBitrate = (bitrate * 1000);

                    DEBUG_LOG ("No bitrate info provided, assuming stream settings at " + tos(bitrate) + "k", LOGNAME, ID());

                    // we need to kick things back into shape at this point...
                    _setupBuffers(logString, true);

                    _YP2_add();
                }
                if (read_bitrate && m_streamInfo.m_streamBitrate && m_streamInfo.m_vbr == false)
                {
                    if (m_lastStreamBitrate)
                    {
                        if (m_lastStreamBitrate != read_bitrate)
                        {
                            m_streamInfo.m_vbr = true;
                            DEBUG_LOG ("Detected bitrate change (vBR) from " + tos (m_streamInfo.m_streamBitrate) + " to " + tos (read_bitrate), LOGNAME, ID());
                        }
                        else
                        {
                            if (read_bitrate != m_streamInfo.m_streamBitrate)
                            {
                                utf8 msg = "expected bitrate of ";
                                msg += tos (m_streamInfo.m_streamBitrate);
                                msg += "k, actually found ";
                                msg += tos(read_bitrate);
                                msg += "k, will assume that";
                                ILOG (msg, LOGNAME, ID());
                                m_streamInfo.m_streamBitrate = read_bitrate;
                            }
                        }
                    }
                    else
                        m_lastStreamBitrate = read_bitrate;
                }
#if defined(_DEBUG) || defined(DEBUG)
                writeSc1((const __uint8 *)&buf[end], found, ID());
#else
                writeSc1((const __uint8 *)&buf[end], found);
#endif
                end += found;
                i += found;
            }
            else
            {
                m_syncFrameCount += 50;
                if (m_syncFrameCount > 700)  // threshold before dropping stream
                    return true;

				// otherwise we just need to move on and keep
				// looking for what is a valid starting frame
                const void *p = memchr (f+1, 255, remain-1);
                if (p)
                {
                    i = (int)((unsigned char*)p - (unsigned char*)buf);
                    DEBUG_LOG ("FMK: found marker, searching from " + tos(i) + " in len " + tos(amt) + ", sync " + tos(m_syncFrameCount), LOGNAME, ID());
                }
                else
                {
                    end = i = amt;
                    DEBUG_LOG ("FMK: skipping to end " + tos (i), LOGNAME, ID());
                }
            }
        }

        amt = (amt - end);
        // DLOG ("EOB: remainder " + tos (remainderSize) + ", end " + tos(end) + ", amt " + tos(amt));
        if ((amt > 0)) // && (samplerate > 0))
        {
            memcpy(&(remainder[remainderSize]), &buf[end], amt);
            remainderSize += amt;
        }
    }

    return false;
}

utf8 streamData::getHTML5Player(const size_t sid) throw()
{
    return "<tr><td colspan=\"2\" align=\"center\"><audio "
        "id=\"audio_" + tos(sid) + "\" controls preload=\"none\" "
        "style=\"width:16em;padding:0;\"><source src=\"" +
		   getStreamPath(sid, true) + (getStreamContentType(sid) == "video/nsv" ?
		   ";stream.nsv" : "") + "?type=http&amp;nocache=" + tos(++gFF_fix) +
		   "\" type='audio/mpeg'></audio></td></tr>";
}

utf8 streamData::getStreamMessage(const size_t sid) throw()
{
	stackLock sml(g_streamMessageMapLock);

	map<streamID_t,utf8>::const_iterator i = g_streamMessageMap.find(sid);
	if (i != g_streamMessageMap.end())
	{
		return (*i).second;
	}
 	return "";
}

void streamData::updateStreamMessage(const size_t sid, const uniString::utf8& message) throw()
{
	stackLock sml(g_streamMessageMapLock);

	if (message.empty() || (message == "<clear>"))
	{
		map<streamID_t,utf8>::iterator i = g_streamMessageMap.find(sid);
		if (i != g_streamMessageMap.end())
		{
			(*i).second.clear();
		}
	}
	else
	{
		g_streamMessageMap[sid] = message;
	}
}

const bool streamData::isSourceCompatible(const streamSetup& setup) const throw()
{
	stackLock sml(m_stateLock);

	if (setup.m_sourceType == SHOUTCAST2)
	{
		if (m_streamInfo.m_contentType != setup.m_config.m_mimeType)
		{
			return false;
		}

		if (m_streamInfo.m_avgBitrate != setup.m_config.m_avgBitrate)
		{
			return false;
		}
		if (m_streamInfo.m_maxBitrate != setup.m_config.m_maxBitrate)
		{
			return false;
		}
		if (m_streamInfo.m_minBitrate != setup.m_config.m_maxBitrate)
		{
			return false;
		}
		return true;
	}
	else
	{
		if (m_streamInfo.m_sourceType == SHOUTCAST2)
		{
			return false;
		}
		if (m_streamInfo.m_contentType != mapGet(setup.m_headers, "content-type", utf8("audio/mpeg")))
		{
			return false;
		}
		if (!m_streamInfo.m_streamBitrate || (m_streamInfo.m_streamBitrate != getStreamBitrate(setup.m_headers)))
		{
			return false;
		}
		return true;
	}
}

// write data into a ring buffer. if packet_starts is provided, then add an entry to it (and also clean it up as necessary)
static void writeToRingBuffer(const __uint8 *data, int amt, AOL_namespace::rwLock &bufferLock,
							  streamData::ringBuffer_t &buffer,
							  deque<streamData::ringBufferAccess_t> *packet_starts = 0) throw()
{
        stackRWLock sl (bufferLock, false);
	if (packet_starts)
	{

		const streamData::ringBufferAccess_t ptr = buffer.m_writePtr,
											 bottom = (ptr - buffer.m_data.size()); // warning this could wrap

		// add new entry
		packet_starts->push_back(buffer.m_writePtr);

		// cleanup old ones
		int remove_count = 0;
		for (deque < streamData::ringBufferAccess_t>::const_iterator i = packet_starts->begin(); i != packet_starts->end(); ++i)
		{
			streamData::ringBufferAccess_t v = (*i);
			// edge case, ptr has rolled over, therefore bottom > ptr
			if (bottom > ptr)
			{
				if ((v < bottom) && (v > ptr))
				{
					++remove_count;
				}
				else
				{
					break;
				}
			}
			else
			{
				// normal case
				if ((v < bottom) || (v > ptr)) // note: (v > ptr) probably not necessary, but can't hurt
				{
					++remove_count;
				}
				else
				{
					break;
				}
			}
		}
		while ((remove_count--) > 0)
		{
			packet_starts->pop_front();
		}
	}

	size_t remain = amt;

	while (remain > 0)
	{
		// get masked ring offset
		const streamData::ringBufferAccess_t o = (buffer.m_writePtr & buffer.m_ptrMask);
		// don't write beyond end of buffer
		const size_t amt_to_write = min (remain, (buffer.m_data.size() - o));

		memcpy(&(buffer.m_data[o]), data, amt_to_write);
		// advance pointers
		data += amt_to_write;
		remain -= amt_to_write;
		buffer.m_writePtr += amt_to_write;
	}
}

// data is shoutcast1 format WITH NO METADATA. Insert into the two ring buffers
#if defined(_DEBUG) || defined(DEBUG)
void streamData::writeSc1(const __uint8 *data, const int amt, const streamID_t)
#else
void streamData::writeSc1(const __uint8 *data, const int amt)
#endif
{
    time_t now = ::time(NULL);
	if (m_insertAdvert)
	{
        adTrigger *t = advertGroups.triggers.empty() ? NULL : advertGroups.triggers.front();
        if (t && t->m_playedAt == (time_t)0)
        {
            writeToRingBuffer((const __uint8*)"SCAdvert", 8, m_sc1StreamLock, m_sc1_ring_buffer);
            writeToRingBuffer((const __uint8*)"SCAdvert", 8, m_sc21StreamLock, m_sc21_ring_buffer);
            t->m_startPosSC1 = m_sc1_ring_buffer.m_writePtr;
            t->m_startPosSC2 = m_sc21_ring_buffer.m_writePtr;
            t->m_returnPtrSC1 = m_sc1_ring_buffer.m_writePtr;
            t->m_returnPtrSC2 = m_sc21_ring_buffer.m_writePtr;
            t->m_type = advertGroups.m_type;

            if (t->m_type == ADVERT_MAP_FIXED || t->m_type == ADVERT_MAP_FLEX) // overlay
            {
                t->m_returnPtrSC1 += advertGroups.overlaySize (t);
                t->m_returnPtrSC2 += advertGroups.overlaySize (t, true);
                DLOG ("overlay of ads, returning to " + tos(t->m_returnPtrSC1), LOGNAME, m_ID);
            }
            else
            {
                DLOG ("insert of ads, returning to " + tos(t->m_returnPtrSC1), LOGNAME, m_ID);
            }
            t->m_playedAt = time(NULL);
            t->m_duration = m_duration;

            metrics::adSummary summary;
            summary.id = t->m_id;
            summary.sid = ID();
            summary.path = getStreamPath (ID());
            summary.tstamp = now;
            summary.count = stats::getUserCount (ID());
            summary.sd = this;

            metrics::metrics_advert_started (summary);
        }
        m_insertAdvert = false;
    }

    long uptime = now - streamData::getStreamUptime(ID());
    if ((uptime & 7) == 7)  // check for some things every 8 seconds
    {
        advertGroups.purge (m_sc1_ring_buffer);

        if (m_streamInfo.m_allowAllFormats == 0 && m_parser && m_parser->getUvoxType() != MP3_DATA)
        {
            WLOG ("Format is not MP3, where only MP3 is allowed", "Licence", ID());
            throwEx<runtime_error>("");
        }
        if (m_streamInfo.m_allowMaxBitrate && m_streamInfo.m_avgBitrate)
        {
            int avg = m_streamInfo.m_avgBitrate/1000;
            if (avg > (m_streamInfo.m_allowMaxBitrate + 5))
            {
                string msg = "Dropping as bitrate needs to be " + tos (m_streamInfo.m_allowMaxBitrate) + "k or less, currently " + tos (avg) +"k";
                WLOG (msg, "Licence", ID());
                throwEx<runtime_error>("");
            }
        }
    }

	// sc1
    if (amt > 0)
    {
		int len = amt;
		// put into the sc1 buffer
		writeToRingBuffer(data, len, m_sc1StreamLock, m_sc1_ring_buffer, &m_sc1_packet_starts);
#if 0
		// TODO would be nice to have this as a more formal thing for debugging and / or backup
		if (!g_streamSaving[id])
		{
			g_streamSaving[id] = uniFile::fopen("D:\\Dev\\git_radionomy\\SHOUTcast\\sc_serv3\\test_" + tos(id) + (m_streamInfo.m_uvoxDataType == MP3_DATA ? ".mp3" : ".aac"), "wb");
		}
		if (g_streamSaving[id])
		{
			fwrite(data, 1, amt, g_streamSaving[id]);
		}
		stackLock sml(m_sc1LimitTriggerLock);
		for_each(m_sc1LimitTriggers.begin(), m_sc1LimitTriggers.end(), mem_fun(&pipeDrivenSignal<AOL_namespace::mutex>::set));
#endif
	}
	// sc1

	// sc21
	m_sc21MetadataLock.lock();
	uvoxMetadata_t md = m_sc21MetadataToPutInline;
	m_sc21MetadataToPutInline.clear();
	m_sc21MetadataLock.unlock();

	// const bool trigger = ((amt > 0) || (!md.empty()));

	if (!md.empty())
	{
		writeToRingBuffer((const __uint8*)&md[0], (int)md.size(), m_sc21StreamLock, m_sc21_ring_buffer);
	}

	int len = amt;
	while (len > 0)
	{
		__uint8 uvoxBuffer[MAX_MESSAGE_SIZE] = {0};
		int left_over = formMessage(data, len, m_streamInfo.m_uvoxDataType, uvoxBuffer);
		writeToRingBuffer((const __uint8*)uvoxBuffer, len - left_over + UV2X_OVERHEAD,
						  m_sc21StreamLock, m_sc21_ring_buffer, &m_sc21_packet_starts);
		data += (len - left_over);
		len = left_over;
	}
#if 0
	if (trigger)
	{
		stackLock sml(m_sc21LimitTriggerLock);
		for_each(m_sc21LimitTriggers.begin(), m_sc21LimitTriggers.end(),
				 mem_fun(&pipeDrivenSignal<AOL_namespace::mutex>::set));
	}
	// sc21
	YP2_update();
#endif
	checkForAdverts();
    pushMetricsYP (false);
}

void streamData::writeSc21(const vector<__uint8> &data) throw()
{
	if (data.empty())
	{
		return;
	}

	m_sc21MetadataLock.lock();
	uvoxMetadata_t md = m_sc21MetadataToPutInline;
	m_sc21MetadataToPutInline.clear();
	m_sc21MetadataLock.unlock();

	if (!md.empty())
	{
		writeToRingBuffer((const __uint8*)&md[0], (int)md.size(),
						  m_sc21StreamLock, m_sc21_ring_buffer);
	}

	writeToRingBuffer(&(data[0]), (int)data.size(), m_sc21StreamLock,
					  m_sc21_ring_buffer, &m_sc21_packet_starts);
#if 0
	stackLock sml(m_sc21LimitTriggerLock);
	for_each(m_sc21LimitTriggers.begin(), m_sc21LimitTriggers.end(),
			 mem_fun(&pipeDrivenSignal<AOL_namespace::mutex>::set));
#endif
}

static streamData::ringBufferAccess_t _getClientStartPosition (const streamData::ringBuffer_t &/*buffer*/,
	deque<streamData::ringBufferAccess_t> &packet_starts,
	AOL_namespace::rwLock &streamLock,
	streamData::ringBufferAccess_t ptr = 0) throw()
{
	stackRWLock sl (streamLock);

	const deque<streamData::ringBufferAccess_t>::size_type packet_count = packet_starts.size();
	if (packet_count < 6)
	{
		return 0;
	}

	streamData::ringBufferAccess_t avail_range = (packet_starts.back() - packet_starts.front());

	if (ptr == 0)
	{
        streamData::ringBufferAccess_t avg = avail_range / packet_count;
        size_t pkts = avg ? (256000/avg) : 0;  // target about 250k burst
        if (pkts)
        {
            if (pkts + 6 > packet_count)
                pkts = packet_count - 6;
            return packet_starts [packet_count - pkts];
        }
        return packet_starts.front();
	}
	deque<streamData::ringBufferAccess_t>::reverse_iterator r = packet_starts.rbegin(), t = r;
	if (*r < ptr && (ptr - *r) > 60000)
		ILOG ("[ADVERT] Expected transition point still to come, returning early", ADLOGNAME);
	for (; r != packet_starts.rend(); ++r)
	{
		if (*r < ptr)
			break;
		t = r;
	}
	// DLOG("start pos, returning " + tos ((long)*t));
	return *t;
}

const streamData::ringBufferAccess_t streamData::getClientStartPosition(const bool sc2) throw()
{ 
	return _getClientStartPosition((!sc2 ? m_sc1_ring_buffer : m_sc21_ring_buffer),
								   (!sc2 ? m_sc1_packet_starts : m_sc21_packet_starts),
								   (!sc2 ? m_sc1StreamLock : m_sc21StreamLock)); 
}


streamData::ringBufferAccess_t streamData::getClientStartPosition (ringBufferAccess_t ptr, bool sc2) throw()
{
       return _getClientStartPosition((!sc2 ? m_sc1_ring_buffer : m_sc21_ring_buffer),
                                                                  (!sc2 ? m_sc1_packet_starts : m_sc21_packet_starts),
                                                                  (!sc2 ? m_sc1StreamLock : m_sc21StreamLock), ptr);
}


const int streamData::getStreamData(streamData::ringBufferAccess_t& amt, const streamData::ringBufferAccess_t& readPtr,
									vector<__uint8>& data, const size_t remSize, const bool sc2) throw() /* for readers only */
{
	stackRWLock sl ((!sc2 ? m_sc1StreamLock : m_sc21StreamLock));

	const streamData::ringBuffer_t& buffer = (!sc2 ? m_sc1_ring_buffer : m_sc21_ring_buffer);

	amt = (buffer.m_writePtr - readPtr);
	if ((amt > 0) && (amt > buffer.m_data.size()))
	{
		// the pointers are too far apart - underrun
		// make the listener handler process a reset
		return -1;
	}

	const streamData::ringBufferAccess_t offset = (readPtr & buffer.m_ptrMask);
	// clamp again so we don't read pass the end of the buffer
	//
	// if we've got more in remainder than what we're wanting
	// to send then we'll prioritise the remainder data first
	// before trying to acquire more new data to try to send.
	amt = min(amt, min((buffer.m_data.size() - offset),
			  (streamData::ringBufferAccess_t)max(0, (SEND_SIZE - (int)remSize))));

	int len = (int)amt;
	if (len > 0)
	{
		const vector<__uint8>::const_iterator pos = buffer.m_data.begin();
		data.insert(data.end(), pos + offset, pos + (offset + len));
	}

	return len;
}

#if 0
void streamData::abandonLimitTrigger(pipeDrivenSignal<AOL_namespace::mutex> *t, const bool sc2) throw()
{
	if (!sc2)
	{
		stackLock sml(m_sc1LimitTriggerLock);
		m_sc1LimitTriggers.erase(t);
	}
	else
	{
		stackLock sml(m_sc21LimitTriggerLock);
		m_sc21LimitTriggers.erase(t);
	}
}

void streamData::_scheduleLimitTrigger(pipeDrivenSignal<AOL_namespace::mutex> *t, const ringBufferAccess_t readPtr,
									   AOL_namespace::mutex &streamLock, const ringBuffer_t &ringBuffer,
									   AOL_namespace::mutex &triggerSetLock,
									   set<pipeDrivenSignal<AOL_namespace::mutex>*> &triggerSet) throw()
{
	bool add_to_set = true;

	streamLock.lock();
	if (ringBuffer.m_writePtr > readPtr)
	{
		t->set();
		add_to_set = false;
	}
	streamLock.unlock();

	stackLock sml(triggerSetLock);
	if (add_to_set)
	{
		triggerSet.insert(t);
	}
	else
	{
		triggerSet.erase(t);
	}
}
		
void streamData::scheduleLimitTrigger(pipeDrivenSignal<AOL_namespace::mutex> *t,
									  const ringBufferAccess_t readPtr, const bool sc2) throw()
{
	_scheduleLimitTrigger(t, readPtr, (!sc2 ? m_sc1StreamLock : m_sc21StreamLock),
						  (!sc2 ? m_sc1_ring_buffer : m_sc21_ring_buffer),
						  (!sc2 ? m_sc1LimitTriggerLock : m_sc21LimitTriggerLock),
						  (!sc2 ? m_sc1LimitTriggers : m_sc21LimitTriggers));
}
#endif

// find the metadata entry that is <= to ptr. 	
template<typename T>
typename T::value_type::second_type _getMetadata(const T &metadataTable, AOL_namespace::mutex &metadataLock,
												 const streamData::ringBufferAccess_t ptr) throw()
{
	stackLock sml(metadataLock);

	typename T::value_type::second_type result;

	if (metadataTable.empty())
	{
		return result;
	}
	streamData::ringBufferAccess_t old_entry = metadataTable.front().first; // "oldest" entry
	streamData::ringBufferAccess_t new_entry = metadataTable.back().first; // "newest" entry

	if (old_entry <= new_entry)
	{
		// normal case. older entries have lower pointers than later ones
		for (typename T::const_iterator i = metadataTable.begin(); i != metadataTable.end(); ++i)
		{
			if ((*i).first <= ptr)
			{
				result = (*i).second;
			}
			else
			{
				break;
			}
		}			
	}
	else
	{
		// wrap case
		typename T::const_iterator i = metadataTable.begin();

		if (ptr >= old_entry) // "upper half"
		{
			for (; i != metadataTable.end(); ++i)
			{
				if (((*i).first <= ptr) && ((*i).first >= old_entry))
				{
					result = (*i).second;
				}
				else
				{
					break;
				}
			}
		}
		else if (ptr > new_entry) // in "gap"
		{
			result = metadataTable.back().second;
		}
		else // lower half
		{
			for (; i != metadataTable.end(); ++i) // skip entries in "upper half"
			{
				if ((*i).first < old_entry)
				{
					break;
				}
			}
			for (; i != metadataTable.end(); ++i)
			{
				if ((*i).first <= ptr)
				{
					result = (*i).second;
				}
				else
				{
					break;
				}
			}
		}
	}
	return result;
}

streamData::sc1MetadataAndExtensionInfo streamData::getSc1Metadata(ringBufferAccess_t ptr) throw()
{
	return _getMetadata(m_sc1MetadataTable, m_sc1MetadataLock, ptr);
}

streamData::uvoxMetadata_t streamData::getSc21Metadata(ringBufferAccess_t ptr) throw()
{
	return _getMetadata(m_sc21MetadataTable, m_sc21MetadataLock, ptr);
}

streamData::uvoxMetadata_t streamData::getSc21StreamAlbumArt(ringBufferAccess_t ptr) throw()
{
	return _getMetadata(m_sc21StreamAlbumArtTable, m_sc21StreamAlbumArtLock, ptr);
}

streamData::uvoxMetadata_t streamData::getSc21PlayingAlbumArt(ringBufferAccess_t ptr) throw()
{
	return _getMetadata(m_sc21PlayingAlbumArtTable, m_sc21PlayingAlbumArtLock, ptr);
}

// remove stale metadata entries by looking for those that are beyond the bottom of the ring buffer.
// HOWEVER, always keep the bottom one so that clients who enter will have some metadata to work with
template<typename METATABLE>
static void _cleanupMetadataEntries(METATABLE &metadataTable,
									const streamData::ringBufferAccess_t ptr,
									const streamData::ringBufferAccess_t siz)
{
	streamData::ringBufferAccess_t bottom = ptr - siz; // warning this could wrap
	int remove_count = -1; // always leave bottom entry
	for (typename METATABLE::const_iterator i = metadataTable.begin(); i != metadataTable.end(); ++i)
	{
		streamData::ringBufferAccess_t v = (*i).first;
		// edge case, ptr has rolled over, therefore bottom > ptr
		if (bottom > ptr)
		{
			if ((v < bottom) && (v > ptr))
			{
				++remove_count;
			}
			else
			{
				break;
			}
		}
		else
		{
			// normal case
			if ((v < bottom) || (v > ptr)) // note: (v > ptr) probably not necessary, but can't hurt
			{
				++remove_count;
			}
			else
			{
				break;
			}
		}
	}

	while ((remove_count--) > 0)
	{
		metadataTable.pop_front();
	}
}

template<typename METADATA,typename MT>
static void _addMetadata(AOL_namespace::rwLock &streamLock, const streamData::ringBuffer_t &ringBuffer,
						 AOL_namespace::mutex &metadataLock, METADATA &metadataTable, const MT &md) throw()
{
	// get current position and size of ring buffer
	streamLock.lock();
	const streamData::ringBufferAccess_t ptr = ringBuffer.m_writePtr,
										 siz = ringBuffer.m_data.size();
	streamLock.unlock();

	// put entry in table
	metadataLock.lock();
	metadataTable.push_back(make_pair(ptr, md));

	// remove any stale entries
	_cleanupMetadataEntries(metadataTable, ptr, siz);
	metadataLock.unlock();
}

bool streamData::validateTitle(uniString::utf8 &m_updinfoSong) throw()
{
	bool allowed = true;

	m_updinfoSong = stripWhitespace(m_updinfoSong);
	if (!m_updinfoSong.empty())
	{
		// work on lowercase comparison as well as doing a check to see if
		// after removing white space + punctuation we have a valid title.
		uniString::utf8 m_checkUpdinfoSong = toLower(m_updinfoSong);

		// exclude weird title updates from being accepted
		// as no point in giving junk to the user later on
		if (m_checkUpdinfoSong.find((utf8)"!doctype") != utf8::npos ||
			m_checkUpdinfoSong.find((utf8)"<script") != utf8::npos ||
			m_checkUpdinfoSong.find((utf8)"<html") != utf8::npos ||
			m_checkUpdinfoSong.find((utf8)"<body") != utf8::npos ||
			m_checkUpdinfoSong.find((utf8)"<div") != utf8::npos ||
			m_checkUpdinfoSong.find((utf8)"%] ") != utf8::npos ||
			m_checkUpdinfoSong.find((utf8)"invalid resource") != utf8::npos ||
			(m_checkUpdinfoSong.find((utf8)"nextsong") != utf8::npos &&
			 m_checkUpdinfoSong.find((utf8)"sctrans2next") != utf8::npos) ||

			m_checkUpdinfoSong.find((utf8)"radio online") != utf8::npos ||

			m_checkUpdinfoSong.find((utf8)"get /") == 0 ||
			m_checkUpdinfoSong.find((utf8)"put /") == 0 ||
			m_checkUpdinfoSong.find((utf8)"post /") == 0 ||
			m_checkUpdinfoSong.find((utf8)"head /") == 0 ||
			m_checkUpdinfoSong.find((utf8)"source /") == 0 ||
			m_checkUpdinfoSong.find((utf8)"track ") == 0 ||
			m_checkUpdinfoSong.find((utf8)"track0") == 0 ||
			m_checkUpdinfoSong.find((utf8)"track1") == 0 ||
			m_checkUpdinfoSong.find((utf8)"stream ") == 0 ||
			m_checkUpdinfoSong.find((utf8)"no artist ") == 0 ||
			m_checkUpdinfoSong.find((utf8)"new artist ") == 0 ||
			m_checkUpdinfoSong.find((utf8)"line-in ") == 0 ||
			m_checkUpdinfoSong.find((utf8)"inter_") == 0 ||
			m_checkUpdinfoSong.find((utf8)"jj mckay - ") == 0 ||
			m_checkUpdinfoSong.find((utf8)"artist - ") == 0 ||

			m_checkUpdinfoSong.find((utf8)"$") == 0 ||
			m_checkUpdinfoSong.find((utf8)"£") == 0 ||
			//m_checkUpdinfoSong.find((utf8)"@") == 0 ||
			m_checkUpdinfoSong.find((utf8)"%") == 0 ||
			//m_checkUpdinfoSong.find((utf8)"#") == 0 ||
			m_checkUpdinfoSong.find((utf8)"*") == 0 ||
			m_checkUpdinfoSong.find((utf8)"+") == 0 ||
			//m_checkUpdinfoSong.find((utf8)"&") == 0 ||
			m_checkUpdinfoSong.find((utf8)"[") == 0 ||
			m_checkUpdinfoSong.find((utf8)"{") == 0 ||
			m_checkUpdinfoSong.find((utf8)"}") == 0 ||
			m_checkUpdinfoSong.find((utf8)"(") == 0 ||
			m_checkUpdinfoSong.find((utf8)")") == 0 ||
			m_checkUpdinfoSong.find((utf8)"?") == 0 ||
			m_checkUpdinfoSong.find((utf8)"_") == 0 ||
			m_checkUpdinfoSong.find((utf8)"~") == 0 ||
			m_checkUpdinfoSong.find((utf8)"^") == 0 ||
			m_checkUpdinfoSong.find((utf8)"`") == 0 ||
			m_checkUpdinfoSong.find((utf8)"»") == 0 ||
			m_checkUpdinfoSong.find((utf8)"'") == 0 ||
			m_checkUpdinfoSong.find((utf8)".") == 0 ||
			m_checkUpdinfoSong.find((utf8)",") == 0 ||
			m_checkUpdinfoSong.find((utf8)"/") == 0 ||
			m_checkUpdinfoSong.find((utf8)"!") == 0 ||
			m_checkUpdinfoSong.find((utf8)":") == 0 ||
			m_checkUpdinfoSong.find((utf8)";") == 0 ||
			m_checkUpdinfoSong.find((utf8)"\"") == 0 ||
			m_checkUpdinfoSong.find((utf8)"- ") == 0 ||
			m_checkUpdinfoSong.find((utf8)". ") == 0 ||

			m_checkUpdinfoSong == utf8("-") ||
			m_checkUpdinfoSong == utf8("http://") ||
			m_checkUpdinfoSong == utf8("https://") ||
			m_checkUpdinfoSong == utf8("auto dj") ||
			m_checkUpdinfoSong == utf8("ao vivo") ||
			m_checkUpdinfoSong == utf8("unknown") ||
			m_checkUpdinfoSong == utf8("test") ||
			m_checkUpdinfoSong == utf8("dsp") ||
			m_checkUpdinfoSong == utf8("demo") ||
			m_checkUpdinfoSong == utf8("line input") ||
			m_checkUpdinfoSong == utf8("dj mike llama - llama whippin` intro") ||
			m_checkUpdinfoSong == utf8("preview"))
		{
			allowed = false;
		}
	}
	else
	{
		// this allows empty titles (which are sometimes needed)
		allowed = m_updinfoSong.empty();
	}

	return allowed;
}

int streamData::addUvoxMetadataAtCurrentPosition(__uint16 voxMsgType, const vector<__uint8> &data) throw()
{
	// convert uvox metadata to sc21, sc2 and sc1
	utf8 sc21_metadata_s;
	vector<__uint8> sc21_albumart_data;
	bool albumart = false;

	sc1MetadataAndExtensionInfo sc1_metadata_and_extension_info;

	if (voxMsgType == MSG_METADATA_XML_NEW) // new shoutcast 2
	{
		sc21_metadata_s.insert(sc21_metadata_s.end(), data.begin(), data.end());
		utf8 meta(sc21_metadata_s.begin(), sc21_metadata_s.end());

		// attempt to get the encoder from the metadata i.e. used to identify the source
		try
		{
			m_streamInfo.m_comingSoon.clear();
			m_streamInfo.m_nextSongs.clear();
			m_streamInfo.m_sourceIdent = metadata::get_XX_from_3902("TENC", meta, m_streamInfo.m_sourceIdent);
			m_streamInfo.m_nextSongs = metadata::get_nextsongs_from_3902(meta, m_streamInfo.m_nextSongs);

			// these allow for updating of information from a source when a DJ
			// is connected e.g. to sc_trans but is not setup to be listed or
			// is just sending through a DJ name update for admin tracking, etc
			m_streamInfo.m_streamUser = metadata::get_XX_from_3902("DJ", meta, m_streamInfo.m_streamUser);

			int addFailIgnore = 0, errorCode = 0;
			if (!m_streamInfo.m_streamPublic || YP2_addSuccessful(addFailIgnore, errorCode))
			{
				if (!m_streamInfo.m_streamPublic || addFailIgnore)
				{
					// only update the stream name from metadata if public or pending a YP update
					m_streamInfo.m_streamName = metadata::get_XX_from_3902("TRSN", meta, m_streamInfo.m_streamName);
				}
			}
		}
		catch (const exception &ex)
		{
			ELOG(ex.what());
			// abort nicely if there was an error
			return 0;
		}

		// attempt to get the next song from the extended information from the metadata
		utf8 m_checkComingSoon = metadata::get_XX_from_3902("extension/soon", meta, m_streamInfo.m_comingSoon);
		if (!m_checkComingSoon.empty())
		{
			if (validateTitle(m_checkComingSoon))
			{
				m_streamInfo.m_comingSoon = m_checkComingSoon;
			}
			else
			{
				if (!(m_checkComingSoon.find((utf8)"nextsong") != utf8::npos && m_checkComingSoon.find((utf8)"sctrans2next") != utf8::npos))
				{
					WLOG("[ADMINCGI sid=" + tos(ID()) + "] Coming soon title rejected - value not allowed: " + m_checkComingSoon, LOGNAME, ID());
				}
				m_streamInfo.m_comingSoon.clear();
			}
		}
		else
		{
			if (!m_streamInfo.m_comingSoon.empty())
			{
				m_streamInfo.m_comingSoon.clear();
				WLOG("[ADMINCGI sid=" + tos(ID()) + "] Coming soon title cleared", LOGNAME, ID());
			}
		}

		// clip out any extension data
		utf8::size_type pos1 = sc21_metadata_s.find(utf8("<extension>")), // 11 chars
						pos2 = sc21_metadata_s.find(utf8("</extension>")); // 12 chars
		if ((pos1 != utf8::npos) && (pos2 != utf8::npos))
		{
			// got it, clip it out
			sc1_metadata_and_extension_info.m_songMetadataForYP2 = 
																   #ifdef XML_DEBUG
																   "\t" +
																   #endif
																   stripWhitespace(sc21_metadata_s.substr(pos1 + 11, pos2 + 12 - pos1 - 11 - 12));
		}

		try
		{
			utf8 m_checkSongTitle = metadata::convert_3902_to_shoutcast1(sc21_metadata_s, ID());
			if (sc1_metadata_and_extension_info.m_songTitle != m_checkSongTitle)
			{
				sc1_metadata_and_extension_info.m_songTitle = m_checkSongTitle;
			}
		}
		catch (const exception &ex)
		{
			ELOG(string("3902 => Shoutcast 1 metadata conversion error. ") + ex.what() + " 3902=" + eol() + sc21_metadata_s, LOGNAME, ID());
		}

		// if we get a title update, if we've not added then do so as we
		// changed adding to the YP to wait a while in 2.1+ so that titles
		// can be obtained to ensure the listing is showing all information
		//
		// 2.4.8 disable this as it's causing quick add / removes to the YP
		// which is ok if the stream is ok, but if there's issues, it's not
		// and that's then also causing lots of METRICS_RESET_URL messages.
		/*if (m_yp2SessionKey == 0)
		{
			_YP2_add();
		}*/
	}
	// handling albumart as required for station and stream artwork
	else
	{
		__uint16 aaMsgType = (voxMsgType & 0xF000);
		if (aaMsgType & MSG_METADATA_ALBUMART)
		{
			sc21_albumart_data.insert(sc21_albumart_data.end(), data.begin(), data.end());
			albumart = true;
		}
	}

	int ret = 0;
	// filter things so we're only doing what is needed at the time
	if (albumart == false)
	{
		uvoxMetadata_t sc21_metadata;

		// ultravox 2.1 style xml
		try
		{
			createMetadataPackets(&sc21_metadata_s[0], (int)sc21_metadata_s.size(), MSG_METADATA_XML_NEW, sc21_metadata);
		}
		catch (const exception &ex)
		{
			ELOG(ex.what(), LOGNAME, ID());
		}

		if (!sc1_metadata_and_extension_info.m_songTitle.empty())
		{
			_addMetadata(m_sc1StreamLock, m_sc1_ring_buffer, m_sc1MetadataLock, m_sc1MetadataTable, sc1_metadata_and_extension_info);
		}
		if (!sc21_metadata.empty())
		{
			_addMetadata(m_sc21StreamLock, m_sc21_ring_buffer, m_sc21MetadataLock, m_sc21MetadataTable, sc21_metadata);

			m_sc21MetadataLock.lock();
			m_sc21MetadataToPutInline = sc21_metadata;
			m_sc21MetadataLock.unlock();
		}

		// maintain history
		g_streamSongHistoryMapLock.lock();

		// attempting to use the pre-filled title information based on forum request so
		// that 'artist - album - title' from sc_trans appears in the history (not client)
		utf8 metadata(sc21_metadata_s.begin(), sc21_metadata_s.end());
		const vector<uniString::utf8> m_nextSong = metadata::get_nextsongs_from_3902(metadata, m_streamInfo.m_nextSongs, true);
		const vector<uniString::utf8>::const_iterator nextSong = m_nextSong.begin();

		utf8 m_checkSongHistory = (((nextSong != m_nextSong.end()) && !(*nextSong).empty()) ?
								  (*nextSong) : metadata::get_song_title_from_3902(metadata));
		if (validateTitle(m_checkSongHistory))
		{
			map<streamID_t, streamHistory_t>::iterator i = g_streamSongHistoryMap.find(ID());
			if (i != g_streamSongHistoryMap.end())
			{
				if ((*i).second.empty() || ((*i).second.front().m_title != m_checkSongHistory))
				{
					(*i).second.push_front(songHistoryInfo(m_checkSongHistory, metadata));
				}
			}
			else
			{
				g_streamSongHistoryMap[ID()].push_front(songHistoryInfo(m_checkSongHistory, metadata));
			}
		}

		// do conversions as needed to cope with funky v1 titles from relays for example
		map<streamID_t, streamHistory_t>::iterator i = g_streamSongHistoryMap.find(ID());
		if (i != g_streamSongHistoryMap.end())
		{
			if (!(*i).second.empty())
			{
				utf8 m_checkCurrentSong = metadata::toFixedString((*i).second.front().m_title);
				if (validateTitle(m_checkCurrentSong))
				{
					if (m_streamInfo.m_currentSong != m_checkCurrentSong)
					{
						m_streamInfo.m_currentSong = m_checkCurrentSong;
					}
				}
			}

			while ((*i).second.size() > gOptions.getSongHistorySize(ID()))
			{
				(*i).second.pop_back();
			}
		}
		g_streamSongHistoryMapLock.unlock();

		resetAdvertTriggers(m_streamInfo.m_currentSong);
	}
	else
	{
		uvoxMetadata_t sc21_albumart;

		// ultravox 2.1 style albumart
		try
		{
			if (!sc21_albumart_data.empty())
			{
				createMetadataPackets(&sc21_albumart_data[0], (int)sc21_albumart_data.size(), voxMsgType, sc21_albumart);
			}
			else
			{
				// indicates we need to clear the cached data
			}
		}
		catch (const exception &ex)
		{
			ELOG(ex.what(), LOGNAME, ID());
		}

		// store a copy of the raw image file and mime type
		// so it's easier to display on the admin pages, etc
		__uint16 ArtType = voxMsgType & 0x0F00;
		if (ArtType & MSG_METADATA_PLAYING_ART)
		{
			m_sc21AlbumArtData[1] = sc21_albumart_data;
			m_sc21AlbumArtMime[1] = (voxMsgType & 0x00FF);
		}
		else
		{
			m_sc21AlbumArtData[0] = sc21_albumart_data;
			m_sc21AlbumArtMime[0] = (voxMsgType & 0x00FF);
		}

		if (!sc21_albumart.empty())
		{
			// uvox metadata must also be injected into the stream itself
			if (ArtType & MSG_METADATA_PLAYING_ART)
			{
				m_sc21PlayingAlbumArtLock.lock();
				m_sc21PlayingAlbumArtToPutInline = sc21_albumart;
				m_sc21PlayingAlbumArtLock.unlock();

				_addMetadata(m_sc21StreamLock,
							 m_sc21_ring_buffer,
							 m_sc21PlayingAlbumArtLock,
							 m_sc21PlayingAlbumArtTable,
							 sc21_albumart);

				ret = addSc1MetadataAtCurrentPosition("", m_streamInfo.m_currentSong,
													  "DNAS/playingart?sid=" + tos(ID()),
													  m_streamInfo.m_comingSoon);

				// due to how things work, it's easier to do it all this way and
				// then directly insert as if it's pass-through but keep a copy
				writeToRingBuffer((const __uint8*)&sc21_albumart[0],
								  (int)sc21_albumart.size(),
								  m_sc21StreamLock, m_sc21_ring_buffer);
			}
			else
			{
				m_sc21StreamAlbumArtLock.lock();
				m_sc21StreamAlbumArtToPutInline = sc21_albumart;
				m_sc21StreamAlbumArtLock.unlock();

				_addMetadata(m_sc21StreamLock,
							 m_sc21_ring_buffer,
							 m_sc21StreamAlbumArtLock,
							 m_sc21StreamAlbumArtTable,
							 sc21_albumart);

				// if we've got playing artwork then we should skip updating this as it's preferred for the client
				// to provide playing artwork first and then the stream branding so it shows playing on joining
				if (m_sc21AlbumArtData[1].empty())
				{
					ret = addSc1MetadataAtCurrentPosition("", m_streamInfo.m_currentSong,
														  "DNAS/streamart?sid=" + tos(ID()),
														  m_streamInfo.m_comingSoon);
				}
			}
		}
		else
		{
			// TODO make sure this is specified if a custom streamart is provided and nothing from the stream
			ret = addSc1MetadataAtCurrentPosition("", m_streamInfo.m_currentSong,
												  (!m_sc21AlbumArtData[0].empty() ? "DNAS/streamart?sid=" + tos(ID()) : ""),
												  m_streamInfo.m_comingSoon);
		}
	}
	return ret;
}

void streamData::updateSongHistorySize() throw()
{
	// maintain history
	stackLock sml(g_streamSongHistoryMapLock);

	map<streamID_t, streamHistory_t>::iterator i = g_streamSongHistoryMap.find(ID());
	if (i != g_streamSongHistoryMap.end())
	{
		while ((*i).second.size() > gOptions.getSongHistorySize(ID()))
		{
			(*i).second.pop_back();
		}
	}
}

void streamData::updateStreamUser(const uniString::utf8& streamUser)
{
	m_streamInfo.m_streamUser = streamUser;
}

void streamData::resetStreamAuthhash()
{
	m_streamInfo.m_authHash.clear();
	_YP2_add();
}

void streamData::resetAdvertTriggers(const uniString::utf8& m_updinfoSong)
{
    if (m_updinfoSong.empty())
        return;

    unsigned d = 120;     // default
    const char *p = (const char *)&m_updinfoSong[0];
    char s[2] = "";
    int matched = sscanf (p, "Advert%1[!:-]", s);

    if (matched == 1)
    {
        if (s[0] == '-')
        {
            char pattern[6];
            matched = ::sscanf (p+7, "%4[0-9]%1[!:]", pattern, s);
            if (matched == 2)
            {
                d = atoi (pattern);
                if (d < 1 || d > 999)
                    d = 120;
            }
            else
                p = NULL;
        }
        if (p) p = strchr (p+6, s[0]);
        if (p)
        {
            p++;
            //::memmove (tag, p, strlen (p)+1); ??
            AD_DEBUG_LOG("[ADVERT sid=" + tos(ID()) + "] " + "Advert trigger detected.", LOGNAME, ID());
            m_insertAdvert = true;
        }
    }
    m_duration = d;
    pushMetricsYP();
}

int streamData::addSc1MetadataAtCurrentPosition(const utf8 &logString, const utf8 &sc1_metadata_raw,
												const utf8 &sc1_url_raw, const utf8 &sc1_next_raw) throw()
{
	const utf8 m_oldCurrentSong = m_streamInfo.m_currentSong;
	const utf8 m_oldCurrentURL = m_streamInfo.m_currentURL;
	const utf8 m_oldComingSoon = m_streamInfo.m_comingSoon;

	// maintain history
	m_stateLock.lock();

	m_streamInfo.m_currentSong = sc1_metadata_raw;
	m_streamInfo.m_comingSoon = sc1_next_raw;

	if (!sc1_url_raw.empty())
	{
		m_streamInfo.m_currentURL = sc1_url_raw;
	}
	else
	{
		// this allows us to force artwork through for all of the clients when
		// it's not provided by the connected source on the first title update
		if (m_sc21AlbumArtData[0].empty() && (!gOptions.m_artworkBody[m_ID].empty() || !gOptions.m_artworkBody[0].empty()))
		{
			vector<__uint8> sc21_albumart_data;
			if (!gOptions.m_artworkBody[m_ID].empty())
			{
				sc21_albumart_data.insert(sc21_albumart_data.end(), gOptions.m_artworkBody[m_ID].begin(), gOptions.m_artworkBody[m_ID].end());
			}
			else
			{
				sc21_albumart_data.insert(sc21_albumart_data.end(), gOptions.m_artworkBody[0].begin(), gOptions.m_artworkBody[0].end());
			}

			int voxMsgType = 0x4000;
			uvoxMetadata_t sc21_albumart;
			if (!sc21_albumart_data.empty())
			{
				// try to find the correct mime-type to give the clients a hint
				utf8::size_type pos = gOptions.artworkFile().rfind((utf8)".");
				if (pos != utf8::npos)
				{
					utf8 ext = gOptions.artworkFile().substr(pos + 1, gOptions.artworkFile().size());
					if ((ext == "jpeg") || (ext == "jpg"))
					{
						voxMsgType = 0x4000;
					}
					else if (ext == "png")
					{
						voxMsgType = 0x4001;
					}
					else if (ext == "bmp")
					{
						voxMsgType = 0x4002;
					}
					else if (ext == "gif")
					{
						voxMsgType = 0x4003;
					}
				}

				createMetadataPackets(&sc21_albumart_data[0], (int)sc21_albumart_data.size(), voxMsgType, sc21_albumart);
			}

			m_sc21AlbumArtData[0] = sc21_albumart_data;
			m_sc21AlbumArtMime[0] = (voxMsgType & 0x00FF);

			m_sc21StreamAlbumArtLock.lock();
			m_sc21StreamAlbumArtToPutInline = sc21_albumart;
			m_sc21StreamAlbumArtLock.unlock();

			_addMetadata(m_sc21StreamLock,
						 m_sc21_ring_buffer,
						 m_sc21StreamAlbumArtLock,
						 m_sc21StreamAlbumArtTable,
						 sc21_albumart);

			m_streamInfo.m_currentURL = "DNAS/streamart?sid=" + tos(ID());
		}
		else
		{
			m_streamInfo.m_currentURL.clear();
		}
	}

	// changed in 2.4 so it will allow repeated 'Advert:xx'
	// update through since it's needed for advert triggers
	// changed in 2.4.8+ so we can provide a 'test' case
	bool sameTitle = (m_oldCurrentSong == m_streamInfo.m_currentSong),
		 sameUrl = (m_oldCurrentURL == m_streamInfo.m_currentURL),
		 sameNext = (m_oldComingSoon == m_streamInfo.m_comingSoon);
	if ((sameTitle && sameUrl && sameNext &&
		((m_streamInfo.m_currentSong.find ((utf8)"Advert:") != 0) &&
		 (m_streamInfo.m_currentSong.find ((utf8)"Test Advert:") != 0))))
	{
		// if the same then we might as well skip as it won't add anything
		// other than filling the song history list with deemed duplicates
		m_stateLock.unlock();
		return 0;
	}

	DEBUG_LOG(logString + "title: " + tos(sameTitle) + " - url: " + tos(sameUrl) + " - next: " + tos(sameNext), LOGNAME, ID());

	// convert sc1 metadata to sc21 metadata
	utf8 sc21_metadata_s = metadata::toXML_fromFilename(m_streamInfo.m_currentSong, m_streamInfo.m_currentURL, "%N");

	g_streamSongHistoryMapLock.lock();
	map<streamID_t, streamHistory_t>::iterator i = g_streamSongHistoryMap.find(ID());
	if (i != g_streamSongHistoryMap.end())
	{
		(*i).second.push_front(songHistoryInfo(m_streamInfo.m_currentSong, sc21_metadata_s));

		while ((*i).second.size() > gOptions.getSongHistorySize(ID()))
		{
			(*i).second.pop_back();
		}
	}
	else
	{
		g_streamSongHistoryMap[ID()].push_front(songHistoryInfo(m_streamInfo.m_currentSong, sc21_metadata_s));;
	}
	g_streamSongHistoryMapLock.unlock();

	resetAdvertTriggers(m_streamInfo.m_currentSong);

	m_stateLock.unlock();

	sc1MetadataAndExtensionInfo sc1_metadata_and_extension_info;
	sc1_metadata_and_extension_info.m_songTitle = "StreamTitle='" + m_streamInfo.m_currentSong + "';";

	if (!m_streamInfo.m_comingSoon.empty())
	{
		// added in 2.5 (dunno if any client will use it or not)
		sc1_metadata_and_extension_info.m_songTitle += "StreamNext='" + m_streamInfo.m_comingSoon + "';";
	}

	if (!m_streamInfo.m_currentURL.empty())
	{
		sc1_metadata_and_extension_info.m_songTitle += "StreamUrl='" + m_streamInfo.m_currentURL + "';";
	}
	else
	{
		if (!m_sc21AlbumArtData[1].empty())
		{
			m_streamInfo.m_currentURL = "DNAS/playingart?sid=" + tos(ID());
			sc1_metadata_and_extension_info.m_songTitle += "StreamUrl='" + m_streamInfo.m_currentURL + "';";
		}
		else
		{
			// **** TODO ****

			// this allows us to force artwork through for all of the clients when
			// it's not provided by the connected source on the first title update
			if (m_sc21AlbumArtData[0].empty() && !gOptions.m_artworkBody[0].empty())
			{
				m_sc21AlbumArtData[0] = vector<__uint8>(&gOptions.m_artworkBody[0][0], &gOptions.m_artworkBody[0][gOptions.m_artworkBody[0].size()]);
				// TODO
				//m_sc21AlbumArtMime[0] = (voxMsgType & 0x00FF);
			}

			if (!m_sc21AlbumArtData[0].empty())
			{
				m_streamInfo.m_currentURL = "DNAS/streamart?sid=" + tos(ID());
				sc1_metadata_and_extension_info.m_songTitle += "StreamUrl='" + m_streamInfo.m_currentURL + "';";
			}
		}
	}

	uvoxMetadata_t sc21_metadata;
	try
	{
		createMetadataPackets(&sc21_metadata_s[0], (int)sc21_metadata_s.size(), MSG_METADATA_XML_NEW, sc21_metadata);
	}
	catch(const exception &ex)
	{
		ELOG(ex.what(), LOGNAME, ID());
	}

	if (!sc1_metadata_and_extension_info.m_songTitle.empty())
	{
		_addMetadata(m_sc1StreamLock, m_sc1_ring_buffer, m_sc1MetadataLock, m_sc1MetadataTable, sc1_metadata_and_extension_info);
	}

	if (!sc21_metadata.empty())
	{
		_addMetadata(m_sc21StreamLock, m_sc21_ring_buffer, m_sc21MetadataLock, m_sc21MetadataTable, sc21_metadata);
	}

	// uvox metadata must also be injected into the stream itself
	if (!sc21_metadata.empty())
	{
		m_sc21MetadataLock.lock();
		m_sc21MetadataToPutInline = sc21_metadata;
		m_sc21MetadataLock.unlock();
	}

	return (!sameTitle ? 1 : 0) | (!sameUrl ? 2 : 0) | (!sameNext ? 4 : 0);
}


void streamData::pushMetricsYP (bool force)
{
    if (m_dead || m_ID <= 0 || m_streamInfo.m_streamPublic == false)
        return;
    time_t now = ::time(NULL);
    if (force == false && m_nextYPPush > now)
        return;

    if (m_parser)
    {
        metrics::metaInfo meta;

        meta.m_sid = m_ID;
        meta.m_audience = -1;
        meta.m_private = m_streamInfo.m_streamPublic ? false : true;
        meta.m_maxListeners = (m_streamInfo.m_streamMaxUser > 0) ? m_streamInfo.m_streamMaxUser : gOptions.maxUser();
        meta.m_song = m_streamInfo.m_currentSong;
        meta.m_format = m_streamInfo.m_contentType;
        meta.m_bitrate = m_streamInfo.m_streamBitrate;
        meta.m_samplerate = m_streamInfo.m_streamSampleRate;
        meta.m_agent = m_streamInfo.m_sourceIdent;
        meta.m_publicIP = m_streamInfo.m_publicIP;
        meta.m_sourceIP = m_streamInfo.m_srcAddr;
        meta.m_version = m_parser->getVersionName();
        metrics::updateMeta (meta);
        m_nextYPPush = now + 600;   // make it at most 10 mins.
    }
    else
        m_nextYPPush = now + 20; // no data currently retry again in 20 seconds
}


void streamData::clearCachedMetadata() throw()
{
	m_sc1MetadataLock.lock();
	m_sc1MetadataTable.clear();
	m_sc1MetadataLock.unlock();

	m_sc21MetadataLock.lock();
	m_sc21MetadataTable.clear();
	m_sc21MetadataToPutInline.clear();
	m_sc21MetadataLock.unlock();

	m_sc21StreamAlbumArtLock.lock();
	m_sc21StreamAlbumArtTable.clear();
	m_sc21StreamAlbumArtToPutInline.clear();
	m_sc21StreamAlbumArtLock.unlock();

	m_sc21PlayingAlbumArtLock.lock();
	m_sc21PlayingAlbumArtTable.clear();
	m_sc21PlayingAlbumArtToPutInline.clear();
	m_sc21PlayingAlbumArtLock.unlock();

	m_sc21AlbumArtData[0].clear();
	m_sc21AlbumArtData[1].clear();

	m_sc21AlbumArtMime[0] = 0;
	m_sc21AlbumArtMime[1] = 0;
}

void streamData::clearCachedArtwork(const size_t index) throw()
{
	if (!index)
	{
		m_sc21StreamAlbumArtLock.lock();
		m_sc21StreamAlbumArtTable.clear();
		m_sc21StreamAlbumArtToPutInline.clear();
		m_sc21StreamAlbumArtLock.unlock();
	}
	else
	{
		m_sc21PlayingAlbumArtLock.lock();
		m_sc21PlayingAlbumArtTable.clear();
		m_sc21PlayingAlbumArtToPutInline.clear();
		m_sc21PlayingAlbumArtLock.unlock();
	}

	m_sc21AlbumArtData[index].clear();
	m_sc21AlbumArtMime[index] = 0;

	addSc1MetadataAtCurrentPosition("", m_streamInfo.m_currentSong,
									(!m_sc21AlbumArtData[1].empty() ? "DNAS/playingart?sid=" + tos(ID()) :
									 !m_sc21AlbumArtData[0].empty() ? "DNAS/streamart?sid=" + tos(ID()) : ""),
									 m_streamInfo.m_comingSoon);
}

streamData::streamID_t streamData::getStreamIdFromPath(const uniString::utf8 &url, bool& htmlPage) throw()
{
	// look for an exact match against / which is sid=1 since if
	// we've gotten to here then it's unlikely to be non-client.
	if ((url == utf8("/")) || (url == utf8("/;")) || url.empty())
	{
		return DEFAULT_CLIENT_STREAM_ID;
	}

	streamData::streamID_t sid = DEFAULT_CLIENT_STREAM_ID;
	config::streams_t streams;
	gOptions.getStreamConfigs(streams);

	if (!streams.empty())
	{
		for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
		{
			// look for an exact match against the streampath
			if ((*i).second.m_urlPath == url)
			{
				sid = (*i).first;
				break;
			}
			else
			{
				// otheriwse look for a streampath with additional paramters on it
				// and then attempt to match things back
				// e.g. /bob;stream.mp3 or /bob/;stream.mp3
				// note: /bob/bob checking doesn't want to work...
				if ((url.find((utf8)";") != utf8::npos) || (url.find((utf8)"/;") != utf8::npos)/* ||
					(url.find(utf8("/")) != utf8::npos && url.find(utf8("/")) != 0)*/)
				{
					utf8 param_free = url.substr(0, (*i).second.m_urlPath.size());
					if (!param_free.empty() && (*i).second.m_urlPath == param_free)
					{
						// we need to treat '/stream' as a special
						// case and skip it and action it later on
						if (!(param_free == "/stream"))
						{
							sid = (*i).first;
							break;
						}
					}
				}
			}

			// otherwise look for the streampath and common values
			// which are appended to the end of the path e.g. /;
			if (!(*i).second.m_urlPath.empty())
			{
				utf8 stream("/stream");
				utf8::size_type pos = url.find((*i).second.m_urlPath),
								pos2 = url.find(stream);

				if ((pos != utf8::npos) && (pos == 0) &&
					// this helps to check for setting /stream as a streampath and it
					// causing all streams to be provided as if that stream instead
					// of the actually requested / expected stream since it's special
					((pos2 == 0) && ((*i).second.m_urlPath != stream)))
				{
					utf8 params = url.substr(pos + (*i).second.m_urlPath.size());
					if (!params.empty())
					{
						if ((params.find(utf8(";")) == 0) || (params.find(utf8("/")) == 0) || (params.find(utf8("/;")) == 0))
						{
							sid = (*i).first;
							break;
						}
					}
				}
			}

			// and if that fails then we instead base it on /stream/x/
			utf8::size_type pos = url.find((utf8)"/stream/");
			if (pos != utf8::npos)
			{
				utf8 stream_id = url.substr(8);
				// look for the raw stream url and if it's not exact then treat as a stream request
				if (!stream_id.empty() &&
					(stream_id.find((utf8)"/") == utf8::npos) &&
					(stream_id.find((utf8)";") == utf8::npos))
				{
					htmlPage = true;
				}
				sid = atoi((const char *)stream_id.c_str());
			}
		}
	}
	else
	{
		// and if that fails then we instead base it on /stream/x/
		utf8::size_type pos = url.find((utf8)"/stream/");
		if (pos != utf8::npos)
		{
			utf8 stream_id = url.substr(8);
			// look for the raw stream url and if it's not exact then treat as a stream request
			if (!stream_id.empty() &&
				(stream_id.find((utf8)"/") == utf8::npos) &&
				(stream_id.find((utf8)";") == utf8::npos))
			{
				htmlPage = true;
			}
			sid = atoi((const char *)stream_id.c_str());
		}
	}

	return sid;
}

//streamData::streamID_t streamData::getStreamUptime(streamID_t id) throw()
time_t streamData::getStreamUptime(streamID_t ID) throw()
{
	stackLock sml(g_streamMapLock);

	return g_streamUptime[ID];
}

streamData::source_t streamData::getClientType(streamData::source_t clientType, const uniString::utf8& userAgent)
{
	if (!userAgent.empty())
	{
		if (isUserAgentRelay(userAgent))
		{
			clientType = ((streamData::source_t)(clientType | streamData::RELAY));
		}
		if ((userAgent.find((utf8)"winampmpeg/") == 0) || (userAgent.find((utf8)"wafa/") == 0))
		{
			// this is a specical case and is unlikely to be any 'real' 5.09
			if (userAgent.find(utf8("winampmpeg/5.09")) == utf8::npos)
			{
				clientType = ((streamData::source_t)(clientType | streamData::WINAMP));
			}
			else
			{
				clientType = ((streamData::source_t)(clientType | streamData::RADIONOMY));
			}
		}
		if (userAgent.find((utf8)"curl/7.") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::CURL_TOOL));
		}
		if (userAgent.find((utf8)"sc_iradio/1.") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::SC_IRADIO));
		}
		if (userAgent.find((utf8)"icecast ") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::ICECAST | streamData::RELAY));
		}
		if (userAgent.find((utf8)"foobar2000/") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::FB2K));
		}
		if ((userAgent.find((utf8)"vlc/") == 0) ||
			(userAgent.find((utf8)" libvlc/") != utf8::npos))
		{
			clientType = ((streamData::source_t)(clientType | streamData::VLC));
		}
		if ((userAgent.find((utf8)"windows-media-player/") == 0) ||
			(userAgent.find((utf8)"nsplayer/") == 0))
		{
			clientType = ((streamData::source_t)(clientType | streamData::WMP));
		}
		if (userAgent.find((utf8)"roku/dvp") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::ROKU));
		}
		if (userAgent.find((utf8)"wiimc/") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::WIIMC));
		}
		if (userAgent.find((utf8)"audiostation/") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::SYNOLOGY));
		}
		if (userAgent.find((utf8)"itunes/") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::ITUNES));
		}
		if ((userAgent.find((utf8)"mplayer/") == 0) ||
			(userAgent.find((utf8)"mplayer ") == 0))
		{
			clientType = ((streamData::source_t)(clientType | streamData::MPLAYER));
		}
		if (userAgent.find((utf8)"applecoremedia/1.") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::APPLE));
		}
		if (userAgent.find((utf8)"psp-internetradioplayer/") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::PS));
		}
		if (userAgent.find((utf8)"radio toolbox/") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::RADIO_TOOLBOX));
		}

		// rough stream ripper identification
		if ((userAgent.find((utf8)"rma/1.0 (compatible; realmedia)") == 0) ||
			(userAgent.find((utf8)"nullsoft winamp3 version 3.0 (compatible)") == 0))
		{
			clientType = ((streamData::source_t)(clientType | streamData::WARNING));
		}

		// rough browser identification
		if (userAgent.find((utf8)"mozilla/5.0") == 0)
		{
			if (userAgent.find((utf8)"trident/") != utf8::npos)	// IE
			{
				clientType = ((streamData::source_t)(clientType | streamData::IE));
			}
			else if (userAgent.find((utf8)"chrome/") != utf8::npos)
			{
				clientType = ((streamData::source_t)(clientType | streamData::CHROME));
			}
			else if (userAgent.find((utf8)"safari/") != utf8::npos)
			{
				clientType = ((streamData::source_t)(clientType | streamData::SAFARI));
			}
			else if (userAgent.find((utf8)"firefox/") != utf8::npos)
			{
				clientType = ((streamData::source_t)(clientType | streamData::FIREFOX));
			}
		}
	}
	return clientType;
}

streamData::source_t streamData::getClientTypeCompat(streamData::source_t clientType, const uniString::utf8& userAgent)
{
	if (!userAgent.empty())
	{
		if ((userAgent.find((utf8)"windows-media-player/") == 0) ||
			(userAgent.find((utf8)"nsplayer/") == 0))
		{
			clientType = ((streamData::source_t)(clientType | streamData::WMP));
		}
		if (userAgent.find((utf8)"roku/dvp") == 0)
		{
			clientType = ((streamData::source_t)(clientType | streamData::ROKU));
		}
	}
	return clientType;
}

// Advert related code.
typedef pair<int, streamData::adGroup> idGroup;

void streamData::adEntry::flush(metrics::adSummary &summary)
{
    if (ad && (summary.sendRest || upper))
    {
        summary.count = (int)upper;
        summary.name = ad->m_description.c_str();
        summary.failed = ad->m_failed;
        summary.missing = ad->m_missing;

        metrics::metrics_advert_stats(summary);
        summary.sendRest = true;
    }
    upper = 0;
}


streamData::adEntry::adEntry (specialFileData *d)
{
    upper = 0;
    ad = d;
    if (d)
    {
        ++d->m_refcount;
        d->m_lastUsed = ::time (NULL);
        AD_DEBUG_LOG ("[STREAMDATA] content new, " + d->m_description + " now has " + tos (d->m_refcount));
    }
}


streamData::adEntry::adEntry (const adEntry &e)
{
    upper = 0;
    ad = e.ad;
    ++(e.ad->m_refcount);
    e.ad->m_lastUsed = ::time (NULL);
    AD_DEBUG_LOG ("[STREAMDATA] content copy, " + e.ad->m_description + " now has " + tos (e.ad->m_refcount));
}


streamData::adGroup::adGroup (adGroup &prev, adTrigger &trig)
{
    m_totalSizeSC1 = m_totalSizeSC2 = 0;
    m_trigger = &trig;

    std::deque <adEntry>::iterator  it = prev.ads.begin();
    for (; it != prev.ads.end(); ++it)
    {
        specialFileData *f = it->ad;
        ads.push_back (adEntry (f));
    }
}


void streamData::adGroup::appendAdvert(streamData::specialFileData &s)
{
	 ads.push_back (adEntry (&s));
	 m_totalSizeSC1 += s.m_sc1Buffer.size();
	 m_totalSizeSC2 += s.m_sc2Buffer.size();
}

void streamData::adGroup::flushCounts(metrics::adSummary &summary)
{
    summary.sendRest = false;
    summary.tstamp = m_trigger->m_playedAt;
    for (std::deque<adEntry>::iterator it = ads.begin(); it != ads.end(); ++it)
    {
        it->flush(summary);
    }
}

streamData::adGroups::adGroups (streamData *sd)
{
    m_sd = sd;
    refcount = 0;
    m_recheck = 0;
    nextUpdate = ::time(NULL);
    m_nextDownloadRun = nextUpdate + 40;
    m_type = ADVERT_MAP_FIXED;
    skippedAds = 0;
    retriever = NULL;
}


streamData::adGroups::~adGroups()
{
    while (triggers.empty() == false) // pop each on the triggers list and run release on it
    {
        adTrigger *trig = *triggers.begin();
        triggers.pop_front();
        releaseTrigger (trig);
    }
	if (retriever)
	{
		delete retriever;
		retriever = NULL;
	}
}


void streamData::adGroups::setType (const uniString::utf8 &s)
{
    if (s == "flex")
        m_type = ADVERT_MAP_FLEX;
    else if (s == "pause")
        m_type = ADVERT_MAP_PAUSE;
    else
        m_type = ADVERT_MAP_FIXED;
}


#if 0
streamData::adGroup *streamData::adGroups::insertAdTestFile(const streamData::streamID_t sid, streamData *sd)
{
	streamData::adGroup *ret = NULL;
	for (int i = 0; i < 4; i++)
	{
		specialFileData *f = new specialFileData("test advert");
		if (f)
		{
			vector<__uint8> v;
			sd->getAdTestFile(i).getSc1Data(v);
			if (!v.empty())
			{
				const int uvoxDataType = sd->streamUvoxDataType();
				unsigned int read_samplerate = sd->streamSampleRate(), samplerate = read_samplerate;
				// if the bitrate is not known, setting -1 will reject things
				// as we cannot be sure well insert ok if using '0' otherwise
				const int read_bitrate = cleanFileData("", v, v.size(), sd->streamBitrate(), samplerate, uvoxDataType,
													   "[ADVERT sid=" + tos(sid) + "] ", "advert", read_samplerate);

				if (!v.empty())
				{
					f->replaceData(v, uvoxDataType, read_bitrate, read_samplerate);
					AD_DEBUG_LOG("[ADVERT sid=" + tos(sid) + "] Adding to group TEST");
					ret = addToGroup(-2, f);
				}
				else
				{
					delete f;
					WLOG("[ADVERT sid=" + tos(sid) + "] Skipping test advert - "
						 "does not match the current stream format. Got " +
						 tos(read_bitrate) + " kbps, " + sampleRateStr(samplerate) +
						 " (stream) vs " + tos(sd ? sd->streamBitrate() : -1) +
						 " kbps, " + sampleRateStr(read_samplerate) + " (test advert)");
				}
			}
		}
	}

	return ret;
}
#endif

bool streamData::adGroups::attachGroupQueue (adGroupAccess &ac)
{
    if (ac.m_groupQueue == NULL && ac.inAdvertMode() == false)
    {
        stackLock sml(m_lock);

        adGroupMap_t::iterator it = mapping.find (ac.group);
        adGroupQueue *q;

        if (it == mapping.end())
        {
            q = new adGroupQueue();
            mapping [ac.group] = q;
        }
        else
            q = (*it).second;
        q->listeners++;
        ac.m_groupQueue = q;
        return true;
    }
    return false;
}


void streamData::adGroups::detachGroupQueue (adGroupAccess &ac)
{
     if (ac.m_groupQueue)
     {
         stackLock sml(m_lock);

         adGroupQueue *q = ac.m_groupQueue;
         adGroup *run = ac.cached_ref;

         q->listeners--;
         AD_DEBUG_LOG ("drop from group " + tos(ac.group) + " referenced, listeners now " + tos(q->listeners), ADLOGNAME, m_sd->ID());
         if (run)
         {
             // admetrics
             releaseTrigger (run->m_trigger);
         }
         ac.m_groupQueue = NULL;
         ac.cached_ref = NULL;
     }
     else
         AD_DEBUG_LOG ("no group queue referenced from listener", ADLOGNAME, m_sd->ID());
}

streamData::adGroup *streamData::adGroups::findGroup(adGroupAccess &ac, ringBufferAccess_t pos)
{
    adGroupQueue *q = ac.m_groupQueue;
    list <adGroup *>::iterator it = q->queue.begin();

    for (; it != q->queue.end(); ++it)
    {
        adGroup *g = *it;
        if (g->m_trigger == NULL) return NULL;  // should not happen
        ringBufferAccess_t tpos = g->m_trigger->getStartPos (ac.m_sc2);

        if (pos == tpos) // found it
            return g;
        if (pos > tpos) // past it, jump out
            break;
    }
    return NULL;
}

#if 0
streamData::adGroup *streamData::adGroups::findGroup(const streamData::streamID_t sid, const int id)
{
	// PSA = -1, TEST = -2
	if (mapping && ((id > 0) || (id == -2)))
	{
		map<int, adGroup>::iterator search = mapping->find(id);
		if (search == mapping->end())
		{
			// if there's no test group, we now
			streamData::adGroup *ret = NULL;
			if (id == -2)
			{
				streamData *sd = streamData::accessStream(sid);
				if (sd)
				{
					ret = insertAdTestFile(sid, sd);
					sd->releaseStream();
				}
			}
			return ret;
		}
		return &search->second;
	}
	// if there's no mapping, we need to create one for the test case
	else if (!mapping && (id == -2))
	{
		streamData::adGroup *ret = NULL;
		mapping = new map<int, adGroup>;
		if (mapping)
		{
			streamData *sd = streamData::accessStream(sid);
			if (sd)
			{
				ret = insertAdTestFile(sid, sd);
				sd->releaseStream();
			}
		}
		return ret;
	}
	return NULL;
}
#endif

void streamData::adGroups::purge (ringBuffer_t &buffer)
{
    if ((m_recheck & 31) == 0)
    {
        // drop reference when off the ring buffer, loop in case there are a few to purge
        while (triggers.empty() == false)
        {
            adTrigger *t = triggers.back();
            if (t== NULL)
                DLOG ("purge trigger NULL");
            if (t && (t->m_playedAt || triggers.size() > 1))
            {
                size_t sz = buffer.m_data.size();
                //DLOG ("trig " + t->m_id + ", played " + tos((long)t->m_playedAt) + ", ret " + tos((long)t->m_returnPtrSC1) + ", wptr " + tos((long)(buffer.m_writePtr)) + ", sz " + tos(sz));
                if (buffer.m_writePtr < sz) // for early cases of stream start
                    break;
                if (t->m_returnPtrSC1 < (buffer.m_writePtr - sz))
                {
                    AD_DEBUG_LOG ("trigger " + t->m_id + " reference off the ring buffer", ADLOGNAME, m_sd->ID());
                    triggers.remove (t);
                    releaseTrigger (t);
                    continue;
                }
            }
            break;
        }
    }
    if ((m_recheck & 255) == 0)
    {
        // DLOG("rechecking for group purge");
        purgeGroups();
    }
    ++m_recheck;
}


void streamData::adGroups::createNewTrigger (const string &reqID)
{
    string id;
    if (reqID == "")
    {
        stringstream s;
        s << tos(time_now_ms());
        id = s.str();
    }
    else
        id = reqID;

    adTrigger *trig = new adTrigger (id, m_sd->ID());
    triggers.push_front (trig);
}


size_t streamData::adGroups::overlaySize (adTrigger *trigger, bool sc2)
{
    int gn = 0;

    if (trigger == NULL) abort();
    if (sc2)   // assume sc2 is done after sc1
        return trigger->m_maxSizeSC2;

    adGroupMap_t::iterator it = mapping.begin();
    for (gn = 0; gn < 8 && it != mapping.end(); ++gn, ++it)  // check so many group queues for filled adverts
    {
        adGroupQueue *q = it->second;

        for (list<adGroup*>::iterator it = q->queue.begin(); it != q->queue.end(); ++it)
        {
            adGroup *g = *it;
            if (g->m_trigger == trigger)
            {
                if (trigger->m_playedAt == 0)
                {
                    g->recalculateTotalSize();
                }
                break;
            }
        }
    }
    if (trigger->m_maxSizeSC1)
        ILOG ("Trigger " + tos(trigger->m_id) + " size is " + tos(trigger->m_maxSizeSC1) + "/" + tos(trigger->m_maxSizeSC2), ADLOGNAME, m_sd->ID());
    return trigger->m_maxSizeSC1;
}


streamData::adGroup *streamData::adGroups::addToGroup (const string &id, const int grp, specialFileData *f)
{

	if ((f == NULL) || !grp)
	{
		return NULL;
	}
    std::map<size_t, adGroupQueue *>::iterator it = mapping.find (grp);
    adGroupQueue *q;
	if (it == mapping.end())
	{
	    AD_DEBUG_LOG ("new queue " + tos(grp) + " on " + id, ADLOGNAME, m_sd->ID());
        mapping [grp] = q = new adGroupQueue();
	}
    else
        q = it->second;
    adTrigger *cur = NULL; //, *prev = NULL;
    cur = triggers.empty() ? NULL : triggers.front();

    if (cur == NULL || cur->m_id != id)
    {
        //prev = cur;
        createNewTrigger (id);
        if (cur && cur->m_playedAt == 0)
        {
            triggers.remove (cur);
            releaseTrigger (cur);
            return addToGroup (id, grp, f);
        }
    }
    else
        if (cur->m_playedAt)
        {
            AD_DEBUG_LOG ("Trigger " + id + " has already started, skipping", ADLOGNAME, m_sd->ID());
            return NULL;
        }

    cur = triggers.front();

    adGroup *g = q->queue.empty() ? NULL : q->queue.front();

    if (g && g->m_trigger != cur)
        g = NULL;
    if (g == NULL || g->m_trigger->m_playedAt)
    {
        g = new adGroup;
        g->m_trigger = cur;
        q->queue.push_front (g);
    }
	g->appendAdvert(*f);

	if (cur->m_maxSizeSC1 < g->m_totalSizeSC1)
		cur->m_maxSizeSC1 = g->m_totalSizeSC1;
	if (cur->m_maxSizeSC2 < g->m_totalSizeSC2)
		cur->m_maxSizeSC2 = g->m_totalSizeSC2;

    return NULL;
}

#if 0
void streamData::adGroups::queueNewSet(const streamID_t sid, adGroups *qGroups)
{
	stackLock sml(m_lock);
	if (qGroups)
	{
		if (queuedGroups)
		{
			AD_DEBUG_LOG("[ADVERT sid=" + tos(sid) + "] Replacing queued advert set");
			delete queuedGroups;
		}
		queuedGroups = qGroups;
		if (queuedGroups->mapping && queuedGroups->adData)
		{
			const size_t group_size = queuedGroups->mapping->size(),
						 adverts_size = queuedGroups->adData->size();
			ILOG("[ADVERT sid=" + tos(sid) + "] Updated adverts: queued " +
				 tos(group_size) + " group" + (group_size != 1 ? "s" : "") + ", " +
				 tos(adverts_size) + " advert" + (adverts_size != 1 ? "s" : "") + ", " +
				 tos((queuedGroups->skippedAds > 0 ? queuedGroups->skippedAds : 0)) +
				 " advert file" + (queuedGroups->skippedAds != 1 ? "s" : "") + " skipped,"
				 " next update in " + tos(queuedGroups->nextUpdate - ::time(NULL)) + " seconds.");
		}
	}
}
#endif

#if 0
void streamData::adGroups::flushGroupCounts(const streamData::streamID_t sid)
{
	if (mapping.empty() == false)
	{
		streamData *sd = streamData::accessStream (sid);
		if (sd == NULL)
			return;

		if (sd->radionomyID().empty() && sd->streamAdvertMode())
		{
		    metrics::adSummary summary;

		    summary.sid = sid;
		    summary.path = getStreamPath(sid);
		    summary.tstamp = playedAt;
		    summary.sd = sd;

		    for (adGroupMap_t::iterator it = mapping.begin(); it != mapping.end(); ++it)
            {
                summary.group = (*it).first;
                (*it).second->flushStats (summary);
            }
		}
		sd->releaseStream();
	}
}
#endif

#if 0
// if the increase is set to false then we require the sid
void streamData::adGroups::join(const bool increase, const streamData::streamID_t sid, const bool testing)
{
	stackLock sml(m_lock);
	if (increase)
	{
		if ((refcount == 0) && (playedAt == 0))
		{
			AD_DEBUG_LOG("[ADVERT sid=" + tos(sid) + "] Starting " + (testing ? "test " : "") + "advert run");
			playedAt = ::time(NULL);
		}
		++refcount;
	}
	else
	{
		--refcount;
		if (refcount == 0)
		{
			AD_DEBUG_LOG("[ADVERT sid=" + tos(sid) + "] End of " + (testing ? "test " : "") + "advert run");
			if (!testing)
			{
				flushGroupCounts(sid);
				nextUpdate = 1; // force new download of adverts
			}
			playedAt = 0;
		}
	}
}
#endif

streamData::adTrigger::~adTrigger ()
{
    AD_DEBUG_LOG ("Trigger destroyed, " + m_id, ADLOGNAME, m_sid);
}


void streamData::adGroups::releaseTrigger (adTrigger *trigger)
{
    if (trigger == NULL) return;

    AD_DEBUG_LOG ("trigger " + trigger->m_id + " had count " + tos(trigger->m_inUse), ADLOGNAME, trigger->m_sid);
    if (trigger->m_inUse > 1)
    {
        --trigger->m_inUse;
        return;
    }
    if (trigger->m_playedAt)
        flushStats (trigger);

    adGroupMap_t::iterator it = mapping.begin();
    unsigned int num = 0;

    while (it != mapping.end())
    {
        size_t grp_num = it->first;
        adGroupQueue *q = it->second;

        ++it;
        if (q->queue.empty())
            continue;

        adGroup *g = q->queue.back();
        if (g->m_trigger == trigger) // should be true
        {
            q->queue.pop_back();
            AD_DEBUG_LOG ("deleting from group " + tos(grp_num) + " matching trigger " + tos(trigger->m_id), ADLOGNAME, trigger->m_sid);
            delete g;
            ++num;
        }
        else
            AD_DEBUG_LOG ("trigger unexpected " + tos(g->m_trigger->m_id), ADLOGNAME, trigger->m_sid);
    }
    ILOG ("trigger " + tos(trigger->m_id) + " dropped with " + tos(num) + " populated groups", ADLOGNAME, trigger->m_sid);
    delete trigger;
}


void streamData::adGroups::purgeGroups()
{
    adGroupMap_t::iterator it = mapping.begin();

    while (it != mapping.end())
    {
        size_t grp_num = it->first;
        adGroupQueue *q = it->second;

        if (q->listeners == 0)
        {
            adGroupMap_t::iterator to_go = it;
            adGroupQueue *q = to_go->second;
            ++it;
            AD_DEBUG_LOG ("Purging group " + tos (grp_num), ADLOGNAME, m_sd->ID());
            mapping.erase (to_go);
            delete q;
            continue;
        }
        // DLOG ("[ADVERT] purge group " + tos(grp_num) + "still has " + tos (q->listeners));
        ++it;
    }
}


streamData::adGroupQueue::~adGroupQueue()
{
    list<adGroup*>::iterator it = queue.begin();

    AD_DEBUG_LOG ("[ADVERT] queue destroyed, blocks remaining " + tos(queue.size()));
    for (; it != queue.end(); ++it)
    {
        AD_DEBUG_LOG ("[ADVERT] Deleting group");
        delete *it;
    }
}


void streamData::adGroupQueue::flushStats (metrics::adSummary &summary, adTrigger *trigger)
{
    for (list<adGroup*>::iterator it = queue.begin(); it != queue.end(); ++it)
    {
        adGroup *g = *it;
        if (g->m_trigger == trigger && g->m_trigger->m_inUse <= 1)
        {
            summary.id = g->m_trigger->m_id;
            g->flushCounts (summary);
            break;
        }
    }
}


int streamData::adGroup::recalculateTotalSize ()
{
    std::deque <adEntry>::iterator it = ads.begin();
    int missing = 0;

    m_totalSizeSC1 = 0;
    m_totalSizeSC2 = 0;
    for (;it != ads.end(); ++it)
    {
        adEntry &e = *it;
        if (e.ad == NULL || e.ad->m_missing)
        {
            ++missing;
            continue;
        }
        m_totalSizeSC1 += e.ad->m_sc1Buffer.size();
        m_totalSizeSC2 += e.ad->m_sc2Buffer.size();
    }
    if (m_trigger)
    {
        bool updated = false;
        if (m_trigger->m_maxSizeSC1 < m_totalSizeSC1)
        {
            m_trigger->m_maxSizeSC1 = m_totalSizeSC1;
            updated = true;
        }
        if (m_trigger->m_maxSizeSC2 < m_totalSizeSC2)
        {
            m_trigger->m_maxSizeSC2 = m_totalSizeSC2;
            updated = true;
        }
        if (updated)
            AD_DEBUG_LOG ("Group size is " + tos(m_totalSizeSC1) + "/" + tos(m_totalSizeSC2), ADLOGNAME, m_trigger->m_sid);
    }
    return missing;
}


void streamData::adGroups::flushStats (adTrigger *trigger)
{
    adGroupMap_t::iterator it = mapping.begin();
    metrics::adSummary summary;

    summary.sid = m_sd->ID();
    summary.path = getStreamPath(summary.sid);
    summary.sd = m_sd;

    //DLOG ("FLUSHing stats for metrics");
    for (; it != mapping.end(); ++it)
    {
        adGroupQueue *q = it->second;

        summary.group = (*it).first;
        q->flushStats (summary, trigger);
    }
}


streamData::adGroup *adGroupAccess::getGroup(streamData::streamID_t, streamData::adGroups &, const bool)
{
    if (cached_ref)
        return cached_ref; // avoids the traversal of map
    if (m_groupQueue == NULL)
        return NULL;

    return cached_ref;
}


bool adGroupAccess::foundNextAdvert (const streamData::streamID_t sid, streamData::adGroups &/*groups*/, streamData::adGroup &g)
{
    do
    {
        if (idx >= (int)g.ads.size())
            break;
        streamData::adEntry *e = &g.ads[idx];
        if (e == NULL || e->ad == NULL)
        {
            AD_DEBUG_LOG("Listener on group " + (group == -2 ? "TEST" : tos(group)) +
                    ", idx " + tos(idx) + ", content missing", ADLOGNAME, sid);
            idx++;
            continue;
        }
        utf8 msg = "Listener (grp ";
        msg += tos (group);
        msg += ", idx ";
        msg += tos (idx);
        msg += ", ad dura ";
        msg += tos(e->ad->m_duration);
        msg += ", rem ";
        msg += tos(m_duration);
        msg += ")";
        AD_DEBUG_LOG (msg, LOGNAME, sid);

        unsigned ad_dur = (unsigned)(e->ad->m_duration+0.5);
        if (g.m_trigger->m_type == ADVERT_MAP_FIXED && ad_dur > m_duration)
        {
            idx++;
            continue;
        }
        if (g.m_trigger->m_type == ADVERT_MAP_FLEX)
            m_duration -= ad_dur;
        ++g.ads [idx].upper;
        return true;
    } while (1);
    return false;
}


bool adGroupAccess::anotherAd(const streamData::streamID_t sid, streamData::adGroups &groups, const bool testing)
{
	streamData::adGroup *g = getGroup(sid, groups, testing);
	if ((g == NULL) || g->ads.empty())
	{
		cached_ref = NULL; 
		return false;
	}

    offset = 0;
    int total = (int)g->ads.size();

    ++idx;
    if (idx < total)
    {
        stackLock sml(groups.m_lock);
        if (foundNextAdvert (sid, groups, *g))
        {
            return true;
        }
    }
    return false;
}

const bool adGroupAccess::inAdvertMode() const
{
	return (cached_ref) ? true : false;
}

void adGroupAccess::setGroup (size_t id)
{
    group = id;
    m_grpChanged = true;
}

void adGroupAccess::changedGroup (streamData::adGroups &groups)
{
    if (m_grpChanged == false || inAdvertMode())
        return;

    if (m_groupQueue)
        groups.detachGroupQueue (*this);
    groups.attachGroupQueue (*this);
    m_grpChanged = false;
}


bool adGroupAccess::haveAdverts(const streamData::streamID_t sid, streamData::adGroups &groups, streamData::ringBufferAccess_t pos)
{
    if (m_groupQueue == NULL)
        return false;

    streamData::adGroup *g = groups.findGroup (*this, pos), *prev = cached_ref;

    if ((g == NULL) || (g->ads.size() == 0))
    {
        return false;
    }
    idx = 0;

    cached_ref = g;
    stackLock sml(groups.m_lock);
    ++g->m_trigger->m_inUse;
    m_duration = g->m_trigger->m_duration;
    if (m_duration == 0)
        m_duration = 3600;
    if (prev)
        groups.releaseTrigger (prev->m_trigger);
    if (foundNextAdvert (sid, groups, *g))
        return true;
    return false;
}

streamData::specialFileData *adGroupAccess::getAd(const streamData::streamID_t sid, streamData::adGroups &groups, const bool testing)
{
    streamData::adGroup *g = getGroup(sid, groups, testing);
    do
    {
        if (g == NULL) break;
        if (idx < 0 || idx >= (int)g->ads.size()) break;

        streamData::adEntry *e = &g->ads[idx];
        return e->ad;
    } while (1);
    return NULL;
}

void adGroupAccess::stopAdRun(const streamData::streamID_t, streamData::adGroups &groups, const bool)
{
    if (cached_ref)
    {
        if (cached_ref->m_trigger)
            groups.releaseTrigger (cached_ref->m_trigger);
        cached_ref = NULL;
    }
}

// routine to catch whatever conditions will trigger an advert
// 
void streamData::checkForAdverts()
{
	// only fire if it's a public stream and the stream has been added to the YP
	// otherwise we're not going to have the radionomyid from the authhash info.
	if (!iskilled() && (m_streamInfo.m_streamPublic) &&
		(!m_dead) && m_ID && m_streamInfo.m_advertMode &&
		(m_streamInfo.m_avgBitrate > 0) && (m_streamInfo.m_streamSampleRate > 0))
	{
		advertGroups.checkForNewAdverts(targetSpotUrl(), ID());
	}
}

void streamData::adGroups::checkForNewAdverts(const uniString::utf8& url, const streamData::streamID_t sid)
{
    time_t now = ::time (NULL);
    bool start_map_retrieval = false;
    do
    {
        stackLock sml (adContentLock);
        if (m_nextDownloadRun && m_nextDownloadRun <= now)
        {
            m_nextDownloadRun = 0;
            SimpleThread (streamData::adGroups::runAdDownload, NULL);
        }

        // in this case, we should just skip until things
        // have been processed. when processed then we
        // can step through and that will allow updates.
        if (nextUpdate == 0)
        {
            if (retriever && retriever->cleanup)
                break;
            return;
        }
        else if (nextUpdate <= now)
        {
            nextUpdate = 0; // disable while this starts up
            start_map_retrieval = true;
            break;
        }
        return;
    } while (0);

    if (start_map_retrieval)
    {
        loadGroups(url, sid);
        return;
    }
    AD_DEBUG_LOG("Cleanup advert map thread", ADLOGNAME, sid);
    delete retriever;
    retriever = NULL;
}

uniString::utf8 streamData::getAdvertGroup()
{
	if (m_streamInfo.m_advertMode)
	{
		utf8 groups = "Ad setup is correct, but no adverts / groups are currently assigned to this stream.";
		// only fire if it's a public stream and the stream has been added to the YP
		// otherwise we're not going to have the radionomyid from the authhash info.
		if ((m_streamInfo.m_streamPublic) && (!m_dead) && m_ID && m_streamInfo.m_advertMode)
		{
			groups = advertGroups.getAdvertGroup();
		}
		else
		{
			if (m_dead)
			{
				return "Ads cannot be provided without an active stream source.";
			}
			else
			{
				return "Stream is not configured properly for ads.<br><br>"
					   "Check that the authhash for this stream is correct, "
					   "and contact support if the condition persists.";
			}
		}
		return groups;
	}

	// otherwise, we have to assume that we've
	// been able to get something or it's just
	// not configured and so need to register.
	return "This stream is not configured for adverts.<br><br>" +
		   utf8(m_streamInfo.m_radionomyID.empty() ? "Please <a target=\"blank\" href=\"https://radiomanager.shoutcast.com\"><b>register</b></a> your stream for the DNAS+ service" :
													 "Please <a target=\"blank\" href=\"https://radiomanager.shoutcast.com\"><b>upgrade</b></a> your stream to the DNAS+ service") +
		   " to enable advert support.";
}

bool streamData::knownAdvertGroup(const streamData::streamID_t &sid, const int group)
{
	if (sid > 0)
	{
		map<streamID_t,streamData*>::const_iterator i = g_streamMap.find(sid);
		if (i == g_streamMap.end())
		{
			return false;
		}
        adGroups &gps = (*i).second->advertGroups;
        stackLock sml (gps.m_lock);
		return ((*i).second->advertGroups.findQueue (group) != NULL);

	}
	return false;
}


streamData::adGroupQueue *streamData::adGroups::findQueue (int grp)
{
    std::map<size_t, adGroupQueue *>::iterator it = mapping.find (grp);
    adGroupQueue *q = NULL;
    if (it != mapping.end())
        q = it->second;
    return q;
}


utf8 streamData::adGroups::getAdvertGroup()
{
	stackLock sml(m_lock);

    utf8  groups;
    unsigned count = 0;
    for (adGroupMap_t::const_iterator it = mapping.begin(); it != mapping.end(); ++it, ++count)
    {
        groups += (count ? ", " : "The following advert groups are currently in use:<br><br><b>") + tos((*it).first);
    }

    if (count)
    {
        groups += "</b>";
    }
    else
        groups +=  "Ad setup is correct, but no adverts / groups are currently assigned to this stream.";

    if (skippedAds > 0)
    {
        groups += "<br><br>There are adverts retrieved which do not match the current stream format.<br>"
            "These will not be played to avoid breaking stream playback.";
    }
    return groups;
}


struct ad_dload_info
{
     stringstream       ss;
     int                respcode;
     int                type;
     size_t             length;

     ad_dload_info()    { respcode = 0; length = 0; type = 0; }
};


static size_t handle_ad_dload_header (void *ptr, size_t size, size_t nmemb, void *stream)
{
    ad_dload_info *info = (ad_dload_info*)stream;
    size_t length = size * nmemb;
    char *s = (char*)ptr;

    do
    {
        if (length == 0) break;

        s [length] = 0;

        if (length > 12 && strncmp (s, "HTTP", 4) == 0)
        {
            unsigned int resp = 0;
            char eol[5] = "";
            if (sscanf (s, "%*8s %u %4[^\r]", &resp, eol) == 2 && strcmp (eol, "OK") == 0 && resp > 99 && resp < 600)
                info->respcode = resp;
            break;
        }
        if (length > 12 && strncasecmp (s, "content-type:", 13) == 0)
        {
            char content[40];
            if (sscanf (s+14, " %40[^\r ]", content) == 1)
            {
                if (strcmp (content, "audio/aac") == 0)
                    info->type = AAC_LC_DATA;
                else if (strcmp (content, "audio/aacp") == 0)
                    info->type = AACP_DATA;
                else if (strcmp (content, "audio/mpeg") == 0)
                    info->type = MP3_DATA;
                else if (strcmp (content, "application/octet-stream") == 0)
                {
                    AD_DEBUG_LOG("[ADVERT] ad download has default mime, assuming aac");
                    info->type = AACP_DATA;
                }
                else
                {
                    WLOG ("[ADVERT] ad download has unexpected mime " + utf8(content));
                    return 0;
                }
                break;
            }
        }
    } while (0);
    return length;
}


static size_t handle_ad_dload_body (void *ptr, size_t size, size_t nmemb, void *stream)
{
    ad_dload_info *info = (ad_dload_info*)stream;
    size_t length = size * nmemb;
    if (info->ss)
    {
        info->ss.write ((const char*)ptr, length);
        info->length += length;
    }

    bandWidth::updateAmount (bandWidth::ADVERTS, length);
    return length;
}


THREAD_FUNC streamData::adGroups::runAdDownload (void* /*arg*/)
{
    int purged = 0, retry = 0, added = 0, count = 0;
    CURL *curl = NULL;
    ad_dload_info info;
    char curl_error [CURL_ERROR_SIZE];

    curl = webClient::setupCurlDefaults (curl, "[ADVERT] ", "", 4, 20L);
    if (curl == NULL)
    {
        // set retry time
        m_nextDownloadRun = ::time(NULL) + 25;
        return NULL;
    }
    curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, handle_ad_dload_header);
    curl_easy_setopt (curl, CURLOPT_HEADERDATA, &info);
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, handle_ad_dload_body);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, &info);
    curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, &(curl_error[0]));

    // use progress/xfer functions to trap for the server kill case
    curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt (curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
    curl_easy_setopt (curl, CURLOPT_XFERINFODATA, (long)-1);

#ifdef CURLOPT_PASSWDFUNCTION
    curl_easy_setopt (curl, CURLOPT_PASSWDFUNCTION, my_getpass);
#endif
    // limit to 200 in one run and assume 6 retries is not a good sign and likely to stall things
    while (count < 200 && retry < 6)
    {
        specialFileData *f;
        if (1)
        {
            // grab the oldest
            stackLock sml (adContentLock);
            if (iskilled() || adContentPending.empty())
                break;
            list<specialFileData*>::iterator it = adContentPending.begin();
            f = *it;
            adContentPending.pop_front();
        }
        ++count;
        if (f->m_url == "")
        {
            ILOG ("[Advert] download requested but no url specified " + f->m_description);
            specialFileData::release (f);
            continue;
        }
        // clear old data
        memset (curl_error, 0, CURL_ERROR_SIZE);
        info.respcode = 0;
        info.length = 0;
        info.type = 0;
        info.ss.str (string());

        curl_easy_setopt (curl, CURLOPT_URL, f->m_url.c_str());
        int ret = curl_easy_perform (curl);
        if (ret != CURLE_OK)
        {
            // assume a timeout, add to the end for checking later
            if (f->m_retried < 2)
            {
                ++f->m_retried;
                ILOG("[ADVERT] failed to get " + f->m_description + ", will retry");
                ++retry;
                stackLock sml (adContentLock);
                adContentPending.push_back (f);
                continue;
            }
            ILOG ("[ADVERT] retried " + f->m_description + " several times, skipping");
        }
        else if (info.respcode == 200 && info.length && info.type)
        {
            // ok, we have something that initially looks like something
            vector<__uint8> &v = f->m_sc1Buffer;
            v.reserve (info.ss.tellp());
            std::copy (std::istreambuf_iterator<char>( info.ss ), std::istreambuf_iterator<char>(), std::back_inserter(v));

            //unsigned int read_samplerate = 0;

            f->verifyData("[ADVERT]");

            // now we either have nothing left or something valid
            if (v.size() > 0)
            {
                string e = " (" + tos (info.length);
                if (info.length != v.size())
                    e += " bytes, processed down to " + tos(v.size());

                added++;
                specialFileData::release (f);   // from content pending not pool
                ILOG("[ADVERT] Loaded advert " + f->m_description + e + " bytes), referenced " + tos(f->getRefcount()));
                continue;
            }
            ILOG ("[ADVERT] no useful data returned for " + f->m_description + " so flag for removal");
        } else {
           ELOG("[ADVERT] failed to get " + f->m_description + ", resp " + tos(info.respcode) + ", len " + tos(info.length) + ", type " + tos (info.type));
        }
        f->m_failed = true;
        // missing content, need to remove from pool
        ++purged;
        specialFileData::release (f);       // from content pending
        stackLock sml (adContentLock);
        adData.erase (f->m_description);    // drop entry from pool
        specialFileData::release (f);       // actually release pool reference
    }
    if (added || purged || retry)
        ILOG("[ADVERT] downloaded " + tos(added) + ", purged " + tos(purged) + ", to be retried " + tos(retry));
    curl_easy_cleanup (curl);

    time_t expire_advert = ::time(NULL) - 5400; // adverts untouched for 90 mins are dropped
    stackLock sml (adContentLock);

    unsigned expired_ads = 0;

    map<string, specialFileData*>::iterator it = adData.begin();
    while (it != adData.end())
    {
        specialFileData *f = it->second;
        map<string, specialFileData*>::iterator to_go = it;
        ++it;
        if (f->getRefcount() == 1 && f->m_lastUsed < expire_advert)
        {
            DLOG ("[ADVERT] ad " + f->m_description + " unused, dropping");
            adData.erase (to_go);    // drop entry from pool
            specialFileData::release (f);       // actually release pool reference
            ++expired_ads;
        }
    }
    if (expired_ads)
        ILOG("[ADVERT] " + tos(expired_ads) + " advert(s) expired from pool");

    if (m_nextDownloadRun == 0)
        m_nextDownloadRun = ::time(NULL) + ((count < 200) ? (retry ? 20 : 60) : 2);

    return NULL;
}


THREAD_FUNC streamData::adGroups::process(void* arg)
{
	streamData *sd = 0;
	try
	{
		adGroupsRetriever*& m_retriever = reinterpret_cast<adGroupsRetriever*&>(arg);
		if (!iskilled() && m_retriever && m_retriever->loaded && m_retriever->m_curl &&
			streamData::isSourceConnected(m_retriever->m_sid))
		{
			AD_DEBUG_LOG ("Trying to retrieve updated advert map", ADLOGNAME, m_retriever->m_sid);
			memset(m_retriever->m_curl_error, 0, CURL_ERROR_SIZE);
			CURL *curl = (!iskilled() ? m_retriever->m_curl : NULL);

            struct curl_slist *list = NULL;
            std::string uuid;
            if (m_retriever->last_uuid != "")
            {
                uuid = "Last-BlueBox-UUID: " + m_retriever->last_uuid;
                list = curl_slist_append (list, uuid.c_str());
                curl_easy_setopt (curl, CURLOPT_HTTPHEADER, list);
            }
			CURLcode ret = (!iskilled() ? curl_easy_perform(curl) : CURLE_FAILED_INIT);

            if (list)
                curl_slist_free_all (list);

			map<string, bool> skipped;
			int duration = 60; // default retry interval
			bool error = false;
            string id;

            stringstream s;
            s << tos(time_now_ms());
            id = s.str();

			sd = streamData::accessStream (m_retriever->m_sid);
			do
			{
				if (ret != CURLE_OK)
				{
					if (ret != CURLE_ABORTED_BY_CALLBACK)
					{
						ELOG ("Advert map retrieval failed with code: " +
							 tos(ret) + " [" + (m_retriever->m_curl_error[0] ?
							 m_retriever->m_curl_error : curl_easy_strerror(ret)) + "]", ADLOGNAME, m_retriever->m_sid);
					}
					else
					{
						curl = 0;  // abort this attempt
						if (sd)
						{
							sd->releaseStream();
							sd = 0;
						}
						break;
					}

					// better to retry in 1 minute than wait 20 minutes for connection
					// failure issues, etc especially if could not connect on startup
					duration = 60;
					break;
				}

				string tag;

				map<string, specialFileData*> &fileContents = sd->advertGroups.adData;

				string line1;
				while (!iskilled() && getline(m_retriever->ss, line1))
				{
					line1 = stripWhitespace(line1);
					if (line1.empty())
					{
						error = true;
						AD_DEBUG_LOG("Blank line detected for tag", ADLOGNAME, m_retriever->m_sid);
						break;
					}

                    if (gOptions.adMetricsDebug())
                    {
                        string outp = line1.substr (0, 40);
                        if (line1.length() > 40)
                            outp += "...";
                        AD_DEBUG_LOG("Retrieved line: " + outp, ADLOGNAME, m_retriever->m_sid);
                    }

					stringstream line(line1);
					line >> tag;
                    if (tag == "NextCall")
                    {
                        line >> duration;
                        if (duration < 5)
                        {
                            duration = 120; // filter out silly values.
                        }
                        continue;
                    }
					if (tag == "ID")
                    {
                        line >> id;
                        continue;
                    }
                    if (tag == "G")
                    {
                        int group = 0;
                        string name;
                        line >> group >> name;
                        // add to group if name exists in map
                        map<string, specialFileData*>::const_iterator it = fileContents.find(name);
                        if (it != fileContents.end())
                        {
                            specialFileData *f = it->second;
                            if (f->m_failed == false)
                            {
                                sd->advertGroups.addToGroup (id, group, f);
                                continue;
                            }
                        }
                        if (skipped.find(name) == skipped.end())
                        {
                            WLOG("Missing advert ref " + name + " on group " + tos(group), ADLOGNAME, m_retriever->m_sid);
                        }
                        continue;
                    }
                    if (tag == "AdURL")
                    {
                        string name, url;
                        line >> name;
                        AD_DEBUG_LOG("Parse groups: ref " + name, ADLOGNAME, m_retriever->m_sid);
                        if (line.fail() || name[0] == '\0')
                            continue;

                        line >> url;
                        if (url.empty() == false)
                        {
                            stackLock sml (adContentLock);

                            map<string, specialFileData*>::const_iterator map_it = fileContents.find(name);
                            if (map_it != fileContents.end())
                                continue;   // already loaded

                            std::list<specialFileData*>::iterator it = adContentPending.begin();

                            for (; it != adContentPending.end(); ++it)
                            {
                                if ((*it)->m_description == name)
                                {
                                    if ((*it)->m_missing)
                                        AD_DEBUG_LOG("found " + tos(name) + " waiting for retrieval", ADLOGNAME, m_retriever->m_sid);
                                    break;
                                }
                            }
                            if (it != adContentPending.end())
                                continue; // already scheduled to be downloaded

                            specialFileData *f = new specialFileData (name, url);
                            if (f)
                            {
                                f->increaseRefcount(); // referenced now in the 2 following places
                                fileContents [name] = f;
                                adContentPending.push_back (f);
                            }
                        }
                    }
					if (tag == "Ad")
					{
						bool process_file = true;
						string name, b64;
						line >> name;
						AD_DEBUG_LOG("Parse groups: ref " + name, ADLOGNAME, m_retriever->m_sid);
                        if (line.fail() || name[0] == '\0')
                            continue;

// we should allow for replacing an existing advert eventually
                        line >> b64;
                        stackLock sml (adContentLock);

						map<string, specialFileData*>::const_iterator it = fileContents.find(name);
						if (it != fileContents.end())
						{
							process_file = false;
						}

                        if (line.fail())
                        {
                            stringstream enc;
#if 1  // one line per base64 encoded advert
                            if (getline(m_retriever->ss, line1) && process_file)
                            {
                                enc << line1;
                            }
#else // multiple lines for base64 content but terminates on blank line
                            while (getline (ss, line1))
                            {
                                if (line1 == "")
                                {
                                    break;  //  End of base64 block
                                }
                                //AD_DEBUG_LOG("[ADVERT sid=" + tos(SID) + "] Read " + tos(line1.size()));
                                if (process_file)
                                {
                                    enc << line1;
                                }
                            }
#endif
                            b64 = enc.str();
                            line.clear();
                        }
						if (!b64.empty())
						{
							specialFileData *f = new specialFileData(name);
							if (f)
							{
								vector<__uint8> v = base64::decode(b64);
								size_t original_size = v.size();


								int uvoxDataType = (sd ? sd->streamUvoxDataType() : MP3_DATA),
									bitrate = (sd ? sd->streamBitrate() : -1);
								unsigned int read_samplerate = (sd ? sd->streamSampleRate() : 0),
											 samplerate = read_samplerate;
								// if the bitrate is not known, setting -1 will reject things
								// as we cannot be sure well insert ok if using '0' otherwise
								int read_bitrate = cleanFileData("", v, v.size(), bitrate, samplerate, uvoxDataType,
																 "[ADVERT sid=" + tos(m_retriever->m_sid) + "] ", "advert", read_samplerate);
								if (!v.empty())
								{
									f->replaceData(v, uvoxDataType, read_bitrate, read_samplerate);
									AD_DEBUG_LOG("Retrieved advert of length " +
												 tos(original_size) + " bytes" + (original_size != v.size() ? " (processed down to " +
												 tos(v.size()) + " bytes)" : ""), ADLOGNAME, m_retriever->m_sid);
									fileContents [name] = f;
								}
								else
								{
									delete f;
									AD_DEBUG_LOG("Skipping advert - does not match the current stream format. Got " +
												 tos(read_bitrate) + " kbps, " + sampleRateStr(read_samplerate) +
												 " vs " + tos(bitrate) + " kbps, " + sampleRateStr(samplerate), ADLOGNAME, m_retriever->m_sid);

									// this is a special case for when there is no advert
									// group but we're getting generic fillers (which we
									// cannot use as we don't know who it's intended for)
									if (name.find("completions") == string::npos)
									{
										skipped[name] = true;
									}
								}
							}
						}
						continue;
					}
				}

				// if we've got configured 'test' files then we
				// add them in at this stage though we have the
				// means to load them on-demand if not done now
				if (!sd)
				{
					sd = streamData::accessStream(m_retriever->m_sid);
				}
				if (!skipped.empty())
				{
					AD_DEBUG_LOG("Skipped " + tos(skipped.size()) + " adverts not matching the current stream format", ADLOGNAME, m_retriever->m_sid);
				}
			} while (iskilled());

			if (!curl)
			{
				// abort as there's nothing we can do
				// that won't otherwise make a crash
				return 0;
			}
			else
            {
                stackLock sml (adContentLock);
                sd->advertGroups.skippedAds = (!error ? (int)skipped.size() : -1);
                AD_DEBUG_LOG("Advert update interval: " + tos(duration), ADLOGNAME, m_retriever->m_sid);
                sd->advertGroups.nextUpdate = ::time(NULL) + duration;
                sd->advertGroups.lastUUID = m_retriever->last_uuid;
                time_t now = ::time(NULL);
                if (m_nextDownloadRun && m_nextDownloadRun > (now + 6))
                    m_nextDownloadRun = now + 4;

                m_retriever->cleanup = 1;
            }
			sd->releaseStream();
			sd = 0;
		}
	}
	catch (...)
	{
		adGroupsRetriever*& m_retriever = reinterpret_cast<adGroupsRetriever*&>(arg);
		if (m_retriever)
		{
			m_retriever->cleanup = 1;
		}

		if (sd)
		{
			sd->releaseStream();
		}
	}
	return 0;
}

void streamData::adGroups::loadGroups(const utf8& url, const streamData::streamID_t sid)
{
	if (sid > 0)
	{
		stackLock sml(m_lock);

		if (retriever)
		{
			if (retriever->cleanup == 0)
			{
				AD_DEBUG_LOG("Retriever thread already in progress", ADLOGNAME, sid);
				nextUpdate = ::time(NULL) + 30; // re-check every so often
				return;
			}

			delete retriever;
			retriever = NULL;
		}

		retriever = new adGroupsRetriever(url, sid);
		if (retriever && (retriever->loaded == 1) && (retriever->cleanup == 0) && isSourceConnected(sid))
		{
			SimpleThread(streamData::adGroups::process, retriever);
		}
		else
		{
			if (retriever)
			{
				// re-check every so often, but if we needed to re-pull
				// the stream's information then force a quicker check
				nextUpdate = ::time(NULL) + (!retriever->loaded ? 10 : 1);
				delete retriever;
				retriever = NULL;
			}
			else
			{
				nextUpdate = ::time(NULL) + 30; // re-check every so often
			}
		}
	}
}


streamData::adTrigger::adTrigger(const string &id, streamID_t sid)
{
    m_inUse = 1;
    m_sid = sid;
    m_id = id;
    m_type = ADVERT_MAP_FIXED;
    m_playedAt = (time_t)0;
    m_returnPtrSC1 = m_returnPtrSC2 = 0;
    m_maxSizeSC1 = m_maxSizeSC2 = 0;
}


streamData::adTrigger::adTrigger(const char *id, streamID_t sid)
{
    m_inUse = 1;
    m_sid = sid;
    m_id = string(id);
    m_playedAt = (time_t)0;
    m_type = ADVERT_MAP_FIXED;
    m_returnPtrSC1 = m_returnPtrSC2 = 0;
    m_maxSizeSC1 = m_maxSizeSC2 = 0;
}

streamData::ringBufferAccess_t streamData::adTrigger::getStartPos (bool sc2)
{
    return sc2 ? m_startPosSC2 : m_startPosSC1;
}


bool operator==(const streamData::adTrigger &lhs, const streamData::adTrigger &rhs)
{
    return (lhs.m_id == rhs.m_id);
}

