#ifdef _WIN32
#include <winsock2.h>
#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif
#endif
#include <stdio.h>
#include "protocol_shoutcastClient.h"
#include "ripList.h"
#include "stats.h"
#include "streamData.h"
#include "w3cLog.h"
#include "global.h"
#include "bandwidth.h"
#include "metrics.h"
#include "auth.h"
#include "yp2.h"
#include "FLV.h"
#include "MP3Header.h"
#include "ADTSHeader.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define LOGNAME             "DST"
#define AD_DEBUG_LOG(...)   do { if (gOptions.adMetricsDebug()) DLOG(__VA_ARGS__); } while(0)

protocol_shoutcastClient::protocol_shoutcastClient (protocol_HTTPStyle &hs, const streamData::streamID_t streamID,
        const uniString::utf8 &hostName, const uniString::utf8 &clientAddr, const uniString::utf8 &XFF, const streamData::source_t clientType)

    : runnable (hs),  m_streamID(streamID <= 0 || streamID > INT_MAX ? DEFAULT_CLIENT_STREAM_ID : streamID),
    m_unique(stats::getNewClientId()),  m_adAccess(clientType == streamData::SHOUTCAST2)
{
    m_newListener = m_timerStart = (time_t)0;
    m_timerFrames = m_frameLimit = m_frameCount = 0;
    m_fps = 0.0;
    m_bytesSentForCurrentTitle = m_totalBytesSent = 0;
    m_metaIntervalCounter = m_backupLoopTries = m_outBufferSize = 0;
    m_backupFileOffset = m_introFileOffset = 0;
    m_readPtr = 0;
    m_outBuffer = NULL;
    m_streamData = NULL;
    m_kickNextRound = m_ignoreDisconnect = false;
    m_lagOffset = 0;
    setGroup (m_unique);

    m_headRequest = (hs.m_httpRequestInfo.m_request == protocol_HTTPStyle::HTTP_HEAD);
    m_clientType = streamData::getClientType (clientType, stringUtil::toLower (hs.m_userAgent)); // double toLower work ??
    m_removeClientFromStats = (isUserAgentOfficial (stringUtil::toLower (hs.m_userAgent)) == false);

    if (gOptions.useXFF() && !XFF.empty() && metrics::metrics_verifyDestIP (gOptions, true, XFF).empty() == false)
        m_clientAddr = XFF;
    else
        m_clientAddr = clientAddr;
    m_clientLogString = dstAddrLogString (hostName, hs.m_clientPort, XFF, streamID);

    m_clientHostName = hostName;

    m_metaInterval = gOptions.getMetaInterval (streamID);
    m_userAgent = hs.m_userAgent;
    m_referer = hs.m_referer;
    m_XFF = XFF;
    m_startTime = m_lastTitleTime = ::time(NULL);
}


// setup tracking variables for W3C log
void protocol_shoutcastClient::setW3CState() throw()
{
	m_lastTitleTime = ::time(NULL);
	m_bytesSentForCurrentTitle = 0;
}

int protocol_shoutcastClient::detectAutoDumpTimeout (time_t &cur_time, const size_t streamID, const uniString::utf8 &msg) throw(runtime_error)
{
    const int autoDumpTime = gOptions.getAutoDumpTime(streamID);
    cur_time = ::time(NULL);
    if ((autoDumpTime > 0) && ((cur_time - m_lastActivityTime) >= autoDumpTime))
    {
        if (m_ignoreDisconnect == false)
            DLOG (msg, LOGNAME, streamID);
        throwEx<runtime_error>("");
    }
    return autoDumpTime;
}

//////////////////////////////////////////////////////////////////////////////

void protocol_shoutcastClient::acquireIntroFile(const bool sc2) throw()
{
	// if we've already loaded something then we won't keep
	// trying to re-acquire it for speed and consistency
	// whilst this specific client is connected at the time
	if (m_streamData && m_introFile.empty())
	{
		if (!sc2)
		{
			m_streamData->getIntroFile().getSc1Data(m_introFile);
		}
		else
		{
			m_streamData->getIntroFile().getSc2Data(m_introFile);
		}
	}

	m_introFileOffset = 0;
}

int protocol_shoutcastClient::acquireBackupFile(int *dataType, const bool sc2) throw()
{
	int read_bitrate = 0;
	// if we've already loaded something then we won't keep
	// trying to re-acquire it for speed and consistency
	// whilst this specific client is connected at the time
	if (m_backupFile.empty())
	{
		if (m_streamData)
		{
			if (!sc2)
			{
				m_streamData->getBackupFile().getSc1Data(m_backupFile);
			}
			else
			{
				m_streamData->getBackupFile().getSc2Data(m_backupFile);
			}

			m_streamData->streamSampleRate();
		}
		else
		{
			// for when there is no stream, we'll manually load the
			// file and try to do what we can to get it running...
			utf8 backupFile = gOptions.stream_backupFile(m_streamID);
			if (!gOptions.read_stream_backupFile(m_streamID))
			{
				backupFile = gOptions.backupFile();
			}

			if (!backupFile.empty())
			{
				const int backuploop = gOptions.getBackupLoop(m_streamID);
				if (!backuploop || (m_backupLoopTries <= backuploop))
				{
					int streamBitrate = gOptions.stream_maxBitrate(m_streamID);
					if (!gOptions.read_stream_maxBitrate(m_streamID) || !streamBitrate)
					{
						streamBitrate = gOptions.maxBitrate();
					}
					if (streamBitrate)
					{
						streamBitrate /= 1000;
					}

					unsigned int read_samplerate = 0;
					const int type = ((toLower(backupFile).rfind(utf8(".aac")) == (backupFile.size() - 4)) ? AACP_DATA : MP3_DATA);
					streamData::specialFileData loadBackupFile((backuploop == 1 ? "podcast" : "backup"));
					read_bitrate = loadBackupFile.loadFromFile(backupFile, streamBitrate, type, read_samplerate, m_clientLogString);
					if (!sc2)
					{
						loadBackupFile.getSc1Data(m_backupFile);
					}
					else
					{
						loadBackupFile.getSc2Data(m_backupFile);
					}
					if (dataType)
					{
						*dataType = type;
                        createFrameRate ((type==MP3_DATA), loadBackupFile.m_samplerate);
					}
				}
				else if (backuploop && (m_backupLoopTries < backuploop))
				{
					m_backupFile.clear();
				}
			}
		}
	}

	m_backupFileOffset = 0;

	return read_bitrate;
}

const int protocol_shoutcastClient::doTimeSlice(const bool sc2) throw(exception)
{
	int listenerTime = (int)gOptions.stream_listenerTime(m_streamID);
	if (!gOptions.read_stream_listenerTime(m_streamID))
	{
		listenerTime = (int)gOptions.listenerTime();
	}

	listenerTime *= 60; // convert to seconds
	const bool timesUp = listenerTime && ((::time(NULL) - m_startTime) > listenerTime);
	if (m_kickNextRound || timesUp || (m_streamData && m_streamData->isDead() && m_backupFile.empty()))
	{
		if (timesUp)
		{
			ILOG(m_clientLogString + "Listener time exceeded.", LOGNAME, m_streamID);
		}
		else if (m_kickNextRound)
		{
			ILOG(m_clientLogString + "Kicked [Agent: `" + m_userAgent + "', UID: " + tos(m_unique) + ", GRID: " + tos(getGroup()) + "]", LOGNAME, m_streamID);
		}
		else
		{
			const int backuploop = gOptions.getBackupLoop(m_streamID);
			if (!backuploop || (m_backupLoopTries <= backuploop))
			{
				// we're done with the backup file. get more data
				acquireBackupFile(0, sc2);
				if (!m_backupFile.empty())
				{
					++m_backupLoopTries;
					m_lastActivityTime = ::time(NULL);
					return 1;
				}
			}
			else if (backuploop && (m_backupLoopTries < backuploop))
			{
				m_backupFileOffset = 0;
				m_backupFile.clear();
			}
		}

		m_result.done();
		return 2;
	}

	return 0;
}

