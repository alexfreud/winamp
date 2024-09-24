#ifndef NULLSOFT_DECODETHREADH
#define NULLSOFT_DECODETHREADH

#include <windows.h>

DWORD WINAPI DecodeThread(LPVOID b);

extern volatile int seek_needed;
extern CRITICAL_SECTION g_lfnscs;
extern int g_ds;
extern int g_sndopened;
extern int g_bufferstat;
extern int g_length;
extern volatile int g_closeaudio;
extern int decode_pos_ms; // current decoding position, in milliseconds. 
extern int g_vis_enabled;


#endif