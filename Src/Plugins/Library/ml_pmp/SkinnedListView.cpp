#include "main.h"
#include "SkinnedListView.h"
#include "resource1.h"
#include "api__ml_pmp.h"
#include "./local_menu.h"
#include "../replicant/nx/nxstring.h"
#include <strsafe.h>

extern DeviceView * currentViewedDevice;
extern winampMediaLibraryPlugin plugin;
extern HMENU m_context_menus;
extern HINSTANCE cloud_hinst;
extern int IPC_GET_CLOUD_HINST;

static bool doneFirstInit=false;
int (*wad_getColor)(int idx);

#define SKIP_THE_AND_WHITESPACE(x) { while (!iswalnum(*x) && *x) x++; if (!_wcsnicmp(x,L"the ",4)) x+=4; while (*x == L' ') x++; }
extern int STRCMP_NULLOK(const wchar_t *pa, const wchar_t *pb);

SkinnedListView::SkinnedListView(ListContents * lc, int dlgitem, HWND libraryParent, HWND parent, bool enableHeaderMenu) 
	: enableHeaderMenu(enableHeaderMenu), skinlistview_handle(0), contents(0), headerWindow(0)
{
	if(!doneFirstInit) {
		*(void **)&wad_getColor=(void*)SendMessage(libraryParent,WM_ML_IPC,1,ML_IPC_SKIN_WADLG_GETFUNC);
		doneFirstInit=true;
	}
	this->dlgitem = dlgitem;
	this->contents = lc;
	this->libraryParent = libraryParent;
}

void SkinnedListView::UpdateList(bool softUpdate) {
	if(!softUpdate) { 
	    ListView_SetItemCount(listview.getwnd(),0);
		ListView_SetItemCount(listview.getwnd(),(contents ? contents->GetNumRows() : 0));
	}
	ListView_RedrawItems(listview.getwnd(),0,(contents ? contents->GetNumRows() - 1 : 0));
}

int SkinnedListView::GetFindItemColumn() {
	return contents->GetSortColumn();
}

HMENU SkinnedListView::GetMenu(bool isFilter, int filterNum, C_Config *c, HMENU themenu) {
	HMENU menu;

	menu = GetSubMenu(themenu, (FALSE != isFilter) ? 9 : 8);
	if (NULL == menu)
		return NULL;

	if(isFilter) 
	{
		MENUITEMINFO m={sizeof(m),MIIM_ID,0};
		int i, count;
		unsigned int filterMarker;

		filterMarker = (((unsigned char)(1+filterNum)) << 24);
		count = GetMenuItemCount(menu);
		for(i = 0; i < count; i++)
		{
			if (GetMenuItemInfo(menu,i,TRUE,&m)) 
			{
				m.wID = filterMarker | (m.wID & 0x00FFFFFF);
				SetMenuItemInfo(menu,i,TRUE,&m);
			}
		}

		wchar_t conf[100] = {0};
		StringCchPrintf(conf, ARRAYSIZE(conf), L"media_scroll_%d",filterNum);
		bool enablescroll = c->ReadInt(conf,0)!=0;

		CheckMenuItem(menu, 
				filterMarker | (ID_FILTERHEADERWND_SHOWHORIZONTALSCROLLBAR & 0x00FFFFFF),
				MF_BYCOMMAND | (enablescroll ? MF_CHECKED : MF_UNCHECKED));
	}

	return menu;
}

