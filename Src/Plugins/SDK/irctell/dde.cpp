#include <windows.h>
#include "dde.h"
#include <strsafe.h>

HDDEDATA CALLBACK DdeGenericCallback(UINT uType, UINT uFmt,HCONV hconv, HSZ hsz1,
                                     HSZ hsz2, HDDEDATA hdata, DWORD dwData1,
                                     DWORD dwData2)
{
	return ((HDDEDATA)0);
}

void DdeCom::sendCommand(wchar_t *application, wchar_t *command, DWORD minInterval)
{
	static DWORD lastCmd=0, now;
	HSZ     string1,string2,string3;
	wchar_t line[512];
	HCONV ddeConv;
	DWORD result;

	now = GetTickCount();
	if (now < lastCmd + minInterval)
		return;

	StringCchCopy(line, 512, command);

	DWORD DDE=0;

	if (DdeInitialize(&DDE, DdeGenericCallback, CBF_SKIP_ALLNOTIFICATIONS+ST_CLIENT,0) != DMLERR_NO_ERROR)
	{
		MessageBox(NULL,L"DDE INITIALIZE", L"Error", MB_OK);

		return;
	}

	string1 = DdeCreateStringHandle(DDE, application, CP_WINANSI);
	string2 = DdeCreateStringHandle(DDE, L"COMMAND", CP_WINANSI);

	if ((ddeConv = DdeConnect(DDE, string1, string2, 0)) != 0)
	{
		string3 = DdeCreateStringHandle(DDE, L"None", CP_WINANSI);
		DdeClientTransaction((LPBYTE)line, (wcslen(line)+1)*sizeof(line[0]), ddeConv, string3, CF_UNICODETEXT, XTYP_POKE, 1000, &result);
		DdeFreeStringHandle(DDE, string3);
		DdeDisconnect(ddeConv);
		lastCmd = now;
	}

	DdeFreeStringHandle(DDE, string1);
	DdeFreeStringHandle(DDE, string2);
	DdeUninitialize(DDE);
}
