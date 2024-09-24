#include "main.h"
#include <windowsx.h>
#include "resource.h"

#include "..\nu\listview.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ChildSizer.h"

#include "config.h"
#include "../winamp/wa_ipc.h"

#include "..\..\General\gen_ml/gaystring.h"

#include <stdio.h>
#include <shlobj.h>
#include <time.h>
#include "../nu/AutoChar.h"
#include "../nu/AutoCharFn.h"
#include "../nu/AutoWide.h"

#include "ReplayGain.h"

#include "M3UWriter.h"
#include "PLSWriter.h"
#include "./settings.h"
#include <shlwapi.h>
#include <windows.h>
#include <strsafe.h>

extern unsigned int FileTimeToUnixTime(FILETIME *ft);

static UINT uMsgRipperNotify = 0;


static INT_PTR WINAPI DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND CreateCDRipWindow(HWND hwndParent, CHAR cLetter)
{
	return WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_CDROM_EX2, hwndParent, DlgProc, (LPARAM)cLetter);
}
//physically update metadata in a given file
int updateFileInfo(char *filename, char *metadata, char *data)
{
	extendedFileInfoStruct efis = {
	                                  filename,
	                                  metadata,
	                                  data ? data : "",
	                                  data ? strlen(data) : 0,
	                              };
	return (INT)(INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFO);
}

//physically update metadata in a given file
int updateFileInfoW(wchar_t *filename, const wchar_t *metadata, const wchar_t *data)
{
	extendedFileInfoStructW efis = {
	                                   filename,
	                                   metadata,
	                                   data ? data : L"",
	                                   data ? lstrlenW(data) : 0,
	                               };
	return (INT)(INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);
}


static int m_extract_curtrack, m_extract_nb, m_extract_nb_total;
static int m_db_has_upd;
static convertFileStructW m_fcs;
static wchar_t m_extract_src[64];
static DWORD m_extract_time;
static int m_extracting;
static cdrip_params *m_rip_params;
HWND m_extract_wnd = 0;
static wchar_t m_last_total_status[512];
static wchar_t m_last_item_status[512];
static int m_cur_rg = 0;
static int done = 0;

bool RegisteredEncoder(DWORD fourcc)
{
	if (fourcc == mmioFOURCC('M', 'P', '3', 'l')
	        || fourcc == mmioFOURCC('A', 'A', 'C', 'H')
	        || fourcc == mmioFOURCC('M', '4', 'A', 'H'))
		return true;
	else
		return false;
}

static void createDirForFile(char *str)
{
	char *p = str;
	if ((p[0] == '\\' || p[0] == '/') && (p[1] == '\\' || p[1] == '/'))
	{
		p += 2;
		while (p && *p && *p != '\\' && *p != '/') p++;
		if (!p || !*p) return ;
		p++;
		while (p && *p && *p != '\\' && *p != '/') p++;
	}
	else
	{
		while (p && *p && *p != '\\' && *p != '/') p++;
	}

	while (p && *p)
	{
		while (p && *p != '\\' && *p != '/' && *p) p = CharNextA(p);
		if (p && *p)
		{
			char lp = *p;
			*p = 0;
			CreateDirectoryA(str, NULL);
			*p++ = lp;
		}
	}
}

static void createDirForFileW(wchar_t *str)
{
	wchar_t *p = str;
	if ((p[0] ==L'\\' || p[0] ==L'/') && (p[1] ==L'\\' || p[1] ==L'/'))
	{
		p += 2;
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
		if (!p || !*p) return ;
		p++;
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
	}
	else
	{
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
	}

	while (p && *p)
	{
		while (p && *p !=L'\\' && *p !=L'/' && *p) p = CharNextW(p);
		if (p && *p)
		{
			wchar_t lp = *p;
			*p = 0;
			CreateDirectoryW(str, NULL);
			*p++ = lp;
		}
	}
}

static void LockCD(char cLetter, BOOL bLock)
{
	wchar_t info[32] = {0};
	wchar_t name[] = L"cda://X";
	name[6] = cLetter;
	getFileInfoW(name, (bLock) ? L"cdlock" : L"cdunlock", info, sizeof(info)/sizeof(wchar_t));
}

static int m_pstat_bytesdone;
static int m_pstat_bytesout;
static int m_pstat_timedone;

static int m_rip_done;

static HWND m_hwndstatus;
static W_ListView m_statuslist;

static void NotifyInfoWindow(HWND hwnd, BOOL bForceRefresh)
{
	HWND hwndParent;
	hwndParent = GetParent(hwnd);
	if (hwndParent) SendMessageW(hwndParent, WM_SHOWFILEINFO, 
							(WPARAM) WISF_MESSAGE | ((bForceRefresh) ? WISF_FORCE : WISF_NORMAL), 
							(LPARAM)WASABI_API_LNGSTRINGW(IDS_INFO_RIPPING));
}

static void ListView_OnItemChanged(HWND hwndDlg, NMLISTVIEW *pnmv)
{
	if (LVIF_STATE & pnmv->uChanged)
	{
		if ((LVIS_FOCUSED & pnmv->uOldState) != (LVIS_FOCUSED & pnmv->uNewState))
		{
			NotifyInfoWindow(hwndDlg, TRUE);
		}
	}
}

static void Window_OnQueryInfo(HWND hwnd)
{
	NotifyInfoWindow(hwnd, FALSE);
}

static INT_PTR Window_OnNotify(HWND hwndDlg, INT ctrlId, LPNMHDR phdr)
{
	switch(phdr->idFrom)
	{
		case IDC_LIST2:
			switch(phdr->code)
			{ 
				case LVN_ITEMCHANGED: ListView_OnItemChanged(hwndDlg, (NMLISTVIEW*)phdr); break;
			}
			break;
	}
	return 0;
}

