#pragma once

#include "../Winamp/in2.h"
extern In_Module plugin;

#include <windows.h>

#include "../Winamp/wa_ipc.h"
extern IVideoOutput *videoOutput;
	
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
extern WNDPROC oldVidProc;
#include "SWFContainer.h"
extern SWFContainer *activeContainer;

extern int playPosition;
extern int playLength;
extern int volume, pan;
void SetVolume(int _volume);
void SetPan(int _pan);

#include "../nu/AutoLock.h"
extern Nullsoft::Utility::LockGuard statusGuard;
extern wchar_t status[256];
