#include "main.h"
#include "ml_local.h"
#include "resource.h"
#include "..\..\General\gen_ml/gaystring.h"
#include "..\..\General\gen_ml/config.h"
#include "..\..\General\gen_ml/ml_ipc.h"
#include "../nde/nde.h"
#include "editquery.h"
#include "../nu/ComboBox.h"
#include "../replicant/nu/AutoWide.h"
#include "../replicant/nu/AutoChar.h"
#include "../replicant/nu/ns_wc.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"

const wchar_t *getFilterName(unsigned int filterId, wchar_t *buffer, size_t bufferSize);

static int m_edit_item;

void queryStrEscape(const wchar_t *p, GayStringW &str) 
{
	if (!p || !*p) return;
	size_t l = wcslen(p);
	wchar_t *escaped = (wchar_t *)calloc((l*3+1), sizeof(wchar_t));
	wchar_t *d = escaped;
	while (p && *p) {
		if (*p == L'%') { *d++ = L'%'; *d++ = L'%'; }
		else if (*p == L'\"') { *d++ = L'%'; *d++ = L'2'; *d++ = L'2'; }
		else if (*p == L'\'') { *d++ = L'%'; *d++ = L'2'; *d++ = L'7'; }
		else if (*p == L'[') { *d++ = L'%'; *d++ = L'5'; *d++ = L'B'; }
		else if (*p == L']') { *d++ = L'%'; *d++ = L'5'; *d++ = L'D'; }
		else if (*p == L'(') { *d++ = L'%'; *d++ = L'2'; *d++ = L'8'; }
		else if (*p == L')') { *d++ = L'%'; *d++ = L'2'; *d++ = L'9'; }
		else if (*p == L'#') { *d++ = L'%'; *d++ = L'2'; *d++ = L'3'; }
		else *d++ = *p;
		p++;
	}
	*d = 0;
	str.Set(escaped);
	free(escaped);
}