void SkinnedListView::ProcessMenuResult(int r, bool isFilter, int filterNum, C_Config *c, HWND parent) {
	int mid = (r >> 24);
	if(!isFilter && mid) return;
	if(isFilter && mid-1 != filterNum) return;
	r &= 0xFFFF;
	switch(r) {
		case ID_HEADERWND_CUSTOMIZECOLUMNS:
			{
				contents->CustomizeColumns(listview.getwnd(),FALSE);
				while(ListView_DeleteColumn(listview.getwnd(),0));
				for(int i=0; i < contents->GetNumColumns(); i++) 
					listview.AddCol(contents->GetColumnTitle(i),contents->GetColumnWidth(i));
			}
		break;

		case ID_FILTERHEADERWND_SHOWHORIZONTALSCROLLBAR:
		{
			wchar_t conf[100] = {0};
			StringCchPrintf(conf, ARRAYSIZE(conf), L"media_scroll_%d",filterNum);
			bool enablescroll = !c->ReadInt(conf,0);
			c->WriteInt(conf,enablescroll?1:0);

			if (FALSE != MLSkinnedScrollWnd_ShowHorzBar(listview.getwnd(), enablescroll))
			{
				RECT rect;
				if(FALSE != GetWindowRect(listview.getwnd(), &rect))
				{
					OffsetRect(&rect, -rect.left, -rect.top);

					SetWindowPos(listview.getwnd(), NULL, 0, 0, rect.right - 1, rect.bottom, 
									SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
					SetWindowPos(listview.getwnd(), NULL, 0, 0, rect.right, rect.bottom, 
									SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
					
					RedrawWindow(listview.getwnd(), NULL, NULL, 
									RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | 
									RDW_ERASENOW | RDW_UPDATENOW);
				}
			}
		}
		break;
	}
}

void SkinnedListView::InitializeFilterData(int filterNum, C_Config *config)
{
	wchar_t buffer[64] = {0};
	BOOL enableHorzScrollbar;

	if (NULL == config)
		return;

	if (filterNum < 0)
		return;

	if(FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), L"media_scroll_%d",filterNum)))
		return;

	enableHorzScrollbar = (FALSE != config->ReadInt(buffer, FALSE));
	if (FALSE != MLSkinnedScrollWnd_ShowHorzBar(listview.getwnd(), enableHorzScrollbar))
	{
		SetWindowPos(listview.getwnd(), NULL, 0, 0, 0, 0, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
}

LRESULT SkinnedListView::pmp_listview(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_NOTIFY)
	{
		LPNMHDR l=(LPNMHDR)lParam;
		switch (l->code)
		{
			case TTN_SHOW:
			{
				LVHITTESTINFO lvh = {0};
				GetCursorPos(&lvh.pt);
				ScreenToClient(hwnd, &lvh.pt);
				ListView_SubItemHitTest(hwnd, &lvh);

				ListContents * contents = (ListContents *)GetPropW(hwnd, L"pmp_list_info");
				if (lvh.iItem != -1 && lvh.iSubItem == contents->cloudcol)
				{
					LPTOOLTIPTEXTW tt = (LPTOOLTIPTEXTW)lParam;
					RECT r = {0};
					if (lvh.iSubItem)
						ListView_GetSubItemRect(hwnd, lvh.iItem, lvh.iSubItem, LVIR_BOUNDS, &r);
					else
					{
						ListView_GetItemRect(hwnd, lvh.iItem, &r, LVIR_BOUNDS);
						r.right = r.left + ListView_GetColumnWidth(hwnd, contents->cloudcol);
					}

					MapWindowPoints(hwnd, HWND_DESKTOP, (LPPOINT)&r, 2);
					SetWindowPos(tt->hdr.hwndFrom, HWND_TOPMOST, r.right, r.top + 2, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
					return 1;
				}
			}
			break;

			case TTN_NEEDTEXTW:
			{
				LVHITTESTINFO lvh = {0};
				GetCursorPos(&lvh.pt);
				ScreenToClient(hwnd, &lvh.pt);
				ListView_SubItemHitTest(hwnd, &lvh);

				static wchar_t tt_buf[256] = {L""};
				ListContents * contents = (ListContents *)GetPropW(hwnd, L"pmp_list_info");
				if (lvh.iItem != -1 && lvh.iSubItem == contents->cloudcol)
				{
					LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO)lParam;
					static int last_item = -1;

					if (last_item == lvh.iItem)
					{
						lpnmtdi->lpszText = tt_buf;
						return 0;
					}

					if (contents->cloud_cache[lvh.iItem] == 4)
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_UPLOAD_TO_SOURCE, tt_buf, ARRAYSIZE(tt_buf));
					}
					else
					{
						if (!cloud_hinst) cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);
						if (cloud_hinst && cloud_hinst != (HINSTANCE)1)
						{
							winampMediaLibraryPlugin *(*gp)();
							gp = (winampMediaLibraryPlugin * (__cdecl *)(void))GetProcAddress(cloud_hinst, "winampGetMediaLibraryPlugin");
							if (gp)
							{
								winampMediaLibraryPlugin *mlplugin = gp();
								if (mlplugin && (mlplugin->version >= MLHDR_VER_OLD && mlplugin->version <= MLHDR_VER))
								{
									WASABI_API_LNGSTRINGW_BUF(IDS_TRACK_AVAILABLE, tt_buf, ARRAYSIZE(tt_buf));

									int message = 0x405;
									wchar_t value[1024] = {0};
									songid_t s = contents->GetTrack(lvh.iItem);
									currentViewedDevice->dev->getTrackExtraInfo(s, L"filepath", value, ARRAYSIZE(value));
									if (!value[0])
									{
										message = 0x407;
										currentViewedDevice->dev->getTrackExtraInfo(s, L"metahash", value, ARRAYSIZE(value));
									}

									nx_string_t *out_devicenames = 0;
									size_t num_names = mlplugin->MessageProc(message, (INT_PTR)&value, (INT_PTR)&out_devicenames, 0);
									if (num_names > 0)
									{
										for (size_t i = 0; i < num_names; i++)
										{
											if (i > 0) StringCchCatW(tt_buf, ARRAYSIZE(tt_buf), L", ");
											StringCchCatW(tt_buf, ARRAYSIZE(tt_buf), out_devicenames[i]->string);
										}
									}
									else
									{
										WASABI_API_LNGSTRINGW_BUF(IDS_UPLOAD_TO_SOURCE, tt_buf, ARRAYSIZE(tt_buf));
									}
									if (out_devicenames)
										NXStringRelease(*out_devicenames);
								}
							}
						}
					}
					last_item = lvh.iItem;
					lpnmtdi->lpszText = tt_buf;

					// bit of a fiddle but it allows for multi-line tooltips
					//SendMessage(l->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 0);
				}
				else
					return CallWindowProcW((WNDPROC)GetPropW(hwnd, L"pmp_list_proc"), hwnd, uMsg, wParam, lParam);
			}
			return 0;
		}
	}

	return CallWindowProcW((WNDPROC)GetPropW(hwnd, L"pmp_list_proc"), hwnd, uMsg, wParam, lParam);
}


