#ifdef _WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#include "socketOps.h"
#include <Ws2tcpip.h>
#include <Mswsock.h>
#include <map>
#include "stl/stringUtils.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace stringUtil;

static std::map<int,std::string> s_errMsgs;

const SOCKET socketOps::cINVALID_SOCKET = INVALID_SOCKET;
const SOCKET socketOps::cSOCKET_ERROR = (SOCKET)SOCKET_ERROR;

class win32_socket_init
{
public:
	~win32_socket_init() { ::WSACleanup(); }
	win32_socket_init()
	{
		WORD wVersion = MAKEWORD( 1, 1 );
		WSADATA wsaData = {0};

		::WSAStartup(wVersion, &wsaData);

		s_errMsgs[WSAEINTR]			= "Interrupted function call: A blocking operation was interrupted by a call to WSACancelBlockingCall.";
		s_errMsgs[WSAEFAULT]		= "Bad address: The system detected an invalid pointer address in attempting to use a pointer argument of a call.";
		s_errMsgs[WSAEINVAL]		= "Invalid argument: Some invalid argument was supplied.";
		s_errMsgs[WSAEMFILE]		= "Too many open descriptors: No more socket descriptors are available.";
		s_errMsgs[WSAEWOULDBLOCK]	= "Call would block: Non-blocking call will block.";
		s_errMsgs[WSAEINPROGRESS]	= "Operation now in progress: A blocking operation is currently executing.";
		s_errMsgs[WSAEALREADY]		= "Operation already in progress: An operation was attempted on a nonblocking socket with an operation already in progress.";
		s_errMsgs[WSAENOTSOCK]		= "Socket operation on non socket: An operation was attempted on something that is not a socket.";
		s_errMsgs[WSAEDESTADDRREQ]	= "Destination address required: A required address was omitted from an operation on a socket.";
		s_errMsgs[WSAEMSGSIZE]		= "Message too long: A message sent on a datagram socket was larger than the internal message buffer.";
		s_errMsgs[WSAEPROTOTYPE]	= "The specified protocol is the wrong type for this socket.";
		s_errMsgs[WSAENOPROTOOPT]	= "Bad Protocol option.";
		s_errMsgs[WSAEPROTONOSUPPORT]	= "The specified protocol is not supported.";
		s_errMsgs[WSAESOCKTNOSUPPORT]	= "The specified socket type is not supported in this address family.";
		s_errMsgs[WSAEOPNOTSUPP]		= "Socket operation not supported.";
		s_errMsgs[WSAEPFNOSUPPORT]	= "Protocol family not supported.";
		s_errMsgs[WSAEAFNOSUPPORT]	= "The specified address family is not supported";
		s_errMsgs[WSAEADDRINUSE]	= "Address already in use.";
		s_errMsgs[WSAEADDRNOTAVAIL]	= "Cannot assign requested address.";
		s_errMsgs[WSAENETDOWN]		= "A network subsystem or the associated service provider has failed";
		s_errMsgs[WSAENETUNREACH]	= "Nework is unreachable.";
		s_errMsgs[WSAENETRESET]		= "Network dropped connection on reset.";
		s_errMsgs[WSAECONNABORTED]	= "Software caused connection abort.";
		s_errMsgs[WSAECONNRESET]	= "Connection reset by peer.";
		s_errMsgs[WSAENOBUFS]		= "No buffer space is available. The socket cannot be created.";
		s_errMsgs[WSAEISCONN]		= "Socket is already connected.";
		s_errMsgs[WSAENOTCONN]		= "Socket is not connected.";
		s_errMsgs[WSAESHUTDOWN]		= "Cannot send after socket shutdown.";
		s_errMsgs[WSAETIMEDOUT]		= "Connection timed out.";
		s_errMsgs[WSAECONNREFUSED]	= "Connection refused.";
		s_errMsgs[WSAEHOSTDOWN]		= "Host is down.";
		s_errMsgs[WSAEHOSTUNREACH]	= "No route to host.";
		s_errMsgs[WSAEPROCLIM]		= "Too many processes.";
	}
};

