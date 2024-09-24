#include "../Winamp/buildType.h"
#include "api__ml_pmp.h"
#include "main.h"
#include "../ml_wire/ifc_podcast.h"
#include "DeviceView.h"
#include "nu/AutoWide.h"
#include "transcoder_imp.h"
#include "nu/listview.h"
#include "nu/AutoLock.h"
#include <Commctrl.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <api/syscb/callbacks/browsercb.h>
#if defined (_WIN64)
	#include "../Elevator/IFileTypeRegistrar_64.h"
#else
	#include "../Elevator/IFileTypeRegistrar_32.h"
#endif

extern winampMediaLibraryPlugin plugin;
extern DeviceView * currentViewedDevice;
extern C_ItemList m_plugins;
extern C_Config * global_config;
extern C_ItemList devices;
extern HWND hwndMediaView;

static void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void link_startsubclass(HWND hwndDlg, UINT id);

INT_PTR CALLBACK global_config_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK config_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK config_dlgproc_sync(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK config_dlgproc_cloud_sync(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK config_dlgproc_podcast_sync(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK config_dlgproc_autofill(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK config_dlgproc_transcode(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK config_dlgproc_mediaview(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK config_dlgproc_cloud_mediaview(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK config_dlgproc_plugins(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

void myOpenURLWithFallback(HWND hwnd, wchar_t *loc, wchar_t *fallbackLoc);

HWND m_hwndTab = NULL, m_hwndTabDisplay = NULL;
int g_prefs_openpage=0;
static C_Config * config;
DeviceView * configDevice;

enum {
	SYNC_PREF_IDX = 0,
	PODSYNC_PREF_IDX,
	AUTOFILL_PREF_IDX,
	TRANS_PREF_IDX,
	MEDIAVIEW_PREF_IDX
};
static pref_tab tabs[] = {
	{L"",IDD_CONFIG_SYNC,config_dlgproc_sync,0},
	{L"",IDD_CONFIG_PODCAST_SYNC,config_dlgproc_podcast_sync,0},
	{L"",IDD_CONFIG_AUTOFILL,config_dlgproc_autofill,0},
	{L"",IDD_CONFIG_TRANSCODE,config_dlgproc_transcode,0},
	{L"",IDD_CONFIG_MEDIAVIEW,config_dlgproc_mediaview,0},
	{L"",0,0,0}, // extra config for plugins
};

static void config_tab_init(HWND tab,HWND m_hwndDlg)
{
	RECT r;
	GetWindowRect(m_hwndTab,&r);
	TabCtrl_AdjustRect(m_hwndTab,FALSE,&r);
	MapWindowPoints(NULL,m_hwndDlg,(LPPOINT)&r,2);
	SetWindowPos(tab,HWND_TOP,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE);
	if(!SendMessage(plugin.hwndWinampParent,WM_WA_IPC,IPC_ISWINTHEMEPRESENT,IPC_USE_UXTHEME_FUNC))
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)tab,IPC_USE_UXTHEME_FUNC);
}

HWND OnSelChanged(HWND hwndDlg, HWND external = NULL, DeviceView *dev = NULL)
{
	static prefsParam p;
	int sel=(!external ? TabCtrl_GetCurSel(m_hwndTab) : MEDIAVIEW_PREF_IDX);
	if (!external)
	{
		g_prefs_openpage=sel;
		if(m_hwndTabDisplay!=NULL)
			DestroyWindow(m_hwndTabDisplay);
		p.config_tab_init = config_tab_init;
		p.dev = configDevice->dev;
		p.parent = hwndDlg;

		// copes with the Transcoding tab being hidden
		if (sel >= TRANS_PREF_IDX && tabs[TRANS_PREF_IDX].title[0] == 0) sel++;
	}
	else
	{
		configDevice = dev;
		config = dev->config;
		p.config_tab_init = config_tab_init;
		p.dev = dev->dev;
		p.parent = hwndDlg;
	}

	// copes with a cloud device which doesn't need podcast sync and autofill
	// as well as factoring in the transcoding page needing to be hidden, etc
	if (configDevice->isCloudDevice)
	{
		if (!external && (sel >= SYNC_PREF_IDX)) sel += (3 + (tabs[TRANS_PREF_IDX].title[0] == 0));
		if (sel == MEDIAVIEW_PREF_IDX)
		{
			tabs[sel].res_id = IDD_CONFIG_CLOUD_MEDIAVIEW;
			tabs[sel].dlg_proc = config_dlgproc_cloud_mediaview;
		}
	}
	else
	{
		if (sel == SYNC_PREF_IDX)
		{
			tabs[sel].res_id = IDD_CONFIG_SYNC;
			tabs[sel].dlg_proc = config_dlgproc_sync;
		}
		if (sel == MEDIAVIEW_PREF_IDX)
		{
			tabs[sel].res_id = IDD_CONFIG_MEDIAVIEW;
			tabs[sel].dlg_proc = config_dlgproc_mediaview;
		}
	}

	if(!tabs[sel].hinst)
		m_hwndTabDisplay=WASABI_API_CREATEDIALOGPARAMW(tabs[sel].res_id, hwndDlg, tabs[sel].dlg_proc, (LPARAM)&p);
	else
		m_hwndTabDisplay=WASABI_API_LNG->CreateLDialogParamW(tabs[sel].hinst, tabs[sel].hinst, tabs[sel].res_id, hwndDlg, tabs[sel].dlg_proc, (LPARAM)&p);

	if (!external)
	{
		RECT r;
		GetClientRect(m_hwndTab, &r);
		TabCtrl_AdjustRect(m_hwndTab, FALSE, &r);
		SetWindowPos(m_hwndTabDisplay, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
	return m_hwndTabDisplay;
}

void Shell_Free(void *p) {
	IMalloc *m = NULL;
	if (SUCCEEDED(SHGetMalloc(&m)) && m)
	{
		m->Free(p);
	}
}

wchar_t* GetDefaultSaveToFolder(wchar_t* path_to_store)
{
	if(FAILED(SHGetFolderPath(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, path_to_store)))
	{
		if(FAILED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path_to_store)))
		{
			// and if that all fails then do a reasonable default
			lstrcpyn(path_to_store, L"C:\\My Music", MAX_PATH);
		}
		// if there's no valid My Music folder (typically win2k) then default to %my_documents%\my music
		else
		{
			PathCombine(path_to_store, path_to_store, L"My Music");
		}
	}
	return path_to_store;
}

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	wchar_t cl[32] = {0};
	GetClassNameW(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiW(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
	{
		wchar_t m_def_extract_path[MAX_PATH] = L"C:\\My Music";
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)global_config->ReadString(L"extractpath", GetDefaultSaveToFolder(m_def_extract_path)));

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	return 0;
}

extern void doFormatFileName(wchar_t out[MAX_PATH], wchar_t *fmt, int trackno, wchar_t *artist, wchar_t *album, wchar_t *title, wchar_t *genre, wchar_t *year, wchar_t *trackartist);

INT_PTR CALLBACK global_config_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	wchar_t m_def_extract_path[MAX_PATH] = L"C:\\My Music";
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			wchar_t m_def_filename_fmt[] = L"<Artist> - <Album>\\## - <Trackartist> - <Title>";
			GetDefaultSaveToFolder(m_def_extract_path);
			if (global_config->ReadInt(L"extractusecdrip", 1)) CheckDlgButton(hwndDlg, IDC_CHECK_USECDRIP, BST_CHECKED);
			SendDlgItemMessage(hwndDlg, IDC_DESTPATH, EM_SETLIMITTEXT, MAX_PATH, 0);
			SetDlgItemText(hwndDlg, IDC_DESTPATH, global_config->ReadString(L"extractpath", m_def_extract_path));
			SetDlgItemText(hwndDlg, IDC_FILENAMEFMT, global_config->ReadString(L"extractfmt2", m_def_filename_fmt));
			SendMessage(hwndDlg,WM_USER,0,0);
		}
		break;
	case WM_USER:
		{
			BOOL enabled = !IsDlgButtonChecked(hwndDlg, IDC_CHECK_USECDRIP);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC_1),enabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_DESTPATH),enabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),enabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC_2),enabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON2),enabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_FILENAMEFMT),enabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_FMTOUT),enabled);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DESTPATH:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				wchar_t buf[1024] = {0};
				GetDlgItemText(hwndDlg, IDC_DESTPATH, buf, 1024);
				global_config->WriteString(L"extractpath", buf);
			}
			return 0;
		case IDC_CHECK_USECDRIP:
			{
				global_config->WriteInt(L"extractusecdrip", !!IsDlgButtonChecked(hwndDlg, IDC_CHECK_USECDRIP));
				SendMessage(hwndDlg,WM_USER,0,0);
			}
			break;
			// run through...
		case IDC_FILENAMEFMT:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				wchar_t buf[1024] = {0};
				GetDlgItemText(hwndDlg, IDC_FILENAMEFMT, buf, 1024);
				if(LOWORD(wParam) == IDC_FILENAMEFMT) global_config->WriteString(L"extractfmt2", buf);
				wchar_t str[MAX_PATH] = {0};
				doFormatFileName(WASABI_API_LNGSTRINGW_BUF(IDS_EXAMPLE_FORMATTING_STRING,str,MAX_PATH),
								 buf, 10, L"U2", L"The Joshua Tree", L"Exit", L"Rock", L"1987", L"U2");

				wchar_t fmt[5]=L"mp3";

				StringCchCat(str, MAX_PATH, L".");
				StringCchCat(str, MAX_PATH, fmt);

				SetDlgItemText(hwndDlg, IDC_FMTOUT, str);
			}
			return 0;
		case IDC_BUTTON1:
			{
				//browse for folder
				BROWSEINFO bi = {0};
				wchar_t name[MAX_PATH] = {0};
				bi.hwndOwner = hwndDlg;
				bi.pidlRoot = NULL;
				bi.pszDisplayName = name;
				bi.lpszTitle = WASABI_API_LNGSTRINGW(IDS_CHOOSE_A_FOLDER);
				bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
				bi.lpfn = BrowseCallbackProc;
				ITEMIDLIST *idlist = SHBrowseForFolder(&bi);
				if (idlist)
				{
					wchar_t path[MAX_PATH] = {0};
					SHGetPathFromIDList( idlist, path );
					Shell_Free(idlist);
					global_config->WriteString(L"extractpath", path);
					SetDlgItemText(hwndDlg, IDC_DESTPATH, global_config->ReadString(L"extractpath", m_def_extract_path));
				}
			}
			break;
		case IDC_BUTTON2:
			wchar_t titleStr[64] = {0};
			MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_COPIED_FILE_FORMAT_INFO),
					   WASABI_API_LNGSTRINGW_BUF(IDS_COPIED_FILE_FORMAT_HELP,titleStr,64),
					   MB_OK);
			break;
		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK config_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			config = global_config;
			DeviceView * dev = NULL;
			for(int i=0; i<devices.GetSize(); i++)
			{
				dev = (DeviceView *)devices.Get(i);
			    if(&dev->devPrefsPage == (prefsDlgRecW *)lParam) { configDevice = dev; config = dev->config; break; }
			}
			if(config == global_config)
			{
				EndDialog(hwndDlg,0);
				return 0;
			}

			// set the prefs page titles at this stage so that we can have localised versions
			int title_ids[] = {IDS_PREFS_SYNC,IDS_PODCAST_SYNC,IDS_PREFS_AUTOFILL,IDS_PREFS_TRANSCODING,IDS_PREFS_VIEW};
			for(int i=0; i<sizeof(title_ids)/sizeof(title_ids[0]); i++)
				WASABI_API_LNGSTRINGW_BUF(title_ids[i],tabs[i].title,(sizeof(tabs[i].title)/sizeof(wchar_t)));

			// set the last item's title to zero otherwise it can cause a dialog to appear incorrectly
			tabs[sizeof(tabs)/sizeof(tabs[0])-1].title[0] = 0;

			m_hwndTab = GetDlgItem(hwndDlg,IDC_TAB1);
			TCITEM tie;
			tie.mask = TCIF_TEXT;
			int num = 0;
			for(int i=0; i<sizeof(tabs)/sizeof(pref_tab); i++)
			{
				if (dev)
				{
					if(tabs[i].title[0] == 0)
					{
						dev->dev->extraActions(DEVICE_GET_PREFS_DIALOG,(intptr_t)&tabs[i],num++,0);
					}

					// check if we need to show the Transcoding page or not
					if (i == TRANS_PREF_IDX && dev->dev->extraActions(DEVICE_VETO_TRANSCODING,0,0,0))
					{
						tabs[i].title[0] = 0;
					}
					else
					{
						// check if a cloud device and drop sync, podcast sync and autofill as appropriate
						if (configDevice->isCloudDevice && (i == SYNC_PREF_IDX || i == PODSYNC_PREF_IDX || i == AUTOFILL_PREF_IDX))
						{
							tabs[i].title[0] = 0;
						}
					}
				}

				if(tabs[i].title[0] != 0)
				{
					tie.pszText=tabs[i].title;
					TabCtrl_InsertItem(m_hwndTab,i,&tie);
				}
			}
			TabCtrl_SetCurSel(m_hwndTab,g_prefs_openpage);
			OnSelChanged(hwndDlg);
		}
		break;

		case WM_NOTIFY:
		{
			LPNMHDR lpn = (LPNMHDR) lParam; 
			if(lpn) if(lpn->code==TCN_SELCHANGE) OnSelChanged(hwndDlg);
		}
		break;

		case WM_DESTROY:
			tabs[4].title[0]=0;
			configDevice = NULL;
		break;
	}
	return 0;
}

