#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <algorithm>
#include <stdio.h>
#include "adminList.h"
#include "global.h"
#include "stl/stringUtils.h"
#include "macros.h"
#include "webNet/socketOps.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"
#include <assert.h>

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define LOGNAME "[ADMINCGI] "

#ifdef _WIN32
typedef unsigned long in_addr_t;
#endif

adminList g_adminList;

class adminList::impl
{
private:
	struct adminEntry: public admin_t
	{
		in_addr_t	m_ip;		// ip as binary type. Old style, but that's how the old sc_serv did it and we'll
								// continue to do it that way until we're ready to break the old software

		bool validIP() throw()
		{
			return ((m_ip != INADDR_NONE) && (m_ip != 0));
		}

		static in_addr_t stringToIP(const utf8 &sIP, utf8 &hostIP)
		{
			// default is to assume a raw IP address in the list
			in_addr_t ip =  inet_addr((const char *)sIP.c_str());
			if (ip == INADDR_NONE)
			{
				// though if that fails then attempt to
				// get an IP address from a hostname...
				string sHost;
				try
				{
					sHost = socketOps::hostNameToAddress(sIP.hideAsString());
				}
				catch(...)
				{
				}
				if (!sHost.empty())
				{
					ip =  inet_addr((const char *)sHost.c_str());
					if (ip != INADDR_NONE)
					{
						hostIP = sHost;
					}
				}
			}
			return ip;
		}

		explicit adminEntry(const utf8 &numericIP) throw() : admin_t(numericIP), m_ip(stringToIP(numericIP, m_hostIP)) {}
		adminEntry() throw() : m_ip(0) {}
	};

	AOL_namespace::mutex	m_lock;
	list<adminEntry>		m_list;

public:
	bool load(const uniFile::filenameType &fn) throw(exception)
	{
		if (fn.empty())
		{
			throwEx<tagged_error>(LOGNAME "No admin access file");
		}
		else if (gOptions.microServerDebug())
		{
			DLOG(LOGNAME "Attempting to read admin access file: " + fileUtil::getFullFilePath(fn));
		}

		stackLock sml(m_lock);

		FILE *f = uniFile::fopen(fn,"rb");
		if (!f) return false;

		bool updating = (!m_list.empty());
		m_list.clear();

		try 
		{
			int l = 0;
			int count = 0;
			while (true)
			{
				char buffer[4096] = {0};

				if (!fgets(buffer, sizeof(buffer), f)) break; // get a line

				++l; // increment line counter

				utf8 s;

				// skip utf-8 BOM
				if ((strlen(buffer) > 2) &&
					(((unsigned char*)buffer)[0] == 0xef) &&
					(((unsigned char*)buffer)[1] == 0xbb) &&
					(((unsigned char*)buffer)[2] == 0xbf))
				{
					s = &(buffer[3]);
				}
				else
				{
					s = buffer;
				}

				adminEntry e(stripWhitespace(s));

				if (!e.validIP())
				{
					WLOG(LOGNAME "Line " + tos(l) + " of admin access list has been ignored (bad IP)");
				}
				else
				{
					if (this->find(e.m_numericIP,false) < 1)
					{
						m_list.push_back(e);
						++count;
					}
				}
			}
			if (!updating)
			{
				if (count > 0)
				{
					ILOG(LOGNAME "Enabled " + tos(count) + " IP" +
						 (count != 1 ? "'s" : "") + " from admin access file");
				}
				else if (gOptions.microServerDebug())
				{
					DLOG(LOGNAME "No IPs read from admin access file");
				}
			}
			else
			{
				ILOG(LOGNAME "Reloaded " + tos(count) + " IP" +
					 (count != 1 ? "'s" : "") + " from admin access file");
			}
		}
		catch(...)
		{
			if (f) ::fclose(f);
			throw;
		}
		if (f) ::fclose(f);
		return true;
	}

	bool add(const utf8 &ipAddr, const bool soft) throw(exception)
	{
		// skip loopback addresses as we treat them specially anyway
		if ((ipAddr.find(utf8("127.")) == utf8::npos))
		{
			adminEntry e(ipAddr);
			if (!e.validIP())
			{
				if (!soft)
				{
					throwEx<runtime_error>(LOGNAME "Invalid IP specified - `" + ipAddr + "'");
				}
				else
				{
					return false;
				}
			}

			stackLock sml(m_lock);
			m_list.push_back(e);
			return true;
		}
		return false;
	}

	// true if removed
	bool remove(const utf8 &ipAddr, const bool allStream, const bool fallback = false, const bool use_lock = true)
	{
		if (use_lock)
		{
			stackLock sml(m_lock);
		}

		for ( list<adminEntry>::iterator i = m_list.begin(); i != m_list.end(); ++i )
		{
			if ( ( allStream || ( ( !allStream && ( ( *i ).m_numericIP == ipAddr ) ) || ( ( *i ).m_hostIP == ipAddr ) ) ) )
			{
				m_list.erase( i );
				return true;
			}
		}

		// attempt to see if we've got a hostname which has not been detected from the loading mapping
		if (!fallback)
		{
			string sHost;
			try
			{
				sHost = socketOps::hostNameToAddress(ipAddr.hideAsString());
			}
			catch(...)
			{
			}
			if (!sHost.empty())
			{
				return remove(sHost, allStream, true, false);
			}
		}

		return false;
	}

	// 1 if found, 0 if not, -1 if empty (assume allowed)
	int find(const utf8 &ipAddr, const bool use_lock=true) throw()
	{
		if (use_lock)
		{
			stackLock sml(m_lock);
		}

		if (!m_list.empty())
		{
			for (list<adminEntry>::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
			{
				if ((ipAddr == (*i).m_numericIP) || (ipAddr == (*i).m_hostIP))
				{
					return 1;
				}
			}
			return 0;
		}
		return -1;
	}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

adminList::adminList():m_impl(0)
{
	m_impl = new adminList::impl;
}

adminList::~adminList() throw()
{
	forget(m_impl);
}

bool adminList::load(const uniFile::filenameType &fn) throw()
{
	assert(m_impl);

	bool result(false);

	try
	{
		result = m_impl->load(fn);
	}
	catch(const exception &ex)
	{
		ELOG(ex.what());
	}
	return result;
}

// throws if parameters are invalid
bool adminList::add(const utf8 &ipAddr, const bool soft) throw(exception)
{
	assert(m_impl);
	return m_impl->add(ipAddr, soft);
}

// true if removed
bool adminList::remove(const utf8 &ipAddr, const bool allStream) throw()
{
	assert(m_impl);
	return m_impl->remove(ipAddr, allStream);
}

// 1 if found, 0 if not, -1 if empty (assume allowed)
int adminList::find(const utf8 &ipAddr) throw()
{
	assert(m_impl);
	return m_impl->find(ipAddr);
}
