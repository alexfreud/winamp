#ifdef _WIN32
#include <winsock2.h>
#endif
#include <assert.h>
#include "protocol_relay.h"
#include "protocol_backup.h"
#include "protocol_relay_shoutcast.h"
#include "protocol_relay_uvox.h"
#include "bandwidth.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace stringUtil;
using namespace uniString;

// g_streamSourceRelayIsActive
// This is a map of sid to int, where the int is a bitmap of flags, the bits being
// 0 - registered, some runnable is working with this.
// 1 - shutdown of relay requested.
// 2 - primary relay in use
// 3 - backup runnable started

#define LOGNAME             "RELAY"
#define DEBUG_LOG(...)      do { if (gOptions.relayDebug()) DLOG(__VA_ARGS__); } while (0)

protocol_relay::protocol_relay(const config::streamConfig &info, const bool retry) throw()
	: m_redirectCount(0), m_outBuffer(0), m_outBufferSize(0), m_relayWaitingToReconnect(false),
	  m_relaySentConnectWait(false), m_retryRelay(retry), m_skip(false),
      m_backupStarted(false), m_registered(false), m_relayInfo(info),
	  m_originalRelayInfo(info), m_retryCount(0), m_relayReconnectStartTime(::time(NULL)),
	  m_state(&protocol_relay::state_Initial), m_nextState(0)
{
	streamData::setRelayActive (info.m_streamID, -1);       // make sure entry exists

	m_srcPort = m_relayInfo.m_relayUrl.port();
	m_srcAddrName = m_relayInfo.m_relayUrl.server();
	m_srcURLpart = m_relayInfo.m_relayUrl.path();
	m_srcLogString = "[RELAY " + m_srcAddrName + ":" + tos(m_srcPort) +
					 (m_srcURLpart == "/" ? "" : m_srcURLpart) +
					 " sid=" + tos(m_relayInfo.m_streamID) + "] ";

	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);
}

protocol_relay::~protocol_relay() throw()
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	socketOps::forgetTCPSocket(m_socket);
    if (m_registered)
    {
        bool noEntry = false;
        streamData::setRelayActiveFlags (m_originalRelayInfo.m_streamID, noEntry, 0, 7);
    }
}

void protocol_relay::timeSlice() throw(exception)
{
	if (iskilled())
	{
		m_result.done();
		return;
	}

    // normal running
    try
    {
        if (m_registered && m_relayWaitingToReconnect && (::time(NULL) < m_relayReconnectStartTime))
        {
            // an error occured in the past and we are waiting for the appropriate time interval before we do anything
            bool noEntry = false;
            int status = streamData::isRelayActive(m_originalRelayInfo.m_streamID, noEntry);
            if ((status & 2) == 0) // if no shutdown req
            {
                m_result.schedule(400);
                return;
            }
            DEBUG_LOG (m_srcLogString + "relay shutdown req", LOGNAME, m_originalRelayInfo.m_streamID);
            throwEx<runtime_error>("");
        }
        (this->*m_state)();
    }
    catch(const exception &ex)
    {
        // close socket and move into waiting state for reconnect
        socketOps::forgetTCPSocket(m_socket);

        // but first see if we've hit the retry limit (which can be set as zero to keep on going)
        ++m_retryCount;
        int retryLimit = gOptions.relayConnectRetries();
        bool noEntry = false;

        DEBUG_LOG(m_srcLogString + __FUNCTION__ + utf8(" m_retryCount:") + tos(m_retryCount) + " retryLimit:" + tos(retryLimit), LOGNAME, m_originalRelayInfo.m_streamID);
        int status = streamData::setRelayActiveFlags (m_originalRelayInfo.m_streamID, noEntry, 0, 4);

#if defined(_DEBUG) || defined(DEBUG)
        bool relayActive = status & 4 ? true : false;

        DLOG(m_srcLogString + __FUNCTION__ + " a: " + tos(retryLimit) +
                " " + tos(m_retryCount) + " "+tos(relayActive) + " " +
                tos(m_retryCount < retryLimit && retryLimit > 0) + " " +
                tos(((m_retryCount < retryLimit && retryLimit > 0)) && relayActive),LOGNAME, m_originalRelayInfo.m_streamID);
#endif

        if (noEntry == false && (status & 2) == 0) // is relay still configured
        {
            if ((status & 8) == 0)
                startBackupConnection ("");  // kick backup off if present, in case of repeated failure

            utf8 msg = ex.what();
            if (!msg.empty())
            {
                ELOG (ex.what(), LOGNAME, m_originalRelayInfo.m_streamID);
            }

            if (retryLimit == 0 || (m_retryCount < retryLimit && retryLimit > 0))
            {

                m_relayWaitingToReconnect = true;
                m_state = &protocol_relay::state_Initial;
                m_relayInfo = m_originalRelayInfo;
                m_relayReconnectStartTime = ::time(NULL) + gOptions.relayReconnectTime();
                m_result.schedule (550);
                DLOG (m_srcLogString + "reconnect time " + tos ((long)m_relayReconnectStartTime), LOGNAME, m_originalRelayInfo.m_streamID);
                return;
            }
        }
        else
        {
            utf8 ss = m_srcLogString + "Abort trying to ";
            if (m_retryCount > 0)
                ss += "re";
            ss += "connect to the source relay";
            ILOG (ss, LOGNAME, m_originalRelayInfo.m_streamID);
        }
        m_result.done();
    }
}

