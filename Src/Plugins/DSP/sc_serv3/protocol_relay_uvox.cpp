#ifdef _WIN32
#include <winsock2.h>
#endif
#include "protocol_relay_uvox.h"
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

#define DEBUG_LOG(...)  do { if (gOptions.relayUvoxDebug()) DLOG(__VA_ARGS__); } while (0)
#define LOGNAME         "RELAY"

protocol_relay_uvox::protocol_relay_uvox(const socketOps::tSOCKET s, const config::streamConfig &originalRelayInfo,
										 const uniString::utf8 &srcAddrName, const uniString::utf8 &srcAddrNumeric,
										 const int srcPort, const uniString::utf8 &srcURLpart,
										 const httpHeaderMap_t &httpHeaders, const int originalBitrate,
										 const uniString::utf8& originalMimeType, const bool backup)  throw(runtime_error)
	: runnable(s), m_backup(backup), m_denied(false), m_remainderSize(0),
	  m_remainder(new __uint8[BUF_SIZE * 4]), m_srcAddrName(srcAddrName),
	  m_srcAddrNumeric(srcAddrNumeric), m_srcURLpart(srcURLpart),
	  m_srcLogString((!backup ? "[RELAY " : "[BACKUP ") + m_srcAddrName +
					  ":" + tos(srcPort) + m_srcURLpart + " sid=" +
					  tos(originalRelayInfo.m_streamID) + "] "),
	  m_outData(new __uint8[MAX_MESSAGE_SIZE]), m_outBuffer(0), m_outBufferSize(0),
	  m_originalRelayInfo(originalRelayInfo), m_streamData(0), m_nextState(0)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	// we need to look in the response headers and figure out what's going on

	// mime type. For uvox2 there is none, for shoutcast 2 it must be there
	// changed in build 22 to match new specs as there's no part in brackets now
	// i.e. should just be 'misc/ultravox if it's a SC2 stream being relayed

	// fixed in build 23 so we set the mime type to the source's details
	// which means older clients can connect and keeps relaying to specs
	switch (strtol((const char*)mapGet(httpHeaders, "ultravox-class-type", utf8()).c_str(), 0, 16))
	{
		case MP3_DATA:
		{
			m_configData.m_mimeType = "audio/mpeg";
			break;
		}
		case AAC_LC_DATA:
		case AACP_DATA:
		{
			m_configData.m_mimeType = "audio/aacp";
			break;
		}
		case OGG_DATA:
		{
			m_configData.m_mimeType = "audio/ogg";
			break;
		}
		default:
		{
			m_configData.m_mimeType = mapGet(httpHeaders, "content-type", utf8("audio/mpeg"));
			break;
		}
	}

	if (m_configData.m_mimeType.empty())
	{
		throwEx<runtime_error>(m_srcLogString + "No mime-type specified.");
	}

	utf8 cdn_authhash;
	if (mapGet(httpHeaders, "cdn-master", false))
	{
		DEBUG_LOG(m_srcLogString + "CDN master response received by slave", LOGNAME, m_originalRelayInfo.m_streamID);
		utf8 authhash = mapGet(httpHeaders, "cdn-token", utf8());
		authhash = XTEA_decipher(authhash.c_str(), authhash.size(), bob().c_str(), bob().size());
		if (yp2::isValidAuthhash(authhash))
		{
			cdn_authhash = authhash;
		}
		else
		{
			DEBUG_LOG(m_srcLogString + "CDN master response rejected - invalid master authash provided", LOGNAME, m_originalRelayInfo.m_streamID);
		}
	}

	// for a backup we need to check that the mimetype matches the original source
	// as otherwise there will be issues with the transition between the sources!
	if (m_backup && !originalMimeType.empty() && (originalMimeType != m_configData.m_mimeType))
	{
		ELOG(m_srcLogString + "Backup source rejected. The content type does not match the original stream "
			 "source - detected `" + m_configData.m_mimeType + "' instead of `" + originalMimeType + "'.", LOGNAME, m_originalRelayInfo.m_streamID);
		loadAndSendMsg("NAK:Unsupported mime type", MSG_BROADCAST_SETUP, &protocol_relay_uvox::state_CloseConnection);
		return;
	}

	utf8 p = mapGet(httpHeaders,"ultravox-max-msg",utf8());
	if (p.empty())
	{
		throwEx<runtime_error>(m_srcLogString + "Missing Ultravox-Max-Msg header");
	}

	int max_msg = p.toInt();
	if ((max_msg < 256) || (max_msg > MAX_PAYLOAD_SIZE))
	{
		throwEx<runtime_error>(m_srcLogString + "Bad Ultravox-Max-Msg value " + tos(max_msg));
	}

	p = mapGet(httpHeaders, "ultravox-samplerate", utf8("0"));
	int samplerate = p.toInt();
	if (samplerate < 0)
	{
		throwEx<runtime_error>(m_srcLogString + "Bad samplerate specified (" + tos(samplerate) + ")");
	}

	// this is basically a hint that should only appear
	// if relaying a MP3 VBR stream from a 2.5+ DNAS...
	p = mapGet(httpHeaders, "ultravox-vbr", utf8("0"));
	const bool vbr = !!p.toInt();

	m_configData.m_avgBitrate = 0;
	m_configData.m_maxBitrate = 0;
	p = mapGet(httpHeaders, "ultravox-avg-bitrate", utf8("0"));
	m_configData.m_avgBitrate = p.toInt();
	p = mapGet(httpHeaders, "ultravox-max-bitrate", utf8("0"));
	m_configData.m_maxBitrate = p.toInt();
	p = mapGet(httpHeaders, "ultravox-bitrate", utf8("0"));

	int x = p.toInt();
	if (x > 0)
	{
		m_configData.m_maxBitrate = m_configData.m_avgBitrate = x;
	}

	if (m_configData.m_avgBitrate <= 0)
	{
		throwEx<runtime_error>(m_srcLogString + "Bad avg bitrate specified (" +
							   tos(m_configData.m_avgBitrate) + ")");
	}

	if (m_configData.m_maxBitrate <= 0)
	{
		throwEx<runtime_error>(m_srcLogString + "Bad max bitrate specified (" +
							   tos(m_configData.m_maxBitrate) + ")");
	}

	// for a backup we need to check that the bitrate matches the original source
	// as otherwise there will be issues with the transition between the sources!
	int bitrate = max(m_configData.m_avgBitrate, m_configData.m_maxBitrate);
	if (m_backup && (originalBitrate > 0) && (originalBitrate != bitrate))
	{
		ELOG(m_srcLogString + "Backup source rejected. The bitrate "
			 "does not match the original stream source - detected " +
			 tos(bitrate / 1000) + " kbps instead of " +
			 tos(originalBitrate / 1000) + " kbps.");
		loadAndSendMsg("NAK:Bit Rate Error", MSG_BROADCAST_SETUP, &protocol_relay_uvox::state_CloseConnection);
		return;
	}

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
			 "- detected " + tos(bitrate / 1000) + " kbps.");
		loadAndSendMsg("NAK:Bit Rate Error", MSG_BROADCAST_SETUP, &protocol_relay_uvox::state_CloseConnection);
		return;
	}


	m_configData.m_minimumBufferSize = m_configData.m_desiredBufferSize = 0;

	m_configData.m_icyName = mapGet(httpHeaders,"ultravox-title", (utf8)"");
	m_configData.m_icyGenre = mapGet(httpHeaders,"ultravox-genre", utf8("Misc"));
	m_configData.m_icyURL = mapGet(httpHeaders,"ultravox-url", (utf8)"");
	m_configData.m_icyPub = m_originalRelayInfo.m_allowPublicRelay;

	config::streamConfig stream;
	const bool found = gOptions.getStreamConfig(stream, m_originalRelayInfo.m_streamID);

	m_streamData = streamData::createStream(streamData::streamSetup(m_srcLogString,
											m_originalRelayInfo.m_relayUrl.server(),
											(!cdn_authhash.empty() ? cdn_authhash :
											(found ? stream.m_authHash : "")), "",
											m_originalRelayInfo.m_relayUrl.url(),
											m_originalRelayInfo.m_backupUrl.url(),
											streamData::SHOUTCAST2,
											m_originalRelayInfo.m_streamID,
											m_originalRelayInfo.m_relayUrl.port(),
											m_originalRelayInfo.m_maxStreamUser,
											m_originalRelayInfo.m_maxStreamBitrate,
											m_originalRelayInfo.m_minStreamBitrate,
											m_originalRelayInfo.m_allowPublicRelay,
											m_backup, samplerate, vbr, m_configData));
	if (!m_streamData)
	{
		throwEx<runtime_error>(m_srcLogString + "Could not create " + (!m_backup ?
							   "relay" : "backup") + " connection.");
	}

	// attempt to determine the version of the source based
	// on the icy-notice2 line (assuming it is provided etc)
	utf8 sourceIdent = mapGet(httpHeaders, "icy-notice2", utf8());
	m_streamData->updateSourceIdent(sourceIdent, true);
	sourceIdent = mapGet(httpHeaders, "server", utf8());
	m_streamData->updateSourceIdent(sourceIdent, true);

	DEBUG_LOG(m_srcLogString + "Stream configuration [" + eol() + m_configData.toLogString() + eol() + "]");
	ILOG(m_srcLogString + "Connected to Shoutcast 2 source " + (!m_backup ? "relay" : "backup") + ".");
	m_state = &protocol_relay_uvox::state_GetStreamData;
}