static INT_PTR CALLBACK config_dlgproc_sync(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
static W_ListView m_list;
static int nonotif;
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			nonotif=1;
			if(lParam) config_tab_init(hwndDlg,((prefsParam*)lParam)->parent);
			if(config->ReadInt(L"syncOnConnect",configDevice->SyncConnectionDefault)==1) CheckDlgButton(hwndDlg,IDC_SYNCONCONNECT,BST_CHECKED);
			else EnableWindow(GetDlgItem(hwndDlg,IDC_SYNCONCONNECT_TIME),FALSE);
			SetDlgItemText(hwndDlg,IDC_SYNCONCONNECT_TIME,config->ReadString(L"syncOnConnect_hours",L"12"));
			SetDlgItemText(hwndDlg,IDC_SYNC_QUERY_STRING,config->ReadString(L"SyncQuery",L"type=0"));
			if(config->ReadInt(L"plsyncwhitelist",1)) CheckDlgButton(hwndDlg,IDC_PL_WHITELIST,BST_CHECKED);
			else CheckDlgButton(hwndDlg,IDC_PL_BLACKLIST,BST_CHECKED);
			if(config->ReadInt(L"syncAllLibrary",1)) CheckDlgButton(hwndDlg,IDC_LIBRARYSYNC,BST_CHECKED);

			m_list.setwnd(GetDlgItem(hwndDlg,IDC_PL_LIST));      
			ListView_SetExtendedListViewStyle(m_list.getwnd(), LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
			m_list.AddCol(L"",353);
			m_list.SetColumnWidth(0, 200);
			mlPlaylistInfo playlist = {0};
			playlist.size = sizeof(mlPlaylistInfo);
			int playlistsnum = SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,0,ML_IPC_PLAYLIST_COUNT);
			for(int i=0; i<playlistsnum; i++) {
				playlist.playlistNum = i;
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&playlist,ML_IPC_PLAYLIST_INFO);
				m_list.InsertItem(i,playlist.playlistName,0);
			}
			ListView_SetColumnWidth(m_list.getwnd(),0,LVSCW_AUTOSIZE);

			for(int i=0; i<playlistsnum; i++) {
				playlist.playlistNum = i;
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&playlist,ML_IPC_PLAYLIST_INFO);
				wchar_t buf[150] = {0};
				StringCchPrintf(buf,150,L"sync-%s",playlist.playlistName);
				BOOL state = config->ReadInt(buf,0)?TRUE:FALSE;
				ListView_SetCheckState(m_list.getwnd(),i,state);
			}
			nonotif=0;

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_list.getwnd(), TRUE);
		}
		break;

		case WM_DESTROY:
			if (NULL != WASABI_API_APP)
			{
				HWND listWindow;
				listWindow = GetDlgItem(hwndDlg,IDC_PL_LIST);
				if (NULL != listWindow)
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
			}
		break;

		case WM_NOTIFY:
		{
		LPNMHDR l=(LPNMHDR)lParam;
			if (l->idFrom==IDC_PL_LIST && l->code == LVN_ITEMCHANGED && !nonotif) {
			LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
				if(lv->iItem == -1) break;
				wchar_t buf[150] = {0}, buf1[125] = {0};
				m_list.GetText(lv->iItem,0,buf1,125);
				StringCchPrintf(buf,150,L"sync-%s",buf1);
				BOOL state = ListView_GetCheckState(m_list.getwnd(),lv->iItem);
				config->WriteInt(buf,state?1:0);
			}
		}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_SYNCONCONNECT:
					config->WriteInt(L"syncOnConnect",IsDlgButtonChecked(hwndDlg,IDC_SYNCONCONNECT)?1:0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_SYNCONCONNECT_TIME),IsDlgButtonChecked(hwndDlg,IDC_SYNCONCONNECT));
				break;

				case IDC_SYNCONCONNECT_TIME:
					if(HIWORD(wParam) == EN_CHANGE) {
					wchar_t buf[100]=L"";
						GetDlgItemText(hwndDlg,IDC_SYNCONCONNECT_TIME,buf,100);
						config->WriteInt(L"syncOnConnect_hours",_wtoi(buf));
					}
				break;

				case IDC_PL_WHITELIST:
				case IDC_PL_BLACKLIST:
					config->WriteInt(L"plsyncwhitelist",IsDlgButtonChecked(hwndDlg,IDC_PL_WHITELIST)?1:0);
				break;

				case IDC_LIBRARYSYNC:
					config->WriteInt(L"syncAllLibrary",IsDlgButtonChecked(hwndDlg,IDC_LIBRARYSYNC)?1:0);
				break;

				case IDC_SYNC_QUERY_STRING:
					if (HIWORD(wParam) == EN_KILLFOCUS) {
					wchar_t buf[1024] = {0};
						GetDlgItemText(hwndDlg,IDC_SYNC_QUERY_STRING,buf,1024);
						config->WriteString(L"SyncQuery",buf);
					}
				break;

				case IDC_SYNC_QUERY_EDIT:
				{
					char temp[1024] = {0};
					GetDlgItemTextA(hwndDlg, IDC_SYNC_QUERY_STRING, temp, sizeof(temp) - 1);
					ml_editview meq = {hwndDlg,temp,"Sync Query",-1};
					meq.name = WASABI_API_LNGSTRING(IDS_SYNC_QUERY);
					if(!(int)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(LPARAM)&meq,ML_IPC_EDITVIEW)) return 0;
					SetDlgItemTextA(hwndDlg, IDC_SYNC_QUERY_STRING, meq.query); 
					config->WriteString(L"SyncQuery",AutoWide(meq.query));
				}
				break;
			}
		break;
	}
	return 0;
}

