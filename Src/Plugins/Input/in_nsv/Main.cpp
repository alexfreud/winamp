#define PLUGIN_NAME "Nullsoft NSV Decoder"
#define PLUGIN_VERSION L"1.76"

#include <windows.h>
#include "../Winamp/in2.h"
#include "../nsv/nsvplay/main.h"
#include "resource.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoCharFn.h"
#define NO_IVIDEO_DECLARE
#include "../winamp/wa_ipc.h"
#include "../Winamp/strutil.h"
#include "api.h"
extern In_Module mod;			// the output module (filled in near the bottom of this file)

#define g_hInstance mod.hDllInstance
#define WNDMENU_CAPTION L"Winamp in_nsv"
#define MODAL_ABOUT
#define LOC_MODAL_ABOUT
#include "../nsv/nsvplay/about.h"
#undef g_hInstance

#include <shlwapi.h>
#include <strsafe.h>
extern int config_precseek;
extern int config_vidoffs;
extern int config_bufms;
extern int config_prebufms;
extern int config_underunbuf;
extern int config_bufms_f;
extern int config_prebufms_f;
extern int config_underunbuf_f;

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
  {
    0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
  };


char lastfn[1024] = {0};	// currently playing file (used for getting info on the current file)
static char statusbuf[1024];

char stream_url[1024] = {0};

ULONGLONG g_bufferstat;
int m_last_bitrate;
void config_read();
void config_write();
void config(HWND hwndParent);

int file_length = 0;	// file length, in bytes
// Used for correcting DSP plug-in pitch changes
int paused = 0;			// are we paused?
volatile int seek_needed; // if != -1, it is the point that the decode
// thread should seek to, in ms.

CRITICAL_SECTION g_decoder_cs;
char g_streaminfobuf[512] = {0};
int g_streaminfobuf_used = 0;

char error_string[128] = {0};

volatile int killDecodeThread = 0;			// the kill switch for the decode thread
HANDLE thread_handle = INVALID_HANDLE_VALUE;	// the handle to the decode thread

int has_opened_outmod = 0;
int m_srate = 0; // seek needs this

int decoders_initted = 0;

api_config *AGAVE_API_CONFIG = 0;
api_memmgr *WASABI_API_MEMMGR = 0;
// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

void process_url(char *url)
{
	lstrcpynA(stream_url, url, sizeof(stream_url));
	
	//  if (!strncmp(stream_url,"hTtP",4))
	//  {
	//    DWORD dw;
	//    SendMessageTimeout(mod.hMainWindow,WM_USER,(WPARAM)0,241,SMTO_NORMAL,500,&dw);
	//  }  // I (Tag) removed this support, its annoying.
	DWORD_PTR dw = 0;
	if (stream_url[0]) SendMessageTimeout(mod.hMainWindow, WM_USER, (WPARAM)stream_url, 241, SMTO_NORMAL, 500, &dw);
}

char last_title_sent[256] = {0};

void process_metadata(char *data, int len)
{
	if (len && *data)
	{
		char *ld;
		int x;
		if (len > 4096) return ;
		for (x = 0; x < len; x ++)
			if (!data[x]) break;
		if (x == len) return ;
		while ((ld = strstr(data, "='")))
		{
			char * n = data;
			ld[0] = 0;
			ld += 2;
			data = strstr(ld, "';");
			if (data)
			{
				data[0] = 0;
				data += 2;
				if (!lstrcmpiA(n, "StreamTitle"))
				{
					lstrcpynA(last_title_sent, ld, sizeof(last_title_sent));
					last_title_sent[sizeof(last_title_sent) - 1] = 0;
					PostMessage(mod.hMainWindow, WM_USER, 0, 243);
				}
				else if (!lstrcmpiA(n, "StreamUrl"))
				{
					process_url(ld);
				}
			}
			else break;
		}
	}
}

class WA2AudioOutput : public IAudioOutput
{
public:
	WA2AudioOutput(int srate, int nch, int bps)
	{
		memset(m_stuffbuf, 0, sizeof(m_stuffbuf));
		decode_pos_samples = 0;
		m_srate = srate; m_bps = bps; m_nch = nch;
		m_open_success = 0;
		m_stuffbuf_u = 0;
		int maxlat = mod.outMod->Open(srate, nch, bps, -1, -1);
		if (maxlat == 0 && strstr(lastfn, "://"))
		{
			maxlat = -1;
			mod.outMod->Close(); // boom
		}
		if (maxlat >= 0)
		{
			mod.SetInfo( -1, srate / 1000, nch, 1);
			mod.SAVSAInit(maxlat, srate);
			mod.VSASetInfo(srate, nch);
			mod.outMod->SetVolume( -666);
			m_open_success = 1;
			has_opened_outmod = 1;
		}
	}
	~WA2AudioOutput(){}

