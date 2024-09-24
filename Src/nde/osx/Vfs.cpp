
/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

Virtual File System

--------------------------------------------------------------------------- */
#include "nde.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Vfs.h"

#ifndef EOF
#define EOF -1
#endif
#include "../nu/strsafe.h"
/* the FILE (fopen/fwrite/etc) implementation for Mac and Linux */



VFILE *Vfnew(const char *fl, const char *mode, BOOL Cached)
{
	if (!fl) return NULL;
	VFILE *f = (VFILE *)malloc(sizeof(VFILE));
	if (!f)
		return NULL;
	memset(f, 0, sizeof(VFILE));
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
		free(f);
		return NULL;
	}

	snprintf(f->lockname, sizeof(f->lockname), "%s.lock", fl);

	return f;
}

//----------------------------------------------------------------------------
VFILE *Vfopen(VFILE *f, const char *fl, const char *mode, BOOL Cached)
{
	if (!f)
	{
		f = Vfnew(fl, mode, Cached);
		if (!f)
			return 0;
	}
	if (!f && !fl) return NULL;

#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		f->rfile = fopen(fl, mode);
		if (f->rfile)

			f->filename = _strdup(fl);
		}
		else
		{
			free(f);
			return NULL;
		}
		return f;
	}
#endif

	if (f->mode & VFS_MUSTEXIST)
	{
		if (access(fl, 0) != 0)
		{
			free(f);
			return NULL;
		}
	}

	if (!(f->mode & VFS_NEWCONTENT))
	{
		FILE *pf;
		pf = fopen(fl, "rb");
		if (!pf)
		{
			f->data = (unsigned char *)calloc(VFILE_INC, 1);
			if (f->data == NULL)
			{
				free(f);
				return NULL;
			}
			f->filesize = 0;
			f->maxsize = VFILE_INC;
		}
		else
		{
			fseek(pf, 0, SEEK_END);
			f->filesize = ftell(pf);
			fseek(pf, 0, SEEK_SET);
			f->data = (unsigned char *)calloc(f->filesize, 1);
			if (f->data == NULL)
			{
				free(f);
				return NULL;
			}
			f->maxsize = f->filesize;
			fread(f->data, f->filesize, 1, pf);
			fclose(pf);
		}

	}

	if (f->mode & VFS_SEEKEOF)
		f->ptr = f->filesize;

	f->filename = strdup(fl);
	return f;
}

//----------------------------------------------------------------------------
void Vfclose(VFILE *f)
{
	if (!f) return;

#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		free(f->filename);
		if (f->rfile)
			fclose(f->rfile);
		f->rfile=0;
//		free(f);
		return;
	}
#endif

	if (!(f->mode & VFS_WRITE))
	{
		free(f->filename);
		free(f->data);
		//free(f);
		return;
	}


	Vsync(f);
	while (f->locks)
		Vfunlock(f, 1);

	free(f->filename);
	free(f->data);
	//free(f);
}

//----------------------------------------------------------------------------
size_t Vfread(void *ptr, size_t size, VFILE *f)
{
	if (!ptr || !f) return 0;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		return fread(ptr, 1, size, f->rfile);
	}
#endif
	size_t s = size;
	if (!s) return 0;
	if (s + f->ptr > f->filesize)
	{
		//FUCKO: remove this
		if (!(f->ptr < f->filesize))
		{
			//      if (!f->flushtable) // this would be ideal, if we could figure out f->flushtable
			//      f->flushtable=MessageBox(g_hwnd,"DB read failed, DB may be corrupted.\r\n\r\n"
			//    "Hit Retry to continue, or Cancel to clear the DB and start over","Winamp Library Error",MB_RETRYCANCEL) == IDCANCEL; //fucko
			//MessageBox(g_hwnd,"DB read failed, DB may be corrupted. If this error persists, remove all files from the library.",
			//        "Winamp Library Error",MB_OK);
			return 0;
		}
		s = f->filesize - f->ptr;
	}
	memcpy(ptr, f->data+f->ptr, s);
	f->ptr += s;
	return (s/size);
}

//----------------------------------------------------------------------------
void Vfwrite(const void *ptr, size_t size, VFILE *f)
{
	if (!ptr || !f) return;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		fwrite(ptr, size*n,  f->rfile);
		return;
	}
#endif
	f->dirty=1;
	if (size + f->ptr > f->maxsize)
	{
		// grow f->data,f->maxsize to be (size + f->ptr + VFILE_INC-1)&~(VFILE_INC-1)
		// instead of calling Vgrow again which gets kinda slow
		size_t newsize=(size + f->ptr + VFILE_INC-1)&~(VFILE_INC-1);
		f->data=(unsigned char *)realloc(f->data,newsize);
		if (f->data == NULL) return;
		memset(f->data+f->maxsize,0,newsize-f->maxsize);
		f->maxsize=newsize;
	}
	memcpy(f->data + f->ptr, ptr, size);
	f->ptr += size;
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
	f->data = (unsigned char *)realloc(f->data, f->maxsize + VFILE_INC);
	f->maxsize += VFILE_INC;
}

//----------------------------------------------------------------------------
void Vfputs(char *str, VFILE *f)
{
	if (!f) return;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		fputs(str, f->rfile);
		return;
	}
#endif
	Vfwrite(str, strlen(str), f);
}

//----------------------------------------------------------------------------
void Vfputc(char c, VFILE *f)
{
	if (!f) return;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		fputc(c, f->rfile);
		return;
	}
