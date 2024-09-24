#include <algorithm>
#include <stdio.h>
#include "agentList.h"
#include "global.h"
#include "stl/stringUtils.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"
#include "macros.h"
#include <assert.h>

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define LOGNAME "[AGENT] "

agentList g_agentList;

class agentList::impl
{
private:
	struct agentEntrySave
	{
		FILE *f;
		size_t stream_ID;
	};

	struct agentEntry: public agent_t
	{
		void save(agentEntrySave entrySave) throw(exception)
		{
			if(m_stream_ID == entrySave.stream_ID)
			{
				utf8 s(m_agent + eol());
				if (fwrite(s.c_str(),1,s.size(),entrySave.f) != s.size())
				{
					throwEx<tagged_error>(LOGNAME "I/O error writing " + (!entrySave.stream_ID ? "global" : "sid=" + tos(entrySave.stream_ID)) + " agent file");
				}
			}
		}

		agentEntry(const utf8 &agent, const size_t stream_ID) throw() : agent_t(agent, stream_ID) {}
		agentEntry() throw() {}
	};

	AOL_namespace::mutex	m_lock;
	list<agentEntry>		m_list;

public:
	bool load(const uniFile::filenameType &fn, size_t stream_ID) throw(exception)
	{
		if (fn.empty())
		{
			throwEx<tagged_error>(LOGNAME "No " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " agent file");
		}
		else if (gOptions.microServerDebug())
		{
			DLOG(LOGNAME "Attempting to read agent file: " + fileUtil::getFullFilePath(fn));
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
					s = &(buffer[3]);
				else
					s = buffer;

				if (stripWhitespace(s).empty())
				{
					WLOG(LOGNAME "Line " + tos(l) + " of " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " user agent list has been ignored");
				}
				else
				{
					agentEntry e(stripWhitespace(s),stream_ID);
					if(this->find(e.m_agent,e.m_stream_ID,false) == false)
					{
						m_list.push_back(e);
						++count;
					}
				}
			}
			if (!updating)
			{
				ILOG(LOGNAME "Loaded " + tos(count) + " blocked user agents" + (count != 1 ? "'s" : "") + " from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " agent file");
			}
			else
			{
				ILOG(LOGNAME "Reloaded " + tos(count) + " blocked user agents" + (count != 1 ? "'s" : "") + " from " + (!stream_ID ? "global" : "sid=" + tos(stream_ID)) + " agent file");
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
								  " agent file `" + fn + "' for writing (" + errMessage().hideAsString() + ")");
		}
		try 
		{
			agentEntrySave entrySave;
			entrySave.f = f;
			entrySave.stream_ID = stream_ID;
			for_each(m_list.begin(),m_list.end(),bind2nd(mem_fun_ref(&agentEntry::save),entrySave));
		}
		catch(...)
		{
			if (f) ::fclose(f);
			throw;
		}
		if (f) ::fclose(f);

		if(!uniFile::fileSize(fn))
		{
			uniFile::unlink(fn);
		}
	}

	bool add(const utf8 &agent, const size_t stream_ID, const bool soft) throw(exception)
	{
		if (agent.empty())
		{
			if (!soft) throwEx<runtime_error>(LOGNAME "Empty User Agent specified");
			else return false;
		}

		agentEntry e(agent,stream_ID);
		stackLock sml(m_lock);
		m_list.push_back(e);
		return true;
	}

	// true if removed
	bool remove(const utf8 &agent, const size_t stream_ID, const bool allStream) throw()
	{
		stackLock sml(m_lock);

		for (list<agentEntry>::iterator i = m_list.begin(); i != m_list.end(); ++i)
		{
			if (allStream || (((!allStream && ((*i).m_agent == agent))) && ((*i).m_stream_ID == stream_ID)))
			{
				m_list.erase(i);
				return true;
			}
		}

		return false;
	}

	// true if found
	bool find(const utf8 &agent, size_t stream_ID, bool use_lock = true) throw()
	{
		if(use_lock)
		{
			stackLock sml(m_lock);
		}

		if(!m_list.empty())
		{
			for (list<agentEntry>::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
			{
				if (((*i).m_stream_ID == stream_ID) && (agent == (*i).m_agent))
				{
					return true;
				}
			}
		}

		return false;
	}

	void get(std::vector<agentList::agent_t> &rl, size_t stream_ID) throw()
	{
		stackLock sml(m_lock);

		for (list<agentEntry>::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
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

agentList::agentList():m_impl(0)
{
	m_impl = new agentList::impl;
}

agentList::~agentList() throw()
{
	forget(m_impl);
}

bool agentList::load(const uniFile::filenameType &fn,size_t stream_ID) throw()
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

bool agentList::save(const uniFile::filenameType &fn,size_t stream_ID) throw()
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
bool agentList::add(const utf8 &agent, const size_t stream_ID, bool soft) throw(exception)
{
	assert(m_impl);
	return m_impl->add(agent,stream_ID,soft);
}

// true if removed
bool agentList::remove(const utf8 &agent, const size_t stream_ID, bool allStream) throw()
{
	assert(m_impl);
	return m_impl->remove(agent,stream_ID,allStream);
}

// true if found
bool agentList::find(const utf8 &agent, const size_t stream_ID) throw()
{
	assert(m_impl);
	return m_impl->find(agent, stream_ID);
}

void agentList::get(vector<agentList::agent_t> &bl,size_t stream_ID) throw()
{
	assert(m_impl);
	m_impl->get(bl,stream_ID);
}
