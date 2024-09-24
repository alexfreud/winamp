#include <shlwapi.h>

#include "main.h"
#include <windowsx.h>
#include "resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ChildSizer.h"
#include "../winamp/wa_ipc.h"
#include "../Winamp/strutil.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/listview.h"
#include <strsafe.h>

#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER		0x00010000
#endif

static INT_PTR WINAPI DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define TIMER_NOTIFYINFO_ID		1985
#define TIMER_NOTIFYINFO_DELAY	200

HWND CreateCDViewWindow(HWND hwndParent, DM_NOTIFY_PARAM *phdr)
{
	return WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_CDROM, hwndParent, DlgProc, (LPARAM)phdr);
}

void TAG_FMT(void *f, void *ff, void *p, char *out, int out_len)
{
	waFormatTitle fmt;
	fmt.out = out;
	fmt.out_len = out_len;
	fmt.p = p;
	fmt.spec = 0;
	*(void **)&fmt.TAGFUNC = f;
	*(void **)&fmt.TAGFREEFUNC = ff;
	*out = 0;
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE);
}


static wchar_t m_artist[128], m_album[128], m_genre[128], m_year[32];
static int m_start_immediate_extract = 0;
static int m_atracks, m_dtracks;


void CopyComment(wchar_t *&dest, wchar_t *comments)
{
	if (comments)
	{
		int numCarriageReturns = 0;
		wchar_t *commentCursor = comments;
		while (commentCursor && *commentCursor)
		{
			if (*commentCursor == '\r')
			{
				commentCursor++;
				if (*commentCursor == '\n')
					commentCursor++;
			}
			if (*commentCursor == '\n')
				numCarriageReturns++;

			commentCursor = CharNext(commentCursor);
		}
		size_t size = commentCursor - comments;
		dest = (wchar_t *)calloc((size + numCarriageReturns + 1), sizeof(wchar_t));
		wchar_t *destCursor = dest;
		commentCursor = comments;
		while (commentCursor && *commentCursor)
		{
			if (*commentCursor == '\r')
			{
				*destCursor++ = *commentCursor++;
				if (*commentCursor == '\n')
					*destCursor++ = *commentCursor++;
			}
			if (*commentCursor == '\n')
				*destCursor++ = '\r';

			wchar_t *next = CharNextW(commentCursor);
			while (commentCursor != next)
			{
				*destCursor++  = *commentCursor++;
			}
		}
		*destCursor = 0;
	}
	else
		dest = 0;
}

