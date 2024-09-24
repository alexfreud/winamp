#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <algorithm>
#include <stdio.h>
#include "ripList.h"
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

#define LOGNAME "[RIP] "

#ifdef _WIN32
typedef unsigned long in_addr_t;
#endif

ripList g_ripList;

class ripList::impl
{
private:
	struct ripEntrySave
	{
		FILE *f;
		size_t stream_ID;
	};

	struct ripEntry: public rip_t
	{
		in_addr_t	m_ip;		// ip as binary type. Old style, but that's how the old sc_serv did it and we'll
								// continue to do it that way until we're ready to break the old software

		void save(ripEntrySave entrySave) throw(exception)
		{
			if (m_stream_ID == entrySave.stream_ID)
			{
				utf8 s(m_numericIP + eol());
				if (fwrite(s.c_str(),1,s.size(),entrySave.f) != s.size())
				{
					throwEx<tagged_error>(LOGNAME "I/O error writing " + (!entrySave.stream_ID ? "global" : "sid=" + tos(entrySave.stream_ID)) + " rip file");
				}
			}
		}

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

		ripEntry(const utf8 &numericIP, const size_t stream_ID) throw() : rip_t(numericIP, stream_ID), m_ip(stringToIP(numericIP, m_hostIP)) {}
		ripEntry() throw() : m_ip(0) {}
	};

	AOL_namespace::mutex	m_lock;
	list<ripEntry>			m_list;

public:
	bool load(const uniFile::filenameType &fn, size_t stream_ID) throw(exception)
	{
		if (fn.empty())
		{
			throwEx<tagged_error>(LOGNAME "No " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip file");
		}
		else if (gOptions.microServerDebug())
		{
			DLOG(LOGNAME "Attempting to read rip file: " + fileUtil::getFullFilePath(fn));
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

				ripEntry e(stripWhitespace(s),stream_ID);

				if (!e.validIP())
				{
					WLOG(LOGNAME "Line " + tos(l) + " of " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip list has been ignored (bad IP)");
				}
				else
				{
					if (this->find(e.m_numericIP,e.m_stream_ID,false) == false)
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
					ILOG(LOGNAME "Reserved " + tos(count) + " IP" + (count != 1 ? "'s" : "") +
								 " from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip file");
				}
				else if (gOptions.microServerDebug())
				{
					DLOG(LOGNAME "No IPs read from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip file");
				}
			}
			else
			{
				ILOG(LOGNAME "Reloaded " + tos(count) + " Reserved IP" + (count != 1 ? "'s" : "") +
					 " from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " rip file");
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

	void save(const uniFile::filenameType &fn,size_t stream_ID) throw(exception)
	{
		stackLock sml(m_lock);

		FILE *f = uniFile::fopen(fn,"wb");
		if (!f)
		{
			throwEx<tagged_error>(LOGNAME "Could not open " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) +
								  " rip file `" + fn + "' for writing (" + errMessage().hideAsString() + ")");
		}
		try 
		{
			ripEntrySave entrySave;
			entrySave.f = f;
			entrySave.stream_ID = stream_ID;
			for_each(m_list.begin(),m_list.end(),bind2nd(mem_fun_ref(&ripEntry::save),entrySave));
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

	bool add(const utf8 &ipAddr, const size_t stream_ID, const bool soft) throw(exception)
	{
		// skip loopback addresses as we treat them specially anyway
		if ((ipAddr.find(utf8("127.")) == utf8::npos))
		{
			ripEntry e(ipAddr, stream_ID);
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
	bool remove(const utf8 &ipAddr, const size_t stream_ID, const bool allStream, const bool fallback = false, const bool use_lock = true)
	{
		if (use_lock)
		{
			stackLock sml(m_lock);
		}

		for (list<ripEntry>::iterator i = m_list.begin(); i != m_list.end(); ++i)
		{
			if (
				(allStream || ((!allStream && ((*i).m_numericIP == ipAddr)) || ((*i).m_hostIP == ipAddr))) &&
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
				return remove(sHost, stream_ID, allStream, true, false);
			}
		}

		return false;
	}

	// true if found
	bool find(const utf8 &ipAddr, const size_t stream_ID, const bool use_lock = true) throw()
	{
		if (use_lock)
		{
			stackLock sml(m_lock);
		}

		if (!m_list.empty())
		{
			for (list<ripEntry>::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
			{
				if (((*i).m_stream_ID == stream_ID) &&
					((ipAddr == (*i).m_numericIP) || (ipAddr == (*i).m_hostIP)))
				{
					return true;
				}
			}
		}

		return false;
	}

	void get(std::vector<ripList::rip_t> &rl, size_t stream_ID) throw()
	{
		stackLock sml(m_lock);

		for (list<ripEntry>::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
		{
			if ((*i).m_stream_ID == stream_ID)
			{
				rl.push_back(*i);
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ripList::ripList() : m_impl(0)
{
	m_impl = new ripList::impl;
}

ripList::~ripList() throw()
{
	forget(m_impl);
}

bool ripList::load(const uniFile::filenameType &fn, size_t stream_ID) throw()
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

bool ripList::save(const uniFile::filenameType &fn, size_t stream_ID) throw()
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
bool ripList::add(const utf8 &ipAddr, const size_t stream_ID, const bool soft) throw(exception)
{
	assert(m_impl);
	return m_impl->add(ipAddr, stream_ID, soft);
}

// true if removed
bool ripList::remove(const utf8 &ipAddr, const size_t stream_ID, const bool allStream) throw()
{
	assert(m_impl);
	return m_impl->remove(ipAddr, stream_ID, allStream);
}

// true if found
bool ripList::find(const utf8 &ipAddr, const size_t stream_ID) throw()
{
	assert(m_impl);
	return m_impl->find(ipAddr, stream_ID);
}

void ripList::get(vector<ripList::rip_t> &bl, const size_t stream_ID) throw()
{
	assert(m_impl);
	m_impl->get(bl, stream_ID);
}
