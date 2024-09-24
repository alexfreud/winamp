#include "w3cLog.h"
#include "webNet/urlUtils.h"
#include "threading/thread.h"
#include "global.h"
#include "file/fileUtils.h"
#include "stl/stringUtils.h"
#include "services/stdServiceImpl.h"
#include <stdio.h>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#endif

using namespace std;
using namespace uniString;
using namespace stringUtil;

std::map<size_t,FILE *> s_w3cFileHandle;

static AOL_namespace::mutex s_w3cLock;

void w3cLog::open(const uniFile::filenameType &fn, size_t streamID) throw()
{
	rotate_log(fn, streamID);

	stackLock sml(s_w3cLock);

	if (!s_w3cFileHandle[streamID])
	{
		s_w3cFileHandle[streamID] = uniFile::fopen(fn,"wb");
		if (!s_w3cFileHandle[streamID])
		{
			ELOG("[W3C] Could not open file " + fn + " (" + errMessage() + ")");
		}
		else
		{
			header(s_w3cFileHandle[streamID]);
		}
	}
}

void w3cLog::header(FILE *w3cFileHandle) throw()
{
	// output header like done in the v1 DNAS for tool compatibility
	utf8 version = gOptions.getVersionBuildStrings();
	utf8 s = "#Software: SHOUTcast" + eol() + "#Version: " + version + eol() +
			 "#Fields: c-ip c-dns date time cs-uri-stem c-status cs(User-Agent) sc-bytes x-duration avgbandwidth" + eol();
	size_t amt = ::fwrite(s.c_str(), 1, s.size(), w3cFileHandle);
	if (amt != s.size())
	{
		ELOG("[W3C] Write error");
	}
	::fflush(w3cFileHandle);
}

void w3cLog::close(size_t streamID) throw()
{
	stackLock sml(s_w3cLock);

	if (s_w3cFileHandle[streamID])
	{
		::fclose(s_w3cFileHandle[streamID]);
		s_w3cFileHandle[streamID] = 0;
	}
}

inline uniFile::filenameType make_backup_log(const uniFile::filenameType &filename, int which) throw()
{
	// 2.4.8+ we now ensure that this is giving a 'full path' to avoid issues
	return fileUtil::getFullFilePath((fileUtil::stripSuffix(filename) + "_" +
									 tobs<uniFile::filenameType>(which) + "." +
									 fileUtil::getSuffix(filename)));
}

uniFile::filenameType make_archive_log() throw()
{
#ifdef _WIN32
	SYSTEMTIME sysTime = {0};
	::GetLocalTime(&sysTime);
	wchar_t d[100] = {0}, t[100] = {0};
	::GetDateFormatW(LOCALE_SYSTEM_DEFAULT,0,&sysTime,_T("yyyy'_'MM'_'dd"),d,99);
	::GetTimeFormatW(LOCALE_SYSTEM_DEFAULT,0,&sysTime,_T("HH'_'mm'_'ss"),t,99);
	return tos((const wchar_t *)d) + "_" + tos((const wchar_t *)t);
#else
	char buf[256] = {0};
	struct tm ttm;
	time_t ttt;
	::time(&ttt);
	::strftime(buf, 255, "%Y_%m_%d_%H_%M_%S", ::localtime_r(&ttt, &ttm));
	return buf;
#endif
}

