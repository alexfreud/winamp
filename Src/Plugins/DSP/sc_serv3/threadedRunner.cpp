#ifdef _WIN32
#include <winsock2.h>
#endif
#include <algorithm>
#include "threadedRunner.h"
#include <openssl/ssl.h>
#include "global.h"
#include "stl/stringUtils.h"
#include "services/stdServiceImpl.h"
#include "protocol_HTTPStyle.h"
#include "protocol_HTTPSource.h"
#include "protocol_shoutcastSource.h"
#include "protocol_FlashPolicyServer.h"
#include "protocol_uvox2Source.h"
#include "uvox2Common.h"
#include "banList.h"
#include "ripList.h"
#ifdef _MSC_VER
#define MSG_DONTWAIT    0
#else
#include <netinet/tcp.h>
#include <sys/resource.h>
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif
#endif

using namespace std;
using namespace stringUtil;
using namespace uniString;

#define LOGNAME "[THREADRUNNER] "
#define DEBUG_LOG(...) do { if (gOptions.threadRunnerDebug()) DLOG(__VA_ARGS__); } while (0)

#ifndef SSL_OP_NO_COMPRESSION
#define SSL_OP_NO_COMPRESSION 0
#endif

// make standard string for logging address
inline utf8 addrLogString(const utf8 &addr, const u_short port, const utf8 &xff) throw()
{
	const bool use_xff = (gOptions.useXFF() && !xff.empty());
	return (use_xff ? xff : addr) + ":" + tos(port) + (use_xff ? " (xff)" : "");
}

// make standard string for logging src address
utf8 srcAddrLogString(const utf8 &addr, const u_short port, const size_t sid) throw()
{
	return "[SRC " + addrLogString(addr, port) + (sid > 0 ? " sid=" + tos(sid) : "") + "] ";
}
// make standard string for loggin dst address
utf8 dstAddrLogString(const utf8 &addr, const u_short port, const utf8 &xff, const size_t sid) throw()
{
	return "[DST " + addrLogString(addr, port, xff) + (sid > 0 ? " sid=" + tos(sid) : "") + "] ";
}
// make standard string for logging unknown address
utf8 recvAddrLogString(const utf8 &addr, const u_short port) throw()
{
	return "[RECV " + addrLogString(addr, port) + "] ";
}
// make standard string for socket error
string socketErrString(int err) throw() { return "err=" + socketOps::errMsg(err) + "(" + tos(err) + ")"; }

static AOL_namespace::mutex sm_globalRunnerLock;
static set<threadedRunner*> sm_runners;

SSL_CTX *threadedRunner::m_sslCtx = NULL;
AOL_namespace::mutex *threadedRunner::m_sslMutexes = NULL;

static bool cmp(threadedRunner* a, threadedRunner* b) throw()
{
	return (a->sizeOfRunList() < b->sizeOfRunList());
}

bool threadedRunner::scheduleRunnable(runnable *r) throw()
{
	if (r)
	{
		stackLock sml(sm_globalRunnerLock);

		// diagnostics. Print load
		if (gOptions.threadRunnerDebug())
		{
			utf8 msg = LOGNAME;
			for (set<threadedRunner*>::const_iterator i = sm_runners.begin(); i != sm_runners.end(); ++i)
			{
				msg += ((i == sm_runners.begin() ? "Thread " : ", thread ") + (*i)->threadNumber() + " amt=" + tos((*i)->sizeOfRunList()));
			}
			DEBUG_LOG(msg);
		}

		// find least busy
		set<threadedRunner*>::const_iterator which = min_element(sm_runners.begin(), sm_runners.end(), cmp);
		if ((which != sm_runners.end()) && (*which)->addRunnable(r))
		{
			return true;
		}

		// didn't work... schedule anywhere
		DEBUG_LOG(LOGNAME "Schedule failure, trying any thread");
		for (set<threadedRunner*>::const_iterator i = sm_runners.begin(); i != sm_runners.end(); ++i)
		{
			if ((*i)->addRunnable(r))
			{
				return true;
			}
		}
	}
	return false;
}

void threadedRunner::wakeup() throw()
{
	stackLock sml(sm_globalRunnerLock);

	for (set<threadedRunner*>::const_iterator i = sm_runners.begin(); i != sm_runners.end(); ++i)
	{
		(*i)->wakeupRunnable();
	}
}

