#include "main.h"
#include "..\..\General\gen_ml/ml.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "..\..\General\gen_ml/itemlist.h"
#include "../nu/listview.h"
#include "..\..\General\gen_ml/childwnd.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"
#include "resource1.h"
#include "SkinnedListView.h"
#include "DeviceView.h"
#include "ArtistAlbumLists.h"
#include "mt19937ar.h" // random number generator
#include "api__ml_pmp.h"
#include "..\..\General\gen_ml/graphics.h"
#include <tataki/export.h>
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>
#include "AlbumArtListView.h"
#include "./local_menu.h"
#include "metadata_utils.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <shlobj.h>

extern winampMediaLibraryPlugin plugin;
extern int currentViewedPlaylist;
extern DeviceView * currentViewedDevice;
extern HMENU m_context_menus;
extern C_ItemList devices;

extern void editInfo(C_ItemList * items, Device * dev, HWND centerWindow); // from editinfo.cpp

static INT_PTR CALLBACK pmp_common_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

int (*wad_handleDialogMsgs)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
static void (*wad_DrawChildWindowBorders)(HWND hwndDlg, int *tab, int tabsize);

HWND hwndMediaView;
static SkinnedListView *artistList=NULL, *albumList=NULL, *albumList2=NULL, *tracksList=NULL;

static int adiv1_nodraw=0,adiv3_nodraw=0;
static int m_nodrawtopborders=0;
static int numFilters;

static int adiv1pos=-1, adiv2pos=-1, adiv3pos=-1;
static int refineHidden=0;

static BOOL g_displaysearch = TRUE;
static BOOL g_displayrefine = TRUE;
static BOOL g_displaystatus = TRUE;


static HRGN g_rgnUpdate = NULL;
static int offsetX = 0, offsetY = 0, header = 0;
viewButtons view = {0};
static bool noSearchTimer=false;

static ArtistAlbumLists *aacontents=NULL;
static PrimaryListContents *tracks=NULL;

static void UpdateStatus(HWND hwndDlg, bool full = false) {
	wchar_t buf[1024]=L"";
	if (tracks)
	{
		tracks->GetInfoString(buf);
		SetDlgItemText(hwndDlg,IDC_STATUS,buf);
	}

	if (full && currentViewedDevice && currentViewedDevice->isCloudDevice)
	{
		int usedPercent = 0;

		__int64 available = currentViewedDevice->dev->getDeviceCapacityAvailable();
		__int64 capacity = currentViewedDevice->dev->getDeviceCapacityTotal();

		if(capacity > 0) usedPercent = (int)((((__int64)100)*available) / capacity);

		if (!header)
		{
			wchar_t buf[128], status[128], availStr[100]=L"";
			currentViewedDevice->dev->getTrackExtraInfo(0, L"cloud_status", status, ARRAYSIZE(status));

			WASABI_API_LNG->FormattedSizeString(availStr, ARRAYSIZE(availStr), (available > 0 ? available : capacity));
			wsprintf(buf, L"%s %s      %s", availStr, (available > 0 ? L"free" : L"used"), status);
			SetDlgItemText(hwndDlg, IDC_HEADER_DEVICE_SIZE, buf);
			SendDlgItemMessage(hwndDlg, IDC_HEADER_DEVICE_BAR, PBM_SETPOS, (100 - usedPercent), 0);
		}
	}
}

static wchar_t *playmode = L"viewplaymode";

typedef void (WINAPI  *DIVIDERMOVED)(HWND, INT, LPARAM);
typedef struct _DIVIDER
{
	BOOL			fVertical;
	DIVIDERMOVED		callback;
	LPARAM			param;
	WNDPROC			fnOldProc;
	BOOL			fUnicode;
	INT				clickoffs;
} DIVIDER;

#define GET_DIVIDER(hwnd) (DIVIDER*)GetPropW(hwnd, L"DIVDATA")

static BOOL AttachDivider(HWND hwnd, BOOL fVertical, DIVIDERMOVED callback, LPARAM param);
static LRESULT CALLBACK div_newWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void WINAPI OnDividerMoved(HWND hwnd, INT nPos, LPARAM param);
void LayoutWindows(HWND hwnd, BOOL fRedraw, INT simple);

static HBITMAP ConvertTo24bpp(HBITMAP bmp, int bpp)
{
	HDC hdcMem, hdcMem2;
    HBITMAP hbm24;
    BITMAP bm;

    GetObjectW(bmp, sizeof(BITMAP), &bm);

    hdcMem = CreateCompatibleDC(0);
    hdcMem2 = CreateCompatibleDC(0);

	void *bits;
	BITMAPINFOHEADER bi;
		 
	ZeroMemory (&bi, sizeof (bi));
	bi.biSize = sizeof (bi);
	bi.biWidth= bm.bmWidth;
	bi.biHeight = -bm.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount= bpp;

	hbm24 = CreateDIBSection(hdcMem2, (BITMAPINFO *)&bi, DIB_RGB_COLORS, &bits, NULL, NULL);

    HBITMAP oBmp = (HBITMAP)SelectObject(hdcMem, bmp);
	HBITMAP oBmp24 = (HBITMAP)SelectObject(hdcMem2, hbm24);
	
	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, oBmp);
    SelectObject(hdcMem2, oBmp24);

    DeleteDC(hdcMem);
    DeleteDC(hdcMem2);


    return hbm24;
}

C_ItemList * getSelectedItems(bool all=false) {
	if(!currentViewedDevice) return NULL;
	C_ItemList * selected = new C_ItemList;
	int l = tracks->GetNumRows();
	if(all || tracksList->listview.GetSelectedCount()==0) for(int i=0; i<l; i++) selected->Add((void*)tracks->GetTrack(i));
	else for(int i=0; i<l; i++) if(tracksList->listview.GetSelected(i)) selected->Add((void*)tracks->GetTrack(i));
	return selected;
}

