#include "main.h"
#include <strsafe.h>

#define SYNCDIALOG_PROP			L"SyncDialogProp"

typedef struct SyncDialog
{
	HWND centerWindow;
	DeviceView *device;
	C_ItemList *libraryList;
	C_ItemList *deviceList;
	BOOL autofillMode;
} SyncDialog;

#define SYNCDIALOG(_hwnd) ((SyncDialog*)GetPropW((_hwnd), SYNCDIALOG_PROP))

static INT_PTR CALLBACK 
SyncDialog_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


INT_PTR
SyncDialog_Show(HWND centerWindow, DeviceView *device, 
				C_ItemList *libraryList, C_ItemList *deviceList, BOOL autofillMode)
{
	SyncDialog param;
	
	param.centerWindow = centerWindow;
	param.device = device;
	param.libraryList = libraryList;
	param.deviceList = deviceList;
	param.autofillMode = autofillMode;

	return WASABI_API_DIALOGBOXPARAMW(IDD_SYNC, plugin.hwndLibraryParent, SyncDialog_DialogProc, (LPARAM)&param);
}

static BOOL 
SyncDialog_FormatSongString(wchar_t *buffer, size_t bufferSize, 
							const wchar_t *artist, const wchar_t *title, const wchar_t *fileName)
{
	HRESULT hr;

	if (NULL == buffer)
		return FALSE;

	if (NULL == title || L'\0' == *title)
	{
		if (NULL == fileName || L'\0' == *fileName)
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN_TRACK, buffer, bufferSize);
			hr = S_OK;
		}
		else
			hr = StringCchCopyEx(buffer, bufferSize, fileName, NULL, NULL, STRSAFE_IGNORE_NULLS);
	}
	else if (NULL == artist || L'\0' == *artist)
	{
		hr = StringCchCopyEx(buffer, bufferSize, title, NULL, NULL, STRSAFE_IGNORE_NULLS);
	}
	else
	{
		hr = StringCchPrintfEx(buffer, bufferSize, NULL, NULL, STRSAFE_IGNORE_NULLS,
								L"%s - %s", artist, title);
	}
	
	return SUCCEEDED(hr);
}

static BOOL
SyncDialog_PopulateTrackLists(HWND hwnd)
{
	SyncDialog *self;
	HWND hList;
	wchar_t buffer[1024] = {0};
	int index, count;

	self = SYNCDIALOG(hwnd);
	if (NULL == self)
		return FALSE;

	hList = GetDlgItem(hwnd, IDC_LIST_ADD);
	if(NULL != hList)
	{
		SendMessage(hList, WM_SETREDRAW, FALSE, 0L);
		SendMessage(hList, LB_RESETCONTENT, FALSE, 0L);

		count = (NULL != self->libraryList) ? self->libraryList->GetSize() : 0;
		if (0 != count)
		{
			SendMessage(hList, LB_SETCOUNT , (WPARAM)count, 0L);

			for(index = 0; index < count; index++)
			{
				itemRecordW *record = (itemRecordW*)self->libraryList->Get(index);
				if (NULL != record && 
					FALSE != SyncDialog_FormatSongString(buffer, ARRAYSIZE(buffer), 
										record->artist, record->title, record->filename))
				{
					SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
				}
			}
		}

		SendMessage(hList, WM_SETREDRAW, TRUE, 0L);
	}

	hList = GetDlgItem(hwnd, IDC_LIST_REMOVE);
	if(NULL != hList)
	{
		SendMessage(hList, WM_SETREDRAW, FALSE, 0L);
		SendMessage(hList, LB_RESETCONTENT, FALSE, 0L);

		count = (NULL != self->deviceList) ? self->deviceList->GetSize() : 0;
		if (0 != count)
		{
			Device *device;
			wchar_t artist[256] = {0}, title[256] = {0};

			SendMessage(hList, LB_SETCOUNT , (WPARAM)count, 0L);

			device = self->device->dev;

			for(index = 0; index < count; index++)
			{
				songid_t songId = (songid_t)self->deviceList->Get(index);

				device->getTrackArtist(songId, artist,ARRAYSIZE(artist));
				device->getTrackTitle(songId, title, ARRAYSIZE(title));

				if (FALSE != SyncDialog_FormatSongString(buffer, ARRAYSIZE(buffer), 
										artist, title, NULL))
				{
					SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
				}
			}
		}

		SendMessage(hList, WM_SETREDRAW, TRUE, 0L);
	}

	return TRUE;
}

static BOOL
SyncDialog_UpdateCaption(HWND hwnd)
{
	SyncDialog *self;
	HWND captionWindow;
	int resourceId;
	wchar_t buffer[1024] = {0}, format[1024] = {0};

	self = SYNCDIALOG(hwnd);
	if (NULL == self)
		return FALSE;

	captionWindow = GetDlgItem(hwnd, IDC_CAPTION);
	if (NULL == captionWindow)
		return FALSE;

	resourceId = (FALSE == self->autofillMode) ? 
						IDS_X_SONGS_WILL_BE_TRANSFERRED_TO_THE_DEVICE : 
						IDS_THIS_WILL_ADD_X_SONGS_AND_DELETE_X_SONGS;

	
	WASABI_API_LNGSTRINGW_BUF(resourceId, format, ARRAYSIZE(format));
			

	if (FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), format, 
							self->libraryList->GetSize(), self->deviceList->GetSize())))
	{
		return FALSE;
	}
	
	SendMessage(captionWindow, WM_SETTEXT, 0, (LPARAM)buffer);
	return TRUE;
}

