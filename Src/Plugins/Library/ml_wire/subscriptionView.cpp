#include "main.h"
#include "./subscriptionView.h"

#include "api__ml_wire.h"
#include "./util.h"
#include "Feeds.h"
#include "RFCDate.h"
#include "Cloud.h"
#include "./channelEditor.h"
#include "BackgroundDownloader.h"
#include "./service.h"
#include "./layout.h"
#include "../nu/menushortcuts.h"
#include "..\..\General\gen_ml/ml_ipc.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "../omBrowser/browserView.h"
#include "./navigation.h"

#include <strsafe.h>

#include "../../../WAT/WAT.h"

#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200
#endif // !HDF_SORTUP

using namespace Nullsoft::Utility;

int   itemTitleWidth        = 200;
int   itemDateWidth         = 120;
int   itemMediaWidth        = 100;
int   itemSizeWidth         = 100;

int   currentItemSort       = 1; // -1 means no sort active
bool  itemSortAscending	    = false;
bool  channelSortAscending  = true;
int   channelLastSelection  = -1;

float htmlDividerPercent    = 0.666f;
float channelDividerPercent = 0.333f;

extern int IPC_LIBRARY_SENDTOMENU;
extern librarySendToMenuStruct s;

static int episode_info_cy;
HMENU g_context_menus3 = NULL;

extern Cloud cloud;
extern wchar_t serviceUrl[1024];

#define SUBSCRIPTIONVIEW_NAME		L"SubscriptionView"

typedef enum
{
    COLUMN_TITLE = 0,
    COLUMN_DATEADDED,
	//COLUMN_MEDIA,
    COLUMN_MEDIA_TIME,
	COLUMN_MEDIA_SIZE,
} ListColumns;

typedef enum 
{
	LI_CHANNEL = 0,
	LI_ITEM,
	LI_VERT,
	LI_FINDNEW,
	LI_ADD,
	LI_EDIT,
	LI_DELETE,
	LI_REFRESH,
	LI_HORZ,
	LI_EPISODE_INFO,
	LI_INFO,
	LI_PLAY,
	LI_ENQUEUE,
	LI_CUSTOM,
	LI_DOWNLOAD,
	LI_VISIT,
	LI_STATUS,
	LI_LAST
} LayoutItems;

typedef struct __PODCAST
{
	LONG	vertDivider;
	LONG	horzDivider;
	LONG	bottomRowSpace;
	LONG	middleRowSpace;
	HRGN	updateRegion;
	POINTS	updateOffset;
	size_t	channelActive;
	BOOL	channelAscending;
	BOOL	itemAscending;
	LPWSTR	infoUrl;
	LPWSTR	description;
} PODCAST;

#define GetPodcast(__hwnd) ((PODCAST*)GetPropW((__hwnd), MAKEINTATOM(VIEWPROP)))

#define LAYOUTREASON_RESIZE      0	
#define LAYOUTREASON_DIV_LEFT    1	
#define LAYOUTREASON_DIV_RIGHT   2
#define LAYOUTREASON_DIV_TOP     3
#define LAYOUTREASON_DIV_BOTTOM  4

#define UPDATE_TIMER		    12
#define UPDATE_DELAY		     0

static void CALLBACK PodcastChannel_UpdateTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, ULONG elapsed);
static void CALLBACK PodcastItem_UpdateTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, ULONG elapsed);
static size_t PodcastChannel_GetActive(HWND hwnd);

ptrdiff_t inline CopyCharW(wchar_t *dest, const wchar_t *src)
{
	wchar_t *end = CharNextW(src);
	ptrdiff_t count = end-src;
	for (ptrdiff_t i=0;i<count;i++)
	{
		*dest++=*src++;
	}

	return count;
}

// TODO for the moment, this is just to cleanup bad
// urls which use spaces instead of %20 in the feed
wchar_t *urlencode( wchar_t *p )
{
	if ( p )
	{
		wchar_t buf[ MAX_PATH * 4 ], *i = buf;
		StringCchCopyW( buf, MAX_PATH * 4, p );
		while ( p && *p )
		{
			if ( !StrCmpNW( i, L" ", 1 ) )
			{
				*i++ = L'%';
				*i++ = L'2';
				*i = L'0';
				p += 1;
			}
			else
			{
				CopyCharW( i, p );
				p = CharNextW( p );
			}

			i = CharNextW( i );
		}

		*i = 0;

		return _wcsdup( buf );
	}

	return NULL;
}

static void SubscriptionView_EnableMenuCommands(HMENU hMenu, const INT *commandList, INT commandCount, BOOL fEnable)
{
	UINT uEnable = MF_BYCOMMAND | MF_ENABLED;
	if (FALSE == fEnable) uEnable |= (MF_GRAYED | MF_DISABLED);

	for (INT i = 0; i < commandCount; i++)
	{
		EnableMenuItem(hMenu, commandList[i], uEnable);
	}
}

static void PodcastItem_EnableButtons(HWND hwnd, BOOL fEnable)
{
	static const INT szButtons[] = { IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM, IDC_DOWNLOAD, };
	for(INT i = 0; i < ARRAYSIZE(szButtons); i++)
	{
		HWND hControl = GetDlgItem(hwnd, szButtons[i]);
		if (NULL != hControl)
		{
			EnableWindow(hControl, fEnable);
		}
	}
}

static void PodcastChannel_EnableButtons(HWND hwnd, BOOL fEnable)
{
	static const INT szButtons[] = { IDC_EDIT, IDC_REFRESH, IDC_DELETE, };
	HWND hControl;

	for(INT i = 0; i < ARRAYSIZE(szButtons); i++)
	{
		hControl = GetDlgItem(hwnd, szButtons[i]);
		if (NULL != hControl)
		{
			EnableWindow(hControl, fEnable);
		}
	}
}

static HRESULT SubscriptionView_FormatFont(LPWSTR pszBuffer, INT cchBufferMax, HFONT hFont)
{
	const WCHAR szTemplate[] = L"font: %s %d %dpx \"%s\";";

	if (NULL == pszBuffer) 
		return E_POINTER;

	pszBuffer[0] = L'\0';

	if (NULL == hFont) return E_POINTER;

	LOGFONT lf = {0};
	if (0 == GetObject(hFont, sizeof(LOGFONT), &lf))
		return E_FAIL;

	if (lf.lfHeight < 0)
		lf.lfHeight = -lf.lfHeight;

	return StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS,
							 szTemplate, ((0 == lf.lfItalic) ? L"normal" : L"italic"),
							 lf.lfWeight, lf.lfHeight, L"Arial"/*lf.lfFaceName*/);
}

static BOOL SubscriptionView_SetDescription(HWND hwnd, LPCWSTR pszInfo)
{
	HWND hBrowser = GetDlgItem(hwnd, IDC_DESCRIPTION);
	if (NULL == hBrowser) return FALSE;

	const WCHAR szTemplate[] =  L"<html>"
								L"<base target=\"_blank\">"
								L"<style type=\"text/css\"> body { %s overflow-y: auto; overflow-x: auto;}</style>"
								L"<body>%s</body>"
								L"</html>";

	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	WCHAR szFont[128] = {0};
	HFONT hFont = (HFONT)SendMessage(hItems, WM_GETFONT, 0, 0L);
	if (FAILED(SubscriptionView_FormatFont(szFont, ARRAYSIZE(szFont), hFont)))
		szFont[0] = L'\0';

	if (NULL == pszInfo) pszInfo = L"";

	INT cchLen = lstrlen(pszInfo) + 1;
	cchLen += lstrlen(szFont);
	cchLen += ARRAYSIZE(szTemplate);

	BSTR documentData = SysAllocStringLen(NULL, cchLen);
	if (NULL == documentData) return FALSE;

	BOOL result;

	if ( SUCCEEDED( StringCchPrintfEx( documentData, cchLen, NULL, NULL, STRSAFE_IGNORE_NULLS, szTemplate, szFont, pszInfo ) ) )
		result = BrowserView_WriteDocument( hBrowser, documentData, TRUE );
	else
		result = FALSE;

	if (FALSE == result)
		SysFreeString(documentData);

	return result;
}

static void SubscriptionView_UpdateInfo( HWND hwnd )
{
	PODCAST *podcast = GetPodcast( hwnd );
	if ( NULL == podcast )
		return;

	AutoLock channelLock( channels LOCKNAME( "UpdateInfo" ) );

	LPCWSTR infoText = NULL;
	BOOL itemEnabled = FALSE;

	size_t iChannel = PodcastChannel_GetActive( hwnd );
	if ( BAD_CHANNEL != iChannel )
	{
		Channel *channel = &channels[ iChannel ];

		HWND hItems = GetDlgItem( hwnd, IDC_ITEMLIST );
		INT selectedCount = ( NULL != hItems ) ? (INT)SNDMSG( hItems, LVM_GETSELECTEDCOUNT, 0, 0L ) : 0;
		size_t iItem = ( 1 == selectedCount ) ? (size_t)SNDMSG( hItems, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED ) : BAD_ITEM;
		if ( iItem < channel->items.size() )
		{
			if ( FALSE == podcast->itemAscending )
				iItem = channel->items.size() - iItem - 1;

			infoText = channel->items[ iItem ].description;
		}

		if ( NULL == infoText || L'\0' == *infoText )
			infoText = channel->description;

		if ( selectedCount > 0 )
		{
			INT iSelected = -1;
			while ( -1 != ( iSelected = SNDMSG( hItems, LVM_GETNEXTITEM, (WPARAM)iSelected, LVNI_SELECTED ) ) )
			{
				size_t t = iSelected;
				if ( t < channel->items.size() )
				{
					if ( FALSE == podcast->itemAscending )
						t = channel->items.size() - t - 1;

					LPCWSTR url = channel->items[ t ].url;
					if ( NULL != url && L'\0' != *url )
					{
						itemEnabled = TRUE;
						break;
					}
				}
			}
		}
	}

	if ( podcast->description != infoText && CSTR_EQUAL != CompareString( CSTR_INVARIANT, 0, podcast->description, -1, infoText, -1 ) )
	{
		Plugin_FreeString( podcast->description );
		podcast->description = Plugin_CopyString( infoText );
		SubscriptionView_SetDescription( hwnd, podcast->description );
	}

	PodcastChannel_EnableButtons( hwnd, ( BAD_CHANNEL != iChannel ) );
	PodcastItem_EnableButtons( hwnd, itemEnabled );
}

static BOOL SubscriptionView_SortItems(size_t iChannel, int sortColumn)
{
	if (iChannel >= channels.size())
		return FALSE;

	AutoLock lock (channels LOCKNAME("SortItems"));
	Channel &channel = channels[iChannel];

	switch (sortColumn)
	{
		case COLUMN_TITLE:		channel.SortByTitle(); return TRUE;
		case COLUMN_MEDIA_TIME:		channel.SortByMediaTime(); return TRUE;
		case COLUMN_MEDIA_SIZE:		channel.SortByMediaSize(); return TRUE;
		//case COLUMN_MEDIA:		channel.SortByMedia(); return TRUE;
		case COLUMN_DATEADDED:	channel.SortByDate(); return TRUE;			
	}

	return FALSE;
}

static BOOL SubscriptionView_SortChannels(int sortColumn)
{	
	AutoLock lock (channels LOCKNAME("SortItems"));
	
	switch (sortColumn)
	{
		case COLUMN_TITLE:	channels.SortByTitle(); return TRUE;
	}

	return FALSE;
}

static INT SubscriptionView_GetListSortColumn(HWND hwnd, INT listId, BOOL *fAscending)
{
	HWND hItems = GetDlgItem(hwnd, listId);
	if (NULL != hItems)
	{
		HWND hHeader = (HWND)SNDMSG(hItems, LVM_GETHEADER, 0, 0L);
		if (NULL != hHeader)
		{
			HDITEM item;
			item.mask = HDI_FORMAT;

			INT count = (INT)SNDMSG(hHeader, HDM_GETITEMCOUNT, 0, 0L);
			for (INT i = 0; i < count; i++)
			{
				if (FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, i, (LPARAM)&item) &&
					0 != ((HDF_SORTUP | HDF_SORTDOWN) & item.fmt))
				{
					if (NULL != fAscending)
					{
						*fAscending = (0 != (HDF_SORTUP & item.fmt));
					}
					return i;
				}
			}

		}
	}
	return -1;
}

