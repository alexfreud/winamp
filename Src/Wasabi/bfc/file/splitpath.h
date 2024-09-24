#ifndef _SPLITPATH_H
#define _SPLITPATH_H

#ifndef _WIN32
#ifdef __cplusplus
extern "C" {
#endif

//#define WANT_UNICODE
#define WANT_ACP

#ifdef WANT_UNICODE
void _wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext);
void _wmakepath( WCHAR *path, const WCHAR *drive, const WCHAR *dir, const WCHAR *fname, const WCHAR *ext );
#endif

#ifdef WANT_ACP
void _splitpath(const char* path, char* drv, char* dir, char* name, char* ext);
void _makepath( char *path, const char *drive, const char *dir, const char *fname, const char *ext);
#endif

#ifdef __cplusplus
}
#endif
#endif
#endif