static void extractFiles(HWND hwndDlg, CHAR cLetter, int all)
{
	HWND hwndList;
	hwndList = GetDlgItem(hwndDlg, IDC_LIST2);

	char cMode;
	wchar_t title[32] = {0}, buf[512] = {0};
	INT msgTextId, l, i, cnt = 0;
	cdrip_params *p;
	wchar_t info[65536] = {0};

	LVITEMW lvitem = {0};

	cMode = DriveManager_GetDriveMode(cLetter);

	switch (cMode)
	{
	case DM_MODE_BURNING:	msgTextId = IDS_ERROR_CD_RIP_IN_PROGRESS; break;
	case DM_MODE_RIPPING:	msgTextId = IDS_ERROR_CD_BURN_IN_PROGRESS; break;
	default:				msgTextId = 0; break;
	}
	if (msgTextId)
	{
		MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(msgTextId), WASABI_API_LNGSTRINGW_BUF(IDS_CD_RIP,title,32), 0);
		return ;
	}	

	if (m_dtracks && !m_atracks)
	{
		MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_ERROR_CANNOT_EXTRACT_DATA_CDS),
		            WASABI_API_LNGSTRINGW_BUF(IDS_CD_RIP,title,32), 0);
		return ;
	}

	l = (hwndList) ? ListView_GetItemCount(hwndList) : 0;
	if (l)
	{
		p = (cdrip_params *)calloc(1, sizeof(cdrip_params));
		if (!p) return;
		p->ntracks = l;
		p->tracks = (wchar_t **)calloc(sizeof(wchar_t*), p->ntracks);
		p->trackArtists = (wchar_t **)calloc(sizeof(wchar_t*), p->ntracks);
		p->composers = (wchar_t **)calloc(sizeof(wchar_t*), p->ntracks);
		p->gracenoteFileIDs = (wchar_t **)calloc(sizeof(wchar_t*), p->ntracks);
		p->gracenoteExtData = (wchar_t **)calloc(sizeof(wchar_t*), p->ntracks);
		p->conductors = (wchar_t **)calloc(sizeof(wchar_t*), p->ntracks);
		p->lengths = (int *)calloc(sizeof(int), p->ntracks);
	}
	else p = NULL;

	lvitem.mask = LVIF_TEXT;
	lvitem.iSubItem = 3;
	lvitem.cchTextMax = sizeof(buf)/sizeof(char);

	for (i = 0;i < l;i++)
	{
		if (all || (LVIS_SELECTED & ListView_GetItemState(hwndList, i, LVIS_SELECTED)))
		{
			wchar_t cdFilename[MAX_PATH] = {0};
			StringCchPrintfW(cdFilename, MAX_PATH, L"cda://%c,%d.cda", cLetter, i + 1);
			//check if track is Data track
			{
				wchar_t buf2[512] = L"";
				getFileInfoW(cdFilename, L"tracktype", buf2, sizeof(buf2)/sizeof(*buf2));
				if (lstrcmpiW(buf2, L"audio")) continue; //skip it
			}

			lvitem.iItem = i;
			lvitem.pszText = buf;
			SendMessageW(hwndList, LVM_GETITEMTEXTW, i, (LPARAM)&lvitem);
			int len = _wtoi(lvitem.pszText) * 60 + _wtoi(wcsstr(lvitem.pszText, L":") + 1); //such hackish :)

			p->total_length_bytes += len * 44100 * 4;

			getFileInfoW(cdFilename, L"title", info, sizeof(info)/sizeof(wchar_t));
			p->tracks[i] = _wcsdup(info);

			getFileInfoW(cdFilename, L"artist", info, sizeof(info)/sizeof(wchar_t));
			p->trackArtists[i] = _wcsdup(info);

			getFileInfoW(cdFilename, L"composer", info, sizeof(info)/sizeof(wchar_t));
			p->composers[i] = _wcsdup(info);

			getFileInfoW(cdFilename, L"conductor", info, sizeof(info)/sizeof(wchar_t));
			p->conductors[i] = _wcsdup(info);

			getFileInfoW(cdFilename, L"GracenoteFileID", info, sizeof(info)/sizeof(wchar_t));
			p->gracenoteFileIDs[i] = _wcsdup(info);

			getFileInfoW(cdFilename, L"GracenoteExtData", info, sizeof(info)/sizeof(wchar_t));
			p->gracenoteExtData[i] = _wcsdup(info);

			p->lengths[i] = len;
			cnt++;
		}
	}
	if (!cnt)
	{
		if (p)
		{
			for (int i = 0 ; i < l; i ++) free(p->tracks[i]);
			free(p->tracks);
			free(p->trackArtists);
			free(p->composers);
			free(p->gracenoteFileIDs);
			free(p->gracenoteExtData);
			free(p->conductors);
			free(p->lengths);
			free(p);
		}
		MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_NO_TRACKS_TO_RIP),
		           WASABI_API_LNGSTRINGW_BUF(IDS_CD_RIP,title,32), MB_OK);
		return ;
	}

	p->filenames = (wchar_t **)calloc(sizeof(wchar_t *), p->ntracks); // allocate for cdrip to use :)
	p->tempFilenames = (wchar_t **)calloc(sizeof(wchar_t *), p->ntracks); // allocate for cdrip to use :)
	p->artist = _wcsdup(m_artist);
	p->album = _wcsdup(m_album);
	p->genre = _wcsdup(m_genre);
	p->year = _wcsdup(m_year);

	wchar_t name[] = L"cda://X.cda";
	name[6] = cLetter;

	info[0] = 0;
	getFileInfoW(name, L"publisher", info, sizeof(info)/sizeof(wchar_t));
	p->publisher = _wcsdup(info);

	info[0] = 0;
	getFileInfoW(name, L"comment", info, sizeof(info)/sizeof(wchar_t));
	CopyComment(p->comment, info);

	info[0] = 0;
	getFileInfoW(name, L"disc", info, sizeof(info)/sizeof(wchar_t));
	p->disc = _wcsdup(info);

	p->drive_letter = cLetter;

	cdrip_extractFiles(p); // will free p when done with it
}

