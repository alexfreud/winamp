//#define PLUGIN_NAME "Nullsoft DirectShow Decoder"
#define PLUGIN_VERSION L"1.15"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <assert.h>
#include "../Agave/Language/api_language.h"
#include <api/service/waServiceFactory.h>
#include "resource.h"
#include "../nu/ns_wc.h"

#define IPC_GETINIFILE 334 // returns a pointer to winamp.ini
#define WM_WA_IPC WM_USER

#define VIDUSER_SET_TRACKSELINTERFACE 0x1003 // give your ITrackSelector interface as param2
#define VIDUSER_SET_INFOSTRING 0x1000

#define DEFGUID 1

#include <AtlBase.h>
//#include <streams.h>
//#include <qedit.h>
#include <qnetwork.h>
#ifdef DEFGUID
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.
#endif

#include "main.h"

#include "CWAAudioRenderer.h"
#include "CWAVideoRenderer.h"
#include "header_asf.h"
#include "header_avi.h"
#include "header_mpg.h"
#include "header_wav.h"
#include "../Winamp/wa_ipc.h"
#include "../nsutil/pcm.h"

static Header *infoHeader=0;
wchar_t *infoFn=0;

DEFINE_GUID(IID_IAMNetworkStatus,0xFA2AA8F3L,0x8B62,0x11D0,0xA5,0x20,0x00,0x00,0x00,0x00,0x00,0x00);

// post this to the main window at end of file (after playback as stopped)
#define WM_WA_MPEG_EOF WM_USER+2
#define IPC_GET_IVIDEOOUTPUT 500
#define VIDEO_MAKETYPE(A,B,C,D) ((A) | ((B)<<8) | ((C)<<16) | ((D)<<24))
#define VIDUSER_SET_VFLIP      0x1002

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
HINSTANCE g_hInstance=0;

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hInstance=hInstance;
	}
	return TRUE;

}

int GetFileLength(const wchar_t *filename)
{
	static wchar_t fn[MAX_PATH*4] = L"";
	static int l = -1;
	if(!_wcsicmp(filename,fn)) return l;
	//return -1000;

	CComPtr<IGraphBuilder> graph;
	graph.CoCreateInstance(CLSID_FilterGraph);
	if (graph)
	{
		HRESULT hr;
		try
		{
			hr = graph->RenderFile(filename,NULL);
		}
		catch (...)
		{
			return -1000;
		}
		if (hr == S_OK)
		{
			CComPtr<IMediaPosition> pMediaPosition;
			graph->QueryInterface(IID_IMediaPosition, (void **)&pMediaPosition);
			if (pMediaPosition)
			{
				REFTIME length;
				pMediaPosition->get_Duration(&length);
				lstrcpynW(fn,filename,sizeof(fn)/sizeof(wchar_t));
				l = (int)(length*1000.0);
				return (int)(length*1000.0);
			}
		}
	}
	return -1000;
}

wchar_t lastfn[MAX_PATH] = {0};	// currently playing file (used for getting info on the current file)

int file_length;		// file length, in bytes
int decode_pos_ms;		// current decoding position, in milliseconds.
// Used for correcting DSP plug-in pitch changes
int paused;				// are we paused?
volatile int seek_needed; // if != -1, it is the point that the decode
// thread should seek to, in ms.

HANDLE input_file=INVALID_HANDLE_VALUE; // input file handle

volatile int killDecodeThread=0;			// the kill switch for the decode thread
HANDLE thread_handle=INVALID_HANDLE_VALUE;	// the handle to the decode thread

const char *INI_FILE;

wchar_t m_lastfn[2048] = {0}; // currently playing file (used for getting info on the current file)
wchar_t m_status[512] = {0};
DWORD m_laststatus;

int m_bitrate=0;

//config.cpp
extern void doConfig(HINSTANCE hInstance, HWND hwndParent);
extern void config_read();
extern char *getfileextensions();
extern void getInfo(const wchar_t *fn, wchar_t *linetext, int linetextCch, wchar_t *fulltext, int fulltextCch, int *bitrate, int *channel);

//info.cpp
extern void doInfo(HINSTANCE hInstance, HWND hwndParent, const wchar_t *fn);

int getoutputtime();
void releaseObjects();

IVideoOutput *m_video_output;

IGraphBuilder *pGraphBuilder=0;
static ICaptureGraphBuilder2 *pCapture=0;
static IBaseFilter *pNullFilter2=0;
static IMediaEvent *pMediaEventEx=0;
IMediaControl *pMediaControl=0;
static IMediaSeeking *pMediaSeeking=0;

static IBaseFilter *pCapVidSrcFilter=0;
static IBaseFilter *pCapAudSrcFilter=0;

bool has_audio, has_video, has_palette;
int video_mediatype;
int video_w,video_h,video_len;
int audio_bps, audio_srate, audio_nch;
RGBQUAD palette[0x100] = {0}; // for RGB8
int m_length=-1;
int g_quit;
DWORD m_starttime,m_time_paused;
unsigned int m_nbframes;
DWORD m_avgfps_start;
int m_float, m_src_bps;
#ifdef IN_DSHOW_CAPTURE_SUPPORT // disable capture, for now
int m_is_capture;
#endif
HWND m_notif_hwnd;

ITrackSelector *pTrackSelector = NULL;

static int doingaudioshit=0;

bool s_using_dsr = false;

wchar_t lastfn_status[256]=L"";
static LONG_PTR m_buffering=0;
int g_bufferstat;

//capture stuff
#include "../nsv/nsvbs.h"

nsv_InBS g_video_refs;
#define G_MAX_FREE_FRAMES 64
void *g_free_frames[G_MAX_FREE_FRAMES];
int g_num_free_frames;

