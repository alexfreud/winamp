#include <windows.h>
#include "audiostub.h"

#define CAPTION "NSV Player Sound Output Error"
#define MAX(x,y) (( y ) < ( x ) ? ( x ) : ( y ))
#define MIN(x,y) (( x ) < ( y ) ? ( x ) : ( y ))

#define S_MINSIZE (1<<28)
#define MAX_NUM_BLOCKS 8

#define NUM_BLOCKS 8
#define BUFSIZE_MS 1500

#define BLOCKSIZE_MAX 32768
#define BLOCKSIZE_MIN 8192

int g_audio_use_mixer=0;

class PCM_AudioOut : public IAudioOutput 
{
  public:
    PCM_AudioOut(int samplerate, int numchannels, int bitspersamp);
    ~PCM_AudioOut();

    int canwrite(); // returns bytes writeable
    void write(void *_buf, int len);
    unsigned int getpos();
    unsigned int getwritepos();
    void flush(unsigned int time_ms);
    void pause(int pause);
    int isplaying(void);
    void setvolume(int volume);
    void setpan(int pan);

    int open_success() { return init; }
    void getdescstr(char *buf)
    {
      *buf=0;
      if (g_srate && g_nch) wsprintf(buf,"%dkHz %s",g_srate/1000,g_nch==2?"stereo":"mono");
    }

  private:
    DWORD ThreadP();

    void _setvol(void);

