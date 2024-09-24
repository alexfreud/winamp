#pragma once
#ifndef _stringUtils_H_
#define _stringUtils_H_

#include <sstream>
#include <string>
#include <functional>
#include <vector>
#include <locale>
#include <ctype.h>

namespace stringUtil
{
	#ifdef _WIN32
	inline std::string eol() throw() { return "\r\n"; }
	#elif MACINTOSH
	inline std::string eol() throw() { return "\r"; }
	#else
	inline std::string eol() throw() { return "\n"; }
	#endif

	//////////////////////////////////////////////////////////////////
	//////////////////// String Conversion ///////////////////////////
	//////////////////////////////////////////////////////////////////

	// the 'safe' functions are to avoid bad_cast exceptions when the
	// passed in type does not have a facet, or the facet indicates that
	// the value is out of range. Since I may be running this against Unicode
	// strings, I need to deal with this. Most likely there's a way to set the
	// proper locale and facet so the error doesn't occur, but this is beyond
	// me right now - NMR
	template<typename T>
	bool safe_is_alpha(T v)
	{
		if (((unsigned int)v) > 0x7f) return false;
		return std::isalpha((char)v,std::locale());
	}

	template<typename T>
	bool safe_is_digit(T v)
	{
		if (((unsigned int)v) > 0x7f) return false;
		return std::isdigit((char)v,std::locale());
	}

	template<typename T>
	bool safe_is_space(T v)
	{
		if (((unsigned int)v) > 0x7f) return false;
		return std::isspace((char)v,std::locale());
	}

	template<typename T>
	T safe_to_lower(T v)
	{
		if (((unsigned int)v) > 0x7f) return v;
		return std::tolower((char)v,std::locale());
	}

	template<typename T>
	T safe_to_upper(T v)
	{
		if (((unsigned int)v) > 0x7f) return v;
		return std::toupper((char)v,std::locale());
	}

	template<typename S>
	S toLower(const S &s) throw()
	{
		S result;
		for (typename S::const_iterator i = s.begin(); i != s.end(); ++i)
		{
			result += (stringUtil::safe_is_alpha(*i) ? stringUtil::safe_to_lower(*i) : *i);
		}

		return result;
	}

	template<typename S>
	S toUpper(const S &s) throw()
	{
		S result;
		for (typename S::const_iterator i = s.begin(); i != s.end(); ++i)
		{
			result += (stringUtil::safe_is_alpha(*i) ? stringUtil::safe_to_upper(*i) : *i);
		}

		return result;
	}

	template<typename S>
	inline bool compareStringsWithoutCase(const S &s1,const S &s2) throw()
	{
		S s1c(toLower(s1));
		S s2c(toLower(s2));
		return (s1c == s2c);
	}

	inline std::string dosToUnix(const std::string &s) throw()
	{
		std::string result;
		bool r(false);
		for (std::string::const_iterator i = s.begin(); i != s.end(); ++i)
		{
			if ((*i) == '\n')
			{
				if (r)
				{
					result += '\n';
				}
				else
				{
					result.push_back(*i);
				}
				r = false;
			}
			else if ((*i) == '\r')
			{
				if (r)
					result.push_back('\r');
				r = true;
			}
			else
			{
				if (r)
					result.push_back('\r');
				r = false;
				result.push_back(*i);
			}
		}
		return result;
	}

	inline std::string unixToDos(const std::string &s) throw()
	{
		std::string result;
		for (std::string::const_iterator i = s.begin(); i != s.end(); ++i)
		{
			if ((*i) == '\n')
				result += "\r\n";
			else 
				result.push_back(*i);
		}
		return result;
	}

	//***************************************************************
	//* tos, tows
	//*
	//* These templates and overloaded functions allow you to convert
	//* any streamable value to a string or wide-string. It can be a 
	//* great convenience to be able to do
	//*
	//* string("test five ") + tos(5);
	//*
	//* instead of
	//*
	//* ostringstream o;
	//* o << "test five " << 5;
	//* o.str()
	//*
	//* tows is just like tos, but for wide strings. You can also use these
	//* to convert standard strings to wide strings and vice-versa
	//*
	//* for example:
	//*
	//* wstring ws = tows(string("hello"));
	//* string s = tos(wstring(L"goodbye"));
	//* 
	//************************************************************************

	template<typename S,typename T>
	S tobs(T v) // to basic string
	{
		std::basic_ostringstream<typename S::value_type> o;
		o << v;
		return o.str();
	}

	template<typename S>
	S tobs(const S v) { return v; }
	template<typename S>
	S tobs(const typename S::value_type *v) { return S(v); }

	template<typename t>
	std::string tos(t v)
	{
		std::ostringstream o;
		o << v;
		return o.str();
	}

	inline std::string tos(const std::string &v) { return v; }
	inline std::string tos(const char *v) { return std::string(v); }

