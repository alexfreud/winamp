#include "main.h"
#include "metadata_utils.h"
#include <strsafe.h>
#include "../playlist/ifc_playlistloadercallback.h"
#include "resource1.h"
#include <shlwapi.h>

#define SYNCCLOUDDIALOG_PROP		L"SyncCloudDialogProp"

typedef struct SyncCloudDialog
{
	HWND centerWindow;
	DeviceView *device;
	C_ItemList *libraryList;
	C_ItemList *playlistsList;
} SyncCloudDialog;

#define SYNCCLOUDDIALOG(_hwnd) ((SyncCloudDialog*)GetPropW((_hwnd), SYNCCLOUDDIALOG_PROP))

static INT_PTR CALLBACK SyncCloudDialog_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


INT_PTR SyncCloudDialog_Show(HWND centerWindow, DeviceView *device, C_ItemList *libraryList)
{
	SyncCloudDialog param;

	param.centerWindow = centerWindow;
	param.device = device;
	param.libraryList = libraryList;
	param.playlistsList = new C_ItemList;

	return WASABI_API_DIALOGBOXPARAMW(IDD_CLOUD_SYNC, plugin.hwndLibraryParent, SyncCloudDialog_DialogProc, (LPARAM)&param);
}

static BOOL SyncCloudDialog_FormatSongString(wchar_t *buffer, size_t bufferSize, const wchar_t *artist,
											 const wchar_t *title, const wchar_t *fileName)
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

WORD WGetListboxStringExtent(HWND hList, wchar_t* psz){
	WORD wExtent = 0;
	HDC hDC = GetDC(hList);
	int tab[] = {0};
	SelectObject(hDC,(HFONT)SendMessage(hList,WM_GETFONT,0,0));
	wExtent = LOWORD(GetTabbedTextExtent(hDC,psz,lstrlen(psz)+1,0,tab));
	ReleaseDC(hList,hDC);
	return wExtent;
}

DWORD fill_listbox(HWND list, wchar_t* string, DWORD width){
	DWORD prev = width;
	DWORD dwExtent = WGetListboxStringExtent(list,string);
	if(prev < dwExtent){prev = dwExtent;}
	return prev;
}

class SyncCloudItemListLoader : public ifc_playlistloadercallback
{
public:
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
	{
		if(pos < len)
		{
			if (info)
			{
				// look at the devices we've already uploaded to vs the destination
				const wchar_t * devices = info->GetExtendedInfo(L"cloud_devices");
				if (devices)
				{
					wchar_t* pt = wcstok((wchar_t*)devices, L"*");
					while (pt)
					{
						if (_wtoi(pt) == device_id)
						{
							done++;
							return;
						}
						pt = wcstok(NULL, L"*");
					}
				}
			}

			// check if the file exists, skipping if not
			if (PathFileExistsW(filename))
			{
				__int64 file_size = INVALID_FILE_SIZE;
				time_t file_time = 0;
				GetFileSizeAndTime(filename, &file_size, &file_time);
				if (!(file_size == INVALID_FILE_SIZE || file_size == 0))
				{
					total_size += file_size;
				}
				songs[pos].filename = _wcsdup(filename);
				pos++;
			}
			else
			{
				done++;
			}
		}
	}
	int64_t total_size;
	int pos, len, done, device_id;
	songMapping * songs;
protected:
	RECVS_DISPATCH;
};

#define CBCLASS SyncCloudItemListLoader
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;
#undef CBCLASS