static void SubscriptionView_SetListSortColumn(HWND hwnd, INT listId, INT index, BOOL fAscending)
{
	HWND hItems = GetDlgItem(hwnd, listId);
	if (NULL == hItems) return;
	
	HWND hHeader = (HWND)SNDMSG(hItems, LVM_GETHEADER, 0, 0L);
	if (NULL == hHeader) return;
		
	HDITEM item;
	item.mask = HDI_FORMAT;
	// reset first (ml req)
	INT count = (INT)SNDMSG(hHeader, HDM_GETITEMCOUNT, 0, 0L);
	for (INT i = 0; i < count; i++)
	{
		if (index != i && FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, i, (LPARAM)&item))
		{
			if (0 != ((HDF_SORTUP | HDF_SORTDOWN) & item.fmt))
			{	
				item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
				SNDMSG(hHeader, HDM_SETITEM, i, (LPARAM)&item);
			}
		}
	}

	if (FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, index, (LPARAM)&item))
	{
		INT fmt = item.fmt & ~(HDF_SORTUP | HDF_SORTDOWN);
		fmt |= (FALSE == fAscending) ? HDF_SORTDOWN : HDF_SORTUP;
		if (fmt != item.fmt)
		{
			item.fmt = fmt;
			SNDMSG(hHeader, HDM_SETITEM, index, (LPARAM)&item);
		}
	}
}

static BOOL SubscriptionView_UpdateInfoUrl(HWND hwnd)
{
	AutoLock lock (channels LOCKNAME("UpdateInfoUrl"));

	BOOL result  = FALSE;
	BOOL buttonEnable = FALSE;

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL != podcast) 
	{
		Plugin_FreeString(podcast->infoUrl);
		podcast->infoUrl = NULL;
		
		LPCWSTR pszUrl = NULL;

		size_t iChannel = PodcastChannel_GetActive(hwnd);
		if (BAD_CHANNEL != iChannel)
		{
			Channel *channel = &channels[iChannel];

			HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
			INT selectedCount = (NULL != hItems ) ? (INT)SNDMSG(hItems, LVM_GETSELECTEDCOUNT, 0, 0L) : 0;
			size_t iItem = (1 == selectedCount) ? (size_t)SNDMSG(hItems, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED) : BAD_ITEM;
			if (iItem < channel->items.size()) 
			{
				if (FALSE == podcast->itemAscending) 
					iItem = channel->items.size() - iItem - 1;
				pszUrl = channel->items[iItem].link;
			}

			if (NULL == pszUrl ||  L'\0' == *pszUrl)
				pszUrl = channel->link;
		}
				
		if (NULL != pszUrl && L'\0' != *pszUrl)
		{
			podcast->infoUrl = Plugin_CopyString(pszUrl);
			if (NULL != podcast->infoUrl)
			{
				buttonEnable = TRUE;
				result = TRUE;
			}
		}
	}

	HWND hButton = GetDlgItem(hwnd, IDC_VISIT);
	if (NULL != hButton) EnableWindow(hButton, buttonEnable);

	return result;
}

static INT SubscriptionView_Play(HWND hwnd, size_t iChannel, size_t *indexList, size_t indexCount, BOOL fEnqueue, BOOL fForce)
{
	AutoLock channelLock (channels LOCKNAME("Play"));
	
	if (BAD_CHANNEL == iChannel || iChannel >= channels.size()) 
		return 0;
	
	Channel *channel = &channels[iChannel];
	
	INT cchBuffer = 0;
    
	for(size_t i = 0; i < indexCount; i++)
	{
		size_t iItem = indexList[i];
		if (iItem < channel->items.size())
		{
			WCHAR szPath[MAX_PATH * 2] = {0};
			RSS::Item *downloadItem = &channel->items[iItem];
			if ((downloadItem->url && downloadItem->url[0]) &&
				SUCCEEDED(downloadItem->GetDownloadFileName(channel->title, szPath, ARRAYSIZE(szPath), TRUE)) &&
				PathFileExists(szPath))
			{
				cchBuffer += (lstrlen(szPath) + 1);
			}
			else
			{
				wchar_t* url = urlencode(channel->items[iItem].url);
				LPCWSTR p = url;
				if (NULL != p)
				{
					cchBuffer += (lstrlen(p) + 1);
					free(url);
				}
			}
		}
	}

	if (0 == cchBuffer) 
		return 0;

	cchBuffer++;
	LPWSTR pszBuffer = (LPWSTR)calloc(cchBuffer, sizeof(WCHAR));
	if (NULL == pszBuffer) return 0;

	LPWSTR c = pszBuffer;
	size_t r = cchBuffer;

	INT playCount = 0;
	for(size_t i = 0; i < indexCount; i++)
	{		
		size_t iItem = indexList[i];
		if (iItem < channel->items.size())
		{
			WCHAR szPath[MAX_PATH * 2] = {0};
			RSS::Item *downloadItem = &channel->items[iItem];
			if ((downloadItem->url && downloadItem->url[0]) &&
				SUCCEEDED(downloadItem->GetDownloadFileName(channel->title, szPath, ARRAYSIZE(szPath), TRUE)) &&
				PathFileExists(szPath))
			{
				LPCWSTR p = szPath;
				if (NULL != p && L'\0' != *p)
				{
					if (FAILED(StringCchCopyExW(c, r, p, &c, &r, STRSAFE_NULL_ON_FAILURE)) || 0 == r)
						break;
					
					c++;
					r--;

					channel->items[iItem].listened = true;
					playCount++;
				}
			}
			else
			{
				wchar_t* url = urlencode(channel->items[iItem].url);
				LPCWSTR p = url;
				if (NULL != p && L'\0' != *p)
				{
					if (FAILED(StringCchCopyExW(c, r, p, &c, &r, STRSAFE_NULL_ON_FAILURE)) || 0 == r)
					{
						free(url);
						break;
					}
					free(url);
					c++;
					r--;

					channel->items[iItem].listened = true;
					playCount++;
				}
			}
		}
	}
	
	if (c != pszBuffer)
	{
		*c = L'\0';
		// make sure this is initialised as default handler requires this being zeroed
		mlSendToWinampStruct send = {ML_TYPE_STREAMNAMESW,pszBuffer,0};
		// otherwise we've a specific action and need to tell ML to do as we want
		if (TRUE == fForce)
			send.enqueue = ((FALSE == fEnqueue) ? 0 : 1) | 2;
		SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SENDTOWINAMP, (WPARAM)&send);
	}

	free(pszBuffer);
	return playCount;
}

static INT SubscriptionView_PlayChannel(HWND hwnd, size_t iChannel, BOOL fEnqueue, BOOL fForce)
{
	AutoLock channelLock (channels LOCKNAME("PlayChannel"));
	
	if (BAD_CHANNEL == iChannel || iChannel >= channels.size()) 
		return 0;

	size_t count = channels.size();
	size_t *list = NULL;
	if (0 != count) 
	{
		list = (size_t*)calloc(count, sizeof(size_t));
		if (NULL == list) return 0;
		for (size_t i = 0; i < count; i++) list[i] = i;
	}

	INT result = SubscriptionView_Play(hwnd, iChannel, list, count, fEnqueue, fForce);

	if (NULL != list) 
		free(list);

	return result;
}

static void PodcastItem_InitializeList(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL == hControl)
		return;

	UINT styleEx = (UINT)GetWindowLongPtr(hControl, GWL_EXSTYLE);
	SetWindowLongPtr(hControl, GWL_EXSTYLE, styleEx & ~WS_EX_NOPARENTNOTIFY);

	styleEx = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | /*LVS_EX_HEADERDRAGDROP |*/ LVS_EX_INFOTIP | LVS_EX_SUBITEMIMAGES;
	SendMessage(hControl, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx, styleEx);
	SendMessage(hControl, LVM_SETUNICODEFORMAT, (WPARAM)TRUE, 0L);

	LVCOLUMN lvc = {0};
	WCHAR szBuffer[128] = {0};
	
	lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.pszText = szBuffer;

	lvc.fmt     = LVCFMT_LEFT;
	lvc.cx      = itemTitleWidth;
	WASABI_API_LNGSTRINGW_BUF( IDS_ITEM, szBuffer, ARRAYSIZE( szBuffer ) );
	SNDMSG(hControl, LVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)&lvc);

	lvc.fmt     = LVCFMT_RIGHT;
	lvc.cx      = itemDateWidth;
	WASABI_API_LNGSTRINGW_BUF( IDS_DATE_ADDED, szBuffer, ARRAYSIZE( szBuffer ) );
	SNDMSG(hControl, LVM_INSERTCOLUMN, (WPARAM)1, (LPARAM)&lvc);

	lvc.fmt     = LVCFMT_RIGHT;
	lvc.cx      = itemMediaWidth;
	WASABI_API_LNGSTRINGW_BUF( IDS_MEDIA_TIME, szBuffer, ARRAYSIZE( szBuffer ) );
	SNDMSG(hControl, LVM_INSERTCOLUMN, (WPARAM)2, (LPARAM)&lvc);

	lvc.fmt     = LVCFMT_RIGHT;
	lvc.cx      = itemSizeWidth;
	WASABI_API_LNGSTRINGW_BUF( IDS_MEDIA_SIZE, szBuffer, ARRAYSIZE( szBuffer ) );
	SNDMSG(hControl, LVM_INSERTCOLUMN, (WPARAM)3, (LPARAM)&lvc);

	HIMAGELIST imageList = ImageList_Create( 15, 15, ILC_COLOR24, 3, 0 );
	if ( imageList != NULL )
	{
		HIMAGELIST prevList = (HIMAGELIST)SNDMSG( hControl, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)imageList );
		if ( prevList != NULL )
			ImageList_Destroy( prevList );
	}

	MLSKINWINDOW skinWindow;
	skinWindow.style      = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_ALTERNATEITEMS;
	skinWindow.skinType   = SKINNEDWND_TYPE_LISTVIEW;
	skinWindow.hwndToSkin = hControl;

	MLSkinWindow( plugin.hwndLibraryParent, &skinWindow );
}

static void PodcastItem_SelectionChanged(HWND hwnd, BOOL fImmediate)
{
	KillTimer(hwnd, UPDATE_TIMER);

	if (FALSE == fImmediate)
	{
		SetTimer(hwnd, UPDATE_TIMER, UPDATE_DELAY, PodcastItem_UpdateTimer);
		return;
	}

	SubscriptionView_UpdateInfoUrl(hwnd);
	SubscriptionView_UpdateInfo(hwnd);
}

static HMENU PodcastItem_GetMenu(HWND hwnd, HMENU baseMenu, INT iItem)
{
	HMENU menu = GetSubMenu(baseMenu, 1);
	if (NULL == menu)
		return NULL;

	PodcastItem_SelectionChanged(hwnd, TRUE);

	// update the explore media menu item from the download button state (hence the two IDC_DOWNLOAD)
	const INT szExtras[] = { IDC_PLAY, IDC_ENQUEUE, IDC_DOWNLOAD, IDC_DOWNLOAD, IDC_VISIT, };
			
	for (INT i = 0; i < ARRAYSIZE(szExtras); i++)
	{
		HWND hButton = GetDlgItem(hwnd, szExtras[i]);

		UINT uEnable = MF_BYCOMMAND | MF_ENABLED;
		if (NULL == hButton || (-1 == iItem) || 0 != (WS_DISABLED & GetWindowLongPtr(hButton, GWL_STYLE)))
			uEnable |= (MF_GRAYED | MF_DISABLED);

		EnableMenuItem(menu, (i == 3 ? IDC_EXPLORERITEMFOLDER : szExtras[i]), uEnable);
	}

	EnableMenuItem(menu, 2, MF_BYPOSITION | ((-1 == iItem) ? (MF_GRAYED | MF_DISABLED) : MF_ENABLED));

	{ // send-to menu shit...
		ZeroMemory(&s, sizeof(s));
		IPC_LIBRARY_SENDTOMENU = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
		if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)0, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
		{
			s.mode = 1;
			s.hwnd = hwnd;
			s.data_type = ML_TYPE_FILENAMESW;
			s.ctx[1] = 1;
			s.build_hMenu = GetSubMenu(menu, 2);
		}
	}

	UpdateMenuItems(hwnd, menu);

	// check if the menu itsm is shown as having been download and
	// if it hasn't then is nicer to hide 'explore media folder'
	{
		AutoLock channelLock (channels LOCKNAME("ItemMenu"));

		PODCAST *podcast = GetPodcast(hwnd);
		if (NULL != podcast)
		{
			size_t iChannel = PodcastChannel_GetActive(hwnd);
			if (BAD_CHANNEL != iChannel)
			{
				Channel *channel = &channels[iChannel];
				
				if (iItem < (INT)channel->items.size())
				{
					if (FALSE == podcast->itemAscending)
						iItem = (INT)channel->items.size() - iItem - 1;
					
					RSS::Item *downloadItem = &channel->items[iItem];
					if(downloadItem->downloaded == false)
					{
						DeleteMenu(menu, IDC_EXPLORERITEMFOLDER, MF_BYCOMMAND);
					}
				}
			}
		}
	}

	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	INT iCount = (NULL != hItems) ? (INT)SNDMSG(hItems, LVM_GETITEMCOUNT, 0, 0L) : 0;
	INT command = IDC_REFRESH;
	SubscriptionView_EnableMenuCommands(menu, &command, 1, (0 != iCount));

	return menu;
}

