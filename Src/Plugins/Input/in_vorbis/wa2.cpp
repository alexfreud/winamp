#include "main.h"
#include "genres.h"
#include "decoder.h"
#include "api__in_vorbis.h"
#include "../Winamp/wa_ipc.h"
#include "../nu/Singleton.h"
#include "mkv_vorbis_decoder.h"
#include <shlwapi.h>
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include <strsafe.h>
#include <api/service/waservicefactory.h>

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (mod.service)
	{
		waServiceFactory *factory = mod.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (mod.service && api_t)
	{
		waServiceFactory *factory = mod.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

VorbisFile * theFile = 0;

extern CfgInt cfg_abr,cfg_httpseek2;

OSVERSIONINFO os_ver = {0};

static int pos_ms;
static int seek_to=-1;
static int length;
static bool kill;

StringW stat_disp;

void show_stat(const wchar_t* txt)
{
	if (txt)
	{
		stat_disp=txt;
		PostMessage(mod.hMainWindow,WM_USER,0,243);
	}
	else 
		stat_disp=L"";
}

static int is_out_open;
static int paused;
static int volume=255;
static int pan=0;
StringW cur_file;

CRITICAL_SECTION sync;

HANDLE hThread=0;

void Config(HWND);
void About(HWND p);
void do_cfg(int s);
void GetFileInfo(const in_char *file, wchar_t *title, int *len);

const char *INI_FILE=0;
const wchar_t *INI_DIRECTORY=0;
int (*warand)()=0;
float (*warandf)()=0;

api_application *WASABI_API_APP = 0;
// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_memmgr* WASABI_API_MEMMGR = 0;
api_config *AGAVE_API_CONFIG=0;

static MKVDecoderCreator mkvCreator;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoderCreator> mkvFactory;

void SetFileExtensions(void)
{
	static char fileExtensionsString[1200] = {0};	// "OGG\0Ogg files (*.OGG)\0"
	char* end = 0;
	StringCchCopyExA(fileExtensionsString, 1200, "OGG;OGA", &end, 0, 0);
	StringCchCopyExA(end+1, 1200, WASABI_API_LNGSTRING(IDS_OGG_FILES), 0, 0, 0);
	mod.FileExtensions = fileExtensionsString;
}

int Init()
{
	if (!IsWindow(mod.hMainWindow))
		return IN_INIT_FAILURE;

	mod.UsesOutputPlug|=8;

	warand = (int (*)())SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_RANDFUNC);
	warandf = (float (*)())SendMessage(mod.hMainWindow, WM_WA_IPC, 1, IPC_GET_RANDFUNC);

	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	// loader so that we can get the localisation service api for use
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);

	mkvFactory.Register(mod.service, &mkvCreator);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,InVorbisLangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_VORBIS_DECODER),VER);
	mod.description = (char*)szDescription;

	SetFileExtensions();

	INI_FILE = (const char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE);
	INI_DIRECTORY = (const wchar_t *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIDIRECTORYW);

	os_ver.dwOSVersionInfoSize=sizeof(os_ver);
	GetVersionEx(&os_ver);

	InitializeCriticalSection(&sync);
	do_cfg(0);
	return IN_INIT_SUCCESS;
}

void Quit()
{
	winampGetExtendedFileInfoW_Cleanup();
	DeleteCriticalSection(&sync);
	mkvFactory.Deregister(mod.service);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
}

int GetLength()
{
	return length;
}

int IsOurFile(const in_char *fn)
{
	if (PathIsURLW(fn))
	{
		const wchar_t *foo=wcsrchr(fn,L'.');
		return foo ? !_wcsicmp(foo,L".ogg") : 0;
	}
	else return 0;
}

static UINT kbps_disp;

static void out_close()
{
	if (is_out_open)
	{
		mod.outMod->Close();
		mod.SAVSADeInit();
		is_out_open=0;
	}
}

static bool need_full_setinfo;

static int out_open(const Decoder &dec)
{
	int max_l=mod.outMod->Open(dec.sr,dec.nch,dec.bps,-1,-1);
	if (max_l<0) return 0;
	mod.outMod->SetVolume(-666);
	mod.outMod->SetPan(pan);
	mod.SAVSAInit(max_l,dec.sr);
	mod.VSASetInfo(dec.sr,dec.nch);

	is_out_open=1;
	need_full_setinfo=1;
	return 1;
}

