#pragma once

extern class CWAAudioRenderer *nullfilter;

#include <windows.h>
#include <streams.h>
#include <strsafe.h>

typedef struct tagVIDEOINFOHEADER2 {
    RECT                rcSource;
    RECT                rcTarget;
    DWORD               dwBitRate;
    DWORD               dwBitErrorRate;
    REFERENCE_TIME      AvgTimePerFrame;
    DWORD               dwInterlaceFlags;
    DWORD               dwCopyProtectFlags;
    DWORD               dwPictAspectRatioX; 
    DWORD               dwPictAspectRatioY; 
    DWORD               dwReserved1;        
    DWORD               dwReserved2;        
    BITMAPINFOHEADER    bmiHeader;
} VIDEOINFOHEADER2;
#include <AtlBase.h>

extern IGraphBuilder *pGraphBuilder;
extern IMediaControl *pMediaControl;

extern bool has_audio;
extern int audio_bps, audio_srate, audio_nch;
extern int m_float, m_src_bps;
extern int m_is_capture;
extern HWND m_notif_hwnd;
extern int m_bitrate;
#include "../Winamp/in2.h"
extern In_Module mod;			// the output module (filled in near the bottom of this file)

void releaseObjects();
