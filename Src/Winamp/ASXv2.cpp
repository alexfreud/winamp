/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"

void loadasxv2fn(const wchar_t *filename, int whattodo)
{
	if (PlayList_getlength())
	{
		if (whattodo < 1)
			PlayList_delete();
	}

	int i=1;
	wchar_t ref[FILENAME_SIZE];
	wchar_t key[100];
	while (1)
	{
		StringCchPrintfW(key, 100, L"Ref%d", i++);
		GetPrivateProfileStringW(L"Reference", key, L"?", ref, FILENAME_SIZE, filename);
		if (!lstrcmpiW(ref, L"?"))
		break;
		else
		{
				if (!_wcsnicmp(ref, L"http://", 7))
				{
					wchar_t *end = scanstr_backW(ref, L"/.", 0);
					if (!end || *end == L'/')
					{
						if (wcschr(ref, L'?'))
							StringCchCatW(ref, FILENAME_SIZE, L"&=.wma");
						else
								StringCchCatW(ref, FILENAME_SIZE, L"?.wma");
					}
				}

			PlayList_append(ref);
		}

	}
}