uniString::utf8 threadedRunner::getRunnabledetails() throw()
{
	stackLock sml(sm_globalRunnerLock);

	utf8 details;
	for (set<threadedRunner*>::const_iterator i = sm_runners.begin(); i != sm_runners.end(); ++i)
	{
		details += (i != sm_runners.begin() ? "<br>" : (utf8)"") + "Thread #" +
				   (*i)->threadNumber() + ": <b>" + tos((*i)->sizeOfRunList()) +
				   "</b><div style=\"padding-left:1em;\">";

		map<utf8, size_t> runners;
		(*i)->enumRunnables(runners);
		for (map<utf8, size_t>::const_iterator r = runners.begin(); r != runners.end(); ++r)
		{
			details += (r != runners.begin() ? "<br>" : "") + (*r).first + " - " + tos((*r).second);
		}
		details += "</div>";
	}
	return details;
}

threadedRunner::threadedRunner() throw() : m_stop(false), m_threadNumber((const short)(sm_runners.size() + 1))
{
	stackLock sml(sm_globalRunnerLock);
	sm_runners.insert(this);
}

threadedRunner::~threadedRunner() throw()
{
	stackLock sml(sm_globalRunnerLock);
	sm_runners.erase(this);
}

// main loop of thread
const unsigned threadedRunner::operator()() throw()
{
    unsigned result = 1;

    try
    {
        m_lock.lock();
        while (!m_stop)
        {
            m_lock.unlock();

            std::set<size_t> readSet, writeSet;
            int timeout = -1;

            // run everyone (as long as not scheduled) and get their status information, etc
            set<runnable*>::const_iterator i = m_runList.begin();
            while (i != m_runList.end())
            {
                runnable::timeSliceResult &tsr = (*i)->m_result;
                __uint64 now = time_now_ms();

                // if we're indicated as being scheduled then we
                // need to skip doing anything else and look at
                // checking if it's ok to process or not, etc
                if (tsr.m_scheduleTime)
                {
                    if (now < tsr.m_scheduleTime)
                    {
                        int time_diff = (int)(tsr.m_scheduleTime - now);
                        if (timeout == -1)
                        {
                            timeout = time_diff;
                        }
                        else
                        {
                            timeout = min(timeout, time_diff);
                        }
                        ++i;
                        continue;
                    }
                    else
                    {
                        // we clear this as we're reached time
                        // but not the rest as we want what was
                        // set to now be used in the processing
                        tsr.m_scheduleTime = 0;
                    }
                }

                // make sure to reset otherwise it gets weird but
                // we're only going to do this if we are able to
                // run the runnable now (i.e. it's not scheduled)
                tsr.reset(now);

                bool exception_occured = false;
                try
                {
                    (*i)->timeSlice();
                }
                catch (const exception &ex)
                {
                    exception_occured = true;
                    utf8 what = ex.what();
                    if (!what.empty())
                    {
                        ELOG(ex.what());
                    }
                }

                if (tsr.m_done || exception_occured)
                {
                    set<runnable*>::const_iterator to_go = i;
                    DEBUG_LOG(LOGNAME "Removing " + (*i)->name() + " [done: " + tos(tsr.m_done) + ", exception: " + tos(exception_occured) + "]");
                    removeRunnable(*to_go);
                    ++i;
                    m_runList.erase (to_go);
                    continue;
                }
                if (!tsr.m_runImmediately)
                {
                    if (!tsr.m_scheduleTime)
                    {
update_sets:
                        if (tsr.m_readSet)
                        {
                            // filter out anything with an invalid socket
                            if ((*i)->m_socket != socketOps::cINVALID_SOCKET)
                            {
                                readSet.insert(readSet.end(), (*i)->m_socket);
                            }
                            if (tsr.m_customSocket != socketOps::cINVALID_SOCKET)
                            {
                                readSet.insert(readSet.end(), tsr.m_customSocket);
                            }
                        }

                        if (tsr.m_writeSet)
                        {
                            // filter out anything with an invalid socket
                            if ((*i)->m_socket != socketOps::cINVALID_SOCKET)
                            {
                                writeSet.insert(writeSet.end(), (*i)->m_socket);
                            }
                            if (tsr.m_customSocket != socketOps::cINVALID_SOCKET)
                            {
                                writeSet.insert(writeSet.end(), tsr.m_customSocket);
                            }
                        }

                        if (tsr.m_timeout != -1)
                        {
                            if (timeout == -1)
                            {
                                timeout = tsr.m_timeout;
                            }
                            else
                            {
                                timeout = min(timeout, tsr.m_timeout);
                            }
                        }
                    }
                    else
                    {
                        // if this is to be scheduled then we'll do a
                        // quick check to see if we're already after
                        // that time and if it isn't (which is how it
                        // should be) then we'll abort, else allow it
                        // and we get the time again to account for
                        // the time it's taken to process the runnable
                        now = time_now_ms();
                        if (now < tsr.m_scheduleTime)
                        {
                            int time_diff = (int)(tsr.m_scheduleTime - now);
                            if (timeout == -1)
                            {
                                timeout = time_diff;
                            }
                            else
                            {
                                timeout = min(timeout, time_diff);
                            }
                        }
                        else
                        {
                            // we clear this as we're reached time
                            // but not the rest as we want what was
                            // set to now be used in the processing
                            tsr.m_scheduleTime = 0;
                            timeout = 50;
                            goto update_sets;
                        }
                    }
                    ++i;
                }
            } // for

            // delete the old guys, no lock required here, only we add to this set
            int released = 0;
            while (true)
            {
                set<runnable*>::const_iterator it = m_runnablesToRemove.begin();
                if (it == m_runnablesToRemove.end())
                    break;
                if (++released > 300)
                {
                    timeout &= 15;   // prevent a large stall but force a quick retry
                    break;
                }
                stlx::delete_fntr<runnable> (*it);
                m_runnablesToRemove.erase (it);
            }

            readSet.insert (m_signal.test());
            if (timeout < 0)
                timeout = 60000;
            int n = socketOps::socketSelect(readSet, writeSet, timeout);

            m_lock.lock();

            // add the new guys, requires lock as set can be added from elsewhere
            m_runList.insert(m_runnablesToAdd.begin(), m_runnablesToAdd.end());
            m_runnablesToAdd.clear();

            if (n > 0)
                m_signal.clear();
        }
        m_lock.unlock();
        result = 0;
    }
    catch (const exception &ex)
    {
        ELOG(LOGNAME  + string(ex.what()));
    }
    catch (...)
    {
        ELOG(LOGNAME "Unknown exception");
    }

    // delete runnables in run list, and those that are queued to be added
    m_lock.lock();
    for_each(m_runnablesToAdd.begin(), m_runnablesToAdd.end(), stlx::delete_fntr<runnable>);
    m_lock.unlock();

    for_each(m_runList.begin(), m_runList.end(), stlx::delete_fntr<runnable>);

    return result;
}