typedef enum DeviceSyncPolicy
{
	DeviceSyncPolicy_LeaveTracks = 0,
	DeviceSyncPolicy_DeleteTracks = 1,
	DeviceSyncPolicy_CopyTracks = 2,
} DeviceSyncPolicy;

static BOOL
SyncDialog_GetDeviceSyncPolicyEnabled(SyncDialog *self)
{
	if (NULL != self &&
		FALSE == self->autofillMode && 
		0 != self->deviceList->GetSize())
	{
		return TRUE;
	}

	return FALSE;
}

static DeviceSyncPolicy
SyncDialog_ReadDeviceSyncPolicy(SyncDialog *self)
{
	if (FALSE != SyncDialog_GetDeviceSyncPolicyEnabled(self) &&
		NULL != self->device && 
		NULL != self->device->config)
	{
		C_Config *config;
		config = self->device->config;
		return (DeviceSyncPolicy)config->ReadInt(L"TrueSync", (int)DeviceSyncPolicy_LeaveTracks);
	}

	return DeviceSyncPolicy_LeaveTracks;
}

static BOOL
SyncDialog_WriteDeviceSyncPolicy(SyncDialog *self, DeviceSyncPolicy policy)
{
	if (FALSE != SyncDialog_GetDeviceSyncPolicyEnabled(self) &&
		NULL != self->device && 
		NULL != self->device->config)
	{
		C_Config *config;
		config = self->device->config;
		config->WriteInt(L"TrueSync", (int)policy);
		return TRUE;
	}

	return FALSE;
}

static BOOL
SyncDialog_SelectDeviceSyncPolicy(HWND hwnd, DeviceSyncPolicy policy)
{
	SyncDialog *self;
	HWND hControl;
	size_t index;
	int policyControl;
	int checkMode;
	int labelText;
	BOOL enableControls;

	const int  controls[] = 
	{
		IDC_TRUESYNC_LEAVE, 
		IDC_TRUESYNC_DELETE, 
		IDC_TRUESYNC_COPY 
	};


	self = SYNCDIALOG(hwnd);
	if (NULL == self)
		return FALSE;
	
	switch(policy)
	{
		case DeviceSyncPolicy_LeaveTracks:	
			policyControl = IDC_TRUESYNC_LEAVE; 
			labelText = IDS_SONGS_NOT_IN_MEDIA_LIBRARY;
			break;
		case DeviceSyncPolicy_DeleteTracks:	
			policyControl = IDC_TRUESYNC_DELETE; 
			labelText = IDS_SONGS_TO_BE_DELETED;
			break;
		case DeviceSyncPolicy_CopyTracks:	
			policyControl = IDC_TRUESYNC_COPY; 
			labelText = IDS_SONGS_TO_BE_COPIED;
			break;
		default: 
			return FALSE;
	}

	enableControls = SyncDialog_GetDeviceSyncPolicyEnabled(self);

	for (index = 0; index < ARRAYSIZE(controls); index++)
	{
		hControl = GetDlgItem(hwnd, controls[index]);
		if (NULL == hControl)
			continue;

		checkMode = (controls[index] == policyControl) ? 
					BST_CHECKED : 
					BST_UNCHECKED;

		SendMessage(hControl, BM_SETCHECK, (WPARAM)checkMode, 0L);
		EnableWindow(hControl, enableControls);
	}

	hControl = GetDlgItem(hwnd, IDC_REMOVELABEL);
	if (NULL != hControl)
	{	
		wchar_t buffer[256] = {0};
		WASABI_API_LNGSTRINGW_BUF(labelText, buffer, ARRAYSIZE(buffer));
		SendMessage(hControl, WM_SETTEXT, 0, (LPARAM)buffer);
	}

	return TRUE;
}

static INT_PTR
SyncDialog_OnInitDialog(HWND hwnd, HWND focusWindow, LPARAM param)
{
	SyncDialog *self;
	C_Config *config;
	
	self = (SyncDialog*)param;

	if (NULL == self ||
		FALSE == SetProp(hwnd, SYNCDIALOG_PROP, self))
	{
		EndDialog(hwnd, -1);
		return 0;
	}

	config = self->device->config;
	
	SyncDialog_UpdateCaption(hwnd);
	SyncDialog_SelectDeviceSyncPolicy(hwnd, SyncDialog_ReadDeviceSyncPolicy(self));
	
	
	if(config->ReadInt(L"syncOnConnect", self->device->SyncConnectionDefault) == (self->autofillMode ? 2:1)) 
		CheckDlgButton(hwnd, IDC_SYNCONCONNECT, BST_CHECKED);
	
	SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_LESS,0), NULL);
		
	if (FALSE != CenterWindow(hwnd, self->centerWindow))
		SendMessage(hwnd, DM_REPOSITION, 0, 0L);
	
	SyncDialog_PopulateTrackLists(hwnd);

	return 0;
}