static void playFiles(HWND hwndDlg, CHAR cLetter, int enqueue, int all)
{
	HWND hwndList;
	hwndList = GetDlgItem(hwndDlg, IDC_LIST2);
	CHAR  cMode;
	INT msgTextId;

	cMode = DriveManager_GetDriveMode(cLetter);

	switch (cMode)
	{
	case DM_MODE_BURNING:	msgTextId = IDS_ERROR_CD_RIP_IN_PROGRESS; break;
	case DM_MODE_RIPPING:	msgTextId = IDS_ERROR_CD_BURN_IN_PROGRESS; break;
	default:				msgTextId = 0; break;
	}
	if (msgTextId)
	{
		wchar_t title[64] = {0};
		MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(msgTextId),
		           WASABI_API_LNGSTRINGW_BUF(IDS_CD_PLAYBACK_ERROR, title, 64), 0);
		return ;
	}

	int cnt = 0;
	int l = (hwndList) ? ListView_GetItemCount(hwndList) : 0;
	if (enqueue && all == 1024) all = 0;

	int firstsel = -1;
	char buf[64] = {0}, titlebuf[2048] = {0};
	enqueueFileWithMetaStruct s;
	LVITEMA lvitem = {0};

	lvitem.cchTextMax = sizeof(buf)/sizeof(char);

	for (int i = 0;i < l;i++)
	{
		if (all == 1024 && firstsel < 0 && (LVIS_SELECTED & ListView_GetItemState(hwndList, i, LVIS_SELECTED))) firstsel = i;

		if (all || (LVIS_SELECTED & ListView_GetItemState(hwndList, i, LVIS_SELECTED)))
		{
			lvitem.iItem = i;

			lvitem.mask = LVIF_PARAM;
			lvitem.iSubItem = 0;
			SendMessageW(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvitem);
			int a = (INT)(INT_PTR)lvitem.lParam;
			if (a > 0)
			{
				if (!cnt)
				{
					if (!enqueue) SendMessageW(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);
					cnt++;
				}

				lvitem.mask = LVIF_TEXT;
				lvitem.iSubItem = 3;
				lvitem.pszText = buf;
				SendMessageW(hwndList, LVM_GETITEMTEXTA, i, (LPARAM)&lvitem);

				s.length = atoi(lvitem.pszText) * 60 + atoi(strstr(lvitem.pszText, ":") + 1); //such hackish :)

				StringCchPrintfA(buf, 64, "cda://%c,%d.cda", cLetter, i + 1);
				TAG_FMT(0, 0, buf, titlebuf, sizeof(titlebuf)/sizeof(*titlebuf));
				s.filename = buf;
				s.title    = titlebuf;
				s.ext      = NULL;
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILE);
			}
		}
	}
	if (cnt && !enqueue)
	{
		if (firstsel >= 0)
		{
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, firstsel, IPC_SETPLAYLISTPOS);
			SendMessage(plugin.hwndWinampParent, WM_COMMAND, 40047, 0); // stop button, literally
			SendMessage(plugin.hwndWinampParent, WM_COMMAND, 40045, 0); // play button, literally
		}
		else SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY);
	}
}

void saveCDToItemRecordList(CHAR cLetter, itemRecordList *obj, char *albumname)
{
	char fname[64] = {0}, buf2[64] = {0};
	StringCchPrintfA(fname, 64, "cda://%c.cda", cLetter);

	getFileInfo(fname, "<begin>", buf2, sizeof(buf2)/sizeof(char));
	getFileInfo(fname, "ntracks", buf2, sizeof(buf2)/sizeof(char));

	int ntracks = atoi(buf2);

	if (ntracks > 0 && ntracks < 256)
	{
		obj->Items = 0; obj->Alloc = 0; obj->Size = 0;
		allocRecordList(obj, ntracks, 0);

		int x;
		for (x = 0; x < ntracks; x ++)
		{
			StringCchPrintfA(fname, 64, "cda://%c,%d.cda", cLetter, x + 1);
			getFileInfo(fname, "tracktype", buf2, sizeof(buf2)/sizeof(char));
			if (!lstrcmpiA(buf2, "audio"))
			{
				int len = -1;
				char titlebuf[FILETITLE_SIZE] = {0};
				mediaLibrary.GetFileInfo(fname, titlebuf, FILETITLE_SIZE, &len);
				itemRecord *pRec = &obj->Items[obj->Size];
				ZeroMemory(pRec, sizeof(itemRecord));
				if (titlebuf) pRec->title = _strdup(titlebuf);
				pRec->length = len;
				if (fname) pRec->filename = _strdup(fname);
				obj->Size++;
			}
		}
	}
	getFileInfo(fname, "<end>", buf2, sizeof(buf2)/sizeof(char));
}




static ChildWndResizeItem cdromwnd_rlist[] =
{
	{IDC_LIST2, 0x0011},
	{IDC_CDINFO, 0x0000},
	{IDC_CDINFO2, 0x0000},
	{IDC_BUTTON_PLAY, 0x0101},
	{IDC_BUTTON_ENQUEUE, 0x0101},
	{IDC_BUTTON_EXTRACT, 0x0101},
	{IDC_BUTTON_EJECT, 0x0101},
	{IDC_BTN_SHOWINFO, 0x1111},
};

typedef struct _VIEWCOLUMN
{
	INT		stringId;
	LPTSTR	pszConfig;
	INT		defaultWidth;
} VIEWCOLUMN;

static VIEWCOLUMN  viewColumns[] =
{
	{ IDS_TRACK_NUMBER, TEXT("col_track"), 60 },
	{ IDS_ARTIST, TEXT("col_artist"), 150 },
	{ IDS_TITLE, TEXT("col_title"), 200 },
	{ IDS_LENGTH, TEXT("col_len"), 80 },
};

static char m_cdrom;

typedef struct _APCPARAM
{
	HWND			hwndDlg;
	CHAR		cLetter;
	INT_PTR		user;
}
APCPARAM;

static void CALLBACK APC_GetCracenoteInfo(ULONG_PTR param);

