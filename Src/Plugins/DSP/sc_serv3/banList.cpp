#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include "banList.h"
#include "global.h"
#include "stl/stringUtils.h"
#include "macros.h"
#include <algorithm>
#include "webNet/socketOps.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"
#include <assert.h>

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define LOGNAME "[BAN] "

#ifdef _WIN32
typedef unsigned long in_addr_t;
#endif

banList g_banList;

class banList::impl
{
private:
	struct banEntrySave
	{
		FILE *f;
		size_t stream_ID;
	};

	struct banEntry: public ban_t
	{
		in_addr_t	m_ip;		// ip as binary type. Old style, but that's how the old sc_serv did it and we'll
								// continue to do it that way until we're ready to break the old software

		void save(banEntrySave entrySave) throw(exception)
		{
			if (m_stream_ID == entrySave.stream_ID)
			{
				utf8 s(m_numericIP + ";" + tos(m_mask) + ";" + m_comment + eol());
				if (fwrite(s.c_str(),1,s.size(),entrySave.f) != s.size())
				{
					throwEx<tagged_error>(LOGNAME "I/O error writing " + (!entrySave.stream_ID ? "global" : "sid=" + tos(entrySave.stream_ID)) + " ban file");
				}
			}
		}

		bool validIP() throw()
		{
			return ((m_ip != INADDR_NONE) && (m_ip != 0));
		}

		bool validMask() throw()
		{
			return (m_mask <= 255);
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

		banEntry(const utf8 &numericIP, const __uint32 mask, const utf8 &comment, const size_t stream_ID) throw()
				 :ban_t(numericIP, mask, comment, stream_ID), m_ip(stringToIP(numericIP, m_hostIP)) {}
		banEntry() throw():m_ip(0){}
	};

