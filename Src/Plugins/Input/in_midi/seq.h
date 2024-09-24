#define SEQ_HAVE_PANEL

class seq_base : public player_base
{
protected:
	
	
	int seq_cmd_start(DWORD cflags);
	void seq_cmd_stop();

	virtual ~seq_base();

	//OVERRIDE ME
	virtual void seq_shortmsg(DWORD msg)=0;
	virtual void seq_sysex(BYTE*,UINT)=0;
	virtual int seq_play_start() {return 1;}
	virtual void seq_play_stop() {}


	seq_base();
private:
	virtual int gettime();
	virtual int settime(int);
	virtual void unpause();
	virtual void pause();	

	DWORD preprocess(DWORD e);
	
	void send_sysex(int n);
//	void reset_ins();
	UINT do_sysex(UINT src,UINT tm);
	BOOL do_ctrl(DWORD e);
	void reset();
	int note_state(int ch,int note);
	void note_on(int ch,int note);
	void note_off(int ch,int note);
	UINT do_seek(DWORD n,DWORD p);
	void thread();
	DWORD get_time();
	void get_ins(UINT c);
	static DWORD WINAPI seq_trd(void* p);
	static void sysexfunc(seq_base* cb,BYTE* s,UINT sz);

	
	MIDI_file* mf;
	bool kill,paused;
	CSysexMap* smap;
	int pan,vol;

	UINT seek_to,n_events;
	MIDI_EVENT* events;

	UINT c_loop,loop_start;
	BYTE notes[256];
	BYTE ctrl_tab[16][128];
	BYTE ins_tab[16];
	DWORD tm_ofs,p_time;
	HANDLE hTrd;
	DWORD ins_set;

#ifdef SEQ_HAVE_PANEL
	HWND hCtrl;
	float tempo;
	BOOL novol,noins;
	DWORD last_time_ms;
	double last_time_ret;
	CRITICAL_SECTION tm_sec;	
	DWORD mute_mask;
	bool initialized;

	static BOOL CALLBACK CtrlProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
	void do_msg(UINT msg,WPARAM wp,LPARAM lp);
	void set_mute(UINT ch,BOOL st);
#endif
};
