/*
** .WV input plug-in for WavPack
** Copyright (c) 2000 - 2006, Conifer Software, All Rights Reserved
*/
#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <mmreg.h>
#include <msacm.h>
#include <math.h>
#include <sys/stat.h>
#include <io.h>
#include <strsafe.h>

#include "in2.h"
#include "wavpack.h"
#include "resource.h"
#include "wasabi/winamp/wa_ipc.h"
#include "wasabi/wasabi.h"
#include "wasabi/nu/autochar.h"
#include "wasabi/nu/autowide.h"

#define fileno _fileno

static float calculate_gain(WavpackContext *wpc, bool allowDefault=true);

#define PLUGIN_VERSION "2.8.1"
//#define DEBUG_CONSOLE
//#define ANSI_METADATA
#define UNICODE_METADATA
//#define OLD_INFO_DIALOG

// post this to the main window at end of file (after playback as stopped)
#define WM_WA_MPEG_EOF WM_USER+2

#define MAX_NCH 8

static struct wpcnxt {
    WavpackContext *wpc;		// WavPack context provided by library
    float play_gain;			// playback gain (for replaygain support)
    //int soft_clipping;		// soft clipping active for playback
    int output_bits;			// 16, 24, or 32 bits / sample
    long sample_buffer[576*MAX_NCH*2];  // sample buffer
    float error [MAX_NCH];		// error term for noise shaping
    char lastfn[MAX_PATH];		// filename stored for comparisons only
    wchar_t w_lastfn[MAX_PATH];	// w_filename stored for comparisons only
    FILE *wv_id, *wvc_id;		// file pointer when we use reader callbacks
} curr, edit, info;

int decode_pos_ms;      // current decoding position, in milliseconds
int paused;             // are we paused?
int seek_needed;        // if != -1, it is the point that the decode thread should seek to, in ms.

#define ALLOW_WVC               0x1
#define REPLAYGAIN_TRACK        0x2
#define REPLAYGAIN_ALBUM        0x4
#define SOFTEN_CLIPPING         0x8
#define PREVENT_CLIPPING        0x10

#define ALWAYS_16BIT            0x20    // new flags added for version 2.5
#define ALLOW_MULTICHANNEL      0x40
#define REPLAYGAIN_24BIT        0x80

int config_bits = ALLOW_WVC | ALLOW_MULTICHANNEL;	// all configuration goes here

int killDecodeThread=0;								// the kill switch for the decode thread
HANDLE thread_handle=INVALID_HANDLE_VALUE;			// the handle to the decode thread

DWORD WINAPI __stdcall DecodeThread(void *b);		// the decode thread procedure

static api_service* WASABI_API_SVC;
static api_language* WASABI_API_LNG;
static api_config *AGAVE_API_CONFIG;
static api_memmgr *WASABI_API_MEMMGR;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static char file_extensions[128] = {"WV\0WavPack Files (*.WV)\0"};

// function definitions for the In_Module stucture
void about (HWND hwndParent);
void init();
void quit();
void getfileinfo(const char *filename, char *title, int *length_in_ms);
int infoDlg(const char *fn, HWND hwnd);
int isourfile(const char *fn);
int play(const char *fn);
void pause();
void unpause();
int ispaused();
void stop();
int getlength();
int getoutputtime();
void setoutputtime(int time_in_ms);
void setvolume(int volume);
void setpan(int pan);
void eq_set(int on, char data [10], int preamp);

In_Module mod =		// the output module
{
    IN_VER,
	"WavPack Decoder v"PLUGIN_VERSION,
    0,          // hMainWindow
    0,          // hDllInstance
	file_extensions,
    1,          // is_seekable
    1,          // uses output
    about,
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
    0,0,0,0,0,0,0,0,0,  // vis stuff
    0,0,                // dsp
    eq_set,
    NULL,               // setinfo
    0                   // out_mod
};

static BOOL CALLBACK WavPackDlgProc(HWND, UINT, WPARAM, LPARAM);

extern long dump_alloc (void);

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
	wchar_t about_string[512];
#ifdef DEBUG_ALLOC
    sprintf (about_string, "alloc_count = %d", dump_alloc ());
#else
	StringCchPrintfW(about_string, 512, WASABI_API_LNGSTRINGW(IDS_ABOUT_MESSAGE), PLUGIN_VERSION, "1998-2010", __DATE__);
#endif
	DoAboutMessageBox(hwndParent, WASABI_API_LNGSTRINGW(IDS_ABOUT), about_string);
}

void init() /* any one-time initialization goes here (configuration reading, etc) */
{
	if (mod.hMainWindow)
	{
		// load all of the required wasabi services from the winamp client
		WASABI_API_SVC = (api_service *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
		if (WASABI_API_SVC == (api_service *)1)
			WASABI_API_SVC=0;

		WASABI_API_SVC->service_register(&albumArtFactory);
	}
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,InWvLangGuid);

	static char szDescription[256];
	StringCchPrintfA(szDescription,256,WASABI_API_LNGSTRING(IDS_DESCRIPTION),PLUGIN_VERSION);
	mod.description = szDescription;

	// set the file extension to the localised version
	char tmp [64], *tmp_ptr = tmp, *fex_ptr = file_extensions;
	WASABI_API_LNGSTRING_BUF(IDS_FILETYPE, tmp, sizeof (tmp));

	*fex_ptr++ = 'W';
	*fex_ptr++ = 'V';
	*fex_ptr++ = 0;

	while (*tmp_ptr)
		*fex_ptr++ = *tmp_ptr++;

	*fex_ptr++ = 0;
	*fex_ptr++ = 0;
}

#ifdef DEBUG_CONSOLE

HANDLE debug_console=INVALID_HANDLE_VALUE;      // debug console

void debug_write (char *str)
{
    static int cant_debug;

    if (cant_debug)
        return;

    if (debug_console == INVALID_HANDLE_VALUE) {
        AllocConsole ();

#if 1
        debug_console = GetStdHandle (STD_OUTPUT_HANDLE);
#else
        debug_console = CreateConsoleScreenBuffer (GENERIC_WRITE, FILE_SHARE_WRITE,
            NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
#endif

        if (debug_console == INVALID_HANDLE_VALUE) {
            MessageBox(NULL, "Can't get a console handle", "WavPack",MB_OK);
            cant_debug = 1;
            return;
        }
        else if (!SetConsoleActiveScreenBuffer (debug_console)) {
            MessageBox(NULL, "Can't activate console buffer", "WavPack",MB_OK);
            cant_debug = 1;
            return;
        }
    }

    WriteConsole (debug_console, str, strlen (str), NULL, NULL);
}

#endif

void quit() /* one-time deinit, such as memory freeing */
{
#ifdef DEBUG_CONSOLE
    if (debug_console != INVALID_HANDLE_VALUE) {
        FreeConsole ();

        if (debug_console != GetStdHandle (STD_OUTPUT_HANDLE))
            CloseHandle (debug_console);

        debug_console = INVALID_HANDLE_VALUE;
    }
#endif
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	WASABI_API_SVC->service_deregister(&albumArtFactory);
}

// used for detecting URL streams.. unused here. strncmp(fn,"http://",7) to detect HTTP streams, etc
int isourfile(const char *fn)
{
    return 0;
}

int play(const char *fn)
{
    int num_chans, sample_rate;
    char error[128];
    int maxlatency;
    int thread_id;
    int open_flags;

#ifdef DEBUG_CONSOLE
    sprintf (error, "play (%s)\n", fn);
    debug_write (error);
#endif

    open_flags = OPEN_TAGS | OPEN_NORMALIZE;

    if (config_bits & ALLOW_WVC)
        open_flags |= OPEN_WVC;

    if (!(config_bits & ALLOW_MULTICHANNEL))
        open_flags |= OPEN_2CH_MAX;

    curr.wpc = WavpackOpenFileInput(fn, error, open_flags, 0);

    if (!curr.wpc)           // error opening file, just return error
        return -1;

    num_chans = WavpackGetReducedChannels(curr.wpc);
    sample_rate = WavpackGetSampleRate(curr.wpc);
    curr.output_bits = WavpackGetBitsPerSample(curr.wpc) > 16 ? 24 : 16;

    if (config_bits & ALWAYS_16BIT)
        curr.output_bits = 16;
    else if ((config_bits & (REPLAYGAIN_TRACK | REPLAYGAIN_ALBUM)) &&
        (config_bits & REPLAYGAIN_24BIT))
            curr.output_bits = 24;
 
    if (num_chans > MAX_NCH)    // don't allow too many channels!
	{
        WavpackCloseFile(curr.wpc);
        return -1;
    }

    curr.play_gain = calculate_gain(curr.wpc);
    lstrcpyn(curr.lastfn, fn, MAX_PATH);

    paused = 0;
    decode_pos_ms = 0;
    seek_needed = -1;

    maxlatency = mod.outMod->Open(sample_rate, num_chans, curr.output_bits, -1, -1);

    if (maxlatency < 0) // error opening device
	{
        curr.wpc = WavpackCloseFile(curr.wpc);
        return -1;
    }

    // dividing by 1000 for the first parameter of setinfo makes it
    // display 'H'... for hundred.. i.e. 14H Kbps.

    mod.SetInfo(0, (sample_rate + 500) / 1000, num_chans, 1);

    // initialize vis stuff

    mod.SAVSAInit(maxlatency, sample_rate);
    mod.VSASetInfo(sample_rate, num_chans);

    mod.outMod->SetVolume(-666);       // set the output plug-ins default volume

    killDecodeThread=0;

    thread_handle = (HANDLE)CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) DecodeThread, (void *) &killDecodeThread, 0, (LPDWORD)&thread_id);

    if (SetThreadPriority(thread_handle, THREAD_PRIORITY_HIGHEST) == 0) {
        curr.wpc = WavpackCloseFile(curr.wpc);
        return -1;
    }

    return 0;
}