static INT_PTR CALLBACK config_dlgproc_cloud_sync(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
static W_ListView m_list;
static int nonotif;
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			nonotif=1;
			if(lParam) config_tab_init(hwndDlg,((prefsParam*)lParam)->parent);
			if(config->ReadInt(L"syncOnConnect",configDevice->SyncConnectionDefault)==1) CheckDlgButton(hwndDlg,IDC_SYNCONCONNECT,BST_CHECKED);
			else EnableWindow(GetDlgItem(hwndDlg,IDC_SYNCONCONNECT_TIME),FALSE);
			SetDlgItemText(hwndDlg,IDC_SYNCONCONNECT_TIME,config->ReadString(L"syncOnConnect_hours",L"12"));
			SetDlgItemText(hwndDlg,IDC_SYNC_QUERY_STRING,config->ReadString(L"SyncQuery",L"type=0"));
			if(config->ReadInt(L"plsyncwhitelist",1)) CheckDlgButton(hwndDlg,IDC_PL_WHITELIST,BST_CHECKED);
			else CheckDlgButton(hwndDlg,IDC_PL_BLACKLIST,BST_CHECKED);
			if(config->ReadInt(L"syncAllLibrary",1)) CheckDlgButton(hwndDlg,IDC_LIBRARYSYNC,BST_CHECKED);

			m_list.setwnd(GetDlgItem(hwndDlg,IDC_PL_LIST));      
			ListView_SetExtendedListViewStyle(m_list.getwnd(), LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
			m_list.AddCol(L"",353);
			mlPlaylistInfo playlist = {0};
			playlist.size = sizeof(mlPlaylistInfo);
			int playlistsnum = SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,0,ML_IPC_PLAYLIST_COUNT);
			for(int i=0; i<playlistsnum; i++) {
				playlist.playlistNum = i;
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&playlist,ML_IPC_PLAYLIST_INFO);
				m_list.InsertItem(i,playlist.playlistName,0);
			}
			ListView_SetColumnWidth(m_list.getwnd(),0,LVSCW_AUTOSIZE);

			for(int i=0; i<playlistsnum; i++) {
				playlist.playlistNum = i;
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&playlist,ML_IPC_PLAYLIST_INFO);
				wchar_t buf[150] = {0};
				StringCchPrintf(buf,150,L"sync-%s",playlist.playlistName);
				BOOL state = config->ReadInt(buf,0)?TRUE:FALSE;
				ListView_SetCheckState(m_list.getwnd(),i,state);
			}
			nonotif=0;

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_list.getwnd(), TRUE);
		}
		break;

		case WM_DESTROY:
			if (NULL != WASABI_API_APP)
			{
				HWND listWindow;
				listWindow = GetDlgItem(hwndDlg,IDC_PL_LIST);
				if (NULL != listWindow)
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
			}
		break;

		case WM_NOTIFY:
		{
		LPNMHDR l=(LPNMHDR)lParam;
			if (l->idFrom==IDC_PL_LIST && l->code == LVN_ITEMCHANGED && !nonotif) {
			LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
				if(lv->iItem == -1) break;
				wchar_t buf[150] = {0}, buf1[125] = {0};
				m_list.GetText(lv->iItem,0,buf1,125);
				StringCchPrintf(buf,150,L"sync-%s",buf1);
				BOOL state = ListView_GetCheckState(m_list.getwnd(),lv->iItem);
				config->WriteInt(buf,state?1:0);
			}
		}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_SYNCONCONNECT:
					config->WriteInt(L"syncOnConnect",IsDlgButtonChecked(hwndDlg,IDC_SYNCONCONNECT)?1:0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_SYNCONCONNECT_TIME),IsDlgButtonChecked(hwndDlg,IDC_SYNCONCONNECT));
				break;

				case IDC_SYNCONCONNECT_TIME:
					if(HIWORD(wParam) == EN_CHANGE) {
					wchar_t buf[100]=L"";
						GetDlgItemText(hwndDlg,IDC_SYNCONCONNECT_TIME,buf,100);
						config->WriteInt(L"syncOnConnect_hours",_wtoi(buf));
					}
				break;

				case IDC_PL_WHITELIST:
				case IDC_PL_BLACKLIST:
					config->WriteInt(L"plsyncwhitelist",IsDlgButtonChecked(hwndDlg,IDC_PL_WHITELIST)?1:0);
				break;

				case IDC_LIBRARYSYNC:
					config->WriteInt(L"syncAllLibrary",IsDlgButtonChecked(hwndDlg,IDC_LIBRARYSYNC)?1:0);
				break;

				case IDC_SYNC_QUERY_STRING:
					if (HIWORD(wParam) == EN_KILLFOCUS) {
					wchar_t buf[1024] = {0};
						GetDlgItemText(hwndDlg,IDC_SYNC_QUERY_STRING,buf,1024);
						config->WriteString(L"SyncQuery",buf);
					}
				break;

				case IDC_SYNC_QUERY_EDIT:
				{
					char temp[1024] = {0};
					GetDlgItemTextA(hwndDlg, IDC_SYNC_QUERY_STRING, temp, sizeof(temp) - 1);
					ml_editview meq = {hwndDlg,temp,"Sync Query",-1};
					meq.name = WASABI_API_LNGSTRING(IDS_SYNC_QUERY);
					if(!(int)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(LPARAM)&meq,ML_IPC_EDITVIEW)) return 0;
					SetDlgItemTextA(hwndDlg, IDC_SYNC_QUERY_STRING, meq.query); 
					config->WriteString(L"SyncQuery",AutoWide(meq.query));
				}
				break;
			}
		break;
	}
	return 0;
}