	int canwrite()
	{
		int a = mod.outMod->CanWrite();
		if (mod.dsp_isactive() == 1) a /= 2;
		return a & ~((m_nch * (m_bps / 8)) - 1);
	} // returns bytes writeable

	void write(void *buf, int len)
	{
		char *b = (char *)buf;
		int s = 576 * m_nch * (m_bps / 8);
		if (s > sizeof(m_stuffbuf)) s = sizeof(m_stuffbuf);

		while (len > 0)
		{
			int l = s;
			if (!m_stuffbuf_u && len >= s) // straight copy of data
			{
				int dms = (int) ((decode_pos_samples * (__int64)1000) / (__int64)m_srate);
				mod.SAAddPCMData(b, m_nch, m_bps, dms);
				mod.VSAAddPCMData(b, m_nch, m_bps, dms);
			}
			else if (m_stuffbuf_u + len >= s)
			{
				int dms = (int) (((decode_pos_samples - (m_stuffbuf_u / m_nch / (m_bps / 8))) * (__int64)1000) / (__int64)m_srate);
				l = (s - m_stuffbuf_u);
				memcpy(m_stuffbuf + m_stuffbuf_u, b, l);
				m_stuffbuf_u = 0;

				mod.SAAddPCMData(m_stuffbuf, m_nch, m_bps, dms);
				mod.VSAAddPCMData(m_stuffbuf, m_nch, m_bps, dms);
			}
			else // put all of len into m_stuffbuf
			{
				memcpy(m_stuffbuf + m_stuffbuf_u, b, len);
				m_stuffbuf_u += len;
				l = len;
			}

			if (l > len)l = len; // this shouldn't happen but we'll leave it here just in case

			decode_pos_samples += (l / m_nch / (m_bps / 8));

			if (mod.dsp_isactive())
			{
				static char sample_buffer[576*2*(16 / 8)*2];
				int spll = l / m_nch / (m_bps / 8);
				memcpy(sample_buffer, b, l);
				int l2 = l;
				if (spll > 0) l2 = mod.dsp_dosamples((short *)sample_buffer, spll, m_bps, m_nch, m_srate) * (m_nch * (m_bps / 8));
				mod.outMod->Write(sample_buffer, l2);
			}
			else mod.outMod->Write(b, l);
			len -= l;
			b += l;
		}
	}
	ULONGLONG getwritepos()
	{
		return (unsigned int) ((decode_pos_samples * 1000) / m_srate);
	}
	ULONGLONG getpos()
	{
		if (seek_needed != -1) return seek_needed;
		return (unsigned int) ((decode_pos_samples * 1000) / m_srate) +
		       (mod.outMod->GetOutputTime() - mod.outMod->GetWrittenTime()) - config_vidoffs;
	}
	void flush(unsigned int newtime)
	{
		m_stuffbuf_u = 0;
		mod.outMod->Flush(newtime);
		decode_pos_samples = (((__int64)newtime) * m_srate) / 1000;
	}
	void pause(int pause)
	{
		mod.outMod->Pause(pause);
	}
	int get_open_success() { return m_open_success; }
	int isplaying(void) { return mod.outMod->IsPlaying(); }
private:
	__int64 decode_pos_samples;		// current decoding position, in milliseconds.
	int m_nch, m_bps;
	int m_open_success;
	int m_stuffbuf_u;
	char m_stuffbuf[576*2*2];
};

IAudioOutput *PCMOUT_CREATE(unsigned int outfmt[8])
{
	if (outfmt[1] && outfmt[2] && outfmt[3] && outfmt[0] == NSV_MAKETYPE('P', 'C', 'M', ' '))
	{
		WA2AudioOutput *r = new WA2AudioOutput(outfmt[1], outfmt[2], outfmt[3]);
		if (r->get_open_success()) return r;
		delete r;
	}
	return NULL;
}