// parse out relayInfo object
void protocol_relay::state_Initial() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	// if we have a moved stream then now we have the stream id
	// then we need to check and block the source as applicable
	utf8 movedUrl = gOptions.stream_movedUrl(m_relayInfo.m_streamID);
	if (!movedUrl.empty())
	{
        ELOG (m_srcLogString + "Relay connection aborted. Stream is configured as having moved.", LOGNAME, m_originalRelayInfo.m_streamID);
		m_result.done();
		return;
	}

    bool noEntry = false;
    if (m_registered == false)
    {
        int state = streamData::setRelayActiveFlags (m_originalRelayInfo.m_streamID, noEntry, 1);
        if (state < 0)
        {
            ILOG (m_srcLogString + "waiting on other relay termination", LOGNAME, m_originalRelayInfo.m_streamID);
            streamData::setRelayActiveFlags (m_originalRelayInfo.m_streamID, noEntry, 2);  // request shutdown of other relay
            m_result.schedule (100);
            return;
        }
        m_registered = true;
    }
    int state = streamData::setRelayActiveFlags (m_originalRelayInfo.m_streamID, noEntry, 4);

    if (noEntry || (state & 2) == 2)
    {
        ILOG (m_srcLogString + "relay shutting down", LOGNAME, m_originalRelayInfo.m_streamID);
        m_result.done();
        return;
    }

	// if m_retryCount is over 1 then pull the stream relay url from
	// the config details as we could be in a re-try or having done
	// a config reload and the stream relay url has then sinc changed
	// - this primarily aids a config change for a pending relay join
	if (m_retryCount > 0)
	{
		m_relayInfo.m_relayUrl = gOptions.stream_relayURL(m_originalRelayInfo.m_streamID);
	}
    m_relayWaitingToReconnect = false;
	m_srcAddrName = m_relayInfo.m_relayUrl.server();
	m_srcPort = m_relayInfo.m_relayUrl.port();
	m_srcURLpart = m_relayInfo.m_relayUrl.path();

	m_srcLogString = "[RELAY " + m_srcAddrName + ":" + tos(m_srcPort) +
					 (m_srcURLpart == "/" ? "" : m_srcURLpart) +
					 " sid=" + tos(m_relayInfo.m_streamID) + "] ";

	m_state = &protocol_relay::state_ResolveServer;
	m_result.run();
}


