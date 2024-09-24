#include "main.h"

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

void WriteFileInfo()
{
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
}

int GetFileInfo(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len)
{
	extendedFileInfoStructW efis = { filename, metadata, dest, (size_t)len, };
	return (int)(INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&efis, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE); //will return 1 if wa2 supports this IPC call
}

void TagUpdated(const wchar_t *filename)
{
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)filename, IPC_FILE_TAG_MAY_HAVE_UPDATEDW);
}