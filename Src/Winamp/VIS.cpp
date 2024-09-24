#include <windowsx.h>

#include "Main.h"
#include "vis.h"
#include "fft.h"
#include <bfc/platform/types.h>
#include <math.h>
#include <assert.h>
#include "./api.h"
#include "../nsutil/window.h"
#include "../nu/threadname.h"

static winampVisModule *vis_mod;
static DWORD WINAPI vis_thread(void *tmp);
static HANDLE hThread;
static DWORD visThreadId=0;
static volatile int killThread;
static int _nch = 2, _numframes=1;
int _srate = 44100;
static wchar_t _visplugin_name[512];
static int _visplugin_num;
static char *vsa_data;
static int vsa_position, vsa_entrysize=577*4;
static int vsa_length,size=576*4;
static int vis_stopping;
static CRITICAL_SECTION cs;
static HWND external_window = NULL;
static HWND external_window_host = NULL;

#ifdef _M_IX86
__inline static int lrint(float flt)
{
	int intgr;

	_asm
	{
		fld flt
		fistp intgr
	}

	return intgr;
}
#else
__inline static int lrint(float flt)
{
	return (int)flt;
}
#endif

//  quantizes to 23 bits - use appropriately
#define FASTMIN(x,b) { x = b - x;   x += (float)fabs(x);   x *= 0.5f;   x = b - x; }

static size_t vis_refCount=0;
static winampVisModule *GetVis()
{
	winampVisModule *ret = 0;
	EnterCriticalSection(&cs);
	if (vis_stopping)
	{
		LeaveCriticalSection(&cs);
		return 0;
	}
	if (vis_mod)
	{
		ret = vis_mod;
		vis_refCount++;
	}
	LeaveCriticalSection(&cs);
	return ret;
}

static void DestroyVis()
{
	killThread=1;
	vis_stopping=1;

	if (GetCurrentThreadId() != visThreadId)
	{
		HANDLE thisThread = hThread;
		LeaveCriticalSection(&cs);

		// run message pump. this shouldn't last long.
		int x=200;
		while (WaitForSingleObject(thisThread,10) == WAIT_TIMEOUT && x-- > 0)
		{
			WASABI_API_APP->app_messageLoopStep();
		}
		WaitForSingleObject(thisThread,INFINITE);
		EnterCriticalSection(&cs);
	}

	CloseHandle(hThread);
	if (vis_mod)
		FreeLibrary(vis_mod->hDllInstance);
	vis_stopping=0;
	vis_mod=0;
	
	hThread=0;
	killThread=0;
}

static void ReleaseVis(winampVisModule *vis)
{
	if (vis)
	{
		EnterCriticalSection(&cs);
		vis_refCount--;
		if (vis_refCount == 0)
		{
			DestroyVis();
		}

		LeaveCriticalSection(&cs);
	}
}

void vis_init(void)
{
	InitializeCriticalSectionAndSpinCount(&cs, 4000);
}

static char *vsa_get(int timestamp);
static void vsa_setdatasize();

int vis_running()
{
	int running=0;
	winampVisModule *vis = GetVis();
	if (vis)
	{
		running = !vis_stopping;
		ReleaseVis(vis);
	}
	return running;
}

static int priorities[5] =
{
	THREAD_PRIORITY_LOWEST,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_HIGHEST
};

void vis_start(HWND hwnd, wchar_t *fn)
{
	if (vis_stopping || g_safeMode) return;
	vis_stop();
	vsa_deinit();
	if (!config_visplugin_name[0]) return;

	killThread=0;
	if (!fn || !*fn)
	{
		PathCombineW(_visplugin_name, VISDIR, config_visplugin_name);
		_visplugin_num=config_visplugin_num;
	}
	else
	{
		wchar_t buf[MAX_PATH] = {0};
		wchar_t *p;
		StringCchCopyW(buf,MAX_PATH,fn);
		p=wcsstr(buf,L",");
		if (p)
		{
			*p++=0;
			_visplugin_num=_wtoi(p);
		}
		else _visplugin_num=0;
		if (PathIsFileSpecW(buf) || PathIsRelativeW(buf))
			PathCombineW(_visplugin_name, VISDIR, buf);
		else
			StringCchCopyW(_visplugin_name,512,buf);
	}
	hThread = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) vis_thread,NULL,0,&visThreadId);
	SetThreadPriority(hThread,priorities[config_visplugin_priority]);
}

