#ifdef _WIN32
#include <winsock2.h>
#endif
#include "protocol_uvox2Source.h"
#include "protocol_backup.h"
#include "uvox2Common.h"
#include "streamData.h"
#include "global.h"
#include "bandwidth.h"
#include "MP3Header.h"
#include "ADTSHeader.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define DEBUG_LOG(...)  do { if (gOptions.uvox2SourceDebug()) DLOG(__VA_ARGS__); } while (0)
#define LOGNAME         "SRC"


protocol_uvox2Source::protocol_uvox2Source (microConnection &mc, const __uint8 *firstPacket, const int sizeOfFirstPacket) throw(exception)

    : runnable (mc), m_srcPort(mc.m_srcPort), m_srcAddr(mc.m_srcHostName)
{
    m_srcLogString = srcAddrLogString (m_srcAddr, m_srcPort);
    m_outData = new __uint8 [MAX_MESSAGE_SIZE];
    m_remainder = new __uint8 [BUF_SIZE * 4];
    m_remainderSize = 0;
    m_srcStreamID = DEFAULT_SOURCE_STREAM;
    m_outBuffer = NULL;
    m_outBufferSize = 0;
    m_flushCachedMetadata = false;
    m_specialFileBytesExpected = 0;
    m_denied = false;
    m_streamData = NULL;
    m_loop = 0;
    m_state = &protocol_uvox2Source::state_SendCrypto;
    m_nextState = NULL;

	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	ILOG(m_srcLogString + "Shoutcast 2 source connection starting.", LOGNAME, m_srcStreamID);

	m_configData.m_maxBitrate = 0;
	m_configData.m_avgBitrate = 0;
	m_configData.m_desiredBufferSize = 0;
	m_configData.m_minimumBufferSize = 0;

	if (sizeOfFirstPacket > MAX_MESSAGE_SIZE)
	{
		m_denied = true;
		throw runtime_error(m_srcLogString.hideAsString() + "Initial data packet too large (" + tos(sizeOfFirstPacket) + ")");
	}

	bandWidth::updateAmount(bandWidth::SOURCE_V2_RECV, sizeOfFirstPacket);
	m_inBuffer.insert(m_inBuffer.end(), firstPacket, firstPacket + sizeOfFirstPacket);
}

protocol_uvox2Source::~protocol_uvox2Source() throw()
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	if (m_streamData)
	{
		streamData::streamSourceLost(m_srcLogString, m_streamData, m_srcStreamID);
		m_streamData = 0;
	}

	socketOps::forgetTCPSocket(m_socket);
	forgetArray(m_outData);
	forgetArray(m_remainder);

	if (!m_denied)
	{
		ILOG(m_srcLogString + "Shoutcast 2 source disconnected.", LOGNAME, m_srcStreamID);
	}
}

void protocol_uvox2Source::timeSlice() throw(std::exception)
{
	try
	{
		if (m_streamData && m_streamData->isDead())
		{
			m_result.done();
			return;
		}
		(this->*m_state)();
	}
	catch(const exception &)
	{
        // if there was a failure, now see if we have a backup and attempt to run
        // before we remove the current handling of the dropped source connection
        vector<config::streamConfig> backupInfo = gOptions.getBackupUrl(m_srcStreamID);
        if (!backupInfo.empty() && !m_denied)
        {
            m_denied = true;
            if (m_streamData)
            {
                m_streamData->clearCachedMetadata();
                streamData::streamSourceLost(m_srcLogString, m_streamData, m_srcStreamID);
                m_streamData = 0;
            }
#ifdef INCLUDE_BACKUP_STREAMS
            ILOG(m_srcLogString + "Shoutcast 2 source disconnected - trying source backup.", LOGNAME, m_srcStreamID);
            threadedRunner::scheduleRunnable(new protocol_backup(backupInfo[0],
                        max(m_configData.m_avgBitrate/1000, m_configData.m_maxBitrate/1000),
                        m_configData.m_mimeType));
#endif
        }
        throw;
	}
}

void protocol_uvox2Source::state_CloseConnection() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	m_result.done();
}

void protocol_uvox2Source::state_SendBuffer() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	if (sendDataBuffer(m_srcStreamID, m_outBuffer, m_outBufferSize, m_srcLogString))
	{
		m_state = m_nextState;
	}
}

