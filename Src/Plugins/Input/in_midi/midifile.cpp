#include "main.h"
#include <intsafe.h>

//#define HUNT_LEAKS
#ifdef MF_USE_DMCRAP
extern IDirectMusicCollection *pCDLS;
#endif

#ifdef HUNT_LEAKS
static UINT n_files;
#endif

MIDI_file::MIDI_file(const char * fn) : path(fn)
{
	flags=0;
	format=0;
	len=0;tix=0;
	size=0;
	data=0;
#ifdef MF_USE_DMCRAP
	pSeg=0;
	pDLS=0;
	pDLSdata=0;
	DLSsize=0;
#endif
	info.fmt=info.ntrax=info.tix=0;
	info.channels=0;
	info.e_type=0;
	loopstart=0;loopend=0;
	loopstart_t=0;
	rmi_data=0;
	rmi_size=0;
	bmp_data=0;
	bmp_size=0;
	kar_track=0;
	tmap=0;
	smap=0;
#ifdef HUNT_LEAKS
	n_files++;
#endif
	refcount=1;
  info.traxnames=0;
}

#ifdef MF_USE_DMCRAP
extern IDirectMusicCollection *pCDLS;
extern IDirectMusicLoader* pLoader;
#endif


static bool is_gmd(const BYTE* b,size_t s)
{
	return s>12 && *(DWORD*)b==_rv('MIDI') && *(DWORD*)(b+8)==_rv('MDpg');
}

static bool is_hmi(const BYTE* b,size_t s)
{
	return s>12 && *(DWORD*)b==_rv('HMI-') && *(DWORD*)(b+4)==_rv('MIDI') && *(DWORD*)(b+8)==_rv('SONG');
}

static bool is_hmp(const BYTE* b,size_t s)
{
	if (s>8 && ((DWORD*)b)[0]==_rv('HMIM') && (((DWORD*)b)[1]==_rv('IDIP') || ((DWORD*)b)[1]==_rv('IDIR')) )
	{
		//DWORD d=*(DWORD*)(b+0x30);
		//return (d<0x40 && d);
		return 1;
	}
	else return 0;
}

static bool is_xmidi(const BYTE* b,size_t s)
{
	return s>0x20 && *(DWORD*)b==_rv('FORM') && *(DWORD*)(b+8)==_rv('XDIR') && *(DWORD*)(b+0x1e)==_rv('XMID');
}

static bool is_rmi(const BYTE* b,size_t s)
{
	return s>20+8+6+8 && *(DWORD*)b==_rv('RIFF') && *(DWORD*)(b+8)==_rv('RMID') && *(DWORD*)(b+12)==_rv('data');
}

static bool is_midi(const BYTE* b,size_t s)
{
	return s>8+6+8 && *(DWORD*)b==_rv('MThd') && *(DWORD*)(b+4)==0x06000000 && *(DWORD*)(b+14)==_rv('MTrk');
}

static bool is_midi_scan(const BYTE* b,size_t s)
{
	int x,m=s;
	if (m>256) m=256;
	m-=8+6+8;
	for(x=0;x<m;x++)
		if (is_midi(b+x,s-x)) return 1;
	return 0;
}

#define REM (unsigned int)(sz-ptr)