void vis_setprio()
{
	if (vis_stopping) return;
	if (hThread) SetThreadPriority(hThread,priorities[config_visplugin_priority]);
}

void vis_stop()
{
	if (vis_stopping||!hThread) return;
	EnterCriticalSection(&cs); // go into critical section so vis_mod doesn't suddenly appear out of nowhere
	winampVisModule *thisVis = vis_mod;
	LeaveCriticalSection(&cs);
	ReleaseVis(thisVis); 
}

void vis_setinfo(int srate, int nch)
{
	if (srate > 0) _srate = srate;
	if (nch > 0) _nch = nch;
	if (!vis_running()) return;
	EnterCriticalSection(&cs);
	winampVisModule *vis = GetVis();
	if (vis)
	{
		vis->sRate = _srate;
		vis->nCh = _nch;
		ReleaseVis(vis);
	}
	LeaveCriticalSection(&cs);
}

void vis_setextwindow(HWND hwnd)
{
	HWND test_window;
	unsigned int test_window_style_ex;
	unsigned long window_thread_id;

	EnterCriticalSection(&cs);

	external_window = hwnd;
	external_window_host = external_window;

	window_thread_id = GetWindowThreadProcessId(external_window, NULL);
	while(NULL != external_window_host)
	{
		test_window = GetAncestor(external_window_host, GA_PARENT);
		if (NULL != test_window && 
			window_thread_id == GetWindowThreadProcessId(test_window, NULL))
		{
			test_window_style_ex = (unsigned int)GetWindowLongPtrW(test_window, GWL_STYLE);
			if (0 != (WS_EX_CONTROLPARENT & test_window_style_ex))
			{
				external_window_host = test_window;
				continue;
			}
		}
		break;		
	}
	
	LeaveCriticalSection(&cs);
}

static winampVisModule *CreateVis(HINSTANCE visLib)
{
	EnterCriticalSection(&cs);
	winampVisModule *ret = 0;
	ret = GetVis();
	if (ret)
	{
		LeaveCriticalSection(&cs);
		return ret;
	}

	winampVisGetHeaderType pr;
	pr = (winampVisGetHeaderType) GetProcAddress(visLib,"winampVisGetHeader");
	if (!pr)
	{
		LeaveCriticalSection(&cs);
		return 0;
	}

	winampVisHeader* pv = pr(hMainWindow);
	if (!pv)
	{
		LeaveCriticalSection(&cs);
		return 0;
	}

	vis_mod = pv->getModule(_visplugin_num);
	if (!vis_mod)
	{
		LeaveCriticalSection(&cs);
		return 0;
	}

	vis_mod->sRate = _srate;
	vis_mod->nCh = _nch;
	vis_mod->hwndParent = hMainWindow;
	vis_mod->hDllInstance = visLib;

	vis_refCount++;
	LeaveCriticalSection(&cs);
	return vis_mod;
}

static BOOL vis_process_message(MSG *msg)
{
	if (msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST &&
		msg->hwnd == external_window &&
		NULL != external_window)	
	{
		return FALSE;
	}

	if (WM_MOUSEWHEEL == msg->message && 
		NULL != external_window_host)
	{
		POINT cursor;
		HWND target_window;
		
		POINTSTOPOINT(cursor, msg->lParam);
		target_window = WindowFromPoint(cursor);

		if (NULL != target_window && 
			FALSE == IsChild(external_window_host, target_window ) &&
			GetWindowThreadProcessId(target_window, NULL) != GetWindowThreadProcessId(external_window_host, NULL))
		{
			PostMessageW(hMainWindow, msg->message, msg->wParam, msg->lParam);
			return TRUE;
		}

	}

	if (NULL != external_window_host)
		return IsDialogMessageW(external_window_host, msg);

	return FALSE;
}