static void GetGracenoteInfo(HWND hwndDlg, CHAR cLetter, HANDLE hThread = NULL)
{
	HWND hwndList;
	int l, x;
	LVITEMW lvitem = {0};
	wchar_t buf[32] = {0}, titlebuf[256] = {0};

	if (hThread)
	{
		APCPARAM *pParam = (APCPARAM*)calloc(1, sizeof(APCPARAM));
		if (pParam)
		{
			pParam->cLetter = cLetter;
			pParam->hwndDlg = hwndDlg;
			if (!QueueUserAPC(APC_GetCracenoteInfo, hThread, (ULONG_PTR)pParam)) free(pParam);
		}
		return;
	}

	hwndList = GetDlgItem(hwndDlg, IDC_LIST2);
	if (!hwndList) return;

	l = (INT)SendMessageW(hwndList, LVM_GETITEMCOUNT, 0,0);

	// first, let's try to get artist/album info
	{
		StringCchPrintfW(buf, 32, L"cda://%c.cda", cLetter);
		wchar_t artistbuf[256] = {0}, albumbuf[256] = {0}, yearbuf[256] = {0}, genrebuf[256] = {0};
		getFileInfoW(buf, L"albumartist", artistbuf, sizeof(artistbuf) / sizeof(artistbuf[0]));
		getFileInfoW(buf, L"album", albumbuf, sizeof(albumbuf) / sizeof(albumbuf[0]));
		getFileInfoW(buf, L"genre", genrebuf, sizeof(genrebuf) / sizeof(genrebuf[0]));
		getFileInfoW(buf, L"year", yearbuf, sizeof(yearbuf) / sizeof(yearbuf[0]));

		wchar_t newbuf[1024] = {0};
		lstrcpynW(m_artist, artistbuf, sizeof(m_artist)/sizeof(*m_artist));
		lstrcpynW(m_album, albumbuf, sizeof(m_album)/sizeof(*m_album));
		lstrcpynW(m_year, yearbuf, sizeof(m_year)/sizeof(*m_year));
		lstrcpynW(m_genre, genrebuf, sizeof(m_genre)/sizeof(*m_genre));
		StringCchPrintfW(newbuf, 1024, WASABI_API_LNGSTRINGW(IDS_ML_VIEW_ARTIST_ALBUM), artistbuf, albumbuf);
		SetDlgItemText(hwndDlg, IDC_CDINFO, newbuf);
		StringCchPrintfW(newbuf, 1024, WASABI_API_LNGSTRINGW(IDS_ML_VIEW_YEAR_GENRE), yearbuf, genrebuf);
		SetDlgItemText(hwndDlg, IDC_CDINFO2, newbuf);
	}

	for (x = 0; x < l; x ++)
	{
		lvitem.iItem = x;
		lvitem.mask = LVIF_PARAM;
		lvitem.iSubItem = 0;
		SendMessageW(hwndList, LVM_GETITEMW, 0, (LPARAM)&lvitem);

		int wt = (INT)(INT_PTR)lvitem.lParam;

		if (wt > 0)
		{
			StringCchPrintfW(buf, 32, L"cda://%c,%d.cda", cLetter, x + 1);

			lvitem.mask = LVIF_TEXT;

			titlebuf[0] = 0;
			getFileInfoW(buf, L"title", titlebuf, sizeof(titlebuf) / sizeof(titlebuf[0]));
			if (titlebuf[0])
			{
				lvitem.iSubItem = 2;
				lvitem.pszText = titlebuf;
				SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvitem);
			}

			titlebuf[0] = 0;
			getFileInfoW(buf, L"artist", titlebuf, sizeof(titlebuf) / sizeof(titlebuf[0]));
			if (titlebuf[0])
			{
				lvitem.iSubItem = 1;
				lvitem.pszText = titlebuf;
				SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvitem);
			}
		}
	}

	if (l && hwndList)
		UpdateWindow(hwndList);

	if ( Plugin_IsExtractScheduled( cLetter ) )
		extractFiles( hwndDlg, cLetter, 1 );
}