static INT_PTR CALLBACK config_dlgproc_podcast_sync(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
static W_ListView m_list2;
static int nonotif;
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			nonotif=1;
			if(lParam) config_tab_init(hwndDlg,((prefsParam*)lParam)->parent);

			m_list2.setwnd(GetDlgItem(hwndDlg,IDC_PC_LIST));
			ListView_SetExtendedListViewStyle(m_list2.getwnd(), LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
			m_list2.AddCol(L"",215);

			if (configDevice->dev->extraActions(DEVICE_SUPPORTS_PODCASTS, 0,0, 0) == 0) {
			int podcastsnum = AGAVE_API_PODCASTS?AGAVE_API_PODCASTS->GetNumPodcasts():0;
				for(int i=0; i<podcastsnum; i++) {
				ifc_podcast *podcast = AGAVE_API_PODCASTS->EnumPodcast(i);
					if (podcast) {
					wchar_t podcast_name[256] = {0};
						if (podcast->GetTitle(podcast_name, 256) == 0 && podcast_name[0])
						{
							m_list2.InsertItem(i,podcast_name,0);
						}
					}
				}
				ListView_SetColumnWidth(m_list2.getwnd(),0,LVSCW_AUTOSIZE);

				for(int i=0; i<podcastsnum; i++) {
				ifc_podcast *podcast = AGAVE_API_PODCASTS->EnumPodcast(i);
					if (podcast) {
					wchar_t podcast_name[256] = {0};
						if (podcast->GetTitle(podcast_name, 256) == 0 && podcast_name[0]) {
						wchar_t buf[300] = {0};
							StringCchPrintf(buf,300,L"podcast-sync-%s",podcast_name);
							BOOL state = config->ReadInt(buf,0)?TRUE:FALSE;
							ListView_SetCheckState(m_list2.getwnd(),i,state);
						}
					}
				}

				int podcast_eps = config->ReadInt(L"podcast-sync_episodes",0);
				if(podcast_eps == 0) {
					podcast_eps=3;
				} else
					CheckDlgButton(hwndDlg,IDC_CHECK_PC,BST_CHECKED);

				for(int i=0; i<6; i++) {
					int a,d;
					if(i==0) {
						a = SendDlgItemMessage(hwndDlg,IDC_COMBO_PC_NUM,CB_ADDSTRING,0,
											   (LPARAM)WASABI_API_LNGSTRINGW(IDS_ALL));
						d = -1;
					} else {
						wchar_t buf[10] = {0};
						StringCchPrintf(buf,100,L"%d",i);
						a = SendDlgItemMessage(hwndDlg,IDC_COMBO_PC_NUM,CB_ADDSTRING,0,(LPARAM)buf);
						d = i;
					}
					SendDlgItemMessage(hwndDlg,IDC_COMBO_PC_NUM,CB_SETITEMDATA,a,(LPARAM)d);
					if(d == podcast_eps) SendDlgItemMessage(hwndDlg, IDC_COMBO_PC_NUM, CB_SETCURSEL, (WPARAM)a, 0);
				}

				if(config->ReadInt(L"podcast-sync_all",1)) CheckDlgButton(hwndDlg,IDC_PC_ALL,BST_CHECKED);
				else CheckDlgButton(hwndDlg,IDC_PC_SEL,BST_CHECKED);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_PC), FALSE);
			}
			SendMessage(hwndDlg,WM_COMMAND,IDC_CHECK_PC,0xdeadbeef);
			nonotif=0;

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_list2.getwnd(), TRUE);
		}
		break;

		case WM_DESTROY:
			if (NULL != WASABI_API_APP)
			{
				HWND listWindow;
				listWindow = GetDlgItem(hwndDlg,IDC_PL_LIST);
				if (NULL != listWindow)
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
			}
			break;

		case WM_NOTIFY:
		{
		LPNMHDR l=(LPNMHDR)lParam;
			if (l->idFrom==IDC_PC_LIST && l->code == LVN_ITEMCHANGED && !nonotif) {
			LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
		        if(lv->iItem == -1) break;
				wchar_t buf[150] = {0}, buf1[125] = {0};
				m_list2.GetText(lv->iItem,0,buf1,125);
				StringCchPrintf(buf,150,L"podcast-sync-%s",buf1);
				BOOL state = ListView_GetCheckState(m_list2.getwnd(),lv->iItem);
				config->WriteInt(buf,state?1:0);
			}
		}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_CHECK_PC:
				{
					BOOL e = IsDlgButtonChecked(hwndDlg,IDC_CHECK_PC);
					EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO_PC_NUM),e);
					EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC_PODTXT),e);
					EnableWindow(GetDlgItem(hwndDlg,IDC_PC_ALL),e);
					EnableWindow(GetDlgItem(hwndDlg,IDC_PC_SEL),e);
					EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC_PODTIP),e);
					EnableWindow(GetDlgItem(hwndDlg,IDC_PC_LIST),e && IsDlgButtonChecked(hwndDlg,IDC_PC_SEL));
					if(lParam != 0xdeadbeef) {
						if(e) {
							int b = SendDlgItemMessage(hwndDlg, IDC_COMBO_PC_NUM, CB_GETCURSEL, 0, 0);
							if(b>=0) {
								int x = SendDlgItemMessage(hwndDlg, IDC_COMBO_PC_NUM, CB_GETITEMDATA, b, 0);
								config->WriteInt(L"podcast-sync_episodes", x);
							}
						}
						else config->WriteInt(L"podcast-sync_episodes",0);
					}
				}
				break;

				case IDC_PC_SEL:
				case IDC_PC_ALL:
				{
					BOOL e = IsDlgButtonChecked(hwndDlg,IDC_PC_SEL);
					EnableWindow(GetDlgItem(hwndDlg,IDC_PC_LIST),e);
					config->WriteInt(L"podcast-sync_all",!e);
				}
				break;

				case IDC_COMBO_PC_NUM:
					if(HIWORD(wParam) == CBN_SELCHANGE) {
					int b = SendDlgItemMessage(hwndDlg, IDC_COMBO_PC_NUM, CB_GETCURSEL, 0, 0);
					if (b >= 0) {
						int x = SendDlgItemMessage(hwndDlg, IDC_COMBO_PC_NUM, CB_GETITEMDATA, b, 0);
						config->WriteInt(L"podcast-sync_episodes", x);
					}
				}
				break;
			}
		break;
	}
	return 0;
}

