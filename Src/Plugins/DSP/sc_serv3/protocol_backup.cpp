#ifdef _WIN32
#include <winsock2.h>
#endif
#include <assert.h>
#include "protocol_backup.h"
#include "protocol_relay_shoutcast.h"
#include "protocol_relay_uvox.h"
#include "bandwidth.h"
#include "streamData.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace stringUtil;
using namespace uniString;

#define LOGNAME             "BACKUP"

#define DEBUG_LOG(...)      do { if (gOptions.relayDebug()) DLOG(__VA_ARGS__); } while (0)

protocol_backup::protocol_backup(const config::streamConfig &info, const int originalBitrate, const uniString::utf8& originalMimeType) throw()
	: m_originalMimeType(originalMimeType), m_outBuffer(0),
	  m_outBufferSize(0), m_originalBitrate(originalBitrate), m_retryCount(0),
	  m_backupWaitingToReconnect(false), m_backupSentConnectWait(false),
	  m_skip(false), m_tryRelaySource(false), m_backupReconnectStartTime(0),
	  m_state(&protocol_backup::state_Initial), m_nextState(0),
	  m_backupInfo(info), m_originalbackupInfo(info), m_redirectCount(0)
{
	m_srcPort = m_backupInfo.m_backupUrl.port();
	m_srcAddrName = m_backupInfo.m_backupUrl.server();
	m_srcURLpart = m_backupInfo.m_backupUrl.path();
	m_srcLogString = "[BACKUP " + m_srcAddrName + ":" + tos(m_srcPort) +
					 (m_srcURLpart == "/" ? "" : m_srcURLpart) +
					 " sid=" + tos(m_backupInfo.m_streamID) + "] ";

	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);
}

protocol_backup::~protocol_backup() throw()
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	socketOps::forgetTCPSocket(m_socket);
}

void protocol_backup::timeSlice() throw(exception)
{
	// check if the DNAS has stopped or that the relay hasn't been stopped as
	// otherwise this'll sit and keep making backup attempts when not needed,
	// making sure that we've given the backup at least an attempt to connect

    bool serverDown = iskilled();
    bool noEntry = false, inactive = ((streamData::isRelayActive(m_backupInfo.m_streamID, noEntry) & 8) != 8);

    if (serverDown || noEntry || inactive)
    {
        bool noEntry = false;
        streamData::setRelayActiveFlags (m_backupInfo.m_streamID, noEntry, 0, 8);

        ILOG (m_srcLogString + "Termination for source backup detected", LOGNAME, m_originalbackupInfo.m_streamID);
        m_result.done();
        return;
    }

	if (m_backupWaitingToReconnect && (::time(NULL) < m_backupReconnectStartTime))
	{
		// an error occured in the past and we are waiting for the appropriate time interval before we do anything
        m_result.schedule(350);
        return;
	}
	else
	{
		// normal running
		try
		{
			(this->*m_state)();
		}
		catch(const exception &ex)
		{
			// close socket and move into waiting state for reconnect
			socketOps::forgetTCPSocket(m_socket);

			// but first see if we've hit the retry limit (which can be set as zero to keep on going)
			++m_retryCount;
			int retryLimit = gOptions.relayConnectRetries();
			DEBUG_LOG(m_srcLogString + __FUNCTION__ + utf8(" m_retryCount:") + tos(m_retryCount) + " retryLimit:" + tos(retryLimit) + " m_tryRelaySource: " + tos(m_tryRelaySource), LOGNAME, m_originalbackupInfo.m_streamID);
			if ((retryLimit == 0) || (m_retryCount < retryLimit && retryLimit > 0))
			{
				utf8 msg = ex.what();
				if (!msg.empty())
				{
					ELOG(ex.what());
				}

				m_backupWaitingToReconnect = true;
				m_state = &protocol_backup::state_Initial;
				m_backupInfo = m_originalbackupInfo;
				m_backupReconnectStartTime = ::time(NULL) + gOptions.relayReconnectTime();
                m_result.schedule (550);
			}
			else
			{
                bool noEntry = false;
                streamData::setRelayActiveFlags (m_backupInfo.m_streamID, noEntry, 0, 8);

				throwEx<runtime_error>(m_srcLogString + ex.what());
			}
		}
	}
}

