#include <windows.h>
#include <mmio.h>
#include <shlwapi.h>
#include "../../winamp/wa_ipc.h"
#include "../../winamp/in2.h"
extern In_Module mikmod;
//big mess here

typedef struct      // quick _thiscall hack
{
	char *(_fastcall *GetDescription)(void*,int);
	int (_fastcall *Open)(void*,int,char *url, int *killswitch);
	int (_fastcall *Read)(void*,int,void *buffer, int length, int *killswitch);
	int (_fastcall *GetLength)(void*);
	int (_fastcall *CanSeek)(void*);
	int (_fastcall *Seek)(void*,int,int position, int *killswitch);
	char *(_fastcall *GetHeader)(void*,int,char *name);
	void (_fastcall *Release)(void*,int,int);   // rough ~WReader() hack
} RF_vtbl;

#define RF_Open(x,a,b) (*x)->Open(x,0,a,b)
#define RF_Read(x,a,b,c) (*x)->Read(x,0,a,b,c)
#define RF_GetLength(x) (*x)->GetLength(x)
#define RF_Seek(x,a,b) (*x)->Seek(x,0,a,b)
#define RF_Release(x) (*x)->Release(x,0,0)

#define READ_VER	0x100

typedef struct 
{
	int version;
	char *description;
	
	RF_vtbl ** (_cdecl *create)();
	
	int (_cdecl *ismine)(char *url);
	
} reader_source;

typedef int (_cdecl  *RF_entry)(HINSTANCE hIns,reader_source** s);

static int initialized,got_dll;
static HINSTANCE hRF;

typedef struct
{
	RF_vtbl ** r;
	UINT size,pos;
} RFstruct;

static RF_vtbl** (_cdecl *rf_create)();

static int rf_init()
{
	wchar_t fn[MAX_PATH] = {0};
	RF_entry rf_entry;
	reader_source * source;

	if (initialized) return got_dll;
	initialized=1;

	PathCombineW(fn, (wchar_t*)SendMessage(mikmod.hMainWindow, WM_WA_IPC, 0, IPC_GETSHAREDDLLDIRECTORYW), L"read_file.dll");

	hRF=LoadLibraryW(fn);
	if (!hRF) return 0;

	rf_entry = (RF_entry)GetProcAddress(hRF,"readerSource");
	if (!rf_entry)
	{
		FreeLibrary(hRF);
		return 0;
	}

	rf_entry(hRF,&source);
	
	if (source->version!=READ_VER)
	{
		FreeLibrary(hRF);
		return 0;
	}

	rf_create=source->create;	
		
	got_dll=1;
	return 1;
}

static void rf_quit()
{
	if (got_dll)
	{
		FreeLibrary(hRF);
		got_dll=0;
	}
	initialized=0;
}

static void * _cdecl  rfopen(const char * fn)
{
	int ks;
	RF_vtbl ** r;
	RFstruct * rs;


	if (!got_dll) return 0;

	r=rf_create();
	if (!r) return 0;

	ks=0;

	if (RF_Open(r,(char*)fn,&ks))
	{
		RF_Release(r);
		return 0;
	}
	
	rs=malloc(sizeof(RFstruct));
	if (!rs)
	{
		RF_Release(r);
		return 0;
	}
	rs->r=r;
	rs->pos=0;
	rs->size=RF_GetLength(r);

	return rs;
}

static size_t _cdecl rfread( void *buffer, size_t size, size_t count, void *stream )
{
	RFstruct * rs;
	int ks,rv;
	UINT siz;
	rs=stream;
	ks=0;
	siz=size*count;
	if (siz>rs->size-rs->pos) siz=rs->size-rs->pos;//just to be sure
	rv=RF_Read(rs->r,buffer,siz,&ks);
	if (rv>0) rs->pos+=rv;
	return rv;

}

static size_t _cdecl rfwrite( const void *buffer, size_t size, size_t count, void *stream ) {return -1;}

static int _cdecl rfgetc( void *stream )
{
	RFstruct * rs;
	int rv,ks;
	rv=0;
	ks=0;
	rs=stream;

	if (RF_Read(rs->r,&rv,1,&ks)>0) rs->pos++;
	else rv=EOF;
	return rv;
}

static int _cdecl rfputc( int c, void *stream )
{
    // not implemented
    return -1;
}

static int _cdecl rfseek( void *stream, long offset, int origin )
{
	RFstruct * rs;
	int ks;
	UINT new_pos;

	ks=0;
	rs=stream;

	switch(origin)
	{
	case SEEK_CUR:
		new_pos=rs->pos+offset;
		break;
	case SEEK_END:
		new_pos=rs->size+offset;
		break;
	case SEEK_SET:
		new_pos=offset;
		break;
	default:
		return -1;
	}
	if (new_pos>rs->size) new_pos=rs->size;
	if (RF_Seek(rs->r,new_pos,&ks))
	{
		return -1;
	}
	rs->pos=new_pos;
	return 0;
}

static long _cdecl rftell(void * stream)
{
	RFstruct * rs=stream;
	return rs->pos;
}

static int _cdecl rfeof(void * stream)
{
	RFstruct * rs=stream;
	return rs->pos==rs->size;
}

static int _cdecl rfclose(void * stream)
{
	RFstruct * rs=stream;
	RF_Release(rs->r);
	free(rs);
	return 0;
}

static const MMSTREAM_CALLBACK callback_rf =
{
	rfread,
	rfwrite,
	rfgetc,
	rfputc,
	rfseek,
	rftell,
	rfeof,
	rfclose
};


MMSTREAM *_mm_fopen_rf(const CHAR *fname)
{
	void * handle;
	if (!rf_init()) return 0;
	handle = rfopen(fname);
	if (!handle) return _mm_fopen(fname,"rb");
	return _mmstream_createfp_callback(handle,0,&callback_rf);
}


