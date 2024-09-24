#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "uniFile.h"
#include "stl/stringUtils.h"

using namespace std;
using namespace stringUtil;

FILE* uniFile::fopen(const uniFile::filenameType &f,const char *mode) throw()
{
#ifdef _WIN32
	uniString::utf32 u32(f);
	std::wstring u16;
	u32.toUtf16(u16);
	return _wfopen(u16.c_str(),tows(mode).c_str());
#else
	return ::fopen((const char *)f.c_str(),mode);
#endif
}

void uniFile::unlink(const uniFile::filenameType &f) throw()
{
#ifdef _WIN32
	uniString::utf32 u32(f);
	std::wstring u16;
	u32.toUtf16(u16);
	::DeleteFileW(u16.c_str());
#else
	::unlink((const char *)f.c_str());
#endif
}

bool uniFile::fileExists(const uniFile::filenameType &f) throw()
{
#ifdef _WIN32
	uniString::utf32 u32(f);
	std::wstring u16;
	u32.toUtf16(u16);
	struct _stat64i32 st = {0};
	int result = _wstat(u16.c_str(),&st);
	if (st.st_mode & _S_IFDIR) return 0;
	return (result == 0);
#else
	struct stat st;
	int result = stat((const char *)f.c_str(),&st);
	if (S_ISDIR(st.st_mode)) return 0;
	return (result == 0);
#endif
}

size_t uniFile::fileSize(const uniFile::filenameType &f) throw()
{
#ifdef _WIN32
	uniString::utf32 u32(f);
	std::wstring u16;
	u32.toUtf16(u16);
	struct _stat64i32 st;
	if (_wstat(u16.c_str(),&st) == -1) return 0;
	return st.st_size;
#else
	struct stat st;
	if (stat((const char *)f.c_str(),&st) == -1) return 0;
	return st.st_size;
#endif
}

time_t uniFile::fileTime(const uniFile::filenameType &f) throw()
{
#ifdef _WIN32
	uniString::utf32 u32(f);
	std::wstring u16;
	u32.toUtf16(u16);
	struct _stat64i32 st;
	if (_wstat(u16.c_str(),&st) == -1) return 0;
	return st.st_mtime;
#else
	struct stat st;
	if (stat((const char *)f.c_str(),&st) == -1) return 0;
	return st.st_mtime;
#endif
}
