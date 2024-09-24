#include "main.h"
#include "api__ml_local.h"
#include "ml_local.h"
#include <windowsx.h>
#include "../nu/listview.h"
#include "resource.h"
#include "..\..\General\gen_ml/config.h"
#include "..\..\General\gen_ml/ml_ipc.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "..\..\General\gen_ml/gaystring.h"
#include "../nde/nde_c.h"
#include <shlwapi.h>
#include "SimpleFilter.h"
#include "AlbumFilter.h"
#include "AlbumArtFilter.h"
#include "../replicant/nu/AutoChar.h"
#include "../replicant/nu/AutoWide.h"
#include "../nu/CGlobalAtom.h"
#include <tataki/export.h>
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>

static CGlobalAtom PROPW_DIVDATA(L"DIVDATA");
#define DELIMSTR "|"
#define LDELIMSTR L"|"

#define MAX_FILTERS 3

ViewFilter *filter[MAX_FILTERS] = {0};

int numFilters = 0;
extern void queryStrEscape(const char *raw, GayString &str);
extern void FixAmps(wchar_t *str, size_t size);
static W_ListView m_list1;
static W_ListView m_list2;
static W_ListView m_list3;
static HWND m_media_hwnd;
static HWND m_hwnd;
GayStringW l_query;
static int IPC_LIBRARY_SENDTOMENU;
static int m_sort_by, m_sort_dir, m_sort_which;
static HRGN g_rgnUpdate = NULL;
static int offsetX = 0, offsetY = 0;

enum
{
	PLAY=0,
	ENQUEUE=1,
};

#define ARTIST 0x01
#define ALBUMARTIST 0x02
#define GENRE 0x03
#define PUBLISHER 0x04
#define COMPOSER 0x05
#define ALBUM 0x06
#define YEAR 0x07
#define ARTISTINDEX 0x08
#define ALBUMARTISTINDEX 0x09
#define PODCASTCHANNEL 0x0A
#define ALBUMART 0x0B
#define CATEGORY 0x0C
#define DIRECTOR 0x0D
#define PRODUCER 0x0E

#define MAKEVIEW_3FILTER(a, b, c) (a | (b << 8) | (c << 16))
#define MAKEVIEW_2FILTER(a, b) (a | (b << 8))

static ViewFilter * getFilter(int n, HWND hwndDlg, int dlgitem, C_Config *c)
{
	switch (n)
	{
		case 0: return new ArtistFilter();
		case 1: return new AlbumArtistFilter();
		case 2: return new GenreFilter();
		case 3: return new PublisherFilter();
		case 4: return new ComposerFilter();
		case 5: return new AlbumFilter();
		case 6: return new YearFilter();
		case 7: return new ArtistIndexFilter();
		case 8: return new AlbumArtistIndexFilter();
		case 9: return new PodcastChannelFilter();
		case 10: return new AlbumArtFilter(hwndDlg,dlgitem,c);
		case 11: return new CategoryFilter();
		case 12: return new DirectorFilter();
		case 13: return new ProducerFilter();
	}
	return new ArtistFilter();
}

const wchar_t *getFilterName(unsigned int filterId, wchar_t *buffer, size_t bufferSize)   // for config
{
	const int filterNames[] = 
	{
		IDS_ARTIST,
		IDS_ALBUM_ARTIST,
		IDS_GENRE,
		IDS_PUBLISHER,
		IDS_COMPOSER,
		IDS_ALBUM,
		IDS_YEAR,
		IDS_ARTIST_INDEX,
		IDS_ALBUM_ARTIST_INDEX,
		IDS_PODCAST_CHANNEL,
		IDS_ALBUM_ART,
		IDS_CATEGORY,
		IDS_DIRECTOR,
		IDS_PRODUCER,
	};

	if (filterId >= ARRAYSIZE(filterNames))
		return NULL;

	return WASABI_API_LNGSTRINGW_BUF(filterNames[filterId], buffer, bufferSize);
}

static void mysearchCallbackAlbumUpdate(itemRecordW *items, int numitems, int user32, int *killswitch)
{
	if (killswitch && *killswitch) return;

	// user32 specifies whether or not to bother with rebuilding artist list
	if (user32 == 2 && numFilters == 2) return;

	if (user32 <= 2 && numFilters >= 3)
		filter[2]->Fill(items,numitems,killswitch,0);

	if (user32 <= 1 && numFilters >= 2)
		filter[1]->Fill(items,numitems,killswitch,numFilters >= 3 ? filter[2]->numGroups : 0);

	if (user32 == 0 && numFilters >= 1)
		filter[0]->Fill(items, numitems, killswitch,numFilters >= 2 ? filter[1]->numGroups : 0);

	PostMessage(m_hwnd, WM_APP + 3, 0x69, user32);
}

static int makeFilterQuery(GayStringW *query, ViewFilter *f)
{
	int ret = 0;
	if (!(f->list->GetSelected(0) && f->HasTopItem()))
	{
		int c = f->list->GetCount();
		int selCount = SendMessageW(f->list->getwnd(), LVM_GETSELECTEDCOUNT, 0, 0L);

		if (selCount && ((f->HasTopItem()) ? selCount < (c - 1) : selCount < c))
		{
			int needor = 0, needclose = 0;
			for (int x = f->HasTopItem()?1:0; x < c; x ++)
			{
				if (f->list->GetSelected(x))
				{
					if (needor)
						query->Append(L"|");
					else
					{
						if (query->Get()[0])
							query->Append(L"&(");
						else
							query->Set(L"(");
						needclose = 1;
					}
					if (!f->MakeFilterQuery(x,query))
					{
						const wchar_t *val = f->GetText(x);
						if (val && *val)
						{
							query->Append(f->GetField());
							query->Append(L" ");
							query->Append(f->GetComparisonOperator());
							query->Append(L" \"");
							GayStringW escaped;
							queryStrEscape(val, escaped);
							query->Append(escaped.Get());
							query->Append(L"\"");
						}
						else
						{
							const wchar_t *field = f->GetField();
							query->Append(field);
							query->Append(L" = \"\" | ");
							query->Append(field);
							query->Append(L" ISEMPTY");
						}
					}
					ret++;
					needor = 1;
				}
			}
			if (needclose) query->Append(L")");
		}
	}
	return ret;
}

static void getParentPlusSearchQuery(GayStringW *gs)
{
	extern wchar_t* m_query;
	wchar_t *parq = m_query;

	if (parq && parq[0])
	{
		gs->Set(L"(");
		gs->Append(parq);
		gs->Append(L")");
	}
	else gs->Set(L"");

	GayStringW query;
	wchar_t buf[2048] = {0};
	GetWindowTextW(GetDlgItem(m_hwnd, IDC_QUICKSEARCH), buf, ARRAYSIZE(buf));
	if (buf[0]) makeQueryStringFromText(&query, buf);

	if (query.Get() && query.Get()[0])
	{
		if (gs->Get()[0])
		{
			gs->Append(L" & (");
			gs->Append(query.Get());
			gs->Append(L")");
		}
		else gs->Set(query.Get());
	}
}


static void playList(int enqueue, int pane)
{
	GayStringW query;
	getParentPlusSearchQuery(&query);

	for (int i=0; i<=pane; i++)
		makeFilterQuery(&query,filter[i]);

	if (!g_table) return;

	main_playQuery(g_view_metaconf, query.Get(), enqueue);
}

static void playRandomList(int enqueue, int pane)
{
	static int inited = 0;
	if (!inited)
		srand((unsigned)(time(0)));
	inited = 1;				
	int n;
	n = filter[pane]->list->GetCount() * rand()/RAND_MAX;
	// Martin> dunno if we should display the results in ML then? atm we are
	filter[pane]->list->UnselectAll();
	filter[pane]->list->SetSelected(n);
	filter[pane]->list->ScrollTo(n);
	playList(enqueue, pane);
}

static void buildRecordListW(itemRecordListW *obj, int pane)
{
	GayStringW query;
	getParentPlusSearchQuery(&query);

	for (int i=0; i<=pane; i++)
		makeFilterQuery(&query,filter[i]);

	if (!g_table) return;

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);
	NDE_Scanner_Query(s, query.Get());
	saveQueryToListW(g_view_metaconf, s, obj, 0, 0, (resultsniff_funcW)-1);
	NDE_Table_DestroyScanner(g_table, s);
	LeaveCriticalSection(&g_db_cs);
}

void UpdateRating_RowCache(const wchar_t *filename, int new_rating);
static bool rateList(int rate)
{
	GayStringW query;
	getParentPlusSearchQuery(&query);
	int valid = 0;
	for (int i=0; i<numFilters; i++)
		valid += makeFilterQuery(&query,filter[i]);
		
	if (valid)
	{
		EnterCriticalSection(&g_db_cs);
		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		NDE_Scanner_Query(s, query.Get());
		NDE_Scanner_First(s);
		do
		{
			nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILENAME);
			if (!f) break;
			// get filename here as calling later fails due to the edit for some reason
			// as was being called directly in the updateFileInfo(..) call previously..
			wchar_t *filename = NDE_StringField_GetString(f);
			if (!filename) break;
			// update before, so we can take advantage of row cache pointer equality (sometimes)
			UpdateRating_RowCache(filename, rate);
			NDE_Scanner_Edit(s);
			db_setFieldInt(s, MAINTABLE_ID_RATING, rate);
			NDE_Scanner_Post(s);
			if (g_config->ReadInt(L"writeratings", 0))
			{
				wchar_t buf[64] = {0};
				if (rate > 0)
				{
					wsprintfW(buf, L"%d", rate);
				}
				else
					buf[0] = 0;

				updateFileInfo(filename, DB_FIELDNAME_rating, buf);
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
			}
			
			g_table_dirty++;
		}
		while (NDE_Scanner_Next(s));

		NDE_Table_DestroyScanner(g_table, s);
		if (g_table_dirty) NDE_Table_Sync(g_table);
		g_table_dirty = 0;
		LeaveCriticalSection(&g_db_cs);

		// refresh media list
//		SendMessage(m_media_hwnd, WM_APP + 1, (WPARAM)0, (LPARAM)0);
		return true;
	}
	return false;
}

static LRESULT CALLBACK div_newWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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