static INT_PTR CALLBACK config_dlgproc_autofill(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg)
	{
		case WM_INITDIALOG:
			if(lParam) config_tab_init(hwndDlg,((prefsParam*)lParam)->parent);
			if(config->ReadInt(L"syncOnConnect",configDevice->SyncConnectionDefault)==2) CheckDlgButton(hwndDlg,IDC_SYNCONCONNECT,BST_CHECKED);
			else EnableWindow(GetDlgItem(hwndDlg,IDC_SYNCONCONNECT_TIME),FALSE);
			SetDlgItemText(hwndDlg,IDC_SYNCONCONNECT_TIME,config->ReadString(L"syncOnConnect_hours",L"12"));
			if(lstrlen(config->ReadString(L"AutoFillRatings",L""))!=0) CheckDlgButton(hwndDlg,IDC_BOOSTRATINGS,BST_CHECKED);
			if(config->ReadInt(L"AlbumAutoFill",0)) CheckDlgButton(hwndDlg,IDC_AUTOFILLALBUMS,BST_CHECKED);
			SetDlgItemText(hwndDlg,IDC_AUTOFILL_QUERY_STRING,config->ReadString(L"AutoFillQuery",L"length > 30"));
			SendMessage(GetDlgItem(hwndDlg,IDC_SPACESLIDER),TBM_SETRANGEMAX, TRUE, 100);
			SendMessage(GetDlgItem(hwndDlg,IDC_SPACESLIDER),TBM_SETRANGEMIN, TRUE, 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_SPACESLIDER),TBM_SETPOS, TRUE, config->ReadInt(L"FillPercent",90));
			break;
		case WM_NOTIFY:
			switch (LOWORD(wParam))
			{
				case IDC_SPACESLIDER:
					{
						int spaceToAutofill = SendMessage(GetDlgItem(hwndDlg,IDC_SPACESLIDER),TBM_GETPOS,0,0);
						wchar_t tmp[100]=L"";
						StringCchPrintf(tmp,100,WASABI_API_LNGSTRINGW(IDS_AIM_TO_AUTOFILL_DEVICE),spaceToAutofill);
						SetDlgItemText(hwndDlg, IDC_FILLCAPTION, tmp);
						config->WriteInt(L"FillPercent",spaceToAutofill);
					}
				break;
			}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_SYNCONCONNECT:
					config->WriteInt(L"syncOnConnect",IsDlgButtonChecked(hwndDlg,IDC_SYNCONCONNECT)?2:0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_SYNCONCONNECT_TIME),IsDlgButtonChecked(hwndDlg,IDC_SYNCONCONNECT));
					break;
				case IDC_SYNCONCONNECT_TIME:
					if(HIWORD(wParam) == EN_CHANGE)
					{
						wchar_t buf[100]=L"";
						GetDlgItemText(hwndDlg,IDC_SYNCONCONNECT_TIME,buf,100);
						config->WriteInt(L"syncOnConnect_hours",_wtoi(buf));
					}
					break;
				case IDC_AUTOFILL_QUERY_STRING:
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						wchar_t buf[1024] = {0};
						GetDlgItemText(hwndDlg,IDC_AUTOFILL_QUERY_STRING,buf,1024);
						config->WriteString(L"AutoFillQuery",buf);
					}
					break;
				case IDC_AUTOFILL_QUERY_EDIT:
					{
						char temp[1024] = {0};
						GetDlgItemTextA(hwndDlg, IDC_AUTOFILL_QUERY_STRING, temp, sizeof(temp) - 1);
						ml_editview meq = {hwndDlg,temp,0,-1};
						meq.name = WASABI_API_LNGSTRING(IDS_AUTOFILL_QUERY);
						if(!(int)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(LPARAM)&meq,ML_IPC_EDITVIEW)) return 0;
						SetDlgItemTextA(hwndDlg, IDC_AUTOFILL_QUERY_STRING, meq.query); 
						config->WriteString(L"AutoFillQuery",AutoWide(meq.query));
					}
					break;
				case IDC_BOOSTRATINGS:
					if(IsDlgButtonChecked(hwndDlg,IDC_BOOSTRATINGS)) config->WriteString(L"AutoFillRatings",L"20:18:15:13:11:10");
					else config->WriteString(L"AutoFillRatings",L"");
					break;
				case IDC_AUTOFILLALBUMS:
					config->WriteInt(L"AlbumAutoFill",IsDlgButtonChecked(hwndDlg,IDC_AUTOFILLALBUMS)?1:0);
					break;
			}
			break;
	}

	const int controls[] = 
	{
		IDC_SPACESLIDER,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
}

static INT_PTR CALLBACK config_dlgproc_transcode(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				Device *dev = 0;
				if(lParam) 
				{
					config_tab_init(hwndDlg,((prefsParam*)lParam)->parent);
					dev = ((prefsParam*)lParam)->dev;

				}
				lParam = (LPARAM)TranscoderImp::ConfigureTranscoder(L"ml_pmp",plugin.hwndWinampParent,config, dev);
			}
			break;
	}
	return TranscoderImp::transcodeconfig_dlgproc(hwndDlg,uMsg,wParam,lParam);
}

static INT_PTR CALLBACK config_dlgproc_mediaview(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
			{
				if(lParam) config_tab_init(hwndDlg,((prefsParam*)lParam)->parent);
				int fieldsBits = (int)configDevice->dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
				if(!fieldsBits) fieldsBits=-1;
				// used to ensure that the filters will work irrespective of language in use
				wchar_t * filters[] = {
					(fieldsBits&SUPPORTS_ARTIST)?L"Artist":0,
					(fieldsBits&SUPPORTS_ALBUM)?L"Album":0,
					(fieldsBits&SUPPORTS_GENRE)?L"Genre":0,
					(fieldsBits&SUPPORTS_YEAR)?L"Year":0,
					(fieldsBits&SUPPORTS_ALBUMARTIST)?L"Album Artist":0,
					(fieldsBits&SUPPORTS_PUBLISHER)?L"Publisher":0,
					(fieldsBits&SUPPORTS_COMPOSER)?L"Composer":0,
					(fieldsBits&SUPPORTS_ARTIST)?L"Artist Index":0,
					(fieldsBits&SUPPORTS_ALBUMARTIST)?L"Album Artist Index":0,
					(fieldsBits&SUPPORTS_ALBUMART)?L"Album Art":0,
					(fieldsBits&SUPPORTS_MIMETYPE)?L"Mime":0,
					(fieldsBits&SUPPORTS_DATEADDED)?L"Date Added":0,
				};
				// used for displayed items - localised crazyness, heh
				int filters_idx[] = {
					(fieldsBits&SUPPORTS_ARTIST)?IDS_ARTIST:0,
					(fieldsBits&SUPPORTS_ALBUM)?IDS_ALBUM:0,
					(fieldsBits&SUPPORTS_GENRE)?IDS_GENRE:0,
					(fieldsBits&SUPPORTS_YEAR)?IDS_YEAR:0,
					(fieldsBits&SUPPORTS_ALBUMARTIST)?IDS_ALBUM_ARTIST:0,
					(fieldsBits&SUPPORTS_PUBLISHER)?IDS_PUBLISHER:0,
					(fieldsBits&SUPPORTS_COMPOSER)?IDS_COMPOSER:0,
					(fieldsBits&SUPPORTS_ARTIST)?IDS_ARTIST_INDEX:0,
					(fieldsBits&SUPPORTS_ALBUMARTIST)?IDS_ALBUM_ARTIST_INDEX:0,
					(fieldsBits&SUPPORTS_ALBUMART)?IDS_ALBUM_ART:0,
					(fieldsBits&SUPPORTS_MIMETYPE)?IDS_MIME_TYPE:0,
					(fieldsBits&SUPPORTS_DATEADDED)?IDS_DATE_ADDED:0,
				};
				int numFilters = config->ReadInt(L"media_numfilters",2);
				CheckRadioButton(hwndDlg, IDC_RADIO_FILTERS1, IDC_RADIO_FILTERS3, (IDC_RADIO_FILTERS1 + numFilters - 1));

				CheckDlgButton(hwndDlg, IDC_REMEMBER_SEARCH, config->ReadInt(L"savefilter", 1));

				for(int i=0; i<3; i++) {
					int id = (i==0)?IDC_COMBO_FILTER1:((i==1)?IDC_COMBO_FILTER2:IDC_COMBO_FILTER3);
					for(int j=0; j<(sizeof(filters)/sizeof(wchar_t*)); j++) {
						if(filters[j]) {
							int a = SendDlgItemMessage(hwndDlg,id,CB_ADDSTRING,0,
									(LPARAM)WASABI_API_LNGSTRINGW(filters_idx[j]));
							SendDlgItemMessage(hwndDlg,id,CB_SETITEMDATA,a,(LPARAM)filters[j]);
						}
					}

					wchar_t name[20] = {0};
					StringCchPrintf(name,20,L"media_filter%d",i);
					extern wchar_t *GetDefFilter(int i,int n);
					SendDlgItemMessage(hwndDlg,id,CB_SETCURSEL,0,0);
					wchar_t* filterStr = config->ReadString(name,GetDefFilter(i,numFilters));
					for(int l = 0; l < (sizeof(filters)/sizeof(wchar_t*)); l++) {
						wchar_t* x = (wchar_t*)SendDlgItemMessage(hwndDlg,id,CB_GETITEMDATA,l,0);
						if(x && x != (wchar_t*)-1 && !_wcsicmp(filterStr,x)) {
							SendDlgItemMessage(hwndDlg,id,CB_SETCURSEL,(WPARAM)l,0);
							break;
						}
					}
				}

				if(configDevice->videoView) CheckDlgButton(hwndDlg,IDC_CHECK_VIDEOVIEW,TRUE);
			}
			SendMessage(hwndDlg,WM_USER,0,0);
			break;
		case WM_USER:
			{
				BOOL full_enable = (IsDlgButtonChecked(hwndDlg,IDC_RADIO_FILTERS1) != BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_FILTER1), full_enable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_FILTER2), full_enable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_FILTER2), full_enable);

				BOOL enable = (IsDlgButtonChecked(hwndDlg, IDC_RADIO_FILTERS3) == BST_CHECKED) && full_enable;
				EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_FILTER3), enable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_FILTER3), enable);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_RADIO_FILTERS3:
				case IDC_RADIO_FILTERS2:
				case IDC_RADIO_FILTERS1:
					SendMessage(hwndDlg,WM_USER,0,0);
					break;
				case IDC_CHECK_VIDEOVIEW:
					configDevice->SetVideoView(IsDlgButtonChecked(hwndDlg,IDC_CHECK_VIDEOVIEW));
					break;
				case IDC_REMEMBER_SEARCH:
					config->WriteInt(L"savefilter", IsDlgButtonChecked(hwndDlg,IDC_REMEMBER_SEARCH));
					config->WriteString(L"savedfilter", L"");
					config->WriteString(L"savedrefinefilter", L"");
					break;
			}
			break;
		case WM_DESTROY:
			{
				int update_needed = false;
				int numFilters = config->ReadInt(L"media_numfilters",2);

				int new_numFilters = 1;
				if (IsDlgButtonChecked(hwndDlg,IDC_RADIO_FILTERS1)) new_numFilters = 1;
				else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO_FILTERS2)) new_numFilters = 2;
				else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO_FILTERS3)) new_numFilters = 3;

				if (new_numFilters != numFilters) update_needed++;
				config->WriteInt(L"media_numfilters",new_numFilters);

				for(int i=0; i<3; i++) {
					int id = (i==0)?IDC_COMBO_FILTER1:((i==1)?IDC_COMBO_FILTER2:IDC_COMBO_FILTER3);
					int sel = SendDlgItemMessage(hwndDlg, id, CB_GETCURSEL, 0, 0);
					wchar_t * x = (wchar_t*)SendDlgItemMessage(hwndDlg, id, CB_GETITEMDATA, sel, 0);
					wchar_t name[20] = {0};
					StringCchPrintf(name,20,L"media_filter%d",i);

					extern wchar_t *GetDefFilter(int i,int n);
					SendDlgItemMessage(hwndDlg,id,CB_SETCURSEL,0,0);
					wchar_t* filterStr = config->ReadString(name,GetDefFilter(i,new_numFilters));

					if(x && x != (wchar_t*)-1)
					{
						config->WriteString(name,x);
						update_needed += (wcscmp(filterStr, x));
					}
				}

				// only refresh the view if it is one of ours (is a bit silly
				// otherwise) and also not refresh unless there was a change
				// with the configDevice check to cope with going elsewhere
				if (update_needed && ((configDevice && configDevice == currentViewedDevice) || IsWindow(hwndMediaView))) {
					PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
				}
			}
			break;
	}
	return 0;
}