const bool protocol_shoutcastClient::sendText() throw(exception)
{
	return sendDataBuffer(m_streamID, m_outBuffer, m_outBufferSize, m_clientLogString);
}

void protocol_shoutcastClient::resetCommon() throw()
{
    if (m_newListener == 0)
    {
        time_t now = ::time (NULL);
        if (m_removeClientFromStats && m_streamData && m_streamData->streamPublic())
        {
            if (m_startTime + 3 < now)
            {
                metrics::metrics_listener_new(*this);
                m_streamData->advertGroups.attachGroupQueue (this->m_adAccess);
                m_newListener = now;
            }
        }
        else
            m_newListener = now;
    }
    if (m_streamData)
        m_adAccess.changedGroup (m_streamData->advertGroups);
}

// set read pointer to a nice distance from the write pointer. Return that distance
const streamData::ringBufferAccess_t protocol_shoutcastClient::resetReadPtr(std::vector<__uint8>& data, const bool sc2) throw()
{
#if defined(_DEBUG) || defined(DEBUG)
	DLOG(m_clientLogString + __FUNCTION__);
#endif

	// we also reset the remainder handling as
	// by this point, it's unlikely that it's
	// worth trying to keep up and so we reset
	m_remainder.clear();

    m_lagOffset = 0;
	if (m_streamData)
	{
		m_readPtr = m_streamData->getClientStartPosition(sc2);
		streamData::ringBufferAccess_t amt = 0;
		return m_streamData->getStreamData(amt, m_readPtr, data, 0, sc2);
	}
	else
	{
		m_readPtr = 0;
		return 0;
	}
}

void protocol_shoutcastClient::resetReadPtr(const bool sc2) throw()
{
#if defined(_DEBUG) || defined(DEBUG)
	DLOG(m_clientLogString + __FUNCTION__);
#endif

	// we also reset the remainder handling as
	// by this point, it's unlikely that it's
	// worth trying to keep up and so we reset
	m_remainder.clear();

	if (m_streamData == NULL)
	{
		m_readPtr = 0;
		return;
	}
    m_lagOffset = 0;
    if (m_adAccess.inAdvertMode ())
    {
        const streamData::adTrigger *t = m_adAccess.getCurrentTrigger();
        streamData::ringBufferAccess_t ptr;

        if (t)
            ptr = sc2 ? t->m_returnPtrSC2 : t->m_returnPtrSC1;
        else
        {
            ptr = sc2 ? m_streamData->advertGroups.m_returnPtrSC2 : m_streamData->advertGroups.m_returnPtrSC1;
            if (!ptr)
                ptr = (sc2 ? m_streamData->m_sc21_packet_starts : m_streamData->m_sc1_packet_starts).back();
        }
        streamData::ringBufferAccess_t lag = ((sc2 ? m_streamData->m_sc21_ring_buffer : m_streamData->m_sc1_ring_buffer).m_writePtr - m_readPtr);

        if (lag < (sc2 ? m_streamData->m_sc21_ring_buffer : m_streamData->m_sc1_ring_buffer).m_data.size())
            m_lagOffset = (sc2 ? m_streamData->m_sc21_ring_buffer : m_streamData->m_sc1_ring_buffer).m_writePtr - m_readPtr;
        else
        {
            ptr = (sc2 ? m_streamData->m_sc21_ring_buffer : m_streamData->m_sc1_ring_buffer).m_writePtr;
            m_lagOffset = 0;
        }

        m_readPtr = m_streamData->getClientStartPosition (ptr, sc2);
        // DLOG ("lag offset is " + tos (m_lagOffset) + " readptr " + tos (m_readPtr));
    }
    else if (m_totalBytesSent > 30000)
    {
        std::deque<streamData::ringBufferAccess_t> &starts = (sc2 ? m_streamData->m_sc21_packet_starts : m_streamData->m_sc1_packet_starts);
        if (starts.size())
            m_readPtr = m_streamData->getClientStartPosition (starts.back(), sc2);
        else
            m_readPtr = 0;
    }
    else
        m_readPtr = m_streamData->getClientStartPosition(sc2);
}


void protocol_shoutcastClient::resetReadPtr (streamData::ringBufferAccess_t ptr, bool sc2) throw()
{
	m_readPtr = (m_streamData ? m_streamData->getClientStartPosition (ptr, sc2) : 0);
    m_lagOffset = 0;
}


const __uint64 protocol_shoutcastClient::calculateFrameLimit(const time_t cur_time)
{
	// to allow for a burst on joining the stream for listener pre-buffering
	// and stuff, we're trying to give an ~5sec of data before we rate limit
	// without such handling then a number of listeners stutter at the start
	if (gOptions.rateLimit())
	{
		time_t conn;

		if (m_timerStart)
		{
			conn = (cur_time ? cur_time : ::time(NULL)) - m_timerStart;
			float fps = conn > 0 ? ((m_timerFrames) / (float)conn) : 0;
			//DLOG ("ad rate values are " + tos (conn) + " TF " + tos((long)m_timerFrames));
			if (fps > m_fps)
			{
				//DLOG ("rate in calc is " + tos (fps));
				return 0; // cause a stall
			}
		}
	}
	// just to make sure something is returned then we'll
	// take the current number of frames and adjust by the
	// intended rate. much better than returning zero rate
	return (__uint64)(m_frameCount + m_fps);
}

void protocol_shoutcastClient::updateFrameCount(const int frames, const time_t)
{
	if (m_timerStart)
		m_timerFrames += frames;
	m_frameCount += frames;
}

const bool protocol_shoutcastClient::calculateDelay(const int autoDumpTime)
{
	bool delayed = false;
	const double diff = ((m_frameCount * 1.0) / (max((time_t)1, (m_lastActivityTime - m_startTime))) - m_fps);
	if (diff >= 0.0)
	{
		// if we're at the limit then we're going to need to sit and spin
		// which will need to be a bit short of the required frame rate
		// so that we've got a bit of leeway on scheduling delays, etc
		// determine things based on the 'next' second when working out
		// the differential as that's how long we want to wait to run
		m_result.schedule((int)(m_result.m_currentTime - (__uint64)(m_lastActivityTime * 1000)));
		delayed = true;
	}

	m_result.write();
	if (autoDumpTime != -1)
	{
		m_result.timeout(autoDumpTime);
	}
	else
	{
		m_result.timeoutSID(m_streamID);
	}
	return delayed;
}


int protocol_shoutcastClient::doSend(const bool debug, const time_t cur_time,
        const int autoDumpTime, int adjust) throw(runtime_error)
{
	int l = (adjust ? adjust : (int)m_output.size()), rval = -1;
    if (l == 0) return 0;
	if (l > 0)
	{
		if (!gOptions.forceShortSends())
		{
			rval = send ((const char*)&m_output[0], l, 0);
		}
		else
		{
			static unsigned int v = 0;
			if (((++v) % 7) == 0)
			{
				char ch = 0;
				rval = ::recv(m_socket, &ch, 1, MSG_DONTWAIT | MSG_PEEK);
				l = 1;
			}
			else
			{
				rval = send ((const char*)&m_output[0], l, 0);
			}
		}
	}
    if (rval > 0)
    {
        m_totalBytesSent += rval;
        if (rval < l)
            m_result.timeout (0,100);
        else
            m_result.timeout (0,10);
        return rval;
    }

	if (rval == 0)
	{
        if (!m_ignoreDisconnect && debug)
            ELOG (m_clientLogString + "Remote socket closed while sending data.", LOGNAME, m_streamID);
        throwEx<runtime_error>("");
	}
	if (rval < 0)
	{
		rval = socketOps::errCode();
		if (rval != SOCKETOPS_WOULDBLOCK)
		{
            if (debug && (
#ifdef _WIN32
                        (rval == WSAECONNABORTED) || (rval == WSAECONNRESET)
#else
                        (rval == ECONNABORTED) || (rval == ECONNRESET) || (rval == EPIPE)
#endif
                        ))
                DLOG (m_clientLogString + "Socket error while waiting to send data. " + socketErrString(rval), LOGNAME, m_streamID);
                throwEx<runtime_error>("");
		}

		m_result.schedule();
		m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));

		rval = 0;
	}
	return rval;
}

