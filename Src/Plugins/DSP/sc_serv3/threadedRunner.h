#pragma once
#ifndef threadedRunner_H_
#define threadedRunner_H_

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "threading/thread.h"
#include "webNet/socketOps.h"
#include "unicode/uniString.h"
#include "stl/stringUtils.h"
#include <set>
#include <queue>
#include <map>
#include <stdexcept>
#include "global.h"

#ifdef _MSC_VER
#define ssize_t int
#endif
/*

threadedRunner - (and everything that is related to it).

Based my work on the webNet module and the microServer class (in sc_trans), it
seems to me that there is a generic model that underlies all of this. At the top level
is a thread which manages a pool of objects (runnables). These runnables offer up
sets of sockets which the threadRunner incorporates into a select() call. When the select
exists, the runnables associated with the sockets are called.

This makes good sense for a server like senario, where you have connections that must
wait for read/write ability on a socket. This can, however, be extended to any object
that must wait for something. Using a pipe, you can simulate wait events that can also
offer up a socket like object. You can then create an object that fits this model that
is waiting for messages or some other type of signal.

*/

// class that has a mutex's interface but does nothing
class nullLock { public: static void lock() throw(){} static void unlock() throw(){}};

// implements a signal based on a pipe. Access to set/clear should be locked,
// but since this class is usually included in another class that already has a lock, we
// make the lock optional by templatizing it. Pass AOL_namespace::mutex as the LOCKABLE parameter
// if you want locking, otherwise pass nullLock.
#pragma pack(push, 1)
template<typename LOCKABLE>
class pipeDrivenSignal
{
private:
	int					m_signalPipe[3];
	LOCKABLE			m_lock;

public:
	pipeDrivenSignal() throw(std::exception)
	{
		m_signalPipe[0] = -1;
		m_signalPipe[1] = -1;
		m_signalPipe[2] = 0;
		if (pgpipe(m_signalPipe))
		{
			if (socketOps::errCode())
			{
				#ifdef _WIN32
				throw std::runtime_error("[MICROSERVER] Could not create signal pipe "
										 "[Too many file handles have been opened, "
										 "code: " + stringUtil::tos(socketOps::errCode()) +
										 " - reason: " + socketOps::errMsg() + "]");
				#else
				throw std::runtime_error("[MICROSERVER] Could not create signal pipe "
										 "[Increase the open files (ulimit -n) limit, "
										 "code: " + stringUtil::tos(socketOps::errCode()) +
										 " - reason: " + socketOps::errMsg() + "]");
				#endif
			}
			else
			{
				throw std::runtime_error("");
			}
		}
	        socketOps::setNonblock (m_signalPipe[0], true);
	        socketOps::setNonblock (m_signalPipe[1], true);
	}

	~pipeDrivenSignal() throw()
	{
		if (m_signalPipe[0] != -1)
		{
			pipeclose(m_signalPipe[0]);
			m_signalPipe[0] = -1;
		}

		if (m_signalPipe[1] != -1)
		{
			pipeclose(m_signalPipe[1]);
			m_signalPipe[1] = -1;
		}

		m_signalPipe[2] = 0;
	}

	// set the signal
	void set() throw()
	{
		if (m_signalPipe[1] != -1)
		{
			static char buf[2] = {19, 64}; // any data will do
			pipewrite(m_signalPipe[1], buf, 1);
		}
	}

	// clear the signal
	void clear() throw()
	{
		if (m_signalPipe[0] != -1)
		{
			// clear pipe
			char buf[35];
			piperead(m_signalPipe[0], buf, sizeof(buf));
		}
	}

	// return file descriptor to test with select(). This is a read descriptor
	const int test() const throw()
	{
		return m_signalPipe[0];
	}
};
#pragma pack(pop)

// make standard string for socket error
extern std::string socketErrString(int err) throw();

// abstract base for classes than can be run by the threadRunner class
class runnable
{
	friend class threadedRunner;
public:
	#pragma pack(push, 1)
	struct timeSliceResult
	{
		bool				m_readSet;			// read socket for poll(..) / select(..) call
		bool				m_writeSet;			// write socket for poll(..) / select(..) call
		bool				m_done;				// runnable is done and can be dispose
		bool				m_runImmediately;	// must be re-run immediately
		int					m_timeout;			// must be run after an interval of time regardless. if zero, then no timeout
												// if not set (-1) then will pick an appropriate timeout to keep it ticking over
		__uint64			m_scheduleTime;		// time from which we're going to allow this to run. if zero thnn no scheduling
		__uint64			m_currentTime;		// time at which the runnable is being tried

		socketOps::tSOCKET	m_customSocket;		// used for providing a custom case if needed

		timeSliceResult() throw() : m_readSet(false), m_writeSet(false), m_done(false),
									m_runImmediately(false), m_timeout(-1),
									m_scheduleTime(0), m_currentTime(0),
									m_customSocket(socketOps::cINVALID_SOCKET) {}

