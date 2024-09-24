#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_flvClient.h"
#include "ripList.h"
#include "stats.h"
#include "streamData.h"
#include "w3cLog.h"
#include "global.h"
#include "bandwidth.h"
#include "MP3Header.h"
#include "ADTSHeader.h"
#include "FLV.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define DEBUG_LOG(...)      do { if (gOptions.flvClientDebug()) DLOG(__VA_ARGS__); } while (0)
#define AD_DEBUG_LOG(...)   do { if (gOptions.adMetricsDebug()) DLOG(__VA_ARGS__); } while (0)

protocol_flvClient::protocol_flvClient (protocol_HTTPStyle &hs, const streamData::streamID_t streamID,
        const uniString::utf8 &hostName, const uniString::utf8 &addr, const uniString::utf8 &XFF) throw (std::exception)

    : protocol_shoutcastClient (hs, streamID, hostName, addr, XFF, streamData::FLV)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);
    m_timestamp = 0;
    m_fileOffset = 0;
	setCallback (&protocol_shoutcastClient::state_AttachToStream);
}

protocol_flvClient::~protocol_flvClient() throw()
{
	cleanup("FLV", gOptions.flvClientDebug(), false, true);
}

//////////////////////////////////////////////////////////////////////////////

void protocol_flvClient::timeSlice() throw(exception)
{
	int ret = doTimeSlice();
	if (ret == 1)
	{
		m_state = &protocol_flvClient::state_Stream;
		return;
	}
	else if (ret == 2)
	{
		return;
	}

	(this->*m_state)();
}


void protocol_flvClient::setCallback (protocol_shoutcastClient::state_t callback, protocol_shoutcastClient::state_t next)
{
    m_state = callback ? callback : m_nextState;
    m_nextState = callback ? next : NULL;
}


void protocol_flvClient::state_Close() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	m_result.done();
}

void protocol_flvClient::state_SendText() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif

	if (sendText())
	{
		m_metaIntervalCounter = 0;
		m_state = m_nextState;
	}
}