LRESULT SkinnedListView::pmp_listview_alt(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_NOTIFY)
	{
		LPNMHDR l=(LPNMHDR)lParam;
		switch (l->code)
		{
			case TTN_SHOW:
			{
				LVHITTESTINFO lvh = {0};
				GetCursorPos(&lvh.pt);
				ScreenToClient(hwnd, &lvh.pt);
				ListView_SubItemHitTest(hwnd, &lvh);

				ListContents * contents = (ListContents *)GetPropW(hwnd, L"pmp_list_info");
				if (lvh.iItem > 0 && lvh.iSubItem == contents->cloudcol)
				{
					LPTOOLTIPTEXTW tt = (LPTOOLTIPTEXTW)lParam;
					RECT r = {0};
					if (lvh.iSubItem)
						ListView_GetSubItemRect(hwnd, lvh.iItem, lvh.iSubItem, LVIR_BOUNDS, &r);
					else
					{
						ListView_GetItemRect(hwnd, lvh.iItem, &r, LVIR_BOUNDS);
						r.right = r.left + ListView_GetColumnWidth(hwnd, contents->cloudcol);
					}

					MapWindowPoints(hwnd, HWND_DESKTOP, (LPPOINT)&r, 2);
					SetWindowPos(tt->hdr.hwndFrom, HWND_TOPMOST, r.right, r.top + 2, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
					return 1;
				}
			}
			break;

			case TTN_NEEDTEXTW:
			{
				LVHITTESTINFO lvh = {0};
				GetCursorPos(&lvh.pt);
				ScreenToClient(hwnd, &lvh.pt);
				ListView_SubItemHitTest(hwnd, &lvh);

				static wchar_t tt_buf[256] = {L""};
				ListContents * contents = (ListContents *)GetPropW(hwnd, L"pmp_list_info");
				if (lvh.iItem > 0 && lvh.iSubItem == contents->cloudcol)
				{
					LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO)lParam;
					static int last_item = -1;

					if (last_item == lvh.iItem)
					{
						lpnmtdi->lpszText = tt_buf;
						return 0;
					}

					wchar_t temp[8] = {0};
					contents->GetCellText(lvh.iItem, lvh.iSubItem, temp, 8);
					int status = _wtoi(temp);
					if (status == 0 || status == 4)
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_ALL_TRACKS_PLAYABLE, tt_buf, ARRAYSIZE(tt_buf));
					}
					else if (status == 1)
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_ALL_TRACKS_PLAYABLE_HERE, tt_buf, ARRAYSIZE(tt_buf));
					}
					else if (status == 2)
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_SOME_TRACKS_PLAYABLE, tt_buf, ARRAYSIZE(tt_buf));
					}
					else if (status == 3)
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_NO_TRACKS_PLAYABLE, tt_buf, ARRAYSIZE(tt_buf));
					}
					last_item = lvh.iItem;
					lpnmtdi->lpszText = tt_buf;

					// bit of a fiddle but it allows for multi-line tooltips
					//SendMessage(l->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 0);
				}
				else
					return CallWindowProcW((WNDPROC)GetPropW(hwnd, L"pmp_list_proc"), hwnd, uMsg, wParam, lParam);
			}
			return 0;
		}
	}

	return CallWindowProcW((WNDPROC)GetPropW(hwnd, L"pmp_list_proc"), hwnd, uMsg, wParam, lParam);
}

