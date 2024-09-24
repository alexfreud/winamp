#ifndef _STD_FILE_H
#define _STD_FILE_H

#include <bfc/platform/platform.h>
#include "wasabi_std.h"
#include <stdio.h>

/* TODO:
  FEOF
	FFLUSH
	FGETPOS  - maybe implement as just FTELL?
	FSETPOS  - maybe implement as just FSEEK?
	FPUTS    - no problems, look at FPRINTF implementation
	FSTAT (in conjunction with FILENO), only fill in _stat::st_size for now (via getFileSize)
*/

#ifndef _NOSTUDIO
// EXTC is used here as some .c files will use these functions

#define NO_FILEREADERS false

#ifdef _WIN32
#define WF_READONLY_BINARY L"rb"
#define WF_WRITE_TEXT L"wt"
#define WF_WRITE_BINARY L"wb"
#define WF_APPEND L"a"
#define WF_APPEND_RW L"a+"
#define OPEN_FAILED INVALID_HANDLE_VALUE
#elif defined(__APPLE__)
#define WF_READONLY_BINARY "r"
#define WF_WRITE_TEXT "w"
#define WF_WRITE_BINARY "w"
#define WF_APPEND "a"
#define WF_APPEND_RW "a+"
#define OPEN_FAILED 0
#endif

#ifdef _WIN32
typedef HANDLE OSFILETYPE;
#else
#error port me
#endif

OSFILETYPE WFOPEN(const wchar_t *filename, OSFNCSTR mode, bool useFileReaders = true);

int FCLOSE(OSFILETYPE stream);
int FSEEK(OSFILETYPE stream, long offset, int origin);
uint64_t FTELL(OSFILETYPE stream);
#undef FREAD // defined on Mac for some reason
size_t FREAD(void *buffer, size_t size, size_t count, OSFILETYPE stream);
#undef FWRITE // defined on Mac for some reason
size_t FWRITE(const void *buffer, size_t size, size_t count, OSFILETYPE stream);
//char *FGETS( char *string, int n, OSFILETYPE stream);
//int FPRINTF(OSFILETYPE stream, const char *format , ...);
uint64_t FGETSIZE(OSFILETYPE stream);
const wchar_t *TMPNAM(wchar_t *string);
OSFNCSTR TMPNAM2(wchar_t *string, int val);
int FEXISTS(const char *filename); // return 1 if true, 0 if not, -1 if unknown
int UNLINK(const wchar_t *filename); // return 1 on success, 0 on error
int WACCESS(const wchar_t *filename, int mode);
#ifdef __cplusplus
// returns 1 on success, 0 on error, -1 if undoable deletes aren't supported
int FDELETE(OSFNCSTR filename, int permanently=TRUE);
#else
int FDELETE(OSFNCSTR filename, int permanently);
#endif

// 1 on success, 0 on fail
// can't move directories between volumes on win32
int MOVEFILE(OSFNCSTR filename, OSFNCSTR destfilename);


#ifdef __cplusplus
namespace StdFile {
#endif
  int resolveShortcut(OSFNCSTR filename, OSFNSTR destfilename, int maxbuf);
#ifdef __cplusplus
};
#endif

#ifdef WASABI_COMPILE_FILEREADER

#ifndef REAL_STDIO
#ifndef __APPLE__
//#define fopen FOPEN
//#define fclose FCLOSE
//#define fseek FSEEK
//#define ftell FTELL
//#define fread FREAD
//#define fwrite FWRITE
//#define fgets FGETS
//#define fprintf FPRINTF
//#define unlink UNLINK
//#define access ACCESS
#endif
#endif //real_stdio

#endif //WASABI_COMPILE_FILEREADER
#endif //_nostudio

#endif
