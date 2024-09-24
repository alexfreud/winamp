#include "main.h"
#include <math.h>
#include "seq.h"
#include "fakedsound.h"
#include "resource.h"

// {B84EB58A-29F5-410b-880A-EB473BF34291}
static const GUID guid_output = 
{ 0xb84eb58a, 0x29f5, 0x410b, { 0x88, 0xa, 0xeb, 0x47, 0x3b, 0xf3, 0x42, 0x91 } };

// {DF0800B6-D1E1-4b53-9C1E-642AF4CB7136}
static const GUID dmusic_driver_guid = 
{ 0xdf0800b6, 0xd1e1, 0x4b53, { 0x9c, 0x1e, 0x64, 0x2a, 0xf4, 0xcb, 0x71, 0x36 } };

enum
{
	MDD_OUT=1,
};


extern cfg_int cfg_dls_active,cfg_dm_keep_port;
extern cfg_string cfg_dls_file;

class MIDI_device_dmusic : public MIDI_device
{
private:
	GUID guid,guid_dmusic;
	DWORD dmFlags;
	bool f_has_output;

	virtual player_base * create();
	virtual bool is_default() {return 0;}
	virtual bool has_freq() {return !!(dmFlags&DMUS_PC_DIRECTSOUND);}
	virtual bool volctrl_happy() {return (dmFlags&DMUS_PC_DIRECTSOUND) || (dmFlags&DMUS_PC_SOFTWARESYNTH);}
public:
	MIDI_device_dmusic(GUID p_guid,bool p_has_output,DWORD p_dmFlags,const wchar_t * p_name,const wchar_t * p_info)
	{
		guid = p_guid;
		guid_dmusic = p_guid;
		dmFlags = p_dmFlags;
		set_name(p_name);
		set_info(p_info);
		f_has_output = p_has_output;
		if (f_has_output)
		{
			const BYTE * src = (const BYTE*) &guid_output;
			BYTE * dst = (BYTE*) &guid;
			int n;
			for(n=0;n<sizeof(GUID);n++) dst[n]^=src[n];
		}
	
	}
	virtual GUID get_guid() {return guid;}
	GUID get_dm_guid() {return guid_dmusic;}
	virtual bool has_output() {return f_has_output;}
	virtual bool has_dls() {return !!(dmFlags&DMUS_PC_DLS);}
};

//bool IsDrumBankOK(BYTE n);

IDirectMusicLoader* pLoader=0;
IDirectMusicPerformance* pPerf=0;
static IDirectMusicCollection *pGM=0;
static IDirectMusic* pDM;
static IDirectMusicPort *pPort;
static IDirectSoundBuffer* pHack;
IDirectMusicCollection *pCDLS=0;


static void SendMsg(IDirectMusicPerformance *pPerf,DWORD msg)
{
    DMUS_MIDI_PMSG     *pMSG;
    if(SUCCEEDED(pPerf->AllocPMsg(sizeof(DMUS_MIDI_PMSG),(DMUS_PMSG**)&pMSG)))
    {
		ZeroMemory(pMSG, sizeof(DMUS_MIDI_PMSG));
		pMSG->dwSize = sizeof(DMUS_MIDI_PMSG);
		pMSG->dwPChannel = msg&0xF;
		pMSG->dwVirtualTrackID = 0;
		pMSG->dwType = DMUS_PMSGT_MIDI;
		pMSG->dwVoiceID = 0;
		pMSG->dwGroupID = 0xFFFFFFFF;
		pMSG->mtTime=0;
		pMSG->dwFlags = DMUS_PMSGF_REFTIME|DMUS_PMSGF_TOOL_IMMEDIATE;//pMSG->dwFlags = DMUS_PMSGF_REFTIME|DMUS_PMSGF_TOOL_IMMEDIATE;
		pMSG->bStatus=(BYTE)(msg&0xFF);
		pMSG->bByte1=(BYTE)((msg>>8)&0xFF);
		pMSG->bByte2=(BYTE)((msg>>16)&0xFF);
		if (FAILED(pPerf->SendPMsg((DMUS_PMSG*)pMSG)))
		{
			pPerf->FreePMsg((DMUS_PMSG*)pMSG);
		}
    }
}