INT_PTR WINAPI DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static ChildWndResizeItem ripwnd_rlist[] = {
		{IDC_LIST2, 0x0011},
		{IDC_CDINFO, 0x0000},
		{IDC_RIPOPTS, 0x0101},
		{IDC_CANCEL_RIP, 0x0101},
		{IDC_BTN_SHOWINFO, 0x1111},
	};

	INT_PTR a = (INT_PTR)dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam); if (a) return a;
	switch (uMsg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			childSizer.Resize(hwndDlg, ripwnd_rlist, sizeof(ripwnd_rlist) / sizeof(ripwnd_rlist[0]));
		}
		break;
	case WM_PAINT:
		{
			int tab[] = { IDC_LIST2 | DCW_SUNKENBORDER};
			dialogSkinner.Draw(hwndDlg, tab, 1);
		}
		return 0;
	case WM_INITDIALOG:

		m_hwndstatus = hwndDlg;

		SendMessageW(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(IDC_BTN_SHOWINFO, BN_EX_GETTEXT), (LPARAM)GetDlgItem(hwndDlg, IDC_BTN_SHOWINFO));

		m_statuslist.setwnd(GetDlgItem(hwndDlg, IDC_LIST2));
		m_statuslist.AddCol(WASABI_API_LNGSTRINGW(IDS_TRACK_NUMBER), g_view_metaconf->ReadInt(L"col_track", 60));
		m_statuslist.AddCol(WASABI_API_LNGSTRINGW(IDS_TITLE), g_view_metaconf->ReadInt(L"col_title", 200));
		m_statuslist.AddCol(WASABI_API_LNGSTRINGW(IDS_LENGTH), g_view_metaconf->ReadInt(L"col_len", 80));
		m_statuslist.AddCol(WASABI_API_LNGSTRINGW(IDS_STATUS), g_view_metaconf->ReadInt(L"col_status", 200));

		SetDlgItemText(hwndDlg, IDC_CDINFO, m_last_total_status);
		{
			int l = m_rip_params->ntracks;
			int x = 0;
			for (int i = 0;i < l;i++) if (m_rip_params->tracks[i])
			{
				wchar_t buf[1024] = {0};
				StringCchPrintf(buf, 1024, L"%d", i + 1);
				m_statuslist.InsertItem(x, buf, 0);
				m_statuslist.SetItemText(x, 1, m_rip_params->tracks[i]);

				StringCchPrintf(buf, 512, L"%d:%02d", m_rip_params->lengths[i] / 60, m_rip_params->lengths[i] % 60);
				m_statuslist.SetItemText(x, 2, buf);

				if (i < m_extract_curtrack || m_rip_done)
					m_statuslist.SetItemText(x, 3, WASABI_API_LNGSTRINGW(IDS_COMPLETED));
				else if (i > m_extract_curtrack)
					m_statuslist.SetItemText(x, 3, WASABI_API_LNGSTRINGW(IDS_QUEUED));
				else
				{
					m_statuslist.SetItemText(x, 3, m_last_item_status[0] ? m_last_item_status : WASABI_API_LNGSTRINGW(IDS_RIPPING));
				}
				x++;
			}
		}

		if (!m_extract_wnd || m_rip_done)
		{
			SetDlgItemText(hwndDlg, IDC_CANCEL_RIP, WASABI_API_LNGSTRINGW(IDS_CLOSE));
		}

		if (g_config->ReadInt(L"cdripautoeject", 0)) CheckDlgButton(hwndDlg, IDC_CHECK2, BST_CHECKED);
		if (g_config->ReadInt(L"cdripautoplay", 0)) CheckDlgButton(hwndDlg, IDC_CHECK3, BST_CHECKED);
		if (g_config->ReadInt(L"cdripautoclose", 1)) CheckDlgButton(hwndDlg, IDC_CHECK1, BST_CHECKED);

		childSizer.Init(hwndDlg, ripwnd_rlist, sizeof(ripwnd_rlist) / sizeof(ripwnd_rlist[0]));

		ListView_SetTextColor(m_statuslist.getwnd(), dialogSkinner.Color(WADLG_ITEMFG));
		ListView_SetBkColor(m_statuslist.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
		ListView_SetTextBkColor(m_statuslist.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
				
		if(m_statuslist.getwnd())
		{
			MLSKINWINDOW sw;
			sw.hwndToSkin = m_statuslist.getwnd();
			sw.skinType = SKINNEDWND_TYPE_LISTVIEW;
			sw.style = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
			MLSkinWindow(plugin.hwndLibraryParent, &sw);
		}
		NotifyInfoWindow(hwndDlg, TRUE);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RIPOPTS:
			{
				RECT r;
				HMENU menu = GetSubMenu(g_context_menus, 5);
				GetWindowRect((HWND)lParam, &r);
				CheckMenuItem(menu, ID_RIPOPTIONS_RIPPINGSTATUSWINDOW, g_config->ReadInt(L"cdripstatuswnd", 0) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(menu, ID_RIPOPTIONS_EJECTCDWHENCOMPLETED, g_config->ReadInt(L"cdripautoeject", 0) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(menu, ID_RIPOPTIONS_PLAYTRACKSWHENCOMPLETED, g_config->ReadInt(L"cdripautoplay", 0) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(menu, ID_RIPOPTIONS_CLOSEVIEWWHENCOMPLETE, g_config->ReadInt(L"cdripautoclose", 0) ? MF_CHECKED : MF_UNCHECKED);

				int prio = g_config->ReadInt(L"extractprio", THREAD_PRIORITY_NORMAL);
				CheckMenuItem(menu, ID_RIPOPTIONS_PRIORITY_IDLE, prio == THREAD_PRIORITY_IDLE ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(menu, ID_RIPOPTIONS_PRIORITY_LOWEST, prio == THREAD_PRIORITY_LOWEST ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(menu, ID_RIPOPTIONS_PRIORITY_BELOWNORMAL, prio == THREAD_PRIORITY_BELOW_NORMAL ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(menu, ID_RIPOPTIONS_PRIORITY_NORMAL, prio == THREAD_PRIORITY_NORMAL ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(menu, ID_RIPOPTIONS_PRIORITY_ABOVENORMAL, prio == THREAD_PRIORITY_ABOVE_NORMAL ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(menu, ID_RIPOPTIONS_PRIORITY_HIGH, prio == THREAD_PRIORITY_HIGHEST ? MF_CHECKED : MF_UNCHECKED);

				int x = Menu_TrackPopup(plugin.hwndLibraryParent, menu,
										TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN |
										TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
										r.left, r.top, hwndDlg, NULL);
				switch (x)
				{
				case ID_RIPOPTIONS_RIPPINGSTATUSWINDOW:
					{
						int x = g_config->ReadInt(L"cdripstatuswnd", 0);
						g_config->WriteInt(L"cdripstatuswnd", !x);
						ShowWindow(m_extract_wnd, x ? SW_HIDE : SW_SHOW);
					}
					break;
				case ID_RIPOPTIONS_EJECTCDWHENCOMPLETED:
					g_config->WriteInt(L"cdripautoeject", !g_config->ReadInt(L"cdripautoeject", 0));
					break;
				case ID_RIPOPTIONS_PLAYTRACKSWHENCOMPLETED:
					g_config->WriteInt(L"cdripautoplay", !g_config->ReadInt(L"cdripautoplay", 0));
					break;
				case ID_RIPOPTIONS_CLOSEVIEWWHENCOMPLETE:
					g_config->WriteInt(L"cdripautoclose", !g_config->ReadInt(L"cdripautoclose", 0));
					break;
				case ID_RIPOPTIONS_PRIORITY_IDLE:
				case ID_RIPOPTIONS_PRIORITY_LOWEST:
				case ID_RIPOPTIONS_PRIORITY_BELOWNORMAL:
				case ID_RIPOPTIONS_PRIORITY_NORMAL:
				case ID_RIPOPTIONS_PRIORITY_ABOVENORMAL:
				case ID_RIPOPTIONS_PRIORITY_HIGH:
					{
						int prio = THREAD_PRIORITY_NORMAL;
						if (x == ID_RIPOPTIONS_PRIORITY_IDLE) prio = THREAD_PRIORITY_IDLE;
						if (x == ID_RIPOPTIONS_PRIORITY_LOWEST) prio = THREAD_PRIORITY_LOWEST;
						if (x == ID_RIPOPTIONS_PRIORITY_BELOWNORMAL) prio = THREAD_PRIORITY_BELOW_NORMAL;
						if (x == ID_RIPOPTIONS_PRIORITY_ABOVENORMAL) prio = THREAD_PRIORITY_ABOVE_NORMAL;
						if (x == ID_RIPOPTIONS_PRIORITY_HIGH) prio = THREAD_PRIORITY_HIGHEST;
						g_config->WriteInt(L"extractprio", prio);
						convertSetPriorityW csp = {
						                             &m_fcs,
						                             prio,
						                         };
						SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&csp, IPC_CONVERT_SET_PRIORITYW);
					}
					break;
				}
				Sleep(100);
				MSG msg;
				while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
			}
			return 0;
		case IDC_CANCEL_RIP:
		{
			wchar_t title[64] = {0};
			if (!m_extract_wnd ||
			        m_rip_done ||
					MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_CANCEL_RIP),
							   WASABI_API_LNGSTRINGW_BUF(IDS_CD_RIP_QUESTION,title,64),
							   MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				if (m_rip_params) LockCD(m_rip_params->drive_letter, FALSE);
				if (m_extract_wnd) DestroyWindow(m_extract_wnd);
				DestroyWindow(hwndDlg);
			}
			return 0;
		}
		case IDC_BTN_SHOWINFO:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED: 
					SendMessageW(GetParent(hwndDlg), WM_COMMAND, wParam, lParam); 
					NotifyInfoWindow(hwndDlg, TRUE);
					break;
			}
			break;
		}
		break;
	case WM_DESTROY:
		if (m_statuslist.getwnd())
		{
			g_view_metaconf->WriteInt(L"col_track", m_statuslist.GetColumnWidth(0));
			g_view_metaconf->WriteInt(L"col_title", m_statuslist.GetColumnWidth(1));
			g_view_metaconf->WriteInt(L"col_len", m_statuslist.GetColumnWidth(2));
			g_view_metaconf->WriteInt(L"col_status", m_statuslist.GetColumnWidth(3));
		}

		m_hwndstatus = 0;
		return 0;
		
	case WM_ERASEBKGND: return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
	case WM_QUERYFILEINFO:	Window_OnQueryInfo(hwndDlg); break;
	case WM_NOTIFY:			return  Window_OnNotify(hwndDlg, (INT)wParam, (LPNMHDR) lParam);
		

	}
	return 0;
}

static BOOL CALLBACK extract_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		m_extract_wnd = hwndDlg;
		m_rip_done = 0;
		SetDlgItemText(hwndDlg, IDC_STATUS, WASABI_API_LNGSTRINGW(IDS_INITIALIZING));
		m_pstat_bytesdone = 0;
		m_pstat_bytesout = 0;
		m_pstat_timedone = 0;
		m_extract_nb = 0;
		m_extract_curtrack = -1;
		m_cur_rg = 0;
		{
			m_extract_nb_total = 0;
			int l = m_rip_params->ntracks;
			for (int i = 0;i < l;i++) if (m_rip_params->tracks[i]) m_extract_nb_total++;
		}

		LockCD(m_rip_params->drive_letter, TRUE);
		
		SetPropW(hwndDlg, L"WARIPPER", (HANDLE)hwndDlg);
		SetPropW(hwndDlg, L"DRIVE", (HANDLE)(INT_PTR)(0xFF & m_rip_params->drive_letter));

		if (!uMsgRipperNotify) uMsgRipperNotify = RegisterWindowMessageA("WARIPPER_BROADCAST_MSG");
		if (uMsgRipperNotify) SendNotifyMessage(HWND_BROADCAST, uMsgRipperNotify, (WPARAM)m_rip_params->drive_letter, (LPARAM)TRUE);
		SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendDlgItemMessage(hwndDlg, IDC_PROGRESS2, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		PostMessage(hwndDlg, WM_APP + 1, 0, 0);
		if (g_config->ReadInt(L"cdripstatuswnd", 0)) 	ShowWindow(hwndDlg, SW_SHOW);

		INT bVal;
		
		if (S_OK == Settings_GetBool(C_EXTRACT, EF_CALCULATERG, &bVal) && bVal)
		{
			CreateGain();
			QueueUserAPC(StartGain, rgThread, 
					(ULONG_PTR)((m_rip_params->ntracks == m_extract_nb_total) ? RG_ALBUM : RG_INDIVIDUAL_TRACKS));
		}
		break;
	case WM_APP + 2:   // track is starting to be RG scanned
		m_statuslist.SetItemText(m_cur_rg, 3, WASABI_API_LNGSTRINGW(IDS_CALCULATING_REPLAY_GAIN));
		break;
	case WM_APP + 3:   // track is starting to be RG scanned
		m_statuslist.SetItemText(m_cur_rg, 3, WASABI_API_LNGSTRINGW(IDS_COMPLETED));
		m_cur_rg++;
		break;
	case WM_APP + 1:
		{
			INT trackOffset, cchDest;
			TCHAR szDestination[MAX_PATH] = {0}, szFormat[MAX_PATH] = {0};
			int l = m_rip_params->ntracks;
			done = 1;
				
			Settings_GetInt(C_EXTRACT, EF_TRACKOFFSET, &trackOffset);
			Settings_ReadString(C_EXTRACT, EF_TITLEFMT, szFormat, ARRAYSIZE(szFormat));
			Settings_ReadString(C_EXTRACT, EF_PATH, szDestination, ARRAYSIZE(szDestination));
			CleanupDirectoryString(szDestination);

			cchDest = lstrlen(szDestination);

			for (int i = m_extract_curtrack + 1;i < l;i++)
			{
				if (m_rip_params->tracks[i])
				{
					StringCchPrintfW(m_extract_src, 64, L"cda://%c,%d.cda", m_rip_params->drive_letter, i + 1);
					szDestination[cchDest] = TEXT('\0');
					if (cchDest) PathAddBackslash(szDestination);

					wchar_t tmp1[32] = {0}, tmp2[32] = {0}, tmp3[32] = {0}, tmp4[32] = {0}, tmp5[32] = {0};
					FormatFileName(szDestination,
								   ARRAYSIZE(szDestination)-11,	// ensure we're leaving enough room for the extension
								   szFormat,
								   i + trackOffset,
								   (m_rip_params->artist && *(m_rip_params->artist)) ? (m_rip_params->artist) : WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN_ARTIST,tmp1,32),
								   (m_rip_params->album && *(m_rip_params->album)) ? (m_rip_params->album) : WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN_ALBUM,tmp2,32),
								   (m_rip_params->tracks[i] && *(m_rip_params->tracks[i])) ? (m_rip_params->tracks[i]) : L"0",
								   (m_rip_params->genre && *(m_rip_params->genre)) ? (m_rip_params->genre) : WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN,tmp3,32),
								   (m_rip_params->year && *(m_rip_params->year)) ? (m_rip_params->year) : WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN,tmp4,32),
								   (m_rip_params->trackArtists[i] && m_rip_params->trackArtists[i][0])?m_rip_params->trackArtists[i] : WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN,tmp5,32),
								   NULL, (m_rip_params->disc && *(m_rip_params->disc)) ? (m_rip_params->disc) : L"");
					memset(&m_fcs, 0, sizeof(m_fcs));
					m_fcs.sourcefile = m_extract_src;
					INT fourcc;
					Settings_GetInt(C_EXTRACT, EF_FOURCC, &fourcc);
					m_fcs.destformat[0]=fourcc;
					if (m_fcs.destformat[0] == OLD_AAC_CODEC) Settings_GetDefault(C_EXTRACT, EF_FOURCC, &m_fcs.destformat[0]);

					// now determine the extension
					wchar_t fmt[10] = {0};
					GetExtensionString(fmt, ARRAYSIZE(fmt), (DWORD)m_fcs.destformat[0]);
					BOOL upperCase;
					if (SUCCEEDED(Settings_GetBool(C_EXTRACT, EF_UPPEREXTENSION, &upperCase)) && FALSE != upperCase)
						CharUpper(fmt);
					else
						CharLower(fmt);

					StringCchCat(szDestination, ARRAYSIZE(szDestination), TEXT("."));
					StringCchCat(szDestination, ARRAYSIZE(szDestination), fmt);

					if (m_rip_params->filenames[i]) free(m_rip_params->filenames[i]);
					m_rip_params->filenames[i] = _wcsdup(szDestination);
				
					wchar_t tempFile[MAX_PATH] = {0};
					wchar_t tmppath[MAX_PATH] = {0};
					GetTempPath(MAX_PATH,tmppath);
					GetTempFileName(tmppath,L"rip",0,tempFile);

					m_rip_params->tempFilenames[i] = _wcsdup(tempFile);
					createDirForFileW(m_rip_params->filenames[i]);
					m_fcs.destfile = _wcsdup(tempFile);
					createDirForFileW(m_fcs.destfile);
					m_fcs.callbackhwnd = hwndDlg;
					m_fcs.error = L"";
					m_extract_time = 0;
					m_extract_curtrack = i;

					wchar_t *ptr = m_rip_params->filenames[i];
					if (cchDest && cchDest < (int)lstrlenW(ptr)) ptr += (cchDest + 1);
					SetDlgItemText(hwndDlg, IDC_CURTRACK, ptr);

					if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&m_fcs, IPC_CONVERTFILEW) != 1)
					{
						wchar_t tmp[512] = {0};
						StringCchPrintf(tmp, 512, WASABI_API_LNGSTRINGW(IDS_ERROR_RIPPING_TRACK), i + 1, m_fcs.error ? m_fcs.error : L"");
						MessageBox(hwndDlg, tmp, WASABI_API_LNGSTRINGW(IDS_ERROR), MB_OK);
						done = -1;
						break;
					}
					convertSetPriorityW csp = {
					                             &m_fcs,
					                             g_config->ReadInt(L"extractprio", THREAD_PRIORITY_NORMAL),
					                         };
					SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&csp, IPC_CONVERT_SET_PRIORITYW);
					m_extracting = 1;
					done = 0;
					PostMessage(hwndDlg, WM_WA_IPC , 0, IPC_CB_CONVERT_STATUS);
					break;
				}
			}
			if (done && m_rip_params)
			{
				LockCD(m_rip_params->drive_letter, FALSE);

				if (g_config->ReadInt(L"cdripautoeject", 0) && done > 0)
				{
					char buf[64] = {0};
					StringCchPrintfA(buf, 64, "cda://%c.cda", m_rip_params->drive_letter);
					char buf2[32] = {0};
					getFileInfo(buf, "<eject>", buf2, sizeof(buf2));
				}

				if (g_config->ReadInt(L"cdripautoplay", 0) && done > 0)
				{
					SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);
					for (int i = 0;i < m_rip_params->ntracks;i++)
					{
						if (m_rip_params->tracks[i])
						{
							COPYDATASTRUCT cds;
							cds.dwData = IPC_PLAYFILEW;
							cds.lpData = (void *) m_rip_params->filenames[i];
							cds.cbData = sizeof(wchar_t) * (lstrlenW(m_rip_params->filenames[i]) + 1); // include space for null char
							SendMessage(plugin.hwndWinampParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
						}
					}
					SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY);
				}


				if (m_rip_params->ntracks == m_extract_nb_total && done > 0)
				{
					INT bVal;
					if ((S_OK == Settings_GetBool(C_EXTRACT, EF_CREATEM3U, &bVal) && bVal) ||
						(S_OK == Settings_GetBool(C_EXTRACT, EF_CREATEPLS, &bVal) && bVal))
					{

						wchar_t str[MAX_PATH] = {0}, fmt[MAX_PATH] = {0};
						Settings_ReadString(C_EXTRACT, EF_PLAYLISTFMT, fmt, ARRAYSIZE(fmt));
						Settings_ReadString(C_EXTRACT, EF_PATH, str, ARRAYSIZE(str));

						int l = lstrlenW(str);
						if (l)
							PathAddBackslash(str);

						FormatFileName(str, ARRAYSIZE(str)-5, // ensure we're leaving enough room for the extension
									   fmt, 0xdeadbeef,
						               m_rip_params->artist ? m_rip_params->artist : L"",
									   m_rip_params->album ? m_rip_params->album : L"",
									   NULL,
									   m_rip_params->genre ? m_rip_params->genre : L"",
									   m_rip_params->year ? m_rip_params->year : L"",
									   NULL,
									   NULL,
									   m_rip_params->disc ? m_rip_params->disc : L"");

						if (S_OK == Settings_GetBool(C_EXTRACT, EF_CREATEM3U, &bVal) && bVal)
						{
							wchar_t str2[MAX_PATH] = {0};
							lstrcpynW(str2, str, MAX_PATH);
							StringCchCatW(str2, MAX_PATH, L".m3u");
							createDirForFileW(str2);
							M3UWriter w;
							FILE *fp=_wfopen(str2, L"wt");
							w.Open(fp, AutoCharFn(str2), TRUE);
							BOOL ext;
							Settings_GetBool(C_EXTRACT, EF_USEM3UEXT, &ext);
							for (int i = 0;i < m_rip_params->ntracks;i++)
							{
								if (m_rip_params->tracks[i] && m_rip_params->filenames[i])
								{
									GayString str;
									str.Set(AutoChar(m_rip_params->artist));
									str.Append(" - ");
									str.Append(AutoChar(m_rip_params->tracks[i]));
									if (ext)
										w.SetExtended(AutoCharFn(m_rip_params->filenames[i]), str.Get(), m_rip_params->lengths[i]);
									else
										w.SetFilename(AutoCharFn(m_rip_params->filenames[i]));

								}
							}
							w.Close();
						}

						if (S_OK == Settings_GetBool(C_EXTRACT, EF_CREATEPLS, &bVal) && bVal)
						{
							char str2[MAX_PATH] = {0};
							lstrcpynA(str2, AutoChar(str), MAX_PATH);
							StringCchCatA(str2, MAX_PATH, ".pls");
							createDirForFile(str2);
							// TODO: check for bad unicode conversion
							PLSWriter w;
							w.Open(str2);
							for (int i = 0;i < m_rip_params->ntracks;i++)
							{
								if (m_rip_params->tracks[i] && m_rip_params->filenames[i])
								{
									GayString str;
									str.Set(AutoChar(m_rip_params->artist));
									str.Append(" - ");
									str.Append(AutoChar(m_rip_params->tracks[i]));
									w.SetFilename(AutoCharFn(m_rip_params->filenames[i]));
									w.SetTitle(str.Get());
									w.SetLength(m_rip_params->lengths[i]);
									w.Next();
								}
							}
							w.Close();
						}
					}

					if (S_OK == Settings_GetBool(C_EXTRACT, EF_CREATEMLPL, &bVal) && bVal)
					{
						itemRecordListW irl = {0, };
						allocRecordList(&irl, m_rip_params->ntracks, 0);
						for (int i = 0;i < m_rip_params->ntracks;i++)
						{
							if (m_rip_params->tracks[i])
							{
								int n = irl.Size;
								memset(&irl.Items[n], 0, sizeof(itemRecordW));
								irl.Items[n].filename = _wcsdup(m_rip_params->filenames[i]);
								irl.Items[n].album = _wcsdup(m_rip_params->album);
								irl.Items[n].artist = _wcsdup(m_rip_params->artist);
								irl.Items[n].title = _wcsdup(m_rip_params->tracks[i]);
								irl.Items[n].genre = _wcsdup(m_rip_params->genre);
								irl.Items[n].year = _wtoi(m_rip_params->year);
								irl.Items[n].length = m_rip_params->lengths[i];
								irl.Size++;
							}
						}

						GayString str;
						str.Set(AutoChar(m_rip_params->artist));
						str.Append(" - ");
						str.Append(AutoChar(m_rip_params->album));
						AutoWide name(str.Get());
						mlMakePlaylist pl = {sizeof(mlMakePlaylist), (const wchar_t*)name, ML_TYPE_ITEMRECORDLISTW, (void *) & irl, 0x01};
						SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&pl, ML_IPC_PLAYLIST_MAKE);
						freeRecordList(&irl);
					}
				} // playlist creation

				if (rgThread)
					QueueUserAPC(WriteGain, rgThread, 0);
				else
					PostMessage(m_extract_wnd, WM_APP + 4, 0, 0);
			}
		}
		break;
	case WM_APP + 4:
		if (m_db_has_upd)
		{
			// TODO: benski> does mldb read metadata from this call or the 'add' call - because it won't have replaygain tags until now
			PostMessage(plugin.hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_DB_SYNCDB);
		}

		m_rip_done = 1;
		if (g_config->ReadInt(L"cdripautoclose", 1))
		{
			DestroyWindow(hwndDlg);
		}
		else
		{
			SetWindowText(hwndDlg, WASABI_API_LNGSTRINGW(done > 0 ? IDS_RIP_COMPLETE : IDS_RIP_FAILED));
			SetDlgItemText(hwndDlg, IDC_BUTTON1, WASABI_API_LNGSTRINGW(IDS_CLOSE));
			if (m_hwndstatus)
			{
				SetDlgItemText(m_hwndstatus, IDC_CANCEL_RIP, WASABI_API_LNGSTRINGW(IDS_CLOSE));
			}
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETPOS, 100, 0);
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS2, PBM_SETPOS, 100, 0);


			int now = m_pstat_timedone;

			int bytesout = m_pstat_bytesout;

			double extracted_time = (double) m_pstat_bytesdone * (1.0 / 44100.0 / 4.0);
			if (extracted_time < 1.0) extracted_time = 1.0;

			int br = (int) (((double)bytesout * 8.0 / extracted_time) / 1000.0 + 0.5);

			if (done > 0)
			{
				wchar_t sstr[16] = {0};
				StringCchPrintf(m_last_total_status, 512,
								WASABI_API_LNGSTRINGW(IDS_X_TRACKS_RIPPED_IN_X),
								m_extract_nb_total,
								WASABI_API_LNGSTRINGW_BUF(m_extract_nb_total == 1 ? IDS_TRACK : IDS_TRACKS,sstr,16),
								now / 1000 / 60, (now / 1000) % 60,
								extracted_time / (now / 1000.0),
								br, (double)bytesout * (1.0 / (1024.0*1024.0))
				);
			}
			else
			{
				WASABI_API_LNGSTRINGW_BUF(IDS_RIP_FAILED,m_last_total_status,512);
			}

			SetDlgItemText(hwndDlg, IDC_STATUS2, m_last_total_status);
			SetDlgItemText(hwndDlg, IDC_CURTRACK, WASABI_API_LNGSTRINGW(IDS_COMPLETED));
			SetDlgItemText(hwndDlg, IDC_STATUS, L"");

			if (m_hwndstatus && IsWindow(m_hwndstatus))
			{
				SetDlgItemText(m_hwndstatus, IDC_CDINFO, m_last_total_status);
				SetDlgItemText(m_hwndstatus, IDC_CANCEL_RIP, WASABI_API_LNGSTRINGW(IDS_DONE));
			}
		}
		break;
	case WM_WA_IPC:
		switch (lParam)
		{
		case IPC_CB_CONVERT_STATUS:
			{
				if (!m_extract_time)
				{
					m_extract_time = GetTickCount();
					break;
				}
				DWORD now = GetTickCount() - m_extract_time;
				if (!now) now = 1000; //safety
				wchar_t tmp[512 + 128] = {0};

				{
					int total_t = 0;
					if (wParam) total_t = MulDiv(100, now, (int)wParam);
					int rem_t = total_t - now;

					double extracted_time = (double) m_fcs.bytes_done * (1.0 / 44100.0 / 4.0);
					if (extracted_time < 1.0) extracted_time = 1.0;
					int br = (int) (((double)m_fcs.bytes_out * 8.0 / extracted_time) / 1000.0 + 0.5);

					int estsize = 0;
					if (m_fcs.bytes_total > 0) estsize = MulDiv(m_fcs.bytes_out, m_fcs.bytes_total, m_fcs.bytes_done);

					if (rem_t < 0) rem_t = 0;
					if (total_t < 0) total_t = 0;

					StringCchPrintf(tmp, 640,
									WASABI_API_LNGSTRINGW(IDS_ELAPSED_X_REMAINING_X_TOTAL_X),
									now / 1000 / 60, (now / 1000) % 60,
									rem_t / 1000 / 60, (rem_t / 1000) % 60,
									total_t / 1000 / 60, (total_t / 1000) % 60,
									extracted_time / ((double)now / 1000.0),
									br, (double)estsize * (1.0 / (1024.0*1024.0))
					);
					SetDlgItemText(hwndDlg, IDC_STATUS, tmp);

					if (m_hwndstatus && IsWindow(m_hwndstatus))
					{
						StringCchPrintf(m_last_item_status, 512,
										WASABI_API_LNGSTRINGW(IDS_X_KBPS_AT_X_REALTIME),
										wParam,
										br,
										extracted_time / ((double)now / 1000.0)
						);
						m_statuslist.SetItemText(m_extract_nb, 3, m_last_item_status);
					}
				}

				{
					int total_in_bytes_calc = m_rip_params->total_length_bytes;

					now += m_pstat_timedone;

					int total_t = 0;
					int bytesdone = m_fcs.bytes_done + m_pstat_bytesdone;
					int bytesout = m_fcs.bytes_out + m_pstat_bytesout;
					if (bytesdone) total_t = MulDiv(total_in_bytes_calc, now, bytesdone);

					int rem_t = total_t - now;

					double extracted_time = (double) bytesdone * (1.0 / 44100.0 / 4.0);
					if (extracted_time < 1.0) extracted_time = 1.0;
					int br = (int) (((double)bytesout * 8.0 / extracted_time) / 1000.0 + 0.5);

					int estsize = 0;
					if (total_in_bytes_calc > 0) estsize = MulDiv(bytesout, total_in_bytes_calc, bytesdone);

					if (rem_t < 0) rem_t = 0;
					if (total_t < 0) total_t = 0;

					StringCchPrintf(m_last_total_status, 512,
									WASABI_API_LNGSTRINGW(IDS_X_OF_X_ELAPSED_X_REMAINING_X),
									m_extract_nb + 1, m_extract_nb_total,
									now / 1000 / 60, (now / 1000) % 60,
									rem_t / 1000 / 60, (rem_t / 1000) % 60,
									total_t / 1000 / 60, (total_t / 1000) % 60,
									extracted_time / ((double)now / 1000.0),
									br, (double)estsize * (1.0 / (1024.0*1024.0))
					);

					if (m_hwndstatus && IsWindow(m_hwndstatus))
					{
						SetDlgItemText(m_hwndstatus, IDC_CDINFO, m_last_total_status);
					}
					SetDlgItemText(hwndDlg, IDC_STATUS2, m_last_total_status);

					int a = 0;
					if (total_in_bytes_calc) a = MulDiv(bytesdone, 100, total_in_bytes_calc);
					SendDlgItemMessage(hwndDlg, IDC_PROGRESS2, PBM_SETPOS, a, 0);

					StringCchPrintf(tmp, 640, WASABI_API_LNGSTRINGW(IDS_X_PERCENT_RIPPING_FROM_CD), a);
					SetWindowText(hwndDlg, tmp);
				}

				SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETPOS, wParam, 0);
			}
			break;
		case IPC_CB_CONVERT_DONE:

			SendMessage(plugin.hwndWinampParent , WM_WA_IPC, (WPARAM)&m_fcs, IPC_CONVERTFILEW_END);
			free(m_fcs.destfile);
			m_pstat_bytesdone += m_fcs.bytes_done;
			m_pstat_bytesout += m_fcs.bytes_out;
			if (m_extract_time) m_pstat_timedone += GetTickCount() - m_extract_time;

			CopyFileW(m_rip_params->tempFilenames[m_extract_curtrack], m_rip_params->filenames[m_extract_curtrack], FALSE);
			DeleteFileW(m_rip_params->tempFilenames[m_extract_curtrack]);
			if (AGAVE_API_STATS)
			{
				AGAVE_API_STATS->IncrementStat(api_stats::RIP_COUNT);
				AGAVE_API_STATS->SetStat(api_stats::RIP_FORMAT, m_fcs.destformat[0]);
			}
			wchar_t *lastfn = m_rip_params->filenames[m_extract_curtrack];

			if (g_config->ReadInt(L"extracttag", 1))
			{
				// add metadata to this file
				if (updateFileInfoW(lastfn, L"title", m_rip_params->tracks[m_extract_curtrack]))
				{
					updateFileInfoW(lastfn, L"conductor", m_rip_params->conductors[m_extract_curtrack]);
					updateFileInfoW(lastfn, L"composer", m_rip_params->composers[m_extract_curtrack]);
					updateFileInfoW(lastfn, L"GracenoteFileID", m_rip_params->gracenoteFileIDs[m_extract_curtrack]);
					updateFileInfoW(lastfn, L"GracenoteExtData", m_rip_params->gracenoteExtData[m_extract_curtrack]);
					updateFileInfoW(lastfn, L"artist", m_rip_params->trackArtists[m_extract_curtrack]);
					//if (lstrcmpiW(m_rip_params->trackArtists[m_extract_curtrack], m_rip_params->artist)) // only write albumartist if they're different
					updateFileInfoW(lastfn, L"albumartist", m_rip_params->artist);
					updateFileInfoW(lastfn, L"album", m_rip_params->album);
					updateFileInfoW(lastfn, L"genre", m_rip_params->genre);
					updateFileInfoW(lastfn, L"year", m_rip_params->year);
					updateFileInfoW(lastfn, L"disc", m_rip_params->disc);
					updateFileInfoW(lastfn, L"publisher", m_rip_params->publisher);
					if (m_rip_params->comment && m_rip_params->comment[0])
						updateFileInfoW(lastfn, L"comment", m_rip_params->comment);
					else
					{
						TCHAR szComment[8192] = {0};
						Settings_ReadString(C_EXTRACT, EF_COMMENTTEXT, szComment, ARRAYSIZE(szComment));
						updateFileInfoW(lastfn, L"comment", szComment);
					}

					wchar_t buf[32] = {0};
					if (m_extract_curtrack >= 0)
					{
						if (g_config->ReadInt(L"total_tracks", 0))
							StringCchPrintfW(buf, 32, L"%d/%d", m_extract_curtrack + g_config->ReadInt(L"trackoffs", 1), m_rip_params->ntracks);
						else
							StringCchPrintfW(buf, 32, L"%d", m_extract_curtrack + g_config->ReadInt(L"trackoffs", 1));
					}
					else buf[0] = 0;
					updateFileInfoW(lastfn, L"track", buf);

					if (WASABI_API_APP)
						updateFileInfoW(lastfn, L"tool", WASABI_API_APP->main_getVersionString());
					SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
				}
			}

			if (g_config->ReadInt(L"extractaddml", 1))
			{
				LMDB_FILE_ADD_INFOW fi = {lastfn, -1, -1};
				SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_DB_ADDORUPDATEFILEW, (WPARAM)&fi);
				m_db_has_upd = 1;
			}

			if (m_hwndstatus && IsWindow(m_hwndstatus))
			{
				m_statuslist.SetItemText(m_extract_nb, 3, WASABI_API_LNGSTRING(IDS_WAITING));
			}

			if (rgThread)
				QueueUserAPC(CalculateGain, rgThread, (ULONG_PTR)_wcsdup(lastfn));
			else 
				PostMessage(m_extract_wnd, WM_APP + 3, 0, 0);


			m_extract_nb++;
			m_extracting = 0;
			PostMessage(hwndDlg, WM_APP + 1, 0, 0);
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
		{
			wchar_t title[64] = {0};
			if (m_rip_done || 
				MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_CANCEL_RIP),
						   WASABI_API_LNGSTRINGW_BUF(IDS_CD_RIP_QUESTION,title,64),
						   MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				LockCD(m_rip_params->drive_letter, FALSE);
				DestroyWindow(hwndDlg);
			}
			return 0;
		}
		case IDCANCEL:
			g_config->WriteInt(L"cdripstatuswnd", 0);
			ShowWindow(hwndDlg, SW_HIDE);
			break;
		}
		break;
	case WM_CLOSE:
		return 0;
	case WM_DESTROY:
		if (m_extracting)
		{
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&m_fcs, IPC_CONVERTFILEW_END);
			// make sure we clean up on cancel!
			m_extracting = 0;
			DeleteFileW(m_rip_params->tempFilenames[m_extract_curtrack]);
		}
		if (uMsgRipperNotify) SendNotifyMessage(HWND_BROADCAST, uMsgRipperNotify, (WPARAM)(m_rip_params) ? m_rip_params->drive_letter : 0, (LPARAM)FALSE);
		if (m_rip_params)
		{
			int i;
			for (i = 0; i < m_rip_params->ntracks; i++)
			{
				free(m_rip_params->tracks[i]);
				free(m_rip_params->trackArtists[i]);
				free(m_rip_params->composers[i]);
				free(m_rip_params->gracenoteFileIDs[i]);
				free(m_rip_params->gracenoteExtData[i]);
				free(m_rip_params->conductors[i]);
				free(m_rip_params->filenames[i]);
				free(m_rip_params->tempFilenames[i]);
			}
			free(m_rip_params->gracenoteFileIDs);
			free(m_rip_params->gracenoteExtData);
			free(m_rip_params->composers);
			free(m_rip_params->conductors);
			free(m_rip_params->tracks);
			free(m_rip_params->trackArtists);
			free(m_rip_params->filenames);
			free(m_rip_params->tempFilenames);
			free(m_rip_params->lengths);

			free(m_rip_params->album);
			free(m_rip_params->artist);
			free(m_rip_params->genre);
			free(m_rip_params->year);
			free(m_rip_params->publisher);
			free(m_rip_params->comment);
			free(m_rip_params->disc);

			free(m_rip_params);
			m_rip_params = 0;
		}
		m_extract_wnd = 0;

		if (rgThread)
			QueueUserAPC(CloseGain, rgThread, 0);
		
		break;
	}
	return 0;
}

