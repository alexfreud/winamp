#ifndef NULLSOFT_IN_FLV_MAIN_H
#define NULLSOFT_IN_FLV_MAIN_H

#include <windows.h>
#include "../Winamp/in2.h"
#include "../nu/VideoClock.h"
#include "../nu/AutoLock.h"

/* main.cpp */
extern In_Module plugin;
extern In_Module *swf_mod;
extern HMODULE in_swf;
extern wchar_t *playFile;
extern HANDLE killswitch;
extern int m_need_seek;
extern int paused;
extern int g_length;
extern nu::VideoClock video_clock;
extern wchar_t *stream_title;
extern Nullsoft::Utility::LockGuard stream_title_guard;

/* PlayThread.cpp */

DWORD CALLBACK PlayProcedure(LPVOID param);


#endif