static void SendSysex(IDirectMusicPerformance *pPerf,BYTE* data,UINT len)
{
    DMUS_SYSEX_PMSG     *pMSG;
    if(SUCCEEDED(pPerf->AllocPMsg(sizeof(DMUS_SYSEX_PMSG) + len,(DMUS_PMSG**)&pMSG)))
    {
		ZeroMemory(pMSG, sizeof(DMUS_SYSEX_PMSG)+len);
		pMSG->dwSize = sizeof(DMUS_SYSEX_PMSG);
		pMSG->dwPChannel = 0;
		pMSG->dwVirtualTrackID = 0;
		pMSG->dwType = DMUS_PMSGT_SYSEX;
		pMSG->dwVoiceID = 0;
		pMSG->dwGroupID = 0xFFFFFFFF;
		pMSG->dwLen = len;
		memcpy(pMSG->abData, (void*)data, len);
		pMSG->mtTime=0;
		pMSG->dwFlags = DMUS_PMSGF_REFTIME|DMUS_PMSGF_TOOL_IMMEDIATE;//pMSG->dwFlags = |DMUS_PMSGF_TOOL_IMMEDIATE;
		if (FAILED(pPerf->SendPMsg((DMUS_PMSG*)pMSG)))
		{
			pPerf->FreePMsg((DMUS_PMSG*)pMSG);
		}
    }
}

static void PortKill()
{
#ifdef USE_LOG
	log_write("portkill()");
#endif
	if (pPort)
	{
		pPort->Activate(0);
		if (pPerf) pPerf->RemovePort(pPort);
		pPort->Release();
		pPort=0;
	}
}

static int PortInit(MIDI_device_dmusic * dev)
{
#ifdef USE_LOG
	log_write("portinit()");
#endif
static int _act_freq;
static int _cfg_reverb,_cfg_chorus;
static GUID last_port;

	if (!pPort || last_port!=dev->get_guid() || _act_freq!=cfg_freq || _cfg_reverb!=cfg_reverb || _cfg_chorus!=cfg_chorus)
	{
#ifdef USE_LOG
		log_write("port settings changed");
#endif		
		if (pPort) PortKill();

		DMUS_PORTPARAMS dmpp;
		ZeroMemory(&dmpp,sizeof(dmpp));
		dmpp.dwSize=sizeof(dmpp);
		dmpp.dwValidParams=DMUS_PORTPARAMS_EFFECTS|DMUS_PORTPARAMS_SAMPLERATE|DMUS_PORTPARAMS_CHANNELGROUPS;
		dmpp.dwChannelGroups=1;
		dmpp.dwSampleRate=cfg_freq;
		if (cfg_reverb) dmpp.dwEffectFlags=DMUS_EFFECT_REVERB;
		if (cfg_chorus) dmpp.dwEffectFlags|=DMUS_EFFECT_CHORUS;
		if (FAILED( pDM->CreatePort(dev->get_dm_guid(),&dmpp,&pPort,0) )) return 0;
		
		pPerf->AddPort(pPort);
		pPerf->AssignPChannelBlock(0,pPort,1);
		last_port = dev->get_guid();

		_act_freq=cfg_freq;
		_cfg_reverb=cfg_reverb;
		_cfg_chorus=cfg_chorus;
	}
	if ((dev->has_output()))
	{
#ifdef USE_LOG
		log_write("initializing output hack");
#endif
		DWORD buf_s=0,blah=0;
		pPort->GetFormat(0,&blah,&buf_s);
		pHack=dhb_create(buf_s,cfg_freq);
		if (FAILED(pPort->SetDirectSound(get_ds(),pHack)))
		{//BORK
			PortKill();
			return 0;
		}
	}
	return 1;
}

/*
	int lastvol1=(vol==0)?0x80000000:((int)(2000.0*log((double)vol/256.0)));
	if (pPerf)
	{
		return SUCCEEDED(pPerf->SetGlobalParam(GUID_PerfMasterVolume,&lastvol1,4));
	}
*/