		~timeSliceResult() throw()
		{
			reset(0);
		}

		void reset(__uint64 current_ms) throw()
		{
			m_customSocket = socketOps::cINVALID_SOCKET;
			m_readSet = false;
			m_writeSet = false;
			m_done = false;
			m_runImmediately = false;
			m_timeout = -1;
			m_scheduleTime = 0;
			m_currentTime = current_ms;
		}

		void done() throw()
		{
			m_done = true;
		}

		void run() throw()
		{
			m_runImmediately = true;
		}

		void read(socketOps::tSOCKET customSocket = socketOps::cINVALID_SOCKET) throw()
		{
			m_readSet = true;
			m_customSocket = customSocket;
		}

		void write() throw()
		{
			m_writeSet = true;
		}

		const int timeout(int sec, int ms_sec = 0)
		{
			m_timeout = (sec > 0 ? (sec * 1000) : 0) +
						 (ms_sec > 0 ? ms_sec : 0);
			return sec;
		}

		const int timeoutSID(const size_t sid = 1)
		{
			int sec = gOptions.getAutoDumpTime(sid);
			m_timeout = (sec > 0 ? (sec * 1000) : 0);
			return sec;
		}

		const __uint64 schedule(int ms_sec = 10)
		{
			// get the current time and schedule on from
			// there so the usage just adds the duration
			return (m_scheduleTime = time_now_ms() + (ms_sec > 0 ? ms_sec : 0));
		}
	};
	#pragma pack(pop)

	virtual void timeSlice() throw(std::exception) = 0; // you override this to take action
	virtual uniString::utf8	name() const throw() = 0;	// object name for diagnostics

    explicit runnable (runnable &r) throw();

    explicit runnable(socketOps::tSOCKET socket = socketOps::cINVALID_SOCKET) throw()
        : m_ssl(NULL), m_socket(socket), m_lastActivityTime(::time(NULL)) {}

    virtual ~runnable() throw() { if (m_ssl) { SSL_shutdown (m_ssl); SSL_free (m_ssl); } }

	// utility to read an HTTP style header line off a socket. Used in most protocols
	// returns zero if lineBuffer is ready, otherwise it returns a timeout in seconds that
	// should be used to wait in a select
	const bool getHTTPStyleHeaderLine(const size_t sid, uniString::utf8 &lineBuffer,
									  const uniString::utf8 &logMsgPrefix, int maxLineLength = 0) throw(std::exception);

	// send a hunk of data out a socket - returns true if send is complete,
	// outBuffer and outBufferSize should be initially set to point to the
	// data and the size of the data - these values are moved and updated.
	const bool sendDataBuffer(const size_t sid, const uniString::utf8::value_type *&outBuffer,
							  int &outBufferSize, const uniString::utf8 &logMsgPrefix) throw(std::exception);

    ssize_t send (const void *buf, size_t len, int flags = 0);
    ssize_t recv (void *buf, size_t len, int flags = 0);

    virtual int detectAutoDumpTimeout (time_t &cur_time, const size_t streamID, const uniString::utf8 &msg) throw(runtime_error);

protected:
    timeSliceResult     m_result;               // for tracking response
    SSL                 *m_ssl;                 // SSL handler
    socketOps::tSOCKET  m_socket;               // we'll need this for read / write handling so
                                                // is easier to hold a copy of it here than do
                                                // it at the higher class levels
    time_t              m_lastActivityTime;     // for tracking timeouts
};

#pragma pack(push, 1)
class threadedRunner : public Vthread
{
///// static for managing collection of these guys
private:
	pipeDrivenSignal<nullLock>	m_signal;			// we must add/remove runnables, or stop, or some other sort of message
	bool						m_stop;				// if true then this thread must shut down
	const short					m_threadNumber;		// for diagnostics only

	// list of runnables to be added or removed from this thread controller
	std::set<runnable*> m_runnablesToAdd;
	std::set<runnable*> m_runnablesToRemove;
	std::set<runnable*> m_runList; // things to run

	AOL_namespace::mutex m_lock;

    friend class microConnection;

    static SSL_CTX *m_sslCtx;
    static AOL_namespace::mutex *m_sslMutexes;
    static unsigned long SSL_idFunction (void);
    static void SSL_lockingFunction (int mode, int n, const char *file, int line);

	const unsigned operator()() throw();

	// add new runnables to the thread. returns false if the runnable could not be queued to this thread object
	// False usually means that the thread is going away and can no longer accept runnables
	const bool addRunnable(runnable*) throw();

	// these runnables will be removed at the next opportunity (does not happen immediately)
	// false means the runnable was not associated with that thread
	const bool removeRunnable(runnable*) throw();

	void enumRunnables(map<uniString::utf8, size_t>& runners) throw();