static bool load_midi_fix(MIDI_file* mf,const BYTE* buf,size_t sz,int n_track,size_t p_ofs)
{
	if (!cfg_recover_tracks) 
		return 0;
	
	size_t malloc_sz;
	if (SizeTAdd(sz, 0x10, &malloc_sz) != S_OK)
		return false;
	
	BYTE* outbuf=(BYTE*)malloc(malloc_sz);
	if (!outbuf) return 0;
	size_t ptr=p_ofs;
	size_t bp=ptr;
	BYTE lc=0;
	while(1)
	{
		bp=ptr;
		if (REM<4) break;
		while(buf[ptr]&0x80)
		{
			if (ptr==bp+4) break;
			ptr++;
		}
		ptr++;
		if (REM<3) break;
		BYTE b=buf[ptr];
		if (b==0xFF)
		{
			ptr+=2;
			if (REM<4) break;
			unsigned int d;
			unsigned int l=DecodeDelta(buf+ptr,&d, sz-ptr);
			if (l+d>REM) break;
			ptr+=l+d;
		}
		else if (b==0xF0)
		{
			ptr++;
			if (REM<4) break;
			unsigned int d;
			unsigned int l=DecodeDelta(buf+ptr,&d, sz-ptr);
			if (l+d>REM) break;
			ptr+=l+d;
		}
		else
		{
			if (b&0x80)
			{
				lc=b&0xF0;
				if (lc==0xF0) break;
				ptr++;
			}
			else if (!lc) break;
			if (lc==0xC0 || lc==0xD0) ptr++;
			else ptr+=2;
		}
	}
	memcpy(outbuf,buf,ptr);
	ptr=bp;
	outbuf[ptr++]=0;
	outbuf[ptr++]=0xFF;
	outbuf[ptr++]=0x2F;
	outbuf[ptr++]=0;
	*(DWORD*)(outbuf+p_ofs-4)=rev32(ptr-p_ofs);
	mf->data=outbuf;
	mf->size=ptr;

	return 1;
}

#undef REM

static bool load_midi(MIDI_file* mf,const BYTE* buf,size_t sz)
{
	int trax=rev16(*(WORD*)(buf+4+4+2));
	size_t ofs=6+8;
	int n;
	for(n=0;n<trax;n++)
	{
		if (ofs>(sz-12) || *(DWORD*)(buf+ofs)!=_rv('MTrk'))
		{
			mf->flags|=FLAG_INCOMPLETE;
			*(WORD*)(buf+4+4+2)=rev16(n);
			sz=ofs;
			break;
		}

		if (SizeTAdd(ofs, 8, &ofs) != S_OK)
			return false;

		size_t p_ofs=ofs;
		DWORD next = rev32(*(DWORD*)(buf+ofs-4));
		if (SizeTAdd(ofs, next, &ofs) != S_OK)
			return false;

		if (ofs>sz)
		{
			mf->flags|=FLAG_INCOMPLETE;
			*(WORD*)(buf+4+4+2)=rev16(n+1);
			if (!load_midi_fix(mf,buf,sz,n,p_ofs))
			{
				*(WORD*)(buf+4+4+2)=rev16(n);
				sz=p_ofs-8;
				break;
			}
			else return 1;
		}
	}

	BYTE * out = (BYTE*)malloc(sz);
	if (!out)
		return 0;
	memcpy(out,buf,sz);
	mf->data=out;
	mf->size=sz;
	return 1;
}

static bool load_gmd(MIDI_file* mf,const BYTE* buf,size_t sz)
{
	if (sz<=0x10) return 0;
	DWORD s=rev32(*(DWORD*)(buf+4));
	if ((sz-8)<s) return 0;
	DWORD ofs=rev32(*(DWORD*)(buf+12))+0x10;
	s-=ofs;
	BYTE * out=(BYTE*)malloc(s);
	if (!out) return 0;
	mf->size=s;
	memcpy(out,buf+ofs,s);
	mf->data=out;
	return 1;
}

#ifdef MF_USE_DMCRAP
void ReleaseObject(IUnknown* o);
#endif