void protocol_shoutcastClient::checkListenerIsValid(const bool debug) throw(std::runtime_error)
{
	char ch = 0;
	int rval = ::recv(m_socket, &ch, 1, MSG_DONTWAIT | MSG_PEEK);
	if (rval == 0)
	{
		throwEx<runtime_error>((!m_ignoreDisconnect && debug ? (m_clientLogString +
							   "Remote socket closed while sending data.") : (utf8)""));
	}
	if (rval < 0)
	{
		rval = socketOps::errCode();
		if (rval != SOCKETOPS_WOULDBLOCK)
		{
			throwEx<runtime_error>((debug ? (((
									#ifdef _WIN32
									(rval == WSAECONNABORTED) || (rval == WSAECONNRESET)
									#else
									(rval == ECONNABORTED) || (rval == ECONNRESET) || (rval == EPIPE)
									#endif
									) ? (utf8)"" : m_clientLogString +
									"Socket error while waiting to send data. " +
									socketErrString(rval))) : (utf8)""));
		}
	}
}

const utf8 protocol_shoutcastClient::getContainer() const
{
	return ((m_clientType & streamData::FLV) ? "video/x-flv" :
		   ((m_clientType & streamData::M4A) ? "audio/mp4" :
		   ((m_clientType & streamData::SHOUTCAST2) ? "misc/ultravox" : "")));
}

void protocol_shoutcastClient::authForStream(streamData *_sd)
{
	streamData *sd = (_sd ? _sd : streamData::accessStream(m_streamID));
	if (sd)
	{
		if (!sd->radionomyID().empty() && sd->streamAdvertMode())
		{
			httpHeaderMap_t vars;
			auth::auth_info *info = new auth::auth_info();
            const streamData::streamInfo &stream = sd->getInfo();

			vars["action"] = "listener_add";
			vars["tstamp"] = tos(::time(NULL));
			vars["host"] = sd->streamPublicIP();
			vars["radionomyid"] = vars["ref"] = sd->radionomyID();
			vars["id"] = tos(m_unique);
			vars["ip"] = m_clientAddr;
			vars["srvid"] = stream.m_serverID;
            if (m_queryParams.empty())
                vars["mount"] = getStreamPath (m_streamID);
            else
                vars["mount"] = getStreamPath (m_streamID) + "?" + m_queryParams;

			vars["agent"] = m_userAgent;
			vars["referer"] = m_referer;
			vars["bitrate"] = tos(sd->streamBitrate());
			vars["codec"] = sd->streamContentType();
			vars["contr"] = getContainer();
            info->m_dataType = sd->streamUvoxDataType();
			if (!_sd)
			{
				sd->releaseStream();
			}

			info->post = encodeVariables(vars);
			info->client = this;
			info->sid = m_streamID;

			if (_sd)
			{
				info->delayed_auth = true;
				info->group = 0;
			}
			auth::schedule(info);
			return;
		}

		if (!_sd)
		{
			sd->releaseStream();
		}
	}

	if (!_sd)
	{
		// if we need to auth after the listener
		// joined the stream due to DNAS / stream
		// start-up then we don't want to re-add
		threadedRunner::scheduleRunnable(this);
	}
}

void protocol_shoutcastClient::return_403()
{
	m_outBuffer = (const utf8::value_type*)"HTTP/1.1 403 Forbidden\r\n"
										   "Connection:close\r\n\r\n";
	m_outBufferSize = 44;
	m_lastActivityTime = ::time(NULL);
}

// create W3C entry. Entries describe the duration a client has listened to a specific title.
// the entry is generated on a title change, or when the client disconnects
void protocol_shoutcastClient::doLogW3C(const utf8 &title) throw()
{
	if (gOptions.w3cEnable())
	{
		time_t durationOfTitle = (::time(NULL) - m_lastTitleTime);
		w3cLog::log(m_streamID, m_clientAddr, m_clientHostName, title,
					m_userAgent, m_bytesSentForCurrentTitle, durationOfTitle,
					(int)(durationOfTitle ? (8 * m_bytesSentForCurrentTitle) / durationOfTitle : 0));

		setW3CState();
	}
}

#if 0
const bool protocol_shoutcastClient::handleNoData(const utf8 &logString, const int remainder,
												  const streamData::ringBufferAccess_t amt,
												  const int autoDumpTime, const time_t cur_time,
												  const bool sc2)
{
	// nothing in the source
	// if the source has gone away, and we have a backup file, send it.
	bool sendBackupFile = false, ret = false;
	if (!streamData::isSourceConnected(m_streamID))
	{
		const int backuploop = gOptions.getBackupLoop(m_streamID);
		if (!backuploop || (m_backupLoopTries <= backuploop))
		{
			acquireBackupFile(0, sc2);
		}
		else if (backuploop && (m_backupLoopTries < backuploop))
		{
			m_backupFile.clear();
		}
		sendBackupFile = !m_backupFile.empty();

		if (sendBackupFile && backuploop && (m_backupLoopTries > backuploop))
		{
			// clear trying to play the backup
			sendBackupFile = false;
			m_backupFile.clear();
		}
	}
	else
	{
		// if we're here then we'll have tried to do something
		// but are very likely are not able to due to being at
		// the frame rate limit or we're not able to send yet
		// but we need to move the readPtr on if anything was
		// read was well as then making sure that we maintain
		// the remainder otherwise we can gltich on playback.
		if (amt > 0)
		{
			m_readPtr += amt;
		}

		if (!remainder)
		{
			m_remainder.clear();
		}

		// if we're starting and we're not able to correctly
		// sync to the stream then this will abort otherwise
		// we can potentially sit and spin which isn't good.
		if (!m_frameCount && (remainder > 0) && (remainder == (int)amt))
		{
			m_ignoreDisconnect = true;
			ELOG(m_clientLogString + logString + " client connection closed. Unable to sync to the stream (" +
				 tos(::time(NULL) - m_startTime).c_str() + " seconds)" + " Agent: `" + m_userAgent + "', UID: " +
				 tos(m_unique) + ", GRID: " + tos(getGroup()) + " [amt=" + tos(amt) + ", rem=" + tos(remainder) + "]");
			m_result.done();	// state_Close
			m_result.write();
			m_result.timeout(autoDumpTime);
		}
	}

	if (sendBackupFile)
	{
		// we also reset the remainder handling as
		// there is no need to send the remainder.
		m_remainder.clear();
		// this is to clear up things with the stream
		// as we can otherwise be in a weird state if
		// the source then later returns and we have
		// a listener still present via backup mode.
		if (m_streamData)
		{
			m_streamData->releaseStream();
			m_streamData = 0;
		}
		++m_backupLoopTries;
		ret = true;	// state_SendBackupFile
		m_result.run();
	}
	else
	{
		m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
		m_result.read(m_limitTrigger.test());
		if (m_streamData)
		{
			m_streamData->scheduleLimitTrigger(&m_limitTrigger, m_readPtr, sc2);
		}
		else
		{
			m_result.done();	// state_Close
			m_result.write();
			m_result.timeout(autoDumpTime);
		}
	}

	return ret;
}
#endif

void protocol_shoutcastClient::createFrameRate(const bool mp3, const int samplerate)
{
	// now we have some stream data we can create
	// a fps for this which we'll assume standard
	// rates if not all of the info is available.
	// this is mainly for AAC where we assume a
	// 44.1kHz samplerate (~43.06 fps) as default
	if (!m_fps)
	{
		m_fps = (mp3 ? 1000.0 / 26 : ((1.0 / (1024.0 / (samplerate > 0 ? samplerate : 44100)))));
	}
}

