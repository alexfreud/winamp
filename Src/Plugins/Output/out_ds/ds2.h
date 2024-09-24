#ifndef _DS2_H
#define _DS2_H

#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "ds_main.h"
#include "Config.h"
#include "SoundBlockList.h"
#include "DevEnum.h"
#include "VolCtrl.h"

class CriticalSection : public CRITICAL_SECTION
{
public:
	inline void Enter() {EnterCriticalSection(this);}
	inline void Leave() {LeaveCriticalSection(this);}
	CriticalSection() {InitializeCriticalSection(this);}
	~CriticalSection() {DeleteCriticalSection(this);}
	//BOOL TryEnter() {return TryEnterCriticalSection(this);}
};

typedef struct
{
	UINT sr,bps,nch;
	UINT buf_size_bytes,buf_size_ms;
	UINT pos_play,pos_write,latency,latency_ms;
	UINT lock_count;
	UINT underruns;
	size_t bytes_async;
	__int64 bytes_written,bytes_played;
	double vol_left,vol_right;
	GUID current_device;
	bool have_primary_buffer;
	bool paused;
	DWORD dscaps_flags;
	DWORD dscaps_flags_primary;
} DS2_REALTIME_STAT;


class DS2
{
private:
	
	DS2(DS2config * cfg);

	SoundBlockList BlockList;
	UINT LockCount;
	UINT Underruns;
	
	bool DoLock();

public:
	~DS2();  
	int WriteData(void * data,UINT size,bool *killswitch);//returns 1 on success and 0 on failure
	int WriteDataNow(void * data,UINT size);//sleep-less version, writes CanWrite() of bytes immediately, returns amount of data written
	int ForceWriteData(void * data,UINT size);//sleep-less force-write all the data w/o sleep/canwrite (async buffer has no size limit), use with caution

	/*
	how to use writedata shit

  a) just WriteData(), will sleep until its done
  b) WriteDataNow() until we're done (writes as much data as possible at the moment, without sleep)
  c) ForceWriteData() (evil!), then sleep while CanWrite()<0

    */

	void StartNewStream();

	UINT GetLatency();
	void _inline Release() {delete this;}//obsolete
	void Pause(int new_state);
	void SetVolume(double);
	inline void SetVolume_Int(int i) {SetVolume((double)i/255.0);}
	void SetPan(double);
	inline void SetPan_Int(int i) {SetPan((double)i/128.0);}
	void Flush();//causes problems with some um.. drivers
	int CanWrite();//can be negative !!!!!!! (eg. after ForceWriteData)
	inline UINT BufferStatusPercent() {return MulDiv(data_buffered+(UINT)BlockList.DataSize(),buf_size,100);}
	void ForcePlay();
	void KillEndGap();

#ifdef DS2_HAVE_FADES
	void FadePause(UINT time);
	void FadeAndForget(UINT time);
	void Fade(UINT time,double destvol);
	inline void Fade_Int(UINT time,int destvol) {Fade(time,(double)destvol/255.0);}
	void FadeX(UINT time,double destvol);//actual fade time depends on volume difference
	inline void FadeX_Int(UINT time,int destvol) {FadeX(time,(double)destvol/255.0);}

#endif

	void WaitFor(DS2 * prev,UINT fadeout=0);

	//gapless mode stuff
	void SetCloseOnStop(bool b);
	bool IsClosed();
	
private:
#ifdef _DEBUG
	DWORD serial;
	UINT sync_n;
#endif

