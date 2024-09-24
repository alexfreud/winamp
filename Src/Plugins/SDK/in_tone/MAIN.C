/*
** Example Winamp input plug-in
** Copyright (c) 1998, Justin Frankel/Nullsoft Inc.
*/

#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <math.h>
#include <stdio.h>

#include "../Winamp/in2.h"

#define WM_WA_MPEG_EOF WM_USER+2


extern In_Module mod;
char lastfn[MAX_PATH];
short sample_buffer[576*2];

/*
// avoid CRT. Evil. Big. Bloated.
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
*/
int paused;
DWORD WINAPI __stdcall PlayThread(void *b);

int gen_freq=2600;

int killDecodeThread=0;
HANDLE thread_handle=INVALID_HANDLE_VALUE;

void config(HWND hwndParent)
{
	MessageBoxA(hwndParent,"To use, open 'tone://2600' (for 2600hz).","Configuration",MB_OK);   // Must stay in ANSI
}
void about(HWND hwndParent)
{
	MessageBoxA(hwndParent,"Nullsoft Tone Generator, by Justin Frankel","About Nullsoft Tone Generator",MB_OK);   // Must stay in ANSI
}

int init()
{
	return 0;
}

void quit() { }

int isourfile(const char *fn) 
{ 
	char buf[8];
	memcpy(buf,fn,7);
	buf[7]=0;
	return lstrcmpA(buf,"tone://")?0:1;    // Must stay in ANSI
}

int play(const char *fn) { 
	int maxlatency;
	int tmp;
	strcpy(lastfn,fn);
	// simple atoi() inline so that we don't have to use libc (for a nice 4k dll)
	{
		const char *s=fn+7;
		int n=0;
		while (*s >= '0' && *s <= '9')
		{
			n*=10;
			n += *s++ -'0';
		}
		gen_freq=n;
	}
	paused=0;
	memset(sample_buffer,0,sizeof(sample_buffer));

	maxlatency = mod.outMod->Open(44100,1,16, -1,-1);
	if (maxlatency < 0)
	{
		return 1;
	}
	mod.SetInfo(0,44,1,1);
	mod.SAVSAInit(maxlatency,44100);
	mod.VSASetInfo(44100,1);
	mod.outMod->SetVolume(-666);		

	killDecodeThread=0;
	thread_handle = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) PlayThread,(void *) &killDecodeThread,0,&tmp);
	
	return 0; 
}

void pause() { paused=1; mod.outMod->Pause(1); }
void unpause() { paused=0; mod.outMod->Pause(0); }
int ispaused() { return paused; }

void stop() { 
	if (thread_handle != INVALID_HANDLE_VALUE)
	{
		killDecodeThread=1;
		if (WaitForSingleObject(thread_handle,INFINITE) == WAIT_TIMEOUT)
		{
			MessageBoxA(mod.hMainWindow,"error asking thread to die!\n","error killing decode thread",0);   // Must stay in ANSI
			TerminateThread(thread_handle,0);
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}

	mod.outMod->Close();
	mod.SAVSADeInit();
}

int getlength() { return -1000; }
int getoutputtime() { return mod.outMod->GetOutputTime(); }
void setoutputtime(int time_in_ms) {  }

void setvolume(int volume) { mod.outMod->SetVolume(volume); }
void setpan(int pan) { mod.outMod->SetPan(pan); }

int infoDlg(const char *fn, HWND hwnd)
{
	return 0;
}

void getfileinfo(const char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename) filename=lastfn;
	if (title) 
	{
		const char *s=filename+7;
		int n=0;
		while (*s >= '0' && *s <= '9')
		{
			n*=10;
			n += *s++ -'0';
		}
		sprintf(title,"%dhz Tone",n);
	}
	if (length_in_ms) *length_in_ms=-1000;
}

void eq_set(int on, char data[10], int preamp) 
{ 
}


In_Module mod = 
{
	IN_VER,
	L"Nullsoft Tone Generator v0.2 "
/* #ifdef __alpha
	"(AXP)"
#else
	"(x86)"
#endif */
	,
	0,	// hMainWindow
	0,  // hDllInstance
	"\0"
	,
	0,	// is_seekable
	1, // uses output
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	
	getlength,
	getoutputtime,
	setoutputtime,

	setvolume,
	setpan,

	0,0,0,0,0,0,0,0,0, // vis stuff


	0,0, // dsp

	eq_set,

	NULL,		// setinfo

	0 // out_mod

};

__declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &mod;
}

int _fltused=0;

DWORD WINAPI __stdcall PlayThread(void *b)
{
	double angle=0.0,dangle=3.14159*2.0*(double)gen_freq/(double)44100.0;
	while (! *((int *)b) ) 
	{
		if (mod.outMod->CanWrite() >= ((sizeof(sample_buffer)/2)<<(mod.dsp_isactive()?1:0)))
		{	
			int l=sizeof(sample_buffer)/2,x=l/2;
			short *s=sample_buffer;
			while (x--)
			{
				int i;
				double d = sin(angle)*32766.5;
#ifndef __alpha
				__asm {
					fld d
					fistp i
				}
#else
				i = (int) d;
#endif
				*s++=i;
				angle += dangle;
			}
			{
				int t=mod.outMod->GetWrittenTime();
				mod.SAAddPCMData((char *)sample_buffer,1,16,t);
				mod.VSAAddPCMData((char *)sample_buffer,1,16,t);
			}
			l=mod.dsp_dosamples(sample_buffer,l/2,16,1,44100)*2;
			mod.outMod->Write((char *)sample_buffer,l);
		}
		else Sleep(50);
	}
	return 0;
}