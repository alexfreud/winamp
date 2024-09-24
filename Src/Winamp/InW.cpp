/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description: Unicode<->ANSI conversion layer for input plugins
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/

#include "main.h"
#include "../nu/AutoCharFn.h"
#include "../nu/ns_wc.h"


int InW_IsOurFile(In_Module *mod, const wchar_t *filename)
{
	if(mod)
	{
		if (mod->version & IN_UNICODE)
			return mod->IsOurFile((in_char *)filename);
		else
			return mod->IsOurFile((in_char *)(char *)AutoCharFn(filename));
	}
	return 0;
}

int InW_Play(In_Module *mod, const wchar_t *filename)
{
	if (mod->version & IN_UNICODE)
		return mod->Play((in_char *)filename);
	else
		return mod->Play((in_char *)(char *)AutoCharFn(filename));
}

int InW_InfoBox(In_Module *mod, const wchar_t *filename, HWND parent)
{
	if (mod->version & IN_UNICODE)
		return mod->InfoBox((in_char *)filename, parent);
	else
		return mod->InfoBox((in_char *)(char *)AutoCharFn(filename), parent);
}

void InW_GetFileInfo(In_Module *mod, const wchar_t *filename, wchar_t *title, int *length)
{
	if (mod->version & IN_UNICODE)
		mod->GetFileInfo((in_char *)filename, (in_char *)title, length);
	else
	{
		char tempTitle[GETFILEINFO_TITLE_LENGTH]="";
		mod->GetFileInfo((in_char *)(char *)AutoCharFn(filename),  tempTitle, length);
		MultiByteToWideCharSZ(CP_ACP, 0, tempTitle, -1, title, GETFILEINFO_TITLE_LENGTH);
	}
}