static win32_socket_init win32_socket_init_force;

std::string socketOps::errMsg(int errCode) throw()
{
	std::map<int,std::string>::const_iterator i = s_errMsgs.find(errCode);
	return (i == s_errMsgs.end() ? "error code " + tos(errCode) : (*i).second);
}

int socketOps::errCode() throw() { return ::WSAGetLastError(); }

#else

#include <poll.h>
#include "socketOps.h"
#include "stl/stringUtils.h"
#include <string.h>

using namespace std;
using namespace stringUtil;

const int socketOps::cINVALID_SOCKET = -1;
const int socketOps::cSOCKET_ERROR = -1;

int socketOps::errCode() throw() { return errno; }

std::string socketOps::errMsg(int errCode) throw()
{
	std::string result = "error code " + tos(errCode);

	char *e = strerror(errCode);
	if (e)
	{
		result = e;
	}
	return result;
}
#endif

std::string socketOps::endpoint::toString() const throw()
{
	return m_address + ":" + tos(m_port);
}

socketOps::tSOCKET socketOps::createTCPSocket() throw()
{
	return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

socketOps::tSOCKET socketOps::createTCPSocketTHROW() throw(std::runtime_error)
{
	tSOCKET result = socketOps::createTCPSocket();
	if (result == socketOps::cINVALID_SOCKET)
	{
		throw std::runtime_error("socketOps::createTCPSocketTHROW() - Could not create socket because " + socketOps::errMsg());
	}
	return result;
}

int	socketOps::setNonblock(const socketOps::tSOCKET s, bool nonBlock) throw()
{
#ifdef _WIN32
	unsigned long i = (nonBlock ? 1 : 0);
	return ioctlsocket(s, FIONBIO, &i);
#else
	int flags, err;

	if ((flags = fcntl(s, F_GETFL, 0)) == socketOps::cSOCKET_ERROR)
	{
		return flags;
	}

	if (nonBlock)
	{
		flags |= O_NONBLOCK;
	}
	else 
	{
		flags &= ~O_NONBLOCK;
	}

	if ((err = fcntl(s, F_SETFL, flags)) == socketOps::cSOCKET_ERROR)
	{
		return err;
	}
	return 0;
#endif
}

void socketOps::setNonblockTHROW(const socketOps::tSOCKET s, const bool nonblock) throw(std::runtime_error)
{
	int err = socketOps::setNonblock(s, nonblock);
	if (err == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error("socketOps::setNonblockTHROW() - Could not get socket flags because " + socketOps::errMsg());
	}
}

void socketOps::closeTCPSocket(socketOps::tSOCKET s) throw()
{
#ifdef _WIN32
	::shutdown(s, SD_BOTH);
	::closesocket(s);
#else
	::shutdown(s, SHUT_RDWR);
	::close(s);
#endif
}

void socketOps::forgetTCPSocket(tSOCKET &s) throw()
{
	if (s != socketOps::cINVALID_SOCKET)
	{
		socketOps::closeTCPSocket(s);
		s = socketOps::cINVALID_SOCKET;
	}
}

int	socketOps::connect(const socketOps::tSOCKET s, const std::string &address, const u_short port) throw()
{
	unsigned long iaddr = inet_addr(address.c_str());
	struct sockaddr_in addr = {0};

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	memcpy(&(addr.sin_addr), &iaddr, 4);

	return ::connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr));
}

int	socketOps::connect(tSOCKET s,const endpoint &e) throw()
{
	return socketOps::connect(s, e.m_address, e.m_port);
}