static DWORD WINAPI vis_thread(void *tmp)
{
	winampVisModule *vis = 0;
	MSG Msg;
	HINSTANCE hLib=0;
	int t=0;
	SetThreadName((DWORD)-1, "Vis (plugin) thread");
	hLib = LoadLibrary(_visplugin_name);
	if (!hLib) 
	{
		t=1;
	}
	else 
	{
		vis = CreateVis(hLib);
		if (!vis)
		{
			FreeLibrary(hLib);
			hLib = 0;
			t=1;
		}
	}

	if (!t)
	{
		if (!(config_no_visseh&1))
		{
			__try
			{
				t = (vis ? vis->Init(vis) : 1);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				t=1;
				char errstr[512] = {0};
				char caption[512] = {0};
				getString(IDS_PLUGINERROR,errstr,512);
				StringCchCatA(errstr, 512, " (1)");
				MessageBoxA(NULL,errstr,getString(IDS_ERROR,caption,512),MB_OK|MB_ICONEXCLAMATION);
			}
		}
		else
		{
			t = vis->Init(vis);
		}
	}

	if (!t)
	{
		if (config_disvis) sa_setthread(0);
		vsa_setdatasize();
	
		while (!killThread)
		{
			if (PeekMessage(&Msg,NULL,0,0,PM_REMOVE))
			{
				if (Msg.message == WM_QUIT) 
					break;

				if (FALSE == vis_process_message(&Msg))
				{
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
			}
			else if (!paused)
			{
				static int upd;
				int p=playing;
				char *data=0;
				if (in_mod && p) data = vsa_get(in_mod->GetOutputTime()+vis->latencyMs);
				if (data)
				{
					int l=vis->spectrumNch;
					for (int n = 0; n < l; n ++)
					{
						memcpy(vis->spectrumData[n],data,576);
						data += 576;
					}
					l=vis->waveformNch;
					for (int n = 0; n < l; n ++)
					{
						memcpy(vis->waveformData[n],data,576);
						data += 576;
					}
				}
				if (!data)
				{
					memset(vis->spectrumData,0,576*2);
					memset(vis->waveformData,0,576*2);
				}
				if (p) upd=1;
				if (upd)
					if (!(config_no_visseh&1))
					{
						__try
						{
							if (vis->Render(vis))
							{
								break;
							}
						}
						__except(EXCEPTION_EXECUTE_HANDLER)
						{
							char errstr[512] = {0};
							char caption[512] = {0};
							getString(IDS_PLUGINERROR,errstr,512);
							StringCchCatA(errstr, 512, " (2)");
							MessageBoxA(NULL,errstr,getString(IDS_ERROR,caption,512),MB_OK|MB_ICONEXCLAMATION);
							break;
						}
					}
					else
					{
						if (vis->Render(vis)) 
						{
							break;
						}
					}
				if (!p) upd=0;
				Sleep(vis->delayMs);
			}
			else Sleep(min(1,vis->delayMs));
		}
		vsa_deinit();
		sa_setthread(config_sa);
		if (!(config_no_visseh&1))
		{
			__try
			{
				vis->Quit(vis);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				wchar_t errstr[512] = {0};
				wchar_t caption[512] = {0};
				getStringW(IDS_PLUGINERROR,errstr,512);
				StringCchCatW(errstr, 512, L" (3)");
				MessageBoxW(NULL,errstr,getStringW(IDS_ERROR,caption,512),MB_OK|MB_ICONEXCLAMATION);
			}
		}
		else
		{
			vis->Quit(vis);
		}
		EnterCriticalSection(&cs);
		if (killThread)
		{
			LeaveCriticalSection(&cs);
			return 0;
		}
		else
		{
			ReleaseVis(vis);
			LeaveCriticalSection(&cs);
			return 0;
		}
	}
	
	ReleaseVis(vis);
	if (hLib)
		FreeLibrary(hLib);
	hLib = 0;
	return 0;
}

static int last_pos;

static void _vsa_init()
{
	vsa_deinit();
	if (_numframes < 1) _numframes=1;
	vsa_entrysize = 4+size;
	vsa_data = (char *) GlobalAlloc(GPTR,vsa_entrysize * _numframes);
	vsa_position=0;
	vsa_length = _numframes;
}

void vsa_init(int numframes)
{
	EnterCriticalSection(&cs);
	if (vis_running())
	{
		last_pos=0;
		_numframes=numframes;
		_vsa_init();
	}
	else
	{
		last_pos=0;
		_numframes=numframes;
	}
	LeaveCriticalSection(&cs);
}

static void vsa_setdatasize()
{
	EnterCriticalSection(&cs);
	winampVisModule *vis = GetVis();
	if (vis)
	{
		size=576*(vis->waveformNch+vis->spectrumNch);
		_vsa_init();
		ReleaseVis(vis);
	}

	LeaveCriticalSection(&cs);
}

int vsa_add(void *data, int timestamp)
{
	if (!vsa_data) return 1;
	EnterCriticalSection(&cs);
	if (vsa_data) // check again, it might have gone away while we were waiting on the CS
	{
		if (vsa_length < 2)
		{
			vsa_position = 0;
		}
		char *c = vsa_data + vsa_position*vsa_entrysize;
		*(int32_t *)c=timestamp;

		memcpy(c+4,data,vsa_entrysize-4);
		if (++vsa_position >= vsa_length) vsa_position -= vsa_length;
		LeaveCriticalSection(&cs);
		return 0;
	}
	else
	{
		LeaveCriticalSection(&cs);
		return 1;
	}
}

void vsa_deinit(void)
{
	EnterCriticalSection(&cs);
	if (vsa_data)
	{
		GlobalFree(vsa_data);
		vsa_data=0;
		vsa_length=0;
	}
	LeaveCriticalSection(&cs);
}

static char *vsa_get(int timestamp)
{
	int i,x, closest=1000000, closest_v = -1;
	if (!vsa_data) return NULL;
	if (vsa_length<2)
	{
		return vsa_data+4;
	}
	EnterCriticalSection(&cs);
	x=last_pos;
	if (x >= vsa_length) x=0;
	for (i = 0; i < vsa_length; i ++)
	{
		int *q = (int *)(vsa_data+x*vsa_entrysize);
		int d = timestamp-*q;
		if (++x == vsa_length) x=0;
		if (d < 0) d = -d;
		if (d < closest)
		{
			closest = d;
			closest_v = x;
		}
		else if (closest<200) break;
	}
	if (closest_v >= 0)
	{
		static char data[576*4];
		last_pos=closest_v;
		memcpy(data,vsa_data+vsa_entrysize*closest_v+4,vsa_entrysize-4);
		LeaveCriticalSection(&cs);
		return data;
	}
	else
	{
		LeaveCriticalSection(&cs);
		return 0;
	}
}

int vsa_getmode(int *sp, int *wa)
{
	int rv=0;
		winampVisModule *vis = GetVis();
	if (vis)
	{
		EnterCriticalSection(&cs);
		*sp=vis->spectrumNch;
		*wa=vis->waveformNch;
		rv=1;

		LeaveCriticalSection(&cs);
		ReleaseVis(vis);
	}
	else *sp=*wa=0;
	return rv;
}

void FillRealSamples_8Bit(unsigned char *data, const int stride, const int channels, float *samples, const float divider)
{
	int frame,c;
	const float p = (float)channels*divider;

	for (frame = 0; frame <512; frame ++)
	{
		//done by memset - samples[x*2]=0;
		for (c=0;c<channels;c++)
		{
			samples[frame] += (float)(*data - 128);
			data+=stride; // jump to the next sample (channels are interleaved)
		}

		samples[frame] /= p;
		//done by memset - wavetrum[x*2+1] = 0.0f;
	}
	nsutil_window_Hann_F32_IP(samples, 512);
}

#define SA_DC_FILTER
void FillRealSamples(char *ptr, const int stride, const int channels, float *samples, const float divider)
{
#ifdef SA_DC_FILTER
	float x1=0, y1=0;
#endif
	int frame, c;
	const float p=(float)channels * divider;

	// we're calculating using only the most significant byte,
	// because we only end up with 6 bit data anyway
	// if you want full resolution, check out CVS tag BETA_2005_1122_182830, file: vis.c

	for (frame = 0;frame <512;frame++)
	{
		//done by memset - wavetrum[x*2]=0;
		float x=0;
		for (c=0;c<channels;c++)
		{
			x += (float)(*ptr);
			ptr+=stride; // jump to the next sample (channels are interleaved)
		}
#ifdef SA_DC_FILTER
		float y = x - x1 + 0.99f * y1;
		y1=y;
		x1=x;
#else
		float y=x;
#endif
		y/=p;

		samples[frame]=y;
		//done by memset - wavetrum[x*2+1] = 0.0f;
	}
		nsutil_window_Hann_F32_IP(samples, 512);
}

void vsa_addpcmdata(void *_data_buf, int numChannels, int numBits, int ts)
{
	char *data_buf = reinterpret_cast<char *>(_data_buf);
	// begin vis plugin stuff
	winampVisModule *vis = GetVis();
	if (vis)
	{
		__declspec(align(32)) float wavetrum[512];
		extern int vsa_add(void *data, int timestamp);
		char data[576*4*2] = {0};
		int data_offs=0;
		int y,x,spectrumChannels, waveformChannels, stride;

		spectrumChannels=min(numChannels,vis->spectrumNch);
		stride=numBits/8;

		for (y = 0; y < spectrumChannels; y ++)
		{
			if (spectrumChannels == 1) // downmix to mono, if necessary
			{
				if (numBits == 8)
					FillRealSamples_8Bit((unsigned char*)data_buf, 1, numChannels, wavetrum, 1.f);
				else
				{
					const int stride=numBits/8; // number of bytes between samples
					char *ptr = data_buf+y*stride+stride-1; // offset for little endian
					FillRealSamples(ptr, stride, numChannels, wavetrum, 1.f);
				}
			}
			else // TODO: deal with 'downmixing' to stereo if channels>2
			{
				if (numBits == 8)
					FillRealSamples_8Bit((unsigned char*)data_buf, numChannels, 1, wavetrum, 1.f);
				else
				{
					const int stride=numBits/8; // number of bytes between samples
					char *ptr = data_buf+y*stride+stride-1; // offset for little endian
					FillRealSamples(ptr, stride*numChannels, 1, wavetrum, 1.f);
				}
			}
			fft_9(wavetrum);
			{
				float la=0;
				int thisBand=0;

				for (x = 0; x < 256; x ++)
				{
					float sinT = wavetrum[x*2];
					float cosT = wavetrum[x*2+1];

					float thisValue=(float)sqrt(sinT*sinT+cosT*cosT)/16.0f;
					thisBand++;

					FASTMIN(thisValue, 255.f);
					data[data_offs++] = lrint((thisValue + la)/2.f);
					//data[data_offs++] = lrint((thisValue + thisValue + la)/3.f);
					data[data_offs++] = lrint(thisValue);
					la=thisValue;
				}
				while ((data_offs % 576)!=0)
				{
					la/=2;
					data[data_offs++]=lrint(la);
				}

				assert((data_offs % 576)==0);
			}
		}

		if (numChannels == 1 && vis->spectrumNch == 2) // upmix, if necessary
		{
			memcpy(data+data_offs,data+data_offs-576,576);
			data_offs+=576;
		}

		waveformChannels=min(numChannels,vis->waveformNch);
		if (waveformChannels == 1) 		// downmix to mono, if necessary
		{
			char *ptr = data_buf+stride-1; // offset for little endian
			for (x=0;x<576;x++)
			{
				__int32 mix=0;
				for (int channel=0;channel<numChannels;channel++)
				{
					mix += (*ptr);
					ptr+=stride; // jump to the next sample (channels are interleaved)
				}
				data[data_offs++] = (char)(mix / numChannels);
			}
		}
		else // TODO: deal with 'downmixing' to stereo if numChannels>2
		{
			for (y = 0; y < waveformChannels; y++)
			{
				char *ptr = data_buf+y*stride+stride-1; // offset for little endian
				for (x=0;x<576;x++)
				{
					data[data_offs++] = *ptr;
					ptr+=stride*numChannels;
				}
			}
		}

		if (numChannels == 1 && vis->waveformNch == 2)
		{
			memcpy(data+data_offs,data+data_offs-576,576);
			//data_offs+=576;
		}
		vsa_add(data,ts);
		ReleaseVis(vis);
	}
}

HWND hVisWindow, hPLVisWindow;

LRESULT CALLBACK VIS_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_LBUTTONDBLCLK:
		{
			RECT r1, r2;
			int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor
			int yPos = GET_Y_LPARAM(lParam);
			if (hwnd == hVisWindow)
			{
				GetWindowRect(hMainWindow,&r1);
			}
			else GetWindowRect(hPLWindow,&r1);
			GetWindowRect(hwnd,&r2);
			xPos += r2.left-r1.left;
			yPos += r2.top-r1.top;
			lParam = MAKELPARAM(xPos,yPos);
			SendMessageW(hwnd == hVisWindow?hMainWindow:hPLWindow,message,wParam,lParam);
			return 0;
		}
		case WM_USER+0xebe:
		case WM_DROPFILES:
			return SendMessageW(GetParent(hwnd),message,wParam,lParam);
		case WM_CREATE:
			if (NULL != WASABI_API_APP)
				WASABI_API_APP->app_registerGlobalWindow(hwnd);
			break;
		case WM_DESTROY:
			if (NULL != WASABI_API_APP)
				WASABI_API_APP->app_unregisterGlobalWindow(hwnd);
			break;
	}

	if (FALSE != IsDirectMouseWheelMessage(message))
	{
		SendMessageW(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}

	return DefWindowProcW(hwnd,message,wParam,lParam);
}