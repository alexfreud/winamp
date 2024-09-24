#pragma once
#ifndef banList_H_
#define banList_H_

#include <map>
#include "unicode/uniFile.h"

// class that manages list of addresses banned from the system

class banList
{
private:
	class impl;
	impl	*m_impl;

public:
	struct ban_t
	{
		size_t				m_mask;
		size_t				m_stream_ID;	// used to differentiate
		uniString::utf8		m_numericIP;
		uniString::utf8		m_comment;		// hostname or other symbolic name
		uniString::utf8		m_hostIP;		// used to hold the converted IP from a hostname

		ban_t(const uniString::utf8 &numericIP, const __uint32 mask, const uniString::utf8 &comment, const size_t stream_ID) throw()
			: m_mask(mask), m_stream_ID(stream_ID), m_numericIP(numericIP), m_comment(comment) {}

		ban_t() throw() : m_mask(0), m_stream_ID(1){}
	};

	// throws if parameters are invalid
	bool add(const uniString::utf8 &ipAddr, const __uint32 mask, const uniString::utf8 &comment, const size_t stream_ID) throw(std::exception);
	// true if removed
	bool remove(const uniString::utf8 &ipAddr, const __uint32 mask, const size_t stream_ID, const bool allStream) throw();
	// true if found
	bool find(const uniString::utf8 &ipAddr, const size_t stream_ID) throw();
	bool find(const uniString::utf8 &ipAddr, const __uint32 mask, const size_t stream_ID) throw();

	bool load(const uniFile::filenameType &fn, const size_t stream_ID) throw();
	bool save(const uniFile::filenameType &fn, const size_t stream_ID) throw();

	banList();
	~banList() throw();

	// for web administration reference
	void get(std::vector<ban_t> &bl, const size_t stream_ID) throw();
};

extern banList g_banList;

#endif
