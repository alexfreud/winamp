#include "logger.h"
#include <stdio.h>
#include <algorithm>
#include "stl/functors.h"
#include "file/fileUtils.h"
#include "stl/stringUtils.h"
#include "macros.h"
#include "../../global.h"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
#define __F__ __FUNCTION__
#else
#define __F__ string(__PRETTY_FUNCTION__) + 
#endif

using namespace std;
using namespace uniString;
using namespace stringUtil;

////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// LOGGER ELEMENTS ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
//////////// Win32 File Logger

inline uniFile::filenameType AOL_logger::fileLogger_element::make_backup_log(const uniFile::filenameType &filename,int which) throw()
{
	return fileUtil::stripSuffix(filename) + "_" +
			tobs<uniFile::filenameType>(which) + "."
			+ fileUtil::getSuffix(filename);
}

uniFile::filenameType AOL_logger::fileLogger_element::make_archive_log() throw()
{
	SYSTEMTIME sysTime = {0};
	::GetLocalTime(&sysTime);
	wchar_t d[100] = {0}, t[100] = {0};
	::GetDateFormatW(LOCALE_SYSTEM_DEFAULT,0,&sysTime,_T("yyyy'_'MM'_'dd"),d,99);
	::GetTimeFormatW(LOCALE_SYSTEM_DEFAULT,0,&sysTime,_T("HH'_'mm'_'ss"),t,99);
	return tos((const wchar_t *)d) + "_" + tos((const wchar_t *)t);
}

