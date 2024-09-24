#pragma once
#ifndef w3cLog_H_
#define w3cLog_H_

#include "unicode/uniString.h"
#include "unicode/uniFile.h"

class w3cLog
{
public:
	static void open(const uniFile::filenameType &fn, const size_t streamID = 0) throw();
	static void header(FILE *w3cFileHandle) throw();
	static void close(const size_t streamID) throw();
	static void rotate_log(const uniFile::filenameType &fn, const size_t streamID = 0) throw();
	static void log(const size_t streamID, const uniString::utf8 &ipAddr, const uniString::utf8 &hostName,
					const uniString::utf8 &songName, const uniString::utf8 &userAgent, __uint64 bytesSent,
					time_t timeInSeconds, const int bitrate);
};

#endif