const size_t threadedRunner::sizeOfRunList() throw()
{
    stackLock sml(m_lock);
    const size_t result = (m_runList.size() + m_runnablesToAdd.size());
    const size_t subtr = m_runnablesToRemove.size();
    return (subtr > result ? 0 : result - subtr);
}

const bool threadedRunner::addRunnable(runnable* r) throw()
{
	if (!r)
	{
		return false;
	}

	stackLock sml(m_lock);
	if (m_stop)
	{
		return false;
	}

	m_runnablesToAdd.insert(r);
	m_signal.set();
	DEBUG_LOG(LOGNAME "Adding " + r->name() + " to thread " + tos(m_threadNumber));

	return true;
}

const bool threadedRunner::removeRunnable(runnable *r) throw()
{
	if (!r)
	{
		return false;
	}

	m_runnablesToRemove.insert(r);
	m_signal.set();
	DEBUG_LOG(LOGNAME "Removing " + r->name() + " from thread " + tos(m_threadNumber));
	return true;
}

void threadedRunner::enumRunnables(map<utf8, size_t>& runners) throw()
{
	stackLock sml(m_lock);

	for (set<runnable*>::const_iterator i = m_runList.begin(); i != m_runList.end(); ++i)
	{
		const utf8::size_type pos = (*i)->name().find((utf8)"protocol_");
		if (pos != utf8::npos)
		{
			++runners[(*i)->name().substr(pos + 9, (*i)->name().length())];
		}
		else
		{
			++runners[(*i)->name()];
		}
	}
}

void threadedRunner::wakeupRunnable() throw()
{
	if (m_lock.timedLock(1000))
	{
		m_signal.set();
		m_lock.unlock();
	}
}

void threadedRunner::stop() throw()
{
	stackLock sml(m_lock);
	m_stop = true;
	m_signal.set();
}

///////////////////////////
#ifdef LOGNAME
#undef LOGNAME
#endif
#define LOGNAME "[MICROSERVER] "

#ifdef DEBUG_LOG
#undef DEBUG_LOG
#endif
#define DEBUG_LOG(x) { if (gOptions.microServerDebug()) DLOG((x)); }