// resolve server name to numeric address	
void protocol_relay::state_ResolveServer() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	assert(m_socket == socketOps::cINVALID_SOCKET);
	m_socket = socketOps::createTCPSocketTHROW();
	socketOps::setNonblock(m_socket, true);
	m_srcAddrNumeric = socketOps::hostNameToAddress(m_srcAddrName.hideAsString(), m_srcPort);
	if (m_srcAddrNumeric.empty())
	{
        ELOG (m_srcLogString + "Could not resolve host address", LOGNAME, m_originalRelayInfo.m_streamID);
        throwEx<runtime_error>("");
	}

	m_state = &protocol_relay::state_Connect;
	m_result.run();
}

// TCP connect	
void protocol_relay::state_Connect() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	assert(m_socket != socketOps::cINVALID_SOCKET);

	if (!m_skip)
    {
        utf8 ss = m_srcLogString;
        if (m_retryCount > 0)
        {
            ss += "Trying to restore connection to source relay [attempt #";
            ss += tos(m_retryCount+1) + "]";
        }
        else
            ss += "Connecting to source relay";
        ILOG (ss, LOGNAME, m_originalRelayInfo.m_streamID);
    }

	m_skip = false;
	socketOps::connectTHROW (m_socket, m_srcAddrNumeric, m_srcPort);

	m_lastActivityTime = ::time(NULL);
	m_state = &protocol_relay::state_ConnectWait;
	m_result.run();
}

// wait for connect to complete
void protocol_relay::state_ConnectWait() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	assert(m_socket != socketOps::cINVALID_SOCKET);

	time_t cur_time;
	const int autoDumpSourceTime = detectAutoDumpTimeout (cur_time, m_originalRelayInfo.m_streamID, (m_srcLogString + "Timeout trying to connect"));

    bool noEntry = false, relayActive = ((streamData::isRelayActive(m_originalRelayInfo.m_streamID, noEntry) & 6) == 4);
    if (!relayActive && !noEntry)
    {
        throwEx<runtime_error>("");
    }

	string error;
	socketOps::nonBlockConnect_t connectResult = socketOps::nonBlockingConnectWait(m_socket, error);
	switch (connectResult)
	{
		case socketOps::NBC_ERROR:
		{
            ELOG (m_srcLogString + error, LOGNAME, m_originalRelayInfo.m_streamID);
            throwEx<runtime_error>("");
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
			m_state = &protocol_relay::state_SendGreeting;
			m_result.run();
			break;
		}
		default:
		{
            ELOG (m_srcLogString + "Unknown non-blocking connect state.", LOGNAME, m_originalRelayInfo.m_streamID);
            throwEx<runtime_error>("");
		}
	}
}

void protocol_relay::state_SendGreeting() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	m_HTTPHeaders.clear();
	m_HTTPGreetingResponse.clear();

	string cdn;
	if (isCDNSlave(m_relayInfo.m_streamID))
	{
		cdn = "cdn-slave:1\r\n";
	}

	m_lineBuffer = "GET " + m_srcURLpart + " " + "HTTP/1.1\r\n" +
				   "Host:" + stripHTTPprefix(m_srcAddrName) + ":" + tos(m_srcPort) + "\r\n" +
				   "User-Agent:" + g_userAgent + " Relay\r\n" +
				   "Ultravox transport type:TCP\r\n" +
				   "Accept:*/*\r\n" +
				   "icy-metadata:1\r\n" +
				   cdn +
				   "icy-host:" + metrics::metrics_verifyDestIP(gOptions) + "\r\n\r\n";

	DEBUG_LOG(m_srcLogString + "Sending request [" + eol() + stripWhitespace(m_lineBuffer) + eol() + "]", LOGNAME, m_originalRelayInfo.m_streamID);
	m_outBuffer = &(m_lineBuffer[0]);
	bandWidth::updateAmount(bandWidth::RELAY_MISC_RECV, (m_outBufferSize = (int)m_lineBuffer.size()));

	m_lastActivityTime = ::time(NULL);
	m_state = &protocol_relay::state_Send;
	m_nextState = &protocol_relay::state_GetGreetingResponse;

	m_result.write();
	m_result.timeoutSID(m_originalRelayInfo.m_streamID);
}