static int DM_setvol(int vol)
{
	int lastvol1=(vol==0)?0x80000000:((int)(2000.0*log10((double)vol/255.0)));
	if (pPerf)
	{
		return SUCCEEDED(pPerf->SetGlobalParam(GUID_PerfMasterVolume,&lastvol1,4));
	}

	return 0;
}

class player_dmusic_imm : public seq_base
{
private:
	MIDI_device_dmusic * dev;
	UINT n_ins,s_ins;
	IDirectMusicDownloadedInstrument ** ins;
	IDirectMusicCollection *edls;

protected:
	virtual void seq_shortmsg(DWORD msg) {SendMsg(pPerf,msg);}
	virtual void seq_sysex(BYTE* d,UINT l) {SendSysex(pPerf,d,l);}
	virtual void eof() {MIDI_core::Eof();}
	int setvol(int t) {return DM_setvol(t);}
public:
	player_dmusic_imm(MIDI_device_dmusic * p_dev)
	{
		dev=p_dev;
		s_ins=n_ins=0;
		ins=0;
		edls=0;		
		if (dev->has_dls())
		{
			s_ins=0x100;
			ins=(IDirectMusicDownloadedInstrument**)malloc(s_ins*sizeof(void*));
		}
		
	}
	~player_dmusic_imm();
	int play();

};


int player_dmusic_imm::play()
{
	if (!PortInit(dev)) return 0;

	MIDI_file * mf=MIDI_core::getFile();
	if (ins)
	{

		if (mf->pDLSdata)
		{
			LoadDLS(mf);
			if (mf->pDLS)
			{
				edls=mf->pDLS;
			}
		}
		
		if (!edls) edls=pCDLS;

		{
			INSTRUMENT_DESC* instr=GetInstruments(mf,1);

			while(instr)
			{
				DWORD i=instr->patch | (instr->bank_lo<<8) | (instr->bank_hi<<16);
				if (instr->drum) i|=0x80000000;
				if (n_ins>=s_ins)
				{
					s_ins<<=1;
					void *t=realloc(ins,s_ins);
//					if (!t) {s_ins>>=1;return ;}
					ins=(IDirectMusicDownloadedInstrument**)t;
				}
				IDirectMusicInstrument * pi=0;
start:
				if (edls)
				{
					edls->GetInstrument(i,&pi);
				}
				if (!pi && pGM)
				{
					pGM->GetInstrument(i,&pi);
				}
				if (!pi)	//cleaner's hacks don't work here
				{
					if (i&0x80000000)
					{
						if (i&0xFFFF00) {i&=0x800000FF;goto start;}
					}
					else
					{
						if (i&0xFF00) {i&=0xFF00FF;goto start;}
						if (i&0xFF0000) {i&=0xFF;goto start;}
					}
				}
#if 0
				if (!pi)
				{
					char tmp[128] = {0};
					if (i&0x80000000)
						wsprintf(tmp,"missing drum kit: %u",i&0xFF);
					else
						wsprintf(tmp,"missing instrument: bank %x:%x / patch %x",(i>>16)&0xFF,(i>>8)&0xFF,i&0xFF);
					Warning(tmp);
				}
#endif
				if (pi)
				{
//					DMUS_NOTERANGE nr = {instr->note_min,instr->note_max};
//					pPort->DownloadInstrument(pi,&ins[n_ins++],&nr,1);
					pPort->DownloadInstrument(pi,&ins[n_ins++],0,0);
					pi->Release();
				}

				{
					INSTRUMENT_DESC * d=instr->next;
					delete instr;
					instr=d;
				}
			}
		}

	}
/*	UINT n;
	for(n=0;n<16;n++)
	{
		pPort->SetChannelPriority(0,n,DAUD_CRITICAL_VOICE_PRIORITY);
	}*/
	pPort->Activate(1);

	DM_setvol(MIDI_core::player_getVol());

	return seq_cmd_start(CLEAN_DM);
}