static BOOL PodcastItem_Sort(HWND hwnd, INT iColumn, BOOL fAscending)
{
	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return FALSE;

	BOOL result = FALSE;
	size_t iChannel = PodcastChannel_GetActive(hwnd);
	if (BAD_CHANNEL != iChannel) 
	{
		result = SubscriptionView_SortItems(iChannel, iColumn);
	}
	
	podcast->itemAscending = fAscending;
	SubscriptionView_SetListSortColumn(hwnd, IDC_ITEMLIST, iColumn, fAscending);
		
	if (FALSE != result)
	{
		HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
		if (NULL != hItems) 
			InvalidateRect(hItems, NULL, TRUE);
	}
	
	return TRUE;
}

static size_t PodcastChannel_GetActive(HWND hwnd)
{
	AutoLock lock (channels LOCKNAME("GetActive"));

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL != podcast)
	{
		size_t iChannel = podcast->channelActive;
		if (iChannel < channels.size()) 
		{
			return (size_t)iChannel;
		}
	}
	return BAD_CHANNEL;
}
static void PodcastInfo_InitializeList(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_EPISODE_INFO);
	if ( hControl == NULL )
		return;

	MLSKINWINDOW skinWindow;
	skinWindow.style      = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_ALTERNATEITEMS;
	skinWindow.skinType   = SKINNEDWND_TYPE_LISTVIEW;
	skinWindow.hwndToSkin = hControl;
	MLSkinWindow(plugin.hwndLibraryParent, &skinWindow);

	MLSkinnedScrollWnd_ShowHorzBar(hControl, FALSE);
	MLSkinnedScrollWnd_ShowVertBar(hControl, FALSE);

	UINT styleEx;
	
	styleEx = LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT;
	SendMessage(hControl, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx,     styleEx);
	SendMessage(hControl, LVM_SETUNICODEFORMAT,        (WPARAM)TRUE, 0L);

	LVCOLUMN lvc           = {0};
	WCHAR    szBuffer[128] = {0};

	lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt     = LVCFMT_LEFT;
	lvc.pszText = szBuffer;
	lvc.cx      = 9999;

	WASABI_API_LNGSTRINGW_BUF(IDS_EPISODE_INFO, szBuffer, ARRAYSIZE(szBuffer));
	SNDMSG(hControl, LVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)&lvc);

	HWND hHeader = ListView_GetHeader(hControl);

	RECT rect;
	GetClientRect(hHeader, &rect);
	SetWindowPos(hControl, 0, 0, 0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE|SWP_NOZORDER);
}

static void PodcastChannel_InitializeList(HWND hwnd)
{
	HWND  hControl = GetDlgItem(hwnd, IDC_CHANNELLIST);
	if (NULL == hControl)
		return;

	MLSKINWINDOW skinWindow = {0};
	skinWindow.style      = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_ALTERNATEITEMS;
	skinWindow.skinType   = SKINNEDWND_TYPE_LISTVIEW;
	skinWindow.hwndToSkin = hControl;

	MLSkinWindow(plugin.hwndLibraryParent, &skinWindow);

	UINT skinStyle = MLSkinnedWnd_GetStyle(hControl);
	skinStyle |= SWLVS_SELALWAYS;
	MLSkinnedWnd_SetStyle(hControl, skinStyle);
	MLSkinnedScrollWnd_ShowHorzBar(hControl, FALSE);

	UINT styleEx = LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT;
	SendMessage(hControl, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx, styleEx);
	SendMessage(hControl, LVM_SETUNICODEFORMAT, (WPARAM)TRUE, 0L);

	LVCOLUMN lvc = {0};
	WCHAR szBuffer[128] = {0};

	lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt     = LVCFMT_LEFT;
	lvc.pszText = szBuffer;
	lvc.cx      = 200;

	WASABI_API_LNGSTRINGW_BUF(IDS_CHANNEL, szBuffer, ARRAYSIZE(szBuffer));
	SNDMSG(hControl, LVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)&lvc);
}

static void PodcastChannel_SelectionChanged(HWND hwnd, BOOL fImmediate)
{
	KillTimer(hwnd, UPDATE_TIMER);

	if (FALSE == fImmediate)
	{
		SetTimer(hwnd, UPDATE_TIMER, UPDATE_DELAY, PodcastChannel_UpdateTimer);
		return;
	}

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast)
		return;

	HWND hChannel = GetDlgItem(hwnd, IDC_CHANNELLIST);
	INT iSelected = (NULL != hChannel) ? (INT)SNDMSG(hChannel, LVM_GETNEXTITEM, -1, LVNI_SELECTED) : -1;
	
	AutoLock channelLock (channels LOCKNAME("Channel Changed"));
	
	if (-1 != iSelected && ((size_t)iSelected) > channels.size())
		iSelected = -1;
	
	if (-1 != iSelected && FALSE == podcast->channelAscending)
		iSelected = (INT)channels.size() - iSelected - 1;
	
	podcast->channelActive = iSelected;

	if (-1 != iSelected)
	{		
		INT iSort = SubscriptionView_GetListSortColumn(hwnd, IDC_ITEMLIST, NULL);
		SubscriptionView_SortItems(iSelected, iSort);
	}
		
	size_t itemsCount = (-1 != iSelected) ? channels[iSelected].items.size() : 0;

	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL != hItems)
	{	
		SNDMSG(hItems, WM_SETREDRAW, FALSE, 0L);

		LVITEM lvi;
		lvi.state = 0;
		lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		SNDMSG(hItems, LVM_SETSELECTIONMARK, 0, (LPARAM)-1);
		SNDMSG(hItems, LVM_SETITEMSTATE, -1, (LPARAM)&lvi);
		SNDMSG(hItems, LVM_SETITEMCOUNT, (WPARAM)itemsCount, 0L);

		SNDMSG(hItems, WM_SETREDRAW, TRUE, 0L);
	}

	SubscriptionView_UpdateInfoUrl(hwnd);
	SubscriptionView_UpdateInfo(hwnd);
	channelLastSelection = iSelected;
}

static HMENU PodcastChannel_GetMenu(HWND hwnd, HMENU baseMenu, INT iItem)
{
	HMENU menu = GetSubMenu(baseMenu, 0);
	if (NULL == menu) return NULL;

	if (iItem != -1) PodcastChannel_SelectionChanged(hwnd, TRUE);

	if (iItem != -1)
	{
		DeleteMenu(menu, IDC_ADD, MF_BYCOMMAND);
		const INT szExtras[] = { IDC_REFRESH, IDC_EDIT, IDC_DELETE, };
		SubscriptionView_EnableMenuCommands(menu, szExtras, ARRAYSIZE(szExtras), (-1 != iItem));
	}
	else
	{
		DeleteMenu(menu, IDC_REFRESH, MF_BYCOMMAND);
		DeleteMenu(menu, IDC_EDIT, MF_BYCOMMAND);
		DeleteMenu(menu, IDC_DELETE, MF_BYCOMMAND);
		DeleteMenu(menu, IDC_VISIT, MF_BYCOMMAND);
		DeleteMenu(menu, 0, MF_BYPOSITION);
		DeleteMenu(menu, 0, MF_BYPOSITION);
	}

	return menu;
}

static BOOL PodcastChannel_SyncHeaderSize(HWND hwnd, BOOL fRedraw)
{
	HWND hChannel = GetDlgItem(hwnd, IDC_CHANNELLIST);
	if (NULL == hChannel) return FALSE;

	RECT channelRect;
	
	HDITEM item;
	item.mask = HDI_WIDTH;

	HWND hHeader = (HWND)SNDMSG(hChannel, LVM_GETHEADER, 0, 0L);
	if (NULL == hHeader ||
		FALSE == GetClientRect(hChannel, &channelRect) ||
		FALSE == SNDMSG(hHeader, HDM_GETITEM, COLUMN_TITLE, (LPARAM)&item))
	{
		return FALSE;
	}

	LONG columnWidth = channelRect.right - channelRect.left;
	
	
	if (item.cxy == columnWidth) return TRUE;
	
	item.cxy = columnWidth;
	
	UINT windowStyle = GetWindowLongPtr(hHeader, GWL_STYLE);
	if (FALSE == fRedraw && 0 != (WS_VISIBLE & windowStyle)) 
		SetWindowLongPtr(hHeader, GWL_STYLE, windowStyle & ~WS_VISIBLE);
	
	SNDMSG(hHeader, HDM_SETITEM, COLUMN_TITLE, (LPARAM)&item);

	if (FALSE == fRedraw && 0 != (WS_VISIBLE & windowStyle)) 
		SetWindowLongPtr(hHeader, GWL_STYLE, windowStyle);

	return TRUE;
}

static BOOL PodcastChannel_Sort(HWND hwnd, INT iColumn, BOOL fAscending)
{
	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return FALSE;

	BOOL result = SubscriptionView_SortChannels(iColumn);
	
	podcast->channelAscending = fAscending;
	SubscriptionView_SetListSortColumn(hwnd, IDC_CHANNELLIST, iColumn, fAscending);
		
	if (FALSE != result)
	{
		HWND hItems = GetDlgItem(hwnd, IDC_CHANNELLIST);
		if (NULL != hItems) 
			InvalidateRect(hItems, NULL, TRUE);
	}
	
	return result;
}

static BOOL PodcastChannel_SelectNext(HWND hwnd, BOOL fForward)
{
	HWND hChannel = GetDlgItem(hwnd, IDC_CHANNELLIST);
	if (NULL == hChannel) return FALSE;

	INT iFocus = (INT)SNDMSG(hChannel, LVM_GETNEXTITEM, -1, (LPARAM)LVNI_FOCUSED);
	
	if (FALSE != fForward) 
	{
		iFocus++;
		INT iCount = (INT)SNDMSG(hChannel, LVM_GETITEMCOUNT, 0, 0L);
		if (iFocus >= iCount) return TRUE;
	}
	else
	{
		if (iFocus <= 0) return TRUE;
		iFocus--;
	}
	
	LVITEM lvi = {0};
	lvi.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
	SNDMSG(hChannel, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
	
	lvi.state = lvi.stateMask;
	if (FALSE != SNDMSG(hChannel, LVM_SETITEMSTATE, (WPARAM)iFocus, (LPARAM)&lvi))
	{
		SNDMSG(hChannel, LVM_ENSUREVISIBLE, (WPARAM)iFocus, (LPARAM)FALSE);
		MLSkinnedScrollWnd_UpdateBars(hChannel, TRUE);
	}

	return TRUE;
}
static void CALLBACK PodcastChannel_UpdateTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, ULONG elapsed)
{
	KillTimer(hwnd, eventId);
	PodcastChannel_SelectionChanged(hwnd, TRUE);
}

static void CALLBACK PodcastItem_UpdateTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, ULONG elapsed)
{
	KillTimer(hwnd, eventId);
	PodcastItem_SelectionChanged(hwnd, TRUE);
}

wchar_t* PodcastCommand_OnSendToSelection(HWND hwnd)
{
	AutoLock channelLock (channels LOCKNAME("SendTo"));

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return 0;

	size_t iChannel = PodcastChannel_GetActive(hwnd);
	if (BAD_CHANNEL == iChannel) return 0;
	Channel *channel = &channels[iChannel];

	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL == hItems) return 0;

	INT selectedCount = (INT)SNDMSG(hItems, LVM_GETSELECTEDCOUNT, 0, 0L);
	if (0 == selectedCount) return 0;

	INT iSelected = -1;
	WCHAR szPath[MAX_PATH * 2] = {0}, *path = NULL;
	int buf_pos = 0, buf_size = 0;

	while(-1 != (iSelected = SNDMSG(hItems, LVM_GETNEXTITEM, (WPARAM)iSelected, LVNI_SELECTED)))
	{
		size_t iItem = iSelected;
		if (iItem < channel->items.size())
		{
			if (FALSE == podcast->itemAscending)
				iItem = channel->items.size() - iItem - 1;

			RSS::Item *downloadItem = &channel->items[iItem];
			if (!((downloadItem->url && downloadItem->url[0]) &&
				SUCCEEDED(downloadItem->GetDownloadFileName(channel->title, szPath, ARRAYSIZE(szPath), TRUE)) &&
				PathFileExists(szPath)))
			{
				lstrcpyn(szPath, downloadItem->url, ARRAYSIZE(szPath));
			}

			listbuild(&path, buf_size, buf_pos, szPath);
		}
	}

	if (path) path[buf_pos] = 0;
	return path;
}