microServer::microServer(const string &listenAddr, const u_short listenPort,
						 const AllowableProtocols_t protocols,
						 const ListenTypes_t types) throw(exception)
	: m_protocols(protocols)
{
	try
	{
		m_socket = socketOps::createTCPSocketTHROW();
		#ifndef _WIN32
		{
			int bflag = 1;
			setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &bflag, sizeof(bflag));
			#if (defined PLATFORM_LINUX || defined PLATFORM_ARMv6 || defined PLATFORM_ARMv7)
			int wait = 1;
			setsockopt(m_socket, IPPROTO_TCP, TCP_DEFER_ACCEPT, &wait, sizeof(wait));
			#endif
			#ifdef PLATFORM_BSD
			struct accept_filter_arg af = {"dataready", ""};
			setsockopt(m_socket, SOL_SOCKET, SO_ACCEPTFILTER, &af, sizeof(af));
			#endif
		}
		#endif
		socketOps::bindTHROW(m_socket, listenPort, listenAddr);
		socketOps::listenTHROW(m_socket);
		socketOps::setNonblockTHROW(m_socket, true);

		bindMessage(types, listenPort);
	}
	catch (const exception &ex)
	{
		socketOps::forgetTCPSocket(m_socket);
		string error = ex.what();
		throw runtime_error(LOGNAME "Error opening port " + tos(listenPort) + " because " + toLower(error));
	}
}

void microServer::bindMessage(const ListenTypes_t types, const u_short listenPort) throw()
{
	string message = "Listening for connections on port ";

	if ((types & microServer::L_SOURCE) && (types & microServer::L_CLIENT))
	{
		message = "Listening for source and client connections on port ";
	}
	else if ((types & microServer::L_FLASH))
	{
		message = "Listening for flash policy server connection on port ";
	}
	else if ((types & microServer::L_SOURCE))
	{
		message = "Listening for legacy source connections on port ";
	}
	else if ((types & microServer::L_SOURCE2))
	{
		message = "Listening for source connections on port ";
	}
	else if ((types & microServer::L_CLIENT_ALT))
	{
		message = "Listening for client connections on alternate port ";
	}
	else if ((types & microServer::L_CLIENT))
	{
		message = "Listening for client connections on port ";
	}
	ILOG(LOGNAME + message + tos(listenPort));
}

void microServer::updateProtocols(AllowableProtocols_t protocols, ListenTypes_t types, const u_short listenPort) throw()
{
	m_protocols = protocols;
	bindMessage(types, listenPort);
}

microServer::~microServer() throw()
{
	string addr;
	u_short port = 0;
	socketOps::getsockname(m_socket, addr, port);
	socketOps::forgetTCPSocket(m_socket);
	if (!iskilled())
	{
		ELOG(LOGNAME "Unexpected stop detected for listening for connections on port " + tos(port));
		ELOG(LOGNAME "This should not happen and prevents the DNAS from working correctly.");
		ELOG(LOGNAME "DNAS restart is required. If this keeps happening, enable all debugging options and provide the logs to Shoutcast support.");
	}
	else
	{
		DEBUG_LOG(LOGNAME "Stopped listening for connections on port " + tos(port));
	}
}

void microServer::timeSlice() throw(exception)
{
	static int repeated = 0;

	// don't allow any new connections when the server is stopping
	if (!iskilled())
	{
		try
		{
			string addr;
			u_short port = 0;

			socketOps::tSOCKET newSock = socketOps::acceptTHROW(m_socket, addr, port, true);
			if (newSock != socketOps::cSOCKET_ERROR)
			{
				socketOps::getpeername(newSock, addr, port);

				string hostName = addr;
				if (gOptions.nameLookups())
				{
					if (socketOps::addressToHostName(addr, port, hostName))
					{
						hostName = addr;
					}
				}

				socketOps::setNonblockTHROW(newSock, true);
				DEBUG_LOG(LOGNAME "Connection received from " + addr + ":" + tos(port));
				threadedRunner::scheduleRunnable(new microConnection(newSock, hostName, addr, port, m_protocols));
				repeated = 0;
			}
		}
		catch (const tagged_error &ex)
		{
			ELOG(ex.what());
		}
		catch (const exception &ex)
		{
			string msg = ex.what();
			if (!msg.empty())
			{
				if (msg.find("Could not call") == 0)
				{
					// serious error, log unless repeated and delay a retry
					if ((repeated & 255) == 0)
						ELOG(LOGNAME + msg);
					++repeated;
					m_result.schedule (1000);
					return;
				}
				ELOG(LOGNAME + msg);
			}
		}

		m_result.read();
		return;
	}

	m_result.done();
}

///////////////////////////////////////////////////
microConnection::microConnection(const socketOps::tSOCKET s, const string &hostName, const string &addr,
								 const u_short port, const microServer::AllowableProtocols_t protocols) throw()
	: runnable(s), m_srcHostName(hostName),
	  m_srcAddress(addr), m_srcPort(port),
	  m_protocols(protocols)
{
}

microConnection::~microConnection() throw()
{
	socketOps::forgetTCPSocket(m_socket);
}