	AOL_namespace::mutex	m_lock;
	list<banEntry>			m_list;

public:
	bool load(const uniFile::filenameType &fn, size_t stream_ID) throw(exception)
	{
		if (fn.empty())
		{
			throwEx<tagged_error>(LOGNAME "No " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban file");
		}
		else if (gOptions.microServerDebug())
		{
			DLOG(LOGNAME "Attempting to read ban file: " + fileUtil::getFullFilePath(fn));
		}
		
		stackLock sml(m_lock);
		
		FILE *f = uniFile::fopen(fn,"rb");
		if (!f) return false;

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

				s = stripWhitespace(s);

				utf8::size_type pos1 = s.find(utf8::value_type(';'));
				if (pos1 == utf8::npos)
				{
					throwEx<tagged_error>(LOGNAME "Parse error in " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban file on line " + tos(l));
				}

				utf8::size_type pos2 = s.find(utf8::value_type(';'),pos1+1);
				if (pos2 == utf8::npos)
				{
					throwEx<tagged_error>(LOGNAME "Parse error in " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + "ban file on line " + tos(l));
				}

				banEntry e(s.substr(0,pos1),utf8(s.substr(pos1+1,pos2 - pos1 - 1)).toInt(),s.substr(pos2+1),stream_ID);

				if (!e.validIP())
				{
					WLOG(LOGNAME "Line " + tos(l) + " of " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban list has been ignored (bad IP)");
				}
				else if (!e.validMask())
				{
					WLOG(LOGNAME "Line " + tos(l) + " of " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban list has been ignored (bad MASK)");
				}
				else
				{
					if (this->find(e.m_numericIP,e.m_mask,e.m_stream_ID,false) == false)
					{
						m_list.push_back(e);
						++count;
					}
				}
			}
			ILOG(LOGNAME "Banned " + tos(count) + " IP" + (count != 1 ? "'s" : "") + " from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " ban file");
		}
		catch(...)
		{
			if (f) ::fclose(f);
			throw;
		}
		if (f) ::fclose(f);
		return true;
	}

	void save(const uniFile::filenameType &fn, size_t stream_ID) throw(exception)
	{
		stackLock sml(m_lock);

		FILE *f = uniFile::fopen(fn,"wb");
		if (!f)
		{
			throwEx<tagged_error>(LOGNAME "Could not open " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) +
								  " ban file `" + fn + "' for writing (" + errMessage().hideAsString() + ")");
		}
		try 
		{
			banEntrySave entrySave;
			entrySave.f = f;
			entrySave.stream_ID = stream_ID;
			for_each(m_list.begin(),m_list.end(),bind2nd(mem_fun_ref(&banEntry::save),entrySave));
		}
		catch(...)
		{
			if (f) ::fclose(f);
			throw;
		}
		if (f) ::fclose(f);

		if (!uniFile::fileSize(fn))
		{
			uniFile::unlink(fn);
		}
	}

	bool add(const utf8 &ipAddr, const __uint32 mask, const utf8 &comment, const size_t stream_ID) throw(exception)
	{
		// skip loopback addresses as we treat them specially anyway
		if ((ipAddr.find(utf8("127.")) == utf8::npos))
		{
			banEntry e(ipAddr, mask,comment, stream_ID);
			if (!e.validIP())
			{
				throwEx<runtime_error>(LOGNAME "Invalid IP specified - `" + ipAddr + "'");
			}
			if (!e.validMask())
			{
				throwEx<runtime_error>(LOGNAME "Invalid MASK specified - `" + tos(mask) + "'");
			}

			stackLock sml(m_lock);
			m_list.push_back(e);
			return true;
		}
		return false;
	}

	// true if removed
	bool remove(const utf8 &ipAddr, const __uint32 mask, const size_t stream_ID,
				const bool allStream, const bool fallback = false, const bool use_lock = true)
	{
		if (use_lock)
		{
			stackLock sml(m_lock);
		}

		for (list<banEntry>::iterator i = m_list.begin(); i != m_list.end(); ++i)
		{
			if (
				(allStream || (!allStream && (((*i).m_numericIP == ipAddr) || ((*i).m_hostIP == ipAddr)) && ((*i).m_mask == mask))) && 
				((*i).m_stream_ID == stream_ID))
			{
				m_list.erase(i);
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
				return remove(sHost, mask, stream_ID, allStream, true, false);
			}
		}

		return false;
	}

	// true if found
	bool find(const utf8 &ipAddr, const size_t mask, const size_t stream_ID, const bool use_lock=true)
	{
		if (use_lock)
		{
			stackLock sml(m_lock);
		}

		if (!m_list.empty())
		{
			utf8 hostIP;
			in_addr_t ip = banEntry::stringToIP(ipAddr, hostIP);
			for (list<banEntry>::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
			{
				if (((*i).m_mask == mask) &&
					((*i).m_stream_ID == stream_ID) &&
					(ntohl((*i).m_ip) & ((*i).m_mask|0xffffff00)) ==
					(ntohl(ip) & ((*i).m_mask|0xffffff00)
					)
				)
				return true;
			}
		}

		return false;
	}

	// true if found
	bool find(const utf8 &ipAddr, const size_t stream_ID, const bool use_lock = true)
	{
		if (use_lock)
		{
			stackLock sml(m_lock);
		}

		if (!m_list.empty())
		{
			utf8 hostIP;
			in_addr_t ip = banEntry::stringToIP(ipAddr, hostIP);
			for (list<banEntry>::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
			{
				if (((*i).m_stream_ID == stream_ID) &&
					(ntohl((*i).m_ip) & ((*i).m_mask|0xffffff00)) ==
					(ntohl(ip) & ((*i).m_mask|0xffffff00)))
				{
					return true;
				}
			}
		}

		return false;
	}

	void get(std::vector<banList::ban_t> &bl, size_t stream_ID) throw()
	{
		stackLock sml(m_lock);

		for (list<banEntry>::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
		{
			if ((*i).m_stream_ID == stream_ID)
			{
				bl.push_back(*i);
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

banList::banList():m_impl(0)
{
	m_impl = new banList::impl;
}

banList::~banList() throw()
{
	forget(m_impl);
}

bool banList::load(const uniFile::filenameType &fn, size_t stream_ID) throw()
{
	assert(m_impl);

	bool result(false);

	try
	{
		result = m_impl->load(fn,stream_ID);
	}
	catch(const exception &ex)
	{
		ELOG(ex.what());
	}
	return result;
}

bool banList::save(const uniFile::filenameType &fn, size_t stream_ID) throw()
{
	assert(m_impl);

	bool result(false);

	try
	{
		m_impl->save(fn,stream_ID);
		result = true;
	}
	catch(const exception &ex)
	{
		ELOG(ex.what());
	}
	return result;
}

// throws if parameters are invalid
bool banList::add(const utf8 &ipAddr, const __uint32 mask, const utf8 &comment, const size_t stream_ID) throw(exception)
{
	assert(m_impl);
	return m_impl->add(ipAddr, mask, comment, stream_ID);
}

// true if removed
bool banList::remove(const utf8 &ipAddr, const __uint32 mask, const size_t stream_ID, const bool allStream) throw()
{
	assert(m_impl);
	return m_impl->remove(ipAddr, mask, stream_ID, allStream);
}

// true if found
bool banList::find(const utf8 &ipAddr, const size_t stream_ID) throw()
{
	assert(m_impl);
	return m_impl->find(ipAddr, stream_ID);
}

// true if found
bool banList::find(const utf8 &ipAddr, const __uint32 mask, const size_t stream_ID) throw()
{
	assert(m_impl);
	return m_impl->find(ipAddr, mask, stream_ID);
}

void banList::get(vector<banList::ban_t> &bl, const size_t stream_ID) throw()
{
	assert(m_impl);
	m_impl->get(bl, stream_ID);
}
