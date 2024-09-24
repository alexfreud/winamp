#pragma once
#ifndef baseOptions_H_
#define baseOptions_H_

#include <string>
#include <vector>
#include "unicode/uniFile.h"
#include "services/logger.h"

struct baseOptions
{
	uniFile::filenameType m_fileLog;
	bool m_consoleLogging;

	// convert a string to a boolean
	static bool toBool(const uniString::utf8 &s) throw();

	// process command line options. Options are key:value pairs. All unprocessed
	// options are returned
	const std::vector<uniString::utf8> fromArgs(const std::vector<uniString::utf8> &cl) throw();

	const uniFile::filenameType &getFileLog() const throw() { return m_fileLog; }
	bool getConsoleLogging() const throw() { return m_consoleLogging; }
	#ifdef _WIN32
	static uniString::utf8 getSystemLogConfigString() throw() { return AOL_logger::systemLogger_element::panicConfiguration(); }
	#else
	static uniString::utf8 getSystemLogConfigString() throw() { return "";}
	#endif
	static uniString::utf8 getVersionBuildStrings() throw();
};

#endif