static INT_PTR CALLBACK scrollChildHostProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK childAdvanced(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

int m_item_mode=0,main_oninitdialog;
int m_item_image = MLTREEIMAGE_DEFAULT;
wchar_t m_item_name[256]=L"";
wchar_t m_item_query[1024]=L"";

typedef struct {
	int title;
	wchar_t *query;
	char sort_by;
	char sort_dir;
	char *columns; //xff terminated list :). NULL means default columns
	int imageIndex;
	int mode;
} smartViewPreset;

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

#define MAKEVIEW_3FILTER(a, b, c) (a | (b << 8) | (c << 16))
#define MAKEVIEW_2FILTER(a, b) (a | (b << 8))

static smartViewPreset presets[] = {
	{IDS_AUDIO, L"type = 0", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, 1},
	{IDS_VIDEO, L"type = 1", 10, 0, "\x7\1\5\x1E\6\3\x20\x8\x9\xA\xff", TREE_IMAGE_LOCAL_VIDEO,MAKEVIEW_2FILTER(GENRE,YEAR)},
	{IDS_MOST_PLAYED, L"playcount > 0", 9, 0, "\x9\0\1\2\3\xA\xff", TREE_IMAGE_LOCAL_MOSTPLAYED,0},
	{IDS_RECENTLY_ADDED, L"dateadded > [3 days ago]", 33, 0, "\x21\0\1\2\3\xff", TREE_IMAGE_LOCAL_RECENTLYADDED,0},
	{IDS_RECENTLY_MODIFIED, L"lastupd > [3 days ago]", 11, 0, "\xB\0\1\2\3\xff", TREE_IMAGE_LOCAL_RECENTLYMODIFIED,0},
	{IDS_RECENTLY_PLAYED, L"lastplay > [2 weeks ago]", 10, 0, "\xA\x9\0\1\2\3\xff", TREE_IMAGE_LOCAL_RECENTLYPLAYED,0},
	{IDS_NEVER_PLAYED, L"playcount = 0 | playcount isempty", 0, 0, "\0\1\2\3\xff", TREE_IMAGE_LOCAL_NEVERPLAYED,0},
	{IDS_TOP_RATED, L"rating >= 3", 8, 0, "\x8\x9\0\1\2\3\xff", TREE_IMAGE_LOCAL_TOPRATED,0},
	{IDS_PODCASTS,L"ispodcast = 1", 0, 0, 0, TREE_IMAGE_LOCAL_PODCASTS, MAKEVIEW_3FILTER(GENRE,PODCASTCHANNEL,YEAR)},
	{IDS_AUDIO_BY_GENRE, L"type = 0", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, MAKEVIEW_3FILTER(GENRE,ARTIST,ALBUM)},
	{IDS_AUDIO_BY_INDEX, L"type = 0", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, MAKEVIEW_3FILTER(ARTISTINDEX,ARTIST,ALBUM)},
	{IDS_SIMPLE_ALBUM, L"type = 0", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, 0x0100000B},
	{IDS_60s_MUSIC, L"type = 0 and year >= 1960 and year < 1970", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, 1},
	{IDS_70s_MUSIC, L"type = 0 and year >= 1970 and year < 1980", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, 1},
	{IDS_80s_MUSIC, L"type = 0 and year >= 1980 and year < 1990", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, 1},
	{IDS_90s_MUSIC, L"type = 0 and year >= 1990 and year < 2000", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, 1},
	{IDS_00s_MUSIC, L"type = 0 and year >= 2000 and year < 2010", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, 1},
	{IDS_ROCK_MUSIC, L"type = 0 and genre has \"Rock\"", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, 1},
	{IDS_CLASSICAL_MUSIC, L"type = 0 and genre has \"Classical\"", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, MAKEVIEW_2FILTER(COMPOSER,ALBUM)},
	{IDS_RECORD_LABELS, L"type = 0", 0, 0, NULL, TREE_IMAGE_LOCAL_AUDIO, MAKEVIEW_3FILTER(PUBLISHER,ALBUMARTIST,ALBUM)},
	{IDS_CUSTOM, L"", 0, 0, NULL, MLTREEIMAGE_DEFAULT, 0},
};

#define NUM_PRESETS (sizeof(presets) / sizeof(smartViewPreset))

void showdlgelements(HWND hwndDlg, int nCmdShow) {
	const int dlgelems[] = {
		IDC_STATIC_FILTER,
		IDC_RADIO_SIMPLE,
		IDC_IMAGE_SIMPLE,
		IDC_STATIC_SIMPLE,
		IDC_RADIO_SIMPLEALBUM,
		IDC_IMAGE_SIMPLEALBUM,
		IDC_STATIC_SIMPLEALBUM,
		IDC_RADIO_TWOFILTERS,
		IDC_IMAGE_TWOFILTERS,
		IDC_STATIC_TWOFILTERS,
		IDC_RADIO_THREEFILTERS,
		IDC_IMAGE_THREEFILTERS,
		IDC_STATIC_THREEFILTERS,
		IDC_COMBO_FILTER1,
		IDC_COMBO_FILTER2,
		IDC_COMBO_FILTER3,
		IDC_STATIC_FILTER2,
		IDC_STATIC_FILTER3,
	};
	for(int i=0; i < (sizeof(dlgelems)/sizeof(int)); i++)
		ShowWindow(GetDlgItem(hwndDlg,dlgelems[i]),nCmdShow);
}

#define SPL_RemoveAll(hwnd) SendMessage(hwnd,WM_USER+41,0,0)
#define SPL_GetQueryString(hwnd) SendMessage(hwnd,WM_USER+40, 0 ,0)
#define SPL_AddFilter(hwnd, filter) (HWND)scrollChildHostProc(hwnd,WM_USER+32, (WPARAM)filter, (LPARAM)1)
#define SPL_AddBlankFilter(hwnd) (HWND)scrollChildHostProc(hwnd,WM_USER+32, 0, (LPARAM)2)
#define SPL_RemoveFilter(hwnd, filterhwnd) SendMessage(hwnd,WM_USER+34,(WPARAM)filterhwnd,(LPARAM)-1)
#define SPL_UpdateScroll(hwnd) SendMessage(hwnd,WM_USER+33,0,0)

static int m_simple_dirty;

void SetStaticItemImage(HWND hwndDlg, UINT ctrl_id, UINT ctrl_img)
{
	SendDlgItemMessage(hwndDlg,ctrl_id,STM_SETIMAGE,IMAGE_BITMAP,
					   (LPARAM)LoadImage(WASABI_API_ORIG_HINST,MAKEINTRESOURCE(ctrl_img),IMAGE_BITMAP,0,0,LR_SHARED));
}

int ResizeComboBoxDropDownW(HWND hwndDlg, UINT id, const wchar_t *str, int width){
	SIZE size = {0};
	HWND control = GetDlgItem(hwndDlg, id);
	HDC hdc = GetDC(control);
	// get and select parent dialog's font so that it'll calculate things correctly
	HFONT font = (HFONT)SendMessage(hwndDlg,WM_GETFONT,0,0), oldfont = (HFONT)SelectObject(hdc,font);
	GetTextExtentPoint32W(hdc, str, wcslen(str)+1, &size);

	int ret = width;
	if(size.cx > width)
	{
		SendDlgItemMessage(hwndDlg, id, CB_SETDROPPEDWIDTH, size.cx, 0);
		ret = size.cx;
	}

	SelectObject(hdc, oldfont);
	ReleaseDC(control, hdc);
	return ret;
}

struct FiltersContext
{
	HWND last1, last2;
	int mode;
	HWND m_scrollchild;
	int *error;
};

static bool EnumFilters(Scanner *scanner, nde_filter_t filter, void *context_in)
{
	FiltersContext *context = (FiltersContext *)context_in;

	HWND x = SPL_AddFilter(context->m_scrollchild,filter);
	if(x) { // we have a filter which talks about a field
		context->last2=context->last1;
		context->last1=x;
		context->mode++;
		if(context->mode > 2) 
			context->error[0]=1; // not in our strict form
	} else { // we have an AND, OR or a NOT
		int f=NDE_Filter_GetOp(filter);
		if(f == FILTER_OR) CheckDlgButton(context->last2,IDC_OR,TRUE);
		else if(f == FILTER_AND) CheckDlgButton(context->last2,IDC_AND,TRUE);
		else context->error[0]=1; // we can't do FILTER_NOT
		context->mode--;
		if(context->mode != 1) context->error[0]=1; // not in our strict form
	}
	return !context->error[0];
}

static INT_PTR CALLBACK addQueryFrameDialogProc2(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	static HWND m_scrollchild;
	static int m_editting=0;
	static C_Config *conf=0;
	static wchar_t * m_item_meta;
	static int mode;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				const int filters[] = {IDC_COMBO_FILTER1, IDC_COMBO_FILTER2,IDC_COMBO_FILTER3};
				HWND controlWindow;
				int iItem;
				wchar_t buffer[512] = {0};

				mode=0;
				m_item_meta=0;
				conf=0;
				m_simple_dirty=0;

				openDb();
				
				CheckDlgButton(hwndDlg,IDC_RADIO_SIMPLE,TRUE);
				SetDlgItemTextW(hwndDlg,IDC_STATIC_INFO, WASABI_API_LNGSTRINGW(IDS_WILLS_UBER_STRING));
				EnableWindow(GetDlgItem(hwndDlg,IDC_HIDE_EXTINFO), g_config->ReadInt(L"useminiinfo2", 0));
				// set up presets combo box
				{
					controlWindow = GetDlgItem(hwndDlg, IDC_COMBO_PRESETS);
					if (NULL != controlWindow)
					{
						int width = 0;
						for(size_t i=0; i < NUM_PRESETS; i++) 
						{
							if (NULL != WASABI_API_LNGSTRINGW_BUF(presets[i].title, buffer, ARRAYSIZE(buffer)))
							{
								iItem = SendMessageW(controlWindow, CB_ADDSTRING, 0, (LPARAM)buffer);
								if (CB_ERR != iItem)
								{
									width = ResizeComboBoxDropDownW(hwndDlg, IDC_COMBO_PRESETS, buffer, width);
									SendMessageW(controlWindow, CB_SETITEMDATA,(WPARAM)iItem, (LPARAM)(i + 1));
								}
							}
						}
					}
				}

				for(size_t i=0; i < ARRAYSIZE(filters); i++) 
				{					
					controlWindow = GetDlgItem(hwndDlg, filters[i]);
					if (NULL != controlWindow)
					{
						int width = 0;
						for(unsigned int filterId = 0;; filterId++) 
						{
							if (NULL == getFilterName(filterId, buffer, ARRAYSIZE(buffer)))
								break;

							iItem = SendMessageW(controlWindow, CB_ADDSTRING, 0, (LPARAM)buffer);
							if (CB_ERR != iItem)
							{
								width = ResizeComboBoxDropDownW(hwndDlg, filters[i], buffer, width);
								SendMessageW(controlWindow, CB_SETITEMDATA,(WPARAM)iItem, (LPARAM)filterId);
							}
						}
					}
				}

				if (lParam == 1)
				{
					if (m_edit_item == -1)
					{
						// nothing to do, m_item_name, m_item_query & m_item_mode have been set already
					}
					else
					{
						lstrcpynW(m_item_name,m_query_list[m_edit_item]->name,sizeof(m_item_name)/sizeof(wchar_t));
						lstrcpynW(m_item_query,m_query_list[m_edit_item]->query,sizeof(m_item_query)/sizeof(wchar_t));
						m_item_mode = m_query_list[m_edit_item]->mode;
						m_item_image = m_query_list[m_edit_item]->imgIndex;

						// re-select the preset name if we can get a match to a known preset (looks nicer and all that)
						controlWindow = GetDlgItem(hwndDlg, IDC_COMBO_PRESETS);
						if (NULL != controlWindow)
						{
							for(size_t i=0; i < NUM_PRESETS; i++) 
							{
								if(m_item_query[0] && !_wcsicmp(presets[i].query, m_item_query)
									&& presets[i].mode == m_item_mode)
								{
									SendMessageW(controlWindow, CB_SETCURSEL, i, 0);
									SendMessage(hwndDlg,WM_COMMAND,(WPARAM)MAKEWPARAM(IDC_COMBO_PRESETS,CBN_SELCHANGE),(LPARAM)GetDlgItem(hwndDlg,IDC_COMBO_PRESETS));
									break;
								}
							}
						}

						// config
						wchar_t configDir[MAX_PATH] = {0};
						PathCombineW(configDir, g_viewsDir, m_query_list[m_edit_item]->metafn);
						conf = new C_Config(configDir);
						CheckDlgButton(hwndDlg, IDC_HIDE_EXTINFO, conf->ReadInt(L"midivvis", 1) ? 0 : 1);
					}
					SetWindowTextW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_EDIT_SMART_VIEW));
					m_editting=1;
				}
				else
				{
					m_editting=0;
					if (m_item_mode != -1) m_item_mode=0;
					m_item_name[0]=0;
					m_item_query[0]=0;
					m_item_image = MLTREEIMAGE_DEFAULT;
					
					wchar_t filename[1024 + 256] = {0};
					extern void makemetafn(wchar_t *filename, wchar_t **out);
					makemetafn(filename, &m_item_meta);
					conf = new C_Config(filename);
				}
				
				if(!mode) m_scrollchild=WASABI_API_CREATEDIALOGW(IDD_SCROLLCHILDHOST,hwndDlg,scrollChildHostProc);
				else m_scrollchild=WASABI_API_CREATEDIALOGW(IDD_ADD_VIEW_CHILD_ADVANCED,hwndDlg,childAdvanced);

				// this will make sure that we've got the little images shown in all languages (done for 5.51+)
				SetStaticItemImage(hwndDlg,IDC_IMAGE_SIMPLE,IDB_NEWFILTER_SIMPLE);
				SetStaticItemImage(hwndDlg,IDC_IMAGE_SIMPLEALBUM,IDB_NEWFILTER_SIMPLEALBUM);
				SetStaticItemImage(hwndDlg,IDC_IMAGE_TWOFILTERS,IDB_NEWFILTER_TWOFILTERS);
				SetStaticItemImage(hwndDlg,IDC_IMAGE_THREEFILTERS,IDB_NEWFILTER_THREEFILTERS);

				if(m_editting==0 && g_config->ReadInt(L"newfilter_showhelp",1))
				{
					showdlgelements(hwndDlg,SW_HIDE);
					ShowWindow(hwndDlg,SW_SHOWNA);
					EnableWindow(GetParent(hwndDlg),0);
					break; // don't run through
				}
				else
				{
					ShowWindow(GetDlgItem(hwndDlg,IDC_STATIC_INFO),SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK_SHOWINFO),SW_HIDE);
					// run through
				}
			}
		case WM_USER+9:
			{ // do stuff when we're shown or preset is changed
				showdlgelements(hwndDlg,SW_SHOWNA);
				ShowWindow(m_scrollchild,SW_SHOWNA);
				// set up filter radio buttons and combo boxes
				extern int GetFilter(int mode, int n);
				extern int GetNumFilters(int mode);
				
				int numFilters=GetNumFilters(m_item_mode);
				int f[3] = {0};
				f[0] = GetFilter(m_item_mode,0);
				f[1] = GetFilter(m_item_mode,1);
				f[2] = GetFilter(m_item_mode,2);

				if(numFilters==0) CheckRadioButton(hwndDlg,IDC_RADIO_TWOFILTERS,IDC_RADIO_SIMPLEALBUM,IDC_RADIO_SIMPLE);
				else if(numFilters==1) CheckRadioButton(hwndDlg,IDC_RADIO_TWOFILTERS,IDC_RADIO_SIMPLEALBUM,IDC_RADIO_SIMPLEALBUM);
				else if(numFilters==2) CheckRadioButton(hwndDlg,IDC_RADIO_TWOFILTERS,IDC_RADIO_SIMPLEALBUM,IDC_RADIO_TWOFILTERS);
				else if(numFilters==3) CheckRadioButton(hwndDlg,IDC_RADIO_TWOFILTERS,IDC_RADIO_SIMPLEALBUM,IDC_RADIO_THREEFILTERS);

				for(int i=0; i<3; i++) {
					ComboBox comboBox(hwndDlg, (i==0)?IDC_COMBO_FILTER1:((i==1)?IDC_COMBO_FILTER2:IDC_COMBO_FILTER3));
					if(f[i]==0) comboBox.SelectItem(i==0?0:5);
					else comboBox.SelectItem(f[i]-1);
				}
				// set up "name" edit control
				SetDlgItemTextW(hwndDlg,IDC_NAME,m_item_name);

				// do the query editor thing...
				SendMessage(hwndDlg,WM_USER+11,0,0);
			}
			// run through
		case WM_USER+10:
			{ // enable filter combo boxes based on radio buttons
				int numFilters=0;
				if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_SIMPLEALBUM)) numFilters=1;
				if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_TWOFILTERS)) numFilters=2;
				else if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_THREEFILTERS)) numFilters=3;
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC_FILTER3),numFilters>2);
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC_FILTER2),numFilters>1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO_FILTER3),numFilters>2);
				EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO_FILTER2),numFilters>1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO_FILTER1),numFilters>1);

				if (WM_INITDIALOG == uMsg)
				{
					// show edit info window and restore last position as applicable
					POINT pt = {g_config->ReadInt(L"smart_x", -1), g_config->ReadInt(L"smart_y", -1)};
					if (!windowOffScreen(hwndDlg, pt))
						SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
					else
						ShowWindow(hwndDlg, SW_SHOWNA);

					HWND parentWindow = GetParent(hwndDlg);
					if (NULL != parentWindow)
						EnableWindow(parentWindow, 0);
				}
				/*else
					ShowWindow(hwndDlg, SW_SHOW);*/
			}
			break;
		case WM_USER+11:
			if(!mode) {
				SPL_RemoveAll(m_scrollchild);

				EnterCriticalSection(&g_db_cs);
				nde_scanner_t p=NDE_Table_CreateScanner(g_table);
				((Scanner *)p)->disable_date_resolution=1; // TODO: don't use C++ NDE api
				
				int error=0; // not really an error, just we can't express it properly

				if (m_item_query[0] && !NDE_Scanner_Query(p, m_item_query))
					error=1; // ok, actually an error.

				if (m_item_query[0] && !error)
				{
					FiltersContext context;
					context.error = &error;
					context.last1 = 0;
					context.last2 = 0;
					context.m_scrollchild = m_scrollchild;
					context.mode = 0;
					((Scanner *)p)->WalkFilters((Scanner::FilterWalker)EnumFilters, &context); // TODO: don't use C++ NDE api
				}
				
				NDE_Table_DestroyScanner(g_table, p);
				LeaveCriticalSection(&g_db_cs);
				if(error) {
					SPL_RemoveAll(m_scrollchild);
					HWND x = SPL_AddBlankFilter(m_scrollchild);
					ComboBox c(x,IDC_COMBO1);
					int n = c.GetCount();
					for(int i=0; i<n; i++)	if(c.GetItemData(i) == 0x1ea7c0de) c.SelectItem(i);
					SetDlgItemTextW(x,IDC_EDIT1,m_item_query);
					SendMessage(x,WM_COMMAND,(WPARAM)MAKEWPARAM(IDC_COMBO1,CBN_SELCHANGE),(LPARAM)GetDlgItem(x,IDC_COMBO1));
				}

				m_simple_dirty=0;
				SPL_UpdateScroll(m_scrollchild);
			}
			break;
		case WM_DESTROY:
			if(conf) delete conf; conf=0;
			if(m_item_meta) free(m_item_meta); m_item_meta=0;
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_BUTTON_MODE:
					if(m_simple_dirty)
						SPL_GetQueryString(m_scrollchild);
					DestroyWindow(m_scrollchild);
					mode = !mode;
					if(!mode) m_scrollchild=WASABI_API_CREATEDIALOGW(IDD_SCROLLCHILDHOST,hwndDlg,scrollChildHostProc);
					else m_scrollchild=WASABI_API_CREATEDIALOGW(IDD_ADD_VIEW_CHILD_ADVANCED,hwndDlg,childAdvanced);
					SendMessage(hwndDlg,WM_USER+11,0,0);
					SendMessage(hwndDlg,WM_USER+10,0,0);
					ShowWindow(m_scrollchild,SW_SHOWNA);
					ShowWindow(GetDlgItem(hwndDlg,IDC_STATIC_INFO),SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK_SHOWINFO),SW_HIDE);
					
					SetDlgItemTextW(hwndDlg,IDC_BUTTON_MODE,WASABI_API_LNGSTRINGW( !mode ? IDS_ADVANCED_MODE : IDS_SIMPLE_MODE));

					showdlgelements(hwndDlg,SW_SHOWNA);
					break;
				case IDC_COMBO_PRESETS:
					switch(HIWORD(wParam)) {
						case CBN_SELCHANGE:
							{
								ComboBox combo(hwndDlg,IDC_COMBO_PRESETS);
								int n = combo.GetItemData(combo.GetSelection());
								if(n>0 && n<=NUM_PRESETS) {
									wchar_t cusStr[16] = {0};
									n--;
									// populate with a preset
									m_item_mode = presets[n].mode;
									m_item_image = presets[n].imageIndex;
									if(!_wcsicmp(WASABI_API_LNGSTRINGW_BUF(IDS_CUSTOM,cusStr,16),
												WASABI_API_LNGSTRINGW(presets[n].title))) m_item_name[0]=0;
									else WASABI_API_LNGSTRINGW_BUF(presets[n].title,m_item_name,256);
									lstrcpynW(m_item_query,presets[n].query,sizeof(m_item_query)/sizeof(wchar_t));
									showdlgelements(hwndDlg,SW_SHOWNA);
									ShowWindow(m_scrollchild,SW_SHOWNA);
									ShowWindow(GetDlgItem(hwndDlg,IDC_STATIC_INFO),SW_HIDE);
									ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK_SHOWINFO),SW_HIDE);
									SendMessage(hwndDlg,WM_USER+9,0,0);
								}
							}
							break;
					}
					break;
				// filter type radio buttons (simulate clicks on the radio button when the image or static text gets clicked)
				case IDC_CHECK_SHOWINFO:
					g_config->WriteInt(L"newfilter_showhelp",0);
					break;
				case IDC_STATIC_SIMPLE:
				case IDC_IMAGE_SIMPLE:
					SendMessage(GetDlgItem(hwndDlg,IDC_RADIO_SIMPLE),BM_CLICK,0,0);
					break;
				case IDC_STATIC_SIMPLEALBUM:
				case IDC_IMAGE_SIMPLEALBUM:
					SendMessage(GetDlgItem(hwndDlg,IDC_RADIO_SIMPLEALBUM),BM_CLICK,0,0);
					break;
				case IDC_STATIC_TWOFILTERS:
				case IDC_IMAGE_TWOFILTERS:
					SendMessage(GetDlgItem(hwndDlg,IDC_RADIO_TWOFILTERS),BM_CLICK,0,0);
					break;
				case IDC_STATIC_THREEFILTERS:
				case IDC_IMAGE_THREEFILTERS:
					SendMessage(GetDlgItem(hwndDlg,IDC_RADIO_THREEFILTERS),BM_CLICK,0,0);
					break;
				// enable filter combo boxes based on radio buttons
				case IDC_RADIO_SIMPLE:
				case IDC_RADIO_SIMPLEALBUM:
				case IDC_RADIO_TWOFILTERS:
				case IDC_RADIO_THREEFILTERS:
					CheckDlgButton(hwndDlg,IDC_RADIO_SIMPLE,0);
					CheckDlgButton(hwndDlg,IDC_RADIO_SIMPLEALBUM,0);
					CheckDlgButton(hwndDlg,IDC_RADIO_TWOFILTERS,0);
					CheckDlgButton(hwndDlg,IDC_RADIO_THREEFILTERS,0);
					CheckDlgButton(hwndDlg,LOWORD(wParam),1);
					SendMessage(hwndDlg,WM_USER+10,0,0);
					break;
				// OK and Cancel buttons
				case IDOK:
				{
					//name
					GetDlgItemTextW(hwndDlg,IDC_NAME,m_item_name,255);
					m_item_name[255]=0;
					if(!m_item_name[0]) {
						wchar_t title[64] = {0};
						MessageBoxW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_MUST_ENTER_A_NAME),
								   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,title,64),MB_OK);
						return 0;
					}
					//query
					if (m_simple_dirty)
						SPL_GetQueryString(m_scrollchild);
					
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_SIMPLEALBUM))
					{
						if(conf)
						{
							conf->WriteInt(L"adiv2pos",100000);
							conf->WriteInt(L"albumartviewmode",2);
						}
						 m_item_mode = 0x0100000B;
					}
					else
					{ // mode
						int numFilters=3;
						if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_SIMPLE)) numFilters=0;
						else if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_TWOFILTERS)) numFilters=2;
						
						int f[3] = {0};
						for(int i=0; i<3; i++) {
							ComboBox comboBox(hwndDlg, (i==0)?IDC_COMBO_FILTER1:((i==1)?IDC_COMBO_FILTER2:IDC_COMBO_FILTER3));
							f[i] = (comboBox.GetItemData(comboBox.GetSelection()) + 1) & 0xff;
						}
						if(f[1] == 6) f[1]=0;
						if(numFilters == 2) f[2]=0;
						if(numFilters == 0) f[0]=f[1]=f[2]=0;
						m_item_mode = f[0] | (f[1] << 8) | (f[2] << 16);
					}
					
					if(conf) {
						int v = IsDlgButtonChecked(hwndDlg,IDC_HIDE_EXTINFO)?0:1;
						conf->WriteInt(L"midivvis",v);
						
						ComboBox combo(hwndDlg,IDC_COMBO_PRESETS);
						int n = combo.GetItemData(combo.GetSelection());
						if(n>0 && n<=NUM_PRESETS) {
							n--;
							conf->WriteInt(L"mv_sort_by", presets[n].sort_by);
							conf->WriteInt(L"mv_sort_dir", presets[n].sort_dir);

							if(presets[n].columns) {
								int cnt = 0;
								while ((unsigned char)presets[n].columns[cnt] != 0xff)
								{
									wchar_t buf[32] = {0};
									StringCchPrintfW(buf, ARRAYSIZE(buf), L"column%d", cnt);
									conf->WriteInt(buf, (unsigned char)presets[n].columns[cnt]);
									cnt++;
								}
								conf->WriteInt(L"nbcolumns", cnt);
							}
						}
					}

					if (m_edit_item == -1) {
						// nothing to do, return values are m_item_name, m_item_query and m_item_mode
					} else {
						if(!m_editting) addQueryItem(m_item_name,m_item_query,m_item_mode,1,m_item_meta, m_item_image);
						else replaceQueryItem(m_edit_item,m_item_name,m_item_query,m_item_mode, m_item_image);
						saveQueryTree();
					}
					if(m_editting) PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
				}
				case IDCANCEL:
				{
					RECT smart_rect = {0};
					GetWindowRect(hwndDlg, &smart_rect);
					g_config->WriteInt(L"smart_x", smart_rect.left);
					g_config->WriteInt(L"smart_y", smart_rect.top);

					EndDialog(hwndDlg,(LOWORD(wParam) == IDOK));
					EnableWindow(GetParent(hwndDlg),1);
					break;
				}
			}
			break;
	}
	return 0;
}

