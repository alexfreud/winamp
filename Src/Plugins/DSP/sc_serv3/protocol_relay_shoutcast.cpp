#ifdef _WIN32
#include <winsock2.h>
#endif
#include "protocol_relay_shoutcast.h"
#include "protocol_backup.h"
#include "protocol_relay.h"
#include "streamData.h"
#include "bandwidth.h"
#include "MP3Header.h"
#include "ADTSHeader.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define DEBUG_LOG(...)  do { if (gOptions.relayShoutcastDebug()) DLOG(__VA_ARGS__); } while (0)
#define LOGNAME         "RELAY"

protocol_relay_shoutcast::protocol_relay_shoutcast(socketOps::tSOCKET s,
			const config::streamConfig &originalRelayInfo, const uniString::utf8 &srcAddrName,
			const uniString::utf8 &srcAddrNumeric, const int srcPort,
			const uniString::utf8 &srcURLpart, httpHeaderMap_t &headers,
			const int originalBitrate, const uniString::utf8& originalMimeType, const bool backup)
	: runnable(s), m_originalBitrate(originalBitrate), m_originalRelayInfo(originalRelayInfo),
	  m_metadataInterval(mapGet(headers, "icy-metaint", (short unsigned int)0)),
	  m_backup(backup), m_denied(false), m_remainderSize(0),
	  m_remainder(new __uint8[BUF_SIZE * 4]), m_srcAddrName(srcAddrName),
	  m_srcAddrNumeric(srcAddrNumeric), m_srcURLpart(srcURLpart), m_streamData(0),
	  m_srcLogString((!backup ? "[RELAY " : "[BACKUP ") + m_srcAddrName + ":" +
					 tos(srcPort) + m_srcURLpart + " sid=" +
					 tos(originalRelayInfo.m_streamID) + "] "),
	  m_bytesSinceMetadata(0), m_metadataSizeByte(-1)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, originalRelayInfo.m_streamID);
    bool noEntry = false;
    streamData::isRelayActive (m_originalRelayInfo.m_streamID, noEntry);

	// for a backup we need to check that the mimetype matches the original source
	// as otherwise there will be issues with the transition between the sources!
	utf8 mimeType = fixMimeType(mapGet(headers, "content-type", utf8("audio/mpeg")));
	if (m_backup && !originalMimeType.empty() && (originalMimeType != mimeType))
	{
		ELOG(m_srcLogString + "Backup source rejected. The content type does not match the original stream "
			 "source - detected `" + mimeType + "' instead of `" + originalMimeType + "'.", (char*)m_srcLogString.c_str(), originalRelayInfo.m_streamID);
		m_state = &protocol_relay_shoutcast::state_CloseConnection;
		return;
	}
	m_originalMimeType = mimeType;

	// for a backup we need to check that the bitrate matches the original source
	// as otherwise there will be issues with the transition between the sources!
	const int bitrate = getStreamBitrate(headers);
	if (m_backup && (m_originalBitrate > 0) && (m_originalBitrate != bitrate) && (m_originalBitrate/1000 != bitrate))
	{
		ELOG(m_srcLogString + "Backup source rejected. The bitrate "
			 "does not match the original stream source - detected " +
			 tos(bitrate) + " kbps instead of " +
			 tos(m_originalBitrate) + " kbps.", (char*)m_srcLogString.c_str(), originalRelayInfo.m_streamID);
		m_state = &protocol_relay_shoutcast::state_CloseConnection;
		return;
	}
	m_originalBitrate = bitrate;

	// check that these bitrates are allowed (looking at both max and average values)
	int streamMaxBitrate = 0, streamMinBitrate = 0;
	const int ret = gOptions.isBitrateDisallowed(originalRelayInfo.m_streamID, bitrate,
												 streamMaxBitrate, streamMinBitrate);
	if (ret)
	{
		m_denied = true;
		utf8 mode = ((streamMaxBitrate == streamMinBitrate) ? "of" : (ret == 2 ? "up to" : "from"));
		ELOG(m_srcLogString + (!m_backup ? "Relay" : "Backup") +
			 " source rejected. Only bitrates " + mode + " " +
			 tos((ret == 1 ? streamMinBitrate : streamMaxBitrate) / 1000) + " kbps are allowed "
			 "- detected " + tos(bitrate) + " kbps.", LOGNAME, m_originalRelayInfo.m_streamID);
		m_state = &protocol_relay_shoutcast::state_CloseConnection;
		return;
	}

	bool allowPublicRelay = gOptions.stream_allowPublicRelay(m_originalRelayInfo.m_streamID);
	if (!gOptions.read_stream_allowPublicRelay(m_originalRelayInfo.m_streamID))
	{
		allowPublicRelay = gOptions.allowPublicRelay();
	}
	headers["icy-pub"] = (allowPublicRelay ? "1" : "0");

	/// data might be encoded in url portion, so decode.
	config::streamConfig stream;
	const bool found = gOptions.getStreamConfig(stream, m_originalRelayInfo.m_streamID);
	m_streamData = streamData::createStream(streamData::streamSetup(m_srcLogString,
											m_originalRelayInfo.m_relayUrl.server(),
											(found ? stream.m_authHash : ""), "",
											m_originalRelayInfo.m_relayUrl.url(),
											m_originalRelayInfo.m_backupUrl.url(),
											streamData::SHOUTCAST1,
											m_originalRelayInfo.m_streamID,
											m_originalRelayInfo.m_relayUrl.port(),
											m_originalRelayInfo.m_maxStreamUser,
											m_originalRelayInfo.m_maxStreamBitrate,
											m_originalRelayInfo.m_minStreamBitrate,
											m_originalRelayInfo.m_allowPublicRelay,
											m_backup, getStreamSamplerate(headers),
											mapGet(headers, "icy-vbr", (bool)false), headers));
	if (!m_streamData)
	{
		throwEx<runtime_error>(m_srcLogString + "Could not create " +
							   (!m_backup ? "relay" : "backup") + " connection.");
	}

	// attempt to determine the version of the source based
	// on the icy-notice2 line (assuming it is provided etc)
	utf8 sourceIdent = mapGet(headers, "icy-notice2", utf8());
	m_streamData->updateSourceIdent(sourceIdent, true);
	sourceIdent = mapGet(headers, "server", utf8());
	m_streamData->updateSourceIdent(sourceIdent, true);

	ILOG(m_srcLogString + "Connected to Shoutcast 1 source " + (!m_backup ? "relay" : "backup") + ".", LOGNAME, m_originalRelayInfo.m_streamID);
	m_state = &protocol_relay_shoutcast::state_GetStreamData;
}