int showContextMenu(int context, HWND hwndDlg, Device * dev, POINT pt) {

	// does cloud specific menu hacks
	int cloud_devices = 0;
	HMENU cloud_menu = 0;
	if (context < 2)
	{
		int mark = tracksList->listview.GetSelectionMark();
		if (mark != -1)
		{
			// if wanting to do on the selection then use getSelectedItems()
			//C_ItemList * items = getSelectedItems();
			// otherwise only work on the selection mark for speed (and like how ml_local does things)
			C_ItemList * items = new C_ItemList;
			items->Add((void*)tracks->GetTrack(mark));
			cloud_menu = (HMENU)dev->extraActions(DEVICE_GET_CLOUD_SOURCES_MENU, (intptr_t)&cloud_devices, 0, (intptr_t)items);
			delete items;
		}
	}

	HMENU menu = GetSubMenu(m_context_menus, context);
	HMENU sendto = GetSubMenu(menu, 2);
	int fieldsBits = (int)dev->extraActions(DEVICE_SUPPORTED_METADATA, 0, 0, 0);

	bool noRatings = !(!fieldsBits || (fieldsBits & SUPPORTS_RATING));
	bool noEdit = dev->extraActions(DEVICE_DOES_NOT_SUPPORT_EDITING_METADATA, 0, 0, 0) != 0;

	HMENU ratingMenu = Menu_FindRatingMenu(menu, FALSE);
	if (NULL != ratingMenu)
	{
		Menu_SetRatingValue(ratingMenu, -1);
	}

	// toggle text of the delete menu item as needed
	MENUITEMINFOW mii = {sizeof(MENUITEMINFOW), 0};
	mii.fMask = MIIM_STRING;
	mii.dwTypeData = WASABI_API_LNGSTRINGW(!currentViewedDevice->isCloudDevice ? IDS_DELETE : IDS_REMOVE);
	mii.cch = wcslen(mii.dwTypeData);
	// just make sure we've got a string to use here
	if (mii.cch > 0)
	{
		SetMenuItemInfoW(menu, ID_TRACKSLIST_DELETE, FALSE, &mii);
	}
	EnableMenuItem(menu, ID_TRACKSLIST_DELETE, MF_BYCOMMAND |
				   (!currentViewedDevice->isCloudDevice ||
				    (currentViewedDevice->isCloudDevice &&
				    !currentViewedDevice->dev->extraActions(DEVICE_NOT_READY_TO_VIEW, 0, 0, 0))) ? MF_ENABLED : MF_GRAYED);

	EnableMenuItem(menu, ID_TRACKSLIST_EDITSELECTEDITEMS, MF_BYCOMMAND | noEdit ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(menu, ID_RATE_0, MF_BYCOMMAND | noRatings ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(menu, ID_RATE_1, MF_BYCOMMAND | noRatings ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(menu, ID_RATE_2, MF_BYCOMMAND | noRatings ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(menu, ID_RATE_3, MF_BYCOMMAND | noRatings ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(menu, ID_RATE_4, MF_BYCOMMAND | noRatings ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(menu, ID_RATE_5, MF_BYCOMMAND | noRatings ? MF_GRAYED : MF_ENABLED);

	EnableMenuItem(menu, ID_TRACKSLIST_COPYTOLIBRARY, MF_BYCOMMAND | dev->copyToHardDriveSupported() ? MF_ENABLED : MF_GRAYED);
	int num = 0;
	// TODO remove once we've got cloud playlist implemented
	if (0 == dev->extraActions(DEVICE_PLAYLISTS_UNSUPPORTED,0,0,0))
	{
		if (EnableMenuItem(menu, ID_ADDTOPLAYLIST_NEWPLAYLIST, MF_BYCOMMAND | (!cloud_menu ? MF_ENABLED : MF_GRAYED)) == -1)
		{
			MENUITEMINFOW m = {sizeof(m), MIIM_TYPE | MIIM_ID | MIIM_SUBMENU, MFT_STRING, 0};
			wchar_t a[100] = {0};
			m.dwTypeData = WASABI_API_LNGSTRINGW_BUF(IDS_SEND_TO_PL, a, 100);
			m.wID = 2;
			sendto = m.hSubMenu = CreatePopupMenu();
			InsertMenuItemW(menu, 2, TRUE, &m);

			m.fMask -= MIIM_SUBMENU;
			m.dwTypeData = WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_CMD_PLAYLIST_CREATE, a, 100);
			m.wID = ID_ADDTOPLAYLIST_NEWPLAYLIST;
			m.fState = (!cloud_menu ? MF_ENABLED : MF_GRAYED);
			InsertMenuItemW(m.hSubMenu, 0, FALSE, &m);
		}

		num = dev->getPlaylistCount();
		if(num > 1) AppendMenu(sendto, MF_SEPARATOR, 0, L"");
		for(int i=1; i<num; i++) {
			wchar_t buf[100] = {0};
			dev->getPlaylistName(i, buf, sizeof(buf)/sizeof(wchar_t));
			AppendMenu(sendto, 0, 100000 + i, buf);
		}
	}
	else
	{
		if (DeleteMenu(menu, ID_ADDTOPLAYLIST_NEWPLAYLIST, MF_BYCOMMAND))
		{
			DeleteMenu(menu, 2, MF_BYPOSITION);
			sendto = NULL;
		}
	}

	if (cloud_menu)
	{
		MENUITEMINFOW m = {sizeof(m), MIIM_TYPE | MIIM_ID | MIIM_SUBMENU, MFT_SEPARATOR, 0};
		m.wID = CLOUD_SOURCE_MENUS - 1;
		m.fType = MFT_SEPARATOR;
		InsertMenuItemW(menu, (sendto ? 3 : 2), TRUE, &m);

		wchar_t a[100] = {0};
		m.fType = MFT_STRING;
		m.dwTypeData = WASABI_API_LNGSTRINGW_BUF(IDS_CLOUD_SOURCES, a, 100);
		m.wID = CLOUD_SOURCE_MENUS;
		m.hSubMenu = cloud_menu;
		InsertMenuItemW(menu, (sendto ? 4 : 3), TRUE, &m);
	}

	if (-1 == pt.x && -1 == pt.y)
	{
		RECT itemRect;
		int selected = ListView_GetNextItem(hwndDlg, -1, LVNI_ALL | LVNI_SELECTED);
		ListView_GetItemRect(hwndDlg, (selected != -1 ? selected : 0), &itemRect, LVIR_BOUNDS);
		pt.x = itemRect.left;
		pt.y = itemRect.top;
		MapWindowPoints(hwndDlg, HWND_DESKTOP, (POINT*)&pt, 1);
	}

	int r = Menu_TrackSkinnedPopup(menu, TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY, pt.x, pt.y, hwndDlg, NULL);
	if(num > 1) DeleteMenu(sendto,1,MF_BYPOSITION);
	for(int i = 1; i < num; i++) DeleteMenu(sendto, 100000+i, MF_BYCOMMAND);
	if (cloud_menu)
	{
		DeleteMenu(menu, (sendto ? 4 : 3), MF_BYPOSITION);
		DeleteMenu(menu, (sendto ? 3 : 2), MF_BYPOSITION);
		DestroyMenu(cloud_menu);
	}
	return r;
}

void handleContextMenuResult(int r, C_ItemList * items0=NULL, DeviceView * dev=NULL) {
	if(!dev) dev = currentViewedDevice;
	if(!dev) return;
	C_ItemList * items = items0;
	switch(r) {
		case ID_TRACKSLIST_PLAYSELECTION:
		case ID_TRACKSLIST_ENQUEUESELECTION:
		{
			if (!items) items = getSelectedItems();
			dev->PlayTracks(items, 0, r==ID_TRACKSLIST_ENQUEUESELECTION, true, hwndMediaView);
			if (!items0) delete items;
		}
		break;
		case ID_ADDTOPLAYLIST_NEWPLAYLIST:
		{
			int n = dev->CreatePlaylist();
			if (n == -1) return;
			r = 100000 + n;
			// the selected tracks will be added by the code at the end of this function
		}
		break;
		case ID_TRACKSLIST_SELECTALL:
		{
			int num = tracksList->listview.GetCount();
			for (int x = 0; x < num; x ++) tracksList->listview.SetSelected(x);
		}
		break;
		case ID_TRACKSLIST_EDITSELECTEDITEMS:
		{
			if (!items) items = getSelectedItems();
			editInfo(items, dev->dev, CENTER_OVER_ML_VIEW);
		}
		break;
		case ID_RATE_5:
		case ID_RATE_4:
		case ID_RATE_3:
		case ID_RATE_2:
		case ID_RATE_1:
		case ID_RATE_0:
		{
			if (!items) items = getSelectedItems();
			for(int i = 0; i<items->GetSize(); i++)
				dev->dev->setTrackRating((songid_t)items->Get(i), ID_RATE_0 - r);
			if (tracksList) tracksList->UpdateList(true);
			dev->DevicePropertiesChanges();
		}
		break;
		case ID_TRACKSLIST_DELETE:
		{
			wchar_t buf[256] = {0};
			if (!items) items = getSelectedItems();
			wsprintf(buf,WASABI_API_LNGSTRINGW((!currentViewedDevice->isCloudDevice ? IDS_PHYSICALLY_REMOVE_X_TRACKS : IDS_CLOUD_REMOVE_X_TRACKS)),items->GetSize());
			bool ckdev = (dev == currentViewedDevice);
			wchar_t titleStr[32] = {0};
			if (MessageBox(hwndMediaView, buf, WASABI_API_LNGSTRINGW_BUF(IDS_ARE_YOU_SURE, titleStr, 32), MB_YESNO | MB_ICONQUESTION) == IDYES) {
				if (ckdev && !currentViewedDevice) return;
				dev->DeleteTracks(items, CENTER_OVER_ML_VIEW);
				if (aacontents && dev == currentViewedDevice) { // full artist-album refresh
					GetDlgItemText(hwndMediaView, IDC_QUICKSEARCH, buf, sizeof(buf)/sizeof(wchar_t));
					// TODO async
					if (aacontents) aacontents->SetSearch(buf);
					GetDlgItemText(hwndMediaView, IDC_REFINE, buf, sizeof(buf)/sizeof(wchar_t));
					if (aacontents) aacontents->SetRefine(buf);
					if (artistList) artistList->UpdateList();
					if (albumList) albumList->UpdateList();
					if (albumList2) albumList2->UpdateList();
				}
				if (tracksList) tracksList->UpdateList();
				dev->DevicePropertiesChanges();
				UpdateStatus(hwndMediaView, true);
			}
		}
		break;
		case ID_TRACKSLIST_COPYTOLIBRARY:
		{
			if (!items) items = getSelectedItems();
			dev->CopyTracksToHardDrive(items);
		}
		break;
	}

	if(r > 100000) { // add songs to existing playlist
		int num = dev->dev->getPlaylistCount();
		if(r < num + 100000) {
			r-=100000;
			if(!items) items = getSelectedItems();
			for(int i = 0; i < items->GetSize(); i++) dev->dev->addTrackToPlaylist(r,(songid_t)items->Get(i));
			dev->DevicePropertiesChanges();
		}
	}
	else if (r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_UPPER) { // deals with cloud specific menus
		if (!items) items = getSelectedItems();
		int ret = dev->dev->extraActions(DEVICE_DO_CLOUD_SOURCES_MENU, (intptr_t)r, items->GetSize(), (intptr_t)items);
		// only send a removal from the view if plug-in says so
		if (ret) SendMessage(hwndMediaView, WM_USER+1, (WPARAM)items->Get(0), ret);
	}

	if (!items0) delete items;
}

void localizeFilter(const wchar_t *f, wchar_t *buf, int len) {
	int r=0;
	if(!_wcsicmp(f,L"Artist")) r = IDS_ARTIST;
	else if(!_wcsicmp(f,L"Album")) r = IDS_ALBUM;
	else if(!_wcsicmp(f,L"Genre")) r = IDS_GENRE;
	else if(!_wcsicmp(f,L"Artist Index")) r = IDS_ARTIST_INDEX;
	else if(!_wcsicmp(f,L"Year")) r = IDS_YEAR;
	else if(!_wcsicmp(f,L"Album Artist")) r = IDS_ALBUM_ARTIST;
	else if(!_wcsicmp(f,L"Publisher")) r = IDS_PUBLISHER;
	else if(!_wcsicmp(f,L"Composer")) r = IDS_COMPOSER;
	else if(!_wcsicmp(f,L"Album Artist Index")) r = IDS_ALBUM_ARTIST_INDEX;
	else if(!_wcsicmp(f,L"Album Art")) r = IDS_ALBUM_ART;
	else {buf[0]=0; return;}
	WASABI_API_LNGSTRINGW_BUF(r,buf,len);
}

wchar_t *GetDefFilter(int i,int n) {
	if(n==2) i++;
	if(i==0) return L"Genre";
	if(i==1) return L"Artist";
	return L"Album";
}

static __forceinline BYTE pm(int a, int b) {
	return (BYTE) ((a * b) / 0xff);
}

extern svc_imageLoader *GetPngLoaderService();

static ARGB32 *LoadPngResource(HINSTANCE module, const wchar_t *name, const wchar_t *type, 
							   BOOL premultily, int *width, int *height)
{
	svc_imageLoader *wasabiPngLoader;
	HRSRC resource;
	HANDLE resourceHandle;
	ARGB32 *result;

	if (NULL == WASABI_API_MEMMGR)
		return NULL;

	wasabiPngLoader = GetPngLoaderService();
	if (NULL == wasabiPngLoader)
		return NULL;

	resource = FindResourceW(module, name, type);
	if (NULL == resource) 
		return NULL;

	result = NULL;

	resourceHandle = LoadResource(module, resource);
	if (NULL != resourceHandle)
	{
		unsigned long resourceSize = SizeofResource(module, resource);
		if (0 !=  resourceSize)
		{
			void *resourceData = LockResource(resourceHandle);
			if (NULL != resourceData)
			{
				result = (FALSE != premultily) ?
							wasabiPngLoader->loadImage(resourceData, resourceSize, width, height) : 
							wasabiPngLoader->loadImageData(resourceData, resourceSize, width, height);
			}
		}
	}

	return result;
}

void DeleteSkinBitmap(SkinBitmap *skinBitmap)
{
	void *bits;
	if (NULL == skinBitmap)
		return;
			
	bits = skinBitmap->getBits();
	if (NULL != bits)
	{
		if (NULL != WASABI_API_MEMMGR)
			WASABI_API_MEMMGR->sysFree(bits);
	}

	delete skinBitmap;
}

BOOL SetToolbarButtonBitmap(HWND hwnd, int controlId, const wchar_t *resourceName, ARGB32 fc) 
{
	int width, height;
	ARGB32 *data, *x, *end;
	BYTE r, g, b;
	SkinBitmap *sbm, *old;
	HWND controlWindow;
	
	controlWindow = GetDlgItem(hwnd, controlId);
	if (NULL == controlWindow)
		return FALSE;

	data = LoadPngResource(plugin.hDllInstance, resourceName, RT_RCDATA, FALSE, &width, &height);
	if (NULL == data)
		return FALSE;
	
	r = (BYTE)(fc & 0x00ff0000 >> 16);
	g = (BYTE)(fc & 0x0000ff00 >> 8);
	b = (BYTE)(fc & 0x000000ff);

	x = data;
	end = data + width*height;

	while(x < end) 
	{
		BYTE a = (BYTE)(~(*x))&0xff;
		*(x++) = (a<<24) | (pm(r,a)<<16) | (pm(g,a)<<8) | pm(b,a);
	}

	sbm = new SkinBitmap(data, width, height);
	old = (SkinBitmap*)SetWindowLongPtr(controlWindow, GWLP_USERDATA,(LONG_PTR)sbm);
	DeleteSkinBitmap(old);
	InvalidateRect(controlWindow, NULL, TRUE);

	return TRUE;
}

typedef struct { int id, id2; } hi;

void do_help(HWND hwnd, UINT id, HWND hTooltipWnd)
{
	RECT r;
	POINT p;
	GetWindowRect(GetDlgItem(hwnd, id), &r);
	GetCursorPos(&p);
	if (p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom)
	{}
	else
	{
		r.left += r.right;
		r.left /= 2;
		r.top += r.bottom;
		r.top /= 2;
		SetCursorPos(r.left, r.top);
	}
	SendMessage(hTooltipWnd, TTM_SETDELAYTIME, TTDT_INITIAL, 0);
	SendMessage(hTooltipWnd, TTM_SETDELAYTIME, TTDT_RESHOW, 0);
}

#define C_BLAH
#define DO_HELP()	\
static HWND hTooltipWnd;	\
C_BLAH	\
if (uMsg == WM_HELP) {		\
	HELPINFO *hi=(HELPINFO *)(lParam); \
	if (hi->iContextType == HELPINFO_WINDOW) { do_help(hwndDlg,hi->iCtrlId,hTooltipWnd);}	\
	return TRUE;	\
} \
if (uMsg == WM_NOTIFY) { LPNMHDR t=(LPNMHDR)lParam; if (t->code == TTN_POP) { SendMessage(hTooltipWnd,TTM_SETDELAYTIME,TTDT_INITIAL,1000); SendMessage(hTooltipWnd,TTM_SETDELAYTIME,TTDT_RESHOW,1000);  } }	\
if (uMsg == WM_DESTROY && IsWindow(hTooltipWnd)) { DestroyWindow(hTooltipWnd); hTooltipWnd=NULL; }	\
if (uMsg == WM_INITDIALOG) {	\
	int x; \
	hTooltipWnd = CreateWindow(TOOLTIPS_CLASS,(LPCWSTR)NULL,TTS_ALWAYSTIP|TTS_NOPREFIX, \
								CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, hwndDlg,NULL,GetModuleHandle(NULL),NULL);	\
	SendMessage(hTooltipWnd,TTM_SETMAXTIPWIDTH,0,587); \
	SendMessage(hTooltipWnd,TTM_SETDELAYTIME,TTDT_INITIAL,250);	\
	SendMessage(hTooltipWnd,TTM_SETDELAYTIME,TTDT_RESHOW,500);	\
	for (x = 0; x < sizeof(helpinfo)/sizeof(helpinfo[0]); x ++) { \
		TOOLINFO ti; ti.cbSize = sizeof(ti); ti.uFlags = TTF_SUBCLASS|TTF_IDISHWND;	\
		ti.uId=(UINT_PTR)GetDlgItem(hwndDlg,helpinfo[x].id); ti.hwnd=hwndDlg; ti.lpszText=WASABI_API_LNGSTRINGW(helpinfo[x].id2);	\
		SendMessage(hTooltipWnd,TTM_ADDTOOL,0,(LPARAM) &ti);	\
	}	\
}

static BOOL ListView_OnCustomDraw(HWND hwndDlg, NMLVCUSTOMDRAW *plvcd, LRESULT *pResult)
{
	static BOOL bDrawFocus;
	static RECT rcView;
	static CLOUDCOLUMNPAINT cloudColumnPaint;

	*pResult = CDRF_DODEFAULT;

	switch (plvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult |= CDRF_NOTIFYITEMDRAW;
			CopyRect(&rcView, &plvcd->nmcd.rc);

			cloudColumnPaint.hwndList = plvcd->nmcd.hdr.hwndFrom;
			cloudColumnPaint.hdc = plvcd->nmcd.hdc;
			cloudColumnPaint.prcView = &rcView;
		return TRUE;

		case CDDS_ITEMPREPAINT:
			*pResult |= CDRF_NOTIFYSUBITEMDRAW;
			bDrawFocus = (CDIS_FOCUS & plvcd->nmcd.uItemState);
			if (bDrawFocus)
			{
				plvcd->nmcd.uItemState &= ~CDIS_FOCUS;
				*pResult |= CDRF_NOTIFYPOSTPAINT;
			}
		return TRUE;

		case CDDS_ITEMPOSTPAINT:
			if (bDrawFocus)
			{
				RECT rc;
				rc.left = LVIR_BOUNDS;
				SendMessageW(plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMRECT, plvcd->nmcd.dwItemSpec, (LPARAM)&rc);
				rc.left += 3;
				DrawFocusRect(plvcd->nmcd.hdc, &rc);
				plvcd->nmcd.uItemState |= CDIS_FOCUS;
				bDrawFocus = FALSE;
			}
			*pResult = CDRF_SKIPDEFAULT;
		return TRUE;

		case(CDDS_SUBITEM | CDDS_ITEMPREPAINT):
			{
				if (aacontents && aacontents->bgThread_Handle || (0 == plvcd->iSubItem && 0 == plvcd->nmcd.rc.right)) break;
				cloudColumnPaint.iItem = plvcd->nmcd.dwItemSpec;
				cloudColumnPaint.iSubItem = plvcd->iSubItem;

				if (plvcd->nmcd.hdr.idFrom == IDC_LIST_TRACKS)
				{
					if (plvcd->iSubItem == tracks->cloudcol)
					{
						cloudColumnPaint.value = tracks->cloud_cache[plvcd->nmcd.dwItemSpec];
					}
					else
						break;
				}
				else if (plvcd->nmcd.hdr.idFrom == IDC_LIST_ARTIST)
				{
					if (plvcd->iSubItem == aacontents->GetFilterList(0)->cloudcol)
					{
						wchar_t buf[16] = {0};
						aacontents->GetFilterList(0)->GetCellText(plvcd->nmcd.dwItemSpec, plvcd->iSubItem, buf, 16);
						cloudColumnPaint.value = _wtoi(buf);
					}
					else
						break;
				}
				else if (plvcd->nmcd.hdr.idFrom == IDC_LIST_ALBUM)
				{
					if (plvcd->iSubItem == aacontents->GetFilterList(1)->cloudcol)
					{
						wchar_t buf[16] = {0};
						aacontents->GetFilterList(1)->GetCellText(plvcd->nmcd.dwItemSpec, plvcd->iSubItem, buf, 16);
						cloudColumnPaint.value = _wtoi(buf);
					}
					else
						break;
				}
				else if (plvcd->nmcd.hdr.idFrom == IDC_LIST_ALBUM2)
				{
					if (plvcd->iSubItem == aacontents->GetFilterList(2)->cloudcol)
					{
						wchar_t buf[16] = {0};
						aacontents->GetFilterList(2)->GetCellText(plvcd->nmcd.dwItemSpec, plvcd->iSubItem, buf, 16);
						cloudColumnPaint.value = _wtoi(buf);
					}
					else
						break;
				}

				cloudColumnPaint.prcItem = &plvcd->nmcd.rc;
				cloudColumnPaint.rgbBk = plvcd->clrTextBk;
				cloudColumnPaint.rgbFg = plvcd->clrText;

				if (MLCloudColumn_Paint(plugin.hwndLibraryParent, &cloudColumnPaint))
				{
					*pResult = CDRF_SKIPDEFAULT;
					return TRUE;
				}
			}
		break;
	}
	return FALSE;
}

void pmp_common_UpdateButtonText(HWND hwndDlg, int _enqueuedef)
{
	if (groupBtn)
	{
		switch(_enqueuedef)
		{
			case 1:
				SetDlgItemTextW(hwndDlg, IDC_BUTTON_PLAY, view.enqueue);
				customAllowed = FALSE;
			break;

			default:
				// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
				//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
				pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK_IN_USE, (INT_PTR)_enqueuedef, 0, 0};

				wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
				if (pszTextW && pszTextW[0] != 0)
				{
					// set this to be a bit different so we can just use one button and not the
					// mixable one as well (leaving that to prevent messing with the resources)
					SetDlgItemTextW(hwndDlg, IDC_BUTTON_PLAY, pszTextW);
					customAllowed = TRUE;
				}
				else
				{
					SetDlgItemTextW(hwndDlg, IDC_BUTTON_PLAY, view.play);
					customAllowed = FALSE;
				}
			break;
		}
	}
}


enum
{
	BPM_ECHO_WM_COMMAND=0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND = 0x2, // just send WM_COMMAND
};

BOOL pmp_common_ButtonPopupMenu(HWND hwndDlg, int buttonId, HMENU menu, int flags=0)
{
	RECT r;
	HWND buttonHWND = GetDlgItem(hwndDlg, buttonId);
	GetWindowRect(buttonHWND, &r);
	MLSkinnedButton_SetDropDownState(buttonHWND, TRUE);
	UINT tpmFlags = TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN;
	if (!(flags & BPM_WM_COMMAND))
		tpmFlags |= TPM_RETURNCMD;
	int x = Menu_TrackSkinnedPopup(menu, tpmFlags, r.left, r.top, hwndDlg, NULL);
	if ((flags & BPM_ECHO_WM_COMMAND) && x)
		SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(x, 0), 0);
	MLSkinnedButton_SetDropDownState(buttonHWND, FALSE);
	return x;
}

static void pmp_common_PlayEnqueue(HWND hwndDlg, HWND from, UINT idFrom)
{
	HMENU listMenu = GetSubMenu(m_context_menus2, 0);
	int count = GetMenuItemCount(listMenu);
	if (count > 2)
	{
		for (int i = 2; i < count; i++)
		{
			DeleteMenu(listMenu, 2, MF_BYPOSITION);
		}
	}

	pmp_common_ButtonPopupMenu(hwndDlg, idFrom, listMenu, BPM_WM_COMMAND);
}

static BOOL restoreDone;
INT_PTR CALLBACK pmp_artistalbum_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	hi helpinfo[]={
		{IDC_BUTTON_ARTMODE,IDS_AUDIO_BUTTON_TT1},
		{IDC_BUTTON_VIEWMODE,IDS_AUDIO_BUTTON_TT2},
		{IDC_BUTTON_COLUMNS,IDS_AUDIO_BUTTON_TT3},
	};
	DO_HELP();

	if(hwndMediaView != hwndDlg && uMsg != WM_INITDIALOG) return 0;
	if (wad_handleDialogMsgs) { BOOL a=wad_handleDialogMsgs(hwndDlg,uMsg,wParam,lParam); if (a) return a; }
	if (artistList) { BOOL a=artistList->DialogProc(hwndDlg,uMsg,wParam,lParam); if(a) return a; }
	if (albumList) { BOOL a=albumList->DialogProc(hwndDlg,uMsg,wParam,lParam); if(a) return a; }
	if (albumList2) { BOOL a=albumList2->DialogProc(hwndDlg,uMsg,wParam,lParam); if(a) return a; }
	if (tracksList) { BOOL a=tracksList->DialogProc(hwndDlg,uMsg,wParam,lParam); if(a) return a; }

	switch (uMsg) {
		case WM_INITDIALOG:
		{
			if (!view.play)
			{
				SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view);
			}

			groupBtn = gen_mlconfig->ReadInt(L"groupbtn", 1);
			enqueuedef = (gen_mlconfig->ReadInt(L"enqueuedef", 0) == 1);

			// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
			//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
			pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG(IDC_BUTTON_CUSTOM, IDC_BUTTON_ENQUEUE), (INT_PTR)L"ml_pmp"};
			wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
			if (pszTextW && pszTextW[0] != 0)
			{
				// set this to be a bit different so we can just use one button and not the
				// mixable one as well (leaving that to prevent messing with the resources)
				customAllowed = TRUE;
				SetDlgItemTextW(hwndDlg, IDC_BUTTON_CUSTOM, pszTextW);
			}
			else
			{
				customAllowed = FALSE;
			}

			restoreDone = FALSE;
			playmode = L"viewplaymode";
			hwndMediaView = hwndDlg;
			*(void **)&wad_handleDialogMsgs=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,2,ML_IPC_SKIN_WADLG_GETFUNC);
			*(void **)&wad_DrawChildWindowBorders=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,3,ML_IPC_SKIN_WADLG_GETFUNC);

			if (!lstrcmpiA(currentViewedDevice->GetName(), "all_sources"))
				header = 1;
			else
				header = currentViewedDevice->config->ReadInt(L"header",0);

			numFilters = currentViewedDevice->config->ReadInt(L"media_numfilters",2);
			if(numFilters != 3 && numFilters != 2) numFilters=2;

			wchar_t filters[300] = {0}, *filtersp[3] = {0};
			bool artfilter[3]={false,false,false};
			for(int i = 0; i < numFilters; i++) {
				filtersp[i]=&filters[i*100];
				wchar_t name[20] = {0};
				wsprintf(name,L"media_filter%d",i);
				lstrcpyn(filtersp[i],currentViewedDevice->config->ReadString(name,GetDefFilter(i,numFilters)),100);
				if(!_wcsicmp(filtersp[i],L"Album Art")) artfilter[i]=true;
			}
			aacontents = new ArtistAlbumLists(currentViewedDevice->dev, currentViewedPlaylist, currentViewedDevice->config, filtersp,
											  numFilters, (currentViewedDevice->videoView ? 0 : -1), (!!currentViewedDevice->isCloudDevice));

			if (!artfilter[0]) artistList = new SkinnedListView(aacontents->GetFilterList(0),IDC_LIST_ARTIST,plugin.hwndLibraryParent, hwndDlg, false);
			else artistList = new AlbumArtListView(aacontents->GetFilterList(0),IDC_LIST_ARTIST,plugin.hwndLibraryParent, hwndDlg, false);
			artistList->DialogProc(hwndDlg,WM_INITDIALOG,0,0);
			artistList->InitializeFilterData(0, currentViewedDevice->config);

			if (!artfilter[1]) albumList = new SkinnedListView(aacontents->GetFilterList(1),IDC_LIST_ALBUM,plugin.hwndLibraryParent, hwndDlg, false);
			else albumList = new AlbumArtListView(aacontents->GetFilterList(1),IDC_LIST_ALBUM,plugin.hwndLibraryParent, hwndDlg, false);
			albumList->DialogProc(hwndDlg,WM_INITDIALOG,0,0);
			albumList->InitializeFilterData(1, currentViewedDevice->config);

			if (numFilters == 3) {
				if(!artfilter[2]) albumList2 = new SkinnedListView(aacontents->GetFilterList(2),IDC_LIST_ALBUM2,plugin.hwndLibraryParent, hwndDlg, false);
				else albumList2 = new AlbumArtListView(aacontents->GetFilterList(2),IDC_LIST_ALBUM2,plugin.hwndLibraryParent, hwndDlg, false);
				albumList2->DialogProc(hwndDlg,WM_INITDIALOG,0,0);
				albumList2->InitializeFilterData(2, currentViewedDevice->config);
			}

			if (currentViewedDevice->config->ReadInt(L"savefilter", 1))
			{
				SetDlgItemTextW(hwndDlg, IDC_QUICKSEARCH, currentViewedDevice->config->ReadString(L"savedfilter", L""));
				SetDlgItemTextW(hwndDlg, IDC_REFINE, currentViewedDevice->config->ReadString(L"savedrefinefilter", L""));
			}
			else
				restoreDone = TRUE;

			HWND list = GetDlgItem(hwndDlg, IDC_LIST_TRACKS);
			// TODO need to be able to change the order of the tracks items (so cloud is in a more appropriate place)
			//ListView_SetExtendedListViewStyleEx(list, LVS_EX_HEADERDRAGDROP, LVS_EX_HEADERDRAGDROP);
			ListView_SetExtendedListViewStyle(list, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);


			MLSKINWINDOW skin = {0};
			skin.hwndToSkin = list;
			skin.skinType = SKINNEDWND_TYPE_LISTVIEW;
			skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
			MLSkinWindow(plugin.hwndLibraryParent, &skin);

			skin.hwndToSkin = artistList->listview.getwnd();
			MLSkinWindow(plugin.hwndLibraryParent, &skin);
			skin.hwndToSkin = albumList->listview.getwnd();
			MLSkinWindow(plugin.hwndLibraryParent, &skin);
			if (numFilters == 3) {
				skin.hwndToSkin = albumList2->listview.getwnd();
				MLSkinWindow(plugin.hwndLibraryParent, &skin);
			}

			if(!currentViewedDevice->config->ReadInt(L"media_scroll_0",0))
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,artistList->skinlistview_handle,ML_IPC_LISTVIEW_DISABLEHSCROLL);
			if(!currentViewedDevice->config->ReadInt(L"media_scroll_1",0))
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,albumList->skinlistview_handle,ML_IPC_LISTVIEW_DISABLEHSCROLL);
			if(numFilters == 3 && !currentViewedDevice->config->ReadInt(L"media_scroll_2",0))
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,albumList2->skinlistview_handle,ML_IPC_LISTVIEW_DISABLEHSCROLL);

			if (aacontents)
			{
				artistList->UpdateList();
				tracks = aacontents->GetTracksList();
			}

			adiv1pos = numFilters == 3?33333:50000;
			adiv3pos = numFilters == 3?66667:0;
			adiv2pos = 50000;
			adiv1pos = currentViewedDevice->config->ReadInt(L"adiv1pos",adiv1pos);
			if(numFilters == 3) adiv3pos = currentViewedDevice->config->ReadInt(L"adiv3pos",adiv3pos);
			adiv2pos = currentViewedDevice->config->ReadInt(L"adiv2pos",adiv2pos);
			if(numFilters == 3 && adiv1pos>adiv3pos) {
				adiv1pos=33333;
				adiv3pos=66667;
			}
			AttachDivider(GetDlgItem(hwndDlg, IDC_VDELIM), TRUE, OnDividerMoved, IDC_VDELIM);
			if(numFilters == 3) AttachDivider(GetDlgItem(hwndDlg, IDC_VDELIM2), TRUE, OnDividerMoved, IDC_VDELIM2);
			AttachDivider(GetDlgItem(hwndDlg, IDC_HDELIM), FALSE, OnDividerMoved, IDC_HDELIM);

			int fieldsBits = (int)currentViewedDevice->dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
			if(!fieldsBits) fieldsBits = -1;
			if(!(fieldsBits & SUPPORTS_ALBUMART))
				SetWindowPos(GetDlgItem(hwndDlg,IDC_BUTTON_ARTMODE),0,0,0,0,0,0); // get rid of art mode button if we don't support albumart

			FLICKERFIX ff = {0};
			ff.mode = FFM_ERASEINPAINT;
			skin.skinType = SKINNEDWND_TYPE_AUTO;
			INT ffcl[] = { IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM,
						   IDC_BUTTON_CLEARSEARCH, IDC_BUTTON_CLEARREFINE,
						   IDC_BUTTON_EJECT, IDC_BUTTON_SYNC, IDC_BUTTON_AUTOFILL,
						   IDC_STATUS, IDC_SEARCH_TEXT, IDC_QUICKSEARCH, IDC_REFINE,
						   IDC_REFINE_TEXT, IDC_BUTTON_ARTMODE, IDC_BUTTON_VIEWMODE,
						   IDC_BUTTON_COLUMNS, 
						   // disabled cloud parts
						   /*IDC_HEADER_DEVICE_ICON, IDC_HEADER_DEVICE_NAME, IDC_HEADER_DEVICE_BAR,
						   IDC_HEADER_DEVICE_SIZE, IDC_HEADER_DEVICE_TRANSFER,*/
			};
			for (int i = 0; i < ARRAYSIZE(ffcl); i++)
			{
				ff.hwnd = GetDlgItem(hwndDlg, ffcl[i]);
				if (IsWindow(ff.hwnd))
				{
					SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&ff, ML_IPC_FLICKERFIX);

					// skip the mode buttons
					if (i < 13)
					{
						if (i < 3)
							skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | (groupBtn ? SWBS_SPLITBUTTON : 0);
						else
							skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
						skin.hwndToSkin = ff.hwnd;
						MLSkinWindow(plugin.hwndLibraryParent, &skin);
					}
				}
			}

			if (0 != currentViewedDevice->dev->extraActions(DEVICE_SYNC_UNSUPPORTED,0,0,0))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_SYNC), FALSE);

			skin.hwndToSkin = hwndDlg;
			skin.skinType = SKINNEDWND_TYPE_AUTO;
			skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
			MLSkinWindow(plugin.hwndLibraryParent, &skin);

			// do this now to get the cloud columns correctly known (doesn't work correctly if done before the skinning is setup)
			MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(GetDlgItem(hwndDlg, IDC_LIST_TRACKS)), tracks->cloudcol);
			MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(GetDlgItem(hwndDlg, IDC_LIST_ARTIST)), artistList->contents->cloudcol);
			MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(GetDlgItem(hwndDlg, IDC_LIST_ALBUM)), albumList->contents->cloudcol);
			if (numFilters == 3) MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(GetDlgItem(hwndDlg, IDC_LIST_ALBUM2)), albumList2->contents->cloudcol);

			g_displayrefine = !!(gen_mlconfig->ReadInt(L"audiorefine", 0));
			adiv1_nodraw=0;
			adiv3_nodraw=0;
			m_nodrawtopborders=0;
			PostMessage(hwndDlg, WM_DISPLAYCHANGE, 0, 0);
			break;
		}

		case WM_DISPLAYCHANGE:
		{
			int (*wad_getColor)(int idx);
			*(void **)&wad_getColor=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,1,ML_IPC_SKIN_WADLG_GETFUNC);
			ARGB32 fc = (wad_getColor?wad_getColor(WADLG_BUTTONFG):RGB(0xFF,0xFF,0xFF)) & 0x00FFFFFF;
			SetToolbarButtonBitmap(hwndDlg,IDC_BUTTON_ARTMODE, MAKEINTRESOURCE(IDR_TOOL_ALBUMART_ICON),fc);
			SetToolbarButtonBitmap(hwndDlg,IDC_BUTTON_VIEWMODE, MAKEINTRESOURCE(IDR_TOOL_VIEWMODE_ICON),fc);
			SetToolbarButtonBitmap(hwndDlg,IDC_BUTTON_COLUMNS, MAKEINTRESOURCE(IDR_TOOL_COLUMNS_ICON),fc);
			UpdateWindow(hwndDlg);
			LayoutWindows(hwndDlg, TRUE, 0);
		}
		break;

		case WM_DROPFILES:
		return currentViewedDevice->TransferFromDrop((HDROP)wParam);

		case WM_DESTROY:
			if (aacontents) aacontents->bgQuery_Stop();
			currentViewedDevice->config->WriteInt(L"adiv1pos",adiv1pos);
			if(numFilters == 3) currentViewedDevice->config->WriteInt(L"adiv3pos",adiv3pos);
			currentViewedDevice->config->WriteInt(L"adiv2pos",adiv2pos);
			tracks=NULL;
			hwndMediaView=NULL;
			if (albumList) delete albumList; albumList=NULL;
			if (artistList) delete artistList; artistList=NULL;
			if (albumList2) delete albumList2; albumList2=NULL;
			if (aacontents) delete aacontents; aacontents=NULL;
			{
				SkinBitmap *s = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_ARTMODE),GWLP_USERDATA);
				DeleteSkinBitmap(s);
				s = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_VIEWMODE),GWLP_USERDATA);
				DeleteSkinBitmap(s);
				s = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_COLUMNS),GWLP_USERDATA);
				DeleteSkinBitmap(s);
			}
		break;

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
			if (di->CtlType == ODT_BUTTON) {
				if(di->CtlID == IDC_BUTTON_ARTMODE || di->CtlID == IDC_BUTTON_VIEWMODE || di->CtlID == IDC_BUTTON_COLUMNS)
				{ // draw the toolbar buttons!
					SkinBitmap* hbm = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,di->CtlID),GWLP_USERDATA);
					if(hbm && di->rcItem.left != di->rcItem.right && di->rcItem.top != di->rcItem.bottom) {
						DCCanvas dc(di->hDC);
						if (di->itemState & ODS_SELECTED) hbm->blitAlpha(&dc,di->rcItem.left+6,di->rcItem.top+5);
						else hbm->blitAlpha(&dc,di->rcItem.left+4,di->rcItem.top+3);
					}
				}
			}
		}
		break;

		case WM_SETCURSOR:
		case WM_LBUTTONDOWN:
		{
			static INT id[] = { IDC_VDELIM, IDC_HDELIM, IDC_VDELIM2 };
			RECT rw;
			POINT pt;

			GetCursorPos(&pt);
			for (INT i = 0; i < sizeof(id)/sizeof(INT); i++)
			{
				HWND hwndDiv = GetDlgItem(hwndDlg, id[i]);
				if (!hwndDiv) continue;

				GetWindowRect(hwndDiv, &rw);
				if (PtInRect(&rw, pt))
				{
					if (WM_SETCURSOR == uMsg)
					{
						SetCursor(LoadCursor(NULL, (IDC_VDELIM == id[i] || IDC_VDELIM2 == id[i]) ? IDC_SIZEWE : IDC_SIZENS));
						return TRUE;
					}
					else
					{
						SendMessage(hwndDiv, uMsg, wParam, MAKELPARAM(pt.x - rw.left, pt.y - rw.top)); 
						return TRUE; 
					}
				}
			}
		}
		break;

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags), 0);
			}
		return TRUE;

		case WM_PAINT:
		{
			int tab[] = {(m_nodrawtopborders==2)?0:(IDC_LIST_TRACKS|DCW_SUNKENBORDER),
						IDC_QUICKSEARCH|DCW_SUNKENBORDER,
						(refineHidden!=0)?0:(IDC_REFINE|DCW_SUNKENBORDER),
						IDC_HDELIM|DCW_DIVIDER,
						(!header?IDC_HDELIM2|DCW_DIVIDER:0),
						IDC_VDELIM|DCW_DIVIDER,
						numFilters == 3?IDC_VDELIM2|DCW_DIVIDER:0,
						adiv1_nodraw==1?0:(IDC_LIST_ARTIST|DCW_SUNKENBORDER),
						(adiv1_nodraw==2 || adiv3_nodraw==1)?0:(IDC_LIST_ALBUM|DCW_SUNKENBORDER),
						(numFilters != 3 || adiv3_nodraw==2)?0:(IDC_LIST_ALBUM2|DCW_SUNKENBORDER)};
			int size = sizeof(tab) / sizeof(tab[0]);
			// do this to prevent drawing parts when the views are collapsed
			if (m_nodrawtopborders==1) size -= 6;
			if (wad_DrawChildWindowBorders) wad_DrawChildWindowBorders(hwndDlg,tab,size);
		}
			return TRUE;

		case WM_ERASEBKGND:
			return 1;

		case WM_USER:
		{
			if (aacontents && aacontents->bgThread_Handle) break;

			if (aacontents)
			{
				SkinnedListView * lists[3]={artistList,albumList,albumList2};
				aacontents->SelectionChanged(wParam==IDC_LIST_ARTIST?0:(wParam==IDC_LIST_ALBUM?1:2),lists);
			}
			noSearchTimer=true;
			SetDlgItemText(hwndDlg,IDC_REFINE,L"");
			noSearchTimer=false;
			UpdateStatus(hwndDlg);
			wParam=0;
		}
		// run through
		case WM_USER+2:
			if (tracksList) tracksList->UpdateList(wParam==1);
			break;

		case WM_USER+1:
			if (tracks) tracks->RemoveTrack((songid_t)wParam);
			if (aacontents) aacontents->RemoveTrack((songid_t)wParam);
			break;

		case WM_NOTIFY:
		{
			LPNMHDR l=(LPNMHDR)lParam;
			if(l->idFrom==IDC_LIST_ARTIST || l->idFrom==IDC_LIST_ALBUM || l->idFrom==IDC_LIST_ALBUM2)
			{
				switch(l->code) {
					case LVN_ITEMCHANGED:
					{
						LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
						if ((lv->uNewState ^ lv->uOldState) & LVIS_SELECTED) {
							PostMessage(hwndDlg,WM_USER,l->idFrom,0);
						}
					}
						break;
					case NM_RETURN:
					case NM_DBLCLK: // play some songs!
					{
						C_ItemList * items = getSelectedItems(true);
						currentViewedDevice->PlayTracks(items, 0, (!(GetAsyncKeyState(VK_SHIFT)&0x8000) ? (enqueuedef == 1) : (enqueuedef != 1)), false);
						delete items;
						break;
					}
					case LVN_KEYDOWN:
						switch(((LPNMLVKEYDOWN)lParam)->wVKey) {
							case VK_DELETE:
							{
								if(!GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									handleContextMenuResult(ID_TRACKSLIST_DELETE);
								}
							}
							break;
							case 0x43: //C
								if(GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									handleContextMenuResult(ID_TRACKSLIST_COPYTOLIBRARY);
								}
							break;
						}
						break;
				}
			}
			else if(l->idFrom == IDC_LIST_TRACKS)
			{
				switch(l->code) {
					case LVN_KEYDOWN:
						switch(((LPNMLVKEYDOWN)lParam)->wVKey) {
							case VK_DELETE:
							{
								if(!GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									handleContextMenuResult(ID_TRACKSLIST_DELETE);
								}
							}
							break;
							case 0x45: //E
								bool noEdit = currentViewedDevice->dev->extraActions(DEVICE_DOES_NOT_SUPPORT_EDITING_METADATA,0,0,0)!=0;
								if(!noEdit && GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									C_ItemList * items = getSelectedItems();
									editInfo(items,currentViewedDevice->dev, CENTER_OVER_ML_VIEW);
									delete items;
								}
							break;
						}
						break;
				}
			}
		}
			break;

		case WM_APP + 3: // send by bgthread
			if (wParam == 0x69)
			{
				if (aacontents)
				{
					aacontents->bgQuery_Stop();
					tracks = aacontents->GetTracksList();

					if (artistList) artistList->UpdateList();
					if (albumList) albumList->UpdateList();
					if (albumList2) albumList2->UpdateList();
					if (tracksList) tracksList->UpdateList();
					UpdateStatus(hwndDlg, true);
				}
			}
			break;

		case WM_APP + 104:
		{
			pmp_common_UpdateButtonText(hwndDlg, wParam);
			LayoutWindows(hwndDlg, TRUE, 0);
			return 0;
		}

		case WM_TIMER:
			switch(wParam) {
				case 123:
				{
					if (aacontents && aacontents->bgThread_Handle)
					{
						HWND hwndList;
						hwndList = tracksList->listview.getwnd();
						if (1 != ListView_GetItemCount(hwndList)) ListView_SetItemCountEx(hwndList, 1, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
						ListView_RedrawItems(hwndList, 0, 0);
					}
				}
				break;

				case 400:
					KillTimer(hwndDlg,400);
					currentViewedDevice->config->WriteInt(L"adiv1pos",adiv1pos);
					if(numFilters == 3) currentViewedDevice->config->WriteInt(L"adiv3pos",adiv3pos);
					currentViewedDevice->config->WriteInt(L"adiv2pos",adiv2pos);
				break;

				case 500:
				{
					KillTimer(hwndDlg,500);
					wchar_t buf[256]=L"";
					GetDlgItemText(hwndDlg,IDC_QUICKSEARCH,buf,sizeof(buf)/sizeof(wchar_t));
					noSearchTimer=true;
					if (restoreDone) SetDlgItemText(hwndDlg,IDC_REFINE,L"");
					noSearchTimer=false;
					aacontents->SetSearch(buf, (!!currentViewedDevice->isCloudDevice));
					if (!aacontents->bgThread_Handle)
					{
						if (artistList) artistList->UpdateList();
						if (albumList) albumList->UpdateList();
						if (albumList2) albumList2->UpdateList();
						if (tracksList) tracksList->UpdateList();
						UpdateStatus(hwndDlg);
					}

					if (!restoreDone)
					{
						restoreDone = TRUE;
						KillTimer(hwndDlg,501);
						SetTimer(hwndDlg,501,250,NULL);
					}
				}
				break;

				case 501:
				{
					KillTimer(hwndDlg,501);
					wchar_t buf[256]=L"";
					GetDlgItemText(hwndDlg,IDC_REFINE,buf,sizeof(buf)/sizeof(wchar_t));
					aacontents->SetRefine(buf, (!!currentViewedDevice->isCloudDevice));
					if (!aacontents->bgThread_Handle)
					{
						if (tracksList) tracksList->UpdateList();
						UpdateStatus(hwndDlg);
					}
				}
				break;
			}
			break;

		case WM_MOUSEMOVE:
			if(wParam==MK_LBUTTON) {
				if(GetCapture() == hwndDlg) ReleaseCapture();
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_BUTTON_ARTMODE:
				{
					int changed=0;
					for(int i=0; i<3; i++) {
						wchar_t name[20] = {0};
						wsprintf(name,L"media_filter%d",i);
						wchar_t * str = currentViewedDevice->config->ReadString(name,GetDefFilter(i,numFilters));
						if(!_wcsicmp(str,L"Album")) { currentViewedDevice->config->WriteString(name,L"Album Art"); changed=1; }
						else if(!_wcsicmp(str,L"Album Art")) { currentViewedDevice->config->WriteString(name,L"Album"); changed=1; }
					}
					if (changed) PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
				}
					break;
				case IDC_BUTTON_VIEWMODE:
				{
					struct {
						const wchar_t *name;
						int numfilters;
						wchar_t* f[3];
						int requiredFields;
					} presets[] = {
									{0,2,{L"Artist",L"Album"},0},
									{0,2,{L"Artist",L"Album Art"},SUPPORTS_ALBUMART},
									{0,2,{L"Album Artist",L"Album"},SUPPORTS_ALBUMARTIST},
									{0,2,{L"Album Artist",L"Album Art"},SUPPORTS_ALBUMARTIST | SUPPORTS_ALBUMART},
									{0,3,{L"Genre",L"Artist",L"Album"},SUPPORTS_GENRE | SUPPORTS_ALBUMART},
									{0,2,{L"Genre",L"Album Art"},SUPPORTS_GENRE},
									{0,3,{L"Year",L"Artist",L"Album"},SUPPORTS_YEAR},
									{0,2,{L"Composer",L"Album"},SUPPORTS_COMPOSER},
									{0,3,{L"Publisher",L"Artist",L"Album"},SUPPORTS_PUBLISHER},
					};
					HMENU menu = CreatePopupMenu();
					int fieldsBits = (int)currentViewedDevice->dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
					if(!fieldsBits) fieldsBits = -1;
					bool checked=false;

					wchar_t filters[300] = {0};
					wchar_t *filtersp[3] = {0};
					for(int i=0; i<numFilters; i++) {
						filtersp[i]=&filters[i*100];
						wchar_t name[20] = {0};
						wsprintf(name,L"media_filter%d",i);
						lstrcpyn(filtersp[i],currentViewedDevice->config->ReadString(name,GetDefFilter(i,numFilters)),100);
					}

					for(int i=0; i < sizeof(presets)/sizeof(presets[0]); i++) {
						if(!presets[i].requiredFields || (presets[i].requiredFields & fieldsBits) == presets[i].requiredFields) {
							wchar_t buf[350] = {0};
							if(!presets[i].name) {
								wchar_t a[100] = {0}, b[100] = {0}, c[100] = {0};
								localizeFilter(presets[i].f[0],a,100);
								localizeFilter(presets[i].f[1],b,100);
								if(presets[i].numfilters == 3) localizeFilter(presets[i].f[2],c,100);
								if(presets[i].numfilters == 3) wsprintf(buf,L"%s/%s/%s",a,b,c);
								else wsprintf(buf,L"%s/%s",a,b);
							}
							AppendMenu(menu,MF_STRING,i+1,presets[i].name?presets[i].name:buf);
							if(numFilters == presets[i].numfilters && !_wcsicmp(presets[i].f[0],filtersp[0]) && !_wcsicmp(presets[i].f[1],filtersp[1]) && (numFilters == 2 || !_wcsicmp(presets[i].f[2],filtersp[2])))
							{ // this is our view...
								CheckMenuItem(menu,i+1,MF_CHECKED);
								checked=true;
							}
						}
					}
					AppendMenu(menu,MF_STRING,0x4000,WASABI_API_LNGSTRINGW(IDS_OTHER2));
					if(!checked)
						CheckMenuItem(menu,0x4000,MF_CHECKED);
					RECT rc;
					GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON_VIEWMODE),&rc);
					int r = Menu_TrackSkinnedPopup(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,rc.left,rc.bottom,hwndDlg,NULL);
					DestroyMenu(menu);
					if(r==0) break;
					else if(r == 0x4000) {
						extern int g_prefs_openpage;
						g_prefs_openpage = (!currentViewedDevice->isCloudDevice ? 4 : 0);
						SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&currentViewedDevice->devPrefsPage,IPC_OPENPREFSTOPAGE);

						extern HWND m_hwndTab;
						if (IsWindow(m_hwndTab))
						{
							TabCtrl_SetCurSel(m_hwndTab, g_prefs_openpage);
							extern HWND OnSelChanged(HWND hwndDlg, HWND external = NULL, DeviceView *dev = NULL);
							OnSelChanged(GetParent(m_hwndTab));
						}
					}
					else {
						r--;
						for(int j=0; j<presets[r].numfilters; j++) {
							wchar_t name[20] = {0};
							wsprintf(name,L"media_filter%d",j);
							currentViewedDevice->config->WriteString(name,presets[r].f[j]);
						}
						currentViewedDevice->config->WriteInt(L"media_numfilters",presets[r].numfilters);
						PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
					}
				}
					break;
				case IDC_BUTTON_COLUMNS:
				{
					HMENU themenu1 = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);
					HMENU themenu2 = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);
					HMENU themenu3 = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);
					HMENU themenu4 = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);

					HMENU menu = CreatePopupMenu();
					MENUITEMINFO m={sizeof(m),MIIM_TYPE | MIIM_ID | MIIM_SUBMENU,MFT_STRING,0};
					wchar_t a[100] = {0}, b[100] = {0}, c[100] = {0};
					localizeFilter(currentViewedDevice->config->ReadString(L"media_filter0",GetDefFilter(0,numFilters)),a,100);
					localizeFilter(currentViewedDevice->config->ReadString(L"media_filter1",GetDefFilter(1,numFilters)),b,100);
					localizeFilter(currentViewedDevice->config->ReadString(L"media_filter2",GetDefFilter(2,numFilters)),c,100);
					wchar_t * d = WASABI_API_LNGSTRINGW(IDS_TRACKS);
					m.wID = 0;
					m.dwTypeData = a;
					m.hSubMenu = artistList->GetMenu(true,0,currentViewedDevice->config,themenu1);
					InsertMenuItem(menu,0,FALSE,&m);
					m.wID = 1;
					m.dwTypeData = b;
					m.hSubMenu = albumList->GetMenu(true,1,currentViewedDevice->config,themenu2);
					InsertMenuItem(menu,1,FALSE,&m);
					if(numFilters == 3) {
						m.wID = 2;
						m.dwTypeData = c;
						m.hSubMenu = albumList2->GetMenu(true,2,currentViewedDevice->config,themenu3);
						InsertMenuItem(menu,2,FALSE,&m);
					}
					m.wID = 3;
					m.dwTypeData = d;
					m.hSubMenu = tracksList->GetMenu(false,0,currentViewedDevice->config,themenu4);
					InsertMenuItem(menu,3,FALSE,&m);

					RECT rc;
					GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON_COLUMNS),&rc);
					int r = Menu_TrackSkinnedPopup(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,rc.left,rc.bottom,hwndDlg,NULL);

					artistList->ProcessMenuResult(r,true,0,currentViewedDevice->config,hwndDlg);
					albumList->ProcessMenuResult(r,true,1,currentViewedDevice->config,hwndDlg);
					if(numFilters == 3) albumList2->ProcessMenuResult(r,true,2,currentViewedDevice->config,hwndDlg);
					tracksList->ProcessMenuResult(r,false,0,currentViewedDevice->config,hwndDlg);

					DestroyMenu(menu);
					DestroyMenu(themenu1);
					DestroyMenu(themenu2);
					DestroyMenu(themenu3);
					DestroyMenu(themenu4);
				}
					break;
				case IDC_QUICKSEARCH:
					if (HIWORD(wParam) == EN_CHANGE && !noSearchTimer) {
						KillTimer(hwndDlg, 500);
						SetTimer(hwndDlg, 500, 350, NULL);

						HWND hwndList = artistList->listview.getwnd();
						if (IsWindow(hwndList))
						{
							ListView_SetItemCountEx(hwndList, 0, 0);
							ListView_RedrawItems(hwndList, 0, 0);
						}

						hwndList = albumList->listview.getwnd();
						if (IsWindow(hwndList))
						{
							ListView_SetItemCountEx(hwndList, 0, 0);
							ListView_RedrawItems(hwndList, 0, 0);
						}

						if (numFilters == 3)
						{
							hwndList = albumList2->listview.getwnd();
							if (IsWindow(hwndList))
							{
								ListView_SetItemCountEx(hwndList, 0, 0);
								ListView_RedrawItems(hwndList, 0, 0);
							}
						}
					}
					break;
				case IDC_REFINE:
				if (HIWORD(wParam) == EN_CHANGE && !noSearchTimer) {
					KillTimer(hwndDlg,501);
					SetTimer(hwndDlg,501,250,NULL);
				}
					break;
				case IDC_BUTTON_CLEARSEARCH:
				{
					SetDlgItemText(hwndDlg,IDC_QUICKSEARCH,L"");
					break;
				}
				case IDC_BUTTON_CLEARREFINE:
					SetDlgItemText(hwndDlg,IDC_REFINE,L"");
					break;
				case IDC_HEADER_DEVICE_TRANSFER:
				{
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BUTTON_SYNC)))
					{
						MessageBox(hwndDlg, L"Transferring files from this device to another is not currently supported.",
											L"Cloud Transfer Not Supported", MB_ICONINFORMATION);
						break;
					}
				}
				case IDC_BUTTON_SYNC:
					if (!currentViewedDevice->isCloudDevice)
						currentViewedDevice->Sync();
					else
						currentViewedDevice->CloudSync();
					break;
				case IDC_BUTTON_AUTOFILL:
					currentViewedDevice->Autofill();
					break;
				case IDC_SEARCH_TEXT:
					if (HIWORD(wParam) == STN_DBLCLK)
					{
						// TODO decide what to do for the 'all_sources'
						if (currentViewedDevice->isCloudDevice &&
							lstrcmpiA(currentViewedDevice->GetName(), "all_sources"))
						{
							header = !header;
							currentViewedDevice->config->WriteInt(L"header",header);
							LayoutWindows(hwndMediaView, TRUE, 0);
							UpdateStatus(hwndDlg, true);
							ShowWindow(hwndDlg, 0);
							ShowWindow(hwndDlg, 1);
						}
					}
					break;
			}
			break;

		case WM_USER + 0x200:
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 1); // yes, we support no - redraw resize
			return TRUE;

		case WM_USER + 0x201:
			offsetX = (short)LOWORD(wParam);
			offsetY = (short)HIWORD(wParam);
			g_rgnUpdate = (HRGN)lParam;
			return TRUE;
	}
	return pmp_common_dlgproc(hwndDlg,uMsg,wParam,lParam);
}