static INT_PTR CALLBACK config_dlgproc_cloud_mediaview(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
			{
				if(lParam) config_tab_init(hwndDlg,((prefsParam*)lParam)->parent);
				int fieldsBits = (int)configDevice->dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
				if(!fieldsBits) fieldsBits=-1;
				// used to ensure that the filters will work irrespective of language in use
				wchar_t * filters[] = {
					(fieldsBits&SUPPORTS_ARTIST)?L"Artist":0,
					(fieldsBits&SUPPORTS_ALBUM)?L"Album":0,
					(fieldsBits&SUPPORTS_GENRE)?L"Genre":0,
					(fieldsBits&SUPPORTS_YEAR)?L"Year":0,
					(fieldsBits&SUPPORTS_ALBUMARTIST)?L"Album Artist":0,
					(fieldsBits&SUPPORTS_PUBLISHER)?L"Publisher":0,
					(fieldsBits&SUPPORTS_COMPOSER)?L"Composer":0,
					(fieldsBits&SUPPORTS_ARTIST)?L"Artist Index":0,
					(fieldsBits&SUPPORTS_ALBUMARTIST)?L"Album Artist Index":0,
					(fieldsBits&SUPPORTS_ALBUMART)?L"Album Art":0,
					(fieldsBits&SUPPORTS_MIMETYPE)?L"Mime":0,
					(fieldsBits&SUPPORTS_DATEADDED)?L"Date Added":0,
				};
				// used for displayed items - localised crazyness, heh
				int filters_idx[] = {
					(fieldsBits&SUPPORTS_ARTIST)?IDS_ARTIST:0,
					(fieldsBits&SUPPORTS_ALBUM)?IDS_ALBUM:0,
					(fieldsBits&SUPPORTS_GENRE)?IDS_GENRE:0,
					(fieldsBits&SUPPORTS_YEAR)?IDS_YEAR:0,
					(fieldsBits&SUPPORTS_ALBUMARTIST)?IDS_ALBUM_ARTIST:0,
					(fieldsBits&SUPPORTS_PUBLISHER)?IDS_PUBLISHER:0,
					(fieldsBits&SUPPORTS_COMPOSER)?IDS_COMPOSER:0,
					(fieldsBits&SUPPORTS_ARTIST)?IDS_ARTIST_INDEX:0,
					(fieldsBits&SUPPORTS_ALBUMARTIST)?IDS_ALBUM_ARTIST_INDEX:0,
					(fieldsBits&SUPPORTS_ALBUMART)?IDS_ALBUM_ART:0,
					(fieldsBits&SUPPORTS_MIMETYPE)?IDS_MIME_TYPE:0,
					(fieldsBits&SUPPORTS_DATEADDED)?IDS_DATE_ADDED:0,
				};
				int numFilters = config->ReadInt(L"media_numfilters",2);
				CheckRadioButton(hwndDlg, IDC_RADIO_FILTERS1, IDC_RADIO_FILTERS3, (IDC_RADIO_FILTERS1 + numFilters - 1));

				CheckDlgButton(hwndDlg, IDC_REMEMBER_SEARCH, config->ReadInt(L"savefilter", 1));

				for(int i=0; i<3; i++) {
					int id = (i==0)?IDC_COMBO_FILTER1:((i==1)?IDC_COMBO_FILTER2:IDC_COMBO_FILTER3);
					for(int j=0; j<(sizeof(filters)/sizeof(wchar_t*)); j++) {
						if(filters[j]) {
							int a = SendDlgItemMessage(hwndDlg,id,CB_ADDSTRING,0,
									(LPARAM)WASABI_API_LNGSTRINGW(filters_idx[j]));
							SendDlgItemMessage(hwndDlg,id,CB_SETITEMDATA,a,(LPARAM)filters[j]);
						}
					}

					wchar_t name[20] = {0};
					StringCchPrintf(name,20,L"media_filter%d",i);
					extern wchar_t *GetDefFilter(int i,int n);
					SendDlgItemMessage(hwndDlg,id,CB_SETCURSEL,0,0);
					wchar_t* filterStr = config->ReadString(name,GetDefFilter(i,numFilters));
					for(int l = 0; l < (sizeof(filters)/sizeof(wchar_t*)); l++) {
						wchar_t* x = (wchar_t*)SendDlgItemMessage(hwndDlg,id,CB_GETITEMDATA,l,0);
						if(x && x != (wchar_t*)-1 && !_wcsicmp(filterStr,x)) {
							SendDlgItemMessage(hwndDlg,id,CB_SETCURSEL,(WPARAM)l,0);
							break;
						}
					}
				}
			}
			SendMessage(hwndDlg,WM_USER,0,0);
			break;
		case WM_USER:
			{
				BOOL full_enable = (IsDlgButtonChecked(hwndDlg,IDC_RADIO_FILTERS1) != BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_FILTER1), full_enable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_FILTER2), full_enable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_FILTER2), full_enable);

				BOOL enable = (IsDlgButtonChecked(hwndDlg, IDC_RADIO_FILTERS3) == BST_CHECKED) && full_enable;
				EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_FILTER3), enable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_FILTER3), enable);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_RADIO_FILTERS3:
				case IDC_RADIO_FILTERS2:
				case IDC_RADIO_FILTERS1:
					SendMessage(hwndDlg,WM_USER,0,0);
					break;
				case IDC_REMEMBER_SEARCH:
					config->WriteInt(L"savefilter", IsDlgButtonChecked(hwndDlg,IDC_REMEMBER_SEARCH));
					config->WriteString(L"savedfilter", L"");
					config->WriteString(L"savedrefinefilter", L"");
					break;
			}
			break;
		case WM_DESTROY:
			{
				int update_needed = false;
				int numFilters = config->ReadInt(L"media_numfilters",2);

				int new_numFilters = 1;
				if (IsDlgButtonChecked(hwndDlg,IDC_RADIO_FILTERS1)) new_numFilters = 1;
				else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO_FILTERS2)) new_numFilters = 2;
				else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO_FILTERS3)) new_numFilters = 3;

				if (new_numFilters != numFilters) update_needed++;
				config->WriteInt(L"media_numfilters",new_numFilters);

				for(int i=0; i<3; i++) {
					int id = (i==0)?IDC_COMBO_FILTER1:((i==1)?IDC_COMBO_FILTER2:IDC_COMBO_FILTER3);
					int sel = SendDlgItemMessage(hwndDlg, id, CB_GETCURSEL, 0, 0);
					wchar_t * x = (wchar_t*)SendDlgItemMessage(hwndDlg, id, CB_GETITEMDATA, sel, 0);
					wchar_t name[20] = {0};
					StringCchPrintf(name,20,L"media_filter%d",i);

					extern wchar_t *GetDefFilter(int i,int n);
					SendDlgItemMessage(hwndDlg,id,CB_SETCURSEL,0,0);
					wchar_t* filterStr = config->ReadString(name,GetDefFilter(i,new_numFilters));

					if(x && x != (wchar_t*)-1)
					{
						config->WriteString(name,x);
						update_needed += (wcscmp(filterStr, x));
					}
				}

				// only refresh the view if it is one of ours (is a bit silly
				// otherwise) and also not refresh unless there was a change
				// with the configDevice check to cope with going elsewhere
				if (update_needed && ((configDevice && configDevice == currentViewedDevice) || IsWindow(hwndMediaView))) {
					PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
				}
			}
			break;
	}
	return 0;
}