BOOL SkinnedListView::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
		{
			#if defined(_UNICODE) || defined(UNICODE)
			SendMessage(hwndDlg,CCM_SETUNICODEFORMAT,TRUE,0);
			#endif

			HWND list = GetDlgItem(hwndDlg, dlgitem);
			headerWindow = (HWND)SendMessageW(list, LVM_GETHEADER, 0, 0L);
			listview.setwnd(list);

			// setup tooltip handling as needed
			if (dlgitem == IDC_LIST_ARTIST || dlgitem == IDC_LIST_ALBUM || dlgitem == IDC_LIST_ALBUM2)
			{
				if (!GetPropW(list, L"pmp_list_proc")) {
					SetPropW(list, L"pmp_list_proc", (HANDLE)SetWindowLongPtrW(list, GWLP_WNDPROC, (LONG_PTR)this->pmp_listview_alt));
					SetPropW(list, L"pmp_list_info", (HANDLE)this->contents);
				}
			}
			else if (dlgitem != IDC_LIST_TRANSFERS)
			{
				if (!GetPropW(list, L"pmp_list_proc")) {
					SetPropW(list, L"pmp_list_proc", (HANDLE)SetWindowLongPtrW(list, GWLP_WNDPROC, (LONG_PTR)this->pmp_listview));
					SetPropW(list, L"pmp_list_info", (HANDLE)this->contents);
				}
			}

			if(!wParam)
			{
				MLSKINWINDOW m = {0};
				m.skinType = SKINNEDWND_TYPE_LISTVIEW;
				m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
				m.hwndToSkin = listview.getwnd();
				MLSkinWindow(libraryParent, &m);
			}

			if (contents)
			{
				for(int i=0; i < contents->GetNumColumns(); i++)
				{
					if (contents->cloud)
					{
						listview.AddCol((i == contents->cloudcol ? L"" : contents->GetColumnTitle(i)),contents->GetColumnWidth(i));
					}
					else
					{
						listview.AddCol(contents->GetColumnTitle(i),contents->GetColumnWidth(i));
					}
				}
				if(contents->GetSortColumn() != -1) // display sort arrow
					SendMessage(headerWindow,WM_ML_IPC,MAKEWPARAM(contents->GetSortColumn(),!contents->GetSortDirection()),ML_IPC_SKINNEDHEADER_DISPLAYSORT);
				UpdateList();
			}
		}
		break;

		case WM_DISPLAYCHANGE:
			ListView_SetTextColor(listview.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMFG):RGB(0xff,0xff,0xff));
			ListView_SetBkColor(listview.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMBG):RGB(0x00,0x00,0x00));
			ListView_SetTextBkColor(listview.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMBG):RGB(0x00,0x00,0x00));
			listview.SetFont((HFONT)SendMessage(libraryParent, WM_ML_IPC, 66, ML_IPC_SKIN_WADLG_GETFUNC));
		break;

		case WM_DESTROY:
			if (contents)
			{
				for(int i=0; i<contents->GetNumColumns(); i++)
					if(contents->GetColumnWidth(i) != listview.GetColumnWidth(i))
						contents->ColumnResize(i,listview.GetColumnWidth(i));
			}
		break;

		case WM_NOTIFYFORMAT:
		return NFR_UNICODE;

		case WM_CONTEXTMENU:
		{
			POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
			HWND hwndFromChild = WindowFromPoint(pt);
			if (enableHeaderMenu && hwndFromChild == ListView_GetHeader(listview.getwnd())) {
				if(contents->CustomizeColumns(listview.getwnd(), TRUE)) {
					while (ListView_DeleteColumn(listview.getwnd(), 0));
					for(int i=0; i < contents->GetNumColumns(); i++) 
						listview.AddCol(contents->GetColumnTitle(i),contents->GetColumnWidth(i));
				}
			}
		}
		break;

		case WM_NOTIFY:
		{
			LPNMHDR l=(LPNMHDR)lParam;
			if (l->idFrom==dlgitem) {
				switch(l->code) {
					case NM_DBLCLK:
						break;
					case LVN_KEYDOWN:
						switch(((LPNMLVKEYDOWN)lParam)->wVKey)  {
							case 0x41: //A
								if(GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT)){
									int num=listview.GetCount();
									for(int x = 0; x < num; x ++) listview.SetSelected(x);
								}
							break;
							case 0x2E: //Delete
							break;
						}
					break;

					case LVN_ODFINDITEM:
					{
						NMLVFINDITEM *t = (NMLVFINDITEM *)lParam;
						int i=t->iStart;
						if (i >= contents->GetNumRows()) i=0;

						int cnt=contents->GetNumRows()-i;
						if (t->lvfi.flags & LVFI_WRAP) cnt+=i;

						while (cnt-->0) {
							wchar_t tmp[128]=L"";
							wchar_t *name=0;

							contents->GetCellText(i,GetFindItemColumn(),tmp,sizeof(tmp)/sizeof(wchar_t));
							name = tmp;

							if (!name) name=L"";
							else SKIP_THE_AND_WHITESPACE(name)

							if (t->lvfi.flags & (4|LVFI_PARTIAL)) {
								if (!_wcsnicmp(name,t->lvfi.psz,lstrlen(t->lvfi.psz))) {
									SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,i);
									return 1;
								}
							}
							else if (t->lvfi.flags & LVFI_STRING) {
								if (!STRCMP_NULLOK(name,t->lvfi.psz)) {
									SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,i);
									return 1;
								}
							}
							else {
								SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,-1);
								return 1;
							}
							if (++i == contents->GetNumRows()) i=0;
						}
						SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,-1);
						return 1;
					}
					break;

					case LVN_GETDISPINFOW:
					{
						NMLVDISPINFO *lpdi = (NMLVDISPINFO*) lParam;
						int item=lpdi->item.iItem;
						if (item < 0 || contents && item >= contents->GetNumRows()) return 0;
						if (lpdi->item.mask & LVIF_TEXT) {
							lpdi->item.pszText[0]=0;
							contents->GetCellText(item,lpdi->item.iSubItem,lpdi->item.pszText,lpdi->item.cchTextMax);
							// will cache the cloud status for use in drawing later on (not ideal but it'll do for now)
							if (lpdi->item.iSubItem == contents->cloudcol) contents->cloud_cache[item] = _wtoi(lpdi->item.pszText);
						}
					}
					break;

					case LVN_COLUMNCLICK:
					{
						NMLISTVIEW *p=(NMLISTVIEW*)lParam;
						contents->ColumnClicked(p->iSubItem);
						if(contents->GetSortColumn() != -1) {
							SendMessage(headerWindow,WM_ML_IPC,MAKEWPARAM(contents->GetSortColumn(),!contents->GetSortDirection()),ML_IPC_SKINNEDHEADER_DISPLAYSORT);
						}
						UpdateList();
					}
					break;
				}
			}

			switch(l->code) {
				case HDN_ITEMCHANGING:
				{
					if (headerWindow == l->hwndFrom)
					{
						LPNMHEADERW phdr = (LPNMHEADERW)lParam;
						if (phdr->pitem && (HDI_WIDTH & phdr->pitem->mask) && phdr->iItem == contents->cloudcol)
						{
							INT width = phdr->pitem->cxy;
							if (MLCloudColumn_GetWidth(plugin.hwndLibraryParent, &width))
							{
								phdr->pitem->cxy = width;
							}
						}
						break;
					}
				}
			}
		}
		break;
	}
	return 0;
}

