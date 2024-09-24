#pragma once
#include "../devices/ifc_deviceactivity.h"

class ConnectActivity : public ifc_deviceactivity
{
public:
	BOOL GetActive();
	BOOL GetCancelable();
	HRESULT GetProgress(unsigned int *percentCompleted);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferMax);
	HRESULT GetStatus(wchar_t *buffer, size_t bufferMax);
	HRESULT Cancel(HWND hostWindow);

protected:
	RECVS_DISPATCH;
};