void cdrip_stop_all_extracts()
{
	if (m_rip_params) LockCD(m_rip_params->drive_letter, FALSE);
	if (m_extract_wnd) DestroyWindow(m_extract_wnd);
	if (m_hwndstatus) DestroyWindow(m_hwndstatus);
}

int cdrip_isextracting(char drive)
{
	if (!m_rip_params) return 0;
	if (drive == -1 && m_rip_done)
	{
		if (m_extract_wnd && IsWindow(m_extract_wnd)) DestroyWindow(m_extract_wnd);
		if (m_hwndstatus && IsWindow(m_hwndstatus)) DestroyWindow(m_hwndstatus);
		return 0;
	}
	if (drive == 0 || drive == -1) return toupper(m_rip_params->drive_letter);
	return toupper(m_rip_params->drive_letter) == toupper(drive);
}

HWND cdrip_FindBurningHWND(char cLetter)
{
	HWND h = 0;
	while (NULL != (h = FindWindowExW(NULL, h, L"#32770", NULL)))
	{
		if (!GetPropW(h, L"WARIPPER")) continue;
		if (((char)(INT_PTR)GetPropW(h, L"DRIVE")) == cLetter) return h;
	}
	return NULL;
}

void cdrip_extractFiles(cdrip_params *parms)
{
	WASABI_API_LNGSTRINGW_BUF(IDS_INITIALIZING,m_last_total_status,512);
	m_last_item_status[0] = 0;
	m_rip_params = parms;
	WASABI_API_CREATEDIALOGW(IDD_VIEW_CDROM_EXTRACT, plugin.hwndWinampParent, extract_dialogProc);
}