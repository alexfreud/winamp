/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "resource.h"
#include <malloc.h>
#include "gen.h"
#include "Options.h"
#include <vector>
#include "./api.h"

//#define DO_COLORS
std::vector<prefsDlgRec*> g_piprefsdlgs;
HWND prefs_hwnd = NULL;
intptr_t prefs_last_page;
RECT prefs_rect = {-1, -1}, alt3_rect = {-1, -1}, ctrle_rect = {-1, -1},
	 about_rect = {-1, -1}, loc_rect = {-1, -1}, time_rect = {-1, -1},
	 load_rect = {-1, -1}, editinfo_rect = {-1, -1};

static intptr_t pluginids;
static HANDLE lp_v = NULL;
static HANDLE _additem(HWND hwnd, HANDLE h, wchar_t *str, int children, intptr_t data)
{
	HANDLE h2;
	TV_INSERTSTRUCTW is = {(HTREEITEM)h, TVI_LAST, {TVIF_PARAM | TVIF_TEXT | TVIF_CHILDREN, 0, 0, 0, str, (int)wcslen(str), 0, 0, children ? 1 : 0, data}};
	h2 = (HTREEITEM)SendMessageW(hwnd, TVM_INSERTITEMW, 0, (LPARAM)&is);
	if (prefs_last_page == data) lp_v = h2;
	return h2;
}

static HANDLE _additem(HWND hwnd, HANDLE h, char *str, int children, intptr_t data)
{
	HANDLE h2;
	TV_INSERTSTRUCTA is = {(HTREEITEM)h, TVI_LAST, {TVIF_PARAM | TVIF_TEXT | TVIF_CHILDREN, 0, 0, 0, str, (int)strlen(str), 0, 0, children ? 1 : 0, data}};
	h2 = TreeView_InsertItem(hwnd, &is);
	if (prefs_last_page == data) lp_v = h2;
	return h2;
}

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
	SendMessageW(hTooltipWnd, TTM_SETDELAYTIME, TTDT_INITIAL, 0);
	SendMessageW(hTooltipWnd, TTM_SETDELAYTIME, TTDT_RESHOW, 0);
}

//////////////////////////
// TAB PROCEDURES
//////////////////////////

int g_taskbar_dirty;

// vis 2tab procedure
static HANDLE insertRootDialog(HWND hwnd, intptr_t which, HANDLE h, intptr_t* pids)
{
	for ( prefsDlgRec *p : g_piprefsdlgs )
	{
		if (which == p->where)
		{
			HANDLE h2;
			if (p->next == PREFS_UNICODE) // unicode
				h2 = _additem(hwnd, h, (wchar_t*)(p->name), 1, p->_id = (p->where < 0 ? (- 1 * p->where) : p->where));
			else // local code page
				h2 = _additem(hwnd, h, p->name, 1, p->_id = (p->where < 0 ? (-1 * p->where) : p->where));

			return h2;
		}
	}

	return INVALID_HANDLE_VALUE;
}

// vis 2tab procedure
static bool insertDialogs(HWND hwnd, intptr_t which, HANDLE h, intptr_t *pids)
{
	bool ret = false;
	for ( prefsDlgRec *p : g_piprefsdlgs )
	{
		if (which == p->where)
		{
			size_t j;
			for (j = 0; j != g_piprefsdlgs.size(); j++)
			{
				if (g_piprefsdlgs[j]->where == (intptr_t)p)
				{
					break;
				}
			}
			if ((intptr_t)p == prefs_last_page) 
				prefs_last_page = *pids;

			HANDLE h2;
			if (p->next == PREFS_UNICODE) // unicode
				h2 = _additem(hwnd, h, (wchar_t *)(p->name), j != g_piprefsdlgs.size(), p->_id = (*pids)++);
			else // local code page
				h2 = _additem(hwnd, h, p->name, j!=g_piprefsdlgs.size(), p->_id = (*pids)++);

			ret = true;
			if (j != g_piprefsdlgs.size() && insertDialogs(hwnd, (intptr_t)p, h2, pids))
				SendMessageW(hwnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)h2);
		}
	}
	return ret;
}

