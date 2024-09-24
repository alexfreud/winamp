#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_shoutcast2Client.h"
#include "ripList.h"
#include "stats.h"
#include "streamData.h"
#include "w3cLog.h"
#include "metadata.h"
#include "uvox2Common.h"
#include "global.h"
#include "bandwidth.h"
#include "MP3Header.h"
#include "ADTSHeader.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define LOGNAME             "DST"
#define DEBUG_LOG(...)      do { if (gOptions.shoutcast2ClientDebug()) DLOG(__VA_ARGS__); } while(0)
#define AD_DEBUG_LOG(...)   do { if (gOptions.adMetricsDebug()) DLOG(__VA_ARGS__); } while(0)

protocol_shoutcast2Client::protocol_shoutcast2Client (protocol_HTTPStyle &hs, const streamData::streamID_t streamID,
        const uniString::utf8 &hostName, const uniString::utf8 &addr,const uniString::utf8 &XFF, const bool cdnSlave)

    : protocol_shoutcastClient (hs, streamID, hostName, addr, XFF, streamData::SHOUTCAST2), m_cdnSlave(cdnSlave)
{
    DEBUG_LOG(m_clientLogString + __FUNCTION__, LOGNAME, streamID);
    m_state = &protocol_shoutcast2Client::state_AttachToStream;
    m_nextState = NULL;
}

protocol_shoutcast2Client::~protocol_shoutcast2Client() throw()
{
	cleanup("Shoutcast 2", gOptions.shoutcast2ClientDebug(), true);
}

///////////////////////////////////// W3C Logging //////////////////////////////////////////////

// create W3C entry. Entries describe the duration a client has listened to a specific
//  title. the entry is generated on a title change, or when the client disconnects
void protocol_shoutcast2Client::logW3C() throw()
{
	if (gOptions.w3cEnable())
	{
		vector<utf8::value_type> md;
		if (!m_lastMetadata.empty())
		{
			// loop through packets and reassemble. Since we put this stuff in there, we don't need to do integrity checks
			size_t total_data_size = m_lastMetadata.size();
			for (size_t x = 0; x < total_data_size;)
			{
				if ((UV2X_OVERHEAD + UV2X_META_HDR_SIZE) > (total_data_size - x))
				{
					break; // out of data
				}

				const uv2xHdr *voxHdr = (const uv2xHdr*)(&(m_lastMetadata[x]));
				const __uint8 *contents = (const __uint8 *)((&(m_lastMetadata[x])) + UV2X_HDR_SIZE);
				const __uint8* metadataContents = contents + UV2X_META_HDR_SIZE;
				const size_t metadataContentsSize = ntohs(voxHdr->msgLen) - UV2X_META_HDR_SIZE;

				if ((UV2X_OVERHEAD + UV2X_META_HDR_SIZE + metadataContentsSize) > (total_data_size - x))
				{
					break; // out of data
				}

				md.insert(md.end(), metadataContents,metadataContents + metadataContentsSize);
				x += UV2X_OVERHEAD + UV2X_META_HDR_SIZE + metadataContentsSize;
			}
		}

		utf8 md_s(md.begin(), md.end());

		// do a sanity check when trying to form the metadata if a uvox 2 connection
		// is attempted from an older Winamp client e.g. 5.54 - means a connection
		// can be made from the client without it aborting if there's no metadata.
		utf8 title;
		if (!md_s.empty())
		{
			title = metadata::get_song_title_from_3902(md_s);
		}

		doLogW3C(title);
	}
}

//////////////////////////////////////////////////////////////////////////////

void protocol_shoutcast2Client::timeSlice() throw(exception)
{
	int ret = doTimeSlice(true);
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


void protocol_shoutcast2Client::setCallback (protocol_shoutcastClient::state_t callback, protocol_shoutcastClient::state_t next)
{
    m_state = callback ? callback : m_nextState;
    m_nextState = callback ? next : NULL;
}


void protocol_shoutcast2Client::state_Close() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	m_result.done();
}

void protocol_shoutcast2Client::state_SendText() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif

	if (sendText())
	{
		m_state = m_nextState;
	}
}

