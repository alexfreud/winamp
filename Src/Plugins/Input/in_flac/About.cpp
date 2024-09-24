/*
** Copyright © 2007-2014 Winamp SA
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
#include "../Agave/Language/api_language.h"
#include "resource.h"
#include <FLAC/all.h>
#include <strsafe.h>

static HMODULE libflac=0;
static char defaultVersionString[64];
static const char **versionString=0;
static const char *GetFLACVersion()
{
	return "1.4.2";
}

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMSW),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCEW(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirectW(&msgbx);
}

void About(HWND hwndParent)
{
	wchar_t message[1024] = {0};
	StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					 plugin.description, __DATE__, GetFLACVersion());
	DoAboutMessageBox(hwndParent,(wchar_t*)plugin.description,message);
}