// parse out backupInfo object
void protocol_backup::state_Initial() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	// if m_retryCount is over 1 then pull the stream relay url from
	// the config details as we could be in a re-try or having done
	// a config reload and the stream relay url has then since changed
	// - this primarily aids a config change for a pending relay join
	if (m_retryCount > 0)
	{
        m_backupInfo.m_backupUrl = gOptions.stream_backupURL (m_backupInfo.m_streamID);
        m_backupInfo.m_relayUrl = gOptions.stream_relayURL (m_backupInfo.m_streamID);
	}

	m_srcAddrName = m_backupInfo.m_backupUrl.server();
	m_srcPort = m_backupInfo.m_backupUrl.port();
	m_srcURLpart = m_backupInfo.m_backupUrl.path();

	m_srcLogString = "[BACKUP " + m_srcAddrName + ":" + tos(m_srcPort) +
					 (m_srcURLpart == "/" ? "" : m_srcURLpart) +
					 " sid=" + tos(m_backupInfo.m_streamID) + "] ";

	// make sure we're not trying to run a backup which is the same relay as failed
	if (m_backupInfo.m_relayUrl.isSet())
	{
		// check on individual parts as the general url may not be formatted the same
		// as well as checking against the 'streampath' version is specified for the
		// /stream/x/ alternative (am sure someone is going to try to hack on that).
		if (!m_tryRelaySource &&
		    (m_backupInfo.m_relayUrl.server() == m_srcAddrName) &&
		    (m_backupInfo.m_relayUrl.port() == m_srcPort) &&
		    (m_backupInfo.m_relayUrl.path() == m_srcURLpart))
		{
            bool noEntry = false;
            streamData::setRelayActiveFlags (m_backupInfo.m_streamID, noEntry, 0, 8);

			ELOG(m_srcLogString + "Backup source rejected. The backup cannot be the same as the original relay source.", LOGNAME, m_originalbackupInfo.m_streamID);
			m_result.done();
			return;
		}
		else
		{
			if ((m_backupInfo.m_relayUrl.server() == m_srcAddrName) &&
			    (m_backupInfo.m_relayUrl.port() == m_srcPort))
			{
				utf8::size_type pos = (!m_srcURLpart.empty() ? m_srcURLpart.find(utf8("/stream/")) : utf8::npos);
				streamData::streamID_t streamID = 0;
				if (pos != utf8::npos)
				{
					streamID = atoi((const char *)m_srcURLpart.substr(pos + 8).c_str());
				}

				if (streamID > 0)
				{
					bool htmlPage = false;
					if (!m_tryRelaySource && streamData::getStreamIdFromPath(m_backupInfo.m_relayUrl.path(), htmlPage) == streamID)
					{
                        bool noEntry = false;
                        streamData::setRelayActiveFlags (m_backupInfo.m_streamID, noEntry, 0, 8);

						ELOG(m_srcLogString + "Backup source rejected. The backup cannot be the same as the original relay source.", LOGNAME, m_originalbackupInfo.m_streamID);
						m_result.done();
						return;
					}
				}
			}
		}
	}

	m_state = &protocol_backup::state_ResolveServer;
	m_result.run();
}

// resolve server name to numeric address	
void protocol_backup::state_ResolveServer() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	assert(m_socket == socketOps::cINVALID_SOCKET);
	m_socket = socketOps::createTCPSocketTHROW();
	socketOps::setNonblock(m_socket,true);
	m_srcAddrNumeric = socketOps::hostNameToAddress(m_srcAddrName.hideAsString(), m_srcPort);
	if (m_srcAddrNumeric.empty())
	{
        bool noEntry = false;
        streamData::setRelayActiveFlags (m_backupInfo.m_streamID, noEntry, 0, 8);

        m_result.done();
        return;
	}

	m_state = &protocol_backup::state_Connect;
	m_result.run();
}