// send whatever is in outBuffer
void protocol_relay::state_Send() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	if (sendDataBuffer(m_relayInfo.m_streamID, m_outBuffer, m_outBufferSize, m_srcLogString))
	{
		m_state = m_nextState;
		m_lineBuffer.clear();
	}
}

void protocol_relay::state_GetLine() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	if (getHTTPStyleHeaderLine(m_relayInfo.m_streamID, m_lineBuffer, m_srcLogString))
	{
		m_state = m_nextState;
	}
}

void protocol_relay::state_GetGreetingResponse() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	m_state = &protocol_relay::state_GetLine;
	m_nextState = &protocol_relay::state_AnalyzeGreetingResponse;

	m_lastActivityTime = ::time(NULL);
	m_result.read();
	m_result.timeoutSID(m_originalRelayInfo.m_streamID);
}

// analyze header lines in greeting response	
void protocol_relay::state_AnalyzeGreetingResponse() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	m_lastActivityTime = ::time(NULL);

	if ((int)m_HTTPHeaders.size() >= gOptions.maxHeaderLineCount())
	{
		throwEx<runtime_error>(m_srcLogString + "Max HTTP header lines exceeded");
	}

	m_lineBuffer = stripWhitespace(m_lineBuffer);
	if (m_lineBuffer.empty())
	{
		m_state = &protocol_relay::state_DetermineProtocol;
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
				throwEx<runtime_error>(m_srcLogString + "Connection rejected. Bad HTTP header string [" + m_lineBuffer + "]");
			}

			utf8 key = toLower(stripWhitespace(m_lineBuffer.substr(0, pos)));
			utf8 value = stripWhitespace(m_lineBuffer.substr(pos + 1));
			// allow empty values. (for urls and what-not)
			if (key.empty())
			{
				throwEx<runtime_error>(m_srcLogString + "Connection rejected. Bad HTTP header string [" + m_lineBuffer + "]");
			}
			m_HTTPHeaders[key] = value;
		}

		m_state = &protocol_relay::state_GetLine;
		m_nextState = &protocol_relay::state_AnalyzeGreetingResponse;
		m_result.read();
		m_result.timeoutSID(m_originalRelayInfo.m_streamID);
		m_lineBuffer.clear();
	}
}

