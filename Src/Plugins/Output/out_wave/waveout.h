/*
simple waveout class.
usage:
create the object
write PCM data using WriteData(); WriteData() returns number of bytes successfully written; CanWrite() returns amout of bytes you can write at given point of time
ForcePlay after writing last data block to ensure that all your PCM data gets queued to waveout
wait for GetLatency() to become 0 to ensure that playback is finished
delete the object
*/


class WaveOutConfig
{
	friend class WaveOut;
private:
	UINT sr,nch,bps,buf_ms,dev,prebuf;
	WCHAR* error;
	void SetError(const WCHAR* x);
	bool use_volume,use_altvol,resetvol;
public:
	WaveOutConfig()
	{
		error=0;
		sr=44100;
		nch=2;
		bps=16;
		buf_ms=2000;
		dev=0;
		prebuf=200;
		use_volume=1;
		use_altvol=0;
		resetvol=0;
	}
	void SetPCM(UINT _sr,UINT _nch,UINT _bps) {sr=_sr;nch=_nch;bps=_bps;}
	void SetBuffer(UINT _buf,UINT _prebuf) {buf_ms=_buf;prebuf=_prebuf;}
	void SetDevice(UINT d) {dev=d;}
	void SetVolumeSetup(bool enabled,bool alt,bool _reset) {use_volume = enabled;use_altvol=alt;resetvol=_reset;}
	~WaveOutConfig()
	{
		if (error) LocalFree(error);
	}

	//call these after attempting to create WaveOut
	const WCHAR* GetError() {return error;}

};

class WaveOut{
public:
	static WaveOut * Create(WaveOutConfig * cfg);

	~WaveOut();
	
	int WriteData(const void * ptr,UINT siz);

	int GetLatency(void);

	void Flush();

  
	void SetVolume(int v);
	void SetPan(int p);
	void Pause(int s);
	void ForcePlay();
	int CanWrite();
	int IsPaused() {return paused?1:0;}
	UINT GetMaxLatency() {return MulDiv(buf_size,1000,fmt_mul);}
	bool PrintState(char * z);

	//for gapless mode
	void SetCloseOnStop(bool b) {closeonstop=b;}
	bool IsClosed();

private:
	WaveOut();

	typedef struct tagHEADER
	{
		tagHEADER * next;
		WAVEHDR hdr;
	} HEADER;
	HEADER * hdr_alloc();
	void hdr_free(HEADER * h);
	static void hdr_free_list(HEADER * h);
	
	HWAVEOUT hWo;
	HANDLE hThread;
	HANDLE hEvent;
	CRITICAL_SECTION sync;
	HEADER *hdrs,*hdrs_free;
	UINT fmt_bps,fmt_nch,fmt_sr,fmt_mul,fmt_align;//,fmt_chan;
	char * buffer;
	UINT buf_size,buf_size_used;
	UINT data_written,write_ptr;
	UINT minblock,maxblock,avgblock;
	DWORD last_time;
	DWORD p_time;
	UINT prebuf;

	UINT n_playing;
	UINT cur_block;
	bool paused,needplay;
	bool die;
	bool use_volume,use_altvol,use_resetvol;
	bool newpause;//,needflush;
	bool closeonstop;
	int myvol,mypan;
	DWORD orgvol;
	

	

	void killwaveout();
	void flush();
	void update_vol();	
	void advance_block();

	void reset_shit();
	void thread();
	void init();
	int open(WaveOutConfig * cfg);
	void do_altvol(char * ptr,UINT s);
	void do_altvol_i(char * ptr,UINT max,UINT start,UINT d,int vol);
	static DWORD WINAPI ThreadProc(WaveOut * p);
};