static BOOL SyncCloudDialog_PopulatePlaylistLists(HWND hwnd)
{
	SyncCloudDialog *self = SYNCCLOUDDIALOG(hwnd);
	if (NULL == self)
		return FALSE;

	HWND hList = GetDlgItem(hwnd, IDC_LIST_ADD_PL);
	if(NULL != hList)
	{
		DWORD list_width = 0;
		SendMessage(hList, WM_SETREDRAW, FALSE, 0L);
		SendMessage(hList, LB_RESETCONTENT, FALSE, 0L);

		// first collect playlists without metadata
		int playlistsnum = SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_PLAYLIST_COUNT);
		for(int i = 0, count = 0; i < playlistsnum; i++)
		{
			SyncCloudItemListLoader* list = new SyncCloudItemListLoader();
			mlPlaylistInfo* playlist = (mlPlaylistInfo*)calloc(sizeof(mlPlaylistInfo),1);
			playlist->size = sizeof(mlPlaylistInfo);
			playlist->playlistNum = i;
			SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)playlist, ML_IPC_PLAYLIST_INFO);

			if (playlist->numItems > 0)
			{
				wchar_t buf[256] = {0};
				list->device_id = self->device->dev->extraActions(DEVICE_GET_CLOUD_DEVICE_ID, 0, 0, 0);
				list->total_size = list->pos = list->done = 0;
				list->len = playlist->numItems;
				list->songs = (songMapping*)calloc(sizeof(songMapping), list->len);
				playlistManager->Load(playlist->filename, list);
				list->len -= list->done;
				if (list->len > 0)
				{
					if (list->done > 0)
					{
						StringCchPrintfW(buf, 256, WASABI_API_LNGSTRINGW(IDS_CLOUD_SYNC_PL_SOME), playlist->playlistName, list->done);
					}
					int pos = SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)(buf[0] ? buf : playlist->playlistName));
					SendMessage(hList, LB_SETITEMDATA, pos, count);
					self->playlistsList->Add(list);
					count++;
				}
				else
				{
					StringCchPrintfW(buf, 256, WASABI_API_LNGSTRINGW(IDS_CLOUD_SYNC_PL_EMPTY), playlist->playlistName);
					int pos = SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)buf);
					SendMessage(hList, LB_SETITEMDATA, pos, -1);
				}
			}
		}

		SendMessage(hList, WM_SETREDRAW, TRUE, 0L);
		SendMessage(hList, LB_SETHORIZONTALEXTENT, list_width, 0L);
	}

	return TRUE;
}

static BOOL SyncCloudDialog_PopulateTrackLists(HWND hwnd)
{
	SyncCloudDialog *self;
	HWND hList;
	wchar_t buffer[1024] = {0};
	int index, count;

	self = SYNCCLOUDDIALOG(hwnd);
	if (NULL == self)
		return FALSE;

	hList = GetDlgItem(hwnd, IDC_LIST_ADD);
	if(NULL != hList)
	{
		DWORD list_width = 0;
		SendMessage(hList, WM_SETREDRAW, FALSE, 0L);
		SendMessage(hList, LB_RESETCONTENT, FALSE, 0L);

		count = (NULL != self->libraryList) ? self->libraryList->GetSize() : 0;
		if (0 != count)
		{
			itemRecordW *record;

			SendMessage(hList, LB_SETCOUNT , (WPARAM)count, 0L);

			for(index = 0; index < count; index++)
			{
				record = (itemRecordW*)self->libraryList->Get(index);
				if (NULL != record && 
					FALSE != SyncCloudDialog_FormatSongString(buffer, ARRAYSIZE(buffer), 
															  record->artist, record->title, record->filename))
				{
					SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
					list_width = fill_listbox(hList,buffer,list_width);
				}
			}
		}

		SendMessage(hList, WM_SETREDRAW, TRUE, 0L);
		SendMessage(hList, LB_SETHORIZONTALEXTENT, list_width, 0L);
	}

	return TRUE;
}

static int SyncCloudDialog_ReadTransferOptions(SyncCloudDialog *self)
{
	if (NULL != self && NULL != self->device && NULL != self->device->config)
	{
		C_Config *config;
		config = self->device->config;
		return config->ReadInt(L"CloudSyncMode", 0);
	}

	return 0;
}

static BOOL SyncCloudDialog_WriteTransferOptions(SyncCloudDialog *self, int mode)
{
	if (NULL != self && NULL != self->device && NULL != self->device->config)
	{
		C_Config *config;
		config = self->device->config;
		config->WriteInt(L"CloudSyncMode", mode);
		return TRUE;
	}

	return FALSE;
}

static void SyncCloudDialog_UpdateTransferOptions(HWND hwnd, int mode)
{
	const int control[] = 
	{
		IDC_CLOUDSYNC_ALL,
		IDC_CLOUDSYNC_SEL,
		IDC_CLOUDSYNC_NOTSEL
	};

	const int controls[] = 
	{
		IDS_CLOUD_SYNC_ALL_SONGS,
		IDS_CLOUD_SYNC_SEL_SONGS,
		IDS_CLOUD_SYNC_NONSEL_SONGS
	};
	const int controls_pl[] = 
	{
		IDS_CLOUD_SYNC_ALL_PL,
		IDS_CLOUD_SYNC_SEL_PL,
		IDS_CLOUD_SYNC_NONSEL_PL
	};

	switch (mode)
	{
		case 0:
			for (int i = 0; i < sizeof(controls_pl)/sizeof(controls_pl[0]); i++)
			{
				SetDlgItemText(hwnd, control[i], WASABI_API_LNGSTRINGW(controls_pl[i]));
			}
			ShowWindow(GetDlgItem(hwnd, IDC_LIST_ADD_PL), TRUE);
			ShowWindow(GetDlgItem(hwnd, IDC_LIST_ADD), FALSE);
		break;

		case 1:
			for (int i = 0; i < sizeof(controls)/sizeof(controls[0]); i++)
			{
				SetDlgItemText(hwnd, control[i], WASABI_API_LNGSTRINGW(controls[i]));
			}
			ShowWindow(GetDlgItem(hwnd, IDC_LIST_ADD_PL), FALSE);
			ShowWindow(GetDlgItem(hwnd, IDC_LIST_ADD), TRUE);
		break;
	}
}