void w3cLog::rotate_log(const uniFile::filenameType &fn, size_t streamID) throw()
{
	stackLock sml(s_w3cLock);

	if (s_w3cFileHandle[streamID])
	{
		::fclose(s_w3cFileHandle[streamID]);
		s_w3cFileHandle[streamID] = 0;
	}

	int numFileBackups = gOptions.logRotates();
	for (int x = numFileBackups; x > 0; --x)
	{
		uniFile::filenameType dest = make_backup_log(fn, x);
		// archive the log file about to be removed into a gz file
		if (x > 0 && x == gOptions.logRotates())
		{
			#ifdef _WIN32
			uniFile::filenameType archive = dest;
			utf8::size_type pos = archive.rfind(utf8("_" + tos(numFileBackups)));
			if ((pos != utf8::npos) && (uniFile::fileSize(dest) > 0))
			{
				archive = (dest.substr(0, pos) + utf8("_" + make_archive_log() + "_w3c.gz"));

				HANDLE m_archive = ::CreateFileW(archive.toWString().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (m_archive != INVALID_HANDLE_VALUE)
				{
					DWORD written(0);
					utf8 out;
					z_stream m_stream = {0};

					FILE* m_logFile = uniFile::fopen(dest, "rb");
					if (m_logFile != NULL)
					{
						bool started = false;
						while (!feof(m_logFile))
						{
							std::vector<uniString::utf8::value_type> m_logFileBuffer;
							const size_t BUFSIZE(1024*16);
							m_logFileBuffer.clear();
							m_logFileBuffer.resize(BUFSIZE + 1);
							size_t amt = fread(&(m_logFileBuffer[0]), 1, BUFSIZE, m_logFile);
							if (amt > 0)
							{
								out = utf8(&(m_logFileBuffer[0]), amt);
								if (started == false)
								{
									compressDataStart(out, &m_stream, (Bytef*)"sc_w3c.log\0");
									started = true;
								}
								else
								{
									compressDataCont(out, &m_stream);
								}
								::WriteFile(m_archive, out.c_str(), (DWORD)out.size(), &written, NULL);
							}
						}

						if (out.size() > 0)
						{
							compressDataFinish(out, &m_stream);
						}
						::WriteFile(m_archive, out.c_str(), (DWORD)out.size(), &written, NULL);

						compressDataEnd(&m_stream);

						::fclose(m_logFile);
						forgetHandleInvalid(m_archive);

						// no need to keep any 0-byte files
						// this is just incase of weirdness
						if (!uniFile::fileSize(archive))
						{
							uniFile::unlink(archive);
						}
					}
					else
					{
						forgetHandleInvalid(m_archive);
						uniFile::unlink(archive);
					}
				}
			}
			#else
			uniFile::filenameType archive = dest;
			utf8::size_type pos = archive.rfind(utf8("_" + tos(numFileBackups)));
			if ((pos != utf8::npos) && (uniFile::fileSize(dest) > 0))
			{
				archive = (dest.substr(0, pos) + utf8("_" + make_archive_log() + "_w3c.gz"));
				int m_archive = ::open(archive.hideAsString().c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
				if (m_archive != -1)
				{
					utf8 out;
					z_stream m_stream = {0};
					FILE* m_logFile = uniFile::fopen(dest, "rb");
					if (m_logFile != NULL)
					{
						bool started = false;
						while (!feof(m_logFile))
						{
							std::vector<uniString::utf8::value_type> m_logFileBuffer;
							const size_t BUFSIZE(1024*16);
							m_logFileBuffer.clear();
							m_logFileBuffer.resize(BUFSIZE + 1);
							size_t amt = fread(&(m_logFileBuffer[0]), 1, BUFSIZE, m_logFile);
							if (amt > 0)
							{
								out = utf8(&(m_logFileBuffer[0]), amt);
								if (started == false)
								{
									compressDataStart(out, &m_stream, (Bytef*)"sc_w3c.log\0");
									started = true;
								}
								else
								{
									compressDataCont(out, &m_stream);
								}
								::write(m_archive, out.c_str(), out.size());
							}
						}

						if (out.size() > 0)
						{
							compressDataFinish(out, &m_stream);
						}

						::write(m_archive, out.c_str(), out.size());

						compressDataEnd(&m_stream);

						::fclose(m_logFile);
						::close(m_archive);

						// no need to keep any 0-byte files
						// this is just incase of weirdness
						if (!uniFile::fileSize(archive))
						{
							uniFile::unlink(archive);
						}
					}
					else
					{
						::close(m_archive);
						uniFile::unlink(archive);
					}
				}
			}
			#endif
		}

		uniFile::unlink(dest);
		#ifdef _WIN32
		::MoveFileW(((x - 1) ? make_backup_log(fn, (x - 1)).toWString().c_str() : fn.toWString().c_str()), dest.toWString().c_str());
		#else
		::rename(((x - 1) ? make_backup_log(fn, (x - 1)).hideAsString().c_str() : fn.hideAsString().c_str()), dest.hideAsString().c_str());
		#endif
	}

	s_w3cFileHandle[streamID] = uniFile::fopen(fn,"wb");
	if (!s_w3cFileHandle[streamID])
	{
		ELOG("[W3C] Could not open file " + fn);
	}
	else
	{
		header(s_w3cFileHandle[streamID]);
	}
}

void w3cLog::log(const size_t streamID, const uniString::utf8 &ipAddr,
				 const uniString::utf8 &hostName, const uniString::utf8 &songName,
				 const uniString::utf8 &userAgent, __uint64 bytesSent,
				 time_t timeInSeconds, int bitrate)
{
	// to prevent some of the stats / logs getting skewed
	// then we filter out the SHOUTcast site 'test' users
	if (isUserAgentOfficial(toLower(userAgent)))
	{
		return;
	}

	stackLock sml(s_w3cLock);

	FILE *w3cFileHandle = s_w3cFileHandle[streamID];
	if (!w3cFileHandle && streamID)
	{
		w3cFileHandle = s_w3cFileHandle[0];
	}

	if (!w3cFileHandle)
	{
		return;
	}

	static time_t last_t;
	static utf8 w3c_buf;
	time_t t = ::time(NULL);
	if (last_t != t)
	{
		struct tm lt = {0};
		::localtime_s(&lt, &t);

		char buf[1024] = {0};
		snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
				 (lt.tm_year % 100) + 2000, lt.tm_mon + 1,
				 lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec);
		w3c_buf = buf;
	}

	// http://forums.shoutcast.com/showthread.php?t=355063
	// http://forums.shoutcast.com/showthread.php?t=358805
	utf8 s = ipAddr + " " + hostName + " " + w3c_buf + " /stream?title=" +
			 (songName.empty() ? utf8("Unknown") : urlUtils::escapeURI_RFC3986(songName)) +
			 " 200 " + urlUtils::escapeURI_RFC3986(userAgent) + " " + tos(bytesSent) + " " +
			 tos(timeInSeconds) + " " + tos(bitrate) + eol();

	size_t amt = ::fwrite(s.c_str(), 1, s.size(), w3cFileHandle);
	if (amt != s.size())
	{
		ELOG("[W3C] Write error");
	}
	::fflush(w3cFileHandle);
}