static void
SyncDialog_OnDestroy(HWND hwnd)
{
	RemoveProp(hwnd, SYNCDIALOG_PROP);
}

static void
SyncDialog_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND controlWindow)
{
	SyncDialog *self;
	C_Config *config;
	BOOL enableRadios;

	self = SYNCDIALOG(hwnd);
	if (NULL == self)
		return;

	config = self->device->config;

	enableRadios = ((!self->autofillMode) && self->deviceList->GetSize());

	switch(commandId)
	{
		case IDC_ADD_REMSEL:
		case IDC_REM_REMSEL:
		case IDC_ADD_CROPSEL:
		case IDC_REM_CROPSEL:
		{
			bool rem = (commandId == IDC_ADD_REMSEL || commandId == IDC_REM_REMSEL);
			bool addlist = (commandId == IDC_ADD_REMSEL || commandId == IDC_ADD_CROPSEL);
			HWND box = GetDlgItem(hwnd,addlist?IDC_LIST_ADD:IDC_LIST_REMOVE);
			C_ItemList * list = addlist?self->libraryList:self->deviceList;
			int l=SendMessage(box,LB_GETCOUNT,0,0);
			int current=0;
			for(int i=0; i<l; ++i)
			{
				if((SendMessage(box,LB_GETSEL,current,0)!=0) == rem)
				{
					SendMessage(box,LB_DELETESTRING,current,0);
					list->Del(current);
				}
				else current++;
			}
			SyncDialog_UpdateCaption(hwnd);
		}
		break;

		case IDC_TRUESYNC_DELETE:
			if (BN_CLICKED == eventId)
				SyncDialog_SelectDeviceSyncPolicy(hwnd, DeviceSyncPolicy_DeleteTracks);
			break;
		case IDC_TRUESYNC_COPY:
			if (BN_CLICKED == eventId)
				SyncDialog_SelectDeviceSyncPolicy(hwnd, DeviceSyncPolicy_CopyTracks);
			break;
		case IDC_TRUESYNC_LEAVE:
			if (BN_CLICKED == eventId)
				SyncDialog_SelectDeviceSyncPolicy(hwnd, DeviceSyncPolicy_LeaveTracks);
			break;

		case IDC_MORE:
		case IDC_LESS:
		{
			bool more = commandId == IDC_MORE;
			int show = more?SW_SHOW:SW_HIDE;
			int hide = more?SW_HIDE:SW_SHOW;
			ShowWindow(GetDlgItem(hwnd,IDCANCEL),hide);
			ShowWindow(GetDlgItem(hwnd,IDOK),hide);
			ShowWindow(GetDlgItem(hwnd,IDC_MORE),hide);
			ShowWindow(GetDlgItem(hwnd,IDC_LESS),show);
			ShowWindow(GetDlgItem(hwnd,IDCANCEL2),show);
			ShowWindow(GetDlgItem(hwnd,IDOK2),show);
			ShowWindow(GetDlgItem(hwnd,IDC_ADDLABEL),show);
			ShowWindow(GetDlgItem(hwnd,IDC_REMOVELABEL),show);
			ShowWindow(GetDlgItem(hwnd,IDC_LIST_ADD),show);
			ShowWindow(GetDlgItem(hwnd,IDC_LIST_REMOVE),show);
			RECT r, r1, r2;
			GetWindowRect(hwnd,&r);
			GetWindowRect(GetDlgItem(hwnd,more?IDC_MORE:IDC_LESS),&r1);
			GetWindowRect(GetDlgItem(hwnd,more?IDCANCEL2:IDCANCEL),&r2);
			SetWindowPos(hwnd,HWND_TOP,r.left,r.top,
			             r2.right  - r.left + (r1.left - r.left),
			             r2.bottom - r.top  + (r1.left - r.left),   0);
		}
		break;
		case IDOK2:
		case IDOK:
			config->WriteInt(L"syncOnConnect",IsDlgButtonChecked(hwnd,IDC_SYNCONCONNECT)?(self->autofillMode?2:1):0);

			if(enableRadios)
				config->WriteInt(L"TrueSync",IsDlgButtonChecked(hwnd,IDC_TRUESYNC_LEAVE)?0:(IsDlgButtonChecked(hwnd,IDC_TRUESYNC_DELETE)?1:2));
			
			EndDialog(hwnd, IDOK);
			break;
		
		case IDCANCEL2:
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
	}
}

static INT_PTR CALLBACK 
SyncDialog_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return SyncDialog_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		SyncDialog_OnDestroy(hwnd); return TRUE;
		case WM_COMMAND:		SyncDialog_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
	}

	return FALSE;
}