#define PA_CLIP_( val, min, max )\
	{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

#if !defined(__alpha) && !defined(_WIN64)
static __inline long float_to_long(double t)
{
	long r;
	__asm fld t
	__asm fistp r
	return r;
}
#else
#define float_to_long(x) ((long)( x ))
#endif

inline static void clip(double &x, double a, double b)
{
	double x1 = fabs(x - a);
	double x2 = fabs(x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
}

template <typename FLOAT_T>
void Float32_To_Int24_Clip(void *destinationBuffer, FLOAT_T *sourceBuffer, unsigned int count, double gain)
{
	FLOAT_T *src = sourceBuffer;
	unsigned char *dest = (unsigned char*)destinationBuffer;

	gain*=65536. * 32768.0;
	while (count--)
	{
		/* convert to 32 bit and drop the low 8 bits */
		double scaled = *src * gain;
		clip(scaled, -2147483648., 2147483647.);
		signed long temp = (signed long) scaled;

		dest[0] = (unsigned char)(temp >> 8);
		dest[1] = (unsigned char)(temp >> 16);
		dest[2] = (unsigned char)(temp >> 24);

		src++;
		dest += 3;
	}
}

template <typename FLOAT_T>
void Float32_To_Int16_Clip(void *destinationBuffer, FLOAT_T *sourceBuffer, unsigned int count, double gain)
{
	FLOAT_T *src = sourceBuffer;
	signed short *dest = (signed short*)destinationBuffer;
	gain*=32768.0;
	while (count--)
	{
		long samp = float_to_long((*src) * gain/* - 0.5*/);

		PA_CLIP_(samp, -0x8000, 0x7FFF);
		*dest = (signed short) samp;

		src ++;
		dest ++;
	}
}


class CAudioGrab : public CSampleCB
{
public:
	CAudioGrab() { }

	void sample_cb(LONGLONG starttime, LONGLONG endtime, IMediaSample *pSample)
	{
		if (g_quit) return;
		doingaudioshit=1;
		int l=pSample->GetActualDataLength();
		if (l)
		{
			unsigned char *b=NULL;
			pSample->GetPointer(&b);
			double t=(double)(starttime)/10000;
			decode_pos_ms=(int)t; //it's a fix so we stay in sync according to what DirectShit is sending us
			if (g_quit)
			{
				doingaudioshit=0;
				return;
			}

			//convert IEEE Floats to PCM
			if (m_float)
			{
				if (m_src_bps==32)
				{
					l/=sizeof(float);
					nsutil_pcm_FloatToInt_Interleaved(b, (const float *)b, audio_bps, l);
					l *= (audio_bps/8);
				}
				else if (m_src_bps==64)
				{
					l/=sizeof(double);
					switch (audio_bps)
					{
					case 16:
						Float32_To_Int16_Clip(b, (double *)b, l, 1.0);
						l*=2;
						break;
					case 24:
						Float32_To_Int24_Clip(b, (double *)b, l, 1.0);
						l*=3;
						break;
					}
				}
			}

			{
				int len=l;
				int s=576*audio_nch*(audio_bps/8);
				while (len>0&&!g_quit)
				{
					if (len>=s)
					{
						mod.SAAddPCMData(b,audio_nch,audio_bps,decode_pos_ms);
						mod.VSAAddPCMData(b,audio_nch,audio_bps,decode_pos_ms);
					}
					int l=min(s,len);
					if (mod.dsp_isactive())
					{
						char *sample_buffer = (char *)alloca(l*2);
						memcpy(sample_buffer,b,l);
						int l2=mod.dsp_dosamples((short *)sample_buffer,l/audio_nch/(audio_bps/8),audio_bps,audio_nch,audio_srate)*(audio_nch*(audio_bps/8));
						while (mod.outMod->CanWrite()<l2 && !g_quit) Sleep(10);
						if (g_quit)
						{
							doingaudioshit=0;
							return;
						}
						mod.outMod->Write(sample_buffer,l2);
					}
					else
					{
						while (mod.outMod->CanWrite()<l && !g_quit) Sleep(10);
						if (g_quit)
						{
							doingaudioshit=0;
							return;
						}
						mod.outMod->Write((char *)b,l);
					}
					//FUCKO:this is clearly having a precision problem
					decode_pos_ms+=((l/audio_nch/(audio_bps/8))*1000)/audio_srate;
					len-=l;
					b+=l;
				}
			}
		}
		doingaudioshit=0;
	}
	void endofstream()
	{
		while (!g_quit && mod.outMod->IsPlaying()) Sleep(10);
		PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
	}
private:
};

CWAAudioRenderer *nullfilter;
CWAVideoRenderer *nullfilter2;

#include "DSTrackSelector.h"
//#define DEBUGVIDEO
#ifdef DEBUGVIDEO
void outputDebugStr(char *str)
{
	FILE *fh=fopen("c:\\dshow_dbg.log","at");
	fprintf(fh,"%s",str);
	fclose(fh);
}
#endif

class CVideoGrab : public CSampleCB
{
public:
	CVideoGrab() {}
	void sample_cb(LONGLONG starttime, LONGLONG endtime, IMediaSample *pSample)
	{
		m_nbframes++;
		if (!m_avgfps_start) m_avgfps_start=(DWORD)GetTickCount64();

		if ((DWORD)GetTickCount64()-m_laststatus>500)
		{
			DWORD t= (DWORD)GetTickCount64()-m_avgfps_start;
			if (t)
			{
				wchar_t text[512] = {0};
				StringCchPrintfW(text,512,L"%s %.02f%s",m_status,(double)m_nbframes*1000/t,WASABI_API_LNGSTRINGW(IDS_FPS));
				m_video_output->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)text,0);
				m_laststatus= (DWORD)GetTickCount64();
			}
		}

		if (g_quit) return;
		unsigned char *b=NULL;
		pSample->GetPointer(&b);
		//wait for the right time
		int evtime=(int)(starttime/10000);

#ifdef IN_DSHOW_CAPTURE_SUPPORT // disable capture, for now
		if (m_is_capture)
		{
			//capture shit

			//check if frame to display
			for (;g_video_refs.avail()>=64;)
			{
				int *ptr = (int *)g_video_refs.getcurbyteptr();
				if (getoutputtime()<ptr[0]) break;
				//display it
				void *b=(void*)ptr[1];
				if (video_mediatype==VIDEO_MAKETYPE('Y','V','1','2'))
				{
					// YV12 needs to pass a YV12_PLANES structure.
					static YV12_PLANES yv12planes;
					int s=video_h*video_w;
					yv12planes.y.baseAddr=(unsigned char *)b;
					yv12planes.y.rowBytes=video_w;
					yv12planes.v.baseAddr=(unsigned char *)b+s;
					yv12planes.v.rowBytes=video_w/2;
					yv12planes.u.baseAddr=(unsigned char *)b+(s*5)/4;
					yv12planes.u.rowBytes=video_w/2;
					m_video_output->draw((void *)&yv12planes);
				}
				else
					m_video_output->draw(b);

				//free frame
				if (g_num_free_frames < G_MAX_FREE_FRAMES)
				{
					g_free_frames[g_num_free_frames++]=b;
				}
				else
				{
					free(b);
				}

				g_video_refs.seek(64);
				g_video_refs.compact();
			}

			//store the frame in buffer

			//alloc frame
			int len=pSample->GetActualDataLength();
			if (!len) //very unlikely but oh well...
			{
				int s=4;
				switch (video_mediatype)
				{
				case VIDEO_MAKETYPE('Y','U','Y','2'):
							case VIDEO_MAKETYPE('Y','V','Y','U'):
								case VIDEO_MAKETYPE('R','G','1','6'):
										s=2;
					break;
				case VIDEO_MAKETYPE('R','G','2','4'):
								s=3;
					break;
				}
				len=video_w*video_h*s;
			}

			void *t;
			if (g_num_free_frames)
	{
				t=g_free_frames[--g_num_free_frames];
				g_free_frames[g_num_free_frames]=0;
			}
			else
				t=malloc(len);

			memcpy(t,b,len);

			g_video_refs.add(&evtime,4);
			g_video_refs.add(&t,4);
			return;
		}

#endif
		if (has_audio)
		{
			//sync based on audio
			if (getoutputtime()>evtime)
			{
				//too late, zap it
				return;
			}
			while (getoutputtime()<evtime && !g_quit) Sleep(1);
		}
		else
		{
			//sync based on time
			while (paused && !g_quit) Sleep(1);
			while ((GetTickCount64()-m_starttime)<(unsigned int)evtime && !g_quit) Sleep(1);
		}
		if (g_quit) return;
		if (video_mediatype==VIDEO_MAKETYPE('Y','V','1','2'))
		{
			// YV12 needs to pass a YV12_PLANES structure.
			static YV12_PLANES yv12planes;
			int s=video_h*video_w;
			yv12planes.y.baseAddr=(unsigned char *)b;
			yv12planes.y.rowBytes=video_w;
			yv12planes.v.baseAddr=(unsigned char *)b+s;
			yv12planes.v.rowBytes=video_w/2;
			yv12planes.u.baseAddr=(unsigned char *)b+(s*5)/4;
			yv12planes.u.rowBytes=video_w/2;
			m_video_output->draw((void *)&yv12planes);
		}
		else
			m_video_output->draw(b);
	}
	void endofstream()
	{
		if (!has_audio) PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
	}
};