// find the appropriate stream and try to attach to it
void protocol_flvClient::state_AttachToStream() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	int read_bitrate = 0;
	m_streamData = streamData::accessStream(m_streamID);
	if (!m_streamData)
	{
		if (processReject("FLV", bandWidth::CLIENT_FLV_SENT, MSG_ICY_HTTP401,
						  MSG_ICY_HTTP401_LEN, &read_bitrate))
		{
			goto fall_through;
		}

		m_state = &protocol_shoutcastClient::state_SendText;
		m_nextState = &protocol_shoutcastClient::state_Close;
	}
	else
	{
fall_through:

		// for FLV, we can only support specific sample rates before we
		// have playback / compatibility issues vs what the spec wants.
		if (m_streamData)
		{
			const unsigned int samplerate = m_streamData->streamSampleRate();
			if ((m_streamData->streamUvoxDataType() == MP3_DATA) &&
				((samplerate != 44100) && (samplerate != 22050) && (samplerate != 11025)))
			{
				m_outBuffer = MSG_ICY_HTTP401.c_str();
				bandWidth::updateAmount(bandWidth::CLIENT_FLV_SENT, (m_outBufferSize = MSG_ICY_HTTP401_LEN));

				m_state = &protocol_shoutcastClient::state_SendText;
				m_nextState = &protocol_shoutcastClient::state_Close;
				m_result.write();
				m_result.timeoutSID(m_streamID);
				m_ignoreDisconnect = true;

				if (gOptions.logClients())
				{
					ELOG(m_clientLogString + "FLV client connection rejected. Stream samplerate is " +
						 sampleRateStr(samplerate) + " which is not supported for MP3-in-FLV streaming " +
						 "(only 44.1 kHz, 22.05 kHz and 11.025 kHz samplerates are allowed).");
				}

				return;
			}
		}

		const utf8 movedUrl = gOptions.stream_movedUrl(m_streamID);
		if (movedUrl.empty())
		{
			const int add = processAdd("FLV", bandWidth::CLIENT_FLV_SENT,
									   MSG_ICY_HTTP401, MSG_ICY_HTTP401_LEN, movedUrl,
									   (m_streamData ? m_streamData->streamBackupServer() : ""));
			if (add != 1)
			{
				m_state = &protocol_shoutcastClient::state_SendText;
				m_nextState = &protocol_shoutcastClient::state_Close;
			}
			else
			{
				const bool isPodcast = (!m_streamData && (gOptions.getBackupLoop(m_streamID) == 1));
				m_OKResponse = MSG_ICY_HTTP200 + "Content-Type:video/x-flv\r\n";

				if (!m_streamData)
				{
					utf8 path = getStreamPath(m_streamID);
					if (!path.empty() && path.find(utf8("/")) == 0)
					{
						path = path.substr(1);
					}

					if (isPodcast)
					{
						m_OKResponse += "Content-Disposition:attachment;filename=\"" + path + "\"\r\n"
										"Content-Length:" + tos(m_backupFile.size()) + "\r\n";
					}
				}

				if (gOptions.clacks())
				{
					m_OKResponse += "X-Clacks-Overhead:GNU Terry Pratchett\r\n";
				}
				m_OKResponse += "\r\n";

				DEBUG_LOG(m_clientLogString + "Sending [" + eol() + stripWhitespace(m_OKResponse) + eol() + "]");
				m_outBuffer = m_OKResponse.c_str();
				bandWidth::updateAmount(bandWidth::CLIENT_FLV_SENT, (m_outBufferSize = (int)m_OKResponse.size()));
				m_state = &protocol_shoutcastClient::state_SendText;
				if (!m_headRequest)
				{
					m_nextState = &protocol_shoutcastClient::state_InitiateStream;
				}
				else
				{
					m_removeClientFromStats = false;
					m_ignoreDisconnect = true;
					m_nextState = &protocol_shoutcastClient::state_Close;
				}
				m_result.write();
				m_result.timeoutSID(m_streamID);
				m_result.run();

				// when the client is added, we get back the unique id of the connection
				// but we now check for being > 0 as we need to filter out some of the
				// YP connections from being counted as valid clients for stats, etc
				reportNewListener("FLV");
			}
		}
		else
		{
			// if we get to here then we attempt to redirect the clients to the moved url
			// which is useful if the stream has moved hosting or it has been deprecated.
			streamMovedOrRejected("FLV", bandWidth::CLIENT_FLV_SENT, movedUrl, 2);
			m_state = &protocol_shoutcastClient::state_SendText;
			m_nextState = &protocol_shoutcastClient::state_Close;
		}
	}
}

void protocol_flvClient::state_SendIntro() throw(exception)
{
	state_SendIntroFile();
	if (m_introFile.empty())
	{
		acquireIntroFile();
		if (m_introFile.empty())
		{
			m_state = &protocol_shoutcastClient::state_Stream;
		}
		else
		{
			m_state = &protocol_shoutcastClient::state_SendIntroFile;
		}
	}
}

void protocol_flvClient::state_InitiateStream() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	resetReadPtr();

	if (!m_streamData || m_introFile.empty())
	{
		// send intro file if we have it
		acquireIntroFile();
		m_state = (m_introFile.empty() ? &protocol_shoutcastClient::state_Stream : &protocol_shoutcastClient::state_SendIntroFile);
	}
	else
	{
		m_state = &protocol_shoutcastClient::state_SendIntro;
	}

	setW3CState();

	m_result.run();
}


void protocol_flvClient::processFrame (int type, const unsigned char *buf, unsigned int len)
{
	bool is_mp3 = (type == MP3_DATA);
	unsigned int read_samplerate = 0;
	int read_bitrate = 0;
	bool read_mono = true;
	__uint8 asc_header[2] = {0};

	if (len < 7)
		return;
	if (is_mp3)
		getMP3FrameInfo ((char*)buf, &read_samplerate, &read_bitrate, &read_mono);
	else
		getADTSFrameInfo ((char*)buf, &read_samplerate, asc_header);

	createFLVTag (m_output, (char*)&buf [is_mp3 ? 0 : 7], len - (is_mp3 ? 0 : 7),
			m_timestamp, is_mp3, read_mono, read_samplerate, read_bitrate, asc_header, m_streamID);
}



void protocol_flvClient::return_403(void)
{
	protocol_shoutcastClient::return_403();
	m_state = &protocol_shoutcastClient::state_SendText;
	m_nextState = &protocol_shoutcastClient::state_Close;
}