static INT_PTR CALLBACK childAdvanced(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				RECT r;
				HWND parentWindow, controlWindow;
				parentWindow = GetAncestor(hwndDlg, GA_PARENT);
				if (NULL != parentWindow && 
					FALSE != GetWindowRect(GetDlgItem(parentWindow,IDC_CHILDFRAME),&r))
				{
					MapWindowPoints(HWND_DESKTOP, parentWindow, (POINT*)&r, 2);
					SetWindowPos(hwndDlg,NULL,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE);
				}
				
				SetDlgItemTextW(hwndDlg,IDC_QUERY,m_item_query);

				controlWindow = GetDlgItem(hwndDlg, IDC_EDIT1);
				if (NULL != controlWindow)
				{
					SetWindowTextA(controlWindow, (char*)WASABI_API_LOADRESFROMFILEW(L"TEXT", MAKEINTRESOURCEW(IDR_QUERIES_TEXT), 0));
				}
			}
			return 1;
		case WM_USER+40: // get query str
			GetDlgItemTextW(hwndDlg,IDC_QUERY,m_item_query,1024);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_EDIT:
					{
						wchar_t query[1024] = {0};
						GetDlgItemTextW(hwndDlg,IDC_QUERY,query, 1024);
						wchar_t querybuf[4096] = {0};
						const wchar_t *newquery = editQuery(hwndDlg, query, querybuf, 4096);
						if (newquery != NULL) {
							SetDlgItemTextW(hwndDlg, IDC_QUERY, newquery);
							m_simple_dirty=1;
						}
					}
					break;
				case IDC_QUERY:
					m_simple_dirty=1;
					break;
			}
			break;
	}

	const int controls[] = 
	{
		IDC_EDIT1,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
}