const wchar_t *extension(const wchar_t *fn)
{
	const wchar_t *x = PathFindExtensionW(fn);

	if (*x)
		return CharNextW(x);
	else
		return x;
}

DWORD WINAPI DecodeThread(LPVOID b); // the decode thread procedure

void config(HWND hwndParent)
{
	doConfig(WASABI_API_LNG_HINST,hwndParent);
	mod.FileExtensions=getfileextensions();
}

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMSW),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCEW(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirectW(&msgbx);
}

void about(HWND hwndParent)
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_DSHOW_PLUGIN_OLD,text,1024);
	StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					 mod.description, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}

int init()
{
	if (!IsWindow(mod.hMainWindow))
		return IN_INIT_FAILURE;

	INI_FILE = (const char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE);

	config_read();

	waServiceFactory *sf = mod.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,IndshowLangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_DSHOW_PLUGIN),PLUGIN_VERSION);
	mod.description = (char*)szDescription;

	mod.FileExtensions=getfileextensions();

	pTrackSelector = new DSTrackSelector();
	return IN_INIT_SUCCESS;
}

void quit()
{
	if (pTrackSelector) { delete pTrackSelector; pTrackSelector = NULL; }
	if (infoFn) { free(infoFn); infoFn = 0; }
	if (infoHeader) { delete infoHeader; infoHeader = 0; }
}

int isourfile(const wchar_t *fn)
{
	// TODO: re-enable this, but only via an option
#if 0
	if (!strncmp(fn,"mms://",6) || !strncmp(fn,"mmst://",7) || !strncmp(fn,"mmsu://",7))
	{
		if (!strstr(fn,".wma")) return 1;
	}
#endif
#ifdef IN_DSHOW_CAPTURE_SUPPORT // disable capture, for now
	return !strncmp(fn,"cap://",6);
#endif
	return 0;
}

#ifdef DEBUG
HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
	IMoniker * pMoniker;
	IRunningObjectTable *pROT;
	if (FAILED(GetRunningObjectTable(0, &pROT)))
	{
		return E_FAIL;
	}
	WCHAR wsz[256] = {0};
	StringCchPrintfW(wsz, 256, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
	HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
	if (SUCCEEDED(hr))
	{
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
		                    pMoniker, pdwRegister);
		pMoniker->Release();
	}
	pROT->Release();
	return hr;
}
#endif

IBaseFilter* GetCaptureDevice(ICreateDevEnum *pDevEnum, const GUID dwCLSID, int nDeviceSelected)
{
	IBaseFilter *pSrc = NULL;

	IEnumMoniker *pClassEnum = NULL;
	pDevEnum->CreateClassEnumerator(dwCLSID, &pClassEnum, 0);

	ULONG cFetched;
	IMoniker *pMoniker = NULL;
	int nEnumPos = 0;
	while (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)
	{
		if (nEnumPos == nDeviceSelected)
		{
			pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
			pMoniker->Release();
			pClassEnum->Release();
			return pSrc;
		}
		pMoniker->Release();
		nEnumPos++;
	}
	pClassEnum->Release();
	return pSrc;
}

