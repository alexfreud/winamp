#include "main.h"
#include "../winamp/wa_ipc.h"

int GetFileInfo(const wchar_t *filename, wchar_t *metadata, wchar_t *dest, int len)
{
	dest[0]=0;
	extendedFileInfoStructW efis=
	{
		filename,
		metadata,
		dest,
		(size_t)len,
	};
	int r = SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efis,IPC_GET_EXTENDED_FILE_INFOW); //will return 1 if wa2 supports this IPC call
	return r;
}

int updateFileInfo(const wchar_t *filename, wchar_t *metadata, wchar_t *data)
{
	extendedFileInfoStructW efis =
	{
		filename,
		metadata,
		data ? data : L"",
		data ? wcslen(data) : 0,
	};
	return SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);
}


void WriteFileInfo(const wchar_t *filename)
{

	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)filename, IPC_FILE_TAG_MAY_HAVE_UPDATEDW);
}