BOOL IsDirectMouseWheelMessage(const UINT uMsg)
{
	static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;

	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
	{
		WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");
		if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
			return FALSE;
	}

	return (WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg);
}

static INT_PTR CALLBACK filterProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      // populate comboboxes
      {
        HWND hwndCB=GetDlgItem(hwndDlg,IDC_COMBO1);
        int strs[]=
        {
          IDS_FILENAME,
          IDS_TITLE,
          IDS_ARTIST,
          IDS_ALBUM,
          IDS_YEAR,
          IDS_GENRE,
          IDS_COMMENT,
          IDS_TRACK_NUMBER,
          IDS_LENGTH,
          IDS_IS_VIDEO,
          IDS_LAST_UPDATED,
          IDS_PLAYED_LAST,
          IDS_RATING,
          NULL, // fear the 13
          NULL, // gracenote
          IDS_PLAY_COUNT,
          IDS_FILE_TIME,
          IDS_FILE_SIZE_KB,
          IDS_BITRATE_KBPS,
		  IDS_DISC_NUMBER,
		  IDS_ALBUM_ARTIST,
		  IDS_ALBUM_GAIN,
		  IDS_TRACK_GAIN,
		  IDS_PUBLISHER,
		  IDS_COMPOSER,
		  IDS_BPM, // no bpm
		  IDS_DISCS,
		  IDS_TRACKS,
		  IDS_IS_PODCAST,
		  IDS_PODCAST_CHANNEL,
		  IDS_PODCAST_PUBLISH_DATE,
          NULL,
          NULL,
          IDS_LOSSLESS,
          IDS_CATEGORY,
          IDS_CODEC,
          IDS_DIRECTOR,
          IDS_PRODUCER,
          IDS_WIDTH,
          IDS_HEIGHT,
		  NULL,
		  IDS_DATE_ADDED,
        };
        int x;
        int cnt = 0, width = 0;
		wchar_t * str;
        for (x = 0; x < sizeof(strs)/sizeof(strs[0]); x ++)
        {
          if (strs[x])
          {
			int a=SendMessageW(hwndCB, CB_ADDSTRING,0,(LPARAM)(str = WASABI_API_LNGSTRINGW(strs[x])));
			width = ResizeComboBoxDropDownW(hwndDlg, IDC_COMBO1, str, width);
            SendMessage(hwndCB,CB_SETITEMDATA,(WPARAM)a,(LPARAM)x);
            cnt++;
          }
        }

		int a=SendMessageW(hwndCB, CB_ADDSTRING,0,(LPARAM)(str = WASABI_API_LNGSTRINGW(IDS_CUSTOM)));
		ResizeComboBoxDropDownW(hwndDlg, IDC_COMBO1, str, width);
		SendMessage(hwndCB,CB_SETITEMDATA,(WPARAM)a,(LPARAM)0x1ea7c0de);
		cnt++;
        if (lParam)
        {
          nde_filter_t filter=(nde_filter_t)lParam;
          int a=NDE_Filter_GetID(filter);
          if (a != -1)
          {
            for (x = 0; x < cnt; x ++)
            {
			  int d = SendMessage(hwndCB,CB_GETITEMDATA,(WPARAM)x,0);
              if (d == a)
                break;
            }
            if (x < cnt)
            {
              SendMessage(hwndCB,CB_SETCURSEL,x,0);
              SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO1,CBN_SELCHANGE),(LPARAM)hwndCB);
              int filtop=NDE_Filter_GetOp(filter);
              HWND hwndCB2=GetDlgItem(hwndDlg,IDC_COMBO2);
              cnt=SendMessage(hwndCB2,CB_GETCOUNT,0,0);
              for (x = 0; x < cnt; x ++)
              {
                if (SendMessage(hwndCB2,CB_GETITEMDATA,(WPARAM)x,0) == filtop)
                  break;
              }
              if (x < cnt)
              {
                SendMessage(hwndCB2,CB_SETCURSEL,x,0);
                SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO2,CBN_SELCHANGE),(LPARAM)hwndCB2);
              }
            }
          }
          nde_field_t f=NDE_Filter_GetData(filter);
          if (f)
          {
            int ft=NDE_Field_GetType(f);
            //hack cause we never actualy get FIELD_LENGTH here.
            if (ft==FIELD_INTEGER && a==MAINTABLE_ID_LENGTH) ft=FIELD_LENGTH;

            switch (ft)
            {
              case FIELD_FILENAME:
              case FIELD_STRING:
                SetDlgItemTextW(hwndDlg,IDC_EDIT1,NDE_StringField_GetString(f));
              break;
              case FIELD_LENGTH:
                {
                  int v=NDE_IntegerField_GetValue(f);
                  wchar_t buf[128] = {0};
                  if (v < 60)
                    wsprintf(buf,L"%d",v);
                  else if (v < 60*60)
                    wsprintf(buf,L"%d:%02d",v/60,v%60);
                  else
                    wsprintf(buf,L"%d:%02d:%02d",v/60/60,(v/60)%60,v%60);

                  SetDlgItemText(hwndDlg,IDC_EDIT1,buf);
                }
              break;
              case FIELD_INTEGER:
                SetDlgItemInt(hwndDlg,IDC_EDIT1,NDE_IntegerField_GetValue(f),TRUE);
              break;
            }
          }
        }
      }
		return 1;
		
	case WM_USER+29:
		if(IsDlgButtonChecked(hwndDlg,IDC_AND)) return 1;
		if(IsDlgButtonChecked(hwndDlg,IDC_OR)) return 0;
		CheckDlgButton(hwndDlg,wParam?IDC_AND:IDC_OR,TRUE);
		return wParam;
    case WM_USER+40:
      if (wParam && lParam>0)
      {
        wchar_t *buf=(wchar_t *)wParam;
        size_t buf_len=(size_t)lParam;
        // produce expression here
        HWND hwndCB=GetDlgItem(hwndDlg,IDC_COMBO1);
        HWND hwndCB2=GetDlgItem(hwndDlg,IDC_COMBO2);
        int x=SendMessage(hwndCB,CB_GETCURSEL,0,0);
        if (x != CB_ERR)
        {
          int Id=SendMessage(hwndCB,CB_GETITEMDATA,x,0);
					if(Id == 0x1ea7c0de) { // custom!
						GetDlgItemTextW(hwndDlg,IDC_EDIT1,buf,buf_len);
						return 0;
					}
          x = SendMessage(hwndCB2,CB_GETCURSEL,0,0);
          if (x != CB_ERR)
          {
            int Op=SendMessage(hwndCB2,CB_GETITEMDATA,x,0);

            if (Id != -1 && Op != -1)
            {
              wchar_t res[256] = {0};
              GetDlgItemTextW(hwndDlg,IDC_EDIT1,res,255);
              res[255]=0;
              nde_field_t p = NDE_Table_GetColumnByID(g_table, (unsigned char)Id);
              if (p)
              {
				const wchar_t *fn=NDE_ColumnField_GetFieldName(p);

			
                wchar_t *opstr=NULL;
                switch (Op)
                {
                  case FILTER_EQUALS: opstr=L"="; break;
                  case FILTER_NOTEQUALS: opstr=L"!="; break;
                  case FILTER_CONTAINS: opstr=L"HAS"; break; 
                  case FILTER_NOTCONTAINS: opstr=L"NOTHAS"; break; 
                  case FILTER_ABOVE: opstr=L">"; break;
                  case FILTER_BELOW: opstr=L"<"; break;
                  case FILTER_ABOVEOREQUAL: opstr=L">="; break;
                  case FILTER_BELOWOREQUAL: opstr=L"<="; break;
                  case FILTER_BEGINS: opstr=L"BEGINS"; break;
                  case FILTER_ENDS: opstr=L"ENDS"; break;
                  case FILTER_LIKE: opstr=L"LIKE"; break;
                  case FILTER_ISEMPTY: opstr=L"ISEMPTY"; break;   
                  case FILTER_ISNOTEMPTY: opstr=L"ISNOTEMPTY"; break;   
                }
                if (fn && fn[0] && opstr && opstr[0])
                {
                  if (Op == FILTER_ISEMPTY || Op == FILTER_ISNOTEMPTY)
                    wsprintfW(buf, L"%s %s",fn,opstr);
                  else if (NDE_ColumnField_GetDataType(p) == FIELD_DATETIME)
                    wsprintfW(buf, L"%s %s [%s]",fn,opstr,res);
                  else
                  {
                    GayStringW escaped;
                    queryStrEscape(res, escaped);
                    wsprintfW(buf, L"%s %s \"%s\"",fn,opstr,escaped.Get());
                  }
                }             
              }
            } 
          }
        }
      }
    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_BUTTON1:
          SendMessage(GetParent(hwndDlg),WM_USER+34,(WPARAM)hwndDlg,-1);
          m_simple_dirty=1;
        return 0;
				case IDC_OR:
				case IDC_AND:
					m_simple_dirty=1;
					break;
				case IDC_BUTTON_QUERYBUILD:
					{
						wchar_t query[1024] = {0};
						GetDlgItemTextW(hwndDlg,IDC_EDIT,query, 1024);
						wchar_t querybuf[4096] = {0};
						const wchar_t *newquery = editQuery(hwndDlg, query, querybuf, 4096);
						if (newquery != NULL) {
							SetDlgItemTextW(hwndDlg, IDC_EDIT, newquery);
							m_simple_dirty=1;
						}
					}
					break;
        case IDC_BUTTON2:
          {
            wchar_t query[1024] = {0};
            GetDlgItemTextW(hwndDlg,IDC_EDIT1,query,1024);

            const wchar_t *newquery = editTime(hwndDlg, query);
            if (newquery != NULL) SetDlgItemTextW(hwndDlg, IDC_EDIT1, newquery);
          }
          // todo: date picker
        return 0;
        case IDC_EDIT1:
          if (HIWORD(wParam) == EN_CHANGE)
          {
            m_simple_dirty=1;
          }
        return 0;
        case IDC_COMBO2:
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            m_simple_dirty=1;
            HWND hwndCB2=(HWND)lParam;
            int x=SendMessage(hwndCB2,CB_GETCURSEL,0,0);
            if (x != CB_ERR)
            {
              int a=SendMessage(hwndCB2,CB_GETITEMDATA,(WPARAM)x,0);
              if (a == FILTER_ISEMPTY || a == FILTER_ISNOTEMPTY)
              {
                EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),0);
              }
              else 
              {
                EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),1);
              }
            }
          }
        return 0;
        case IDC_COMBO1:
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            m_simple_dirty=1;
            HWND hwndCB=(HWND)lParam;
            HWND hwndCB2=GetDlgItem(hwndDlg,IDC_COMBO2);

            int x=SendMessage(hwndCB,CB_GETCURSEL,0,0);
