#pragma once
#ifndef logger_H_
#define logger_H_

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <stdexcept>
#include "unicode/uniFile.h"
#include "stl/stringUtils.h"
#include "stl/functors.h"
#include "threading/messageThread.h"
#include "unicode/uniFile.h"

namespace AOL_logger 
{
	/*
	A function object, used in conjunction with the messageThread template to create a
	thread safe logging entity which runs on it's own thread. 

	The logger is meant to be run in the context of a messageThread object like so:

		extern messageThread<logger>	*gLog;

	You post messages using the postMessage method from the messageThread class

		gLog->postMessage(whatever);

	The E (logger element) type has the following requirements

		1) Must be a heap element that can be deleted.
		2) An install() method which is called when the element becomes part of
			the logging system. Install may throw an exception
		3) An uninstall() mehod which is called when before the log element is
			destroyed by the logger system. It MUST NOT throw an exception.

	The M (message) type has the following requirements

		1) Must support a static makeError(const uniString::utf8 &s) throw() for
			constructing an error message
		2) Must have 
				bool done() const throw()
			which returns true to indicate it's a message to shutdown the logger
		3) Any other signatures required by the logger element
	*/

	class message;
	template<class M = AOL_logger::message> class logger_element;

	template<typename M = AOL_logger::message,typename E = AOL_logger::logger_element<M> >
	class logger
	{
		// NOTE: logger takes ownership of the elements and deletes them
		// when done.
	private:
		std::vector<E*> m_elements;

		void uninstallElements() throw()
		{
			std::for_each(m_elements.begin(), m_elements.end(), std::mem_fun(&E::uninstall));
		}

	public:		
		typedef M message_t;

		// constructors. If the constructor succeeds (does not throw), then this
		// logger object has posession of the elements, and will delete them
		// itself.
		logger() throw() {}

		// create the logger with a single element. An exception
		// means that the element was not added. The logger has not taken
		// posession of the element and it's up to the caller to delete it.
		explicit logger(E *e) throw(std::exception) 
		{ 
			addElement(e); 
		}

		// create the logger from a container of elements. If the install() method
		// of any element throws, then this constructor will throw. All methods that
		// were installed() will be uninstalled() but NO elements will be deleted. Thatt
		// is up to the caller.
		template <typename ITER>
		logger(ITER first,ITER last) throw (std::exception) 
		{ 
			try
			{
				addElement(first, last); 
			}
			catch(...)
			{
				uninstallElements();
				m_elements.clear();
				throw;
			}
		}

		/////////////////////////

		// destructor
		~logger() throw()
		{
			uninstallElements();
			std::for_each(m_elements.begin(), m_elements.end(), stlx::delete_fntr<E>);
		}

		// warning... there is no lock protection. Do not add a logger element
		// while the thread is running
		void addElement(E *e) throw(std::exception)
		{
			// note: element is not added to internal list if install() throws
			e->install();
			m_elements.push_back(e);
		}

		// if any element throws, then all the ones passed in will be uninstalled if
		// they were installed, and no objects in the list will be taken posession of
		template <typename ITER>
		void addElement(ITER first, ITER last) throw(std::exception)
		{
			std::vector<E*> tmp;
			try
			{
				for (ITER i = first; i != last; ++i) 
				{
					(*i)->install(); 
					tmp.push_back(*i);
				}
			}
			catch(...)
			{
				for (typename std::vector<E*>::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
				{
					(*i)->uninstall();
				}
				throw;
			}
			m_elements.insert(m_elements.end(), tmp.begin(), tmp.end());
		}

		//////////////////////////////////////////////////////

		// main dispatch loop
		bool operator()(M &m) throw()
		{
			if (m.done()) // if this is the done message, exit the loop
			{
				return false;
			}

			if (m.rotate())
			{
				for (typename std::vector<E*>::const_iterator i = m_elements.begin(); i != m_elements.end(); ++i)
				{
					(*i)->rotate();
				}
				return true;
			}

			for (typename std::vector<E*>::const_iterator i = m_elements.begin(); i != m_elements.end(); ++i)
			{
				try 
				{
					(*i)->log(m);
				}
				catch (const std::exception &ex)
				{
					// on an exception, create an error message and dispatch it to everyone
					// via their 'NOTHROW' handler
					M mex = M::makeError(ex.what());
					for (typename std::vector<E*>::const_iterator ix = m_elements.begin(); ix != m_elements.end(); ++ix)
					{
						(*ix)->logNOTHROW(mex);
					}
				}
			}
			return true;
		}
	};

///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////// Here is a basic set of elements you can use ///////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

    class fileLogger_element;

	// the message class
	class message
	{
	public:
		typedef enum
		{
			BM_DONE,
			BM_ROTATE,
			BM_ERROR = 'E',
			BM_WARNING = 'W',
			BM_INFO = 'I',
			BM_DEBUG = 'D',
			BM_UPDATE = 'U'
		} message_t;

        size_t m_streamID;
	private:
		//uniString::utf8 m_typeStr;
        friend class AOL_logger::fileLogger_element;