static void getStars(int stars, wchar_t * buf, int buflen) {
	wchar_t * r=L"";
	switch(stars) {
	    case 1: r=L"\u2605"; break;
		case 2: r=L"\u2605\u2605"; break;
		case 3: r=L"\u2605\u2605\u2605"; break;
		case 4: r=L"\u2605\u2605\u2605\u2605"; break;
		case 5: r=L"\u2605\u2605\u2605\u2605\u2605"; break;
	}
	lstrcpyn(buf,r,buflen);
}

__inline void TimetToFileTime(time_t t, LPFILETIME pft)
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD) ll;
    pft->dwHighDateTime = ll >>32;
}

void timeToString(__time64_t time, wchar_t * buf, int buflen) {
	if((__int64)time<=0) { lstrcpyn(buf,L"",buflen); return; }

	FILETIME ft = {0};
	SYSTEMTIME st = {0};
	TimetToFileTime(time, &ft);
	FileTimeToSystemTime(&ft, &st);

	int adjust = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, buf, buflen);
	buf[adjust-1] = ' ';
	GetTimeFormat(LOCALE_USER_DEFAULT, NULL, &st, NULL, &buf[adjust], buflen - adjust);
}

extern void GetInfoString(wchar_t * buf, Device * dev, int numTracks, __int64 totalSize, int totalPlayLength, int cloud);