// find the appropriate stream and try to attach to it
void protocol_shoutcast2Client::state_AttachToStream() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	int read_bitrate = 0;
	m_streamData = streamData::accessStream(m_streamID);
	if (!m_streamData)
	{
		if (processReject("Shoutcast 2", bandWidth::CLIENT_V2_SENT, MSG_HTTP404,
						  MSG_HTTP404_LEN, &read_bitrate, 0, true))
		{
			goto fall_through;
		}

		m_state = &protocol_shoutcast2Client::state_SendText;
		m_nextState = &protocol_shoutcast2Client::state_Close;
	}
	else
	{
fall_through:
		const utf8 movedUrl = gOptions.stream_movedUrl(m_streamID);
		if (movedUrl.empty())
		{
			// we use this to control the cdn mode so we'll only provide the
			// headers needed if it's all enabled and the client asks for it
			string cdn;
			if (isCDNMaster(m_streamID) && m_cdnSlave)
			{
				DEBUG_LOG(m_clientLogString + "CDN slave request received by master", LOGNAME, m_streamID);
				utf8 authhash = (m_streamData ? m_streamData->streamAuthhash() : "");
				if (yp2::isValidAuthhash(authhash))
				{
					utf8 key = XTEA_encipher(authhash.c_str(), authhash.size(), bob().c_str(), bob().size());
					cdn = "cdn-master:1\r\n"
						  "cdn-token:" + key.hideAsString() + "\r\n";
					m_clientType = ((streamData::source_t)(m_clientType | streamData::SC_CDN_SLAVE));
				}
				else
				{
					DEBUG_LOG(m_clientLogString + "CDN slave request not sent - invalid authhash provided", LOGNAME, m_streamID);
				}
			}

			const int add = processAdd("FLV", bandWidth::CLIENT_V2_SENT,
									   MSG_HTTP404, MSG_HTTP404_LEN, movedUrl,
									   (m_streamData ? m_streamData->streamBackupServer() : ""));
			if (add != 1)
			{
				m_state = &protocol_shoutcast2Client::state_SendText;
				m_nextState = &protocol_shoutcast2Client::state_Close;
			}
			else
			{
				utf8 pub = "0", genre = "";
				const bool isPodcast = (!m_streamData && (gOptions.getBackupLoop(m_streamID) == 1));
				if (!isPodcast)
				{
					pub = (m_streamData ? tos(m_streamData->streamPublic()) : "1");
					if (m_streamData)
					{
						if (isUserAgentRelay(toLower(m_userAgent)) && (!m_streamData->allowPublicRelay()))
						{
							pub = "0";
						}

						for (int i = 0; i < 5; i++)
						{
							if (!m_streamData->m_streamInfo.m_streamGenre[i].empty())
							{
								genre += (i ? ", " : "") + m_streamData->m_streamInfo.m_streamGenre[i];
							}
						}
					}
					// if running from a backup file then no need to set the states
					else
					{
						pub = toLower(gOptions.stream_publicServer(m_streamID));
						if (pub.empty())
						{
							pub = toLower(gOptions.publicServer());
						}
						if (pub == "always")
						{
							pub = "1";
						}
						else if (pub == "never")
						{
							pub = "0";
						}
					}
				}
				else
				{
					// TODO how do we handle podcasts for 2.x
					//		as we should somehow swap to 1.x
				}

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

				m_OKResponse = MSG_UVOX_HTTP200 +
							   "icy-pub:" + pub + "\r\n" + cdn +
							   "Ultravox-SID:" + tos(m_streamID) + "\r\n" +
							   "Ultravox-Bitrate:" + tos(m_streamData ? m_streamData->streamAvgBitrate() : (read_bitrate * 1000)) + "\r\n" +
							   "Ultravox-Samplerate:" + tos(m_streamData ? m_streamData->streamSampleRate() : 0) + "\r\n" +
							   "Ultravox-Title:" + title + "\r\n" +
							   "Ultravox-Genre:" + genre + "\r\n" +
							   "Ultravox-URL:" + (m_streamData ? m_streamData->streamURL() : (utf8)""/*"TODO"*/) + "\r\n" +
							   "Ultravox-Max-Msg:" + tos(MAX_PAYLOAD_SIZE) + "\r\n" +
							   "Ultravox-Class-Type:" + tohex(m_streamData ? m_streamData->streamUvoxDataType() : MP3_DATA) + "\r\n" +
							   (m_streamData && m_streamData->streamIsVBR() ? "Ultravox-VBR:1\r\n" : "") +
							   (gOptions.clacks() ? "X-Clacks-Overhead:GNU Terry Pratchett\r\n\r\n" : "\r\n");

				DEBUG_LOG(m_clientLogString + "Sending [" + eol() + stripWhitespace(m_OKResponse) + eol() + "]");
				m_outBuffer = m_OKResponse.c_str();
				bandWidth::updateAmount(bandWidth::CLIENT_V2_SENT, (m_outBufferSize = (int)m_OKResponse.size()));
				m_state = &protocol_shoutcast2Client::state_SendText;
				if (!m_headRequest)
				{
					m_nextState = &protocol_shoutcast2Client::state_InitiateStream;
				}
				else
				{
					m_removeClientFromStats = false;
					m_ignoreDisconnect = true;
					m_nextState = &protocol_shoutcast2Client::state_Close;
				}
				m_result.write();
				m_result.timeoutSID(m_streamID);
				m_result.run();

				// when the client is added, we get back the unique id of the connection
				// but we now check for being > 0 as we need to filter out some of the
				// YP connections from being counted as valid clients for stats, etc
				reportNewListener("Shoutcast 2");
			}
		}
		else
		{
			// if we get to here then we attempt to redirect the clients to the moved url
			// which is useful if the stream has moved hosting or it has been deprecated.
			streamMovedOrRejected("Shoutcast 2", bandWidth::CLIENT_V2_SENT, movedUrl, 2);
			m_state = &protocol_shoutcast2Client::state_SendText;
			m_nextState = &protocol_shoutcast2Client::state_Close;
		}
	}
}

