// Winamp test dsp library 0.9 for Winamp 2
// Copyright (C) 1997, Justin Frankel/Nullsoft
// Feel free to base any plugins on this "framework"...

#include <windows.h>
#include <commctrl.h>
#include <math.h>
#include "../Winamp/dsp.h"
#include "resource.h"

// avoid stupid CRT silliness
//BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
//{
//	return TRUE;
//}


// pitch value
int g_pitch=100;

// pitch control window
HWND pitch_control_hwnd;

// auxilary pitch buffer (for resampling from)
short *pitch_buffer=NULL;
int pitch_buffer_len=0;
int quit_pitch=0;

// module getter.
winampDSPModule *getModule(int which);

void config(struct winampDSPModule *this_mod);
int init(struct winampDSPModule *this_mod);
int initpitch(struct winampDSPModule *this_mod);
void quit(struct winampDSPModule *this_mod);
void quitpitch(struct winampDSPModule *this_mod);
int modify_samples1(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);
int modify_samples2(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);
int modify_samples3(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);
int modify_samples_lopass(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);
int modify_samples_hipass(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);

static INT_PTR CALLBACK pitchProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

// Module header, includes version, description, and address of the module retriever function
typedef struct {
  int version;       // DSP_HDRVER
  char *description; // description of library
  winampDSPModule* (*getModule)(int);	// module retrieval function
  int (*sf)(int);
} winampDSPHeaderEx;

int sf(int v)
{
    int res;
    res = v * (unsigned long)1103515245;
    res += (unsigned long)13293;
    res &= (unsigned long)0x7FFFFFFF;
    res ^= v;
	return res;
}

winampDSPHeaderEx hdr = { DSP_HDRVER+1, "Nullsoft DSP v0.35 for Winamp 2ARG", getModule, sf };


// first module
winampDSPModule mod =
{
	"Nullsoft Echo v0.2",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	init,
	modify_samples1,
	quit
};


// second module
winampDSPModule mod2 =
{
	"Nullsoft Stereo Voice Removal v0.2",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	init,
	modify_samples2,
	quit
};

winampDSPModule mod3 =
{
	"Nullsoft Pitch/Tempo Control v0.2",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	initpitch,
	modify_samples3,
	quitpitch
};

winampDSPModule mod4 =
{
	"Nullsoft Lowpass Filter v1.0",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	init,
	modify_samples_lopass,
	quit
};
winampDSPModule mod5 =
{
	"Nullsoft Highpass Filter v1.0",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	init,
	modify_samples_hipass,
	quit
};


#ifdef __cplusplus
extern "C" {
#endif
// this is the only exported symbol. returns our main header.
__declspec( dllexport ) winampDSPHeaderEx *winampDSPGetHeader2()
{
	return &hdr;
}
#ifdef __cplusplus
}
#endif

// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1 or mod2 depending on 'which'.
winampDSPModule *getModule(int which)
{
	switch (which)
	{
		case 0: return &mod;
		case 1: return &mod2;
		case 2: return &mod3;
//		case 3: return &mod4;
//		case 4: return &mod5;
		default:return NULL;
	}
}

// configuration. Passed this_mod, as a "this" parameter. Allows you to make one configuration
// function that shares code for all your modules (you don't HAVE to use it though, you can make
// config1(), config2(), etc...)
void config(struct winampDSPModule *this_mod)
{
	MessageBox(this_mod->hwndParent,"This module is Copyright(C) 1997-1999, Nullsoft\n"
									"Notes:\n"
									" * 8 bit samples aren't supported.\n"
									" * Pitch control rules!\n"
									" * Voice removal sucks (works about 10% of the time)!\n"
									" * Echo isn't very good!\n"
									"etc... this is really just a test of the new\n"
									"DSP plug-in system. Nothing more.",
									"Configuration",MB_OK);
}

