/*
 * Copyright 2000 Martin Fuchs
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// 03/29/2004, f. gastellu : added _makepath and _wmakepath

#include "splitpath.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WANT_UNICODE

void _wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext)
{
        const WCHAR* end; /* end of processed string */
        const WCHAR* p;   /* search pointer */
        const WCHAR* s;   /* copy pointer */

        /* extract drive name */
        if (path[0] && path[1]==':') {
                if (drv) {
                        *drv++ = *path++;
                        *drv++ = *path++;
                        *drv = L'\0';
                }
        } else if (drv)
                *drv = L'\0';

        /* search for end of string or stream separator */
        for(end=path; *end && *end!=L':'; )
                end++;

        /* search for begin of file extension */
        for(p=end; p>path && *--p!=L'\\' && *p!=L'/'; )
                if (*p == L'.') {
                        end = p;
                        break;
                }

        if (ext)
                for(s=end; *ext=*s++; )
                        ext++;

        /* search for end of directory name */
        for(p=end; p>path; )
                if (*--p=='\\' || *p=='/') {
                        p++;
                        break;
                }

        if (name) {
                for(s=p; s<end; )
                        *name++ = *s++;

                *name = L'\0';
        }

        if (dir) {
                for(s=path; s<p; )
                        *dir++ = *s++;

                *dir = L'\0';
        }
}

#endif

#ifdef WANT_ACP

void _splitpath(const char* path, char* drv, char* dir, char* name, char* ext)
{
        const char* end; /* end of processed string */
        const char* p;   /* search pointer */
        const char* s;   /* copy pointer */

        /* extract drive name */
        if (path[0] && path[1]==':') {
                if (drv) {
                        *drv++ = *path++;
                        *drv++ = *path++;
                        *drv = '\0';
                }
        } else if (drv)
                *drv = '\0';

        /* search for end of string or stream separator */
        for(end=path; *end && *end!=':'; )
                end++;

        /* search for begin of file extension */
        for(p=end; p>path && *--p!='\\' && *p!='/'; )
                if (*p == '.') {
                        end = p;
                        break;
                }

        if (ext)
                for(s=end; (*ext=*s++); )
                        ext++;

        /* search for end of directory name */
        for(p=end; p>path; )
                if (*--p=='\\' || *p=='/') {
                        p++;
                        break;
                }

        if (name) {
                for(s=p; s<end; )
                        *name++ = *s++;

                *name = '\0';
        }

        if (dir) {
                for(s=path; s<p; )
                        *dir++ = *s++;

                *dir = '\0';
        }
}

#endif

#ifdef WANT_UNICODE

void _wmakepath( WCHAR *path, const WCHAR *drive, const WCHAR *dir, const WCHAR *fname, const WCHAR *ext ) {
  if (!path) return;
  *path = 0;
  if (drive) {
    *path++ = *drive;
    *path++ = ':';
  }
  if (dir) {
    strcat(path, dir);
    if (dir[strlen(dir)-1] != '\\' && dir[strlen(dir)-1] != '/')
    strcat(path, "/");
    path += strlen(path);
  }
  if (fname) strcat(path, fname);
  if (ext) {
    if (*ext != '.') strcat(path++, ".");
    strcat(path, ext);
  }
}

#endif

#ifdef WANT_ACP

void _makepath( char *path, const char *drive, const char *dir, const char *fname, const char *ext ) {
  if (!path) return;
  *path = 0;
  if (drive) {
    *path++ = *drive;
    *path++ = ':';
  }
  if (dir) {
    strcat(path, dir);
    if (dir[strlen(dir)-1] != '\\' && dir[strlen(dir)-1] != '/')
    strcat(path, "/");
    path += strlen(path);
  }
  if (fname) strcat(path, fname);
  if (ext) {
    if (*ext != '.') strcat(path++, ".");
    strcat(path, ext);
  }
}

#endif


/*
void main()     // test splipath()
{
        TCHAR drv[_MAX_DRIVE+1], dir[_MAX_DIR], name[_MAX_FNAME], ext[_MAX_EXT];

        _tsplitpath(L"x\\y", drv, dir, name, ext);
        _tsplitpath(L"x\\", drv, dir, name, ext);
        _tsplitpath(L"\\x", drv, dir, name, ext);
        _tsplitpath(L"x", drv, dir, name, ext);
        _tsplitpath(L"", drv, dir, name, ext);
        _tsplitpath(L".x", drv, dir, name, ext);
        _tsplitpath(L":x", drv, dir, name, ext);
        _tsplitpath(L"a:x", drv, dir, name, ext);
        _tsplitpath(L"a.b:x", drv, dir, name, ext);
        _tsplitpath(L"W:\\/\\abc/Z:~", drv, dir, name, ext);
        _tsplitpath(L"abc.EFGH:12345", drv, dir, name, ext);
        _tsplitpath(L"C:/dos/command.com", drv, dir, name, ext);
}
*/

#ifdef __cplusplus
}
#endif