void socketOps::connectTHROW(const socketOps::tSOCKET s, const std::string &address, const u_short port) throw(std::runtime_error)
{
	if (socketOps::connect(s, address, port) == socketOps::cSOCKET_ERROR)
    {
        int err = socketOps::errCode();
#ifdef _WIN32
        if ((err == WSAEINPROGRESS) || (err == WSAEWOULDBLOCK))
#else
        if ((err == EINPROGRESS) || (err == EWOULDBLOCK) || (err == ECONNABORTED) || (err == EINTR))
#endif
            return;

        throw std::runtime_error("Could not connect to " + address + ":" + tos(port) + " because " + socketOps::errMsg());
	}
}

void socketOps::connectTHROW(tSOCKET s, const endpoint &e) throw(std::runtime_error)
{
	if (socketOps::connect(s, e) == socketOps::cSOCKET_ERROR)
    {
        int err = socketOps::errCode();
#ifdef _WIN32
        if ((err == WSAEINPROGRESS) || (err == WSAEWOULDBLOCK))
#else
        if ((err == EINPROGRESS) || (err == EWOULDBLOCK) || (err == ECONNABORTED) || (err == EINTR))
#endif
            return;

        throw std::runtime_error("Could not connect to " + e.toString() + " because " + socketOps::errMsg());
    }
}

socketOps::nonBlockConnect_t socketOps::nonBlockingConnectWait(const socketOps::tSOCKET s, std::string &errorString) throw()
{
	try
	{
#ifdef _WIN32
		// non-blocking connects suck
		fd_set rset, wset, eset;
		struct timeval tval;

		FD_ZERO(&rset); FD_ZERO(&wset); FD_ZERO(&eset);
		FD_SET(s,&rset);
		FD_SET(s,&wset);
		FD_SET(s,&eset);

		tval.tv_sec = 0;
		tval.tv_usec = 1;

		int n = ::select(((int)s)+1,&rset,&wset,&eset,&tval);

		// if n == 0 then we timed out, and must stay in this state
		if (n == 0)
		{
			return socketOps::NBC_INPROGRESS;
		}

		if (n < 0) // fatal select error
		{
			try
			{
				errorString = socketOps::errMsg();
			}
			catch(...)
			{
				errorString = "Impossible connection failure. (1)";
			}
			return socketOps::NBC_ERROR;
		}

		// detecting when a non-blocking connect is ready (or has errored)
		// is highly platform dependent. Microsoft docs specify one way, Stevens
		// (2nd edition page 411) specifies another way to cover various Unix flavors
		if (FD_ISSET(s,&eset))
		{
			errorString = "Connection failure.";
			return socketOps::NBC_ERROR;
		}
		else if (FD_ISSET(s,&wset))
		{
			return socketOps::NBC_CONNECTED;
		}
		else
		{
			errorString = "Impossible connection failure. (2)";
			return socketOps::NBC_ERROR;
		}
#else
        struct pollfd check;
        int val = -1;
        socklen_t size = sizeof val;
        check.fd = s;
        check.events = POLLOUT;
        switch (poll (&check, 1, 0))
        {
			case 0:
			{
				return socketOps::NBC_INPROGRESS;
			}
			default:
			{
				if (getsockopt (s, SOL_SOCKET, SO_ERROR, (void*) &val, &size) == 0)
				{
					if (val == 0)
					{
						return socketOps::NBC_CONNECTED;
					}
					errno = val;
				}
                // fall thru
			}
			case -1:
			{
				if (errno == EINTR)
				{
					return socketOps::NBC_INPROGRESS;
				}
                throw std::runtime_error("");
			}
		}
#endif
    }
    catch(...)
    {
#ifdef _WIN32
        errorString = "Impossible connection failure. (3)";
#else
		errorString = "Connection failure.";
#endif
        return socketOps::NBC_ERROR;
	}
}