void protocol_shoutcastClient::setupWorkingBuffer(const std::vector<__uint8>& data, std::vector<__uint8>& tempBuf, int& len) throw()
{
	// if we had anything left over then now we
	// need to copy it back into the buffer and
	// adjust the max data amount to be read in
	if (!m_remainder.empty() && ((len + m_remainder.size()) <= (BUF_SIZE * 4)))
	{
		tempBuf = m_remainder;
		tempBuf.insert(tempBuf.end(), data.begin(), data.end());
		len += (int)m_remainder.size();
	}
	else if (len > 0)
	{
		tempBuf = data;
	}
	m_remainder.clear();
}

#if 0
const int protocol_shoutcastClient::doUvoxFrameSync(const int type, const bool debug, const int len,
													const int offset, const std::vector<__uint8>& buf,
													const time_t cur_time, int &frames, bool &advert,
													const bool fill_remainder) throw()
{
	bool mp3;
	if (streamData::isAllowedType(type, mp3))
	{
		if (!m_shortSend.empty())
		{
			m_output.insert(m_output.end(), m_shortSend.begin(), m_shortSend.end());
			m_shortSend.clear();

			if (!fill_remainder)
			{
				// for intro / backup / advert then it's
				// better to just send what was left and
				// then look at processing new when this
				// is empty else it'll duplicate output.
				return 0;
			}
		}

		int end = 0;
		if ((len > 8) && !buf.empty())
		{
			int last_size = 0;
			for (int i = 0; (i < (len - 8)) && !iskilled();)
			{
				const uv2xHdr *voxHdr = (const uv2xHdr*)(&(buf[i + offset]));
				const int msgLen = ntohs(voxHdr->msgLen);
				const int found = ((voxHdr->sync == UVOX2_SYNC_BYTE) &&
								   (msgLen > UV2X_OVERHEAD) &&
								   (msgLen <= MAX_PAYLOAD_SIZE) ? (msgLen + UV2X_OVERHEAD) : 0);

				// need to find frames and that the input is the correct format!
				//
				// is a bit of a pain for AAC though as we've already done the
				// rough bitrate match when the advert / intro / backup was read
				// we'll just pass things through as though the bitrate is ok...
				if ((found > 0) && (found <= len))
				{
					if (!frames)
					{
						end = (i + offset);
						protocol_shoutcastClient::createFrameRate(mp3, (m_streamData ? m_streamData->streamSampleRate() : 0));
					}

					i += (last_size = found);

					// only count valid full-size frames
					if (i <= len)
					{
						if (advert)
						{
							// skip over the advert trigger
							end += 8;
						}

						const std::vector<__uint8>::const_iterator pos = buf.begin();
						m_output.insert(m_output.end(), pos + end, pos + (end + last_size));
						end += last_size;

						// we only want to count audio frames and not metadata
						const __uint16 voxMsgType = ntohs(voxHdr->msgType);
						if ((voxMsgType >= 0x7000) && (voxMsgType < 0x9000))
						{
							++frames;
						}

						// we use this to help clamp to the desired frame
						// rate as well as not sending anything else asap
						if (advert || (gOptions.rateLimit() ? (frames > m_fps) ||
									   ((m_frameCount + frames) > calculateFrameLimit(cur_time)) : 0))
						{
							break;
						}
					}
				}
				else
				{
					// we now look for the "SCAdvert" marker
					// for detecting if we've got an advert.
					if ((i <= len + 8) && ((buf[i + offset]) == 'S') && ((buf[i + offset + 1]) == 'C'))
					{
						if (!memcmp(&buf[i + offset], "SCAdvert", 8))
						{
							advert = true;
							i += 8;
						}
						else
						{
							++i;
						}
					}
					else
					{
						// if we found something but there is not enough
						// data in the read buffer then we'll abort asap
						if ((found > 0) && (found > len))
						{
							break;
						}

						if (m_frameCount && debug)
						{
							DLOG(m_clientLogString + "Bad frame found at pos: " +
								 tos(i) + " [" + tos(found) + "]");
						}

						// otherwise we just need to move on and keep
						// looking for what is a valid starting frame
						++i;
					}
				}
			}
		}

		if (fill_remainder && !buf.empty())
		{
			const int remainder = (len - end);
			if (remainder > 0)
			{
				const std::vector<__uint8>::const_iterator pos = buf.begin();
				m_remainder.insert(m_remainder.end(), pos + end, pos + (end + remainder));
			}
			return remainder;
		}
	}
	return 0;
}
#endif


const int protocol_shoutcastClient::doFrameSync(const int type, const bool debug, const int len,
		const int offset, const std::vector<__uint8>& inbuf,
		const time_t /*cur_time*/, const int bitrate,
		const unsigned int samplerate, int &frames,
		bool &advert, bool fill_remainder) throw()
{
	bool mp3;
	int end = 0;
	bool VBR = m_streamData ? m_streamData->streamIsVBR() : false;

	if (streamData::isAllowedType(type, mp3))
	{
		int last = min (len, (int)(inbuf.size() - offset));
		const unsigned char *buf;

		if (m_remainder.empty () || offset < last)
		{
			buf = &inbuf [offset];
			//DLOG ("last " + tos(last) + ", off " + tos(offset));
		}
		else
		{
			// const std::vector<__uint8>::const_iterator pos = inbuf.begin();
			size_t cc = min (inbuf.size(), (size_t)len);

			cc = min (cc, (size_t)4096);
			if (cc > m_remainder.size())
				cc -= m_remainder.size();
			if (cc < 1 || cc > inbuf.size())
			{
			    ILOG ("sync, cc is " + tos(cc));
			    abort();
			}
			m_remainder.insert (m_remainder.end(), inbuf.begin(), inbuf.begin() + cc);

			buf = &m_remainder [0];
			last = min (len, (int)m_remainder.size());
			fill_remainder = false;
			// DLOG ("merged remainder, len " + tos(len) + " now " + tos (last) + ", added " + tos(cc));
		}

		if (last > 8)
		{
			int last_size = 0;
            bool report_frame_settings = m_streamData ? true : false;

			fill_remainder = false;

			for (int i = 0; (i < last-8) && !iskilled();)
			{
				unsigned int read_samplerate = 0;
				int read_bitrate = bitrate;
				//const char* f = (const char *)&(buf[(i + offset)]);
				const int found = (mp3 ? getMP3FrameInfo ((char*)buf+i, &read_samplerate, &read_bitrate) :
						getADTSFrameInfo ((char*)buf+i, &read_samplerate));

                if (found > len)   // do we need more
                    break;
                if (found > 0)
                {
                    bool bitrate_odd = (VBR || (read_bitrate == 0 || read_bitrate == bitrate)) ? false : true;
                    bool srate_odd = (samplerate == 0 || samplerate == read_samplerate) ? false : true;

                    // need to find frames and that the input is the correct format!
                    //
                    // is a bit of a pain for AAC though as we've already done the
                    // rough bitrate match when the advert / intro / backup was read
                    // we'll just pass things through as though the bitrate is ok...
                    if (report_frame_settings && (bitrate_odd || srate_odd))
                    {
                        utf8 msg = m_clientLogString + "Frame settings changed, could cause playback problems:";
                        if (bitrate_odd)
                        {
                            msg += " bitrate " + tos(read_bitrate) + "(" + tos(bitrate) + ")";
                            m_streamData->m_streamInfo.m_streamBitrate = read_bitrate;
                        }
                        if (srate_odd)
                        {
                            msg += " samplerate " + tos(read_samplerate) + "(" + tos(samplerate) + ")";
                        }
                        WLOG (msg, LOGNAME, m_streamID);
                        protocol_shoutcastClient::createFrameRate(mp3, read_samplerate);
                        report_frame_settings = false;
                    }
                    if (!frames)
                    {
                        end = i;
                        protocol_shoutcastClient::createFrameRate(mp3, read_samplerate);
                    }

                    i += (last_size = found);

                    // only count valid full-size frames
                    if (i <= last)
                    {
                        processFrame (type, buf + end, last_size);
                        ++frames;
                        end += last_size;
                        if (m_output.size() > SEND_SIZE)
                            break;
                    }
                    else
                    {
                        // DLOG ("EOB, " + tos (i) + ", " + tos(offset) + ", " + tos(inbuf.size()));
                        if (m_remainder.empty())
                        {
                            if (i+offset > inbuf.size())
                                fill_remainder = true;
                        }
                        else if (end + offset < inbuf.size())
                            fill_remainder = true;
                        break;
                    }
                    continue;
                }
                // we now look for the "SCAdvert" marker
                // for detecting if we've got an advert.
                if (i < (len - 8) && ((buf[i]) == 'S') && ((buf[i+1]) == 'C'))
                {
                    if (!memcmp(&buf[i], "SCAdvert", 8))
                    {
                        if (frames == 0)
                        {
                            // DLOG ("Found SCAdvert");
                            advert = true;
                            end += 8;
                            m_remainder.clear();
                        }
                        break;
                    }
                }
                if (m_frameCount && debug)
                {
                    DLOG(m_clientLogString + "Bad frame found at pos: " +
                            tos(i) + " [" + tos(found) + "], " +
                            tos(read_bitrate) + "(" + tos(bitrate) + "), " +
                            tos(read_samplerate) + "(" + tos(samplerate) + ")", LOGNAME, m_streamID);
                }
                // looking for what is a valid starting frame
                ++i;
			}
		}
		else
			fill_remainder = true;

        if (m_remainder.empty() == false)
        {
            if (last < m_remainder.size())
                fill_remainder = false;
            if (frames)
                m_remainder.clear();
        }
        if (last < (inbuf.size() - offset))
            fill_remainder = false;

		if (fill_remainder)
		{
			const int remainder = (last - end);
			if (remainder > 0)
				m_remainder.insert(m_remainder.end(), buf + end, buf + (end + remainder));
		}
	}
	return (len - end);
}