void Decoder::wa2_setinfo(UINT cur)
{
	UINT disp=file->get_avg_bitrate();
	if (!cfg_abr)
	{
		disp=cur;
	}
	if ((disp && disp!=kbps_disp) || need_full_setinfo)
	{
		kbps_disp=disp;
		if (need_full_setinfo)
		{
			mod.SetInfo(disp,sr/1000,nch,1);
			need_full_setinfo=0;
		}
		else mod.SetInfo(disp,-1,-1,1);
	}
}

static bool need_movefile;
static void process_movefile();

void alloc_buffers(Decoder & dec,short ** visbuf,char ** sample_buf,int * s_size)
{
	*s_size=576 * (dec.bps>>3) * dec.nch;

	if (*sample_buf) *sample_buf=(char*)realloc(*sample_buf,*s_size*2);
	else *sample_buf=(char*)malloc(*s_size*2);

	if (dec.bps>16)
	{
		int vs=576*2*dec.nch;
		if (*visbuf) *visbuf=(short*)realloc(*visbuf,vs);
		else *visbuf=(short*)malloc(vs);
	}
	else if (*visbuf) {free(*visbuf);*visbuf=0;}
}

static DWORD WINAPI PlayThread(Decoder &dec)
{
	int pos_base=0;
	int samp_wr=0;
	int done=0;
	int upd=0;
	__int64 brate;
	int br_div,br_t;
	short* visbuf=0;
	char *sample_buf=0;
	int retries=0;
	int s_size=0;

	pos_ms=0;

	{
		int r;
		r=dec.play_init();
		if (r)
		{
			if (!kill) Sleep(50);
			if (!kill) Sleep(50);
			if (!kill) Sleep(50);
			if (!kill) Sleep(50);
			if (!kill)
			{
				if (r==2) PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				else PostMessage(mod.hMainWindow,WM_COMMAND,40047,0);
			}
			delete &dec;
			return 0;
		}
		theFile->do_prebuf();
	}

	brate=0;
	br_div=0;
	upd=0;

	alloc_buffers(dec,&visbuf,&sample_buf,&s_size);

	//int f_type=theFile->GetType();
	bool is_live=theFile->IsLive();

	while(!kill)
	{
		if (!theFile) break;//ugh
		if (seek_to!= -1)
		{
			UINT _st=seek_to;
			int r=1;
			seek_to=-1;
			if (theFile)
			{
				theFile->use_prebuf=0;
				int link=theFile->vf.current_link;
				r=dec.Seek((double)_st*0.001);
				if (link!=theFile->vf.current_link) PostMessage(mod.hMainWindow,WM_USER,0,243);
			}
			else r=1;
			if (!r)
			{
				pos_base=pos_ms=_st;
				mod.outMod->Flush(pos_ms);
				samp_wr=0;
				done=0;
				theFile->do_prebuf();
			}
		}

		if (need_movefile && paused)//HACK, prevent stupid lockup
		{
			process_movefile();
			if (!theFile) break;//#@!
			dec.file=theFile;
			dec.Flush();
		}

		if (done)
		{
			//			mod.outMod->CanWrite();
			if (!mod.outMod->IsPlaying())
			{
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				break;
			}
			Sleep(10);
		}
		else if (mod.outMod->CanWrite() >= (s_size<<(mod.dsp_isactive()?1:0)))
		{	
			int l=0;
			while(1)
			{
				if (!dec.need_reopen)
				{
					l+=dec.Read(s_size-l,sample_buf+l);
					if (l>=s_size) break;

					int link=theFile->vf.current_link;
					if (need_movefile)//safe not to flush here
					{
						process_movefile();
						if (!theFile) break;//#@!
						dec.file=theFile;
					}
					if (!dec.DoFrame()) break;
					if (kill) break;

					if (link!=theFile->vf.current_link)
					{
						PostMessage(mod.hMainWindow,WM_USER,0,243);
					}
					br_t=ov_bitrate_instant(&theFile->vf);
					if (br_t>0)
					{
						int i = dec.DataAvailable()/((dec.bps/8)*dec.nch);
						br_div+=i;
						brate+=(__int64)(br_t*i);
					}
					if (need_full_setinfo || (!((++upd)%200) && br_div))
					{
						if (!br_div) {br_div=1;brate=theFile->get_avg_bitrate();}
						dec.wa2_setinfo((int)((__int64)brate/(__int64)br_div/(__int64)1000));
						brate=0;
						br_div=0;
					}
				}
				if (dec.need_reopen)
				{//blargh, new PCM format
					if (l>0) break;//got samples to play, we'll deal with this later
					//l=0;
					while(!kill && mod.outMod->IsPlaying()) Sleep(1);
					if (kill) break;
					out_close();
					if (!out_open(dec))//boo
					{
						PostMessage(mod.hMainWindow,WM_COMMAND,40047,0);
						kill=1;
						break;
					}
					alloc_buffers(dec,&visbuf,&sample_buf,&s_size);
					dec.need_reopen=0;
				}
			}

			if (kill || !theFile) break;

			if (l<=0 && (!is_live || (--retries)<0))
			{
				mod.outMod->Write(sample_buf,0);
				done=1;
			}
			else if (l<=0)
			{
				int r;
				out_close();
				EnterCriticalSection(&sync);
				delete theFile;
				theFile=0;
				LeaveCriticalSection(&sync);
				if (sample_buf)
				{
					free(sample_buf);
					sample_buf=0;
				}

				r=dec.play_init();

				if (r)
				{
					mod.outMod->Write(sample_buf,0);
					done=1;
				}
				else
				{
					theFile->do_prebuf();
				}
			}
			else
			{
				if (l<s_size) memset(sample_buf+l,dec.bps==8 ? 0x80 : 0,s_size-l);
				char * vis=sample_buf;
				UINT vis_bps=dec.bps;
				if (dec.bps>16)
				{
				UINT n;
				UINT d=dec.bps>>3;
				char * foo=sample_buf+d-2;
				for(n=0;n<576*dec.nch;n++)
				{
					visbuf[n]=*(short*)foo;
					foo+=d;
				}
				vis=(char*)visbuf;
				vis_bps=16;
				}
				mod.SAAddPCMData(vis,dec.nch,vis_bps,pos_ms);
				mod.VSAAddPCMData(vis,dec.nch,vis_bps,pos_ms);

				if (mod.dsp_isactive())
				{
				l=(l<<3)/(dec.bps*dec.nch);
				l=mod.dsp_dosamples((short*)sample_buf,l,dec.bps,dec.nch,dec.sr);
				l*=(dec.nch*dec.bps)>>3;
				}
				if (kill) break;
				mod.outMod->Write((char*)sample_buf,l);

				samp_wr+=(8*l)/(dec.bps*dec.nch);
				pos_ms=pos_base+MulDiv(1000,samp_wr,dec.sr);
			}
		}
		else
		{
			theFile->Idle();
		}
	}

	//	out_close();
	//  gay gapless plugins puke, need to call this from stop
	//  ok, hetero (out_wave v2.x / out_ds v1.4+) gapless plugins wouldn't puke anymore

	if (theFile)
	{
		VorbisFile * t=theFile;
		EnterCriticalSection(&sync);
		theFile=0;
		LeaveCriticalSection(&sync);
		delete t;
	}	

	if (sample_buf)
	{
		free(sample_buf);
		sample_buf=0;
	}

	if (need_movefile) process_movefile();

	/*	if (!kill)
	{
	CloseHandle(hThread);
	hThread=0;
	}*/

	if (visbuf) free(visbuf);

	delete &dec;
	return 0;
}

