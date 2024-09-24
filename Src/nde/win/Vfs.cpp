/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

Virtual File System

--------------------------------------------------------------------------- */
#include "../nde.h"

#include "vfs.h"
#include <malloc.h>
#ifndef EOF
#define EOF -1
#endif
#include <Sddl.h>
#include <strsafe.h>

#if defined(NDE_ALLOW_NONCACHED)
size_t ReadFileN(void *buffer, size_t size, VFILE *f)
{
	uint8_t *b = (uint8_t *) buffer;
	size_t total_size=0;
	while (size)
	{
		DWORD bytesRead = 0;
		DWORD toRead = min(0xffffffffUL, size);
		ReadFile(f->hfile, b, toRead, &bytesRead, NULL);
		if (bytesRead != toRead)
		{
			f->endoffile=true;
			// TODO: rewind
			return total_size+bytesRead;
		}
		size-=toRead;
		b+=toRead;
		total_size+=toRead;
	}
	return total_size;
}

size_t WriteFileN(void *buffer, size_t size,  VFILE *f)
{
	uint8_t *b = (uint8_t *) buffer;
	size_t total_size=0;
		while (size)
		{
			DWORD bytesRead = 0;
			DWORD toRead = min(0xffffffffUL, size);
			WriteFile(f->hfile, b, toRead, &bytesRead, NULL);
			if (bytesRead != toRead)
			{
				f->endoffile=true;
				// TODO: rewind
				return total_size+bytesRead;
			}
			size-=toRead;
			b+=toRead;
			total_size+=toRead;
		}
	
	return total_size;
}
#endif

// benski> i havn't the slightest fucking clue why this works, it's copypasta code from the internets
static LPCWSTR LOW_INTEGRITY_SDDL_SACL_W = L"S:(ML;;NW;;;LW)";
static bool GetLowIntegrity(SECURITY_ATTRIBUTES *attributes)
{
	PSECURITY_DESCRIPTOR pSD = NULL;

	if ( ConvertStringSecurityDescriptorToSecurityDescriptorW (
		LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL ) )
	{
		attributes->nLength = sizeof(SECURITY_ATTRIBUTES);
		attributes->lpSecurityDescriptor = pSD;
		attributes->bInheritHandle = FALSE;

		//LocalFree ( pSD );
		return true;
	}

	return false;
}

VFILE *Vfnew(const wchar_t *fl, const char *mode, BOOL Cached)
{
	if (!fl) return NULL;
	VFILE *f = (VFILE *)calloc(1, sizeof(VFILE));
	if (!f) return NULL;

#ifdef NDE_ALLOW_NONCACHED
	if (!Cached)
	{
		f->r.reserve(256); // heuristically determined
	}
	f->hfile = INVALID_HANDLE_VALUE;
#endif

#ifndef NO_TABLE_WIN32_LOCKING
	// TODO: should we retrieve a better filename, e.g. GetLongPathName, or GetFinalPathNameByHandle  on vista+
	wchar_t mutex_name[1024] = {0};
	StringCbPrintfW(mutex_name, sizeof(mutex_name), L"Global\\nde-%s", fl);

	CharLowerW(mutex_name+7);
	wchar_t *sw = mutex_name+7;
	wchar_t *has_extension=0;
	while (sw && *sw)
	{
		if (*sw == L'\\')
		{
			has_extension=0;
			*sw = L'/';
		}
		else if (*sw == L'.')
			has_extension=sw;
		sw++;
	}
	if (has_extension)
		*has_extension = 0;

	SECURITY_ATTRIBUTES attr = {0};
	if (GetLowIntegrity(&attr))
	{
		f->mutex = CreateMutexW(&attr, FALSE, mutex_name);
		LocalFree(attr.lpSecurityDescriptor);
	}
	else
		f->mutex = CreateMutexW(0, FALSE, mutex_name);

#endif
	return f;
}

