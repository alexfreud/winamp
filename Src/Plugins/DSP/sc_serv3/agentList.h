#pragma once
#ifndef agentList_H_
#define agentList_H_

#include "unicode/uniFile.h"

// class that manages lists of blocked user agents

class agentList
{
private:
	class impl;
	impl *m_impl;

public:
	struct agent_t
	{
		uniString::utf8		m_agent;		// used to hold the user agent to not allow
		size_t				m_stream_ID;	// used to differentiate

		agent_t(const uniString::utf8 &agent, const size_t stream_ID) throw() : m_agent(agent), m_stream_ID(stream_ID) {}
		agent_t() throw() : m_stream_ID(0) {}
	};

	// throws if parameters are invalid
	bool add(const uniString::utf8 &agent, const size_t stream_ID, const bool soft) throw(std::exception);
	// true if removed
	bool remove(const uniString::utf8 &agent, const size_t stream_ID, const bool allStream) throw();
	// true if found
	bool find(const uniString::utf8 &agent, const size_t stream_ID) throw();

	bool load(const uniFile::filenameType &fn, const size_t stream_ID) throw();
	bool save(const uniFile::filenameType &fn, const size_t stream_ID) throw();

	agentList();
	~agentList() throw();

	// for web administration reference
	void get(std::vector<agent_t> &rl, const size_t stream_ID) throw();
};

extern agentList g_agentList;

#endif