static BOOL SyncCloudDialog_UpdateCaption(HWND hwnd)
{
	SyncCloudDialog *self;
	HWND captionWindow, hList;
	wchar_t buffer[1024] = {0};

	self = SYNCCLOUDDIALOG(hwnd);
	if (NULL == self)
		return FALSE;

	captionWindow = GetDlgItem(hwnd, IDC_DETAILS);
	if (NULL == captionWindow)
		return FALSE;

	int mode = SyncCloudDialog_ReadTransferOptions(self);
	hList = GetDlgItem(hwnd, (mode ? IDC_LIST_ADD : IDC_LIST_ADD_PL));
	if (NULL == hList)
		return FALSE;

	int64_t total_size = 0, sel_size = 0,
			device_size = self->device->dev->getDeviceCapacityAvailable();

	int all = (IsDlgButtonChecked(hwnd, IDC_CLOUDSYNC_ALL) == BST_CHECKED);
	int sel = (IsDlgButtonChecked(hwnd, IDC_CLOUDSYNC_SEL) == BST_CHECKED);
	if (mode)
	{
		for (int i = 0; i < self->libraryList->GetSize(); i++)
		{
			if (all || SendMessage(hList, LB_GETSEL, i, 0) != !sel)
			{
				itemRecordW *record = (itemRecordW *)self->libraryList->Get(i);
				total_size += record->filesize * 1024;
				sel_size += 1;
			}
		}
	}
	else
	{
		for (int i = 0; i < SendMessage(hList, LB_GETCOUNT, 0, 0); i++)
		{
			if (all || SendMessage(hList, LB_GETSEL, i, 0) != !sel)
			{
				int item = SendMessage(hList, LB_GETITEMDATA, i, 0);
				if (item != -1)
				{
					SyncCloudItemListLoader *list = (SyncCloudItemListLoader *)self->playlistsList->Get(item);
					total_size += list->total_size;
					sel_size += list->len;
				}
			}
		}
	}

	wchar_t buf[64] = {0}, buf2[64] = {0}, buf3[128] = {0}, buf4[128] = {0},
			buffer2[256] = {0}, local_lib[128] = {0};
	WASABI_API_LNG->FormattedSizeString(buf, ARRAYSIZE(buf), total_size);
	WASABI_API_LNG->FormattedSizeString(buf2, ARRAYSIZE(buf2), device_size);
	int64_t over_limit = (total_size - device_size);
	if (over_limit > 0)
	{
		WASABI_API_LNG->FormattedSizeString(buf4, ARRAYSIZE(buf4), over_limit);
	}
	self->device->GetDisplayName(buf3, ARRAYSIZE(buf3));

	int over_size = (device_size > 0 && total_size <= device_size);

	// enable the transfer button as applicable
	EnableWindow(GetDlgItem(hwnd, IDOK), sel_size && over_size);

	WASABI_API_LNGSTRINGW_BUF(IDS_CLOUD_LOCAL_LIBRARY, local_lib, 128);

	StringCchPrintf(buffer, ARRAYSIZE(buffer),
					WASABI_API_LNGSTRINGW(IDS_CLOUD_TX_X_TO_Y),
					local_lib, buf3);
	SetWindowText(hwnd, buffer);

	StringCchPrintf(buffer, ARRAYSIZE(buffer), WASABI_API_LNGSTRINGW(IDS_CLOUD_TX_HAVE_SELECTED_X), local_lib, buf3, sel_size, buf, buf3, buf2);

	if (sel_size && over_limit > 0)
	{
		StringCchPrintf(buffer2, ARRAYSIZE(buffer2), WASABI_API_LNGSTRINGW(IDS_CLOUD_TX_OVER_LIMIT), buf4, buf3);
		StringCchCat(buffer, ARRAYSIZE(buffer), buffer2);
	}
	else if (!sel_size)
	{
		StringCchPrintf(buffer, ARRAYSIZE(buffer), WASABI_API_LNGSTRINGW(IDS_CLOUD_TX_NO_SEL), local_lib, buf3);
	}

	SendMessage(captionWindow, WM_SETTEXT, 0, (LPARAM)buffer);
	return TRUE;
}

