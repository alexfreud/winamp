#include "precomp_wasabi_bfc.h"

#include "std_file.h"
#include <bfc/file/readdir.h>
#include <bfc/platform/strcmp.h>

#ifdef WIN32
#include <shellapi.h>	// for ShellExecute
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

#define TMPNAME_PREFIX L"WTF"

#ifndef _NOSTUDIO
#include <bfc/parse/pathparse.h>

#undef fopen
#undef fclose
#undef fseek
#undef ftell
#undef fread
#undef fwrite
#undef fgets
#undef fprintf
#undef unlink
#undef access

#ifdef WASABI_COMPILE_FILEREADER
static PtrList<void> fileReaders;

OSFILETYPE FileReaderOpen(const wchar_t *filename, OSFNCSTR mode)
{
	OSFILETYPE ret = NULL;

	const wchar_t *rFilename = filename;
	wchar_t str[WA_MAX_PATH] = L"";

	if (wcsstr(filename, L".."))
	{
		PathParserW pp(filename);
		for (int i = 0;i < pp.getNumStrings();i++)
		{
			if (!wcscmp(pp.enumString(i), L".."))
			{
				PathParserW pp2(str);
				if (pp2.getNumStrings() <= 0)
					return NULL;
				ASSERTPR(pp2.getNumStrings() > 0, "we don't handle this right, and I'm not sure how to fix it because I'm not sure what the code should do with a leading .. --BU");
				int l = (int)wcslen(pp2.enumString(pp2.getNumStrings() - 1));
				str[wcslen(str) - l - 1] = 0;
				continue;
			}
			if (!wcscmp(pp.enumString(i), L"."))
				continue;
			wcscat(str, pp.enumString(i));
			wcscat(str, L"/");
		}
		str[wcslen(str) - 1] = 0;
		rFilename = str;
	}

	if (WASABI_API_FILE && (ret = (OSFILETYPE )WASABI_API_FILE->fileOpen(rFilename, mode)))
	{
		fileReaders.addItem((void *)ret);
		return ret;
	}
	return 0;
}
#endif

static DWORD mode_to_access(const wchar_t *mode)
{
	DWORD access_flags=0;
	if (mode)
	{
		if (mode[0]=='r')
			access_flags|=GENERIC_READ;

		if (mode[0]=='w' || mode[0] == 'a')
		{
			access_flags|=GENERIC_WRITE;
			if (mode[1] == '+')
				access_flags|=GENERIC_READ;
		}
	}
	return access_flags;
}

static DWORD mode_to_create(const wchar_t *mode)
{
	if (mode[0]=='r')
return OPEN_EXISTING;
	if (mode[0] == 'w')
		return CREATE_ALWAYS;
		if (mode[0] == 'a')
			return OPEN_ALWAYS;

		return OPEN_ALWAYS;
}

OSFILETYPE WFOPEN(const wchar_t *filename, OSFNCSTR mode, bool useFileReaders)
{
	if (!filename || !*filename)
		return OPEN_FAILED;

	if (!mode)
		mode = WF_WRITE_BINARY;

	OSFILETYPE ret = OPEN_FAILED;	

	if (!WCSNICMP(filename, L"file:", 5))
		filename += 5;

#ifdef _WIN32
	ret = CreateFileW(filename, mode_to_access(mode), FILE_SHARE_READ, 0, mode_to_create(mode), FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if (ret != OPEN_FAILED && mode[0]=='a')
		SetFilePointer(ret, 0, 0, FILE_END);
#elif defined(__APPLE__)
  // this is kind of slow, but hopefully this function isn't called enough for a major performance impact
  // maybe it'd be faster if we did -fshort-wchar and used CFStringCreateWithCharactersNoCopy
  CFStringRef cfstr =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)filename, wcslen(filename)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
  if (cfstr)
  {
  size_t len = CFStringGetMaximumSizeOfFileSystemRepresentation(cfstr);
  if (len)
  {
    char *tmpfn = alloca(len);
    if (tmpfn)
    {
      if (CFStringGetFileSystemRepresentation(cfstr, tmpfn, len))
        ret = fopen(tmpfn, mode);
    }
  }
  CFRelease(cfstr);
  }
#else
#error port me
#endif

	if (ret != OPEN_FAILED)
		return ret;

	// File not found... try to open it with the file readers
	// but before that, resolve ".." in path so zip can find it
#ifdef WASABI_COMPILE_FILEREADER
	if (useFileReaders)
	{
		if (ret = FileReaderOpen(filename, mode))
			return ret;
		else
			return OPEN_FAILED;
	}
#endif
	// File still not found ...

	return OPEN_FAILED;
}

