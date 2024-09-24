#ifdef _WIN32
#include <winsock2.h>
#endif
#include "protocol_HTTPSource.h"
#include "protocol_backup.h"
#include "streamData.h"
#include "global.h"
#include "base64.h"
#include "bandwidth.h"
#include "MP3Header.h"
#include "ADTSHeader.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define LOGNAME             "SRC"
#define DEBUG_LOG(...)      do { if (gOptions.HTTPSourceDebug()) DLOG(__VA_ARGS__); } while (0)

protocol_HTTPSource::protocol_HTTPSource (microConnection &mc, const string &firstLine) throw(exception) :
    runnable (mc), m_srcPort(mc.m_srcPort), m_srcAddr(mc.m_srcAddress)
{
    m_srcStreamID = DEFAULT_SOURCE_STREAM;
    m_remainder = new __uint8[BUF_SIZE * 4];
    m_remainderSize = 0;
    m_denied = false;
    m_outBuffer = NULL;
    m_outBufferSize = 0;
    m_streamData = NULL;
    m_state = &protocol_HTTPSource::state_GetLine;
    m_nextState = &protocol_HTTPSource::state_AnalyzeHeaders;

	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	// to be able to do certain things later on
	// we need to determine the stream from the
	// mountpoint which is in the PUT request
	string request, url, protocolAndVersion;
	getHTTPRequestDetails(firstLine, request, url, protocolAndVersion);

	bool htmlPage = false;
	m_srcStreamID = (int)streamData::getStreamIdFromPath(url, htmlPage);
	if ((m_srcStreamID > 0) && (m_srcStreamID <= INT_MAX))
	{
		// update the log message for the read stream number
		m_srcLogString = srcAddrLogString (m_srcAddr, m_srcPort, m_srcStreamID);

		// if we have a moved stream then now we have the stream id
		// then we need to check and block the source as applicable
		utf8 movedUrl = gOptions.stream_movedUrl(m_srcStreamID);

		if (!movedUrl.empty())
		{
			m_denied = true;
			ELOG(m_srcLogString + "HTTP source rejected. Stream is configured as having moved.");

			m_outBuffer = MSG_STREAMMOVED;
			bandWidth::updateAmount(bandWidth::SOURCE_V1_SENT, (m_outBufferSize = MSG_STREAMMOVED_LEN));
			m_state = &protocol_HTTPSource::state_SendBuffer;
			m_nextState = &protocol_HTTPSource::state_CloseConnection;
			return;
		}
	}
	else
	{
		m_denied = true;
		ELOG(m_srcLogString + "Bad Stream ID (" + tos(m_srcStreamID) + "). Stream ID cannot be below 1 or above 2147483647.");

		m_outBuffer = MSG_BADSTREAMID;
		bandWidth::updateAmount(bandWidth::SOURCE_V1_SENT, (m_outBufferSize = MSG_BADSTREAMID_LEN));
		m_state = &protocol_HTTPSource::state_SendBuffer;
		m_nextState = &protocol_HTTPSource::state_CloseConnection;
	}
}

protocol_HTTPSource::~protocol_HTTPSource() throw()
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	if (m_streamData)
	{
		streamData::streamSourceLost(m_srcLogString, m_streamData, m_srcStreamID);
		m_streamData = 0;
	}

	socketOps::forgetTCPSocket(m_socket);
	forgetArray(m_remainder);

	if (!m_denied)
	{
		ILOG(m_srcLogString + "HTTP source disconnected.");
	}
}