static bool load_rmi(MIDI_file* mf,const BYTE* source,size_t source_size)
{
	if (source_size < 8)
		return 0;

	unsigned int sx=*(DWORD*)(source+4);
	size_t _p=0;
	BYTE * out;
	if (sx>source_size-8) goto _er;
	mf->size=*(DWORD*)(source+16);
	if (mf->size+20>source_size) goto _er;
	out=(BYTE*)malloc(mf->size);
	if (!out) goto _er;
	memcpy(out,source+20,mf->size);
	mf->data = out;

	_p=20+mf->size;
	if (_p&1) _p++;
	while(_p<source_size)
	{
		if (! mf->bmp_data && *(DWORD*)(source+_p)==_rv('DISP') && *(DWORD*)(source+_p+8)==8)//bitmap
		{
			DWORD s=*(DWORD*)(source+_p+4)-4;
			void * r=malloc(s);
			if (r)
			{
				memcpy(r,source+_p+12,s);
				mf->bmp_size=s;
				mf->bmp_data=r;
			}
		}
		else if (! mf->title && *(DWORD*)(source+_p)==_rv('DISP') && *(DWORD*)(source+_p+8)==1)
		{
			DWORD s=*(DWORD*)(source+_p+4)-4;
			char * src=(char*)(source+_p+12);	//remove eol's
			char * dst=mf->title.buffer_get(s+1);
			char * src_b=src;
			while(src && *src && (UINT)(src-src_b)<s)
			{
				if (*src!=10 && *src!=13) *(dst++)=*src;
				src++;
			}
			*dst=0;
			mf->title.buffer_done();
		}
		else if (! mf->rmi_data && *(DWORD*)(source+_p)==_rv('LIST') && *(DWORD*)(source+_p+8)==_rv('INFO'))
		{
			DWORD s=*(DWORD*)(source+_p+4);
			void * r=malloc(s);
			if (r)
			{
				memcpy(r,source+_p+8,s);
				mf->rmi_size=s;
				mf->rmi_data=r;
			}
		}
#ifdef MF_USE_DMCRAP
		else if (!mf->pDLSdata && *(DWORD*)(source+_p)==_rv('RIFF') && *(DWORD*)(source+_p+8)==_rv('DLS '))
		{
			int rs=*(long*)(source+_p+4)+8;
			if (rs+_p>source_size) break;

			mf->DLSsize=rs;
			mf->pDLSdata=(BYTE*)malloc(rs);
			memcpy(mf->pDLSdata,source+_p,rs);
	
		}
#endif
		_p+=*(DWORD*)(source+_p+4)+8;
		if (_p&1) _p++;
	}
	return 1;
_er:
	return 0;
}

bool load_xmi(MIDI_file* mf,const BYTE*,size_t);
bool load_hmp(MIDI_file* mf,const BYTE*,size_t);
bool load_hmi(MIDI_file* mf,const BYTE*,size_t);
bool load_mus(MIDI_file* mf,const BYTE*,size_t);
bool load_cmf(MIDI_file* mf,const BYTE*,size_t);
bool load_mids(MIDI_file* mf,const BYTE*,size_t);
bool load_gmf(MIDI_file* mf,const BYTE*,size_t);
bool is_mus(const BYTE*,size_t);
bool is_cmf(const BYTE*,size_t);
bool is_mids(const BYTE*,size_t);
bool is_gmf(const BYTE*,size_t);

static bool load_midi_scan(MIDI_file* mf,const BYTE* ptr,size_t size)
{
	int max = size-3;
	if (max>256) max=256;
	int x;
	for(x=0;x<256;x++)
	{
		if (*(DWORD*)(ptr+x)==_rv('MThd') && *(DWORD*)(ptr+x+4)==_rv(6))
		{
			size-=x;
			ptr+=x;
			void * buf=malloc(size);
			if (!buf) return 0;
			memcpy(buf,ptr,size);
			bool r=load_midi(mf,(BYTE*)buf,size);
			if (!r) free(buf);
			return r;
		}
	}
	return 0;
}


struct
{
	bool ( * test ) (const BYTE* b,size_t s);
	bool ( * load ) (MIDI_file* mf,const BYTE* ptr,size_t size);
} format_list[] = 
{
	{is_midi,load_midi},
	{is_rmi,load_rmi},
	{is_hmp,load_hmp},
	{is_hmi,load_hmi},
	{is_xmidi,load_xmi},
	{is_mus,load_mus},
	{is_cmf,load_cmf},
	{is_gmd,load_gmd},
	{is_mids,load_mids},
	{is_gmf,load_gmf},
	{is_midi_scan,load_midi_scan}
};