#define GAP 2
            if (x != CB_ERR)
            {
              int lastsel=SendMessage(hwndCB2,CB_GETCURSEL,0,0);
              SendMessage(hwndCB2,CB_RESETCONTENT,0,0);
              // populate with proper comparisons for type. give a default, too
              int myId = SendMessage(hwndCB,CB_GETITEMDATA,(WPARAM)x,0);
			  if(myId == 0x1ea7c0de) { // custom!
				ShowWindow(GetDlgItem(hwndDlg,IDC_BUTTON_QUERYBUILD),SW_SHOWNA);
				ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO2),SW_HIDE);
								
				RECT r;
                GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON2),&r);
                ScreenToClient(hwndDlg,((LPPOINT)&r));
								
				RECT r1;
                GetWindowRect(GetDlgItem(hwndDlg,IDC_COMBO1),&r1);
                ScreenToClient(hwndDlg,((LPPOINT)&r1));
								ScreenToClient(hwndDlg,((LPPOINT)&r1)+1);
								
				RECT r2;
                GetWindowRect(GetDlgItem(hwndDlg,IDC_EDIT1),&r2);
                ScreenToClient(hwndDlg,((LPPOINT)&r2));
                ScreenToClient(hwndDlg,((LPPOINT)&r2)+1);
								
				SetWindowPos(GetDlgItem(hwndDlg,IDC_EDIT1),0,r1.right+GAP,r1.top,r.left-r1.right-GAP-GAP,r2.bottom-r2.top,/*SWP_NOMOVE|*/SWP_NOACTIVATE|SWP_NOZORDER);
				return 0;
		      }

			  ShowWindow(GetDlgItem(hwndDlg,IDC_BUTTON_QUERYBUILD),SW_HIDE);
			  ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO2),SW_SHOWNA);

			  nde_field_t p = (-1 != myId) ? NDE_Table_GetColumnByID(g_table, (unsigned char)myId) : NULL;
              if (p)
              {
                if (NDE_ColumnField_GetDataType(p)==FIELD_DATETIME)
                {
                  //if (!IsWindowVisible(GetDlgItem(hwndDlg,IDC_BUTTON2)))
                  {
                    RECT r;
                    GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON2),&r);
                    ScreenToClient(hwndDlg,((LPPOINT)&r));

                    ShowWindow(GetDlgItem(hwndDlg,IDC_BUTTON2),SW_SHOWNA);
			
					RECT r1;
                    GetWindowRect(GetDlgItem(hwndDlg,IDC_COMBO2),&r1);
                    ScreenToClient(hwndDlg,((LPPOINT)&r1));
										ScreenToClient(hwndDlg,((LPPOINT)&r1)+1);

                    RECT r2;
                    GetWindowRect(GetDlgItem(hwndDlg,IDC_EDIT1),&r2);
                    ScreenToClient(hwndDlg,((LPPOINT)&r2));
                    ScreenToClient(hwndDlg,((LPPOINT)&r2)+1);
										SetWindowPos(GetDlgItem(hwndDlg,IDC_EDIT1),0,r1.right+GAP,r1.top,r.left-r1.right-GAP-GAP,r2.bottom-r2.top,/*SWP_NOMOVE|*/SWP_NOACTIVATE|SWP_NOZORDER);
                  }
                }
                else
                {
                  //if (IsWindowVisible(GetDlgItem(hwndDlg,IDC_BUTTON2)))
                  {
                    RECT r;
                    GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON2),&r);
                    ScreenToClient(hwndDlg,((LPPOINT)&r) + 1);

                    ShowWindow(GetDlgItem(hwndDlg,IDC_BUTTON2),SW_HIDE);
										
					RECT r1;
                    GetWindowRect(GetDlgItem(hwndDlg,IDC_COMBO2),&r1);
                    ScreenToClient(hwndDlg,((LPPOINT)&r1));
										ScreenToClient(hwndDlg,((LPPOINT)&r1)+1);

                    RECT r2;
                    GetWindowRect(GetDlgItem(hwndDlg,IDC_EDIT1),&r2);
                    ScreenToClient(hwndDlg,((LPPOINT)&r2));
                    ScreenToClient(hwndDlg,((LPPOINT)&r2)+1);
								   SetWindowPos(GetDlgItem(hwndDlg,IDC_EDIT1),0,r1.right+GAP,r1.top,r.right-r1.right-GAP,r2.bottom-r2.top,/*SWP_NOMOVE|*/SWP_NOACTIVATE|SWP_NOZORDER);
                  }
                }
