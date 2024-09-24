#include "main.h"
#include "ViewFilter.h"
#include "resource.h"
#include "AlbumArtContainer.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include <algorithm>

wchar_t *emptyQueryListString = L"";
void emptyQueryListObject(queryListObject *obj)
{
	queryListItem *p = obj->Items;
	while (obj->Size-- > 0)
	{
		if (p->name && p->name != emptyQueryListString)
			ndestring_release(p->name);
		ndestring_release(p->albumGain);
		ndestring_release(p->gracenoteFileId);
		ndestring_release(p->genre);
		if (p->artist && p->artist != emptyQueryListString)
			ndestring_release(p->artist);

		if(p->art) { p->art->updateMsg.hwnd = 0; p->art->Release(); }
		p++;
	}
	obj->Size = 0;
}

void freeQueryListObject(queryListObject *obj)
{
	emptyQueryListObject(obj);
	free(obj->Items);
	obj->Items = 0;
	obj->Alloc = obj->Size = 0;
}

int reallocQueryListObject(queryListObject *obj) // 0 on success
{
	if (obj->Size >= obj->Alloc)
	{
		size_t old_Alloc = obj->Alloc;
		obj->Alloc = obj->Size + 32;
		queryListItem *data = (queryListItem*)realloc(obj->Items, sizeof(queryListItem) * obj->Alloc);
		if (data)
		{
			obj->Items = data;
		}
		else
		{
			data = (queryListItem*)malloc(sizeof(queryListItem) * obj->Alloc);
			if (data)
			{
				memcpy(data, obj->Items, sizeof(queryListItem) * old_Alloc);
				free(obj->Items);
				obj->Items = data;
			}
			else obj->Alloc = old_Alloc;
		}

		if (!obj->Items)
		{
			obj->Alloc = 0;
			return -1;
		}
	}
	return 0;
}

int ViewFilter::BuildSortFunc(const void *elem1, const void *elem2, const void *context)
{
	ViewFilter *sortFilter = (ViewFilter *)context;
	itemRecordW *a = (itemRecordW *)elem1;
	itemRecordW *b = (itemRecordW *)elem2;
	wchar_t ab[100] = {0}, bb[100] = {0};
	int v=WCSCMP_NULLOK(sortFilter->GroupText(a,ab,100),sortFilter->GroupText(b,bb,100));
	if (v)
		return v;
	if(sortFilter->nextFilter) 
		return WCSCMP_NULLOK(sortFilter->nextFilter->GroupText(a,ab,100),sortFilter->nextFilter->GroupText(b,bb,100));
	return 0;
}

static int sortFunc_cols(const void *elem1, const void *elem2)
{
  ListField * a = (ListField *)elem1;
  ListField * b = (ListField *)elem2;
	return a->pos - b->pos;
}
static bool sortFunc_cols_V2(const void* elem1, const void* elem2)
{
	return sortFunc_cols(elem1, elem2) < 0;
}
void ViewFilter::AddColumns() {
	ClearColumns();

	AddColumns2();

	for(unsigned int i=0; i<showncolumns.size(); i++){
		ListField *l = showncolumns[i];
		if(l->hidden) {
			hiddencolumns.push_back(l);
			showncolumns.erase(showncolumns.begin() + i);
			i--;
		}
	}
	//qsort(showncolumns.first(),showncolumns.size(),sizeof(void*),sortFunc_cols);
	std::sort(showncolumns.begin(), showncolumns.end(), sortFunc_cols_V2);

	for ( ListField *l_column : showncolumns )
		list->AddCol( l_column->name, l_column->width );
}

void ViewFilter::ClearColumns()
{
	for ( ListField *l_shown_column : showncolumns )
		delete l_shown_column;

	showncolumns.clear();


	for ( ListField *l_hidden_column : hiddencolumns )
		delete l_hidden_column;

	hiddencolumns.clear();
}

void ViewFilter::SaveColumnWidths()
{
	char *av = GetConfigId();
	for ( size_t i = 0; i < showncolumns.size(); i++ )
	{
		wchar_t configname[ 256 ] = { 0 };
		int field = showncolumns[ i ]->field;
		StringCchPrintfW( configname, ARRAYSIZE( configname ), L"%hs_col_%d", av, field );
		g_view_metaconf->WriteInt( configname, list->GetColumnWidth( i ) );
		StringCchPrintfW( configname, ARRAYSIZE( configname ), L"%hs_col_%d_pos", av, field );
		g_view_metaconf->WriteInt( configname, i );
		StringCchPrintfW( configname, ARRAYSIZE( configname ), L"%hs_col_%d_hidden", av, field );
		g_view_metaconf->WriteInt( configname, 0 );
	}

	for ( ListField *l_hidden_column : hiddencolumns )
	{
		wchar_t configname[ 256 ] = { 0 };
		StringCchPrintfW( configname, ARRAYSIZE( configname ), L"%hs_col_%d_hidden", av, l_hidden_column->field );
		g_view_metaconf->WriteInt( configname, 1 );
	}
}