void protocol_shoutcast2Client::state_SendCachedMetadata() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	if (sendDataBuffer(m_streamID, m_outBuffer, m_outBufferSize, m_clientLogString))
	{
		bool playingAlbumArt = false;

		// send intro file if we have it
		acquireIntroFile(true);

		// see if there's any albumart and attempt to send stream and then playing or whatever is present
		m_cachedMetadata = (m_streamData ? m_streamData->getSc21StreamAlbumArt(0xFFFFFFFF) : streamData::uvoxMetadata_t());
		if (m_cachedMetadata.empty())
		{
			m_cachedMetadata = m_streamData->getSc21PlayingAlbumArt(m_readPtr);
			playingAlbumArt = true;
		}

		if (m_cachedMetadata.empty())
		{
			// send intro file if we have it
			acquireIntroFile(true);
			m_state = (m_introFile.empty() ? &protocol_shoutcastClient::state_Stream : &protocol_shoutcastClient::state_SendIntroFile);
		}
		else
		{
			bandWidth::updateAmount(bandWidth::CLIENT_V2_SENT, (m_outBufferSize = (int)m_cachedMetadata.size()));
			m_outBuffer = (uniString::utf8::value_type*)&(m_cachedMetadata[0]); // slam cast so we can reuse variable
			m_state = (playingAlbumArt == false ? &protocol_shoutcast2Client::state_SendCachedStreamAlbumArt : &protocol_shoutcast2Client::state_SendCachedPlayingAlbumArt);
		}
	}
}

void protocol_shoutcast2Client::state_InitiateStream() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	resetReadPtr(true);

	m_cachedMetadata = (m_streamData ? m_streamData->getSc21Metadata(m_readPtr) : streamData::uvoxMetadata_t());

	if (m_cachedMetadata.empty())
	{
		// send intro file if we have it
		acquireIntroFile(true);
		m_state = (m_introFile.empty() ? &protocol_shoutcastClient::state_Stream : &protocol_shoutcastClient::state_SendIntro);
	}
	else
	{
		bandWidth::updateAmount(bandWidth::CLIENT_V2_SENT, (m_outBufferSize = (int)m_cachedMetadata.size()));
		m_outBuffer = (uniString::utf8::value_type*)&(m_cachedMetadata[0]); // slam cast so we can reuse variable
		m_state = &protocol_shoutcast2Client::state_SendCachedMetadata;
	}

	m_result.run();
}

void protocol_shoutcast2Client::state_SendCachedStreamAlbumArt() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__ + "sending stream albumart: " + tos(m_outBufferSize) +
			  " bytes, mime type: " + tos(m_streamData->streamAlbumArtMime()));

	if (sendDataBuffer(m_streamID, m_outBuffer, m_outBufferSize, m_clientLogString))
	{
		// if all went ok then look to send the playing album art if it is present otherwise do intro / stream data
		m_cachedMetadata = m_streamData->getSc21PlayingAlbumArt(m_readPtr);

		if (m_cachedMetadata.empty())
		{
			// send intro file if we have it
			acquireIntroFile(true);
			m_state = (m_introFile.empty() ? &protocol_shoutcastClient::state_Stream : &protocol_shoutcastClient::state_SendIntroFile);
		}
		else
		{
			bandWidth::updateAmount(bandWidth::CLIENT_V2_SENT, (m_outBufferSize = (int)m_cachedMetadata.size()));
			m_outBuffer = (uniString::utf8::value_type*)&(m_cachedMetadata[0]); // slam cast so we can reuse variable
			m_state = &protocol_shoutcast2Client::state_SendCachedPlayingAlbumArt;
		}
	}
}

