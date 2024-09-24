#include "main.h"
#include "./util.h"
#include "./channelEditor.h"
#include "api__ml_wire.h"
#include "./defaults.h"
#include "./updateTime.h"
#include "./UpdateAutoDownload.h"
#include "./errors.h"
#include "./feeds.h"
#include "./feedUtil.h"
#include "./cloud.h"
#include "./subscriptionView.h"


#include <strsafe.h>

using namespace Nullsoft::Utility;
extern Cloud cloud;

typedef struct __CHANNELEDITOR
{
	HWND			hOwner;
	size_t		iChannel;
	UINT		flags;
	BOOL		enableAutoDownload;
	BOOL		enableAutoUpdate;
	__time64_t	updateTime;
	INT			numOfAutoDownloadEpisodes;
} CHANNELEDITOR;


#define GetEditor(__hwnd) ((CHANNELEDITOR*)GetPropW((__hwnd), MAKEINTATOM(VIEWPROP)))

static INT_PTR CALLBACK ChannelEditor_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR ChannelEditor_Show(HWND hOwner, size_t channelIndex, UINT flags)
{
	if (0 == VIEWPROP)
		return 0;

	if (NULL == hOwner) 
		hOwner = plugin.hwndLibraryParent;
	
	if (0 == (CEF_CREATENEW & flags) && BAD_CHANNEL == channelIndex)
		return 0;

	CHANNELEDITOR editor;
	ZeroMemory(&editor, sizeof(editor));
    
	editor.hOwner = hOwner;
	editor.iChannel = channelIndex;
	editor.flags = flags;

	INT_PTR result = WASABI_API_DIALOGBOXPARAMW(IDD_ADDURL, hOwner, ChannelEditor_DialogProc, (LPARAM)&editor);
	return result;
}


static BOOL ChannelEditor_SetUpdateTime(HWND hwnd, __time64_t updateTime, BOOL enableAutoUpdate)
{
	HWND hCombo = GetDlgItem(hwnd, IDC_UPDATELIST);
	if (NULL == hCombo) return FALSE;
	
	INT selectedVal = Update::GetSelection(updateTime, (FALSE != enableAutoUpdate));
	INT iCount = (INT)SNDMSG(hCombo, CB_GETCOUNT, 0, 0L);
	INT iSelect = CB_ERR;
	for (INT i = 0; i < iCount && CB_ERR == iSelect; i++)
	{
		if (selectedVal == (INT)SNDMSG(hCombo, CB_GETITEMDATA, i, 0L))
			iSelect = i;
	}
	
	if (CB_ERR == iSelect) 
		return FALSE;

	if (iSelect != (INT)SNDMSG(hCombo, CB_GETCURSEL, 0, 0L) && 
		CB_ERR == SNDMSG(hCombo, CB_SETCURSEL, iSelect, 0))
	{
		return FALSE;
	}
	    
	return TRUE;
}

static BOOL ChannelEditor_SetAutoDownload(HWND hwnd, INT numOfEpisodes, BOOL enableAutoDownload)
{
	HWND hCombo = GetDlgItem(hwnd, IDC_AUTODOWNLOADLIST);
	if (NULL == hCombo) return FALSE;
	
	INT selectedVal = UpdateAutoDownload::GetSelection(numOfEpisodes, (FALSE != enableAutoDownload));
	INT iCount = (INT)SNDMSG(hCombo, CB_GETCOUNT, 0, 0L);
	INT iSelect = CB_ERR;
	for (INT i = 0; i < iCount && CB_ERR == iSelect; i++)
	{
		if (selectedVal == (INT)SNDMSG(hCombo, CB_GETITEMDATA, i, 0L))
			iSelect = i;
	}
	
	if (CB_ERR == iSelect) 
		return FALSE;

	if (iSelect != (INT)SNDMSG(hCombo, CB_GETCURSEL, 0, 0L) && 
		CB_ERR == SNDMSG(hCombo, CB_SETCURSEL, iSelect, 0))
	{
		return FALSE;
	}
	    
	return TRUE;
}