class PlaylistContents : public PrimaryListContents {
public:
	int playlistId;
	Device * dev;
	PlaylistContents(int playlistId, Device * dev) {
		this->playlistId = playlistId;
		this->dev = dev;
		int fieldsBits = (int)dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
		if(!fieldsBits) fieldsBits = -1;
		C_Config *c = currentViewedDevice->config;
		this->config = c;
		fields.Add(new ListField(-1,20, WASABI_API_LNGSTRINGW(IDS_NUMBER),c));
		if(fieldsBits & SUPPORTS_ARTIST)	fields.Add(new ListField(1000, 200,WASABI_API_LNGSTRINGW(IDS_ARTIST),c));
		if(fieldsBits & SUPPORTS_TITLE)		fields.Add(new ListField(1001, 200,WASABI_API_LNGSTRINGW(IDS_TITLE),c));
		if(fieldsBits & SUPPORTS_ALBUM)		fields.Add(new ListField(1002, 200,WASABI_API_LNGSTRINGW(IDS_ALBUM),c));
		if(fieldsBits & SUPPORTS_LENGTH)	fields.Add(new ListField(1003, 64, WASABI_API_LNGSTRINGW(IDS_LENGTH),c));
		if(fieldsBits & SUPPORTS_TRACKNUM)	fields.Add(new ListField(1004, 50, WASABI_API_LNGSTRINGW(IDS_TRACK_NUMBER),c));
		if(fieldsBits & SUPPORTS_DISCNUM)	fields.Add(new ListField(1005, 38, WASABI_API_LNGSTRINGW(IDS_DISC),c));
		if(fieldsBits & SUPPORTS_GENRE)		fields.Add(new ListField(1006, 100,WASABI_API_LNGSTRINGW(IDS_GENRE),c));
		if(fieldsBits & SUPPORTS_YEAR)		fields.Add(new ListField(1007, 38, WASABI_API_LNGSTRINGW(IDS_YEAR),c));
		if(fieldsBits & SUPPORTS_BITRATE)	fields.Add(new ListField(1008, 45, WASABI_API_LNGSTRINGW(IDS_BITRATE),c));
		if(fieldsBits & SUPPORTS_SIZE)		fields.Add(new ListField(1009, 90, WASABI_API_LNGSTRINGW(IDS_SIZE),c));
		if(fieldsBits & SUPPORTS_PLAYCOUNT)	fields.Add(new ListField(1010, 64, WASABI_API_LNGSTRINGW(IDS_PLAY_COUNT),c));
		if(fieldsBits & SUPPORTS_RATING)	fields.Add(new ListField(1011, 64, WASABI_API_LNGSTRINGW(IDS_RATING),c));
		if(fieldsBits & SUPPORTS_LASTPLAYED)	fields.Add(new ListField(1012, 100,WASABI_API_LNGSTRINGW(IDS_LAST_PLAYED),c));
		if(fieldsBits & SUPPORTS_ALBUMARTIST)	fields.Add(new ListField(1013, 200,WASABI_API_LNGSTRINGW(IDS_ALBUM_ARTIST),c,true));
		if(fieldsBits & SUPPORTS_PUBLISHER)	fields.Add(new ListField(1014, 200,WASABI_API_LNGSTRINGW(IDS_PUBLISHER),c,true));
		if(fieldsBits & SUPPORTS_COMPOSER)	fields.Add(new ListField(1015, 200,WASABI_API_LNGSTRINGW(IDS_COMPOSER),c,true));
		if(fieldsBits & SUPPORTS_MIMETYPE)	fields.Add(new ListField(1016, 100,WASABI_API_LNGSTRINGW(IDS_MIME_TYPE),c,true));
		if(fieldsBits & SUPPORTS_DATEADDED)	fields.Add(new ListField(1017, 100,WASABI_API_LNGSTRINGW(IDS_DATE_ADDED),c,true));
		this->SortColumns();
	}