void pause()
{
#ifdef DEBUG_CONSOLE
    debug_write ("pause ()\n");
#endif

    paused = 1;
    mod.outMod->Pause(1);
}

void unpause()
{
#ifdef DEBUG_CONSOLE
    debug_write ("unpause ()\n");
#endif

    paused = 0;
    mod.outMod->Pause(0);
}

int ispaused()
{
    return paused;
}

void stop()
{
#ifdef DEBUG_CONSOLE
    debug_write ("stop ()\n");
#endif

    if (thread_handle != INVALID_HANDLE_VALUE)
	{
        killDecodeThread = 1;

        if (WaitForSingleObject(thread_handle, INFINITE) == WAIT_TIMEOUT)
		{
            MessageBox(mod.hMainWindow,"error asking thread to die!\n", "error killing decode thread", 0);
            TerminateThread(thread_handle,0);
        }

        CloseHandle(thread_handle);
        thread_handle = INVALID_HANDLE_VALUE;
    }

    if (curr.wpc)
        curr.wpc = WavpackCloseFile(curr.wpc);

    mod.outMod->Close();
    mod.SAVSADeInit();
}

int getlength()
{
    return (int)(WavpackGetNumSamples (curr.wpc) * 1000.0 / WavpackGetSampleRate (curr.wpc));
}

int getoutputtime()
{
    if (seek_needed == -1)
        return decode_pos_ms + (mod.outMod->GetOutputTime () - mod.outMod->GetWrittenTime ());
    else
        return seek_needed;
}

void setoutputtime (int time_in_ms)
{
#ifdef DEBUG_CONSOLE
    char str [40];
    sprintf (str, "setoutputtime (%d)\n", time_in_ms);
    debug_write (str);
#endif

    seek_needed = time_in_ms;
}

void setvolume (int volume)
{
    mod.outMod->SetVolume(volume);
}

void setpan (int pan)
{
    mod.outMod->SetPan(pan);
}

static void generate_format_string(WavpackContext *wpc, wchar_t *string, int maxlen, int wide);
static void AnsiToUTF8(char *string, int len);
static int UTF8ToWideChar(const char *pUTF8, wchar_t *pWide);
static void UTF8ToAnsi(char *string, int len);

int infoDlg(const char *fn, HWND hwnd)
{
#ifdef OLD_INFO_DIALOG
    char string[2048];
    wchar_t w_string[2048];
    WavpackContext *wpc;
    int open_flags;

    open_flags = OPEN_TAGS | OPEN_NORMALIZE;

    if (config_bits & ALLOW_WVC)
        open_flags |= OPEN_WVC;

    if (!(config_bits & ALLOW_MULTICHANNEL))
        open_flags |= OPEN_2CH_MAX;

    wpc = WavpackOpenFileInput(fn, string, open_flags, 0);

    if (wpc)
	{
        int mode = WavpackGetMode(wpc);

        //generate_format_string(wpc, string, sizeof (string), 1);
		wchar_t *temp = (wchar_t *)malloc(sizeof(string) * sizeof(wchar_t));
        generate_format_string(wpc, temp, sizeof(string), 0);
		lstrcpyn(string, AutoChar(temp, CP_UTF8), sizeof(string));
		free(temp);

        if (WavpackGetMode(wpc) & MODE_VALID_TAG)
		{
            char value [128];

            if (config_bits & (REPLAYGAIN_TRACK | REPLAYGAIN_ALBUM))
			{
                int local_clipping = 0;
                float local_gain;

                local_gain = calculate_gain(wpc);

                if (local_gain != 1.0)
                    StringCchPrintf(string + strlen (string), 2048, "Gain:  %+.2f dB %s\n",
									log10 (local_gain) * 20.0, local_clipping ? "(w/soft clipping)" : "");
            }

            if (WavpackGetTagItem(wpc, "title", value, sizeof (value)))
			{
                if (!(mode & MODE_APETAG))
                     AnsiToUTF8(value, sizeof (value));

                StringCchPrintf(string + strlen (string), 2048, "\nTitle:  %s", value);
            }

            if (WavpackGetTagItem(wpc, "artist", value, sizeof (value)))
			{
                if (!(mode & MODE_APETAG))
                     AnsiToUTF8(value, sizeof (value));

                StringCchPrintf(string + strlen (string), 2048, "\nArtist:  %s", value);
            }

            if (WavpackGetTagItem (wpc, "album", value, sizeof (value)))
			{
                if (!(mode & MODE_APETAG))
                     AnsiToUTF8(value, sizeof (value));

                StringCchPrintf (string + strlen (string), 2048, "\nAlbum:  %s", value);
            }

            if (WavpackGetTagItem (wpc, "genre", value, sizeof (value)))
			{
                if (!(mode & MODE_APETAG))
                     AnsiToUTF8(value, sizeof (value));

                StringCchPrintf(string + strlen (string), 2048, "\nGenre:  %s", value);
            }

            if (WavpackGetTagItem (wpc, "comment", value, sizeof (value)))
			{
                if (!(mode & MODE_APETAG))
                     AnsiToUTF8(value, sizeof (value));

                StringCchPrintf(string + strlen (string), 2048, "\nComment:  %s", value);
            }

            if (WavpackGetTagItem(wpc, "year", value, sizeof (value)))
                StringCchPrintf(string + strlen (string), 2048, "\nYear:  %s", value);

            if (WavpackGetTagItem(wpc, "track", value, sizeof (value)))
                StringCchPrintf(string + strlen (string), 2048, "\nTrack:  %s", value);

            StringCchCat(string, 2048, "\n");
        }

        UTF8ToWideChar(string, w_string);
        MessageBoxW(hwnd, w_string, L"WavPack File Info Box", MB_OK);
        wpc = WavpackCloseFile(wpc);
    }
    else
        MessageBox(hwnd, string, "WavPack Decoder", MB_OK);

    return 0;
#else
	return 1;
#endif
}