// TCP connect	
void protocol_backup::state_Connect() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	assert(m_socket != socketOps::cINVALID_SOCKET);

	bool isSourceActive = false, relayRunning = false;
	streamData *sd = streamData::accessStream(m_backupInfo.m_streamID, isSourceActive);
	if (sd)
	{
		relayRunning = sd->isRelayStream(m_backupInfo.m_streamID);
		sd->releaseStream();
	}

	if ((isSourceActive == false) || relayRunning)
	{
		if ((m_retryCount > 0) && !m_skip)
		{
			ILOG(m_srcLogString + "Connecting to source backup [attempt #" + tos(m_retryCount+1) + "]", LOGNAME, m_originalbackupInfo.m_streamID);
		}

		m_skip = false;
		socketOps::connect(m_socket, m_srcAddrNumeric, m_srcPort);

		m_lastActivityTime = ::time(NULL);
		m_state = &protocol_backup::state_ConnectWait;
		m_result.run();
	}
	else
	{
        bool noEntry = false;
        streamData::setRelayActiveFlags (m_backupInfo.m_streamID, noEntry, 0, 8);

		m_result.done();
	}
}

// wait for connect to complete
void protocol_backup::state_ConnectWait() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	assert(m_socket != socketOps::cINVALID_SOCKET);

	time_t cur_time;
	const int autoDumpSourceTime = detectAutoDumpTimeout (cur_time, m_originalbackupInfo.m_streamID, m_srcLogString + "Timeout trying to connect");

    if (streamData::isSourceConnected (m_backupInfo.m_streamID))
    {
        bool noEntry = false;
        streamData::setRelayActiveFlags (m_backupInfo.m_streamID, noEntry, 0, 8);

        ILOG (m_srcLogString + "Original stream available, Abort trying to " +
                (m_retryCount > 0 ? "re-" : "") +
                "connect to the source backup", LOGNAME, m_originalbackupInfo.m_streamID);
        m_result.done();
        return;
    }

	// not pretty but the 'sent' checks allow this to work within the expected timeouts
	// irrespective of being run on Windows or Linux as it could just sit and do nothing
	// for a few minutes if there was no network activity on the DNAS causing there to be
	// no attempt made to re-connect to the backup source (hopefully resolved in build 56)
	string error;
	socketOps::nonBlockConnect_t connectResult = socketOps::nonBlockingConnectWait(m_socket, error);
	switch (connectResult)
	{
		case socketOps::NBC_ERROR:
		{
			throwEx<runtime_error>(m_srcLogString + error);
			break;
		}
		case socketOps::NBC_INPROGRESS:
		{
            // try again but wait a bit
            m_result.schedule(100);
            m_result.timeout((autoDumpSourceTime - (int)(cur_time - m_lastActivityTime)));
            break;
        }
        case socketOps::NBC_CONNECTED:
		{
			m_lastActivityTime = ::time(NULL);
			m_state = &protocol_backup::state_SendGreeting;
			m_result.run();
			break;
		}
		default:
		{
            ELOG(m_srcLogString + "Unknown non-blocking connect state.", LOGNAME, m_originalbackupInfo.m_streamID);
            throwEx<runtime_error>("");
        }
	}
}

void protocol_backup::state_SendGreeting() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	m_HTTPHeaders.clear();
	m_HTTPGreetingResponse.clear();

	string cdn;
	if (isCDNSlave(m_backupInfo.m_streamID))
	{
		cdn = "cdn-slave:1\r\n";
	}

	m_lineBuffer = "GET " + m_srcURLpart + " " + "HTTP/1.1\r\n" +
				   "Host:" + stripHTTPprefix(m_srcAddrName) + ":" + tos(m_srcPort) + "\r\n" +
				   "User-Agent:" + g_userAgent + " Relay" +
				   (!m_tryRelaySource ? " Backup" : "") + "\r\n" +
				   "Ultravox transport type:TCP\r\n" +
				   "Accept:*/*\r\n" +
				   "icy-metadata:1\r\n" +
				   cdn +
				   "icy-host:" + metrics::metrics_verifyDestIP(gOptions) + "\r\n\r\n";

	DEBUG_LOG(m_srcLogString + "Sending request [" + eol() + stripWhitespace(m_lineBuffer) + eol() + "]", LOGNAME, m_originalbackupInfo.m_streamID);
	m_outBuffer = &(m_lineBuffer[0]);
	bandWidth::updateAmount(bandWidth::RELAY_MISC_RECV, (m_outBufferSize = (int)m_lineBuffer.size()));

	m_lastActivityTime = ::time(NULL);
	m_state = &protocol_backup::state_Send;
	m_nextState = &protocol_backup::state_GetGreetingResponse;

	m_result.write();
	m_result.timeoutSID(m_originalbackupInfo.m_streamID);
}