	int Open(DS2config * cfg);
	DS2 * next;	
	DS2 * wait;
	UINT prebuf;
	DWORD flags;
	enum
	{
		FLAG_UPDATED=1,
		FLAG_WAITED=1<<1,
		FLAG_NEED_PLAY_NOW=1<<2,
		FLAG_DIE_ON_STOP=1<<3,
		FLAG_CLOSE_ON_STOP=1<<4,
		FLAG_PAUSED=1<<5,
		FLAG_PLAYING=1<<6,
//		FLAG_UNDERRUNNING=1<<7,
		FLAG_USE_CPU_MNGMNT=1<<8,
		FLAG_FADEPAUSE=1<<9,
		FLAG_FADEPAUSING=1<<10,
		FLAG_STARTSIL=1<<11,
		FLAG_SWMIXED=1<<12,
	};
	IDirectSoundBuffer * pDSB;
	IDirectSound * myDS;
	UINT fmt_nch,fmt_bps,fmt_sr,fmt_mul;
	UINT buf_size,clear_size;
	__int64 last_nonsil,pos_delta,pos_delta2;
	inline __int64 GetCurPos() {return data_written-data_buffered;}
	__int64 GetSafeWrite();
	__int64 data_written;
	UINT data_buffered;
	UINT silence_buffered;

	UINT bytes2ms(UINT bytes);
	UINT ms2bytes(UINT ms);
	

#ifdef DS2_HAVE_FADES
	DsVolCtrl VolCtrl;
	double fadepause_orgvol;
	UINT fadepause_time;
	UINT waitfade;

#else
	class DsVolCtrl
	{
	private:
		double CurVol,CurPan;
	public:
		DsVolCtrl() {CurVol=1;CurPan=0;}
		inline void SetTime(__int64) {}
		inline void SetFade(__int64,double) {}
		inline void SetFadeVol(__int64,double,double) {}
		inline void SetFadePan(__int64,double,double) {}
		inline void SetVolume(double v) {CurVol=v;}
		inline void SetPan(double p) {CurPan=p;}
//		inline __int64 RelFade(__int64 max,double destvol) {return 0;}
		void Apply(IDirectSoundBuffer * pDSB);
		inline bool Fading() {return 0;}
//		inline double GetCurVol() {return CurVol;}
//		inline double GetDestVol() {return CurVol;}
		inline void Reset() {}
	};
	DsVolCtrl VolCtrl;
#endif

	void SetVolumeI(double);
	void SetPanI(int _pan);
	void _setvol();
	void _setpan();
	void update_pos();
	void ds_stop();
	void ds_kill();
	bool Update();

	void reset_vars();
	void do_reset_vars();

	int silence_delta;
	void test_silence(char * buf,int len,int * first,int* last);

	static DWORD WINAPI ThreadFunc(void*);
	UINT _inline _align() {return (fmt_bps>>3)*fmt_nch;}
	UINT _inline _align_var(UINT var) {return var-(var%_align());}


	static void SYNC_IN();
	static void SYNC_OUT();

	typedef HRESULT (WINAPI *tDirectSoundCreate)( const GUID * lpGuid,  LPDIRECTSOUND * ppDS,    IUnknown FAR * pUnkOuter  );
	static tDirectSoundCreate pDirectSoundCreate;

	static IDirectSound * pDS;

public:

	static void Init();//init is OBSOLETE
	static void Quit(bool wait=0);//must be called on exit, NOT from DllMain, its ok to call it if init never happened

	static DS2 * Create(DS2config * cfg);

	static bool InitDLL();//used internally

	static HRESULT myDirectSoundCreate(const GUID * g,IDirectSound ** out);

	static void SetTotalTime(__int64);
	static __int64 GetTotalTime();
	static UINT InstanceCount();

	void GetRealtimeStat(DS2_REALTIME_STAT * stat);
	static bool GetRealtimeStatStatic(DS2_REALTIME_STAT * stat);
	__int64 GetOutputTime();

#ifdef DS2_HAVE_DEVICES
	static bool TryGetDevCaps(const GUID *g,LPDSCAPS pCaps,DWORD * speakercfg=0);//for current device

	typedef HRESULT (WINAPI *tDirectSoundEnumerate)(LPDSENUMCALLBACK lpDSEnumCallback,LPVOID lpContext);
	static tDirectSoundEnumerate pDirectSoundEnumerate;

	static GUID GetCurDev();
#endif


#ifdef DS2_HAVE_PITCH
	void SetPitch(double p);
#endif

};

#endif