void protocol_shoutcastClient::processFrame (int, const unsigned char *buf, unsigned int len)
{
	//#define USE_CHUNKED
#ifdef USE_CHUNKED
	const utf8 chunked_start (tohex(len) + "\r\n"), chunked_end("\r\n");
	m_output->insert (m_output->end(), chunked_start.begin(), chunked_start.end());
#endif
	//const std::vector<__uint8>::const_iterator pos = buf.begin();
	m_output.insert (m_output.end(), buf, buf + len);
#ifdef USE_CHUNKED
	m_output.insert (m_output.end(), chunked_end.begin(), chunked_end.end());
#endif
}


const int protocol_shoutcastClient::addClient()
{
    size_t stream_id = (gOptions.read_stream_ripFile(m_streamID) && !gOptions.stream_ripFile(m_streamID).empty()) ? m_streamID : 0;

    return stats::addClient (m_streamID, m_clientHostName, m_clientAddr, m_userAgent,
            g_ripList.find (m_clientAddr, stream_id), m_unique, this);
}


void protocol_shoutcastClient::reportNewListener(const utf8 &logString)
{
	if (!logString.empty() && gOptions.logClients())
	{
		if (m_removeClientFromStats)
		{
			// setGroup (500);
			// ILOG(m_clientLogString + logString + " client connection accepted. User-Agent: `" +
				 // m_userAgent + "', UID: " + tos(m_unique) + ", GRID: " + tos(getGroup()), LOGNAME, m_streamID);
			utf8 s = m_clientLogString + logString + " client connection accepted. User-Agent: `" +
				 m_userAgent + "', UID: " + tos(m_unique) + ", GRID: " + tos(getGroup());
            ILOG (s, LOGNAME, m_streamID);
		}
		else
		{
			m_ignoreDisconnect = true;
		}
	}
}

void protocol_shoutcastClient::reportStopListener()
{
	if (m_removeClientFromStats && m_streamData && m_streamData->streamPublic())
	{
		metrics::metrics_listener_drop(*this);
	}
}

const bool protocol_shoutcastClient::processAdvertTrigger(const bool advert)
{
	if (advert && m_streamData)
	{
		// check if we've got an advert trigger and if the stream is either
		// public or it is private but the DNAS is running in any CDN mode.
		if ((m_streamData->m_streamInfo.m_streamPublic ||
			(!gOptions.cdn().empty())) &&
			// if a CDN slave then we're ok to send the stream but we don't
			// want to be sending the adverts to it as those should instead
			// be actioned on the slave's ->listener handling and not here.
			!isCDNSlave(m_streamID))
		{
			// DLOG ("in trigger with g " + tos(m_group));
			if ((getGroup() > 0) || m_streamData->m_adTest)
			{
				if (m_adAccess.haveAdverts(m_streamID, m_streamData->advertGroups, m_readPtr))
				{
					time_t now = ::time(NULL);
					ILOG(m_clientLogString + "Transitioning to advert(s) [Agent: `" +
						 m_userAgent + "', UID: " + tos(m_unique) + ", GRID: " + tos(getGroup()) + "]", LOGNAME, m_streamID);
					m_lastActivityTime = now;
					stats::updateTriggers(m_streamID, m_unique);
					m_adAccess.total_processed = 0;
					m_adAccess.start_time = now;
					m_remainder.clear();
					m_result.schedule(60);
                    m_result.timeoutSID(m_streamID);
					return true;
				}
				else
				{
					AD_DEBUG_LOG("[ADVERT sid=" + tos(m_streamID) + "] Advert trigger detected - no advert(s) available "
								 "[Agent: `" + m_userAgent + "', UID: " + tos(m_unique) + ", Group: " + tos(getGroup()) + "]", LOGNAME, m_streamID);
				}
			}
		}
	}
	return false;
}

const utf8 protocol_shoutcastClient::fixICYMetadata(utf8 metadata)
{
	// we use this to provide a nicer title to clients when the advert update occurs
	// for detected relays, we will skip over the filtering so it will be relayed on
	if (!metadata.empty() && !(((streamData::source_t)(m_clientType & streamData::RELAY))) &&
		!metadata.find(utf8("StreamTitle='Advert:")) && !metadata.find(utf8("StreamTitle='Advert!")))
	{
		// got a first matching block
		metadata = metadata.replace(13, 7, (utf8)"");

		// look for an end block
		utf8::size_type pos = metadata.find(utf8("Advert:"));
		if (pos != utf8::npos)
		{
			metadata = metadata.replace(pos, 7, (utf8)"");
		}
		metadata = stripWhitespace(metadata);
	}
	return metadata;
}

// create W3C entry. Entries describe the duration a client has listened to a specific title.
// the entry is generated on a title change, or when the client disconnects
void protocol_shoutcastClient::logW3C() throw()
{
	utf8 title;
	if (!m_lastSentMetadata.empty())
	{
		utf8::size_type p1 = m_lastSentMetadata.find(utf8("itle='"));
		if (p1 != utf8::npos)
		{
			p1 += 6;
			utf8::size_type p2 = m_lastSentMetadata.find(utf8("';"),p1);
			if (p2 != utf8::npos)
			{
				title = m_lastSentMetadata.substr(p1, p2 - p1);
			}
		}
	}
	doLogW3C(title);
}

void protocol_shoutcastClient::processTitleW3C() throw()
{
	if (gOptions.w3cEnable()) // put inside this if block, because this can slow us down a bit
	{
		if (m_metaIntervalCounter >= m_metaInterval)
		{
			m_metaIntervalCounter = 0;
			const utf8 metadata = fixICYMetadata((m_streamData ? m_streamData->getSc1Metadata(m_readPtr).m_songTitle : ""));
			if (m_lastSentMetadata != metadata)
			{
				logW3C();
				m_lastSentMetadata = metadata;
			}
		}
	}
}

