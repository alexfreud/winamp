#include "main.h"
#include "resource.h"
#include <shlwapi.h>
#include <api/service/waServiceFactory.h>
#include "../winamp/wa_ipc.h"
#include "../Agave/language/api_language.h"
#include "CompressionUtility.h"
#include "minizip/unzip.h"

static bool paused;
static int volume=255;
static int pan=0;
static string cur_file;
static HANDLE thread;
static HINSTANCE hRFdll;
static reader_source * pRF;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

#define IPC_GETHTTPGETTER 240

typedef int (*t_getf)(HWND hwnd, char *url, char *file, char *dlgtitle);

static WReader * get_reader(const char * fn);//wa2 hack


static int reader_process_file(WReader * r,const char * fn,void * &out_data, size_t &out_size)
{
	void * data=0;
	int size=0;
	char ks=0;
	if (r->Open((char*)fn,&ks)) 
		return 0;

	size = r->GetLength();
	
	if (size==-1 || size<0x20) 
		return 0;
	
	data=malloc(size);//scan funcs assume that theres at least 256 data
	
	if (!data) 
		return 0;
	
	if (r->Read((char*)data,size,&ks)!=size) 
	{
		free(data);
		return 0;
	}

	void* pvOut;
	size_t nSizeOut = 0;
	// GZIP
	if (((*(DWORD*)data) & 0xFFFFFF) == 0x088b1f)
	{
		if (CompressionUtility::DecompressGZip(data, size, &pvOut, nSizeOut) >= 0)
		{
			out_data = pvOut;
			out_size = nSizeOut;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	// PKZIP
	else if (*(DWORD*)data == 0x04034B50)
	{
		if (CompressionUtility::DecompressPKZip(fn, &pvOut, nSizeOut) >= 0)
		{
			out_data = pvOut;
			out_size = nSizeOut;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	out_size = size;
	out_data = data;
	return 1;
}

MIDI_file * wa2_open_file(const char * url)
{
	WReader * r=get_reader(url);
	if (!r) return 0;
	void * data=0;
	size_t size=0;
	MIDI_file* mf=0;
	if (reader_process_file(r,url,data,size))
	{
		mf=MIDI_file::Create(url,data,size);
		free(data);
	}
	delete r;
	return mf;
}

static void build_fmtstring();

static cfg_int cfg_mod_output("dev_output",1);

static void wa2_onCfgUpdate()
{
	MIDI_device * dev = MIDI_driver::find_device(cfg_driver,cfg_device);
	if (!dev)
	{
		dev = MIDI_driver::find_device_default();
	}

	//hack for wa2input.wac in wa3
	mod.UsesOutputPlug=(dev->has_output() || (cfg_smp && cfg_sampout)) ? 1 : 0x8001;
	cfg_mod_output=mod.UsesOutputPlug;
	build_fmtstring();
}

static char fmts_string[1024];

#define NSEEK(a) {while(!(cfg_ext_mask&(1<<a)) && a<n_exts) a++;}
static void build_fmtstring()
{
	UINT n_exts = MIDI_core::FileTypes_GetNum();
	if (!cfg_ext_mask)
	{
		fmts_string[1]=fmts_string[0]=0;
		return;
	}
	UINT n=0;
	NSEEK(n);
	const char* d=MIDI_core::FileTypes_GetDescription(n);
	char* o=fmts_string;
	while(1)
	{
		UINT f=n;
		while(n<n_exts && d==MIDI_core::FileTypes_GetDescription(n))
		{
			const char * e=MIDI_core::FileTypes_GetExtension(n);
			while(e && *e) *(o++)=*(e++);
			n++;
			NSEEK(n);
			*(o++)=';';
		}
		o[-1]=0;
		while(d && *d) *(o++)=*(d++);
		*(o++)=' ';
		*(o++)='(';
		while(f<n)
		{
			const char * e=MIDI_core::FileTypes_GetExtension(f);
			while(e && *e) *(o++)=*(e++);
			f++;
			NSEEK(f);
			*(o++)=',';			
		}
		o[-1]=')';
		*(o++)=0;
		if (n>=n_exts) break;
		d=MIDI_core::FileTypes_GetDescription(n);
	}
	if (cfg_extra_exts.get_string().length()>0)
	{
		d=cfg_extra_exts;
		while(d && *d) *(o++)=*(d++);
		*(o++)=0;
		d=WASABI_API_LNGSTRING(STRING_FILES_OTHER);
		while(d && *d) *(o++)=*(d++);
		d=cfg_extra_exts;
		while(d && *d)
		{
			if (*d==';') *o=',';
			else *o=*d;
			o++;
			d++;
		}
		*(o++)=')';
		*(o++)=0;
	}
	*(o++)=0;
}
#undef NSEEK


static void Config(HWND p)
{
	if (MIDI_core::Config(p))
	{
		MIDI_core::WriteConfig();
		wa2_onCfgUpdate();
	}
}

void About(HWND);

class CMemReader : public WReader
{
public:
	BYTE* mem;
	UINT sz;
	UINT pos;
	int Open(char *url, char *killswitch);
	int Read(char *buffer, int length, char* killswitch) {if (!mem) return 0;if (length+pos>sz) length=sz-pos;memcpy(buffer,mem+pos,length);pos+=length;return length;}
	int GetLength(void) {return sz;} 
	int CanSeek(void) {return 1;};
	int Seek(int position, char*killswitch) {pos=position;return 0;};
	
	CMemReader() {mem=0;sz=0;pos=0;}
	~CMemReader() {if (mem) free(mem);}

};

static int Download(char* url,UINT* f_size,BYTE** m_buf)
{
	typedef int (*t_getf)(HWND hwnd, char *url, char *file, char *dlgtitle);

	t_getf getf;

	int t=SendMessage(mod.hMainWindow,WM_USER,0,IPC_GETHTTPGETTER);
	if (!t || t==1)
	{
#ifndef WINAMPX
    MessageBoxA(mod.hMainWindow,WASABI_API_LNGSTRING(STRING_URL_ERROR),ERROR,MB_ICONERROR);
#endif
		return 0;
	}
	else
	{
		int rv=0;
		char tmp[MAX_PATH] = {0};
		get_temp_file(tmp);
		HANDLE f;
		DWORD br = 0,s = 0;
		void* b;
		getf=(t_getf)t;
		if (getf(mod.hMainWindow,url,tmp,WASABI_API_LNGSTRING(STRING_RETRIEVING_FILE))) goto fail;
		f=CreateFileA(tmp,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
		if (f==INVALID_HANDLE_VALUE) goto fail;
		br=0;
		s=GetFileSize(f,0);
		if (!s) goto fail;
		b=malloc(s);
		if (!b) goto fail;
		ReadFile(f,b,s,&br,0);
		rv=1;
		*f_size=br;
		*m_buf=(BYTE*)b;
fail:
		CloseHandle(f);
		DeleteFileA(tmp);
		return rv;
	}
}


int CMemReader::Open(char* url,char*)
{
	sz=pos=0;
	if (mem) {free(mem);mem=0;}
	HANDLE f=CreateFileA(url,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);
	if (f!=INVALID_HANDLE_VALUE)
	{
		sz=GetFileSize(f,0);
		mem=(BYTE*)malloc(sz);
		if (!mem) {CloseHandle(f);return 1;}
		ReadFile(f,mem,sz,(DWORD*)&sz,0);
		CloseHandle(f);
		return 0;
	}
	return !Download(url,&sz,&mem);
}

class CFileReader : public WReader
{
  public:
	HANDLE f;
	CFileReader() {f=0;};
	~CFileReader() {if (f) CloseHandle(f);}
	int Open(char *url, char*killswitch) {f=CreateFileA(url,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);return f==INVALID_HANDLE_VALUE;}
	int Read(char *buffer, int length, char*killswitch) {DWORD br=0;ReadFile(f,buffer,length,&br,0);return br;}
	int GetLength(void) {return GetFileSize(f,0);} 
	int CanSeek(void) {return 1;};
	int Seek(int position, char*killswitch) {SetFilePointer(f,position,0,FILE_BEGIN);return 0;}
	
};

static WReader *get_reader(const char* url)
{
	if (!_strnicmp(url,"file://",7)) url+=7;
	WReader* ret=0;
	if (pRF && pRF->ismine((char*)url)) ret=pRF->create();
	if (ret)
	{
		ret->m_player=0;
		return ret;
	}

	if (_strnicmp(url,"http://",7)==0 || _strnicmp(url,"ftp://",6)==0 || _strnicmp(url,"https://",8)==0) return new CMemReader;
	return new CFileReader();
}

int Init()
{
	if (!IsWindow(mod.hMainWindow))
		return IN_INIT_FAILURE;

	// loader so that we can get the localisation service api for use
	waServiceFactory *sf = mod.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,InMidiLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_MIDI_PLAYER),VER);
	mod.description = (char*)szDescription;

	MIDI_core::GlobalInit();
	mod.UsesOutputPlug=cfg_mod_output;
	build_fmtstring();
	return IN_INIT_SUCCESS;
}

void Quit()
{
	MIDI_core::GlobalQuit();
}

void GetFileInfo(const char *url, char *title, int *len)
{
	if (!url || !*url)
	{
		url=cur_file;
	}
	if (len) *len=0;
	if (title) *title=0;

	char ks=0;

	bool file_local=0;
	MIDI_file * file=0;
	if (MIDI_core::getFile() && !_stricmp(url,MIDI_core::getFile()->path))
	{
		file = MIDI_core::getFile()->AddRef();
	}

	if (!file)
	{
		file = wa2_open_file(url);
    if (!file) {
      return;
    }
		file_local=1;
	}

	if (len) 
		*len=file->GetLength();
	if (title) 
		file->GetTitle(title,256);

	file->Free();
}

BOOL CALLBACK InfoProc(HWND,UINT,WPARAM,LPARAM);

int show_rmi_info(HWND w,MIDI_file* mf);

int infoDlg(const char *fn, HWND hwnd)
{
	int rv=1;
	MIDI_file *mf=wa2_open_file(fn);

	if (!mf) return INFOBOX_UNCHANGED;
	
	if (cfg_rmi_def) rv=show_rmi_info(hwnd,mf);
	else
	{
	    rv = WASABI_API_DIALOGBOXPARAM(IDD_INFO,hwnd,InfoProc,(LPARAM)mf);
	}
	if (!rv && !_stricmp(mf->path,cur_file))
	{
		PostMessage(mod.hMainWindow,WM_USER,0,243);
	}
	mf->Free();
	return rv;
}

int InfoBox(const char *file, HWND parent)
{
	if (!file) file=cur_file;
	return infoDlg(file,parent);
}

static char kill;
static int pos_ms;
static int seek_to;
static bool out_open;

DWORD WINAPI PlayThread(void*)
{
#ifdef USE_LOG
	log_write("PlayThread");
#endif
	short * visbuf;

	char *sample_buf;
	int sr,bps,nch;
	pos_ms=0;
	int pos_base=0;
	int samp_wr=0;
	int max_l=0;
	MIDI_core::GetPCM(&sr,&nch,&bps);
	int s_size=576 * (bps/8) * nch;
	if (bps>16)
	{
		visbuf=(short*)malloc(576*2*nch);
	}
	else visbuf=0;

	sample_buf = (char*)malloc(576 * 2 * (bps/8) * nch);

	bool done=0;
	while(!(kill&1))
	{
#ifdef USE_LOG
		log_write("main loop");
#endif
		if (paused) {
#ifdef USE_LOG
			log_write("paused");
#endif
			Sleep(10);
			continue;
		}
		if (seek_to!=-1)
		{
#ifdef USE_LOG
			log_write("seeking");
#endif
			if (MIDI_core::SetPosition(seek_to))
			{
				pos_ms=seek_to;
				if (out_open)
				{
					mod.outMod->Flush(pos_ms);
				}
				pos_base=pos_ms;
				samp_wr=0;
				done=0;
			}
			kill&=~2;
			seek_to=-1;
		}
		if (done)
		{
#ifdef USE_LOG
			log_write("done");
#endif
			if (!mod.outMod->IsPlaying())
			{
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				break;
			}
			Sleep(10);continue;
		}
#ifdef USE_LOG
		log_write("calling GetSamples");
#endif
		int l=MIDI_core::GetSamples(sample_buf,s_size,&kill);
		if (kill&1) {
#ifdef USE_LOG
			log_write("kill&1");
#endif
			break;
		}
		if (kill&2) {
#ifdef USE_LOG
			log_write("kill&2");
#endif
			continue;
		}
		if (l<=0 && !paused)
		{
#ifdef USE_LOG
			log_write("done(?)");
#endif
			done=1;
			if (out_open)
			{
				mod.outMod->Write(sample_buf,0);
				continue;
			}
			else
			{
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				break;
			}
		}
		if (mod.dsp_isactive())
		{
#ifdef USE_LOG
			log_write("DSP");
#endif
			l=(8*l)/(bps*nch);
			l=mod.dsp_dosamples((short*)sample_buf,l,bps,nch,sr);
			l*=(nch*bps)/8;
		}
		if (out_open)
		{
#ifdef USE_LOG
			log_write("sending to output");
#endif
			if (kill&1) break;
			while(mod.outMod->CanWrite()<l && !kill) Sleep(2);
			if (kill&1) break;

			if (!kill) mod.outMod->Write((char*)sample_buf,l);
		}
		{
			char * vis=sample_buf;
			UINT vis_bps=bps;
			if (bps>16)
			{
				int n;
				UINT d=bps>>3;
				char * foo=sample_buf+d-2;
				for(n=0;n<576*nch;n++)
				{
					visbuf[n]=*(short*)foo;
					foo+=d;
				}
				vis=(char*)visbuf;
				vis_bps=16;
			}
#ifdef USE_LOG
			log_write("doing vis");
#endif
			mod.SAAddPCMData(vis,nch,vis_bps,pos_ms);
			mod.VSAAddPCMData(vis,nch,vis_bps,pos_ms);

		}
		samp_wr+=(8*l)/(bps*nch);
		pos_ms=pos_base+MulDiv(1000,samp_wr,sr);
	}

	free(sample_buf);
	if (visbuf) free(visbuf);
	return 0;
}

int initDefaultDeviceShit()
{
  //CT> find default device if no device set
	MIDI_device * dev = MIDI_driver::find_device(cfg_driver,cfg_device);
  if(dev) return 1;

  //reinit to default
  MIDI_driver *driver=MIDI_driver::driver_enumerate(0);
  if(!driver) return 0;
  MIDI_device *device=driver->device_enumerate(0);
  if(!device) return 0;
  cfg_driver=driver->get_guid();
  cfg_device=device->get_guid();
	return 1;
}

int Play(const char *fn)
{
  if(!initDefaultDeviceShit()) return 0;

	paused=0;
	seek_to=-1;
	kill=0;

	if (!MIDI_core::Init()) return 0;

	if (!MIDI_core::UsesOutput())
	{
		MIDI_core::SetVolume(volume);
		MIDI_core::SetPan(pan);
	}
	else
	{
		MIDI_core::SetVolume(255);
		MIDI_core::SetPan(0);
	}

	MIDI_file * file = wa2_open_file(fn);
	if (!file) return -1;

	int rv=MIDI_core::OpenFile(file);
	
	file->Free();

	if (rv==0)
	{
		MIDI_core::Close();
		return 1;
	}
	cur_file=fn;
	int sr,nch,bps;
	MIDI_core::GetPCM(&sr,&nch,&bps);

	{
		MIDI_file * mf=MIDI_core::getFile();
		UINT nc=0;
		if (mf) nc=mf->info.channels;
		mod.SetInfo(nc*10000,sr/1000,2,1);
	}

	if (MIDI_core::HavePCM())
	{
		int max_l=0;
		MIDI_core::GetPCM(&sr,&nch,&bps);
		if (MIDI_core::UsesOutput())
		{
#ifdef USE_LOG
			log_write("output init");
#endif
			max_l=mod.outMod->Open(sr,nch,bps,-1,-1);
			if (max_l<0)
			{
				MIDI_core::Close();
				return 1;
			}
			out_open=1;
			mod.outMod->SetVolume(volume);
			mod.outMod->SetPan(pan);
		}
		mod.SAVSAInit(max_l,sr);
		mod.VSASetInfo(sr,nch);
#ifdef USE_LOG
		log_write("Creating thread");
#endif
		DWORD id;
		thread=CreateThread(0,0,PlayThread,0,CREATE_SUSPENDED,&id);
#ifndef _DEBUG
		SetThreadPriority(thread,THREAD_PRIORITY_TIME_CRITICAL);
#endif
		ResumeThread(thread);

	}
	else
	{
#ifdef USE_LOG
		log_write("threadless mode");
#endif
		thread=0;
	}
	
	return 0;
}

void Pause()
{
	if (MIDI_core::HavePlayer() && !paused)
	{
		MIDI_core::Pause(paused=1);
		if (MIDI_core::UsesOutput()) mod.outMod->Pause(1);
	}
}

void UnPause()
{
	if (MIDI_core::HavePlayer() && paused)
	{
		MIDI_core::Pause(paused=0);
		if (MIDI_core::UsesOutput())
		{
			mod.outMod->Flush(0);
			mod.outMod->Pause(0);
		}
	}
}

int IsPaused()
{
	return paused;
}

void Stop()
{
	if (thread)
	{
		kill|=1;
		WaitForSingleObject(thread,INFINITE);
		CloseHandle(thread);
		thread=0;
		mod.SAVSADeInit();

		if (out_open)
		{
			out_open=0;
			mod.outMod->Close();
		}
	}

	MIDI_core::Close();
}

void EQSet(int on, char data[10], int preamp)
{
}

int GetLength()
{
	return MIDI_core::GetLength();
}

int GetOutputTime()
{
	if (seek_to!=-1) return seek_to;

	if (thread && MIDI_core::UsesOutput()) return pos_ms+mod.outMod->GetOutputTime()-mod.outMod->GetWrittenTime();
	else return MIDI_core::GetPosition();
}

void SetOutputTime(int t)
{
	if (thread)
	{
		seek_to=t;
		kill|=2;
	}
	else MIDI_core::SetPosition(t);
}

void SetVolume(int v)
{
	volume=v;
	if (MIDI_core::UsesOutput()) mod.outMod->SetVolume(v);
	else MIDI_core::SetVolume(v);
}

void SetPan(int p)
{
	pan=p;
	if (MIDI_core::UsesOutput()) mod.outMod->SetPan(p);
	else MIDI_core::SetPan(p);
}

In_Module mod=
{
	IN_VER_RET,
	"nullsoft(in_midi.dll)",
	0,0,
	
	fmts_string,
	
	1,
	1,

	Config,
	About,

	Init,
	Quit,

	GetFileInfo,
	InfoBox,
	
	MIDI_core::IsOurFile,
	Play,
	Pause,
	UnPause,
	IsPaused,
	Stop,

	GetLength,
	GetOutputTime,
	SetOutputTime,

	SetVolume,
	SetPan,
	
	0,0,0,0,0,0,0,0,0,0,0,
	EQSet,

	0,
	0,
};

extern "C"
{
	__declspec( dllexport ) In_Module * winampGetInModule2()
	{
		return &mod;
	}
}

void MIDI_callback::NotifyEOF() {PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);}
HWND MIDI_callback::GetMainWindow() {return mod.hMainWindow;}
HINSTANCE MIDI_callback::GetInstance() {return mod.hDllInstance;}

void MIDI_callback::Error(const char * tx)
{
#ifndef WINAMPX
  MessageBoxA(mod.hMainWindow,tx,0,MB_ICONERROR);
#endif
}

BOOL APIENTRY DllMain(HANDLE hMod,DWORD r,void*)
{
    if (r==DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls((HMODULE)hMod);
	}
	return 1;
	
}

void MIDI_callback::Idle(int ms)
{
	int start = timeGetTime();
	do Sleep(1); while( (int)timeGetTime() - start < ms);
}