void protocol_relay_uvox::cleanup()
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	if (m_streamData)
	{
		int killed = m_streamData->isKill();
		if (!m_denied)
		{
			ILOG(m_srcLogString + "Disconnected from Shoutcast 2 source " +
				 (!m_backup ? "relay" : "backup") +
				 (!killed ? "." : " - original source connected."));
		}

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
	forgetArray(m_outData);
	forgetArray(m_remainder);
}

protocol_relay_uvox::~protocol_relay_uvox() throw()
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	cleanup();
}

void protocol_relay_uvox::timeSlice() throw(exception)
{
	const int killed = (m_streamData ? m_streamData->isKill() : 0);
    try
    {
        if (m_streamData && (m_streamData->isDead() || (!m_backup && killed == 1) || (m_backup && killed == 2)))
        {
            DLOG(m_srcLogString + "Detected termination of stream", LOGNAME, m_originalRelayInfo.m_streamID);
            m_state = &protocol_relay_uvox::state_Fail;
        }
        (this->*m_state)();
    }
    catch (const exception &ex)
    {
		// on error, we should get ready to retry if applicable
		utf8 str = ex.what();
		if (!str.empty())
		{
			ELOG(ex.what());
		}
        if (m_streamData)
            m_streamData->setKill (0);
		m_state = &protocol_relay_uvox::state_Fail;
		m_result.run();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Similar to protocol_uvox2Source from here on in ///////////////////////////

template<typename T>
void protocol_relay_uvox::loadAndSendMsg(const T &msg, int type, state_t nextState) throw()
{
	formMessage(msg, type, m_outData, m_outBufferSize);
	bandWidth::updateAmount(bandWidth::RELAY_V2_SENT, m_outBufferSize);
	m_outBuffer = m_outData;
	m_state = &protocol_relay_uvox::state_SendBuffer;
	m_nextState = nextState;
}

// load outbound message into buffer, and establish state to transition to after send
#define SEND_AND_TRANSITION(msg, vtype, state)\
	loadAndSendMsg(msg, vtype, state);\
	m_result.write();\
	m_result.run();\
	return;

// get next packet, without acknowledgement
#define NEXT_PACKET\
	m_inBuffer.clear();\
	m_nextState = m_state;\
	m_state = &protocol_relay_uvox::state_GetPacket;\
	m_result.read();\
	m_result.schedule();\
	m_result.timeoutSID(m_originalRelayInfo.m_streamID);\
	return;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void protocol_relay_uvox::state_GetPacket() throw(exception)
{
	time_t cur_time;

    try
    {
        const int autoDumpTime = detectAutoDumpTimeout (cur_time, m_originalRelayInfo.m_streamID, (m_srcLogString + "Timeout waiting for data"));

        while (true)
        {
            // calculate optimal read size
            char buf[BUF_SIZE] = {0};
            int amt = MAX_MESSAGE_SIZE;
            int len = (int)m_inBuffer.size();

            if (!len)
            {
                amt = UV2X_HDR_SIZE;
            }
            else if (len >= UV2X_HDR_SIZE)
            {
                amt = min(MAX_MESSAGE_SIZE, (int)((ntohs(reinterpret_cast<const uv2xHdr*>(&(m_inBuffer[0]))->msgLen) + UV2X_OVERHEAD) - len));
            }
            else
            {
                amt = min(MAX_MESSAGE_SIZE, (UV2X_OVERHEAD - len));
            }

            int rval = 0;
            if ((rval = recv (buf, amt, 0x0)) < 1)
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

            m_lastActivityTime = ::time(NULL);
            m_inBuffer.insert(m_inBuffer.end(), buf, buf + rval);

            len = (int)m_inBuffer.size();
            if ((len > 1) && (len <= UV2X_HDR_SIZE))
            {
                // check for sync byte as we cannot be
                // certain of good data coming in from
                // the connection and so we check it
                int found = -1;
                for (int i = 0; i < len - 1; i++)
                {
                    // check for sync byte
                    if ((buf[i] == UVOX2_SYNC_BYTE) && (buf[i + 1] == 0))
                    {
                        found = i;
                        break;
                    }
                }

                // track what we've received for the bandwidth stats
                bandWidth::updateAmount(bandWidth::RELAY_V2_RECV, len);

                if (found != -1)
                {
                    // we need to re-sync and so need to
                    // clear the buffer and replace it
                    // according to the re-sync position
                    if (found > 0)
                    {
                        DEBUG_LOG(m_srcLogString + "Shoutcast 2 source relay re-synced to stream [pos: " + tos(found) + "].");

                        m_inBuffer.clear();

                        // we insert in to the start of the buffer
                        // what appears to be 'good' sync'd data.
                        m_inBuffer.insert(m_inBuffer.end(), &buf[found], &buf[found] + rval - found);
                    }
                }
                else
                {
                    // and then we clear out the buffer which
                    // is ok to do since we're trying to find
                    // the frame (as first or next in stream)
                    m_inBuffer.clear();
                }

                continue;
            }
            else if (len > MAX_MESSAGE_SIZE)
            {
                bandWidth::updateAmount(bandWidth::RELAY_V2_RECV, len);
                throwEx<runtime_error>(m_srcLogString + "UVOX packet is too large"
                        " [got: " + tos(len) + " bytes, max: " +
                        tos(MAX_MESSAGE_SIZE) + " bytes]");
            }
            else if (len > UV2X_HDR_SIZE)
            {
                if ((int)(ntohs(reinterpret_cast<const uv2xHdr*>(&(m_inBuffer[0]))->msgLen) + UV2X_OVERHEAD) == len)
                {
                    // got it
                    bandWidth::updateAmount(bandWidth::RELAY_V2_RECV, len);

                    m_result.run();
                    m_state = m_nextState;
                    return;
                }
            }
        }
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

void protocol_relay_uvox::state_SendBuffer() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	if (sendDataBuffer(m_originalRelayInfo.m_streamID, m_outBuffer, m_outBufferSize, m_srcLogString))
	{
		m_state = m_nextState;
	}
}

// normal streaming state
void protocol_relay_uvox::state_GetStreamData() throw(std::exception)
{
	if (!m_inBuffer.empty())
	{
		const uv2xHdr *voxHdr = (const uv2xHdr*)(&(m_inBuffer[0]));
		const __uint16 voxMsgType = ntohs(voxHdr->msgType);

		if ((voxMsgType >= 0x7000) && (voxMsgType < 0x9000))
		{
			// if we have old uvox, then we don't know our mime-type (since old uvox 2 doesn't
			// specify it, though 2.1 does). In the case that mime_type is empty, inspect the packet type and
			// set it in the stream
			if (m_configData.m_mimeType.empty())
			{
				switch (voxMsgType)
				{
					case MP3_DATA:
					{
						m_configData.m_mimeType = "audio/mpeg";
						break;
					}
					case AAC_LC_DATA:
					case AACP_DATA:
					{
						m_configData.m_mimeType = "audio/aacp";
						break;
					}
					case OGG_DATA:
					{
						m_configData.m_mimeType = "audio/ogg";
						break;
					}
				}

				if (!m_configData.m_mimeType.empty())
				{
					m_streamData->streamSetMimeType(m_configData.m_mimeType);
				}
			}

			char buf[BUF_SIZE * 4] = {0};
			__uint16 amt = ntohs(voxHdr->msgLen);

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

			memcpy(&buf[m_remainderSize], (const __uint8 *)((&(m_inBuffer[UV2X_HDR_SIZE]))), amt);
			amt += m_remainderSize;
			m_remainderSize = 0;

			int br = m_streamData->streamBitrate();
			if (m_streamData->syncToStream (m_remainderSize, m_remainder, amt, br, voxMsgType, buf, m_srcLogString))
			{
				m_denied = true;
				throwEx<runtime_error>(m_srcLogString + (!m_backup ? "Relay" : "Backup") +
									   " source rejected. Unable to sync to the stream. Please "
									   "check the source is valid and in a supported format.");
			}
		}
		else if ((voxMsgType >= 0x3000) && (voxMsgType < 0x5000))
		{
			DEBUG_LOG(m_srcLogString + "Cacheable metadata received type=0x" + tohex(voxMsgType), LOGNAME, m_originalRelayInfo.m_streamID);
			const __uint16 voxPayloadSize = ntohs(voxHdr->msgLen);
			if (voxPayloadSize >= UV2X_META_HDR_SIZE) // make sure there's enough data
			{
				const __uint8 *contents = (const __uint8 *)((&(m_inBuffer[UV2X_HDR_SIZE])));
				const uv2xMetadataHdr *metaHdr = reinterpret_cast<const uv2xMetadataHdr*>(contents);
				const __uint16 metadataID = ntohs(metaHdr->id);
				const __uint16 metadataSpan = ntohs(metaHdr->span);
				const __uint16 metadataIndex = ntohs(metaHdr->index) - 1;
				const __uint8* metadataContents = contents + UV2X_META_HDR_SIZE;
				const size_t metadataContentsSize = voxPayloadSize - UV2X_META_HDR_SIZE;

				if ((metadataSpan <= MAX_METADATA_FRAGMENTS) &&
					(metadataSpan > 0) &&
					(metadataIndex < MAX_METADATA_FRAGMENTS) &&
					(metadataIndex < metadataSpan))
				{
					assemblyTableIndex_t ati = makeAssemblyTableIndex(voxMsgType, metadataID);
					metadataEntry_t &me = m_metadataAssemblyTable[ati];

					if (metadataSpan != me.m_expectedFragments) // span changed, clear the entire thing
					{
						__uint16 expectedFragments = me.m_expectedFragments;
						me.clear();
						DEBUG_LOG(m_srcLogString + "Cacheable metadata reset due to span change [" + tos(metadataSpan) + "," + tos(expectedFragments) + "]", LOGNAME, m_originalRelayInfo.m_streamID);
					}

					me.m_expectedFragments = metadataSpan;
					if (me.m_fragments[metadataIndex].m_isValid) // duplicate fragment, clear the entire thing
					{
						me.clear();
						DEBUG_LOG(m_srcLogString + "Cacheable metadata reset due to duplicate fragment", LOGNAME, m_originalRelayInfo.m_streamID);
					}

					me.m_fragments[metadataIndex].m_isValid = true;
					me.m_fragments[metadataIndex].m_fragment.insert(me.m_fragments[metadataIndex].m_fragment.end(),
																	metadataContents, metadataContents + metadataContentsSize);

					if ((++me.m_receivedFragments) == me.m_expectedFragments)
					{
						// assembly, send and clear
						vector<__uint8> assembledData;
						for (__uint16 x = 0; x < me.m_expectedFragments; ++x)
						{
							assembledData.insert(assembledData.end(), me.m_fragments[x].m_fragment.begin(), me.m_fragments[x].m_fragment.end());
						}

						// send
						m_streamData->addUvoxMetadataAtCurrentPosition(voxMsgType,assembledData);

						if (gOptions.relayUvoxDebug())
						{
							if ((voxMsgType >= 0x3000) && (voxMsgType < 0x4000))
							{
								ILOG(m_srcLogString + "Got complete metadata message type=0x" + tohex(voxMsgType) +
													  " [" + tos(assembledData.size()) + " bytes]" +
													  " id=" + tos(metadataID) +
													  " span=" + tos(metadataSpan) +
													  " content=" + eol() + utf8(&(assembledData[0]), assembledData.size()), LOGNAME, m_originalRelayInfo.m_streamID);
							}
							else
							{
								ILOG(m_srcLogString + "Got complete metadata message type=0x" + tohex(voxMsgType) +
													  " [" + tos(assembledData.size()) + " bytes]" +
													  " id=" + tos(metadataID) +
													  " span=" + tos(metadataSpan), LOGNAME, m_originalRelayInfo.m_streamID);
							}
						}
						else
						{
							if ((voxMsgType >= 0x3000) && (voxMsgType < 0x4000))
							{
								utf8 currentSong, comingSoon;
								std::vector<uniString::utf8> nextSongs;
								m_streamData->getStreamNextSongs(m_originalRelayInfo.m_streamID, currentSong, comingSoon, nextSongs);

								if (!currentSong.empty())
								{
									if (!comingSoon.empty())
									{
										ILOG(m_srcLogString + "Title update [now: \"" + currentSong + "\", next: \"" + comingSoon + "\"]", LOGNAME, m_originalRelayInfo.m_streamID);
									}
									else
									{
										ILOG(m_srcLogString + "Title update [" + currentSong + "]", LOGNAME, m_originalRelayInfo.m_streamID);
									}
									m_streamData->resetAdvertTriggers(currentSong);
								}
							}
						}

						// clear
						m_metadataAssemblyTable.erase(ati);
					}
				}
				else
				{
					ELOG(m_srcLogString + "Badly formed metadata packet type=0x" + tohex(voxMsgType) + " id=" +
						 tos(metadataID) + " span=" + tos(metadataSpan) + " index=" + tos(metadataIndex + 1), LOGNAME, m_originalRelayInfo.m_streamID);
				}
			}
			else
			{
				ELOG(m_srcLogString + "Badly formed metadata packet type=0x" + tohex(voxMsgType) +
					 " content of packet is too small payloadsize=" + tos(voxPayloadSize), LOGNAME, m_originalRelayInfo.m_streamID);
			}
		}
		else if ((voxMsgType >= 0x5000) && (voxMsgType < 0x7000))
		{
			// pass thru metadata
			DEBUG_LOG(m_srcLogString + "Pass thru metadata");
			m_streamData->writeSc21(m_inBuffer); // like data, but don't write to sc1 buffers
		}
		else
		{
			ELOG(m_srcLogString + "Unknown or out of sequence packet " + tos(voxMsgType), LOGNAME, m_originalRelayInfo.m_streamID);
			if ((voxMsgType < 0x2000) && (voxMsgType != MSG_FILE_TRANSFER_DATA))
			{
				// probably have to NAK it
				SEND_AND_TRANSITION("NAK:Unsupported packet type", voxMsgType, &protocol_relay_uvox::state_GetStreamData);
			}
		}
	}

	NEXT_PACKET;
}

void protocol_relay_uvox::state_Fail() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	if (!m_backup)
	{
		cleanup();
		threadedRunner::scheduleRunnable(new protocol_relay(m_originalRelayInfo, true));
	}
#ifdef INCLUDE_BACKUP_STREAMS
	else
	{
        threadedRunner::scheduleRunnable(new protocol_backup(m_originalRelayInfo, max(m_configData.m_avgBitrate/1000,
                        m_configData.m_maxBitrate/1000), m_configData.m_mimeType));
	}
#endif
	m_result.done();
}

void protocol_relay_uvox::state_CloseConnection() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	m_result.done();
}