	template<typename t>
	std::string tohex(t v)
	{
		std::ostringstream o;
		o << std::hex << v;
		return o.str();
	}

	#ifdef _WIN32
	template<typename t>
	std::wstring tows(t v)
	{
		std::wostringstream o;
		o << v;
		return o.str();
	}

	inline std::string tos(const wchar_t *value)
	{
		if (!value) return "";

		size_t len = ::wcslen(value);

		char *s = new char[len + 1];
		::wcstombs(s,value,len);
		s[len] = '\0';
		std::string result(s);
		delete [] s;

		return result;
	}

	inline std::string tos(const std::wstring &s)
	{
		return tos(s.c_str());
	}

	inline std::wstring tows(const char *value)
	{
		if (!value) return L"";

		size_t len = ::strlen(value);

		wchar_t *s = new wchar_t[len + 1];
		::mbstowcs(s,value,len);
		s[len] = 0;
		std::wstring result(s);
		delete [] s;

		return result;
	}

	inline std::wstring tows(const std::string &value)
	{
		return tows(value.c_str());
	}
	#endif

	////////////////////////////////////////////////////////////////////
	////////////////////// String stripping ////////////////////////////
	////////////////////////////////////////////////////////////////////

	//******************************************************************
	//* A collection of templates and functions to allow stripping of
	//* leading and trailing items from a string
	//*
	//* --- stripItem
	//*
	//* Strip leading and trailing items from a container
	//* template parameters are the container type, and a function which returns 
	//* true if we have the value to strip
	//*
	//*
	//* ---- stripChar
	//* strips characters off a string by either a function (which returns
	//* true if the character is found), or by an actual character mathc.
	//* There are also forms for wide strings
	//*
	//* ---- stripWhitespace
	//* removes leading and trailing spaces from a string using isspace()
	//*************************************************************************
	template <typename S,typename F>
	S stripItem(const S &s,F func,bool stripLeading = true, bool stripTrailing = true)
	{
		typename S::const_iterator leftit  = s.begin();
		typename S::const_reverse_iterator rightit = s.rbegin(); 

		if (s.length() < 1 )
			return s;

		if (stripLeading)
		{
			while (leftit != s.end() && func(*leftit))
			{
				++leftit;
			}
		}
		if (leftit == s.end()) return S();

		if (stripTrailing)
		{
			while (rightit != s.rend() && func(*rightit))
			{
				++rightit;
			}
		}
		if (rightit == s.rend()) return S();

		typename S::const_iterator endpnt = (++rightit).base();

		if (leftit > endpnt)
			return S();

		return s.substr( (leftit - s.begin()) ,(endpnt-leftit) +1);
	}

	template<typename S,typename FUNC>
	S stripChar(const S &s,FUNC f,bool stripLeading = true,bool stripTrailing = true) 
		{ return stripItem(s,f,stripLeading,stripTrailing); }

	template<typename S>
	inline S stripChar(const S &s,typename S::value_type c,bool stripLeading = true,bool stripTrailing = true) 
		{ return stripItem(s,bind1st(std::equal_to<typename S::value_type>(),c),stripLeading,stripTrailing); }

	template<typename S>
	bool myspace(S c) { return safe_is_space(c); }
	template<typename S>
	inline S stripWhitespace(const S &s) { return stripChar(s,stringUtil::myspace<typename S::value_type>); }

	template<typename S>
	bool myalphadigit(S c) { return !(safe_is_alpha(c) || safe_is_digit(c)); }
	template<typename S>
	inline S stripAlphaDigit(const S &s) { return stripChar(s,stringUtil::myalphadigit<typename S::value_type>); }

	//*****************************************************************
	//* tokenizer
	//*
	//* break up a string into substrings based on a delimiter item.
	//******************************************************************
	template<typename S,typename F>
	std::vector<S> tokenizer_if(const S &ins,F isdelimiter) throw()
	{
		std::vector<S> result;
		S accum;

		for (typename S::const_iterator i = ins.begin(); i != ins.end(); ++i)
		{
			if (!isdelimiter(*i))
			{
				accum.push_back(*i);// was +=
			}
			else
			{
				if (!accum.empty())
				{
					result.push_back(accum);
					accum = S();
				}
			}
		}

		if (!accum.empty())
		{
			result.push_back(accum);
		}
		return result;
	}

	template<typename S>
	inline std::vector<S> tokenizer(const S &ins,typename S::value_type delim) throw()
		{ return tokenizer_if(ins,bind1st(std::equal_to<typename S::value_type>(),delim)); }

	inline std::string escapeBackslashes(const std::string &s) throw()
	{
		std::string result;
		for (std::string::const_iterator i = s.begin(); i != s.end(); ++i)
		{
			result += (*i);
			if ((*i) == '\\')
				result += (*i);
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
}

#endif