void microConnection::timeSlice() throw(exception)
{
	time_t cur_time;
	const int autoDumpTime = ::detectAutoDumpTimeout(cur_time, m_lastActivityTime, (recvAddrLogString(m_srcAddress, m_srcPort) +
												   "Got timeout waiting for data"), gOptions.microServerDebug());

	const int maxHeaderLineSize = gOptions.maxHeaderLineSize();
	const bool flash_policy = !!(m_protocols & P_FLASHPOLICYFILE);
	bool uvox_checked = false;
    runnable *runnable = NULL;

	char buf[MAX_MESSAGE_SIZE] = {0};
	// if we've got a 1.x source connection then only handle
	// on a per-byte basis, for everything else, try getting
	// a few bytes so we can use that as a guide on how then
	// to try to process things a bit quicker than per-byte.
    // int amt = (!(m_protocols & P_SHOUTCAST1SOURCE) ? UV2X_HDR_SIZE : 1);
    int amt = (m_ssl) ? 1 : 4096;
    ssize_t  rval = 0;

	while (true)
	{
		if (iskilled() || (size_t)amt > sizeof(buf))
		{
			m_result.done();
			return;
		}

        int flags = (m_lineBuffer.size() || m_ssl) ? 0 : MSG_PEEK;  // use PEEK initially as SSL requires bytes in the socket
        if ((rval = recv (buf, amt, flags|MSG_DONTWAIT)) < 1)
		{
			if (rval == 0)
			{
				throwEx<runtime_error>((gOptions.microServerDebug() ? (recvAddrLogString(m_srcAddress, m_srcPort) +
									   "Remote socket closed while waiting for data.") : (utf8)""));
			}
			else if (rval < 0)
			{
				rval = socketOps::errCode();
				if (rval != SOCKETOPS_WOULDBLOCK)
				{
					throwEx<runtime_error>((gOptions.microServerDebug() ? (recvAddrLogString(m_srcAddress, m_srcPort) +
										   "Socket error while waiting for data. " + socketErrString(rval)) : (utf8)""));
				}

				m_result.schedule();
				m_result.read();
				m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
				return;
			}
		}

		m_lineBuffer.insert (m_lineBuffer.end(), buf, buf + rval);
		int lineSize = (int)m_lineBuffer.size();

        if (lineSize > maxHeaderLineSize)
        {
            throwEx<runtime_error>((gOptions.microServerDebug() ? (recvAddrLogString(m_srcAddress, m_srcPort) +
                            "Protocol header line is too large - exceeds " + tos(maxHeaderLineSize) +
                            " bytes") : (utf8)""));
        }
        if (m_ssl == NULL && flags && (m_lineBuffer [0] == 0x16)) // SSLv3 / TLSv1.x ?
        {
            if (threadedRunner::m_sslCtx == NULL)
            {
                throwEx<runtime_error>((gOptions.microServerDebug() ? (recvAddrLogString(m_srcAddress, m_srcPort) +
                                       "Remote socket closed, no SSL configured.") : (utf8)""));
            }
            if (lineSize < 6)
            {
				m_result.schedule();
				m_result.read();
				m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
				return;
            }
            if ((m_lineBuffer [1] == 0x3) && (m_lineBuffer [5] == 0x1))
            {
                DLOG ("detected ssl request, checking further");
                m_ssl = SSL_new (threadedRunner::m_sslCtx);
                SSL_set_accept_state (m_ssl);
                SSL_set_fd (m_ssl, (int)m_socket);
                SSL_set_mode (m_ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER|SSL_MODE_ENABLE_PARTIAL_WRITE);
                m_lineBuffer.clear();
                continue;
            }
        }
        utf8::size_type nl = m_lineBuffer.find ((unsigned char)'\n');
        if (nl != utf8::npos)
        {
            rval = lineSize = (int)nl+1; // 0 offset
            if (flags) m_lineBuffer.erase (lineSize); // truncate line to maintain parsing
        }

        if (flags)
            ::recv (m_socket, buf, rval, MSG_DONTWAIT); // pull bytes from input, passed any PEEK requirement

        if ((lineSize > 0) && (lineSize < UV2X_HDR_SIZE) && (m_lineBuffer [lineSize - 1] != '\n'))
        {
			if (m_lineBuffer[0] == UVOX2_SYNC_BYTE)
			{
				// if it looks like it might be a uvox
				// frame then grab more on the next go
				amt = 3;
			}
			else
			{
				// no point in doing any of the checks
				// if there is not enough data to view
				// e.g. we need to see a valid newline
				amt = 1;
			}

			m_lastActivityTime = ::time(NULL);
			continue;
		}
        if ((lineSize >= UV2X_HDR_SIZE) && (uvox_checked == false))
		{
			// look at first uvox packet to see if we're running uvox 2 or uvox 2.1
			// NOTE: This is a protocol change. We need to add a new packet to 2.1 so request cipher key
			const uv2xHdr *voxhdr = reinterpret_cast<const uv2xHdr*>(m_lineBuffer.c_str());
			if ((voxhdr->sync == UVOX2_SYNC_BYTE) &&
				(ntohs(voxhdr->msgType) == (u_short)MSG_CIPHER))
			{
				const int wanted = (ntohs(voxhdr->msgLen) + UV2X_OVERHEAD);
				if (wanted == lineSize)
				{
                    // we have uvox 2.1
                    if (m_protocols & P_SHOUTCAST2SOURCE) // only if allowed
                    {
                        runnable = new protocol_uvox2Source (*this, (const __uint8 *)m_lineBuffer.c_str(), lineSize);
                    }
                    break;
				}
                amt = min(MAX_MESSAGE_SIZE, (wanted - lineSize));
                m_lastActivityTime = ::time(NULL);
                continue;
            }
            // if we've got enough and there's no sync
            // byte then there's not point to re-check.
            uvox_checked = true;
        }

		if ((lineSize > 0) && (m_lineBuffer[lineSize - 1] == '\n'))
		{
			// look at start of line, if it's a GET or POST or some standard HTTP thing, then we
			// have either a web request or a client connection request. If that is missing, then
			// we have to assume it's a shoutcast source, and we have just received the password.
			//
			// this should be enough to detect absolute and relative requests made to the server
			// if there's no / for absolute paths then we'll reject the request as a bad access.
			if ((m_lineBuffer.find((utf8)"GET /") == 0) ||
				(m_lineBuffer.find((utf8)"GET h") == 0) ||
				(m_lineBuffer.find((utf8)"POST /") == 0) ||
				(m_lineBuffer.find((utf8)"POST h") == 0) ||
				(m_lineBuffer.find((utf8)"HEAD /") == 0) ||
				(m_lineBuffer.find((utf8)"HEAD h") == 0))
			{
				if (m_protocols & (P_SHOUTCAST1CLIENT |
								   P_SHOUTCAST2CLIENT |
								   P_WEB | P_WEB_SETUP))
				{
					runnable = new protocol_HTTPStyle (*this, stripWhitespace(m_lineBuffer).hideAsString());
				}
                break;
			}
			else // assume shoutcast source, and this is the password (though do some checks to sanitise)
			{
				// and now look for invalid HTTP requests and
				// reject them as the earlier handling should
				// allow valid relative and absolute requests
				if (lineSize > 5)
				{
					if ((m_lineBuffer.find((utf8)"GET ") == 0) ||
						(m_lineBuffer.find((utf8)"POST ") == 0) ||
						(m_lineBuffer.find((utf8)"SOURCE ") == 0) ||
						(m_lineBuffer.find((utf8)"PUT ") == 0) ||
						(m_lineBuffer.find((utf8)"HEAD ") == 0))
					{
						throwEx<runtime_error>((gOptions.microServerDebug() ? (recvAddrLogString(m_srcAddress, m_srcPort) +
											   "Invalid HTTP request detected - only valid relative and absolute paths are allowed.") : (utf8)""));
					}
				}

				// if we appear to have a 'PUT' or 'SOURCE' request then we'll need to
				// do some different handling in-order to get the correct details before
				// we can then actually process the stream as a valid (icecast?) source
				if (((m_lineBuffer.find((utf8)"SOURCE ") == 0) ||
					 (m_lineBuffer.find((utf8)"PUT ") == 0)) &&
					((m_lineBuffer.find((utf8)"HTTP/1.") != utf8::npos) ||
					 (m_lineBuffer.find((utf8)"ICE/1.") != utf8::npos)))
                {
                    runnable = new protocol_HTTPSource (*this, stripWhitespace(m_lineBuffer).hideAsString());
                }
                else
                {
                    runnable = new protocol_shoutcastSource (*this, stripWhitespace(m_lineBuffer));
                }
            }
            break;
		}
		if (flash_policy && (m_lineBuffer.find((utf8)"<policy-file-request/>") == 0))
		{
			runnable = new protocol_FlashPolicyServer (m_socket, dstAddrLogString(m_srcHostName, m_srcPort));
            break;
        }
        amt = 1;
        m_lastActivityTime = ::time(NULL);
    } // while

    if (runnable)
    {
        threadedRunner::scheduleRunnable (runnable);
    }
    m_result.done();
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////

// return 0 if line is ready, or a timeout in seconds for next select call if we are still waiting
// lineBuffer and lastActivityTime are updated by this call
const bool runnable::getHTTPStyleHeaderLine(const size_t sid, utf8 &lineBuffer, const utf8 &logMsgPrefix, int maxLineLength) throw(exception)
{
	time_t cur_time;
        const int autoDumpTime = ::detectAutoDumpTimeout (cur_time, m_lastActivityTime,
                (logMsgPrefix + "Timeout waiting for data"), gOptions.microServerDebug(), sid);

        const int maxHeaderLineSize = maxLineLength > 0 ? maxLineLength : gOptions.maxHeaderLineSize();
        int count = 0;
        bool ret = true;

        while (true) 
        {
            int rval = 0;
            char buf[2] = {0};
            if ((rval = recv(buf, 1, 0x0)) < 1)
            {
                if (rval == 0)
                {
                    if (gOptions.microServerDebug())
                        ELOG (logMsgPrefix + "Remote socket closed while waiting for data.", LOGNAME, sid);
                    throwEx<runtime_error>((utf8)"");
                }
                rval = socketOps::errCode();
                if (rval != SOCKETOPS_WOULDBLOCK)
                {
                    if (gOptions.microServerDebug())
                        ELOG (logMsgPrefix + "Socket error while waiting for data. " + socketErrString(rval), LOGNAME, sid);
                    throwEx<runtime_error>((utf8)"");
                }

                // if we've read something then it's likely to be from a POST response
                if (lineBuffer.empty() == false)
                {
                    ret = false;
                    if (count) break;
                }

                // try again but wait a bit
                // so we don't overload it.
                m_result.schedule(30);
                m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
                return false;
            }
            ++count;

            lineBuffer.insert (lineBuffer.end(), buf, buf + rval);

            const int lineSize = (int)lineBuffer.size();
            if (lineSize == maxLineLength) break;
            if (lineSize > maxHeaderLineSize)
            {
                ELOG (logMsgPrefix + "Protocol header line is too large - exceeds "
                        + tos(maxHeaderLineSize) + " bytes", LOGNAME, sid);
                throwEx<runtime_error> ((utf8)"");
            }
            if ((lineSize > 0) && lineBuffer [lineSize - 1] == '\n')
            {
                break;
            }
        }
        m_result.run();
        m_lastActivityTime = ::time(NULL);
        return ret;
}

// send a hunk of data out a socket - returns true if send is complete,
// outBuffer and outBufferSize should be initially set to point to the
// data and the size of the data - these values are moved and updated.
const bool runnable::sendDataBuffer(const size_t sid, const uniString::utf8::value_type *&outBuffer,
									int &outBufferSize, const uniString::utf8 &logMsgPrefix) throw(std::exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(logMsgPrefix + __FUNCTION__ + " " + tos(outBufferSize));
#endif

	if (outBufferSize > 0) // done
	{
		time_t cur_time;
		const int autoDumpTime = ::detectAutoDumpTimeout(cur_time, m_lastActivityTime,
													   (logMsgPrefix + "Timeout waiting to send data"),
													   gOptions.microServerDebug(), sid);

			int rval = send ((const char *)outBuffer, outBufferSize, 0);
			if (rval == 0)
			{
				throwEx<std::runtime_error>((gOptions.microServerDebug() ? (logMsgPrefix +
											"Remote socket closed while sending data") :
											(uniString::utf8)""));
			}
			else if (rval < 0)
			{
				rval = socketOps::errCode();
				if (rval != SOCKETOPS_WOULDBLOCK)
				{
					throwEx<std::runtime_error>((gOptions.microServerDebug() ? (((
												  #ifdef _WIN32
												  rval == WSAECONNABORTED || rval == WSAECONNRESET
												  #else
												  rval == ECONNABORTED || rval == ECONNRESET || rval == EPIPE
												  #endif
												  ) ? (uniString::utf8)"" : logMsgPrefix +
												  "Socket error while waiting to send data. " +
												  socketErrString(rval))) : (uniString::utf8)""));
				}

				// try again but wait a bit
				// so we don't overload it.
				m_result.schedule();
				m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
				return false;
			}

			// move pointers
			outBufferSize -= rval;
			outBuffer += rval;

			// update time
			m_lastActivityTime = ::time(NULL);
			m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));

			if (outBufferSize == 0) // done
			{
				m_result.schedule();
				return true;
			}
			m_result.schedule (160);
			return false;
	}

	m_result.write();
	m_result.schedule();
	m_result.timeoutSID(sid);
	return true;
}