DWORD WINAPI DecodeThread(LPVOID b); // the decode thread procedure

void about(HWND hwndParent)
{
	do_about(hwndParent,WASABI_API_LNG_HINST);
}

void SetFileExtensions(void)
{
	static char fileExtensionsString[1200] = {0};	// "NSV;NSA\0Nullsoft Audio/Video File (*.NSV;*.NSA)\0"
	char* end = 0;
	StringCchCopyExA(fileExtensionsString, 1200, "NSV;NSA", &end, 0, 0);
	StringCchCopyExA(end+1, 1200, WASABI_API_LNGSTRING(IDS_NSA_NSV_FILE), 0, 0, 0);
	mod.FileExtensions = fileExtensionsString;
}

int init()
{
	if (!IsWindow(mod.hMainWindow))
		return IN_INIT_FAILURE;

	waServiceFactory *sf = mod.service->service_getServiceByGuid(AgaveConfigGUID);
	if (sf) AGAVE_API_CONFIG = (api_config *)sf->getInterface();

	sf = mod.service->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) WASABI_API_MEMMGR = reinterpret_cast<api_memmgr*>(sf->getInterface());

	// loader so that we can get the localisation service api for use
	sf = mod.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,InNSVLangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_NSV_DECODER),PLUGIN_VERSION);
	mod.description = (char*)szDescription;

	SetFileExtensions();

	config_read();
	InitializeCriticalSection(&g_decoder_cs);
	return IN_INIT_SUCCESS;
}

void quit()
{
	Decoders_Quit();
	DeleteCriticalSection(&g_decoder_cs);
	waServiceFactory *sf = mod.service->service_getServiceByGuid(AgaveConfigGUID);
	if (sf) sf->releaseInterface(AGAVE_API_CONFIG);

	sf = mod.service->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) sf->releaseInterface(WASABI_API_MEMMGR);
}

int isourfile(const char *fn)
{
	// used for detecting URL streams.. unused here.
	// return !strncmp(fn,"http://",7); to detect HTTP streams, etc
	return !_strnicmp( fn, "unsv://", 7 );
}

NSVDecoder *m_decoder = 0;
IVideoOutput *m_video_output = 0;

int g_play_needseek = -1;

int play(const char *fn)
{
	m_last_bitrate = -1;
	g_play_needseek = -1;
	last_title_sent[0] = 0;
	g_bufferstat = 0;
	mod.is_seekable = 0;
	has_opened_outmod = 0;
	error_string[0] = 0;
	if (!decoders_initted)
	{
		decoders_initted = 1;
		char buf[MAX_PATH] = {0}, *p = buf;
		GetModuleFileNameA(mod.hDllInstance, buf, sizeof(buf));
		while (p && *p) p++;
		while (p && p > buf && *p != '\\') p--;
		if (p) *p = 0;
		Decoders_Init(buf);
	}

	unsigned long thread_id = 0;

	paused = 0;
	seek_needed = -1;
	EnterCriticalSection(&g_decoder_cs);
	if (strstr(fn, "://"))
		WASABI_API_LNGSTRING_BUF(IDS_CONNECTING,error_string,128);
	else
		WASABI_API_LNGSTRING_BUF(IDS_OPENING,error_string,128);

	LeaveCriticalSection(&g_decoder_cs);

	m_video_output = (IVideoOutput *)SendMessage(mod.hMainWindow, WM_USER, 0, IPC_GET_IVIDEOOUTPUT);
	if (!m_video_output) return 1;

	m_video_output->open(0, 0, 0, 0, 0);

	m_decoder = new NSVDecoder(fn, m_video_output, NULL);
	lstrcpynA(lastfn, fn, sizeof(lastfn));

	if (strstr(fn, "://") || !strncmp(fn, "\\\\", 2))
	{
		m_decoder->SetPreciseSeeking(config_precseek&2);
		m_decoder->SetBuffering(config_bufms, config_prebufms, config_underunbuf);
	}
	else
	{
		m_decoder->SetPreciseSeeking(config_precseek&1);
		m_decoder->SetBuffering(config_bufms_f, config_prebufms_f, config_underunbuf_f);
	}

	// launch decode thread
	killDecodeThread = 0;
	thread_handle = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) DecodeThread, NULL, 0, &thread_id);
	SetThreadPriority(thread_handle, (int)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));

	return 0;
}

