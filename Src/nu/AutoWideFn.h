#ifndef NULLSOFT_UTILITY_AUTOWIDEFN_H
#define NULLSOFT_UTILITY_AUTOWIDEFN_H


/* Winamp defines this, but this little block lets us use this thing outside of Winamp */
#ifndef FILENAME_SIZE
#define FILENAME_SIZE (MAX_PATH*4)
#define REMOVE_FILENAME_SIZE
#endif

#include <windows.h>
#include "AutoWide.h"
#include "AutoChar.h"
#include <shlwapi.h> 
/*
Tries to find a filename that underwent a destructive Unicode-to-ANSI conversion
*/

#pragma warning(push)
#pragma warning(disable:4995)
class AutoWideFn
{
public:
	AutoWideFn(const char *narrowFn)
	{
		wideFn[0]=0;
		if (!narrowFn)
			return;
		CreateFile_HACK(narrowFn, wideFn);
	}

	operator wchar_t *() { return wideFn; }
	bool unicode_find(/*char *path,*/ char *pattern, wchar_t *out, UINT out_ptr, bool dir, HANDLE *f)
	{
		WIN32_FIND_DATAW fd = {0};

		if (*f == INVALID_HANDLE_VALUE)
		{
			lstrcpyW(out + out_ptr, L"*");
			*f = FindFirstFileW(out, &fd);
			out[out_ptr] = 0;
			if (*f == INVALID_HANDLE_VALUE) return 0;
		}
		else
		{
			if (!FindNextFileW(*f, &fd))
			{
				FindClose(*f);
				*f = INVALID_HANDLE_VALUE;
				return 0;
			}
		}

		if (*f == INVALID_HANDLE_VALUE) return 0;
		char temp[MAX_PATH*2 + 1] = {0};
		do
		{
			if (dir)
			{
				if (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) continue;
			}
			else
			{
				if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) continue;
			}
			WideCharToMultiByte(CP_ACP, 0, fd.cFileName, -1, temp, sizeof(temp), 0, 0);
			//wcstombs(temp,fd.cFileName,sizeof(temp));
			if (!_stricmp(temp, pattern))
			{ //found
				lstrcpyW(out + out_ptr, fd.cFileName);
				return 1;
			}

			WideCharToMultiByte(CP_ACP, 0, fd.cAlternateFileName, -1, temp, sizeof(temp), 0, 0);
			if (!_stricmp(temp, pattern))
			{ //found
				lstrcpyW(out + out_ptr, fd.cFileName);
				return 1;
			}
		}
		while (FindNextFileW(*f, &fd));
		FindClose(*f);
		*f = INVALID_HANDLE_VALUE;

		return 0;
	}

	bool unicode_open_recur(char *path, char *ptr, wchar_t *out, UINT out_ptr)
	{
		char * next = strchr(ptr, '\\');
		if (next)
		{ //dig another dir
			HANDLE f = INVALID_HANDLE_VALUE;
			bool found;
			do
			{
				next[0] = 0;
				char * zz = _strdup(ptr);
				char bk = *ptr;
				*ptr = 0;
				found = unicode_find(/*path,*/ zz, out, out_ptr, 1, &f);
				free(zz);
				*ptr = bk;
				next[0] = '\\';
				if (found)
				{
					UINT op_bk = out_ptr;
					while (out_ptr < FILENAME_SIZE && out[out_ptr]) out_ptr++;
					out[out_ptr++] = '\\';
					if (unicode_open_recur(path, next + 1, out, out_ptr))
					{
						if (f != INVALID_HANDLE_VALUE) FindClose(f);
						return 1;
					}
					out_ptr = op_bk;
					out[out_ptr] = 0;
				}
			} while (found);
		}
		else
		{ //final dir
			HANDLE f = INVALID_HANDLE_VALUE;
			char * zz = _strdup(ptr);
			char bk = *ptr;
			*ptr = 0;
			bool found = unicode_find(/*path,*/ zz, out, out_ptr, 0, &f);
			if (!found)
			{
				if (f != INVALID_HANDLE_VALUE) 
				{
					FindClose(f);
					f = INVALID_HANDLE_VALUE;
				}
				found = unicode_find(/*path,*/ zz, out, out_ptr, 1, &f);
			}
			free(zz);
			*ptr = bk;
			if (f != INVALID_HANDLE_VALUE) FindClose(f);
			return found;
		}
		return 0;
	}


	void CreateFile_HACK(const char * path, wchar_t out[FILENAME_SIZE])
	{
		MultiByteToWideChar(CP_ACP, 0, path, -1, out, FILENAME_SIZE);

		if (PathIsURLW(out))
			return ;

		if (!StrChrW(out, L'?'))
			return ; // no unconvertables?  Great!

//		if (PathFileExistsW(out))
	//		return ; // no unconvertables?  Great!

		bool found = false;

		memset(out, 0, FILENAME_SIZE * sizeof(wchar_t));

		char * _p = _strdup(path);
		char * t = strchr(_p, '\\');

		if (t)
		{
			char bk = t[1];
			t[1] = 0;
			UINT o = MultiByteToWideChar(CP_ACP, 0, _p, -1, out, FILENAME_SIZE);
			o--;
			t[1] = bk;
			found = unicode_open_recur(_p, t + 1, out, o);
		}
		else
			found = unicode_open_recur(_p, _p, out, 0);
		free(_p);

		if (!found)
			MultiByteToWideChar(CP_ACP, 0, path, -1, out, FILENAME_SIZE);
	}
private:
	wchar_t wideFn[FILENAME_SIZE];
};

#pragma warning(pop)

#ifdef REMOVE_FILENAME_SIZE
#undef FILENAME_SIZE
#endif

#endif