void getfileinfo(const char *filename, char *title, int *length_in_ms)
{
    if (!filename || !*filename)      // currently playing file
	{

        if (length_in_ms)
            *length_in_ms = getlength ();

        if (title)
		{
            if (WavpackGetTagItem(curr.wpc, "title", NULL, 0))
			{
                char art [128], ttl [128];

                WavpackGetTagItem(curr.wpc, "title", ttl, sizeof (ttl));

                if (WavpackGetMode(curr.wpc) & MODE_APETAG)
					UTF8ToAnsi(ttl, sizeof (ttl));

                if (WavpackGetTagItem(curr.wpc, "artist", art, sizeof (art)))
				{
                    if (WavpackGetMode(curr.wpc) & MODE_APETAG)
                        UTF8ToAnsi(art, sizeof (art));

                    StringCchPrintf(title, GETFILEINFO_TITLE_LENGTH, "%s - %s", art, ttl);
                }
                else
					lstrcpyn(title, ttl, GETFILEINFO_TITLE_LENGTH);
            }
            else
			{
                char *p = curr.lastfn + strlen (curr.lastfn);

                while (*p != '\\' && p >= curr.lastfn)
                    p--;

                lstrcpyn(title, ++p, GETFILEINFO_TITLE_LENGTH);
            }
        }
    }
    else      // some other file
	{
        WavpackContext *wpc;
        char error [128];
        int open_flags;

        if (length_in_ms)
            *length_in_ms = -1000;

        if (title)
            *title = 0;

        open_flags = OPEN_TAGS | OPEN_NORMALIZE;

        if (config_bits & ALLOW_WVC)
            open_flags |= OPEN_WVC;

        if (!(config_bits & ALLOW_MULTICHANNEL))
            open_flags |= OPEN_2CH_MAX;

        wpc = WavpackOpenFileInput(filename, error, open_flags, 0);

        if (wpc)
		{
            if (length_in_ms)
                *length_in_ms = (int)(WavpackGetNumSamples(wpc) * 1000.0 / WavpackGetSampleRate(wpc));

            if (title && WavpackGetTagItem(wpc, "title", NULL, 0))
			{
                char art [128], ttl [128];

                WavpackGetTagItem(wpc, "title", ttl, sizeof (ttl));

                if (WavpackGetMode(wpc) & MODE_APETAG)
					UTF8ToAnsi(ttl, sizeof (ttl));

                if (WavpackGetTagItem(wpc, "artist", art, sizeof (art)))
				{
                    if (WavpackGetMode(wpc) & MODE_APETAG)
                        UTF8ToAnsi(art, sizeof (art));

                    StringCchPrintf(title, GETFILEINFO_TITLE_LENGTH, "%s - %s", art, ttl);
                }
                else
					lstrcpyn(title, ttl, GETFILEINFO_TITLE_LENGTH);
            }

            wpc = WavpackCloseFile(wpc);
        }

        if (title && !*title)
		{
            char *p = (char*)filename + strlen (filename);

            while (*p != '\\' && p >= filename) p--;
            lstrcpyn(title, ++p, GETFILEINFO_TITLE_LENGTH);
        }
    }
}

void eq_set(int on, char data [10], int preamp)
{
        // most plug-ins can't even do an EQ anyhow.. I'm working on writing
        // a generic PCM EQ, but it looks like it'll be a little too CPU
        // consuming to be useful :)
}

static int read_samples(struct wpcnxt *cnxt, int num_samples);

DWORD WINAPI __stdcall DecodeThread(void *b)
{
    int num_chans, sample_rate;
    int done = 0;

    memset(curr.error, 0, sizeof (curr.error));
    num_chans = WavpackGetReducedChannels(curr.wpc);
    sample_rate = WavpackGetSampleRate(curr.wpc);
 
    while (!*((int *)b) )
	{
        if (seek_needed != -1)
		{
            int seek_position = seek_needed;
            int bc = 0;

            seek_needed = -1;

            if (seek_position > getlength() - 1000 && getlength() > 1000)
                seek_position = getlength() - 1000; // don't seek to last second

            mod.outMod->Flush(decode_pos_ms = seek_position);

            if (WavpackSeekSample(curr.wpc, (int)(sample_rate / 1000.0 * seek_position))) {
                decode_pos_ms = (int)(WavpackGetSampleIndex(curr.wpc) * 1000.0 / sample_rate);
                mod.outMod->Flush(decode_pos_ms);
                continue;
            }
            else
                done = 1;
        }

        if (done) {
            mod.outMod->CanWrite();

            if (!mod.outMod->IsPlaying()) {
                PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
                return 0;
            }

            Sleep(10);
        }
        else if (mod.outMod->CanWrite() >= ((576 * num_chans * (curr.output_bits / 8)) << (mod.dsp_isactive () ? 1 : 0)))
		{
            int tsamples = read_samples (&curr, 576) * num_chans;
            int tbytes = tsamples * (curr.output_bits/8);

            if (tsamples)
			{
                mod.SAAddPCMData((char *) curr.sample_buffer, num_chans, curr.output_bits, decode_pos_ms);
                mod.VSAAddPCMData((char *) curr.sample_buffer, num_chans, curr.output_bits, decode_pos_ms);
                decode_pos_ms = (int)(WavpackGetSampleIndex(curr.wpc) * 1000.0 / sample_rate);

                if (mod.dsp_isactive())
                    tbytes = mod.dsp_dosamples ((short *) curr.sample_buffer,
                        tsamples / num_chans, curr.output_bits, num_chans, sample_rate) * (num_chans * (curr.output_bits/8));

                mod.outMod->Write ((char *) curr.sample_buffer, tbytes);
            }
            else
                done = 1;
        }
        else
		{
            mod.SetInfo((int) ((WavpackGetInstantBitrate (curr.wpc) + 500.0) / 1000.0), -1, -1, 1);
            Sleep(20);
        }
    }

    return 0;
}

/********* These functions provide the "transcoding" mode of winamp. *********/

extern "C" __declspec (dllexport) intptr_t winampGetExtendedRead_open (const char *fn, int *size, int *bps, int *nch, int *srate)
{
    struct wpcnxt *cnxt = (struct wpcnxt *)malloc(sizeof (struct wpcnxt));
    int num_chans, sample_rate, open_flags;
    char error[128];

#ifdef DEBUG_CONSOLE
    sprintf (error, "Read_open (%s)\n", fn);
    debug_write (error);
#endif

    if (!cnxt)
        return 0;

    memset(cnxt, 0, sizeof (struct wpcnxt));
    open_flags = OPEN_NORMALIZE | OPEN_WVC;

    if (!(config_bits & ALLOW_MULTICHANNEL) || *nch == 2)
        open_flags |= OPEN_2CH_MAX;

    if (config_bits & (REPLAYGAIN_TRACK | REPLAYGAIN_ALBUM))
        open_flags |= OPEN_TAGS;
 
    cnxt->wpc = WavpackOpenFileInput(fn, error, open_flags, 0);

    if (!cnxt->wpc)           // error opening file, just return error
	{
        free (cnxt);
        return 0;
    }

    num_chans = WavpackGetReducedChannels(cnxt->wpc);
    sample_rate = WavpackGetSampleRate(cnxt->wpc);

    if (num_chans > MAX_NCH)
	{
        WavpackCloseFile(cnxt->wpc);
        free (cnxt);
        return 0;
    }

    if (*bps != 16 && *bps != 24 && *bps != 32)
	{
        cnxt->output_bits = WavpackGetBitsPerSample(cnxt->wpc) > 16 ? 24 : 16;

        if (config_bits & ALWAYS_16BIT)
            cnxt->output_bits = 16;
        else if ((config_bits & (REPLAYGAIN_TRACK | REPLAYGAIN_ALBUM)) &&
            (config_bits & REPLAYGAIN_24BIT))
                cnxt->output_bits = 24;
    }
    else
        cnxt->output_bits = *bps;
 
    if (num_chans > MAX_NCH)    // don't allow too many channels!
	{
        WavpackCloseFile(cnxt->wpc);
        free (cnxt);
        return 0;
    }

    *nch = num_chans;
    *srate = sample_rate;
    *bps = cnxt->output_bits;
    *size = WavpackGetNumSamples(cnxt->wpc) * (*bps / 8) * (*nch);

    cnxt->play_gain = calculate_gain(cnxt->wpc);

#ifdef DEBUG_CONSOLE
    sprintf (error, "Read_open success! nch=%d, srate=%d, bps=%d, size=%d\n",
        *nch, *srate, *bps, *size);
    debug_write (error);
#endif

    return (intptr_t) cnxt;
}