player_dmusic_imm::~player_dmusic_imm()
{
	seq_cmd_stop();
	if (ins)
	{
		if (pPort)
		{
			pPort->Activate(0);
			UINT n;
			for(n=0;n<n_ins;n++)
			{
				if (ins[n])
				{
					pPort->UnloadInstrument(ins[n]);
					ins[n]=0;
				}
			}
		}
		free(ins);
	}

	if (pHack)
	{
		pPort->SetDirectSound(0,0);
		pHack->Release();
		pHack=0;
	}
	if (!cfg_dm_keep_port) PortKill();
}


static void CALLBACK TimerProc(HWND,UINT,UINT id,DWORD)
{
	DMUS_NOTIFICATION_PMSG* pMsg;
	while(pPerf->GetNotificationPMsg(&pMsg)==S_OK)
	{
		if (IsEqualGUID(pMsg->guidNotificationType,GUID_NOTIFICATION_SEGMENT))
		{
			if (MIDI_core::HavePlayer() && pMsg->dwNotificationOption == DMUS_NOTIFICATION_SEGEND)
			{
				MIDI_core::Eof();
			}
		}
		pPerf->FreePMsg((DMUS_PMSG*)pMsg);
	}
}

class player_dmusic : public player_base
{
public:
	~player_dmusic();
	int gettime();
	int settime(int);
	int setvol(int vol) {return DM_setvol(vol);}
	void pause();
	void unpause();
	int play();
	player_dmusic(MIDI_device_dmusic * p_dev)
	{
		dev = p_dev;
		pSeg=0;
		pSS=0;
		rtStart=rtOffset=0;
		mtStart=mtOffset=0;
	}

private:
	MIDI_device_dmusic * dev;
	IDirectMusicSegment* pSeg;
	IDirectMusicSegmentState* pSS;

	REFERENCE_TIME rtStart,rtOffset;
	MUSIC_TIME mtOffset,mtStart;
	bool dloaded,paused;
	UINT timer_id;
};

player_base * MIDI_device_dmusic::create()
{
#ifdef USE_LOG
	log_write("DM_create");
#endif

	CoInitialize(0);
	if (!pLoader)
	{
		try {
			
			CoCreateInstance(CLSID_DirectMusicLoader,0,CLSCTX_INPROC,IID_IDirectMusicLoader,(void**)&pLoader);
			if (!pLoader) return 0;
			pLoader->EnableCache(GUID_DirectMusicAllTypes,0);
		} catch(...) {
			return 0;
		}
	}

	if (!pPerf)
	{
		try {
			CoCreateInstance(CLSID_DirectMusicPerformance,0,CLSCTX_INPROC,IID_IDirectMusicPerformance,(void**)&pPerf);
			if (!pPerf) return 0;
			pPerf->Init(&pDM,0,0);
			pPerf->AddNotificationType(GUID_NOTIFICATION_SEGMENT);
		} catch(...) {
			return 0;
		}
	}

	if (!pGM)
	{
		DMUS_OBJECTDESC desc;
		ZeroMemory(&desc,sizeof(desc));
		desc.dwSize=sizeof(desc);
		desc.dwValidData=DMUS_OBJ_OBJECT|DMUS_OBJ_CLASS;
		desc.guidObject=GUID_DefaultGMCollection;
		desc.guidClass=CLSID_DirectMusicCollection;
		try {
			pLoader->GetObject(&desc,IID_IDirectMusicCollection,(void**)&pGM);
		} catch(...) {
			return 0;
		}
	}
		

	if (has_dls())
	{
		static string current_dls;
		if (!cfg_dls_active)
		{
			if (pCDLS) {pCDLS->Release();pCDLS=0;}
		}
		else
		{
			if (pCDLS && _stricmp(current_dls,cfg_dls_file)) {pCDLS->Release();pCDLS=0;}
			if (!pCDLS)
			{
				DMUS_OBJECTDESC desc;
				ZeroMemory(&desc,sizeof(desc));
				desc.dwSize=sizeof(desc);
				desc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
				desc.guidClass = CLSID_DirectMusicCollection;
				mbstowcs(desc.wszFileName,cfg_dls_file,DMUS_MAX_FILENAME);
				if (FAILED(pLoader->GetObject(&desc,IID_IDirectMusicCollection,(void**)&pCDLS)))
				{
//					ErrorBox(Can't load DLS file.);
					pCDLS=0;
					cfg_dls_active=0;
				}
				else
				{
					ReleaseObject(pCDLS);
					current_dls = cfg_dls_file;
				}
			}
		}
	}

	if (cfg_playback_mode)
	{
		player_dmusic_imm * p=new player_dmusic_imm(this);
		if (p)
		{
			if (!p->play())
			{
				delete p;
				p=0;
			}
		}
		return p;
	}
	else
	{
		player_dmusic* p=new player_dmusic(this);
		if (p)
		{
			if (!p->play()) {delete p;p=0;}
		}
		return p;
	}
}

