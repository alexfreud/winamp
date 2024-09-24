#include "main.h"
#include "resource.h"
#include <stdio.h>
#include <strsafe.h>

//static const GUID INVALID_GUID = 
//{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
void WaitForEvent(HANDLE hEvent, DWORD msMaxWaitTime)
{
	//  DWORD   i;
	MSG msg;
	const unsigned long eachWait = 10;
	unsigned long totalWait = 0;

	while (WaitForSingleObject(hEvent, eachWait) == WAIT_TIMEOUT)
	{
		while (PeekMessage(&msg, (HWND) NULL, 0, 0, PM_REMOVE))
		{
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		totalWait += eachWait;
		if (totalWait >= msMaxWaitTime)
			break;

	}
}

char *HRErrorCode(HRESULT hr)
{
#define HR_ERROR_CODE(x) case x: return #x
	switch (hr)
	{
		HR_ERROR_CODE(E_OUTOFMEMORY);
		HR_ERROR_CODE(E_UNEXPECTED);
		HR_ERROR_CODE(S_OK);
		HR_ERROR_CODE(S_FALSE);
		HR_ERROR_CODE(E_NOINTERFACE);
		HR_ERROR_CODE(NS_E_PROTECTED_CONTENT);
		HR_ERROR_CODE(E_INVALIDARG);
		HR_ERROR_CODE(NS_E_DRM_NO_RIGHTS);
		HR_ERROR_CODE(NS_E_DRM_LICENSE_NOTACQUIRED);
		HR_ERROR_CODE(NS_E_DRM_ACQUIRING_LICENSE);
		HR_ERROR_CODE(NS_S_DRM_ACQUIRE_CANCELLED);
		HR_ERROR_CODE(NS_E_LICENSE_OUTOFDATE);
		HR_ERROR_CODE(NS_E_LICENSE_INCORRECT_RIGHTS);
		HR_ERROR_CODE(NS_E_DRM_REOPEN_CONTENT);
		HR_ERROR_CODE(NS_E_DRM_LICENSE_APPSECLOW);
		HR_ERROR_CODE(E_ABORT);
		HR_ERROR_CODE(NS_E_INVALID_REQUEST);
		HR_ERROR_CODE(NS_S_DRM_LICENSE_ACQUIRED);
		HR_ERROR_CODE(NS_S_DRM_MONITOR_CANCELLED);
		HR_ERROR_CODE(NS_E_FILE_NOT_FOUND);
		HR_ERROR_CODE(NS_E_FILE_OPEN_FAILED);
		HR_ERROR_CODE(NS_E_SERVER_NOT_FOUND);
		HR_ERROR_CODE(NS_E_UNRECOGNIZED_STREAM_TYPE);
		HR_ERROR_CODE(NS_E_NO_STREAM);
		HR_ERROR_CODE(E_ACCESSDENIED);
		HR_ERROR_CODE(NS_S_DRM_BURNABLE_TRACK);
		HR_ERROR_CODE(NS_S_DRM_BURNABLE_TRACK_WITH_PLAYLIST_RESTRICTION);
		HR_ERROR_CODE(NS_E_DRM_TRACK_EXCEEDED_PLAYLIST_RESTICTION);
		HR_ERROR_CODE(NS_E_DRM_TRACK_EXCEEDED_TRACKBURN_RESTRICTION);
	}
	static char temp[50];
	StringCchPrintfA(temp, 50, WASABI_API_LNGSTRING(IDS_UNKNOWN_ERROR), hr);
	return temp;
	
}

void GuidString(GUID guid, wchar_t *target, size_t len)
{
	 StringCchPrintf( target, len, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
    (int)guid.Data1, (int)guid.Data2, (int)guid.Data3,
    (int)guid.Data4[0], (int)guid.Data4[1],
    (int)guid.Data4[2], (int)guid.Data4[3],
    (int)guid.Data4[4], (int)guid.Data4[5],
    (int)guid.Data4[6], (int)guid.Data4[7] );
}

 const wchar_t *UserTextDescription(unsigned char *binary, size_t size)
{
	WM_USER_TEXT *userText  = (WM_USER_TEXT *)binary;
	
	return userText->pwszDescription;
}

const wchar_t *UserTextString(unsigned char *binary, size_t size)
{
	WM_USER_TEXT *userText  = (WM_USER_TEXT *)binary;
	
	return userText->pwszText;
}

void BinaryString(unsigned char *binary, size_t size, wchar_t *final, size_t len)
{
	wchar_t * const start = new wchar_t[2 + size*2 + 1]; // 0x + 2 hex per byte + null terminator
	wchar_t *target = start;
	*target++='0';
	*target++='x';

	size_t i;
	for (i = 0;i!=size;i++)
	{
		wchar_t temp[3] = {0};
		_itow(binary[i], temp, 16);
		if (!temp[0])
		{
			*target++ = '0';
			*target++ = '0';
		}
		else if (!temp[1])
		{
			*target++ = '0';
			*target++ = temp[0];
		}
		else
		{
			*target++ = temp[0];
			*target++ = temp[1];
		}
	}
	*target = 0;
	lstrcpyn(final, start, len);
	delete [] start;
}

GUID StringGUID(const wchar_t *source) 
{
  if (source == NULL) return INVALID_GUID;

  GUID guid = GUID_NULL;
  int Data1, Data2, Data3;
  int Data4[8] = {0};

  // {1B3CA60C-DA98-4826-B4A9-D79748A5FD73}
  int n = swscanf(source, L" { %08x - %04x - %04x - %02x%02x - %02x%02x%02x%02x%02x%02x } ",
    &Data1, &Data2, &Data3, Data4 + 0, Data4 + 1,
    Data4 + 2, Data4 + 3, Data4 + 4, Data4 + 5, Data4 + 6, Data4 + 7 );

  if (n != 11) return INVALID_GUID;

  // Cross assign all the values
  guid.Data1 = Data1;
  guid.Data2 = Data2;
  guid.Data3 = Data3;
  guid.Data4[0] = Data4[0];
  guid.Data4[1] = Data4[1];
  guid.Data4[2] = Data4[2];
  guid.Data4[3] = Data4[3];
  guid.Data4[4] = Data4[4];
  guid.Data4[5] = Data4[5];
  guid.Data4[6] = Data4[6];
  guid.Data4[7] = Data4[7];

  return guid;
}

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMS msgbx = {sizeof(MSGBOXPARAMS),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCE(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirect(&msgbx);
}