void protocol_shoutcastClient::streamMovedOrRejected(const utf8 &logString, const bandWidth::usageType_t type,
													 const utf8 &serverUrl, const int mode) throw()
{
	// if we get to here then we attempt to redirect the clients to the moved url
	// which is useful if the stream has moved hosting or it has been deprecated.
	if (!mode)
	{
		m_outBuffer = MSG_HTTP503;
		bandWidth::updateAmount(type, (m_outBufferSize = MSG_HTTP503_LEN));
	}
	else
	{
		const utf8::size_type check = (!serverUrl.empty() ? serverUrl.find(utf8("://")) : 0);
		if (check == utf8::npos)
		{
			m_OKResponse = http302("http://" + serverUrl);
		}
		else
		{
			m_OKResponse = http302(serverUrl);
		}

		m_OKResponse = http302(serverUrl);
		m_outBuffer = m_OKResponse.c_str();
		bandWidth::updateAmount(type, (m_outBufferSize = (int)m_OKResponse.size()));
	}
	m_ignoreDisconnect = true;

	if (gOptions.logClients())
	{
		switch (mode)
		{
			case 0:
			{
				ELOG(m_clientLogString + logString + " client connection "
					 "rejected. Max users reached. Agent: `" + m_userAgent + "'", LOGNAME, m_streamID);
				break;
			}
			case 1:
			{
				WLOG(m_clientLogString + logString + " client connection rejected. Max users "
					 "reached. Redirecting to " + serverUrl + ". Agent: `" + m_userAgent + "'", LOGNAME, m_streamID);
				break;
			}
			case 2:
			{
				WLOG(m_clientLogString + logString + " client connection rejected. Stream has "
					 "moved. Redirecting to " + serverUrl + ". Agent: `" + m_userAgent + "'", LOGNAME, m_streamID);
				break;
			}
		}
	}

	m_result.run();
	m_result.write();
	m_result.timeoutSID(m_streamID);
}

void protocol_shoutcastClient::cleanup(const utf8 &logString, const bool debug, const bool , const bool altLog) throw(exception)
{
	if (debug)
	{
		DLOG(m_clientLogString + logString + " " + __FUNCTION__, LOGNAME, m_streamID);
	}

	try
	{
		if (gOptions.logClients() && !m_ignoreDisconnect)
		{
			ILOG(m_clientLogString + logString + " client connection closed (" +
				 tos(::time(NULL) - m_startTime).c_str() + " seconds)" + " [Bytes: " +
				 tos(m_totalBytesSent).c_str() + "] Agent: `" + m_userAgent +
				 "', UID: " + tos(m_unique) + ", GRID: " + tos(getGroup()), LOGNAME, m_streamID);
		}

		if (m_removeClientFromStats)
		{
			stats::removeClient(m_streamID, m_unique);
			// create W3C entry. Entries describe the duration a client has listened to a specific title.
			// the entry is generated on a title change, or when the client disconnects
			if (!altLog)
			{
				logW3C();
			}
			else
			{
				doLogW3C();
			}
		}

		if (m_streamData)
		{
			releaseAdvert();
			m_streamData->advertGroups.detachGroupQueue (m_adAccess);

			reportStopListener();

			m_streamData->releaseStream();
			m_streamData = 0;
		}
	}
	catch(const exception &ex) 
	{
		ELOG(ex.what());
	}
}

const int protocol_shoutcastClient::processAdd(const utf8 &logString, const bandWidth::usageType_t type,
											   const utf8 &msg, const int msgLen, const utf8 &movedUrl,
											   const utf8 &serverUrl) throw()
{
	// if we've got a HEAD request then we need to just force things through
	const int add = (!m_headRequest ? addClient() : 1);
	if (add != 1)
	{
		if (!add)
		{
			utf8 backup_server = (m_streamData ? m_streamData->streamBackupServer() : "");
			if (backup_server.empty())
			{
				streamMovedOrRejected(logString, type, movedUrl, 0);
			}
			else
			{
				streamMovedOrRejected(logString, type, serverUrl, 1);
			}
		}
		else
		{
			m_outBuffer = msg.c_str();
			bandWidth::updateAmount(type, (m_outBufferSize = msgLen));

			m_result.write();
			m_result.timeoutSID(m_streamID);
			m_ignoreDisconnect = true;

			if (gOptions.logClients())
			{
				const bool missing = (add == -1);
				ELOG(m_clientLogString + logString + " client connection rejected. "
					 "Client connection " + (missing ? "missing a" : "from a banned") +
					 " user-agent" + (missing ? "." : ": `" + m_userAgent + "'"), LOGNAME, m_streamID);
			}
		}
	}
	return add;
}

const bool protocol_shoutcastClient::processReject(const utf8 &logString, const bandWidth::usageType_t type,
												   const utf8 &msg, const int msgLen, int *read_bitrate,
												   int *dataType, const bool sc2) throw()
{
	m_outBuffer = msg.c_str();
	bandWidth::updateAmount(type, (m_outBufferSize = msgLen));

	m_result.write();
	m_result.timeoutSID(m_streamID);
	m_ignoreDisconnect = true;
	if (gOptions.logClients())
	{
		// to prevent some of the stats / logs getting skewed
		// then we filter out the SHOUTcast site 'test' users
		if (m_removeClientFromStats)
		{
			const utf8 movedUrl = gOptions.stream_movedUrl(m_streamID);
			if (movedUrl.empty())
			{
				*read_bitrate = acquireBackupFile(dataType, sc2);
				if (!m_backupFile.empty())
				{
					m_ignoreDisconnect = false;
					return true;//goto fall_through;
				}
				else
				{
					ELOG(m_clientLogString + logString + " client connection rejected. " +
						 (!iskilled() ? "Stream not available as there is no source connected" :
						 "Server is shutting down") + ". Agent: `" + m_userAgent + "'", LOGNAME, m_streamID);
				}
			}
			else
			{
				// if we get to here then we attempt to redirect the clients to the moved url
				// which is useful if the stream has moved hosting or it has been deprecated.
				streamMovedOrRejected(logString, type, movedUrl, 2);
			}
		}
	}
	return false;
}


void protocol_shoutcastClient::state_SendIntroFile() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
    // DLOG(m_clientLogString + __FUNCTION__);
#endif
    bool sc2 = (m_clientType & streamData::SHOUTCAST2) ? true : false;

    resetCommon();

    time_t cur_time;
    const bool debug = gOptions.HTTPClientDebug();
    const int autoDumpTime = detectAutoDumpTimeout (cur_time, m_streamID, (m_clientLogString + "Timeout waiting to send data"));

    if (m_timerStart == 0)
    {
        m_timerStart = cur_time - 10; // prime the rate regulation
        m_timerFrames = (__uint64)m_fps * 10;
        time_t conn = ::time(NULL) - m_startTime;
        if (conn >= 0 && conn < 3)
           m_timerStart -= (3 - conn);
    }
    if (m_frameCount > calculateFrameLimit(cur_time))
    {
        m_result.schedule(150);
        m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
        //DLOG ("Limited at " + tos(m_frameCount));

        return;
    }
    int amt = (int)(m_introFile.size() - m_introFileOffset);
    if (amt == 0)
    {
        // we're done with the intro file
        m_introFile.clear();
        m_introFile.resize(0);
        m_introFileOffset = 0;
        m_lastActivityTime = ::time(NULL);
        setCallback (&protocol_shoutcastClient::state_Stream);
        resetReadPtr (sc2);
        m_result.run();
    }
    else if (amt > 0)
    {
        checkListenerIsValid(debug);

        if (!m_lastSentMetadata.empty())
        {
            logW3C();
            m_lastSentMetadata.clear();
        }

        amt = min(amt, SEND_SIZE);

        int len = (int)amt,
            frames = 0; // this will be uvox frames as we pre-process earlier to ensure
                        // that the audio frames are frame-synced when this is created
        bool advert = false;
        if (m_output.size() < 2000)
        {
            int rval = doFrameSync(m_streamData->streamUvoxDataType(), debug,
                    len, m_introFileOffset, m_introFile,
                    cur_time, m_streamData->streamBitrate(),
                    m_streamData->streamSampleRate(), frames, advert);
            m_introFileOffset += (len - rval);
            //DLOG ("offset now " + tos (m_introFileOffset));
            updateFrameCount(frames, cur_time);
        }

        int rval = doSend(debug, cur_time, autoDumpTime);
        if (rval > 0)
        {
            bandWidth::updateAmount(bandWidth::CLIENT_HTTP_SENT, rval);
            m_bytesSentForCurrentTitle += rval;
            m_lastActivityTime = ::time(NULL);
            if (rval < m_output.size())
            {
                //DLOG ("short send " + tos(rval) + "/" + tos(m_output.size()));
                m_output.erase (m_output.begin(), m_output.begin() + rval);
                m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
                m_result.write();
                return;
            }
            m_output.clear();
        }
        m_result.schedule(20);
        m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
    }
}