void protocol_relay::state_DetermineProtocol() throw(exception)
{
	DEBUG_LOG(m_srcLogString + __FUNCTION__, LOGNAME, m_originalRelayInfo.m_streamID);

	if (m_HTTPGreetingResponse.empty())
	{
		throwEx<runtime_error>(m_srcLogString + "Empty greeting response");
	}

	// parse into three fields
	utf8 s = m_HTTPGreetingResponse;

	utf8::size_type pos = (!s.empty() ? s.find(utf8(" ")) : utf8::npos);
	if (pos == utf8::npos)
	{
		ELOG (m_srcLogString + "Badly formed response line [" + m_HTTPGreetingResponse + "]", LOGNAME, m_originalRelayInfo.m_streamID);
		throwEx<runtime_error>("");
	}

	s = stripWhitespace(s.substr(pos));
	pos = (!s.empty() ? s.find(utf8(" ")) : utf8::npos);
	if (pos == utf8::npos)
	{
        ELOG (m_srcLogString + "Badly formed response line [" + m_HTTPGreetingResponse + "]", LOGNAME, m_originalRelayInfo.m_streamID);
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
		DEBUG_LOG(m_srcLogString + "Received headers [" + eol() + stripWhitespace(diag) + eol() + "]", LOGNAME, m_originalRelayInfo.m_streamID);

		const socketOps::tSOCKET s = m_socket;

		m_result.done();

		// changed order of this in build 19 so that all uvox2 is reported as misc/ultravox
		// and we then base things off the user-agent containing 'Ultravox/2.1'
		// we look at content-type to determine the protocol
		if ((m_HTTPHeaders["content-type"] == "misc/ultravox") &&
			(m_HTTPHeaders["server"].find(utf8("Ultravox/2.1")) != utf8::npos))
		{
			// uvox 2.1
			threadedRunner::scheduleRunnable(new protocol_relay_uvox(s, m_originalRelayInfo, m_srcAddrName, m_srcAddrNumeric,
																	 m_srcPort, (m_srcURLpart == "/" ? "" : m_srcURLpart), m_HTTPHeaders));
		}
		else
		{
			// shoutcast
			threadedRunner::scheduleRunnable(new protocol_relay_shoutcast(s, m_originalRelayInfo, m_srcAddrName, m_srcAddrNumeric,
																		  m_srcPort, (m_srcURLpart == "/" ? "" : m_srcURLpart), m_HTTPHeaders));
		}
        m_socket = socketOps::cINVALID_SOCKET;
        m_backupStarted = false; // any backup client should terminate automatically
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
			utf8::size_type pos2 = m_relayInfo.m_relayUrl.path().find(utf8("/stream/"));
			if (pos2 != utf8::npos)
			{
				streamData::streamID_t origID = atoi((const char *)m_relayInfo.m_relayUrl.path().substr(pos2 + 8).c_str());
				if (foundID == origID)
				{
					m_relayInfo.m_relayUrl = m_relayInfo.m_relayUrl.url() + "/";
					gOptions.setOption(utf8("streamrelayurl_"+tos(m_originalRelayInfo.m_streamID)),
									   m_relayInfo.m_relayUrl.url());

					m_skip = true;
					m_state = &protocol_relay::state_Initial;
					socketOps::forgetTCPSocket(m_socket);
					m_result.run();
					WLOG(m_srcLogString + "Received an invalid redirect to `" + location + "' - trying " + m_relayInfo.m_relayUrl.url(), LOGNAME, m_originalRelayInfo.m_streamID);
					return;
				}
			}
		}

		WLOG(m_srcLogString + "Received redirect to " + location, LOGNAME, m_originalRelayInfo.m_streamID);
		if (++m_redirectCount > gOptions.maxHTTPRedirects())
		{
            ELOG ("Max redirects exceeded", LOGNAME, m_originalRelayInfo.m_streamID);
            throwEx<runtime_error>("");
		}

		m_skip = true;
		m_relayInfo.m_relayUrl = location;
		m_state = &protocol_relay::state_Initial;
		socketOps::forgetTCPSocket(m_socket);
		m_result.run();
	}
	else
	{
        ELOG ((resultCode >= 400 ? "Source responded with error [" : "Unsupported HTTP response code [") + m_HTTPGreetingResponse + "]");
        throwEx<runtime_error>("");
	}
}

#ifdef INCLUDE_BACKUP_STREAMS
void protocol_relay::startBackupConnection(uniString::utf8 errorMessage) throw(exception)
{
    if (!m_relayInfo.m_backupUrl.url().empty() && m_backupStarted == false)
    {
        if (errorMessage.empty() == false)
            ELOG(m_srcLogString + errorMessage, LOGNAME, m_originalRelayInfo.m_streamID);
        // this should pass through the bitrate of the original stream but as we do not
        // know what it is then we have to effectively force through to allow the backup

        bool noEntry = false;
        streamData::setRelayActiveFlags (m_originalRelayInfo.m_streamID, noEntry, 8);
        threadedRunner::scheduleRunnable(new protocol_backup(m_relayInfo, 0, ""));
        m_backupStarted = true;
        return;
    }
}
#else
void protocol_relay::startBackupConnection(uniString::utf8) throw(exception)
{
}
#endif