static StringW move_src,move_dst;
static bool mf_ret;

static void do_movefile()
{
	mf_ret=1;
	winampGetExtendedFileInfoW_Cleanup();
	if (!DeleteFileW(move_dst)) mf_ret=0;
	else
	{
		if (!MoveFileW(move_src,move_dst))
		{
			if (!CopyFileW(move_src,move_dst,0)) mf_ret=0;
			DeleteFileW(move_src);
		}
	}
}

static void process_movefile()
{
	if (theFile)
	{
		StringW f_path;
		f_path.AddString(theFile->url);
		double pos=theFile->GetPos();
		EnterCriticalSection(&sync);
		delete theFile;
		theFile=0;

		do_movefile();

		theFile=VorbisFile::Create(f_path,0);
		LeaveCriticalSection(&sync);
		if (theFile)
		{
			theFile->Seek(pos);
		}
	}
	else do_movefile();
	need_movefile=0;
}

bool sync_movefile(const wchar_t * src,const wchar_t * dst)//called from info_.cpp
{
	move_src=src;
	move_dst=dst;
	need_movefile=1;
	if (!theFile) process_movefile();
	else
	{
		while(need_movefile && hThread) Sleep(1);
		if (need_movefile) process_movefile();//shouldnt really happen
		move_src=L"";
		move_dst=L"";
		PostMessage(mod.hMainWindow,WM_USER,0,243);
	}

	return mf_ret;
}