void protocol_HTTPSource::state_AnalyzeHeaders() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	int maxHeaderLineCount = gOptions.maxHeaderLineCount();
	if ((int)m_headers.size() >= maxHeaderLineCount)
	{
		m_denied = true;
		throwEx<runtime_error>(m_srcLogString + "Max header lines exceeded");
	}

	m_lineBuffer = stripWhitespace(m_lineBuffer);
	if (m_lineBuffer.empty())
	{
		// adjust headers for titleFormat and urlFormat 
		utf8 titleFormat = gOptions.titleFormat();
		utf8 urlFormat = gOptions.urlFormat();
		if (!titleFormat.empty())
		{
			utf8::size_type pos = titleFormat.find(utf8("%s"));
			m_headers["ice-name"] = (pos == utf8::npos ? titleFormat : titleFormat.replace(pos,2,m_headers["ice-name"]));
		}

		if (!urlFormat.empty())
		{
			utf8::size_type pos = urlFormat.find(utf8("%s"));
			m_headers["ice-url"] = (pos == utf8::npos ? urlFormat : urlFormat.replace(pos,2,m_headers["ice-url"]));
		}

		// dump headers to log
		if (gOptions.HTTPSourceDebug())
		{
			for (map<utf8,utf8>::const_iterator i = m_headers.begin(); i != m_headers.end(); ++i)
			{
				DEBUG_LOG(m_srcLogString + "Source client header [" + (*i).first + ":" + (*i).second + "]");
			}
		}

		config::streamConfig stream;
		const bool found = gOptions.getStreamConfig(stream, m_srcStreamID);

		if (!found && gOptions.requireStreamConfigs())
		{
			m_denied = true;
			throwEx<runtime_error>(m_srcLogString + "HTTP source rejected. Stream " +
								   tos(m_srcStreamID) + " must be defined in config file");
		}

		// check that these bitrates are allowed (looking at both max and average values)
		int streamMaxBitrate = 0, streamMinBitrate = 0;
		const int bitrate = getStreamBitrate(m_headers) * 1000,
				  ret = gOptions.isBitrateDisallowed(m_srcStreamID, bitrate, streamMinBitrate, streamMaxBitrate);
		if (ret)
		{
			m_denied = true;
			utf8 mode = ((streamMaxBitrate == streamMinBitrate) ? "of" : (ret == 2 ? "up to" : "from"));
			throwEx<runtime_error>(m_srcLogString + "HTTP source rejected. Only bitrates " + mode + " " +
								   tos((ret == 1 ? streamMinBitrate : streamMaxBitrate) / 1000) +
								   " kbps are allowed - detected " + tos(bitrate / 1000) + " kbps.");
		}

		m_streamData = streamData::createStream(streamData::streamSetup(m_srcLogString, m_srcAddr,
												(found ? stream.m_authHash : ""), m_srcUserID, "",
												stream.m_backupUrl.url(), streamData::HTTP,
												m_srcStreamID, m_srcPort, stream.m_maxStreamUser,
												stream.m_maxStreamBitrate, stream.m_minStreamBitrate,
												stream.m_allowPublicRelay, false, getStreamSamplerate(m_headers),
												mapGet(m_headers, "icy-vbr", (bool)false), m_headers));
		if (!m_streamData)
		{
			m_denied = true;
			ELOG(m_srcLogString + "HTTP source rejected. A source is already connected.");
			m_outBuffer = MSG_STREAMINUSE;
			bandWidth::updateAmount(bandWidth::SOURCE_V1_SENT, (m_outBufferSize = MSG_STREAMINUSE_LEN));
			m_state = &protocol_HTTPSource::state_SendBuffer;
			m_nextState = &protocol_HTTPSource::state_CloseConnection;
			m_result.run();
			return;
		}

		utf8 sourceIdent = mapGet(m_headers, "user-agent", utf8());
		m_streamData->updateSourceIdent(sourceIdent);

		m_outBuffer = MSG_HTTP_VALIDPASSWORD;
		bandWidth::updateAmount(bandWidth::SOURCE_V1_SENT, (m_outBufferSize = MSG_HTTP_VALIDPASSWORD_LEN));
		m_state = &protocol_HTTPSource::state_SendBuffer;
		m_nextState = &protocol_HTTPSource::state_GetStreamData;
		m_result.run();
	}
	else
	{
		// find the colon that divides header lines into key/value fields
		utf8::size_type pos = m_lineBuffer.find(utf8(":"));
		utf8 key = toLower(stripWhitespace(m_lineBuffer.substr(0, pos)));
		if (pos == utf8::npos)
		{
			if (!key.empty() && ((key == "ice-name") || (key == "ice-url")))
			{
				// allow through ice-name and ice-url if there is
				// a titleformat and urlformat to use respectively
			}
			else
			{
				m_denied = true;
				throwEx<runtime_error>(m_srcLogString + "HTTP source connection rejected. "
									   "Bad header string [" + m_lineBuffer + "]");
			}
		}

		utf8 value = stripWhitespace(m_lineBuffer.substr(pos+1));
		if (key.empty() || value.empty())
		{
			if (!key.empty() && value.empty())
			{
				if (key == "ice-genre")
				{
					value = "Misc";
				}
				else if (((key == "ice-name") && !gOptions.titleFormat().empty()) ||
						 ((key == "ice-url") && !gOptions.urlFormat().empty()))
				{
					// allow through ice-name and ice-url if there is
					// a titleformat and urlformat to use respectively
				}
				else if (key == "ice-url")
				{
					value = "http://www.shoutcast.com";
				}
			}
			else
			{
				m_denied = true;
				throwEx<runtime_error>(m_srcLogString + "HTTP source connection rejected. "
									   "Bad header string [" + m_lineBuffer + "]");
			}
		}

		if (!key.empty())
		{
			if (key == "authorization")
			{
				if (!value.empty())
				{
					utf8 password;
					vector<utf8> vauth = tokenizer(value, (utf8::value_type)' ');
					// format " Basic xxxxxxxxx"
					vector<__uint8> va = base64::decode((vauth.size() < 2 ? "" : vauth[1]).hideAsString().c_str());
					password.insert(password.end(), va.begin(), va.end());

					utf8::size_type ppos = password.find((utf8)":");
					if (ppos != utf8::npos)
					{
						// we're looking to see if this is an updated 1.x source
						// which is able to indicate the stream # for the stream
						// so that we're able to support multiple 1.x sources so
						// we need to parse the password and extract the parts
						utf8 m_srcPassword = password;
						extractPassword(m_srcPassword, m_srcUserID, m_srcStreamID);

						// as we are a v1 source then we must adhere to the master password
						// instead of using a specific per stream password as in v2 streams
						// though we also accept connections as sid=1 so check for that too
						utf8 srcPassword = gOptions.stream_password(m_srcStreamID);
						if (srcPassword.empty())
						{
							srcPassword = gOptions.password();
						}

						if (m_srcPassword.empty() || (m_srcPassword != srcPassword))
						{
							m_denied = true;
							ELOG(m_srcLogString + "HTTP source connection denied" + (m_srcUserID.empty() ? "" : " for user (" + m_srcUserID + ")") +
								 ". " + (m_srcPassword.empty() ? "Empty password not allowed." : "Bad password: " + m_srcPassword));
							m_outBuffer = MSG_INVALIDPASSWORD;
							bandWidth::updateAmount(bandWidth::SOURCE_V1_SENT, (m_outBufferSize = MSG_INVALIDPASSWORD_LEN));
							m_state = &protocol_HTTPSource::state_SendBuffer;
							m_nextState = &protocol_HTTPSource::state_CloseConnection;
						}
						else
						{
							// if we've got a source already connected and it's not a backup
							// then it's better that we just abort processing now than later
							bool isSourceActive = false;
							streamData *sd = streamData::accessStream(m_srcStreamID, isSourceActive);
							if (sd && (isSourceActive == true) && (sd->isBackupStream(m_srcStreamID) == false))
							{
								m_denied = true;
								ELOG(m_srcLogString + "HTTP source rejected. A source is already connected.");
								m_outBuffer = MSG_STREAMINUSE;
								bandWidth::updateAmount(bandWidth::SOURCE_V1_SENT, (m_outBufferSize = MSG_STREAMINUSE_LEN));
								m_state = &protocol_HTTPSource::state_SendBuffer;
								m_nextState = &protocol_HTTPSource::state_CloseConnection;
								return;
							}
							else
							{
								ILOG(m_srcLogString + "HTTP source connection starting.");
							}

							if (sd)
							{
								sd->releaseStream();
							}
						}
					}
					else
					{
						// TODO need to send a valid rejection response for this
						m_denied = true;
						throwEx<runtime_error>(m_srcLogString + "HTTP source connection rejected. "
											   "Bad header string [" + m_lineBuffer + "]");
					}

				}
				else
				{
					m_denied = true;
					throwEx<runtime_error>(m_srcLogString + "HTTP source connection rejected. "
										   "Bad header string [" + m_lineBuffer + "]");
				}
			}
		}
		m_headers[key] = value;
		m_nextState = &protocol_HTTPSource::state_AnalyzeHeaders;
		m_state = &protocol_HTTPSource::state_GetLine;
		m_result.read();
		m_lineBuffer.clear();
	}
}