BOOL AttachDivider(HWND hwnd, BOOL fVertical, DIVIDERMOVED callback, LPARAM param)
{
	if (!hwnd) return FALSE;

	DIVIDER *pd = (DIVIDER*)calloc(1, sizeof(DIVIDER));
	if (!pd) return FALSE;

	pd->fUnicode = IsWindowUnicode(hwnd);
	pd->fnOldProc = (WNDPROC)((pd->fUnicode) ? SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)div_newWndProc) :
											   SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)div_newWndProc));
	if (!pd->fnOldProc || !SetPropW(hwnd, PROPW_DIVDATA, pd))
	{
		free(pd);
		return FALSE;
	}
	pd->fVertical = fVertical;
	pd->param = param;
	pd->callback = callback;

	return TRUE;
}
#define GET_DIVIDER(hwnd) (DIVIDER*)GetPropW(hwnd, PROPW_DIVDATA)

static LRESULT CALLBACK div_newWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DIVIDER *pd;
	pd = GET_DIVIDER(hwnd);
	if (!pd) return (IsWindowUnicode(hwnd)) ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : DefWindowProcA(hwnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_DESTROY:
		RemovePropW(hwnd, PROPW_DIVDATA);
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

static int m_nodrawtopborders;
static int adiv1_nodraw, adiv2_nodraw;

#define DIVIDER_FILTER		0x1
#define DIVIDER_FILTER2		0x3
#define DIVIDER_MEDIA		0x2

static int div_filterpos, div_filter2pos, div_mediapos;
static BOOL g_displaysearch = TRUE;

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
	BOOL	*pHidden;
}LAYOUT, PLAYOUT;

typedef struct _LCTRL
{
	INT id;
	BOOL hidden;
} LCTRL;

#define SETLAYOUTPOS(_layout, _x, _y, _cx, _cy) { _layout->x=_x; _layout->y=_y;_layout->cx=_cx;_layout->cy=_cy;_layout->rgn=NULL; }
#define SETLAYOUTFLAGS(_layout, _r)																						\
	{																													\
		BOOL fVis;																										\
		fVis = (WS_VISIBLE & (LONG)GetWindowLongPtr(_layout->hwnd, GWL_STYLE));											\
		if (_layout->x == _r.left && _layout->y == _r.top) _layout->flags |= SWP_NOMOVE;									\
		if (_layout->cx == (_r.right - _r.left) && _layout->cy == (_r.bottom - _r.top)) _layout->flags |= SWP_NOSIZE;	\
		if (fVis && (_layout->cx < 1 || _layout->cy < 1)) { _layout->flags |= SWP_HIDEWINDOW; *_layout->pHidden = TRUE; }	\
		if (!fVis && *_layout->pHidden && _layout->cx > 0 && _layout->cy > 0) { _layout->flags |= SWP_SHOWWINDOW; *_layout->pHidden = FALSE; }	\
		if ((SWP_HIDEWINDOW & _layout->flags) && !fVis) _layout->flags &= ~SWP_HIDEWINDOW;								\
		if ((SWP_SHOWWINDOW & _layout->flags) && fVis) _layout->flags &= ~SWP_SHOWWINDOW;									\
	}
#define LAYOUTNEEEDUPDATE(_layout) ((SWP_NOMOVE | SWP_NOSIZE) != ((SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_SHOWWINDOW) & _layout->flags))

#define GROUP_MIN			0x1
#define GROUP_MAX			0x3
#define GROUP_SEARCH		0x1
#define GROUP_FILTER		0x2
#define GROUP_MEDIA			0x3

static void LayoutWindows(HWND hwnd, BOOL fRedraw)
{
	static LCTRL controls[] =
	{
		{GROUP_SEARCH, FALSE}, {IDC_BUTTON_ARTMODE, FALSE}, {IDC_BUTTON_VIEWMODE, FALSE}, {IDC_BUTTON_COLUMNS, FALSE}, {IDC_SEARCHCAPTION, FALSE}, {IDC_CLEAR, FALSE}, {IDC_QUICKSEARCH, FALSE},
		{GROUP_MEDIA, FALSE}, {IDC_HDELIM, FALSE}, {IDD_VIEW_MEDIA, FALSE},
		{GROUP_FILTER, FALSE}, {IDC_LIST1, FALSE}, {IDC_DIV1, FALSE}, {IDC_LIST2, FALSE}, {IDC_DIV2, FALSE}, {IDC_LIST3, FALSE}
	};

	INT index, divY, divX, divX2;
	RECT rc, rg, ri;
	LAYOUT layout[sizeof(controls)/sizeof(controls[0])], *pl;
	BOOL skipgroup;
	HRGN rgn = NULL;

	GetClientRect(hwnd, &rc);
	if (rc.bottom == rc.top || rc.right == rc.left) return;

	SetRect(&rg, rc.left, rc.top, rc.right, rc.top);

	pl = layout;
	skipgroup = FALSE;

	divY = (div_mediapos * (rc.bottom- rc.top)) / 100000;
	divX = numFilters==1? rc.right : (div_filterpos * (rc.right- rc.left)) / 100000;
	divX2 = numFilters==3? (div_filter2pos * (rc.right- rc.left)) / 100000  : rc.right;

	if (divX > divX2 && numFilters==3)
	{
		div_filterpos=33333;
		div_filter2pos=66667;
		divX = (div_filterpos * (rc.right- rc.left)) / 100000;
		divX2 = (div_filter2pos * (rc.right- rc.left)) / 100000;
	}

	InvalidateRect(hwnd, NULL, TRUE);

	for (index = 0; index < sizeof(controls) / sizeof(LCTRL); index++)
	{
		if (controls[index].id >= GROUP_MIN && controls[index].id <= GROUP_MAX) // group id
		{
			skipgroup = FALSE;
			switch (controls[index].id)
			{
			case GROUP_SEARCH:
				if (g_displaysearch)
				{
					wchar_t buffer[128] = {0};
					HWND ctrl = GetDlgItem(hwnd, IDC_CLEAR);
					GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

					SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1),
							rc.top + WASABI_API_APP->getScaleY(2),
							rc.right - WASABI_API_APP->getScaleX(2),
							rc.top + WASABI_API_APP->getScaleY(HIWORD(idealSize)+1));
					rc.top = rg.bottom + WASABI_API_APP->getScaleY(3);
				}
				skipgroup = !g_displaysearch;
				break;

			case GROUP_MEDIA:
				m_nodrawtopborders = 0;
				if (divY > (rc.bottom - WASABI_API_APP->getScaleY(85)))
				{
					RECT rw;
					GetWindowRect(GetDlgItem(hwnd, IDC_HDELIM), &rw);
					divY = rc.bottom - (rw.bottom - rw.top);
				}
				else if (divY < (rc.top + WASABI_API_APP->getScaleY(24)))
				{
					divY = rc.top; m_nodrawtopborders = 1;
				}

				SetRect(&rg, rc.left, divY, rc.right, rc.bottom);
				rc.bottom = rg.top;
				break;

			case GROUP_FILTER:
				if (divX < (rc.left + WASABI_API_APP->getScaleX(15)))
				{
					divX = rc.left;
					adiv1_nodraw = 1;
				}
				// fixes moving the dividers over to the far right so that the
				// 'docking' is consistant in both the two and three pain view
				else if (divX > (rc.right - (WASABI_API_APP->getScaleX(17)*(numFilters-1)+(numFilters==2?WASABI_API_APP->getScaleX(10):0))))
				{
					RECT rw;
					GetWindowRect(GetDlgItem(hwnd, IDC_DIV1), &rw);
					divX = rc.right - (rw.right - rw.left)*(numFilters-1);
					adiv1_nodraw = 2;
				}
				else adiv1_nodraw = 0;

				if (divX2 < (rc.left + WASABI_API_APP->getScaleX(15)*(numFilters-2)))
				{
					divX2 = rc.left;
					adiv2_nodraw = 1;
				}
				else if (divX2 > (rc.right - WASABI_API_APP->getScaleX(18*2)))
				{
					RECT rw;
					GetWindowRect(GetDlgItem(hwnd, IDC_DIV2), &rw);
					divX2 = rc.right - (rw.right - rw.left);
					adiv2_nodraw = 2;
				}
				else adiv2_nodraw = 0;

				SetRect(&rg, rc.left, rc.top, rc.right, rc.bottom);
				break;
			}
			continue;
		}
		if (skipgroup) continue;

		pl->id = controls[index].id;
		pl->hwnd = GetDlgItem(hwnd, pl->id);
		pl->pHidden = &controls[index].hidden;
		if (!pl->hwnd) continue;

		GetWindowRect(pl->hwnd, &ri);
		MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2);
		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS;

		switch (pl->id)
		{
			case IDC_BUTTON_ARTMODE:
				if(numFilters == 1)
				{
					SETLAYOUTPOS(pl, 0, 0, 0, 0);
					break;
				}
			case IDC_BUTTON_VIEWMODE:
			case IDC_BUTTON_COLUMNS:
				pl->flags |= (rg.top < rg.bottom) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.left, rg.top, (ri.right - ri.left), (rg.bottom - rg.top));
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(1));
				break;
			case IDC_SEARCHCAPTION:
			{
				wchar_t buffer[128] = {0};
				GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
				LRESULT idealSize = MLSkinnedStatic_GetIdealSize(pl->hwnd, buffer);

				SETLAYOUTPOS(pl, rg.left + WASABI_API_APP->getScaleX(6),
							 rg.top + WASABI_API_APP->getScaleY(1),
							 WASABI_API_APP->getScaleX(LOWORD(idealSize)),
							 (rg.bottom - rg.top));
				rg.left += (pl->cx + WASABI_API_APP->getScaleX(8));
				break;
			}
			case IDC_CLEAR:
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
			case IDC_QUICKSEARCH:
				pl->flags |= (rg.right > rg.left) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left - WASABI_API_APP->getScaleX(1),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				break;
			case IDC_LIST1:
				SETLAYOUTPOS(pl, rg.left + WASABI_API_APP->getScaleX(1),
							 rg.top + WASABI_API_APP->getScaleY(1),
							 (numFilters == 1 ? rg.right - rg.left - WASABI_API_APP->getScaleX(3) : divX),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				rg.left += pl->cx;
				break;
			case IDC_DIV1:
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), ri.right - ri.left, (rg.bottom - rg.top));
				rg.left += pl->cx;
				break;
			case IDC_LIST2:
				if (numFilters == 3)
				{
					SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), divX2 - rg.left, (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				}
				else
				{
					SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), (rg.right - rg.left) - WASABI_API_APP->getScaleX(3), (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				}
				rg.left += pl->cx;
				break;
			case IDC_DIV2:
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), ri.right - ri.left, (rg.bottom - rg.top));
				rg.left += pl->cx;
				break;
			case IDC_LIST3:
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1), (rg.right - rg.left) - WASABI_API_APP->getScaleX(3), (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				break;
			case IDC_HDELIM:
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left - WASABI_API_APP->getScaleX(2), (ri.bottom - ri.top));
				rg.top += pl->cy;
				break;
			case IDD_VIEW_MEDIA:
				pl->flags |= (rg.top < rg.bottom) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				if ((SWP_HIDEWINDOW & pl->flags) && !IsWindowVisible(pl->hwnd)) continue;
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left, (rg.bottom - rg.top));
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
				if (!rgn) rgn = CreateRectRgn(0,0,0,0);
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
		for (pc = layout; pc < pl && hdwp; pc++)
		{
			if (IDD_VIEW_MEDIA == pc->id)
			{
				SetRect(&ri, pc->x, pc->y, pc->x + pc->cx, pc->y + pc->cy);
				ValidateRect(hwnd, &ri);
				pc->rgn = CreateRectRgn(0, 0, pc->cx, pc->cy);
				GetWindowRect(pc->hwnd, &ri);
				MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&ri, 1);
				SendMessage(pc->hwnd, WM_USER + 0x201, MAKEWPARAM(offsetX + (pc->x - ri.left), offsetY + (pc->y - ri.top)), (LPARAM)pc->rgn);
			}
			hdwp = DeferWindowPos(hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags);
		}
		if (hdwp) EndDeferWindowPos(hdwp);

		if (!rgn) rgn = CreateRectRgn(0,0,0,0);

		for (pc = layout; pc < pl && hdwp; pc++)
		{
			switch (pc->id)
			{
			case IDD_VIEW_MEDIA:
				SendMessage(pc->hwnd, WM_USER + 0x201, 0, 0L);
				break;
			}
		}

		if (fRedraw)
		{
			GetUpdateRgn(hwnd, rgn, FALSE);
			for (pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn)
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(rgn, rgn, pc->rgn, RGN_OR);
				}
			}
			RedrawWindow(hwnd, NULL, rgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE | RDW_NOINTERNALPAINT | RDW_ALLCHILDREN);
		}
		if (g_rgnUpdate)
		{
			GetUpdateRgn(hwnd, g_rgnUpdate, FALSE);
			for (pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn)
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(g_rgnUpdate, g_rgnUpdate, pc->rgn, RGN_OR);
				}
			}
		}

		for (pc = layout; pc < pl && hdwp; pc++)
			if (pc->rgn) DeleteObject(pc->rgn);
	}

	if (rgn) DeleteObject(rgn);
	ValidateRgn(hwnd, NULL);
}