static void SubscriptionView_ListContextMenu(HWND hwnd, INT controlId, POINTS pts)
{
	HWND hControl = GetDlgItem(hwnd, controlId);
	if (NULL == hControl) return;

	POINT pt;
	POINTSTOPOINT(pt, pts);

	RECT controlRect, headerRect;
	if (FALSE == GetClientRect(hControl, &controlRect))
		SetRectEmpty(&controlRect);
	else
		MapWindowPoints(hControl, HWND_DESKTOP, (POINT*)&controlRect, 2);

	HWND hHeader = (HWND)SNDMSG(hControl, LVM_GETHEADER, 0, 0L);
	if (0 == (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) || FALSE == GetWindowRect(hHeader, &headerRect))
	{
		SetRectEmpty(&headerRect);
	}

	if (-1 == pt.x && -1 == pt.y)
	{
		RECT rect;

		rect.left = LVIR_BOUNDS;
		INT iMark = SNDMSG(hControl, LVM_GETNEXTITEM, -1, LVNI_SELECTED | LVNI_FOCUSED);
		if (-1 == iMark) iMark = SNDMSG(hControl, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
		if (-1 != iMark && FALSE != SNDMSG(hControl, LVM_GETITEMRECT, (WPARAM)iMark, (LPARAM)&rect))
		{
			pt.x = rect.left + 4;
			if(pt.x > rect.right) pt.x = rect.right;
			pt.y = rect.bottom - (rect.bottom - rect.top)/3;

			MapWindowPoints(hControl, HWND_DESKTOP, &pt, 1);
		}
	}

	INT iItem = -1;
	
	if ((-1 != pt.x || -1 != pt.y) && FALSE != PtInRect(&controlRect, pt))
	{
		if (FALSE != PtInRect(&headerRect, pt))
		{
			return; 
		}
		else
		{
			LVHITTESTINFO hitTest;
			hitTest.pt = pt;
			MapWindowPoints(HWND_DESKTOP, hControl, &hitTest.pt, 1);

			iItem =	(INT)SNDMSG(hControl, LVM_HITTEST, 0, (LPARAM)&hitTest);
			if (0 == (LVHT_ONITEM & hitTest.flags)) iItem = -1;
		}
	}
	else
	{
		pt.x = controlRect.left + 2;
		pt.y = controlRect.top + 2;
		if (headerRect.top == controlRect.top && headerRect.bottom > controlRect.top)
			pt.y = headerRect.bottom + 2;
	}

	HMENU baseMenu = WASABI_API_LOADMENU(IDR_MENU1);
	if (NULL == baseMenu) return;

	HMENU menu = NULL;
	switch(controlId)
	{
		case IDC_ITEMLIST:		menu = PodcastItem_GetMenu(hwnd, baseMenu, iItem); break;
		case IDC_CHANNELLIST:	menu = PodcastChannel_GetMenu(hwnd, baseMenu, iItem); break;
	}

	if (NULL != menu)
	{
		int r = Menu_TrackPopup(plugin.hwndLibraryParent, menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | 0x1000/*TPM_VERPOSANIMATION*/, pt.x, pt.y, hwnd, NULL);
		if (!SendMessage(hwnd, WM_COMMAND, r, 0))
		{
			s.menu_id = r;
			if (s.mode == 2 && SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
			{
				s.mode = 3;

				wchar_t* path = PodcastCommand_OnSendToSelection(hwnd);
				if (path && *path)
				{
					s.data = path;
					SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s, IPC_LIBRARY_SENDTOMENU);
					free(path);
				}
			}
		}

		if (s.mode)
		{
			s.mode=4;
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU); // cleanup
		}
	}

	DestroyMenu(baseMenu);
}

static void SubscriptionView_UpdateLayout(HWND hwnd, BOOL fRedraw, UINT uReason, HRGN validRegion, POINTS layoutOffset)
{
	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	RECT clientRect;
	if (FALSE == GetClientRect(hwnd, &clientRect)) return;

	LONG clientWidth = clientRect.right - clientRect.left - WASABI_API_APP->getScaleX(2);
	LONG clientHeight = clientRect.bottom - clientRect.top;

	const INT szItems[LI_LAST] = {  IDC_CHANNELLIST, IDC_ITEMLIST, IDC_VDIV, 
									IDC_FINDNEW, IDC_ADD, IDC_EDIT, IDC_DELETE, IDC_REFRESH, 
									IDC_HDIV, 
                                    IDC_EPISODE_INFO, IDC_DESCRIPTION, 
                                    IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM, IDC_DOWNLOAD, IDC_VISIT, IDC_STATUS};

	LAYOUTITEM *layout = (LAYOUTITEM*)calloc(ARRAYSIZE(szItems), sizeof(LAYOUTITEM));
	if (NULL == layout) return;

	Layout_Initialize(hwnd, szItems, ARRAYSIZE(szItems), layout);

	if (episode_info_cy == 0 || (layout[LI_EPISODE_INFO].cy && episode_info_cy != layout[LI_EPISODE_INFO].cy))
	{
		HWND hControl = GetDlgItem(hwnd, IDC_EPISODE_INFO);
		if (IsWindow(hControl))
		{
			HWND hHeader = ListView_GetHeader(hControl);
			if (IsWindow(hHeader))
			{
				RECT rect;
				GetClientRect(hHeader, &rect);
				SetWindowPos(hControl, 0, 0, 0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE|SWP_NOZORDER);
				episode_info_cy = rect.bottom-rect.top;
			}
			else
				episode_info_cy = layout[LI_EPISODE_INFO].cy;
		}
		else
			episode_info_cy = layout[LI_EPISODE_INFO].cy;
	}

	LONG t = (clientWidth - layout[LI_VERT].cx) * podcast->vertDivider / 10000;
	LONG t2 = clientRect.right - layout[LI_VERT].cx;

	if (LAYOUTREASON_DIV_LEFT == uReason && t < clientRect.left + WASABI_API_APP->getScaleX(36))
		t = clientRect.left;

	if (LAYOUTREASON_DIV_RIGHT == uReason && t > (t2 - WASABI_API_APP->getScaleX(36)))
		t = t2;

	if (t < clientRect.left + WASABI_API_APP->getScaleX(36)) t = clientRect.left;
	else if (t > t2 - WASABI_API_APP->getScaleX(36)) t = t2;

	layout[LI_CHANNEL].x = WASABI_API_APP->getScaleX(1);
	layout[LI_CHANNEL].y = WASABI_API_APP->getScaleX(1);
	layout[LI_CHANNEL].cx = t;
	layout[LI_VERT].x = LI_GET_R(layout[LI_CHANNEL]);
	layout[LI_VERT].y = WASABI_API_APP->getScaleX(1);
	layout[LI_ITEM].x = LI_GET_R(layout[LI_VERT]);
	layout[LI_ITEM].y = WASABI_API_APP->getScaleX(1);
	LI_SET_R(layout[LI_ITEM], clientWidth);

	layout[LI_HORZ].cx = clientWidth - layout[LI_HORZ].x*WASABI_API_APP->getScaleX(2);
	layout[LI_EPISODE_INFO].x = WASABI_API_APP->getScaleX(1);
	layout[LI_EPISODE_INFO].cx = clientWidth - layout[LI_EPISODE_INFO].x;
	layout[LI_INFO].cx = clientWidth - layout[LI_INFO].x*WASABI_API_APP->getScaleX(2) - 1;
	LI_SET_R(layout[LI_STATUS], clientWidth);
	if (layout[LI_STATUS].cx < WASABI_API_APP->getScaleX(20)) layout[LI_STATUS].cx = 0;

	t = clientHeight - layout[LI_PLAY].cy;
	int cum_width = 0, inc = 0, btnY = 0;
	const INT szBottomRow[] = { LI_PLAY, LI_ENQUEUE, LI_CUSTOM, LI_DOWNLOAD, LI_VISIT, LI_STATUS, };
	for (INT i = 0; i < ARRAYSIZE(szBottomRow); i++)
	{
		LAYOUTITEM *p = &layout[szBottomRow[i]];
		if (IsWindow(p->hwnd))
		{
			if (!inc) p->x = 1;
			else p->x = 1 + cum_width + WASABI_API_APP->getScaleX(4)*inc;

			wchar_t buffer[128] = {0};
			GetWindowTextW(p->hwnd, buffer, ARRAYSIZE(buffer));
			LRESULT idealSize = MLSkinnedButton_GetIdealSize(p->hwnd, buffer);

			btnY = p->cy = WASABI_API_APP->getScaleY(HIWORD(idealSize));
			p->y = clientHeight - p->cy;

			if (szBottomRow[i] != LI_STATUS)	// exclude the last one so it'll span the remainder of the space
			{
				if (LI_CUSTOM != szBottomRow[i] || customAllowed)
				{
					if (groupBtn && szBottomRow[i] == LI_PLAY && enqueuedef == 1)
					{
						p->flags |= SWP_HIDEWINDOW;
						p->cy = 0;
						continue;
					}

					if (groupBtn && szBottomRow[i] == LI_ENQUEUE && enqueuedef != 1)
					{
						p->flags |= SWP_HIDEWINDOW;
						p->cy = 0;
						continue;
					}

					if (groupBtn && (szBottomRow[i] == LI_PLAY || szBottomRow[i] == LI_ENQUEUE) && customAllowed)
					{
						p->flags |= SWP_HIDEWINDOW;
						p->cy = 0;
						continue;
					}

					LONG width = LOWORD(idealSize) + WASABI_API_APP->getScaleX(6);
					p->cx = width;
					p->cy = WASABI_API_APP->getScaleY(HIWORD(idealSize));
					cum_width += width;
					inc++;
				}
				else
				{
					p->flags |= SWP_HIDEWINDOW;
					p->cy = 0;
					continue;
				}
			}
			else
			{
				LRESULT idealSize = MLSkinnedStatic_GetIdealSize(p->hwnd, buffer);
				btnY = p->cy = WASABI_API_APP->getScaleY(HIWORD(idealSize));
				p->y = clientHeight - p->cy;
				p->cx = clientWidth - cum_width - (WASABI_API_APP->getScaleX(6) * 2);
			}
		}
	}

	t = (clientHeight - layout[LI_HORZ].cy) * podcast->horzDivider / 10000;
	t2 = layout[LI_PLAY].y - layout[LI_HORZ].cy;

	if (LAYOUTREASON_DIV_TOP == uReason && t < clientRect.top + WASABI_API_APP->getScaleY(36 + btnY))
		t = clientRect.top;

	if (LAYOUTREASON_DIV_BOTTOM == uReason && t > (t2 - WASABI_API_APP->getScaleY(36 + btnY)))
		t = t2;

	if (t < clientRect.top + WASABI_API_APP->getScaleY(36 + btnY)) t = clientRect.top;
	else if (t > t2 - WASABI_API_APP->getScaleY(36 + btnY)) t = t2;

	cum_width = 0;
	const INT szMiddleRow[] = { LI_FINDNEW, LI_ADD, LI_EDIT, LI_DELETE, LI_REFRESH, };
	for (INT i = 0; i < ARRAYSIZE(szMiddleRow); i++)
	{
		LAYOUTITEM *p = &layout[szMiddleRow[i]];
		p->x = 1 + (i ? cum_width + WASABI_API_APP->getScaleX(4)*i : 0);

		wchar_t buffer[128] = {0};
		GetWindowTextW(p->hwnd, buffer, ARRAYSIZE(buffer));
		LRESULT idealSize = MLSkinnedButton_GetIdealSize(p->hwnd, buffer);
		// used to hide the 'directory' button if not using one
		LONG width = (!i && !serviceUrl[0] ? WASABI_API_APP->getScaleX(-4) : LOWORD(idealSize) + WASABI_API_APP->getScaleX(6));
		p->cx = width;
		cum_width += width;

		p->cy = (t < WASABI_API_APP->getScaleY(48)) ? 0 : WASABI_API_APP->getScaleY(HIWORD(idealSize));
		p->y = t - p->cy;
	}

	layout[LI_HORZ].y = LI_GET_B(layout[LI_FINDNEW]);
	layout[LI_EPISODE_INFO].y = LI_GET_B(layout[LI_HORZ]);
	if (clientRect.bottom - layout[LI_EPISODE_INFO].y < WASABI_API_APP->getScaleY(episode_info_cy))
		layout[LI_EPISODE_INFO].cy = 0;
	else
		layout[LI_EPISODE_INFO].cy = episode_info_cy;

	layout[LI_INFO].y = LI_GET_B(layout[LI_EPISODE_INFO]);
	LI_SET_B(layout[LI_INFO], layout[LI_PLAY].y - WASABI_API_APP->getScaleY(4));

	t = layout[LI_FINDNEW].y;
	if (layout[LI_FINDNEW].cy > 0) t -= podcast->middleRowSpace;

	layout[LI_CHANNEL].cy = t;
	layout[LI_VERT].cy = t;
	layout[LI_ITEM].cy = t;
	layout[LI_INFO].flags |= SWP_NOREDRAW;

	Layout_SetVisibility(&clientRect, layout, ARRAYSIZE(szItems));
	Layout_Perform(hwnd, layout, ARRAYSIZE(szItems), fRedraw);
	PodcastChannel_SyncHeaderSize(hwnd, fRedraw);

	if (NULL != validRegion)
	{
		RECT validRect;
		CopyRect(&validRect, &clientRect);
		validRect.right += (layout[LI_INFO].rect.right - LI_GET_R(layout[LI_INFO]));
		validRect.bottom += (layout[LI_PLAY].rect.bottom - LI_GET_B(layout[LI_PLAY]));
		Layout_GetValidRgn(validRegion, layoutOffset, &validRect, layout, ARRAYSIZE(szItems));
	}

	free(layout);
}