MUSIC_TIME GetMTforMS(IDirectMusicSegment* pS,DWORD ms)
{
	MUSIC_TIME mtSeg,mct=0,mnt;
	DMUS_TEMPO_PARAM tp;
	pS->GetLength(&mtSeg);
	double _r=0,r1;
	while(mct<mtSeg)
	{
		if (FAILED(pS->GetParam(GUID_TempoParam,-1,0,mct,&mnt,&tp))) break;
		if (!mnt) mnt=mtSeg-mct;
		r1=_r;
		_r+=(mnt)/tp.dblTempo*78;
		if (_r>ms)
		{
			return (MUSIC_TIME)(mct+mnt*((double)ms-r1)/(_r-r1));
		}
		mct+=mnt;
	}
	return mtSeg;
}



DWORD GetSegLen(IDirectMusicSegment* pS)
{
	MUSIC_TIME mtSeg,mct=0,mnt;
	DMUS_TEMPO_PARAM tp;
	pS->GetLength(&mtSeg);
	double _r=0;
	while(mct<mtSeg)
	{
		pS->GetParam(GUID_TempoParam,-1,0,mct,&mnt,&tp);
		if (!mnt) mnt=mtSeg-mct;
		_r+=(double)mnt/tp.dblTempo*78;
		mct+=mnt;
	}
	return (DWORD)_r;
}

void ReleaseObject(IUnknown* o)
{
	IDirectMusicObject* pObject=0;
	if (pLoader && SUCCEEDED(o->QueryInterface(IID_IDirectMusicObject,(void**)&pObject)))
	{
		pLoader->ReleaseObject(pObject);
		pObject->Release();
	}
}

player_dmusic::~player_dmusic()
{
	pPerf->Stop(0,0,0,0);

	if (pPort) pPort->Activate(0);
//	pPerf->Invalidate(0,0);

	mtOffset=0;
	rtOffset=0;
	mtStart=0;
	rtStart=0;
	if (pSS) {pSS->Release();pSS=0;}

	if (pSeg)
	{
		if (dloaded) pSeg->SetParam(GUID_Unload,0xFFFFFFFF,0,0,(void*)pPerf);
		pSeg->Release();
		pSeg=0;
	}

	if (pHack)
	{
		pPort->SetDirectSound(0,0);
		pHack->Release();
		pHack=0;
	} 
	if (!cfg_dm_keep_port) PortKill();
	if (timer_id) KillTimer(0,timer_id);
}


void player_dmusic::pause()
{
	MUSIC_TIME mt;
	REFERENCE_TIME rt;
	pPerf->Stop(0,0,0,0);
	if (pSS)
	{
		pSS->Release();
		pSS=0;
	}
	pPerf->GetTime(&rt,&mt);
	mtOffset+=mt-mtStart;
	rtOffset+=rt-rtStart;
	pSeg->SetStartPoint(mtOffset);
	paused=1;
}

void player_dmusic::unpause()
{
	if (pSS)
	{
		pSS->Release();
		pSS=0;
	}
	if (SUCCEEDED(pPerf->PlaySegment(pSeg,0,0,&pSS)))
	{
		pSS->GetStartTime(&mtStart);
		pPerf->MusicToReferenceTime(mtStart,&rtStart);
	}
	paused=0;
}

