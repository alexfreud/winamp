#include "urlUtils.h"
#include "stl/stringUtils.h"
#include <set>
#include <ctype.h>
#ifdef _WIN32
#include <crtdbg.h>
#define snprintf _snprintf
#define unused_attribute
#else
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define _ASSERTE assert
#define unused_attribute __attribute__((unused))
#endif

using namespace std;
using namespace stringUtil;

static string gExcluded = " <>#%\"{}|\\^[]`";
static string gReserved = ";/?:@&=+$,";
static string gMark = "-_.!~*'()";
static string gDigit = "0123456789";
static string gLowAlpha = "abcdefghijklmnopqrstuvwxyz";
static string gHiAlpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static string gUnreserved_RFC3986 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
static string gReserved_RFC3986 = "!*'();:@&=+$,/?%#[]";

template<class T>
static set<T> setunion(const set<T> &s1, const set<T> &s2)
{
	set<T> result(s1);
	result.insert(s2.begin(), s2.end());
	return result;
}

static set<char> gSetExcluded(gExcluded.begin(), gExcluded.end());
static set<char> gSetReserved(gReserved.begin(), gReserved.end());
static set<char> gSetExcludedAndReserved(setunion(gSetExcluded, gSetReserved));
static set<char> gSetMark(gMark.begin(), gMark.end());
static set<char> gSetLowAlpha(gLowAlpha.begin(), gLowAlpha.end());
static set<char> gSetHiAlpha(gHiAlpha.begin(), gHiAlpha.end());
static set<char> gSetDigit(gDigit.begin(), gDigit.end());
static set<char> gSetAlpha(setunion(gSetLowAlpha, gSetHiAlpha));
static set<char> gSetAlphanum(setunion(gSetAlpha, gSetDigit));
static set<char> gSetUnreserved(setunion(gSetAlphanum, gSetMark));
static set<char> gSetUnreserved_RFC3986(gUnreserved_RFC3986.begin(), gUnreserved_RFC3986.end());
static set<char> gSetReserved_RFC3986(gReserved_RFC3986.begin(), gReserved_RFC3986.end());

////////////////////////////////////////////////
////////// Escaping Funcs //////////////////////
///////////////////////////////////////////////

// convert a character to its hex representation
static string escapeChar(__uint8 c) throw()
{
	char buf[8] = {0};
	int len unused_attribute = snprintf(buf, sizeof(buf), "%%%02X", (int)c);
	_ASSERTE(len == 3);
	return buf;
}

// convert the char if it's in the set
static string escapeIfInSet(char c,const set<char> &st) throw()
{
	return (st.find(c) == st.end() ? string(1, c) : escapeChar(c)); 
}

static string escapeIfNotInSet(char c, const set<char> &st) throw()
{
	return (st.find(c) == st.end() ? escapeChar(c) : string(1, c));
}

template<typename ESC,typename STYPE>
static string escapeString(const STYPE &s, const set<char> &st, ESC func) throw()
{
	string result;
	for (typename STYPE::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		result += func(*i,st);
	}
	return result;
}

string urlUtils::escape(const std::string &s) throw()
{
	return escapeString(s, gSetExcludedAndReserved, escapeIfInSet);
}

string urlUtils::escapeURI_RFC3986(const uniString::utf8 &s) throw()
{
	return escapeString(s, gSetUnreserved_RFC3986, escapeIfNotInSet);
}

/////////////////////////////////////////////////////
//////////// Unescaping funcs //////////////////////
////////////////////////////////////////////////////

// convert %xx to a character
inline char unescapeSequence(const string &s) throw()
{
	_ASSERTE(s.size() == 3);
	_ASSERTE(s[0] == '%' && isxdigit(s[1]) && isxdigit(s[2]));
	unsigned int v = 0;
	sscanf(s.c_str(), "%%%02x", &v);
	return (char)v;
}

uniString::utf8 urlUtils::unescapeString(const string &s) throw()
{
	string result;
	string escTok;
	int ccnt(0);

	for (string::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		bool escChar(false);
		switch (ccnt)
		{
			case 0:
			{
				escChar = ((*i) == '%');
				break;
			}
			case 1:
			{
				escChar = (isxdigit(*i) ? true : false);
				break;
			}
			case 2:
			{
				escChar = (isxdigit(*i) ? true : false);
				break;
			}
		}

		if (escChar)
		{
			escTok += (*i);
			++ccnt;
		}
		else
		{
			result += escTok;
			ccnt = 0;
			if ((*i) == '+')
			{
				result += " ";
			}
			else
			{
				result += (*i);
			}
		}
		if (ccnt == 3)
		{
			result += unescapeSequence(escTok);
			escTok = "";
			ccnt = 0;
		}
	}

	result += escTok;
	return uniString::utf8(result);
}

////////////////////////////////////////////
/////////// Classes ////////////////////////
string urlUtils::urlQueryEntry::escape() const throw()
{
	string result(escapeString(m_entry.first, gSetExcludedAndReserved, escapeIfInSet));

	if (!m_entry.second.empty())
	{
		result += "=" + escapeString(m_entry.second, gSetExcludedAndReserved, escapeIfInSet);
	}

	return result;
}

urlUtils::urlQueryEntry urlUtils::urlQueryEntry::parse(const string &s) throw()
{
	urlUtils::urlQueryEntry result;

	if (!s.empty())
	{
		string::size_type pos = s.find("=");
		result.m_entry.first = unescapeString(s.substr(0,pos));
		result.m_entry.second = (pos == string::npos ? "" : unescapeString(s.substr(pos+1)));
	}

	return result;
}

string urlUtils::urlQuery::escape() const throw()
{
	string result;

	for (vector<urlQueryEntry>::const_iterator i = m_query.begin(); i != m_query.end(); ++i)
	{
		result += (*i).escape();
		if (i + 1 != m_query.end())
		{
			result += "&";
		}
	}

	return result;
}

urlUtils::urlQuery urlUtils::urlQuery::parse(const std::string &sin) throw()
{
	urlUtils::urlQuery result;
	parse(sin, result);
	return result;
}

void urlUtils::urlQuery::parse(const std::string &sin, urlQuery &q) throw()
{
	q.clear();
	string s(sin);

	while (s != "")
	{
		if (!s.empty())
		{
			string::size_type pos = s.find("&");
			q.m_query.push_back(urlUtils::urlQueryEntry::parse(s.substr(0, pos)));
			if (pos == string::npos)
			{
				break;
			}
			s = s.substr(pos + 1);
		}
		else
		{
			break;
		}
	}
}
