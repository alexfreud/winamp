#ifndef NULLSOFT_IN_FLAC_MAIN_H
#define NULLSOFT_IN_FLAC_MAIN_H

#define PLUGIN_VER L"3.2"

#include <windows.h>
extern HANDLE killswitch;

#include "../Winamp/in2.h"
extern In_Module plugin;

DWORD CALLBACK FLACThread(LPVOID param);
extern int pan, volume;
extern volatile int currentSongLength;

void CALLBACK APCPause(ULONG_PTR data);
void CALLBACK APCSeek(ULONG_PTR data);

void ResetMetadataCache();

#include <FLAC/all.h>
void InterleaveAndTruncate(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize, double gain=1.0);

extern const wchar_t *winampINI;
char *BuildExtensions(const char *extensions);
extern bool config_average_bitrate;
extern bool fixBitrate;

extern int m_force_seek; // set this to something other than -1 to make the file start from the given time (in ms)
extern wchar_t *lastfn;

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
  {
    0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
  };

#define DEFAULT_EXTENSIONS "FLAC"
#define DEFAULT_EXTENSIONSW L"FLAC"
#endif