#pragma once
#ifndef _socketOps_H_
#define _socketOps_H_

#ifdef _WIN32
#include <winsock2.h>
#else
#include <poll.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif
#include <set>
#include "stl/stringUtils.h"
#include <string>
#include <stdexcept>
#include <vector>
#include <functional>
#include <algorithm>

using namespace std;

/*
	socketOps namespace

	Encased in this namespace is a collection of constants, types and functions
	to help bridge the gap between Win32 and Unix socket implementations, and also
	to simplify some socket coding.

	Since the Win32 and Unix berkeley socket implementations are extremely similar, I
	felt that a collection of helper functions were of more use, and more flexible, than
	full blown socket wrapper classes.
*/

namespace socketOps
{
#ifdef _WIN32
	typedef SOCKET tSOCKET;
	extern const SOCKET cINVALID_SOCKET;
	extern const SOCKET cSOCKET_ERROR;
	//#define EINPROGRESS WSAEINPROGRESS
	//#define EWOULDBLOCK WSAEWOULDBLOCK
	#define SOCKETOPS_WOULDBLOCK	WSAEWOULDBLOCK
#else
	typedef int tSOCKET;
	extern const int cINVALID_SOCKET;
	extern const int cSOCKET_ERROR;
	#define WSAEINPROGRESS	EINPROGRESS
	#define WSAEWOULDBLOCK	EWOULDBLOCK	
	#define SOCKETOPS_WOULDBLOCK	EWOULDBLOCK
#endif

	// socket endpoint
	struct endpoint
	{
		std::string m_address;
		u_short		m_port;

		endpoint() : m_port(0) {}
		endpoint(const std::string &address, u_short port) : m_address(address), m_port(port) {}

		bool operator==(const endpoint &e) const throw()
		{
			return ((m_address == e.m_address) && (m_port == e.m_port));
		}

		bool operator<(const endpoint &e) const throw()
		{
			if (m_address < e.m_address) return true;
			if (m_address > e.m_address) return false;
			if (m_port < e.m_port) return true;
			if (m_port > e.m_port) return false;
			return false;
		}

		std::string toString() const throw(); // for messages
	};

	std::string errMsg(int code) throw();
	int errCode() throw();
	inline std::string errMsg() throw() { return socketOps::errMsg(socketOps::errCode()); }

	// createTCPSocket: creates a standard TCP/IP stream socket. The "throw" version
	//    will throw an exception if an error occurs instead of returning cSOCKET_ERROR
	tSOCKET createTCPSocket() throw();
	tSOCKET createTCPSocketTHROW() throw(std::runtime_error);

	// setNonblock: non blocking is set different under unix and windows. These calls
	//		hide the different. First version returns the typical cSOCKET_ERROR when
	//		there is a problem. Second will throw an exception instead
	int setNonblock(tSOCKET s,bool nonblock) throw();
	void setNonblockTHROW(tSOCKET s,bool nonblock) throw(std::runtime_error);

	// closeTCPSocket: does a blocking shutdown() and close()
	void closeTCPSocket(tSOCKET s) throw();
	void forgetTCPSocket(tSOCKET &s) throw(); // similar, but checks s first and then clear it
	
	// connect: connects are the same across platforms. These just simplify the calls
	// addresses for these calls most be #.#.#.#
	int connect(tSOCKET s, const std::string &address, u_short port) throw();
	int connect(tSOCKET s, const endpoint &e) throw();
	void connectTHROW(tSOCKET s, const std::string &address, u_short port) throw(std::runtime_error);
	void connectTHROW(tSOCKET s, const endpoint &e) throw(std::runtime_error);

	// bind: binds are the same across platforms. These just simplify the calls
	tSOCKET bind(tSOCKET s, u_short port, const std::string &address = "") throw();
	tSOCKET bind(tSOCKET s, const endpoint &e) throw();
	void bindTHROW(tSOCKET s, u_short port, const std::string &address = "") throw(std::runtime_error);
	void bindTHROW(tSOCKET s, const endpoint &e) throw(std::runtime_error);