void protocol_HTTPSource::timeSlice() throw(std::exception)
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
		throw;
	}
}

void protocol_HTTPSource::state_SendBuffer() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	if (sendDataBuffer(m_srcStreamID, m_outBuffer, m_outBufferSize, m_srcLogString))
	{
		m_state = m_nextState;
	}
}

void protocol_HTTPSource::state_GetLine() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	if (getHTTPStyleHeaderLine(m_srcStreamID, m_lineBuffer, m_srcLogString))
	{
		m_state = m_nextState;
	}
}

void protocol_HTTPSource::state_GetStreamData() throw(exception)
{
	time_t cur_time;

    try
    {
        const int autoDumpTime = detectAutoDumpTimeout (cur_time, m_srcStreamID, (m_srcLogString + "Timeout waiting for stream data"));

        int bitrate = m_streamData->streamBitrate();
        const int type = m_streamData->streamUvoxDataType();
        while (true)
        {
            char buf[BUF_SIZE * 4] = {0};
            int amt = (BUF_SIZE - 1);

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
            if ((rval = recv (&buf[m_remainderSize], (BUF_SIZE - 1), 0x0)) < 1)
            {
                if (rval < 0)
                {
                    rval = socketOps::errCode();
                    if (rval == SOCKETOPS_WOULDBLOCK)
                    {
                        m_result.schedule (80);
                        m_result.read();
                        m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
                        return;
                    }
                    WLOG (m_srcLogString + "Socket error while waiting for data. " + socketErrString(rval), LOGNAME, m_srcStreamID);
                }
                else
                    DLOG (m_srcLogString + "Remote socket closed while waiting for data.", LOGNAME, m_srcStreamID);
                throwEx<runtime_error> ("");
            }

            // update these details before we mess with anything
            bandWidth::updateAmount(bandWidth::SOURCE_V1_RECV, rval);

            // if we're here then we account for what we already had in the total
            // so that we then don't skip the new data read with the original data
            rval += m_remainderSize;
            m_remainderSize = 0;
            amt = rval;

            if (m_streamData->syncToStream(m_remainderSize, m_remainder, amt, bitrate,
                        type, buf, m_srcLogString))
            {
                m_denied = true;
                throwEx<runtime_error>(m_srcLogString + "HTTP source disconnected. "
                        "Unable to sync to the stream. Please check the "
                        "source is valid and in a supported format.");
            }

            m_lastActivityTime = ::time(NULL);
        }
    }
    catch (exception &e)
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
            ILOG (m_srcLogString + "HTTP source disconnected - trying source backup.");
            threadedRunner::scheduleRunnable (new protocol_backup(backupInfo[0], getStreamBitrate(m_headers),
                        fixMimeType(m_headers["content-type"])));
#endif
        }
        throw;
    }
}

void protocol_HTTPSource::state_CloseConnection() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__);

	m_result.done();
}