static BOOL SubscriptionView_UpdateImageList(HIMAGELIST imageList, COLORREF rgbBk, COLORREF rgbFg)
{
	if(NULL == imageList) 
		return FALSE;

	INT imageCount = ImageList_GetImageCount(imageList);

	MLIMAGESOURCE source;
	ZeroMemory(&source, sizeof(source));
	source.cbSize = sizeof(source);
	source.hInst  = plugin.hDllInstance;
	source.type   = SRC_TYPE_PNG;
	
	MLIMAGEFILTERAPPLYEX filter;
	ZeroMemory(&filter, sizeof(filter));
	filter.cbSize    = sizeof(filter);
	filter.filterUID = MLIF_FILTER3_UID;
	filter.rgbBk     = rgbBk;
	filter.rgbFg     = rgbFg;

	const INT szImages[] = { IDR_TEXT_ICON, IDR_DOWNLOAD_ICON, IDR_MEDIA_ICON, IDR_MEDIA_ICON, };

	for (INT i = 0; i < ARRAYSIZE(szImages); i++)
	{
		source.lpszName = MAKEINTRESOURCE(szImages[i]);
		HBITMAP bitmap = MLImageLoader_LoadDib(plugin.hwndLibraryParent, &source);
		if (NULL != bitmap)
		{
			DIBSECTION dib;
			if (sizeof(dib) == GetObject(bitmap, sizeof(dib), &dib))
			{
				filter.pData = (BYTE*)dib.dsBm.bmBits;
				filter.bpp   = dib.dsBm.bmBitsPixel;
				filter.cx    = dib.dsBm.bmWidth;
				filter.cy    = dib.dsBm.bmHeight;

				if (filter.cy < 0)
					filter.cy = -filter.cy;
				
				if (MLImageFilter_ApplyEx(plugin.hwndLibraryParent, &filter))
				{
					INT result = (i < imageCount) ? 
								ImageList_Replace(imageList, i, bitmap, NULL) :
								ImageList_Add(imageList, bitmap, NULL);
				}
			}

			DeleteObject(bitmap);
		}
	}

	return TRUE;
}

static void SubscriptionView_UpdateSkin(HWND hwnd)
{
	UINT windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

	HWND hControl;
	COLORREF rgbBk = dialogSkinner.Color(WADLG_ITEMBG);
	COLORREF rgbText = dialogSkinner.Color(WADLG_ITEMFG);
	HFONT hFont = dialogSkinner.GetFont();

	episode_info_cy = 0;

	static const INT szLists[] = {IDC_CHANNELLIST, IDC_ITEMLIST, IDC_EPISODE_INFO};
	for (INT i = 0; i < ARRAYSIZE(szLists); i++)
	{
		if (NULL != (hControl = GetDlgItem(hwnd, szLists[i])))
		{		
			ListView_SetBkColor(hControl, rgbBk);
			ListView_SetTextBkColor(hControl, rgbBk);
			ListView_SetTextColor(hControl, rgbText);
			if (NULL != hFont) 
			{
				SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, 0L); 
			}

			HIMAGELIST imageList = (HIMAGELIST)SendMessage(hControl, LVM_GETIMAGELIST, LVSIL_SMALL, 0L);
			if (NULL != imageList)
			{
				SubscriptionView_UpdateImageList(imageList, rgbBk, rgbText);		
			}
		}
	}

	if (0 != (WS_VISIBLE & windowStyle))
	{
		windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (0 == (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);

		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
		SubscriptionView_UpdateInfo(hwnd);
	}

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	SubscriptionView_SetDescription(hwnd, podcast->description);	

	HRGN validRegion = (NULL != podcast->updateRegion) ? CreateRectRgn(0, 0, 0, 0) : NULL;
	SubscriptionView_UpdateLayout(hwnd, TRUE, LAYOUTREASON_RESIZE, validRegion, podcast->updateOffset);
	if (NULL != validRegion)
	{
		CombineRgn(podcast->updateRegion, podcast->updateRegion, validRegion, RGN_DIFF);
		DeleteObject(validRegion);
	}
}

static void SubscriptionView_SkinControls(HWND hwnd, const INT *itemList, INT itemCount, UINT skinType, UINT skinStyle)
{
	FLICKERFIX ff = {0, FFM_ERASEINPAINT};
	MLSKINWINDOW skinWindow = {0};
	skinWindow.style = skinStyle;
	skinWindow.skinType = skinType;

	for(INT i = 0; i < itemCount; i++)
	{
		ff.hwnd = skinWindow.hwndToSkin = GetDlgItem(hwnd, itemList[i]);
		if (IsWindow(skinWindow.hwndToSkin))
		{
			MLSkinWindow(plugin.hwndLibraryParent, &skinWindow);
			SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_FLICKERFIX, (WPARAM)&ff);
		}
	}
}

static void SubscriptionView_InitMetrics(HWND hwnd)
{
	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	HWND hControl;
	RECT rect;
	LONG t;

	podcast->vertDivider = (LONG)(channelDividerPercent * 10000);
	podcast->horzDivider = (LONG)(htmlDividerPercent * 10000);
	
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_ADD)) && FALSE != GetWindowRect(hControl, &rect))
	{
		t = rect.top + WASABI_API_APP->getScaleY(2);
		if (NULL != (hControl = GetDlgItem(hwnd, IDC_CHANNELLIST)) && FALSE != GetWindowRect(hControl, &rect))
			podcast->middleRowSpace = t - rect.bottom;
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_PLAY)) && FALSE != GetWindowRect(hControl, &rect))
	{
		t = rect.top + WASABI_API_APP->getScaleY(2);
		if (NULL != (hControl = GetDlgItem(hwnd, IDC_DESCRIPTION)) && FALSE != GetWindowRect(hControl, &rect))
			podcast->bottomRowSpace = t - rect.bottom;
	}
}

static void PodcastChannel_OnKeyDown(HWND hwnd, NMLVKEYDOWN *pkd)
{
	bool ctrl = (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL))),
		 shift =  (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)));
	switch(pkd->wVKey)
	{
		case VK_DELETE:
			if(!shift && !ctrl)
			{
				SENDCMD(hwnd, IDC_DELETE, 0, 0);
			}
			break;

		case VK_F5:
			if(!ctrl)
			{
				SENDCMD(hwnd, (!shift ? IDC_REFRESH : IDC_REFRESHALL), 0, 0);
			}
			break;

		case VK_F7:
			if (!shift && !ctrl)
			{
				SENDCMD(hwnd, IDC_VISIT, 0, 0);
			}
			break;

		case VK_F2:
			if(!shift && !ctrl)
			{
				SENDCMD(hwnd, IDC_EDIT, 0, 0);
			}
			break;

		case VK_INSERT:
			if(!shift && !ctrl)
			{
				SENDCMD(hwnd, IDC_ADD, 0, 0);
			}
			break;

		case VK_LEFT:
			if(!shift && !ctrl)
			{
				PodcastChannel_SelectNext(hwnd, FALSE);
			}	
			break;

		case VK_RIGHT:
			if(!shift && !ctrl)
			{
				PodcastChannel_SelectNext(hwnd, TRUE);
			}
			break;
	}
}

static void PodcastChannel_OnColumnClick(HWND hwnd, NMLISTVIEW *plv)
{
	BOOL fAscending;
	INT iSort = SubscriptionView_GetListSortColumn(hwnd, IDC_CHANNELLIST, &fAscending);
	fAscending = (-1 != iSort && iSort == plv->iSubItem) ? (!fAscending) : TRUE;
		
	PodcastChannel_Sort(hwnd, plv->iSubItem, fAscending);
}