void stop();

void releaseObjects()
{
	if (m_notif_hwnd)
	{
		KillTimer(m_notif_hwnd,111);
		DestroyWindow(m_notif_hwnd);
	}
	m_notif_hwnd=NULL;

	if (pGraphBuilder)
		pGraphBuilder->Release();
	pGraphBuilder=0;

	if (pCapture)
		pCapture->Release();
	pCapture=0;


	if (pMediaEventEx)
		pMediaEventEx->Release();
	pMediaEventEx=0;

	if (pMediaControl)
		pMediaControl->Release();
	pMediaControl=0;

	if (pMediaSeeking)
		pMediaSeeking->Release();
	pMediaSeeking=0;

	if (pCapVidSrcFilter)
		pCapVidSrcFilter->Release();
	pCapVidSrcFilter=0;

	if (pCapAudSrcFilter)
		pCapAudSrcFilter->Release();
	pCapAudSrcFilter=0;

	/*
	if (nullfilter)
	{
		((CBaseFilter *)nullfilter)->Release();
	}
	*/
	nullfilter=0;

	/*
	if (nullfilter2)
	{
		nullfilter2->Release();
		 //TODO: why does this still have 3 refcounts?
	}
	*/
	nullfilter2=0;

}

#define BeginEnumFilters(pFilterGraph, pEnumFilters, pBaseFilter) \
{CComPtr<IEnumFilters> pEnumFilters; \
	if(pFilterGraph && SUCCEEDED(pFilterGraph->EnumFilters(&pEnumFilters))) \
{ \
	for(CComPtr<IBaseFilter> pBaseFilter; S_OK == pEnumFilters->Next(1, &pBaseFilter, 0); pBaseFilter = NULL) \
{ \
 
#define EndEnumFilters }}}

void handleNotifyEvents()
{
	for (;;)
	{
		long evCode;
		LONG_PTR param1, param2;
		HRESULT h = pMediaEventEx->GetEvent(&evCode, &param1, &param2, 0);
		if (FAILED(h)) break;
		switch (evCode)
		{
		case EC_OLE_EVENT:
		{
			char str[MAX_PATH] = {0};
			WideCharToMultiByteSZ(CP_ACP,0,(BSTR)param1,-1,str,MAX_PATH,NULL,NULL);
			if (!lstrcmpiA(str,"URLAndExit"))
			{
				//FUCKO
				/*
				WCHAR str[16384],*str2;
				WCHAR m_filename[MAX_PATH] = {0};
				MultiByteToWideChar( CP_ACP, 0, infos->getFilename(), lstrlen( infos->getFilename() ) + 1, m_filename, sizeof( m_filename ) );
				wcscpy(str,(BSTR)param2);
				wcscat(str,L"&filename=");
				MakeEscapedURL(m_filename,&str2);
				wcscat(str,str2);
				delete str2;
				LaunchURL(str);
				*/
			}
			break;
		}
		case EC_BUFFERING_DATA:
		{
			m_buffering=param1;
			if (!m_buffering)
			{
				lastfn_status[0]=0;
				g_bufferstat=0;
				PostMessage(mod.hMainWindow,WM_USER,0,243);
				break;
			}
		}
		break;
		}
		pMediaEventEx->FreeEventParams(evCode, param1, param2);
	}
}

LRESULT CALLBACK notif_wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg==WM_TIMER && wParam==111 && m_notif_hwnd)
	{
		handleNotifyEvents();
		if (m_buffering)
		{
			BeginEnumFilters(pGraphBuilder, pEF, pBF)
			{
				if (CComQIPtr<IAMNetworkStatus, &IID_IAMNetworkStatus> pAMNS = pBF)
				{
					long BufferingProgress = 0;
					if (SUCCEEDED(pAMNS->get_BufferingProgress(&BufferingProgress)) && BufferingProgress > 0)
					{
						StringCchPrintfW(lastfn_status, 256,WASABI_API_LNGSTRINGW(IDS_BUFFERING),BufferingProgress);
						if (m_video_output) m_video_output->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)lastfn_status,0);

						int bpos=BufferingProgress;
						int csa = mod.SAGetMode();
						char tempdata[75*2]={0,};
						int x;
						if (csa&1)
						{
							for (x = 0; x < bpos*75/100; x ++)
							{
								tempdata[x]=x*16/75;
							}
						}
						if (csa&2)
						{
							int offs=(csa&1) ? 75 : 0;
							x=0;
							while (x < bpos*75/100)
							{
								tempdata[offs + x++]=-6+x*14/75;
							}
							while (x < 75)
							{
								tempdata[offs + x++]=0;
							}
						}
						if (csa==4)
						{
							tempdata[0]=tempdata[1]=(bpos*127/100);
						}
						if (csa) mod.SAAdd(tempdata,++g_bufferstat,(csa==3)?0x80000003:csa);

						PostMessage(mod.hMainWindow,WM_USER,0,243);
						break;
					}
				}
			}
			EndEnumFilters
		}
	}
	return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

int play(const wchar_t *fn)
{
	paused=0;
	decode_pos_ms=0;
	seek_needed=-1;
	m_length=-1;
	g_quit=0;
	lstrcpyn(m_lastfn,fn, 2048);
	m_avgfps_start=0;
	m_nbframes=0;
	m_float=0;
	m_notif_hwnd=NULL;
	lastfn_status[0]=0;
	m_buffering=0;
	g_bufferstat=0;

	double aspect=1.0;

	HRESULT hr;

	assert(pGraphBuilder==0);
	hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void**)&pGraphBuilder);
	if (FAILED(hr))
		return 1;

#ifdef DEBUG
	DWORD dwRegister;
	AddToRot(pGraphBuilder, &dwRegister);
#endif
#ifdef IN_DSHOW_CAPTURE_SUPPORT // disable capture, for now
	m_is_capture=!wcsncmp(fn, L"cap://",6);