	//virtual ~PlaylistContents() { }
	virtual int GetNumColumns() { return fields.GetSize(); }

	virtual int GetNumRows() { return dev->getPlaylistLength(playlistId); }

	virtual int GetColumnWidth(int num) {
		if(num >=0 && num < fields.GetSize())
			return ((ListField *)fields.Get(num))->width;
		return 0;
	}

	virtual wchar_t * GetColumnTitle(int num) {
		if(num >=0 && num < fields.GetSize())
			return ((ListField *)fields.Get(num))->name;
		return L"";
	}

	virtual void GetCellText(int row, int col, wchar_t * buf, int buflen) {
		if(col >=0 && col < fields.GetSize()) col = ((ListField *)fields.Get(col))->field;
		if(col != -1) col -= 1000;
		songid_t s = dev->getPlaylistTrack(playlistId,row);
		buf[0]=0;
		switch(col) {
			case -1: wsprintf(buf,L"%d",row+1); return;
			case 0: dev->getTrackArtist(s,buf,buflen); return;
			case 1: dev->getTrackTitle(s,buf,buflen); return;
			case 2: dev->getTrackAlbum(s,buf,buflen); return;
			case 3: { int l=dev->getTrackLength(s); if (l>=0) wsprintf(buf,L"%d:%02d",l/1000/60,(l/1000)%60); return; }
			case 4: { int d = dev->getTrackTrackNum(s); if(d>=0)  wsprintf(buf,L"%d",d); return; }
			case 5: { int d = dev->getTrackDiscNum(s); if(d>=0)  wsprintf(buf,L"%d",d); return; }
			case 6: dev->getTrackGenre(s,buf,buflen); return;
			case 7: { int d = dev->getTrackYear(s); if(d>0)  wsprintf(buf,L"%d",d); return; }
			case 8: { int d = dev->getTrackBitrate(s); if(d>0)  wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_KBPS),d); return; }
			case 9: WASABI_API_LNG->FormattedSizeString(buf, buflen, dev->getTrackSize(s)); return;
			case 10: { int d = dev->getTrackPlayCount(s); if(d>=0)  wsprintf(buf,L"%d",d); return; }
			case 11: getStars(dev->getTrackRating(s),buf,buflen); return;
			case 12: timeToString(dev->getTrackLastPlayed(s),buf,buflen); return;
			case 13: dev->getTrackAlbumArtist(s,buf,buflen); return;
			case 14: dev->getTrackPublisher(s,buf,buflen); return;
			case 15: dev->getTrackComposer(s,buf,buflen); return;
		}
	}

	virtual void GetInfoString(wchar_t * buf) {
		__int64 totalSize=0;
		int totalPlayLength=0;
		int millis=0;
		int l = dev->getPlaylistLength(playlistId);
		for(int i=0; i < l; i++) {
			songid_t s = dev->getPlaylistTrack(playlistId,i);
			totalSize += (__int64)dev->getTrackSize(s);
			int len=dev->getTrackLength(s);
			totalPlayLength += len/1000;
			millis += len%1000;
		}
		totalPlayLength += millis/1000;
		::GetInfoString(buf, dev, l, totalSize, totalPlayLength, 0);
	}

	virtual songid_t GetTrack(int pos) { return dev->getPlaylistTrack(playlistId,pos); }

	virtual void ColumnResize(int col, int newWidth) {
		if(col >=0 && col < fields.GetSize()) {
			ListField * lf = (ListField *)fields.Get(col);
			lf->width = newWidth;
			wchar_t buf[100] = {0};
			wsprintf(buf,L"colWidth_%d",lf->field);
			currentViewedDevice->config->WriteInt(buf,newWidth);
		}
	}
};

static INT_PTR CALLBACK find_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG:
			SetDlgItemText(hwndDlg,IDC_EDIT,currentViewedDevice->config->ReadString(L"plfind",L""));
			if (FALSE != CenterWindow(hwndDlg, (HWND)lParam))
				SendMessage(hwndDlg, DM_REPOSITION, 0, 0L);
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				{
					wchar_t buf[256] = {0};
					GetDlgItemText(hwndDlg,IDC_EDIT,buf,sizeof(buf)/sizeof(wchar_t));
					buf[255]=0;
					currentViewedDevice->config->WriteString(L"plfind",buf);
					EndDialog(hwndDlg,(INT_PTR)_wcsdup(buf));
				}
				break;
				case IDCANCEL:
					EndDialog(hwndDlg,0);
				break;
			}
		break;
	}
	return 0;
}

extern C_ItemList * FilterSongs(const wchar_t * filter, const C_ItemList * songs, Device * dev, bool cloud);

static int m_pldrag;

INT_PTR CALLBACK pmp_playlist_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	static bool pldrag_changed;
	if (wad_handleDialogMsgs) { BOOL a=wad_handleDialogMsgs(hwndDlg,uMsg,wParam,lParam); if (a) return a; }
	if (tracksList) { BOOL a=tracksList->DialogProc(hwndDlg,uMsg,wParam,lParam); if(a) return a; }

	switch (uMsg) {
		case WM_INITDIALOG:
		{
			if (!view.play)
			{
				SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view);
			}

			groupBtn = gen_mlconfig->ReadInt(L"groupbtn", 1);
			enqueuedef = (gen_mlconfig->ReadInt(L"enqueuedef", 0) == 1);

			// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
			//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
			pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG(IDC_BUTTON_CUSTOM, IDC_BUTTON_ENQUEUE), (INT_PTR)L"ml_pmp"};
			wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
			if (pszTextW && pszTextW[0] != 0)
			{
				// set this to be a bit different so we can just use one button and not the
				// mixable one as well (leaving that to prevent messing with the resources)
				customAllowed = TRUE;
				SetDlgItemTextW(hwndDlg, IDC_BUTTON_CUSTOM, pszTextW);
			}
			else
			{
				customAllowed = FALSE;
			}

			playmode = L"plplaymode";
			hwndMediaView = hwndDlg;
			pldrag_changed=false;
			*(void **)&wad_handleDialogMsgs=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,2,ML_IPC_SKIN_WADLG_GETFUNC);
			*(void **)&wad_DrawChildWindowBorders=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,3,ML_IPC_SKIN_WADLG_GETFUNC);
			tracks = new PlaylistContents(currentViewedPlaylist,currentViewedDevice->dev);
			m_pldrag=-1;

			{
				MLSKINWINDOW skin = {0};
				skin.skinType = SKINNEDWND_TYPE_AUTO;
				FLICKERFIX ff = {0};
				ff.mode = FFM_ERASEINPAINT;
				INT ffcl[] = { IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM, IDC_BUTTON_SORT, IDC_BUTTON_EJECT, IDC_STATUS};
				for (int i = 0; i < sizeof(ffcl) / sizeof(INT); i++)
				{
					ff.hwnd = GetDlgItem(hwndDlg, ffcl[i]);
					if (IsWindow(ff.hwnd))
					{
						SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&ff, ML_IPC_FLICKERFIX);

						skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | (groupBtn && (i < 3) ? SWBS_SPLITBUTTON : 0);
						skin.hwndToSkin = ff.hwnd;
						MLSkinWindow(plugin.hwndLibraryParent, &skin);
					}
				}
			}
		}
		break;

		case WM_DESTROY:
			if(tracks) delete tracks; tracks=0;
		break;

		case WM_DROPFILES:
		return currentViewedDevice->TransferFromDrop((HDROP)wParam, currentViewedPlaylist);

		case WM_NOTIFY:
		{
			LPNMHDR l=(LPNMHDR)lParam;
			if(l->idFrom==IDC_LIST_TRACKS)
				switch(l->code) {
					case LVN_KEYDOWN:
						switch(((LPNMLVKEYDOWN)lParam)->wVKey) {
							case 0x46: //F
							{
								if(!GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									wchar_t * findstr = (wchar_t*)WASABI_API_DIALOGBOXPARAMW(IDD_FIND,hwndDlg,find_dlgproc, (LPARAM)hwndDlg);
									if(!findstr) break;
									C_ItemList songs;
									int l = currentViewedDevice->dev->getPlaylistLength(currentViewedPlaylist);
									int i;
									for(i=0; i<l; i++) songs.Add((void*)currentViewedDevice->dev->getPlaylistTrack(currentViewedPlaylist,i));
									C_ItemList * found = FilterSongs(findstr,&songs,currentViewedDevice->dev,0);
									free(findstr);
									int j=0;
									HWND wnd = tracksList->listview.getwnd();
									for(i=0; i<l; i++) {
									if(found->Get(j) == songs.Get(i)) {
										ListView_SetItemState(wnd,i,LVIS_SELECTED,LVIS_SELECTED);
										if(j++ == 0) ListView_EnsureVisible(wnd,i,TRUE);
									}
									else ListView_SetItemState(wnd,i,0,LVIS_SELECTED);
								}
								delete found;
							}
						}
					break;
					case VK_DELETE:
						if(!GetAsyncKeyState(VK_CONTROL)){
							if(!GetAsyncKeyState(VK_SHIFT)){
								handleContextMenuResult(ID_TRACKSLIST_DELETE);
							}
							else{
								SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(ID_TRACKSLIST_REMOVEFROMPLAYLIST,0),0);
							}
						}
					break;
					case 0x43: //C
						if(GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
							handleContextMenuResult(ID_TRACKSLIST_COPYTOLIBRARY);
						}
					break;
				}
			}
		}
		break;

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags), 2);
			}
			return TRUE;

		case WM_DISPLAYCHANGE:
		{
			UpdateWindow(hwndDlg);
			LayoutWindows(hwndDlg, TRUE, 2);
		}
		break;

		case WM_APP + 104:
		{
			pmp_common_UpdateButtonText(hwndDlg, wParam);
			LayoutWindows(hwndDlg, TRUE, 2);
			return 0;
		}

		case WM_PAINT:
		{
			int tab[] = {IDC_LIST_TRACKS|DCW_SUNKENBORDER};
			int size = sizeof(tab) / sizeof(tab[0]);
			if (wad_DrawChildWindowBorders) wad_DrawChildWindowBorders(hwndDlg,tab,size);
		}
		break;

		case WM_LBUTTONDOWN:
			m_pldrag=-1;
			//m_pldrag = tracksList->listview.FindItemByPoint(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		break;

		case WM_LBUTTONUP:
			m_pldrag=-1;
			if(GetCapture() == hwndDlg) ReleaseCapture();
			if(pldrag_changed) { currentViewedDevice->DevicePropertiesChanges(); pldrag_changed=false;}
		break;

		case WM_MOUSEMOVE:
			if(wParam==MK_LBUTTON) {
			if(m_pldrag==-1) { m_pldrag=tracksList->listview.FindItemByPoint(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)); break; } // m_pldrag=GET_Y_LPARAM(lParam);
			int p = tracksList->listview.FindItemByPoint(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)); // new point
			if(p==-1) break;
			//work out how much to move and how
			int h=1; //height of one item
			HWND listWnd = tracksList->listview.getwnd();
			Device * dev = currentViewedDevice->dev;
			if(p - m_pldrag >= h) {
				//moved down
				int start=-1,end=-1;
				int i;
				int l = dev->getPlaylistLength(currentViewedPlaylist);
				for(i=l-1; i>=0; i--) if(ListView_GetItemState(listWnd, i, LVIS_SELECTED) == LVIS_SELECTED) {
					if(i == l-1) break;
					if(end == -1) end = i+1;
					start = i;
					//change playlist
					dev->playlistSwapItems(currentViewedPlaylist,i,i+1);
					pldrag_changed=true;
					//set selection correctly
					ListView_SetItemState(listWnd,i,0,LVIS_SELECTED); 
					ListView_SetItemState(listWnd,i+1,LVIS_SELECTED,LVIS_SELECTED);
					if(ListView_GetItemState(listWnd, i, LVIS_FOCUSED)==LVIS_FOCUSED) {
						ListView_SetItemState(listWnd,i,0,LVIS_FOCUSED); 
						ListView_SetItemState(listWnd,i+1,LVIS_FOCUSED,LVIS_FOCUSED);
					}
				}
				if(start != -1) {
					ListView_RedrawItems(listWnd,start,end);
					m_pldrag += h;
				}
			}
			else if(m_pldrag - p >= h){
				//moved up
				int start=-1,end=-1;
				int i;
				int l = dev->getPlaylistLength(currentViewedPlaylist);
				for(i=0; i<l; i++) if(ListView_GetItemState(listWnd, i, LVIS_SELECTED) == LVIS_SELECTED) {
					if(i == 0) break;
					if(start == -1) start = i-1;
					end = i;
					//change playlist
					dev->playlistSwapItems(currentViewedPlaylist,i,i-1);
					pldrag_changed=true;
					//set selection correctly
					ListView_SetItemState(listWnd,i,0,LVIS_SELECTED); 
					ListView_SetItemState(listWnd,i-1,LVIS_SELECTED,LVIS_SELECTED);
					if(ListView_GetItemState(listWnd, i, LVIS_FOCUSED)==LVIS_FOCUSED) {
						ListView_SetItemState(listWnd,i,0,LVIS_FOCUSED); 
						ListView_SetItemState(listWnd,i-1,LVIS_FOCUSED,LVIS_FOCUSED);
					}
				}
				if(start != -1) {
					ListView_RedrawItems(listWnd,start,end);
					m_pldrag -= h;
				}
			}
			return 0;
		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_TRACKSLIST_REMOVEFROMPLAYLIST:
				{
					int l = tracksList->listview.GetCount();
					while(l-- > 0) if(tracksList->listview.GetSelected(l))
					currentViewedDevice->dev->removeTrackFromPlaylist(currentViewedPlaylist,l);
					tracksList->UpdateList();
					currentViewedDevice->DevicePropertiesChanges();
				}
				break;
				case IDC_BUTTON_SORT:
				{
					HMENU menu = GetSubMenu(m_context_menus,5);
					POINT p;
					GetCursorPos(&p);
					int r = Menu_TrackSkinnedPopup(menu, TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY, p.x, p.y, hwndDlg, NULL);
					if(r >= ID_SORTPLAYLIST_ARTIST && r <= ID_SORTPLAYLIST_LASTPLAYED) {
						currentViewedDevice->dev->sortPlaylist(currentViewedPlaylist,r - ID_SORTPLAYLIST_ARTIST);
					}
					else if(r == ID_SORTPLAYLIST_RANDOMIZE) {
						int elems = currentViewedDevice->dev->getPlaylistLength(currentViewedPlaylist);
						if (elems > 1) {
							for (int p = 0; p < elems; p ++) {
								int np=genrand_int31()%elems;
								if (p != np)  // swap pp and np
									currentViewedDevice->dev->playlistSwapItems(currentViewedPlaylist,p,np);
							}
						}
					}
					else if(r == ID_SORTPLAYLIST_REVERSEPLAYLIST) {
						int i=0;
						int j = currentViewedDevice->dev->getPlaylistLength(currentViewedPlaylist) - 1;
						while(i < j)
							currentViewedDevice->dev->playlistSwapItems(currentViewedPlaylist,i++,j--);
					}
					if(r > 0) {
						tracksList->UpdateList(true);
						currentViewedDevice->DevicePropertiesChanges();
					}
				}
				break;
			}
		break; // WM_COMMAND
	}
	return pmp_common_dlgproc(hwndDlg,uMsg,wParam,lParam);
}