int Decoder::play_init()//still messy
{
	if (play_inited) return 0;

	kbps_disp=0;

	VorbisFile * t=VorbisFile::Create(cur_file,0);
	if (!t)
	{
#ifdef _DEBUG
		OutputDebugString(L"can't open file\n");
#endif
		//		if (scream) MessageBox(mod.hMainWindow,"error opening file",0,MB_ICONERROR);
		return 2;
	}
	Init(t);
	if (!out_open(*this))
	{
#ifdef _DEBUG
		OutputDebugString(L"can't open output\n");
#endif
		delete t;		
		return 1;
	}

	EnterCriticalSection(&sync);
	theFile=t;
	LeaveCriticalSection(&sync);

	wa2_setinfo(theFile->get_avg_bitrate());

	{
		double v=theFile->Length();
		if (v==OV_EINVAL || v<=0) length=-1;
		else length=(int)(v*1000.0);
	}

	play_inited=1;

  return 0;
}

int Play(const in_char *fn)
{
	seek_to=-1;
	kill=0;
	length=0;
	paused=0;

	show_stat(0);

	EnterCriticalSection(&sync);
	cur_file=fn;
	LeaveCriticalSection(&sync);

	Decoder * dec=new Decoder;

	if (!PathIsURLW(fn))
	{
		mod.is_seekable=1;
#if 1
		int rv=dec->play_init();
		if (rv)
		{
		delete dec;
		if (rv==2) return -1;
		return 1;
		}
#endif
	}
	else mod.is_seekable=cfg_httpseek2;

	{
		DWORD id;
		hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)PlayThread,dec,CREATE_SUSPENDED,&id);
	}

	if (hThread)
	{
			SetThreadPriority(hThread, (int)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
		ResumeThread(hThread);
		return 0;
	}
	else
	{
		out_close();
		delete dec;
		return 1;
	}
}

void Pause()
{
	if (!paused)
	{
		mod.outMod->Pause(1);
		paused=1;
	}
}

void UnPause()
{
	if (paused)
	{
		mod.outMod->Pause(0);
		paused=0;
	}
}

int IsPaused()
{
	return paused;
}

void Stop()
{
	if (hThread)
	{
		kill=1;
		EnterCriticalSection(&sync);
		if (theFile) theFile->stopping=1;
		LeaveCriticalSection(&sync);		
		if (WaitForSingleObject(hThread,10000)!=WAIT_OBJECT_0)
		{
			TerminateThread(hThread,0);
			//MessageBox(mod.hMainWindow,"error asking thread to die",0,MB_ICONERROR);
		}
		CloseHandle(hThread);
		hThread=0;
		out_close();
	}
	show_stat(0);
	winampGetExtendedFileInfoW_Cleanup();
}

void EQSet(int on, char data[10], int preamp)
{
}

int GetOutputTime()
{
	return pos_ms+(mod.outMod->GetOutputTime()-mod.outMod->GetWrittenTime()); 
}

void SetOutputTime(int t)
{
	seek_to=t;
	EnterCriticalSection(&sync);
	if (theFile) theFile->abort_prebuf=1;
	LeaveCriticalSection(&sync);
}

void SetVolume(int v)
{
	mod.outMod->SetVolume(volume=v);
}

void SetPan(int p)
{
	mod.outMod->SetPan(pan=p);
}

//int InfoBox(char *file, HWND parent);	//old
int RunInfoDlg(const in_char * url,HWND parent);