ListContents::~ListContents() {
	for(int i=0; i<fields.GetSize(); i++) delete ((ListField*)fields.Get(i));
	for(int i=0; i<hiddenfields.GetSize(); i++) delete ((ListField*)hiddenfields.Get(i));
}

static int sortFunc_cols(const void *elem1, const void *elem2) {
	ListField * a = *(ListField **)elem1;
	ListField * b = *(ListField **)elem2;
	return a->pos - b->pos;
}

void ListContents::SortColumns() {
	for(int i=0; i<fields.GetSize(); i++){
		ListField *l = (ListField*)fields.Get(i);
		if(l->hidden) {
			hiddenfields.Add(l);
			fields.Del(i--);
		}
	}
	qsort(fields.GetAll(),fields.GetSize(),sizeof(void*),sortFunc_cols);
}

typedef struct CustomizeColumnsCreateParam
{
	ListContents *list;
	HWND ownerWindow;
} CustomizeColumnsCreateParam;

static INT_PTR CALLBACK custColumns_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND m_curlistbox_hwnd, m_availlistbox_hwnd;
	static ListContents *list;

	switch (uMsg) {
		case WM_INITDIALOG:
		{
			CustomizeColumnsCreateParam *param;
			HWND centerWindow;

			m_curlistbox_hwnd = GetDlgItem(hwndDlg, IDC_LIST1);
			m_availlistbox_hwnd = GetDlgItem(hwndDlg, IDC_LIST2);

			param = (CustomizeColumnsCreateParam*)lParam;
			if (NULL != param)
			{
				list = param->list;
				centerWindow = param->ownerWindow;
				if (NULL == centerWindow)
					centerWindow = CENTER_OVER_ML_VIEW;
			}
			else
			{
				list = NULL;
				centerWindow  = CENTER_OVER_ML_VIEW;
			}

			if (FALSE != CenterWindow(hwndDlg, centerWindow))
			{
				if (FALSE == IS_INTRESOURCE(centerWindow))
				{
					wchar_t buffer[64] = {0};

					if (FALSE != GetClassName(centerWindow, buffer, ARRAYSIZE(buffer)) &&
						CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 
													NORM_IGNORECASE, buffer, -1, L"SysListView32", -1))
					{
						RECT rect;
						long top = 0;

						if (FALSE != GetClientRect(centerWindow, &rect))
						{
							MapWindowPoints(centerWindow, HWND_DESKTOP, (POINT*)&rect, 2);
							top = rect.top;

							HWND headerWindow = (HWND)SendMessage(centerWindow, LVM_GETHEADER, 0, 0L);
							if (NULL != headerWindow && 
								(0 != (WS_VISIBLE & GetWindowLongPtr(headerWindow, GWL_STYLE))) &&
								GetWindowRect(headerWindow, &rect))
							{
								if (rect.top == top)
									top = rect.bottom;
							}
						}

						top += 12;
						if (FALSE != GetWindowRect(hwndDlg, &rect) &&
							rect.top != top)
						{
							SetWindowPos(hwndDlg, NULL, rect.left, top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
						}
					}
				}
				SendMessage(hwndDlg, DM_REPOSITION, 0, 0L);
			}
		}
		case WM_USER + 32:
		{
			int i;
			for (i=0; i<list->fields.GetSize(); i++) {
				ListField * l = (ListField *)list->fields.Get(i);
				int r = SendMessage(m_curlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)l->name);
				SendMessage(m_curlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)l);
			}
			for (i=0; i<list->hiddenfields.GetSize(); i++) {
				ListField * l = (ListField *)list->hiddenfields.Get(i);
				int r = SendMessage(m_availlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)l->name);
				SendMessage(m_availlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)l);
			}
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_DEFS:
					SendMessage(m_curlistbox_hwnd, LB_RESETCONTENT, 0, 0);
					SendMessage(m_availlistbox_hwnd, LB_RESETCONTENT, 0, 0);
					list->ResetColumns();
					SendMessage(hwndDlg, WM_USER + 32, 0, 0);
					break;
				case IDC_LIST2:
					if (HIWORD(wParam) != LBN_DBLCLK) {
						if (HIWORD(wParam) == LBN_SELCHANGE) {
							int r = SendMessage(m_availlistbox_hwnd, LB_GETSELCOUNT, 0, 0) > 0;
							EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), r);
						}
						return 0;
					}
				case IDC_BUTTON2:
					//add column
				{
					for (int i = 0;i < SendMessage(m_availlistbox_hwnd, LB_GETCOUNT, 0, 0);i++) {
						if (SendMessage(m_availlistbox_hwnd, LB_GETSEL, i, 0)) {
							ListField* c = (ListField*)SendMessage(m_availlistbox_hwnd, LB_GETITEMDATA, i, 0);
							if(!c) continue;
							SendMessage(m_availlistbox_hwnd, LB_DELETESTRING, i--, 0);
							int r = SendMessage(m_curlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)c->name);
							SendMessage(m_curlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)c);
						}
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), 0);
				}
				break;
				case IDC_LIST1:
					if (HIWORD(wParam) != LBN_DBLCLK) {
						if (HIWORD(wParam) == LBN_SELCHANGE) {
							int r = SendMessage(m_curlistbox_hwnd, LB_GETSELCOUNT, 0, 0) > 0;
							EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON3), r);
							EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON4), r);
							EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON5), r);
						}
						return 0;
					}
				case IDC_BUTTON3:
					//remove column
				{
					for (int i = 0;i < SendMessage(m_curlistbox_hwnd, LB_GETCOUNT, 0, 0);i++) {
						if (SendMessage(m_curlistbox_hwnd, LB_GETSEL, i, 0)) {
							ListField* c = (ListField*)SendMessage(m_curlistbox_hwnd, LB_GETITEMDATA, i, 0);
							if(!c) continue;
							SendMessage(m_curlistbox_hwnd, LB_DELETESTRING, i, 0);
							i--;
							int r = SendMessage(m_availlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)c->name);
							SendMessage(m_availlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)c);
						}
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON3), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON4), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON5), 0);
				}
				break;
				case IDC_BUTTON4:
					//move column up
				{
					for (int i = 0;i < (INT)SendMessage(m_curlistbox_hwnd, LB_GETCOUNT, 0, 0);i++)
					{
						if (i != 0 && (INT)SendMessage(m_curlistbox_hwnd, LB_GETSEL, i, 0))
						{
							ListField* c = (ListField*)SendMessage(m_curlistbox_hwnd, LB_GETITEMDATA, i - 1, 0);
							SendMessage(m_curlistbox_hwnd, LB_DELETESTRING, i - 1, 0);
							int r = (INT)SendMessage(m_curlistbox_hwnd, LB_INSERTSTRING, i, (LPARAM)c->name);
							SendMessage(m_curlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)c);
						}
					}
				}
				break;
				case IDC_BUTTON5:
					//move column down
				{
					int l = SendMessage(m_curlistbox_hwnd, LB_GETCOUNT, 0, 0);
					for (int i = l - 2;i >= 0;i--)
					{
						if (SendMessage(m_curlistbox_hwnd, LB_GETSEL, i, 0))
						{
							ListField* c = (ListField*)SendMessage(m_curlistbox_hwnd, LB_GETITEMDATA, i + 1, 0);
							SendMessage(m_curlistbox_hwnd, LB_DELETESTRING, i + 1, 0);
							int r = (INT)SendMessage(m_curlistbox_hwnd, LB_INSERTSTRING, i, (LPARAM)c->name);
							SendMessage(m_curlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)c);
						}
					}
				}
				break;
				case IDOK:
					// read and apply changes...
					{
						while(list->fields.GetSize()) list->fields.Del(0);
						while(list->hiddenfields.GetSize()) list->hiddenfields.Del(0);
						wchar_t buf[100] = {0};
						int i;
						int l = (INT)SendMessage(m_curlistbox_hwnd, LB_GETCOUNT, 0, 0);
						for (i = 0;i < l;i++) {
							ListField* c = (ListField*)SendMessage(m_curlistbox_hwnd, LB_GETITEMDATA, i, 0);
							list->fields.Add(c);
							c->pos=i;
							c->hidden=false;
							StringCchPrintf(buf, ARRAYSIZE(buf), L"colPos_%d",c->field);
							list->config->WriteInt(buf,i);
							StringCchPrintf(buf, ARRAYSIZE(buf), L"colHidden_%d",c->field);
							list->config->WriteInt(buf,0);
						}
						l = (INT)SendMessage(m_availlistbox_hwnd, LB_GETCOUNT, 0, 0);
						for (i = 0;i < l;i++) {
							ListField* c = (ListField*)SendMessage(m_availlistbox_hwnd, LB_GETITEMDATA, i, 0);
							list->hiddenfields.Add(c);
							c->hidden=true;
							StringCchPrintf(buf, ARRAYSIZE(buf), L"colHidden_%d",c->field);
							list->config->WriteInt(buf,1);
						}
						list->SortColumns();
					}
					EndDialog(hwndDlg, 1);
					break;
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					break;
			}
			break;
	}
	return FALSE;
}