socketOps::tSOCKET socketOps::bind(tSOCKET s, u_short port, const std::string &address) throw()
{
	struct sockaddr_in servaddr = {0};
	servaddr.sin_family = AF_INET;

	if (address == "")
	{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		unsigned long iaddr = inet_addr(address.c_str());
		memcpy(&(servaddr.sin_addr), &iaddr, 4);
	}

	servaddr.sin_port = htons(port);
	return ::bind(s, (const sockaddr *)&servaddr, sizeof(servaddr));
}

socketOps::tSOCKET socketOps::bind(tSOCKET s, const socketOps::endpoint &e) throw()
{
	return socketOps::bind(s,e.m_port,e.m_address);
}

void socketOps::bindTHROW(tSOCKET s, u_short port, const std::string &address) throw(std::runtime_error)
{
	if (socketOps::bind(s,port,address) == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error("Could not bind to " + (!address.empty() ? address + ":" : "") + tos(port) + " because " + socketOps::errMsg());
	}
}

void socketOps::bindTHROW(tSOCKET s,const endpoint &e) throw(std::runtime_error)
{
	if (socketOps::bind(s,e) == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error("Could not bind to " + e.toString() + " because " + socketOps::errMsg());
	}
}

int	socketOps::accept(tSOCKET s, std::string &address, u_short &port) throw()
{
	struct sockaddr_in client_addr = {0};
#ifdef _WIN32
	int len = sizeof(client_addr);
#else
	socklen_t len = sizeof(client_addr);
#endif

	socketOps::tSOCKET sC = ::accept(s, (sockaddr*)&client_addr, &len);
	if (sC != socketOps::cSOCKET_ERROR)
	{
		port = ntohs(client_addr.sin_port);
		address = inet_ntoa(client_addr.sin_addr);
	}
	return (int)sC;
}

int	socketOps::accept(tSOCKET s,endpoint &e) throw()
{
	return socketOps::accept(s,e.m_address,e.m_port);
}

int	socketOps::acceptTHROW(tSOCKET s, std::string &address, u_short &port, bool nonblocking) throw(std::runtime_error)
{
	socketOps::tSOCKET sC = socketOps::accept(s, address, port);
	if (sC == socketOps::cSOCKET_ERROR)
	{
		if (!nonblocking)
		{
			throw std::runtime_error(socketOps::errMsg());
		}

		int e = socketOps::errCode();
#ifdef _WIN32
		if ((e == WSAEINPROGRESS) || (e == WSAEWOULDBLOCK))
		{
			return (int)socketOps::cSOCKET_ERROR;
		}
#else
		if ((e == EWOULDBLOCK) || (e == ECONNABORTED) || (e == EINTR))
		{
			return socketOps::cSOCKET_ERROR;
		}
#endif
		throw std::runtime_error("Could not call accept() on socket because " + socketOps::errMsg(e));
	}

	return (int)sC;
}

int	socketOps::acceptTHROW(tSOCKET s, endpoint &e, bool nonblocking) throw(std::runtime_error)
{
	return socketOps::acceptTHROW(s, e.m_address, e.m_port, nonblocking);
}

void socketOps::listenTHROW(tSOCKET s, int backlog) throw(std::runtime_error)
{
	if (::listen(s, backlog) == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error("Could not call listen() on socket because " + socketOps::errMsg());
	}
}

int socketOps::getsockname(tSOCKET s, std::string &address, u_short &port) throw()
{
	sockaddr_in in4 = {0};
#ifdef _WIN32
	int in4len = sizeof(in4);
#else
	socklen_t in4len = sizeof(in4);
#endif

	int result = ::getsockname(s,(sockaddr*)&in4,&in4len);	
	if (result != socketOps::cSOCKET_ERROR)
	{
		port = ntohs(in4.sin_port);
		address = inet_ntoa(in4.sin_addr);
	}
	return result;
}

int socketOps::getsockname(tSOCKET s,endpoint &e) throw()
{
	return socketOps::getsockname(s, e.m_address, e.m_port);
}

void socketOps::getsocknameTHROW(tSOCKET s, endpoint &e) throw(std::runtime_error)
{
	if (socketOps::getsockname(s, e) == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error(socketOps::errMsg());
	}
}