extern void unloadPlugin(PMPDevicePlugin *devplugin, int n=-1);
extern PMPDevicePlugin * loadPlugin(wchar_t * file);

HRESULT RemovePMPPlugin(LPCWSTR file, HINSTANCE hDllInstance) {
	if(!hDllInstance) {
		SHFILEOPSTRUCT op = {0};
		wchar_t srcFile[MAX_PATH+1], *end;
		op.wFunc = FO_DELETE;
		StringCchCopyExW(srcFile, MAX_PATH, file, &end, 0, 0);
		if (end) end[1]=0; // double null terminate
		op.pFrom = srcFile;
		op.fFlags=FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_SIMPLEPROGRESS|FOF_NORECURSION|FOF_NOERRORUI|FOF_SILENT;
		return (!SHFileOperation(&op)? S_OK : E_FAIL);
	}
	else {
		wchar_t buf[1024],
				*ini = (wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETINIFILEW);
		GetModuleFileName(hDllInstance, buf, ARRAYSIZE(buf));
		WritePrivateProfileString(L"winamp", L"remove_genplug", buf, ini);
		WritePrivateProfileString(L"winamp", L"show_prefs", L"-1", ini);
		PostMessage(plugin.hwndWinampParent, WM_USER, 0, IPC_RESTARTWINAMP);
		return S_OK;
	}
}