extern "C" __declspec (dllexport) intptr_t winampGetExtendedRead_getData (intptr_t handle, char *dest, int len, int *killswitch)
{
    struct wpcnxt *cnxt = (struct wpcnxt *)handle;
    int num_chans = WavpackGetReducedChannels(cnxt->wpc);
    int bytes_per_sample = num_chans * cnxt->output_bits / 8;
    int used = 0;

#ifdef DEBUG_CONSOLE
    char error [128];
#endif

    while (used < len && !*killswitch)
	{
        int nsamples = (len - used) / bytes_per_sample, tsamples;

        if (!nsamples)
            break;
        else if (nsamples > 576)
            nsamples = 576;

        tsamples = read_samples(cnxt, nsamples) * num_chans;

        if (tsamples)
		{
            int tbytes = tsamples * (cnxt->output_bits/8);

            memcpy (dest + used, cnxt->sample_buffer, tbytes);
            used += tbytes;
        }
        else
            break;
    }

#ifdef DEBUG_CONSOLE
    sprintf (error, "Read_getData (%d), actualy read %d\n", len, used);
    debug_write (error);
#endif

    return used;
}

extern "C" __declspec (dllexport) int winampGetExtendedRead_setTime (intptr_t handle, int millisecs)
{
    struct wpcnxt *cnxt = (struct wpcnxt *) handle;
    int sample_rate = WavpackGetSampleRate(cnxt->wpc);

    return WavpackSeekSample(cnxt->wpc, (int)(sample_rate / 1000.0 * millisecs));
}

extern "C" __declspec (dllexport) void winampGetExtendedRead_close (intptr_t handle)
{
    struct wpcnxt *cnxt = (struct wpcnxt *) handle;

#ifdef DEBUG_CONSOLE
    char error [128];

    sprintf (error, "Read_close ()\n");
    debug_write (error);
#endif

    WavpackCloseFile(cnxt->wpc);
    free (cnxt);
}

/* This is a generic function to read WavPack samples and convert them to a
 * form usable by winamp. It includes conversion of any WavPack format
 * (including ieee float) to 16, 24, or 32-bit integers (with noise shaping
 * for the 16-bit case) and replay gain implementation (with optional soft
 * clipping). It is used by both the regular "play" code and the newer
 * transcoding functions.
 *
 * The num_samples parameter is the number of "composite" samples to
 * convert and is limited currently to 576 samples for legacy reasons. The
 * return value is the number of samples actually converted and will be
 * equal to the number requested unless an error occurs or the end-of-file
 * is encountered. The converted samples are stored (interleaved) at
 * cnxt->sample_buffer[].
 */

static int read_samples (struct wpcnxt *cnxt, int num_samples)
{
    int num_chans = WavpackGetReducedChannels(cnxt->wpc), samples, tsamples;

    samples = WavpackUnpackSamples(cnxt->wpc, (int32_t*) cnxt->sample_buffer, num_samples);
    tsamples = samples * num_chans;

    if (tsamples)
	{
        if (!(WavpackGetMode(cnxt->wpc) & MODE_FLOAT))
		{
            float scaler = (float) (1.0 / ((unsigned long) 1 << (WavpackGetBytesPerSample(cnxt->wpc) * 8 - 1)));
            float *fptr = (float *) cnxt->sample_buffer;
            long *lptr = cnxt->sample_buffer;
            int cnt = tsamples;

            while (cnt--)
                *fptr++ = *lptr++ * scaler;
        }

        if (cnxt->play_gain != 1.0)
		{
            float *fptr = (float *) cnxt->sample_buffer;
            int cnt = tsamples;
            double outval;

            while (cnt--)
			{
                outval = *fptr * cnxt->play_gain;

                /*if (cnxt->soft_clipping)
				{
                    if (outval > 0.75)
                        outval = 1.0 - (0.0625 / (outval - 0.5));
                    else if (outval < -0.75)
                        outval = -1.0 - (0.0625 / (outval + 0.5));
                }*/

                *fptr++ = (float) outval;
            }
        }

        if (cnxt->output_bits == 16)
		{
            float *fptr = (float *) cnxt->sample_buffer;
            short *sptr = (short *) cnxt->sample_buffer;
            int cnt = samples, ch;

            while (cnt--)
                for (ch = 0; ch < num_chans; ++ch)
				{
                    int dst;

                    *fptr -= cnxt->error [ch];

                    if (*fptr >= 1.0)
                        dst = 32767;
                    else if (*fptr <= -1.0)
                        dst = -32768;
                    else
                        dst = (int) floor (*fptr * 32768.0);

                    cnxt->error [ch] = (float)(dst / 32768.0 - *fptr++);
                    *sptr++ = dst;
                }
        }
        else if (cnxt->output_bits == 24)
		{
            unsigned char *cptr = (unsigned char *) cnxt->sample_buffer;
            float *fptr = (float *) cnxt->sample_buffer;
            int cnt = tsamples;
            long outval;

            while (cnt--) {
                if (*fptr >= 1.0)
                    outval = 8388607;
                else if (*fptr <= -1.0)
                    outval = -8388608;
                else
                    outval = (int) floor (*fptr * 8388608.0);

                *cptr++ = (unsigned char) outval;
                *cptr++ = (unsigned char) (outval >> 8);
                *cptr++ = (unsigned char) (outval >> 16);
                fptr++;
            }
        }
        else if (cnxt->output_bits == 32)
		{
            float *fptr = (float *) cnxt->sample_buffer;
            long *sptr = (long *) cnxt->sample_buffer;
            int cnt = tsamples;

            while (cnt--)
			{
                if (*fptr >= 1.0)
                    *sptr++ = 8388607 << 8;
                else if (*fptr <= -1.0)
                    *sptr++ = -8388608 << 8;
                else
                    *sptr++ = ((int) floor (*fptr * 8388608.0)) << 8;

                fptr++;
            }
        }
    }

    return samples;
}

extern "C" __declspec (dllexport) In_Module * winampGetInModule2()
{
    return &mod;
}

// This code provides an interface between the reader callback mechanism that
// WavPack uses internally and the standard fstream C library.

static int32_t read_bytes(void *id, void *data, int32_t bcount)
{
    FILE *file = id ? *(FILE**)id : NULL;

    if (file)
        return (int32_t) fread(data, 1, bcount, file);
    else
        return 0;
}

static uint32_t get_pos(void *id)
{
    FILE *file = id ? *(FILE**)id : NULL;

    if (file)
        return ftell(file);
    else
        return -1;
}

static int set_pos_abs(void *id, uint32_t pos)
{
    FILE *file = id ? *(FILE**)id : NULL;

    if (file)
        return fseek(file, pos, SEEK_SET);
    else
        return 0;
}

static int set_pos_rel(void *id, int32_t delta, int mode)
{
    FILE *file = id ? *(FILE**)id : NULL;

    if (file)
        return fseek(file, delta, mode);
    else
        return -1;
}

static int push_back_byte(void *id, int c)
{
    FILE *file = id ? *(FILE**)id : NULL;

    if (file)
        return ungetc(c, file);
    else
        return EOF;
}

static uint32_t get_length(void *id)
{
    FILE *file = id ? *(FILE**)id : NULL;
    struct stat statbuf;

    if (!file || fstat (fileno (file), &statbuf) || !(statbuf.st_mode & S_IFREG))
        return 0;
    else
        return statbuf.st_size;
}

static int can_seek(void *id)
{
    FILE *file = id ? *(FILE**)id : NULL;
    struct stat statbuf;

    return file && !fstat (fileno (file), &statbuf) && (statbuf.st_mode & S_IFREG);
}