INT_PTR CALLBACK pmp_video_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (wad_handleDialogMsgs) { BOOL a=wad_handleDialogMsgs(hwndDlg,uMsg,wParam,lParam); if (a) return a; }
	if (tracksList) { BOOL a=tracksList->DialogProc(hwndDlg,uMsg,wParam,lParam); if(a) return a; }

	switch (uMsg) {
		case WM_INITDIALOG:
		{
			if (!view.play)
			{
				SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view);
			}

			groupBtn = gen_mlconfig->ReadInt(L"groupbtn", 1);
			enqueuedef = (gen_mlconfig->ReadInt(L"enqueuedef", 0) == 1);

			// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
			//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
			pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG(IDC_BUTTON_CUSTOM, IDC_BUTTON_ENQUEUE), (INT_PTR)L"ml_pmp"};
			wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
			if (pszTextW && pszTextW[0] != 0)
			{
				// set this to be a bit different so we can just use one button and not the
				// mixable one as well (leaving that to prevent messing with the resources)
				customAllowed = TRUE;
				SetDlgItemTextW(hwndDlg, IDC_BUTTON_CUSTOM, pszTextW);
			}
			else
			{
				customAllowed = FALSE;
			}

			playmode = L"viewplaymode";
			hwndMediaView = hwndDlg;
			*(void **)&wad_handleDialogMsgs=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,2,ML_IPC_SKIN_WADLG_GETFUNC);
			*(void **)&wad_DrawChildWindowBorders=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,3,ML_IPC_SKIN_WADLG_GETFUNC);

			if (!lstrcmpiA(currentViewedDevice->GetName(), "all_sources"))
				header = 1;
			else
				header = currentViewedDevice->config->ReadInt(L"header",0);

			HWND list = GetDlgItem(hwndDlg, IDC_LIST_TRACKS);
			// TODO need to be able to change the order of the tracks items (so cloud is in a more appropriate place)
			//ListView_SetExtendedListViewStyleEx(list, LVS_EX_HEADERDRAGDROP, LVS_EX_HEADERDRAGDROP);
			ListView_SetExtendedListViewStyle(list, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

			if (currentViewedDevice->config->ReadInt(L"savefilter", 1))
			{
				SetDlgItemTextW(hwndDlg, IDC_QUICKSEARCH, currentViewedDevice->config->ReadString(L"savedfilter", L""));
			}

			// depending on how this is called, it will either do a video only view or will be re-purposed as a 'simple' view
			aacontents = new ArtistAlbumLists(currentViewedDevice->dev, currentViewedPlaylist, currentViewedDevice->config,
											  NULL, 0, (lParam ? -1 : (currentViewedDevice->videoView ? 1 : -1)), (lParam == 1));

			if (aacontents)
			{
				tracks = aacontents->GetTracksList();
			}

			MLSKINWINDOW skin = {0};
			skin.hwndToSkin = list;
			skin.skinType = SKINNEDWND_TYPE_LISTVIEW;
			skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
			MLSkinWindow(plugin.hwndLibraryParent, &skin);
			MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(list), tracks->cloudcol);

			FLICKERFIX ff = {0};
			ff.mode = FFM_ERASEINPAINT;
			INT ffcl[] = { IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM,
						   IDC_BUTTON_CLEARSEARCH, IDC_BUTTON_CLEARREFINE,
						   IDC_BUTTON_EJECT, IDC_BUTTON_SYNC, IDC_BUTTON_AUTOFILL,
						   IDC_STATUS, IDC_SEARCH_TEXT, IDC_QUICKSEARCH, IDC_REFINE,
						   IDC_REFINE_TEXT,
						   // disabled cloud parts
						   /*IDC_HEADER_DEVICE_ICON, IDC_HEADER_DEVICE_NAME,
						   IDC_HEADER_DEVICE_BAR, IDC_HEADER_DEVICE_SIZE,
						   IDC_HEADER_DEVICE_TRANSFER,*/
			};
			skin.skinType = SKINNEDWND_TYPE_AUTO;
			for (int i = 0; i < sizeof(ffcl) / sizeof(INT); i++)
			{
				ff.hwnd = GetDlgItem(hwndDlg, ffcl[i]);
				if (IsWindow(ff.hwnd))
				{
					SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&ff, ML_IPC_FLICKERFIX);

					skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | (groupBtn && (i < 3) ? SWBS_SPLITBUTTON : 0);
					skin.hwndToSkin = ff.hwnd;
					MLSkinWindow(plugin.hwndLibraryParent, &skin);
				}
			}

			skin.hwndToSkin = hwndDlg;
			skin.skinType = SKINNEDWND_TYPE_AUTO;
			skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
			MLSkinWindow(plugin.hwndLibraryParent, &skin);

			if (0 != currentViewedDevice->dev->extraActions(DEVICE_SYNC_UNSUPPORTED,0,0,0))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_SYNC), FALSE);
		}
		break;

		case WM_USER+1:
			if (tracks)
			{
				if (lParam)
				{
					tracks->RemoveTrack((songid_t)wParam);
				}
				tracksList->UpdateList();
			}
		break;

		case WM_DESTROY:
			if (aacontents) aacontents->bgQuery_Stop();
			tracks = NULL;
			if (aacontents) delete aacontents; aacontents=NULL;
		break;

		case WM_DROPFILES:
		return currentViewedDevice->TransferFromDrop((HDROP)wParam);

		case WM_NOTIFY:
		{
			LPNMHDR l=(LPNMHDR)lParam;
			if (l->idFrom == IDC_LIST_TRACKS)
			{
				switch (l->code)
				{
					case LVN_KEYDOWN:
						switch(((LPNMLVKEYDOWN)lParam)->wVKey) {
							case VK_DELETE:
							{
								if(!GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									handleContextMenuResult(ID_TRACKSLIST_DELETE);
								}
							}
							break;
							case 0x45: //E
								bool noEdit = currentViewedDevice->dev->extraActions(DEVICE_DOES_NOT_SUPPORT_EDITING_METADATA,0,0,0)!=0;
								if(!noEdit && GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									C_ItemList * items = getSelectedItems();
									editInfo(items,currentViewedDevice->dev, CENTER_OVER_ML_VIEW);
									delete items;
								}
							break;
						}
					break;
				}
			}
		}
		break;

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags), 1);
			}
			return TRUE;

		case WM_DISPLAYCHANGE:
		{
			UpdateWindow(hwndDlg);
			LayoutWindows(hwndDlg, TRUE, 1);
		}
		break;

		case WM_APP + 104:
		{
			pmp_common_UpdateButtonText(hwndDlg, wParam);
			LayoutWindows(hwndDlg, TRUE, 1);
			return 0;
		}

		case WM_PAINT:
		{
			int tab[] = {IDC_LIST_TRACKS|DCW_SUNKENBORDER,
						 IDC_QUICKSEARCH|DCW_SUNKENBORDER,
						 (!header?IDC_HDELIM2|DCW_DIVIDER:0),
			};
			int size = sizeof(tab) / sizeof(tab[0]);
			// do this to prevent drawing parts when the views are collapsed
			if (!currentViewedDevice->isCloudDevice) size -= 1;
			if (wad_DrawChildWindowBorders) wad_DrawChildWindowBorders(hwndDlg,tab,size);
		}
		return TRUE;

		case WM_APP + 3: // send by bgthread
			if (wParam == 0x69)
			{
				if (aacontents)
				{
					aacontents->bgQuery_Stop();
					tracks = aacontents->GetTracksList();
					if (tracksList) tracksList->UpdateList();
					UpdateStatus(hwndDlg, true);
				}
			}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_QUICKSEARCH:
					if (HIWORD(wParam) == EN_CHANGE && !noSearchTimer) {
						KillTimer(hwndDlg,500);
						SetTimer(hwndDlg,500,250,NULL);
					}
					break;
				case IDC_HEADER_DEVICE_TRANSFER:
				{
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BUTTON_SYNC)))
					{
						MessageBox(hwndDlg, L"Transferring files from this device to another is not currently supported.",
											L"Cloud Transfer Not Supported", MB_ICONINFORMATION);
						break;
					}
				}
				case IDC_BUTTON_SYNC:
					if (!currentViewedDevice->isCloudDevice)
						currentViewedDevice->Sync();
					else
						currentViewedDevice->CloudSync();
					break;
				case IDC_SEARCH_TEXT:
					if (HIWORD(wParam) == STN_DBLCLK)
					{
						// TODO decide what to do for the 'all_sources'
						if (currentViewedDevice->isCloudDevice &&
							lstrcmpiA(currentViewedDevice->GetName(), "all_sources"))
						{
							header = !header;
							currentViewedDevice->config->WriteInt(L"header",header);
							LayoutWindows(hwndMediaView, TRUE, 1);
							UpdateStatus(hwndDlg);
							ShowWindow(hwndDlg, 0);
							ShowWindow(hwndDlg, 1);
						}
					}
					break;
				case IDC_BUTTON_CLEARSEARCH:
					SetDlgItemText(hwndDlg,IDC_QUICKSEARCH,L"");
					break;
			}
		break; //WM_COMMAND

		case WM_TIMER:
			switch(wParam) {
				case 123:
				{
					if (aacontents && aacontents->bgThread_Handle && tracksList)
					{
						HWND hwndList;
						hwndList = tracksList->listview.getwnd();
						if (1 != ListView_GetItemCount(hwndList)) ListView_SetItemCountEx(hwndList, 1, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
						ListView_RedrawItems(hwndList, 0, 0);
					}
				}
				break;

				case 500:
				{
					KillTimer(hwndDlg,500);
					if (currentViewedDevice && aacontents)
					{
						wchar_t buf[256]=L"";
						GetDlgItemText(hwndDlg,IDC_QUICKSEARCH,buf,sizeof(buf)/sizeof(wchar_t));
						aacontents->SetSearch(buf, (!!currentViewedDevice->isCloudDevice));
						if (!aacontents->bgThread_Handle)
						{
							if (tracksList) tracksList->UpdateList();
							UpdateStatus(hwndDlg);
						}
					}
				}
				break;
			}
		break;
	}

	return pmp_common_dlgproc(hwndDlg,uMsg,wParam,lParam);
}