//static fmtfunc fmts[]={is_midi,is_rmi,is_hmp,is_hmi,is_xmidi,is_mus,is_cmf,is_gmd,is_mids,is_gmf,is_midi_scan};
//loadfunc loaders[]={load_midi,load_rmi,load_hmp,load_hmi,load_xmi,load_mus,load_cmf,load_gmd,load_mids,load_gmf,load_midi_scan};

#ifdef MF_USE_DMCRAP
void LoadDLS(MIDI_file* mf)
{
	if (mf->pDLSdata && ! mf->pDLS)
	{
		DMUS_OBJECTDESC ObjDesc;
		ZeroMemory(&ObjDesc,sizeof(ObjDesc));
		ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
		ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
		ObjDesc.llMemLength = mf->DLSsize;
		ObjDesc.pbMemData = mf->pDLSdata;
		ObjDesc.guidClass = CLSID_DirectMusicCollection;
		
		pLoader->GetObject(&ObjDesc,IID_IDirectMusicCollection,(void**)&mf->pDLS);
		if (mf->pDLS)
		{
			ReleaseObject(mf->pDLS);
		}
	}
}

void _LoadSegment(MIDI_file* mf)
{
#ifdef USE_LOG
	log_write("_LoadSegment()");
#endif
	mf->pSeg=0;
	int data_size=0;
	void * data_ptr=0;
	

	if (
		!DoCleanUp(mf,CLEAN_DM|CLEAN_DLS|(cfg_nosysex ? CLEAN_NOSYSEX : 0),&data_ptr,&data_size)
		) 
		return;


	IDirectMusicSegment* pSeg=0;
	DMUS_OBJECTDESC ObjDesc;
	ZeroMemory(&ObjDesc,sizeof(ObjDesc));
	ObjDesc.dwSize=sizeof(ObjDesc);
	ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
	ObjDesc.llMemLength = data_size;
	ObjDesc.pbMemData = (BYTE*)data_ptr;
	ObjDesc.guidClass=CLSID_DirectMusicSegment;
#ifdef USE_LOG
	log_write("pLoader->EnableCache(GUID_DirectMusicAllTypes,1);");
#endif
	pLoader->EnableCache(GUID_DirectMusicAllTypes,1);
//	pLoader->ClearCache(CLSID_DirectMusicSegment);	//%$%&%@!	this->sucks = TRUE
#ifdef USE_LOG
	log_write("pLoader->GetObject(&ObjDesc,IID_IDirectMusicSegment,(void**)&pSeg);");
#endif
	pLoader->GetObject(&ObjDesc,IID_IDirectMusicSegment,(void**)&pSeg);

	if (!pSeg)
	{
#ifdef USE_LOG
		log_write("attempting memdump");
#endif
		char tmpf[MAX_PATH] = {0};
		get_temp_file(tmpf);
		HANDLE f=CreateFileA(tmpf,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
		if (f!=INVALID_HANDLE_VALUE)
		{
			DWORD bw = 0;
			WriteFile(f,data_ptr,data_size,&bw,0);
			CloseHandle(f);
			mbstowcs(ObjDesc.wszFileName,tmpf,MAX_PATH);
			ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FULLPATH | DMUS_OBJ_FILENAME;
			pLoader->GetObject(&ObjDesc,IID_IDirectMusicSegment,(void**)&pSeg);
			DeleteFileA(tmpf);
		}
	}
	if (pSeg)
	{
#ifdef USE_LOG
		log_write("got IDirectMusicSegment");
		log_write("pSeg->SetParam(GUID_StandardMIDIFile,-1,0,0,0);");
#endif
		pSeg->SetParam(GUID_StandardMIDIFile,-1,0,0,0);
#ifdef USE_LOG
		log_write("pSeg->SetStartPoint(0);");
#endif
		pSeg->SetStartPoint(0);
#ifdef USE_LOG
		log_write("pSeg->SetLength();");
#endif
		{
			bool ok=0;
			if (cfg_eof_delay)
			{
				MUSIC_TIME mnt;
				DMUS_TEMPO_PARAM tp;
				if (SUCCEEDED(pSeg->GetParam(GUID_TempoParam,-1,0,mf->tix,&mnt,&tp)))
				{
					pSeg->SetLength((MUSIC_TIME)(mf->tix+(double)cfg_eof_delay*78.0/tp.dblTempo));
					ok=1;
				}
			}
			if (!ok) pSeg->SetLength(mf->tix);
		}

		mf->pSeg=pSeg;

		LoadDLS(mf);

		if (pCDLS) pSeg->SetParam(GUID_ConnectToDLSCollection,0xFFFFFFFF,0,0,(void*)pCDLS);
		if (mf->pDLS) pSeg->SetParam(GUID_ConnectToDLSCollection,0xFFFFFFFF,0,0,(void*)mf->pDLS);

		if (mf->loopstart)
		{
			pSeg->SetLoopPoints(mf->loopstart,mf->loopend);
		}
	}
	free(data_ptr);
	pLoader->EnableCache(GUID_DirectMusicAllTypes,0);
}

IDirectMusicSegment* LoadSegment(MIDI_file* mf)
{
#ifdef USE_LOG
	log_write("LoadSegment()");
#endif
	if (!pLoader) return 0;
	IDirectMusicSegment* pSeg=0;
	if (!mf->pSeg) _LoadSegment(mf);
	if (mf->pSeg)
	{
#ifdef USE_LOG
		log_write("LoadSegment() : got IDirectMusicSegment");
#endif
		pSeg=mf->pSeg;
#ifdef USE_LOG
		log_write("pSeg->AddRef()");
#endif
		pSeg->AddRef();
#ifdef USE_LOG
		log_write("pSeg->AddRef() returned");
#endif
	}
#ifdef USE_LOG
	log_write("LoadSegment() returning");
#endif
	return pSeg;
}
#endif

MIDI_file::~MIDI_file()
{
#ifdef MF_USE_DMCRAP
	if (pSeg) pSeg->Release();
	if (pDLS) pDLS->Release();
	if (pDLSdata) free(pDLSdata);
#endif
	if (data) free((BYTE*)data);
	if (tmap) delete tmap;
	if (smap) delete smap;
	if (info.traxnames) delete[] info.traxnames;
	if (rmi_data) free(rmi_data);
	if (bmp_data) free(bmp_data);
#ifdef HUNT_LEAKS
	n_files--;
#endif
}

bool GetMidiInfo(MIDI_file*);

static bool try_format(const void * data,int size,int idx)
{
	bool rv;
	try	{
		rv = format_list[idx].test((const BYTE*)data,size);
	} catch(...) 
	{
		rv = 0;
	}
	return rv;
}

int MIDI_file::HeaderTest(const void * data,int size)
{
	int n;
	for(n=0;n<tabsize(format_list);n++)
	{
		if (try_format(data,size,n)) return 1;
	}
	return 0;
}

int MIDI_file::Load(const void * data,int size)
{
#ifdef USE_LOG
	log_write("Load()");
#endif
	
	{
		int n;
		int fmt=-1;
		for(n=0;n<tabsize(format_list);n++)
		{
			if (try_format(data,size,n)) {fmt=n;break;}
		}
		if (fmt==-1) return 0;
		format = fmt;
	}



	{
		bool r;
		try {
			r=format_list[format].load(this,(const BYTE*)data,size);
		} catch(...) {
#ifdef USE_LOG
			log_write("midi loader crashed");
#endif
			r=0;
		}
		if (!r) return 0;
	}

	return GetMidiInfo(this);
}

MIDI_file* MIDI_file::Create(const char* fn,const void * data, size_t size)
{
	MIDI_file* mf=new MIDI_file(fn);
	if (!mf->Load(data,size))
	{
		delete mf;
		mf=0;
	}
	return mf;
}