//----------------------------------------------------------------------------
VFILE *Vfopen(VFILE *f, wchar_t *fl, const char *mode, BOOL Cached)
{
	if (!fl) return NULL;
	if (!f)
	{
		f = Vfnew(fl, mode, Cached);
		if (!f)
			return NULL;
	}

#ifdef NDE_ALLOW_NONCACHED
	f->cached = Cached;
#else
	f->cached = TRUE;
#endif

	if (!strchr(mode, '+'))
	{
		if (strchr(mode, 'r'))
			f->mode = VFS_READ | VFS_MUSTEXIST;
		if (strchr(mode, 'w'))
			f->mode = VFS_WRITE | VFS_CREATE | VFS_NEWCONTENT;
		if (strchr(mode, 'a'))
			f->mode = VFS_WRITE | VFS_CREATE | VFS_SEEKEOF;
	}
	else
	{
		if (strstr(mode, "r+"))
			f->mode = VFS_WRITE | VFS_MUSTEXIST;
		if (strstr(mode, "w+"))
			f->mode = VFS_WRITE | VFS_CREATE | VFS_NEWCONTENT;
		if (strstr(mode, "a+"))
			f->mode = VFS_WRITE | VFS_CREATE | VFS_SEEKEOF;
	}

	if (f->mode == 0 || ((f->mode & VFS_READ) && (f->mode & VFS_WRITE)))
	{
		Vfdestroy(f);
		return NULL;
	}

#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		f->endoffile=false;
		int readFlags=GENERIC_READ, openFlags=0;
		if (f->mode & VFS_WRITE)			readFlags|=GENERIC_WRITE;
		if (f->mode & VFS_MUSTEXIST)			openFlags=OPEN_EXISTING;
		if (f->mode & VFS_CREATE) openFlags = OPEN_ALWAYS;
		if (f->mode & VFS_NEWCONTENT) openFlags = CREATE_ALWAYS;
		f->hfile=CreateFile(fl,readFlags,FILE_SHARE_READ,0,openFlags,0,0);
		if (f->hfile!=INVALID_HANDLE_VALUE)
			f->filename = _strdup(fl);
		else
		{
			Vfdestroy(f);
			return NULL;
		}
		return f;
	}
#endif

	if (f->mode & VFS_MUSTEXIST)
	{
		if (GetFileAttributesW(fl) == INVALID_FILE_ATTRIBUTES)
		{
			Vfdestroy(f);
			return NULL;
		}
	}

	if (!(f->mode & VFS_NEWCONTENT))
	{
		int attempts=0;
		HANDLE hFile=INVALID_HANDLE_VALUE;
again:
		if (attempts<100) // we'll try for 10 seconds
		{
			hFile=CreateFileW(fl,GENERIC_READ,FILE_SHARE_READ/*|FILE_SHARE_WRITE*/,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
			if (hFile == INVALID_HANDLE_VALUE && GetLastError() == ERROR_SHARING_VIOLATION)
			{
				Sleep(100); // let's try again
				goto again;
			}
		}
		else if (hFile == INVALID_HANDLE_VALUE && GetLastError() == ERROR_SHARING_VIOLATION)
		{
			// screwed up STILL? eeergh I bet it's another program locking it, let's try with more sharing flags
			hFile=CreateFileW(fl,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
		}

		if (hFile==INVALID_HANDLE_VALUE)
		{
			f->data = (uint8_t *)calloc(VFILE_INC, 1);
			if (f->data == NULL)
			{
				Vfdestroy(f);
				return NULL;
			}
			f->filesize = 0;
			f->maxsize = VFILE_INC;
		}
		else
		{
			size_t fsize_ret_value=GetFileSize(hFile,NULL);
			if (fsize_ret_value==INVALID_FILE_SIZE)
			{
				Vfdestroy(f);
				return NULL;
			}
			f->filesize = (uint32_t)fsize_ret_value;
			f->data = (uint8_t *)calloc(f->filesize, 1);
			if (f->data == NULL)
			{
				CloseHandle(hFile);
				Vfdestroy(f);
				return NULL;
			}
			f->maxsize = f->filesize;
			DWORD r = 0;
			// TODO: benski> I think we should switch this to overlapped I/O (to allow I/O to happen as we're parsing)
			//               or switch to a memory mapped file... but we'll need to check with the profiler
			if (!ReadFile(hFile,f->data,f->filesize,&r,NULL) || r != f->filesize)
			{
				CloseHandle(hFile);
				Vfdestroy(f);
				return NULL;
			}
			CloseHandle(hFile);
		}
	}

	if (f->mode & VFS_SEEKEOF)
		f->ptr = f->filesize;

	f->filename = fl;
	ndestring_retain(f->filename);
	return f;
}

//----------------------------------------------------------------------------
void Vfclose(VFILE *f)
{
	if (!f) return;

#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		if (f->hfile!=INVALID_HANDLE_VALUE)
			CloseHandle(f->hfile);
		f->hfile=INVALID_HANDLE_VALUE;
	}
	else
#endif
	{
		if (f->mode & VFS_WRITE)
		{
			Vsync(f);
		}
	}

	ndestring_release(f->filename);
	f->filename=0;
	free(f->data);
	f->data = 0;
	f->ptr=0;
	f->filesize=0;
	f->maxsize=0;
	f->dirty=0;
}