void AOL_logger::fileLogger_element::rotate() throw()
{
	m_lastRolloverTime = ::time(NULL);

	if (m_file == INVALID_HANDLE_VALUE || m_first == false)
	{
		return;
	}

	// close the log
	forgetHandleInvalid(m_file);

	// rotate
	for (int x = m_numFileBackups; x > 0; --x)
	{
		uniFile::filenameType dest = make_backup_log(m_fileName,x);

		// archive the log file about to be removed into a gz file
		if (m_numFileBackups > 0 && x == m_numFileBackups && m_archiveFileBackups)
		{
			uniFile::filenameType archive = dest;
			utf8::size_type pos = archive.rfind(utf8("_"+tos(m_numFileBackups)));
			if ((pos != utf8::npos) && (uniFile::fileSize(dest) > 0))
			{
				archive = (dest.substr(0,pos) + utf8("_log_" + make_archive_log() + ".gz"));

				HANDLE m_archive = ::CreateFileW(archive.toWString().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (m_archive != INVALID_HANDLE_VALUE)
				{
					DWORD written(0);
					utf8 out;
					z_stream m_stream = {0};

					if (uniFile::fileSize(dest) > 0)
					{
						FILE* m_logFile = uniFile::fopen(dest,"rb");
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
										compressDataStart(out, &m_stream);
										started = true;
									}
									else
									{
										compressDataCont(out, &m_stream);
									}
									::WriteFile(m_archive, out.c_str(), (DWORD)out.size(), &written, NULL);
								}
							}

							compressDataFinish(out, &m_stream);
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
			}
		}
		::DeleteFileW(dest.toWString().c_str());
		::MoveFileW(
			((x-1) ? make_backup_log(m_fileName,(x-1)).toWString().c_str()
				: m_fileName.toWString().c_str()),
					dest.toWString().c_str());
	}

	// open new log
	m_file = ::CreateFileW(m_fileName.toWString().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

AOL_logger::fileLogger_element::fileLogger_element(const uniFile::filenameType &filename, const uniFile::filenameType &defaultFilename,
        bool &useDefaultPath, int backups, bool archive, int rolloverInterval, size_t SID) throw(exception)

    : m_fileName(filename), m_file(INVALID_HANDLE_VALUE), m_lastRolloverTime(0), m_rolloverInterval(rolloverInterval), m_numFileBackups(backups),
      m_archiveFileBackups(archive), m_first(false), m_SID(SID)
{
	// this will fill in the default log path as required
	wchar_t s_defaultFileName[MAX_PATH] = {0};
	ExpandEnvironmentStringsW(defaultFilename.toWString().c_str(), s_defaultFileName, MAX_PATH);
	utf8 m_defaultFilename(utf32(s_defaultFileName).toUtf8());

	useDefaultPath = false;
	m_file = ::CreateFileW(m_fileName.toWString().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_file == INVALID_HANDLE_VALUE)
	{
		m_file = ::CreateFileW(m_defaultFilename.toWString().c_str(),GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_file == INVALID_HANDLE_VALUE)
		{
			uniFile::filenameType fallbackFilename("%temp%\\sc_serv_" + tos(getpid()) + ".log");
			wchar_t s_fallbackFileName[MAX_PATH] = {0};
			ExpandEnvironmentStringsW(fallbackFilename.toWString().c_str(), s_fallbackFileName, MAX_PATH);
			utf8 m_fallbackFilename(utf32(s_fallbackFileName).toUtf8());
			m_file = ::CreateFileW(m_fallbackFilename.toWString().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_file == INVALID_HANDLE_VALUE)
			{
				throw runtime_error("Logger could not open the log file \"" + m_fileName.hideAsString() + "\" for writing [" + errMessage().hideAsString() + "]. Check the directory exists and another instance is not already running.");
			}
			else
			{
				m_fileName = m_fallbackFilename;
			}
		}
		else
		{
			m_fileName = m_defaultFilename;
		}
	}

	::SetFilePointer(m_file,0,0,FILE_END); // seek to end
	if (m_fileName == m_defaultFilename)
	{
		useDefaultPath = true;
	}

	gOptions.setOption(utf8("reallogfile"), m_fileName);

	// using this to prevent rotating empty log files on startup
	m_first = fileUtil::fileExists(m_fileName.hideAsString());
	if (m_first && !uniFile::fileSize(m_fileName))
	{
		m_first = false;
	}

	rotate();
}

AOL_logger::fileLogger_element::~fileLogger_element() throw()
{
	forgetHandleInvalid(m_file);
}

void AOL_logger::fileLogger_element::log(message &m) throw(exception)
{
	time_t t = ::time(NULL);
	if (t < m_lastRolloverTime || (((t - m_lastRolloverTime) > m_rolloverInterval) && m_rolloverInterval))
	{
		rotate();
		rotatew3cFiles("w3c");
	}
    if (m.m_alreadyLogged)
        return;
    if (m_SID != m.m_streamID && m_SID)
    {
        return;
    }
    m.m_alreadyLogged = true;
	m_first = true;

	utf8 ss = m.getTimestamp() + "\t" + m.typeAsStr();
    const utf8 &msg = m.getMsg();

    if (msg[0] != '[')
    {
        const char *section = m.fromSection();
        if (section)
        {
            bool wrap = (section[0] == '[') ? false : true;
            size_t ID = m.getID();
            if (wrap)
            {
                ss += "\t[";
                ss += section;
                if (ID > 0)
                {
                    ss += " sid=";
                    ss += tos(ID);
                }
                ss += "] ";
            }
            else
            {
                ss += "\t";
                ss += section;
                // if (ID > 0)
                // ss += " ID present ";
            }
        }
        else
            ss += "\t";
    }
    else
        ss += "\t";

	const map<utf8,utf8> *fields = m.getFields();
    if (fields)
    {
	    for (map<utf8,utf8>::const_iterator i = fields->begin(); i != fields->end(); ++i)
   	    {
		    if (!(*i).first.empty())
		    {
			    if (!(*i).second.empty())
			    {
				    ss += "\t" + (*i).second;
			    }
			    else
			    {
				    return;
			    }
		    }
	    }
    }
    ss += msg;
	ss += eol();

	if (m_file == INVALID_HANDLE_VALUE)
	{
		throw runtime_error(__F__ " Error writing to log file " + m_fileName.hideAsString());
	}

	DWORD written(0);
	if ((!::WriteFile(m_file, ss.c_str(), (DWORD)ss.size(), &written, NULL)) || (written != ss.size()))
	{
		throw runtime_error(__F__ " Error writing to log file " + m_fileName.hideAsString());
	}
}

#else

/////////// Unix File Logger

uniFile::filenameType AOL_logger::fileLogger_element::make_backup_log(const uniFile::filenameType &filename,int which) throw()
{
	return fileUtil::stripSuffix(filename) + "_" + tos(which) + "." + fileUtil::getSuffix(filename);
}

uniFile::filenameType AOL_logger::fileLogger_element::make_archive_log() throw()
{
	char buf[256] = {0};
	struct tm ttm;
	time_t ttt;
	::time(&ttt);
	::strftime(buf, 255, "%Y_%m_%d_%H_%M_%S", ::localtime_r(&ttt, &ttm));
	return buf;
}

void AOL_logger::fileLogger_element::rotate() throw()
{
	m_lastRolloverTime = ::time(NULL);

	if ((m_file == -1) || (m_first == false))
	{
		return;
	}

	// close the log
	::close(m_file);
	m_file = -1;

	// rotate
	for (int x = m_numFileBackups; x > 0; --x)
	{
		uniFile::filenameType dest = make_backup_log(m_fileName,x);

		// archive the log file about to be removed into a gz file
		if (m_numFileBackups > 0 && x == m_numFileBackups && m_archiveFileBackups)
		{
			uniFile::filenameType archive = dest;
			utf8::size_type pos = archive.rfind(utf8("_"+tos(m_numFileBackups)));
			if ((pos != utf8::npos) && (uniFile::fileSize(dest) > 0))
			{
				archive = (dest.substr(0,pos) + utf8("_log_" + make_archive_log() + ".gz"));

				int m_archive = ::open(archive.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
				if (m_archive != -1)
				{
					utf8 out;
					z_stream m_stream = {0};

					if (uniFile::fileSize(dest) > 0)
					{
						FILE* m_logFile = uniFile::fopen(dest,"rb");
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
										compressDataStart(out, &m_stream);
										started = true;
									}
									else
									{
										compressDataCont(out, &m_stream);
									}
									::write(m_archive,out.c_str(),out.size());
								}
							}

							compressDataFinish(out, &m_stream);
							::write(m_archive,out.c_str(),out.size());

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
			}
		}

		::remove(dest.hideAsString().c_str());
		::rename(
			((x-1) ? make_backup_log(m_fileName,(x-1)).hideAsString().c_str()
				: m_fileName.hideAsString().c_str()),
					dest.hideAsString().c_str());
	}

	// open new log
	m_file = ::open(m_fileName.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
}

AOL_logger::fileLogger_element::fileLogger_element(const uniFile::filenameType &filename,
												   const uniFile::filenameType &defaultFilename,
												   bool &useDefaultPath, int backups,
												   bool archive, int rolloverInterval, size_t SID) throw(exception):
		m_fileName(filename), m_file(-1), m_lastRolloverTime(0),
		m_rolloverInterval(rolloverInterval), m_numFileBackups(backups),
		m_archiveFileBackups(archive), m_first(false), m_SID(SID)
{
	umask(0);
	useDefaultPath = false;
	m_file = ::open(filename.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (m_file == -1)
	{
        if (SID == 0)
            m_file = ::open (defaultFilename.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (m_file == -1)
		{
			uniFile::filenameType fallbackFilename("/tmp/sc_serv_" + tos(getpid()) + ".log");
			m_file = ::open(fallbackFilename.hideAsString().c_str(),O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			if (m_file == -1)
			{
				throw runtime_error("Logger could not open the log file \"" + m_fileName.hideAsString() + "\" for writing [" + errMessage().hideAsString() + "]. Check the directory exists and another instance is not already running.");
			}
			else
			{
				m_fileName = fallbackFilename;
			}
		}
		else
		{
			m_fileName = defaultFilename;
		}
	}

	if (m_fileName == defaultFilename)
	{
		useDefaultPath = true;
	}

	gOptions.setOption(utf8("reallogfile"), m_fileName);

	// using this to prevent rotating empty log files on startup
	m_first = fileUtil::fileExists(m_fileName.hideAsString());
	if (m_first && !uniFile::fileSize(m_fileName))
	{
		m_first = false;
	}

	rotate();
}

AOL_logger::fileLogger_element::~fileLogger_element() throw()
{
	if (m_file != -1)
		::close(m_file);
	m_file = -1;
}

void AOL_logger::fileLogger_element::log(message &m) throw(exception)
{
	time_t t = ::time(NULL);
	if (t < m_lastRolloverTime || (((t - m_lastRolloverTime) > m_rolloverInterval) && m_rolloverInterval))
	{
		rotate();
		rotatew3cFiles("w3c");
	}
    if (m.m_alreadyLogged)
        return;
    if (m_SID != m.m_streamID && m_SID)
    {
        return;
    }
    m.m_alreadyLogged = true;
	m_first = true;

	utf8 ss = m.getTimestamp() + "\t" + m.typeAsStr();
    const utf8 &msg = m.getMsg();

    if (msg[0] != '[')
    {
        const char *section = m.fromSection();
        if (section)
        {
            bool wrap = (section[0] == '[') ? false : true;
            size_t ID = m.getID();
            if (wrap)
            {
                ss += "\t[";
                ss += section;
                if (ID > 0)
                {
                    ss += " sid=";
                    ss += tos(ID);
                }
                ss += "] ";
            }
            else
            {
                ss += "\t";
                ss += section;
                // if (ID > 0)
                    // ss += " ID present ";
            }
        }
        else
            ss += "\t";
    }
    else
        ss += "\t";

    const map<utf8,utf8> *fields = m.getFields();
    if (fields)
    {
        for (map<utf8,utf8>::const_iterator i = fields->begin(); i != fields->end(); ++i)
        {
            if (!(*i).first.empty())
            {
                if (!(*i).second.empty())
                {
                    ss += "\t" + (*i).second;
                }
                else
                {
                    return;
                }
            }
        }
        ss += " ";
    }
    ss += msg;
    ss += eol();

	if (m_file == -1)
	{
		throw runtime_error(__F__ " Error writing to log file " + m_fileName.hideAsString());
	}

	if (::write(m_file,ss.c_str(),ss.size()) == -1)
	{
		throw runtime_error(__F__ " Error writing to log file " + m_fileName.hideAsString());
	}
}

#endif

#ifdef _WIN32
//////// Win32 console logger
	
AOL_logger::consoleLogger_element::consoleLogger_element() throw(exception) : m_stdoutConsole(NULL), m_stderrConsole(NULL)
{
	::AllocConsole();
	::SetConsoleOutputCP(65001); // utf-8
	::SetConsoleCP(CP_UTF8);//65001);

	m_stdoutConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
	m_stderrConsole = ::GetStdHandle(STD_ERROR_HANDLE);

	if ((!m_stdoutConsole) || (!m_stderrConsole))
	{
		throw runtime_error("Logger could not open console");
	}
}

AOL_logger::consoleLogger_element::~consoleLogger_element() throw()
{
	if (m_stdoutConsole != NULL)
	{
		::FreeConsole();
		m_stdoutConsole = NULL; 
		m_stderrConsole = NULL;
	}
}

void AOL_logger::consoleLogger_element::log(message &m) throw(exception)
{
	static const DWORD maxLogLine=2048;

	utf8 ss = m.getTimestamp() + "\t" + m.typeAsStr();

    const utf8 &msg = m.getMsg();
    if (msg[0] != '[')
    {
        const char *section = m.fromSection();
        if (section)
        {
            bool wrap = (section[0] == '[') ? false : true;
            size_t ID = m.getID();
            if (wrap)
            {
                ss += "\t[";
                ss += section;
                if (ID > 0)
                {
                    ss += " sid=";
                    ss += tos(ID);
                }
                ss += "] ";
            }
            else
            {
                ss += "\t";
                ss += section;
                // if (ID > 0)
                    // ss += " ID present ";
            }
        }
        else
            ss += "\t";
    }
    else
        ss += "\t";

    const map<utf8,utf8> *fields = m.getFields();
    if (fields)
    {
        for (map<utf8,utf8>::const_iterator i = fields->begin(); i != fields->end(); ++i)
        {
            if (!(*i).first.empty())
            {
                if (!(*i).second.empty())
                {
                    ss += "\t" + (*i).second;
                }
                else
                {
                    return;
                }
            }
        }
       }

    ss += m.getMsg();
	ss += eol();

	HANDLE h = (m.getType() == AOL_logger::message::BM_ERROR ? m_stderrConsole : m_stdoutConsole);
	if (h != INVALID_HANDLE_VALUE)
	{
		// see if we need to colour the output - only used for errors and warnings
		AOL_logger::message::message_t type = m.getType();
		bool painted = false;
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo = {0};
		WORD wOldColorAttrs = 0;
		if (type != AOL_logger::message::BM_INFO && GetConsoleScreenBufferInfo(h, &csbiInfo))
		{
			wOldColorAttrs = csbiInfo.wAttributes;

			// red for error, yellow for warnings, green for debug, blue for update
			if (type == AOL_logger::message::BM_ERROR)
			{
				painted = true;
				SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
			}
			else if (type == AOL_logger::message::BM_WARNING)
			{
				painted = true;
				SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			}
			else if (type == AOL_logger::message::BM_DEBUG)
			{
				painted = true;
				SetConsoleTextAttribute(h, FOREGROUND_GREEN);
			}
			else if (type == AOL_logger::message::BM_UPDATE)
			{
				painted = true;
				SetConsoleTextAttribute(h, FOREGROUND_BLUE);
			}
		}

		// utf8 sometimes returns  a size that does not match input
		DWORD written(0);
		DWORD amtToWrite = (DWORD)ss.size();
		bool truncated = false;
		if (amtToWrite > maxLogLine) 
		{
			amtToWrite = maxLogLine;
			truncated = true;
		}

		if (truncated)
		{
			utf8 truncateMsg = m.getTimestamp() + "\t" + ((char)message::BM_WARNING) + "\t[MAIN] Next line is truncated" + eol();
			::WriteFile(h,truncateMsg.c_str(),(DWORD)truncateMsg.size(),&written,NULL);
			written = 0;
		}

		if (::WriteFile(h,ss.c_str(),amtToWrite,&written,NULL))
		{
			if (truncated)
			{
				// eol
				written = 0;
				::WriteFile(h,eol().c_str(),(DWORD)eol().size(),&written,NULL);
			}
		}

		// revert the colouring if we needed to change it
		if (painted)
		{
			SetConsoleTextAttribute(h, wOldColorAttrs);
		}
	}
}

#else
/////// Unix console logger

AOL_logger::consoleLogger_element::consoleLogger_element() throw(exception) :
	m_stdoutConsole(STDOUT_FILENO), m_stderrConsole(STDERR_FILENO)
{
	if ((m_stdoutConsole == -1) || (m_stderrConsole == -1))
	{
		throw runtime_error("Logger could not open console");
	}
}

AOL_logger::consoleLogger_element::~consoleLogger_element() throw()
{
	m_stdoutConsole = m_stderrConsole = -1;
}

void AOL_logger::consoleLogger_element::log(message &m) throw(exception)
{
	utf8 sc = "";
	// see if we need to colour the output - only used for errors, warnings and debugs
	AOL_logger::message::message_t type = m.getType();
    if (type != AOL_logger::message::BM_INFO)
	{
		// red for error, yellow for warnings, green for debug, blue for update
		if (type == AOL_logger::message::BM_ERROR)
		{
			sc = "\033[01;31m";
		}
		else if (type == AOL_logger::message::BM_WARNING)
		{
			sc = "\033[01;33m";
		}
		else if (type == AOL_logger::message::BM_DEBUG)
		{
			sc = "\033[0;32m";
		}
		else if (type == AOL_logger::message::BM_UPDATE)
		{
			sc = "\033[0;34m";
		}
    }

    utf8 ss = sc + m.getTimestamp() + "\t" + m.typeAsStr();
    const utf8 &msg = m.getMsg();
    if (msg[0] != '[')
    {
        const char *section = m.fromSection();
        if (section)
        {
            bool wrap = (section[0] == '[') ? false : true;
            size_t ID = m.getID();
            if (wrap)
            {
                ss += "\t[";
                ss += section;
                if (ID > 0)
                {
                    ss += " sid=";
                    ss += tos(ID);
                }
                ss += "] ";
            }
            else
            {
                ss += "\t";
                ss += section;
                // if (ID > 0)
                // ss += " ID present ";
            }
        }
        else
            ss += "\t";
    }
    else
        ss += "\t";
    const map<utf8,utf8> *fields = m.getFields();
    if (fields)
    {
        for (map<utf8,utf8>::const_iterator i = fields->begin(); i != fields->end(); ++i)
        {
            if (!(*i).first.empty())
            {
                if (!(*i).second.empty())
                {
                    ss += "\t" + (*i).second;
                }
                else
                {
                    return;
                }
            }
        }
    }

    ss += m.getMsg();
	ss += eol() + (!sc.empty() ? "\033[0m" : "");

	int console = (m.getType() == AOL_logger::message::BM_ERROR ? m_stderrConsole : m_stdoutConsole);

	if (console != -1)
	{
		::write(console,ss.c_str(),ss.size());
	}
}
#endif

#ifdef _WIN32

// Win32 system logger
void AOL_logger::systemLogger_element::registerEventLog(const utf8 &log_object_name,
														const uniFile::filenameType &fullExePath) throw()
{
	HKEY key = NULL;
	DWORD disposition = 0;

	wstring regEntry = L"System\\CurrentControlSet\\Services\\EventLog\\Application\\";
	regEntry += log_object_name.toWString();

	LONG err = ::RegCreateKeyExW(HKEY_LOCAL_MACHINE, regEntry.c_str(), 0, NULL,
								 REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &key, &disposition);

	if (err == ERROR_SUCCESS)
	{
		DWORD ts = 1;
		wstring widePath = fullExePath.toWString();
		::RegSetValueExW(key,L"EventMessageFile",0,REG_SZ,(const BYTE *)widePath.c_str(),(DWORD)((fullExePath.size() + 1) * 2));
		::RegSetValueExW(key,L"TypesSupported",0,REG_DWORD,(const BYTE *)&ts,sizeof(ts));
		::RegCloseKey(key);
	}
}

AOL_logger::systemLogger_element::systemLogger_element(const utf8 &log_object_name,
													   const uniFile::filenameType &fullExePath,
													   const utf8 &loggerConfigString) throw(exception)
		: m_systemLog(NULL), m_loggerConfigString(loggerConfigString)
{
	registerEventLog(log_object_name,fullExePath);
    m_systemLog = ::RegisterEventSourceW(NULL,  // uses local computer 
										 log_object_name.toWString().c_str()); // source name 
    if (m_systemLog == NULL)
	{
        throw runtime_error("Could not register the event source for the system logs."); 
	}
}

AOL_logger::systemLogger_element::~systemLogger_element() throw()
{
	if (m_systemLog != NULL)
	{
		::DeregisterEventSource(m_systemLog);
		m_systemLog = NULL;
	}
}

static WORD charToLogType(char c) throw()
{
	if (c == 'E')
	{
		return EVENTLOG_ERROR_TYPE;
	}
	if (c == 'W')
	{
		return EVENTLOG_WARNING_TYPE;
	}
	if (c == 'I')
	{
		return EVENTLOG_INFORMATION_TYPE;
	}
	return 0;
}

static WORD messageTypeToLogType(AOL_logger::message::message_t t, const utf8 &configString) throw()
{
	if (configString.size() < 5)
	{
		return EVENTLOG_ERROR_TYPE;
	}

	switch (t)
	{
		case AOL_logger::message::BM_INFO:
		{
			return charToLogType(configString[3]);
		}
		case AOL_logger::message::BM_WARNING:
		{
			return charToLogType(configString[2]);
		}
		case AOL_logger::message::BM_ERROR:
		{
			return charToLogType(configString[0]);
		}
		case AOL_logger::message::BM_DEBUG:
		{
			return charToLogType(configString[4]);
		}
	}
	return 0;
}

void AOL_logger::systemLogger_element::log(message &m) throw(exception)
{
	if (m_systemLog == NULL)
	{
		throw runtime_error(__F__ " Error writing to system log");
	}

	const map<utf8,utf8> *fields = m.getFields();

	utf8 ss;
    if (fields)
    {
	    for (map<utf8,utf8>::const_iterator i = fields->begin(); i != fields->end(); ++i)
	    {
		    if (!(*i).first.empty())
		    {
			    if (!(*i).second.empty())
			    {
				    ss += (!ss.empty() ? "\t" : "") + (*i).second;
			    }
			    else
			    {
			        return;
                }
			}
		}
	}
    ss += m.getMsg();
	if (!ss.empty())
	{
		utf32 u32(stripWhitespace(ss));
		utf16 u16(u32.toUtf16(true));

		const wchar_t *s = (const wchar_t *)u16.c_str();
		WORD et = messageTypeToLogType(m.getType(), m_loggerConfigString);
		if (et)
		{
			::ReportEventW(m_systemLog,				// event log handle
						   et,						// event type
						   0,						// category 0
						   ((DWORD)0xC0000001L),	// event identifier
						   NULL,					// no user security identifier
						   1,						// one substitution string
						   0,						// no data
						   &s,						// pointer to string array
						   NULL);
		}
	}
}

#endif
