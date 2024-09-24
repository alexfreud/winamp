#ifndef __NDE_VFS_H
#define __NDE_VFS_H

#include <bfc/platform/types.h>
#include <windows.h>

//#define NDE_ALLOW_NONCACHED

/*
#ifdef NDE_ALLOW_NONCACHED
  #ifndef NDE_NOWIN32FILEIO
    #error NDE_ALLOW_NONCACHED at least for now requires NDE_NOWIN32FILEIO
  #endif
#endif
*/

#define VFILE_INC		65536

#define VFS_READ		1
#define VFS_WRITE		2
#define VFS_SEEKEOF		4
#define VFS_CREATE		8
#define VFS_NEWCONTENT	16
#define VFS_MUSTEXIST	32
#if defined(NDE_ALLOW_NONCACHED)
#include "../nu/RingBuffer.h"
#endif
typedef struct VFILEStruct
{
	void VFILE()
	{
		data = 0;
		ptr = 0;
		filesize = 0;
		maxsize = 0;
		filename = 0;
		mode = 0;
		cached = FALSE;
		dirty = 0;

	#ifdef NDE_ALLOW_NONCACHED
	  #ifdef NDE_NOWIN32FILEIO
		rfile = 0;
	  #else
		hfile = NULL;
		endoffile = false;
		r = 0;
	  #endif
	#endif

		mutex = NULL;
		locks = 0;
	}

    uint8_t *data;
    uint32_t ptr;
    uint32_t filesize;
    uint32_t maxsize;
    wchar_t *filename;
    char mode;
    BOOL cached;
    int dirty;

#ifdef NDE_ALLOW_NONCACHED
  #ifdef NDE_NOWIN32FILEIO
    FILE *rfile;
  #else
    HANDLE hfile;
	bool endoffile;
	RingBuffer r;
  #endif
#endif
	HANDLE mutex;
	int locks;
} VFILE;

#ifdef __cplusplus
extern "C" {
#endif

VFILE *Vfnew(const wchar_t *filename, const char *mode, BOOL Cached);
VFILE *Vfopen(VFILE *f, wchar_t *filename, const char *mode, BOOL Cached); // filename must be an NDE string
size_t Vfread(void *ptr, size_t size, VFILE *buf);
void Vfseek(VFILE *fl, uint32_t i, int whence);
uint32_t Vftell(VFILE *fl);
void Vfclose(VFILE *fl);
void Vfdestroy(VFILE *fl); // benski> TODO: 
void Vfwrite(const void *ptr, size_t size, VFILE *f);
int Vfeof(VFILE *fl);
int Vsync(VFILE *fl); // 1 on error updating
int Vflock(VFILE *fl, BOOL is_sync=TRUE); // returns 0 on failure
void Vfunlock(VFILE *fl, BOOL is_sync=TRUE);

#ifdef __cplusplus
}
#endif

#endif