// send whatever is in outBuffer
void protocol_backup::state_Send() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	if (sendDataBuffer(m_backupInfo.m_streamID, m_outBuffer, m_outBufferSize, m_srcLogString))
	{
		m_state = m_nextState;
		m_lineBuffer.clear();
	}
}

void protocol_backup::state_GetLine() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	if (getHTTPStyleHeaderLine(m_backupInfo.m_streamID, m_lineBuffer, m_srcLogString))
	{
		m_state = m_nextState;
	}
}

void protocol_backup::state_GetGreetingResponse() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	m_state = &protocol_backup::state_GetLine;
	m_nextState = &protocol_backup::state_AnalyzeGreetingResponse;

	m_lastActivityTime = ::time(NULL);
	m_result.read();
	m_result.timeoutSID(m_originalbackupInfo.m_streamID);
}

// analyze header lines in greeting response	
void protocol_backup::state_AnalyzeGreetingResponse() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	m_lastActivityTime = ::time(NULL);

	if ((int)m_HTTPHeaders.size() >= gOptions.maxHeaderLineCount())
	{
        ELOG (m_srcLogString + "Max HTTP header lines exceeded", LOGNAME, m_originalbackupInfo.m_streamID);
		throwEx<runtime_error>("");
	}

	m_lineBuffer = stripWhitespace(m_lineBuffer);
	if (m_lineBuffer.empty())
	{
		m_state = &protocol_backup::state_DetermineProtocol;
		m_result.run();
	}
	else
	{
		if (m_HTTPGreetingResponse.empty())
		{
			m_HTTPGreetingResponse = m_lineBuffer;
		}
		else
		{
			// find the colon that divides header lines into key/value fields
			utf8::size_type pos = m_lineBuffer.find(utf8(":"));
			if (pos == utf8::npos)
			{
                ELOG (m_srcLogString + "Connection rejected. Bad HTTP header string [" + m_lineBuffer + "]", LOGNAME, m_originalbackupInfo.m_streamID);
                throwEx<runtime_error>("");
			}

			utf8 key = toLower(stripWhitespace(m_lineBuffer.substr(0, pos)));
			utf8 value = stripWhitespace(m_lineBuffer.substr(pos + 1));
			// allow empty values. (for urls and what-not)
			if (key.empty())
			{
                ELOG (m_srcLogString + "Connection rejected. Bad HTTP header string [" + m_lineBuffer + "]", LOGNAME, m_originalbackupInfo.m_streamID);
                throwEx<runtime_error>("");
			}
			m_HTTPHeaders[key] = value;
		}

		m_state = &protocol_backup::state_GetLine;
		m_nextState = &protocol_backup::state_AnalyzeGreetingResponse;
		m_result.read();
		m_result.timeoutSID(m_originalbackupInfo.m_streamID);
		m_lineBuffer.clear();
	}
}