// standard pause implementation
void pause()
{
	paused = 1;
	//  if (has_opened_outmod) mod.outMod->Pause(1);
	//  else
	if (m_decoder) m_decoder->pause(1);
}
void unpause()
{
	paused = 0;
	//  if (has_opened_outmod) mod.outMod->Pause(0);
	//else
	if (m_decoder) m_decoder->pause(0);
}
int ispaused() { return paused; }

// stop playing.
void stop()
{
	g_play_needseek = -1;
		
	if (thread_handle != INVALID_HANDLE_VALUE)
	{
		killDecodeThread = 1;
		int nTimes = 0;
		const int maxTimes = 1000;
		while (WaitForSingleObject(thread_handle, 0) == WAIT_TIMEOUT)
		{
			MSG msg = {0};
			if (PeekMessage(&msg, NULL, 0, 0, 1))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
				Sleep(10);

			nTimes++;
			if (nTimes == maxTimes)
			{
#ifdef WINAMPX
				SendStatus( WINAMPX_STATUS_ERROR_KILLING_THREAD, 0 );
#else
				/*MessageBox(mod.hMainWindow, "error asking thread to die!\n",
						   "error killing decode thread", 0);*/
#endif
				TerminateThread(thread_handle, 0);
				break;
			}
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}
	if (has_opened_outmod) mod.outMod->Close();
	g_bufferstat = 0;
	has_opened_outmod = 0;
	mod.SAVSADeInit();

	EnterCriticalSection(&g_decoder_cs);
	delete(m_decoder);
	m_decoder = NULL;
LeaveCriticalSection(&g_decoder_cs);
	g_streaminfobuf[0] = 0;
}

int getlength()
{
	if (m_decoder)
	{
		int x = m_decoder->getlen();
		if (x != -1) return x;
	}
	return -1000;
}

int getoutputtime()
{
	if (g_bufferstat) return (int)g_bufferstat;
	EnterCriticalSection(&g_decoder_cs);
	if (m_decoder) 
	{
		LeaveCriticalSection(&g_decoder_cs);
		return (int)(m_decoder ? m_decoder->getpos() + config_vidoffs : 0);
	}
	LeaveCriticalSection(&g_decoder_cs);
	return 0;
}

void setoutputtime(int time_in_ms)
{
	seek_needed = time_in_ms;
}

void setvolume(int volume) { mod.outMod->SetVolume(volume); }

void setpan(int pan) { mod.outMod->SetPan(pan); }

int infoDlg(const char *fn, HWND hwnd);

// this is an odd function. it is used to get the title and/or
// length of a track.
// if filename is either NULL or of length 0, it means you should
// return the info of lastfn. Otherwise, return the information
// for the file in filename.
// if title is NULL, no title is copied into it.
// if length_in_ms is NULL, no length is copied into it.
void getfileinfo(const char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename)  // currently playing file
	{
		EnterCriticalSection(&g_decoder_cs);
		if (length_in_ms) *length_in_ms = getlength();
		if (title) // get non-path portion.of filename
		{
			char *p = NULL;
			if (m_decoder)
			{
				p = m_decoder->getTitle();
			}
			if (!p)
			{
				p = lastfn + strlen(lastfn);
				while (p && *p != '\\' && p >= lastfn) p--;
				p++;
			}
			while (p && *p == ';') p++;
			title[0] = 0;

			if (error_string[0])
			{
				StringCchPrintfA(title, GETFILEINFO_TITLE_LENGTH, "[%s] ", error_string);
			}
			if (!error_string[0] && last_title_sent[0])
			{
				StringCchPrintfA(title, GETFILEINFO_TITLE_LENGTH-strlen(title), "%s (%s)", last_title_sent, p);
			}
			else
				lstrcpynA(title + strlen(title), p, FILETITLE_SIZE);
		}
		LeaveCriticalSection(&g_decoder_cs);
	}
	else if (1) // some other file
	{
		if ((length_in_ms || title) && !strstr(filename, "://"))
		{
			nsv_InBS bs;
			nsv_fileHeader hdr = {0, };
			if (length_in_ms) // calculate length
			{
				*length_in_ms = -1000; // the default is unknown file length (-1000).
			}
			if (title) // get non path portion of filename
			{
				const char *p = filename + strlen(filename);
				while (p && *p != '\\' && p >= filename) p--;
				lstrcpynA(title, ++p, GETFILEINFO_TITLE_LENGTH);
			}

			IDataReader *rdr = CreateReader(filename);

			if (rdr)
			{
				while (!rdr->iseof())
				{
					char buf[1024] = {0};
					int l = (int)rdr->read(buf, sizeof(buf));
					if (!l) break;
					bs.add(buf, l);
					l = nsv_readheader(bs, &hdr);
					if (l <= 0)
					{
						if (!l)
						{
							if (length_in_ms) *length_in_ms = hdr.file_lenms;
							if (title && hdr.metadata)
							{
								char *t = nsv_getmetadata(hdr.metadata, "TITLE");
								if (t) lstrcpynA(title, t, 1024);
							}
						}
						free(hdr.metadata);
						free(hdr.toc);
						break;
					}
				}
				delete rdr;
			}
			// try to parse out lengths
		}
	}
}