void protocol_shoutcast2Client::state_SendCachedPlayingAlbumArt() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__ + "sending playing albumart: " + tos(m_outBufferSize) +
			  " bytes, mime type: " + tos(m_streamData->streamPlayingAlbumArtMime()));

	if (sendDataBuffer(m_streamID, m_outBuffer, m_outBufferSize, m_clientLogString))
	{
		// send intro file if we have it
		acquireIntroFile(true);
		m_state = (m_introFile.empty() ? &protocol_shoutcastClient::state_Stream : &protocol_shoutcastClient::state_SendIntroFile);
	}
}


void protocol_shoutcast2Client::return_403(void)
{
	protocol_shoutcastClient::return_403();
	m_state = &protocol_shoutcast2Client::state_SendText;
	m_nextState = &protocol_shoutcast2Client::state_Close;
}


const int protocol_shoutcast2Client::doFrameSync(const int type, const bool debug, const int len,
                                                    const int offset, const std::vector<__uint8>& inbuf,
                                                    const time_t /*cur_time */, const int, const unsigned int,
                                                    int &frames, bool &advert,
                                                    bool fill_remainder) throw()
{
    bool mp3;
    int end = 0;

    if (streamData::isAllowedType(type, mp3))
    {
        int last = min (len, (int)(inbuf.size() - offset));
        const unsigned char *buf;

        if (m_remainder.empty () || offset < last)
        {
            buf = &inbuf [offset];
            // DLOG ("last " + tos(last) + ", off " + tos(offset));
        }
        else
        {
            const std::vector<__uint8>::const_iterator pos = inbuf.begin();
            size_t cc = min (inbuf.size(), (size_t)len);

            cc = min (cc, (size_t)4096);
            if (cc > m_remainder.size())
                cc -= m_remainder.size();
            if (cc < 1 || cc > inbuf.size())
            {
                ILOG ("sync2, cc is " + tos (cc));
                abort();
            }
            m_remainder.insert (m_remainder.end(), inbuf.begin(), inbuf.begin() + cc);

            buf = &m_remainder [0];
            last = min (len, (int)m_remainder.size());
            fill_remainder = false;
            // DLOG ("merged remainder, now " + tos (last) + ", added " + tos(cc));
        }
        if (last > 8)
        {
            int last_size = 0;
            //double fps_limit = m_fps*2;

            fill_remainder = false;

            for (int i = 0; (i < last-8) && !iskilled();)
            {
                const uv2xHdr *voxHdr = (const uv2xHdr*)(&(buf[i]));
                const int msgLen = ntohs(voxHdr->msgLen);
                const int found = ((voxHdr->sync == UVOX2_SYNC_BYTE) &&
                        (msgLen > UV2X_OVERHEAD) &&
                        (msgLen <= MAX_PAYLOAD_SIZE) ? (msgLen + UV2X_OVERHEAD) : 0);

                // need to find frames and that the input is the correct format!
                //
                // is a bit of a pain for AAC though as we've already done the
                // rough bitrate match when the advert / intro / backup was read
                // we'll just pass things through as though the bitrate is ok...
                if ((found > 0)) // && (found <= len))
                {
                    if (!frames)
                    {
                        end = i;
                        protocol_shoutcastClient::createFrameRate(mp3, (m_streamData ? m_streamData->streamSampleRate() : 0));
                    }

                    i += (last_size = found);
                    // only count valid full-size frames
                    if (i <= last)
                    {
                        //const std::vector<__uint8>::const_iterator pos = inbuf.begin();

                        m_output.insert(m_output.end(), buf+end, buf+end+last_size);
                        end += last_size;

                        // we only want to count audio frames and not metadata
                        const __uint16 voxMsgType = ntohs(voxHdr->msgType);
                        if ((voxMsgType >= 0x7000) && (voxMsgType < 0x9000))
                        {
                            ++frames;
                        }
                        // DLOG ("frame " + tos(m_frameCount+frames) + " (" + tos(found) + ") last " + tos(last) + " end " + tos(end) + " i " + tos(i));
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
                }
                else
                {
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
                                    tos(i) + " [" + tos(found) + "]", LOGNAME, m_streamID);
                        }
                    }
                    // otherwise we just need to move on and keep
                    // looking for what is a valid starting frame
                    ++i;
                }
            }
        }
        else
            fill_remainder = true;
        if (m_remainder.empty() == false && frames)
            m_remainder.clear();
        if (fill_remainder)
        {
            const int remainder = (last - end);
            if (remainder > 0)
                m_remainder.insert(m_remainder.end(), buf + end, buf + (end + remainder));
        }
    }
    return (len - end);
}


void protocol_shoutcast2Client::setIntro (vector<__uint8> &buf, int uvoxDataType)
{
    m_introFile.clear();
    if (buf.empty())
        return;
    streamData::convertRawToUvox (m_introFile, buf, uvoxDataType, 0, 0);
}