void protocol_backup::state_DetermineProtocol() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalbackupInfo.m_streamID);

	if (m_HTTPGreetingResponse.empty())
	{
        ELOG (m_srcLogString + "Empty greeting response", LOGNAME, m_originalbackupInfo.m_streamID);
        throwEx<runtime_error>("");
	}

	// parse into three fields
	utf8 s = m_HTTPGreetingResponse;

	utf8::size_type pos = (!s.empty() ? s.find(utf8(" ")) : utf8::npos);
	if (pos == utf8::npos)
	{
        ELOG (m_srcLogString + "Badly formed response line [" + m_HTTPGreetingResponse + "]", LOGNAME, m_originalbackupInfo.m_streamID);
        throwEx<runtime_error>("");
	}

	s = stripWhitespace(s.substr(pos));
	pos = (!s.empty() ? s.find(utf8(" ")) : utf8::npos);
	if (pos == utf8::npos)
	{
        ELOG (m_srcLogString + "Badly formed response line [" + m_HTTPGreetingResponse + "]", LOGNAME, m_originalbackupInfo.m_streamID);
        throwEx<runtime_error>("");
	}

	int resultCode = utf8(s.substr(0, pos)).toInt();
	if (resultCode == 200)
	{
		utf8 diag;
		for (httpHeaderMap_t::const_iterator i = m_HTTPHeaders.begin(); i != m_HTTPHeaders.end(); ++i)
		{
			diag += (*i).first + ": " + (*i).second + eol();
		}
		DEBUG_LOG(m_srcLogString + "Received headers [" + eol() + stripWhitespace(diag) + eol() + "]", LOGNAME, m_originalbackupInfo.m_streamID);

		const socketOps::tSOCKET s = m_socket;
		m_socket = socketOps::cINVALID_SOCKET;

		// changed order of this in build 19 so that all uvox2 is reported as misc/ultravox
		// and we then base things off the user-agent containing 'Ultravox/2.1'
		// we look at content-type to determine the protocol
		if ((m_HTTPHeaders["content-type"] == "misc/ultravox") &&
			(m_HTTPHeaders["server"].find(utf8("Ultravox/2.1")) != utf8::npos))
		{
			// uvox 2.1
			threadedRunner::scheduleRunnable(new protocol_relay_uvox(s, m_originalbackupInfo, m_srcAddrName, m_srcAddrNumeric,
																	 m_srcPort, (m_srcURLpart == "/" ? "" : m_srcURLpart),
																	 m_HTTPHeaders, m_originalBitrate, m_originalMimeType,
																	 !m_tryRelaySource));
		}
		else
		{
			// shoutcast
			threadedRunner::scheduleRunnable(new protocol_relay_shoutcast(s, m_originalbackupInfo, m_srcAddrName, m_srcAddrNumeric,
																		  m_srcPort, (m_srcURLpart == "/" ? "" : m_srcURLpart),
																		  m_HTTPHeaders, m_originalBitrate, m_originalMimeType,
																		  !m_tryRelaySource));
		}

		m_result.done();
	}
	else if ((resultCode >= 300) && (resultCode < 400))
	{
		utf8 location = m_HTTPHeaders["location"];

		// do we maybe have a /stream/x url and gotten a /index.html?sid=# redirect?
		// if so then we should attempt to access a relay on /stream/x/ (note end / )
		utf8::size_type pos = (!location.empty() ? location.find(utf8("/index.html?sid=")) : utf8::npos);
		if (pos != utf8::npos)
		{
			streamData::streamID_t foundID = atoi((const char *)location.substr(pos + 16).c_str());
			utf8::size_type pos2 = m_backupInfo.m_backupUrl.path().find(utf8("/stream/"));
			if (pos2 != utf8::npos)
			{
				streamData::streamID_t origID = atoi((const char *)m_backupInfo.m_backupUrl.path().substr(pos2 + 8).c_str());
				if (foundID == origID)
				{
					m_backupInfo.m_backupUrl = m_backupInfo.m_backupUrl.url() + "/";
					gOptions.setOption(utf8("streambackupurl_" + tos(m_originalbackupInfo.m_streamID)),
									   m_backupInfo.m_backupUrl.url());

					m_skip = true;
					m_state = &protocol_backup::state_Initial;
					socketOps::forgetTCPSocket(m_socket);
					m_result.run();
					WLOG(m_srcLogString + "Received an invalid redirect to `" + location + "' - trying " + m_backupInfo.m_backupUrl.url(), LOGNAME, m_originalbackupInfo.m_streamID);
					return;
				}
			}
		}

		ILOG(m_srcLogString + "Received redirect to " + location, LOGNAME, m_originalbackupInfo.m_streamID);
		if (++m_redirectCount > gOptions.maxHTTPRedirects())
		{
            WLOG (m_srcLogString + "Max redirects exceeded", LOGNAME, m_originalbackupInfo.m_streamID);
            throwEx<runtime_error>("");
		}

		m_skip = true;
		m_backupInfo.m_backupUrl = location;
		m_state = &protocol_backup::state_Initial;
		socketOps::forgetTCPSocket(m_socket);
		m_result.run();
	}
	else
	{
        WLOG (m_srcLogString + (resultCode >= 400 ?  "Source responded with error [" : "Unsupported HTTP response code [") +
                m_HTTPGreetingResponse + "]", LOGNAME, m_originalbackupInfo.m_streamID);
        throwEx<runtime_error>("");
    }
}