runnable::runnable (runnable &r) throw()
{
    m_socket = r.m_socket;
    m_ssl = r.m_ssl;
    m_lastActivityTime = ::time (NULL);
    // the following are handed off to this sub-protocol, so make sure they cannot affect them
    r.m_socket = socketOps::cINVALID_SOCKET;
    r.m_ssl = NULL;
}


ssize_t runnable::recv (void *buf, size_t len, int flags)
{
    if (m_ssl)
    {
        ssize_t bytes = SSL_read (m_ssl, buf, len);
        int code = SSL_get_error (m_ssl, bytes);
        // char err[128];

        switch (code)
        {
            case SSL_ERROR_NONE:
            case SSL_ERROR_ZERO_RETURN:
                break;
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                return -1;
            default:
                bytes = 0;
        }
        return bytes;
    }
    return (ssize_t)::recv (m_socket, (char*)buf, len, flags);
}


ssize_t runnable::send(const void *buf, size_t len, int flags)
{
    if (m_ssl)
    {
        ssize_t bytes = SSL_write (m_ssl, buf, len);
        int code = SSL_get_error (m_ssl, bytes);
        // char err[128];

        switch (code)
        {
            case SSL_ERROR_NONE:
            case SSL_ERROR_ZERO_RETURN:
                break;
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                return -1;
            default:
                return -1;
        }
        return bytes;
    }
    return (ssize_t)::send(m_socket, (char*)buf, len, flags);
}