bool ListContents::CustomizeColumns(HWND parent, BOOL showmenu) {
	if(!fields.GetSize()) return false;
	if(showmenu) {
		HMENU menu = GetSubMenu(m_context_menus, 8);
		POINT p;
		GetCursorPos(&p);
		int r = Menu_TrackSkinnedPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, p.x, p.y, parent, NULL);
		if(r != ID_HEADERWND_CUSTOMIZECOLUMNS) return false;
	}

	CustomizeColumnsCreateParam param;
	param.list = this;
	param.ownerWindow = parent;

	bool r = !!WASABI_API_DIALOGBOXPARAMW(IDD_CUSTCOLUMNS, parent, custColumns_dialogProc,(LPARAM)&param);
	MSG msg;
	while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return

	MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(parent), (this->cloudcol = -1));	// reset the cloud status column so it'll be correctly removed
	if (cloud)
	{
		// not pretty but it'll allow us to know the current
		// position of the cloud column for drawing purposes
		for(int i = 0; i < fields.GetSize(); i++)
		{
			if (!lstrcmpi(((ListField *)fields.Get(i))->name, L"cloud"))
			{
				this->cloudcol = ((ListField *)fields.Get(i))->pos;
				// update the cloud column
				MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(parent), this->cloudcol);
				break;
			}
		}
	}

	return r;
}

