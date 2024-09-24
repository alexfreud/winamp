#ifndef _VIDEO_H
#define _VIDEO_H
#include "main.h"
#include "wa_ipc.h"

#ifdef __cplusplus
extern "C"  {
#endif
	void SetExteriorSize(int width, int height);
	int video_getNumAudioTracks();
	int video_getNumVideoTracks();
	int video_getCurAudioTrack();
	int video_getCurVideoTrack();
	int video_setCurAudioTrack(int track);
	int video_setCurVideoTrack(int track);
	void Vid_Cmd( windowCommand *wc);
	HWND videoGetHwnd();
	DWORD videoGetWidthHeightDWORD();
	void *video_getIVideoOutput();
	void videoAdSizeChanged();
	void videoReinit();
	void videoGoFullscreen();
	void videoSetFlip(int on);
#ifdef __cplusplus
}
#endif

#endif