int init(struct winampDSPModule *this_mod)
{
	return 0;
}
int initpitch(struct winampDSPModule *this_mod)
{
	pitch_buffer_len=0;
	pitch_buffer=NULL;
	quit_pitch=0;		
	ShowWindow((pitch_control_hwnd=CreateDialog(this_mod->hDllInstance,MAKEINTRESOURCE(IDD_DIALOG1),this_mod->hwndParent,pitchProc)),SW_SHOW);
	return 0;
}

// cleanup (opposite of init()). Destroys the window, unregisters the window class
void quit(struct winampDSPModule *this_mod)
{
}

void quitpitch(struct winampDSPModule *this_mod)
{
	if (this_mod == &mod3)
	{
		if (pitch_buffer) GlobalFree(pitch_buffer);
		pitch_buffer_len=0;
		pitch_buffer=NULL;
		quit_pitch=1;
		if (pitch_control_hwnd)
		{
			DestroyWindow(pitch_control_hwnd);
			pitch_control_hwnd=0;
		}

	}
}

short echo_buf[65536], echo_buf2[65536];

int modify_samples1(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	// echo doesn't support 8 bit right now cause I'm lazy.
	if (bps==16)
	{
        int x,s;
        s = numsamples*nch;

        memcpy(echo_buf2,       echo_buf,       s*2);
        memcpy(echo_buf,        echo_buf+s,     s*2);
        memcpy(echo_buf+s,      echo_buf+s*2, s*2);
        memcpy(echo_buf+s*2,echo_buf+s*3, s*2);
        memcpy(echo_buf+s*3,samples, s*2);

        for (x = 0; x < s; x ++)
        {
                int s = samples[x]/2+echo_buf2[x]/2;
                samples[x] = (s>32767?32767:s<-32768?-32768:s);
        }
	}
	return numsamples;
}

int modify_samples3(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	int pitch=g_pitch;
	int rlen =numsamples*bps/8*nch;
	int index=0, x;
	int n; 
	int dindex; 
	if (quit_pitch || g_pitch==100) return numsamples;
	if (g_pitch > 200) g_pitch=200;
	if (g_pitch < 50) g_pitch=50;
	pitch = 100000/pitch;
	n=(numsamples*pitch)/1000;
	dindex=(numsamples<<11)/n;
	if (pitch_buffer_len < rlen) 
	{
		pitch_buffer_len = rlen;
		GlobalFree(pitch_buffer);
		pitch_buffer=GlobalAlloc(GMEM_FIXED,rlen);
	}
	if (bps == 16 && nch == 2)
	{
		short *buf=pitch_buffer;
		memcpy(buf,samples,rlen);
		for (x = 0; x < n; x ++)
		{
			int p=(index>>11)<<1;
			index+=dindex;
			samples[0] = buf[p];
			samples[1] = buf[p+1];
			samples+=2;
		}
		return n;
	}
	else if (bps == 16 && nch == 1)
	{
		short *buf=pitch_buffer;
		memcpy(buf,samples,rlen);
		for (x = 0; x < n; x ++)
		{
			int p=(index>>11);
			index+=dindex;
			*samples++ = buf[p];
		}
		return n;
	}
	return numsamples;
}


int modify_samples2(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	int x = numsamples;
	if (bps == 16)
	{
		short *a = samples;
		if (nch == 2) while (x--)
		{
			int l, r;
			l = a[1]-a[0];
			r = a[0]-a[1];		
			if (l < -32768) l = -32768;
			if (l > 32767) l = 32767;
			if (r < -32768) r = -32768;
			if (r > 32767) r = 32767;
			a[0] = l;
			a[1] = r;
			a+=2;
		}
	}
	return numsamples;
}

/*
int modify_samples_lopass(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	if (bps==16)
	{
		static int lastspl=0,lastspl2=0;
		int x;
		x=numsamples;
		if (nch==1) while (x--)
		{
			int thisspl=*samples;
			*samples++=(lastspl+thisspl)/2;
			lastspl=thisspl;
		}
		else if (nch == 2) while (x--)
		{
			int thisspl=*samples;
			*samples++=(lastspl+thisspl)/2;
			lastspl=thisspl;
			thisspl=*samples;
			*samples++=(lastspl2+thisspl)/2;
			lastspl2=thisspl;
		}
	}
	return numsamples;
}
*/


