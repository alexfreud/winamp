#pragma once
#include "../winamp/in2.h"
#include "../nu/VideoClock.h"
#include "../nsavi/nsavi.h"
extern In_Module plugin, *dshow_mod;
DWORD CALLBACK AVIPlayThread(LPVOID param);
extern HANDLE killswitch, seek_event;
extern volatile LONG seek_position;
extern int g_duration;
extern nu::VideoClock video_clock;
extern int video_only;
extern HMODULE in_dshow;

/* InfoDialog.cpp */
INT_PTR CALLBACK InfoDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void GetVideoCodecName(wchar_t *str, size_t str_cch, nsavi::STRF *stream_format); 
void GetVideoCodecDescription(wchar_t *str, size_t str_cch, nsavi::STRF *stream_format);
void GetAudioCodecName(wchar_t *str, size_t str_cch, nsavi::STRF *stream_format);
void GetAudioCodecDescription(wchar_t *str, size_t str_cch, nsavi::STRF *stream_format);