static BOOL ChannelEditor_InitCombobox(HWND hwnd)
{
	HWND hCombo = GetDlgItem(hwnd, IDC_UPDATELIST);
	HWND hAutoDownloadCombo = GetDlgItem(hwnd, IDC_AUTODOWNLOADLIST);
	if ( NULL == hCombo || NULL == hAutoDownloadCombo ) return FALSE;

	WCHAR szBuffer[256] = {0};
	INT iPos = 0;

	SNDMSG(hCombo, WM_SETREDRAW, FALSE, 0L);

	for (INT i = 0; i < Update::TIME_NUMENTRIES; i++) 
	{
		if (NULL != Update::GetTitle(i, szBuffer, ARRAYSIZE(szBuffer)))
		{
			iPos = (INT)SNDMSG(hCombo, CB_ADDSTRING, 0, (LPARAM)szBuffer);
			if (CB_ERR != iPos)
			{
				SNDMSG(hCombo, CB_SETITEMDATA, (WPARAM)iPos, (LPARAM)i);
			}
		}
	}

	SNDMSG(hCombo, WM_SETREDRAW, TRUE, 0L);

	SNDMSG(hAutoDownloadCombo, WM_SETREDRAW, FALSE, 0L);

	for (INT i = 0; i < UpdateAutoDownload::AUTODOWNLOAD_NUMENTRIES; i++) 
	{
		if (NULL != UpdateAutoDownload::GetTitle(i, szBuffer, ARRAYSIZE(szBuffer)))
		{
			iPos = (INT)SNDMSG(hAutoDownloadCombo, CB_ADDSTRING, 0, (LPARAM)szBuffer);
			if (CB_ERR != iPos)
			{
				SNDMSG(hAutoDownloadCombo, CB_SETITEMDATA, (WPARAM)iPos, (LPARAM)i);
			}
		}
	}
	
	SNDMSG(hAutoDownloadCombo, WM_SETREDRAW, TRUE, 0L);
	
	return TRUE;
}

static BOOL ChannelEditor_EnableUserSettings(HWND hwnd, BOOL fEnable)
{
	BOOL result;
	UINT activeButton = (FALSE == fEnable) ? IDC_USEDEFAULTS : IDC_USECUSTOM;

	result = CheckRadioButton(hwnd, IDC_USEDEFAULTS, IDC_USECUSTOM, activeButton);
		
	if (FALSE != result)
	{
		SendDlgItemMessage(hwnd, activeButton, BM_CLICK, 0, 0L);	
	}
	return result;
}

#if 0
static BOOL ChannelEditor_EnableAutoDownload(HWND hwnd, BOOL fEnable)
{
	BOOL result;
	result = CheckDlgButton(hwnd, IDC_AUTODOWNLOAD, (FALSE != fEnable) ? BST_CHECKED : BST_UNCHECKED);
	return result;
}
#endif

static BOOL ChannelEditor_SetUrl(HWND hwnd, LPCWSTR pszUrl)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_EDITURL);
	if (NULL == hEdit) return FALSE;

	return SNDMSG(hEdit, WM_SETTEXT, 0, (LPARAM)pszUrl);
}
static void ChannelEditor_UpdateTitle(HWND hwnd, BOOL fModified)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL == editor) return;

	if (0 != (CEF_CREATENEW & editor->flags))
		return;

	

	WCHAR szBuffer[256] = {0};
	size_t remaining = ARRAYSIZE(szBuffer);
	LPWSTR cursor = szBuffer;
	
	if (FALSE != fModified)
	{		
		StringCchCopyEx(cursor, remaining, L"*", &cursor, &remaining, 0);
	}

	WASABI_API_LNGSTRINGW_BUF(IDS_EDIT_CHANNEL, cursor, remaining);
	INT cchLen = lstrlen(cursor);
	cursor += cchLen;
	remaining -= cchLen;
	
	AutoLock lock (channels);
	LPCWSTR title = NULL;
	if (editor->iChannel < channels.size())
	{
		Channel *channel = &channels[editor->iChannel];
		title = channel->title;
		if (NULL != title && L'\0' != title)
		{
			StringCchPrintf(cursor, remaining, L": %s", title);
		}
	}
	
	SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)szBuffer);
}

