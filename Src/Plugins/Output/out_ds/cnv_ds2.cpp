#include "cnv_ds2.h"
#include "ds2.h"
#include "../studio/services/svc_textfeed.h"
#include "../studio/services/svc_action.h"
#include "../common/nsguid.h"
#include "../bfc/timerclient.h"
#include "../bfc/textfeed.h"

static WACNAME wac;
WAComponentClient *the = &wac;

// {F6FE7F49-B017-4bcc-842C-2FFA842FB033}
static const GUID guid = 
{ 0xf6fe7f49, 0xb017, 0x4bcc, { 0x84, 0x2c, 0x2f, 0xfa, 0x84, 0x2f, 0xb0, 0x33 } };

GUID WACNAME::getGUID() {
  return guid;
}

class _int_limited : public _int
{
private:
	int min,max;
public:
//	void setMinMax(int _min,int _max) {min=_min;max=_max;}
	_int_limited(const char * name,int defval,int _min,int _max) : _int(name,defval) {min=_min;max=_max;}
	friend class fooViewer;
	int operator =(int newval) { return setValueAsInt(newval) ? newval : getValueAsInt(); }
};

class fooViewer : public DependentViewerT<_int_limited>
{
protected:
	//took me some time to notice when bas borked it last time
	//virtual int viewer_onItemDataChange(_int_limited *item, int hint1, int hint2)
	virtual int viewer_onEvent(_int_limited *item, int event, int param2, void *ptr, int ptrlen)
	{
		int i = (int)(*item);
		if (i>item->max) (*item)=item->max;
		else if (i<item->min) (*item)=item->min;
		return 1;
	}
};
	
static fooViewer fooviewer;

#define DEVICE_DEFAULT "(default device)"

_string cfg_device("Device",DEVICE_DEFAULT);
_int_limited cfg_buf_ms("Buffer length (ms)",DS2config::DEFAULT_BUFFER,100,20000);
_int_limited cfg_prebuf("Prebuffer (ms)",DS2config::DEFAULT_PREBUFFER,0,20000);
_int_limited cfg_fade("Default fade time (ms)",333,0,20000);

_bool cfg_dofades("Use fades",0);
_bool cfg_oldpausefade("Old-style fade on pause",0);
_bool cfg_killsil("Kill silence",0);
_int_limited cfg_sil_db("Cutoff (in -dB)",40,15,200);
_bool cfg_hw_mix("Allow hardware mixing",1);
_bool cfg_delayed("Delayed DirectSound shutdown",1);
_bool cfg_wait("Full fadeout when exiting Winamp",1);
_bool cfg_fade_volume("Smooth volume changes",1);
_bool cfg_create_primary("Create primary buffer",0);
_bool cfg_override_primary("Override primary buffer format",0);
_int_limited cfg_primary_sr("Primary buffer override sample rate",44100,8000,192000);
_int_limited cfg_primary_nch("Primary buffer override number of channels",2,1,6);
_int_limited cfg_primary_bps("Primary buffer override bits per sample",16,8,32);
_int cfg_logvol_min("Logarithmic volume control scale",100);
_bool cfg_logfades("Logarithmic fades",0);
_bool cfg_pitch_enabled("Use pitch control",0);
_int cfg_pitch("Pitch",100);
_string cfg_volume("Volume control mode","linear");

_int_limited cfg_refresh("Status display refresh",50,1,5000);

class FADE_CTRL
{
private:
	_bool enabled,custom;
	_int_limited time;

public:
	FADE_CTRL(const char * name,int enab)
	: enabled(StringPrintf("Fade on %s enabled",name),enab), custom(StringPrintf("Fade on %s use custom time",name),0), time(StringPrintf("Fade on %s custom time",name),333,0,20000)
	{
	}

	void registerme(CfgItemI * diz)
	{
		diz->registerAttribute(&enabled);
		diz->registerAttribute(&custom);
		diz->registerAttribute(&time);
		fooviewer.viewer_addViewItem(&time);
	}

	operator int()
	{
		if (!enabled || !cfg_dofades) return 0;
		else if (custom) return (UINT)time;
		else return (UINT)cfg_fade;
	}
};