In_Module mod=
{
	IN_VER_RET,
    "nullsoft(in_vorbis.dll)",
    0,0,
    0,
    1,
    1,

    Config,
    About,

    Init,
    Quit,

    GetFileInfo,
    RunInfoDlg,

    IsOurFile,
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

extern "C" {
	__declspec( dllexport ) In_Module * winampGetInModule2()
	{
		return &mod;
	}
}

void VorbisFile::Status(const wchar_t * zzz)
{
	if (primary) 
		show_stat(zzz);
}

bool VorbisFile::Aborting()
{
	return stopping || (primary && kill);
}


Info::Info(const wchar_t *filename) : filename(filename), vc(0)
{
	VorbisFile * vf = VorbisFile::Create(filename,true);
	if(!vf)
		return;

	numstreams = vf->vf.links;
	if(numstreams)
	{	
		// now copy the comment section to our own memory...
		stream = vf->vf.current_link; // this is the stream we're editing...
		vc = (vorbis_comment*)calloc(sizeof(vorbis_comment),numstreams);
	  
		for(int i=0; i<numstreams; i++)
		{ // one comment section per stream
			vorbis_comment *c = ov_comment(&vf->vf,i);

			vc[i].comments = c->comments;
			vc[i].user_comments = (char **)malloc(sizeof(char*)*c->comments);
			vc[i].comment_lengths = (int *)malloc(sizeof(int)*c->comments);

			for(int j=0;j<vc[i].comments;j++)
			{ // copy the comments over
				vc[i].user_comments[j] = _strdup(c->user_comments[j]);
				vc[i].comment_lengths[j] = c->comment_lengths[j];
			}
			vc[i].vendor=_strdup(c->vendor);
		}
	}
	delete vf;
}

Info::~Info()
{
	if(vc) {
		for(int i=0; i < numstreams; i++)
			vorbis_comment_clear(&vc[i]);
		free(vc);
	}
}

bool Info::Save()
{
	return !!modify_file(filename,vc,numstreams);
}

int Info::GetNumMetadataItems()
{
	return vc[stream].comments;
}

void Info::EnumMetadata(int n, wchar_t *key, int keylen, wchar_t *val, int vallen)
{
	if(keylen) key[0]=0;
	if(vallen) val[0]=0;
	if(!vc) return;
	if(!vc[stream].user_comments[n]) return;
	AutoWide comment(vc[stream].user_comments[n],CP_UTF8);
	const wchar_t* eq = wcschr((const wchar_t*)comment,L'=');
	if(eq)
	{
		if(keylen) lstrcpynW(key,comment,(int)min(eq - comment + 1,keylen));
		if(vallen) lstrcpynW(val,eq+1,vallen);
	}
	else
	{
		if(keylen) lstrcpynW(key,L"COMMENT",keylen);
		if(vallen) lstrcpynW(val,comment,vallen);
	}
}

void Info::RemoveMetadata(wchar_t * key)
{
	wchar_t k[256] = {0};
	for(int i=0; i<GetNumMetadataItems(); i++)
	{
		EnumMetadata(i,k,256,0,0);
		if(_wcsicmp(k,key)==0)
			RemoveMetadata(i);
	}
}

void Info::RemoveMetadata(int n)
{
	if(!vc) return;
	free(vc[stream].user_comments[n]);
	
	for(int i=n+1; i<vc[stream].comments; i++)
	{
		vc[stream].user_comments[i-1] = vc[stream].user_comments[i];
		if(vc[stream].comment_lengths)
			vc[stream].comment_lengths[i-1] = vc[stream].comment_lengths[i];
	}
	vc[stream].comments--;
	vc[stream].user_comments = (char**)realloc(vc[stream].user_comments,sizeof(vc[stream].user_comments[0]) * vc[stream].comments);
	if(vc[stream].comment_lengths)
		vc[stream].comment_lengths = (int*)realloc(vc[stream].comment_lengths,sizeof(vc[stream].comment_lengths[0]) * vc[stream].comments);
}

void Info::SetMetadata(wchar_t *key, wchar_t *val)
{
	bool set=false;
	wchar_t k[256] = {0};
	for(int i=0; i<GetNumMetadataItems(); i++)
	{
		EnumMetadata(i,k,256,0,0);
		if(_wcsicmp(k,key)==0)
		{
			SetMetadata(i,key,val);
			set=true;
		}
	}
	if(!set)
	{
		int n = vc[stream].comments++;
		vc[stream].user_comments = (char**)realloc(vc[stream].user_comments,sizeof(vc[stream].user_comments[0]) * vc[stream].comments);
		if(vc[stream].comment_lengths)
			vc[stream].comment_lengths = (int*)realloc(vc[stream].comment_lengths,sizeof(vc[stream].comment_lengths[0]) * vc[stream].comments);
		vc[stream].user_comments[n] = NULL;
		SetMetadata(n,key,val);
	}
}

void Info::SetMetadata(int n, wchar_t *key, wchar_t *val)
{
	AutoChar k(key,CP_UTF8);
	AutoChar v(val,CP_UTF8);

	int l = (int)(strlen(k)+strlen(v)+2);
	char * c = (char*)malloc(l);
	StringCchPrintfA(c,l,"%s=%s",(char*)k,(char*)v);
	
	if(vc[stream].user_comments[n])
		free(vc[stream].user_comments[n]);

	vc[stream].user_comments[n] = c;
	if(vc[stream].comment_lengths)
		vc[stream].comment_lengths[n] = l-1;
}

void Info::SetTag(int n,wchar_t *key) // changes the key name
{
	wchar_t val[2048] = {0};
	EnumMetadata(n,NULL,0,val,2048);
	SetMetadata(n,key,val);
}

Info *setMetadata = 0;

extern "C" 
{
	static wchar_t m_lastfn[2048];

	#define START_TAG_ALIAS(name, alias) if (KeywordMatch(data, name)) lookup=alias
	#define TAG_ALIAS(name, alias) else if (KeywordMatch(data, name)) lookup=alias

	__declspec( dllexport ) int winampSetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *val)
	{
		if (!setMetadata || setMetadata && wcscmp(fn,m_lastfn))
		{
			if (setMetadata)
			{
				delete setMetadata;
				setMetadata = 0;
			}

			setMetadata = new Info(fn);
			if(setMetadata->Error())
			{
				delete setMetadata;
				m_lastfn[0] = 0;
				return 0;
			}
			lstrcpynW(m_lastfn,fn, 2048);
		}

		wchar_t *lookup=0;
		START_TAG_ALIAS("artist", L"ARTIST");
		TAG_ALIAS("title", L"TITLE");
		TAG_ALIAS("album", L"ALBUM");
		TAG_ALIAS("genre", L"GENRE");
		TAG_ALIAS("comment", L"COMMENT");
		TAG_ALIAS("year", L"DATE");
		TAG_ALIAS("track", L"TRACKNUMBER");
		TAG_ALIAS("albumartist", L"ALBUM ARTIST");
		TAG_ALIAS("composer", L"COMPOSER");
		TAG_ALIAS("disc", L"DISCNUMBER");
		TAG_ALIAS("publisher", L"PUBLISHER");
		TAG_ALIAS("conductor", L"CONDUCTOR");
		TAG_ALIAS("tool", L"ENCODED-BY");
		TAG_ALIAS("replaygain_track_gain", L"REPLAYGAIN_TRACK_GAIN");
		TAG_ALIAS("replaygain_track_peak", L"REPLAYGAIN_TRACK_PEAK");
		TAG_ALIAS("replaygain_album_gain", L"REPLAYGAIN_ALBUM_GAIN");
		TAG_ALIAS("replaygain_album_peak", L"REPLAYGAIN_ALBUM_PEAK");
		TAG_ALIAS("GracenoteFileID", L"GRACENOTEFILEID");
		TAG_ALIAS("GracenoteExtData", L"GRACENOTEEXTDATA");
		TAG_ALIAS("bpm", L"BPM");
		TAG_ALIAS("remixing", L"REMIXING");
		TAG_ALIAS("subtitle", L"VERSION");
		TAG_ALIAS("isrc", L"ISRC");
		TAG_ALIAS("category", L"CATEGORY");
		TAG_ALIAS("rating", L"RATING");
		TAG_ALIAS("producer", L"PRODUCER");

		if (!lookup)
			return 0;

#if 0
		if (val && *val)
		{
			if(KeywordMatch("rating",data)) 
			{
				wchar_t temp[128] = {0};
				StringCchPrintfW(temp, 128, L"%u", _wtoi(val)*20);
				val=temp;
			}
		}
		AutoChar utf8(val, CP_UTF8);

		for(int i=0;i<m_vc->comments;i++)
		{
			char *c=m_vc[m_curstream].user_comments[i];
			if(!c) continue;
			char *p=strchr(c,'=');
			if (p && *p)
			{
				if(strlen(data) == (p-c) && !_strnicmp(c,data,p-c))
				{
					//found!
					if (val && val[0])
					{
						int added_buf_len = strlen(utf8)+strlen(lookup)+2;
						m_vc[m_curstream].user_comments[i]=(char *)realloc(m_vc[m_curstream].user_comments[i],added_buf_len);
						StringCchPrintfA(m_vc[m_curstream].user_comments[i],added_buf_len,"%s=%s",lookup,(char *)utf8);
						m_vc[m_curstream].comment_lengths[i]=strlen(m_vc[m_curstream].user_comments[i]);
					}
					else
					{
						free(m_vc[m_curstream].user_comments[i]);
						m_vc[m_curstream].user_comments[i]=0;
						m_vc[m_curstream].comment_lengths[i]=0;
					}
					return 1;
				}
			}
		}

		//not found, so create new field
		if (val && val[0])
		{
			int k=m_vc[m_curstream].comments++;
			m_vc[m_curstream].user_comments=(char **)realloc(m_vc[m_curstream].user_comments,sizeof(char*)*m_vc[m_curstream].comments);
			m_vc[m_curstream].comment_lengths=(int *)realloc(m_vc[m_curstream].comment_lengths,sizeof(int)*m_vc[m_curstream].comments);
			int added_buf_len = strlen(utf8)+strlen(lookup)+2;
			m_vc[m_curstream].user_comments[k]=(char *)malloc(added_buf_len);
			StringCchPrintfA(m_vc[m_curstream].user_comments[k],added_buf_len,"%s=%s",lookup,(char *)utf8);
			m_vc[m_curstream].comment_lengths[k]=strlen(m_vc[m_curstream].user_comments[k]);
		}
#endif

		if (val && *val)
		{
			if(KeywordMatch("rating",data)) 
			{
				wchar_t temp[128] = {0};
				StringCchPrintfW(temp, 128, L"%u", _wtoi(val)*20);
				setMetadata->SetMetadata(lookup, temp);
			}
			else
			{
				setMetadata->SetMetadata(lookup, val);
			}
		}
		else
		{
			setMetadata->RemoveMetadata(lookup);
			if(KeywordMatch("comment",data)) 
			{
				// need to remove this one also, or else it's gonna look like delete doesn't work
				// if the file was tagged using this alternate field
				setMetadata->RemoveMetadata(L"DESCRIPTION");
			}
			else if(KeywordMatch("year",data)) 
			{
				// need to remove this one also, or else it's gonna look like delete doesn't work
				// if the file was tagged using this alternate field
				setMetadata->RemoveMetadata(L"YEAR");
			}
			else if(KeywordMatch("track",data)) 
			{
				// need to remove this one also, or else it's gonna look like delete doesn't work
				// if the file was tagged using this alternate field
				setMetadata->RemoveMetadata(L"TRACK");
			}
			else if(KeywordMatch("albumartist",data)) 
			{
				// need to remove these two, also, or else it's gonna look like delete doesn't work
				// if the file was tagged using these alternate fields
				setMetadata->RemoveMetadata(L"ALBUMARTIST");
				setMetadata->RemoveMetadata(L"ENSEMBLE");
			}
			else if(KeywordMatch("publisher",data)) 
			{
				// need to remove this one also, or else it's gonna look like delete doesn't work
				// if the file was tagged using this alternate field
				setMetadata->RemoveMetadata(L"ORGANIZATION");
			}
			else if(KeywordMatch("category",data)) 
			{
				// need to remove these two also, or else it's gonna look like delete doesn't work
				// if the file was tagged using these alternate fields
				setMetadata->RemoveMetadata(L"CONTENTGROUP");
				setMetadata->RemoveMetadata(L"GROUPING");
			}
		}

		return 1;
	}

	__declspec( dllexport ) int winampWriteExtendedFileInfo()
	{
		if(!setMetadata) return 0;

		bool ret = setMetadata->Save();
		delete setMetadata;
		setMetadata = 0;

		// update last modified so we're not re-queried on our own updates
		UpdateFileTimeChanged(m_lastfn);

		return ret;
	}
}