static void PodcastChannel_OnDoubleClick(HWND hwnd, NMITEMACTIVATE *pia)
{
	if(pia->iItem != -1)
	{
		LVITEM lvi;
		lvi.state = LVIS_SELECTED;
		lvi.stateMask = LVIS_SELECTED;

		HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);

		if (NULL != hItems && 
			FALSE != SNDMSG(hItems, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi))
		{
			SENDCMD(hwnd, (((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^ ML_ENQDEF_VAL()) ? IDC_ENQUEUEACTION : IDC_PLAYACTION), 0, 0);
			lvi.state = 0;
			SNDMSG(hItems, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
		}
	}	
}

static void PodcastChannel_OnReturn(HWND hwnd, NMHDR *pnmh)
{
	LVITEM lvi;
	lvi.state = LVIS_SELECTED;
	lvi.stateMask = LVIS_SELECTED;

	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);

	if (NULL != hItems && 
		FALSE != SNDMSG(hItems, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi))
	{
		SENDCMD(hwnd, (((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^ ML_ENQDEF_VAL()) ? IDC_ENQUEUEACTION : IDC_PLAYACTION), 0, 0);
		lvi.state = 0;
		SNDMSG(hItems, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
	}
}

static void PodcastChannel_OnGetDispInfo(HWND hwnd, NMLVDISPINFO *pdi)
{
	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	if (0 != (LVIF_TEXT & pdi->item.mask))
	{
		switch (pdi->item.iSubItem)
		{
			case COLUMN_TITLE:
				{	
					pdi->item.pszText[0] = 0; // turn into an empty string as a safety precaution
					AutoLock channelLock(channels LOCKNAME("FillChannelTitle"));
					size_t iChannel =  (size_t)pdi->item.iItem;
                    if (iChannel < channels.size())
					{
						if (FALSE == podcast->channelAscending)
							iChannel = channels.size() - iChannel - 1;
						
						const wchar_t *title = channels[iChannel].title;
						if (title)
							StringCchCopy(pdi->item.pszText, pdi->item.cchTextMax, title);
					}
				}
				break;
		}
	}
}

static void PodcastChannel_OnItemChanged(HWND hwnd, NMLISTVIEW *plv)
{
	if ((plv->iItem != -1) && (LVIS_SELECTED & plv->uNewState) != (LVIS_SELECTED & plv->uOldState))
		PodcastChannel_SelectionChanged(hwnd, FALSE);
}

static void PodcastChannel_OnSetFocus(HWND hwnd, NMHDR *pnmh)
{
	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL == hItems) return;

	LVITEM lvi;
	lvi.state = 0;
	lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	SNDMSG(hItems, LVM_SETITEMSTATE, -1, (LPARAM)&lvi);
	SNDMSG(hItems, LVM_SETSELECTIONMARK, (WPARAM)0, (LPARAM)-1);
}

static LRESULT PodcastChannel_OnNotify(HWND hwnd, NMHDR *pnmh)
{
	switch (pnmh->code)
	{
		case LVN_KEYDOWN:		PodcastChannel_OnKeyDown(hwnd, (NMLVKEYDOWN *)pnmh); break;
		case LVN_COLUMNCLICK:	PodcastChannel_OnColumnClick(hwnd, (NMLISTVIEW*)pnmh); break;
		case NM_DBLCLK:			PodcastChannel_OnDoubleClick(hwnd, (NMITEMACTIVATE*)pnmh); break;
		case LVN_GETDISPINFO:	PodcastChannel_OnGetDispInfo(hwnd, (NMLVDISPINFO*)pnmh); break;
		case LVN_ITEMCHANGED:	PodcastChannel_OnItemChanged(hwnd, (NMLISTVIEW*)pnmh); break;
		case NM_SETFOCUS:		PodcastChannel_OnSetFocus(hwnd, pnmh); break;
		case NM_RETURN:			PodcastChannel_OnReturn(hwnd, pnmh); break;
	}

	return 0;
}

bool ParseDuration(const wchar_t *duration, int *out_hours, int *out_minutes, int *out_seconds)
{
	if (duration && duration[0])
	{
		const wchar_t *colon_position = wcschr(duration, L':');
		if (colon_position == 0)
		{
			int v = _wtoi(duration);
			*out_hours = v / 3600;
			*out_minutes = (v/60)%60;
			*out_seconds = v % 60;
			return true;
			}
			else
			{
				int first_time = _wtoi(duration);
				duration = colon_position+1;
				colon_position = wcschr(duration, L':');
				if (colon_position == 0) // only have MM:SS
				{
					*out_hours=0;
					*out_minutes = first_time;
					*out_seconds = _wtoi(duration);
				}
				else
				{
					*out_hours = first_time;
					*out_minutes = _wtoi(duration);
					*out_seconds = _wtoi(colon_position+1);
				}
				return true;
		}
	}
	else
	{
return false;
	}
}

static void PodcastItem_OnColumnClick(HWND hwnd, NMLISTVIEW *plv)
{		
	BOOL fAscending;
	INT iSort = SubscriptionView_GetListSortColumn(hwnd, IDC_ITEMLIST, &fAscending);
	fAscending = (-1 != iSort && iSort == plv->iSubItem) ? (!fAscending) : TRUE;
		
	PodcastItem_Sort(hwnd, plv->iSubItem, fAscending);
}

static void PodcastItem_OnGetDispInfo(HWND hwnd, NMLVDISPINFO *pdi)
{
	AutoLock channelLock (channels LOCKNAME("Item LVN_GETDISPINFO"));

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;
	
	size_t iChannel = podcast->channelActive;
	if (iChannel >= channels.size()) return;
	
	size_t iItem = pdi->item.iItem;
	if (iItem >= channels[iChannel].items.size()) return;
	if (FALSE == podcast->itemAscending) iItem = channels[iChannel].items.size() - iItem - 1;

	RSS::Item *item = &channels[iChannel].items[iItem];

	if (0 != (LVIF_IMAGE & pdi->item.mask))
	{
		switch (pdi->item.iSubItem)
		{
			case COLUMN_TITLE:
				if (!item->url || !item->url[0])
					pdi->item.iImage = 0;
				else if (item->downloaded)
					pdi->item.iImage = 1;
				else if (item->listened)
					pdi->item.iImage = 2;
				else 
					pdi->item.iImage = 3;
				break;
		}
	}

	if (0 != (LVIF_TEXT &pdi->item.mask))
	{
		pdi->item.pszText[0] = L'\0';
		switch (pdi->item.iSubItem)
		{
			case COLUMN_TITLE:
				lstrcpyn(pdi->item.pszText, item->itemName, pdi->item.cchTextMax);
				break;
			case COLUMN_MEDIA_TIME:
				{
					int hours, minutes, seconds;
					if (ParseDuration(item->duration, &hours, &minutes, &seconds))
					{
						if (hours)
						{
							StringCchPrintfW(pdi->item.pszText, pdi->item.cchTextMax, L"%d:%02d:%02d", hours, minutes, seconds);
						}
						else if (minutes)
						{
								StringCchPrintfW(pdi->item.pszText, pdi->item.cchTextMax, L"%d:%02d", minutes, seconds);
						}
						else
						{
							StringCchPrintfW(pdi->item.pszText, pdi->item.cchTextMax, L"0:%02d", seconds);
						}
					}
				}
		
				break;
			case COLUMN_MEDIA_SIZE:
				if (item->size)
				{
					WASABI_API_LNG->FormattedSizeString(pdi->item.pszText, pdi->item.cchTextMax, item->size);
				}
				break;
			//case COLUMN_MEDIA:
			//	WASABI_API_LNGSTRINGW_BUF((item->url && item->url[0]) ? IDS_ARTICLE_WITH_MEDIA : IDS_TEXT_ARTICLE, 
			//			pdi->item.pszText, pdi->item.cchTextMax);
			//	break;
			case COLUMN_DATEADDED:
				MakeDateString(item->publishDate, pdi->item.pszText, pdi->item.cchTextMax);
				break;
		}
	}
}

static void PodcastItem_OnItemChanged(HWND hwnd, NMLISTVIEW *plv)
{
	if ((LVIS_SELECTED & plv->uNewState) != (LVIS_SELECTED & plv->uOldState))
		PodcastItem_SelectionChanged(hwnd, FALSE);
}

static void PodcastItem_OnSetFocus(HWND hwnd, NMHDR *pnmh)
{
	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL == hItems) return;

	INT iMark = (INT)SNDMSG(hItems, LVM_GETNEXTITEM, -1, LVNI_FOCUSED | LVNI_SELECTED);
	if (-1 == iMark)
	{
		POINT pt;
		RECT rect;
		if (GetCursorPos(&pt) && GetClientRect(hwnd, &rect))
		{
			MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rect, 2);
			if (PtInRect(&rect, pt))
			{
				const INT szKey[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, /*VK_XBUTTON1*/0x05, /*VK_XBUTTON2*/0x06, };
				for (INT i = 0; i < ARRAYSIZE(szKey); i++)
				{
					if (0 != (0x80000000 & GetAsyncKeyState(szKey[i])))
						return;
				}
			}
		}

		LVITEM lvi;
		lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
		lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		if (FALSE != SNDMSG(hItems, LVM_SETITEMSTATE, 0, (LPARAM)&lvi))
		{
			SNDMSG(hItems, LVM_SETSELECTIONMARK, (WPARAM)0, (LPARAM)0);
			SNDMSG(hItems, LVM_ENSUREVISIBLE, 0, FALSE);
		}
			
	}
}

static void PodcastItem_OnDoubleClick(HWND hwnd, NMITEMACTIVATE *pia)
{
	SENDCMD(hwnd, (((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^ ML_ENQDEF_VAL()) ? IDC_ENQUEUEACTION : IDC_PLAYACTION), 0, 0);
}

static void PodcastItem_OnKeyDown(HWND hwnd, NMLVKEYDOWN *pkd)
{
	bool ctrl = (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL))),
		 shift =  (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)));
	switch(pkd->wVKey)
	{
		case VK_F5:
			if (!shift && !ctrl)
			{
				SENDCMD(hwnd, IDC_REFRESH, 0, 0);
			}
			break;

		case VK_F7:
			if (!shift && !ctrl)
			{
				SENDCMD(hwnd, IDC_VISIT, 0, 0);
			}
			break;

		case 'A':
			if (!shift && ctrl)
			{
				SENDCMD(hwnd, IDC_SELECTALL, 0, 0);
			}
			break;

		case 'F':		
			if (!shift && ctrl)
			{
				SENDCMD(hwnd, IDC_EXPLORERITEMFOLDER, 0, 0);
			}
			break;

		case 'D':
			if (!shift && ctrl)
			{
				SENDCMD(hwnd, IDC_DOWNLOAD, 0, 0);
			}
			break;
	}
}

static void PodcastItem_OnReturn(HWND hwnd, NMHDR *pnmh)
{
	SENDCMD(hwnd, (((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^ ML_ENQDEF_VAL()) ? IDC_ENQUEUEACTION : IDC_PLAYACTION), 0, 0);
}

static LRESULT PodcastItem_OnNotify(HWND hwnd, NMHDR *pnmh)
{
	switch (pnmh->code)
	{
		case NM_SETFOCUS:		PodcastItem_OnSetFocus(hwnd, pnmh); break;
		case LVN_COLUMNCLICK:	PodcastItem_OnColumnClick(hwnd, (NMLISTVIEW*)pnmh); break;
		case NM_DBLCLK:			PodcastItem_OnDoubleClick(hwnd, (NMITEMACTIVATE*)pnmh); break;
		case LVN_GETDISPINFO:	PodcastItem_OnGetDispInfo(hwnd, (NMLVDISPINFO*)pnmh); break;
		case LVN_ITEMCHANGED:	PodcastItem_OnItemChanged(hwnd, (NMLISTVIEW*)pnmh); break;
		case LVN_KEYDOWN:		PodcastItem_OnKeyDown(hwnd, (NMLVKEYDOWN *)pnmh); break;
		case NM_RETURN:			PodcastItem_OnReturn(hwnd, pnmh); break;
	}

	return 0;
}

static void CALLBACK SubscriptionView_OnDividerMoved(HWND hDivider, INT position, LPARAM param)
{
	HWND hwnd = GetParent(hDivider);
	if(NULL == hwnd) return;

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	RECT clientRect, dividerRect;
	if (FALSE == GetClientRect(hwnd, &clientRect) ||
		FALSE == GetWindowRect(hDivider, &dividerRect))
	{
		return;
	}

	LONG t;

	UINT reason = 0;

	switch(param)
	{
		case IDC_HDIV:
			t = (clientRect.bottom - clientRect.top) - (dividerRect.bottom - dividerRect.top);
			t = 10000 * position / t;
			if (podcast->horzDivider != t)
			{
				reason = (podcast->horzDivider > t) ? LAYOUTREASON_DIV_TOP : LAYOUTREASON_DIV_BOTTOM;
				podcast->horzDivider = t;
			}
			break;
		case IDC_VDIV:
			t = (clientRect.right - clientRect.left) - (dividerRect.right - dividerRect.left);
			t = 10000 * position / t;
			if (podcast->vertDivider != t)
			{
				reason = (podcast->vertDivider > t) ? LAYOUTREASON_DIV_LEFT : LAYOUTREASON_DIV_RIGHT;
				podcast->vertDivider = t;
			}
			break;
	}
	
	if (0 != reason)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		POINTS pts = {0,0};

		HRGN updateRegion = CreateRectRgnIndirect(&rc);
		HRGN validRegion = CreateRectRgn(0, 0, 0, 0);

		SubscriptionView_UpdateLayout(hwnd, FALSE, reason, validRegion, pts);
		CombineRgn(updateRegion, updateRegion, validRegion, RGN_DIFF);
		RedrawWindow(hwnd, NULL ,updateRegion, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);

		DeleteObject(updateRegion);
		DeleteObject(validRegion);
	}
}