static bool pluginsLoaded;
INT_PTR CALLBACK config_dlgproc_plugins(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			pluginsLoaded = false;
			link_startsubclass(hwndDlg, IDC_PLUGINVERS);
			{
				HWND listWindow = GetDlgItem(hwndDlg, IDC_PLUGINSLIST);
				if (NULL != listWindow)
				{
					RECT r = {0}, rc = {0};
					GetWindowRect(listWindow, &r);
					GetClientRect(listWindow, &r);
					MapWindowPoints(listWindow, hwndDlg, (LPPOINT)&r, 2);
					InflateRect(&r, 2, 2);
					DestroyWindow(listWindow);
					listWindow = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
												WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL |
												LVS_SHOWSELALWAYS | LVS_SORTASCENDING | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER,
												r.left, r.top, r.right - r.left, r.bottom - r.top,
												hwndDlg, (HMENU)IDC_PLUGINSLIST, NULL, NULL);
					SetWindowPos(listWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
					ListView_SetExtendedListViewStyleEx(listWindow, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
					SendMessage(listWindow, WM_SETFONT, SendMessage(hwndDlg, WM_GETFONT, 0, 0), FALSE);

					LVCOLUMNW lvc = {0};
					ListView_InsertColumn(listWindow, 0, &lvc);
					ListView_InsertColumn(listWindow, 1, &lvc);

					wchar_t buf[1024] = {0}, fn[MAX_PATH] = {0};
					for (int x = 0; x < m_plugins.GetSize(); x ++)
					{
						PMPDevicePlugin * devplugin=(PMPDevicePlugin *)m_plugins.Get(x);
						if (devplugin)
						{
							GetModuleFileNameW(devplugin->hDllInstance, fn, MAX_PATH);
							PathStripPath(fn);

							LVITEMW lvi = {LVIF_TEXT | LVIF_PARAM, x, 0};
							lvi.pszText = devplugin->description;
							lvi.lParam = x;
							lvi.iItem = ListView_InsertItem(listWindow, &lvi);

							lvi.mask = LVIF_TEXT;
							lvi.iSubItem = 1;
							lvi.pszText = fn;
							ListView_SetItem(listWindow, &lvi);
						}
					}

					WIN32_FIND_DATA d = {0};
					wchar_t *pluginPath = (wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORYW);
					wchar_t dirstr[MAX_PATH] = {0};
					PathCombine(dirstr, pluginPath, L"PMP_*.DLL");
					HANDLE h = FindFirstFile(dirstr, &d);
					if (h != INVALID_HANDLE_VALUE)
					{
						do
						{
							PathCombine(dirstr, pluginPath, d.cFileName);
							HMODULE b = LoadLibraryEx(dirstr, NULL, LOAD_LIBRARY_AS_DATAFILE);
							int x = 0;
							for (; b && (x != m_plugins.GetSize()); x ++)
							{
								PMPDevicePlugin *devplugin = (PMPDevicePlugin *)m_plugins.Get(x);
								if (devplugin->hDllInstance == b)
								{
									break;
								}
							}

							if (x == m_plugins.GetSize() || !b)
							{
								LVITEMW lvi = {LVIF_TEXT | LVIF_PARAM, x, 0};
								lvi.pszText = d.cFileName;
								lvi.lParam = -2;
								lvi.iItem = ListView_InsertItem(listWindow, &lvi);

								lvi.mask = LVIF_TEXT;
								lvi.iSubItem = 1;
								lvi.pszText = WASABI_API_LNGSTRINGW(IDS_NOT_LOADED);
								ListView_SetItem(listWindow, &lvi);
							}
							FreeLibrary(b);
						}
						while (FindNextFile(h, &d));
						FindClose(h);
					}

					GetClientRect(listWindow, &r);
					ListView_SetColumnWidth(listWindow, 1, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(listWindow, 0, (r.right - r.left) - ListView_GetColumnWidth(listWindow, 1));

					if (NULL != WASABI_API_APP)
						WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);

					pluginsLoaded = true;
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR p = (LPNMHDR)lParam;
			if (p->idFrom == IDC_PLUGINSLIST)
			{
				if (p->code == LVN_ITEMCHANGED)
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
					LVITEM lvi = {LVIF_PARAM, pnmv->iItem};
					if (ListView_GetItem(p->hwndFrom, &lvi) && (pnmv->uNewState & LVIS_SELECTED))
					{
						int loaded = (lvi.lParam != -2);
						if (loaded)
						{
							PMPDevicePlugin *devplugin;
							if (lvi.lParam >= 0 && lvi.lParam < m_plugins.GetSize() &&
								(devplugin = (PMPDevicePlugin *)m_plugins.Get(lvi.lParam)))
							{
								// enables / disables the config button as applicable instead of the
								// "This plug-in has no configuration implemented" message (opt-in)
								EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIGPLUGIN), (!devplugin->MessageProc(PMP_NO_CONFIG, 0, 0, 0)));
							}
						}
						else
						{
							EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIGPLUGIN), 0);
						}
						EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTALLPLUGIN), 1);
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIGPLUGIN), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTALLPLUGIN), 0);
					}
				}
				else if (p->code == NM_DBLCLK)
				{
					PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CONFIGPLUGIN, 0), (LPARAM)GetDlgItem(hwndDlg, IDC_CONFIGPLUGIN));
				}
			}
			else if (p->code == HDN_ITEMCHANGINGW)
			{
				if (pluginsLoaded)
				{
#if defined(_WIN64)
					SetWindowLong(hwndDlg, DWLP_MSGRESULT, TRUE);
#else
					SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
#endif
					return TRUE;
				}
			}
			break;
		}

		case WM_DESTROY:
			{
				HWND listWindow = GetDlgItem(hwndDlg, IDC_PLUGINSLIST);
				if (IsWindow(listWindow) && (NULL != WASABI_API_APP))
					WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_CONFIGPLUGIN:
					{
						if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CONFIGPLUGIN)))
						{
							HWND listWindow = GetDlgItem(hwndDlg, IDC_PLUGINSLIST);
							LVITEM lvi = {LVIF_PARAM, ListView_GetSelectionMark(listWindow)};
							if (ListView_GetItem(listWindow, &lvi))
							{
								PMPDevicePlugin *devplugin;
								if(lvi.lParam >= 0 && lvi.lParam < m_plugins.GetSize() && (devplugin=(PMPDevicePlugin *)m_plugins.Get(lvi.lParam)))
								{
									if(devplugin->MessageProc(PMP_CONFIG,(intptr_t)hwndDlg,0,0) == 0)
									{
										wchar_t titleStr[64] = {0};
										MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_PLUGIN_HAS_NO_CONFIG_IMPLEMENTED),
										WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_PLUGINS,titleStr,64),0);
									}
								}
							}
						}
					}
					break;

				case IDC_UNINSTALLPLUGIN:
					{
						if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_UNINSTALLPLUGIN)))
						{
							HWND listWindow = GetDlgItem(hwndDlg, IDC_PLUGINSLIST);
							int which_sel = ListView_GetSelectionMark(listWindow);
							LVITEM lvi = {LVIF_PARAM, which_sel};
							if (ListView_GetItem(listWindow, &lvi))
							{
								PMPDevicePlugin *devplugin = 0;
								wchar_t titleStr[32] = {0};
								int msgBox = MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_PERMANENTLY_UNINSTALL_THIS_PLUGIN),
																 WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRMATION, titleStr, 32), MB_YESNO | MB_ICONEXCLAMATION);

								if (lvi.lParam >= 0 && lvi.lParam <= m_plugins.GetSize() && (devplugin=(PMPDevicePlugin *)m_plugins.Get(lvi.lParam)) && msgBox == IDYES)
								{
									wchar_t buf[1024] = {0};
									GetModuleFileName(devplugin->hDllInstance,buf,sizeof(buf)/sizeof(wchar_t));
									int ret = PMP_PLUGIN_UNINSTALL_NOW;
									int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);
									*(void**)&pr = (void*)GetProcAddress(devplugin->hDllInstance,"winampUninstallPlugin");
									if(pr) ret = pr(devplugin->hDllInstance,hwndDlg,0);
									if(pr && ret == PMP_PLUGIN_UNINSTALL_NOW) { // dynamic unload
										ListView_DeleteItem(listWindow, lvi.lParam);
										unloadPlugin(devplugin,lvi.lParam);

										// removing the plugin (bit convoluted to hopefully not cause crashes with dynamic removal)
										// try to use the elevator to do this
										IFileTypeRegistrar *registrar = (IFileTypeRegistrar*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GET_FILEREGISTRAR_OBJECT);
										if(registrar && (registrar != (IFileTypeRegistrar*)1)) {
											if(registrar->DeleteItem(buf) != S_OK) {
												// we don't always free by default as it can cause some crashes
												FreeLibrary(devplugin->hDllInstance);
												if(registrar->DeleteItem(buf) != S_OK) {
													// all gone wrong so non-dynamic unload (restart winamp)
													RemovePMPPlugin(buf, devplugin->hDllInstance);
												}
											}
											registrar->Release();
										}
										// otherwise revert to a standard method
										else {
											if(RemovePMPPlugin(buf, 0) != S_OK){
												// we don't always free by default as it can cause some crashes
												FreeLibrary(devplugin->hDllInstance);
												if(RemovePMPPlugin(buf, 0) != S_OK) {
													// all gone wrong so non-dynamic unload (restart winamp)
													RemovePMPPlugin(buf, devplugin->hDllInstance);
												}
											}
										}
									}
									else if(!pr)
									{ // non-dynamic unload (restart winamp)
										RemovePMPPlugin(buf,devplugin->hDllInstance);
									}
								}
								// will cope with not loaded plug-ins so we can still remove them, etc
								else if (lvi.lParam == -2 && msgBox == IDYES)
								{
									wchar_t buf[1024] = {0}, base[1024] = {0};
									GetModuleFileName(plugin.hDllInstance,base,sizeof(base)/sizeof(wchar_t));

									LVITEM lvi = {LVIF_TEXT, which_sel};
									lvi.pszText = buf;
									lvi.cchTextMax = ARRAYSIZE(buf);
									ListView_GetItem(listWindow, &lvi);

									wchar_t *p = wcschr(buf, L'.');
									if (p && *p == L'.')
									{
										p += 4;
										*p = 0;
										PathRemoveFileSpec(base);
										PathAppend(base, buf);
									}

									// try to use the elevator to do this
									IFileTypeRegistrar *registrar = (IFileTypeRegistrar*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GET_FILEREGISTRAR_OBJECT);
									if(registrar && (registrar != (IFileTypeRegistrar*)1)) {
										if(registrar->DeleteItem(base) != S_OK){
											RemovePMPPlugin(base, 0);
										}
										else
											ListView_DeleteItem(listWindow, which_sel);
										registrar->Release();
									}
									// otherwise revert to a standard method
									else {
										RemovePMPPlugin(base, 0);
									}
								}

								// resets the focus to the listbox so it'll keep ui response working
								SetFocus(GetDlgItem(hwndDlg, IDC_PLUGINSLIST));
							}
						}
					}
					break;

                case IDC_PLUGINVERS:
					myOpenURLWithFallback(hwndDlg, L"http://www.google.com/search?q=Winamp+Portable+Plugins",L"http://www.google.com/search?q=Winamp+Portable+Plugins");
					break;
			}
			break;
	}
	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return 0;
}

void myOpenURLWithFallback(HWND hwnd, wchar_t *loc, wchar_t *fallbackLoc)
{
	bool override=false;
	if (loc)
	{
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::BROWSER, BrowserCallback::ONOPENURL, reinterpret_cast<intptr_t>(loc), reinterpret_cast<intptr_t>(&override));
	}
	if (!override && fallbackLoc)
		ShellExecuteW(hwnd, L"open", fallbackLoc, NULL, NULL, SW_SHOWNORMAL);
}

static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if(uMsg == WM_SETCURSOR)
	{
		if((HWND)wParam == hwndDlg)
		{
			if(!link_hand_cursor)
			{
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}

	return ret;
}

void link_startsubclass(HWND hwndDlg, UINT id){
HWND ctrl = GetDlgItem(hwndDlg, id);
	SetPropW(ctrl, L"link_proc",
			(HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
}

static void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON)
		{
			wchar_t wt[123] = {0};
			int y;
			RECT r;
			HPEN hPen, hOldPen;
			GetDlgItemText(hwndDlg, wParam, wt, sizeof(wt)/sizeof(wchar_t));

			// draw text
			SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			r = di->rcItem;
			r.left += 2;
			DrawText(di->hDC, wt, -1, &r, DT_VCENTER | DT_SINGLELINE);

			memset(&r, 0, sizeof(r));
			DrawText(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);

			// draw underline
			y = di->rcItem.bottom - ((di->rcItem.bottom - di->rcItem.top) - (r.bottom - r.top)) / 2 - 1;
			hPen = CreatePen(PS_SOLID, 0, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			hOldPen = (HPEN) SelectObject(di->hDC, hPen);
			MoveToEx(di->hDC, di->rcItem.left + 2, y, NULL);
			LineTo(di->hDC, di->rcItem.right + 2 - ((di->rcItem.right - di->rcItem.left) - (r.right - r.left)), y);
			SelectObject(di->hDC, hOldPen);
			DeleteObject(hPen);
		}
	}
}