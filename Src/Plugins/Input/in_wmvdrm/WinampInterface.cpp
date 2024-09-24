#include "Main.h"
#include "WinampInterface.h"
#include "../Winamp/wa_ipc.h"
#include <cassert>
#include "WMDRMModule.h"
#include <strsafe.h>
#include "WMPlaylist.h"
#include "../nu/AutoChar.h"
WinampInterface winamp;

extern WMDRM mod;
using namespace Nullsoft::Utility;

#ifndef NO_DRM
#include "vid_overlay.h"
#include "vid_ddraw.h"

OverlayVideoOutput overlay;
DDrawVideoOutput ddraw;
#endif


WinampInterface::WinampInterface()
		: videoWindow(0), bufferCount(0),
		statusGuard(GUARDNAME("WinampInterface::statusGuard"))
{
	statusFilename[0] = 0;
	status[0] = 0;
}

/*
@returns winamp's video window handle 
*/
HWND WinampInterface::GetVideoWindow()
{
	return (HWND)GetVideoOutput()->extended(VIDUSER_GET_VIDEOHWND, 0, 0); // ask for the video hwnd
}

IVideoOutput *WinampInterface::GetVideoOutput()
{
	if (!videoWindow)
		videoWindow = (IVideoOutput *)SendMessage(GetWinampWindow(), WM_WA_IPC, 0, IPC_GET_IVIDEOOUTPUT);		// ask winamp for an interface to the video output
	return videoWindow;

}

void WinampInterface::EndOfFile()
{
	PostMessage(GetWinampWindow(), WM_WA_MPEG_EOF, 0, 0);
}

HWND WinampInterface::GetWinampWindow()
{
	return plugin.hMainWindow;
}

void WinampInterface::SetStatus(wchar_t *_status)
{
	{
		AutoLock lock (statusGuard);
		StringCchCopy(status, 1024, _status);
		StringCchCopy(statusFilename, FILENAME_SIZE, activePlaylist.GetFileName());
	}
	PostMessage(GetWinampWindow(), WM_WA_IPC, 0, IPC_UPDTITLE);
}

bool WinampInterface::GetStatus(wchar_t *title, size_t titleLen, const wchar_t *filename)
{
	AutoLock lock (statusGuard);

	if (status[0] && title && filename && !lstrcmpi(statusFilename, filename))
	{
		StringCchPrintf(title, titleLen, L"[%s]%s", status, filename);
		return true;
	}
	else
		return false;
}

bool WinampInterface::GetStatusHook(wchar_t *title, size_t titleLen, const wchar_t *filename)
{
	AutoLock lock (statusGuard);

	if (status[0] && title && filename && !lstrcmpi(statusFilename, filename))
	{
		wchar_t *oldTitle = _wcsdup(title);
		StringCchPrintf(title, titleLen, L"[%s]%s", status, oldTitle);
		free(oldTitle);
		return true;
	}
	else
		return false;
}

bool WinampInterface::HasStatus(const wchar_t *filename)
{
	AutoLock lock (statusGuard);
	if (status[0] && filename && !lstrcmpi(statusFilename, filename))
		return true;
	return false;
}

void WinampInterface::ClearStatus()
{
	{
		//AutoLock lock (statusGuard); // should be safe not to lock here
		status[0] = 0;
		statusFilename[0]=0;
	}
	PostMessage(GetWinampWindow(), WM_WA_IPC, 0, IPC_UPDTITLE);
}

void WinampInterface::EncryptedDrawFrame(void *frame)
{
#ifndef NO_DRM
	overlay.SetFrame(frame);
	ddraw.SetFrame(frame);
	SecureZeroMemory(&frame, sizeof(void *));
	GetVideoOutput()->draw((void *)1);
#endif
}

bool WinampInterface::OpenEncryptedVideo(int width, int height, bool flip, double aspect, int fourcc)
{
#ifndef NO_DRM
	VideoOpenStruct openVideo = {width, height, flip, aspect, fourcc};

	bool openedOK = false;
	if (config_video.overlays())
	{
		openedOK = !!GetVideoOutput()->extended(VIDUSER_OPENVIDEORENDERER, (intptr_t)(VideoRenderer *)&overlay, (intptr_t)&openVideo);
		if (openedOK)
			return true;
	}

	openedOK = !!GetVideoOutput()->extended(VIDUSER_OPENVIDEORENDERER, (intptr_t)(VideoRenderer *)&ddraw, (intptr_t)&openVideo);
  if (openedOK)
		return true;
#endif

	return false;
}

void WinampInterface::CloseEncryptedVideo()
{
	GetVideoOutput()->extended(VIDUSER_CLOSEVIDEORENDERER, 0, 0);
}

void WinampInterface::Buffering(int bufStatus, const wchar_t *displayString)
{
	char tempdata[75*2] = {0, };

	int csa = plugin.SAGetMode();
	if (csa & 1)
	{
		for (int x = 0; x < bufStatus*75 / 100; x ++)
			tempdata[x] = x * 16 / 75;
	}
	else if (csa&2)
	{
		int offs = (csa & 1) ? 75 : 0;
		int x = 0;
		while (x < bufStatus*75 / 100)
		{
			tempdata[offs + x++] = -6 + x * 14 / 75;
		}
		while (x < 75)
		{
			tempdata[offs + x++] = 0;
		}
	}
	else if (csa == 4)
	{
		tempdata[0] = tempdata[1] = (bufStatus * 127 / 100);
	}
	if (csa)	plugin.SAAdd(tempdata, ++bufferCount, (csa == 3) ? 0x80000003 : csa);

	wchar_t temp[64] = {0};
	StringCchPrintf(temp, 64, L"%s: %d%%",displayString, bufStatus);
	SetStatus(temp);
	//SetVideoStatusText(temp); // TODO: find a way to set the old status back
	GetVideoOutput()->notifyBufferState(static_cast<int>(bufStatus*2.55f));
}
