#ifndef __MIKAMP_MAIN_H__
#define __MIKAMP_MAIN_H__

#include <windows.h>
#include <stdio.h>
#include "mikmod.h"
#include "mplayer.h"
#include "resource.h"
#include "in2.h"

#define INFO_CPAGES 3 

#define CPLAYFLG_LOOPALL      (1ul<<0)  // disables selective looping - loop everything!
#define CPLAYFLG_PLAYALL      (1ul<<1)  // plays hidden patterns (tack onto end of the song)
#define CPLAYFLG_FADEOUT      (1ul<<2)  // Fadeout the song before the end cometh?
#define CPLAYFLG_STRIPSILENCE (1ul<<3)  // Strip silence at the end of the song?
#define CPLAYFLG_SEEKBYORDERS (1ul<<4)  // Seek by orders instead of seconds
#define CPLAYFLG_CONT_LOOP    (1ul<<5)  // continue after loop

typedef struct tag_dlghdr
{   
    HWND     hwndTab;       // tab control 
    HWND     hwndDisplay;   // current child dialog box 
	int      left,top;
    HWND     apRes[INFO_CPAGES]; 

	UNIMOD  *module;
    MPLAYER *seeker;
    int      maxv;

    BOOL     inUse, ownModule;
    BOOL    *suse;

} DLGHDR;


typedef struct INFOBOX
{
    HWND    hwnd;
    DLGHDR  dlg;
    struct INFOBOX *next;
} INFOBOX;


#ifdef __cplusplus
extern "C" {
#endif

extern UBYTE      config_nopan, config_savestr;
extern MD_DEVICE  drv_amp;
extern MD_DEVICE drv_buffer;
extern In_Module  mikmod;
extern UNIMOD    *mf;
extern MPLAYER   *mp;

// Defined in INFO.C
// -----------------
extern INFOBOX *infobox_list;
extern void     infoDlg(HWND hwnd, UNIMOD *m, BOOL activate, BOOL primiary);
extern int      config_info_x, config_info_y, config_track;


// Defined in INFO.C
// -----------------
// defined in config.c

extern UBYTE    config_interp;
extern UBYTE    config_panrev;
extern UBYTE    config_cpu;
extern uint     config_srate,    config_voices, config_playflag;
extern int      config_pansep, config_loopcount;
extern UBYTE    config_samplesize;

extern UBYTE    config_resonance;
extern int      config_fadeout;
extern int		config_tsel;

extern int      paused;


// config.c shizat
// ---------------
extern void  set_priority(void);

extern void __cdecl config(HWND hwndParent);
extern void __cdecl about(HWND hwndParent);

extern void config_read();
extern void config_write();

extern void info_killseeker(HWND hwnd);

int GetNumChannels();
int AllowSurround();
int GetThreadPriorityConfig();

BOOL GetTypeInfo(LPCWSTR pszType, LPWSTR pszDest, INT cchDest);

#ifdef __cplusplus
};
#endif

//#define PLUGIN_NAME "Nullsoft Module Decoder"
#define PLUGIN_VER L"2.94"

#endif