static int32_t write_bytes(void *id, void *data, int32_t bcount)
{
    FILE *file = id ? *(FILE**)id : NULL;

    if (file)
        return (int32_t) fwrite (data, 1, bcount, file);
    else
        return 0;
}

static WavpackStreamReader freader = {
    read_bytes, get_pos, set_pos_abs,
	set_pos_rel, push_back_byte,
	get_length, can_seek, write_bytes
};

/* These functions provide UNICODE support for the winamp media library */

static int metadata_we_can_write(const char *metadata);

static void close_context(struct wpcnxt *cxt)
{
    if (cxt->wpc)
        WavpackCloseFile(cxt->wpc);

    if (cxt->wv_id)
        fclose(cxt->wv_id);

    if (cxt->wvc_id)
        fclose(cxt->wvc_id);

    memset(cxt, 0, sizeof (*cxt));
}

#ifdef ANSI_METADATA

extern "C" __declspec (dllexport) int winampGetExtendedFileInfo(char *filename, char *metadata, char *ret, int retlen)
{
    int open_flags = OPEN_TAGS;
    char error[128];
    int retval = 0;

#ifdef DEBUG_CONSOLE
    sprintf (error, "winampGetExtendedFileInfo (%s)\n", metadata);
    debug_write (error);
#endif

	if (!_stricmp(metadata, "type"))
	{
		ret[0] = '0';
		ret[1] = 0;
		return 1;
	}
	else if (!_stricmp(metadata, "family"))
	{
		int len;
		const char *p;
		if (!filename || !filename[0]) return 0;
		len = lstrlen(filename);
		if (len < 3 || '.' != filename[len - 3])  return 0;
		p = &filename[len - 2];
		if (!_stricmp(p, "wv")) { WASABI_API_LNGSTRING_BUF(IDS_FAMILY_STRING, ret, retlen); return 1; }
		return 0;
	}

    if (!filename || !*filename)
        return retval;

    if (!_stricmp(metadata, "length")) {   /* even if no file, return a 1 and write "0" */
        StringCchPrintf(ret, retlen, "%d", 0);
        retval = 1;
    }

    if (!info.wpc || strcmp(filename, info.lastfn) || !_stricmp(metadata, "formatinformation"))
	{
        close_context(&info);

        if (!(info.wv_id = fopen(filename, "rb")))
            return retval;

        if (config_bits & ALLOW_WVC)
		{
			int length = strlen(filename) + 10;
            char *wvc_name = (char *)malloc(length);

            if (wvc_name)
			{
                lstrcpyn(wvc_name, filename, length);
                StringCchCat(wvc_name, length, "c");
                info.wvc_id = fopen(wvc_name, "rb");
                free(wvc_name);
            }
        }

        info.wpc = WavpackOpenFileInputEx(&freader, &info.wv_id, info.wvc_id ? &info.wvc_id : NULL, error, open_flags, 0);

        if (!info.wpc)
		{
            close_context(&info);
            return retval;
        }

        lstrcpyn(info.lastfn, filename, MAX_PATH);
        info.w_lastfn [0] = 0;
    }

    if (!_stricmp(metadata, "formatinformation"))
	{
		wchar_t *temp = (wchar_t *)malloc(retlen * sizeof(wchar_t));
        generate_format_string(info.wpc, temp, retlen, 0);
		lstrcpyn(ret, AutoChar(temp), retlen);
		free(temp);
        retval = 1;
    }
    else if (!_stricmp(metadata, "length"))
	{
        StringCchPrintf(ret, retlen, "%d", (int)(WavpackGetNumSamples(info.wpc) * 1000.0 / WavpackGetSampleRate(info.wpc)));
        retval = 1;
    }
    else if (!_stricmp(metadata, "lossless"))
	{
        StringCchPrintf (ret, retlen, "%d", (WavpackGetMode(info.wpc) & MODE_LOSSLESS) ? 1 : 0);
        retval = 1;
    }
    else if (!_stricmp(metadata, "numsamples"))
	{
        StringCchPrintf(ret, retlen, "%d", WavpackGetNumSamples(info.wpc));
        retval = 1;
    }
    else if (!_stricmp(metadata, "mime"))
	{
		lstrcpyn(ret, L"audio/x-wavpack", retlen);
        retval = 1;
    }
	else if (!_stricmp(metadata, "gain"))
	{
		StringCchPrintf(ret, retlen, "%-+.2f dB", calculate_gain(info.wpc, false));
		retval = 1;
	}
    else if (WavpackGetTagItem(info.wpc, metadata, ret, retlen))
	{
		if (!_stricmp(metadata, "rating"))
		{
			int rating = atoi(ret);
			// appears to be generally 0-5 or 0-100
			if (rating > 10) {
				rating /= 20;
			}
			// or maybe we're dealing with a 1-10 range
			else if (rating > 5) {
				rating /= 2;
			}
			// otherwise it is hopefully in the 0-5 range

			StringCchPrintf(ret, retlen, "%u", rating);
		}
		else
		{
			if (WavpackGetMode(info.wpc) & MODE_APETAG)
			{
				UTF8ToAnsi(ret, retlen);
			}
		}

        retval = 1;
    }
    else if (metadata_we_can_write(metadata))
	{
        if (retlen)
            *ret = 0;

        retval = 1;
    }

    // This is a little ugly, but since the WavPack library has read the tags off the
    // files, we can close the files (but not the WavPack context) now so that we don't
    // leave handles open. We may access the file again for the "formatinformation"
    // field, so we reopen the file if we get that one.

    if (info.wv_id)
	{
        fclose(info.wv_id);
        info.wv_id = NULL;
    }

    if (info.wvc_id)
	{
        fclose(info.wvc_id);
        info.wvc_id = NULL;
    }

    return retval;
}

#endif

#ifdef UNICODE_METADATA

