#include "ConnectActivity.h"
#include "resource.h"
#include "api.h"
#include <strsafe.h>

BOOL ConnectActivity::GetActive()
{
	return TRUE;
}

BOOL ConnectActivity::GetCancelable()
{
	return FALSE;
}

HRESULT ConnectActivity::GetProgress(unsigned int *percentCompleted)
{
	return E_NOTIMPL;
}

HRESULT ConnectActivity::GetDisplayName(wchar_t *buffer, size_t bufferMax)
{
	if (NULL == buffer)
		return E_POINTER;

	WASABI_API_LNGSTRINGW_BUF(IDS_ACTIVITY_CONNECT, buffer, bufferMax);
	return S_OK;
}

HRESULT ConnectActivity::GetStatus(wchar_t *buffer, size_t bufferMax)
{
	if (NULL == buffer)
		return E_POINTER;

	WASABI_API_LNGSTRINGW_BUF(IDS_ACTIVITY_CONNECT_DESC, buffer, bufferMax);
	return S_OK;
}

HRESULT ConnectActivity::Cancel(HWND hostWindow)
{
	return E_NOTIMPL;
}

#define CBCLASS ConnectActivity
START_DISPATCH;
CB(API_GETACTIVE, GetActive);
CB(API_GETCANCELABLE, GetCancelable);
CB(API_GETPROGRESS, GetProgress);
CB(API_GETDISPLAYNAME, GetDisplayName);
CB(API_GETSTATUS, GetStatus);
CB(API_CANCEL, Cancel);
END_DISPATCH;
#undef CBCLASS