void eq_set(int on, char data[10], int preamp)
{}

DWORD WINAPI DecodeThread(LPVOID b)
{
	int last_bpos = -1;
	int firstsynch = 0;
	ULONGLONG next_status_time = 0;
	while (!killDecodeThread)
	{
		EnterCriticalSection(&g_decoder_cs);
		int r = m_decoder->run((int*)&killDecodeThread);
		LeaveCriticalSection(&g_decoder_cs);
		if (r < 0)
		{
			if (m_decoder->get_error())
			{
				EnterCriticalSection(&g_decoder_cs);
				lstrcpynA(error_string, m_decoder->get_error(), sizeof(error_string));
				LeaveCriticalSection(&g_decoder_cs);
				PostMessage(mod.hMainWindow, WM_USER, 0, 243);
				Sleep(200);
			}
			break;
		}
		else if (!r)
		{
			Sleep(1);
			int br = m_decoder->getBitrate() / 1000;
			if (br != m_last_bitrate)
			{
				m_last_bitrate = br;
				mod.SetInfo(br, -1, -1, -1);
			}

			int bpos = m_decoder->getBufferPos();
			if (bpos > 255)
			{
				ULONGLONG obuf = g_bufferstat;
				g_bufferstat = 0;
				if (last_bpos >= 0)
				{
					EnterCriticalSection(&g_decoder_cs);
					error_string[0] = 0;
					LeaveCriticalSection(&g_decoder_cs);
					PostMessage(mod.hMainWindow, WM_USER, 0, 243);
					last_bpos = -1;
					int csa = mod.SAGetMode();
					if (csa && obuf)
					{
						char tempdata[75*2] = {0, };
						mod.SAAdd(tempdata, (int)++obuf, (csa == 3) ? 0x80000003 : csa);
					}
				}
			}
			else
			{
				if (!g_bufferstat)
				{
					if (!has_opened_outmod) mod.SAVSAInit(10, 44100);

					g_bufferstat = m_decoder->getpos() + 1;
				}

				if (bpos != last_bpos)
				{
					last_bpos = bpos;
					EnterCriticalSection(&g_decoder_cs);
					StringCchPrintfA(error_string, 128, WASABI_API_LNGSTRING(IDS_BUFFER_X), (bpos*100) / 256);
					LeaveCriticalSection(&g_decoder_cs);
					int csa = mod.SAGetMode();
					char tempdata[2*75] = {0, };
					int x;
					if (csa&1)
					{
						for (x = 0; x < bpos*75 / 256; x ++)
						{
							tempdata[x] = x * 16 / 75;
						}
					}
					if (csa&2)
					{
						int offs = (csa & 1) ? 75 : 0;
						x = 0;
						while (x < bpos*75 / 256)
						{
							tempdata[offs + x++] = -6 + x * 14 / 75;
						}
						while (x < 75)
						{
							tempdata[offs + x++] = 0;
						}
					}
					if (csa == 4)
					{
						tempdata[0] = tempdata[1] = (bpos * 127 / 256);
					}

					if (csa) mod.SAAdd(tempdata, (int)++g_bufferstat, (csa == 3) ? 0x80000003 : csa);
					PostMessage(mod.hMainWindow, WM_USER, 0, 243);
				}
			}

			if (GetTickCount64() > next_status_time || GetTickCount64() < next_status_time - 5000)
			{
				char statusbuf[1024] = {0};
				EnterCriticalSection(&g_decoder_cs);
				g_streaminfobuf[0] = 0;

				if (g_streaminfobuf_used)
				{
					char *outp = g_streaminfobuf;
					size_t size = 512;
					const char *p = m_decoder->getServerHeader("server");
					if (!p) p = m_decoder->getServerHeader("icy-notice2");

					if (p && strlen(p) < sizeof(g_streaminfobuf) - (outp - g_streaminfobuf) - 32)
						StringCchPrintfExA(outp, size, &outp, &size, 0, "%s: %s\r\n", WASABI_API_LNGSTRING(IDS_SERVER), p);

					p = m_decoder->getServerHeader("content-type");
					if (p && strlen(p) < sizeof(g_streaminfobuf) - (outp - g_streaminfobuf) - 32)
						StringCchPrintfExA(outp, size, &outp, &size, 0, "%s: %s\r\n", WASABI_API_LNGSTRING(IDS_CONTENT_TYPE), p);

					p = m_decoder->getServerHeader("content-length");
					if (p && strlen(p) < sizeof(g_streaminfobuf) - (outp - g_streaminfobuf) - 32)
						StringCchPrintfExA(outp, size, &outp, &size, 0, "%s: %s\r\n", WASABI_API_LNGSTRING(IDS_CONTENT_LENGTH), p);

					p = m_decoder->getServerHeader("icy-name");
					if (p && strlen(p) < sizeof(g_streaminfobuf) - (outp - g_streaminfobuf) - 32)
						StringCchPrintfExA(outp, size, &outp, &size, 0, "%s: %s\r\n", WASABI_API_LNGSTRING(IDS_STREAM_NAME), p);
				}

				lstrcpynA(statusbuf, "NSV: ", 1024);
				{ // codecs
					char *sb = statusbuf;
					size_t size = 1024;

					int l = m_decoder->getlen();
					if (l > 0)
					{
						l /= 1000;
						if (l >= 3600)
						{
							StringCchPrintfExA(sb, size, &sb, &size, 0, "%d:", l / 3600);
							l %= 3600;
							StringCchPrintfExA(sb, size, &sb, &size, 0, "%02d:%02d", l / 60, l % 60);
						}
						else
							StringCchPrintfExA(sb, size, &sb, &size, 0, "%d:%02d", l / 60, l % 60);
					}

					int a = (m_decoder->getBitrate() + 500) / 1000;
					if (a)
					{
						if (strlen(statusbuf) > 5)
							StringCchCatExA(sb, size, " @ ", &sb, &size, 0);
						StringCchPrintfExA(sb, size, &sb, &size, 0, "%d%s", a, WASABI_API_LNGSTRING(IDS_KBPS));
					}

					if (strlen(statusbuf) > 5)
						StringCchCatExA(sb, size, ", ", &sb, &size, 0);

					char *p = statusbuf + strlen(statusbuf);
					m_decoder->getVideoDesc(p);
					if (p && !*p) StringCchCopyExA(p, size, "?, ", &p, &size, 0);
					else if (!strncmp(p, "NONE", 4)) *p = 0;
					else StringCchCatExA(p, size, ", ", &p, &size, 0);

					p = statusbuf + strlen(statusbuf);
					m_decoder->getAudioDesc(p);
					if (p && !*p) StringCchCopyExA(p, size, "?", &p, &size, 0);
					else if (!strncmp(p, "NONE", 4)) *p = 0;
				}
				LeaveCriticalSection(&g_decoder_cs);
				m_video_output->extended(VIDUSER_SET_INFOSTRING, (intptr_t)statusbuf, 0);

				next_status_time = GetTickCount64() + 2500;
			}
		}
		else
		{
			if (!firstsynch)
			{
				if (m_decoder->canseek())
				{
					mod.is_seekable = 1;
					if (g_play_needseek >= 0) seek_needed = g_play_needseek;
					g_play_needseek = -1;
				}
				firstsynch++;
				PostMessage(mod.hMainWindow, WM_USER, 0, 243);
			}
		}
		if (seek_needed >= 0)
		{
			EnterCriticalSection(&g_decoder_cs);
			m_decoder->seek(seek_needed);
			seek_needed = -1;
			LeaveCriticalSection(&g_decoder_cs);
		}
	}
	if (!killDecodeThread)
	{
		PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
	}
	m_decoder->CloseVideo();
	return 0;
}