static FADE_CTRL 
	fade_start("start",0),
	fade_firststart("first start",0),
	fade_endoftrack("end of track",0),
	fade_pause("stop/pause",1),
	fade_seek("seek",1);

class myCfgItemI : public CfgItemI
{
public:
	myCfgItemI(const char * n,GUID _guid=INVALID_GUID) : CfgItemI(n,_guid) {setParentGuid(guid);}
};

class fadeShiz : public myCfgItemI
{
public:
	void registerStuff()
	{
		registerAttribute(&cfg_dofades);
		registerAttribute(&cfg_fade);
		registerAttribute(&cfg_oldpausefade);
		registerAttribute(&cfg_wait);
		registerAttribute(&cfg_fade_volume);
		fade_start.registerme(this);
		fade_firststart.registerme(this);
		fade_endoftrack.registerme(this);
		fade_pause.registerme(this);
		fade_seek.registerme(this);
	}
	fadeShiz(const char *n, GUID guid) : myCfgItemI(n,guid) {}
};


static fadeShiz fadeshiz("Fading",nsGUID::fromChar("{4D981DA3-F75D-431a-B617-46F3E45D2A1F}"));

static myCfgItemI cmpt("Compatibility",nsGUID::fromChar("{CBDF55F4-6EB6-45c1-B1DF-7A9F95C33758}"));

#define FEEDID_DEVICELIST "DirectSound:DEVICES"
#define FEEDID_VERSION "DirectSound:VERSION"
#define FEEDID_VOLCTRL "DirectSound:VOLCTRL"
#define FEEDID_STATUS "DirectSound:STATUS"

#if 0
class MyTextFeed : public TextFeed
{
public:
  MyTextFeed() {
    registerFeed(FEEDID_VOLCTRL, "linear;logarithmic;hybrid;disabled");
  }
//CUT	virtual int hasFeed(const char * name) 
//CUT	{
//CUT		return !_stricmp(name,FEEDID_DEVICELIST) || !_stricmp(name,FEEDID_VERSION) || !_stricmp(name,FEEDID_VOLCTRL);
//CUT	}
//CUT	virtual void registerCallback(const char *feedid, TextFeedCallback *cb)
//CUT	{
		if (!_stricmp(feedid,FEEDID_DEVICELIST))
		{
			String devlist="";
			DsDevEnum e;
			while(e)
			{
				if (!devlist.isempty()) devlist += ";";
				devlist+=e.GetName();
				e++;
			}

			cb->textfeed_onReceiveText(devlist);
		
			if (!strcmp(cfg_device,DEVICE_DEFAULT)) cfg_device=DsDevEnumDefault().GetName();
		}
		else if (!_stricmp(feedid,FEEDID_VERSION))
		{
			cb->textfeed_onReceiveText(DS2_ENGINE_VER);
		}
		else if (!_stricmp(feedid,FEEDID_VOLCTRL))
		{
			cb->textfeed_onReceiveText("linear;logarithmic;hybrid;disabled");
		}
	}
	virtual void unregisterCallback(const char *feedid, TextFeedCallback *cb) {}
};
#endif

class StatusTextFeed : public TextFeed, private TimerClientDI
{
public:
  StatusTextFeed() {
    cc = 0;
    registerFeed(FEEDID_STATUS);
  }
  int cc;
  virtual void onRegClient() {
    cc++;
    if (cc == 1) timerclient_setTimer(666,cfg_refresh);
  }
  virtual void onDeregClient() {
    cc--;
    if (cc == 0) timerclient_killTimer(666);
  }
private:
	void UpdateStatus();
//CUT	PtrList<TextFeedCallback> list;
	virtual void timerclient_timerCallback(int id) {UpdateStatus();}
	static char display[1024];
public:
	static const char * getDisplay() {return display;}
	static const char *getServiceName() { return "DirectSound Status Display"; }
//CUT	virtual int hasFeed(const char * name) {return !_stricmp(name,FEEDID_STATUS);}
#if 0//CUT
	virtual void registerCallback(const char *feedid, TextFeedCallback *cb)
	{
		if (!_stricmp(feedid,FEEDID_STATUS))
		{
			int n=list.getNumItems();
			list.addItem(cb);
			if (n==0)
			{
				timerclient_setTimer(666,cfg_refresh);
			}
			UpdateStatus();
		}
	}
	virtual void unregisterCallback(const char *feedid, TextFeedCallback *cb)
	{
		list.removeItem(cb);
		if (list.getNumItems()==0)
		{
			timerclient_killTimer(666);
		}
	}
#endif
};