void protocol_relay_shoutcast::cleanup()
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	if (m_streamData)
	{
		int killed = m_streamData->isKill();
		// if this was a kill i.e. when a source re-joins then we need to keep things intact
		if (!killed)
		{
			streamData::streamSourceLost(m_srcLogString, m_streamData, m_originalRelayInfo.m_streamID);
            m_streamData = 0;
            bool remove_relay = false;
            if (gOptions.stream_relayURL(m_originalRelayInfo.m_streamID).empty() &&
                    gOptions.stream_backupURL(m_originalRelayInfo.m_streamID).empty())
                remove_relay = true;
            if (remove_relay)
                streamData::removeRelayStatus (m_originalRelayInfo.m_streamID);
		}
		else
		{
			m_streamData->setKill(false);
		}
	}

	socketOps::forgetTCPSocket(m_socket);
	forgetArray(m_remainder);
}

protocol_relay_shoutcast::~protocol_relay_shoutcast() throw()
{
    DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);
    ILOG(m_srcLogString + "Disconnected from Shoutcast 1 source " +
            (!m_backup ? "relay" : "backup"), (char*)m_srcLogString.c_str(), m_originalRelayInfo.m_streamID);
    cleanup();
}

void protocol_relay_shoutcast::timeSlice() throw(exception)
{
	const int killed = (m_streamData ? m_streamData->isKill() : 0);

    try
    {
        if (m_streamData && (m_streamData->isDead() || (!m_backup && killed == 1) || (m_backup && killed == 2)))
        {
            DLOG(m_srcLogString + "Detected termination of stream", LOGNAME, m_originalRelayInfo.m_streamID);
            m_state = &protocol_relay_shoutcast::state_Fail;
        }
		return (this->*m_state)();
	}
	catch (const exception &ex)
	{
		// on error, we should get ready to retry if applicable
        utf8 str = ex.what();
        if (!str.empty())
        {
            ELOG(ex.what(), LOGNAME, m_originalRelayInfo.m_streamID);
        }
        if (m_streamData)
            m_streamData->setKill (0);
        m_state = &protocol_relay_shoutcast::state_Fail;
        m_result.run();
    }
}

void protocol_relay_shoutcast::state_Fail() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	if (!m_backup)
	{
		cleanup();
		threadedRunner::scheduleRunnable(new protocol_relay(m_originalRelayInfo));
	}