int player_dmusic::gettime()
{
	static DWORD tm;
	if (pSS)
	{
		REFERENCE_TIME rt;
		pPerf->GetTime(&rt,0);
		tm=(int)((rt-rtStart+rtOffset)/10000);
	}
	return tm;
}

int player_dmusic::play()
{
#ifdef USE_LOG
	log_write("player_dmusic::play()");
#endif
	if (!PortInit(dev)) return 0;
	pSeg=LoadSegment(MIDI_core::getFile());
	if (!pSeg)
	{
#ifdef USE_LOG
		log_write("LoadSegment() failed");
#endif
//		Error("Unable to get IDirectMusicSegment.");
		return 0;
	}
#ifdef USE_LOG
	log_write("player_dmusic::play() : got IDirectMusicSegment");
#endif
	pSeg->SetRepeats( (cfg_loop_type==2 || (cfg_loop_type==1 && MIDI_core::getFile()->loopstart)) ? (cfg_loop_infinite ? -1 : cfg_loop_count-1) : 0);
	
	dloaded=0;
	if (dev->has_dls())
		if (SUCCEEDED(pSeg->SetParam(GUID_Download,-1,0,0,(void*)pPerf)))
			dloaded=1;

	pSeg->SetStartPoint(0); 
#ifdef USE_LOG
	log_write("Activating port...");
#endif
	pPort->Activate(1);
#ifdef USE_LOG
	log_write("IDirectMusicPort::Activate() returned");
#endif
	sysex_startup((SYSEXFUNC)SendSysex,pPerf);
#ifdef USE_LOG
	log_write("IDirectMusicPerformance::PlaySegment()");
#endif
	pSS=0;
	
	DM_setvol(MIDI_core::player_getVol());

	if (FAILED(pPerf->PlaySegment(pSeg,DMUS_SEGF_DEFAULT,0,&pSS)))
	{
//		Error("IDirectMusicPerformance::PlaySegment() failed.");
		return 0;
	}
#ifdef USE_LOG
	log_write("IDirectMusicPerformance::PlaySegment() returned OK");
#endif
	
	rtOffset=0;
	if (pSS)
	{
		pSS->GetStartTime(&mtStart);
	}
	else
	{
#ifdef USE_LOG
		log_write("no segment starte. WTF ?");
#endif
		mtStart=0;
	}

	pPerf->MusicToReferenceTime(mtStart,&rtStart);
	
	timer_id=SetTimer(0,0,33,(TIMERPROC)TimerProc);

	return 1;
}


static struct
{
	int name;
	UINT flag;
} DMCAPZ[]=
{
/*
#define DMUS_PC_DLS              (0x00000001)   // Supports DLS downloading and DLS level 1.
#define DMUS_PC_EXTERNAL         (0x00000002)   // External MIDI module.
#define DMUS_PC_SOFTWARESYNTH    (0x00000004)   // Software synthesizer.
#define DMUS_PC_MEMORYSIZEFIXED  (0x00000008)   // Memory size is fixed.
#define DMUS_PC_GMINHARDWARE     (0x00000010)   // GM sound set is built in, no need to download.
#define DMUS_PC_GSINHARDWARE     (0x00000020)   // GS sound set is built in.
#define DMUS_PC_XGINHARDWARE     (0x00000040)   // XG sound set is built in.
#define DMUS_PC_DIRECTSOUND      (0x00000080)   // Connects to DirectSound via a DSound buffer.
#define DMUS_PC_SHAREABLE        (0x00000100)   // Synth can be actively shared by multiple apps at once.
#define DMUS_PC_DLS2             (0x00000200)   // Supports DLS2 instruments.
#define DMUS_PC_AUDIOPATH        (0x00000400)   // Multiple outputs can be connected to DirectSound for audiopaths.
#define DMUS_PC_WAVE             (0x00000800)   // Supports streaming and one shot waves.
*/

	{STRING_DMCAPS_DLS1,DMUS_PC_DLS},
	{STRING_DMCAPS_DLS2,DMUS_PC_DLS2},
	{STRING_DMCAPS_SOFTSYNTH,DMUS_PC_SOFTWARESYNTH},
	{STRING_DMCAPS_GM,DMUS_PC_GMINHARDWARE},
	{STRING_DMCAPS_GS,DMUS_PC_GSINHARDWARE},
	{STRING_DMCAPS_XG,DMUS_PC_XGINHARDWARE},
//	{STRING_DMCAPS_DSOUND,DMUS_PC_DIRECTSOUND},
	{STRING_DMCAPS_SHARE,DMUS_PC_SHAREABLE},
};

