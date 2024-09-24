#include "main.h"
#include "../winamp/wa_ipc.h"

LRESULT SetFileInfo(const wchar_t *filename, const wchar_t *metadata, const wchar_t *data)
{
	extendedFileInfoStructW efis = {
	                                   filename,
	                                   metadata,
	                                   data ? data : L"",
	                                   data ? (size_t)lstrlenW(data) : 0,
	                               };
	return SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);
}

void WriteFileInfo(const wchar_t *file)
{
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)file, IPC_FILE_TAG_MAY_HAVE_UPDATEDW);
}

int GetFileInfo(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len)
{
	extendedFileInfoStructW efis = { filename, metadata, dest, (size_t)len, };
	return (int)(INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&efis, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE); //will return 1 if wa2 supports this IPC call
}