#ifdef INCLUDE_BACKUP_STREAMS
	else
	{
		threadedRunner::scheduleRunnable(new protocol_backup(m_originalRelayInfo, m_originalBitrate, m_originalMimeType));
	}
#endif
	m_result.done();
}

void protocol_relay_shoutcast::state_GetMetadata() throw(exception)
{
	time_t cur_time;
	const bool debug = gOptions.relayShoutcastDebug();
	const int autoDumpTime = detectAutoDumpTimeout (cur_time, m_originalRelayInfo.m_streamID, (m_srcLogString + "Timeout waiting for stream data"));

	while (m_metadataSizeByte != 0)
	{
		char buf[BUF_SIZE] = {0};
		// don't read beyond metadata interval
		int amt = (m_metadataSizeByte < 0 ? 1 : m_metadataSizeByte);
		amt = min(amt, (BUF_SIZE - 1));

		int rval = 0;
		if ((rval = recv (buf, amt, 0x0)) < 1)
		{
			if (rval == 0)
			{
				throwEx<runtime_error>((debug ? (m_srcLogString + "Remote socket "
									   "closed while waiting for stream data.") : (utf8)""));
			}
			else if (rval < 0)
			{
				rval = socketOps::errCode();
				if (rval != SOCKETOPS_WOULDBLOCK)
				{
					throwEx<runtime_error>((debug ? (m_srcLogString + "Socket error "
										   "while waiting for stream data. " +
										   socketErrString(rval)) : (utf8)""));
				}

				m_result.schedule();
				m_result.read();
				m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
				return;
			}
		}
		bandWidth::updateAmount(bandWidth::RELAY_V1_RECV, rval);
		m_lastActivityTime = ::time(NULL);
		if (m_metadataSizeByte < 0)
		{
			m_metadataSizeByte = buf[0] * 16;
			m_metadataBuffer.clear();
		}
		else
		{
			m_metadataBuffer.insert(m_metadataBuffer.end(), buf, buf + rval);
			m_metadataSizeByte -= rval;
		}
	}

	// parse and add
	// this will pull StreamTitle='' and StreamUrl='' from the string
	if (!m_metadataBuffer.empty())
	{
		bool song = false, url = false, next = false;
		utf8 songStr, urlStr, nextStr;

		// StreamTitle=''
		utf8::size_type pos_start = m_metadataBuffer.find(utf8("itle='"));
		if (pos_start != utf8::npos)
		{
			pos_start += 6;
			utf8::size_type pos_end = m_metadataBuffer.find(utf8("';"));
			if (pos_end != utf8::npos)
			{
				songStr = m_metadataBuffer.substr(pos_start,pos_end - pos_start);
				if (!songStr.empty() && !songStr.isValid())
				{
					// use this as a way to try to ensure we've got a utf-8
					// encoded title to improve legacy source title support
					songStr = asciiToUtf8(songStr.toANSI(true));
				}

				// advance the buffer as StreamUrl=''; has to follow StreamTitle=''
				m_metadataBuffer = m_metadataBuffer.substr(pos_end + 2);
				song = true;
			}
			else
			{
				ELOG(m_srcLogString + "Bad metadata string [" + m_metadataBuffer + "]", LOGNAME, m_originalRelayInfo.m_streamID);
			}
		}

		// StreamUrl=''
		pos_start = m_metadataBuffer.find(utf8("mUrl='"));
		if (pos_start != utf8::npos)
		{
			pos_start += 6;
			utf8::size_type pos_end = m_metadataBuffer.find(utf8("';"));
			if (pos_end != utf8::npos)
			{
				urlStr = m_metadataBuffer.substr(pos_start,pos_end - pos_start);
				url = true;
			}
			else
			{
				ELOG(m_srcLogString + "Bad metadata string [" + m_metadataBuffer + "]", LOGNAME, m_originalRelayInfo.m_streamID);
			}
		}

		// StreamNext=''
		pos_start = m_metadataBuffer.find(utf8("mNext='"));
		if (pos_start != utf8::npos)
		{
			pos_start += 7;
			utf8::size_type pos_end = m_metadataBuffer.find(utf8("';"));
			if (pos_end != utf8::npos)
			{
				nextStr = m_metadataBuffer.substr(pos_start,pos_end - pos_start);
				next = true;
			}
			else
			{
				ELOG(m_srcLogString + "Bad metadata string [" + m_metadataBuffer + "]", LOGNAME, m_originalRelayInfo.m_streamID);
			}
		}

		if (!song && !url && !next)
		{
			ELOG(m_srcLogString + "Bad metadata string [" + m_metadataBuffer + "]", LOGNAME, m_originalRelayInfo.m_streamID);
		}
		else
		{
			if (m_streamData->addSc1MetadataAtCurrentPosition(m_srcLogString, songStr, urlStr, nextStr) & 1)
			{
				ILOG(m_srcLogString + "Title update [" + songStr + "]", LOGNAME, m_originalRelayInfo.m_streamID);
			}
		}
	}

	// it's streaming time
	m_metadataSizeByte = -1;
	m_metadataBuffer.clear();
	m_bytesSinceMetadata = 0;
	m_state = &protocol_relay_shoutcast::state_GetStreamData;

	m_result.run();
}