static HTREEITEM dofindByParam(intptr_t param, HWND treeview, HTREEITEM Tree)
{
	HTREEITEM f=TreeView_GetChild(treeview,Tree);
	do 
	{
		TVITEM tvi={TVIF_HANDLE|TVIF_PARAM|TVIF_CHILDREN,f,};
		TreeView_GetItem(treeview,&tvi);
		if (tvi.lParam==param) return tvi.hItem;
		if (tvi.cChildren)
		{
			HTREEITEM s=dofindByParam(param,treeview,tvi.hItem);
			if (s) return s;
		}
	} while (NULL != (f=TreeView_GetNextItem(treeview,f,TVGN_NEXT)));
	return NULL;
}

void prefs_liveDlgAdd(prefsDlgRec * p)
{
	if(!IsWindow(prefs_hwnd)) return;
	HWND htree = GetDlgItem(prefs_hwnd, IDC_TREE1);
	HTREEITEM parent = NULL;
	switch(p->where)
	{
		case -1:	parent = TVI_ROOT; break;
		case 0:		parent = dofindByParam(0,htree,NULL); break;
		case 1:		parent = dofindByParam(30,htree,NULL); break;
		case 2:		parent = dofindByParam(40,htree,NULL); break;
		case 6:		parent = dofindByParam(6, htree, NULL); break;
		default:
		{
			if (p->where < 0)
			{
				parent = TVI_ROOT;
				break;
			}
			else if (p->where < 65536)
				return;

			prefsDlgRec * pa = (prefsDlgRec *)p->where;
			parent = dofindByParam(pa->_id,htree,NULL);
			if(!parent) return;
			TVITEM t = {TVIF_CHILDREN,parent,0};
			TreeView_GetItem(htree,&t);
			if(!t.cChildren)
			{
				t.cChildren = 1;
				TreeView_SetItem(htree,&t);
			}
		}
	}
	if (!parent) return;
	if (p->next == PREFS_UNICODE) // unicode
		_additem(htree,parent,(wchar_t *)(p->name), 0, p->_id = pluginids++);
	else // local code page
		_additem(htree,parent,p->name, 0, p->_id = pluginids++);

	SendMessageW(htree, TVM_EXPAND, TVE_EXPAND, (LPARAM)parent);
}

void prefs_liveDlgRemove(prefsDlgRec * p)
{
	if(!IsWindow(prefs_hwnd)) return;
	HWND htree = GetDlgItem(prefs_hwnd, IDC_TREE1);
	HTREEITEM t = dofindByParam(p->_id,htree,NULL);
	if(!t) return;

	HTREEITEM tp = TreeView_GetParent(htree, t);
	TreeView_DeleteItem(htree,t);
	if (!TreeView_GetChild(htree, tp))
	{
		// clears the [+]/[-] box if no children
		TVITEM t2 = {TVIF_CHILDREN,tp,0};
		TreeView_GetItem(htree,&t2);
		t2.cChildren = 0;
		TreeView_SetItem(htree,&t2);
	}
}

void prefs_liveDlgUpdate(prefsDlgRec * p)
{
	if(!IsWindow(prefs_hwnd)) return;
	HWND htree = GetDlgItem(prefs_hwnd, IDC_TREE1);
	HTREEITEM t = dofindByParam(p->_id,htree,NULL);
	if(!t) return;

	if (p->next == PREFS_UNICODE)
	{
		TVITEMW tvi = {TVIF_TEXT,t,0};
		tvi.pszText = (wchar_t *)p->name;
		tvi.cchTextMax = (int)wcslen(tvi.pszText);
		SendMessageW(htree,TVM_SETITEMW,0,(LPARAM)&tvi);
	}
	else
	{
		TVITEMA tvi = {TVIF_TEXT,t,0};
		tvi.pszText = (char *)p->name;
		tvi.cchTextMax = (int)strlen(tvi.pszText);
		SendMessageW(htree,TVM_SETITEMA,0,(LPARAM)&tvi);
	}
}

