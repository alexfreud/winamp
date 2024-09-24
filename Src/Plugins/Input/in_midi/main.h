#ifndef STRICT
#define STRICT
#endif
#include "../Agave/Language/api_language.h"
#include <windows.h>
#include <malloc.h>

#ifndef NOVTABLE
#define NOVTABLE _declspec(novtable)
#endif

#include <mmsystem.h>
#include "core_api.h"
#include "../pfc/string_unicode.h"
#include "locale.h"

#define VER L"3.57"

#include "utils.h"

//#define USE_LOG

#ifdef USE_LOG
void log_write(char*);
void log_start();
void log_quit();
#else
#define log_write(X)
#define log_start()
#define log_quit()
#endif

class CStream;
struct CTempoMap;
struct CSysexMap;

class player_base;

class NOVTABLE MIDI_device
{
	friend class MIDI_driver;
private:
	MIDI_device * next;
	MIDI_driver * driver;
	string_w dev_name,dev_info;
protected:
	void set_name(const wchar_t * src) {dev_name=src;}
	void set_info(const wchar_t * src) {dev_info=src;}
public:
	//override me
	virtual player_base * create()=0;
	virtual GUID get_guid()=0;
	virtual ~MIDI_device() {};
	virtual bool is_default() {return 0;}
	virtual bool has_output() {return 0;}
	virtual bool has_dls() {return 0;}
	virtual bool has_freq() {return 0;}
	virtual bool volctrl_happy() {return 0;}

	const wchar_t * get_name() {return dev_name;}
	const wchar_t * get_info() {return dev_info;}

	MIDI_driver * get_driver() {return driver;}
};

class NOVTABLE MIDI_driver//ONLY for static objects !!!!!
{
private:
	static MIDI_driver * driver_list;
	MIDI_driver * next;
	MIDI_device * device_list;
	bool inited;
	void init() {if (!inited) {do_init();inited=1;};}
	void deinit() {if (inited) {do_deinit();reset_devices();inited=0;};}
protected:
	MIDI_driver();
	~MIDI_driver();

	void reset_devices();

	void add_device(MIDI_device * dev);//call this to add new device

	//override me
	virtual void do_init() {};
	virtual void do_deinit() {}

public:

	static MIDI_driver * driver_enumerate(int n);
	static int driver_count();
	
	MIDI_device * device_enumerate(int n);
	int device_count();

	static MIDI_device * find_device(GUID guid_driver,GUID guid_device);
	static MIDI_driver * find_driver(GUID guid_driver);
	static MIDI_device * find_device_default();
	
	static void shutdown();

	//override me
	virtual const wchar_t * get_name()=0;
	virtual GUID get_guid()=0;
	virtual bool is_default() {return 0;}
};


#include "midifile.h"

class NOVTABLE player_base
{
public:
	virtual ~player_base() {}
	virtual int gettime()=0;
	virtual int settime(int)=0;
	virtual void pause()=0;
	virtual void unpause()=0;
	virtual int setvol(int) {return 0;};
	virtual int setpan(int) {return 0;};
};

class MIDI_core
{
public:
	static int Init();
	static int UsesOutput() {return use_out;}
	static int OpenFile(MIDI_file * file);
	static void Close();
	static int GetSamples(void *sample_buffer, int bytes, char *killswitch);
	static void GetPCM(int * srate,int * nch,int * bps) {*srate=format_srate;*nch=format_nch;*bps=format_bps;}
	static int SetPosition(int); 
	static void Pause(int pause);
	static int GetPosition(void);
	static int GetLength(void);
	static void Eof();

	static int SetVolume(int volume); 
	static int SetPan(int pan);
	//setvolune/setpan safe to call at any moment

	static int player_getVol() {return volume;}
	static int player_getPan() {return pan;}

	static inline void player_setSource(CStream *s) {data_src=s;}


	static void MM_error(DWORD code);
	


	static inline MIDI_file * getFile() {return theFile;}
	static inline MIDI_device * getDevice() {return device;}
	static inline bool HavePCM() {return !!data_src;}
	static inline bool HavePlayer() {return !!plr;}

	static int IsOurFile(const char *fn);

	static void GlobalInit();
	static void GlobalQuit();
	static int Config(HWND wnd);
	static void WriteConfig();

	static int FileTypes_GetNum();
	static const char * FileTypes_GetExtension(int);
	static char * FileTypes_GetDescription(int);

private:
	static BOOL CALLBACK KarProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	static void update_vol();

	static bool use_out,use_smp;
	static MIDI_file* theFile;
	static CStream* data_src;
	static player_base* plr;
	static int format_srate,format_nch,format_bps;
	static int volume,pan;
	static bool eof;
	static UINT volmod;
	static UINT mix_dev,mix_idx;

	static HWND kwnd;
	static KAR_ENTRY *kmap;
	static UINT kmap_size,kmap_ptr;
	static char * kmap_data;
	static critical_section sync;

	static MIDI_device * device;
};

namespace MIDI_callback	//per-winamp implementations
{
	HWND GetMainWindow();
	HINSTANCE GetInstance();
	void NotifyEOF();
	void Error(const char *);
	void Idle(int ms=0);
};

//#pragma warning(disable:4800)

void get_temp_file(char* fn);

extern cfg_int cfg_hardware_reset;
extern cfg_int cfg_smp,cfg_reverb,cfg_chorus,cfg_nosysex;
extern cfg_int cfg_sampout,cfg_dm_imm;
extern cfg_int cfg_loop_type,cfg_loop_count,cfg_loop_infinite;
extern cfg_int cfg_wavein_dev,cfg_wavein_sr,cfg_wavein_ch,cfg_wavein_bps,cfg_wavein_src;
extern cfg_int cfg_ctrl_x,cfg_ctrl_y;
extern cfg_int cfg_ext_mask;
extern cfg_string cfg_extra_exts;
extern cfg_int cfg_volmode;
extern cfg_int cfg_recover_tracks;
extern cfg_int cfg_quick_seek;
extern cfg_int cfg_rmi_def;
extern cfg_int cfg_logvol;
extern cfg_struct_t<GUID> cfg_driver,cfg_device;
extern cfg_int cfg_playback_mode;
extern cfg_int cfg_eof_delay;
extern cfg_int cfg_bugged;
extern cfg_int cfg_freq;
extern cfg_int cfg_cur_tab;

enum{BUGGED_BLAH=0x10};

extern sysex_table cfg_sysex_table;

void ReleaseObject(IUnknown* o);

#include "in2.h"
extern In_Module mod;