#endif
	//  CWAAudioRenderer *nullfilter;
	//  CWAVideoRenderer *nullfilter2;

	has_audio=true; has_video=true; has_palette=false;

	// insert audio renderer
	nullfilter=new CWAAudioRenderer();
	pGraphBuilder->AddFilter(nullfilter,L"Null Audio");
	nullfilter->SetCallback(new CAudioGrab());

	// insert video renderer

	nullfilter2=new CWAVideoRenderer();
	pGraphBuilder->AddFilter(nullfilter2,L"Null Video");
	nullfilter2->SetCallback(new CVideoGrab());

	assert(pMediaEventEx==0);
	pGraphBuilder->QueryInterface(IID_IMediaEvent, (void **)&pMediaEventEx);
	if (!pMediaEventEx)
	{
		releaseObjects();
		return 1;
	}

	m_video_output=(IVideoOutput *)SendMessage(mod.hMainWindow,WM_USER,0,IPC_GET_IVIDEOOUTPUT);
	if (!m_video_output)
	{
		releaseObjects();
		return 1;
	}

	m_video_output->extended(VIDUSER_SET_TRACKSELINTERFACE, (INT_PTR)pTrackSelector, 0);

	//create window that will receive filter notifications
	static int classReg=0;
	if (!classReg)
	{
		WNDCLASS wc={0,};
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = notif_wndProc;
		wc.hInstance = mod.hDllInstance;
		wc.hIcon = NULL;
		wc.hCursor = NULL;
		wc.lpszClassName = L"in_dshowClass";
		if (!RegisterClassW(&wc))
			return 1;
		classReg=1;
	}
	m_notif_hwnd=CreateWindow(L"in_dshowClass",L"dshow_notif",NULL,0,0,1,1,NULL,NULL,mod.hDllInstance,NULL);
	SetTimer(m_notif_hwnd,111,500,0);

#ifdef IN_DSHOW_CAPTURE_SUPPORT // disable capture, for now
	if (m_is_capture)
	{
		//build capture graph
		assert(pCapture==0);
		HRESULT hr = CoCreateInstance((REFCLSID)CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, (REFIID)IID_ICaptureGraphBuilder2, (void **)&pCapture);
		pCapture->SetFiltergraph(pGraphBuilder);

		CComPtr<ICreateDevEnum> pDevEnum = NULL;
		if (CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pDevEnum) != S_OK)
		{
			releaseObjects();
			return 1;
		}

		int vcapdev=_wtoi(fn+6);
		int acapdev=_wtoi(fn+8); //FUCKO

		assert(pCapVidSrcFilter==0);
		pCapVidSrcFilter = GetCaptureDevice(pDevEnum, CLSID_VideoInputDeviceCategory, vcapdev);
		if (!pCapVidSrcFilter || pGraphBuilder->AddFilter(pCapVidSrcFilter, L"Video Capture") != S_OK)
		{
			releaseObjects();
			return 1;
		}

		//if (g_config_vidcap) {
		if (1)
		{
			IAMStreamConfig *pSC;
			hr = pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pCapVidSrcFilter, IID_IAMStreamConfig, (void **)&pSC);

			if (hr != NOERROR)
				pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,				&MEDIATYPE_Video, pCapVidSrcFilter,				IID_IAMStreamConfig, (void **)&pSC);

			ISpecifyPropertyPages *pSpec=NULL;
			if (pSC)
			{
				pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
				if (pSpec)
				{
					CAUUID cauuid;
					pSpec->GetPages(&cauuid);
					OleCreatePropertyFrame(NULL, 0, 0, L"Poopie", 1, (IUnknown **)&pSC, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL);
					CoTaskMemFree(cauuid.pElems);
					pSpec->Release();
				}
				pSC->Release();
			}
		}

		if (pCapVidSrcFilter && pCapture->RenderStream(0,&MEDIATYPE_Video, pCapVidSrcFilter, 0, nullfilter2) != S_OK)
		{
			releaseObjects();
			return 1;
		}

		assert(pCapAudSrcFilter==0);
		pCapAudSrcFilter = GetCaptureDevice(pDevEnum, CLSID_AudioInputDeviceCategory, acapdev);
		if (!pCapVidSrcFilter || pGraphBuilder->AddFilter(pCapAudSrcFilter, L"Audio Capture") != S_OK)
		{
			releaseObjects();
			return 1;
		}
		if (pCapVidSrcFilter && pCapture->RenderStream(0,&MEDIATYPE_Audio, pCapAudSrcFilter, 0, nullfilter) != S_OK)
		{
			releaseObjects();
			return 1;
		}
	}
	else
