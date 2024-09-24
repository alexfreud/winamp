#ifndef NULLSOFT_IN_MP4_MAINH
#define NULLSOFT_IN_MP4_MAINH

#include "mp4.h"
#include "../Winamp/in2.h"
#include "mpeg4audio.h"
#include "mpeg4video.h"
#include "AudioSample.h"
#include "../nu/AutoLock.h"
#include "../nu/VideoClock.h"

extern nu::VideoClock video_clock;

MP4TrackId GetAudioTrack(MP4FileHandle infile);
MP4TrackId GetVideoTrack(MP4FileHandle infile);
int GetAACTrack(MP4FileHandle infile);
class waServiceFactory;
bool CreateVideoDecoder(MP4FileHandle file, MP4TrackId track, MP4VideoDecoder *&decoder, waServiceFactory *&serviceFactory);

class MP4AudioDecoder;

bool CreateDecoder(MP4FileHandle file, MP4TrackId track, MP4AudioDecoder *&decoder, waServiceFactory *&serviceFactory);

void ConfigureDecoderASC(MP4FileHandle file, MP4TrackId track, MP4AudioDecoder *decoder);
bool GetCustomMetadata(MP4FileHandle mp4, char *metadata, wchar_t *dest, int destlen, const char *owner=0);
float GetGain(MP4FileHandle mp4, bool allowDefault=true);
void GetGaps(MP4FileHandle mp4, unsigned __int32 &pre, unsigned __int32 &post);

struct ThreadInfoBox
{
	HWND hwndDlg;
	HANDLE completionEvent;
};
VOID CALLBACK CurrentlyPlayingInfoBox(ULONG_PTR param);

extern wchar_t lastfn[MAX_PATH*4];

extern HANDLE killEvent, seekEvent, pauseEvent;
extern In_Module mod;			// the output module 
extern MP4FileHandle MP4hFile;
extern MP4TrackId audio_track, video_track;
class AudioSample;
int TryWriteAudio(AudioSample *sample);
extern MP4AudioDecoder *audio;
extern MP4VideoDecoder *video;
extern unsigned int audio_bitrate, video_bitrate;
extern MP4SampleId numSamples, numVideoSamples;
extern DWORD WINAPI PlayProc(LPVOID lpParameter);
extern bool first;

extern HANDLE hThread;

extern Nullsoft::Utility::LockGuard play_mp4_guard;

extern volatile int m_needseek;
void CALLBACK Seek(ULONG_PTR data);
void CALLBACK Pause(ULONG_PTR data);
extern uint32_t m_video_timescale;
extern uint32_t m_timescale;
extern const wchar_t *defaultExtensions;
extern wchar_t m_ini[MAX_PATH];
char *BuildExtensions(const char *extensions);
extern bool config_show_average_bitrate;
void FlushOutput();
extern int m_force_seek;
MP4Duration GetClock();
MP4Duration GetDecodeClock();
extern bool audio_chunk;
// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
  {
    0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
  };


#endif