static void ChannelEditor_UpdateModifiedState(HWND hwnd)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL == editor) return;

	if (0 != (CEF_CREATENEW & editor->flags))
		return;

	AutoLock lock (channels);

	if (editor->iChannel >= channels.size())
		return;

	BOOL fModified = FALSE;

	Channel *channel = &channels[editor->iChannel];

	HWND hControl;

	if (FALSE == fModified && (NULL != (hControl = GetDlgItem(hwnd, IDC_USECUSTOM))) &&
		(BST_CHECKED == SNDMSG(hControl, BM_GETCHECK, 0, 0L)) != (false == channel->useDefaultUpdate))
	{
		fModified = TRUE;
	}

	if (false == channel->useDefaultUpdate)
	{
		if (editor->enableAutoDownload != (BOOL)channel->autoDownload ||
			editor->enableAutoUpdate != (BOOL)channel->autoUpdate ||
			editor->updateTime != channel->updateTime ||
			editor->numOfAutoDownloadEpisodes != channel->autoDownloadEpisodes)
		{
			fModified = TRUE;
		}
	}

	hControl = GetDlgItem(hwnd, IDOK);
	if (NULL != hControl) EnableWindow(hControl, fModified);

	ChannelEditor_UpdateTitle(hwnd, fModified);
}

static BOOL ChannelEditor_Reposition(HWND hwnd)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL == editor) return FALSE;

	RECT rectEditor, rectOwner;

	if (0 != (CEF_CENTEROWNER & editor->flags))
	{
		if (FALSE == GetWindowRect(editor->hOwner, &rectOwner) ||
			FALSE == GetWindowRect(hwnd, &rectEditor))
		{
			return FALSE;
		}

		LONG x = rectOwner.left + ((rectOwner.right - rectOwner.left) - (rectEditor.right - rectEditor.left))/2;
		LONG y = rectOwner.top + ((rectOwner.bottom - rectOwner.top) - (rectEditor.bottom - rectEditor.top))/2;
		SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
		SendMessage(hwnd, DM_REPOSITION, 0, 0L);
	}


	
	return TRUE;
}

static INT_PTR ChannelEditor_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{

	CHANNELEDITOR *editor = (CHANNELEDITOR*)lParam;
	if (NULL == editor || FALSE == SetProp(hwnd, MAKEINTATOM(VIEWPROP), editor))
	{
		EndDialog(hwnd, 0);
		return 0;
	}

	ChannelEditor_InitCombobox(hwnd);

	editor->enableAutoDownload        = autoDownload;
	editor->enableAutoUpdate          = autoUpdate;
	editor->updateTime                = updateTime;
	editor->numOfAutoDownloadEpisodes = autoDownloadEpisodes;
	
	if (0 != (CEF_CREATENEW & editor->flags))
	{		
		ChannelEditor_SetUrl(hwnd, L"");
		ChannelEditor_EnableUserSettings(hwnd, FALSE);
	}
	else
	{
		AutoLock lock (channels);
		if (editor->iChannel >= channels.size())
		{
			EndDialog(hwnd, 0);
			return 0;
		}
		Channel *channel = &channels[editor->iChannel];

		HWND hControl;
					
		hControl = GetDlgItem(hwnd, IDC_EDITURL);
		if (NULL != hControl) SNDMSG(hControl, EM_SETREADONLY, TRUE, 0L);

		hControl = GetDlgItem(hwnd, IDOK);
		if (NULL != hControl) 
		{
			WCHAR szBuffer[128] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_SAVE, szBuffer, ARRAYSIZE(szBuffer));
			SNDMSG(hControl, WM_SETTEXT, 0, (LPARAM)szBuffer);
		}

		if (false == channel->useDefaultUpdate)
		{
			editor->enableAutoDownload        = channel->autoDownload;
			editor->enableAutoUpdate          = channel->autoUpdate;
			editor->updateTime                = channel->updateTime;
			editor->numOfAutoDownloadEpisodes = channel->autoDownloadEpisodes;
		}

		ChannelEditor_SetUrl(hwnd, channel->url);
		ChannelEditor_EnableUserSettings(hwnd, !channel->useDefaultUpdate);
	}
	
	ChannelEditor_Reposition(hwnd);
	
	return TRUE;
}

static void ChannelEditor_OnDestroy(HWND hwnd)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	RemoveProp(hwnd, MAKEINTATOM(VIEWPROP));
}

