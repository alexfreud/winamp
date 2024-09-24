#pragma once
#ifndef adminList_H_
#define adminList_H_

#include "unicode/uniFile.h"

// class that manages lists of reserved IPs
// these are remote addresses that must always be allowed in no matter what.

class adminList
{
private:
	class impl;
	impl	*m_impl;

public:
	struct admin_t
	{
		uniString::utf8		m_numericIP;
		uniString::utf8		m_hostIP;		// used to hold the converted IP from a hostname

		explicit admin_t(const uniString::utf8 &numericIP) throw() : m_numericIP(numericIP) {}
		admin_t() throw() {}
	};

	// throws if parameters are invalid
	bool add(const uniString::utf8 &ipAddr, const bool soft) throw(std::exception);
	// true if removed
	bool remove(const uniString::utf8 &ipAddr, const bool allStream) throw();
	// 1 if found, 0 if not, -1 if empty (assume allowed)
	int find(const uniString::utf8 &ipAddr) throw();

	bool load(const uniFile::filenameType &fn) throw();

	adminList();
	~adminList() throw();
};

extern adminList g_adminList;

#endif