static void SyncCloudDialog_GenerateList(HWND hwnd)
{
	SyncCloudDialog *self;
	HWND hList;

	self = SYNCCLOUDDIALOG(hwnd);
	if (NULL == self)
		return;

	int mode = SyncCloudDialog_ReadTransferOptions(self);
	hList = GetDlgItem(hwnd, (mode ? IDC_LIST_ADD : IDC_LIST_ADD_PL));
	if (NULL == hList)
		return;

	if (mode)
	{
		if ((IsDlgButtonChecked(hwnd, IDC_CLOUDSYNC_ALL) == BST_UNCHECKED))
		{
			int sel = (IsDlgButtonChecked(hwnd, IDC_CLOUDSYNC_SEL) == BST_CHECKED),
				current = 0, l = self->libraryList->GetSize();
			for (int i = 0; i < l; i++)
			{
				if (SendMessage(hList, LB_GETSEL, i, 0) != !sel)
				{
					current++;
				}
				else
				{
					self->libraryList->Del(current);
				}
			}
		}
	}
	else
	{
		// no need for this so clear out
		int l = self->libraryList->GetSize();
		for (int i = 0; i < l; i++)
		{
			self->libraryList->Del(0);
		}

		int all = (IsDlgButtonChecked(hwnd, IDC_CLOUDSYNC_ALL) == BST_CHECKED);
		int sel = (IsDlgButtonChecked(hwnd, IDC_CLOUDSYNC_SEL) == BST_CHECKED);
		for (int i = 0; i < SendMessage(hList, LB_GETCOUNT, 0, 0); i++)
		{
			if (all || SendMessage(hList, LB_GETSEL, i, 0) != !sel)
			{
				int item = SendMessage(hList, LB_GETITEMDATA, i, 0);
				if (item != -1)
				{
					SyncCloudItemListLoader *list = (SyncCloudItemListLoader *)self->playlistsList->Get(item);
					if (list)
					{
						for (int j = 0; j < list->len; j++)
						{
							songMapping* song = &list->songs[j];
							if (song)
							{
								itemRecordW *result = AGAVE_API_MLDB->GetFile(song->filename);
								song->ice = (itemRecordW*)calloc(sizeof(itemRecordW), 1);
								if (result)
								{
									copyRecord(song->ice, result);
									AGAVE_API_MLDB->FreeRecord(result);
								}
								else
								{
									filenameToItemRecord(song->filename, song->ice); // ugh. Disk intensive.
								}
								if (song->ice)
								{
									self->libraryList->Add(song->ice);
								}
							}
						}
					}
				}
			}
		}
	}
}

typedef enum DeviceSyncPolicy
{
	DeviceSyncPolicy_CopyAllTracks = 0,
	DeviceSyncPolicy_CopySelTracks = 1,
	DeviceSyncPolicy_CopyNotSelTracks = 2,
} DeviceSyncPolicy;

static DeviceSyncPolicy SyncCloudDialog_ReadDeviceSyncPolicy(SyncCloudDialog *self)
{
	if (NULL != self && NULL != self->device && NULL != self->device->config)
	{
		C_Config *config;
		config = self->device->config;
		return (DeviceSyncPolicy)config->ReadInt(L"CloudSync", (int)DeviceSyncPolicy_CopyAllTracks);
	}

	return DeviceSyncPolicy_CopyAllTracks;
}

static BOOL SyncCloudDialog_WriteDeviceSyncPolicy(SyncCloudDialog *self, DeviceSyncPolicy policy)
{
	if (NULL != self && NULL != self->device && NULL != self->device->config)
	{
		C_Config *config;
		config = self->device->config;
		config->WriteInt(L"CloudSync", (int)policy);
		return TRUE;
	}

	return FALSE;
}

static BOOL SyncCloudDialog_SelectDeviceSyncPolicy(HWND hwnd, DeviceSyncPolicy policy)
{
	SyncCloudDialog *self;
	int policyControl;

	const int controls[] = 
	{
		IDC_CLOUDSYNC_ALL,
		IDC_CLOUDSYNC_SEL,
		IDC_CLOUDSYNC_NOTSEL
	};

	self = SYNCCLOUDDIALOG(hwnd);
	if (NULL == self)
		return FALSE;

	switch(policy)
	{
		case DeviceSyncPolicy_CopyAllTracks:
			policyControl = IDC_CLOUDSYNC_ALL;
			break;
		case DeviceSyncPolicy_CopySelTracks:
			policyControl = IDC_CLOUDSYNC_SEL;
			break;
		case DeviceSyncPolicy_CopyNotSelTracks:
			policyControl = IDC_CLOUDSYNC_NOTSEL;
			break;
		default:
			return FALSE;
	}

	CheckRadioButton(hwnd, controls[0], controls[2], policyControl);

	return TRUE;
}

