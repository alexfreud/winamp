#pragma once
#ifndef _fileUtils_H_
#define _fileUtils_H_

#include <string>
#include <vector>
#include <stdexcept>
#include "unicode/uniString.h"
#include "unicode/uniFile.h"

namespace fileUtil
{
	#ifdef _WIN32
	const char filePathDelimiter='\\';
	#else
	const char filePathDelimiter='/';
	#endif

	static uniString::utf8 getFilePathDelimiter()
	{
		#ifdef _WIN32
		return (uniString::utf8)"\\";
		#else
		return (uniString::utf8)"/";
		#endif
	}

	//********************************************
	//* stripPath
	//*
	//* remove the path from a filename
	//********************************************
	template<typename S>
	S stripPath(const S &s,const S &delim) throw()
	{
		typename S::size_type pos = s.find_last_of(delim);
		if (pos == S::npos) return s;
		if (pos == s.size() -1) return S();
		return s.substr(pos+1);
	}

	template<typename S>
	S stripPath(const S &s) throw()
	{
		return fileUtil::stripPath(s,S(1,(typename S::value_type)fileUtil::filePathDelimiter));
	}

	template<typename S>
	S onlyPath(const S &s,const S &delim) throw()
	{
		typename S::size_type pos = s.find_last_of(delim);
		if (pos == S::npos) return S();
		return s.substr(0,pos+1);
	}

	template<typename S>
	S onlyPath(const S &s) throw()
	{
		return fileUtil::onlyPath(s,S(1,(typename S::value_type)fileUtil::filePathDelimiter));
	}

	template<typename S>
	S mustEndIn(const S &s,typename S::value_type c) throw()
	{
		return ((s.empty() || ((*(s.rbegin())) != c)) ? (s+c) : s);
	}

	template<class S>
	S stripSuffix(const S &s,const S &delim) throw()
	{
		return s.substr(0,s.rfind(delim));
	}

	template<typename S>
	S stripSuffix(const S &s) throw()
	{
		return fileUtil::stripSuffix(s,S(1,(typename S::value_type)'.'));
	}

	template<class S>
	S getSuffix(const S &s,const S &delim) throw()
	{
		S empty;
		typename S::size_type pos = s.rfind(delim);
		if (pos == S::npos)
			return empty;
		return s.substr(pos+1);
	}

	template<typename S>
	S getSuffix(const S &s) throw()
	{ 
		return fileUtil::getSuffix(s,S(1,(typename S::value_type)'.')); 
	}

	////////////

	// return true if a file exists
	bool fileExists(const uniFile::filenameType &fullPath) throw();

	// return true if a directory exists
	bool directoryExists(const uniFile::filenameType &fullPath) throw();

	// get list of all files that match a pattern
	#ifdef _WIN32
	std::vector<std::wstring> directoryFileList(const std::wstring &pattern, const std::wstring &currentPath,
												bool fullPaths, bool preserveCase = false) throw();
	#else
	std::vector<std::string> directoryFileList(const std::string &pattern, const std::string &currentPath) throw();
	#endif

	bool convertOSFilePathDelimiter(uniString::utf8 &value) throw();

	uniFile::filenameType getFullFilePath(const uniFile::filenameType &partial) throw();
}

#endif
