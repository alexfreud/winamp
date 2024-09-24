#pragma once
#ifndef  WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "giofile.h"
#include "dxhead.h"
#include "CVbriHeader.h"
#include "resource.h"
#include "in2.h"

#define PLUGIN_VERSION L"4.6"

extern In_Module mod;

extern char INI_FILE[MAX_PATH];
extern int g_length;
extern int lastfn_data_ready;
extern int id3Dlg(const wchar_t *fn, HWND hwnd);
extern int getlength();
extern void getfileinfo(const wchar_t *filename, wchar_t *title, int *length_in_ms);

extern int config_max_bufsize_k;
extern int config_gapless;
extern int config_fastvis;
extern unsigned char config_miscopts;
extern unsigned char config_downmix, config_downsample;
extern int config_http_buffersize, config_http_prebuffer, config_http_prebuffer_underrun;
extern unsigned char allow_sctitles,sctitle_format, allow_scartwork;
extern char config_http_save_dir[MAX_PATH];

extern wchar_t lastfn[8192];	// currently playing file (used for getting info on the current file)
extern char g_stream_title[256];
extern char lastfn_status[256];
extern int lastfn_status_err;
extern int paused;				// are we paused?
extern void  config_read();
extern void about(HWND hwndParent);

extern void strmBuf_Quit();
extern int strmBuf_Start(char *streamurl, int num_bytes, int pre_buffer_top, int pre_buffer_bottom);
extern int strmBuf_Read(void *data, int bytes_requested);

extern void config(HWND hwndParent);
extern volatile int killDecodeThread;

extern unsigned char eq_preamp;
extern unsigned char eq_enabled;
extern unsigned char eq_tab[10];
extern unsigned char config_eqmode;
extern unsigned int winampVersion;
extern int g_eq_ok;

extern CRITICAL_SECTION g_lfnscs;
extern CRITICAL_SECTION streamInfoLock;

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

extern void processMetaDataC(char *data, int len, int msgId );

enum
{
	UVOX_METADATA_STYLE_AOLRADIO = 0,
	UVOX_METADATA_STYLE_SHOUTCAST = 1,
	UVOX_METADATA_STYLE_SHOUTCAST2 = 2,
	UVOX_METADATA_STYLE_SHOUTCAST2_ARTWORK = 3,
	UVOX_METADATA_STYLE_SHOUTCAST2_ARTWORK_PLAYING = 4,
};

typedef struct {
	void *Next;
	int style;
	long timer;
	char title[16384];
	int part;
	int total_parts;
	int part_len;
	int type;
	} TitleType;
#define TITLELISTTYPE TitleType

extern TITLELISTTYPE *TitleLinkedList;
extern TITLELISTTYPE  TitleListTerminator;

extern void initTitleList(void);
extern TITLELISTTYPE *newTitleListEntry(void);
extern void removeTitleListEntry(TITLELISTTYPE *Entry);
extern void clearTitleList(void);

char *GetUltravoxUserAgent();
char *GetUserAgent();
void w9x_lstrcpynW(wchar_t *dest, const wchar_t *src, int maxLen);

// maximum acceptable deviance between LAME header bytesize and real filesize (minus id3 tag)
// has to be large enough to accomodate unknown tags (APE, lyrics3) 
const int MAX_ACCEPTABLE_DEVIANCE = 16384; 
void get_extended_info(const wchar_t *fn, int *len);
#define UVOX_3901_LEN 32768

void ConvertTryUTF8(const char *in, wchar_t *out, size_t outlen);

#ifdef AAC_SUPPORT
extern char config_extlist_aac[129];
#define config_extlist config_extlist_aac
#else
extern char config_extlist[129];
#endif

extern int m_force_seek;
extern CGioFile *g_playing_file;

int _r_i(char *name, int def);
void _w_i(char *name, int d);