static INT_PTR SubscriptionView_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)SUBSCRIPTIONVIEW_NAME);

	HACCEL accel = WASABI_API_LOADACCELERATORSW(IDR_VIEW_DOWNLOAD_ACCELERATORS);
	if (accel)
		WASABI_API_APP->app_addAccelerators(hwnd, &accel, 1, TRANSLATE_MODE_CHILD);

	PODCAST *podcast = (PODCAST*)calloc(1, sizeof(PODCAST));
	if (NULL == podcast || FALSE == SetProp(hwnd, MAKEINTATOM(VIEWPROP), podcast))
	{
		if (NULL != podcast) free(podcast);
		return 0;
	}

	current_window = hwnd;

	if (!view.play)
	{
		SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view);
	}

	g_context_menus3 = WASABI_API_LOADMENU(IDR_MENU1);
	groupBtn = ML_GROUPBTN_VAL();
	enqueuedef = (ML_ENQDEF_VAL() == 1);

	// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
	//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
	pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwnd, (INT_PTR)MAKELONG(IDC_CUSTOM, IDC_ENQUEUE), (INT_PTR)L"ml_wire"};
	wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
	if (pszTextW && pszTextW[0] != 0)
	{
		// set this to be a bit different so we can just use one button and not the
		// mixable one as well (leaving that to prevent messing with the resources)
		customAllowed = TRUE;
		SetDlgItemTextW(hwnd, IDC_CUSTOM, pszTextW);
	}
	else
		customAllowed = FALSE;

	SubscriptionView_InitMetrics(hwnd);

	podcast->channelActive = -1;

	HWND hLibrary = plugin.hwndLibraryParent;
	MLSkinWindow2(hLibrary, hwnd, SKINNEDWND_TYPE_AUTO, SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	PodcastChannel_InitializeList(hwnd);
	PodcastItem_InitializeList(hwnd);
	PodcastInfo_InitializeList(hwnd);

	const INT szControls[] = { IDC_FINDNEW, IDC_ADD, IDC_EDIT, IDC_DELETE, IDC_REFRESH, IDC_DOWNLOAD, IDC_VISIT, IDC_STATUS, };
	SubscriptionView_SkinControls(hwnd, szControls, ARRAYSIZE(szControls), SKINNEDWND_TYPE_AUTO,
								  SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	const INT szControlz[] = { IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM, };
	SubscriptionView_SkinControls(hwnd, szControlz, ARRAYSIZE(szControlz), SKINNEDWND_TYPE_AUTO,
								  SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | (groupBtn ? SWBS_SPLITBUTTON : 0));

	HWND hControl = GetDlgItem(hwnd, IDC_HDIV);
	MLSkinWindow2(hLibrary, hControl, SKINNEDWND_TYPE_DIVIDER, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWDIV_HORZ /*| SWDIV_NOHILITE*/);
	MLSkinnedDivider_SetCallback(hControl, SubscriptionView_OnDividerMoved, IDC_HDIV);

	hControl = GetDlgItem(hwnd, IDC_VDIV);
	MLSkinWindow2(hLibrary, hControl, SKINNEDWND_TYPE_DIVIDER, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWDIV_VERT /*| SWDIV_NOHILITE*/);
	MLSkinnedDivider_SetCallback(hControl, SubscriptionView_OnDividerMoved, IDC_VDIV);

	PodcastChannel_Sort(hwnd, 0, channelSortAscending);
	PodcastItem_Sort(hwnd, currentItemSort, itemSortAscending);

	OmService *service = (OmService*)lParam;
	HWND hBrowser = NULL;
	if (NULL != OMBROWSERMNGR && 
		SUCCEEDED(OMBROWSERMNGR->Initialize(NULL, plugin.hwndWinampParent)) &&
		SUCCEEDED(OMBROWSERMNGR->CreateView(service, hwnd, NAVIGATE_BLANK, NBCS_NOTOOLBAR | NBCS_NOSTATUSBAR, &hBrowser)))
	{
		HWND hTarget = GetDlgItem(hwnd, IDC_DESCRIPTION);
		if (NULL != hTarget)
		{
			RECT rect;
			if (GetWindowRect(hTarget, &rect))
			{
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
				SetWindowPos(hBrowser, hTarget, rect.left, rect.top, rect.right -rect.left, rect.bottom - rect.top, 
					SWP_NOACTIVATE | SWP_NOOWNERZORDER);
			}
			DestroyWindow(hTarget);
		}

		SetWindowLongPtr(hBrowser, GWLP_ID, IDC_DESCRIPTION);
		ShowWindow(hBrowser, SW_SHOWNA);
	}

	Downloads_UpdateButtonText(hwnd, enqueuedef);
	SendMessage(hwnd, WM_DISPLAYCHANGE, 0, 0L);

	wchar_t status[256] = {0};
	cloud.GetStatus(status, 256);
	SubscriptionView_SetStatus(hwnd, status);
	SubscriptionView_RefreshChannels(hwnd, TRUE);

	// evil to do but as the data is known and kept by us then this should be ok
	if (channelLastSelection != -1)
	{
		static LV_ITEM _macro_lvi;
		_macro_lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		_macro_lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
		PostMessage(GetDlgItem(hwnd, IDC_CHANNELLIST), LVM_SETITEMSTATE,
					(WPARAM)channelLastSelection, (LPARAM)(LV_ITEM *)&_macro_lvi);
	}

	return 0;
}

static void SubscriptionView_OnDestroy(HWND hwnd)
{
	PODCAST *podcast = GetPodcast(hwnd);
	RemoveProp(hwnd, MAKEINTATOM(VIEWPROP));

	if (NULL != podcast)
	{
		htmlDividerPercent = podcast->horzDivider / 10000.0f;
		channelDividerPercent = podcast->vertDivider / 10000.0f;

		Plugin_FreeString(podcast->infoUrl);
		Plugin_FreeString(podcast->description);
		free(podcast);
	}

	BOOL fAscending;

	currentItemSort = SubscriptionView_GetListSortColumn(hwnd, IDC_ITEMLIST, &fAscending);
	itemSortAscending = (-1 != currentItemSort) ? (FALSE != fAscending) : true;

	INT iSort = SubscriptionView_GetListSortColumn(hwnd, IDC_CHANNELLIST, &fAscending);
	channelSortAscending = (-1 != iSort) ? (FALSE != fAscending) : true;

	INT iSelected = (INT)SNDMSG(GetDlgItem(hwnd, IDC_CHANNELLIST), LVM_GETNEXTITEM, -1, LVNI_SELECTED);

	HWND hControl = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL != hControl)
	{
		itemTitleWidth = ListView_GetColumnWidth(hControl, COLUMN_TITLE);
		itemDateWidth = ListView_GetColumnWidth(hControl, COLUMN_DATEADDED);
		//itemMediaWidth = ListView_GetColumnWidth(hControl, COLUMN_MEDIA);
		itemMediaWidth = ListView_GetColumnWidth(hControl, COLUMN_MEDIA_TIME);
		itemSizeWidth = ListView_GetColumnWidth(hControl, COLUMN_MEDIA_SIZE);

		HIMAGELIST imageList = (HIMAGELIST)SendMessage(hControl, LVM_GETIMAGELIST, LVSIL_SMALL, 0L);
		if (NULL != imageList) 
			ImageList_Destroy(imageList);
	}

	// ensure view settings are saved...
	SaveAll(true);
}

static void SubscriptionView_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	HRGN validRegion = (NULL != podcast->updateRegion) ? CreateRectRgn(0, 0, 0, 0) : NULL;

	SubscriptionView_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)), 
								  LAYOUTREASON_RESIZE, validRegion, podcast->updateOffset);

	if (NULL != validRegion)
	{
		CombineRgn(podcast->updateRegion, podcast->updateRegion, validRegion, RGN_DIFF);
		DeleteObject(validRegion);
	}
}

static void SubscriptionView_OnContextMenu(HWND hwnd, HWND hTarget, POINTS pts)
{
	INT controlId = (NULL != hTarget) ? (INT)GetWindowLongPtr(hTarget, GWLP_ID) : 0;

	switch(controlId)
	{
		case IDC_ITEMLIST:
		case IDC_CHANNELLIST:
			SubscriptionView_ListContextMenu(hwnd, controlId, pts); 
			break;
	}
}

static void SubscriptionView_OnSetUpdateRegion(HWND hwnd, HRGN updateRegion, POINTS updateOffset)
{
	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	podcast->updateRegion = updateRegion;
	podcast->updateOffset = updateOffset;
}

static void PodcastCommand_OnDeleteChannel(HWND hwnd)
{
	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	AutoLock lock (channels LOCKNAME("DeleteChannel"));

	size_t iChannel = PodcastChannel_GetActive(hwnd);
	if (BAD_CHANNEL == iChannel) return;

	WCHAR szText[1024] = {0}, szBuffer[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_SURE_YOU_WANT_TO_REMOVE_THIS, szBuffer, ARRAYSIZE(szBuffer));
	StringCchPrintf(szText, ARRAYSIZE(szText), szBuffer, channels[iChannel].title, channels[iChannel].url);

	WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRM, szBuffer, ARRAYSIZE(szBuffer));

	if (IDYES == MessageBox(hwnd, szText, szBuffer, MB_YESNO | MB_ICONWARNING))
	{
		channels.RemoveChannel((int)iChannel);
		SubscriptionView_RefreshChannels(hwnd, FALSE);
		SaveAll();
	}
}

static void PodcastCommand_OnVisitSite(HWND hwnd)
{
	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast || NULL == podcast->infoUrl) 
		return;

	SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, podcast->infoUrl);
}

static void PodcastCommand_OnRefreshChannel(HWND hwnd)
{
	size_t iChannel = PodcastChannel_GetActive(hwnd);
	if (BAD_CHANNEL != iChannel)
	{
		channels[iChannel].needsRefresh = true;
		cloud.Pulse();
	}
}

static void PodcastCommand_OnEditChannel(HWND hwnd)
{
	size_t iChannel = PodcastChannel_GetActive(hwnd);
	if (BAD_CHANNEL == iChannel) return;

	ChannelEditor_Show(hwnd, iChannel, CEF_CENTEROWNER);

	HWND hChannel = GetDlgItem(hwnd, IDC_CHANNELLIST);
	if (NULL != hChannel)
		PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hChannel, TRUE);
}

static void PodcastCommand_OnAddChannel(HWND hwnd)
{
	ChannelEditor_Show(hwnd, 0, CEF_CENTEROWNER | CEF_CREATENEW);

	HWND hChannel = GetDlgItem(hwnd, IDC_CHANNELLIST);
	if (NULL != hChannel)
		PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hChannel, TRUE);
}

static void PodcastCommand_OnFindNewChannel(HWND hwnd)
{
	HNAVITEM hItem = Navigation_FindService(SERVICE_PODCAST, NULL, NULL);
	MLNavItem_Select(plugin.hwndLibraryParent, hItem);
}

static void PodcastCommand_OnRefreshAll(HWND hwnd)
{
	cloud.RefreshAll();
	cloud.Pulse();
}

static void PodcastCommand_OnPlaySelection(HWND hwnd, BOOL fEnqueue, BOOL fForce)
{
	AutoLock channelLock (channels LOCKNAME("PlaySelection"));

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	size_t iChannel = PodcastChannel_GetActive(hwnd);
	if (BAD_CHANNEL == iChannel) return;
	Channel *channel = &channels[iChannel];

	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL == hItems) return;

	INT selectedCount = (INT)SNDMSG(hItems, LVM_GETSELECTEDCOUNT, 0, 0L);
	if (0 == selectedCount) return;

	size_t indexCount = 0;
	size_t *indexList = (size_t*)calloc(selectedCount, sizeof(size_t));
	if (NULL == indexList) return;

	INT iSelected = -1;

	while(-1 != (iSelected = SNDMSG(hItems, LVM_GETNEXTITEM, (WPARAM)iSelected, LVNI_SELECTED)))
	{
		size_t iItem = iSelected;
		if (iItem < channel->items.size())
		{
			if (FALSE == podcast->itemAscending)
				iItem = channel->items.size() - iItem - 1;

			indexList[indexCount++] = iItem;
		}
	}

	if (0 != indexCount)
	{
		SubscriptionView_Play(hwnd, iChannel, indexList, indexCount, fEnqueue, fForce);
	}

	free(indexList);
}

static void PodcastCommand_OnDownloadSelection( HWND hwnd )
{
	AutoLock channelLock( channels LOCKNAME( "DownloadSelection" ) );

	PODCAST *podcast = GetPodcast( hwnd );
	if ( NULL == podcast ) return;

	size_t iChannel = PodcastChannel_GetActive( hwnd );
	if ( BAD_CHANNEL == iChannel ) return;
	Channel *channel = &channels[ iChannel ];

	HWND hItems = GetDlgItem( hwnd, IDC_ITEMLIST );
	if ( NULL == hItems ) return;

	INT selectedCount = (INT)SNDMSG( hItems, LVM_GETSELECTEDCOUNT, 0, 0L );
	if ( 0 == selectedCount ) return;

	INT iSelected = -1;

	WCHAR szPath[ MAX_PATH * 2 ] = { 0 };

	size_t scheduled = 0;

	while ( -1 != ( iSelected = SNDMSG( hItems, LVM_GETNEXTITEM, (WPARAM)iSelected, LVNI_SELECTED ) ) )
	{
		size_t iItem = iSelected;
		if ( iItem < channel->items.size() )
		{
			if ( FALSE == podcast->itemAscending )
				iItem = channel->items.size() - iItem - 1;

			RSS::Item *downloadItem = &channel->items[ iItem ];
			if ( ( downloadItem->url && downloadItem->url[ 0 ] ) && SUCCEEDED( downloadItem->GetDownloadFileName( channel->title, szPath, ARRAYSIZE( szPath ), TRUE ) ) )
			{
				if ( !wa::files::file_exists( szPath ) )
				{
					wchar_t *url = urlencode( downloadItem->url );
					downloader.Download( url, szPath, channel->title, downloadItem->itemName, downloadItem->publishDate );
					downloadItem->downloaded = true;
					if ( 0 == scheduled )
					{
						SendMessage( hwnd, SVM_SETSTATUS, 0, (LPARAM)IDS_ADD_TO_DOWNLOADS );
					}
					free( url );
					scheduled++;
				}
			}
		}
	}

	if ( 0 == scheduled )
		SendMessage( hwnd, SVM_SETSTATUS, 0, (LPARAM)IDS_NO_MEDIA_TO_DOWNLOAD );
}