HWND _dosetsel(HWND hwndDlg, HWND subwnd, int* last_page, multiPage* pages, int numpages)
{
	HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
	int sel=TabCtrl_GetCurSel(tabwnd);

	if (sel >= 0 && (sel != *last_page || !subwnd))
	{		
		*last_page = sel;
		if (IsWindow(subwnd)) DestroyWindow(subwnd);
		subwnd=0;
		
		if (sel < numpages)
		{
			int t=0;
			WNDPROC p;

			t = pages[sel].id;
			p = pages[sel].proc;
			
			if (t) subwnd = LPCreateDialogW(t, hwndDlg, p);
		}
				
		if (IsWindow(subwnd))
		{
			RECT r;
			GetClientRect(tabwnd,&r);
			TabCtrl_AdjustRect(tabwnd,FALSE,&r);
			SetWindowPos(subwnd,HWND_TOP,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE);
			ShowWindow(subwnd,SW_SHOWNA);
		}

		if (IsWinXPTheme()) 
		{
			DoWinXPStyle(tabwnd);
			DoWinXPStyle(subwnd);
		}
	}
	return subwnd;
}

//////////////////////////
// OUTER PROCEDURE
//////////////////////////
static LRESULT CALLBACK OuterProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND cur_wnd;
	static HWND prevActiveWindow = NULL;
	switch (uMsg)
	{
#ifdef DO_COLORS
		case WM_CTLCOLORBTN:
			if (lParam == (LPARAM)GetDlgItem(hwndDlg, IDOK))
			{
				//				SetTextColor(wParam,RGB(0,199,0));
				return (LRESULT)GetStockObject(WHITE_BRUSH);
			}
			break;
		case WM_CTLCOLOREDIT:
			if (lParam == (LPARAM)GetDlgItem(hwndDlg, IDC_TREE1))
			{
				return (LRESULT)GetStockObject(BLACK_BRUSH);
			}
			break;
		case WM_CTLCOLORDLG:
			return (LRESULT)GetStockObject(DKGRAY_BRUSH);
#endif
		case WM_USER + 32:
			if (cur_wnd) return SendMessageW(cur_wnd, uMsg, wParam, lParam);
			return 0;

		case WM_USER + 33:
			// use this to check a specified control on the dialog page (5.59+)
			CheckDlgButton(cur_wnd, wParam, (lParam ? 1: 0));
			return 0;

		case WM_INITDIALOG:
		{
			// this is a bit of a hack to help gen_nopro.dll
			if (!IsWindow((HWND)wParam)) prefs_last_page = 35;

			if (g_safeMode) SetWindowTextW(hwndDlg, getStringW(IDS_PREFS_SAFE_MODE, NULL, 0));
			prefs_hwnd = hwndDlg;
			pluginids = 60;
			HWND hwnd = GetDlgItem(hwndDlg, IDC_TREE1);
			HANDLE h;

			lp_v = NULL;

			h = _additem(hwnd, TVI_ROOT, getStringW(IDS_PREFS_SETUP, NULL, 0), 1, 0);
			_additem(hwnd, h, getStringW(IDS_PREFS_FT, NULL, 0), 0, 1);
			_additem(hwnd, h, getStringW(IDS_PREFS_SHUFFLE, NULL, 0), 0, 23);
			_additem(hwnd, h, getStringW(IDS_PREFS_TITLES, NULL, 0), 0, 21);
			_additem(hwnd, h, getStringW(IDS_PREFS_PLAYBACK, NULL, 0), 0, 42);
			//_additem(hwnd, h, getStringW(IDS_STATIONINFOCAPTION, NULL, 0), 0, 41);
			//insertDialogs(hwnd,4,h,&pluginids);
			//SendMessageW(hwnd,TVM_EXPAND,TVE_EXPAND,(long)h);
			//h=_additem(hwnd,TVI_ROOT,getStringW(IDS_PREFS_OPTIONS,NULL,0),1,20);
			if (g_has_video_plugin || (g_no_video_loaded == 1)) _additem(hwnd, h, getStringW(IDS_PREFS_VIDEO, NULL, 0), 0, 24);

			_additem(hwnd, h, getStringW(IDS_LOCALIZATION, NULL, 0), 0, 25);

			insertDialogs(hwnd, 0, h, &pluginids);

			SendMessageW(hwnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)h);

			//media library
			h = insertRootDialog(hwnd, -6, TVI_ROOT, &pluginids);
			insertDialogs(hwnd, 6, h, &pluginids);
			SendMessageW(hwnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)h);

			insertDialogs(hwnd, -1, TVI_ROOT, &pluginids);   // test

			h = _additem(hwnd, TVI_ROOT, getStringW(IDS_PREFS_SKIN, NULL, 0), 1, 40);
			_additem(hwnd, h, getStringW(IDS_PREFS_CLASSICSKIN, NULL, 0), 0, 22);

			insertDialogs(hwnd, 2, h, &pluginids);
			SendMessageW(hwnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)h);

			h = _additem(hwnd, TVI_ROOT, getStringW(IDS_PREFS_PLUG, NULL, 0), 1, 30);
			_additem(hwnd, h, getStringW(IDS_PREFS_PLUG_IN, NULL, 0), 0, 31);
			_additem(hwnd, h, getStringW(IDS_PREFS_PLUG_OUT, NULL, 0), 0, 32);
			if (!g_safeMode)
			{
				_additem(hwnd, h, getStringW(IDS_PREFS_PLUG_VIS, NULL, 0), 0, 33);
				_additem(hwnd, h, getStringW(IDS_PREFS_PLUG_DSP, NULL, 0), 0, 34);
			}
			_additem(hwnd, h, getStringW(IDS_PREFS_PLUG_GEN, NULL, 0), 0, 35);
			insertDialogs(hwnd, 1, h, &pluginids);

			insertDialogs(hwnd, -2, TVI_ROOT, &pluginids);   // used to force gen_crasher to the bottom (below plug-ins)

			SendMessageW(hwnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)h);
			//h=_additem(hwnd,TVI_ROOT,getStringW(IDS_PREFS_BOOK,NULL,0),1,50);
			//insertDialogs(hwnd,3,h,&pluginids);
			//SendMessageW(hwnd,TVM_EXPAND,TVE_EXPAND,(long)h);

			SendMessageW(hwnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)lp_v);
			PostMessageW(hwnd, TVM_SELECTITEM, TVGN_FIRSTVISIBLE, (LPARAM)lp_v);
			DirectMouseWheel_EnableConvertToMouseWheel(hwnd, TRUE);