static INT_PTR CALLBACK pmp_common_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG:
		{
			hwndMediaView = hwndDlg;

			HACCEL accel = WASABI_API_LOADACCELERATORSW(IDR_ACCELERATORS);
			if (accel)
				WASABI_API_APP->app_addAccelerators(hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD);

			if (currentViewedDevice->isCloudDevice)
			{
				wchar_t buf[256] = {0};
				currentViewedDevice->GetDisplayName(buf, 128);
				SetDlgItemText(hwndDlg, IDC_HEADER_DEVICE_NAME, buf);

				int icon_id = currentViewedDevice->dev->extraActions(0x22, 0, 0, 0);
				HBITMAP bm = (HBITMAP)LoadImage(GetModuleHandle(L"pmp_cloud.dll"),
												MAKEINTRESOURCE(icon_id > 0 ? icon_id : 103), IMAGE_BITMAP, 16, 16,
												LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);

				SendDlgItemMessage(hwndDlg, IDC_HEADER_DEVICE_ICON, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bm);

				HWND progress = GetDlgItem(hwndDlg, IDC_HEADER_DEVICE_BAR);
				if (IsWindow(progress))
				{
					MLSKINWINDOW skin = {0};
					skin.hwndToSkin = progress;
					skin.skinType = SKINNEDWND_TYPE_PROGRESSBAR;
					skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
					MLSkinWindow(plugin.hwndLibraryParent, &skin);
				}
			}

			tracksList = new SkinnedListView(tracks,IDC_LIST_TRACKS,plugin.hwndLibraryParent, hwndDlg);
			tracksList->DialogProc(hwndDlg,WM_INITDIALOG,0,0);
			if (tracksList->contents && !tracksList->contents->cloud)
			{
				tracksList->UpdateList();
				UpdateStatus(hwndDlg);
			}

			pmp_common_UpdateButtonText(hwndDlg, enqueuedef == 1);
		}

		case WM_DISPLAYCHANGE:
		{
			if (currentViewedDevice->isCloudDevice)
			{
				int (*wad_getColor)(int idx);
				*(void **)&wad_getColor=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,1,ML_IPC_SKIN_WADLG_GETFUNC);
				ARGB32 fc = (!wad_getColor?wad_getColor(WADLG_WNDBG):RGB(0xFF,0xFF,0xFF)) & 0x00FFFFFF;
				SetToolbarButtonBitmap(hwndDlg,IDC_HEADER_DEVICE_TRANSFER, MAKEINTRESOURCE(IDR_TRANSFER_SMALL_ICON),fc);
			}
		}
		break;

		case WM_DESTROY:
		{
			if (currentViewedDevice)
			{
				SkinBitmap *s = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_HEADER_DEVICE_TRANSFER),GWLP_USERDATA);
				if (s) DeleteSkinBitmap(s);

				if (currentViewedDevice->config->ReadInt(L"savefilter", 1))
				{
					wchar_t buf[256] = {0};
					GetDlgItemTextW(hwndDlg, IDC_QUICKSEARCH, buf, 256);
					currentViewedDevice->config->WriteString(L"savedfilter", buf);
					buf[0] = 0;
					GetDlgItemTextW(hwndDlg, IDC_REFINE, buf, 256);
					currentViewedDevice->config->WriteString(L"savedrefinefilter", buf);
				}
			}

			WASABI_API_APP->app_removeAccelerators(hwndDlg);

			hwndMediaView = NULL;
			currentViewedDevice = NULL;
			if (tracksList)
			{
				delete tracksList;
				tracksList = NULL;
			}
		}
		break;

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
			if (di->CtlType == ODT_BUTTON || di->CtlType == ODT_STATIC) {
				if(di->CtlID == IDC_HEADER_DEVICE_TRANSFER)
				{ // draw the toolbar buttons!
					int (*wad_getColor)(int idx);
					*(void **)&wad_getColor=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,1,ML_IPC_SKIN_WADLG_GETFUNC);
					HBRUSH hbr = CreateSolidBrush((wad_getColor?wad_getColor(WADLG_WNDBG):RGB(0xFF,0xFF,0xFF)));
					FillRect(di->hDC, &di->rcItem, hbr);
					DeleteBrush(hbr);

					SkinBitmap* hbm = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,di->CtlID),GWLP_USERDATA);
					if(hbm && di->rcItem.left != di->rcItem.right && di->rcItem.top != di->rcItem.bottom) {
						DCCanvas dc(di->hDC);
						if (di->itemState & ODS_SELECTED) hbm->blitAlpha(&dc,di->rcItem.left+1,di->rcItem.top+1);
						else hbm->blitAlpha(&dc,di->rcItem.left,di->rcItem.top);
						dc.Release();
					}
				}
			}
		}
		break;

		case WM_NOTIFY:
		{
			LPNMHDR l=(LPNMHDR)lParam;
			if(l->code == LVN_KEYDOWN && ((LPNMLVKEYDOWN)lParam)->wVKey == VK_F5)
			{
				if (currentViewedDevice->dev->extraActions(DEVICE_REFRESH,0,0,0))
				{
					SetTimer(hwndDlg, (artistList ? 501 : 500), 250, NULL);
				}
			}
			if(l->idFrom == IDC_LIST_TRACKS)
			{
				switch(l->code) {
					case LVN_KEYDOWN:
						switch(((LPNMLVKEYDOWN)lParam)->wVKey) {
							case 0x45: //E
							{
								bool noEdit = currentViewedDevice->dev->extraActions(DEVICE_DOES_NOT_SUPPORT_EDITING_METADATA,0,0,0)!=0;
								if(!noEdit && GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									C_ItemList * items = getSelectedItems();
									editInfo(items,currentViewedDevice->dev, CENTER_OVER_ML_VIEW);
									delete items;
								}
							}
							break;
						}
					break;
					case NM_RETURN:
					case NM_DBLCLK:
					{
						C_ItemList * items = getSelectedItems(true);
						int start=0;
						int l=tracksList->listview.GetCount();

						int i = 0;
						for (; i < l; i++)
							if (tracksList->listview.GetSelected(i))
							{ 
								start = i;
								break;
							}
						if (items->GetSize() && (!!(GetAsyncKeyState(VK_SHIFT)&0x8000) || !gen_mlconfig->ReadInt(playmode,1))) {
							void* x = items->Get(i);
							delete items;
							items = new C_ItemList;
							items->Add(x);
							start = 0;
						}
						currentViewedDevice->PlayTracks(items, start, (!(GetAsyncKeyState(VK_SHIFT)&0x8000) ? (enqueuedef == 1) : (enqueuedef != 1)), false);
						delete items;
					}
					break;

					case NM_CUSTOMDRAW:
					{
						LRESULT result = 0;
						if (ListView_OnCustomDraw(hwndDlg, (NMLVCUSTOMDRAW*)lParam, &result))
						{
							SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)result);
							return 1;
						}
					}
					break;

					case LVN_GETDISPINFOW:
					{
						NMLVDISPINFOW* pdi = (NMLVDISPINFOW*)lParam;
						if (aacontents && aacontents->bgThread_Handle)
						{
							LVITEMW *pItem = &pdi->item;
							if (0 == pItem->iItem && (LVIF_TEXT & pItem->mask))
							{
								if (0 == pItem->iSubItem)
								{
									static char bufpos = 0;
									static int buflen = 17;
									static wchar_t buffer[64];//L"Scanning  _\0/-\\|";
									if (!buffer[0])
									{
										//WASABI_API_LNGSTRINGW_BUF(IDS_SCANNING_PLAIN,buffer,54);
										StringCchCopyW(buffer,64,L"Scanning");
										StringCchCatW(buffer,64,L"  _");
										StringCchCatW(buffer+(buflen=lstrlenW(buffer)+1),64,L"/-\\|");
										buflen+=4;
									}
									int pos = buflen - 5;;
									buffer[pos - 1] = buffer[pos + (bufpos++&3) + 1];
									pItem->pszText = buffer;
									SetDlgItemText(hwndDlg, IDC_STATUS, buffer);
								}
								else
								{
									pItem->pszText = L"";
								}
							}
							return 1;
						}
					}
					break;

					case NM_CLICK:
					{
						// prevent the right-click menus appearing when scanning
						if (aacontents && aacontents->bgThread_Handle) break;

						LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
						if (lpnmitem->iItem != -1 && lpnmitem->iSubItem == tracks->cloudcol)
						{
							RECT itemRect = {0};
							if (lpnmitem->iSubItem)
								ListView_GetSubItemRect(l->hwndFrom, lpnmitem->iItem, lpnmitem->iSubItem, LVIR_BOUNDS, &itemRect);
							else
							{
								ListView_GetItemRect(l->hwndFrom, lpnmitem->iItem, &itemRect, LVIR_BOUNDS);
								itemRect.right = itemRect.left + ListView_GetColumnWidth(l->hwndFrom, tracks->cloudcol);
							}
							MapWindowPoints(l->hwndFrom, HWND_DESKTOP, (POINT*)&itemRect, 2);

							int cloud_devices = 0;
							int mark = tracksList->listview.GetSelectionMark();
							if (mark != -1)
							{
								// if wanting to do on the selection then use getSelectedItems()
								//C_ItemList * items = getSelectedItems();
								// otherwise only work on the selection mark for speed (and like how ml_local does things)
								C_ItemList * items = new C_ItemList;
								items->Add((void*)tracks->GetTrack(mark));
								HMENU cloud_menu = (HMENU)currentViewedDevice->dev->extraActions(DEVICE_GET_CLOUD_SOURCES_MENU, (intptr_t)&cloud_devices, 0, (intptr_t)items);
								if (cloud_menu)
								{
									int r = Menu_TrackSkinnedPopup(cloud_menu, TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,
																   itemRect.right, itemRect.top, hwndDlg, NULL);
									if (r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_UPPER) { // deals with cloud specific menus
										int ret = currentViewedDevice->dev->extraActions(DEVICE_DO_CLOUD_SOURCES_MENU, (intptr_t)r, 1, (intptr_t)items);
										// only send a removal from the view if plug-in says so
										if (ret) SendMessage(hwndDlg, WM_USER+1, (WPARAM)items->Get(0), ret);
									}
									DestroyMenu(cloud_menu);
								}
								delete items;
							}
						}
					}
					break;
				}
			}
			else if(l->idFrom==IDC_LIST_ARTIST || l->idFrom==IDC_LIST_ALBUM || l->idFrom==IDC_LIST_ALBUM2)
			{
				switch(l->code) {
					case NM_CUSTOMDRAW:
					{
						LRESULT result = 0;
						if (ListView_OnCustomDraw(hwndDlg, (NMLVCUSTOMDRAW*)lParam, &result))
						{
							SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)result);
							return 1;
						}
					}
					break;
				}
			}
		}
		break; //WM_NOTIFY

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_BUTTON_EJECT:
					currentViewedDevice->Eject();
				break;

				case IDC_BUTTON_PLAY:
				case ID_TRACKSLIST_PLAYSELECTION:
				case IDC_BUTTON_ENQUEUE:
				case ID_TRACKSLIST_ENQUEUESELECTION:
				case IDC_BUTTON_CUSTOM:
				{
					if (HIWORD(wParam) == MLBN_DROPDOWN)
					{
						pmp_common_PlayEnqueue(hwndDlg, (HWND)lParam, LOWORD(wParam));
					}
					else
					{
						bool action;
						if (LOWORD(wParam) == IDC_BUTTON_PLAY || LOWORD(wParam) == ID_TRACKSLIST_PLAYSELECTION)
							action = (HIWORD(wParam) == 1) ? enqueuedef == 1 : 0;
						else if (LOWORD(wParam) == IDC_BUTTON_ENQUEUE || LOWORD(wParam) == ID_TRACKSLIST_ENQUEUESELECTION)
							action = (HIWORD(wParam) == 1) ? (enqueuedef != 1) : 1;
						else
							// so custom can work with the menu item part
							break;

						C_ItemList * selected = getSelectedItems();
						currentViewedDevice->PlayTracks(selected, 0, action, true);
						delete selected;
					}
				}
				break;

				default:
					handleContextMenuResult(LOWORD(wParam));
					break;
			}
		break; //WM_COMMAND

		case WM_CONTEXTMENU:
		{
			// prevent the right-click menus appearing when scanning
			if (aacontents && aacontents->bgThread_Handle) break;

			POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
			UINT_PTR idFrom = GetDlgCtrlID((HWND)wParam);
			HWND hwndFrom = (HWND)wParam;
			HWND hwndFromChild = WindowFromPoint(pt);

			// deal with the column headers first
			SkinnedListView * list=NULL;
			int n = 0;
			if (artistList && hwndFromChild == ListView_GetHeader(artistList->listview.getwnd())) { n=0; list=artistList; }
			if (albumList && hwndFromChild == ListView_GetHeader(albumList->listview.getwnd())) { n=1; list=albumList; }
			if (albumList2 && hwndFromChild == ListView_GetHeader(albumList2->listview.getwnd())) { n=2; list=albumList2; }
			if (list)
			{
				HMENU menu = list->GetMenu(true,n,currentViewedDevice->config,m_context_menus);
				int r = Menu_TrackSkinnedPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, pt.x, pt.y, hwndDlg, NULL);
				list->ProcessMenuResult(r,true,n,currentViewedDevice->config,hwndDlg);
				return 0;
			}

			// and then do the list menus
			if(idFrom==IDC_LIST_ARTIST || idFrom==IDC_LIST_ALBUM || idFrom==IDC_LIST_ALBUM2)
			{
				handleContextMenuResult(showContextMenu(1,hwndFrom,currentViewedDevice->dev,pt));
			}
			else if(idFrom == IDC_LIST_TRACKS)
			{
				// prevents funkiness if having shown the 'customize columns' menu before
				// as well as alloing the shift+f10 key to work within the track listview
				if ((hwndFrom == hwndFromChild) || (pt.x == -1 && pt.y == -1))
				{
					int r = showContextMenu((GetDlgItem(hwndDlg, IDC_BUTTON_SORT) ? 2 : 0), hwndFrom, currentViewedDevice->dev, pt);
					switch(r) {
						case ID_TRACKSLIST_REMOVEFROMPLAYLIST:
							SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(ID_TRACKSLIST_REMOVEFROMPLAYLIST,0),0);
						break;
						default:
							handleContextMenuResult(r);
						break;
					}
				}
			}
			break;
		}

		case WM_ML_CHILDIPC:
			if(lParam == ML_CHILDIPC_GO_TO_SEARCHBAR)
			{
				SendDlgItemMessage(hwndDlg, IDC_QUICKSEARCH, EM_SETSEL, 0, -1);
				SetFocus(GetDlgItem(hwndDlg, IDC_QUICKSEARCH));
			}
			else if (lParam == ML_CHILDIPC_REFRESH_SEARCH)
			{
				restoreDone = FALSE;
				PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_QUICKSEARCH, EN_CHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_QUICKSEARCH));
			}
			break;
	}
	return 0;
}

typedef struct _LAYOUT
{
	INT		id;
	HWND	hwnd;
	INT		x;
	INT		y;
	INT		cx;
	INT		cy;
	DWORD	flags;
	HRGN	rgn;
}LAYOUT, PLAYOUT;

#define SETLAYOUTPOS(_layout, _x, _y, _cx, _cy) { _layout->x=_x; _layout->y=_y;_layout->cx=_cx;_layout->cy=_cy;_layout->rgn=NULL; }
#define SETLAYOUTFLAGS(_layout, _r)																						\
	{																													\
		BOOL fVis;																										\
		fVis = (WS_VISIBLE & (LONG)GetWindowLongPtr(_layout->hwnd, GWL_STYLE));											\
		if (_layout->x == _r.left && _layout->y == _r.top) _layout->flags |= SWP_NOMOVE;									\
		if (_layout->cx == (_r.right - _r.left) && _layout->cy == (_r.bottom - _r.top)) _layout->flags |= SWP_NOSIZE;	\
		if ((SWP_HIDEWINDOW & _layout->flags) && !fVis) _layout->flags &= ~SWP_HIDEWINDOW;								\
		if ((SWP_SHOWWINDOW & _layout->flags) && fVis) _layout->flags &= ~SWP_SHOWWINDOW;									\
	}
#define LAYOUTNEEEDUPDATE(_layout) ((SWP_NOMOVE | SWP_NOSIZE) != ((SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_SHOWWINDOW) & _layout->flags))

// disabled cloud parts
/*#define GROUP_MIN			0x1
#define GROUP_MAX			0x7
#define GROUP_HEADER		0x1
#define GROUP_SEARCH		0x2
#define GROUP_FILTER		0x3
#define GROUP_HDELIM		0x4
#define GROUP_REFINE		0x5
#define GROUP_TRACKS		0x6
#define GROUP_STATUS		0x7*/
#define GROUP_MIN			0x1
#define GROUP_MAX			0x6
#define GROUP_SEARCH		0x1
#define GROUP_FILTER		0x2
#define GROUP_HDELIM		0x3
#define GROUP_REFINE		0x4
#define GROUP_TRACKS		0x5
#define GROUP_STATUS		0x6