	// note: for non-blocking accepts, the various "errno" errors can be different
	//   under different OS's. If you use the calls that do not throw exception you
	//   will have to handle these differences yourself. If you call the ones that
	//   throw, I try to handle it for you. In these calls, if the nonblocking parameter
	//   is true, the call will return cSOCKET_ERROR if accept has nothing to do (returns
	//   EWOULDBLOCK or similar). If a REAL error occurs, then it throws an exception.
	int accept(tSOCKET s, endpoint &e) throw();
	int accept(tSOCKET s, std::string &address, u_short &port) throw();
	int acceptTHROW(tSOCKET s, endpoint &e, bool nonblocking) throw(std::runtime_error);
	int acceptTHROW(tSOCKET s, std::string &address, u_short &port, bool nonblocking) throw(std::runtime_error);
	
	// listen: helper func that throws an exception instead of returning an error code
	void listenTHROW(tSOCKET s, int backlog = SOMAXCONN) throw(std::runtime_error);

	// waiting on a non-blocking connect call is messy. Use this call to avoid
	// the muck	
	typedef enum { NBC_ERROR, NBC_INPROGRESS, NBC_CONNECTED } nonBlockConnect_t;
	nonBlockConnect_t nonBlockingConnectWait(tSOCKET s, std::string &error) throw();

	int  socketSelect(std::set<size_t> &readSockets, std::set<size_t> writeSockets, const int timeout) throw();

	int getsockname(tSOCKET s, endpoint &e) throw();
	int getsockname(tSOCKET s, std::string &address, u_short &port) throw();
	void getsocknameTHROW(tSOCKET s, endpoint &e) throw(std::runtime_error);
	void getsocknameTHROW(tSOCKET s, std::string &address, u_short &port) throw(std::runtime_error);

	int getpeername(tSOCKET s, endpoint &e) throw();
	int getpeername(tSOCKET s, std::string &address, u_short &port) throw();
	void getpeernameTHROW(tSOCKET s, endpoint &e) throw(std::runtime_error);
	void getpeernameTHROW(tSOCKET s, std::string &address, u_short &port) throw(std::runtime_error);

	int addressToHostName(const endpoint &e, std::string &hostname) throw();
	int addressToHostName(const std::string &address, u_short port, std::string &hostname) throw();
	void addressToHostNameTHROW(const endpoint &e, std::string &hostname) throw(std::runtime_error);
	void addressToHostNameTHROW(const std::string &address, u_short port, std::string &hostname) throw(std::runtime_error);

	int hostNameToAddress(std::string &address, const std::string &hostname, u_short port = 0) throw(std::runtime_error);
	std::string hostNameToAddress(const std::string &hostname, u_short port = 0) throw(std::runtime_error);
	std::string hostNameToAddressTHROW(const std::string &hostname, u_short port = 0) throw(std::runtime_error);
}

#ifdef _WIN32
/*-------------------------------------------------------------------------
 *
 *      This is a replacement version of pipe for Win32 which allows
 *      returned handles to be used in select(). Note that read/write calls
 *      must be replaced with recv/send.
 *
 *
 *-------------------------------------------------------------------------
 */
int pgpipe(int handles[2]);
#endif

#ifndef _WIN32
#define pgpipe(a)				pipe(a)
#define piperead(a,b,c)			read(a,b,c)
#define pipewrite(a,b,c)		write(a,b,c)
#define pipeclose(a)			{ ::shutdown(a,2); ::close(a);}
#else
#define piperead(a,b,c)			recv(a,b,c,0)
#define pipewrite(a,b,c)		send(a,b,c,0)
#define pipeclose(a)			 { ::shutdown(a,SD_BOTH); ::closesocket(a); }
#endif

#endif
