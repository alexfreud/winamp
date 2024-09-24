#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_HTTPClient.h"
#include "ripList.h"
#include "stats.h"
#include "streamData.h"
#include "w3cLog.h"
#include "global.h"
#include "bandwidth.h"
#include "MP3Header.h"
#include "ADTSHeader.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define DEBUG_LOG(...)      do { if (gOptions.HTTPClientDebug()) DLOG(__VA_ARGS__); } while (0)
#define AD_DEBUG_LOG(...)   do { if (gOptions.adMetricsDebug()) DLOG(__VA_ARGS__); } while (0)

protocol_HTTPClient::protocol_HTTPClient (protocol_HTTPStyle &hs, const streamData::streamID_t streamID,
        const uniString::utf8 &hostName, const utf8 &addr, const uniString::utf8 &XFF) throw(std::exception)
    : protocol_shoutcastClient (hs, streamID, hostName, addr, XFF, streamData::HTTP)
{
    setCallback (&protocol_shoutcastClient::state_AttachToStream);
}


protocol_HTTPClient::~protocol_HTTPClient() throw()
{
	cleanup("HTTP", gOptions.HTTPClientDebug(), false, true);
}

//////////////////////////////////////////////////////////////////////////////

void protocol_HTTPClient::timeSlice() throw(exception)
{
	int ret = doTimeSlice();
	if (ret == 1)
	{
		m_state = &protocol_HTTPClient::state_Stream;
		return;
	}
	else if (ret == 2)
	{
		return;
	}

	(this->*m_state)();
}


void protocol_HTTPClient::setCallback (protocol_shoutcastClient::state_t callback, protocol_shoutcastClient::state_t next)
{
    m_state = callback ? callback : m_nextState;
    m_nextState = callback ? next : NULL;
}


void protocol_HTTPClient::state_Close() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	m_result.done();
}

void protocol_HTTPClient::state_SendText() throw(exception)
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
void protocol_HTTPClient::state_AttachToStream() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	int read_bitrate = 0, dataType = 0;
	m_streamData = streamData::accessStream(m_streamID);
	if (!m_streamData)
	{
		if (processReject("HTTP", bandWidth::CLIENT_HTTP_SENT, MSG_ICY_HTTP401,
						  MSG_ICY_HTTP401_LEN, &read_bitrate, &dataType))
		{
			goto fall_through;
		}

		m_state = &protocol_shoutcastClient::state_SendText;
		m_nextState = &protocol_shoutcastClient::state_Close;
	}
	else
	{
fall_through:
		const utf8 movedUrl = gOptions.stream_movedUrl(m_streamID);
		if (movedUrl.empty())
		{
			const int add = processAdd("HTTP", bandWidth::CLIENT_HTTP_SENT,
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
				m_OKResponse = MSG_ICY_HTTP200;
				if (!isPodcast)
				{
					utf8 title = (m_streamData ? m_streamData->streamName() : gOptions.stream_backupTitle(m_streamID));
					if (!m_streamData)
					{
						if (!gOptions.read_stream_backupTitle(m_streamID))
						{
							title = gOptions.backupTitle();
						}

						if (title.empty())
						{
							title = gOptions.stream_backupFile(m_streamID);
							if (!gOptions.read_stream_backupFile(m_streamID))
							{
								title = gOptions.backupFile();
							}

							if (!title.empty())
							{
								title = fileUtil::stripSuffix(fileUtil::stripPath(title));
							}
						}
					}

					m_OKResponse += "icy-name:" + title + "\r\n"
									"icy-genre:";
					if (m_streamData)
					{
						for (int i = 0; i < 5; i++)
						{
							if (!m_streamData->m_streamInfo.m_streamGenre[i].empty())
							{
								m_OKResponse += (i ? ", " : "") + m_streamData->m_streamInfo.m_streamGenre[i];
							}
						}
					}
					else
					{
						m_OKResponse += "Misc";
					}

					m_OKResponse += "\r\n"
									"icy-br:" + tos((m_streamData ? m_streamData->streamBitrate() : read_bitrate)) + "\r\n" +
									"icy-sr:" + tos((m_streamData ? m_streamData->streamSampleRate() : 0)) + "\r\n" +
									(m_streamData ? (m_streamData->streamIsVBR() ? "icy-vbr:1\r\n" : "") : "");
				}

				if (m_streamData)
				{
					m_OKResponse += "icy-url:" + m_streamData->streamURL() + "\r\n";

					if (isUserAgentRelay(toLower(m_userAgent)) && (!m_streamData->allowPublicRelay()))
					{
						m_OKResponse += "icy-pub:0\r\n";
					}
					else
					{
						m_OKResponse += "icy-pub:" + tos(m_streamData->streamPublic()) + "\r\n";
					}

					m_OKResponse += "content-type:" + m_streamData->streamContentType() + "\r\n";
				}
				else
				{
					utf8 path = getStreamPath(m_streamID);
					if (!path.empty() && path.find(utf8("/")) == 0)
					{
						path = path.substr(1);
					}

					m_OKResponse += "Content-Type:" + utf8(dataType == AACP_DATA ? "audio/aacp" : "audio/mpeg") + "\r\n";

					if (!isPodcast)
					{
						utf8 pub = toLower(gOptions.stream_publicServer(m_streamID));
						if (pub.empty())
						{
							pub = toLower(gOptions.publicServer());
						}
						if (pub == "always")
						{
							m_OKResponse += "icy-pub:1\r\n";
						}
						else if (pub == "never")
						{
							m_OKResponse += "icy-pub:0\r\n";
						}
					}
					else
					{
						m_OKResponse += "Content-Disposition:attachment;filename=\"" + path + "\"\r\n"
										"Content-Length:" + tos(m_backupFile.size()) + "\r\n";
					}
				}

				if (gOptions.clacks())
				{
					m_OKResponse += "X-Clacks-Overhead:GNU Terry Pratchett\r\n";
				}
//#define USE_CHUNKED
#ifdef USE_CHUNKED
				m_OKResponse += "Transfer-Encoding:chunked\r\n\r\n";
#else
				m_OKResponse += "\r\n";
#endif
				DEBUG_LOG(m_clientLogString + "Sending [" + eol() + stripWhitespace(m_OKResponse) + eol() + "]");
				m_outBuffer = m_OKResponse.c_str();
				bandWidth::updateAmount(bandWidth::CLIENT_HTTP_SENT, (m_outBufferSize = (int)m_OKResponse.size()));
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
				m_result.schedule();
				m_result.timeoutSID(m_streamID);
				m_result.run();

				// when the client is added, we get back the unique id of the connection
				// but we now check for being > 0 as we need to filter out some of the
				// YP connections from being counted as valid clients for stats, etc
				reportNewListener("HTTP");
			}
		}
		else
		{
			// if we get to here then we attempt to redirect the clients to the moved url
			// which is useful if the stream has moved hosting or it has been deprecated.
			streamMovedOrRejected("HTTP", bandWidth::CLIENT_HTTP_SENT, movedUrl, 2);
			m_state = &protocol_shoutcastClient::state_SendText;
			m_nextState = &protocol_shoutcastClient::state_Close;
		}
	}
}

void protocol_HTTPClient::state_SendIntro() throw(exception)
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

void protocol_HTTPClient::state_InitiateStream() throw(exception)
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


void protocol_HTTPClient::return_403(void)
{
	protocol_shoutcastClient::return_403();
	m_state = &protocol_shoutcastClient::state_SendText;
	m_nextState = &protocol_shoutcastClient::state_Close;
}
