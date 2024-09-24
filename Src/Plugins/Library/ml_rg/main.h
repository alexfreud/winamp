#ifndef NULLSOFT_ML_RG_MAIN_H
#define NULLSOFT_ML_RG_MAIN_H

#include <windows.h>
#include "../../General/gen_ml/ml.h"
#include <windowsx.h>
#include "../winamp/wa_ipc.h"
#include "../../General/gen_ml/ml.h"
#include "resource.h"
#include <string>
#include <vector>
#include <map>

extern winampMediaLibraryPlugin plugin;
extern char *iniFile;

LRESULT SetFileInfo(const wchar_t *filename, const wchar_t *metadata, const wchar_t *data);
int GetFileInfo(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len);
void WriteFileInfo();
void TagUpdated(const wchar_t *filename);

struct RGWorkFile
{
	RGWorkFile(const wchar_t *_filename=0) 
	{
		if (_filename)
			lstrcpynW(filename, _filename, MAX_PATH);
		else
			filename[0]=0;
		track_gain[0]=0;
		track_peak[0]=0;
		album_gain[0]=0;
		album_peak[0]=0;
	}

	wchar_t filename[MAX_PATH];
	wchar_t track_gain[64];
	wchar_t track_peak[64];
	wchar_t album_gain[64];
	wchar_t album_peak[64];
};

class ProgressCallback;

class WorkQueue
{
public:
	WorkQueue() : totalFiles(0){}
	void Add(const wchar_t *filename);
	void Calculate(ProgressCallback *callback, int *killSwitch);
	typedef std::vector<RGWorkFile> RGWorkQueue;
	typedef std::map<std::wstring, RGWorkQueue> AlbumMap;
	AlbumMap albums;
	RGWorkQueue unclassified;
	size_t totalFiles;
};

constexpr auto TIME_SPAN_MS = 10;

class ProgressCallback
{
public:
	ProgressCallback(HWND _c)
	: callback(_c),
	  totalReceived(0)
	{
		ticks = GetTickCount64();
	}

	void InformSize(size_t bytes)
	{
		if (!PostMessage(callback, WM_USER + 3, 0, bytes))
		{
			// LOG the error
			DWORD e = GetLastError();
		}
	}
	/// <summary>
	///  This function may fire an "ERROR_NOT_ENOUGH_QUOTA" 1816 (0x718) error when the limit is hit!
	///  Put some throttle here, post message every 10 ms, not each time we receive a progress.
	/// </summary>
	/// <param name="bytes"></param>
	void Progress(size_t bytes)
	{
		totalReceived += bytes;
		ULONGLONG currentTicks = GetTickCount64();
		if (currentTicks - ticks >= TIME_SPAN_MS)
		{
			ticks = currentTicks;
			if (!PostMessage(callback, WM_USER + 4, 0, totalReceived))
			{
				// LOG the error
				DWORD e = GetLastError();
			}

			totalReceived = 0;
		}
	}
	void FileFinished()
	{
		// notify remaining bytes
		if (totalReceived)
		{
			PostMessage(callback, WM_USER + 4, 0, totalReceived);
			totalReceived = 0;
		}

		if(!PostMessage(callback, WM_USER, 0, 0))
		{
			// LOG the error
			DWORD e = GetLastError();
		}
	}
	void AlbumFinished(WorkQueue::RGWorkQueue *album)
	{
		if(!PostMessage(callback, WM_USER + 1, 0, (LPARAM)album))
		{
			// LOG the error
			DWORD e = GetLastError();
		}
	}

	HWND callback;
	ULONGLONG ticks;
	size_t totalReceived;
};

void CopyAlbumData(WorkQueue::RGWorkQueue &workQueue, const wchar_t *album_gain, const wchar_t *album_peak);
void WriteAlbum(WorkQueue::RGWorkQueue &workQueue);
void WriteTracks(WorkQueue::RGWorkQueue &workQueue);
void DoResults(WorkQueue::RGWorkQueue &queue);
void DoResults(WorkQueue &queue);
void DoProgress(WorkQueue &workQueue);

void DestroyRG(void *context);
void *CreateRG();
void CalculateAlbumRG(void *context, wchar_t album_gain[64], wchar_t album_peak[64], float &albumPeak);
void StartRG(void *context);
void CalculateRG(void *context, const wchar_t *filename, wchar_t track_gain[64], wchar_t track_peak[64], ProgressCallback *callback, int *killSwitch, float &albumPeak);

HWND GetDialogBoxParent();
BOOL windowOffScreen(HWND hwnd, POINT pt);

extern int config_ask, config_ask_each_album, config_ignore_gained_album;
INT_PTR WINAPI RGConfig(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
#endif