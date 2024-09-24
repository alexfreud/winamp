#include "../studio/services/svc_mediaconverter.h"
#include "../studio/wac.h"
#include "../common/rootcomp.h"
#include "../studio/services/svc_action.h"
#include "../unpack/unpack_helper.h"
#include "main.h"

#define WACNAME WACcnv_midi

class WACNAME : public WAComponentClient{
public:
  WACNAME();
  virtual ~WACNAME();

  virtual const char *getName() { return NAME; };
  virtual GUID getGUID();

  virtual void onCreate();
  virtual void onDestroy();
  
  virtual int getDisplayComponent() { return FALSE; };

  virtual CfgItem *getCfgInterface(int n) { return this; }

private:
};


static WACNAME wac;
WAComponentClient *the = &wac;


// {28FDCD38-26A2-482c-A691-55901A355D9E}
static const GUID guid = 
{ 0x28fdcd38, 0x26a2, 0x482c, { 0xa6, 0x91, 0x55, 0x90, 0x1a, 0x35, 0x5d, 0x9e } };

GUID WACNAME::getGUID() {
  return guid;
}

static void update_extensions()
{
	static int old_mask;
	int new_mask = cfg_ext_mask;
	int n;
	for(n=0;n<MIDI_core::FileTypes_GetNum();n++)
	{
		int bit = 1<<n;
		if ( (new_mask & bit) && !(old_mask & bit) ) 
			api->core_registerExtension(StringPrintf("*.%s",MIDI_core::FileTypes_GetExtension(n)),MIDI_core::FileTypes_GetDescription(n),"Audio");
		else if ( !(new_mask & bit) && (old_mask & bit) )
		{
			api->core_unregisterExtension(StringPrintf("*.%s",MIDI_core::FileTypes_GetExtension(n)));
		}
	}
	old_mask = new_mask;
}

void WACNAME::onCreate()
{
	// {EDAA0599-3E43-4eb5-A65D-C0A0484240E7}
	static const GUID cfg_audio_guid = 
	{ 0xedaa0599, 0x3e43, 0x4eb5, { 0xa6, 0x5d, 0xc0, 0xa0, 0x48, 0x42, 0x40, 0xe7 } };
	
	registerSkinFile("xml/midi-prefs.xml"); 

	api->preferences_registerGroup("winamp.preferences.midi", "MIDI playback", guid, cfg_audio_guid);

	MIDI_core::GlobalInit();

	
	update_extensions();
}

void WACNAME::onDestroy() {
	MIDI_core::GlobalQuit();
}

static void check_messages()
{
	MSG msg;
	while(PeekMessage(&msg,0,0,0,PM_REMOVE))
		DispatchMessage(&msg);
}

//note: multiinstance support is NOT working, and will never be; it makes no sense anyway. also, multiinstance safety was totally fuct in directmusic drivers last time i bothered trying.

class cnv_MIDI : public svc_mediaConverterI {
private:
	static critical_section core_owner_sync;
	static cnv_MIDI * core_owner;
	
	DWORD thread_id;

	MemBlock<char> sample_buffer;
	
	MIDI_file * file;

	int is_open;
	int eof_flag;

	void core_reset()
	{
		core_owner_sync.enter();
		if (core_owner==this) {core_owner=0;MIDI_core::Close();}
		core_owner_sync.leave();
		if (file) {file->Free();file=0;}
		is_open=0;
		eof_flag=0;
	}

	int core_takeover()
	{
		core_owner_sync.enter();
		if (core_owner!=this)
		{
			if (core_owner!=0) {core_owner_sync.leave();return 0;}
			core_owner=this;
			thread_id = GetCurrentThreadId();
			MIDI_core::Init();
		}
		core_owner_sync.leave();
		return 1;
	}

	int check_file(MediaInfo * infos)
	{
		if (file && !STRICMP(file->path,infos->getFilename())) return 1;
		core_reset();
		MemBlock<char> data;
		int size;

		try {
			svc_fileReader * reader = infos->getReader();
			if (!reader) return 0;
			size = reader->getLength();
			if (size<=0) return 0;
			reader->seek(0);
			int firstread = size > 256 ? 256 : size;
			data.setSize(firstread);
			if (reader->read(data.getMemory(),firstread)!=firstread) return 0;
			if (MIDI_file::HeaderTest(data.getMemory(),size))
			{
				if (firstread != size)
				{
					if (data.setSize(size)==0) return 0;
					if (reader->read(data.getMemory()+firstread,size-firstread)!=size-firstread) return 0;
				}
			}
			else
			{
				void * unpack = unpack_helper::unpack_getHandle(reader);
				if (!unpack) return 0;
				size = api->fileGetFileSize(unpack);
				firstread = size > 256 ? 256 : size;
				data.setSize(firstread);
				if (api->fileRead(data.getMemory(),firstread,unpack)!=firstread) {api->fileClose(unpack);return 0;}
				if (!MIDI_file::HeaderTest(data.getMemory(),size)) {api->fileClose(unpack);return 0;}

				if (firstread != size)
				{
					if (data.setSize(size)==0) {api->fileClose(unpack);return 0;}
					if (api->fileRead(data.getMemory()+firstread,size-firstread,unpack)!=size-firstread) {api->fileClose(unpack);return 0;}
				}
				
				api->fileClose(unpack);				
			}
			file = MIDI_file::Create(infos->getFilename(),data.getMemory(),size);

			return !!file;
		}
		catch(...)
		{
			file = 0;
			return 0;
		}
	}

public:

	static const char *getServiceName() { return NAME; }

	cnv_MIDI()
	{
		file=0;
		is_open=0;
		eof_flag=0;
		thread_id=0;
	}

