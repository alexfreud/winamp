#include "Main.h"
#include "video.h"
extern "C"
{ 
	extern wchar_t vidoutbuf_save[];
}
int WINAPI VideoIPCProcedure(int which, WPARAM data, LRESULT *returnValue)
{
	switch (which)
	{
	case IPC_GETNUMAUDIOTRACKS:
		*returnValue = video_getNumAudioTracks();
		return 1;

	case IPC_GETNUMVIDEOTRACKS:
		*returnValue = video_getNumVideoTracks();
		return 1;

	case IPC_GETAUDIOTRACK:
		*returnValue = video_getCurAudioTrack();
		return 1;

	case IPC_GETVIDEOTRACK:
		*returnValue = video_getCurVideoTrack();
		return 1;

	case IPC_SETAUDIOTRACK:
		*returnValue = video_setCurAudioTrack(data);
		return 1;

	case IPC_SETVIDEOTRACK:
		*returnValue = video_setCurVideoTrack(data); 
		return 1;

	case IPC_SETSTOPONVIDEOCLOSE:
		config_video_stopclose = data;
		*returnValue = 0;
		return 1;

	case IPC_GETSTOPONVIDEOCLOSE:
		*returnValue = config_video_stopclose;
		return 1;

	case IPC_GETVIDEORESIZE:
		*returnValue = config_video_updsize;
		return 1;

	case IPC_SETVIDEORESIZE:
		config_video_updsize = data;
		*returnValue = 0;
		return 1;

	case IPC_GETWND:
		if (data == IPC_GETWND_VIDEO)
		{
			*returnValue = (LRESULT)hVideoWindow;
			return 1;
		}
		break;

	case IPC_ISWNDVISIBLE:
		if (data == IPC_GETWND_VIDEO)
		{
			*returnValue = config_video_open;
			return 1;
		}
		break;

	case IPC_ENABLEDISABLE_ALL_WINDOWS:
		EnableWindow(hVideoWindow, data != 0xdeadbeef);
		break;

	case IPC_GETINFO:
		if (data == 3)
		{
			*returnValue = videoGetWidthHeightDWORD();
			return 1;
		}
		if (data == 4)
		{
			*returnValue = (LRESULT) vidoutbuf_save;
			return 1;
		}
		break;

	case IPC_HAS_VIDEO_SUPPORT:
		*returnValue = g_has_video_plugin;
		return 1;

	case IPC_IS_PLAYING_VIDEO:
		*returnValue = video_isVideoPlaying() ? 2 : 0;
		return 1;

	case IPC_IS_FULLSCREEN:
		if (is_fullscreen_video)
		{
			*returnValue = 1;
			return 1;
		}
		break;

	case IPC_GET_IVIDEOOUTPUT:
		*returnValue = (LRESULT)video_getIVideoOutput();
		return 1;

	case IPC_VIDCMD:
		Vid_Cmd((windowCommand*)data);
		break;
	}
	return 0;
}