char StatusTextFeed::display[1024];

#define ACTIONID_COPY "DirectSound:COPY"

class DsoundActions : public svc_actionI {
  public:
    DsoundActions() {
      registerAction(ACTIONID_COPY, 0);
    }
    virtual ~DsoundActions() { }

    virtual int onActionId(int id, const char *action, const char *param,int,int,void*,int,RootWnd*) {
      switch (id) {
        case 0:
			if (!_stricmp(action,ACTIONID_COPY))
			{
				const char * display = StatusTextFeed::getDisplay();
				if (OpenClipboard(0))
				{
					HANDLE hMem=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,strlen(display)+1);
					strcpy((char*)GlobalLock(hMem),display);
					GlobalUnlock(hMem);
					SetClipboardData(CF_TEXT,hMem);
					CloseClipboard();
				}
			}
          return 1;
      }
    return 0;
    }
  static const char *getServiceName() { return "DirectSound Actions Service"; }
};


static void FormatProgress(UINT pos,UINT max,UINT len,char * out)
{
	UINT pos1=MulDiv(pos,len,max);
	UINT n;
	*(out++)='[';
	for(n=0;n<len;n++)
	{
		*(out++)= (n==pos1) ? '#' : '=';
	}

	*(out++)=']';
	*(out++)=0;
}

static void FormatTime(__int64 t,char * out)
{
	int w,d,h,m,s,ms;
	w=(int)(t/(1000*60*60*24*7));	
	d=(int)(t/(1000*60*60*24))%7;
	h=(int)(t/(1000*60*60))%24;
	m=(int)(t/(1000*60))%60;
	s=(int)(t/(1000))%60;
	ms=(int)(t)%1000;
	if (w)
	{
		wsprintf(out,"%iw ",w);
		while(*out) out++;
	}
	if (d)
	{
		wsprintf(out,"%id ",d);
		while(*out) out++;
	}
	if (h)
	{
		wsprintf(out,"%i:",h);
		while(*out) out++;
	}
	wsprintf(out,h ? "%02i:":"%i:",m);
	while(*out) out++;
	wsprintf(out,"%02i.%03i",s,ms);
}