static void UpdateCDView(HWND hwndDlg, DM_NOTIFY_PARAM *phdr)
{
	HWND hwndList;

	m_cdrom	= phdr->cLetter;
	m_atracks = m_dtracks = 0;

	hwndList = GetDlgItem(hwndDlg, IDC_LIST2);
	if (!hwndList) return;

	ListView_DeleteAllItems(hwndList);

	SetDlgItemText(hwndDlg, IDC_CDINFO2, L"");

	SendMessageW(hwndList, WM_SETREDRAW, FALSE, 0L);

	if (DMOP_MCIINFO == phdr->opCode)
	{
		DM_MCI_PARAM *pmci;
		LVITEMW lvitem = {0};
		wchar_t buffer[512] = {0};
		INT strid, param;
		pmci = (DM_MCI_PARAM*)phdr;

		for (int i = 0; i < pmci->nTracks; i++)
		{
			INT time = (0x7FFFFFFF & pmci->pTracks[i])/1000;

			if (0x80000000 & pmci->pTracks[i])
			{
				param = i + 1; strid = IDS_AUDIO_TRACK; m_atracks++;
			}
			else
			{
				param = -1; strid = IDS_DATA_TRACK; m_dtracks++;
			}

			StringCchPrintfW(buffer, sizeof(buffer)/sizeof(wchar_t), L"%d", i + 1);

			lvitem.mask		= LVIF_TEXT | LVIF_PARAM;
			lvitem.iItem		= i;
			lvitem.iSubItem	= 0;
			lvitem.pszText	= buffer;
			lvitem.lParam	= param;
			INT index = (INT)SendMessageW(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvitem);

			if (-1 != index)
			{
				lvitem.iItem = index;
				lvitem.mask = LVIF_TEXT;

				lvitem.iSubItem = 2;
				lvitem.pszText = WASABI_API_LNGSTRINGW(strid);
				SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvitem);

				if (time < 0) StringCchCopyW(buffer, sizeof(buffer)/sizeof(wchar_t), L"???");
				else StringCchPrintfW(buffer, sizeof(buffer)/sizeof(wchar_t), L"%d:%02d", time / 60, time % 60);

				lvitem.iSubItem = 3;
				lvitem.pszText = buffer;
				SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvitem);
			}
		}

		SetDlgItemText(hwndDlg, IDC_CDINFO, WASABI_API_LNGSTRINGW((m_atracks) ? IDS_CD_AUDIO : ((m_dtracks) ? IDS_DATA_CD : IDS_NO_CD)));

		if (m_atracks) GetGracenoteInfo(hwndDlg, phdr->cLetter, GetCurrentThread());
	}
	else SetDlgItemText(hwndDlg, IDC_CDINFO, WASABI_API_LNGSTRINGW(IDS_NO_CD));

	SendMessageW(hwndList, WM_SETREDRAW, TRUE, 0L);
	UpdateWindow(hwndList);
}

static LRESULT editCDInfo(HWND hwndDlg, CHAR cLetter, int trackNum)
{
	wchar_t name[MAX_PATH] = {0};
	if (trackNum)
		StringCchPrintfW(name, MAX_PATH, L"cda://%c,%d", cLetter, trackNum);
	else
		StringCchPrintfW(name, MAX_PATH, L"cda://%c.cda", cLetter);
	infoBoxParamW p;
	p.filename = name;
	p.parent = hwndDlg;
	return SendMessageW(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&p, IPC_INFOBOXW);
}

static void NotifyInfoWindow(HWND hwnd, CHAR cLetter, INT nTrack, BOOL bForceRefresh)
{
	HWND hwndParent;
	hwndParent = GetParent(hwnd);

	if (hwndParent)
	{
		wchar_t szFileName[MAX_PATH], *p;
		if (nTrack && S_OK == StringCchPrintfW(szFileName, sizeof(szFileName)/sizeof(wchar_t), L"cda://%c,%d.cda", cLetter, nTrack)) p = szFileName;
		else p = L"";
		SendMessageW(hwndParent, WM_SHOWFILEINFO, (WPARAM)((bForceRefresh) ? WISF_FORCE : WISF_NORMAL), (LPARAM)p);
	}
}

static BOOL Window_OnInitDialog(HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
	HWND hwndList;

	if (!lParam)
	{
		DestroyWindow(hwndDlg);
		return 0;
	}

	SendMessageW(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(IDC_BTN_SHOWINFO, BN_EX_GETTEXT), (LPARAM)GetDlgItem(hwndDlg, IDC_BTN_SHOWINFO));

	childSizer.Init(hwndDlg, cdromwnd_rlist, sizeof(cdromwnd_rlist) / sizeof(cdromwnd_rlist[0]));

	hwndList = GetDlgItem(hwndDlg, IDC_LIST2);

	if (hwndList)
	{
		MLSKINWINDOW sw;
		LVCOLUMNW column;

		sw.hwndToSkin = hwndList;
		sw.skinType = SKINNEDWND_TYPE_LISTVIEW;
		sw.style = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
		MLSkinWindow(plugin.hwndLibraryParent, &sw);

		column.mask = LVCF_WIDTH | LVCF_TEXT;
		for (int i = 0; i < sizeof(viewColumns)/sizeof(VIEWCOLUMN); i++)
		{
			column.cx = g_view_metaconf->ReadInt(viewColumns[i].pszConfig, viewColumns[i].defaultWidth);
			column.pszText = WASABI_API_LNGSTRINGW(viewColumns[i].stringId);
			SendMessageW(hwndList, LVM_INSERTCOLUMNW, (WPARAM)0xEFFF, (LPARAM)&column);
		}
	}


	SetDlgItemText(hwndDlg, IDC_CDINFO, L"");
	SetDlgItemText(hwndDlg, IDC_CDINFO2, L"");

	NotifyInfoWindow(hwndDlg, ((DM_NOTIFY_PARAM*)lParam)->cLetter, NULL, TRUE); // ignore cache

	UpdateCDView(hwndDlg, (DM_NOTIFY_PARAM*)lParam);

	return FALSE;
}
static void Window_OnDestroy(HWND hwndDlg)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST2);

	if (hwndList)
	{
		for (int i = 0; i < sizeof(viewColumns)/sizeof(VIEWCOLUMN); i++)
		{
			g_view_metaconf->WriteInt(viewColumns[i].pszConfig,
			                          (INT)SendMessageW(hwndList, LVM_GETCOLUMNWIDTH, i, 0L));
		}
	}

	if (m_cdrom) NotifyInfoWindow(hwndDlg, m_cdrom, NULL, FALSE);

}


