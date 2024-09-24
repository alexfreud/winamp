#include "main.h"
#include "loadini.h"
#include "AutoWide.h"
#include "../Winamp/wa_ipc.h"
extern wchar_t INI_FILE[MAX_PATH];
void IniFile(HWND hMainWindow)
{
	if (!INI_FILE[0])
	{
		lstrcpyn(INI_FILE, (wchar_t *)SendMessage(hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILEW), MAX_PATH);
	}
}