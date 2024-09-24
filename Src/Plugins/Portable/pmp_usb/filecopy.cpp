#include "api.h"
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "resource.h"
#include <strsafe.h>

typedef struct CopyData 
{
	void * callbackContext;
	void (*callback)(void * callbackContext, wchar_t * status);
} CopyData;

DWORD CALLBACK CopyToIpodProgressRoutine(LARGE_INTEGER TotalFileSize,	LARGE_INTEGER TotalBytesTransferred,
	LARGE_INTEGER StreamSize,	LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber,
	DWORD dwCallbackReason,
	HANDLE hSourceFile,	HANDLE hDestinationFile,
	LPVOID lpData)
{
	CopyData *inst = (CopyData *)lpData;
	if (inst && inst->callback) 
	{
		wchar_t status[100] = {0};
		wchar_t langtemp[100] = {0};
		StringCbPrintf(status, sizeof(status), WASABI_API_LNGSTRINGW_BUF(IDS_TRANSFERING_PERCENT, langtemp, 100), (int)(100ULL * TotalBytesTransferred.QuadPart / (TotalFileSize.QuadPart)));
		inst->callback(inst->callbackContext,status);
	}
	return PROGRESS_CONTINUE; 
}

int CopyFile(const wchar_t *infile, const wchar_t *outfile, void * callbackContext, void (*callback)(void * callbackContext, wchar_t * status), int * killswitch) 
{
	wchar_t langtemp[100] = {0};

	CopyData c;
	c.callback = callback;
	c.callbackContext = callbackContext;

	if (CopyFileEx(infile, outfile, CopyToIpodProgressRoutine, &c, killswitch,0))
	{
		if (callback)
		{
			callback(callbackContext, WASABI_API_LNGSTRINGW_BUF(IDS_DONE, langtemp, 100));
		}
		return 0;
	}
	else
	{
		switch(GetLastError())
		{
		case ERROR_REQUEST_ABORTED:
			DeleteFile(outfile);
			if (callback)
			{
				callback(callbackContext, WASABI_API_LNGSTRINGW_BUF(IDS_CANCELLED, langtemp, 100));
			}

		default:
			if (callback)
			{
				callback(callbackContext, WASABI_API_LNGSTRINGW_BUF(IDS_TRANSFER_FAILED, langtemp, 100));
			}
		}
		return -1;
	}
}