#undef GAP
                typedef struct 
                {
                  int str;
                  char id;
                } fillT;

                int myfillt_len=0;
                fillT *myfillt=NULL;
                static fillT foo1[]=
                {
                  {IDS_EQUALS,FILTER_EQUALS},
                  {IDS_DOES_NOT_EQUAL,FILTER_NOTEQUALS},
                  {IDS_CONTAINS,FILTER_CONTAINS},
                  {IDS_DOES_NOT_CONTAIN,FILTER_NOTCONTAINS},
                  {IDS_IS_ABOVE,FILTER_ABOVE},
                  {IDS_IS_BELOW,FILTER_BELOW},
                  {IDS_EQUALS_OR_IS_ABOVE,FILTER_ABOVEOREQUAL},
                  {IDS_EQUALS_OR_IS_BELOW,FILTER_BELOWOREQUAL},
                  {IDS_IS_EMPTY,FILTER_ISEMPTY},
                  {IDS_IS_NOT_EMPTY,FILTER_ISNOTEMPTY},
                  {IDS_BEGINS_WITH,FILTER_BEGINS},
                  {IDS_ENDS_WITH,FILTER_ENDS},
                  {IDS_IS_SIMILAR_TO,FILTER_LIKE},
                };
                static fillT foo2[]=
                {
                  {IDS_AT,FILTER_EQUALS},
                  {IDS_NOT_AT,FILTER_NOTEQUALS},
                  {IDS_AFTER,FILTER_ABOVE},
                  {IDS_BEFORE,FILTER_BELOW},
                  {IDS_SINCE,FILTER_ABOVEOREQUAL},
                  {IDS_UNTIL,FILTER_BELOWOREQUAL},
                  {IDS_IS_EMPTY,FILTER_ISEMPTY},
                  {IDS_IS_NOT_EMPTY,FILTER_ISNOTEMPTY},
                };
                switch (NDE_ColumnField_GetDataType(p))
                {
                  case FIELD_DATETIME:
                    {
                      myfillt_len = sizeof(foo2)/sizeof(foo2[0]);
                      myfillt=foo2;
                    }
                  break;
                  case FIELD_LENGTH:
                  case FIELD_INTEGER:
                    {
                      myfillt_len = sizeof(foo1)/sizeof(foo1[0]) - 3;
                      myfillt=foo1;
                    }
                  break;
									case FIELD_FILENAME:
                  case FIELD_STRING:
                    {
                      myfillt_len = sizeof(foo1)/sizeof(foo1[0]);
                      myfillt=foo1;
                    }
                  break;
                  default:
                  break;
                }
                if (myfillt)
                {
				  wchar_t *str;
				  int width = 0;
                  while (myfillt_len--)
                  {
					int a=SendMessageW(hwndCB2,CB_ADDSTRING,0,(LPARAM)(str = WASABI_API_LNGSTRINGW(myfillt->str)));
					width = ResizeComboBoxDropDownW(hwndDlg, IDC_COMBO2, str, width);
                    SendMessage(hwndCB2,CB_SETITEMDATA,a,(LPARAM)myfillt->id);
                    myfillt++;
                  }
                  if (lastsel != CB_ERR)
                    SendMessage(hwndCB2,CB_SETCURSEL,lastsel,0);
                }
              }
            }
          }
        return 0;
      }
    return 0;
  }

  if (FALSE != IsDirectMouseWheelMessage(uMsg) ||
		WM_MOUSEWHEEL == uMsg)
	{
		HWND parentWindow;
		parentWindow = GetAncestor(hwndDlg, GA_PARENT);
		if (NULL != parentWindow)
		{
			SendMessageW(parentWindow, WM_MOUSEWHEEL, wParam, lParam);
			SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (long)TRUE);
			return TRUE;
		}
	}

  return 0;
}