void ViewFilter::ResetColumns()
{
	for ( unsigned int i = 0; i < hiddencolumns.size(); i++ )
	{
		showncolumns.push_back( hiddencolumns[ i ] );
		hiddencolumns.erase( hiddencolumns.begin() + i );
		i--;
	}

	for ( unsigned int i = 0; i < showncolumns.size(); i++ )
	{
		ListField *l = showncolumns[ i ];
		l->ResetPos();
		if ( l->hidden )
		{
			hiddencolumns.push_back( l );
			showncolumns.erase( showncolumns.begin() + i );
			i--;
		}
	}
	//qsort(showncolumns.first(),showncolumns.size(),sizeof(void*),sortFunc_cols);
	std::sort( showncolumns.begin(), showncolumns.end(), sortFunc_cols_V2 );

	for ( ListField *l_show_column : showncolumns )
		list->AddCol( l_show_column->name, l_show_column->width );
}

ListField::ListField(int field, int width0, const wchar_t * name, C_Config * config,char * av, bool hidden0, bool hiddenDefault, bool readini,int pos):field(field),name(name),hiddenDefault(hiddenDefault),hidden(hidden0),pos(pos),width(width0) {
	this->name = _wcsdup(name);
	if(readini) {
		wchar_t buf[100] = {0};
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%d", av, field);
		this->width = config->ReadInt(buf, width0);
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%d_pos", av, field);
		this->pos = config->ReadInt(buf, field);
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%d_hidden", av, field);
		this->hidden = config->ReadInt(buf, hidden0 ? 1: 0 ) != 0;
	}
}

ListField::ListField(int field, int width0, int name0, C_Config * config,char * av, bool hidden0, bool hiddenDefault, bool readini,int pos):field(field),name(name),hiddenDefault(hiddenDefault),hidden(hidden0),pos(pos),width(width0) {
	this->name = _wcsdup(WASABI_API_LNGSTRINGW(name0));
	if(readini) {
		wchar_t buf[100] = {0};
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%d", av, field);
		this->width = config->ReadInt(buf,width0);
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%d_pos", av, field);
		this->pos = config->ReadInt(buf,field);
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%d_hidden", av, field);
		this->hidden = config->ReadInt(buf,hidden0?1:0)!=0;
	}
}

void ListField::ResetPos() { pos = field; hidden = hiddenDefault;}