void StatusTextFeed::UpdateStatus()
{
	DS2_REALTIME_STAT stat;
	char total[32];
	__int64 time_total=DS2::GetTotalTime();
	FormatTime(time_total,total);

	if (DS2::GetRealtimeStatStatic(&stat))
	{
		char bigint1[32],bigint2[32];
		_i64toa(stat.bytes_written,bigint1,10);
		_i64toa(stat.bytes_played,bigint2,10);
		char time1[32],time2[32];
		FormatTime(stat.bytes_written/(stat.bps/8*stat.nch)*1000/stat.sr,time1);
		__int64 time_played=stat.bytes_played/(stat.bps/8*stat.nch)*1000/stat.sr;
		FormatTime(time_played,time2);
		
		char prog1[56],prog2[56];
		FormatProgress(stat.pos_play,stat.buf_size_bytes,48,prog1);
		FormatProgress(stat.pos_write,stat.buf_size_bytes,48,prog2);
#define EOL "\x0d\x0a"

		
				sprintf(display,
					"Output format: %u Hz, %u bits per sample, %u channel%s" EOL
					"Active buffer size: %u ms (%u bytes)" EOL
					"Device: \"%s\"" EOL 
					"Mixing: %s, primary buffer: %s%s" EOL EOL
					"Buffer playback cursor: %u bytes%s" EOL "%s" EOL
					"Buffer write cursor: %u bytes" EOL "%s" EOL 
					EOL
					"Data buffered:"EOL
					"Total: %u ms (%u bytes)" EOL
					"Async buffer: %u ms (%u bytes)"EOL
					EOL
					"Buffer locks done: %u" EOL
					"Underruns: %u" EOL
					"Time played: %s (%s bytes)" EOL
					"Time written: %s (%s bytes)" EOL
					"Total time played: %s" EOL 
					"Volume: %f dB / %f dB" EOL
					,
					stat.sr,stat.bps,stat.nch,stat.nch>1 ? "s":"",
					stat.buf_size_ms,stat.buf_size_bytes,
					DsDevEnumGuid(stat.current_device).GetName(),
					(stat.dscaps_flags&DSBCAPS_LOCHARDWARE) ? "hardware" : (stat.dscaps_flags&DSBCAPS_LOCSOFTWARE) ? "software" : "unknown",
					stat.have_primary_buffer ? "active" : "inactive",
					(stat.dscaps_flags_primary&DSBCAPS_LOCHARDWARE) ? " (hardware)" : (stat.dscaps_flags_primary&DSBCAPS_LOCSOFTWARE) ? " (software)" : "",
					stat.pos_play,stat.paused?" (paused)":"",prog1,stat.pos_write,prog2,
					stat.latency_ms,stat.latency,
					MulDiv(stat.bytes_async,1000,stat.sr*stat.nch*(stat.bps>>3)),stat.bytes_async,
					stat.lock_count,stat.underruns,
					time2,bigint2,
					time1,bigint1,
					total,
					stat.vol_left,stat.vol_right
				);

	}
	else
	{
		wsprintf(display,"Not active." EOL EOL "Total time played: %s",total);
#undef EOL
	}
#if 0//CUT
 	foreach(list)
		list.getfor()->textfeed_onReceiveText(display);
	endfor;
#endif
  sendFeed(FEEDID_STATUS, display);
}

//static waServiceTSingle<svc_textFeed, TextFeed> g_feed;
static waServiceTSingle<svc_textFeed, StatusTextFeed> g_statusfeed;
static waServiceTSingle<svc_action, DsoundActions> g_actions;

WACNAME::WACNAME()  {
#ifdef FORTIFY
  FortifySetName("cnv_pcmdsound.wac");
  FortifyEnterScope();
#endif
	addChildItem(&fadeshiz);
	addChildItem(&cmpt);
	registerService(new waServiceFactoryT<svc_mediaConverter, cnvDS2>);
	registerService(&g_statusfeed);
	registerService(&g_actions);
//	registerService(&g_feed);
}

WACNAME::~WACNAME() {
#ifdef FORTIFY
  FortifyLeaveScope();
#endif
}