	~cnv_MIDI()
	{
		core_reset();
	}

	virtual int canConvertFrom(svc_fileReader *reader, const char *name, const char *chunktype)
	{
		return reader && !chunktype && name && MIDI_core::IsOurFile(name);
	}

	virtual const char *getConverterTo()
	{
		if (!core_takeover()) return "FINAL";
		return MIDI_core::UsesOutput() ? "PCM" : "FINAL";
	}

	virtual int getInfos(MediaInfo *infos)
	{
		if (!check_file(infos)) return 0;
		infos->setTitle(Std::filename(file->path));
		infos->setLength(file->len);

		infos->setInfo(
			StringPrintf("%sMIDI %i channels",
				file->info.e_type ? StringPrintf("%s ",file->info.e_type) : ""
				,file->info.channels)
			);

		return 1;
	}

	virtual int processData(MediaInfo *infos, ChunkList *chunk_list, bool *killswitch)
	{
		if (!check_file(infos)) return 0;
		if (!core_takeover()) return 0;

		if (!is_open)
		{
			MIDI_core::SetVolume(api->core_getVolume(m_coretoken));
			MIDI_core::SetPan(api->core_getPan(m_coretoken));

			if (!MIDI_core::OpenFile(file)) 
				return 0;
			is_open=1;
			eof_flag=0;
		}

		check_messages();

		if (!MIDI_core::HavePCM()) {Sleep(1);check_messages();return eof_flag ? 0 : 1;}
		else
		{
			int srate,nch,bps;
			int size;
			MIDI_core::GetPCM(&srate,&nch,&bps);
			size = 576 * nch * (bps/8);
			if (sample_buffer.getSize()<size) sample_buffer.setSize(size);
			size = MIDI_core::GetSamples(sample_buffer.getMemory(),size,(char*)killswitch);
			if (size<=0) 
				return 0;

			ChunkInfosI *ci=new ChunkInfosI();
			ci->addInfo("srate",srate);
			ci->addInfo("bps",bps);
			ci->addInfo("nch",nch);

			chunk_list->setChunk("PCM",sample_buffer.getMemory(),size,ci);
			

			return 1;
		}
	}

	virtual int getLatency(void) { return 0; }

	// callbacks
	virtual int corecb_onSeeked(int newpos)
	{
		if (core_owner==this) MIDI_core::SetPosition(newpos);
		return 0;
	}

	int getPosition(void)
	{
		if (core_owner==this && !MIDI_core::UsesOutput()) return MIDI_core::GetPosition();
		return -1;
	}

	virtual int corecb_onVolumeChange(int v)
	{
		if (core_owner==this) MIDI_core::SetVolume(v);
		return 0;
	}
	virtual int corecb_onPanChange(int v)
	{
		if (core_owner==this) MIDI_core::SetPan(v);
		return 0;
	}
	virtual int corecb_onAbortCurrentSong() {return 0;};
	virtual int corecb_onPaused()
	{
		if (core_owner==this) MIDI_core::Pause(1);
		return 0; 
	}
	virtual int corecb_onUnpaused() 
	{
		if (core_owner==this) MIDI_core::Pause(0);
		return 0; 
	}

	static void notify_eof()
	{
		core_owner_sync.enter();
		if (core_owner) core_owner->eof_flag=1;
		core_owner_sync.leave();		
	}

	static DWORD get_core_thread()
	{
		DWORD ret = 0;
		core_owner_sync.enter();
		if (core_owner) ret = core_owner->thread_id;
		core_owner_sync.leave();
		return ret;
	}

};

cnv_MIDI * cnv_MIDI::core_owner=0;
critical_section cnv_MIDI::core_owner_sync;

static waServiceFactoryT<svc_mediaConverter, cnv_MIDI> midi_svc;

#define ACTIONID_CONFIG "MIDI:CONFIG"

class MIDI_actions : public svc_actionI {
  public:
    MIDI_actions() {
      registerAction(ACTIONID_CONFIG, 0);
    }
    virtual ~MIDI_actions() { }

    virtual int onActionId(int id, const char *action, const char *param,int,int,void*,int,RootWnd*) {
      switch (id) {
        case 0:
			if (!_stricmp(action,ACTIONID_CONFIG))
			{
				if (MIDI_core::Config(MIDI_callback::GetMainWindow()))
				{
					update_extensions();
				}
			}
          return 1;
      }
    return 0;
    }
  static const char *getServiceName() { return "MIDI Player Actions Service"; }
};


static waServiceFactoryTSingle<svc_actionI, MIDI_actions> actions;

WACNAME::WACNAME() {
#ifdef FORTIFY
	FortifySetName("cnv_midi.wac");
	FortifyEnterScope();
#endif
	registerService(&midi_svc);
	registerService(&actions);
}

WACNAME::~WACNAME() {
#ifdef FORTIFY
  FortifyLeaveScope();
#endif
}


void MIDI_callback::NotifyEOF() {cnv_MIDI::notify_eof();}
HWND MIDI_callback::GetMainWindow() {return api->main_getRootWnd()->gethWnd();}
HINSTANCE MIDI_callback::GetInstance() {return wac.gethInstance();}
void MIDI_callback::Error(const char * tx) {}

void MIDI_callback::Idle(int ms)
{
	int core_thread = (GetCurrentThreadId() == cnv_MIDI::get_core_thread());
	int start = timeGetTime();
	do {
		if (core_thread) check_messages();
		Sleep(1);
	} while((int)timeGetTime() - start < ms);
	if (core_thread) check_messages();
}

extern "C" {
BOOL APIENTRY DllMain(HANDLE hMod,DWORD r,void*)
{
    if (r==DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls((HMODULE)hMod);
	}
	return 1;
	
}
}