void socketOps::getsocknameTHROW(tSOCKET s, std::string &address, u_short &port) throw(std::runtime_error)
{
	if (socketOps::getsockname(s, address, port) == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error(socketOps::errMsg());
	}
}

int socketOps::getpeername(tSOCKET s, std::string &address, u_short &port) throw()
{
	address = "";
	port = 0;
	sockaddr_in in4 = {0};
#ifdef _WIN32
	int in4len = sizeof(in4);
#else
	socklen_t in4len = sizeof(in4);
#endif

	int result = ::getpeername(s, (sockaddr*)&in4,&in4len);	
	if (result != socketOps::cSOCKET_ERROR)
	{
		port = ntohs(in4.sin_port);
		address = inet_ntoa(in4.sin_addr);
	}
	return result;
}

int socketOps::getpeername(tSOCKET s, endpoint &e) throw()
{
	return socketOps::getpeername(s, e.m_address, e.m_port);
}

void socketOps::getpeernameTHROW(tSOCKET s, endpoint &e) throw(std::runtime_error)
{
	if (socketOps::getpeername(s, e) == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error(socketOps::errMsg());
	}
}

void socketOps::getpeernameTHROW(tSOCKET s, std::string &address, u_short &port) throw(std::runtime_error)
{
	if (socketOps::getpeername(s, address, port) == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error(socketOps::errMsg());
	}
}

// accepts IPv4 or IPv6
int socketOps::addressToHostName(const std::string &address, u_short port, std::string &hostname) throw()
{
	struct addrinfo *result/*, hints = {AI_PASSIVE | AI_CANONNAME, AF_INET, SOCK_STREAM}*/;

	std::string portS = tos(port);
	int err = ::getaddrinfo(address.c_str(),portS.c_str(),0,&result);
	if ((!err) && result)
	{
		char h[NI_MAXHOST + 1] = {0};
		err = ::getnameinfo(result->ai_addr, (int)result->ai_addrlen, h, NI_MAXHOST, 0, 0, 0);
		hostname = h;
		::freeaddrinfo(result);
	}
	return err;
}

int socketOps::addressToHostName(const endpoint &e, std::string &hostname) throw()
{
	return socketOps::addressToHostName(e.m_address, e.m_port, hostname);
}

void socketOps::addressToHostNameTHROW(const std::string &address, u_short port, std::string &hostname) throw(std::runtime_error)
{
	if (socketOps::addressToHostName(address, port, hostname) == socketOps::cSOCKET_ERROR)
	{
		throw std::runtime_error(socketOps::errMsg());
	}
}

void socketOps::addressToHostNameTHROW(const endpoint &e, std::string &hostname) throw(std::runtime_error)
{
	socketOps::addressToHostNameTHROW(e.m_address, e.m_port, hostname);
}

int socketOps::hostNameToAddress(std::string &address, const std::string &hostname, u_short port) throw(std::runtime_error)
{
	struct addrinfo *result, hints = {AI_PASSIVE | AI_CANONNAME, AF_INET, SOCK_STREAM};
	address = "";

	std::string portS = tos(port);
	int err = ::getaddrinfo(hostname.c_str(), (port ? portS.c_str() : 0), &hints, &result);
	if ((!err) && result)
	{
		address = ::inet_ntoa(((sockaddr_in*)(result->ai_addr))->sin_addr);
		if (!address.empty() && address.find("0.") == 0)
		{
			address = "";
		}
		::freeaddrinfo(result);
	}
	return err;
}

std::string socketOps::hostNameToAddress(const std::string &hostname, u_short port) throw(std::runtime_error)
{
	std::string address;
	hostNameToAddress(address, hostname, port);
	return address;
}