extern "C" __declspec (dllexport) int winampGetExtendedFileInfoW (wchar_t *filename, char *metadata, wchar_t *ret, int retlen)
{
    char error[128], res[256];
    int open_flags = OPEN_TAGS;
    int retval = 0;

#ifdef DEBUG_CONSOLE
    sprintf (error, "winampGetExtendedFileInfoW (%s)\n", metadata);
    debug_write (error);
#endif

	if (!_stricmp(metadata, "type"))
	{
		ret[0] = '0';
		ret[1] = 0;
		return 1;
	}
	else if (!_stricmp(metadata, "family"))
	{
		int len;
		const wchar_t *p;
		if (!filename || !filename[0]) return 0;
		len = lstrlenW(filename);
		if (len < 3 || L'.' != filename[len - 3])  return 0;
		p = &filename[len - 2];
		if (!_wcsicmp(p, L"wv")) { WASABI_API_LNGSTRINGW_BUF(IDS_FAMILY_STRING, ret, retlen); return 1; }
		return 0;
	}

    if (!filename || !*filename)
        return retval;

    if (!_stricmp(metadata, "length"))   /* even if no file, return a 1 and write "0" */
	{
        StringCchPrintfW(ret, retlen, L"%d", 0);
        retval = 1;
    }

    if (!info.wpc || wcscmp(filename, info.w_lastfn) || !_stricmp(metadata, "formatinformation"))
	{
        close_context(&info);

        if (!(info.wv_id = _wfopen(filename, L"rb")))
            return retval;

        if (config_bits & ALLOW_WVC) {
			int length = wcslen(filename) + 10;
            wchar_t *wvc_name = (wchar_t *)malloc(length * sizeof(wchar_t));

            if (wvc_name) {
                lstrcpynW(wvc_name, filename, length);
                StringCchCatW(wvc_name, length, L"c");
                info.wvc_id = _wfopen(wvc_name, L"rb");
                free(wvc_name);
            }
        }

        info.wpc = WavpackOpenFileInputEx(&freader, &info.wv_id, info.wvc_id ? &info.wvc_id : NULL, error, open_flags, 0);

        if (!info.wpc)
		{
            close_context(&info);
            return retval;
        }

        lstrcpynW(info.w_lastfn, filename, MAX_PATH);
        info.lastfn[0] = 0;
    }

    if (!_stricmp(metadata, "formatinformation"))
	{
		generate_format_string(info.wpc, ret, retlen, 0);
		retval = 1;
    }
    else if (!_stricmp (metadata, "length"))
	{
        StringCchPrintfW(ret, retlen, L"%d", (int)(WavpackGetNumSamples(info.wpc) * 1000.0 / WavpackGetSampleRate(info.wpc)));
        retval = 1;
    }
    else if (!_stricmp(metadata, "lossless"))
	{
        StringCchPrintfW(ret, retlen, L"%d", (WavpackGetMode(info.wpc) & MODE_LOSSLESS) ? 1 : 0);
        retval = 1;
    }
	else if (!_stricmp(metadata, "gain"))
	{
		StringCchPrintfW(ret, retlen, L"%-+.2f dB", calculate_gain(info.wpc, false));
		retval = 1;
	}
    else if (!_stricmp(metadata, "numsamples"))
	{
        StringCchPrintfW(ret, retlen, L"%d", WavpackGetNumSamples(info.wpc));
        retval = 1;
    }
    else if (!_stricmp(metadata, "mime"))
	{
		lstrcpynW(ret, L"audio/x-wavpack", retlen);
        retval = 1;
    }
    else if (WavpackGetTagItem(info.wpc, metadata, res, sizeof (res)))
	{
		if (!_stricmp(metadata, "rating"))
		{
			int rating = atoi(res);
			// appears to be generally 0-5 or 0-100
			if (rating > 10) {
				rating /= 20;
			}
			// or maybe we're dealing with a 1-10 range
			else if (rating > 5) {
				rating /= 2;
			}
			// otherwise it is hopefully in the 0-5 range

			StringCchPrintfW(ret, retlen, L"%u", rating);
		}
		else
		{
			if (!(WavpackGetMode(info.wpc) & MODE_APETAG))
				lstrcpynW(ret, AutoWide(res), retlen);
			else
				lstrcpynW(ret, AutoWide(res, CP_UTF8), retlen);
		}

        retval = 1;
    }
    else if (metadata_we_can_write (metadata))
	{
        if (retlen)
            *ret = 0;

        retval = 1;
    }

    // This is a little ugly, but since the WavPack library has read the tags off the
    // files, we can close the files (but not the WavPack context) now so that we don't
    // leave handles open. We may access the file again for the "formatinformation"
    // field, so we reopen the file if we get that one.

    if (info.wv_id)
	{
        fclose (info.wv_id);
        info.wv_id = NULL;
    }

    if (info.wvc_id)
	{
        fclose (info.wvc_id);
        info.wvc_id = NULL;
    }

    return retval;
}

#endif

#ifdef ANSI_METADATA

extern "C" int __declspec (dllexport) winampSetExtendedFileInfo(const char *filename, const char *metadata, char *val)
{
    char error[128];

#ifdef DEBUG_CONSOLE
    sprintf (error, "winampSetExtendedFileInfo (%s=%s)\n", metadata, val);
    debug_write (error);
#endif

    if (!filename || !*filename || !metadata_we_can_write(metadata))
        return 0;

    if (!edit.wpc || strcmp(filename, edit.lastfn))
	{
        if (edit.wpc)
            WavpackCloseFile(edit.wpc);

        edit.wpc = WavpackOpenFileInput(filename, error, OPEN_TAGS | OPEN_EDIT_TAGS, 0);

        if (!edit.wpc)
            return 0;

        lstrcpyn(edit.lastfn, filename, MAX_PATH);
        edit.w_lastfn [0] = 0;
    }

    if (strlen(val))
        return WavpackAppendTagItem(edit.wpc, metadata, val, strlen (val));
    else
        return WavpackDeleteTagItem(edit.wpc, metadata);
}

#endif

#ifdef UNICODE_METADATA

extern "C" int __declspec (dllexport) winampSetExtendedFileInfoW(const wchar_t *filename, const char *metadata, wchar_t *val)
{
    char error[128], utf8_val[256];

	lstrcpyn(utf8_val,AutoChar(val, CP_UTF8),sizeof(utf8_val) - 1);

#ifdef DEBUG_CONSOLE
    sprintf (error, "winampSetExtendedFileInfoW (%s=%s)\n", metadata, utf8_val);
    debug_write (error);
#endif

    if (!filename || !*filename || !metadata_we_can_write(metadata))
        return 0;

    if (!edit.wpc || wcscmp(filename, edit.w_lastfn))
	{
        if (edit.wpc)
		{
            WavpackCloseFile(edit.wpc);
            edit.wpc = NULL;
        }

        if (edit.wv_id)
            fclose(edit.wv_id);

        if (!(edit.wv_id = _wfopen(filename, L"r+b")))
            return 0;

        edit.wpc = WavpackOpenFileInputEx(&freader, &edit.wv_id, NULL, error, OPEN_TAGS | OPEN_EDIT_TAGS, 0);

        if (!edit.wpc)
		{
            fclose(edit.wv_id);
            return 0;
        }

        lstrcpynW(edit.w_lastfn, filename, MAX_PATH);
        edit.lastfn [0] = 0;
    }

    if (strlen(utf8_val))
        return WavpackAppendTagItem(edit.wpc, metadata, utf8_val, strlen (utf8_val));
    else
        return WavpackDeleteTagItem(edit.wpc, metadata);
}

#endif

extern "C" int __declspec (dllexport) winampWriteExtendedFileInfo(void)
{
#ifdef DEBUG_CONSOLE
    debug_write ("winampWriteExtendedFileInfo ()\n");
#endif

    if (edit.wpc)
	{
        WavpackWriteTag(edit.wpc);
        WavpackCloseFile(edit.wpc);
        edit.wpc = NULL;
    }

    if (edit.wv_id)
	{
        fclose(edit.wv_id);
        edit.wv_id = NULL;
    }

    close_context(&info);      // make sure we re-read info on any open files
    return 1;
}

// return 1 if you want winamp to show it's own file info dialogue, 0 if you want to show your own (via In_Module.InfoBox)
// if returning 1, remember to implement winampGetExtendedFileInfo("formatinformation")!
extern "C" __declspec(dllexport) int winampUseUnifiedFileInfoDlg(const wchar_t * fn)
{
    return 1;
}

static const char *writable_metadata [] = {
    "track", "genre", "year", "comment", "artist",
	"album", "title", "albumartist", "composer",
	"publisher", "disc", "tool", "encoder", "bpm",
	"category", "rating",
    "replaygain_track_gain", "replaygain_track_peak",
    "replaygain_album_gain", "replaygain_album_peak"
};

#define NUM_KNOWN_METADATA (sizeof (writable_metadata) / sizeof (writable_metadata [0]))

static int metadata_we_can_write(const char *metadata)
{
    if (!metadata || !*metadata)
        return 0;

    for (int i = 0; i < NUM_KNOWN_METADATA; ++i)
        if (!_stricmp(metadata, writable_metadata[i]))
            return 1;

    return 0;
}