static void ChannelEditor_UpdateUserSettings(HWND hwnd)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL == editor) return;

	HWND hControl;
	FALSE;

	hControl = GetDlgItem(hwnd, IDC_USECUSTOM);
    BOOL enableUser = (NULL != hControl && BST_CHECKED == (INT)SNDMSG(hControl, BM_GETCHECK, 0, 0L));

	if (FALSE == enableUser)
	{
		ChannelEditor_SetUpdateTime(hwnd, updateTime, autoUpdate);
		ChannelEditor_SetAutoDownload(hwnd, autoDownloadEpisodes, autoDownload);
	}
	else
	{
		ChannelEditor_SetUpdateTime(hwnd, editor->updateTime, editor->enableAutoUpdate);
		ChannelEditor_SetAutoDownload(hwnd, editor->numOfAutoDownloadEpisodes, editor->enableAutoDownload);
	}

	const INT szControl[] = { IDC_UPDATELIST, IDC_AUTODOWNLOADLIST, IDC_STATIC_UPDATEEVERY, IDC_STATIC_AUTODOWNLOAD, };
	for (INT i = 0; i < ARRAYSIZE(szControl); i++)
	{
		hControl = GetDlgItem(hwnd, szControl[i]);
		if (NULL != hControl) EnableWindow(hControl, enableUser);
	}
	
	ChannelEditor_UpdateModifiedState(hwnd);

}

static INT_PTR ChannelEditor_SaveChannel(HWND hwnd)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL == editor) return 0;

	AutoLock lock (channels);
	if (editor->iChannel >= channels.size()) 
		return 0;

	HWND hControl = GetDlgItem(hwnd, IDC_USECUSTOM);
	if (NULL == hControl) return 0;

	Channel *channel = &channels[editor->iChannel];

	if (BST_CHECKED == SNDMSG(hControl, BM_GETCHECK, 0, 0L))
	{
		channel->useDefaultUpdate     = false;
		channel->autoDownload         = (FALSE != editor->enableAutoDownload);
		channel->updateTime           = editor->updateTime;
		channel->autoUpdate           = (FALSE != editor->enableAutoUpdate);
		channel->autoDownloadEpisodes = editor->numOfAutoDownloadEpisodes;
	}
	else
	{
		channel->useDefaultUpdate     = true;
		channel->autoDownload         = autoDownload;
		channel->updateTime           = updateTime;
		channel->autoUpdate           = autoUpdate;
		channel->autoDownloadEpisodes = autoDownloadEpisodes;
	}

	if (channel->autoUpdate)
		cloud.Pulse();
	return
		IDOK;
}

static DWORD AddUrlThread(void *vBuffer, HWND hwndDlg)
{
	Channel newFeed;
	newFeed.autoUpdate   = false; // TODO check defaults
	newFeed.autoDownload = false; // leave this as false
	newFeed.SetURL((const wchar_t *)vBuffer);
	delete vBuffer;

	int downloadError = DownloadFeedInformation(newFeed);
	if (downloadError != DOWNLOAD_SUCCESS)
		return downloadError;

	 // TODO check defaults;
	if (IsDlgButtonChecked(hwndDlg, IDC_USECUSTOM) == BST_CHECKED)
	{
		LRESULT timeSelection;
		timeSelection = SendMessage(GetDlgItem(hwndDlg, IDC_UPDATELIST), CB_GETCURSEL, 0, 0);
		if (timeSelection != CB_ERR)
		{
			newFeed.autoUpdate = Update::GetAutoUpdate(timeSelection);
			newFeed.updateTime = Update::GetTime(timeSelection);
		}
		LRESULT episodesSelection;
		episodesSelection = SendMessage(GetDlgItem(hwndDlg, IDC_AUTODOWNLOADLIST), CB_GETCURSEL, 0, 0);
		if (episodesSelection != CB_ERR)
		{
			newFeed.autoDownload         = UpdateAutoDownload::GetAutoDownload(episodesSelection);
			newFeed.autoDownloadEpisodes = UpdateAutoDownload::GetAutoDownloadEpisodes(episodesSelection);
		}
		newFeed.useDefaultUpdate = false;
	}
	else
	{
		newFeed.useDefaultUpdate     = true;
		newFeed.autoUpdate           = autoUpdate;
		newFeed.updateTime           = updateTime;
		newFeed.autoDownloadEpisodes = autoDownloadEpisodes;
		newFeed.autoDownload         = autoDownload;
	}

	//newFeed.title += L" ";
	AutoLock lock (channels LOCKNAME("AddURL"));
	if (!channels.AddChannel(newFeed))
	{
		wchar_t error_msg[1024] = {0}, titleStr[64] = {0};
		StringCchPrintf( error_msg, 1024, WASABI_API_LNGSTRINGW( IDS_CHANNEL_ALREADY_PRESENT ), newFeed.title, newFeed.url );
		MessageBox( hwndDlg, error_msg, WASABI_API_LNGSTRINGW_BUF( IDS_DUPLICATE_CHANNEL, titleStr, 64 ), MB_OK );

		return DOWNLOAD_DUPLICATE;
	}
	else
	{
		cloud.Pulse(); // TODO why?
		HWND hView = SubscriptionView_FindWindow();
		if (NULL != hView)
		{
			SubscriptionView_RefreshChannels(hView, TRUE);
		}
		
	}
	return DOWNLOAD_SUCCESS; // success
}

