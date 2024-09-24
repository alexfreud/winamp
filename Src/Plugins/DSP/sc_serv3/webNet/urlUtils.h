#pragma once
#ifndef _urlUtils_H_
#define _urlUtils_H_

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <stdexcept>
#include "unicode/uniString.h"
#include "stl/stringUtils.h"

//*************************************************************************//
/*  urlUtils

	Collection of classes to help create properly escaped URLs, and to
	parse out and convert escaped URLs (see RFC 2396).

	1) Creating a properly escaped url

		Create an urlBuilder object. Add the query data to it. Call escape() to
		retrieve the entire URL as a properly escaped string.

		urlBuilder ub("http",urlAuthority("127.0.0.1","80","neil%%%radisch"),"/this/is/a/test");
		ub.addQueryEntry(urlQueryEntry("full name","Neil Mark Radisch"));
		ub.addQueryEntry(urlQueryEntry("flag",""));
		ub.addQueryEntry(urlQueryEntry("wierd_stuff","!@#$%^&&&===\"\'*()"));
		cout << ub.escape() << endl; // print escaped url to screen

	2) Parse out query data from an escaped url

		These utilities do not parse out a complete url. They were designed
		to work in conjunction with the mclib webserver utility which
		takes care of parsing out everything up to the query string. As such,
		the parsing here is limited to the query.

		urlQuery uq(urlQuery::parse(getEscapedQueryString())); // parse the query string
		// now walk through the results and print to screen
		for (urlQuery::const_iterator i = uq.begin(); i != uq.end(); ++i)
		{
			cout << "key = " << (*i).getKey() << endl;
			cout << "value = " << (*i).getValue() << endl;
			cout << endl;
		}
*/
//*************************************************************************//

namespace urlUtils {

	class urlQueryEntry
	{
		std::pair<uniString::utf8, uniString::utf8> m_entry;
	public:
		urlQueryEntry(){}
		template<typename KEY, typename VALUE> // can be string or utf8
		urlQueryEntry(const KEY &key, const VALUE &value)
			:m_entry(key, value){}
		urlQueryEntry(const urlQueryEntry &uqe)
			:m_entry(uqe.m_entry){}
		urlQueryEntry& operator = (const urlQueryEntry &uqe)
			{ m_entry = uqe.m_entry; return *this; }
		bool operator == (const urlQueryEntry &uqe) const throw() { return m_entry == uqe.m_entry; }

		// set with unescaped data
		template<typename KEY,typename VALUE> // string or utf8
		void set(const KEY &key, const VALUE &value)
			{ m_entry = std::pair<uniString::utf8, uniString::utf8>(key, value); }

		// return query as escaped string
		std::string escape() const throw();

		// parse from escaped data
		static urlQueryEntry parse(const std::string &s) throw();

		// get unescaped values
		std::pair<uniString::utf8, uniString::utf8> get() const throw() { return m_entry; }
		uniString::utf8 getKey() const throw()	{ return m_entry.first; }
		uniString::utf8 getValue() const throw() { return m_entry.second; }
	};

	class urlQuery
	{
		std::vector<urlQueryEntry>	m_query;
	public:
		typedef std::vector<urlQueryEntry>::iterator iterator;
		typedef std::vector<urlQueryEntry>::const_iterator const_iterator;

		urlQuery(){}
		bool operator==(const urlQuery &uq) const throw() { return m_query == uq.m_query; }

		void addQueryEntry(const urlQueryEntry &uqe) { m_query.push_back(uqe); }
		void addQueryEntry(const std::string &key, const std::string &value) { m_query.push_back(urlQueryEntry(key, value)); }
		void addQueryEntry(const uniString::utf8 &key, const uniString::utf8 &value) { m_query.push_back(urlQueryEntry(key, value)); }
		std::string escape() const throw();
		bool empty() const throw()	{ return m_query.empty(); }
		size_t size() const throw() { return m_query.size(); }
		urlQueryEntry& operator[](size_t index) { return m_query[index]; }
		iterator begin() throw() { return m_query.begin(); }
		iterator end() throw() { return m_query.end(); }
		const_iterator begin() const throw() { return m_query.begin(); }
		const_iterator end() const throw() { return m_query.end(); }
		void clear() throw() { m_query.clear(); }

		static urlQuery parse(const std::string &s) throw();
		static void parse(const std::string &s, urlQuery &q) throw();

		void parseString(const std::string &s) throw() { parse(s,*this); }

		urlQuery::const_iterator findKey(const uniString::utf8 &key) const throw()
		{
			for (std::vector<urlQueryEntry>::const_iterator i = m_query.begin(); i != m_query.end(); ++i)
			{
				if ((*i).getKey() == key)
					return i;
			}
			return m_query.end();
		}

		bool hasKey(const uniString::utf8 &key) const throw() { return (findKey(key) != m_query.end()); }

		template<typename T>
		T keyValue(const uniString::utf8 &key,const T &deflt) const throw()
		{
			urlQuery::const_iterator i = findKey(key);
			return (i != m_query.end() ? (*i).getValue() : deflt);
		}
		template<typename T>
		T keyValue(const uniString::utf8 &key) const throw()
		{
			return keyValue(key,T());
		}
	};

	template<>
	inline bool urlQuery::keyValue(const uniString::utf8 &key, const bool &deflt) const throw()
	{
		urlQuery::const_iterator i = findKey(key);
		if (i == m_query.end())
		{
			return deflt;
		}

		uniString::utf8 v = (*i).getValue();
		while (!v.empty() && stringUtil::safe_is_space(v[0]))
		{
			v = v.substr(1);
		}
		if (v.empty())
		{
			return deflt;
		}
		return (v[0] == 't' || v[0] == 'T' || v[0] == '1' || v[0] == 'y' || v[0] == 'Y');
	}

	template<>
	inline int urlQuery::keyValue(const uniString::utf8 &key, const int &deflt) const throw()
	{
		urlQuery::const_iterator i = findKey(key);
		return (i != m_query.end() ? atoi((const char *)(*i).getValue().c_str()) : deflt);
	}

	template<>
	inline long urlQuery::keyValue(const uniString::utf8 &key, const long &deflt) const throw()
	{
		urlQuery::const_iterator i = findKey(key);
		return (i != m_query.end() ? atol((const char *)(*i).getValue().c_str()) : deflt);
	}

	template<>
	inline unsigned int urlQuery::keyValue(const uniString::utf8 &key, const unsigned int &deflt) const throw()
	{
		urlQuery::const_iterator i = findKey(key);
		return (i != m_query.end() ? (unsigned int)atol((const char *)(*i).getValue().c_str()) : deflt);
	}

#ifdef _WIN32
	template<>
	inline __int64 urlQuery::keyValue(const uniString::utf8 &key, const __int64 &deflt) const throw()
	{
		urlQuery::const_iterator i = findKey(key);
		return (i != m_query.end() ? _atoi64((const char *)(*i).getValue().c_str()) : deflt);
	}
#endif

	template<>
	inline bool urlQuery::keyValue<bool>(const uniString::utf8 &key) const throw()
	{
		return keyValue(key, false);
	}

	std::string escape(const std::string &s) throw();
	uniString::utf8 unescapeString(const std::string &s) throw();
	std::string escapeURI_RFC3986(const uniString::utf8 &s) throw();
}

#endif
