#include "cnv_pcmwaveout.h"
#include <mmsystem.h>
#include "waveout.h"
#include "../studio/services/svc_textfeed.h"

#define FEEDID_DEVICELIST "waveOut:DEVICES"

static String * devlist;
static UINT n_devs;

#define DEVICE_DEFAULT "(default device)"	//trick: use this string for default wave mapper, without querying device names and shit

_string cfg_dev("Device",DEVICE_DEFAULT);


static void devlist_init()
{
	if (devlist) return;
	n_devs=waveOutGetNumDevs()+1;
	UINT n;
	WAVEOUTCAPS caps;
	devlist=new String[n_devs];
	for(n=0;n<n_devs;n++)
	{
		if (waveOutGetDevCaps(n-1,&caps,sizeof(caps))==MMSYSERR_NOERROR)
		{
			devlist[n]=caps.szPname;
		}
	}
}

class TextFeed : public svc_textFeedI {
public:
	TextFeed() {
		registerFeed(FEEDID_DEVICELIST); // we output shit in <List> objects whose feed param is "DirectSound::DEVICES"
	}
	static const char *getServiceName() { return "waveOut TextFeed Service"; }

protected:
	virtual void registerCallback(const char *feedid, TextFeedCallback *cb)
	{
		if (!STRICMP(feedid,FEEDID_DEVICELIST))
		{//HACK: nice delayed init - don't query device list before we really need to, for really short loading time
			static bool inited;
			if (!inited)
			{
				inited=1;

				devlist_init();

				String feed_devlist="";
				UINT n;
				for(n=0;n<n_devs;n++)
				{
					if (!feed_devlist.isempty()) feed_devlist += ";";
					feed_devlist+=devlist[n];
				}
				sendFeed(FEEDID_DEVICELIST, feed_devlist);
				if (!strcmp(cfg_dev,DEVICE_DEFAULT)) cfg_dev=devlist[0];
			}
		}
		svc_textFeedI::registerCallback(feedid,cb);
	}
};

static waServiceTSingle<svc_textFeed, TextFeed> g_feed;


_int cfg_buf_ms("Buffer length (ms)",2000);
_int cfg_prebuf("Prebuffer (ms)",0);
_bool cfg_vol_enabled("Volume control enabled",1);
_bool cfg_vol_alt("Alt volume control method",0);
_bool cfg_vol_reset("Reset volume on stop",0);

class cnv_pcmwaveout: public svc_mediaConverterI {
private:
	WaveOut * pWO;
	int fmt_sr,fmt_bps,fmt_nch;
public:
	cnv_pcmwaveout()
	{
		pWO = 0;
		fmt_sr = 0;
		fmt_bps = 0;
		fmt_nch = 0;
	}
	~cnv_pcmwaveout()
	{
		if (pWO) delete pWO;
	}

	static const char *getServiceName() { return "WaveOut Output"; }

	virtual int canConvertFrom(svc_fileReader *reader, const char *name, const char *chunktype) { 
		if(chunktype && !STRICMP(chunktype,"pcm")) return 1;
		return 0;
	}
	virtual const char *getConverterTo() { return "OUTPUT:waveOut"; }

	virtual int getInfos(MediaInfo *infos) {return 0;}