std::string socketOps::hostNameToAddressTHROW(const std::string &hostname, u_short port) throw(std::runtime_error)
{
	std::string address;
	if (hostNameToAddress(address, hostname, port))
	{
		throw std::runtime_error(socketOps::errMsg());
	}
	return address;
}

#ifdef _WIN32
LPFN_WSAPOLL fnWSAPoll = NULL;
typedef unsigned long nfds_t;

static void add_to_fd_set_fntr(const socketOps::tSOCKET s, fd_set *fdset) throw()
{
	FD_SET(s, fdset);
}
#else
#define fnWSAPoll poll
#endif

#include <stdio.h>
int socketOps::socketSelect(std::set<size_t> &readSockets, std::set<size_t> writeSockets, const int timeout) throw()
{
	//ELOG("socketSelect: " + tos(readSet.size()) + " - " + tos(writeSet.size()) + " - " + tos(timeout));

#ifdef _WIN32
	if (!fnWSAPoll)
	{
		fd_set readSet;
		fd_set writeSet;
		fd_set excepSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		FD_ZERO(&excepSet);
		for_each(readSockets.begin(), readSockets.end(), std::bind2nd(std::ptr_fun(add_to_fd_set_fntr), &readSet));
		for_each(writeSockets.begin(), writeSockets.end(), std::bind2nd(std::ptr_fun(add_to_fd_set_fntr), &writeSet));
		for_each(readSockets.begin(), readSockets.end(), std::bind2nd(std::ptr_fun(add_to_fd_set_fntr), &excepSet));
		for_each(writeSockets.begin(), writeSockets.end(), std::bind2nd(std::ptr_fun(add_to_fd_set_fntr), &excepSet));

		// as we're workig in milliseconds then we need to convert to
		// provide select the timeout that is wanted as a timeval
		struct timeval tm = {(timeout / 1000), ((timeout % 1000) * 1000)};
		// The Windows version ignores the first arg, no need to calculate it
		return ::select(0, &readSet, &writeSet, &excepSet, &tm);
	}
#endif

	size_t i = 0;
	nfds_t length = (nfds_t)(readSockets.size() + writeSockets.size());
        std::vector <struct pollfd>	dataSet (length);

	for (std::set<size_t>::iterator it = readSockets.begin(); it != readSockets.end(); ++it)
	{
		dataSet[i].events = POLLIN;
		dataSet[i++].fd = *it;
	}

	for (std::set<size_t>::iterator it = writeSockets.begin(); it != writeSockets.end(); ++it)
	{
		dataSet[i].events = POLLOUT;
		dataSet[i++].fd = *it;
	}

	return fnWSAPoll (&dataSet[0], length, timeout);
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

int pgpipe(int handles[2])
{
	SOCKET s = {0};
	struct sockaddr_in serv_addr = {0};
    int len = sizeof(serv_addr);

	handles[0] = handles[1] = (int)INVALID_SOCKET;

	if ((s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		return -1;
	}

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(0);
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (::bind(s, (SOCKADDR *)&serv_addr, len) == SOCKET_ERROR)
	{
		::closesocket(s);
        return -1;
	}

    if (::listen(s, 1) == SOCKET_ERROR)
	{
		::closesocket(s);
        return -1;
	}

    if (::getsockname(s, (SOCKADDR *)&serv_addr, &len) == SOCKET_ERROR)
	{
		::closesocket(s);
        return -1;
	}

    if ((handles[1] = (int)::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		::closesocket(s);
        return -1;
    }

	if (::connect(handles[1], (SOCKADDR *)&serv_addr, len) == socketOps::cSOCKET_ERROR)
	{
		::closesocket(s);
        return -1;
	}

	if ((handles[0] = (int)::accept(s, (SOCKADDR *)&serv_addr, &len)) == INVALID_SOCKET)
	{
		::closesocket(handles[1]);
		handles[1] = (int)INVALID_SOCKET;
		::closesocket(s);
		return -1;
    }

	::closesocket(s);
	return 0;
}
	
#endif