static void WINAPI OnDividerMoved(HWND hwnd, INT nPos, LPARAM param)
{
	RECT rc;
	HWND hwndParent;
	hwndParent = GetParent(hwnd);
	if (hwndParent)
	{
		GetClientRect(hwndParent, &rc);
		switch ((INT)param)
		{
		case DIVIDER_FILTER:
			div_filterpos = (nPos * 100000) / (rc.right - rc.left);
			if (div_filterpos + 500 > div_filter2pos) div_filter2pos = div_filterpos+500;
			break;
		case DIVIDER_FILTER2:
			div_filter2pos = (nPos * 100000) / (rc.right - rc.left);
			if (div_filter2pos - 500 < div_filterpos) div_filterpos = div_filter2pos-500;
			break;
		case DIVIDER_MEDIA:
			div_mediapos = (nPos * 100000) / (rc.bottom - rc.top);
			break;
		}
		LayoutWindows(hwndParent, TRUE);
	}
}

static int ignore_selections;

//returns true if a selection has been set which needs to be propagated to the next pane
static bool UpdateFilterPane(int pane, char* selconfigname)
{
	filter[pane]->SortResults(g_view_metaconf, pane, 0);
	
	ListView_SetItemCount(filter[pane]->list->getwnd(), filter[pane]->Size());
	InvalidateRect(filter[pane]->list->getwnd(), NULL, TRUE);

	ignore_selections = 1;
	char *_selstr = g_config->ReadInt(L"remembersearch", 0) ? g_view_metaconf->ReadString(selconfigname, "") : NULL;
	wchar_t * selstr = AutoWide(_selstr, CP_UTF8);
	int a = 0;
	if (selstr && *selstr)
	{
		int len = (wcslen(selstr) + 2);
		wchar_t *t = (wchar_t*)calloc(len, sizeof(wchar_t));
		wcsncpy(t, selstr, len);
		g_view_metaconf->WriteString(selconfigname, "");

		t[wcslen(t)+1] = 0;
		wchar_t *outp = t;
		wchar_t *inp = t;
		while (inp && *inp)
		{
			if (*inp == *LDELIMSTR)
			{
				if (inp[1] == *LDELIMSTR)
				{
					*outp++ = *LDELIMSTR; // unescape the output
					inp += 2;
				}
				else
				{
					*outp++ = 0; inp++;
				}
			}
			else *outp++ = *inp++;
		}
		*outp = 0;

		int x;
		for (x = 1; x < filter[pane]->Size(); x ++)
		{
			wchar_t *p = t;
			while (p && *p)
			{
				if (!lstrcmpiW(filter[pane]->GetText(x), p)) break;
				p += wcslen(p) + 1;
			}
			if (p && *p)
			{
				if (!a)
				{
					ListView_SetSelectionMark(filter[pane]->list->getwnd(), x);
					ListView_SetItemState(filter[pane]->list->getwnd(), x, LVIS_FOCUSED, LVIS_FOCUSED);
					ListView_EnsureVisible(filter[pane]->list->getwnd(), x, NULL);
				}
				a++;
				filter[pane]->list->SetSelected(x);
			}
		}
		free(t);
	}
	if (a)
	{
		ignore_selections = 0;
		return true;
	}
	else
	{
		wchar_t buf[1024] = {0};
		GetDlgItemText(m_media_hwnd, IDC_QUICKSEARCH, buf, sizeof(buf));
		if (buf[0])
		{
			ignore_selections = 0;
			l_query.Set(L""); // force refresh
		}
		if(filter[pane]->HasTopItem())
			filter[pane]->list->SetSelected(0);
		ignore_selections = 0;
		return false;
	}
}