#ifdef DO_COLORS
			SendMessageW(hwnd, TVM_SETBKCOLOR, 0, RGB(0, 0, 0));
			SendMessageW(hwnd, TVM_SETTEXTCOLOR, 0, RGB(0, 220, 0));
#endif
		}
			if (NULL != WASABI_API_APP) 
				WASABI_API_APP->app_registerGlobalWindow(hwndDlg);
			
			prevActiveWindow = GetActiveWindow();
			if (NULL != prevActiveWindow && 
				hMainWindow != GetAncestor(prevActiveWindow, GA_ROOTOWNER))
			{
				prevActiveWindow = NULL;
			}

			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
				case IDOK:
					DestroyWindow(hwndDlg);
					return FALSE;
			}
			break;
		case WM_NOTIFY:
		{
			NM_TREEVIEW *p;
			p = (NM_TREEVIEW *)lParam;

			if (p->hdr.code == TVN_SELCHANGEDW)
			{
				HANDLE hTreeItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE1));
				TV_ITEM i = {TVIF_HANDLE, (HTREEITEM)hTreeItem, 0, 0, 0, 0, 0};
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE1), &i);
				if (i.lParam >= 1024)
				{}
				else if (i.lParam >= 256)
				{}
				else
				{
					int id = -1;
					DLGPROC proc = NULL;
					HINSTANCE hinst = NULL;
					prefs_last_page = i.lParam;
					LPARAM param = 0;
					switch (i.lParam)
					{
						case 0: id = IDD_NEWSETUP; proc = SetupProc; break; // general
						case 1: id = (!IsWin8() ? IDD_NEWFTYPES : IDD_WIN8_FTYPES);	proc = FtypeProc; break; // filetypes
//						case 2: id=IDD_NEWAGENT; proc=AgentProc; break;
						case 21: id = IDD_NEWTITLE; proc = TitleProc; break;
						case 22: id = IDD_PREFS_CLASSICSKIN; proc = classicSkinProc; break;
						case 23: id = IDD_NEWSHUFFLEOPTS; proc = PlaybackOptionsProc; break; // playlist
						case 24: id = IDD_NEWVIDEOOPTS; proc = VideoProc; break;
						case 25: id = IDD_NEWLANG; proc = LangProc; break;
						case 30: id = IDD_NEWPLUG; proc = PlugProc; break;
						case 31: id = IDD_NEWINPUT; proc = InputProc; break;
						case 32: id = IDD_NEWOUTPUT; proc = OutputProc; break;
						case 33: id = IDD_NEWVIS; proc = VisProc; break;
						case 34: id = IDD_NEWDSP; proc = DspProc; break;
						case 35: id = IDD_NEWGEN; proc = GenProc; break;
						case 40: id = IDD_NEWSKIN; proc = SkinProc; break;
//						case 41: id = IDD_STATIONINFO; proc = StationInfoProc; break;
						case 42: id = IDD_PREFS_CLASSICSKIN; proc = PlaybackProc; break;
//						case 50: id=IDD_NEWBOOKMARKS; proc = BookProc; break;
						default:
						{
							size_t m;
							for (m = 0; m != g_piprefsdlgs.size(); m++)
							{
								if (g_piprefsdlgs[m]->_id == i.lParam) 
									break;
							}

							if (m != g_piprefsdlgs.size())
							{
								id = g_piprefsdlgs[m]->dlgID;
								hinst = g_piprefsdlgs[m]->hInst;
								proc = (DLGPROC)g_piprefsdlgs[m]->proc;
								param = (LPARAM)g_piprefsdlgs[m];
							}
						}
					}

					if (IsWindow(cur_wnd))
					{
						DestroyWindow(cur_wnd);
						cur_wnd = 0;
					}

					if (id != -1)
					{
						RECT r;
						if (!hinst) cur_wnd = LPCreateDialogW(id, hwndDlg, (WNDPROC)proc);
						else cur_wnd = CreateDialogParamW(hinst, MAKEINTRESOURCEW(id), hwndDlg, proc, param);

						extern int prev_wlz_ex_state;
						if (!IsWindow(cur_wnd) && prev_wlz_ex_state)
						{
							// will attempt to find a in-dll version of the resource
							// if the version from the language pack was not loaded
							wchar_t filename[MAX_PATH] = {0};
							if (GetModuleFileNameW(hinst, filename, MAX_PATH))
							{
								PathRemoveExtensionW(filename);
								PathAddExtensionW(filename, L".dll");
								wchar_t *f = scanstr_backW(filename, L"\\/", filename);
								if (f && *f)
								{
									if (*f == '\\' || *f == '/') f = CharNextW(f);
								}
								if (f && *f)
								{
									HINSTANCE fallback = GetModuleHandleW(f);
									if (fallback != GetModuleHandle(NULL))
									{
										cur_wnd = CreateDialogParamW(fallback, MAKEINTRESOURCEW(id), hwndDlg, proc, param);
									}
								}
							}
						}

						// if we get here then show a non-localised fallback page
						if (!IsWindow(cur_wnd))
						{
							cur_wnd = CreateDialogW(hMainInstance, MAKEINTRESOURCEW(IDD_PREFS_FAIL), hwndDlg, 0);
						}

						GetWindowRect(GetDlgItem(hwndDlg, IDC_RECT), &r);
						ScreenToClient(hwndDlg, (LPPOINT)&r);
						SetWindowPos(cur_wnd, 0, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
						ShowWindow(cur_wnd, SW_SHOWNA);
					}
				}
			}
		}
			break;
		case WM_DESTROY:
			if (NULL != WASABI_API_APP) 
				WASABI_API_APP->app_unregisterGlobalWindow(hwndDlg);

			{
				HWND treeWindow;
				treeWindow = GetDlgItem(hwndDlg, IDC_TREE1);
				if (NULL != treeWindow)
					DirectMouseWheel_EnableConvertToMouseWheel(treeWindow, FALSE);
			}

			GetWindowRect(hwndDlg, &prefs_rect);
			if (cur_wnd) DestroyWindow(cur_wnd);
			cur_wnd = 0;
			//prefs_hwnd = NULL;
			CheckMenuItem(main_menu, WINAMP_OPTIONS_PREFS, MF_UNCHECKED);
			PostMessageW(hMainWindow, WM_WA_IPC, 0, IPC_WRITECONFIG);
			
			if (NULL != prevActiveWindow)
			{
				if (IsWindowVisible(prevActiveWindow) &&
					IsWindowEnabled(prevActiveWindow))
				{
					SetActiveWindow(prevActiveWindow);
				}
				prevActiveWindow = NULL;
			}

			break;
		case WM_ACTIVATE:
			if (NULL != WASABI_API_APP)
			{
				if (WA_INACTIVE == LOWORD(wParam))
					WASABI_API_APP->ActiveDialog_Unregister(hwndDlg);
				else
					WASABI_API_APP->ActiveDialog_Register(hwndDlg);
			}
			break;
	}
	return FALSE;
}