		uniString::utf8 m_timestamp;
		std::map<uniString::utf8,uniString::utf8> *m_fields;
        uniString::utf8 m_msg;
        const char *m_section;
		message_t m_type;
        bool m_alreadyLogged;

		static uniString::utf8 timeStamp() throw()
		{
#ifdef _WIN32
			SYSTEMTIME lastTime = {0};
			wchar_t d[100] = {0}, t[100] = {0};
			uniString::utf8 lastMsg;

			SYSTEMTIME sysTime = {0};
			::GetLocalTime(&sysTime);

			::GetDateFormatW(LOCALE_SYSTEM_DEFAULT, 0, &sysTime, _T("yyyy'-'MM'-'dd"), d, 99);
			::GetTimeFormatW(LOCALE_SYSTEM_DEFAULT, 0, &sysTime, _T("HH':'mm':'ss"), t, 99);
			lastMsg = stringUtil::tos((const wchar_t *)d) + " " + stringUtil::tos((const wchar_t *)t);
			return lastMsg;
#else
			char buf[32] = {0};

			struct tm ttm;
			time_t ttt = ::time(NULL);
			::strftime(buf, sizeof (buf), "%Y-%m-%d %H:%M:%S", ::localtime_r(&ttt, &ttm));
			return buf;
#endif
		}

        message (const message_t m, const uniString::utf8 &msg, const char *section = NULL, size_t id = 0)
            : m_streamID(id), m_timestamp(timeStamp()), m_fields(NULL), m_msg(msg), m_section(section), m_type(m), m_alreadyLogged(false)  { }

        message(const message_t m, const char *msg, const char *section = NULL, size_t id = 0)
            : m_streamID(id), m_timestamp(timeStamp()), m_fields(NULL), m_msg(msg), m_section(section), m_type(m), m_alreadyLogged(false)  { }

        message(const message_t m, const char *section = NULL, size_t id = 0, const std::map<uniString::utf8,uniString::utf8> *fields = NULL)
            : m_streamID(id), m_timestamp(timeStamp()), m_section(section), m_type(m), m_alreadyLogged(false)
        {
            if (fields)
                m_fields = new std::map<uniString::utf8,uniString::utf8> (*fields);
        }

	public:
		inline bool done() const throw() { return (m_type == BM_DONE); }
		inline bool rotate() const throw() { return (m_type == BM_ROTATE); }

		inline message_t getType() const throw() { return m_type; }
		inline void  setType(message_t m) { m_type = m; }
		inline const uniString::utf8 &getTimestamp() const throw() { return m_timestamp; }
		inline const std::map<uniString::utf8,uniString::utf8> *getFields() const throw() { return m_fields; }
        inline const char *fromSection() const throw() { return m_section; }
        inline const uniString::utf8 &getMsg() const throw() { return m_msg; }
        inline size_t getID() const throw() { return m_streamID; }

        static message makeDone() throw() { return message(BM_DONE); }
        static message makeRotate() throw() { return message(BM_ROTATE); }
        static message makeUpdate (const std::map<uniString::utf8,uniString::utf8> *f) throw() { return message(BM_UPDATE,NULL,0,f); }
        static message makeDebug (const std::map<uniString::utf8,uniString::utf8> *f) throw() { return message(BM_DEBUG,NULL,0,f); }
        static message makeInfo (const std::map<uniString::utf8,uniString::utf8> *f) throw() { return message(BM_INFO,NULL,0,f); }

        static message makeUpdate (const uniString::utf8 &s) throw() { std::map<uniString::utf8,uniString::utf8> f; f["msg"] = s; return message(BM_UPDATE,NULL,0,&f); }

        static message makeDebug (const uniString::utf8 &s, const char *sct = NULL, size_t id = 0) throw() { return message(BM_DEBUG,s,sct,id); }
        static message makeDebug (const char *s, const char *sct = NULL, size_t id = 0) throw() { return message(BM_DEBUG,s,sct,id); }

        static message makeInfo (const uniString::utf8 &s, const char *sct = NULL, size_t id = 0) throw() { return message(BM_INFO,s,sct,id); }
        static message makeInfo (const char *s, const char *sct = NULL, size_t id = 0) throw() { return message(BM_INFO,s,sct,id); }

        static message makeWarning (const uniString::utf8 &s, const char *sct = NULL, size_t id = 0) throw() { return message(BM_WARNING,s,sct,id); }
        static message makeWarning (const char *s, const char *sct = NULL, size_t id = 0) throw() { return message(BM_WARNING,s,sct,id); }

        static message makeError (const uniString::utf8 &s, const char *sct = NULL, size_t id = 0) throw() { return message(BM_ERROR,s,sct,id); }
        static message makeError (const char *s, const char *sct = NULL, size_t id = 0) throw() { return message(BM_ERROR,s,sct,id); }

		const char *typeAsStr() const
		{
            const char *str;
			switch (m_type)
			{
				case BM_INFO: str = "INFO"; break;
				case BM_ERROR: str = "ERROR"; break;
				case BM_WARNING: str = "WARN"; break;
				case BM_DEBUG: str = "DEBUG"; break;
				case BM_UPDATE: str = "UPDATE"; break;
				default: str = ""; break;
			}
            return str;
		}
	};