int FCLOSE(OSFILETYPE stream)
{
#ifdef WASABI_COMPILE_FILEREADER
	if (fileReaders.searchItem((void *)stream) != -1)
	{
		fileReaders.removeItem((void *)stream);
		WASABI_API_FILE->fileClose((void *)stream);
		return 0;
	}
#endif
	return !CloseHandle(stream);
}

static __int64 Seek64(HANDLE hf, __int64 distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;

	li.QuadPart = distance;

	li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}

int FSEEK(OSFILETYPE stream, long offset, int origin)
{
#ifdef WASABI_COMPILE_FILEREADER
	if (fileReaders.searchItem((void *)stream) != -1)
		return WASABI_API_FILE->fileSeek(offset, origin, (void *)stream);
#endif
	return (int)Seek64(stream, offset, origin);
}

uint64_t FTELL(OSFILETYPE stream)
{
#ifdef WASABI_COMPILE_FILEREADER
	if (fileReaders.searchItem((void *)stream) != -1)
		return WASABI_API_FILE->fileTell((void *)stream);
#endif
	return Seek64(stream, 0, FILE_CURRENT);
}

size_t FREAD(void *buffer, size_t size, size_t count, OSFILETYPE stream)
{
#ifdef WASABI_COMPILE_FILEREADER
	if (fileReaders.searchItem((void *)stream) != -1)
		return WASABI_API_FILE->fileRead(buffer, size*count, (void *)stream);
#endif
	DWORD bytesRead=0;
	ReadFile(stream, buffer, (DWORD)(size*count), &bytesRead, NULL);
	return bytesRead;
}

size_t FWRITE(const void *buffer, size_t size, size_t count, OSFILETYPE stream)
{
#ifdef WASABI_COMPILE_FILEREADER
	if (fileReaders.searchItem((void *)stream) != -1)
		return WASABI_API_FILE->fileWrite(buffer, (int)(size*count), (void *)stream);
#endif

		DWORD bytesWritten=0;
		WriteFile(stream, buffer, (DWORD)(size*count), &bytesWritten, NULL);
		return bytesWritten;
}

uint64_t FGETSIZE(OSFILETYPE stream)
{
#ifdef WASABI_COMPILE_FILEREADER
	if (fileReaders.searchItem((void *)stream) != -1)
		return WASABI_API_FILE->fileGetFileSize((void *)stream);
#endif
		LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(stream, (LPDWORD)&position.HighPart); 	
	
	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return INVALID_FILE_SIZE;
	else
		return position.QuadPart;
}
/*
char *FGETS(char *string, int n, OSFILETYPE stream)
{
#ifdef WASABI_COMPILE_FILEREADER
	if (fileReaders.searchItem((void *)stream) != -1)
	{
		char c;
		char *p = string;
		for (int i = 0;i < (n - 1);i++)
		{
			if (!WASABI_API_FILE->fileRead(&c, 1, stream))
			{
				if (!i) return NULL;
				break;
			}
			if (c == 0x0d) continue;
			if (c == 0x0a) break;
			*p++ = c;
		}
		*p = 0;
		return string;
	}
#endif
	return fgets(string, n, stream);
}
*/
/*
int FPRINTF(OSFILETYPE stream, const char *format , ...)
{
	int ret;
	va_list args;
	va_start (args, format);
#ifdef WASABI_COMPILE_FILEREADER
	if (fileReaders.searchItem((void *)stream) != -1)
	{
		String p;
		ret = p.vsprintf(format, args);
		FWRITE(p.v(), p.len(), 1, stream);
	}
	else
#endif
		ret = vfprintf(stream, format, args); //real stdio
	va_end (args);
	return ret;
}*/