#endif
	{
		try
		{
			hr = pGraphBuilder->RenderFile(fn,NULL);
		}
		catch (...)
		{
			releaseObjects();
			return 1;
		}

		if (FAILED(hr))
		{
			// check for URL launch (WMA/ASF/...)
			handleNotifyEvents();
			releaseObjects();
#ifdef WINAMPX
			if ((hr == CLASS_E_CLASSNOTAVAILABLE) || (hr == VFW_E_UNSUPPORTED_VIDEO) || (hr == VFW_E_NO_DECOMPRESSOR))
			{
				if (ReportMissingCodec(fn)) // returns true if we sent a message
					return -500;  // Unsupported format
				return -200; // Can't play file
			}
#endif // WINAMPX
			return 1;
		}

#ifdef WINAMPX
		// Check if it's a partial playing of the file (likely video missing)
		if ((hr == VFW_S_PARTIAL_RENDER) || (hr == VFW_S_VIDEO_NOT_RENDERED))
		{
			if (!ReportMissingCodec(fn)) // Report the missing codec if we can determine it
				mod.fire_winampstatus(WINAMPX_STATUS_MISSING_AVI_CODEC, 0); // If we can't report a null codec missing
		}
#endif // WINAMPX
	}

	// check if video has been negociated
	{
		CMediaType *mt=nullfilter2->GetAcceptedType();
		GUID t=mt->subtype;
		if (t==MEDIASUBTYPE_YUY2) video_mediatype=VIDEO_MAKETYPE('Y','U','Y','2');
		else if (t==MEDIASUBTYPE_YV12) video_mediatype=VIDEO_MAKETYPE('Y','V','1','2');
		else if (t==MEDIASUBTYPE_RGB32) video_mediatype=VIDEO_MAKETYPE('R','G','3','2');
		else if (t==MEDIASUBTYPE_RGB24) video_mediatype=VIDEO_MAKETYPE('R','G','2','4');
		else if (t==MEDIASUBTYPE_RGB8) video_mediatype=VIDEO_MAKETYPE('R','G','B','8');
		else if (t==MEDIASUBTYPE_YVYU) video_mediatype=VIDEO_MAKETYPE('Y','V','Y','U');
		else
		{
			has_video=NULL;
		}
		if (has_video)
		{

#ifdef DEBUGVIDEO
			char tmp[512] = {0};
			int a=video_mediatype;
			wsprintf(tmp,"file: %s %c%c%c%c\n",fn,(char)(a&0xff),(char)((a>>8)&0xff),(char)((a>>16)&0xff),(char)((a>>24)&0xff));
			outputDebugStr(tmp);
#endif

			GUID format=mt->formattype;
			int pw,ph;
			if (format==FORMAT_VideoInfo)
			{
				VIDEOINFOHEADER *pHeader=(VIDEOINFOHEADER*)mt->pbFormat;
				pw=pHeader->bmiHeader.biWidth;
				ph=abs(pHeader->bmiHeader.biHeight);
				if (pHeader->bmiHeader.biBitCount==8)
				{
					VIDEOINFO *pHeader=(VIDEOINFO*)mt->pbFormat;
					memcpy(palette,&pHeader->bmiColors,sizeof(RGBQUAD)*0x100);
					has_palette = true;
				}

#ifdef DEBUGVIDEO
				RECT r=pHeader->rcSource;
				RECT r2=pHeader->rcTarget;
				char tmp[512] = {0};
				wsprintf(tmp,"init videoheader1: %i %i %i %i, %i %i %i %i\n",r.left,r.right,r.top,r.bottom,r2.left,r2.right,r2.top,r2.bottom);
				outputDebugStr(tmp);
#endif

			}
			else
			{
				VIDEOINFOHEADER2 *pHeader=(VIDEOINFOHEADER2*)mt->pbFormat;
				pw=pHeader->bmiHeader.biWidth;
				ph=abs(pHeader->bmiHeader.biHeight);
				if (pHeader->dwPictAspectRatioX) aspect *= (double)pHeader->dwPictAspectRatioY * (double)pw / ((double)ph * (double)pHeader->dwPictAspectRatioX);

#ifdef DEBUGVIDEO
				RECT r=pHeader->rcSource;
				RECT r2=pHeader->rcTarget;
				char tmp[512] = {0};
				wsprintf(tmp,"init videoheader2: %i %i %i %i, %i %i %i %i\n",r.left,r.right,r.top,r.bottom,r2.left,r2.right,r2.top,r2.bottom);
				outputDebugStr(tmp);
#endif
			}
			video_w=pw; video_h=ph;
			video_len=(video_w*video_h*4)+sizeof(double); //CT> might be wrong for YUY2, etc...
		}
		else
		{
			pGraphBuilder->RemoveFilter(nullfilter2);
			// TODO: release?
			nullfilter2=0;
		}
	}

	// check if audio has been negociated
	{
		CMediaType *mt=nullfilter->GetAcceptedType();
		if (mt->subtype!=MEDIASUBTYPE_PCM && mt->subtype!=MEDIASUBTYPE_IEEE_FLOAT)
			has_audio=NULL;
		else
		{
			WAVEFORMATEX *pHeader = (WAVEFORMATEX*)mt->pbFormat;
			// reget this cause this is the real UNCOMPRESSED format
			audio_bps	= pHeader->wBitsPerSample;
			audio_srate = pHeader->nSamplesPerSec;
			audio_nch	= pHeader->nChannels;
			
			if (mt->subtype == MEDIASUBTYPE_IEEE_FLOAT/*WAVE_FORMAT_IEEE_FLOAT*//*audio_bps==32 || audio_bps==64*/)
			{
				m_float		= 1;
				m_src_bps	= audio_bps;
				//audio_bps	= 16; //TODO: read bits from AGAVE_API_CONFIG :)
			}
		}
	}

	// if none has been negociated, fuck off
	if (!has_video && !has_audio)
	{
		releaseObjects();
		return 1;
	}

	if (!has_audio)
	{
		pGraphBuilder->RemoveFilter(nullfilter);
		// TODO: release?
		nullfilter=0;
	}
	// QueryInterface for some basic interfaces
	assert(pMediaControl==0);
	pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
	assert(pMediaSeeking==0);
	pGraphBuilder->QueryInterface(IID_IMediaSeeking, (void **)&pMediaSeeking);
	if (pMediaControl == NULL || pMediaEventEx == NULL)
	{
		releaseObjects();
		return 1;
	}

	CComPtr<IVideoWindow> pVideoWindow;
	pGraphBuilder->QueryInterface(IID_IVideoWindow, (void**)&pVideoWindow);
	pVideoWindow->put_AutoShow(OAFALSE);

	CComPtr<IMediaFilter> pMediaFilter;
	pGraphBuilder->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);

	//FUCKO: verify if setsyncsource is really necessary (might be useful for sync under
	// heavy cpu load)
	/*if(!STRICMP(ext,"wma") || !STRICMP(ext,"asf") || !STRICMP(ext,"wmv")) { }
	else*/
#if IN_DSHOW_CAPTURE_SUPPORT // disable
	if (!m_is_capture)
		pMediaFilter->SetSyncSource(NULL);