static INT_PTR CALLBACK custColumns_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND m_curlistbox_hwnd, m_availlistbox_hwnd;
	static ViewFilter *filter;

	switch (uMsg) {
		case WM_INITDIALOG:
			m_curlistbox_hwnd = GetDlgItem(hwndDlg, IDC_LIST1);
			m_availlistbox_hwnd = GetDlgItem(hwndDlg, IDC_LIST2);
			filter = (ViewFilter*)lParam;

			if (NULL != WASABI_API_APP)
			{
				if (NULL != m_curlistbox_hwnd)
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_curlistbox_hwnd, TRUE);
				if (NULL != m_availlistbox_hwnd)
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_availlistbox_hwnd, TRUE);
			}
		case WM_USER + 32:
		{
			for ( ListField *l : filter->showncolumns )
			{
				int r = SendMessageW( m_curlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)l->name );
				SendMessageW( m_curlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)l );
			}

			for ( ListField *l : filter->hiddencolumns )
			{
				int r = SendMessageW( m_availlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)l->name );
				SendMessageW( m_availlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)l );
			}
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_DEFS:
					SendMessage(m_curlistbox_hwnd, LB_RESETCONTENT, 0, 0);
					SendMessage(m_availlistbox_hwnd, LB_RESETCONTENT, 0, 0);
					filter->ResetColumns();
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
							int r = SendMessageW(m_curlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)c->name);
							SendMessageW(m_curlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)c);
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
							int r = SendMessageW(m_availlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)c->name);
							SendMessageW(m_availlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)c);
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
							int r = (INT)SendMessageW(m_curlistbox_hwnd, LB_INSERTSTRING, i, (LPARAM)c->name);
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
							int r = (INT)SendMessageW(m_curlistbox_hwnd, LB_INSERTSTRING, i, (LPARAM)c->name);
							SendMessage(m_curlistbox_hwnd, LB_SETITEMDATA, r, (LPARAM)c);
						}
					}
				}
				break;
				case IDOK:
					// read and apply changes...
					{
						filter->showncolumns.clear();
						filter->hiddencolumns.clear();
						int i;
						int l = (INT)SendMessage(m_curlistbox_hwnd, LB_GETCOUNT, 0, 0);
						for (i = 0;i < l;i++) {
							ListField* c = (ListField*)SendMessage(m_curlistbox_hwnd, LB_GETITEMDATA, i, 0);
							filter->showncolumns.push_back(c);
							c->pos=i;
							c->hidden=false;
						}
						l = (INT)SendMessage(m_availlistbox_hwnd, LB_GETCOUNT, 0, 0);
						for (i = 0;i < l;i++) {
							ListField* c = (ListField*)SendMessage(m_availlistbox_hwnd, LB_GETITEMDATA, i, 0);
							filter->hiddencolumns.push_back(c);
							c->hidden=true;
						}
						filter->SaveColumnWidths();
					}
					EndDialog(hwndDlg, 1);
					break;
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					break;

			}
			break;
		case WM_DESTROY:
			if (NULL != WASABI_API_APP)
			{
				if (NULL != m_curlistbox_hwnd)
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_curlistbox_hwnd, FALSE);
				if (NULL != m_availlistbox_hwnd)
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_availlistbox_hwnd, FALSE);
			}
			break;
	}
	return FALSE;
}


void ViewFilter::CustomizeColumns( HWND parent, BOOL showmenu )
{
	if ( showmenu )
	{
		HMENU menu = GetSubMenu( g_context_menus, 4 );
		POINT p;
		GetCursorPos( &p );
		int r = TrackPopupMenu( menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, p.x, p.y, 0, parent, NULL );
		if ( r != ID_HEADERWND_CUSTOMIZECOLUMNS ) return;
	}

	bool r = !!WASABI_API_DIALOGBOXPARAMW( IDD_CUSTCOLUMNS, parent, custColumns_dialogProc, (LPARAM)this );
	MSG msg;
	while ( PeekMessage( &msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) ); //eat return
	if ( r )
	{
		while ( ListView_DeleteColumn( list->getwnd(), 0 ) );
		for ( ListField *l_show_column : showncolumns )
			list->AddCol( l_show_column->name, l_show_column->width );
	}
}

HMENU ViewFilter::GetMenu(bool isFilter, int filterNum, C_Config *c, HMENU themenu) {
	HMENU menu = GetSubMenu(themenu, 5);
	wchar_t scrollConf[] = L"av0_hscroll";
	scrollConf[2] = (wchar_t)('0'+filterNum);
	BOOL enablescroll = c->ReadInt(scrollConf, 0);
	CheckMenuItem(menu,ID_FILTERHEADERWND_SHOWHORIZONTALSCROLLBAR,enablescroll?MF_CHECKED:MF_UNCHECKED);

	if(isFilter) {
		MENUITEMINFO m={sizeof(m),MIIM_ID,0};
		int i=0;
		while(GetMenuItemInfo(menu,i,TRUE,&m)) {
			m.wID |= (1+filterNum) << 16;
			SetMenuItemInfo(menu,i,TRUE,&m);
			i++;
		}
	}
	return menu;
}

void ViewFilter::ProcessMenuResult(int r, bool isFilter, int editFilter, C_Config *c, HWND hwndDlg) {
	int mid = (r >> 16);
	if(!isFilter && mid) return;
	if(isFilter && mid-1 != editFilter) return;
	r &= 0xFFFF;

	switch(r) {
		case ID_HEADERWND_CUSTOMIZECOLUMNS:
			CustomizeColumns(hwndDlg, FALSE);
			break;
		case ID_FILTERHEADERWND_SHOWHORIZONTALSCROLLBAR:
			{
				wchar_t scrollConf[] = L"av0_hscroll";
				scrollConf[2]=(wchar_t)('0'+editFilter);
				BOOL enablescroll = !c->ReadInt(scrollConf,0);
				g_view_metaconf->WriteInt(scrollConf,enablescroll);
				MLSkinnedScrollWnd_ShowHorzBar(list->getwnd(),enablescroll);
			}
			break;
	}
}