void protocol_uvox2Source::state_GetPacket() throw(exception)
{
	time_t cur_time;

    const int autoDumpTime = detectAutoDumpTimeout (cur_time, m_srcStreamID, (m_srcLogString + "Timeout waiting for data"));

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
                m_loop = 0;
                rval = socketOps::errCode();
                if (rval == SOCKETOPS_WOULDBLOCK)
                {
                    m_result.schedule(75);
                    m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
                    return;
                }
                WLOG (m_srcLogString + "Socket error while waiting for data. " + socketErrString(rval), LOGNAME, m_srcStreamID);
            }
            else
                DLOG (m_srcLogString + "Remote socket closed while waiting for data.", LOGNAME, m_srcStreamID);

            throwEx<runtime_error> ("");
        }
        if (rval < amt)
            m_loop = 100000; // force a stop when processed.

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
            bandWidth::updateAmount(bandWidth::SOURCE_V2_RECV, len);

            if (found != -1)
            {
                // we need to re-sync and so need to
                // clear the buffer and replace it
                // according to the re-sync position
                if (found > 0)
                {
                    DEBUG_LOG(m_srcLogString + "Shoutcast 2 source re-synced to stream [pos: " + tos(found) + "].");

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
            m_denied = true;
            bandWidth::updateAmount(bandWidth::SOURCE_V2_RECV, rval);
            throwEx<runtime_error>(m_srcLogString + "UVOX packet is too large"
                    " [got: " + tos(len) + " bytes, max: " +
                    tos(MAX_MESSAGE_SIZE) + " bytes]");
        }
        else if (len > UV2X_HDR_SIZE)
        {
            if ((int)(ntohs(reinterpret_cast<const uv2xHdr*>(&(m_inBuffer[0]))->msgLen) + UV2X_OVERHEAD) == len)
            {
                // got it
                bandWidth::updateAmount(bandWidth::SOURCE_V2_RECV, rval);

                m_result.run();
                m_state = m_nextState;
                return;
            }
        }
    }
}

template<typename T>
void protocol_uvox2Source::loadAndSendMsg(const T &msg, int type, state_t nextState) throw()
{
	formMessage(msg, type, m_outData, m_outBufferSize);
	bandWidth::updateAmount(bandWidth::SOURCE_V2_SENT, m_outBufferSize);
	m_outBuffer = m_outData;
	m_state = &protocol_uvox2Source::state_SendBuffer;
	m_nextState = nextState;
}

// load outbound message into buffer, and establish state to transition to after send
#define SEND_AND_TRANSITION(msg, vtype, state)\
	loadAndSendMsg(msg, vtype, state);\
	m_result.write();\
	m_result.run();\
	return;

// get a packet and then transition to indicated state
#define GET_AND_TRANSITION(a)\
	m_inBuffer.clear();\
	m_state = &protocol_uvox2Source::state_GetPacket;\
	m_nextState = a;\
	m_result.run();\
	m_result.read();\
	m_result.timeoutSID(m_srcStreamID);\
	return;

// send a packet then transition to closed state
#define SEND_AND_CLOSE(a, b)\
	SEND_AND_TRANSITION(a, b, &protocol_uvox2Source::state_CloseConnection)

// get next packet, without acknowledgement
#define NEXT_PACKET\
	m_inBuffer.clear();\
	m_nextState = m_state;\
	m_state = &protocol_uvox2Source::state_GetPacket;\
	m_result.schedule();\
	m_result.timeoutSID(m_srcStreamID);\
	return;

void protocol_uvox2Source::state_ConfirmPasswordGet() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	GET_AND_TRANSITION(&protocol_uvox2Source::state_ConfirmPassword);
}

void protocol_uvox2Source::state_SendCrypto() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	// parse the contents of the packet to get user,password,streamid etc.
	const uv2xHdr *voxHdr = (const uv2xHdr*)(&(m_inBuffer[0]));
	const __uint16 voxMsgType = ntohs(voxHdr->msgType);

	if (voxMsgType != MSG_CIPHER)
	{
		m_denied = true;
		ELOG(m_srcLogString + "Out of sequence uvox packet. Got " + tos(voxMsgType) + " but expected " + tos(MSG_CIPHER), LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Sequence Error", voxMsgType);
	}

	const char *contents = (const char *)((&(m_inBuffer[UV2X_HDR_SIZE])));
	if (strcmp(contents,"2.1"))
	{
		ELOG(m_srcLogString + "Cipher request has bad version (" + contents + ")", LOGNAME, m_srcStreamID);
		SEND_AND_TRANSITION("NAK:Version Error", voxMsgType, &protocol_uvox2Source::state_ConfirmPasswordGet);
	}

	m_cipherKey = gOptions.stream_uvoxCipherKey(m_srcStreamID);
	if (!gOptions.read_stream_uvoxCipherKey(m_srcStreamID) || m_cipherKey.empty())
	{
		m_cipherKey = gOptions.uvoxCipherKey();
	}

	SEND_AND_TRANSITION(("ACK:" + m_cipherKey).hideAsString(), voxMsgType, &protocol_uvox2Source::state_ConfirmPasswordGet);
}