void WACNAME::onCreate()
{
  	{
		char temp[128];
		api->getStringPrivate("Total time", temp,127, "0");
		temp[127]=0;
		DS2::SetTotalTime(_atoi64(temp));
	}


	// {EDAA0599-3E43-4eb5-A65D-C0A0484240E7}
	static const GUID cfg_audio_guid = 
	{ 0xedaa0599, 0x3e43, 0x4eb5, { 0xa6, 0x5d, 0xc0, 0xa0, 0x48, 0x42, 0x40, 0xe7 } };

	// {689D3A8E-3DDF-4d56-8BA4-8E068CF86F2D}
	static const GUID cfg_fade_guid = 
	{ 0x689d3a8e, 0x3ddf, 0x4d56, { 0x8b, 0xa4, 0x8e, 0x6, 0x8c, 0xf8, 0x6f, 0x2d } };

	// {27D1BBF0-6F65-4149-BE77-6FB2A2F59AA8}
	static const GUID cfg_status_guid = 
	{ 0x27d1bbf0, 0x6f65, 0x4149, { 0xbe, 0x77, 0x6f, 0xb2, 0xa2, 0xf5, 0x9a, 0xa8 } };

	// {9F60BF8B-1F3F-4c11-9BCD-AA15C9EAD1C4}
	static const GUID cfg_misc_guid = 
	{ 0x9f60bf8b, 0x1f3f, 0x4c11, { 0x9b, 0xcd, 0xaa, 0x15, 0xc9, 0xea, 0xd1, 0xc4 } };


	registerSkinFile("xml/directsound-prefs.xml"); 
	registerSkinFile("xml/directsound-status.xml"); 
	registerSkinFile("xml/directsound-fading.xml"); 
	registerSkinFile("xml/directsound-misc.xml"); 

	api->preferences_registerGroup("directsound", "DirectSound", guid, cfg_audio_guid);
	api->preferences_registerGroup("directsound.fading", "Fading", cfg_fade_guid, guid);
	api->preferences_registerGroup("directsound.status", "Status display", cfg_status_guid, guid);
	api->preferences_registerGroup("directsound.misc", "Other settings", cfg_misc_guid, guid);

	fadeshiz.registerStuff();

	registerAttribute(&cfg_device);
	registerAttribute(&cfg_buf_ms);
	registerAttribute(&cfg_prebuf);
	registerAttribute(&cfg_killsil);
	registerAttribute(&cfg_sil_db);
	registerAttribute(&cfg_pitch_enabled);
	registerAttribute(&cfg_pitch);
	registerAttribute(&cfg_volume);
	registerAttribute(&cfg_logvol_min);
	registerAttribute(&cfg_logfades);
	registerAttribute(&cfg_refresh);

	fooviewer.viewer_addViewItem(&cfg_buf_ms);
	fooviewer.viewer_addViewItem(&cfg_prebuf);
	fooviewer.viewer_addViewItem(&cfg_fade);
	fooviewer.viewer_addViewItem(&cfg_sil_db);
	fooviewer.viewer_addViewItem(&cfg_refresh);

	cmpt.registerAttribute(&cfg_delayed);
	cmpt.registerAttribute(&cfg_hw_mix);
	cmpt.registerAttribute(&cfg_create_primary);
	cmpt.registerAttribute(&cfg_override_primary);
	cmpt.registerAttribute(&cfg_primary_sr);
	cmpt.registerAttribute(&cfg_primary_nch);
	cmpt.registerAttribute(&cfg_primary_bps);
}


void WACNAME::onDestroy() {
	WAComponentClient::onDestroy();
	if (cfg_wait)
	{
		while(DS2::InstanceCount()>0) Sleep(1);
	}

	DS2::Quit();

	{
		char temp[128];
		_i64toa(DS2::GetTotalTime(),temp,10);
		api->setStringPrivate("Total time",temp);
	}
}

cnvDS2::cnvDS2() {
	m_ds2=0;
	ds2_paused=0;
	fadenow=DS2::InstanceCount()>0 ? fade_start : fade_firststart;
	pitch_set=1;
	sr=nch=bps=chan=0;
}

cnvDS2::~cnvDS2() {
	if (m_ds2)
	{
		m_ds2->FadeAndForget(fade_pause);
	}
}

int cnvDS2::getInfos(MediaInfo *infos)
{
	return 0;
}

unsigned long tea_key[4]={0xef542687,0x4d5c68ac,0x54274ef9,0x844dfc52};
unsigned long tea_sum=0xC6EF3720;
unsigned long tea_delta=0x9E3779B9;

static int strings_decrypt=0;
static char crypted_bps[]={'b'^0x25,'p'^0x25,'s'^0x25,0};
static char crypted_srate[]={(char)('s'+0x41),(char)('r'+0x41),(char)('a'+0x41),(char)('t'+0x41),(char)('e'+0x41),0};
static char crypted_nch[]={'n'-0x18,'c'-0x18,'h'-0x18,0};