static void generate_format_string(WavpackContext *wpc, wchar_t *string, int maxlen, int wide)
{
    int mode = WavpackGetMode(wpc);
    unsigned char md5_sum[16];
    wchar_t modes[256];
	wchar_t fmt[256];

	WASABI_API_LNGSTRINGW_BUF(IDS_ENCODER_VERSION, fmt, sizeof(fmt));
	StringCchPrintfW(string, maxlen, fmt, WavpackGetVersion(wpc));
	while (*string && string++ && maxlen--);

	WASABI_API_LNGSTRINGW_BUF(IDS_SOURCE, fmt, sizeof (fmt));
	StringCchPrintfW(string, maxlen, fmt, WavpackGetBitsPerSample(wpc),
					 WASABI_API_LNGSTRINGW((WavpackGetMode(wpc) & MODE_FLOAT) ? IDS_FLOATS : IDS_INTS),
					 WavpackGetSampleRate(wpc));
	while (*string && string++ && maxlen--);

    if (WavpackGetNumChannels(wpc) > 2)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_MULTICHANNEL, fmt, sizeof (fmt));
		StringCchPrintfW(string, maxlen, fmt, WavpackGetNumChannels(wpc));
		while (*string && string++ && maxlen--);
	}
    else if (WavpackGetNumChannels(wpc) == 2)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_STEREO, fmt, sizeof (fmt));
		StringCchPrintfW(string, maxlen, fmt);
		while (*string && string++ && maxlen--);
	}
	else
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_MONO, fmt, sizeof (fmt));
		StringCchPrintfW(string, maxlen, fmt);
		while (*string && string++ && maxlen--);
	}

    modes [0] = 0;

    if (WavpackGetMode(wpc) & MODE_HYBRID)
	{
        StringCchCatW(modes, 256, WASABI_API_LNGSTRINGW(IDS_HYBRID));
        StringCchCatW(modes, 256, L" ");
	}

    StringCchCatW(modes, 256, WASABI_API_LNGSTRINGW((WavpackGetMode(wpc) & MODE_LOSSLESS) ? IDS_LOSSLESS : IDS_LOSSY));

    if (WavpackGetMode(wpc) & MODE_FAST)
		StringCchCatW(modes, 256, WASABI_API_LNGSTRINGW(IDS_FAST));
    else if (WavpackGetMode(wpc) & MODE_VERY_HIGH)
        StringCchCatW(modes, 256, WASABI_API_LNGSTRINGW(IDS_VHIGH));
    else if (WavpackGetMode(wpc) & MODE_HIGH)
        StringCchCatW(modes, 256, WASABI_API_LNGSTRINGW(IDS_HIGH));

    if (WavpackGetMode(wpc) & MODE_EXTRA)
		StringCchCatW(modes, 256, WASABI_API_LNGSTRINGW(IDS_EXTRA));

    StringCchPrintfW(string, maxlen, L"%s:%s  %s\n",
					 WASABI_API_LNGSTRINGW(IDS_MODES),
					 (wide || lstrlenW(modes) < 24) ? L"" : L"\n", modes);
    while (*string && string++ && maxlen--);

    if (WavpackGetRatio(wpc) != 0.0)
	{
		wchar_t str_kbps[32];
        StringCchPrintfW(string, maxlen, L"%s:  %d %s \n",
						 WASABI_API_LNGSTRINGW(IDS_BITRATE),
						 (int)((WavpackGetAverageBitrate(wpc, TRUE) + 500.0) / 1000.0),
						 WASABI_API_LNGSTRINGW_BUF(IDS_KBPS, str_kbps, sizeof(str_kbps)));
        while (*string && string++ && maxlen--);
        StringCchPrintfW(string, maxlen, L"%s:  %.2f : 1 \n",
						 WASABI_API_LNGSTRINGW(IDS_RATIO), 1.0 / WavpackGetRatio(wpc));
        while (*string && string++ && maxlen--);
    }

    if (WavpackGetMD5Sum(wpc, md5_sum))
	{
        wchar_t md5s1 [17], md5s2 [17];
        int i;

        for (i = 0; i < 8; ++i)
		{
            StringCchPrintfW(md5s1 + i * 2, sizeof(md5s1), L"%02x", md5_sum [i]);
            StringCchPrintfW(md5s2 + i * 2, sizeof(md5s2), L"%02x", md5_sum [i+8]);
        }

		StringCchPrintfW(string, maxlen, (wide ? L"%s:  %s%s\n" : L"%s:\n  %s\n  %s\n"),
						 WASABI_API_LNGSTRINGW(IDS_MD5), md5s1, md5s2);
    }
}

///////////////////// native "C" functions required for AlbumArt support ///////////////////////////

#ifdef DEBUG_CONSOLE

static char temp_buff [256];

static char *wide2char (const wchar_t *src)
{
	char *dst = temp_buff;

	while (*src)
		*dst++ = (char) *src++;

	*dst = 0;
	return temp_buff;
}

#endif

int WavPack_HandlesExtension(const wchar_t *extension)
{
	return !_wcsicmp(extension, L"wv");
}

int WavPack_GetAlbumArt(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mime_type)
{
	char *buffer;
    char error[128];
	int tag_size, i;
    int retval = 1;

#ifdef DEBUG_CONSOLE
    sprintf (error, "WavPack_GetAlbumArt (%s)\n", wide2char (type));
    debug_write (error);
#endif

    if (!filename || !*filename || _wcsicmp(type, L"cover"))
        return retval;

    if (!info.wpc || wcscmp(filename, info.w_lastfn))
	{
        close_context(&info);

        if (!(info.wv_id = _wfopen(filename, L"rb")))
            return retval;

        if (config_bits & ALLOW_WVC)
		{
			int length = wcslen(filename) + 10;
            wchar_t *wvc_name = (wchar_t *)malloc(length * sizeof(wchar_t));

            if (wvc_name)
			{
                lstrcpynW(wvc_name, filename, length);
                StringCchCatW(wvc_name, length, L"c");
                info.wvc_id = _wfopen(wvc_name, L"rb");
                free(wvc_name);
            }
        }

        info.wpc = WavpackOpenFileInputEx(&freader, &info.wv_id, info.wvc_id ? &info.wvc_id : NULL, error, OPEN_TAGS, 0);

        if (!info.wpc)
		{
            close_context(&info);
            return retval;
        }

        lstrcpynW(info.w_lastfn, filename, MAX_PATH);
        info.lastfn[0] = 0;
    }

	tag_size = WavpackGetBinaryTagItem(info.wpc, "Cover Art (Front)", NULL, 0);

	if (!tag_size)
		return retval;

	buffer = (char*)WASABI_API_MEMMGR->sysMalloc(tag_size);
	WavpackGetBinaryTagItem(info.wpc, "Cover Art (Front)", buffer, tag_size);

	for (i = 0; i < tag_size - 1; ++i)
		if (!buffer[i] && strrchr(buffer, '.')) {
			char *ext = strrchr(buffer, '.') + 1;
			wchar_t *wcptr;

			wcptr = *mime_type = (wchar_t*)WASABI_API_MEMMGR->sysMalloc(strlen(ext) * 2 + 2);

			while (*ext)
				*wcptr++ = *ext++;

			*wcptr = 0;
			*bits = buffer;
			*len = tag_size - i - 1;
			memmove(buffer, buffer + i + 1, *len);
			retval = 0;
#ifdef DEBUG_CONSOLE
			sprintf (error, "WavPack_GetAlbumArt (\"%s\", %d) success!\n", wide2char (*mime_type), *len);
			debug_write (error);
#endif
		}

	if (retval)
		WASABI_API_MEMMGR->sysFree(buffer);

    // This is a little ugly, but since the WavPack library has read the tags off the
    // files, we can close the files (but not the WavPack context) now so that we don't
    // leave handles open. We may access the file again for the "formatinformation"
    // field, so we reopen the file if we get that one.

    if (info.wv_id)
	{
        fclose(info.wv_id);
        info.wv_id = NULL;
    }

    if (info.wvc_id)
	{
        fclose(info.wvc_id);
        info.wvc_id = NULL;
    }

    return retval;
}