	// a base virtual message class for use by the logger
	template<class M>
	class logger_element
	{
	protected:
		// the message class used by the logger

	private:
		virtual void install() throw(std::exception) = 0;
		virtual void log(M &m) throw(std::exception) = 0;
		virtual void logNOTHROW(M &m) throw() { try { log(m); } catch(...){} }
		virtual void uninstall() throw() = 0;
		virtual void rotate() throw() {}

	public:
		virtual ~logger_element() throw() {}
		friend class AOL_logger::logger<M,AOL_logger::logger_element<M> >;
	};

////////////////////////////////////////////////////////////////////////////////////
///////////////////////// WIN32 ELEMENTS ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
	// Win32 file logger
	class fileLogger_element: public logger_element<AOL_logger::message>
	{
	private:
		uniFile::filenameType	m_fileName;
		HANDLE		m_file;
		time_t		m_lastRolloverTime;
		const int	m_rolloverInterval;
		const int	m_numFileBackups;
		const bool	m_archiveFileBackups;
		bool		m_first;
        size_t      m_SID;

		void rotate() throw();

		virtual void install() throw(std::exception){}
		virtual void log(AOL_logger::message &m) throw(std::exception);
		virtual void uninstall() throw(){}

		static uniFile::filenameType make_backup_log(const uniFile::filenameType &filename, int which) throw();
		static uniFile::filenameType make_archive_log() throw();

	public:
		fileLogger_element(const uniFile::filenameType &filename,
						   const uniFile::filenameType &defaultFilename,
						   bool &useDefaultPath, int backups,
						   bool archive, int rolloverInterval, size_t SID = 0) throw(std::exception);
		~fileLogger_element() throw();
	};

	class consoleLogger_element: public logger_element<AOL_logger::message>
	{
	private:
		HANDLE m_stdoutConsole;
		HANDLE m_stderrConsole;

		virtual void install() throw(std::exception){}
		virtual void log(AOL_logger::message &m) throw(std::exception);
		virtual void uninstall() throw(){}

	public:
		consoleLogger_element() throw(std::exception);
		~consoleLogger_element() throw();
	};

	class systemLogger_element: public logger_element<AOL_logger::message>
	{
	private:
		HANDLE m_systemLog;
		const uniString::utf8 m_loggerConfigString;

		void registerEventLog(const uniString::utf8 &log_object_name,
							  const uniFile::filenameType &fullExePath) throw();

		virtual void install() throw(std::exception){}
		virtual void uninstall() throw() {}
		virtual void log(AOL_logger::message &m) throw(std::exception);

	public:
		systemLogger_element(const uniString::utf8 &log_object_name,
							 const uniFile::filenameType &fullExePath,
							 const uniString::utf8 &loggerConfigString) throw(std::exception);
		~systemLogger_element() throw();
		static uniString::utf8 panicConfiguration() throw() { return "EEW  Z"; }
	};

#else // Unix

	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////// Unix ELEMENTS ///////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	class fileLogger_element: public logger_element<AOL_logger::message>
	{
	private:
		uniFile::filenameType	m_fileName;
		int			m_file;
		time_t		m_lastRolloverTime;
		const int	m_rolloverInterval;
		const int	m_numFileBackups;
		const bool	m_archiveFileBackups;
		bool		m_first;
        size_t      m_SID;

		void rotate() throw();

		virtual void install() throw(std::exception){}
		virtual void log(AOL_logger::message &m) throw(std::exception);
		virtual void uninstall() throw(){}

		static uniFile::filenameType make_backup_log(const uniFile::filenameType &filename,int which) throw();
		static uniFile::filenameType make_archive_log() throw();

	public:
		fileLogger_element(const uniFile::filenameType &filename,
						   const uniFile::filenameType &defaultFilename,
						   bool &useDefaultPath, int backups,
						   bool archive, int rolloverInterval, size_t SID = 0) throw(std::exception);
		~fileLogger_element() throw();
	};

	class consoleLogger_element: public logger_element<AOL_logger::message>
	{
		private:
		int m_stdoutConsole;
		int m_stderrConsole;

		virtual void install() throw(std::exception){}
		virtual void log(AOL_logger::message &m) throw(std::exception);
		virtual void uninstall() throw(){}

	public:
		consoleLogger_element() throw(std::exception);
		~consoleLogger_element() throw();
	};

	// unix system logger (not implemented)
	class systemLogger_element: public logger_element<AOL_logger::message>
	{
	private:
		virtual void install() throw(std::exception){}
		virtual void uninstall() throw() {}
		virtual void log(const AOL_logger::message &/*m*/) throw(std::exception){}

	public:
		explicit systemLogger_element(const uniString::utf8 &/*srcName*/) throw(){}
		~systemLogger_element() throw(){}
	};

	#endif
	typedef messageThread<AOL_logger::logger<AOL_logger::message,AOL_logger::logger_element<AOL_logger::message> > > stdLog_t;
} // namespace AOLLogger

#endif