static INT_PTR CALLBACK scrollChildProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	static int osize;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT r;
			GetClientRect(hwndDlg,&r);
			osize=r.bottom;
		}
		return 1;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON1)
			SPL_AddBlankFilter(hwndDlg);
		return 0;
	case WM_USER+40: // get query string
		if (!lParam)
		{
			// now we go through our children, asking each one for a string, and combine them.
			GayStringW str;
			HWND h=GetWindow(hwndDlg,GW_CHILD);
			wchar_t * nextop=NULL;
			while (h)
			{
				if (h != GetDlgItem(hwndDlg,IDC_BUTTON1))
				{
					wchar_t buf[512] = {0};
					buf[0]=0;
					SendMessage(h,WM_USER+40,(WPARAM)buf,(LPARAM)sizeof(buf)/sizeof(*buf));
					if (buf[0])
					{
						if(nextop) str.Append(nextop);
						nextop = IsDlgButtonChecked(h,IDC_AND) ? L" AND " : L" OR ";
						//if (str.Get() && str.Get()[0]) str.Append(isOr ? L" OR " : L" AND ");
						str.Append(buf);
					}
				}
				h=GetWindow(h,GW_HWNDNEXT);
			}
			lstrcpynW(m_item_query,str.Get()?str.Get():L"",sizeof(m_item_query)/sizeof(*m_item_query));
		}
		return 0;
	case WM_USER+41: // remove all
		{
			HWND h=GetWindow(hwndDlg,GW_CHILD);
			std::vector<void*> w;
			while (h) {
				if (h != GetDlgItem(hwndDlg,IDC_BUTTON1))
					w.push_back((void*)h);
				h=GetWindow(h,GW_HWNDNEXT);
			}
			for ( void *l_w : w )
				SendMessage( hwndDlg, WM_USER + 34, (WPARAM)(HWND)l_w, -1 );
		}
		break;
	case WM_USER+34: // remove filter by hwnd
		if (wParam && lParam == -1)
		{
			HWND hwndRemove = (HWND) wParam;
			RECT r;

			GetClientRect(hwndRemove,&r);
			int lh=r.bottom; // height to remove

			GetWindowRect(hwndRemove,&r);
			ScreenToClient(hwndDlg,(LPPOINT)&r);
			int ltop=r.top;

			DestroyWindow(hwndRemove);

			HWND h=GetWindow(hwndDlg,GW_CHILD);
			while (h)
			{
				RECT r;

				GetWindowRect(h,&r);
				ScreenToClient(hwndDlg,(LPPOINT)&r);

				if (r.top > ltop)
				{
					SetWindowPos(h,0,r.left,r.top - lh,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
				}
				h=GetWindow(h,GW_HWNDNEXT);
			}

			GetClientRect(hwndDlg,&r);
			r.bottom -= lh;
			SetWindowPos(hwndDlg,0,0,0,r.right,r.bottom,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

			SendMessage(GetParent(hwndDlg),WM_USER+33,17,0);
		}
		return 0;
	case WM_USER+32: // add filter. lParam is mode. 1 means add by filter, 2 means add blank
		if(lParam == 2) {
			BOOL b=0;
			HWND h=GetWindow(hwndDlg,GW_CHILD);
			while (h) {
				if (h != GetDlgItem(hwndDlg,IDC_BUTTON1))
					b = filterProc(h,WM_USER+29,(WPARAM)b,0);
				h=GetWindow(h,GW_HWNDNEXT);
			}
		}
		if ((lParam == 1 && wParam) || (lParam == 2 && !wParam))
		{
			HWND newChild=0;
			nde_filter_t filter=(nde_filter_t)wParam;
			if (lParam == 2 || (NULL != NDE_Table_GetColumnByID(g_table, NDE_Filter_GetID(filter))))
			{
				newChild=WASABI_API_CREATEDIALOGPARAMW(IDD_SCROLLCHILDFILTER,hwndDlg,filterProc,(LPARAM)filter);
				RECT r,r2;
				GetClientRect(hwndDlg,&r);
				GetClientRect(newChild,&r2);
				SetWindowPos(hwndDlg,0,0,0,r.right,r.bottom + r2.bottom,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
				SetWindowPos(newChild,0,0,r.bottom - osize,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

				HWND h = GetDlgItem(hwndDlg,IDC_BUTTON1);
				GetWindowRect(h,&r);
				ScreenToClient(hwndDlg,(LPPOINT)&r);
				SetWindowPos(h,0,r.left,r.top + r2.bottom,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);          
				ShowWindow(newChild,SW_SHOWNA);
			}
			if (lParam == 2) {
				// update scroll, hugging bottom
				SendMessage(GetParent(hwndDlg),WM_USER+33,16,0);
			}
			return (intptr_t)newChild;
		}
		return 0;
	}

	if (FALSE != IsDirectMouseWheelMessage(uMsg) ||
		WM_MOUSEWHEEL == uMsg)
	{
		HWND parentWindow;
		parentWindow = GetAncestor(hwndDlg, GA_PARENT);
		if (NULL != parentWindow)
		{
			SendMessageW(parentWindow, WM_MOUSEWHEEL, wParam, lParam);
			SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (long)TRUE);
			return TRUE;
		}
	}
	return 0;
}

static INT_PTR CALLBACK scrollChildHostProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
  static HWND m_child;
  switch (uMsg)
  {
    case WM_INITDIALOG:
      {
        RECT r;
        GetWindowRect(GetDlgItem(GetParent(hwndDlg),IDC_CHILDFRAME),&r);
        ScreenToClient(GetParent(hwndDlg),(LPPOINT)&r);
        ScreenToClient(GetParent(hwndDlg),((LPPOINT)&r)+1);
        SetWindowPos(hwndDlg,NULL,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE);
      }
      m_child=WASABI_API_CREATEDIALOGW(IDD_SCROLLCHILD,hwndDlg,scrollChildProc);
    return 1;
    case WM_USER+33:
      if (m_child) 
      {
        RECT r;
        RECT r2;
        GetClientRect(hwndDlg,&r2);
        GetClientRect(m_child,&r);
        if (r2.bottom < r.bottom)
        {
          SCROLLINFO si={sizeof(si),SIF_RANGE|SIF_PAGE|SIF_POS,0,r.bottom,r2.bottom,wParam == 16 ? r.bottom : 0,0};

          if (wParam == 17)
            si.fMask &= ~SIF_POS;

          SetScrollInfo(hwndDlg,SB_VERT,&si,TRUE);

          if (wParam != 17)
            SetWindowPos(m_child,NULL,0,wParam == 16 ? r2.bottom-r.bottom : 0,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        } else {
          //hide the scrollbar
          SetWindowPos(m_child,NULL,0,0,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
          ShowScrollBar(hwndDlg,SB_VERT,FALSE);
        }
        ShowWindow(m_child,SW_SHOWNA);
      }
    return 0;
    case WM_USER+32:
	case WM_USER+41:
    case WM_USER+40:
	  if (m_child) return scrollChildProc(m_child,uMsg,wParam,lParam);
	  return 0;
    case WM_VSCROLL:
      {
        RECT r;
        RECT r2;
        GetClientRect(hwndDlg,&r2);
        GetClientRect(m_child,&r);

        if (r2.bottom < r.bottom)
        {
          int v=0;
          if (LOWORD(wParam) == SB_THUMBPOSITION || LOWORD(wParam) == SB_THUMBTRACK)
          {
            SCROLLINFO si={sizeof(si),SIF_TRACKPOS|SIF_POS};
            GetScrollInfo(hwndDlg,SB_VERT,&si);
            v=si.nTrackPos;
          }
          else if (LOWORD(wParam) == SB_TOP)
          {
            v=0;
          }
          else if (LOWORD(wParam) == SB_BOTTOM)
          {
            v=r.bottom-r2.bottom;
          }
          else if (LOWORD(wParam) == SB_PAGEDOWN || LOWORD(wParam) == SB_LINEDOWN)
          {
            SCROLLINFO si={sizeof(si),SIF_TRACKPOS|SIF_POS};
            GetScrollInfo(hwndDlg,SB_VERT,&si);
            v=si.nPos + r2.bottom;
            if (v > r.bottom-r2.bottom) v=r.bottom-r2.bottom;
          }
          else if (LOWORD(wParam) == SB_PAGEUP || LOWORD(wParam) == SB_LINEUP)
          {
            SCROLLINFO si={sizeof(si),SIF_TRACKPOS|SIF_POS};
            GetScrollInfo(hwndDlg,SB_VERT,&si);
            v=si.nPos - r2.bottom;
            if (v < 0) v=0;
          }
          else return 0;

          SetScrollPos(hwndDlg,SB_VERT,v,!(LOWORD(wParam) == SB_THUMBPOSITION || LOWORD(wParam) == SB_THUMBTRACK));
          SetWindowPos(m_child,NULL,0,0-v,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
        else
        {
          SetScrollPos(hwndDlg,SB_VERT,0,!(LOWORD(wParam) == SB_THUMBPOSITION || LOWORD(wParam) == SB_THUMBTRACK));
          SetWindowPos(m_child,NULL,0,0,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
      }
    return 0;
  }

	if (FALSE != IsDirectMouseWheelMessage(uMsg) ||
		WM_MOUSEWHEEL == uMsg)
	{
		WORD scrollCommand;
		short delta;
		
		delta = HIWORD(wParam);
		
		scrollCommand = (delta > 0) ? SB_LINEUP : SB_LINEDOWN;

		SendMessageW(hwndDlg, WM_VSCROLL, MAKEWPARAM(scrollCommand, 0), 0L);
	}
	return 0;
}

void addNewQuery(HWND parent) {
	HWND hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_ADD_VIEW_2, parent, addQueryFrameDialogProc2, 0);
	SetActiveWindow(hwnd);
}

void queryEditItem(int n)
{
	m_edit_item=n;
	HWND hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_ADD_VIEW_2, plugin.hwndLibraryParent, addQueryFrameDialogProc2, 1);
	SetActiveWindow(hwnd);
}

// returns true if edition was validated, false if cancel was clicked
// actual return values are in m_item_query, m_item_name and m_item_mode
int queryEditOther(HWND hwnd, const char *query, const char *viewname, int mode) 
{
	int notnew=1;
	if (query == NULL || !*query) notnew = 0;
	m_edit_item = -1;
	if (notnew) 
	{
		MultiByteToWideCharSZ(CP_ACP, 0, viewname, -1, m_item_name, 256);
		MultiByteToWideCharSZ(CP_ACP, 0, query, -1, m_item_query, 1024);
	}
	m_item_mode = mode;
	if(mode == -1)
		return WASABI_API_DIALOGBOXPARAMW(IDD_ADD_VIEW_2_NF, hwnd, addQueryFrameDialogProc2, notnew);
	else
		return WASABI_API_DIALOGBOXPARAMW(IDD_ADD_VIEW_2, hwnd, addQueryFrameDialogProc2, notnew);
}

void queryDeleteItem(HWND parent, int n)
{
	QueryList::iterator iter;
	iter = m_query_list.find(n);
	if (iter == m_query_list.end()) return;

	wchar_t title[64] = {0};
	queryItem *item = iter->second;
	if (MessageBoxW(parent,WASABI_API_LNGSTRINGW(IDS_DELETE_THIS_VIEW),
				   WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRMATION,title,64),
				   MB_YESNO|MB_ICONQUESTION) == IDYES)
	{
		mediaLibrary.RemoveTreeItem(iter->first);      
		m_query_list.erase(iter->first);
		saveQueryTree();

    // we deleted the item from the tree, which apparently is enough to close the current dialog if it was
    // the current dialog. hot.

		for(iter = m_query_list.begin(); iter != m_query_list.end(); iter++)
			if(iter->second && iter->second->index > item->index) iter->second->index--;

		wchar_t configDir[MAX_PATH] = {0};
		PathCombineW(configDir, g_viewsDir, item->metafn);
		DeleteFileW(configDir);
   
		free(item->metafn);
		free(item->name);
		free(item->query);
		free(item);
	}
}


 int IPC_LIBRARY_SENDTOMENU;
static librarySendToMenuStruct s_menu;

static WNDPROC ml_oldWndProc2;
static INT_PTR CALLBACK ml_newWndProc2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITMENUPOPUP:
			if (wParam && (HMENU)wParam == s_menu.build_hMenu && s_menu.mode==1)
			{
				myMenu = TRUE;
				if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s_menu, IPC_LIBRARY_SENDTOMENU)==(LRESULT)-1)
					s_menu.mode=2;
				myMenu = FALSE;
				if(ml_oldWndProc2) SetWindowLongPtrW(hwndDlg,GWLP_WNDPROC,(LONG_PTR)ml_oldWndProc2);
				ml_oldWndProc2 = NULL;

			}
			return 0;
	}
	if (ml_oldWndProc2) return CallWindowProc(ml_oldWndProc2,hwndDlg,uMsg,wParam,lParam);
	return 0;
}