void protocol_relay_shoutcast::state_GetStreamData() throw(exception)
{
	time_t cur_time;


    try
    {
        const int autoDumpTime = detectAutoDumpTimeout (cur_time, m_originalRelayInfo.m_streamID, (m_srcLogString + "Timeout waiting for stream data"));

        int bitrate = m_streamData->streamBitrate();
        const int type = m_streamData->streamUvoxDataType();
        while ((!m_metadataInterval) || (m_bytesSinceMetadata < m_metadataInterval)) 
        {
            char buf[BUF_SIZE * 4] = {0};
            // don't read beyond metadata interval otherwise we'll have audio glitching issues :o(
            int amt = ((m_metadataInterval > 0) ? m_metadataInterval - m_bytesSinceMetadata : BUF_SIZE);
            amt = min(amt, (BUF_SIZE - 1));

            // if we had anything left over then now we
            // need to copy it back into the buffer and
            // adjust the max data amount to be read in
            if ((m_remainderSize > 0) && ((amt + m_remainderSize) <= (BUF_SIZE * 4)))
            {
                memcpy(buf, m_remainder, m_remainderSize);
            }
            else
            {
                m_remainderSize = 0;
            }

            // adjust the position in the buffer based on the prior
            // state of the remaining data as part of frame syncing
            int rval = 0;
            if ((rval = recv (&buf[m_remainderSize], amt, 0x0)) < 1)
            {
                if (rval < 0)
                {
                    rval = socketOps::errCode();
                    if (rval == SOCKETOPS_WOULDBLOCK)
                    {
                        m_result.schedule (70);
                        m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
                        return;
                    }
                    WLOG (m_srcLogString + "Socket error while waiting for data. " + socketErrString(rval), LOGNAME, m_originalRelayInfo.m_streamID);
                }
                else
                    ILOG (m_srcLogString + "Remote socket closed while waiting for data.", LOGNAME, m_originalRelayInfo.m_streamID);
                throwEx<runtime_error> ("");
            }

            // update these details before we mess with anything
            // else as we have read things and it's needed to
            // ensure that we don't break the metadata detection
            bandWidth::updateAmount(bandWidth::RELAY_V1_RECV, rval);
            m_bytesSinceMetadata += rval;

            // if we're here then we account for what we already had in the total
            // so that we then don't skip the new data read with the original data
            rval += m_remainderSize;
            m_remainderSize = 0;
            amt = rval;

            if (m_streamData->syncToStream(m_remainderSize, m_remainder, amt, bitrate,
                        type, buf, m_srcLogString))
            {
                m_denied = true;
                ELOG (m_srcLogString + (!m_backup ? "Relay" : "Backup") +
                        " source rejected. Unable to sync to the stream. Please "
                        "check the source is valid and in a supported format.", LOGNAME, m_originalRelayInfo.m_streamID);
                throwEx<runtime_error> ("");
            }

            m_lastActivityTime = ::time(NULL);
        }

        if (m_metadataInterval > 0)
        {
            // it's metadata time!
            m_metadataSizeByte = -1;
            m_metadataBuffer.clear();
            m_state = &protocol_relay_shoutcast::state_GetMetadata;
        }

        m_result.run();
    }
    catch (exception &e)
    {
        // if there was a failure, now see if we have a backup and attempt to run
        // before we remove the current handling of the dropped source connection
        vector<config::streamConfig> backupInfo = gOptions.getBackupUrl(m_originalRelayInfo.m_streamID);
        if (!m_backup && !backupInfo.empty() && !m_denied)
        {
            m_denied = true;
            if (m_streamData)
            {
                m_streamData->clearCachedMetadata();
                streamData::streamSourceLost(m_srcLogString, m_streamData, m_originalRelayInfo.m_streamID);
                m_streamData = 0;
            }
        }
        throw;
    }
}

void protocol_relay_shoutcast::state_CloseConnection() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	m_result.done();
}