void protocol_shoutcastClient::state_SendIntro() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
    // AD_DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif
    state_SendIntroFile();
    if (m_introFile.empty())
    {
        acquireIntroFile();
        if (m_introFile.empty())
        {
            setCallback (&protocol_shoutcastClient::state_Stream);
        }
        else
        {
            setCallback (&protocol_shoutcastClient::state_SendIntroFile);
        }
    }
}


void protocol_shoutcastClient::state_Stream() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
    //DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif

    resetCommon();

    const time_t cur_time = ::time(NULL);
    const bool debug = gOptions.shoutcast1ClientDebug();
    const int autoDumpTime = gOptions.getAutoDumpTime(m_streamID); // don't want this value to change during this call
    int remain = autoDumpTime - (int)(cur_time - m_lastActivityTime);

    if ((autoDumpTime > 0) && ((cur_time - m_lastActivityTime) >= autoDumpTime))
    {
        throwEx<runtime_error>((!m_ignoreDisconnect && debug ?
                                (m_clientLogString + "Timeout waiting to send data (" +
                                 tos(cur_time) + " " + tos(m_lastActivityTime) + " [" +
                                 tos(cur_time - m_lastActivityTime) + "] )") : (utf8)""));
    }

    bool advert = false;
    bool sc2 = (m_clientType & streamData::SHOUTCAST2) ? true : false;
    AOL_namespace::rwLock &lock = (sc2 ? m_streamData->m_sc21StreamLock : m_streamData->m_sc1StreamLock);

    do
    {
        int samplerate = (m_streamData ? m_streamData->streamSampleRate() : 0);
        int bitrate = (m_streamData ? m_streamData->streamBitrate() : 0);
        int type = (m_streamData ? m_streamData->streamUvoxDataType() : MP3_DATA);

        m_result.timeout (remain);
        if (streamData::isSourceConnected (m_streamID) == false)
        {
            acquireBackupFile (0, sc2);
            //DLOG ("listener detected source drop, backup " + utf8(m_backupFile.size() ? "present" : "not present"));
            if (m_backupFile.size())
            {
                setCallback (&protocol_shoutcastClient::state_SendBackupFile);
                m_timerStart = 0;
                m_backupLoopTries = 1;
                m_result.run();
                return;
            }
            if (m_streamData == NULL)
            {
                m_result.schedule (300);
                return;
            }
        }

        stackRWLock sl (lock, false, false);
        if (sl.tryRdLock() == false)
        {
                m_result.schedule (3);
                return;
        }
        int frames = 0;

        if (m_output.size() < 2000)
        {
            streamData::ringBufferAccess_t lag = ((sc2 ? m_streamData->m_sc21_ring_buffer : m_streamData->m_sc1_ring_buffer).m_writePtr - m_readPtr);

            if (lag == 0)
            {
                if (m_frameCount > calculateFrameLimit(cur_time))
                {
                    m_result.schedule(333);
                }
                else
                {
                    m_result.schedule(150);
                }
                return;
            }

            const streamData::ringBufferAccess_t offset = (m_readPtr & (sc2 ? m_streamData->m_sc21_ring_buffer : m_streamData->m_sc1_ring_buffer).m_ptrMask);

            if (lag > (sc2 ? m_streamData->m_sc21_ring_buffer : m_streamData->m_sc1_ring_buffer).m_data.size())
                break; // off the queue, lagging too much

            lag -= m_lagOffset;

            size_t remainder = doFrameSync (type, debug, (int)lag, (int)offset,
                    (sc2 ? m_streamData->m_sc21_ring_buffer : m_streamData->m_sc1_ring_buffer).m_data,
                    cur_time, bitrate, samplerate, frames, advert, true);

            m_readPtr += (lag-remainder);
            updateFrameCount (frames, cur_time);
        }
        // processTitleW3C();    needs looking into 

        // lets jump out if there isn't much to send except in cases where an advert, as we need to
        // process the transistion
        if (m_output.size() < 1000 && (frames || advert == false))
        {
            //DLOG ("not enough to send really");
            m_result.schedule(150);
            return;
        }
        int rval = doSend (debug, cur_time, autoDumpTime);
        if (rval > 0)
        {
            bandWidth::updateAmount(bandWidth::CLIENT_V1_SENT, rval);
            m_bytesSentForCurrentTitle += rval;
            m_lastActivityTime = ::time(NULL);

            if (rval < m_output.size())
            {
                // DLOG ("short send " + tos(rval) + "/" + tos(m_output.size()));
                m_output.erase (m_output.begin(), m_output.begin() + rval);
                // if we short send and don't need to transition to advert then jump out
                if (advert == false)
                {
                    m_result.schedule(80); // back off on sending too much
                    return;
                }
            }
            else
                m_output.clear();
        }
        if (processAdvertTrigger(advert))
        {
            // DLOG ("advert detected by listener");
            setCallback (&protocol_shoutcastClient::state_SendAdverts);
            m_result.schedule(200);
        }
        else
            m_result.schedule(10);
        return;
    } while (0);

    m_result.schedule();
    resetReadPtr (sc2);
}