static void PodcastCommand_OnExploreItemFolder(HWND hwnd)
{
	AutoLock channelLock (channels LOCKNAME("Explore"));

	PODCAST *podcast = GetPodcast(hwnd);
	if (NULL == podcast) return;

	size_t iChannel = PodcastChannel_GetActive(hwnd);
	if (BAD_CHANNEL == iChannel) return;
	Channel *channel = &channels[iChannel];

	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL == hItems) return;

	INT selectedCount = (INT)SNDMSG(hItems, LVM_GETSELECTEDCOUNT, 0, 0L);
	if (0 == selectedCount) return;

	INT iSelected = -1;

	WCHAR szPath[MAX_PATH * 2] = {0};
	WASABI_API_EXPLORERFINDFILE->Reset();	

	while(-1 != (iSelected = SNDMSG(hItems, LVM_GETNEXTITEM, (WPARAM)iSelected, LVNI_SELECTED)))
	{
		size_t iItem = iSelected;
		if (iItem < channel->items.size())
		{
			if (FALSE == podcast->itemAscending)
				iItem = channel->items.size() - iItem - 1;

			RSS::Item *downloadItem = &channel->items[iItem];
			if ((downloadItem->url && downloadItem->url[0]) &&
				SUCCEEDED(downloadItem->GetDownloadFileName(channel->title, szPath, ARRAYSIZE(szPath), TRUE)) &&
				PathFileExists(szPath))
			{
				WASABI_API_EXPLORERFINDFILE->AddFile(szPath);
			}
		}
	}
	WASABI_API_EXPLORERFINDFILE->ShowFiles();
}

static void PodcastCommand_OnSelectAll(HWND hwnd)
{
	HWND hItems = GetDlgItem(hwnd, IDC_ITEMLIST);
	if (NULL == hItems) return;

	INT iCount = (INT)SNDMSG(hItems, LVM_GETITEMCOUNT, 0, 0L);
	if (0 != iCount)
	{
		INT iSelected = (INT)SNDMSG(hItems, LVM_GETSELECTEDCOUNT, 0, 0L);
		if (iSelected != iCount)
		{
			LVITEM lvi;
			lvi.state = LVIS_SELECTED;
			lvi.stateMask = LVIS_SELECTED;
			SNDMSG(hItems, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
		}
	}
}

static void PodcastCommand_PlayEnqueue(HWND hwndDlg, HWND from, UINT idFrom)
{
	HMENU listMenu = GetSubMenu(g_context_menus3, 1);
	int count = GetMenuItemCount(listMenu);
	if (count > 2)
	{
		for (int i = 2; i < count; i++)
		{
			DeleteMenu(listMenu, 2, MF_BYPOSITION);
		}
	}

	Downloads_ButtonPopupMenu(hwndDlg, idFrom, listMenu, BPM_WM_COMMAND);
}

static void SubscriptionView_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch (controlId)
	{
		case IDC_CUSTOM:
		case IDC_PLAY:
		case IDC_ENQUEUE:
		case IDC_PLAYACTION:
		case IDC_ENQUEUEACTION:

			if (eventId == MLBN_DROPDOWN)
			{
				PodcastCommand_PlayEnqueue(hwnd, hControl, controlId);
			}
			else
			{
				int action;
				if (controlId == IDC_PLAY || controlId == IDC_PLAYACTION)
					action = (eventId == 1) ? enqueuedef == 1 : 0;
				else if (controlId == IDC_ENQUEUE || controlId == IDC_ENQUEUEACTION)
					action = (eventId == 1) ? (enqueuedef != 1) : 1;
				else
					// so custom can work with the menu item part
					break;

				PodcastCommand_OnPlaySelection(hwnd, action, (controlId == IDC_PLAY || controlId == IDC_ENQUEUE));
			}
			break;

		case IDC_VISIT: 		PodcastCommand_OnVisitSite(hwnd); break;
		case IDC_DOWNLOAD:		PodcastCommand_OnDownloadSelection(hwnd); break;
		case IDC_EXPLORERITEMFOLDER:	PodcastCommand_OnExploreItemFolder(hwnd); break;
		case IDC_SELECTALL:		PodcastCommand_OnSelectAll(hwnd); break;
		case IDC_REFRESH:		PodcastCommand_OnRefreshChannel(hwnd); break;
		case IDC_EDIT:			PodcastCommand_OnEditChannel(hwnd); break;
		case IDC_ADD:			PodcastCommand_OnAddChannel(hwnd); break;
		case IDC_FINDNEW:		PodcastCommand_OnFindNewChannel(hwnd); break;
		case IDC_DELETE:		PodcastCommand_OnDeleteChannel(hwnd); break;
		case IDC_REFRESHALL:	PodcastCommand_OnRefreshAll(hwnd); break;
	}
}

static LRESULT SubscriptionView_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	LRESULT result = 0;
	switch (controlId)
	{
		case IDC_CHANNELLIST:	result = PodcastChannel_OnNotify(hwnd, pnmh);  break;
		case IDC_ITEMLIST:		result = PodcastItem_OnNotify(hwnd, pnmh); break;
	}

	return result;
}

static void SubscriptionView_OnChar(HWND hwnd, UINT vKey, UINT flags)
{
	switch(vKey)
	{
		case VK_DELETE:
			if (GetFocus() == GetDlgItem(hwnd, IDC_CHANNELLIST))
				SENDCMD(hwnd, IDC_DELETE, 0, 0L);
			break;
	}
}

static BOOL SubscriptionView_OnRefreshChannels(HWND hwnd, BOOL fSort)
{
	AutoLock lock (channels LOCKNAME("All_ChannelsUpdated"));

	HWND hChannel = GetDlgItem(hwnd, IDC_CHANNELLIST);
	if (NULL == hChannel) return FALSE;

	size_t channelSize = channels.size();
	size_t iSelected = (size_t)SNDMSG(hChannel, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

	if (FALSE != fSort)
	{
		INT iSort = SubscriptionView_GetListSortColumn(hwnd, IDC_CHANNELLIST, NULL);
		SubscriptionView_SortChannels(iSort);
	}

	SNDMSG(hChannel, LVM_SETITEMCOUNT, channelSize, 0L);
	PodcastChannel_SyncHeaderSize(hwnd, TRUE);

	BOOL selectChannel = FALSE;
	if(BAD_CHANNEL != iSelected && iSelected >= channelSize)
	{
		iSelected = (channelSize - 1);
		selectChannel = TRUE;
	}

	if (BAD_CHANNEL == iSelected && channelSize > 0)
	{
		iSelected = 0;
		selectChannel = TRUE;
	}

	if (FALSE != selectChannel)		
	{
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		lvi.state = lvi.stateMask;

		if (FALSE == SNDMSG(hChannel, LVM_SETITEMSTATE, (WPARAM)iSelected, (LPARAM)&lvi))
			iSelected = -1;
		else
		{
			SNDMSG(hChannel, LVM_SETSELECTIONMARK, (WPARAM)0, (LPARAM)iSelected);
			SNDMSG(hChannel, LVM_ENSUREVISIBLE, (WPARAM)iSelected, (LPARAM)FALSE);
		}
	}

	if (FALSE == selectChannel || 0 == channelSize)
	{
		PodcastChannel_SelectionChanged(hwnd, FALSE);
	}

	return TRUE;
}

static void SubscriptionView_OnParentNotify(HWND hwnd, UINT uMsg, HWND childId, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case /*WM_XBUTTONDOWN*/0x020B:
			{
				POINT pt;
				GetCursorPos(&pt);

				RECT rect;
				HWND hItem = GetDlgItem(hwnd, IDC_ITEMLIST);
				if (NULL != hItem && hItem != GetFocus() && FALSE != GetClientRect(hItem, &rect))
				{
					MapWindowPoints(hItem, HWND_DESKTOP, (POINT*)&rect, 2);
					if (PtInRect(&rect, pt))
					{
						SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hItem, MAKELPARAM(TRUE, 0));
					}
				}
			}
			break;
	}
}

static LRESULT SubscriptionView_OnSetStatus(HWND hwnd, LPCWSTR pszStatus)
{
	HWND hStatus = GetDlgItem(hwnd, IDC_STATUS);
	if (NULL == hStatus) return FALSE;

	WCHAR szBuffer[512] = {0};
	if (IS_INTRESOURCE(pszStatus))
	{
		pszStatus = WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszStatus, szBuffer, ARRAYSIZE(szBuffer));
	}

    return SetWindowText(hStatus, pszStatus);	
}

static INT_PTR CALLBACK SubscriptionView_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:			return SubscriptionView_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:			SubscriptionView_OnDestroy(hwnd); return TRUE;
		case WM_WINDOWPOSCHANGED:	SubscriptionView_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_DISPLAYCHANGE:		SubscriptionView_UpdateSkin(hwnd); return TRUE;
		case WM_CONTEXTMENU:		SubscriptionView_OnContextMenu(hwnd, (HWND)wParam, MAKEPOINTS(lParam)); return TRUE;
		case WM_NOTIFYFORMAT:		MSGRESULT(hwnd, NFR_UNICODE); 
		case WM_COMMAND:			SubscriptionView_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_NOTIFY:				MSGRESULT(hwnd, SubscriptionView_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam));
		case WM_CHAR:				SubscriptionView_OnChar(hwnd, (UINT)wParam, (UINT)lParam); return TRUE;
		case WM_PARENTNOTIFY:		SubscriptionView_OnParentNotify(hwnd, LOWORD(wParam), (HWND)HIWORD(wParam), lParam); return TRUE;

		case WM_USER + 0x200:		MSGRESULT(hwnd, TRUE);
		case WM_USER + 0x201:		SubscriptionView_OnSetUpdateRegion(hwnd, (HRGN)lParam, MAKEPOINTS(wParam)); return TRUE;

		case SVM_REFRESHCHANNELS:	MSGRESULT(hwnd, SubscriptionView_OnRefreshChannels(hwnd, (BOOL)wParam));
		case SVM_SETSTATUS:			MSGRESULT(hwnd, SubscriptionView_OnSetStatus(hwnd, (LPCWSTR)lParam));

		case WM_INITMENUPOPUP:
			if (wParam && (HMENU)wParam == s.build_hMenu && s.mode==1)
			{
				if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
					s.mode = 2;
			}
			return 0;

		case WM_APP + 104:
		{
			Downloads_UpdateButtonText(hwnd, enqueuedef);
			SendMessage(hwnd, WM_DISPLAYCHANGE, 0, 0);
			return 0;
		}

		case WM_PAINT:
			if (IsWindowVisible(GetDlgItem(hwnd, IDC_DESCRIPTION)))
			{
				int tab[] = { IDC_DESCRIPTION|DCW_SUNKENBORDER};
				dialogSkinner.Draw(hwnd, tab, sizeof(tab) / sizeof(tab[0]));
			}
			return 0;
	}
	return 0;
}

HWND CALLBACK SubscriptionView_Create(HWND hParent, OmService *service)
{
	return WASABI_API_CREATEDIALOGPARAMW(IDD_PODCAST, hParent, SubscriptionView_DlgProc, (LPARAM)service);
}

HWND CALLBACK SubscriptionView_FindWindow()
{
	HWND hView = (HWND)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GETCURRENTVIEW, 0);
	if (NULL == hView) return NULL;

	WCHAR szBuffer[128] = {0};
	if (0 == GetClassName(hView, szBuffer, ARRAYSIZE(szBuffer)) ||
		CSTR_EQUAL != CompareStringW(CSTR_INVARIANT, 0, szBuffer, -1, L"#32770", -1) ||
		0 == GetWindowText(hView, szBuffer, ARRAYSIZE(szBuffer)) ||
		CSTR_EQUAL != CompareStringW(CSTR_INVARIANT, 0, szBuffer, -1, SUBSCRIPTIONVIEW_NAME, -1))
	{
		hView = NULL;
	}

	return hView;
}