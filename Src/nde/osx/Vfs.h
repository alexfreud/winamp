#ifndef __NDE_VFS_H
#define __NDE_VFS_H

#include <bfc/platform/types.h>
#include <stdio.h>

//#define NDE_ALLOW_NONCACHED

/*
#ifdef NDE_ALLOW_NONCACHED
  #ifndef NDE_NOWIN32FILEIO
    #error NDE_ALLOW_NONCACHED at least for now requires NDE_NOWIN32FILEIO
  #endif
#endif
*/

#define VFILE_INC 65536

#define VFS_READ        1
#define VFS_WRITE        2
#define VFS_SEEKEOF      4
#define VFS_CREATE      8
#define VFS_NEWCONTENT 16
#define VFS_MUSTEXIST  32

typedef struct {
    uint8_t *data;
    unsigned long ptr;
    unsigned long filesize;
    unsigned long maxsize;
    char *filename;
    char mode;
    BOOL cached;
    int dirty;

#ifdef NDE_ALLOW_NONCACHED
    FILE *rfile;
#endif
		FILE *lockFile;
		char lockname[1024];
		int locks;
} VFILE;


#ifdef __cplusplus
extern "C" {
#endif

VFILE *Vfnew(const char *fl, const char *mode, BOOL Cached);
VFILE *Vfopen(VFILE *f, const char *fl, const char *mode, BOOL Cached);
size_t Vfread(void *ptr, size_t size, VFILE *buf);
void Vfseek(VFILE *fl, long i, int whence);
unsigned long Vftell(VFILE *fl);
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