	void wakeupRunnable() throw();
		
public:
	threadedRunner() throw();
	virtual ~threadedRunner() throw();

	static bool scheduleRunnable(runnable *r) throw();

	static uniString::utf8 getRunnabledetails() throw();

	static void wakeup() throw();

	void stop() throw();
	const size_t sizeOfRunList() throw();
	const uniString::utf8 threadNumber() const throw() { return stringUtil::tos(m_threadNumber); } // for diagnostics only

    static bool isSSLCapable()  { return m_sslCtx ? true : false; }
    static void SSL_init ();
    static void SSL_shutdown ();
};
#pragma pack(pop)

// microserver is a runnable object that implements basic server functionality.
// it listens on a socket. When a connection occurs it creates a microConnection
// object to handle it
class microServer: public runnable
{
public:
	// what stuff we will accept
	typedef u_short AllowableProtocols_t;
	#define P_WEB 1
	#define P_SHOUTCAST1CLIENT 2
	#define P_SHOUTCAST2CLIENT 4
	#define P_SHOUTCAST1SOURCE 8
	#define P_SHOUTCAST2SOURCE 16
	#define P_FLASHPOLICYFILE 64
	#define P_WEB_SETUP 128

	// what stuff we will listen for
	typedef enum { 
		L_MISC = 0,
		L_CLIENT = 1,
		L_SOURCE = 2,
		L_FLASH = 4,
		L_SOURCE2 = 8,
		L_CLIENT_ALT = 16
	} ListenTypes_t;

private:
	AllowableProtocols_t	m_protocols;

	void bindMessage(const ListenTypes_t types, const u_short listenPort) throw();

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "microServer"; }

public:
	microServer(const std::string &listenAddr, const u_short listenPort, const AllowableProtocols_t protocols, const ListenTypes_t types) throw(std::exception);
	void updateProtocols(AllowableProtocols_t protocols, ListenTypes_t types, const u_short listenPort) throw();
	virtual ~microServer() throw();
};

// this class receives a connection and starts processing it until it knows
// what to do with it. When it has figured that out, it creates a protocol object
// and forwards all commands to that object from now on.	
class microConnection: public runnable
{
    friend class protocol_HTTPStyle;
    friend class protocol_uvox2Source;
    friend class protocol_HTTPSource;
    friend class protocol_shoutcastSource;

private:
	std::string			m_srcHostName;
	std::string			m_srcAddress;
	uniString::utf8		m_lineBuffer;
	const u_short		m_srcPort;
	const microServer::AllowableProtocols_t	m_protocols; // what protocols I accept

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8	name() const throw() { return "microConnection"; }

public:
	microConnection(const socketOps::tSOCKET s, const std::string &hostName,
					const std::string &addr, const u_short port,
					const microServer::AllowableProtocols_t protocols) throw();
	virtual ~microConnection() throw();
};

///////////// Utilities that are used often in related components /////////////////////////

// common base used for kicking clients and general bookkeeping
class clientProtocol
{
protected:
	clientProtocol() throw() {}
	virtual ~clientProtocol() throw(){}

public:
	virtual void kickNextRound() throw() {}
	virtual void setGroup(int /*group*/) throw() {}
};

// make standard string for loggin address
extern uniString::utf8 addrLogString(const uniString::utf8 &addr, const u_short port, const uniString::utf8 &xff = "") throw();
// make standard string for logging src address
extern uniString::utf8 srcAddrLogString(const uniString::utf8 &addr, const u_short port, const size_t sid = 0) throw();
// make standard string for loggin dst address	
extern uniString::utf8 dstAddrLogString(const uniString::utf8 &addr, const u_short port, const uniString::utf8 &xff = "", const size_t sid = 0) throw();

// get value from map. Return default if doesn't exist
template<typename T> inline T stringToType(const uniString::utf8 &s) throw();
template<> inline uniString::utf8 stringToType(const uniString::utf8 &s) throw() { return s; }
template<> inline int stringToType(const uniString::utf8 &s) throw() { return atoi((const char *)s.c_str()); }
template<> inline u_short stringToType(const uniString::utf8 &s) throw() { return (u_short)atoi((const char *)s.c_str()); }
template<> inline std::string stringToType(const uniString::utf8 &s) throw() { return s.hideAsString(); }
template<> inline bool stringToType(const uniString::utf8 &s) throw()
{
	if (s.empty())
	{
		return false;
	}
	uniString::utf8::value_type v = *(s.begin());
	if (v == 'f' || v == 'F' || v == 'n' || v == 'N' || v == '0')
	{
		return false;
	}
	return true;
}

template<typename T>
inline T mapGet(const httpHeaderMap_t &m, const uniString::utf8 &key, const T deflt) throw()
{
	httpHeaderMap_t::const_iterator i = m.find(key);
	return (i == m.end() ? deflt : stringToType<T>((*i).second));
}

#endif