void Vfdestroy(VFILE *f)
{
	// benski> TODO: 
	if (f)
	{
		Vfclose(f);

		while (f->locks)
			Vfunlock(f, 1);

		if (f->mutex) CloseHandle(f->mutex);
		free(f);
	}
}

//----------------------------------------------------------------------------
size_t Vfread( void *ptr, size_t size, VFILE *f )
{
	assert( ptr && f );
#ifdef NDE_ALLOW_NONCACHED
	if ( !f->cached )
	{
		size_t read = f->r.read( ptr, size );
		ptr = (uint8_t *)ptr + read;
		size -= read;
		if ( size == 0 ) return 1; // yay fully buffered read
		// if we got here, the ring buffer is empty
		f->r.clear(); // reset back to normal
		if ( size > f->r.avail() )
		{
			return ReadFileN( ptr, size, f ) == size;
		}
		void *data = f->r.LockBuffer();
		size_t bytes_read = ReadFileN( data, f->r.avail(), f );
		f->r.UnlockBuffer( bytes_read );
		read = f->r.read( ptr, size );
		return read == size;
	}
#endif
	//if (!size) return 0;
	if ( size + f->ptr > f->filesize )
	{
		//FUCKO: remove this
		if ( !( f->ptr < f->filesize ) )
		{
#ifdef _DEBUG
			char buf[ 128 ] = { 0 };
			StringCbPrintfA( buf, sizeof( buf ), "NDE/VFS: VFS read at %d/%d (%d bytes) is bad\n", f->ptr, f->filesize, size );
			OutputDebugStringA( buf );
#endif

			//      if (!f->flushtable) // this would be ideal, if we could figure out f->flushtable
			//      f->flushtable=MessageBox(g_hwnd,"DB read failed, DB may be corrupted.\r\n\r\n"
			//    "Hit Retry to continue, or Cancel to clear the DB and start over","Winamp Library Error",MB_RETRYCANCEL) == IDCANCEL; //fucko
			//MessageBox(g_hwnd,"DB read failed, DB may be corrupted. If this error persists, remove all files from the library.",
			//        "Winamp Library Error",MB_OK);
			return 0;
		}

		size = f->filesize - f->ptr;
	}

	memcpy( ptr, f->data + f->ptr, size );
	f->ptr += (uint32_t)size;

	return 1;
}

//----------------------------------------------------------------------------
void Vfwrite(const void *ptr, size_t size, VFILE *f)
{
	if (!ptr || !f) return;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		// TODO: with some cleverness we might be able to make this write to the read cache

		// if we're cached, then our file position is off and we need to adjust
		if (!f->r.empty())
			Vfseek(f, -(f->r.size()), SEEK_CUR);

		f->r.clear(); 
		WriteFileN(ptr, size, f);
		return;
	}
#endif
	f->dirty=1;
	size_t s = (size);
	if (s + f->ptr > f->maxsize)
	{
		// grow f->data,f->maxsize to be (s + f->ptr + VFILE_INC-1)&~(VFILE_INC-1)
		// instead of calling Vgrow again which gets kinda slow
		size_t newsize=(s + f->ptr + VFILE_INC-1)&~(VFILE_INC-1);
		uint8_t *newdata=(uint8_t *)realloc(f->data,newsize);
		if (newdata == NULL) return;
		f->data = newdata;
		memset(f->data+f->maxsize,0,newsize-f->maxsize);
		f->maxsize=(uint32_t)newsize;
	}
	memcpy(f->data + f->ptr, ptr, s);
	f->ptr += (uint32_t)s;
	if (f->ptr > f->filesize)
		f->filesize = f->ptr;
}

//----------------------------------------------------------------------------
void Vgrow(VFILE *f)
{
	if (!f) return;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached) return;
#endif
	uint8_t *newdata=(uint8_t *)realloc(f->data, f->maxsize + VFILE_INC);
	if (newdata == NULL) return;
	f->data = newdata;
	f->maxsize += VFILE_INC;
}

