#include <windows.h>
#include <shlobj.h>
#include "../winamp/out.h"

#define PI_VER2 "v1.2"

/* #ifdef __alpha
#define PI_VER PI_VER2 " (AXP)"
#else
#define PI_VER PI_VER2 " (x86)"
#endif */

int getwrittentime();
int getoutputtime();

int srate, numchan, bps, active;
volatile int writtentime, w_offset;


BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

Out_Module out;

static int last_pause=0;

void config(HWND hwnd)
{
}

void about(HWND hwnd)
{
}

void init()
{
}

void quit()
{
}

int start_t;

int open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
	start_t=GetTickCount();
 	w_offset = writtentime = 0;
	active=1;
	numchan = numchannels;
	srate = samplerate;
	bps = bitspersamp;
	return 1;
}

void close()
{
}

int write(char *buf, int len)
{
	writtentime += len;
	return 0;
}

int canwrite()
{
	if (last_pause) return 0;
	if (getwrittentime() < getoutputtime()+MulDiv(65536,1000,srate*bps*numchan/8)) return 65536;
	return 0;
}

int isplaying()
{
	return 0;
}

int pause(int pause)
{
	int t=last_pause;
	if (!last_pause && pause) { w_offset+=GetTickCount()-start_t;  writtentime=0; }
	if (last_pause && !pause) { start_t=GetTickCount(); }
	last_pause=pause;
	return t;
}

void setvolume(int volume)
{
}

void setpan(int pan)
{
}

void flush(int t)
{
  w_offset=t;
  start_t=GetTickCount();
  writtentime=0;
}
	
int getoutputtime()
{
	if (last_pause)
		return w_offset;
	return GetTickCount()-start_t + w_offset;
}

int getwrittentime()
{
	int t=srate*numchan,l;
	int ms=writtentime;

	if (t)
	{
	l=ms%t;
	ms /= t;
	ms *= 1000;
	ms += (l*1000)/t;

	ms/=(bps/8);

	return ms + w_offset;
	}
	else
		return ms;
}

Out_Module out = {
	OUT_VER,
	"Nullsoft NULL Output " PI_VER2
	
	,
	65,
	0, // hmainwindow
	0, // hdllinstance
	config,
	about,
	init,
	quit,
	open,
	close,
	write,
	canwrite,
	isplaying,
	pause,
	setvolume,
	setpan,
	flush,
	getoutputtime,
	getwrittentime
};

__declspec( dllexport ) Out_Module * winampGetOutModule()
{
	return &out;
}