#endif
	Vfwrite(&c, 1, f);
}

/* benski> unused:
// not mac compliant
//----------------------------------------------------------------------------
char *Vfgets(char *dest, int n, VFILE *f) {
if (!f) return NULL;
#ifdef NDE_ALLOW_NONCACHED
if (!f->cached)
{
#ifdef NDE_NOWIN32FILEIO
return fgets(dest, n, f->rfile);
#else
#error port me!
#endif
}
#endif

unsigned char c=0;
char *p;
int l=0;

p = dest;
while (l < n && !Vfeof(f)) {
c = f->data[f->ptr];
f->ptr++;
*p = c;
p++;
l++;
if (c == '\n') {
if (!Vfeof(f) && f->data[f->ptr] == '\r') {
f->ptr++;
}
break;
}
}
*p=0;
return dest;
}
*/

/* benski> unused:
//----------------------------------------------------------------------------
char Vfgetc(VFILE *f) {
if (!f) return EOF;
#ifdef NDE_ALLOW_NONCACHED
if (!f->cached)
#ifdef NDE_NOWIN32FILEIO
return fgetc(f->rfile);
#else
#error port me#
#endif
#endif
if (!Vfeof(f))
return f->data[f->ptr++];
return EOF;
}
*/

//----------------------------------------------------------------------------
unsigned long Vftell(VFILE *f)
{
	if (!f) return (unsigned)-1;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		return ftell(f->rfile);
	}
#endif
	return f->ptr;
}

//----------------------------------------------------------------------------
void Vfseek(VFILE *f, long i, int whence)
{
	if (!f) return;
#ifdef NDE_ALLOW_NONCACHED
	if (!f->cached)
	{
		fseek(f->rfile, i, whence);
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
		return feof(f->rfile);
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
			int p=ftell(f->rfile);
			fclose(f->rfile);
			f->rfile = fopen(f->filename, "r+b");
			if (!f->rfile)
				return 1;
			fseek(f->rfile, p, SEEK_SET);

			return 0;
		}
#endif

		char newfn[1024], oldfn[1024];
		DWORD mypid=getpid();

		snprintf(newfn,sizeof(newfn), "%s.n3w%08X",f->filename,mypid);
		snprintf(oldfn,sizeof(oldfn), "%s.o1d%08X",f->filename,mypid);

		unlink(newfn);
		unlink(oldfn);


tryagain:
		FILE *pf = fopen(newfn, "wb");
		if (!pf || fwrite(f->data, f->filesize, 1, pf) != 1)
		{
			if (pf) fclose(pf);


			printf("Error flushing DB to disk (is your drive full?)\n\nHit (R)etry to try again (recommended), or (C)ancel to abort (NOT recommended, and may leave the DB in an unuseable state)");
			fflush(stdout);
			char c;
			while (1)
			{
				scanf("%c", &c);
//				clear_stdin();
				c = toupper(c);
				if (c == 'R') goto tryagain;
				else if (c == 'C') break;
			}
			unlink(newfn);
			return 1;
		}
		fclose(pf);
		rename(f->filename,oldfn); // save old file



		int rv=0;

tryagain2:
		if (rename(newfn,f->filename))
		{

			printf("Error updating DB file on disk. This should never really happen\n\nHit (R)etry to try again (recommended), or (C)ancel to abort (NOT recommended, and may leave the DB in an unuseable state)");
			fflush(stdout);
			char c;
			while (1)
			{
				scanf("%c", &c);
//				clear_stdin();
				c = toupper(c);
				if (c == 'R') goto tryagain2;
				else if (c == 'C') break;
			}

			rename(oldfn,f->filename); // restore old file
			rv=1;
		}

		// clean up our temp files
		unlink(oldfn);
		unlink(newfn);


		//free(newfn);
		//free(oldfn);
		if (rv == 0)
			f->dirty=0;
		return rv;
	}
	f->dirty=0;
	return 0;
}

void Vfdestroy(VFILE *f)
{
	// benski> TODO: 
	if (f)
	{
		Vfclose(f);
		
		while (f->locks)
			Vfunlock(f, 1);
		
// TODO		if (f->mutex) CloseHandle(f->mutex);
		free(f);
	}
}

// benski> TODO implement these with fopen

// returns 0 on failure
int Vflock(VFILE *fl, BOOL is_sync)
{	
#ifndef NO_TABLE_WIN32_LOCKING
	if (!fl) return 0;
	if (!is_sync && fl->cached)
		return 1;
	if (fl->locks++ == 0)
	{
		int retry_cnt=0;
		FILE *fFile;
		do
		{
			fFile = fopen(fl->lockname, "wb");
			if (fFile == 0) Sleep(100);
		}
		while (fFile == 0 && retry_cnt++ < 100); // try for 10 seconds

		if (fFile == INVALID_HANDLE_VALUE) return 0; // db already locked, fail gracefully
		fl->lockFile = fFile;
	}
#endif
	return 1;

}

void Vfunlock(VFILE *fl, BOOL is_sync)
{
#ifndef NO_TABLE_WIN32_LOCKING
	if (!is_sync && fl->cached)
		return;

	if (--fl->locks == 0)
	{
		if (fl && fl->lockFile && fl->lockFile != INVALID_HANDLE_VALUE)
		{
			fclose(fl->lockFile);
			unlink(fl->lockname);
		}
	}
#endif
}