int cnvDS2::processData(MediaInfo *infos, ChunkList *chunk_list, bool *killswitch)
{
/*	if (ds2_paused && m_ds2)
	{
		m_ds2->Pause(0);
		ds2_paused=0;
	}*/

	// strings "encrypted" for WMA pcm "secure" stuff
	
	int old_canwrite=m_ds2 ? m_ds2->CanWrite() : 0;

	char pcmstr[5]={(char)('p'+23),'c'^12,'M'-64,0};

	pcmstr[4]=0;
	pcmstr[1]^=12;
	pcmstr[0]-=23;
	pcmstr[2]+=64;
	Chunk *chunk1=chunk_list->getChunk(pcmstr/*"PCM"*/);
	pcmstr[3]=(char)('x'+85);
	pcmstr[3]-=85;
	Chunk *chunk=chunk_list->getChunk(pcmstr/*"PCMx"*/);
	if(chunk) {
		// decrypt using TEA (128-bit)
		int i=chunk->getSize()/8;
		unsigned long *v=(unsigned long *)chunk->getData();
		const unsigned long *k=tea_key;
		while(i) {
			register unsigned long y=v[0],z=v[1],sum=tea_sum, delta=tea_delta,n=32;
			/* sum = delta<<5, in general sum = delta * n */
			while(n-->0) {
				z -= (y << 4 ^ y >> 5) + y ^ sum + k[sum>>11 & 3];
				sum -= delta;
				y -= (z << 4 ^ z >> 5) + z ^ sum + k[sum&3];
			}
			v[0]=y; v[1]=z; v+=2; i--;
		}
	} else {
		chunk=chunk1;
		if(!chunk) return 0;
	}

	Chunk * c=chunk;//chunk_list->getChunk("PCM");
	if (!c) return 0;
	int size=c->getSize();
  if (size<=0 || !c->getData()) {
    if(infos->getData("audio_need_canwrite")) {
      int cw;
      if(m_ds2)
	  {
		  cw=old_canwrite;//can be negative
		  if (cw<0) cw=0;
	  }
      else cw=65536;
      infos->setDataInt("audio_canwrite",cw,MediaInfo::INFO_NOSENDCB);
    }
    return 1;
  }


	UINT _sr=c->getInfo("srate"),_nch=c->getInfo("nch"),_bps=c->getInfo("bps");
	DWORD _chan=0;
	{
		Chunk *cc=chunk_list->getChunk("SPEAKER_SETUP");
		if (cc)
		{
			if (cc->getSize()==4)
			{
				chan = *(DWORD*)cc->getData();
			}
		}
	}

	UINT _fade=fadenow;
	fadenow=0;
	DS2 * wait=0;
	UINT waitfade=0;
	
	if (_sr!=sr || _nch!=nch || _bps!=bps || _chan!=chan || _fade)
	{
		if (m_ds2)
		{
			wait=m_ds2;
			if (_fade)
			{
				waitfade=_fade;
			}
			else 
			{
				waitfade=cfg_dofades ? 50 : 0;//let's pretend that we're gapless hehe
				wait=m_ds2;
			}
			m_ds2=0;

			if (*killswitch) return 0;
		}
		sr=_sr;
		bps=_bps;
		nch=_nch;
		chan=_chan;
	}
	if (!m_ds2)
	{
		DS2config cfg;
		if (_stricmp(cfg_device,DEVICE_DEFAULT)) cfg.SetDeviceGUID(DsDevEnumName(cfg_device).GetGuid());
		cfg.SetPCM(sr,bps,nch);
		cfg.SetCreatePrimary(cfg_create_primary);
		cfg.SetPrimaryOverride(cfg_override_primary);
		cfg.SetPrimaryOverrideFormat(cfg_primary_sr,cfg_primary_bps,cfg_primary_nch);
		if (chan) cfg.SetChanMask(chan);
		cfg.SetWindow(api->main_getRootWnd()->gethWnd());
		cfg.SetMixing(cfg_hw_mix ? 0 : 2);

		use_pitch=cfg_pitch_enabled;		

		if (!_stricmp(cfg_volume,"disabled")) use_vol=0;
		else
		{
			use_vol=1;
			int volmode;
			if (!_stricmp(cfg_volume,"logarithmic")) volmode=1;
			else if (!_stricmp(cfg_volume,"hybrid")) volmode=2;
			else volmode=0;

			cfg.SetVolMode(volmode,cfg_logvol_min,cfg_logfades);
		}


		{//automagic idiotproof buffer size config (no more "too short fading" lamers)
			UINT bs=(UINT)cfg_buf_ms;
			UINT bs1=bs;
			UINT v=fade_endoftrack;
			if (bs<v) bs=v;
			v=fade_pause;
			if (bs<v) bs=v;
			v=fade_seek;
			if (bs<v) bs=v;
			UINT pb=cfg_prebuf;
			cfg.SetBuffer(bs,pb>bs ? bs : pb);
		}		
		cfg.SetSilence(cfg_killsil ? (float)cfg_sil_db : 0);
		cfg.SetImmediateShutdown(cfg_delayed);
		cfg.SetHavePitch(use_pitch);

		m_ds2=DS2::Create(&cfg);
		if (!m_ds2)
		{
			const char *moo=cfg.GetError();
			if (moo) infos->error(moo);
			return 0;
		}
		if (use_vol) m_ds2->SetPan_Int(api->core_getPan(m_coretoken));
		if (_fade)
		{
			m_ds2->SetVolume_Int(0);
			m_ds2->Fade_Int(_fade,use_vol ? api->core_getVolume(m_coretoken) : 255);
		}
		else
			m_ds2->SetVolume_Int(use_vol ? api->core_getVolume(m_coretoken) : 255);
		if (wait) m_ds2->WaitFor(wait,waitfade);
		pitch_set=1.0;
	}
	int ret=0;
	if (m_ds2->ForceWriteData(c->getData(),(UINT)size))
	{
		ret=1;
		if (old_canwrite<0) while(!*killswitch && m_ds2->CanWrite()<0) Sleep(1);
	}
  if(infos->getData("audio_need_canwrite")) infos->setDataInt("audio_canwrite",m_ds2->CanWrite(),MediaInfo::INFO_NOSENDCB);
  if (use_pitch)
  {
	  double foo=(double)cfg_pitch / 100.0;
	  if (foo<0.25) foo=0.25;
	  else if (foo>4.0) foo=4.0;
	  if (pitch_set!=foo)
	  {
		  m_ds2->SetPitch(foo);
		  pitch_set=foo;
	  }
  }
  return ret;
}