OSFNCSTR TMPNAM2(OSFNSTR str, int val)
{
#ifdef WIN32
	wchar_t tempPath[MAX_PATH-14] = {0};
	static wchar_t tempName[MAX_PATH];
	GetTempPathW(MAX_PATH-14, tempPath);
	GetTempFileNameW(tempPath, TMPNAME_PREFIX, val, tempName);

	if (str)
	{
		wcsncpy(str, tempName, MAX_PATH);
		return str;
	}
	else
	{
		return tempName;
	}
#elif defined(LINUX) || defined(__APPLE__)
	mkstemp(StringPrintf("%sXXXXXX", str).getNonConstVal());
	return (const char *)str;
#endif
}

OSFNCSTR TMPNAM(OSFNSTR string)
{
	return TMPNAM2(string, 0);
}

int UNLINK(OSFNCSTR filename)
{
#ifdef WASABI_COMPILE_FILEREADER
	return FDELETE(filename);
#elif defined(_WIN32)
	return _wunlink(filename);
#else
	return unlink(filename); // this has been undefed at the top of this file
#endif
}

int ACCESS(const char *filename, int mode)
{
#ifdef WIN32
	return _access(filename, mode);
#else
	return access(filename, mode); // this has been undefed at the top of this file
#endif
}

int WACCESS(OSFNCSTR filename, int mode)
{
#ifdef WIN32
	return _waccess(filename, mode);
#elif defined(__APPLE__)
	return access(filename, mode); // this has been undefed at the top of this file
#endif
}

int FDELETE(const wchar_t *filename, int permanently)
{
#ifdef WASABI_COMPILE_FILEREADER
	if (permanently)
		return WASABI_API_FILE->fileRemove(filename);
	else
		return WASABI_API_FILE->fileRemoveUndoable(filename);
#else
	return UNLINK(filename);
#endif
}

int MOVEFILE(const wchar_t * filename, const wchar_t *destfilename)
{
#ifdef WASABI_COMPILE_FILEREADER
	return WASABI_API_FILE->fileMove(filename, destfilename);
#elif defined(_WIN32)
	return MoveFileW(filename, destfilename);
#else
	return rename(filename, destfilename);
#endif
}

#ifdef WIN32
#include <shlobj.h>
#include <shellapi.h>

static HRESULT ResolveShortCut(LPCWSTR pszShortcutFile, LPWSTR pszPath, int maxbuf)
{
	HRESULT hres;
	IShellLinkW* psl;
	wchar_t szGotPath[MAX_PATH] = {0};
	WIN32_FIND_DATAW wfd;

	*pszPath = 0;   // assume failure

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
	                        IID_IShellLinkW, (void **) & psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		hres = psl->QueryInterface(IID_IPersistFile, (void **) & ppf); // OLE 2!  Yay! --YO
		if (SUCCEEDED(hres))
		{
			hres = ppf->Load(pszShortcutFile, STGM_READ);
			if (SUCCEEDED(hres))
			{
				hres = psl->Resolve(HWND_DESKTOP, SLR_ANY_MATCH);
				if (SUCCEEDED(hres))
				{
					wcsncpy(szGotPath, pszShortcutFile, MAX_PATH);
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATAW *) & wfd,
					                    SLGP_SHORTPATH );
					wcsncpy(pszPath, szGotPath, maxbuf);
					if (maxbuf) pszPath[maxbuf] = 0;
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	return SUCCEEDED(hres);
}
#endif

// ommitting a maxbuf param was just asking for trouble...
int StdFile::resolveShortcut(OSFNCSTR filename, OSFNSTR destfilename, int maxbuf)
{
#ifdef WIN32
	return ResolveShortCut(filename, destfilename, maxbuf);
#elif defined(LINUX) || defined(__APPLE__)
	return readlink(filename, destfilename, maxbuf);
#else
#error port me
#endif
}

#endif // ndef _NOSTUDIO