// This pick the dump time for sources, as there is no general class for that yet, unlike listeners
int runnable::detectAutoDumpTimeout (time_t &cur_time, const size_t streamID, const utf8 &msg) throw(runtime_error)
{
    const int autoDumpTime = gOptions.stream_autoDumpSourceTime(streamID);

    cur_time = ::time(NULL);
    if ((autoDumpTime > 0) && ((cur_time - m_lastActivityTime) >= autoDumpTime))
    {
        WLOG (msg, LOGNAME, streamID);
        throwEx<runtime_error>("");
    }
    return autoDumpTime;
}


unsigned long threadedRunner::SSL_idFunction (void)
{
    return threadedRunner::getCurrentThreadID();
}

void threadedRunner::SSL_lockingFunction (int mode, int n, const char * /*file*/, int /*line*/)
{
    if (mode & CRYPTO_LOCK)
        m_sslMutexes[n].lock();
    else
        m_sslMutexes[n].unlock();
}


void threadedRunner::SSL_shutdown ()
{
#if !defined(WIN32) && OPENSSL_VERSION_NUMBER < 0x10000000
    CRYPTO_set_id_callback (NULL);
#endif
    CRYPTO_set_locking_callback (NULL);
    if (m_sslCtx)
    {
        ::SSL_CTX_free (m_sslCtx);
        m_sslCtx = NULL;
    }
    if (m_sslMutexes)
        delete [] m_sslMutexes;
    m_sslMutexes = NULL;
}

