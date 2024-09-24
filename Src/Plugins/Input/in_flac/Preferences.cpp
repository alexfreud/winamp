/*
** Copyright (C) 2007-2011 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/
#include "main.h"
#include <FLAC/all.h>
#include "resource.h"
#include "../Agave/Language/api_language.h"
#include "../nu/AutoChar.h"
#include <assert.h>
#include <strsafe.h>

bool fixBitrate=false;
bool config_average_bitrate=true;

// the return pointer has been malloc'd.  Use free() when you are done.
char *BuildExtensions(const char *extensions)
{
	char name[64] = {0};
	WASABI_API_LNGSTRING_BUF(IDS_FLAC_FILES,name,64);
	size_t length = strlen(extensions) + 1 + strlen(name) + 2;
	char *newExt = (char *)malloc(length);
	char *ret = newExt; // save because we modify newExt

	// copy extensions
	StringCchCopyExA(newExt, length, extensions, &newExt, &length, 0);
	newExt++;
	length--;

	// copy description
	StringCchCopyExA(newExt, length, name, &newExt, &length, 0);
	newExt++;
	length--;

	// double null terminate
	assert(length == 1);
	*newExt = 0;

	return ret;
}

static INT_PTR CALLBACK PreferencesProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			wchar_t config_extensions[128] = {0};
			GetPrivateProfileStringW(L"in_flac", L"extensions", DEFAULT_EXTENSIONSW, config_extensions, 128, winampINI);
			SetDlgItemTextW(hwndDlg, IDC_EXTENSIONS, config_extensions);
			CheckDlgButton(hwndDlg, IDC_AVERAGE_BITRATE, config_average_bitrate?BST_CHECKED:BST_UNCHECKED);
		}
		return TRUE;

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				wchar_t config_extensions[128] = {0};
				GetDlgItemTextW(hwndDlg, IDC_EXTENSIONS, config_extensions, 128);
				if (!lstrcmpiW(config_extensions, DEFAULT_EXTENSIONSW))
					WritePrivateProfileStringW(L"in_flac", L"extensions", 0, winampINI);
				else
					WritePrivateProfileStringW(L"in_flac", L"extensions", config_extensions, winampINI);

				plugin.FileExtensions = BuildExtensions(AutoChar(config_extensions));
				config_average_bitrate = !!IsDlgButtonChecked(hwndDlg, IDC_AVERAGE_BITRATE);
				if (config_average_bitrate)
					WritePrivateProfileStringW(L"in_flac", L"average_bitrate", L"1", winampINI);
				else
					WritePrivateProfileStringW(L"in_flac", L"average_bitrate", L"0", winampINI);

				fixBitrate=true;
				EndDialog(hwndDlg, 0);
			}
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
		}
		break;
	}
	return 0;
}

void Config(HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_PREFERENCES, hwndParent, PreferencesProc);
}