static INT ChannelEditor_AddUrl(HWND hwnd)
{
	size_t length = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDITURL)) + 1;
	wchar_t *buffer = new wchar_t[length];
	ZeroMemory(buffer,sizeof(wchar_t)*length);
	GetDlgItemText(hwnd, IDC_EDITURL, buffer, (int)length);
	return AddUrlThread((void *)buffer, hwnd);
}

static INT_PTR ChannelEditor_CreateChannel(HWND hwnd)
{
	INT result = ChannelEditor_AddUrl(hwnd);
	if (DOWNLOAD_SUCCESS == result)
		return IDOK;

	INT messageId = IDS_ERROR_ADDING_URL;
	INT titleId = IDS_ERROR_ADDING_URL;

	switch (result)
	{
		case DOWNLOAD_ERROR_PARSING_XML:	messageId = IDS_ERROR_PARSING_XML_FROM_SERVER; break;
		case DOWNLOAD_NOTRSS:				messageId = IDS_LINK_HAS_NO_RSS_INFO; break;
		case DOWNLOAD_404:					messageId = IDS_INVALID_LINK; break;
		case DOWNLOAD_NOHTTP:				messageId = IDS_NO_JNETLIB; titleId = IDS_JNETLIB_MISSING; break;
		case DOWNLOAD_NOPARSER:				messageId = IDS_NO_EXPAT; titleId = IDS_EXPAT_MISSING; break;
		case DOWNLOAD_CONNECTIONRESET:		messageId = IDS_CONNECTION_RESET_BY_PEER; break;
	}

	if(result != DOWNLOAD_DUPLICATE)
	{
		WCHAR szMessage[512] = {0}, szTitle[128] = {0};

		WASABI_API_LNGSTRINGW_BUF(messageId, szMessage, ARRAYSIZE(szMessage));
		WASABI_API_LNGSTRINGW_BUF(titleId, szTitle, ARRAYSIZE(szTitle));
		MessageBox(hwnd, szMessage, szTitle, MB_OK | MB_ICONERROR);
	}

	return IDIGNORE;
}

static void ChannelEditor_OnCancel(HWND hwnd)
{
	EndDialog(hwnd, IDCANCEL);
}

static void ChannelEditor_OnOk(HWND hwnd)
{	
	INT_PTR result = 0;

	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL != editor) 
	{
		if (0 == (CEF_CREATENEW & editor->flags))
		{
			result = ChannelEditor_SaveChannel(hwnd);
		}
		else
		{
			result = ChannelEditor_CreateChannel(hwnd);
		}
	}

	if (IDIGNORE != result)
	{
		EndDialog(hwnd, result);
	}
}

#if 0
static void ChannelEditor_OnAutoDownloadChanged(HWND hwnd)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL == editor) return;

	HWND hControl = GetDlgItem(hwnd, IDC_USECUSTOM);
	if (NULL == hControl || BST_CHECKED != SNDMSG(hControl, BM_GETCHECK, 0, 0L)) return;

	hControl = GetDlgItem(hwnd, IDC_AUTODOWNLOAD);
	if (NULL == hControl) return;

	editor->enableAutoDownload = (BST_CHECKED == SNDMSG(hControl, BM_GETCHECK, 0, 0L));

	ChannelEditor_UpdateModifiedState(hwnd);
}
#endif