static void Window_OnSize(HWND hwndDlg, UINT nType, INT cx, INT cy)
{
	if (nType != SIZE_MINIMIZED)
	{
		childSizer.Resize(hwndDlg, cdromwnd_rlist, sizeof(cdromwnd_rlist) / sizeof(cdromwnd_rlist[0]));
		InvalidateRect(hwndDlg, NULL, TRUE);
	}
}

static void CALLBACK Window_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	HWND hwndList;

	switch (idEvent)
	{
	case TIMER_NOTIFYINFO_ID:
		KillTimer(hwnd, TIMER_NOTIFYINFO_ID);
		hwndList = GetDlgItem(hwnd, IDC_LIST2);
		NotifyInfoWindow(hwnd, m_cdrom,
		                 (hwndList) ? (INT)SendMessage(hwndList, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_FOCUSED) + 1: 0,
		                 FALSE);
		break;
	}
}

static void Window_OnCommand(HWND hwndDlg, INT eventId, INT ctrlId, HWND hwndCtrl)
{
	switch (ctrlId)
	{
	case IDC_BUTTON_ENQUEUE:
	case IDC_BUTTON_PLAY:
	{
		HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST2);
		if (hwndList)
		{
			int selcnt = (INT)SendMessageW(hwndList, LVM_GETSELECTEDCOUNT, 0, 0L);
			playFiles(hwndDlg, m_cdrom, (IDC_BUTTON_ENQUEUE == ctrlId), (selcnt) ? ((selcnt == 1) ? 1024 : 0) : 1);
		}
	}
	break;
	case IDC_BUTTON_EXTRACT:
	{
		RECT r;
		GetWindowRect(hwndCtrl, &r);
		int x = Menu_TrackPopup(plugin.hwndLibraryParent, GetSubMenu(g_context_menus, 0),
								TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN |
								TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
								r.left, r.top, hwndDlg, NULL);
		switch (x)
		{
		case ID_EXTRACTMENU_EXTRACTSELECTEDTRACKS:	extractFiles(hwndDlg, m_cdrom, 0); break;
		case ID_EXTRACTMENU_EXTRACTALLTRACKS:		extractFiles(hwndDlg, m_cdrom, 1); break;
		case ID_EXTRACTMENU_CONFIGURE:				Plugin_ShowRippingPreferences(); break;
		}
		UpdateWindow(hwndDlg);
		Sleep(100);
		MSG msg;
		while (PeekMessageW(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
	}
	break;
	case IDC_BUTTON_EJECT:
	{
		wchar_t result[32] = {0};
		wchar_t name[] = L"cda://X.cda";

		name[6] = m_cdrom;
		getFileInfoW(name, L"<eject>", result, sizeof(result)/sizeof(wchar_t));
	}
	break;
	case IDC_BTN_SHOWINFO:
		switch (eventId)
		{
		case BN_CLICKED:
			SendMessageW(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(ctrlId, eventId), (LPARAM)hwndCtrl);
			break;
		}
		break;
	}
}
static void ListView_OnItemChanged(HWND hwndDlg, NMLISTVIEW *pnmv)
{
	if (LVIF_STATE & pnmv->uChanged)
	{
		if ((LVIS_FOCUSED & pnmv->uOldState) != (LVIS_FOCUSED & pnmv->uNewState))
		{
			KillTimer(hwndDlg, TIMER_NOTIFYINFO_ID);
			SetTimer(hwndDlg, TIMER_NOTIFYINFO_ID, TIMER_NOTIFYINFO_DELAY, Window_TimerProc);

		}
	}
}
static INT_PTR Window_OnNotify(HWND hwndDlg, INT ctrlId, LPNMHDR phdr)
{
	switch (phdr->idFrom)
	{
	case IDC_LIST2:
		switch (phdr->code)
		{
		case LVN_ITEMCHANGED: ListView_OnItemChanged(hwndDlg, (NMLISTVIEW*)phdr); break;
		case LVN_KEYDOWN:
		{
			LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)phdr;
			switch (pnkd->wVKey)
			{
			case '3':
				if (GetAsyncKeyState(VK_MENU)&0x8000)
				{
					W_ListView view(GetDlgItem(hwndDlg, IDC_LIST2));
					if (view.GetSelectedCount() == 0 || view.GetSelectedCount() == view.GetCount())
						editCDInfo(hwndDlg, m_cdrom, 0);
					
					int sel =-1;
					while ((sel = view.GetNextSelected(sel)) != -1)
					{
						if (editCDInfo(hwndDlg, m_cdrom, sel+1) == 1)
							break;
					}


					PostMessageW(hwndDlg, WM_NEXTDLGCTL, (WPARAM)phdr->hwndFrom, TRUE);
				}
				break;
			case 'A':
				if (GetAsyncKeyState(VK_CONTROL)) ListView_SetItemState(phdr->hwndFrom, -1, LVIS_SELECTED, LVIS_SELECTED);
				break;
			}
		}
		break;
		case NM_DBLCLK:
			playFiles(hwndDlg, m_cdrom, (!!g_config->ReadInt(L"enqueuedef", 0)) ^(!!(GetAsyncKeyState(VK_SHIFT)&0x8000)), 1024);
			break;
		case LVN_BEGINDRAG:
			SetCapture(hwndDlg);
			break;
		case NM_RETURN:
			SendMessageW(hwndDlg, WM_COMMAND, ((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^(!!g_config->ReadInt(L"enqueuedef", 0)))
			             ? IDC_BUTTON_ENQUEUE : IDC_BUTTON_PLAY, 0);
			break;
		}
		break;
	}
	return 0;
}

static void Window_OnMouseMove(HWND hwndDlg, INT vKey, POINTS pts)
{
	mlDropItemStruct m = {0};
	if (GetCapture() != hwndDlg) return;

	POINTSTOPOINT(m.p, pts);
	MapWindowPoints(hwndDlg, HWND_DESKTOP, &m.p, 1);

	m.type = ML_TYPE_CDTRACKS;

	SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_HANDLEDRAG, (WPARAM)&m);

}