// get stream,userID and password out of MSG_AUTH (0x1001) packet
void protocol_uvox2Source::state_ConfirmPassword() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	// parse the contents of the packet to get user,password,streamid etc.
	const uv2xHdr *voxHdr = (const uv2xHdr*)(&(m_inBuffer[0]));
	const __uint16 voxMsgType = ntohs(voxHdr->msgType);

	if (voxMsgType != MSG_AUTH)
	{
		m_denied = true;
		ELOG(m_srcLogString + "Out of sequence uvox packet. Got " + tos(voxMsgType) + " but expected " + tos(MSG_AUTH), LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Sequence Error", voxMsgType);
	}

	const char *contents = (const char *)((&(m_inBuffer[UV2X_HDR_SIZE])));
	const char *pos = strstr(contents, ":");
	if (!pos) 
	{
		m_denied = true;
		ELOG(m_srcLogString + "Bad uvox packet " + tos(voxMsgType) + ". No version field.", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Parse Error", voxMsgType);
	}

	std::string srcVersion = stripWhitespace(string(contents, pos));
	if (srcVersion != "2.1") 
	{
		m_denied = true;
		ELOG(m_srcLogString + "Uvox version of type " + srcVersion + " is not accepted. Expecting 2.1", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Version Error", voxMsgType);
	}

	contents = pos+1;
	pos = strstr(contents, ":");
	if (!pos) 
	{
		m_denied = true;
		ELOG(m_srcLogString + "Bad uvox packet " + tos(voxMsgType) + ". No SID field.", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Parse Error", voxMsgType);
	}

	int sourceStreamID = atoi(string(contents, pos).c_str());
	// ensure that only valid stream id's are allowed to connect (1 -> 2147483647)
	if (!sourceStreamID || (sourceStreamID > INT_MAX))
	{
		m_denied = true;
		ELOG(m_srcLogString + "Bad Stream ID (" + tos(sourceStreamID) + "). Stream ID cannot be below 1 or above 2147483647.", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Stream ID Error", voxMsgType);
	}

	// if we have a moved stream then now we have the stream id
	// then we need to check and block the source as applicable
	utf8 movedUrl = gOptions.stream_movedUrl(sourceStreamID);
	if (!movedUrl.empty())
	{
		m_denied = true;
		ELOG(m_srcLogString + "Shoutcast 2 source rejected. Stream is configured as having moved.", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Deny", voxMsgType);
	}

	m_srcStreamID = sourceStreamID;

	// if stream configs are required then we have an error if we didn't find one
	if (gOptions.requireStreamConfigs())
	{
		config::streamConfig stream;
		if (!gOptions.getStreamConfig(stream, m_srcStreamID))
		{
			m_denied = true;
			ELOG(m_srcLogString + "Shoutcast 2 source rejected. Stream " + tos(m_srcStreamID) + " must be defined in config file.", LOGNAME, m_srcStreamID);
			SEND_AND_CLOSE("NAK:2.1:Stream ID Error", voxMsgType);
		}
	}

	contents = pos+1;
	pos = strstr(contents, ":");
	if (!pos) 
	{
		m_denied = true;
		ELOG(m_srcLogString + "Bad uvox packet " + tos(voxMsgType) + ". No UID field.", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Parse Error", voxMsgType);
	}
	m_srcUserID = string(contents, pos);
	if (!(m_srcUserID.size() % 16))
	{
		m_srcUserID = XTEA_decipher(m_srcUserID.c_str(), m_srcUserID.size(), m_cipherKey.c_str(), m_cipherKey.size());
	}
	else
	{
		m_denied = true;
		ELOG(m_srcLogString + "Bad username format. Size of parameters not matching specification.", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Parse Error", voxMsgType);
	}

	contents = pos + 1;
	utf8 srcPassword = contents;

	if (!(srcPassword.size() % 16))
	{
		srcPassword = XTEA_decipher(srcPassword.c_str(), srcPassword.size(), m_cipherKey.c_str(), m_cipherKey.size());
	}
	else
	{
		m_denied = true;
		ELOG(m_srcLogString + "Bad password format. Size of parameters not matching specification.", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Parse Error", voxMsgType);
	}

	// if no stream configuration is specified then we need to fallback
	utf8 password = gOptions.stream_password(m_srcStreamID);
	if (!gOptions.read_stream_password(m_srcStreamID) || password.empty())
	{
		password = gOptions.password();
	}

	// look at the password and check for the multi-1.x style support,
	// extracting as needed so we are consistent with all other modes.
	// note: we ignore the other parameters and just want 'password'.
	utf8 dj_name;	// throw-away (equivalent of 'm_srcUserID')
	extractPassword(srcPassword, dj_name, sourceStreamID);

	if (srcPassword != password)
	{
		m_denied = true;
		ELOG(m_srcLogString + "Shoutcast 2 source connection denied" + (m_srcUserID.empty() ? "" : " for user (" + m_srcUserID + ")") +
			 " on stream #" + tos(m_srcStreamID) + ". " + (srcPassword.empty() ? "Empty password not allowed." : "Bad password: " + srcPassword), LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:2.1:Deny", voxMsgType);
	}

	// if we've got a source already connected and it's not a backup
	// then it's better that we just abort processing now than later
	bool isSourceActive = false;
	streamData *sd = streamData::accessStream(m_srcStreamID, isSourceActive);
	if (sd && (isSourceActive == true) && (sd->isBackupStream(m_srcStreamID) == false))
	{
		sd->releaseStream();
		m_denied = true;
		ELOG(m_srcLogString + "Shoutcast 2 source rejected. A source is already connected.", LOGNAME, m_srcStreamID);
		SEND_AND_CLOSE("NAK:Stream In Use", voxMsgType);
	}

	if (sd)
	{
		sd->releaseStream();
	}

	m_srcLogString = srcAddrLogString(m_srcAddr, m_srcPort, m_srcStreamID);
	DEBUG_LOG(m_srcLogString + "Password accepted. Stream id is " + tos(m_srcStreamID), LOGNAME, m_srcStreamID);
	SEND_AND_TRANSITION("ACK:2.1:Allow", voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
}

// first part of stream configuration state, get the next packet, then process it
void protocol_uvox2Source::state_StreamConfigurationGet() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	GET_AND_TRANSITION(&protocol_uvox2Source::state_StreamConfiguration);
}

// handle various stream configuration packets
// note: to allow real negotiation, most errors are rejected, but do not cause the connection to be shut
void protocol_uvox2Source::state_StreamConfiguration() throw(std::exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	const uv2xHdr *voxHdr = (const uv2xHdr*)(&(m_inBuffer[0]));
	const char *contents = (const char *)((&(m_inBuffer[UV2X_HDR_SIZE])));
	const __uint16 voxMsgType = ntohs(voxHdr->msgType);

	switch (voxMsgType)
	{
		case MSG_MIME_TYPE:
		{
			// this is mainly to cope with legacy / incomplete uvox2.x implementations
			m_configData.m_mimeType = contents;
			DEBUG_LOG(m_srcLogString + "original mime type=" + m_configData.m_mimeType, LOGNAME, m_srcStreamID);
			m_configData.m_mimeType = fixMimeType(m_configData.m_mimeType);
			DEBUG_LOG(m_srcLogString + "mime type=" + m_configData.m_mimeType, LOGNAME, m_srcStreamID);
			break;
		}
		case MSG_BROADCAST_SETUP:
		{
			const char *pos = strstr(contents, ":");
			if (!pos)
			{
				ELOG(m_srcLogString + "Bad uvox packet " + tos(voxMsgType) + ". No avg bitrate field.", LOGNAME, m_srcStreamID);
				SEND_AND_TRANSITION("NAK:Parse Error", voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
			}
			m_configData.m_avgBitrate = atoi(string(contents,pos).c_str());
			contents = pos + 1;
			m_configData.m_maxBitrate = atoi(contents);

			// check that these bitrates are allowed (looking at both max and average values)
			const int bitrate = max(m_configData.m_avgBitrate, m_configData.m_maxBitrate);
			int streamMaxBitrate = 0, streamMinBitrate = 0;
			const int ret = gOptions.isBitrateDisallowed(m_srcStreamID, bitrate, streamMaxBitrate, streamMinBitrate);
			if (ret)
			{
				m_denied = true;
				utf8 mode = ((streamMaxBitrate == streamMinBitrate) ? "of" : (ret == 2 ? "up to" : "from"));
				ELOG(m_srcLogString + "Shoutcast 2 source rejected. Only bitrates " + mode + " " +
					 tos((ret == 1 ? streamMinBitrate : streamMaxBitrate) / 1000) +
					 " kbps are allowed - detected " + tos(bitrate / 1000) + " kbps.", LOGNAME, m_srcStreamID);
				SEND_AND_CLOSE("NAK:Bit Rate Error", voxMsgType);
			}

			DEBUG_LOG(m_srcLogString + "avg bitrate=" + tos(m_configData.m_avgBitrate) + " max bitrate=" + tos(m_configData.m_maxBitrate), LOGNAME, m_srcStreamID);
			break;
		}
		case MSG_NEGOTIATE_BUFFER_SIZE:
		{
			const char *pos = strstr(contents, ":");
			if (!pos)
			{
				ELOG(m_srcLogString + "Bad uvox packet " + tos(voxMsgType) + ". No desired buffer size field.", LOGNAME, m_srcStreamID);
				SEND_AND_TRANSITION("NAK:Parse Error", voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
			}
			m_configData.m_desiredBufferSize = atoi(string(contents, pos).c_str());
			contents = pos + 1;
			m_configData.m_minimumBufferSize = atoi(contents);
			DEBUG_LOG(m_srcLogString + "desired buffer size=" + tos(m_configData.m_desiredBufferSize) + " min buffer size=" + tos(m_configData.m_minimumBufferSize), LOGNAME, m_srcStreamID);
			SEND_AND_TRANSITION("ACK:" + tos(m_configData.m_desiredBufferSize), voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
		}
		case MSG_MAX_PAYLOAD_SIZE:
		{
			const char *pos = strstr(contents, ":");
			if (!pos)
			{
				ELOG(m_srcLogString + "Bad uvox packet " + tos(voxMsgType) + ". Parse error.", LOGNAME, m_srcStreamID);
				SEND_AND_TRANSITION("NAK:Parse Error", voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
			}
			int max1 = atoi(string(contents, pos).c_str());
			contents = pos + 1;
			int max2 = atoi(contents);
			DEBUG_LOG(m_srcLogString + "max payload size 1=" + tos(max1) + " max payload size 2=" + tos(max2), LOGNAME, m_srcStreamID);
			int new_max = max(max1, max2);
			if ((new_max > MAX_PAYLOAD_SIZE) || (new_max < 256))
			{
				ELOG(m_srcLogString + "Bad max payload size of " + tos(new_max), LOGNAME, m_srcStreamID);
				SEND_AND_TRANSITION("NAK:Payload Size Error", voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
			}

			SEND_AND_TRANSITION("ACK:" + tos(MAX_PAYLOAD_SIZE), voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
		}
		case MSG_ICYNAME:
		{
			m_configData.m_icyName = contents;
			utf8 titleFormat = gOptions.titleFormat();
			if (!titleFormat.empty())
			{
				utf8::size_type pos = titleFormat.find(utf8("%s"));
				m_configData.m_icyName = (pos == utf8::npos ? titleFormat : titleFormat.replace(pos, 2, m_configData.m_icyName));
			}
			DEBUG_LOG(m_srcLogString + "icy-name=" + m_configData.m_icyName, LOGNAME, m_srcStreamID);
			break;
		}
		case MSG_ICYGENRE:
		{
			m_configData.m_icyGenre = contents;
			DEBUG_LOG(m_srcLogString + "icy-genre=" + m_configData.m_icyGenre, LOGNAME, m_srcStreamID);
			break;
		}
		case MSG_ICYURL:
		{
			m_configData.m_icyURL = contents;
			if (m_configData.m_icyURL.empty())
			{
				m_configData.m_icyURL = "http://www.shoutcast.com";
			}
			utf8 urlFormat = gOptions.urlFormat();
			if (!urlFormat.empty())
			{
				utf8::size_type pos = urlFormat.find(utf8("%s"));
				m_configData.m_icyURL = (pos == utf8::npos ? urlFormat : urlFormat.replace(pos,2,m_configData.m_icyURL));
			}
			DEBUG_LOG(m_srcLogString + "icy-url=" + m_configData.m_icyURL, LOGNAME, m_srcStreamID);
			break;
		}
		case MSG_ICYPUB:
		{
			m_configData.m_icyPub = atoi(contents);
			DEBUG_LOG(m_srcLogString + "icy-pub=" + tos(m_configData.m_icyPub), LOGNAME, m_srcStreamID);
			break;
		}
		case MSG_STANDBY:
		{
			DEBUG_LOG(m_srcLogString + "Standby", LOGNAME, m_srcStreamID);
			if (m_streamData)
			{
				SEND_AND_CLOSE("NAK:Sequence Error", voxMsgType);
			}
			else
			{
				try
				{
					// build the correct path for the stream clients
					config::streamConfig stream;
					const bool found = gOptions.getStreamConfig(stream, m_srcStreamID);
					m_streamData = streamData::createStream(streamData::streamSetup(m_srcLogString, m_srcAddr,
															(found ? stream.m_authHash : ""), m_srcUserID, "",
															(found ? stream.m_backupUrl.url() : ""),
															streamData::SHOUTCAST2, m_srcStreamID, m_srcPort,
															(found ? stream.m_maxStreamUser : 0),
															(found ? stream.m_maxStreamBitrate : 0),
															(found ? stream.m_minStreamBitrate : 0),
															(found ? stream.m_allowPublicRelay : true),
															false, 0, false, m_configData));
				}
				catch(const exception &ex)
				{
					ELOG(m_srcLogString + ex.what(), LOGNAME, m_srcStreamID);
					SEND_AND_TRANSITION(string("NAK:") + ex.what(), voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
				}

				if (!m_streamData)
				{
					m_denied = true;
					ELOG(m_srcLogString + "Shoutcast 2 source rejected. A source is already connected.", LOGNAME, m_srcStreamID);
					SEND_AND_CLOSE("NAK:Stream In Use", voxMsgType);
				}
				else
				{
					if (m_flushCachedMetadata)
					{
						m_streamData->clearCachedMetadata();
						m_metadataAssemblyTable.clear();
					}
					m_flushCachedMetadata = false;
				}
			}
			DEBUG_LOG(m_srcLogString + "Stream configuration complete. [" + eol() + m_configData.toLogString() + eol() + "]", LOGNAME, m_srcStreamID);
			SEND_AND_TRANSITION("ACK:Data transfer mode", voxMsgType, &protocol_uvox2Source::state_StreamDataGet);
		}
		case MSG_FLUSH_CACHED_METADATA:
		{
			DEBUG_LOG(m_srcLogString + "Flush cached metadata", LOGNAME, m_srcStreamID);
			m_metadataAssemblyTable.clear();
			m_flushCachedMetadata = true;
			break;
		}
		case MSG_TERMINATE:
		{
			DEBUG_LOG(m_srcLogString + "Terminate", LOGNAME, m_srcStreamID);
			throwEx<runtime_error> ("");
            break;
		}
		case MSG_LISTENER_AUTHENTICATION: // not supported, just ACK it
		{
			WLOG(m_srcLogString + "Listener Auth - not supported", LOGNAME, m_srcStreamID);
			break;
		}
		default: // out of sequence
		{
			ELOG(m_srcLogString + "Unknown or out of sequence packet " + tos(voxMsgType), LOGNAME, m_srcStreamID);
			SEND_AND_TRANSITION("NAK:Sequence Error", voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
		}
	}

	SEND_AND_TRANSITION("ACK", voxMsgType, &protocol_uvox2Source::state_StreamConfigurationGet);
}

// this is needed to allow the source to process the
// response before starting to send the stream data
void protocol_uvox2Source::state_StreamDataGet() throw(std::exception)
{
	GET_AND_TRANSITION(&protocol_uvox2Source::state_GetStreamData);
}

// normal streaming state
void protocol_uvox2Source::state_GetStreamData() throw(std::exception)
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

            __uint16 amt = ntohs(voxHdr->msgLen);
            vector <__uint8>  tempbuf;
            const char *buf;

            // if we had anything left over then now we
            // need to copy it back into the buffer
            if (m_remainderSize > 0 && m_remainder)
            {
                tempbuf.insert (tempbuf.end(), m_remainder, m_remainder+m_remainderSize);
                tempbuf.insert (tempbuf.end(), m_inBuffer.begin() + UV2X_HDR_SIZE, m_inBuffer.begin()+UV2X_HDR_SIZE+amt);
                amt = (__uint16)tempbuf.size();
                buf = (char *)&tempbuf[0];
            }
            else
            {
                // buf.insert (buf.end(), m_inBuffer.begin() + UV2X_HDR_SIZE, m_inBuffer.begin()+UV2X_HDR_SIZE+amt);
                buf = (char*)&m_inBuffer [UV2X_HDR_SIZE];
            }

            m_remainderSize = 0;

			int br = m_streamData->streamBitrate();
			if (m_streamData->syncToStream (m_remainderSize, m_remainder, amt, br,
										   m_streamData->streamUvoxDataType(), buf,
										   m_srcLogString))
			{
				m_denied = true;
				throwEx<runtime_error>(m_srcLogString + "Shoutcast 2 source disconnected. "
									   "Unable to sync to the stream. Please check the "
									   "source is valid and in a supported format.");
			}
            if (++m_loop < 20)
            {
                // force several small uvox packets to read/processed for now
                m_inBuffer.clear();
                m_nextState = m_state;
                m_state = &protocol_uvox2Source::state_GetPacket;
                m_result.run();
                return;
            }
            m_loop = 0;
		}
		else if ((voxMsgType >= 0x3000) && (voxMsgType < 0x5000))
		{
			DEBUG_LOG(m_srcLogString + "Cacheable metadata received type=0x" + tohex(voxMsgType), LOGNAME, m_srcStreamID);
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
					assemblyTableIndex_t ati = makeAssemblyTableIndex(voxMsgType, m_srcStreamID);
					metadataEntry_t &me = m_metadataAssemblyTable[voxMsgType][ati];

					if (metadataSpan != me.m_expectedFragments) // span changed, clear the entire thing
					{
						__uint16 expectedFragments = me.m_expectedFragments;
						me.clear();
						DEBUG_LOG(m_srcLogString + "Cacheable metadata reset due to span change [" + tos(metadataSpan) + "," + tos(expectedFragments) + "]", LOGNAME, m_srcStreamID);
					}

					me.m_expectedFragments = metadataSpan;
					if (me.m_fragments[metadataIndex].m_isValid) // duplicate fragment, clear the entire thing
					{
						me.clear();
						DEBUG_LOG(m_srcLogString + "Cacheable metadata reset due to duplicate fragment", LOGNAME, m_srcStreamID);
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

						if (gOptions.uvox2SourceDebug())
						{
							if ((voxMsgType >= 0x3000) && (voxMsgType < 0x4000))
							{
								ILOG(m_srcLogString + "Got complete metadata message type=0x" + tohex(voxMsgType) +
													  " [" + tos(assembledData.size()) + " bytes]" +
													  " id=" + tos(metadataID) +
													  " span=" + tos(metadataSpan) +
													  " content=" + eol() + utf8(&(assembledData[0]), assembledData.size()), LOGNAME, m_srcStreamID);
							}
							else
							{
								ILOG(m_srcLogString + "Got complete metadata message type=0x" + tohex(voxMsgType) +
													  " [" + tos(assembledData.size()) + " bytes]" +
													  " id=" + tos(metadataID) +
													  " span=" + tos(metadataSpan), LOGNAME, m_srcStreamID);
							}
						}
						else
						{
							if ((voxMsgType >= 0x3000) && (voxMsgType < 0x4000))
							{
								utf8 currentSong, comingSoon;
								std::vector<uniString::utf8> nextSongs;
								m_streamData->getStreamNextSongs(m_srcStreamID, currentSong, comingSoon, nextSongs);

								if (!currentSong.empty())
								{
									if (!comingSoon.empty())
									{
										ILOG(m_srcLogString + "Title update [now: \"" + currentSong + "\", next: \"" + comingSoon + "\"]", LOGNAME, m_srcStreamID);
									}
									else
									{
										ILOG(m_srcLogString + "Title update [" + currentSong + "]", LOGNAME, m_srcStreamID);
									}
									m_streamData->resetAdvertTriggers(currentSong);
								}
							}
							else
							{
								utf8 mimeType[] = {
									"image/jpeg",
									"image/png",
									"image/bmp",
									"image/gif"
								};

								__uint16 ArtType = voxMsgType & 0x0F00;
								if (!assembledData.size())
								{
									ILOG(m_srcLogString + ((ArtType & MSG_METADATA_PLAYING_ART) ? "Playing" : "Stream") + " artwork cleared", LOGNAME, m_srcStreamID);
									m_streamData->clearCachedArtwork(((ArtType & MSG_METADATA_PLAYING_ART) ? 1 : 0));
								}
								else
								{
									ILOG(m_srcLogString + ((ArtType & MSG_METADATA_PLAYING_ART) ? "Playing" : "Stream") +
										 " artwork update [mime=" + mimeType[(voxMsgType & 0x00FF)] + ", " + tos(assembledData.size()) + " bytes]", LOGNAME, m_srcStreamID);
								}
							}
						}

						// clear
						m_metadataAssemblyTable[voxMsgType].erase(ati);
					}
				}
				else
				{
					ELOG(m_srcLogString + "Badly formed metadata packet type=0x" + tohex(voxMsgType) + " id=" +
						 tos(metadataID) + " span=" + tos(metadataSpan) + " index=" + tos(metadataIndex + 1), LOGNAME, m_srcStreamID);
				}
			}
			else
			{
				ELOG(m_srcLogString + "Badly formed metadata packet type=0x" + tohex(voxMsgType) +
					 " content of packet is too small payloadsize=" + tos(voxPayloadSize), LOGNAME, m_srcStreamID);
			}
		}
		else if ((voxMsgType >= 0x5000) && (voxMsgType < 0x7000))
		{
			// pass thru metadata
			DEBUG_LOG(m_srcLogString + "Pass thru metadata", LOGNAME, m_srcStreamID);
			m_streamData->writeSc21(m_inBuffer); // like data, but don't write to sc1 buffers
		}
		else if (voxMsgType == MSG_FILE_TRANSFER_BEGIN)
		{
			const __uint8 *contents = (const __uint8 *)((&(m_inBuffer[UV2X_HDR_SIZE])));
			const __uint8 *pos = (const __uint8*)strstr((const char *)contents, ":");
			if (!pos)
			{
				ELOG(m_srcLogString + "Bad uvox packet " + tos(voxMsgType) + ". Parse error.", LOGNAME, m_srcStreamID);
				SEND_AND_TRANSITION("NAK:Parse Error", voxMsgType, &protocol_uvox2Source::state_StreamDataGet);
			}
			m_specialFileType = toLower(string(contents, pos));
			if ((m_specialFileType != "intro") && (m_specialFileType != "backup"))
			{
				ELOG(m_srcLogString + "Bad uvox packet " + tos(voxMsgType) + " Special file type (" + m_specialFileType + ") is not supported", LOGNAME, m_srcStreamID);
				SEND_AND_TRANSITION("NAK:Type Error", voxMsgType, &protocol_uvox2Source::state_StreamDataGet);
			}

			contents = pos + 1;
			m_specialFileBytesExpected = atoi((const char *)contents);
			if (m_specialFileBytesExpected > gOptions.maxSpecialFileSize())
			{
				ELOG(m_srcLogString + "Bad special file size " + tos(m_specialFileBytesExpected) + ". Parse error.", LOGNAME, m_srcStreamID);
				m_specialFileBytesExpected = 0;
				SEND_AND_TRANSITION("NAK:Size Error",voxMsgType,&protocol_uvox2Source::state_StreamDataGet);
			}

			m_specialFileBytes.clear();
			if (m_specialFileBytesExpected == 0)
			{
				ILOG(m_srcLogString + "Clearing " + m_specialFileType + " file.", LOGNAME, m_srcStreamID);
				streamData::specialFileData &fd = (m_specialFileType == "intro" ? m_streamData->getIntroFile() : m_streamData->getBackupFile());
				fd.replaceData(m_specialFileBytes, m_streamData->streamUvoxDataType(),
							   m_streamData->streamBitrate(), m_streamData->streamSampleRate());
			}
			else
			{
				ILOG(m_srcLogString + "Beginning " + m_specialFileType + " file transfer of size " + tos(m_specialFileBytesExpected) + " bytes.", LOGNAME, m_srcStreamID);
			}
			SEND_AND_TRANSITION("ACK", voxMsgType, &protocol_uvox2Source::state_StreamDataGet);
		}
		else if (voxMsgType == MSG_FILE_TRANSFER_DATA)
		{
			// do not NAK or ACK this packet
			if (m_specialFileBytesExpected > 0)
			{
				int amt = ntohs(voxHdr->msgLen);
				if (amt > m_specialFileBytesExpected)
				{
					WLOG(m_srcLogString + "Received too many bytes during special file transfer for " + m_specialFileType + ". Data will be truncated", LOGNAME, m_srcStreamID);
					amt = m_specialFileBytesExpected;
				}

				const __uint8 *contents = (const __uint8 *)((&(m_inBuffer[UV2X_HDR_SIZE])));
				m_specialFileBytes.insert(m_specialFileBytes.end(), contents, contents + amt);
				m_specialFileBytesExpected -= amt;
				if (m_specialFileBytesExpected == 0)
				{
					streamData::specialFileData &fd = (m_specialFileType == "intro" ? m_streamData->getIntroFile() : m_streamData->getBackupFile());
					fd.replaceData(m_specialFileBytes, m_streamData->streamUvoxDataType(),
								   m_streamData->streamBitrate(), m_streamData->streamSampleRate());
					m_specialFileBytes.clear();
					string fileType = m_specialFileType;
					fileType[0] -= ('a' - 'A');
					ILOG(m_srcLogString + fileType + " file transfer complete.", LOGNAME, m_srcStreamID);
				}
			}
		}
		else if (voxMsgType == MSG_FLUSH_CACHED_METADATA)
		{
			ILOG(m_srcLogString + "Flush cached metadata", LOGNAME, m_srcStreamID);
			m_streamData->clearCachedMetadata();
			m_metadataAssemblyTable.clear();
			SEND_AND_TRANSITION("ACK", voxMsgType, &protocol_uvox2Source::state_StreamDataGet);
		}
		else if (voxMsgType == MSG_TERMINATE)
		{
			ILOG(m_srcLogString + "Terminate", LOGNAME, m_srcStreamID);
			throwEx<runtime_error> ("");
		}
		else
		{
			ELOG(m_srcLogString + "Unknown or out of sequence packet " + tos(voxMsgType), LOGNAME, m_srcStreamID);
			if ((voxMsgType < 0x2000) && (voxMsgType != MSG_FILE_TRANSFER_DATA))
			{
				// probably have to NAK it
				SEND_AND_TRANSITION("NAK:Unsupported packet type", voxMsgType, &protocol_uvox2Source::state_StreamDataGet);
			}
		}
	}

	NEXT_PACKET;
}