    static DWORD WINAPI _threadproc(LPVOID p);
    static void CALLBACK cbFunc(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
    {
	    if (uMsg == WOM_DONE) { ReleaseSemaphore((HANDLE)dwInstance,1,NULL);}
    }
    void do_set_blocksizes(void);
    void do_samples_altvol(char *in, int blen);

    int init;

    int min_blocksize;
    int max_blocksize;

    int ispcnt;
    int num_blocks;

    char            *g_buffer, *g_buffer_write, *g_buffer_read;
    int              g_buffer_length, g_buffer_valid,g_buffer_inlength;
    HWAVEOUT         g_hWaveOut;
    WAVEHDR          g_wave_headers[MAX_NUM_BLOCKS];
    int				g_bps,g_nch,g_srate;
    volatile int		g_pause, g_wavecnt, g_prebuf,g_writeall, g_quit_flag,g_writetime_bytes, g_outtime_bytes, g_outtime_interp;

    HANDLE           g_hSem,g_hEvent, g_hThread;
    CRITICAL_SECTION g_cs;

    int              g_bytes_per_sec;
    int a_v,a_p;

    unsigned char *g_vol_table;
};


PCM_AudioOut::PCM_AudioOut(int samplerate, int numchannels, int bitspersamp)
{
  init=0;
  a_v=255;
  a_p=0;
  num_blocks=0;

  g_buffer_valid=g_buffer_inlength=0;
  g_hWaveOut=NULL;
  memset(g_wave_headers,0,sizeof(g_wave_headers));

  g_hSem=g_hEvent=g_hThread=0;

	int x;
	DWORD id;
	MMRESULT res;
	WAVEFORMATEX wfx={WAVE_FORMAT_PCM,numchannels,samplerate,samplerate*numchannels*(bitspersamp/8),
		numchannels*(bitspersamp/8),bitspersamp};
	
	g_bps=bitspersamp;
	g_nch=numchannels;
  g_srate=samplerate;
	g_bytes_per_sec=wfx.nAvgBytesPerSec;

  {
    char *p=(char*)g_wave_headers;
    int n=sizeof(g_wave_headers);
    while (n--) *p++=0;
  }
	g_buffer_length = MulDiv(g_bytes_per_sec, BUFSIZE_MS, 1000);
	g_buffer_length &= ~1023;
	if (g_buffer_length < 4096) g_buffer_length=4096;

	g_buffer=(char *)GlobalAlloc(GMEM_FIXED,g_buffer_length+min(65536,g_buffer_length));
	if (g_buffer == NULL)
	{
		MessageBox(NULL,"Error allocating buffer", CAPTION,MB_OK|MB_ICONSTOP);
		return;
	}

	g_prebuf=g_buffer_length/4;
	g_buffer_read=g_buffer_write=g_buffer;
	g_wavecnt=g_pause=g_writeall=g_buffer_valid=g_writetime_bytes=g_outtime_bytes=g_buffer_inlength=0;
	g_quit_flag=0;
	
  g_vol_table=NULL;

	do_set_blocksizes();

	g_hSem=CreateSemaphore(NULL,0,256,NULL);
	for (x = 0; (res=waveOutOpen(&g_hWaveOut,WAVE_MAPPER,&wfx,(DWORD)cbFunc,(DWORD)g_hSem,CALLBACK_FUNCTION))==MMSYSERR_ALLOCATED && x<10; x ++)
		Sleep(100);
	if (res != MMSYSERR_NOERROR)
	{
		char t[512];
		waveOutGetErrorText(res,t,sizeof(t));
		MessageBox(NULL,t, CAPTION,MB_OK|MB_ICONSTOP);
		GlobalFree((HGLOBAL) g_buffer);
		CloseHandle(g_hSem);
		g_buffer = NULL;
		return;
	}

  ispcnt=0;
	g_outtime_interp=GetTickCount();
	g_hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	InitializeCriticalSection(&g_cs);

	g_hThread=CreateThread(NULL,0,_threadproc,(void*)this,0,&id);
  SetThreadPriority(g_hThread,THREAD_PRIORITY_HIGHEST);

  init=1;
}


void PCM_AudioOut::do_set_blocksizes(void)
{
	int t,t2,t4;
	t=(MulDiv(BLOCKSIZE_MIN,g_bytes_per_sec,44100)+1023)&~1023;
	if (t<1024) t=1024;
	if (t>32768) t=32768;

	t2=(MulDiv(BLOCKSIZE_MAX,g_bytes_per_sec,44100*4)+1023)&~1023;
	if (t2 < t) t2 = t;
	if (t2 > 65536) t2=65536;

	t4 = NUM_BLOCKS;

	num_blocks=t4;
	max_blocksize=t2;
	min_blocksize=t;
}


PCM_AudioOut::~PCM_AudioOut(void)
{
  if (init)
  {
	  int x;
	  g_quit_flag=1;
	  SetEvent(g_hEvent);
	  while (g_quit_flag == 1) Sleep(70);

	  waveOutReset(g_hWaveOut);

	  for (x = 0; x < MAX_NUM_BLOCKS; x++)
		  if (g_wave_headers[x].dwFlags & WHDR_PREPARED)
			  waveOutUnprepareHeader(g_hWaveOut,&g_wave_headers[x],sizeof(g_wave_headers[0]));
	  if (waveOutClose(g_hWaveOut) != MMSYSERR_NOERROR)
	  {
		  MessageBox(NULL,"Error closing sound device.",CAPTION,MB_OK);
	  }
	  if (g_buffer) GlobalFree((HGLOBAL) g_buffer);
	  DeleteCriticalSection(&g_cs);
	  CloseHandle(g_hThread);
	  CloseHandle(g_hSem);
	  CloseHandle(g_hEvent);

    if(g_vol_table) GlobalFree((HGLOBAL)g_vol_table);
  }
}

void PCM_AudioOut::write(void *_buf, int len)
{
  char *buf=(char *)_buf;
	int l2;
	if (len > 8192) len=8192;
	l2=(g_buffer_write+len)-(g_buffer+g_buffer_length);
	if (len <= 0 || !buf)
	{
		g_writeall=1;
    //g_prebuf=0;
		SetEvent(g_hEvent);
		return;
	}
  ispcnt=0;
	g_writeall=0;
	if (l2 > 0)
	{
		int l1=len-l2;
		memcpy(g_buffer_write,buf,l1);
		memcpy(g_buffer,buf+l1,l2);
		g_buffer_write=g_buffer+l2;
	}
	else
	{
		memcpy(g_buffer_write,buf,len);
		g_buffer_write += len;
		if (g_buffer_write == g_buffer+g_buffer_length) g_buffer_write=g_buffer;
	}

	EnterCriticalSection(&g_cs);
	g_buffer_valid+=len;
	LeaveCriticalSection(&g_cs);
	g_writetime_bytes+=len;
	if (g_wavecnt < num_blocks) 
	{
		SetEvent(g_hEvent);
	}
	return;
}

int PCM_AudioOut::canwrite(void)
{
	int t=(g_buffer_length-g_buffer_valid);
	if (g_wavecnt==0) 
	{
		SetEvent(g_hEvent);
	}
  if (t>8192) t=8192;  // RG: since write() caps the # of bytes at 8192, this should reflect that!  otherwise, if we call write() with too many bytes, it throws the extra away and we'll never know it.
	return t;
}

int PCM_AudioOut::isplaying(void)
{
	if (g_wavecnt==0) 
	{
		SetEvent(g_hEvent);
	}
  if (ispcnt < 7) ispcnt++;
  if (g_buffer_valid < MIN(g_buffer_length/2,min_blocksize) && ispcnt==7) 
  {
    g_writeall=1;
    g_prebuf=0;
  }
	return (g_wavecnt>0) || (g_buffer_valid>0);
}

void PCM_AudioOut::pause(int pause)
{
	if (g_hWaveOut)
	{
		int lastp=g_pause;
		g_pause=pause;
		if (pause == lastp) return;
		if (g_pause)
			waveOutPause(g_hWaveOut);
		else
		{
			waveOutRestart(g_hWaveOut);
			g_outtime_interp=GetTickCount();
			SetEvent(g_hEvent);
		}
	}
}

void PCM_AudioOut::_setvol(void)
{
	DWORD vol, vo2, gv;
	if (g_hWaveOut)
	{
		a_v=MIN(255,MAX(a_v,0));
		a_p=MIN(127,MAX(a_p,-127));

		vo2 = vol = (a_v*65535) / 255;

		if (a_p > 0)
		{
			vol *= (127-a_p);
			vol /= 127;
		}
		else if (a_p < 0)
		{
			vo2 *= (127+a_p);
			vo2 /= 127;
		}
		gv=vol|(vo2<<16);
    if(g_audio_use_mixer) {
      if(g_vol_table) {
        EnterCriticalSection(&g_cs);
        GlobalFree((HGLOBAL)g_vol_table);
        g_vol_table=NULL;
        LeaveCriticalSection(&g_cs);
      }
      waveOutSetVolume(g_hWaveOut,gv);
    } else {
      EnterCriticalSection(&g_cs);
      if(!g_vol_table) {
        int l=(g_bps==8)?512:(4*32769);
				g_vol_table=(unsigned char *)GlobalAlloc(GPTR,l);
      }
      //compute volume lookup table
      int x;
      if (g_bps==8) {
			  if (g_nch==1) for (x = 0; x < 256; x ++) g_vol_table[x] = (x*a_v)/256;
				else for (x = 0; x < 256; x ++)
				{
					g_vol_table[x]     = (x*(int)vol)/65536;
					g_vol_table[x+256] = (x*(int)vo2)/65536;
				}
      } else {
				short *vol_tab16 = (short *)g_vol_table;
				if (g_nch==1) for (x = 0; x < 32769; x ++) vol_tab16[x] = -(x*a_v)/256;
				else for (x = 0; x < 32769; x ++)
				{
					vol_tab16[x]       = -(x*(int)vol)/65536;
					vol_tab16[x+32769] = -(x*(int)vo2)/65536;
				}
			}
      LeaveCriticalSection(&g_cs);
    }
	}
}


void PCM_AudioOut::flush(unsigned int time_ms)
{
	int x;
	EnterCriticalSection(&g_cs);
	g_outtime_bytes=g_writetime_bytes=MulDiv(time_ms,g_bytes_per_sec,1000);
	waveOutReset(g_hWaveOut);
	for (x = 0; x < MAX_NUM_BLOCKS; x++)
		if ((g_wave_headers[x].dwFlags & WHDR_PREPARED))
		{
			waveOutUnprepareHeader(g_hWaveOut,&g_wave_headers[x],sizeof(g_wave_headers[0]));
			g_wave_headers[x].dwFlags =0;
		}
	while (WaitForSingleObject(g_hSem,0)==WAIT_OBJECT_0);
	g_prebuf=g_buffer_length/8;
	g_buffer_read=g_buffer_write=g_buffer;
	g_writeall=g_wavecnt=g_buffer_valid=g_buffer_inlength=0;
  ispcnt=0;
	LeaveCriticalSection(&g_cs);
}

unsigned int PCM_AudioOut::getwritepos(void)
{
  return MulDiv(g_writetime_bytes,1000,g_bytes_per_sec);
}

unsigned int PCM_AudioOut::getpos(void)
{
	unsigned int t;
	if (!g_pause) 
	{
		t=GetTickCount()-g_outtime_interp;
		if (t > 1000) t=1000;
	}
	else t=0;
	return t+MulDiv(g_outtime_bytes,1000,g_bytes_per_sec);
}

DWORD WINAPI PCM_AudioOut::_threadproc(LPVOID p)
{
  return ((PCM_AudioOut *)p)->ThreadP();
}

DWORD PCM_AudioOut::ThreadP()
{
	HANDLE hs[2]={g_hSem,g_hEvent};
	while (1) 
	{
		int i;
		i=WaitForMultipleObjects(2,hs,FALSE,INFINITE);
		if (g_quit_flag) break;
		if (i == WAIT_OBJECT_0)
		{
			int x;
			for (x = 0; x < MAX_NUM_BLOCKS && !(g_wave_headers[x].dwFlags & WHDR_DONE); x++);
			if (x < MAX_NUM_BLOCKS) 
			{
				EnterCriticalSection(&g_cs);
				if (g_wave_headers[x].dwFlags & WHDR_DONE)
				{
					int r=g_wave_headers[x].dwBufferLength;
					g_outtime_interp=GetTickCount();
					g_buffer_valid   -=r;
					g_buffer_inlength-=r;
					g_outtime_bytes  +=r;
					waveOutUnprepareHeader(g_hWaveOut,&g_wave_headers[x],sizeof(g_wave_headers[0]));
					g_wave_headers[x].dwFlags=0;
					g_wavecnt--;
					i++;
				}
				LeaveCriticalSection(&g_cs);
			}
		}
		if (i == WAIT_OBJECT_0+1 && !g_pause)
		{
			int l;
			int m;
			int t=num_blocks;
			EnterCriticalSection(&g_cs);
			l=g_buffer_valid-g_buffer_inlength;
			LeaveCriticalSection(&g_cs);
		again:
			if (g_quit_flag) break;
			if (g_writeall && l<max_blocksize) 
      {
        ispcnt=0;
        g_writeall=0;
        m=1;
      }
			else
			{
				m=MAX(MIN(g_buffer_length/2,min_blocksize),g_prebuf);
				l&=~1023;
			}
			if (l >= m && g_wavecnt < t)
			{
				int x;
				if (l > max_blocksize) l=max_blocksize;
				for (x = 0; x < t && (g_wave_headers[x].dwFlags & WHDR_PREPARED); x++);
				if (x < t)
				{
					int ml=(g_buffer+g_buffer_length)-g_buffer_read;
          if (l > g_buffer_length) l=g_buffer_length;
					if (l>ml) 
          {
            int addlen=l-ml;
            if (addlen > 65536) addlen=65536;
            if (addlen > g_buffer_length) addlen=g_buffer_length;
            memcpy(g_buffer+g_buffer_length,g_buffer,addlen);
          }

					g_wave_headers[x].dwBufferLength=l;
					g_wave_headers[x].lpData=g_buffer_read;

          if (waveOutPrepareHeader(g_hWaveOut,&g_wave_headers[x],sizeof(g_wave_headers[0])) == MMSYSERR_NOERROR)
					{
            do_samples_altvol(g_wave_headers[x].lpData,g_wave_headers[x].dwBufferLength);

						if (waveOutWrite(g_hWaveOut,&g_wave_headers[x],sizeof(g_wave_headers[0])) == MMSYSERR_NOERROR)
						{
							g_prebuf=0;

							g_wavecnt++;

							g_buffer_inlength += l;
							g_buffer_read += l;
							
							if (g_buffer_read >= g_buffer+g_buffer_length) g_buffer_read-=g_buffer_length;

							if (g_wavecnt < t)
							{
								EnterCriticalSection(&g_cs);
								l=g_buffer_valid-g_buffer_inlength;
								LeaveCriticalSection(&g_cs);
								if (l >= m) goto again;
							}
						}
						else 
						{
							waveOutUnprepareHeader(g_hWaveOut,&g_wave_headers[x],sizeof(g_wave_headers[0]));
							g_wave_headers[x].dwFlags=0;
						}
					}
					else g_wave_headers[x].dwFlags=0;
				}
			} 
		}
	}
	g_quit_flag=2;
	return 0;
}

void PCM_AudioOut::do_samples_altvol(char *in, int blen)
{
	if ((a_v != 255 || a_p) && g_vol_table) 
	{
		EnterCriticalSection(&g_cs);
	  if (g_bps == 8) 
		{
			unsigned char *i=(unsigned char *)in;
			int x = blen;
			if (g_nch==1)
			{
				while (x--)	{ *i = g_vol_table[*i]; i ++; }
			}
			else
			{
				x>>=1;
				while (x--)
				{
					i[0] = g_vol_table[i[0]];
					i[1] = g_vol_table[i[1] + 256];
					i+=2;
				}
			}
		}
		else if (g_bps == 16) 
		{
			short *i = (short *) in;
			short *tab= (short *)g_vol_table;
			int x = blen>>1;
			if (g_nch==1)
			{
				while (x--)
				{
					int a = i[0];
					if (a <= 0)	i[0] = tab[-a];
					else i[0] = -tab[a];
					i++;
				}
			}
			else
			{
				x>>=1;
				while (x--)
				{
					int a = i[0];
					if (a <= 0)	i[0] = tab[-a];
					else i[0] = -tab[a];
					a=i[1];
					if (a <= 0)	i[1] = tab[32769-a];
					else i[1] = -tab[32769+a];
					i+=2;
				}
			}
		}
	  LeaveCriticalSection(&g_cs);
	}
}

void PCM_AudioOut::setvolume(int volume)
{
	if (volume >= 0 && volume <= 255) a_v=volume;
	_setvol();
}

void PCM_AudioOut::setpan(int pan)
{
	if (pan >= -255 && pan <= 255) a_p=pan;
	_setvol();
}


IAudioOutput *PCMOUT_CREATE(unsigned int outfmt[8])
{
  if (outfmt[0] != NSV_MAKETYPE('P','C','M',' ') ||
      !outfmt[1] || !outfmt[2] || !outfmt[3]) return NULL;
  PCM_AudioOut *p=new PCM_AudioOut(outfmt[1],outfmt[2],outfmt[3]);
  if (p->open_success()) return p;
  delete p;
  return NULL;
}