#define N_DMCAPZ (sizeof(DMCAPZ)/sizeof(DMCAPZ[0]))

static struct
{
	int name;
	UINT flag;
} DMCAPZ1[]=	//effects
{
	{STRING_REVERB,DMUS_EFFECT_REVERB},
	{STRING_CHORUS,DMUS_EFFECT_CHORUS},
};

#define N_DMCAPZ1 (sizeof(DMCAPZ1)/sizeof(DMCAPZ1[0]))

int player_dmusic::settime(int tm)
{
	int rv;
#ifdef USE_LOG
	log_write("player_dmusic::settime");
#endif
	rtOffset=UInt32x32To64(tm,10000);
#ifdef USE_LOG
	log_write("calling IDirectMusicPerformance::Stop()");
#endif

	if (!paused)
	{
#ifdef USE_LOG
		log_write("IDirectMusicPerformance::Stop() returned");
#endif
		if (pSS) {pSS->Release();pSS=0;}
	}

	// not ideal but a pause, seek and unpause seems to resolve a failed seek issue
	// with the 'Direct Music / Microsoft Synthesizer' and 'streamed' output mode
	pause();

	MUSIC_TIME time=GetMTforMS(pSeg,tm);
	rv = SUCCEEDED( pSeg->SetStartPoint(time) );
	if (rv) mtOffset=time;

	unpause();

	if (!paused)
	{
#ifdef USE_LOG
		log_write("calling IDirectMusicPerformance::PlaySegment()");
#endif
		pSS=0;
		pPerf->PlaySegment(pSeg,0,0,&pSS);
		if (pSS) pSS->GetStartTime(&mtStart);
		pPerf->MusicToReferenceTime(mtStart,&rtStart);
	}
	return rv;
}

BOOL test_ins_dls(DWORD patch,IDirectMusicCollection* pDLS)
{
	IDirectMusicInstrument *pi=0;
	BOOL rv=0;
	if (SUCCEEDED(pDLS->GetInstrument(patch,&pi)))
	{
		pi->Release();
		rv=1;
	}
	return rv;
}

int test_drum_kit(DWORD no,IDirectMusicCollection* dls)
{
	DWORD p=no|0x80000000;

	if (pGM)
		if (test_ins_dls(p,pGM)) return 1;

	if (pCDLS)
		if (test_ins_dls(p,pCDLS)) return 1;
	if (dls)
		if (test_ins_dls(p,dls)) return 1;

	return 0;
}

void do_dls_check(DWORD * i,IDirectMusicCollection * dls)
{

start:
	if (pGM)
		if (test_ins_dls(*i,pGM)) return;
	if (pCDLS)
		if (test_ins_dls(*i,pCDLS)) return;
	if (dls)
		if (test_ins_dls(*i,dls)) return;
	//hack hack hack

	if (*i&0xFF00)
	{
		*i&=0xFF00FF;
		goto start;
	}
	if (*i&0xFF0000)
	{
		*i&=0xFF;
		return;
	}
}

static cfg_int cfg_show_all("dmusic_show_all",0);

class MIDI_driver_dmusic : MIDI_driver
{
	bool dm_inited;