HTREEITEM FindParameter(HWND treeHWND, HTREEITEM node, int findParameter)
{
	if (!node) 
		return 0;

	/* first, see if the passed node matches */
	TVITEM pv;
	pv.mask = TVIF_HANDLE | TVIF_PARAM;
	pv.hItem = node;
	if (TreeView_GetItem(treeHWND, &pv)
		&& pv.lParam == findParameter)
			return node;

	/* now do breadth-first search */
	HTREEITEM found = NULL;
	
	/* search next sibling */
	found = FindParameter(treeHWND, TreeView_GetNextSibling(treeHWND, node), findParameter);
		
	/* search the first child
	 * the child's sibling search should help the rest of this node's children ...  */
	if (!found)
		found = FindParameter(treeHWND, TreeView_GetChild(treeHWND, node), findParameter);
	
	return found;
}

void prefs_dialog(int modal)
{
	if (IsWindow(prefs_hwnd) && modal)
	{
		HTREEITEM hti;
		HTREEITEM lp_v = NULL;
		HWND hwndTV = GetDlgItem(prefs_hwnd, IDC_TREE1);

		//CT> update prefs_last_page to the correct value if its a plugin pref
		for ( prefsDlgRec *p : g_piprefsdlgs )
		{
			if ( (intptr_t)p == prefs_last_page )
			{
				prefs_last_page = p->_id;
				break;
			}
		}

		//hti=TreeView_GetFirstVisible(hwndTV);
		hti = TreeView_GetRoot(hwndTV);
		if (hti)
			lp_v = FindParameter(hwndTV, hti, prefs_last_page);

		if (lp_v) SendMessageW(hwndTV, TVM_SELECTITEM, TVGN_CARET, (LPARAM)lp_v);
		SetForegroundWindow(prefs_hwnd);
		return ;
	}
	if (IsWindow(prefs_hwnd))
	{
		SendMessageW(prefs_hwnd, WM_COMMAND, IDOK, 0);
		return ;
	}
	CheckMenuItem(main_menu, WINAMP_OPTIONS_PREFS, MF_CHECKED);
	prefs_hwnd = (HWND)LPCreateDialogW(IDD_NEWPREFS, DIALOG_PARENT(hMainWindow), OuterProc);
	// show prefs window and restore last position as applicable
	POINT pt = {prefs_rect.left, prefs_rect.top};
	if (!windowOffScreen(prefs_hwnd, pt))
		SetWindowPos(prefs_hwnd, HWND_TOP, prefs_rect.left, prefs_rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
	else
		ShowWindow(prefs_hwnd, SW_SHOW);
}