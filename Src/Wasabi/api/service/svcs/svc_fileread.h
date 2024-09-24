#ifndef _SVC_FILEREAD_H
#define _SVC_FILEREAD_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include <api/service/services.h>
#include <stdint.h>

namespace SvcFileReader
{
	enum {
	    READ = 1,
	    WRITE = 2,
	    APPEND = 4,
	    PLUS = 8,
	    BINARY = 16,
	    TEXT = 32,
	};
};

class api_readercallback;

class NOVTABLE svc_fileReader : public Dispatchable
{
public:
	static FOURCC getServiceType() { return WaSvc::FILEREADER; }

	int isMine(const wchar_t *filename, int mode = SvcFileReader::READ); //don't really open. returns -1 if "don't know until I open it"
	int open(const wchar_t *filename, int mode = SvcFileReader::READ);
	size_t read(int8_t *buffer, size_t length);
	size_t write(const int8_t *buffer, size_t length);
	void close(); // safe to call even when not open

	int canSetEOF();
	/**
	  Asks the file reader to change the file length to newlen. Will fail if file
	  was not opened for writing.
	  @ret 1 on success, 0 on failure, -1 if operation is unsupported by this reader.
	*/
	int setEOF(uint64_t newlen);

	void abort();

	uint64_t getLength();
	uint64_t getPos();

	int canSeek();
	int seek(uint64_t position);
	uint64_t bytesAvailable(uint64_t requested);

	int hasHeaders();
	const char *getHeader(const char *header);

	int exists(const wchar_t *filename);

	int remove(const wchar_t *filename);

	int removeUndoable(const wchar_t *filename);

	int move(const wchar_t *filename, const wchar_t *destfilename);

	int canPrefetch();

	void setMetaDataCallback(api_readercallback *cb);

	enum
	{
	    ISMINE = 0,
	    OPEN = 10,
	    READ = 20,
	    WRITE = 30,
	    CLOSE = 40,
	    ABORT = 50,
	    GETLENGTH = 60,
	    GETPOS = 70,
	    CANSEEK = 80,
	    SEEK = 90,
	    HASHEADERS = 100,
	    GETHEADER = 110,
	    EXISTS = 120,
	    REMOVE = 130,
	    REMOVEUNDOABLE = 135,
	    BYTESAVAILABLE = 140,
	    SETMETADATACALLBACK = 150,
	    MOVE = 160,
	    CANPREFETCH = 170,
	    CANSETEOF = 180,
	    SETEOF = 190,
	};
};

inline
int svc_fileReader::isMine(const wchar_t *filename, int mode)
{
	return _call(ISMINE, -1, filename, mode);
}

inline int svc_fileReader::open(const wchar_t *filename, int mode)
{
	return _call(OPEN, 0, filename, mode);
}

inline size_t svc_fileReader::read(int8_t *buffer, size_t length)
{
	return _call(READ, 0, buffer, length);
}

inline size_t svc_fileReader::write(const int8_t *buffer, size_t length)
{
	return _call(WRITE, 0, buffer, length);
}

inline void svc_fileReader::close()
{
	_voidcall(CLOSE);
}

inline int svc_fileReader::canSetEOF()
{
	return _call(CANSETEOF, 0);
}

inline int svc_fileReader::setEOF(uint64_t newlen)
{
	return _call(SETEOF, -1, newlen);
}

inline void svc_fileReader::abort()
{
	_voidcall(ABORT);
}

inline uint64_t svc_fileReader::getLength()
{
	return _call(GETLENGTH, (uint64_t)-1);
}

inline uint64_t svc_fileReader::getPos()
{
	return _call(GETPOS, (uint64_t)0);
}

inline int svc_fileReader::canSeek()
{
	return _call(CANSEEK, 0);
}

inline int svc_fileReader::seek(uint64_t position)
{
	return _call(SEEK, 0, position);
}

inline uint64_t svc_fileReader::bytesAvailable(uint64_t requested)
{
	return _call(BYTESAVAILABLE, requested, requested);
}

inline int svc_fileReader::hasHeaders()
{
	return _call(HASHEADERS, 0);
}

inline const char *svc_fileReader::getHeader(const char *header)
{
	return _call(GETHEADER, (const char *)NULL, header);
}

inline int svc_fileReader::exists(const wchar_t *filename)
{
	return _call(EXISTS, -1, filename);
}

inline int svc_fileReader::remove(const wchar_t *filename)
{
	return _call(REMOVE, 0, filename);
}

inline
int svc_fileReader::removeUndoable(const wchar_t *filename)
{
	return _call(REMOVEUNDOABLE, -1, filename);
}

inline int svc_fileReader::move(const wchar_t *filename, const wchar_t *destfilename)
{
	return _call(MOVE, 0, filename, destfilename);
}

inline void svc_fileReader::setMetaDataCallback(api_readercallback *cb)
{
	_voidcall(SETMETADATACALLBACK, cb);
}

inline int svc_fileReader::canPrefetch()
{
	return _call(CANPREFETCH, 1);
}

#endif