// module definition.
In_Module mod =
    {
        IN_VER_RET,	// defined in IN2.H
        "nullsoft(in_nsv.dll)",
        0,  	// hMainWindow (filled in by winamp)
        0,		// hDllInstance (filled in by winamp)
        0,		// this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
        1,  	// is_seekable
        1,  	// uses output plug-in system
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

        0, 0, 0, 0, 0, 0, 0, 0, 0,   // visualization calls filled in by winamp

        0, 0,   // dsp calls filled in by winamp

        eq_set,

        NULL,  		// setinfo call filled in by winamp

        0 // out_mod filled in by winamp
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
	__declspec( dllexport ) In_Module * winampGetInModule2()
	{
		return &mod;
	}

	wchar_t lastextfn[1024] = {0};
	// Keep track of file timestamp for file system change notification handling
	FILETIME last_write_time = {0, 0};
	static int valid;
	static nsv_fileHeader hdr;

	bool isFileChanged(const wchar_t* file)
	{
		WIN32_FIND_DATAW FindFileData = {0};
		HANDLE hFind = FindFirstFileW(file, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return true;
		FindClose(hFind);

		if ((last_write_time.dwHighDateTime ==
		     FindFileData.ftLastWriteTime.dwHighDateTime) &&
		    (last_write_time.dwLowDateTime ==
		     FindFileData.ftLastWriteTime.dwLowDateTime))
		{
			return false;
		}
		return true;
	}

	__declspec( dllexport ) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
	{
		if (!_stricmp(data, "type"))
		{
			if (!fn || !fn[0] || _wcsicmp(PathFindExtensionW(fn), L".nsa")) // if extension is NOT nsa 
				lstrcpyn(dest, L"1", destlen); //video
			else
				lstrcpyn(dest, L"0", destlen); // audio
			return 1;
		}

		if (!fn || (fn && !fn[0]))
		return 0;

		if (!_stricmp(data, "family"))
		{  
			int pID = -1;
			LPCWSTR e = PathFindExtensionW(fn);
			if (L'.' != *e) return 0;
			e++;
			DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
			if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"NSV", -1)) pID = IDS_FAMILY_STRING_NSV;
			if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"NSA", -1)) pID = IDS_FAMILY_STRING_NSA;
			if (pID != -1 && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(pID))) return 1;
			return 0;
		}

		//the file name differs from the last file
		//name but we need to check the time stamp too
		if (_wcsicmp(fn, lastextfn) || isFileChanged(fn) || HasFileTimeChanged(fn))
		{
			free(hdr.metadata);
			memset(&hdr, 0, sizeof(hdr));
			valid = 0;

			lstrcpyn(lastextfn, fn, ARRAYSIZE(lastextfn));

			nsv_InBS bs;

			IDataReader *rdr = CreateReader(AutoCharFn(lastextfn));

			if (rdr)
			{
				while (!rdr->iseof())
				{
					char buf[1024] = {0};
					int l = (int)rdr->read(buf, sizeof(buf));
					if (!l) break;
					bs.add(buf, l);
					l = nsv_readheader(bs, &hdr);
					if (l <= 0)
					{
						free(hdr.toc);
						if (!l)
						{
							valid = 1;

							//Save time stamp
							WIN32_FIND_DATAW FindFileData = {0};
							HANDLE hFind = FindFirstFileW(fn, &FindFileData);
							if (hFind == INVALID_HANDLE_VALUE)
							{
								last_write_time.dwHighDateTime = NULL;
								last_write_time.dwLowDateTime = NULL;
							}
							else
							{
								last_write_time.dwHighDateTime = FindFileData.ftLastWriteTime.dwHighDateTime;
								last_write_time.dwLowDateTime = FindFileData.ftLastWriteTime.dwLowDateTime;
								FindClose(hFind);
							}
							break;
						}
						break;
					}
				}
				delete rdr;
			}
		}

		dest[0] = 0;

		if (!valid)
		{
			return 0;
		}

		if (!_stricmp(data, "length"))
		{
			if (hdr.file_lenms > 0)
			{
				StringCchPrintfW(dest, destlen, L"%d", hdr.file_lenms);
			}
		}
		else if (hdr.metadata)
		{
			const char *t = nsv_getmetadata(hdr.metadata, (char*)data);
			if (t) lstrcpyn(dest, AutoWide(t), destlen);
		}

		return 1;
	}
}