ListField::ListField(int field, int defwidth, wchar_t * name, C_Config * config, bool hidden):field(field),name(name),hiddenDefault(hidden) {
	wchar_t buf[100] = {0};
	StringCchPrintf(buf, ARRAYSIZE(buf), L"colWidth_%d",field);
	width = config->ReadInt(buf,defwidth);
	StringCchPrintf(buf, ARRAYSIZE(buf), L"colPos_%d",field);
	pos = config->ReadInt(buf,field==-1?-1:(field%100));
	StringCchPrintf(buf, ARRAYSIZE(buf), L"colHidden_%d",field);
	this->hidden = config->ReadInt(buf,hidden?1:0)!=0;
	this->name = _wcsdup(name);
}

void ListField::ResetPos() { pos = ((field==-1)?-1:(pos%100)); hidden = hiddenDefault;}

void ListContents::ResetColumns() {
	while(hiddenfields.GetSize()) { fields.Add(hiddenfields.Get(0)); hiddenfields.Del(0); }
	for (int i=0; i<fields.GetSize(); i++) ((ListField*)fields.Get(i))->ResetPos();
	SortColumns();
}

int ListContents::GetSortDirection() { return TRUE; }
int ListContents::GetSortColumn() { return -1; }
void ListContents::ColumnClicked(int col) {}
void ListContents::ColumnResize(int col, int newWidth) {}
void ListContents::GetInfoString(wchar_t * buf) { buf[0]=0; }