int WavPack_SetAlbumArt(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mime_type)
{
#if 1
	return 2;	// return 2 to indicate "read-only" cover art for now
#else
    char error [128], name [50], *cp;
	int tag_size, retval = 0;
	unsigned char *buffer;

#ifdef DEBUG_CONSOLE
    sprintf (error, "WavPack_SetAlbumArt (%s)\n", wide2char (mime_type));
    debug_write (error);
#endif

    if (!filename || !*filename || _wcsicmp (type, L"cover") || wcslen (mime_type) > 16)
        return 1;

	strcpy (name, "Cover Art (Front)");
	cp = name + strlen (name);
	*cp++ = '.';

	while (*mime_type)
		*cp++ = (char) *mime_type++;

	*cp = 0;
	tag_size = strlen (name) + 1 + len;
	buffer = malloc (tag_size);
	strcpy (buffer, name);
	memcpy (buffer + strlen (buffer) + 1, bits, len);

    if (!edit.wpc || wcscmp (filename, edit.w_lastfn)) {
        if (edit.wpc) {
            WavpackCloseFile (edit.wpc);
            edit.wpc = NULL;
        }

        if (edit.wv_id)
            fclose (edit.wv_id);

        if (!(edit.wv_id = _wfopen (filename, L"r+b"))) {
			free (buffer);
            return 1;
		}

        edit.wpc = WavpackOpenFileInputEx (&freader, &edit.wv_id, NULL, error, OPEN_TAGS | OPEN_EDIT_TAGS, 0);

        if (!edit.wpc) {
            fclose (edit.wv_id);
			free (buffer);
            return 1;
        }

        wcscpy (edit.w_lastfn, filename);
        edit.lastfn [0] = 0;
    }

	retval = WavpackAppendTagItem (edit.wpc, "Cover Art (Front)", buffer, tag_size);
	free (buffer);

	if (retval) {
		winampWriteExtendedFileInfo ();
		return 0;
	}
	else {
		close_context (&edit);
		return 1;
	}
#endif
}

int WavPack_DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
#if 1
	return 2;	// return 2 to indicate "read-only" cover art for now
#else
    char error [128];

#ifdef DEBUG_CONSOLE
    sprintf (error, "WavPack_DeleteAlbumArt ()\n");
    debug_write (error);
#endif

    if (!filename || !*filename || _wcsicmp (type, L"cover"))
        return 0;

    if (!edit.wpc || wcscmp (filename, edit.w_lastfn)) {
        if (edit.wpc) {
            WavpackCloseFile (edit.wpc);
            edit.wpc = NULL;
        }

        if (edit.wv_id)
            fclose (edit.wv_id);

        if (!(edit.wv_id = _wfopen (filename, L"r+b")))
            return 1;

        edit.wpc = WavpackOpenFileInputEx (&freader, &edit.wv_id, NULL, error, OPEN_TAGS | OPEN_EDIT_TAGS, 0);

        if (!edit.wpc) {
            fclose (edit.wv_id);
            return 1;
        }

        wcscpy (edit.w_lastfn, filename);
        edit.lastfn [0] = 0;
    }

	if (WavpackDeleteTagItem (edit.wpc, "Cover Art (Front)")) {
		winampWriteExtendedFileInfo ();
		return 0;
	}
	else {
		close_context (&edit);
		return 1;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
// This function uses the ReplayGain mode selected by the user and the info //
// stored in the specified tag to determine the gain value used to play the //
// file and whether "soft clipping" is required. Note that the gain is in   //
// voltage scaling (not dB), so a value of 1.0 (not 0.0) is unity gain.     //
//////////////////////////////////////////////////////////////////////////////

static float calculate_gain(WavpackContext *wpc, bool allowDefault)
{
	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain", false))
	{
		float dB = 0, peak = 1.0f;
		char gain[128] = "", peakVal[128] = "";
		_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();

		switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_source", 0))
		{
		case 0:   // track
			if ((!WavpackGetTagItem(wpc, "replaygain_track_gain", gain, sizeof(gain)) || !gain[0])
				&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				WavpackGetTagItem(wpc, "replaygain_album_gain", gain, sizeof(gain));

			if ((!WavpackGetTagItem(wpc, "replaygain_track_peak", peakVal, sizeof(peakVal)) || !peakVal[0])
				&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				WavpackGetTagItem(wpc, "replaygain_album_peak", peakVal, sizeof(peakVal));
			break;
		case 1:
			if ((!WavpackGetTagItem(wpc, "replaygain_album_gain", gain, sizeof(gain)) || !gain[0])
				&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				WavpackGetTagItem(wpc, "replaygain_track_gain", gain, sizeof(gain));

			if ((!WavpackGetTagItem(wpc, "replaygain_album_peak", peakVal, sizeof(peakVal)) || !peakVal[0])
				&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				WavpackGetTagItem(wpc, "replaygain_track_peak", peakVal, sizeof(peakVal));
			break;
		}

		if (gain[0])
		{
			if (gain[0] == L'+')
				dB = (float)_atof_l(&gain[1],C_locale);
			else
				dB = (float)_atof_l(gain,C_locale);
		}
		else if (allowDefault)
		{
			dB = AGAVE_API_CONFIG->GetFloat(playbackConfigGroupGUID, L"non_replaygain", -6.0);
			return powf(10.0f, dB / 20.0f);
		}

		if (peakVal[0])
		{
			peak = (float)_atof_l(peakVal,C_locale);
		}

		switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_mode", 1))
		{
		case 0:   // apply gain
			return powf(10.0f, dB / 20.0f);
		case 1:   // apply gain, but don't clip
			return min(powf(10.0f, dB / 20.0f), 1.0f / peak);
		case 2:   // normalize
			return 1.0f / peak;
		case 3:   // prevent clipping
			if (peak > 1.0f)
				return 1.0f / peak;
			else
				return 1.0f;
		}
	}

	return 1.0f; // no gain
}

// Convert a Ansi string into its Unicode UTF-8 format equivalent. The
// conversion is done in-place so the maximum length of the string buffer must
// be specified because the string may become longer or shorter. If the
// resulting string will not fit in the specified buffer size then it is
// truncated.
#ifdef OLD_INFO_DIALOG
static void AnsiToUTF8(char *string, int len)
{
    int max_chars = (int) strlen(string);
    wchar_t *temp = (wchar_t *) malloc((max_chars + 1) * 2);

    MultiByteToWideChar(CP_ACP, 0, string, -1, temp, max_chars + 1);
	lstrcpyn(string, AutoChar(temp, CP_UTF8), len);
    free(temp);
}
#endif

// Convert Unicode UTF-8 string to wide format. UTF-8 string must be NULL
// terminated. Resulting wide string must be able to fit in provided space
// and will also be NULL terminated. The number of characters converted will
// be returned (not counting terminator).

static int UTF8ToWideChar(const char *pUTF8, wchar_t *pWide)
{
    int trail_bytes = 0;
    int chrcnt = 0;

    while (*pUTF8)
	{
        if (*pUTF8 & 0x80)
		{
            if (*pUTF8 & 0x40)
			{
                if (trail_bytes)
				{
                    trail_bytes = 0;
                    chrcnt++;
                }
                else
				{
                    char temp = *pUTF8;

                    while (temp & 0x80)
					{
                        trail_bytes++;
                        temp <<= 1;
                    }

                    pWide [chrcnt] = temp >> trail_bytes--;
                }
            }
            else if (trail_bytes)
			{
                pWide [chrcnt] = (pWide [chrcnt] << 6) | (*pUTF8 & 0x3f);

                if (!--trail_bytes)
                    chrcnt++;
            }
        }
        else
            pWide [chrcnt++] = *pUTF8;

        pUTF8++;
    }

    pWide [chrcnt] = 0;
    return chrcnt;
}

// Convert a Unicode UTF-8 format string into its Ansi equivalent. The
// conversion is done in-place so the maximum length of the string buffer must
// be specified because the string may become longer or shorter. If the
// resulting string will not fit in the specified buffer size then it is
// truncated.

static void UTF8ToAnsi(char *string, int len)
{
    int max_chars = (int)strlen(string);
    wchar_t *temp = (wchar_t *)malloc((max_chars + 1) * 2);
    int act_chars = UTF8ToWideChar(string, temp);

    while (act_chars)
	{
        memset (string, 0, len);

        if (WideCharToMultiByte(CP_ACP, 0, temp, act_chars, string, len - 1, NULL, NULL))
            break;
        else
            act_chars--;
    }

    if (!act_chars)
        *string = 0;

    free (temp);
}

extern "C" __declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param)
{
	// as we're not hooking anything and have no settings we can support an on-the-fly uninstall action
	return IN_PLUGIN_UNINSTALL_NOW;
}