HMENU main_sendto_hmenu;
int main_sendto_mode;

void view_queryContextMenu(INT_PTR param1, HWND hHost, POINTS pts, int n)
{
	queryItem *item=m_query_list[n];
	if (item == NULL) return;

	ml_oldWndProc2 = (WNDPROC)SetWindowLongPtrW(hHost, GWLP_WNDPROC, (LONG_PTR)ml_newWndProc2);

	HMENU menu=GetSubMenu(g_context_menus,2);
	main_sendto_hmenu=GetSubMenu(menu,2);

	s_menu.mode = 0;
	s_menu.hwnd = 0;
	s_menu.build_hMenu = 0;

	IPC_LIBRARY_SENDTOMENU = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)0, IPC_LIBRARY_SENDTOMENU)==(LRESULT)-1)
	{
		s_menu.mode = 1;
		s_menu.hwnd = hHost;
		s_menu.data_type = ML_TYPE_ITEMRECORDLIST;
		s_menu.ctx[1] = 1;
		s_menu.build_hMenu = main_sendto_hmenu;
	}

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		HNAVITEM hItem = (HNAVITEM)param1;
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}

	UpdateMenuItems(NULL, menu, IDR_QUERY_ACCELERATORS);
	int r = DoTrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
						pt.x, pt.y, hHost, NULL);
	if(ml_oldWndProc2) SetWindowLongPtrW(hHost,GWLP_WNDPROC,(LONG_PTR)ml_oldWndProc2);
	switch(r) 
	{
		case ID_QUERYWND_EDIT:
			queryEditItem(n);
			break;
		case ID_QUERYWND_DELETE:
			queryDeleteItem(hHost,n);
			break;
		case ID_QUERYWND_PLAYQUERY:
			{
				wchar_t configDir[MAX_PATH] = {0};
				PathCombineW(configDir, g_viewsDir, item->metafn);
				C_Config viewconf(configDir);
				main_playQuery(&viewconf,item->query,0);
			}
		    break;
		case ID_QUERYWND_ENQUEUEQUERY:
			{
				wchar_t configDir[MAX_PATH] = {0};
				PathCombineW(configDir, g_viewsDir, item->metafn);
				C_Config viewconf(configDir);
				main_playQuery(&viewconf,item->query,1);
			}
			break;
		case ID_QUERYMENU_ADDNEWQUERY:
			addNewQuery(hHost);
			break;
		default:
			if (s_menu.mode == 2)
			{
				s_menu.menu_id = r;
				if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s_menu, IPC_LIBRARY_SENDTOMENU) == (LRESULT)-1)
				{
					// build my data.
					s_menu.mode=3;
					s_menu.data_type = ML_TYPE_ITEMRECORDLISTW;
					wchar_t configDir[MAX_PATH] = {0};
					PathCombineW(configDir, g_viewsDir, item->metafn);
					C_Config viewconf(configDir);

					EnterCriticalSection(&g_db_cs);
					nde_scanner_t s=NDE_Table_CreateScanner(g_table);
					NDE_Scanner_Query(s, item->query);
					itemRecordListW obj={0,};
					saveQueryToListW(&viewconf, s, &obj, 0, 0, (resultsniff_funcW)-1);
					NDE_Table_DestroyScanner(g_table, s);
					LeaveCriticalSection(&g_db_cs);
					s_menu.data = (void*)&obj;

					LRESULT result = SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s_menu,IPC_LIBRARY_SENDTOMENU);
					if (result != 1)
					{
						s_menu.mode=3;
						s_menu.data_type = ML_TYPE_ITEMRECORDLIST;
						itemRecordList objA={0,};
						convertRecordList(&objA, &obj);
						s_menu.data = (void*)&objA;
						SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s_menu,IPC_LIBRARY_SENDTOMENU);
						freeRecordList(&objA);
					}
					freeRecordList(&obj);
					
				}
			}
			break;
	}
	if (s_menu.mode) 
	{
		s_menu.mode=4;
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s_menu,IPC_LIBRARY_SENDTOMENU); // cleanup
	}
	main_sendto_hmenu=0;
	EatKeyboard();
}

void queriesContextMenu(INT_PTR param1, HWND hHost, POINTS pts) {
	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		HNAVITEM hItem = (HNAVITEM)param1;
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}

	HMENU menu=GetSubMenu(g_context_menus,3);
	int r = DoTrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY,
						 pt.x, pt.y, hHost, NULL);
	switch(r) {
		case ID_QUERYMENU_ADDNEWQUERY:
			addNewQuery(hHost);
		break;
		case ID_QUERYMENU_PREFERENCES:
			SENDWAIPC(plugin.hwndWinampParent, IPC_OPENPREFSTOPAGE, &preferences);
		break;
		case ID_QUERYMENU_HELP:
			SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"https://help.winamp.com/hc/articles/8105304048660-The-Winamp-Media-Library");
		break;
	}
	EatKeyboard();
}