void threadedRunner::SSL_init ()
{
    SSL_load_error_strings();
    SSL_library_init ();
    utf8 cert_file = gOptions.sslCertificateFile();
    utf8 key_file = gOptions.sslCertificateKeyFile();

    do {
        if (cert_file == "") break;

        CRYPTO_set_id_callback (&threadedRunner::SSL_idFunction);
#if !defined(WIN32) && OPENSSL_VERSION_NUMBER < 0x10000000
        CRYPTO_set_locking_callback (&threadedRunner::SSL_lockingFunction);
#endif

        m_sslMutexes = new AOL_namespace::mutex [CRYPTO_num_locks()];
        if (m_sslMutexes == NULL)
            break;

        m_sslCtx = ::SSL_CTX_new (::SSLv23_server_method());

        long ssl_opts = ::SSL_CTX_get_options (m_sslCtx);
        ::SSL_CTX_set_options (m_sslCtx, ssl_opts|SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3|SSL_OP_NO_COMPRESSION);

        if (::SSL_CTX_use_certificate_chain_file (m_sslCtx, (char*)cert_file.c_str()) <= 0)
        {
            WLOG ("[MAIN] Invalid certificate file " + cert_file);
            break;
        }

        utf8 &pkfile = key_file.empty() ? cert_file : key_file;
        if (::SSL_CTX_use_PrivateKey_file (m_sslCtx, (char*)pkfile.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            WLOG ("[MAIN] Invalid private key file " + pkfile);
            break;
        }
        if (! SSL_CTX_check_private_key (m_sslCtx))
        {
            WLOG ("[MAIN] Invalid, private key does not match public key, " + pkfile);
            break;
        }
        ILOG ("[MAIN] SSL keys installed");
        return;

    } while (0);
    if (m_sslCtx)
    {
        WLOG ("[MAIN] failed to set up SSL, " + utf8(::ERR_reason_error_string (::ERR_peek_last_error())));
        ::SSL_CTX_free (m_sslCtx);
        m_sslCtx = NULL;
    }
    CRYPTO_set_id_callback (NULL);
    CRYPTO_set_locking_callback (NULL);
    if (m_sslMutexes)
        delete [] m_sslMutexes;
    m_sslMutexes = NULL;
}