#endif

	// retrieves length
	{
		CComPtr<IMediaPosition> pMediaPosition=NULL;
		pGraphBuilder->QueryInterface(IID_IMediaPosition, (void **)&pMediaPosition);
		if (pMediaPosition)
		{
			REFTIME length;
			pMediaPosition->get_Duration(&length);
			m_length=(int)(length*1000);
		}
	}

	getInfo(fn, m_status, 512, NULL,0, &m_bitrate, &audio_nch);

	if (has_audio)
	{
		//open output plugin
		int maxlat=mod.outMod->Open(audio_srate,audio_nch,audio_bps,-1,-1);
		if (maxlat<0)
		{
			releaseObjects();
			return 1;
		}

//		if (has_video)
		mod.SetInfo(m_bitrate,audio_srate/1000,audio_nch,1);
		//else
//			mod.SetInfo(audioBitrate,audio_srate/1000,audio_nch,1);
		mod.SAVSAInit(maxlat,audio_srate);
		mod.VSASetInfo(audio_srate,audio_nch);
		mod.outMod->SetVolume(-666);
	}

	if (has_video)
	{
		//open video stuff
		m_video_output->extended(VIDUSER_SET_THREAD_SAFE, 0, 0); // we are NOT thread safe - we call draw() than a different thread than open()
		m_video_output->open(video_w,video_h,0,aspect,video_mediatype);
#ifdef WINAMPX
		if (has_palette)
		{
			m_video_output->extended(VIDUSER_SET_PALETTE, (int)palette, 0);
		}
		HWND hVideoWnd = (HWND)m_video_output->extended(VIDUSER_GET_VIDEOHWND, 0, 0);

		InvalidateRect(hVideoWnd, NULL, TRUE);
#endif
	}

	m_video_output->extended(VIDUSER_SET_INFOSTRINGW,(INT_PTR)m_status,0);
	m_laststatus=GetTickCount();

	m_starttime=GetTickCount(); //used for non-audio videos
	hr = pMediaControl->Run();
	if (FAILED(hr))
	{
		stop();
		releaseObjects();
		return 1;
	}

	lstrcpynW(lastfn,fn, MAX_PATH);
	return 0;
}

// standard pause implementation
void pause()
{
	paused=1;
	m_time_paused=GetTickCount()-m_starttime;
	if (has_audio) mod.outMod->Pause(1);
}

void unpause()
{
	paused=0;
	if (has_audio)
		mod.outMod->Pause(0);
	m_starttime=GetTickCount()-m_time_paused;
	m_nbframes=m_avgfps_start=0;
	m_laststatus=GetTickCount();
}

int ispaused()
{
	return paused;
} // Note: Shared with the dsr routines

// stop playing.
void stop()
{
	g_quit=1;

	while (doingaudioshit) Sleep(10);

	if (pMediaControl)
		pMediaControl->Stop();

	if (m_video_output)
		m_video_output->close();

	mod.outMod->Close();
	mod.SAVSADeInit();

	releaseObjects();

	m_length=-1;

	m_lastfn[0]=0;
}

int getlength()
{
	return m_length;
}

int getoutputtime()
{

	if (g_bufferstat) return g_bufferstat;

	if (has_audio)
	{
		return decode_pos_ms+
		       (mod.outMod->GetOutputTime()-mod.outMod->GetWrittenTime());
	}
	else
	{
		if (paused)
			return m_time_paused;
		return GetTickCount()-m_starttime;
	}
}

void setoutputtime(int time_in_ms)
{
	if (pMediaSeeking)
	{
		DWORD dwCaps = AM_SEEKING_CanSeekAbsolute;
		if (pMediaSeeking->CheckCapabilities(&dwCaps) == S_OK)
		{
			int oldpause=paused;
			if (oldpause) unpause();
			pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
			LONGLONG l=((LONGLONG)time_in_ms)*10000;
			pMediaSeeking->SetPositions(&l,AM_SEEKING_AbsolutePositioning|AM_SEEKING_SeekToKeyFrame|AM_SEEKING_ReturnTime ,NULL,AM_SEEKING_NoPositioning);
			l/=10000;
			time_in_ms=(int)l;
			mod.outMod->Flush(time_in_ms);
			decode_pos_ms=time_in_ms;
			m_starttime=GetTickCount()-time_in_ms; //non-audio videos
			m_nbframes=m_avgfps_start=0;
			m_laststatus=GetTickCount();
			if (oldpause) pause();
		}
	}
}

void setvolume(int volume)
{
	{
		mod.outMod->SetVolume(volume);
	}
}
void setpan(int pan)
{
	mod.outMod->SetPan(pan);
}

int infoDlg(const wchar_t *fn, HWND hwnd)
{
	doInfo(WASABI_API_LNG_HINST,hwnd, fn);
	return INFOBOX_UNCHANGED;
}

// this is an odd function. it is used to get the title and/or
// length of a track.
// if filename is either NULL or of length 0, it means you should
// return the info of lastfn. Otherwise, return the information
// for the file in filename.
// if title is NULL, no title is copied into it.
// if length_in_ms is NULL, no length is copied into it.
void getfileinfo(const wchar_t *filename, wchar_t *title, int *length_in_ms)
{
	if (!filename || !*filename)  // currently playing file
	{
		if (length_in_ms) *length_in_ms=getlength();
		if (title) // get non-path portion.of filename
		{
			wchar_t *p = PathFindFileNameW(lastfn);

			if (lastfn_status[0])
			{
				StringCchPrintfW(title, GETFILEINFO_TITLE_LENGTH, L"[%s] %s",lastfn_status,p);
			}
			else
			{
				lstrcpynW(title,p, GETFILEINFO_TITLE_LENGTH);
			}
		}
	}
	else // some other file
	{
		if (length_in_ms) // calculate length
		{
			*length_in_ms = GetFileLength(filename);
		}
		if (title) // get non path portion of filename
		{
			lstrcpynW(title, filename, GETFILEINFO_TITLE_LENGTH);
			PathStripPathW(title);
			PathRemoveExtensionW(title);
		}
	}
}

void eq_set(int on, char data[10], int preamp)
{
}


// module definition.

