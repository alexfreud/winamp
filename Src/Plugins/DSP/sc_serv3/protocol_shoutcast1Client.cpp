#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_shoutcast1Client.h"
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

#define DEBUG_LOG(...)      do { if (gOptions.shoutcast1ClientDebug()) DLOG(__VA_ARGS__); } while (0)
#define AD_DEBUG_LOG(...)   do { if (gOptions.adMetricsDebug()) DLOG(__VA_ARGS__); } while (0)


protocol_shoutcast1Client::protocol_shoutcast1Client (protocol_HTTPStyle &hs, const streamData::streamID_t streamID,
        const uniString::utf8 &hostName, const uniString::utf8 &addr, const uniString::utf8 &XFF) throw (std::exception)

    : protocol_shoutcastClient (hs, streamID, hostName, addr, XFF, streamData::SHOUTCAST1)
{
	setCallback (&protocol_shoutcastClient::state_AttachToStream);
}

protocol_shoutcast1Client::~protocol_shoutcast1Client() throw()
{
	cleanup("Shoutcast 1", gOptions.shoutcast1ClientDebug());
}

//////////////////////////////////////////////////////////////////////////////

void protocol_shoutcast1Client::timeSlice() throw(exception)
{
	int ret = doTimeSlice();
	if (ret == 1)
	{
		m_state = &protocol_shoutcastClient::state_Stream;
		return;
	}
	else if (ret == 2)
	{
		return;
	}

	(this->*m_state)();
}

void protocol_shoutcast1Client::setCallback (protocol_shoutcastClient::state_t callback, protocol_shoutcastClient::state_t next)
{
    m_state = callback ? callback : m_nextState;
    m_nextState = callback ? next : NULL;
}


void protocol_shoutcast1Client::state_Close() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	m_result.done();
}

// find the appropriate stream and try to attach to it
void protocol_shoutcast1Client::state_AttachToStream() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	int read_bitrate = 0, dataType = 0;
	m_streamData = streamData::accessStream(m_streamID);
	if (!m_streamData)
	{
		m_outBuffer = MSG_ICY_HTTP401.c_str();
		bandWidth::updateAmount(bandWidth::CLIENT_V1_SENT, (m_outBufferSize = MSG_ICY_HTTP401_LEN));

		if (processReject("Shoutcast 1", bandWidth::CLIENT_V1_SENT, MSG_ICY_HTTP401,
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
			const int add = processAdd("Shoutcast 1", bandWidth::CLIENT_V1_SENT,
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
				m_OKResponse = (m_clientType == streamData::WMP ? MSG_ICY200 : MSG_ICY_HTTP200);
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
						//m_OKResponse += "icy-url:" + utf8("TODO") + "\r\n";

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

				// ignore setting if we've already determined we shouldn't send in-stream metadata
				// otherwise for some clients where we force disable just in-case, if it explicitly
				// includes a valid "icy-metadata" header, then see if we should override things...
				m_OKResponse += "icy-metaint:" + tos(m_metaInterval) + "\r\n";

				if (gOptions.clacks())
				{
					m_OKResponse += "X-Clacks-Overhead:GNU Terry Pratchett\r\n";
				}
				m_OKResponse += "\r\n";

				DEBUG_LOG(m_clientLogString + "Sending [" + eol() + stripWhitespace(m_OKResponse) + eol() + "]");
				m_outBuffer = m_OKResponse.c_str();
				bandWidth::updateAmount(bandWidth::CLIENT_V1_SENT, (m_outBufferSize = (int)m_OKResponse.size()));
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

				if (!m_userAgent.empty() && toLower(m_userAgent).find(utf8("shoutcast-to-dnas message sender")) == 0)
				{
					utf8::size_type pos = toLower(m_userAgent).find(utf8("[message:"));
					if (pos != utf8::npos)
					{
						utf8 message = m_userAgent.substr(pos + 9);
						pos = message.rfind(utf8("]"));
						if (pos != utf8::npos)
						{
							message = message.substr(0, pos);
						}

						if ((message.find(utf8("!doctype")) != utf8::npos) ||
							(message.find(utf8("<script")) != utf8::npos) ||
							(message.find(utf8("<html")) != utf8::npos) ||
							(message.find(utf8("<body")) != utf8::npos) ||
							(message.find(utf8("<div")) != utf8::npos) ||
							(message.find(utf8("http://")) != utf8::npos) ||
							(message.find(utf8("https://")) != utf8::npos))
						{
							WLOG("[MESSAGE sid=" + tos(m_streamID) + "] Invalid message received.");
						}
						else
						{
							WLOG("[MESSAGE sid=" + tos(m_streamID) + "] " + ((message == "<clear>") ? "Message cleared." : message));
							if (m_streamData)
							{
								m_streamData->updateStreamMessage(m_streamID, message);
							}
						}
						m_ignoreDisconnect = true;
					}
				}
				else
				{
					// setGroup(500); // for testing
					// when the client is added, we get back the unique id of the connection
					// but we now check for being > 0 as we need to filter out some of the
					// YP connections from being counted as valid clients for stats, etc
					reportNewListener("Shoutcast 1");
				}
			}
		}
		else
		{
			// if we get to here then we attempt to redirect the clients to the moved url
			// which is useful if the stream has moved hosting or it has been deprecated.
			streamMovedOrRejected("Shoutcast 1", bandWidth::CLIENT_V1_SENT, movedUrl, 2);
			m_state = &protocol_shoutcastClient::state_SendText;
			m_nextState = &protocol_shoutcastClient::state_Close;
		}
	}
}