	virtual void do_init()
	{
		dm_inited=1;
		try {
#ifdef USE_LOG
			log_write("CoInitialize()");
#endif
			CoInitialize(0);
#ifdef USE_LOG
			log_write("CoCreateInstance / IDirectMusic");
#endif
			IDirectMusic* pDM=0;
			if (SUCCEEDED(CoCreateInstance(CLSID_DirectMusic,0,CLSCTX_INPROC,IID_IDirectMusic,(void**)&pDM)) && pDM)
			{
#ifdef USE_LOG
				log_write("IDirectMusic created OK");
#endif
				DMUS_PORTCAPS dmpc;
				memset(&dmpc,0,sizeof(dmpc));
				dmpc.dwSize=sizeof(dmpc);
				UINT np=0;
				GUID def;
				pDM->GetDefaultPort(&def);

				while(1)
				{
					if (pDM->EnumPort(np++,&dmpc)==S_FALSE) break;
					if (dmpc.dwClass==DMUS_PC_OUTPUTCLASS && (cfg_show_all || (dmpc.dwType!=DMUS_PORT_WINMM_DRIVER && dmpc.dwType==DMUS_PORT_KERNEL_MODE) || (dmpc.dwFlags&DMUS_PC_DLS) ))
					{
						wchar_t name_mbs[2*DMUS_MAX_DESCRIPTION] = {0};
						wcsncpy(name_mbs,dmpc.wszDescription,256);

						string_w info;
						{
							if (dmpc.dwType<3)
							{
								int dmport_types[3]={STRING_DMCAPS_WINMM,STRING_DMCAPS_USERMODE,STRING_DMCAPS_WDM};
								info+=WASABI_API_LNGSTRINGW(STRING_DEVICE_TYPE);
								info+=WASABI_API_LNGSTRINGW(dmport_types[dmpc.dwType]);
								info+=L"\x0d\x0a";
							}

							UINT z;
							for(z=0;z<N_DMCAPZ;z++)
							{
								if (dmpc.dwFlags & DMCAPZ[z].flag)
								{
									info+=WASABI_API_LNGSTRINGW(DMCAPZ[z].name);
									info+=L"\x0d\x0a";
								}
							}
							UINT n_effects=0;
							for(z=0;z<N_DMCAPZ1;z++)
							{
								if (dmpc.dwEffectFlags&DMCAPZ1[z].flag)
								{
									info+=n_effects ? L", " : WASABI_API_LNGSTRINGW(STRING_EFFECTS);
									info+=WASABI_API_LNGSTRINGW(DMCAPZ1[z].name);
									n_effects++;
								}
							}
							if (n_effects) info+=L"\x0d\x0a";
						}

						add_device(new MIDI_device_dmusic(dmpc.guidPort,0,dmpc.dwFlags,name_mbs,info));
						if ((dmpc.dwFlags&DMUS_PC_DIRECTSOUND)&&(dmpc.dwFlags&DMUS_PC_SOFTWARESYNTH))
						{
							wcscat(name_mbs,WASABI_API_LNGSTRINGW(IDS_WITH_OUTPUT));
							info+=WASABI_API_LNGSTRINGW(IDS_USES_WINAMPS_OUTPUT_PLUGINS);
							add_device(new MIDI_device_dmusic(dmpc.guidPort,1,dmpc.dwFlags,name_mbs,info));
						}
					}
				}
				pDM->Release();
			} 
		} catch(...) {
			// bewm.
			reset_devices();
		}

	}
	virtual const wchar_t * get_name() {return L"DirectMusic";}
	virtual GUID get_guid() {return dmusic_driver_guid;}
public:
	MIDI_driver_dmusic() {dm_inited=0;}
protected:
	void do_deinit()
	{
		if (!dm_inited) return;
		if (pGM)
		{
			pGM->Release();
			pGM=0;
		}
		if (pCDLS) {pCDLS->Release();pCDLS=0;}
		if (pLoader) {pLoader->Release();pLoader=0;}
		if (pPort) PortKill();
		if (pDM)
		{
			pDM->Release();
			pDM=0;
		}
		if (pPerf)
		{
			pPerf->CloseDown();
			pPerf->Release();
			pPerf=0;
		}

		CoUninitialize();
	}
};

static MIDI_driver_dmusic midi_driver_dmusic;