enum FILTER_TYPE {
	lowpass,highpass,bandpass
};

typedef struct 
{
	float		m_a0,m_a1;
	float		m_b0,m_b1,m_b2;
	float		m_d1,m_d2;
	float		m_k;
} filter;

float Filter(filter *f, const float x )
{
	float	d0,y;

	d0 = f->m_k*x - f->m_a1*f->m_d1 - f->m_a0*f->m_d2;
	y = d0*f->m_b2 + f->m_d1*f->m_b1 + f->m_d2*f->m_b0;
	f->m_d2 = f->m_d1;
	f->m_d1 = d0;

	return y;
}


void makefilter( filter *f, int t , float sample_rate , float cutoff , float dampening )
{
	float	a2,c;

	c = (float)( 1.f / tan( 3.14159265359*cutoff / sample_rate ) );
	a2 = 1.f + c*(c+dampening);

	f->m_a1 = 2.f * (1.f - c*c) / a2;
	f->m_a0 = (1.f + c*(c-dampening)) / a2;
	f->m_d1 = f->m_d2 = 0.f;

	switch( t )
	{
		case lowpass:
			f->m_k = 1.f / a2;
			f->m_b1 = 2.f;
			f->m_b0 = 1.f;
			break;

		case highpass:
			f->m_k = c*c / a2;
			f->m_b1 = -2.f;
			f->m_b0 = 1.f;
			break;

		case bandpass:
			f->m_k = c*dampening / a2;
			f->m_b1 = 0.f;
			f->m_b0 = -1.f;
			break;
	}
	f->m_b2 = 1.f;
}

int modify_samples_lopass(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	static int i;
	static filter f1,f2;
	if (!i)
	{
		i=1;
		makefilter(&f1,bandpass,44100,1000.0,0.5);
		makefilter(&f2,lowpass,44100,1.0,1.0);
	}
	if (bps==16)
	{
		int x;
		x=numsamples;
		if (nch == 2) while (x--)
		{
			int t=(int)Filter(&f1,*samples);
			*samples++=min(max(t,-32768),32767);
			t=(int)Filter(&f2,*samples);
			*samples++=min(max(t,-32768),32767);
		}
	}
	return numsamples;
}

#define mulspl(a,b) _mulspl((int)(a),(int)((b)*65536.0))

int _mulspl(int a, int b)
{
	a *= b;
	a >>= 16;
	return a;
}


int modify_samples_hipass(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	if (bps==16 && nch==2)
	{
		static short splbuf[32768+2];
		short int *spls=splbuf+nch;
		int x;
		memcpy(spls,samples,numsamples*sizeof(short)*nch);
		x=numsamples;
		while (x--)
		{
			int ch;
			for (ch = 0; ch < nch; ch ++)
			{
				int r=mulspl(spls[0],0.93) + mulspl(spls[-nch],-0.93) + mulspl(samples[-nch],0.86);
				samples[0] = max(min(r,32767),-32768);
				spls++;
				samples++;
			}
		}
		memcpy(splbuf,&splbuf[numsamples*nch],nch*sizeof(short));
	}
	return numsamples;
}


static BOOL CALLBACK pitchProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,50);
		SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,-50);
	    SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,1);
	    SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,0);
		{
			char str[123];
			wsprintf(str,"%s%d%%",g_pitch>=100?"+":"",g_pitch-100);
			SetDlgItemText(hwndDlg,IDC_BOOGA,str);
		}
	}
	if (uMsg == WM_VSCROLL)
	{
		HWND swnd = (HWND) lParam;
		if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
		{
			g_pitch = -SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_GETPOS,0,0)+100;
			{
				char str[123];
				wsprintf(str,"%s%d%%",g_pitch>=100?"+":"",g_pitch-100);
				SetDlgItemText(hwndDlg,IDC_BOOGA,str);
			}
		}
	}
	return 0;
}