void protocol_shoutcast1Client::state_SendIntro() throw(exception)
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

void protocol_shoutcast1Client::state_InitiateStream() throw(exception)
{
	resetReadPtr();

	// if it's a SHOUTcast Directory test connection then we send just the title (which will
	// be a default string if nothing is available) which will still allow the connection to
	// work and validate the stream exists whilst only sending the minimum of data nededed.
	if (!m_userAgent.empty() && (toLower(m_userAgent) == utf8("shoutcast directory tester")))
	{
		// TODO check if artwork needs to also be indicated
		const utf8 metadata = fixICYMetadata((m_streamData ? m_streamData->getSc1Metadata(m_readPtr).m_songTitle : ""));
		sendICYMetadata((!metadata.empty() ? metadata : "StreamTitle='';"));
		m_nextState = &protocol_shoutcastClient::state_Close;
		m_ignoreDisconnect = true;
	}
	else
	{
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
	}

	m_result.run();
}

// construct the necessary metadata information and load into outbound buffers.
void protocol_shoutcast1Client::sendICYMetadata(const utf8 &md) throw()
{
	m_ICYMetadata.clear();
	m_ICYMetadata.push_back(1); // placeholder
	if (md != m_lastSentMetadata) // don't sent duplicates
	{
		m_ICYMetadata.insert(m_ICYMetadata.end(),md.begin(),md.end());
		if (!m_lastSentMetadata.empty())
		{
			logW3C();
		}
		m_lastSentMetadata = md;
	}

	unsigned int dlen = (unsigned int)m_ICYMetadata.size();
	if (dlen == 1)
	{
		dlen = 0;
	}
	unsigned int l1 = ((dlen + 15) & ~15);
	m_ICYMetadata[0] = l1 / 16;
	unsigned int send_len = l1 + 1;
	m_ICYMetadata.insert(m_ICYMetadata.end(), send_len - m_ICYMetadata.size(), 0);
	//assert(m_ICYMetadata.size() == (size_t)((m_ICYMetadata[0] * 16) + 1));
	m_metaIntervalCounter = 0;

	m_outBuffer = &m_ICYMetadata[0];
	bandWidth::updateAmount(bandWidth::CLIENT_V1_SENT, (m_outBufferSize = (int)m_ICYMetadata.size()));
	m_nextState = m_state;
	m_state = &protocol_shoutcastClient::state_SendText;
	m_result.run();
}


int protocol_shoutcast1Client::doSend(const bool debug, const time_t cur_time, const int autoDumpTime, int adjust) throw(std::runtime_error)
{
	// we are going to do a forced short-send if we
	// are now needing to be trying to send metadata
	int size = (int)m_output.size();
	int rval = 0;

	if ((m_metaIntervalCounter + size) > m_metaInterval)
	{
		adjust = (size - ((m_metaIntervalCounter + size) - m_metaInterval));
		if (adjust)
		{
			rval = protocol_shoutcastClient::doSend(debug, cur_time, autoDumpTime, adjust);
			// DLOG ("Adjusted send ret " + tos(rval) + " for per-meta " + tos(adjust));
		}

		if (rval == adjust)
		{
			const utf8 metadata = (!m_streamData ? "StreamTitle='';" :
					fixICYMetadata((m_streamData ?
							m_streamData->getSc1Metadata(m_readPtr).m_songTitle : "")));

			sendICYMetadata(metadata);
			return rval;
		}
		size = adjust;
	}
	else
		rval = protocol_shoutcastClient::doSend(debug, cur_time, autoDumpTime);
	m_metaIntervalCounter += rval;
	if (rval < size)
		m_result.schedule(120);

	return rval;
}


void protocol_shoutcast1Client::return_403(void)
{
	protocol_shoutcastClient::return_403();
	setCallback (&protocol_shoutcastClient::state_SendText, &protocol_shoutcastClient::state_Close);
}
