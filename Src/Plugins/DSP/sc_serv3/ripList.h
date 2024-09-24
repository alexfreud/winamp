#pragma once
#ifndef ripList_H_
#define ripList_H_

#include "unicode/uniFile.h"

// class that manages lists of reserved IPs
// these are remote addresses that must always be allowed in no matter what.

class ripList
{
private:
	class impl;
	impl	*m_impl;

public:
	struct rip_t
	{
		uniString::utf8		m_numericIP;
		uniString::utf8		m_hostIP;		// used to hold the converted IP from a hostname
		size_t				m_stream_ID;	// used to differentiate

		rip_t(const uniString::utf8 &numericIP, const size_t stream_ID) throw() : m_numericIP(numericIP), m_stream_ID(stream_ID) {}
		rip_t() throw() : m_stream_ID(0) {}
	};

	// throws if parameters are invalid
	bool add(const uniString::utf8 &ipAddr, const size_t stream_ID, const bool soft) throw(std::exception);
	// true if removed
	bool remove(const uniString::utf8 &ipAddr, const size_t stream_ID, const bool allStream) throw();
	// true if found
	bool find(const uniString::utf8 &ipAddr, const size_t stream_ID) throw();

	bool load(const uniFile::filenameType &fn, const size_t stream_ID) throw();
	bool save(const uniFile::filenameType &fn, const size_t stream_ID) throw();

	ripList();
	~ripList() throw();

	// for web administration reference
	void get(std::vector<rip_t> &rl, size_t stream_ID) throw();
};

extern ripList g_ripList;

#endif