HBITMAP ConvertTo24bpp(HBITMAP bmp, int bpp)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbm24;
	BITMAP bm;

	GetObjectW(bmp, sizeof(BITMAP), &bm);

	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);

	void *bits;
	BITMAPINFOHEADER bi;

	ZeroMemory(&bi, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth= bm.bmWidth;
	bi.biHeight = -bm.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount= (WORD)bpp;

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

static __forceinline BYTE pm(int a, int b)
{
	return (BYTE)((a * b) / 0xff);
}

void SetToolbarButtonBitmap(HWND hwndDlg, int buttonrc, int bitmaprc, ARGB32 fc)
{
	// benski> we could use this if it wasn't for the lack of WASABI_API_IMGLDR on classic skin
	//         even though we have the resource loader ...
	// SkinBitmap *sbm = new SkinBitmap( bitmaprc);
	HBITMAP hbm1 = (HBITMAP)LoadImage(plugin.hDllInstance,MAKEINTRESOURCE(bitmaprc),IMAGE_BITMAP,0,0,LR_VGACOLOR);
	HBITMAP hbm = ConvertTo24bpp(hbm1,32);
	DeleteObject(hbm1);
	BITMAP bm;
	GetObjectW(hbm, sizeof(BITMAP), &bm);
	int l = bm.bmWidth * bm.bmHeight;
	ARGB32 *x = (ARGB32*)bm.bmBits;
	ARGB32 *end = x+l;
	BYTE r = (BYTE)(fc & 0x00ff0000 >> 16);
	BYTE g = (BYTE)(fc & 0x0000ff00 >> 8);
	BYTE b = (BYTE)(fc & 0x000000ff);
	while (x < end)
	{
		BYTE a = (BYTE)((~(*x))&0xff);
		*(x++) = (a<<24) | (pm(r,a)<<16) | (pm(g,a)<<8) | pm(b,a);
	}

	void * mem = malloc(bm.bmWidth*bm.bmHeight*sizeof(ARGB32));
	memcpy(mem,bm.bmBits,bm.bmWidth*bm.bmHeight*sizeof(ARGB32));

	SkinBitmap *sbm = new SkinBitmap((ARGB32*)mem,bm.bmWidth,bm.bmHeight);
	DeleteObject(hbm);
	HWND btn = GetDlgItem(hwndDlg,buttonrc);
	if (IsWindow(btn))
	{
		SkinBitmap* old = (SkinBitmap*)SetWindowLongPtr(btn,GWLP_USERDATA,(LONG_PTR)sbm);
		if (old)
		{
			if (old->getBits()) free(old->getBits()); delete old;
		}
		InvalidateRect(btn, NULL, TRUE);
	}
}

int GetFilter(int mode, int n)
{
	if (n==0) return ((mode & 0x0000ff));
	if (n==1)
	{
		int f=((mode & 0x00ff00) >> 8); if (f) return f; return 6;
	} // album
	if (n==2) return ((mode & 0xff0000) >> 16);
	return 0;
}

int GetNumFilters(int mode)
{
	if (mode == 0) return 0;
	if (mode > 0xffffff) return 1;
	if (GetFilter(mode,2) == 0) return 2;
	return 3;
}

typedef struct { int id, id2; } hi;

//extern "C"
//{
//void __cdecl do_help(HWND hwnd, UINT id, HWND hTooltipWnd);

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
//}

static int refresh;
INT_PTR CALLBACK view_audioDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int we_are_drag_and_dropping;
	static HMENU sendto_hmenu;
	static librarySendToMenuStruct s;

	hi helpinfo[]={
		{IDC_BUTTON_ARTMODE,IDS_AUDIO_BUTTON_TT1},
		{IDC_BUTTON_VIEWMODE,IDS_AUDIO_BUTTON_TT2},
		{IDC_BUTTON_COLUMNS,IDS_AUDIO_BUTTON_TT3},
	};
	DO_HELP();

	BOOL a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam); if (a) return a;
	if (numFilters>=1)
	{
		a = filter[0]->DialogProc(hwndDlg,uMsg,wParam,lParam); if (a) return a;
	}
	if (numFilters>=2)
	{
		a = filter[1]->DialogProc(hwndDlg,uMsg,wParam,lParam); if (a) return a;
	}
	if (numFilters>=3)
	{
		a = filter[2]->DialogProc(hwndDlg,uMsg,wParam,lParam); if (a) return a;
	}

	switch (uMsg)
	{
	case WM_INITMENUPOPUP:
		if (wParam)
		{ 
			if((HMENU)wParam == s.build_hMenu && s.mode == 1)
			{
				myMenu = TRUE;
				if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU) == (LRESULT)-1)
					s.mode = 2;
				myMenu = FALSE;
			}
			// only populate the average rating when needed otherwise it makes all of the menu slow
			else if((HMENU)wParam == GetSubMenu(GetSubMenu(g_context_menus, 1),4))
			{
				int ave_rating = 0, valid = 0;
				GayStringW query;
				getParentPlusSearchQuery(&query);
				for (int i=0; i<numFilters; i++)
				{
					valid += makeFilterQuery(&query,filter[i]);
				}
				EnterCriticalSection(&g_db_cs);
				nde_scanner_t q = NDE_Table_CreateScanner(g_table);
				NDE_Scanner_Query(q, query.Get());
				NDE_Scanner_First(q);
				int count = 0;
				do
				{
					nde_field_t f = NDE_Scanner_GetFieldByID(q, MAINTABLE_ID_FILENAME);
					if (!f) break;
					nde_field_t g = NDE_Scanner_GetFieldByID(q, MAINTABLE_ID_RATING);
					int nde_rating = NDE_IntegerField_GetValue(g);
					if(nde_rating > 0) ave_rating += nde_rating;
					count++;
				}
				while (NDE_Scanner_Next(q));

				NDE_Table_DestroyScanner(g_table, q);
				LeaveCriticalSection(&g_db_cs);

				if(ave_rating > 0)
				{
					float ave = ave_rating/(count*1.0f);
					int diff = ave_rating/count;
					int adjust = (ave-diff)>=0.5f;
					ave_rating = (int)(ave+adjust);
				}

				Menu_SetRatingValue((HMENU)wParam, (valid?ave_rating:-1));
			}
		}
		return 0;
	case WM_DISPLAYCHANGE:
	{
		ARGB32 fc = dialogSkinner.Color(WADLG_BUTTONFG) & 0x00FFFFFF;
		SetToolbarButtonBitmap(hwndDlg,IDC_BUTTON_ARTMODE,IDB_TOOL_ART,fc);
		SetToolbarButtonBitmap(hwndDlg,IDC_BUTTON_VIEWMODE,IDB_TOOL_MODE,fc);
		SetToolbarButtonBitmap(hwndDlg,IDC_BUTTON_COLUMNS,IDB_TOOL_COLS,fc);
		if (IsWindow(m_media_hwnd)) SendNotifyMessageW(m_media_hwnd, WM_DISPLAYCHANGE, wParam, lParam);
		UpdateWindow(hwndDlg);
		LayoutWindows(hwndDlg, TRUE);
	}
	return 0;
	case WM_INITDIALOG:
	{
		hwndSearchGlobal = GetDlgItem(hwndDlg, IDC_QUICKSEARCH);		// Set the hwnd for the search to a global so we can access it from the media view dialog
		ResumeCache();
		int numFilters0=GetNumFilters(m_query_mode);
		int f1 = GetFilter(m_query_mode,0);
		int f2 = GetFilter(m_query_mode,1);
		int f3 = GetFilter(m_query_mode,2);

		filter[0] = getFilter(f1-1,hwndDlg,IDC_LIST1,g_view_metaconf);
		filter[0]->list = &m_list1;
		filter[0]->nextFilter = NULL;
		filter[0]->list->setwnd(GetDlgItem(hwndDlg, IDC_LIST1));
		
		if (numFilters0>=2)
		{
			filter[1] = getFilter(f2-1,hwndDlg,IDC_LIST2,g_view_metaconf);
			filter[1]->list = &m_list2;
			filter[1]->list->setwnd(GetDlgItem(hwndDlg, IDC_LIST2));
			filter[1]->nextFilter = NULL;
			filter[0]->nextFilter = filter[1];
		}

		if (numFilters0>=3)
		{
			filter[2] = getFilter(f3-1,hwndDlg,IDC_LIST3,g_view_metaconf);
			filter[2]->list = &m_list3;
			filter[2]->list->setwnd(GetDlgItem(hwndDlg, IDC_LIST3));
			filter[2]->nextFilter = NULL;
			filter[1]->nextFilter = filter[2];
		}
		numFilters=numFilters0;

		m_hwnd = hwndDlg;
		getParentPlusSearchQuery(&l_query);

		for (int j=0; j<numFilters0; j++)
		{
			filter[j]->list->ForceUnicode();
			filter[j]->AddColumns();
		}

		div_mediapos = g_view_metaconf->ReadInt(L"adiv2pos", 50000);

		if (numFilters == 1)
		{
			div_filterpos = g_view_metaconf->ReadInt(L"adiv1pos", 100000);
			div_filter2pos = -1;
		}
		else if (numFilters == 2)
		{
			div_filterpos = g_view_metaconf->ReadInt(L"adiv1pos", 50000);
			div_filter2pos = -1;
		}
		else if (numFilters == 3)
		{
			div_filterpos = g_view_metaconf->ReadInt(L"adiv1pos", 33333);
			div_filter2pos = g_view_metaconf->ReadInt(L"adiv3pos", 66667);
			if (div_filterpos + div_filter2pos > 200000)  // sanity check?
			{
				div_filterpos = 33333;
				div_filter2pos = 66667;
			}
		}

		if (numFilters >= 2) AttachDivider(GetDlgItem(hwndDlg, IDC_DIV1), TRUE, OnDividerMoved, DIVIDER_FILTER);
		if (numFilters >= 3) AttachDivider(GetDlgItem(hwndDlg, IDC_DIV2), TRUE, OnDividerMoved, DIVIDER_FILTER2);
		AttachDivider(GetDlgItem(hwndDlg, IDC_HDELIM), FALSE, OnDividerMoved, DIVIDER_MEDIA);

		m_media_hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_MEDIA, hwndDlg, view_mediaDialogProc, (LPARAM)!g_config->ReadInt(L"audiorefine", 0));
		SetWindowLongPtr(m_media_hwnd, GWLP_ID, IDD_VIEW_MEDIA);

		MLSKINWINDOW m = {0};
		m.skinType = SKINNEDWND_TYPE_LISTVIEW;
		m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;

		m.hwndToSkin = m_list1.getwnd();
		MLSkinWindow(plugin.hwndLibraryParent, &m);
		m.hwndToSkin = m_list2.getwnd();
		MLSkinWindow(plugin.hwndLibraryParent, &m);
		if (numFilters == 3) 
		{
			m.hwndToSkin = m_list3.getwnd();
			MLSkinWindow(plugin.hwndLibraryParent, &m);
		}
		
		if (!g_view_metaconf->ReadInt(L"av0_hscroll",0)) MLSkinnedScrollWnd_ShowHorzBar(m_list1.getwnd(), FALSE);
		if (!g_view_metaconf->ReadInt(L"av1_hscroll",0)) MLSkinnedScrollWnd_ShowHorzBar(m_list2.getwnd(), FALSE);
		if (!g_view_metaconf->ReadInt(L"av2_hscroll",0) && numFilters == 3) MLSkinnedScrollWnd_ShowHorzBar(m_list3.getwnd(), FALSE);

		FLICKERFIX ff;
		ff.mode = FFM_ERASEINPAINT;
		m.skinType = SKINNEDWND_TYPE_AUTO;
		m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
		INT ffcl[] = { IDC_QUICKSEARCH, IDC_SEARCHCAPTION, IDC_CLEAR, IDC_BUTTON_ARTMODE, IDC_BUTTON_VIEWMODE, IDC_BUTTON_COLUMNS};
		for (int  i = 0; i < sizeof(ffcl) / sizeof(INT); i++)
		{
			ff.hwnd = GetDlgItem(hwndDlg, ffcl[i]);
			if (IsWindow(ff.hwnd))
			{
				SendMessage(mediaLibrary.library, WM_ML_IPC, (WPARAM)&ff, ML_IPC_FLICKERFIX);
				if (i < 3)
				{
					m.hwndToSkin = ff.hwnd;
					MLSkinWindow(plugin.hwndLibraryParent, &m);
				}
			}
		}

		SendMessageW(hwndDlg, WM_DISPLAYCHANGE, 0, 0L);

		// clear the media windows refine shit
		SetDlgItemText(m_media_hwnd, IDC_QUICKSEARCH, L"");
		SetDlgItemText(m_media_hwnd, IDC_SEARCHCAPTION, WASABI_API_LNGSTRINGW(IDS_REFINE));
		SetDlgItemText(m_media_hwnd, IDC_CLEAR, WASABI_API_LNGSTRINGW(IDS_CLEAR_REFINE));
		KillTimer(m_media_hwnd, UPDATE_QUERY_TIMER_ID); // keep it from researching :)

		if (g_config->ReadInt(L"remembersearch", 0))
		{
			char *query = g_view_metaconf->ReadString("lastquery_a_utf8", "");
			SetDlgItemText(hwndDlg, IDC_QUICKSEARCH, AutoWide(query, CP_UTF8));
			KillTimer(hwndDlg, 205);
		}
		for (int j=0; j<numFilters; j++)
			filter[j]->SortResults(g_view_metaconf, j, 0);

		{
			wchar_t buf[32] = {0};
			for (int i = 0; i < numFilters; i++)
			{
				int dlgitem = (i==0)?IDC_LIST1:((i==1)?IDC_LIST2:IDC_LIST3);
				StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_sort_by_%d", filter[i]->GetConfigId(), i);
				int l_sc = g_view_metaconf->ReadInt(buf, 0);
				StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_sort_dir_%d", filter[i]->GetConfigId(), i);
				int l_sd = g_view_metaconf->ReadInt(buf, 0);
				SendMessage((HWND)SendMessage(GetDlgItem(hwndDlg,dlgitem), LVM_GETHEADER, 0, 0L),WM_ML_IPC,MAKEWPARAM(l_sc,!l_sd),ML_IPC_SKINNEDHEADER_DISPLAYSORT);
			}
		}

		MLSKINWINDOW skin = {0};
		skin.hwndToSkin = hwndDlg;
		skin.skinType = SKINNEDWND_TYPE_AUTO;
		skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
		MLSkinWindow(plugin.hwndLibraryParent, &skin);
		return TRUE;
	}
	case WM_DRAWITEM:
	{
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON)
		{
			if (di->CtlID == IDC_BUTTON_ARTMODE || di->CtlID == IDC_BUTTON_VIEWMODE || di->CtlID == IDC_BUTTON_COLUMNS)
			{
				// draw the toolbar buttons!
				SkinBitmap* hbm = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,di->CtlID),GWLP_USERDATA);
				if (hbm && di->rcItem.left != di->rcItem.right && di->rcItem.top != di->rcItem.bottom)
				{
					DCCanvas dc(di->hDC);
					if (di->itemState & ODS_SELECTED) hbm->blitAlpha(&dc,di->rcItem.left+6,di->rcItem.top+5);
					else hbm->blitAlpha(&dc,di->rcItem.left+4,di->rcItem.top+3);
				}
				return TRUE;
			}
		}
	}
	break;
	case WM_TIMER:
		if (wParam == 165) // list3 sel change
		{
			KillTimer(hwndDlg, 165);
			if (numFilters < 3) break;
			GayStringW query;
			getParentPlusSearchQuery(&query);
			int r = makeFilterQuery(&query,filter[0]);
			r += makeFilterQuery(&query,filter[1]);
			r += makeFilterQuery(&query,filter[2]);

			if (wcscmp(query.Get(), l_query.Get()))
			{
				bgQuery_Stop();
				l_query.Set(query.Get());

				wchar_t buf[2048] = {0};
				GetDlgItemTextW(m_media_hwnd, IDC_QUICKSEARCH, buf, ARRAYSIZE(buf));
				if (buf[0]) makeQueryStringFromText(&query, buf);

				if (!refresh) SetDlgItemText(m_media_hwnd, IDC_QUICKSEARCH, L"");
				KillTimer(m_media_hwnd, UPDATE_QUERY_TIMER_ID); // keep it from researching :)

				EnterCriticalSection(&g_db_cs);
				if (m_media_scanner) NDE_Scanner_Query(m_media_scanner, query.Get());
				LeaveCriticalSection(&g_db_cs);

				if (r > 0) SendMessage(m_media_hwnd, WM_APP + 3, 0x666, 0); // notify the child to use the first item as a media lookup
				SendMessage(m_media_hwnd, WM_APP + 1, 0, 0);
			}
		}
		if (wParam == 166) // album sel change
		{
			KillTimer(hwndDlg, 166);

			GayStringW query;
			getParentPlusSearchQuery(&query);
			int r = makeFilterQuery(&query,filter[0]);
			r += makeFilterQuery(&query,filter[1]);

			if (wcscmp(query.Get(), l_query.Get()))
			{
				bgQuery_Stop();
				l_query.Set(query.Get());

				wchar_t buf[2048] = {0};
				GetDlgItemTextW(m_media_hwnd, IDC_QUICKSEARCH, buf, ARRAYSIZE(buf));
				if (buf[0]) makeQueryStringFromText(&query, buf);

				if (!refresh) SetDlgItemText(m_media_hwnd, IDC_QUICKSEARCH, L"");
				KillTimer(m_media_hwnd, UPDATE_QUERY_TIMER_ID); // keep it from researching :)

				EnterCriticalSection(&g_db_cs);
				if (m_media_scanner) NDE_Scanner_Query(m_media_scanner, query.Get());
				LeaveCriticalSection(&g_db_cs);
				if (r > 0) SendMessage(m_media_hwnd, WM_APP + 3, 0x666, 0); // notify the child to use the first item as a media lookup
				SendMessage(m_media_hwnd, WM_APP + 1, (WPARAM)mysearchCallbackAlbumUpdate, 2);
			}
		}
		if (wParam == 167) // artist sel change
		{
			KillTimer(hwndDlg, 167);

			GayStringW query;
			getParentPlusSearchQuery(&query);

			int r = makeFilterQuery(&query,filter[0]);

			if (wcscmp(query.Get(), l_query.Get()))
			{
				bgQuery_Stop();
				l_query.Set(query.Get());

				if (!refresh) SetDlgItemText(m_media_hwnd, IDC_QUICKSEARCH, L"");
				KillTimer(m_media_hwnd, UPDATE_QUERY_TIMER_ID); // keep it from researching :)

				EnterCriticalSection(&g_db_cs);
				NDE_Scanner_Query(m_media_scanner, query.Get());
				LeaveCriticalSection(&g_db_cs);

				ListView_SetItemCount(m_list2.getwnd(), 0);
				if (r > 0) SendMessage(m_media_hwnd, WM_APP + 3, 0x666, 0); // notify the child to use the first item as a media lookup
				SendMessage(m_media_hwnd, WM_APP + 1, (WPARAM)mysearchCallbackAlbumUpdate, (LPARAM)1); // pass a callback for sort
			}
		}
		if (wParam == 205)
		{
			KillTimer(hwndDlg, 205);
			bgQuery_Stop();
			getParentPlusSearchQuery(&l_query);
			EnterCriticalSection(&g_db_cs);
			NDE_Scanner_Query(m_media_scanner, l_query.Get());
			LeaveCriticalSection(&g_db_cs);

			ignore_selections = 1;
			m_list3.SetSelected(0);
			m_list2.SetSelected(0);
			m_list1.SetSelected(0);
			ListView_SetItemCount(m_list1.getwnd(), 0);
			ListView_SetItemCount(m_list2.getwnd(), 0);
			ListView_SetItemCount(m_list3.getwnd(), 0);
			ignore_selections = 0;
			if (!refresh) SetDlgItemText(m_media_hwnd, IDC_QUICKSEARCH, L"");
			KillTimer(m_media_hwnd, UPDATE_QUERY_TIMER_ID); // keep it from researching :)
			SendMessage(m_media_hwnd, WM_APP + 1, (WPARAM)mysearchCallbackAlbumUpdate, (LPARAM)0);
			refresh -= 1;
		}
		break;
	case WM_MOUSEMOVE:
		if (we_are_drag_and_dropping && GetCapture() == hwndDlg)
		{
			POINT p;
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			ClientToScreen(hwndDlg, &p);
			mlDropItemStruct m;
			ZeroMemory(&m, sizeof(mlDropItemStruct));
			m.type = ML_TYPE_ITEMRECORDLIST;
			m.p = p;
	
			pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
			if (m.result <= 0)
			{
				ZeroMemory(&m, sizeof(mlDropItemStruct));
				m.type = ML_TYPE_QUERYSTRING;
				pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
			}
	
			break;
		}
		break;

	case WM_SETCURSOR:
	case WM_LBUTTONDOWN:
	{
		static INT id[] = { IDC_HDELIM, IDC_DIV1, IDC_DIV2 };
		RECT rw;
		POINT pt;
		INT count;

		count = numFilters; 
		if (count > sizeof(id)/sizeof(INT)) count = sizeof(id)/sizeof(INT);
		
		GetCursorPos(&pt);
		for (INT i = 0; i < count; i++)
		{
			HWND hwndDiv = GetDlgItem(hwndDlg, id[i]);
			if (!hwndDiv) continue;

			GetWindowRect(hwndDiv, &rw);
			if (PtInRect(&rw, pt))
			{
				if (WM_SETCURSOR == uMsg)
				{
					SetCursor(LoadCursor(NULL, (IDC_HDELIM != id[i]) ? IDC_SIZEWE : IDC_SIZENS));
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

	case WM_LBUTTONUP:
		if (we_are_drag_and_dropping && GetCapture() == hwndDlg)
		{
			int whichlist = we_are_drag_and_dropping;
			we_are_drag_and_dropping = 0;
			ReleaseCapture();

			POINT p;
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			ClientToScreen(hwndDlg, &p);
			mlDropItemStruct m = {0};
			m.type = ML_TYPE_ITEMRECORDLISTW;
			m.p = p;
			m.flags = ML_HANDLEDRAG_FLAG_NOCURSOR;

			pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
			itemRecordListW obj = {0, };
			buildRecordListW(&obj, whichlist-1);

			if (m.result > 0)
			{
				/*
				if (whichlist == 1 || m_list2.GetSelectionMark() <= 0)
					buildRecordListW(&obj, 0);
				else if(whichlist == 2 || m_list3.GetSelectionMark() <= 0 || numFilters < 3)
					buildRecordListW(&obj,1);
				else
					buildRecordListW(&obj,2);
				*/
				m.flags = 0;
				m.result = 0;
				m.data = (void*) & obj;
				pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
			}
			else
			{
				m.result = 0;
				m.type = ML_TYPE_ITEMRECORDLIST;
				pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
				if (m.result > 0)
				{
					itemRecordList objA = {0, };
					convertRecordList(&objA, &obj);

					/*
					if (whichlist == 1 || m_list2.GetSelectionMark() <= 0)
						buildRecordList(&obj, 0);
					else if(whichlist == 2 || m_list3.GetSelectionMark() <= 0 || numFilters < 3)
						buildRecordList(&obj, 1);
					else
						buildRecordList(&obj, 2);
					*/
					m.flags = 0;
					m.result = 0;
					m.data = (void*) & objA;
					GayString namebuf;
					int wl = whichlist;
					ViewFilter * filter1 = filter[wl-1];
					{
						int a = filter1->list->GetSelectionMark();
						if (a < 0 || (filter1->HasTopItem() && filter1->list->GetSelected(0)))
						{
							filter1 = filter[0]; wl--;
						}

						if (wl > 0)
						{
							int x;
							int cnt = 0;
							for (x = filter1->HasTopItem()?1:0; x < filter1->Size(); x ++)
							{
								if (filter1->list->GetSelected(x))
								{
									if (cnt) namebuf.Append(", ");
									if (cnt++ > 2)
									{
										namebuf.Append("..."); break;
									}
									namebuf.Append(AutoChar(filter1->GetText(x)));
								}
							}
						}
					}
					if (namebuf.Get() && namebuf.Get()[0]) m.name = namebuf.Get();
					if (m.name)
						m.flags |= ML_HANDLEDRAG_FLAG_NAME;
					pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
					freeRecordList(&objA);
				}

				else
				{
					m.result = 0;
					m.type = ML_TYPE_QUERYSTRING;
					pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
					if (m.result > 0)
					{
						GayStringW query;
						getParentPlusSearchQuery(&query);
						bool allSelected[3] = {false,false,false};
						for (int k=0; k<numFilters; k++)
						{
							W_ListView &w = k==0?m_list1:(k==1?m_list2:m_list3);
							if (filter[k]->HasTopItem()) allSelected[k] = (w.GetSelectionMark() <= 0);
							else allSelected[k] = (w.GetSelectionMark() < 0);
						}

						if (whichlist == 1 || allSelected[1])
						{
							makeFilterQuery(&query,filter[0]);
						}
						else if (whichlist == 2 || allSelected[2] || numFilters < 3)
						{
							makeFilterQuery(&query,filter[0]);
							makeFilterQuery(&query,filter[1]);
						}
						else
						{
							makeFilterQuery(&query,filter[0]);
							makeFilterQuery(&query,filter[1]);
							makeFilterQuery(&query,filter[2]);
						}

						m.data = (void*)query.Get();
						m.result = 0;
						GayString namebuf;
						int wl = whichlist;
						ViewFilter * filter1 = wl==2?filter[1]:filter[0];
						{
							int a = filter1->list->GetSelectionMark();
							if (a < 0 || (filter1->HasTopItem() && filter1->list->GetSelected(0)))
							{
								if (wl==2)
								{
									filter1 = filter[0];
								}
								else m.name = WASABI_API_LNGSTRING(IDS_ALL_ARTISTS);
								wl--;
							}
							if (wl > 0)
							{
								int x;
								int cnt = 0;
								for (x = filter1->HasTopItem()?1:0; x < filter1->Size(); x ++)
								{
									if (filter1->list->GetSelected(x))
									{
										if (cnt) namebuf.Append(", ");
										if (cnt++ > 2)
										{
											namebuf.Append("..."); break;
										}
										namebuf.Append(AutoChar(filter1->GetText(x)));
									}
								}
							}
						}
						if (namebuf.Get() && namebuf.Get()[0])
						{
							m.name = namebuf.Get();
							m.flags |= ML_HANDLEDRAG_FLAG_NAME;
						}
						pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
					}
				}
			}
								freeRecordList(&obj);
		}
		break;
	case WM_CONTEXTMENU:
	{
		HWND hwndFrom = (HWND)wParam;
		int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
		POINT pt = {x,y};
		int idFrom = GetDlgCtrlID(hwndFrom);
		HWND hHeader;
		W_ListView m_list;
		int editFilter=-1;

		if (idFrom == IDC_LIST1)
		{
			m_list = m_list1;
			hHeader = (HWND)SNDMSG(m_list.getwnd(), LVM_GETHEADER, 0, 0L);
			RECT headerRect;
			if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
			{
				if (FALSE != PtInRect(&headerRect, pt))
				{
					editFilter = 0;
				}
			}
		}
		else if (idFrom == IDC_LIST2)
		{
			m_list = m_list2;
			hHeader = (HWND)SNDMSG(m_list.getwnd(), LVM_GETHEADER, 0, 0L);
			RECT headerRect;
			if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
			{
				if (FALSE != PtInRect(&headerRect, pt))
				{
					editFilter = 1;
				}
			}
		}
		else if (idFrom == IDC_LIST3)
		{
			m_list = m_list3;
			hHeader = (HWND)SNDMSG(m_list.getwnd(), LVM_GETHEADER, 0, 0L);
			RECT headerRect;
			if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
			{
				if (FALSE != PtInRect(&headerRect, pt))
				{
					editFilter = 2;
				}
			}
		}
		else
			break;

		if (editFilter != -1)
		{
			HMENU themenu = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);
			HMENU menu = filter[editFilter]->GetMenu(true,editFilter,g_view_metaconf,themenu);
			POINT p;
			GetCursorPos(&p);
			int r = DoTrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, p.x, p.y, hwndDlg, NULL);
			DestroyMenu(themenu);
			filter[editFilter]->ProcessMenuResult(r,true,editFilter,g_view_metaconf,hwndDlg);
			MSG msg;
			while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
			break;
		}

		int pane=0;
		if (numFilters > 1)
		{
			if (idFrom == IDC_LIST2 && m_list2.GetSelectionMark() >= 0) pane=1;
		}
		if (numFilters > 2)
		{
			if (idFrom == IDC_LIST3 && m_list3.GetSelectionMark() >= 0) pane=2;
		}

		if (x == -1 || y == -1) // x and y are -1 if the user invoked a shift-f10 popup menu
		{
			RECT itemRect = {0};
			int selected = m_list.GetNextSelected();
			if (selected != -1) // if something is selected we'll drop the menu from there
			{
				m_list.GetItemRect(selected, &itemRect);
				ClientToScreen(m_list.getwnd(), (POINT *)&itemRect);
			}
			else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
			{
				GetWindowRect(m_list.getwnd(), &itemRect);

				HWND hHeader = (HWND)SNDMSG(m_list.getwnd(), LVM_GETHEADER, 0, 0L);
				RECT headerRect;
				if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
				{
					itemRect.top += (headerRect.bottom - headerRect.top);
				}
			}
			x = itemRect.left;
			y = itemRect.top;
		}

		RECT headerRect;
		if (0 == (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) || FALSE == GetWindowRect(hHeader, &headerRect))
		{
			SetRectEmpty(&headerRect);
		}

		if (FALSE != PtInRect(&headerRect, pt))
		{
			break;
		}

		HMENU menu = GetSubMenu(g_context_menus, 1);
		sendto_hmenu = GetSubMenu(menu, 2);

		s.mode = 0;
		s.hwnd = 0;
		s.build_hMenu = 0;

		IPC_LIBRARY_SENDTOMENU = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
		if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)0, IPC_LIBRARY_SENDTOMENU) == (LRESULT)-1)
		{
			s.mode = 1;
			s.hwnd = hwndDlg;
			s.data_type = ML_TYPE_ITEMRECORDLIST;
			s.ctx[1] = 1;
			s.build_hMenu = sendto_hmenu;
		}

		wchar_t str[128] = {0};
		StringCchPrintfW(str, 128, (!play_enq_rnd_alt?WASABI_API_LNGSTRINGW(IDS_PLAY_RANDOM_ITEM):L"%s"),
						 filter[pane]->GetNameSingularAlt(play_enq_rnd_alt?1:0));
		FixAmps(str, 128);
		MENUITEMINFOW mii =
		{
			sizeof(MENUITEMINFOW),
			MIIM_TYPE | MIIM_ID,
			MFT_STRING,
			MFS_ENABLED,
			ID_AUDIOWND_PLAYRANDOMITEM,
			NULL,
			NULL,
			NULL,
			0,
			str,
			0,
		};
		SetMenuItemInfoW(menu, ID_AUDIOWND_PLAYRANDOMITEM, false, &mii);

		StringCchPrintfW(str, 128, (!play_enq_rnd_alt?WASABI_API_LNGSTRINGW(IDS_ENQUEUE_RANDOM_ITEM):L"%s"),
						 filter[pane]->GetNameSingularAlt(play_enq_rnd_alt?2:0));
		FixAmps(str, 128);
		mii.wID = ID_AUDIOWND_ENQUEUERANDOMITEM;
		SetMenuItemInfoW(menu, ID_AUDIOWND_ENQUEUERANDOMITEM, false, &mii);

		filter[pane]->DialogProc(hwndDlg,WM_USER+600,(WPARAM)menu,0x7000);
		UpdateMenuItems(NULL, menu, IDR_VIEW_ACCELERATORS);
		int r = DoTrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, x, y, hwndDlg, NULL);

		filter[pane]->DialogProc(hwndDlg,WM_USER+601,(WPARAM)menu,0x7000);

		if(r >= 0x7000 && r <= 0x8000) {
			GayStringW query;
			getParentPlusSearchQuery(&query);
			for (int i=0; i<=pane; i++)
				makeFilterQuery(&query,filter[i]);
			filter[pane]->DialogProc(hwndDlg,WM_USER+602,(WPARAM)query.Get(),r - 0x7000);
		}

		switch (r)
		{
			case ID_AUDIOWND_PLAYSELECTION:
				playList(PLAY,pane);
				break;
			case ID_AUDIOWND_ENQUEUESELECTION:
				playList(ENQUEUE,pane);
				break;
			case ID_AUDIOWND_PLAYRANDOMITEM:
			case ID_AUDIOWND_ENQUEUERANDOMITEM:
				playRandomList(r == ID_AUDIOWND_PLAYRANDOMITEM ? PLAY : ENQUEUE, pane);
				break;
			case ID_RATE_1:
			case ID_RATE_2:
			case ID_RATE_3:
			case ID_RATE_4:
			case ID_RATE_5:
			case ID_RATE_0:
			{
				int rate = r - ID_RATE_1 + 1;
				if (r == ID_RATE_0) rate = 0;
				if(rateList(rate))
				{
					wchar_t buf[10] = {0};
					wsprintfW(buf,L"%d",rate);
					int s = -1;
					while((s = filter[pane]->list->GetNextSelected(s)) != -1)
						filter[pane]->MetaUpdate(s, DB_FIELDNAME_rating,buf);
				}
				PostMessage(hwndDlg, WM_APP + 4, (WPARAM)rate, (LPARAM)0);
			}
			break;
			default:
				if (s.mode == 2)
				{
					s.menu_id = r;
					if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU) == (LRESULT)-1)
					{
						// build my data.
						s.mode = 3;
						s.data_type = ML_TYPE_ITEMRECORDLISTW;
						itemRecordListW myObj = {0, };
						buildRecordListW(&myObj, pane);

						s.data = (void*) & myObj;
						LRESULT result = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU);
						
						if (result != 1)
						{
							s.mode = 3;
							s.data_type = ML_TYPE_ITEMRECORDLIST;
							itemRecordList objA = {0, };
							convertRecordList(&objA, &myObj);

							s.data = (void*) & objA;
							SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU);
							freeRecordList(&objA);
						}
						freeRecordList(&myObj);
					}
				}
				break;
		}
		if (s.mode)
		{
			s.mode = 4;
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU); // cleanup
		}
		sendto_hmenu = 0;
		EatKeyboard();
		return 0;
	}
	case WM_NOTIFY:
	{
		LPNMHDR l = (LPNMHDR)lParam;

		if (l->code == LVN_ODFINDITEMW)
		{
			ViewFilter * f;
			if (l->idFrom == IDC_LIST2) f = filter[1];
			else if (l->idFrom == IDC_LIST3) f = filter[2];
			else f = filter[0];

			NMLVFINDITEMW *t = (NMLVFINDITEMW *)lParam;
			int i = t->iStart;
			if (i >= f->Size()) i = 0;

			int cnt = f->Size() - i;
			if (t->lvfi.flags & LVFI_WRAP) cnt += i;

			while (cnt-- > 0)
			{
				const wchar_t *name = f->GetText(i);
				SKIP_THE_AND_WHITESPACEW(name)

				if (t->lvfi.flags & (4 | LVFI_PARTIAL))
				{
					if (!StrCmpNIW(name, t->lvfi.psz, wcslen(t->lvfi.psz)))
					{
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, i);
						return 1;
					}
				}
				else if (t->lvfi.flags & LVFI_STRING)
				{
					if (!lstrcmpiW(name, t->lvfi.psz))
					{
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, i);
						return 1;
					}
				}
				else
				{
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
					return 1;
				}
				if (++i == f->Size()) i = 0;
			}
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
			return 1;
		}
		else if (l->code == LVN_GETDISPINFOW)
		{
			NMLVDISPINFOW *lpdi = (NMLVDISPINFOW*) lParam;
			int item = lpdi->item.iItem;
			if (item < 0) return 0;

			if (lpdi->item.mask & LVIF_TEXT)
			{
				if (l->idFrom == IDC_LIST3 && numFilters == 3)
					lpdi->item.pszText = (wchar_t*)filter[2]->CopyText2(item,lpdi->item.iSubItem,lpdi->item.pszText,lpdi->item.cchTextMax);

				if (l->idFrom == IDC_LIST2)
					lpdi->item.pszText = (wchar_t*)filter[1]->CopyText2(item,lpdi->item.iSubItem,lpdi->item.pszText,lpdi->item.cchTextMax);

				else if (l->idFrom == IDC_LIST1)
					lpdi->item.pszText = (wchar_t*)filter[0]->CopyText2(item, lpdi->item.iSubItem, lpdi->item.pszText, lpdi->item.cchTextMax);
			}
			return 0;
		}
		else if (l->code == LVN_COLUMNCLICK)
		{
			NMLISTVIEW *p = (NMLISTVIEW*)lParam;
			int which = l->idFrom == IDC_LIST3 ? 2 : (l->idFrom == IDC_LIST2 ? 1 : 0);
			wchar_t buf[32] = {0}, buf2[32] = {0};

			StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_sort_by_%d",filter[which]->GetConfigId(), which);
			int l_sc = g_view_metaconf->ReadInt(buf, 0);

			StringCchPrintfW(buf2, ARRAYSIZE(buf2), L"%hs_sort_dir_%d",filter[which]->GetConfigId(), which);
			int l_sd = g_view_metaconf->ReadInt(buf2, 0);
			if (p->iSubItem == l_sc) l_sd = !l_sd;
			else l_sc = p->iSubItem;

			g_view_metaconf->WriteInt(buf, l_sc);
			g_view_metaconf->WriteInt(buf2, l_sd);

			int s;
			s = filter[which]->Size();
			int dlgitem = (which==0)?IDC_LIST1:((which==1)?IDC_LIST2:IDC_LIST3);
			SendMessage((HWND)SendMessage(GetDlgItem(hwndDlg,dlgitem), LVM_GETHEADER, 0, 0L),WM_ML_IPC,MAKEWPARAM(l_sc,!l_sd/* : !l_sd/*!l_sd : l_sd*/),ML_IPC_SKINNEDHEADER_DISPLAYSORT);
			filter[which]->SortResults(g_view_metaconf, which);
			ListView_SetItemCount(l->hwndFrom, s);
			InvalidateRect(l->hwndFrom, NULL, TRUE);
		}
		else if (l->idFrom == IDC_LIST1 || l->idFrom == IDC_LIST2 || l->idFrom == IDC_LIST3)
		{
			int pane = 0;
			if (l->idFrom == IDC_LIST2) pane = 1;
			if (l->idFrom == IDC_LIST3) pane = 2;

			if (l->code == NM_DBLCLK)
			{
				// allow doubleclick on all, yes
				int c = filter[pane]->list->GetCount(), x=0;
				for (x = 0; x < c; x ++) if (filter[pane]->list->GetSelected(x)) break;

				if (!x && pane>0 && filter[pane]->HasTopItem())
					playList((!!g_config->ReadInt(L"enqueuedef", PLAY)) ^(!!(GetAsyncKeyState(VK_SHIFT)&0x8000)),pane-1);
				else if (x < c)
					playList((!!g_config->ReadInt(L"enqueuedef", PLAY)) ^(!!(GetAsyncKeyState(VK_SHIFT)&0x8000)),pane);
			}
			else if (l->code == LVN_ITEMCHANGED)
			{
				LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
				if (ignore_selections || !((lv->uNewState ^ lv->uOldState) & LVIS_SELECTED)) return 0;
				int t;
				if (pane==0) t=167;
				else if (pane==1) t=166;
				else t=165;

				KillTimer(hwndDlg, t);
				SetTimer(hwndDlg, t, 100, NULL);
			}
			else if (l->code == LVN_BEGINDRAG)
			{
				if (pane==0) we_are_drag_and_dropping = 1;
				else if (pane==1) we_are_drag_and_dropping = 2;
				else we_are_drag_and_dropping = 3;
				SetCapture(hwndDlg);
			}
		}
		if (l->code == NM_RETURN)
		{
			extern void playFiles(int enqueue, int all);
			playFiles(((!!g_config->ReadInt(L"enqueuedef", 0)) ^(!!(GetAsyncKeyState(VK_SHIFT)&0x8000))), 1);
		}
	}
	break;
	case WM_USER+710:
		if(rateList(wParam))
		{
			ViewFilter *f = (ViewFilter *)lParam;

			// first check the validity of lParam
			bool found=false;
			for(int i=0; i<numFilters; i++)
				if(filter[i] == f) found=true;
			if(!found) break;

			wchar_t buf[10] = {0};
			wsprintfW(buf,L"%d",wParam);
			int s = -1;
			while((s = f->list->GetNextSelected(s)) != -1)
				f->MetaUpdate(s, DB_FIELDNAME_rating,buf);
		}
		break;
	case WM_DROPFILES:
	{
		extern void Window_OnDropFiles(HWND hwndDlg, HDROP hdrop);
		Window_OnDropFiles(hwndDlg, (HDROP)wParam);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CLEAR:
			SetDlgItemText(hwndDlg, IDC_QUICKSEARCH, L"");
			break;
		case IDC_QUICKSEARCH:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				KillTimer(hwndDlg, 205);
				SetTimer(hwndDlg, 205, g_querydelay, NULL);
			}
			break;
		case IDC_BUTTON_ARTMODE:
		{
			int changed=0;
			int f[3]={0,0,0};
			for (int i=0; i<numFilters; i++)
			{
				f[i] = GetFilter(m_query_mode,i);
				if (f[i] == 6)
				{
					f[i] = 11; changed=true;
				}
				else if (f[i] == 11)
				{
					f[i] = 6; changed=true;
				}
			}

			if (changed)
			{
				if (f[1] == 6) f[1]=0;
				if (numFilters == 2) f[2]=0;
				int m = f[0] | (f[1] << 8) | (f[2] << 16);

				int par = mediaLibrary.GetSelectedTreeItem();
				QueryList::iterator iter;
				iter = m_query_list.find(par);
				if (iter != m_query_list.end())
					iter->second->mode = m;
				saveQueryTree();
				PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
			}
		}
		break;
		case IDC_BUTTON_VIEWMODE:
		{
			int presets[] =
			{
				MAKEVIEW_2FILTER(ARTIST,ALBUM),
				MAKEVIEW_2FILTER(ARTIST,ALBUMART),
				MAKEVIEW_2FILTER(ALBUMARTIST,ALBUM),
				MAKEVIEW_2FILTER(ALBUMARTIST,ALBUMART),
				MAKEVIEW_3FILTER(GENRE,ARTIST,ALBUM),
				MAKEVIEW_2FILTER(GENRE,ALBUMART),
				MAKEVIEW_3FILTER(YEAR,ARTIST,ALBUM),
				MAKEVIEW_2FILTER(COMPOSER,ALBUM),
				MAKEVIEW_3FILTER(PUBLISHER,ARTIST,ALBUM),
			};
			HMENU menu = CreatePopupMenu();
			bool hasCheck=false;
			for (int i=0; i < sizeof(presets)/sizeof(presets[0]); i++)
			{
				wchar_t buf[350] = {0}, filterName[128] = {0};
				buf[0] = L'\0';
				int l = GetNumFilters(presets[i]);
				bool check = (l == GetNumFilters(m_query_mode));
				for (int j=0; j<l; j++)
				{
					int f = GetFilter(presets[i],j)-1;
					wcscat(buf,getFilterName(f, filterName, ARRAYSIZE(filterName)));
					if (GetFilter(presets[i],j+1)) wcscat(buf,L"\\");
					if (GetFilter(presets[i],j) != GetFilter(m_query_mode,j)) check=false; 
				}
				AppendMenuW(menu,MF_STRING,i+1,buf);
				if(check)
				{
					CheckMenuItem(menu,i+1,MF_CHECKED);
					hasCheck=true;
				}
			}
			AppendMenuW(menu,MF_STRING,0x4000,WASABI_API_LNGSTRINGW(IDS_OTHER2));
			if(!hasCheck)
				CheckMenuItem(menu,0x4000,MF_CHECKED);
			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON_VIEWMODE),&rc);
			int r = DoTrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY,
								 rc.left, rc.bottom, hwndDlg, NULL);
			DestroyMenu(menu);
			if (r==0) break;
			else if (r == 0x4000)
				queryEditItem(mediaLibrary.GetSelectedTreeItem());
			else
			{
				int par = mediaLibrary.GetSelectedTreeItem();
				QueryList::iterator iter;
				iter = m_query_list.find(par);
				if (iter != m_query_list.end())
					iter->second->mode = presets[r-1];;
				saveQueryTree();
				PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
			}
		}
		break;
		case IDC_BUTTON_COLUMNS:
		{
			HMENU themenu[3] = {0};
			wchar_t *name[3] = {0}, filterName[128] = {0};
			HMENU menu = CreatePopupMenu();

			MENUITEMINFOW m={sizeof(m),MIIM_TYPE | MIIM_ID | MIIM_SUBMENU,MFT_STRING,0};
			for (int i=0; i<numFilters; i++)
			{
				m.wID = i;
				m.dwTypeData = name[i] = _wcsdup(getFilterName(GetFilter(m_query_mode,i)-1, filterName, ARRAYSIZE(filterName)));
				themenu[i] = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);
				m.hSubMenu = filter[i]->GetMenu(true,i,g_view_metaconf,themenu[i]);
				InsertMenuItemW(menu,i,FALSE,&m);
			}

			m.wID = 3;
			m.dwTypeData = WASABI_API_LNGSTRINGW(IDS_TRACKS_MENU);
			m.hSubMenu = GetSubMenu(themenu[0], 4);
			InsertMenuItemW(menu,3,FALSE,&m);

			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON_COLUMNS),&rc);
			int r = DoTrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, rc.left,rc.bottom, hwndDlg, NULL);

			DestroyMenu(menu);

			if (r == ID_HEADERWND_CUSTOMIZECOLUMNS)
			{
				void customizeColumnsDialog(HWND hwndParent);
				customizeColumnsDialog(hwndDlg);
			}

			for (int i=0; i<numFilters; i++)
			{
				filter[i]->ProcessMenuResult(r,true,i,g_view_metaconf,hwndDlg);
				DestroyMenu(themenu[i]);
				free(name[i]);
			}
		}
		break;
		}
		break;
	case WM_WINDOWPOSCHANGED:
		if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) ||
		    (SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
		{
			LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags));
		}
		return TRUE;
	case WM_APP + 1: //sent by parent for resizing window
		bgQuery_Stop();
		getParentPlusSearchQuery(&l_query);

		EnterCriticalSection(&g_db_cs);
		NDE_Scanner_Query(m_media_scanner, l_query.Get());
		LeaveCriticalSection(&g_db_cs);

		ignore_selections = 1;
		for (int i=0; i<numFilters; i++)
		{
			filter[i]->list->SetSelected(0);
			ListView_SetItemCount(filter[i]->list->getwnd(), 0);
		}
		ignore_selections = 0;
		if (!refresh) SetDlgItemText(m_media_hwnd, IDC_QUICKSEARCH, L"");
		KillTimer(m_media_hwnd, UPDATE_QUERY_TIMER_ID); // keep it from researching :)
		SendMessage(m_media_hwnd, WM_APP + 1, (WPARAM)mysearchCallbackAlbumUpdate, (LPARAM)0);
		refresh -= 1;
		break;
	case WM_QUERYFILEINFO:	if (m_media_hwnd) PostMessageW(m_media_hwnd, uMsg, wParam, lParam);  break;
	case WM_SHOWFILEINFO:	
		SendMessageW(GetParent(hwndDlg), uMsg, wParam, lParam); break;
	case WM_USER + 66:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, SendMessage(GetParent(hwndDlg), uMsg, wParam, lParam));
		return TRUE;
	case WM_APP + 2: //sent by media child to get current query
		if (lParam) *((const wchar_t **)(lParam)) = l_query.Get();
		break;
	case WM_APP + 3: // sent by bg thread to update our shit
		if (wParam == 0x69)
		{
			if (lParam == 0 && numFilters >= 1)
			{
				if (UpdateFilterPane(0,"artist_sel_utf8"))
				{
					PostMessage(hwndDlg,WM_TIMER,167,0);
					break;
				}
			}

			if (lParam <= 1 && numFilters >= 2)
			{
				if (UpdateFilterPane(1,"album_sel_utf8"))
				{
					PostMessage(hwndDlg,WM_TIMER,166,0);
					break;
				}
			}

			if (numFilters >= 3)
			{
				if (UpdateFilterPane(2,"list3_sel_utf8"))
				{
					PostMessage(hwndDlg,WM_TIMER,165,0);
				}
			}
		}
		break;
	// used for working around some quirks with the ratings so it'll update on-the-fly(ish)
	case WM_APP + 4:
	{
		if(!lParam)
		{
			wchar_t buf[64] = {0};
			StringCchPrintfW(buf,64,L"%d",wParam);
			for(int i=0; i<numFilters; i++)
			{
				int s = -1;
				while((s = filter[i]->list->GetNextSelected(s)) != -1)
				{
					filter[i]->MetaUpdate(s, DB_FIELDNAME_rating, buf);
					SendMessage(filter[i]->list->getwnd(),LVM_REDRAWITEMS,s,s);
				}
			}
		}
		else
		{
			PostMessage(m_media_hwnd, WM_APP + 1, (WPARAM)mysearchCallbackAlbumUpdate, (LPARAM)1);
		}
	}
	break;
	case WM_APP + 5:
		// TODO
		break;
	case WM_APP + 6:	// handles the ml_cloud 'first pull' announces so we
	{					// can then show the cloud column & update the cache
		SendMessage(m_media_hwnd, WM_APP + 6, wParam, lParam);
		break;
	}
	case WM_DESTROY:
		FlushCache();

		if (numFilters >= 2) g_view_metaconf->WriteInt(L"adiv1pos", div_filterpos);
		if (numFilters >= 3) g_view_metaconf->WriteInt(L"adiv3pos", div_filter2pos);
		
		g_view_metaconf->WriteInt(L"adiv2pos", div_mediapos);
		// save a list of selected artists and albums
		for (int i=0; i<numFilters; i++)
		{
			GayStringW gs;
			if (!(filter[i]->list->GetSelected(0) && filter[i]->HasTopItem()))
			{
				int c = filter[i]->list->GetCount(), delim = 0;
				for (int x = filter[i]->HasTopItem()?1:0; x < c; x ++)
				{
					if (filter[i]->list->GetSelected(x))
					{
						if (delim) gs.Append(LDELIMSTR);
						const wchar_t *p = filter[i]->GetText(x);
						while (p && *p)
						{
							if (*p == *LDELIMSTR) gs.Append(LDELIMSTR LDELIMSTR);
							else
							{
								wchar_t buf[2] = {*p, 0};
								gs.Append(buf);
							}
							p++;
						}
						delim = 1;
					}
				}
			}
			char *confname = "artist_sel_utf8";
			if (i==1) confname = "album_sel_utf8";
			if (i==2) confname = "list3_sel_utf8";
			g_view_metaconf->WriteString(confname, AutoChar(gs.Get(), CP_UTF8));
		}
		{
			wchar_t buf[2048] = {0};
			GetDlgItemTextW(hwndDlg, IDC_QUICKSEARCH, buf, ARRAYSIZE(buf));
			g_view_metaconf->WriteString("lastquery_a_utf8", AutoChar(buf, CP_UTF8));
		}

		bgQuery_Stop();

		for (int i=0; i<numFilters; i++)
		{
			filter[i]->SaveColumnWidths();
			filter[i]->Empty();
			filter[i]->ClearColumns();
			delete filter[i];
		}
		numFilters=0;
		{
			SkinBitmap *s = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_ARTMODE),GWLP_USERDATA);
			if (s)
			{
				void *bits = s->getBits();
				if (bits) free(bits); delete s;
				SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_ARTMODE),GWLP_USERDATA, 0);
			}
			s = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_VIEWMODE),GWLP_USERDATA);
			if (s)
			{
				void *bits = s->getBits();
				if (bits) free(bits); delete s;
				SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_VIEWMODE),GWLP_USERDATA, 0);
			}
			s = (SkinBitmap*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_COLUMNS),GWLP_USERDATA);
			if (s)
			{
				void *bits = s->getBits();
				if (bits) free(bits); delete s;
				SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BUTTON_COLUMNS),GWLP_USERDATA, 0);
			}
		}
		break;
	case WM_PAINT:
	{
		int tab[] =
		{
			IDC_HDELIM | DCW_DIVIDER, // 0
			IDC_QUICKSEARCH | DCW_SUNKENBORDER, // 1
			IDC_DIV1 | DCW_DIVIDER, // 2
			IDC_LIST1 | DCW_SUNKENBORDER, //3
			IDC_LIST2 | DCW_SUNKENBORDER, //4
			IDC_LIST3 | DCW_SUNKENBORDER, //5
			IDC_DIV2 | DCW_DIVIDER, //6
		};
		int size = 5;
		if (numFilters == 3) size = 7;

		if (m_nodrawtopborders) size = 2;
		else
		{
			if (numFilters == 3)
			{
				if (adiv1_nodraw == 2) tab[4]=0;
				if (adiv1_nodraw == 1) tab[3]=0;
				if (adiv2_nodraw == 2) tab[5]=0;
				if (adiv2_nodraw == 1) tab[4]=0;
			}
			else
			{
				if (adiv1_nodraw == 2) size = 4;
				if (adiv1_nodraw == 1)
				{
					size = 4; tab[3] = tab[4];
				}
			}
		}
		dialogSkinner.Draw(hwndDlg, tab, size);
	}
	break;

	case WM_ERASEBKGND:
		return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT

	case WM_USER + 0x201:
		offsetX = (short)LOWORD(wParam);
		offsetY = (short)HIWORD(wParam);
		g_rgnUpdate = (HRGN)lParam;
		return TRUE;

	case WM_ML_CHILDIPC:
		switch (lParam)
		{
			case ML_CHILDIPC_GO_TO_SEARCHBAR:
				SendDlgItemMessage(hwndDlg, IDC_QUICKSEARCH, EM_SETSEL, 0, -1);
				SetFocus(GetDlgItem(hwndDlg, IDC_QUICKSEARCH));
				break;
			case ML_CHILDIPC_REFRESH_SEARCH:
				refresh = (1 + numFilters);
				PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_QUICKSEARCH, EN_CHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_QUICKSEARCH));
				break;
		}
		break;

	}
	return FALSE;
}