// handle state where we are sending backup files
void protocol_shoutcastClient::state_SendBackupFile() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
    // DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif
    bool sc2 = (m_clientType & streamData::SHOUTCAST2) ? true : false;

    resetCommon();

    int amt = (int)(m_backupFile.size() - m_backupFileOffset);

    if (amt == 0)
    {
        const int backuploop = gOptions.getBackupLoop(m_streamID);
        if (!backuploop || (m_backupLoopTries <= backuploop))
        {
            // we're done with the backup file. get more data
            acquireBackupFile (0, sc2);
            if (m_backupFile.empty())
            {
                ++m_backupLoopTries;
                resetReadPtr (sc2);
                m_lastActivityTime = ::time(NULL);
                setCallback (&protocol_shoutcastClient::state_Stream);
                return;
            }
        }
        else if (backuploop && (m_backupLoopTries < backuploop))
        {
            m_backupFileOffset = 0;
            m_backupFile.clear();
        }
        else
        {
            resetReadPtr (sc2);
            m_backupFileOffset = 0;
            m_lastActivityTime = ::time(NULL);
            setCallback (&protocol_shoutcastClient::state_Stream);
        }
        m_result.run();
    }
    else if (amt > 0)
    {
        const bool debug = gOptions.HTTPClientDebug();
        checkListenerIsValid(debug);

        time_t cur_time = ::time(NULL);
        const int autoDumpTime = detectAutoDumpTimeout (cur_time, m_streamID, (m_clientLogString + "Timeout waiting to send data"));

        if (m_timerStart == 0)
        {
            int adj = m_frameCount < 50 ? 3 : 10;
            m_timerStart = cur_time - 10; // prime the rate regulation, make it on the slow side to rejoin queue
            m_timerFrames = (__uint64)m_fps * adj;
        }
        if (m_frameCount > calculateFrameLimit(cur_time))
        {
            m_result.schedule (333);
            m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
            return;
        }

        if (!m_lastSentMetadata.empty())
        {
            logW3C();
            m_lastSentMetadata.clear();
        }

        amt = min(amt, SEND_SIZE);
        int len = (int)amt, backup_type = MP3_DATA,
            frames = 0; // this will be uvox frames as we pre-process earlier to ensure
                        // that the audio frames are frame-synced when this is created
        if (!m_streamData)
        {
            utf8 backupFile = gOptions.stream_backupFile(m_streamID);
            if (!gOptions.read_stream_backupFile(m_streamID))
            {
                backupFile = gOptions.backupFile();
            }
            if (!backupFile.empty())
            {
                backup_type = ((backupFile.rfind((utf8)".aac") == utf8::npos) ? MP3_DATA : AACP_DATA);
            }
        }

        bool advert = false;
        if (m_output.size() < 4096)
        {
            int rval = doFrameSync((m_streamData ? m_streamData->streamUvoxDataType() : backup_type),
                    debug, len, m_backupFileOffset, m_backupFile, cur_time,
                    (m_streamData ? m_streamData->streamBitrate() : 0),
                    (m_streamData ? m_streamData->streamSampleRate() : 0), frames, advert);
            m_backupFileOffset += (len - rval);
            updateFrameCount (frames, cur_time);
        }
        int rval = doSend(debug, cur_time, autoDumpTime);

        m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
        if (rval > 0)
        {
            bandWidth::updateAmount(bandWidth::CLIENT_HTTP_SENT, rval);
            m_bytesSentForCurrentTitle += rval;
            m_lastActivityTime = cur_time;
            if (rval < m_output.size())
            {
                // DLOG ("short send " + tos(rval) + "/" + tos(m_output.size()));
                m_output.erase (m_output.begin(), m_output.begin() + rval);
                return;
            }
            m_output.clear();
            // look at moving now that a complete frame is sent
            if (streamData::isSourceConnected (m_streamID))
            {
                if (m_streamData)
                {
                    m_streamData->releaseStream();
                }
                m_streamData = streamData::accessStream(m_streamID);

                if (m_streamData->m_sc1_packet_starts.size() == 0)
                {
                    m_result.schedule (100);  // no data from stream, so ignore for now.
                    return;
                }

                // we're done with the backup file
                m_backupFile.clear();
                m_backupFile.resize(0);
                m_backupFileOffset = 0;
                m_backupLoopTries = 0;
                setCallback (&protocol_shoutcastClient::state_Stream);
                resetReadPtr (sc2);
                m_result.run();
                return;
            }
            m_result.schedule ((rval/512) + 15);
        }
        else
            m_result.schedule(90);
    }
}



// handle state where we are sending advert content
void protocol_shoutcastClient::state_SendAdverts() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
    //AD_DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif

    resetCommon();

    streamData::specialFileData *ad = m_adAccess.getAd(m_streamID, m_streamData->advertGroups, !!m_streamData->m_adTest);
    bool sc2 = (m_clientType & streamData::SHOUTCAST2) ? true : false;

    while (true)
    {
        if (ad)
        {
            std::vector<__uint8> &adBuffer = (sc2 ? ad->m_sc2Buffer : ad->m_sc1Buffer);
            int amt = (ad ? (int)(adBuffer.size() - m_adAccess.offset) : 0);

            if (amt > 0)
                break;
        }
        if (m_adAccess.anotherAd(m_streamID, m_streamData->advertGroups, !!m_streamData->m_adTest))
        {
            ad = m_adAccess.getAd (m_streamID, m_streamData->advertGroups, !!m_streamData->m_adTest);
            continue;
        }
        // no more adverts
        m_lastActivityTime = ::time(NULL);
        if (m_timerStart)
        {
            // add a small retry here to allow for queue priming after adverts
            m_result.schedule (300);
            m_result.timeoutSID(m_streamID);
            m_timerStart = 0;
            //DLOG ("kicking back to retry before transition");
            return;
        }
        setCallback (&protocol_shoutcastClient::state_Stream);
        resetReadPtr (sc2); // this should do a search for a nearest matching frame
        releaseAdvert();
        m_result.schedule ();
        m_result.timeoutSID(m_streamID);

        ILOG(m_clientLogString + "Transitioning back to stream [Agent: `" +
                m_userAgent + "', UID: " + tos(m_unique) + ", GRID: " + tos(getGroup()) + "]", LOGNAME, m_streamID);
        return;
    }

    try
    {
        std::vector<__uint8> &adBuffer = (sc2 ? ad->m_sc2Buffer : ad->m_sc1Buffer);
        int amt = (ad ? (int)(adBuffer.size() - m_adAccess.offset) : 0);
        const bool debug = gOptions.HTTPClientDebug();
        checkListenerIsValid(debug);

        time_t cur_time;
        const int autoDumpTime = detectAutoDumpTimeout (cur_time, m_streamID, (m_clientLogString + "Timeout waiting to send data"));

        if (m_timerStart == 0)
        {
            m_timerStart = cur_time - 8; // prime the rate regulation, make it on the slow side to rejoin queue
            m_timerFrames = (__uint64)m_fps * 8;
        }
        if (m_frameCount > calculateFrameLimit(cur_time))
        {
            m_result.schedule(333);
            m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
            // DLOG ("throttle in send adverts, count " + tos(m_frameCount));
            return;
        }

        if (!m_lastSentMetadata.empty())
        {
            logW3C();
            m_lastSentMetadata.clear();
        }

        amt = min(amt, SEND_SIZE);

        int len = (int)amt,
            frames = 0; // this will be uvox frames as we pre-process earlier to ensure
                        // that the audio frames are frame-synced when this is created
        bool advert = false;
        if (m_output.size() < 4096)
        {
            int rval = doFrameSync (m_streamData->streamUvoxDataType(), debug, len,
                        (int)m_adAccess.offset, adBuffer,
                        cur_time, m_streamData->streamBitrate(),
                        m_streamData->streamSampleRate(), frames, advert);
            int processed = len - rval;
            m_adAccess.offset += processed;
            m_adAccess.total_processed += processed;
            updateFrameCount (frames, cur_time);
        }
        int rval = doSend(debug, cur_time, autoDumpTime);
        m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
        if (rval > 0)
        {
            bandWidth::updateAmount(bandWidth::CLIENT_HTTP_SENT, rval);
            m_bytesSentForCurrentTitle += rval;
            m_lastActivityTime = ::time(NULL);
            if (rval < m_output.size())
            {
                // DLOG ("short send " + tos(rval) + "/" + tos(m_output.size()));
                m_output.erase (m_output.begin(), m_output.begin() + rval);
                return;
            }
            m_output.clear();
        }
        m_result.write();
    }
    catch (std::runtime_error &)
    {
        releaseAdvert();
        throw;
    }
}


void protocol_shoutcastClient::state_SendText() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
    // AD_DEBUG_LOG (m_clientLogString + __FUNCTION__);
#endif

    if (sendDataBuffer (m_streamID, m_outBuffer, m_outBufferSize, m_clientLogString))
    {
        setCallback ();
    }
}

// wrapper for the StopAdRun, as we also need to kick off some listener metrics
void protocol_shoutcastClient::releaseAdvert ()
{
    if (m_adAccess.inAdvertMode ())
    {
        metrics::adSummary summary;

        summary.sid = m_streamID;
        summary.path = getStreamPath (m_streamID);
        summary.tstamp = ::time(NULL);
        summary.id = m_adAccess.getCurrentTrigger()->m_id;
        summary.sd = m_streamData;
        metrics_adListener (*this, summary);

        m_adAccess.stopAdRun (m_streamID, m_streamData->advertGroups, false);
    }
}