void LayoutWindows(HWND hwnd, BOOL fRedraw, int simple)
{
	static INT controls[] = 
	{
		// disabled cloud parts
		//GROUP_HEADER, IDC_HEADER_DEVICE_TRANSFER, IDC_HEADER_DEVICE_ICON, IDC_HEADER_DEVICE_NAME, IDC_HEADER_DEVICE_BAR, IDC_HEADER_DEVICE_SIZE, IDC_HDELIM2,
		GROUP_SEARCH, IDC_BUTTON_ARTMODE, IDC_BUTTON_VIEWMODE, IDC_BUTTON_COLUMNS, IDC_SEARCH_TEXT, IDC_BUTTON_CLEARSEARCH, IDC_QUICKSEARCH,
		GROUP_STATUS, IDC_BUTTON_EJECT, IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM, IDC_BUTTON_SYNC, IDC_BUTTON_AUTOFILL, IDC_BUTTON_SORT, IDC_STATUS,
		GROUP_HDELIM, IDC_HDELIM,
		GROUP_REFINE, IDC_REFINE_TEXT, IDC_BUTTON_CLEARREFINE, IDC_REFINE,
		GROUP_TRACKS, IDC_LIST_TRACKS,
		GROUP_FILTER, IDC_LIST_ARTIST, IDC_VDELIM, IDC_LIST_ALBUM, IDC_VDELIM2, IDC_LIST_ALBUM2,
	};

	INT index, divY, divX, divX2, divCY;
	RECT rc, rg, ri;
	LAYOUT layout[sizeof(controls)/sizeof(controls[0])], *pl;
	BOOL skipgroup;
	HRGN rgn;

	rgn = NULL;

	GetClientRect(hwnd, &rc);
	if (rc.right == rc.left || rc.bottom == rc.top) return;
	if (rc.right > 4) rc.right -= 4;
	SetRect(&rg, rc.left, rc.top, rc.right, rc.top);

	pl = layout;
	skipgroup = FALSE;

	divX = (adiv1pos * (rc.right- rc.left)) / 100000;
	divX2 = numFilters == 3 ? ((adiv3pos * (rc.right- rc.left)) / 100000) : rc.right;
	divY = (((-1 == adiv2pos) ? 50000 : adiv2pos) * (rc.bottom- rc.top)) / 100000;
	divCY = 0;

	InvalidateRect(hwnd, NULL, TRUE);

	for (index = 0; index < sizeof(controls) / sizeof(INT); index++)
	{
		if (controls[index] >= GROUP_MIN && controls[index] <= GROUP_MAX) // group id
		{
			skipgroup = FALSE;
			switch(controls[index])
			{
				// disabled cloud parts
				/*case GROUP_HEADER:
					if (currentViewedDevice->isCloudDevice && simple != 2)
					{
						SetRect(&rg, rc.left, rc.top + WASABI_API_APP->getScaleY(2),
								rc.right + WASABI_API_APP->getScaleX(1),
								(!header ? rc.top + WASABI_API_APP->getScaleY(18) : rc.top));
						rc.top = rg.bottom + WASABI_API_APP->getScaleY(3);
					}
					else
						skipgroup = 1;
				break;*/
				case GROUP_SEARCH:
					if (g_displaysearch && simple != 2)  
					{
						wchar_t buffer[128] = {0};
						HWND ctrl = GetDlgItem(hwnd, IDC_BUTTON_CLEARSEARCH);
						GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
						LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

						SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1),
								rc.top + WASABI_API_APP->getScaleY(2),
								rc.right - WASABI_API_APP->getScaleX(2),
								rc.top + WASABI_API_APP->getScaleY(HIWORD(idealSize)+1));
						rc.top = rg.bottom + WASABI_API_APP->getScaleY(3);
					}
					else
						skipgroup = 1;
					break;
				case GROUP_STATUS:
					if (g_displaystatus)  
					{
						wchar_t buffer[128] = {0};
						HWND ctrl = GetDlgItem(hwnd, IDC_BUTTON_EJECT);
						GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
						LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

						SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1),
								rc.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)),
								rc.right, rc.bottom);
						rc.bottom = rg.top - WASABI_API_APP->getScaleY(3);
					}
					skipgroup = !g_displaystatus;
					break;
				case GROUP_HDELIM:
					SetRect(&rg, rc.left, rc.top, rc.right, rc.bottom);
					break;
				case GROUP_REFINE:
				{
					wchar_t buffer[128] = {0};
					HWND ctrl = GetDlgItem(hwnd, IDC_BUTTON_CLEARREFINE);
					GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

					rg.top = divY + divCY;
					refineHidden = (g_displayrefine) ? ((rg.top + WASABI_API_APP->getScaleY(HIWORD(idealSize))) >= rc.bottom) : TRUE;
					SetRect(&rg, rc.left, rg.top, rc.right, (refineHidden) ? rg.top : (rg.top + WASABI_API_APP->getScaleY(HIWORD(idealSize))));
					break;
				}
				case GROUP_TRACKS:
					if (!simple)
					{
						wchar_t buffer[128] = {0};
						HWND ctrl = GetDlgItem(hwnd, IDC_BUTTON_CLEARREFINE);
						GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
						LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

						SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1),
								(divY + divCY) + ((!refineHidden) ? WASABI_API_APP->getScaleY(HIWORD(idealSize) + 3) : 0),
								rc.right, rc.bottom);
					}
					else
						SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1), rc.top, rc.right, rc.bottom);
					break;
				case GROUP_FILTER:
					if (!simple)
					{
						if (divX < (rc.left + 15)) 
						{ 
							divX = rc.left;
							adiv1_nodraw = 1;
						}
						else if (divX > (rc.right - (numFilters == 3 ? 16 * 2 : 24)))
						{ 
							RECT rw;
							GetWindowRect(GetDlgItem(hwnd, IDC_VDELIM), &rw);
							divX = rc.right - (rw.right - rw.left) - (numFilters == 3 ? 6 : 0) + (numFilters == 2 ? 1 : -1);
							adiv1_nodraw = 2;
						}
						else adiv1_nodraw = 0;

						if (divX2 < (rc.left + 16 * (numFilters == 3 ? 2 : 1))) 
						{ 
							RECT rw;
							GetWindowRect(GetDlgItem(hwnd, IDC_VDELIM), &rw);
							divX2 = rc.left;
							adiv3_nodraw = 1;
						}
						else if (divX2 > (rc.right - 16 * 2))
						{
							RECT rw;
							GetWindowRect(GetDlgItem(hwnd, IDC_VDELIM2), &rw);
							divX2 = rc.right - ((rw.right - rw.left) - 1) * 2;
							adiv3_nodraw = 2;
						}
						else adiv3_nodraw = 0;
						SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1), rc.top, rc.right, divY);
					}
					else
						skipgroup = 1;
					break;
			}
			continue;
		}
		if (skipgroup) continue;

		pl->id = controls[index];
		pl->hwnd = GetDlgItem(hwnd, pl->id);
		if (!pl->hwnd) continue;

		GetWindowRect(pl->hwnd, &ri);
		MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2);
		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS;

		switch(pl->id)
		{
			// disabled cloud parts
			/*case IDC_HEADER_DEVICE_TRANSFER:
				SETLAYOUTPOS(pl, rg.right - 18, rg.top - WASABI_API_APP->getScaleY(1),
							 WASABI_API_APP->getScaleX(16), WASABI_API_APP->getScaleY(16));
				pl->flags |= (!header && (rg.right - rg.left) > (ri.right - ri.left)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				if (SWP_SHOWWINDOW & pl->flags) rg.right -= (pl->cx + WASABI_API_APP->getScaleX(3+16));
				break;
			case IDC_HEADER_DEVICE_ICON:
				SETLAYOUTPOS(pl, rg.left, rg.top - WASABI_API_APP->getScaleY(1),
							 WASABI_API_APP->getScaleX(16), WASABI_API_APP->getScaleY(16));
				pl->flags |= (!header && (rg.right - rg.left + WASABI_API_APP->getScaleX(16)) > (ri.right - ri.left)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			case IDC_HEADER_DEVICE_NAME:
			{
				SIZE s = {0};
				wchar_t buf[128] = {0};
				HDC dc = GetDC(pl->hwnd);
				HFONT font = (HFONT)SendMessage(pl->hwnd, WM_GETFONT, 0, 0), oldfont = (HFONT)SelectObject(dc, font);
				int len = GetWindowText(pl->hwnd, buf, sizeof(buf));
				GetTextExtentPoint32(dc, buf, len, &s);
				SelectObject(dc, oldfont);
				ReleaseDC(pl->hwnd, dc);

				SETLAYOUTPOS(pl, rg.left + WASABI_API_APP->getScaleX(4), rg.top, s.cx + ((s.cx / len)), WASABI_API_APP->getScaleY(16));
				pl->flags |= (!header && (rg.right - rg.left + WASABI_API_APP->getScaleX(12)) > (ri.right - ri.left)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			}
			case IDC_HEADER_DEVICE_BAR:
				SETLAYOUTPOS(pl, rg.left + WASABI_API_APP->getScaleX(4), rg.top + WASABI_API_APP->getScaleY(2),
							 WASABI_API_APP->getScaleX(150), WASABI_API_APP->getScaleY(11));
				pl->flags |= (!header && (rg.right - rg.left) > (ri.right - ri.left)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(8));
				break;
			case IDC_HEADER_DEVICE_SIZE:
				SETLAYOUTPOS(pl, rg.left + WASABI_API_APP->getScaleX(4), rg.top, rg.right - rg.left, WASABI_API_APP->getScaleY(16));
				pl->flags |= (!header && (ri.right - ri.left) > WASABI_API_APP->getScaleX(60)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			case IDC_HDELIM2:
				SETLAYOUTPOS(pl, 0, rg.bottom + WASABI_API_APP->getScaleY(1), rg.right + WASABI_API_APP->getScaleX(35), 0);
				pl->flags |= (!header ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
				break;*/
			case IDC_BUTTON_ARTMODE:
			case IDC_BUTTON_VIEWMODE:
			case IDC_BUTTON_COLUMNS:
				pl->flags |= (rg.top < rg.bottom) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.left, rg.top, (ri.right - ri.left), (rg.bottom - rg.top));
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(1));
				break;
			case IDC_SEARCH_TEXT:
			{
				wchar_t buffer[128] = {0};
				GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
				LRESULT idealSize = MLSkinnedStatic_GetIdealSize(pl->hwnd, buffer);

				pl->flags |= (rg.top < rg.bottom) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.left + WASABI_API_APP->getScaleX(6),
							 rg.top + WASABI_API_APP->getScaleY(1),
							 WASABI_API_APP->getScaleX(LOWORD(idealSize)),
							 (rg.bottom - rg.top));
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			}
			case IDC_REFINE_TEXT:
			{
				pl->flags |= (rg.top < rg.bottom) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;

				wchar_t buffer[128] = {0};
				GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
				LRESULT idealSize = MLSkinnedStatic_GetIdealSize(pl->hwnd, buffer);

				SETLAYOUTPOS(pl, rg.left + WASABI_API_APP->getScaleX(2),
							 rg.top + WASABI_API_APP->getScaleY(1),
							 WASABI_API_APP->getScaleX(LOWORD(idealSize)),
							 (rg.bottom - rg.top));
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			}
			case IDC_BUTTON_CLEARSEARCH:
			{
				wchar_t buffer[128] = {0};
				GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
				LRESULT idealSize = MLSkinnedButton_GetIdealSize(pl->hwnd, buffer);
				LONG width = LOWORD(idealSize) + WASABI_API_APP->getScaleX(6);
				pl->flags |= (((rg.right - rg.left) - width) > WASABI_API_APP->getScaleX(40)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.right - width, rg.top, width, (rg.bottom - rg.top));
				if (SWP_SHOWWINDOW & pl->flags) rg.right -= (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			}
			case IDC_BUTTON_CLEARREFINE:
			{
				wchar_t buffer[128] = {0};
				GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
				LRESULT idealSize = MLSkinnedButton_GetIdealSize(pl->hwnd, buffer);
				LONG width = LOWORD(idealSize) + WASABI_API_APP->getScaleX(6);
				pl->flags |= (((rg.right - rg.left) - width) > WASABI_API_APP->getScaleX(40)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW ;
				SETLAYOUTPOS(pl, rg.right - width, rg.top, width, rg.bottom - rg.top);
				if (SWP_SHOWWINDOW & pl->flags) rg.right -= (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			}
			case IDC_QUICKSEARCH:
				pl->flags |= SWP_SHOWWINDOW;
				pl->flags |= (rg.right > rg.left) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left - WASABI_API_APP->getScaleX(1),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				break;
			case IDC_REFINE:
				pl->flags |= ((rg.right > rg.left) && (rg.top < rg.bottom)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left - WASABI_API_APP->getScaleX(1),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				break;
			case IDC_BUTTON_EJECT:
			{
				if (currentViewedDevice->isCloudDevice)
				{
					if (currentViewedDevice->dev->extraActions(DEVICE_DOES_NOT_SUPPORT_REMOVE,0,0,0))
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}
				}

				wchar_t buffer[128] = {0};
				GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
				LRESULT idealSize = MLSkinnedButton_GetIdealSize(pl->hwnd, buffer);
				LONG width = LOWORD(idealSize) + WASABI_API_APP->getScaleX(6);
				SETLAYOUTPOS(pl, rg.right - width + WASABI_API_APP->getScaleX(2),
							 rg.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)),
							 width, WASABI_API_APP->getScaleY(HIWORD(idealSize)));
				pl->flags |= ((rg.right - rg.left) > width) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				if (SWP_SHOWWINDOW & pl->flags) rg.right -= (pl->cx + WASABI_API_APP->getScaleX(3));
				break;
			}
			case IDC_BUTTON_AUTOFILL:
				if (currentViewedDevice->isCloudDevice)
				{
					pl->flags |= SWP_HIDEWINDOW;
					break;
				}
			case IDC_BUTTON_SYNC:
				if (currentViewedDevice->isCloudDevice)
				{
					if (!lstrcmpiA(currentViewedDevice->GetName(), "all_sources"))
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}
				}
			case IDC_BUTTON_PLAY:
			case IDC_BUTTON_ENQUEUE:
			case IDC_BUTTON_CUSTOM:
			case IDC_BUTTON_SORT:
			{
				if (IDC_BUTTON_CUSTOM != pl->id || customAllowed)
				{
					if (groupBtn && (pl->id == IDC_BUTTON_PLAY) && (enqueuedef == 1))
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if (groupBtn && (pl->id == IDC_BUTTON_ENQUEUE) && (enqueuedef != 1))
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if (groupBtn && (pl->id == IDC_BUTTON_PLAY || pl->id == IDC_BUTTON_ENQUEUE) && customAllowed)
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					wchar_t buffer[128] = {0};
					GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(pl->hwnd, buffer);
					LONG width = LOWORD(idealSize) + WASABI_API_APP->getScaleX(6);
					SETLAYOUTPOS(pl, rg.left, rg.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)),
								 width, WASABI_API_APP->getScaleY(HIWORD(idealSize)));
					pl->flags |= ((rg.right - rg.left) > width) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
					if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(4));
				}
				else
					pl->flags |= SWP_HIDEWINDOW;
				break;
			}
			case IDC_STATUS:
				SETLAYOUTPOS(pl, rg.left, rg.bottom - WASABI_API_APP->getScaleY(18), rg.right - rg.left, WASABI_API_APP->getScaleY(17));
				if (SWP_SHOWWINDOW & pl->flags) rg.top = (pl->y + pl->cy + WASABI_API_APP->getScaleY(1));
				break;
			case IDC_HDELIM:
				divCY = ri.bottom - ri.top;
				if (divY > (rg.bottom - WASABI_API_APP->getScaleY(70))) { divY = rg.bottom - divCY; m_nodrawtopborders = 2; }
				else if (divY < (rg.top + WASABI_API_APP->getScaleY(36))) { divY = rg.top; m_nodrawtopborders = 1; }
				else  m_nodrawtopborders = 0;
				SETLAYOUTPOS(pl, rg.left, divY, rg.right - rg.left + WASABI_API_APP->getScaleX(2), (ri.bottom - ri.top));
				break;
			case IDC_LIST_TRACKS:
				pl->flags |= (rg.top < rg.bottom) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;

				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1),
							 (rg.right - rg.left) + WASABI_API_APP->getScaleX(1),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(2));

				break;
			case IDC_LIST_ARTIST:
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), divX, (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				rg.left += pl->cx;
				break;
			case IDC_VDELIM:
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), (ri.right - ri.left), (rg.bottom - rg.top));
				rg.left += pl->cx;
				break;
			case IDC_LIST_ALBUM:
				if(numFilters == 3)
				{
					BOOL hide = ((divX2 - divX - 1) < WASABI_API_APP->getScaleX(15));
					pl->flags |= hide ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;
					SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), (hide ? 0 : divX2 - divX - WASABI_API_APP->getScaleX(1)),
								 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				}
				else { SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), (rg.right - rg.left) + WASABI_API_APP->getScaleX(1),
									(rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1)); }
				rg.left += pl->cx;
				break;
			case IDC_VDELIM2:
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), (numFilters == 2 ? 0 : ri.right - ri.left), (rg.bottom - rg.top));
				rg.left += pl->cx;
				break;
			case IDC_LIST_ALBUM2:
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), (rg.right - rg.left) + WASABI_API_APP->getScaleX(1),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(2));
				break;
		}
		SETLAYOUTFLAGS(pl, ri);
		if (LAYOUTNEEEDUPDATE(pl))
		{
			if (SWP_NOSIZE == ((SWP_HIDEWINDOW | SWP_SHOWWINDOW | SWP_NOSIZE) & pl->flags) && 
				ri.left == (pl->x + offsetX) && ri.top == (pl->y + offsetY) && IsWindowVisible(pl->hwnd))
			{
				SetRect(&ri, pl->x, pl->y, pl->cx + pl->x, pl->y + pl->cy);
				ValidateRect(hwnd, &ri);
			}
			pl++;
		}
		else if ((fRedraw || (!offsetX && !offsetY)) && IsWindowVisible(pl->hwnd)) 
		{
			ValidateRect(hwnd, &ri);
			if (GetUpdateRect(pl->hwnd, NULL, FALSE)) 
			{
				if (!rgn) rgn = CreateRectRgn(0, 0, 0, 0);
				GetUpdateRgn(pl->hwnd, rgn, FALSE);
				OffsetRgn(rgn, pl->x, pl->y);
				InvalidateRgn(hwnd, rgn, FALSE);
			}
		}
	}

	if (pl != layout)
	{
		LAYOUT *pc;
		HDWP hdwp = BeginDeferWindowPos((INT)(pl - layout));
		for(pc = layout; pc < pl && hdwp; pc++)
		{
			hdwp = DeferWindowPos(hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags);
		}
		if (hdwp) EndDeferWindowPos(hdwp);

		if (!rgn) rgn = CreateRectRgn(0, 0, 0, 0);

		if (fRedraw) 
		{
			GetUpdateRgn(hwnd, rgn, FALSE);
			for(pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn) 
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(rgn, rgn, pc->rgn, RGN_OR);
				}
			}

			RedrawWindow(hwnd, NULL, rgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
		}
		if (g_rgnUpdate)
		{
			GetUpdateRgn(hwnd, g_rgnUpdate, FALSE);
			for(pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn) 
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(g_rgnUpdate, g_rgnUpdate, pc->rgn, RGN_OR);
				}
			}
		}

		for(pc = layout; pc < pl && hdwp; pc++) if (pc->rgn) DeleteObject(pc->rgn);
	}

	if (rgn) DeleteObject(rgn);
	ValidateRgn(hwnd, NULL);
}

static void WINAPI OnDividerMoved(HWND hwnd, INT nPos, LPARAM param)
{
	RECT rc;
	HWND hwndParent;
	hwndParent = GetParent(hwnd);
	KillTimer(hwndParent,400);
	SetTimer(hwndParent,400,1000,NULL);

	if (hwndParent)
	{
		GetClientRect(hwndParent, &rc);
		switch((INT)param)
		{
			case IDC_VDELIM:
				{
					adiv1pos = (nPos * 100000) / (rc.right - rc.left + 10);
					if(adiv1pos + 500 >= adiv3pos) adiv3pos = adiv1pos + 500;
				}
				break;
			case IDC_HDELIM:
				adiv2pos = (nPos * 100000) / (rc.bottom - rc.top);
				break;
			case IDC_VDELIM2:
				{
					adiv3pos = (nPos * 100000) / (rc.right - rc.left + 10);
					if(adiv3pos - 500 < adiv1pos) adiv1pos = adiv3pos - 500;
				}
				break;
		}
		LayoutWindows(hwndParent, TRUE, 0);
	}
}

static BOOL AttachDivider(HWND hwnd, BOOL fVertical, DIVIDERMOVED callback, LPARAM param)
{
	if (!hwnd) return FALSE;

	DIVIDER *pd = (DIVIDER*)calloc(1, sizeof(DIVIDER));
	if (!pd) return FALSE;

	pd->fUnicode = IsWindowUnicode(hwnd);
	pd->fnOldProc = (WNDPROC) ((pd->fUnicode) ? SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)div_newWndProc) :
												SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)div_newWndProc));
	if (!pd->fnOldProc || !SetPropW(hwnd, L"DIVDATA", pd))
	{
		free(pd);
		return FALSE;
	}
	pd->fVertical = fVertical;
	pd->param = param;
	pd->callback = callback;

	return TRUE;
}

static LRESULT CALLBACK div_newWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DIVIDER *pd;
	pd = GET_DIVIDER(hwnd);
	if (!pd) return (IsWindowUnicode(hwnd)) ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : DefWindowProcA(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
		case WM_DESTROY:
			RemovePropW(hwnd, L"DIVDATA");
			(pd->fUnicode) ? CallWindowProcW(pd->fnOldProc, hwnd, uMsg, wParam, lParam) : CallWindowProcA(pd->fnOldProc, hwnd, uMsg, wParam, lParam);
			(pd->fUnicode) ? SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)pd->fnOldProc) : SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)pd->fnOldProc);
			free(pd);
			return 0;
		case WM_LBUTTONDOWN: 
			pd->clickoffs = (pd->fVertical) ? LOWORD(lParam) : HIWORD(lParam);
			SetCapture(hwnd);
			break;
		case WM_LBUTTONUP:
			ReleaseCapture();
			break;
		case WM_SETCURSOR:
			SetCursor(LoadCursor(NULL, (pd->fVertical) ? IDC_SIZEWE : IDC_SIZENS));
			return TRUE;
		case WM_MOUSEMOVE:
			{
				RECT rw;
				GetWindowRect(hwnd, &rw);
				GetCursorPos(((LPPOINT)&rw) + 1);
				(pd->fVertical) ? rw.right -= pd->clickoffs : rw.bottom -= pd->clickoffs;

				if ((pd->fVertical && rw.left != rw.right) || (!pd->fVertical && rw.top != rw.bottom))
				{					
					MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), ((LPPOINT)&rw) + 1, 1);
					if (pd->callback) pd->callback(hwnd, (pd->fVertical) ? rw.right : rw.bottom, pd->param);
				}
			}
			break;
	}

	return (pd->fUnicode) ? CallWindowProcW(pd->fnOldProc, hwnd, uMsg, wParam, lParam) : CallWindowProcA(pd->fnOldProc, hwnd, uMsg, wParam, lParam);
}