	virtual int processData(MediaInfo *infos, ChunkList *chunk_list, bool *killswitch)
	{
		Chunk * c=chunk_list->getChunk("PCM");
		if (!c) return 0;
		char * data=(char*)c->getData();
		int size=c->getSize();
		if (size<=0) {
			if (pWO && infos->getData("audio_need_canwrite")) infos->setDataInt("audio_canwrite",pWO->CanWrite(),FALSE);
			return 1;
		}

		int sr,bps,nch;
		sr=c->getInfo("srate");
		bps=c->getInfo("bps");
		nch=c->getInfo("nch");
		
		if (pWO && (fmt_sr!=sr || fmt_bps!=bps || fmt_nch!=nch))
		{
			while(!*killswitch && pWO->GetLatency()>0) Sleep(1);
			delete pWO;
			pWO=0;
		}

		if (*killswitch) return 0;


		if (!pWO)
		{
			fmt_sr=sr;
			fmt_bps=bps;
			fmt_nch=nch;
			WaveOutConfig cfg;
			cfg.SetPCM(sr,nch,bps);
			cfg.SetBuffer(cfg_buf_ms,cfg_prebuf);
			const char * devname=cfg_dev;
			if (_stricmp(devname,DEVICE_DEFAULT))
			{
				devlist_init();
				for(UINT n=0;n<n_devs;n++)
				{
					if (!_stricmp(devname,devlist[n]))
					{
						cfg.SetDevice(n);
						break;
					}
				}
			}
			cfg.SetVolumeSetup(cfg_vol_enabled,cfg_vol_alt,cfg_vol_reset);
			pWO=WaveOut::Create(&cfg);
			if (!pWO)
			{
				//todo: cfg.GetError() yadda yadda
				return 0;
			}
			pWO->SetVolume(api->core_getVolume(m_coretoken));
			pWO->SetPan(api->core_getPan(m_coretoken));
		}

		while(!*killswitch)
		{
			int d=pWO->WriteData(data,size);
			if (d>0)
			{
				size-=d;
				if (size<=0) break;
				data+=d;
			}
			Sleep(1);
		}
		return 1;
	}

	virtual int getLatency(void) {return pWO ? pWO->GetLatency() : 0;}

	virtual int corecb_onSeeked(int newpos) {if (pWO) pWO->Flush();return 0;}

  
	virtual int corecb_onVolumeChange(int v) {pWO->SetVolume(v);return 0;}
	virtual int corecb_onPanChange(int v) {pWO->SetPan(v);return 0;}

	virtual int corecb_onAbortCurrentSong() {if (pWO) pWO->Flush();return 0;}
	virtual int corecb_onPaused() {if (pWO) pWO->Pause(1);return 0;}
	virtual int corecb_onUnpaused() {if (pWO) pWO->Pause(0);return 0;}
};



static WACNAME wac;
WAComponentClient *the = &wac;

#include "../studio/services/servicei.h"
static waServiceT<svc_mediaConverter, cnv_pcmwaveout> waveout;

// {E91551F2-E1CE-4484-B331-B3BE2B754B52}
static const GUID guid = 
{ 0xe91551f2, 0xe1ce, 0x4484, { 0xb3, 0x31, 0xb3, 0xbe, 0x2b, 0x75, 0x4b, 0x52 } };

WACNAME::WACNAME() {
#ifdef FORTIFY
  FortifySetName("cnv_pcmwaveout.wac");
  FortifyEnterScope();
#endif
  registerService(&g_feed);//autoderegistered on shutdown
  registerService(&waveout);
  registerSkinFile("Wacs/xml/waveout/waveout-prefs.xml"); 
}

WACNAME::~WACNAME() {
#ifdef FORTIFY
  FortifyLeaveScope();
#endif
  if (devlist)
  {
	delete[] devlist;
	devlist=0;
  }
  n_devs=0;
}

GUID WACNAME::getGUID() {
  return guid;
}

void WACNAME::onDestroy() {

}

void WACNAME::onCreate()
{
	// {EDAA0599-3E43-4eb5-A65D-C0A0484240E7}
	static const GUID cfg_audio_guid = 
	{ 0xedaa0599, 0x3e43, 0x4eb5, { 0xa6, 0x5d, 0xc0, 0xa0, 0x48, 0x42, 0x40, 0xe7 } };

	api->preferences_registerGroup("waveout", "waveOut", guid, cfg_audio_guid);
	


	registerAttribute(&cfg_dev);
	registerAttribute(&cfg_buf_ms);
	registerAttribute(&cfg_prebuf);

	registerAttribute(&cfg_vol_enabled);
	registerAttribute(&cfg_vol_alt);
	registerAttribute(&cfg_vol_reset);


}