static void ChannelEditor_OnUrlChange(HWND hwnd)
{		
	WCHAR szBuffer[4096] = {0};

	HWND hControl = GetDlgItem(hwnd, IDC_EDITURL);
	if (NULL == hControl || 0 == (INT)SNDMSG(hControl, WM_GETTEXT, (WPARAM)ARRAYSIZE(szBuffer), (LPARAM)szBuffer))
		szBuffer[0] = L'\0';

	LPCWSTR p = szBuffer;
	while (L' ' == *p && L'\0' != *p) p++;
	BOOL enableButton = (L'\0' != *p);

	hControl = GetDlgItem(hwnd, IDOK);
	if (NULL != hControl)
	{
		EnableWindow(hControl, enableButton);
	}
}

static void ChannelEditor_OnUpdateTimeChange(HWND hwnd)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL == editor) return;

	HWND hControl = GetDlgItem(hwnd, IDC_USECUSTOM);
	if (NULL == hControl || BST_CHECKED != SNDMSG(hControl, BM_GETCHECK, 0, 0L)) return;

	hControl = GetDlgItem(hwnd, IDC_UPDATELIST);
	if (NULL == hControl) return;
		
	INT iSelected = (INT)SNDMSG(hControl, CB_GETCURSEL, 0, 0L);
	if (CB_ERR == iSelected) return;

	INT selectedVal = (INT)SNDMSG(hControl, CB_GETITEMDATA, iSelected, 0L);
	if (selectedVal < 0 || selectedVal >= Update::TIME_NUMENTRIES) return;
	
	editor->updateTime = Update::GetTime(selectedVal);
	editor->enableAutoUpdate = Update::GetAutoUpdate(selectedVal);

	ChannelEditor_UpdateModifiedState(hwnd);
}

static void ChannelEditor_OnUpdateAutoDownloadChange(HWND hwnd)
{
	CHANNELEDITOR *editor = GetEditor(hwnd);
	if (NULL == editor) return;

	HWND hControl = GetDlgItem(hwnd, IDC_USECUSTOM);
	if (NULL == hControl || BST_CHECKED != SNDMSG(hControl, BM_GETCHECK, 0, 0L)) return;

	hControl = GetDlgItem(hwnd, IDC_AUTODOWNLOADLIST);
	if (NULL == hControl) return;
		
	INT iSelected = (INT)SNDMSG(hControl, CB_GETCURSEL, 0, 0L);
	if (CB_ERR == iSelected) return;

	INT selectedVal = (INT)SNDMSG(hControl, CB_GETITEMDATA, iSelected, 0L);
	if (selectedVal < 0 || selectedVal >= UpdateAutoDownload::AUTODOWNLOAD_NUMENTRIES) return;
	
	editor->numOfAutoDownloadEpisodes = UpdateAutoDownload::GetAutoDownloadEpisodes(selectedVal);
	editor->enableAutoDownload = UpdateAutoDownload::GetAutoDownload(selectedVal);

	ChannelEditor_UpdateModifiedState(hwnd);
}

static void ChannelEditor_OnCommand(HWND hwnd, UINT commandId, UINT eventId, HWND hControl)
{
	switch(commandId)
	{
		case IDCANCEL:			ChannelEditor_OnCancel(hwnd); break;
		case IDOK:				ChannelEditor_OnOk(hwnd); break;
		case IDC_USEDEFAULTS:
		case IDC_USECUSTOM:
			switch(eventId)
			{
				case BN_CLICKED:	ChannelEditor_UpdateUserSettings(hwnd); break;
			}
			break;
		case IDC_EDITURL:
			switch(eventId)
			{
				case EN_CHANGE:		ChannelEditor_OnUrlChange(hwnd); break;
			}
			break;
		case IDC_UPDATELIST:
			switch(eventId)
			{
				case CBN_SELCHANGE:	ChannelEditor_OnUpdateTimeChange(hwnd); break;
			}
			break;
		case IDC_AUTODOWNLOADLIST:
			switch(eventId)
			{
				case CBN_SELCHANGE:	ChannelEditor_OnUpdateAutoDownloadChange(hwnd); break;
			}
			break;
	}
}

static INT_PTR CALLBACK ChannelEditor_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:	return ChannelEditor_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		ChannelEditor_OnDestroy(hwnd); return TRUE;
		case WM_COMMAND:		ChannelEditor_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
	}
	return 0;
}