In_Module mod =
{
	IN_VER_RET,	// defined in IN2.H
	"nullsoft(in_dshow.dll)",
	0,	// hMainWindow (filled in by winamp)
	0,  // hDllInstance (filled in by winamp)
	/*"MPG;MPEG;M2V\0MPG File (*.MPG;*.MPEG;*.M2V)\0"
	"AVI\0AVI File (*.AVI)\0"
	"ASF;WMV\0ASF/WMV File (*.ASF;*.WMV)\0"*/
	0,	// this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
	1,	// is_seekable
	1,	// uses output plug-in system
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
	0,0,0,0,0,0,0,0,0, // visualization calls filled in by winamp
	0,0, // dsp calls filled in by winamp
	eq_set,
	NULL,		// setinfo call filled in by winamp
	0 // out_mod filled in by winamp
};

static char default_extlist[]="MPG;MPEG;M2V";

static const wchar_t *pExtList[]={L"MPG",L"MPEG",L"M2V",L"AVI",L"MOV",L"FLV",L"FLV1",L"OGV",L"OGA",L"OGM",L"RMVB",L"RM",L"VOB",L"AC3",L"MKV",L"MP4",L"M4V",L"3GP"};
static const int pExtDescIdList[] = {0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14,};
static const int pExtDescList[] =
{
	IDS_FAMILY_STRING_MPEG,
	IDS_FAMILY_STRING_MPEG2,
	IDS_FAMILY_STRING_AVI,
	IDS_FAMILY_STRING_MOV,
	IDS_FAMILY_STRING_FLV,
	IDS_FAMILY_STRING_OGV,
	IDS_FAMILY_STRING_OGA,
	IDS_FAMILY_STRING_OGM,
	IDS_FAMILY_STRING_RM,
	IDS_FAMILY_STRING_VOB,
	IDS_FAMILY_STRING_AC3,
	IDS_FAMILY_STRING_MKV,
	IDS_FAMILY_STRING_MP4,
	IDS_FAMILY_STRING_M4V,
	IDS_FAMILY_STRING_3GPP,
};
static FILETIME ftLastWriteTime;

// is used to determine if the last write time of the file has changed when
// asked to get the metadata for the same cached file so we can update things
BOOL HasFileTimeChanged(const wchar_t *fn)
{
	WIN32_FILE_ATTRIBUTE_DATA fileData = {0};
	if (GetFileAttributesExW(fn, GetFileExInfoStandard, &fileData) == TRUE)
	{
		if(CompareFileTime(&ftLastWriteTime, &fileData.ftLastWriteTime))
		{
			ftLastWriteTime = fileData.ftLastWriteTime;
			return TRUE;
		}
	}
	return FALSE;
}

extern "C"
{

	__declspec(dllexport) In_Module * winampGetInModule2()
	{
		return &mod;
	}

	_declspec(dllexport) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
	{
		if (!fn || !*fn)
		{
			if (!lstrcmpiA(data,"type"))
			{
				lstrcpyn(dest,L"1", destlen); //video
				return 1;
			}

			return 0;
		}
		if (!infoFn || !infoHeader || lstrcmpiW(fn, infoFn) || HasFileTimeChanged(fn))
		{
			free(infoFn);
			infoFn = _wcsdup(fn);
			delete infoHeader;
			infoHeader = MakeHeader(fn, true);
		}

		if (!lstrcmpiA(data,"type"))
		{
			if (infoHeader)
			{
				if (infoHeader->has_video)
					lstrcpyn(dest,L"1", destlen); //video
				else
					lstrcpyn(dest,L"0", destlen); // no video
			}
			else // assume video
			{
				lstrcpyn(dest,L"1", destlen); //video
			}
			return 1;
		}

		if (!lstrcmpiA(data, "family"))
		{
			INT index;
			LPCWSTR e;
			DWORD lcid;
			e = PathFindExtension(fn);
			if (L'.' != *e || 0x00 == *(++e)) return 0;

			lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
			for (index = sizeof(pExtList)/sizeof(wchar_t*) - 1; index >= 0 && CSTR_EQUAL != CompareStringW(lcid, NORM_IGNORECASE, e, -1, pExtList[index], -1); index--);
			if (index >= 0 && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(pExtDescList[pExtDescIdList[index]]))) return 1;
			return 0;
		}

		if (!lstrcmpiA(data,"length"))
		{
			int len = GetFileLength(fn);
			if (len == -1000)
				dest[0]=0;
			else
				StringCchPrintf(dest, destlen, L"%d", len);
			return 1;
		}
		else if (!lstrcmpiA(data, "bitrate"))
		{
			int bitrate;
			getInfo(fn, NULL, 0, NULL,0, &bitrate, NULL);
			StringCchPrintf(dest, destlen, L"%d", bitrate);
			return 1;
		}
		else if (!lstrcmpiA(data,"title"))
		{
			if (infoHeader && infoHeader->title)
				lstrcpyn(dest,infoHeader->title, destlen);
			else
				dest[0]=0;
			return 1;
		}
		else if (!lstrcmpiA(data,"artist"))
		{
			if (infoHeader && infoHeader->artist)
				lstrcpyn(dest,infoHeader->artist, destlen);
			else
				dest[0]=0;
			return 1;
		}
		else if (!lstrcmpiA(data,"comment"))
		{
			if (infoHeader && infoHeader->comment)
				lstrcpyn(dest,infoHeader->comment, destlen);
			else
				dest[0]=0;
			return 1;
		}
		else if (!lstrcmpiA(data,"genre"))
		{
			if (infoHeader && infoHeader->genre)
				lstrcpyn(dest,infoHeader->genre, destlen);
			else
				dest[0]=0;
			return 1;
		}
		else if (!lstrcmpiA(data,"album"))
		{
			if (infoHeader && infoHeader->album)
				lstrcpyn(dest,infoHeader->album, destlen);
			else
				dest[0]=0;
			return 1;
		}
		else if (!lstrcmpiA(data,"composer"))
		{
			if (infoHeader && infoHeader->composer)
				lstrcpyn(dest,infoHeader->composer, destlen);
			else
				dest[0]=0;
			return 1;
		}
		else if (!lstrcmpiA(data,"publisher"))
		{
			if (infoHeader && infoHeader->publisher)
				lstrcpyn(dest,infoHeader->publisher, destlen);
			else
				dest[0]=0;
			return 1;
		}
		return 0;
	}
};