//----------------------------------------------------------------------------
uint32_t Vftell(VFILE *f)
{
	if (!f) return (unsigned)-1;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		return SetFilePointer(f->hfile, 0, NULL, FILE_CURRENT);
	}
#endif
	return f->ptr;
}

//----------------------------------------------------------------------------
void Vfseek(VFILE *f, uint32_t i, int whence)
{
	if (!f) return;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		if (whence == SEEK_CUR && i > 0 && i <f->r.size())
		{
			f->r.advance(i);
		}
		else
		{
			f->r.clear();
			SetFilePointer(f->hfile, i, NULL, whence);
			f->endoffile = false;
		}
		return;
	}
#endif
	switch (whence)
	{
		case SEEK_SET:
			f->ptr = i;
			break;
		case SEEK_CUR:
			f->ptr += i;
			break;
		case SEEK_END:
			f->ptr = f->filesize+i;
			break;
	}
}

//----------------------------------------------------------------------------
int Vfeof(VFILE *f)
{
	if (!f) return -1;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		return !!f->endoffile;
	}
#endif
	return (f->ptr >= f->filesize);
}

//----------------------------------------------------------------------------
int Vsync(VFILE *f)
{
	if (!f) return 0;
	if (!f->dirty) return 0;

	if (f->mode & VFS_WRITE)
	{
#ifdef NDE_ALLOW_NONCACHED
		if (!f->cached)
		{
			LONG p = SetFilePointer(f->hfile, 0, NULL, FILE_CURRENT);
			CloseHandle(f->hfile);
			f->hfile = CreateFileW(f->filename,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,0,NULL);
			if (f->hfile == INVALID_HANDLE_VALUE)
				return 1;
			SetFilePointer(f->hfile, p, NULL, SEEK_SET);
			f->endoffile=false;

			return 0;
		}
#endif

		wchar_t newfn[MAX_PATH] = {0};
		wchar_t oldfn[MAX_PATH] = {0};
		DWORD mypid=GetCurrentProcessId();

		StringCchPrintfW(newfn, MAX_PATH, L"%s.n3w%08X",f->filename,mypid);
		StringCchPrintfW(oldfn, MAX_PATH, L"%s.o1d%08X",f->filename,mypid);

		DeleteFileW(newfn);
		DeleteFileW(oldfn);

		HANDLE hFile = CreateFileW(newfn,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,NULL);
		int success=0;
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD o = 0;
			if (WriteFile(hFile,f->data,f->filesize,&o,NULL) && o == f->filesize) success++;
			CloseHandle(hFile);
		}
		if (!success)
		{
			DeleteFileW(newfn);
			return 1;
		}

		// TODO use this to keep a backup of the database file for edit fails, etc
		if (MoveFileW(f->filename,oldfn) == 0) // if the function fails
		{
			CopyFileW(f->filename,oldfn, FALSE);
			DeleteFileW(f->filename);
		}

		int rv=0;
		if (MoveFileW(newfn,f->filename) == 0 && CopyFileW(newfn,f->filename, FALSE) == 0)
		{
			MoveFileW(oldfn,f->filename); // restore old file
			rv=1;
		}
		else
		{
			f->dirty=0;
		}

		// clean up our temp files
		DeleteFileW(oldfn);
		DeleteFileW(newfn);

		return rv;
	}
	f->dirty=0;
	return 0;
}

// returns 0 on failure
int Vflock(VFILE *fl, BOOL is_sync)
{	
#ifndef NO_TABLE_WIN32_LOCKING
	if (!fl) return 0;
	if (!is_sync && fl->cached)
		return 1;
	// try for 10 seconds
	if (fl->locks++ == 0)
	{
		if (WaitForSingleObject(fl->mutex, 10000) != WAIT_OBJECT_0)
		{
			fl->locks--;
			return 0;
		}
	}
#endif
	return 1;
}

void Vfunlock(VFILE *fl, BOOL is_sync)
{
#ifndef NO_TABLE_WIN32_LOCKING
	if (!is_sync && fl->cached)
		return;

	if (fl && fl->locks == 0)
		DebugBreak();
	if (--fl->locks == 0)
	{
		if (fl && fl->mutex)
		{
			ReleaseMutex(fl->mutex);
		}
	}
#endif
}