int cnvDS2::corecb_onSeeked(int newpos)
{
	if (m_ds2)
	{
		fadenow=fade_seek;
		m_ds2->FadeAndForget(fadenow);
		m_ds2=0;
	}
	return 0;
}


int cnvDS2::getLatency(void)
{
	return m_ds2 ? m_ds2->GetLatency() : 0;
}

int cnvDS2::corecb_onAbortCurrentSong()
{
	if (m_ds2)
	{
		m_ds2->FadeAndForget(fade_pause);
		m_ds2=0;
		fadenow=fade_start;
	}
	return 0;
}

int cnvDS2::corecb_onVolumeChange(int v)
{
	if (m_ds2 && use_vol)
	{
		if (cfg_fade_volume) m_ds2->FadeX_Int(100,v);
		else m_ds2->SetVolume_Int(v);
	}
	return 0;
}

int cnvDS2::corecb_onPanChange(int v)
{
	if (m_ds2 && use_vol)
	{
		m_ds2->SetPan_Int(v);
	}
	return 0;
}

int cnvDS2::corecb_onPaused()
{
	if (m_ds2)
	{
		UINT v=fade_pause;
		if (!v)
		{
			m_ds2->Pause(1);
		}
		else if (cfg_oldpausefade)
		{
			m_ds2->FadeAndForget(v);
			m_ds2=0;
			fadenow=v;
		}
		else
		{
			m_ds2->FadePause(v);
		}
		ds2_paused=1;
	}
	return 0;
}

int cnvDS2::corecb_onUnpaused()
{
	if (ds2_paused && m_ds2) {m_ds2->Pause(0);}
	ds2_paused=0;
	return 0;
}


int cnvDS2::corecb_onEndOfDecode()
{
	if (m_ds2)
	{
		m_ds2->KillEndGap();
		m_ds2->ForcePlay();
		fadenow=fade_endoftrack;
	}
	return 0;
}

int cnvDS2::sortPlacement(const char *oc)
{
//	if (!_stricmp(oc,"crossfader")) {return -1;}
	return 0;
}