static void Window_OnLButtonUp(HWND hwndDlg, INT vKey, POINTS pts)
{
	mlDropItemStruct m = {0};

	if (GetCapture() != hwndDlg) return;

	ReleaseCapture();

	m.type = ML_TYPE_CDTRACKS;
	m.flags = ML_HANDLEDRAG_FLAG_NOCURSOR;

	POINTSTOPOINT(m.p, pts);
	MapWindowPoints(hwndDlg, HWND_DESKTOP, &m.p, 1);

	SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_HANDLEDRAG, (WPARAM)&m);

	if (m.result > 0)
	{
		HWND hwndList;
		LVITEMW lvitem = {0};
		int i, l, len;
		itemRecordList myObj = {0};
		char trackname[] = "cda://X,%d.cda";
		char name[32] = {0}, total[512] = {0};

		hwndList = GetDlgItem(hwndDlg, IDC_LIST2);

		l = (hwndList) ? (INT)SendMessageW(hwndList, LVM_GETITEMCOUNT, 0,0) : 0;
		if (l > 256) l = 256;


		allocRecordList(&myObj, l, 0);

		lvitem.mask = LVIF_PARAM;
		lvitem.iSubItem = 0;

		trackname[6] = m_cdrom;

		for (i = 0; i < l; i++)
		{
			lvitem.iItem = i;
			SendMessageW(hwndList, LVM_GETITEMW, 0, (LPARAM)&lvitem);
			int p = (INT)(INT_PTR)lvitem.lParam;

			if ((LVIS_SELECTED & SendMessageW(hwndList, LVM_GETITEMSTATE, i, LVIS_SELECTED)) && p > 0)
			{

				StringCchPrintfA(name, sizeof(name)/sizeof(char), trackname, p);

				total[0] = 0;
				mediaLibrary.GetFileInfo(name, total, sizeof(total)/sizeof(char), &len);
				memset(myObj.Items + myObj.Size, 0, sizeof(itemRecord));
				myObj.Items[myObj.Size].filename = _strdup(name);
				myObj.Items[myObj.Size].length = len;
				myObj.Items[myObj.Size++].title = _strdup(total);
			}
		}
		if (myObj.Size)
		{
			m.flags = 0;
			m.result = 0;
			m.data = (void*)&myObj;
			SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_HANDLEDROP, (WPARAM)&m);
		}

		freeRecordList(&myObj);
	}
}



static void Window_OnQueryInfo(HWND hwnd)
{
	KillTimer(hwnd, TIMER_NOTIFYINFO_ID);
	NotifyInfoWindow(hwnd, m_cdrom, NULL, TRUE);
	SetTimer(hwnd, TIMER_NOTIFYINFO_ID, TIMER_NOTIFYINFO_DELAY, Window_TimerProc);
}


static void Window_OnFileTagUpdated(HWND hwnd, CHAR cLetter, LPCWSTR pszFileName)
{
	INT len, lcid;

	len = (pszFileName) ? lstrlenW(pszFileName) : 0;
	if (len < 7) return;
	lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	if (CSTR_EQUAL == CompareStringW(lcid, 0, L"cda://", 6, pszFileName, 6) && pszFileName[6] == cLetter)
	{
		GetGracenoteInfo(hwnd, cLetter, GetCurrentThread());
	}
}

