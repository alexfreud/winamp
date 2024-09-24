#ifndef NULLSOFT_WINAMPINTERFACEH
#define NULLSOFT_WINAMPINTERFACEH

#include "Main.h"
#include <windows.h>
#include "../Winamp/wa_ipc.h"
#include "../Winamp/In2.h"
#include "../Winamp/strutil.h"
#include "output/AudioOut.h"
#include "../nu/AutoLock.h"

extern AudioOut *out;
extern In_Module plugin;

class WinampInterface
{
public:
	WinampInterface();

	HWND GetVideoWindow();

	IVideoOutput *GetVideoOutput();
	void EndOfFile();
	HWND GetWinampWindow();

	void RefreshTitle()
	{
		PostMessage(plugin.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
	}

	const char *GetProxy()
	{
		return (const char *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GET_PROXY_STRING);
	}

	void ResetBuffering() { bufferCount=0;}
	void Buffering(int bufStatus, const wchar_t *displayString);

	bool OpenEncryptedVideo(int width, int height, bool flip, double aspect, int fourcc);
	bool OpenVideo(int width, int height, bool flip, double aspect, int fourcc)
	{
		GetVideoOutput()->extended(VIDUSER_SET_THREAD_SAFE, 1, 0);
		bool video = (GetVideoOutput()->open(width, height, flip ? 1 : 0, aspect, fourcc) == 0);
		return video;
	}

	ULONG_PTR GetStart()
	{
		return SendMessage(plugin.hMainWindow, WM_WA_IPC,0,IPC_GETPLAYITEM_START);
	}

	ULONG_PTR GetEnd()
	{
		return SendMessage(plugin.hMainWindow, WM_WA_IPC,0,IPC_GETPLAYITEM_END);
	}

	void PressStop()
	{
		SendMessage(plugin.hMainWindow, WM_COMMAND, 40047, 0);
	}

	void PressPlay()
	{
		SendMessage(plugin.hMainWindow, WM_COMMAND,40045, 0);
	}

	void DrawFrame(void *frame)
	{
		GetVideoOutput()->draw(frame);
	}

	void EncryptedDrawFrame(void *frame);
	void SetVideoStatusText(char *text)
	{
		GetVideoOutput()->extended(VIDUSER_SET_INFOSTRING,(INT_PTR)text,0);
	}
	void SetVideoPalette(RGBQUAD *palette)
	{
		GetVideoOutput()->extended(VIDUSER_SET_PALETTE,(INT_PTR)palette,0);
	}
	void CloseViz()
	{
		plugin.SAVSADeInit();
	}
	void CloseEncryptedVideo();
	void CloseVideo()
	{
		GetVideoOutput()->close();
	}

	void SetAudioInfo(int bitRateKiloBits, int sampleRateKiloHertz, int channels)
	{
		plugin.SetInfo(bitRateKiloBits, sampleRateKiloHertz, channels, 1);
	}

	void OpenViz(int maxLatency, int sampleRate)
	{
		plugin.SAVSAInit(maxLatency, sampleRate);
	}

	void SetVizInfo(int sampleRate, int channels)
	{
		plugin.VSASetInfo(sampleRate, channels);
	}

	bool GetStatusHook(wchar_t *title, size_t titleLen, const wchar_t *filename);
	bool HasStatus(const wchar_t *filename);
	void SetStatus(wchar_t *_status);
	bool GetStatus(wchar_t *title, size_t titleLen, const wchar_t *filename);

	void ClearStatus();
	Nullsoft::Utility::LockGuard statusGuard;
	int bufferCount;

private:
	wchar_t status[1024];
	wchar_t statusFilename[FILENAME_SIZE];
	IVideoOutput *videoWindow;
};

#endif