static INT_PTR SyncCloudDialog_OnInitDialog(HWND hwnd, HWND focusWindow, LPARAM param)
{
	SyncCloudDialog *self;
	C_Config *config;

	self = (SyncCloudDialog*)param;

	if (NULL == self ||
		FALSE == SetProp(hwnd, SYNCCLOUDDIALOG_PROP, self))
	{
		EndDialog(hwnd, -1);
		return 0;
	}

	config = self->device->config;

	SyncCloudDialog_SelectDeviceSyncPolicy(hwnd, SyncCloudDialog_ReadDeviceSyncPolicy(self));

	if (FALSE != CenterWindow(hwnd, self->centerWindow))
		SendMessage(hwnd, DM_REPOSITION, 0, 0L);

	int mode = SyncCloudDialog_ReadTransferOptions(self);
	SendDlgItemMessageW(hwnd, IDC_TX_MODE, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_PLAYLISTS));
	SendDlgItemMessageW(hwnd, IDC_TX_MODE, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_SONGS));
	SendDlgItemMessageW(hwnd, IDC_TX_MODE, CB_SETCURSEL, (WPARAM)mode, 0);

	SyncCloudDialog_UpdateTransferOptions(hwnd, mode);
	SyncCloudDialog_PopulateTrackLists(hwnd);
	SyncCloudDialog_PopulatePlaylistLists(hwnd);
	SyncCloudDialog_UpdateCaption(hwnd);

	return 0;
}

static void SyncCloudDialog_OnDestroy(HWND hwnd)
{
	RemoveProp(hwnd, SYNCCLOUDDIALOG_PROP);
}

static void SyncCloudDialog_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND controlWindow)
{
	SyncCloudDialog *self;
	C_Config *config;

	self = SYNCCLOUDDIALOG(hwnd);
	if (NULL == self)
		return;

	config = self->device->config;

	switch(commandId)
	{
		case IDC_TX_MODE:
			if (CBN_SELCHANGE == eventId)
			{
				int mode = SendDlgItemMessage(hwnd, commandId, CB_GETCURSEL, 0, 0);
				SyncCloudDialog_WriteTransferOptions(self, mode);
				SyncCloudDialog_UpdateTransferOptions(hwnd, mode);
				SyncCloudDialog_UpdateCaption(hwnd);
			}
			break;

		case IDC_CLOUDSYNC_ALL:
			if (BN_CLICKED == eventId)
			{
				SyncCloudDialog_WriteDeviceSyncPolicy(self, DeviceSyncPolicy_CopyAllTracks);
				SyncCloudDialog_SelectDeviceSyncPolicy(hwnd, DeviceSyncPolicy_CopyAllTracks);
				SyncCloudDialog_UpdateCaption(hwnd);
			}
			break;

		case IDC_CLOUDSYNC_SEL:
			if (BN_CLICKED == eventId)
			{
				SyncCloudDialog_WriteDeviceSyncPolicy(self, DeviceSyncPolicy_CopySelTracks);
				SyncCloudDialog_SelectDeviceSyncPolicy(hwnd, DeviceSyncPolicy_CopySelTracks);
				SyncCloudDialog_UpdateCaption(hwnd);
			}
			break;

		case IDC_CLOUDSYNC_NOTSEL:
			if (BN_CLICKED == eventId)
			{
				SyncCloudDialog_WriteDeviceSyncPolicy(self, DeviceSyncPolicy_CopyNotSelTracks);
				SyncCloudDialog_SelectDeviceSyncPolicy(hwnd, DeviceSyncPolicy_CopyNotSelTracks);
				SyncCloudDialog_UpdateCaption(hwnd);
			}
			break;

		case IDOK:
			SyncCloudDialog_GenerateList(hwnd);
			EndDialog(hwnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;

		case IDC_LIST_ADD:
		case IDC_LIST_ADD_PL:
			if (eventId == LBN_SELCHANGE)
			{
				SyncCloudDialog_UpdateCaption(hwnd);
			}
			break;
	}
}

static INT_PTR CALLBACK SyncCloudDialog_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return SyncCloudDialog_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		SyncCloudDialog_OnDestroy(hwnd); return TRUE;
		case WM_COMMAND:		SyncCloudDialog_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
	}

	return FALSE;
}