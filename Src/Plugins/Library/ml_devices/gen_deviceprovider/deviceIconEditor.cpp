#include "main.h"
#include "./deviceIconEditor.h"


#define DEVICEICONEDITOR_PROP		L"NullsoftDevicesIconEditorProp"


static INT_PTR
DeviceIconEditor_DialogProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR 
DeviceIconEditor_Show(HWND parentWindow, DeviceIconInfo *iconInfo)
{
	if (NULL == iconInfo)
		return -1;

	return WASABI_API_DIALOGBOXPARAMW((INT_PTR)IDD_ICON_EDITOR, parentWindow, 
					DeviceIconEditor_DialogProc, (LPARAM)iconInfo);

}

static void
DeviceIconEditor_UpdateInfo(HWND hwnd)
{
	DeviceIconInfo *iconInfo;
	HWND controlWindow;
	wchar_t *string;

	iconInfo = (DeviceIconInfo*)GetProp(hwnd, DEVICEICONEDITOR_PROP);
	if (NULL == iconInfo)
		return;
	
	controlWindow = GetDlgItem(hwnd, IDC_EDIT_PATH);
	if (NULL != controlWindow)
	{
		String_Free(iconInfo->path);
		iconInfo->path = String_FromWindow(controlWindow);
	}

	controlWindow = GetDlgItem(hwnd, IDC_EDIT_WIDTH);
	if (NULL != controlWindow)
	{		
		string = String_FromWindow(controlWindow);
		if (NULL == string || 
			FALSE == StrToIntEx(string, STIF_DEFAULT, &iconInfo->width))
		{
			iconInfo->width = 0;
		}
		String_Free(string);
	}

	controlWindow = GetDlgItem(hwnd, IDC_EDIT_HEIGHT);
	if (NULL != controlWindow)
	{
		string = String_FromWindow(controlWindow);
		if (NULL == string || 
			FALSE == StrToIntEx(string, STIF_DEFAULT, &iconInfo->height))
		{
			iconInfo->height = 0;
		}
		String_Free(string);
	}
}

static INT_PTR 
DeviceIconEditor_OnInitDialog(HWND hwnd, HWND focusWindow, LPARAM param)
{
	DeviceIconInfo *iconInfo;
	HWND controlWindow;

	iconInfo = (DeviceIconInfo*)param;
	SetProp(hwnd, DEVICEICONEDITOR_PROP, iconInfo);
	
	if (NULL != iconInfo)
	{
		wchar_t buffer[64];

		controlWindow = GetDlgItem(hwnd, IDC_EDIT_PATH);
		if (NULL != controlWindow)
			SetWindowText(controlWindow, iconInfo->path);

		controlWindow = GetDlgItem(hwnd, IDC_EDIT_WIDTH);
		if (NULL != controlWindow)
		{
			_itow_s(iconInfo->width, buffer, 10);
			SetWindowText(controlWindow, buffer);
		}

		controlWindow = GetDlgItem(hwnd, IDC_EDIT_HEIGHT);
		if (NULL != controlWindow)
		{
			_itow_s(iconInfo->height, buffer, 10);
			SetWindowText(controlWindow, buffer);
		}

	}
	return 0;
}

static void
DeviceIconEditor_DisplayFileOpen(HWND hwnd)
{
	wchar_t buffer[MAX_PATH * 2];
	OPENFILENAME ofn;
	HWND controlWindow;

	buffer[0] = L'\0';
	
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"Portable Network Graphics\0"    L"*.png\0"
                      L"\0";
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = ARRAYSIZE(buffer);
    ofn.lpstrTitle = L"Load Icon";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	
	if (FALSE == GetOpenFileName(&ofn))
		return;

	controlWindow = GetDlgItem(hwnd, IDC_EDIT_PATH);
	if (NULL != controlWindow)
		SetWindowText(controlWindow, buffer);
}

static void
DeviceIconEditor_OnDestroy(HWND hwnd)
{
	RemoveProp(hwnd, DEVICEICONEDITOR_PROP);
}

static void
DeviceIconEditor_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND controlWindow)
{
	switch(commandId)
	{
		case IDOK:
			switch(eventId)
			{
				case BN_CLICKED:
					DeviceIconEditor_UpdateInfo(hwnd);
					EndDialog(hwnd, IDOK);
					break;
			}
			break;
		case IDCANCEL:
			switch(eventId)
			{
				case BN_CLICKED:
					EndDialog(hwnd, IDCANCEL);
					break;
			}
			break;
		case IDC_BUTTON_BROWSE:
			switch(eventId)
			{
				case BN_CLICKED:
					DeviceIconEditor_DisplayFileOpen(hwnd);
					break;
			}
			break;

	}
}

static INT_PTR
DeviceIconEditor_DialogProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return DeviceIconEditor_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		DeviceIconEditor_OnDestroy(hwnd); return TRUE;
		case WM_COMMAND:		DeviceIconEditor_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
	}
	return 0;
}