static void Window_OnContextMenu(HWND hwndDlg, HWND hwndFrom, int x, int y)
{
	POINT pt = {x,y};
	W_ListView view(GetDlgItem(hwndDlg, IDC_LIST2));

	if(view.GetCount() == 0) return;

	if (x == -1 || y == -1) // x and y are -1 if the user invoked a shift-f10 popup menu
	{
		RECT itemRect = {0};
		int selected = view.GetNextSelected();
		if (selected != -1) // if something is selected we'll drop the menu from there
		{
			view.GetItemRect(selected, &itemRect);
			ClientToScreen(view.getwnd(), (POINT *)&itemRect);
		}
		else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
		{
			GetWindowRect(view.getwnd(), &itemRect);

			HWND hHeader = (HWND)SNDMSG(hwndFrom, LVM_GETHEADER, 0, 0L);
			RECT headerRect;
			if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
			{
				itemRect.top += (headerRect.bottom - headerRect.top);
			}
		}
		x = itemRect.left;
		y = itemRect.top;
	}

	HWND hHeader = (HWND)SNDMSG(hwndFrom, LVM_GETHEADER, 0, 0L);
	RECT headerRect;
	if (0 == (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) || FALSE == GetWindowRect(hHeader, &headerRect))
	{
		SetRectEmpty(&headerRect);
	}

	if (FALSE != PtInRect(&headerRect, pt))
	{
		return; 
	}

	int r = Menu_TrackPopup(plugin.hwndLibraryParent, GetSubMenu(g_context_menus, 1),
							TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY,
							x, y, hwndDlg, NULL);
	switch (r)
	{
		case ID_PE_ID3:
		{
			if (view.GetSelectedCount() == 0 || view.GetSelectedCount() == view.GetCount())
				editCDInfo(hwndDlg, m_cdrom, 0);

			int sel =-1;
			while ((sel = view.GetNextSelected(sel)) != -1)
			{
				if (editCDInfo(hwndDlg, m_cdrom, sel+1) == 1)
					break;
			}
		}
		PostMessageW(hwndDlg, WM_NEXTDLGCTL, (WPARAM)hwndFrom, TRUE);
		break;
		case ID_CDROMMENU_PLAYSELECTEDITEMS:		playFiles(hwndDlg, m_cdrom, 0, 0); break;
		case ID_CDROMMENU_ENQUEUESELECTEDITEMS:	playFiles(hwndDlg, m_cdrom, 1, 0); break;
		case ID_CDROMMENU_SELECTALL:				ListView_SetItemState(hwndFrom, -1, LVIS_SELECTED, LVIS_SELECTED); break;
		case ID_CDROMMENU_PLAYALL:				playFiles(hwndDlg, m_cdrom, 0, 1); break;
		case ID_CDROMMENU_ENQUEUEALL:			playFiles(hwndDlg, m_cdrom, 1, 1); break;
		case ID_CDROMMENU_EXTRACT_EXTRACTSELECTEDITEMS: extractFiles(hwndDlg, m_cdrom, 0); break;
		case ID_CDROMMENU_EXTRACT_EXTRACTALL:	extractFiles(hwndDlg, m_cdrom, 1); break;
		case ID_CDROMMENU_EXTRACT_CONFIGURE:		Plugin_ShowRippingPreferences(); break;
	}
	UpdateWindow(hwndDlg);
	Sleep(100);
	MSG msg;
	while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
}

static INT_PTR WINAPI DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR a;
	a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam);
	if (a) return a;

	switch (uMsg)
	{
	case WM_INITDIALOG:		return Window_OnInitDialog(hwndDlg, (HWND)wParam, lParam);
	case WM_DESTROY:		Window_OnDestroy(hwndDlg); break;
	case WM_SIZE:			Window_OnSize(hwndDlg, (UINT)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(wParam)); break;
	case WM_COMMAND:		Window_OnCommand(hwndDlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam); break;
	case WM_NOTIFY:			return  Window_OnNotify(hwndDlg, (INT)wParam, (LPNMHDR) lParam);
	case WM_MOUSEMOVE:		Window_OnMouseMove(hwndDlg, (INT)wParam, MAKEPOINTS(lParam)); break;
	case WM_LBUTTONUP:		Window_OnLButtonUp(hwndDlg, (INT)wParam, MAKEPOINTS(lParam)); break;
	case WM_CONTEXTMENU:	Window_OnContextMenu(hwndDlg, (HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 1;
	case WM_ERASEBKGND:		return 1;
	case WM_TAGUPDATED:		Window_OnFileTagUpdated(hwndDlg, m_cdrom, (LPCWSTR)lParam); break;
	case WM_QUERYFILEINFO:	Window_OnQueryInfo(hwndDlg); break;
	case WM_PAINT:
	{
		int tab[] = { IDC_LIST2 | DCW_SUNKENBORDER};
		dialogSkinner.Draw(hwndDlg, tab, 1);
	}
	return 0;

	case WM_EXTRACTDISC:
		if ((CHAR)wParam == m_cdrom) extractFiles(hwndDlg, m_cdrom, TRUE);
		SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, ((CHAR)wParam == m_cdrom));
		return TRUE;

	}

	return 0;
}

static void CALLBACK APC_GetCracenoteInfo(ULONG_PTR param)
{
	GetGracenoteInfo(((APCPARAM*)param)->hwndDlg, ((APCPARAM*)param)->cLetter, NULL);
	free((APCPARAM*)param);
}