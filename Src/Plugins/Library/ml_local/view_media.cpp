#include "main.h"
#include "ml_local.h"
#include <windowsx.h>
#include "../nu/listview.h"
#include "..\..\General\gen_ml/config.h"
#include "resource.h"
#include <time.h>
#include "..\..\General\gen_ml/ml_ipc.h"
#include "../ml_pmp/pmp.h"
#include "..\..\General\gen_ml/gaystring.h"
#include "../nde/nde.h"
#include "../replicant/nu/AutoWide.h"
#include "../replicant/nu/AutoChar.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include <math.h>
#include <shlwapi.h>
#include <strsafe.h>
#include "..\..\General\gen_ml/menufucker.h"
#include "api_mldb.h"
#include "../replicant/foundation/error.h"

static wchar_t oldText[4096];

static int IPC_LIBRARY_SENDTOMENU;
static HINSTANCE cloud_hinst;
const int ML_MSG_PDXS_STATUS = 0x1001;
const int ML_MSG_PDXS_MIX = 0x1002;
void RefreshMetadata(HWND parent);

static HRGN g_rgnUpdate = NULL;
static int offsetX, offsetY, customAllowed;
int groupBtn = 1, enqueuedef = 0;
static viewButtons view;
HWND hwndSearchGlobal = 0;

//timers
#define TIMER_RATINGAUTOUNHOVER_ID		65520
#define TIMER_RATINGAUTOUNHOVER_DELAY	200

void FixAmps(wchar_t *str, size_t len)
{
	size_t realSize = 0;
	size_t extra = 0;
	wchar_t *itr = str;
	while (itr && *itr)
	{
		if (itr && *itr == L'&')
			extra++;
		itr++;
		realSize++;
	}

	extra = min(len - (realSize + 1), extra);

	while (extra)
	{
		str[extra+realSize] = str[realSize];
		if (str[realSize] == L'&')
		{
			extra--;
			str[extra+realSize] = L'&';
		}
		realSize--;
	}
}

void MakeDateString(__time64_t convertTime, wchar_t *dest, size_t destlen)
{
	SYSTEMTIME sysTime;
	tm *newtime = _localtime64(&convertTime);
	if (newtime)
	{
		sysTime.wYear	= (WORD)(newtime->tm_year + 1900);
		sysTime.wMonth	= (WORD)(newtime->tm_mon + 1);
		sysTime.wDayOfWeek = (WORD)newtime->tm_wday;
		sysTime.wDay		= (WORD)newtime->tm_mday;
		sysTime.wHour	= (WORD)newtime->tm_hour;
		sysTime.wMinute	= (WORD)newtime->tm_min;
		sysTime.wSecond	= (WORD)newtime->tm_sec;
		sysTime.wMilliseconds = 0;

		GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sysTime, NULL, dest, destlen);

		size_t dateSize = lstrlenW(dest);
		dest += dateSize;
		destlen -= dateSize;
		if (destlen)
		{
			*dest++ = L' ';
			destlen--;
		}

		GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &sysTime, NULL, dest, destlen);

		//wcsftime(expire_time, 63, L"%b %d, %Y", _localtime64(&convertTime));
	}
	else
		dest[0] = 0;
}

#define MAINTABLE_ID_CLOUD (unsigned char)-1
const unsigned char extra_idsW[] =
{
	MAINTABLE_ID_ISPODCAST,
	MAINTABLE_ID_PODCASTCHANNEL,
	MAINTABLE_ID_PODCASTPUBDATE,
	MAINTABLE_ID_GRACENOTEFILEID,
	MAINTABLE_ID_GRACENOTEEXTDATA,
	MAINTABLE_ID_LOSSLESS,
	MAINTABLE_ID_CODEC,
	MAINTABLE_ID_DIRECTOR,
	MAINTABLE_ID_PRODUCER,
	MAINTABLE_ID_WIDTH,
	MAINTABLE_ID_HEIGHT,
	MAINTABLE_ID_MIMETYPE,
	0,
	MAINTABLE_ID_DATEADDED,
	MAINTABLE_ID_CLOUD,
};

const ExtendedFields extended_fields =
{	
	L"ispodcast",
	L"podcastchannel",
	L"podcastpubdate",
	L"GracenoteFileID",
	L"GracenoteExtData",
	L"lossless",
	L"codec",
	L"director",
	L"producer",
	L"width",
	L"height",
	L"mime",
	L"realsize",
	L"dateadded",
	L"cloud",
};

const wchar_t *extra_strsW[] =
{
	extended_fields.ispodcast,
	extended_fields.podcastchannel,
	extended_fields.podcastpubdate,
	extended_fields.GracenoteFileID,
	extended_fields.GracenoteExtData,
	extended_fields.lossless,
	extended_fields.codec,
	extended_fields.director,
	extended_fields.producer,
	extended_fields.width,
	extended_fields.height,
	extended_fields.mimetype,
	extended_fields.realsize,
	extended_fields.dateadded,
	extended_fields.cloud,
};

const int NUM_EXTRA_COLSW = sizeof(extra_idsW) / sizeof(*extra_idsW);

bool isMixable(itemRecordW &song);
static int predixisExist;

static BOOL g_displaysearch		= TRUE;
static BOOL g_displaycontrols	= TRUE;

nde_scanner_t m_media_scanner = 0;

W_ListView resultlist;
static int resultSkin;

void fileInfoDialogs(HWND hwndParent);
void editInfo(HWND hwndParent);
void customizeColumnsDialog(HWND hwndParent);

static HWND m_hwnd;

itemRecordListW itemCache;
static int bgThread_Kill = 0;
static HANDLE bgThread_Handle;
static bool isMixablePresent = true;

CloudFiles cloudFiles, cloudUploading;

typedef struct
{
	UINT column_id;
	char *config_name;
	WORD defWidth;
	WORD minWidth;
	WORD maxWidth;
}
headerColumn;

#define UNLIMITED_WIDTH			((WORD)-1)

#define COLUMN_DEFMINWIDTH		12
#define COLUMN_DEFMAXWIDTH		UNLIMITED_WIDTH

#define MAX_COLUMN_ORDER (MEDIAVIEW_COL_NUMS+1)

static headerColumn columnList[MAX_COLUMN_ORDER - 1] =
{
	{IDS_ARTIST, "mv_col_artist", 140, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_TITLE, "mv_col_title", 140, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_ALBUM, "mv_col_album", 140, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_LENGTH, "mv_col_length", 55, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_TRACK_NUMBER, "mv_col_track", 55, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_GENRE, "mv_col_genre", 75, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_YEAR, "mv_col_year", 55, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_FILENAME, "mv_col_fn", 140, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_RATING, "mv_col_rating", 65, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_PLAY_COUNT, "mv_col_playcount", 70, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_PLAYED_LAST, "mv_col_lastplay", 115, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_LAST_UPDATED, "mv_col_lastupd", 115, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_FILE_TIME, "mv_col_filetime", 115, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_COMMENT, "mv_col_comment", 140, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_FILE_SIZE, "mb_col_filesize", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_BITRATE, "mb_col_bitrate", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_TYPE, "mb_col_type", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_DISC, "mb_col_disc", 55, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_ALBUM_ARTIST, "mb_col_albumartist", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_FILE_PATH, "mb_col_filepath", 140, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_ALBUM_GAIN, "mb_col_albumgain", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_TRACK_GAIN, "mb_col_trackgain", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_PUBLISHER, "mb_col_publisher", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_COMPOSER, "mb_col_composer", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_EXTENSION, "mb_col_extension", 55, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_IS_PODCAST, "mb_col_ispodcast", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_PODCAST_CHANNEL, "mb_col_podcastchannel", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_PODCAST_PUBLISH_DATE, "mb_col_podcastpubdate", 115, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_BPM, "mb_col_bpm", 55, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_CATEGORY, "mb_col_category", 75, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_DIRECTOR, "mb_col_director", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_PRODUCER, "mb_col_producer", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_DIMENSION, "mb_col_dimension", 100, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_DATE_ADDED, "mb_col_dateadded", 115, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
	{IDS_CLOUD, "mv_col_cloud", 27, COLUMN_DEFMINWIDTH, COLUMN_DEFMAXWIDTH},
};

//default column order
static signed char defColumnOrderCloud[MAX_COLUMN_ORDER] =
{
	MEDIAVIEW_COL_ARTIST,
	MEDIAVIEW_COL_ALBUM,
	MEDIAVIEW_COL_TRACK,
	MEDIAVIEW_COL_CLOUD,
	MEDIAVIEW_COL_TITLE,
	MEDIAVIEW_COL_LENGTH,
	MEDIAVIEW_COL_GENRE,
	MEDIAVIEW_COL_RATING,
	MEDIAVIEW_COL_PLAYCOUNT,
	MEDIAVIEW_COL_LASTPLAY,
	MEDIAVIEW_COL_YEAR,
	-1
};
static signed char defColumnOrder[MAX_COLUMN_ORDER] =
{
	MEDIAVIEW_COL_ARTIST,
	MEDIAVIEW_COL_ALBUM,
	MEDIAVIEW_COL_TRACK,
	MEDIAVIEW_COL_TITLE,
	MEDIAVIEW_COL_LENGTH,
	MEDIAVIEW_COL_GENRE,
	MEDIAVIEW_COL_RATING,
	MEDIAVIEW_COL_PLAYCOUNT,
	MEDIAVIEW_COL_LASTPLAY,
	MEDIAVIEW_COL_YEAR,
	-1
};
static signed char columnOrder[MAX_COLUMN_ORDER];


int WCSCMP_NULLOK(const wchar_t *pa, const wchar_t *pb)
{
	if (!pa) pa = L"";
	else SKIP_THE_AND_WHITESPACEW(pa)

		if (!pb) pb = L"";
		else SKIP_THE_AND_WHITESPACEW(pb)

			return CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE /*| NORM_IGNORENONSPACE*/, pa, -1, pb, -1) - 2;
//			return lstrcmpi(pa, pb);
}


int FLOATWCMP_NULLOK(const wchar_t *pa, const wchar_t *pb)
{
	if (pa)	SKIP_THE_AND_WHITESPACEW(pa)

		if (pb) SKIP_THE_AND_WHITESPACEW(pb)

			if ((!pa || !*pa) && (!pb || !*pb))
				return 0;
	if (!pa || !*pa)
		return 1;
	if (!pb || !*pb)
		return -1;

	_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();
	float a = (float)_wtof_l(pa,C_locale);
	float b = (float)_wtof_l(pb,C_locale);

	if (a > b)
		return 1;
	else if (a < b)
		return -1;
	else
		return 0;
}


typedef struct
{
	resultsniff_funcW cb;
	int user32;
}
bgThreadParms;

static int bg_total_len_s;
static __int64 bg_total_len_bytes;
static LARGE_INTEGER querytime;

static HMENU rate_hmenu = NULL;
static HMENU cloud_hmenu = NULL;
static HMENU sendto_hmenu = NULL;
static librarySendToMenuStruct s;

static RATINGCOLUMN ratingColumn;
static WCHAR ratingBackText[128];

#define IDC_LIST2HEADER			2001			// WM_INITDIALOG assign this is to the IDC_LIST2 header

// internal messages
#define IM_SYNCHEADERORDER		(WM_USER + 0xFFF0)

static DWORD WINAPI bgThreadQueryProc(void *tmp)
{
	bgThreadParms *p = (bgThreadParms*)tmp;
	bg_total_len_s = 0;
	bg_total_len_bytes = 0;
	LARGE_INTEGER starttime, endtime;
	QueryPerformanceCounter(&starttime);

	bg_total_len_s = saveQueryToListW(g_view_metaconf, m_media_scanner, &itemCache,
									  &cloudFiles, &cloudUploading, p->cb, p->user32,
									  &bgThread_Kill, &bg_total_len_bytes);
	QueryPerformanceCounter(&endtime);
	querytime.QuadPart = endtime.QuadPart - starttime.QuadPart;

	if (!bgThread_Kill) PostMessage(m_hwnd, WM_APP + 3, 0x69, 0);

	return 0;
}

void bgQuery_Stop() // exported for other people to call since it is useful (eventually
// we should have bgQuery pass the new query info along but I'll do that soon)
{
	if (bgThread_Handle)
	{
		bgThread_Kill = 1;
		WaitForSingleObject(bgThread_Handle, INFINITE);
		CloseHandle(bgThread_Handle);
		bgThread_Handle = 0;
	}
	KillTimer(m_hwnd, 123);
}

static void bgQuery(resultsniff_funcW cb = 0, int user32 = 0) // only internal used
{
	bgQuery_Stop();
	SetDlgItemTextW(m_hwnd, IDC_MEDIASTATUS, WASABI_API_LNGSTRINGW(IDS_SCANNING));
	StringCchCopyW(oldText, 4096, WASABI_API_LNGSTRINGW(IDS_SCANNING));

	SetTimer(m_hwnd, 123, 200, NULL);

	DWORD id;
	static bgThreadParms parms;
	parms.cb = cb;
	parms.user32 = user32;
	bgThread_Kill = 0;
	bgThread_Handle = CreateThread(NULL, 0, bgThreadQueryProc, (LPVOID) & parms, 0, &id);
}

// this thing does not produce a fully valid itemRecordList. be afraid.
static void copyFilesToItemCacheW(itemRecordListW *obj)
{
	if (bgThread_Handle) return ;

	int cnt = itemCache.Size;
	int i, l = cnt;
	cnt = 0;
	for (i = 0;i < l;i++)
	{
		if (resultlist.GetSelected(i)) cnt++;
	}
	obj->Alloc = obj->Size = 0;

	if (!cnt) return ;

	allocRecordList(obj, cnt, 0);
	if (!obj->Items)
	{
		obj->Size = obj->Alloc = 0;
		return ;
	}

	for (i = 0; i < itemCache.Size; i ++)
	{
		if (resultlist.GetSelected(i))
		{
			obj->Items[obj->Size++] = itemCache.Items[i];
			// makes sure that we are providing filesize in kb as
			// per spec even if we store it as __int64 internally
			obj->Items[obj->Size-1].filesize /= 1024;
		}
	}
}

void playFiles(int enqueue, int all)
{
	if (bgThread_Handle) return ;

	int cnt = 0;
	int l = itemCache.Size;

	int foo_all = 0; // all but play the only selected
	int foo_selected = -1;

	if (!enqueue && !all && g_config->ReadInt(L"viewplaymode", 1))
	{
		int selcnt = 0;
		for (int i = 0;i < l;i++)
		{
			if (resultlist.GetSelected(i)) selcnt++;
		}
		if (selcnt == 1)
		{
			foo_all = -1;
		}
	}

	for (int i = 0;i < l;i++)
	{
		if (foo_all || all || resultlist.GetSelected(i))
		{
			if (foo_all && foo_selected < 0 && resultlist.GetSelected(i)) foo_selected = i;

			if (!cnt)
			{
				if (!enqueue) SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);
				cnt++;
			}

			wchar_t title[2048] = {0};
			TAG_FMT_EXT(itemCache.Items[i].filename, itemrecordWTagFunc, ndeTagFuncFree, (void*)&itemCache.Items[i], title, 2048, 0);

			enqueueFileWithMetaStructW s;
			s.filename = itemCache.Items[i].filename;
			s.title    = title;
			s.ext      = NULL;
			s.length   = itemCache.Items[i].length;
#ifndef _DEBUG
			ndestring_retain(itemCache.Items[i].filename);
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW_NDE);
#else
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
#endif
		}
	}
	if (cnt)
	{
		if (foo_selected >= 0)
		{
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, foo_selected, IPC_SETPLAYLISTPOS);
			SendMessage(plugin.hwndWinampParent, WM_COMMAND, 40047, 0); // stop button, literally
			SendMessage(plugin.hwndWinampParent, WM_COMMAND, 40045, 0); // play button, literally
		}
		else if (!enqueue) SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY);
	}
}

// out can never be bigger than in+1
static void parsequicksearch(wchar_t *out, wchar_t *in) // parses a list into a list of terms that we are searching for
{
	int inquotes = 0, neednull = 0;
	while (in && *in)
	{
		wchar_t c = *in++;
		if (c != ' ' && c != '\t' && c != '\"')
		{
			neednull = 1;
			*out++ = c;
		}
		else if (c == '\"')
		{
			inquotes = !inquotes;
			if (!inquotes)
			{
				*out++ = 0;
				neednull = 0;
			}
		}
		else
		{
			if (inquotes) *out++ = c;
			else if (neednull)
			{
				*out++ = 0;
				neednull = 0;
			}
		}
	}
	*out++ = 0;
	*out++ = 0;
}

void makeQueryStringFromText(GayStringW *query, wchar_t *text, int nf)
{
	int ispar = 0;
	if (query->Get()[0])
	{
		ispar = 1;
		query->Append(L"&(");
	}

	if (!_wcsnicmp(text, L"query:", 6))
		query->Append(text + 6); // copy the query as is
	else if (text[0] == L'?')
		query->Append(text + 1);
	else
	{
		int isAny = 0;
		if (*text == L'*' && text[1] == L' ')
		{
			isAny = 1;
			text += 2;
		}
		wchar_t tmpbuf[2048 + 32] = {0};
		parsequicksearch(tmpbuf, text);

		int x;
		wchar_t *fields[] =
		{
			L"filename",
			L"title",
			L"artist",
			L"album",
			L"genre",
			L"albumartist",
			L"publisher",
			L"composer",
		};
		wchar_t *p = tmpbuf;
		while (p && *p)
		{
			size_t lenp = wcslen(p);

			if (p == tmpbuf) query->Append(L"(");
			else if (isAny) query->Append(L")|(");
			else query->Append(L")&(");
			if (p[0] == L'<' && p[wcslen(p) - 1] == L'>' && wcslen(p) > 2)
			{
				wchar_t *op = p;
				while (op && *op)
				{
					if (*op == L'\'') *op = L'\"';
					op++;
				}

				p[lenp - 1] = 0; // remove >
				query->Append(p + 1);
			}
			else
			{
				for (x = 0; x < (int)min(sizeof(fields) / sizeof(fields[0]), nf); x ++)
				{
					wchar_t *field = fields[x];
					if (x) query->Append(L"|");
					query->Append(field);
					query->Append(L" HAS \"");
					GayStringW escaped;
					queryStrEscape(p, escaped);
					query->Append(escaped.Get());
					query->Append(L"\"");
				}
			}
			p += lenp + 1;
		}
		query->Append(L")");
	}
	if (ispar) query->Append(L")");
}

static void doQuery(HWND hwndDlg, wchar_t *text, int dobg = 1)
{
	bgQuery_Stop();

	GayStringW query;
	if (text[0]) makeQueryStringFromText(&query, text);

	wchar_t *parent_query = NULL;
	extern wchar_t* m_query;
	parent_query = m_query;
	SendMessage(GetParent(hwndDlg), WM_APP + 2, 0, (LPARAM)&parent_query);
	GayStringW q;

	if (parent_query && parent_query[0])
	{
		q.Set(L"(");
		q.Append(parent_query);
		q.Append(L")");
	}
	if (query.Get() && query.Get()[0])
	{
		if (q.Get()[0])
		{
			q.Append(L" & (");
			q.Append(query.Get());
			q.Append(L")");
		}
		else q.Set(query.Get());
	}

	EnterCriticalSection(&g_db_cs);
	NDE_Scanner_Query(m_media_scanner, q.Get());
	LeaveCriticalSection(&g_db_cs);
	if (dobg) bgQuery();
}

static void RecycleSelectedItems()
{
	int totalItems = resultlist.GetSelectedCount();

	if (!totalItems)
		return ;

	SHFILEOPSTRUCTW fileOp;
	fileOp.hwnd = m_hwnd;
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = 0;
	fileOp.pTo = 0;
	fileOp.fFlags = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_USES_RECYCLEBIN) ? FOF_ALLOWUNDO : 0;
	fileOp.fAnyOperationsAborted = 0;
	fileOp.hNameMappings = 0;
	fileOp.lpszProgressTitle = 0;

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);

	int cchLen = totalItems * (MAX_PATH + 1) + 1;
	wchar_t *files = new wchar_t[cchLen];  // need room for each file name, null terminated. then have to null terminate the whole list

	if (files) // if malloc succeeded
	{
		wchar_t *curFile = files;
		for (int i = 0; i < itemCache.Size; i++)
		{
			if (resultlist.GetSelected(i))
			{
				if (NDE_Scanner_LocateNDEFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, itemCache.Items[i].filename))
				{
					StringCchCopyW(curFile, cchLen, itemCache.Items[i].filename);
					curFile += wcslen(itemCache.Items[i].filename) + 1;
				}
			}
		}
		if (curFile != files)
		{
			curFile[0] = 0; // null terminate

			fileOp.pFrom = files;
			fileOp.fAnyOperationsAborted = 0;
			if (SHFileOperationW(&fileOp))
			{
				wchar_t title[64] = {0};
				MessageBoxW(m_hwnd, WASABI_API_LNGSTRINGW(IDS_ERROR_DELETING_FILES),
				           WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,title,64), MB_OK);
			}
			else
			{
				// only remove items if deletion was allowed
				if (!fileOp.fAnyOperationsAborted)
				{
					for (int j = 0; j < itemCache.Size; j++)
					{
						if (resultlist.GetSelected(j))
						{
							if (NDE_Scanner_LocateNDEFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, itemCache.Items[j].filename))
							{
								// Wasabi callback event for pre remove
								WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_PRE, (size_t)itemCache.Items[j].filename, 0);
								
								NDE_Scanner_Delete(s);
								NDE_Scanner_Post(s);
								g_table_dirty++;
								
								// Wasabi callback event for post remove
								// ToDo: (BigG) Move outside of critical section somehow
								WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_POST, (size_t)itemCache.Items[j].filename, 0);

							}
						}
					}
				}
			}
		}

		delete [] files;
	}
	else // if malloc failed ... maybe because there's too many items.
	{
		files = new wchar_t[MAX_PATH + 1 + 1]; // double null termination
		if (!files) // if this malloc failed, just bail out
		{
			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);
			return ;
		}

		fileOp.pFrom = files;

		for (int i = 0;i < itemCache.Size;i++)
		{
			if (resultlist.GetSelected(i))
			{
				if (NDE_Scanner_LocateNDEFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, itemCache.Items[i].filename))
				{
					StringCchCopyW(files, MAX_PATH + 1 + 1, itemCache.Items[i].filename);
					files[wcslen(itemCache.Items[i].filename) + 1] = 0; // double null terminate
					fileOp.fAnyOperationsAborted = 0;
					if (SHFileOperationW(&fileOp))
					{
						wchar_t mes[4096] = {0};
						StringCchPrintfW(mes, 4096, WASABI_API_LNGSTRINGW(IDS_ERROR_DELETING_X), files);
						MessageBoxW(m_hwnd, mes, WASABI_API_LNGSTRINGW(IDS_ERROR), MB_OK);
						continue;
					}
					if (!fileOp.fAnyOperationsAborted)
					{
						// Wasabi callback event for pre remove
						WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_PRE, (size_t)itemCache.Items[i].filename, 0);
						
						NDE_Scanner_Delete(s);
						NDE_Scanner_Post(s);
						g_table_dirty++;

						// Wasabi callback event for post remove
						// ToDo: (BigG) Move outside of critical section somehow
						WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_POST, (size_t)itemCache.Items[i].filename, 0);
					}
				}
			}
			delete files;
		}
	}
	NDE_Table_DestroyScanner(g_table, s);
	if (g_table_dirty) NDE_Table_Sync(g_table);
	g_table_dirty = 0;
	LeaveCriticalSection(&g_db_cs);

	resultlist.Clear();
	emptyRecordList(&itemCache);

	// this might be gay, refreshing it completely (i.e. the cursor pos gets put back to normal, etc),
	// but really it is necessary for the view to be accurate.

	SendMessage(m_hwnd, WM_APP + 1, 0, 0); //refresh current view
}

static void removeSelectedItems(int physical)
{	
	if (physical)
	{
		RecycleSelectedItems();
		return ;
	}

	int hasdel = 0;

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);

	for (int i = 0;i < itemCache.Size;i++)
	{
		if (resultlist.GetSelected(i))
		{
			if (NDE_Scanner_LocateNDEFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, itemCache.Items[i].filename))
			{
				wchar_t conf[32] = {0};
				if (!hasdel && MessageBoxW(m_hwnd, WASABI_API_LNGSTRINGW(IDS_SURE_YOU_WANT_TO_REMOVE_SELECTED_FROM_LIBRARY),
										   WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRMATION,conf,32), MB_YESNO | MB_ICONQUESTION) != IDYES)
				{
					NDE_Table_DestroyScanner(g_table, s);
					LeaveCriticalSection(&g_db_cs);
					return ;
					//FUCKO> need to eat the RETURN msg
					//MSG msg;
					//while(PeekMessage(&msg,m_hwnd,WM_COMMAND,WM_COMMAND,1));
				}
				if (!hasdel) // stop any background queries
				{
					bgQuery_Stop();
				}
				// Wasabi callback event for pre remove
				WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_PRE, (size_t)itemCache.Items[i].filename, 0);
				
				hasdel = 1;
				NDE_Scanner_Delete(s);
				NDE_Scanner_Post(s);
				g_table_dirty++;

				// Wasabi callback event for post remove
				// ToDo: (BigG) Move this outside of the critical section
				WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_POST, (size_t)itemCache.Items[i].filename, 0);
			}
		}
	}

	NDE_Table_DestroyScanner(g_table, s);
	if (g_table_dirty) NDE_Table_Sync(g_table);
	g_table_dirty = 0;
	LeaveCriticalSection(&g_db_cs);

	if (!hasdel) return ;

	resultlist.Clear();
	emptyRecordList(&itemCache);

	// this might be gay, refreshing it completely (i.e. the cursor pos gets put back to normal, etc),
	// but really it is necessary for the view to be accurate.

	SendMessage(m_hwnd, WM_APP + 1, 0, 0); //refresh current view
}

static void exploreItemFolder(HWND hwndDlg)
{
	if (resultlist.GetSelectionMark() >= 0)
	{
		int l=resultlist.GetCount();
		for(int i=0;i<l;i++)
		{
			if (resultlist.GetSelected(i))
			{
				WASABI_API_EXPLORERFINDFILE->AddFile(itemCache.Items[i].filename);
			}
		}
		WASABI_API_EXPLORERFINDFILE->ShowFiles();
	}
}

static void removeDeadFiles(HWND hwndDlg)
{
	Scan_RemoveFiles(hwndDlg);

	// this might be gay, refreshing it completely (i.e. the cursor pos gets put back to normal, etc),
	// but really it is necessary for the view to be accurate.
	SendMessage(m_hwnd, WM_APP + 1, 0, 0); //refresh current view
}

static WNDPROC search_oldWndProc;
static DWORD WINAPI search_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN && wParam == VK_DOWN)
	{
		PostMessageW(GetParent(hwndDlg), WM_NEXTDLGCTL, (WPARAM)resultlist.getwnd(), (LPARAM)TRUE);
		ListView_SetItemState(resultlist.getwnd(), 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	}
	return CallWindowProcW(search_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}

typedef struct _LAYOUT
{
	INT		id;
	HWND		hwnd;
	INT		x;
	INT		y;
	INT		cx;
	INT		cy;
	DWORD	flags;
	HRGN	rgn;
}
LAYOUT, PLAYOUT;

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

#define GROUP_MIN			0x1
#define GROUP_MAX			0x3
#define GROUP_SEARCH		0x1
#define GROUP_STATUSBAR		0x2
#define GROUP_MAIN			0x3


static void LayoutWindows(HWND hwnd, BOOL fRedraw, BOOL fUpdateAll = FALSE)
{
	static INT controls[] =
	{
		GROUP_SEARCH, IDC_SEARCHCAPTION, IDC_CLEAR, IDC_QUICKSEARCH,
		GROUP_STATUSBAR, IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_MIX, IDC_BUTTON_CREATEPLAYLIST, IDC_BUTTON_INFOTOGGLE, IDC_MIXABLE, IDC_MEDIASTATUS,
		GROUP_MAIN, IDC_LIST2
	};

	INT index;
	RECT rc, rg, ri;
	LAYOUT layout[sizeof(controls)/sizeof(controls[0])], *pl;
	BOOL skipgroup;
	HRGN rgn = NULL;

	GetClientRect(hwnd, &rc);
	if (rc.bottom == rc.top || rc.right == rc.left) return;

	SetRect(&rg, rc.left, rc.top, rc.right, rc.bottom);

	pl = layout;
	skipgroup = FALSE;

	InvalidateRect(hwnd, NULL, TRUE);

	for (index = 0; index < sizeof(controls) / sizeof(*controls); index++)
	{
		if (controls[index] >= GROUP_MIN && controls[index] <= GROUP_MAX) // group id
		{
			skipgroup = FALSE;
			switch (controls[index])
			{
			case GROUP_SEARCH:
				if (g_displaysearch)
				{
					wchar_t buffer[128] = {0};
					HWND ctrl = GetDlgItem(hwnd, IDC_CLEAR);
					GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

					SetRect(&rg, rc.left,
							rc.top + WASABI_API_APP->getScaleY(2),
							rc.right - WASABI_API_APP->getScaleX(2),
							rc.top + WASABI_API_APP->getScaleY(HIWORD(idealSize)+1));
					rc.top = rg.bottom + WASABI_API_APP->getScaleY(3);
				}
				skipgroup = !g_displaysearch;
				break;
			case GROUP_STATUSBAR:
				if (g_displaycontrols)
				{
					wchar_t buffer[128] = {0};
					HWND ctrl = GetDlgItem(hwnd, IDC_BUTTON_PLAY);
					GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

					SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1),
							rc.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)),
							rc.right, rc.bottom);
					rc.bottom = rg.top - WASABI_API_APP->getScaleY(3);
				}
				skipgroup = !g_displaycontrols;
				break;
			case GROUP_MAIN:
				SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1), rc.top, rc.right, rc.bottom);
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
		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW  | SWP_NOCOPYBITS;

		switch (pl->id)
		{
			case IDC_SEARCHCAPTION:
			{
				wchar_t buffer[128] = {0};
				GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
				LRESULT idealSize = MLSkinnedStatic_GetIdealSize(pl->hwnd, buffer);

				SETLAYOUTPOS(pl, rg.left + WASABI_API_APP->getScaleX(2),
							 rg.top + WASABI_API_APP->getScaleY(1),
							 WASABI_API_APP->getScaleX(LOWORD(idealSize)),
							 (rg.bottom - rg.top));
				rg.left += (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			}
			case IDC_CLEAR:
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
				pl->flags |= (rg.right > rg.left) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left - WASABI_API_APP->getScaleX(1),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				break;
			case IDC_BUTTON_PLAY:
			case IDC_BUTTON_ENQUEUE:
			case IDC_BUTTON_MIX:
			case IDC_BUTTON_CREATEPLAYLIST:
				if (IDC_BUTTON_MIX != pl->id || customAllowed)
				{
					if (groupBtn && pl->id == IDC_BUTTON_PLAY && enqueuedef == 1)
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if (groupBtn && pl->id == IDC_BUTTON_ENQUEUE && enqueuedef != 1)
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if (groupBtn && (pl->id == IDC_BUTTON_PLAY || pl->id == IDC_BUTTON_ENQUEUE) && customAllowed)
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if (pl->id == IDC_BUTTON_CREATEPLAYLIST && !AGAVE_API_PLAYLIST_GENERATOR)
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
			case IDC_BUTTON_INFOTOGGLE:
				switch (SendMessage(GetParent(hwnd), WM_USER + 66,  0, 0))
				{
					case 0xFF:
					case 0xF0:
					{
						wchar_t buffer[128] = {0};
						GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
						LRESULT idealSize = MLSkinnedButton_GetIdealSize(pl->hwnd, buffer);
						LONG width = LOWORD(idealSize) + WASABI_API_APP->getScaleX(6);

						pl->flags |= (((rg.right - rg.left) - (ri.right - ri.left)) > WASABI_API_APP->getScaleX(60)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW ;
						SETLAYOUTPOS(pl, rg.right - width - WASABI_API_APP->getScaleX(2),
									 rg.top, width, (rg.bottom - rg.top));
						if (SWP_SHOWWINDOW & pl->flags) rg.right -= (pl->cx + WASABI_API_APP->getScaleX(4));
						break;
					}
				}
				break;
			case IDC_MIXABLE:
				if (predixisExist & 1)
				{
					SETLAYOUTPOS(pl, rg.right - (ri.right - ri.left), rg.top, (ri.right - ri.left), (rg.bottom - rg.top));
					pl->flags |= ((rg.right - rg.left) - (ri.right - ri.left) > WASABI_API_APP->getScaleX(60)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
					if (SWP_SHOWWINDOW & pl->flags) rg.right -= (pl->cx + WASABI_API_APP->getScaleX(4));
				}
				break;
			case IDC_MEDIASTATUS:
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left, (rg.bottom - rg.top));
				pl->flags |= (pl->cx > WASABI_API_APP->getScaleX(16)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				break;
			case IDC_LIST2:
				SETLAYOUTPOS(pl, rg.left, rg.top + 1, (rg.right - rg.left) - WASABI_API_APP->getScaleX(3), (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(2));
				break;
		}

		SETLAYOUTFLAGS(pl, ri);
		if (LAYOUTNEEEDUPDATE(pl))
		{
			if (SWP_NOSIZE == ((SWP_HIDEWINDOW | SWP_SHOWWINDOW | SWP_NOSIZE) & pl->flags) &&
			    ri.left == (pl->x + offsetX) && ri.top == (pl->y + offsetY) && !fUpdateAll && IsWindowVisible(pl->hwnd))
			{
				SetRect(&ri, pl->x, pl->y, pl->cx + pl->x, pl->y + pl->cy);
				ValidateRect(hwnd, &ri);
			}

			pl++;
		}
		else if (!fUpdateAll && (fRedraw || (!offsetX && !offsetY)) && IsWindowVisible(pl->hwnd))
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
			hdwp = DeferWindowPos(hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags);
		}
		if (hdwp) EndDeferWindowPos(hdwp);

		if (!rgn) rgn = CreateRectRgn(0, 0, 0, 0);

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
			RedrawWindow(hwnd, NULL, rgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
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

static void updateInfoText(HWND hwndDlg, bool x = false)
{
	int a = SendMessage(GetParent(hwndDlg), WM_USER + 66, x ? -1 : 0, 0);
	if (a == 0xff)
		SetDlgItemTextW(hwndDlg, IDC_BUTTON_INFOTOGGLE, WASABI_API_LNGSTRINGW(IDS_HIDE_INFO));
	else if (a == 0xf0)
		SetDlgItemTextW(hwndDlg, IDC_BUTTON_INFOTOGGLE, WASABI_API_LNGSTRINGW(IDS_SHOW_INFO));
	else ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON_INFOTOGGLE), SW_HIDE);
}

static void initColumnsHeader(HWND hwndList)
{
	INT index, sortby;
	LVCOLUMNW lvc = {0, };
	if (!hwndList || !IsWindow(hwndList)) return;

	SendMessageW(hwndList, WM_SETREDRAW, FALSE, 0L);

	while (SendMessageW(hwndList, LVM_DELETECOLUMN, 0, 0L));

	sortby = g_view_metaconf->ReadInt(L"mv_sort_by", 1);

	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = L"";
	lvc.cx = 30;
	SendMessageW(hwndList, LVM_INSERTCOLUMNW, 0, (LPARAM)&lvc); // create dummy column

	// TODO set to a zero width if not available
	MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(hwndList), -1);	// reset the cloud status column so it'll be correctly removed
	SetPropW(hwndList, L"pmp_list_info", (HANDLE)-1);

	for (index = 0; columnOrder[index] != -1; index++)
	{
		headerColumn *cl = &columnList[columnOrder[index]];
		lvc.pszText = WASABI_API_LNGSTRINGW(cl->column_id);

		lvc.cx = g_view_metaconf->ReadInt(AutoWide(cl->config_name), cl->defWidth);
		if (lvc.cx < cl->minWidth) lvc.cx = cl->minWidth;

		// update position of the cloud column icon
		if (columnOrder[index] == MEDIAVIEW_COL_CLOUD)
		{
			if (!cloud_hinst || cloud_hinst == (HINSTANCE)1 ||
				!SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_ACTIVE))
			{
				MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(hwndList), -1);
				SetPropW(hwndList, L"pmp_list_info", (HANDLE)-1);
				lvc.cx = 0;
			}
			else
			{
				MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(hwndList), index);
				SetPropW(hwndList, L"pmp_list_info", (HANDLE)index);
				lvc.cx = 27;
				MLCloudColumn_GetWidth(plugin.hwndLibraryParent, &lvc.cx);
			}
		}

		SendMessageW(hwndList, LVM_INSERTCOLUMNW, 0xFFFF, (LPARAM)&lvc);
	
		if (sortby == columnOrder[index]) MLSkinnedListView_DisplaySort(hwndList, index, !g_view_metaconf->ReadInt(L"mv_sort_dir", 0));
	}
	SendMessageW(hwndList, LVM_DELETECOLUMN, 0, 0L); // Delete dummy column

	for (index = 0; -1 != columnOrder[index] && MEDIAVIEW_COL_RATING != columnOrder[index]/* && MEDIAVIEW_COL_CLOUD != columnOrder[index]*/; index++);
	if (-1 != index) SendMessageW(hwndList, LVM_SETCOLUMNWIDTH, index, (LPARAM)SendMessageW(hwndList, LVM_GETCOLUMNWIDTH, index, 0L));

	SendMessageW(hwndList, WM_SETREDRAW, TRUE, 0L);
}

static int m_last_selitem = -1;
static int m_bgupdinfoviewerflag;

extern void add_to_library(HWND wndparent);

static INT_PTR CALLBACK needAddFilesProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		if (AGAVE_API_ITUNES_IMPORTER && AGAVE_API_ITUNES_IMPORTER->iTunesExists())
			EnableWindow(GetDlgItem(hwndDlg, IDC_IMPORT_ITUNES), TRUE);
		else
			EnableWindow(GetDlgItem(hwndDlg, IDC_IMPORT_ITUNES), FALSE);
		
		SetTimer(hwndDlg, 1, 1000, NULL);
		return 1;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDOK:
			case IDCANCEL:
				if (BN_CLICKED == HIWORD(wParam))
				{
					if (IsDlgButtonChecked(hwndDlg, IDC_CHECK1)) g_config->WriteInt(L"noshowadddlg", 1);
					EndDialog(hwndDlg, 0);
				}
				break;
			case ID_ADD_FILES:
				if (BN_CLICKED == HIWORD(wParam))
				{
					add_to_library(hwndDlg);
					PostMessage(hwndDlg, WM_TIMER, 1, 0);
				}
				break;
			case IDC_IMPORT_ITUNES:
				if (BN_CLICKED == HIWORD(wParam))
				{
					if (AGAVE_API_ITUNES_IMPORTER)
						AGAVE_API_ITUNES_IMPORTER->ImportFromiTunes(hwndDlg);

					PostMessage(hwndDlg, WM_TIMER, 1, 0);
					if (m_curview_hwnd) PostMessage(m_curview_hwnd, WM_APP + 1, 0, 0); //update current view
				}
				break;
			case IDC_BTN_LINK_PROMO:
				if (BN_CLICKED == HIWORD(wParam)) ShellExecuteA(plugin.hwndWinampParent, "open", "https://help.winamp.com/hc/articles/8105244490772-Player-Overview", NULL, ".", 0);
				break;
		}
		break;
	case WM_TIMER:
		if (g_table && NDE_Table_GetRecordsCount(g_table))
		{
			wchar_t buf[512] = {0};
			StringCchPrintfW(buf, 512, WASABI_API_LNGSTRINGW(IDS_THERE_ARE_NOW_X_ITEMS_IN_THE_LIBRARY), NDE_Table_GetRecordsCount(g_table));
			SetDlgItemTextW(hwndDlg, IDC_TEXT, buf);
			SetDlgItemTextW(hwndDlg, ID_ADD_FILES, WASABI_API_LNGSTRINGW(IDS_ADD_MORE));
		}
		break;
	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
			if (di->CtlType == ODT_BUTTON)
			{
				wchar_t wt[123] = {0};
				int y;
				RECT r;
				HPEN hPen, hOldPen;
				DWORD style;
				GetDlgItemText(hwndDlg, (INT)wParam, wt, ARRAYSIZE(wt));

				style  = (DWORD)GetWindowLongPtrW(di->hwndItem, GWL_STYLE);
				// draw text
				SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
				
				memset(&r, 0, sizeof(r));
				DrawText(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);
				

				if (BS_RIGHT & style) r.left = max(di->rcItem.left+ 2, di->rcItem.right - r.right - 2);
				else if (BS_LEFT & style) r.left = di->rcItem.left+ 2;
				else r.left = ((di->rcItem.right - di->rcItem.left - 4) - r.right) / 2;
				
				if (r.left < di->rcItem.left + 2)
				{
					r.left = di->rcItem.left + 2;
					r.right = di->rcItem.right - 2;
				}
				else r.right += r.left;
				if (r.right > di->rcItem.right - 2)  r.right = di->rcItem.right - 2;
		

				if (BS_TOP & style) r.top = di->rcItem.top;
				else if(BS_VCENTER & style) r.top = ((di->rcItem.bottom - di->rcItem.top - 2) - r.bottom) /2;
				else r.top = di->rcItem.bottom - 2 - r.bottom; 
								
				if (r.top < di->rcItem.top)
				{
					r.top  = di->rcItem.top;
					r.bottom = di->rcItem.bottom - 2;
				}
				else r.bottom += r.top;
				if (r.bottom > di->rcItem.bottom - 2)  r.bottom = di->rcItem.bottom - 2;

				DrawText(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_WORD_ELLIPSIS);

				// draw underline
				y = min(di->rcItem.bottom, r.bottom + 1);
				hPen = CreatePen(PS_SOLID, 0, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
				hOldPen = (HPEN) SelectObject(di->hDC, hPen);
				MoveToEx(di->hDC, r.left, y, NULL);
				LineTo(di->hDC, r.right, y);
				SelectObject(di->hDC, hOldPen);
				DeleteObject(hPen);
			}
		}
	}
	
	return 0;

};

static void SetStatusText(HWND hwndStatus, LPCWSTR *ppsz, INT count)
{
	WCHAR buffer[4096] = {0};
	if (0 == count || !ppsz)
	{
		SetWindowText(hwndStatus, L"");
		return;
	}
	buffer[0] = 0x00;
	for (int i = 0; i < count; i++)
	{
		StringCchCatW(buffer, 4096, ppsz[i]);
		StringCchCatW(buffer, 4096, L" ");
	}
	SetWindowTextW(hwndStatus, buffer);

	StringCchCopyW(oldText, 4096, buffer);
}

static void SetRating(UINT iItem, INT newRating, HWND hwndList)
{
	if (0 == newRating) newRating = -1;
	if (iItem < (UINT)itemCache.Size)
	{
		if (g_table && newRating != itemCache.Items[iItem].rating)
		{
			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);

			if (NDE_Scanner_LocateNDEFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, itemCache.Items[iItem].filename))
			{
				NDE_Scanner_Edit(s);
				db_setFieldInt(s, MAINTABLE_ID_RATING, newRating);
				NDE_Scanner_Post(s);
				itemCache.Items[iItem].rating = newRating;
				if (g_config->ReadInt(L"writeratings", 0))
				{
					wchar_t buf[64] = {0};
					if (newRating > 0)
					{
						wsprintfW(buf, L"%d", newRating);
					}
					else
						buf[0] = 0;
					updateFileInfo(itemCache.Items[iItem].filename, DB_FIELDNAME_rating, buf);
					SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
				}
			}
			NDE_Table_DestroyScanner(g_table, s);

			if (g_table_dirty)
			{
				g_table_dirty = 0;
				NDE_Table_Sync(g_table);
			}
			LeaveCriticalSection(&g_db_cs);
		}
		if (newRating == itemCache.Items[iItem].rating && hwndList)
		{
			ratingColumn.hwndList = hwndList;
			ratingColumn.iItem = iItem;
			ratingColumn.iSubItem = 600;
			MLRatingColumn_Animate(plugin.hwndLibraryParent, &ratingColumn);
			ListView_RedrawItems(resultlist.getwnd(), iItem, iItem);
			// TODO: benski> update the top panes w/o refreshing, if possible
			// CUT: PostMessage(GetParent(GetParent(hwndList)), WM_APP + 4, (WPARAM)newRating, (LPARAM)1);
		}
	}
}

/////////// Header Messages / Notifications
static BOOL Header_OnItemChanging(HWND hwndDlg, NMHEADERW *phdr, LRESULT *pResult, UINT uMsg)
{
	if (phdr->pitem && (HDI_WIDTH & phdr->pitem->mask))
	{
		INT test;
		test = columnList[columnOrder[phdr->iItem]].minWidth;
		if (phdr->pitem->cxy < test) phdr->pitem->cxy = test;
		test = columnList[columnOrder[phdr->iItem]].maxWidth;
		if (test != UNLIMITED_WIDTH && phdr->pitem->cxy > test) phdr->pitem->cxy = test;

		if (MEDIAVIEW_COL_RATING == columnOrder[phdr->iItem])
		{
			RATINGWIDTH rw;

			rw.fStyle = RCS_DEFAULT;
			rw.width = phdr->pitem->cxy;
			if (MLRatingColumn_GetWidth(plugin.hwndLibraryParent, &rw)) phdr->pitem->cxy = rw.width;
			if (0 == phdr->iItem)
			{
				RATINGBACKTEXT rbt;
				rbt.pszText = ratingBackText;
				rbt.cchTextMax = sizeof(ratingBackText)/sizeof(WCHAR);
				rbt.nColumnWidth = phdr->pitem->cxy;
				rbt.fStyle = RCS_DEFAULT;
				MLRatingColumn_FillBackString(plugin.hwndLibraryParent, &rbt);
			}
			else ratingBackText[0] = 0x00;
		}
		else if (MEDIAVIEW_COL_CLOUD == columnOrder[phdr->iItem])
		{
			if (!cloud_hinst || cloud_hinst == (HINSTANCE)1 ||
				!SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_ACTIVE))
				phdr->pitem->cxy = 0;
			else
			{
				INT width = phdr->pitem->cxy;
				if (MLCloudColumn_GetWidth(plugin.hwndLibraryParent, &width))
				{
					phdr->pitem->cxy = width;
				}
			}
		}
	}
	return FALSE;
}

static BOOL Header_OnEndDrag(HWND hwndDlg, NMHEADERW *phdr, LRESULT *pResult)
{
	PostMessageW(hwndDlg, IM_SYNCHEADERORDER, 0, (LPARAM)phdr->hdr.hwndFrom);
	return FALSE;
}

static BOOL Header_OnRightClick(HWND hwndDlg, NMHDR *pnmh, LRESULT *pResult)
{
	HMENU menu = GetSubMenu(g_context_menus, 4);
	POINT p;
	GetCursorPos(&p);
	int r = DoTrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, p.x, p.y, hwndDlg, NULL);
	switch (r)
	{
	case ID_HEADERWND_CUSTOMIZECOLUMNS:
		customizeColumnsDialog(hwndDlg);
		break;
	}
	return FALSE;
}

/////////// ListView Messages / Notifications
static BOOL ListView_OnItemChanged(HWND hwndDlg, NMLISTVIEW *pnmv)
{
	if (pnmv->uNewState & LVIS_SELECTED)
	{
		//if (GetFocus()==resultlist.getwnd())
		{
			m_last_selitem = pnmv->iItem;
			KillTimer(hwndDlg, 6600);
			SetTimer(hwndDlg, 6600, 250, NULL);
		}
	}
	else
	{
		if (isMixablePresent)
		{
			SetDlgItemText(hwndDlg, IDC_MIXABLE, L"");
			isMixablePresent = false;
		}
	}
	return FALSE;
}

static BOOL ListView_OnDoubleClick(HWND hwndDlg, NMITEMACTIVATE *pnmitem)
{
	playFiles((!!g_config->ReadInt(L"enqueuedef", 0)) ^(!!(GetAsyncKeyState(VK_SHIFT)&0x8000)), 0);
	return FALSE;
}

void EatKeyboard()
{
	Sleep(100);
	MSG msg;
	while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
}

static void Dialog_OnContextMenu(HWND hwndDlg, HWND hwndFrom, int x, int y)
{
	if (hwndFrom != resultlist.getwnd())
		return;

	POINT pt = {x,y};

	if (x == -1 || y == -1) // x and y are -1 if the user invoked a shift-f10 popup menu
	{
		RECT itemRect = {0};
		int selected = resultlist.GetNextSelected();
		if (selected != -1) // if something is selected we'll drop the menu from there
		{
			resultlist.GetItemRect(selected, &itemRect);
			ClientToScreen(resultlist.getwnd(), (POINT *)&itemRect);
		}
		else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
		{
			GetWindowRect(resultlist.getwnd(), &itemRect);

			HWND hHeader = (HWND)SNDMSG(resultlist.getwnd(), LVM_GETHEADER, 0, 0L);
			RECT headerRect;
			if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
			{
				itemRect.top += (headerRect.bottom - headerRect.top);
			}
		}
		x = itemRect.left;
		y = itemRect.top;
	}

	HWND hHeader = (HWND)SNDMSG(resultlist.getwnd(), LVM_GETHEADER, 0, 0L);
	RECT headerRect;
	if (0 == (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) || FALSE == GetWindowRect(hHeader, &headerRect))
	{
		SetRectEmpty(&headerRect);
	}

	if (FALSE != PtInRect(&headerRect, pt))
	{
		return; 
	}

	HMENU globmenu = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);
	HMENU menu = GetSubMenu(globmenu, 0);
	int rate_idx = 9;
	sendto_hmenu = GetSubMenu(menu, 2);
	rate_hmenu = GetSubMenu(menu, rate_idx);

	ConvertRatingMenuStar(rate_hmenu, ID_RATE_5);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_4);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_3);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_2);
	ConvertRatingMenuStar(rate_hmenu, ID_RATE_1);

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

	wchar_t *artist = NULL;
	wchar_t *album = NULL;
	int n = resultlist.GetSelectionMark();
	if (n != -1)
	{
		artist = itemCache.Items[n].artist;
		album = itemCache.Items[n].album;
		wchar_t str[2048] = {0}, str2[128] = {0};
		// (BigG): Check the ini settings for viewing vs playing and create the menu accordingly
		// Keeping this here in case we want to use the ini setting:
		//StringCchPrintfW(str, 2048, WASABI_API_LNGSTRINGW( (g_viewnotplay != 0) ? IDS_VIEW_ALL_FILES_BY : IDS_PLAY_ALL_FILES_BY), artist ? artist : L"");	

		int len = lstrlenW(artist);
		if (len > 39)
		{
			StringCchPrintfW(str2, 40, L"%.36s...", artist);//WASABI_API_LNGSTRINGW(IDS_VIEW_ALL_FILES_BY), artist ? artist : L"");
			StringCchPrintfW(str, 2048, WASABI_API_LNGSTRINGW(IDS_VIEW_ALL_FILES_BY), str2);
		}
		else
		{
			StringCchPrintfW(str, 2048, WASABI_API_LNGSTRINGW(IDS_VIEW_ALL_FILES_BY), artist ? artist : L"");
		}
		FixAmps(str, 2048);
		MENUITEMINFOW mii =
		{
			sizeof(MENUITEMINFOW),
			MIIM_TYPE | MIIM_ID,
			MFT_STRING,
			MFS_ENABLED,
			0x1234,
			NULL,
			NULL,
			NULL,
			0,
			str,
			0,
		};

		if (!(!cloud_hinst || cloud_hinst == (HINSTANCE)1 ||
			  !SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_ACTIVE)))
		{
			MENUITEMINFOW m = {sizeof(m), MIIM_TYPE | MIIM_ID | MIIM_SUBMENU, MFT_SEPARATOR, 0};
			m.wID = CLOUD_SOURCE_MENUS - 1;
			InsertMenuItemW(menu, 0, FALSE, &m);

			wchar_t a[100] = {0};
			m.fType = MFT_STRING;
			m.dwTypeData = WASABI_API_LNGSTRINGW_BUF(IDS_CLOUD_SOURCES, a, 100);
			m.wID = CLOUD_SOURCE_MENUS;
			m.hSubMenu = cloud_hmenu = CreatePopupMenu();
			InsertMenuItemW(menu, 0, FALSE, &m);
		}

		if (artist && artist[0])
		{
			InsertMenuItemW(menu, ID_EDITITEMINFOS, FALSE, &mii);
		}
		// (BigG): Check the ini settings for viewing vs playing and create the menu accordingly
		// Keeping this here in case we want to use the ini setting:
		len = lstrlenW(album);
		if (len > 39)
		{
			StringCchPrintfW(str2, 40, L"%.36s...", album);
			StringCchPrintfW(str, 2048, WASABI_API_LNGSTRINGW(IDS_VIEW_ALL_FILES_FROM), str2);
		}
		else
		{
			StringCchPrintfW(str, 2048, WASABI_API_LNGSTRINGW(IDS_VIEW_ALL_FILES_FROM), album ? album : L"");
		}
		FixAmps(str, 2048);
		mii.cch = wcslen(str);
		mii.wID = 0x1235;
		if (album && album[0])
		{
			InsertMenuItemW(menu, ID_EDITITEMINFOS, FALSE, &mii);
		}

		{
			mii.wID = 0xdeadbeef;
			mii.fType = MFT_SEPARATOR;
			InsertMenuItemW(menu, ID_EDITITEMINFOS, FALSE, &mii);
		}
	}
	else
	{
		EnableMenuItem(menu, ID_MEDIAWND_PLAYSELECTEDFILES, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, ID_MEDIAWND_ENQUEUESELECTEDFILES, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, IDC_REFRESH_METADATA, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, ID_MEDIAWND_REMOVEFROMLIBRARY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, ID_EDITITEMINFOS, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, ID_MEDIAWND_EXPLOREFOLDER, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, ID_PE_ID3, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, 2, MF_BYPOSITION | MF_GRAYED); //grays the "Add to playlist..." menu
		EnableMenuItem(menu, rate_idx, MF_BYPOSITION | MF_GRAYED); //grays the "Rate..." menu
		EnableMenuItem(menu, 13, MF_BYPOSITION | MF_GRAYED); //grays the "Remove..." menu
	}

	menufucker_t mf = {sizeof(mf),MENU_MEDIAVIEW,menu,0x3000,0x4000,0};
	mf.extinf.mediaview.list = resultlist.getwnd();
	mf.extinf.mediaview.items = &itemCache;
	pluginMessage message_build = {ML_IPC_MENUFUCKER_BUILD,(intptr_t)&mf,0};
	SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&message_build, ML_IPC_SEND_PLUGIN_MESSAGE);

	Menu_SetRatingValue(rate_hmenu, 0);
	UpdateMenuItems(hwndDlg, menu, IDR_VIEW_ACCELERATORS);
	int r = DoTrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, x, y, hwndDlg, NULL);

	pluginMessage message_result = {ML_IPC_MENUFUCKER_RESULT,(intptr_t)&mf,r,0};
	SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&message_result, ML_IPC_SEND_PLUGIN_MESSAGE);

	switch (r)
	{
		case ID_MEDIAWND_PLAYSELECTEDFILES:
		case ID_MEDIAWND_ENQUEUESELECTEDFILES:
			playFiles((r == ID_MEDIAWND_ENQUEUESELECTEDFILES), 0);
			break;
		case ID_MEDIAWND_SELECTALL:
			{
				LVITEM item = {0};
				item.state = LVIS_SELECTED;
				item.stateMask = LVIS_SELECTED;
				SendMessageW(hwndFrom, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&item);
			}
			break;
		case ID_MEDIAWND_REMOVEFROMLIBRARY:
			removeSelectedItems(0);
			break;
		case ID_EDITITEMINFOS:
			if (resultlist.GetSelectedCount() > 0)
				editInfo(hwndDlg);
			break;
		case ID_PE_ID3:
			fileInfoDialogs(hwndDlg);
			PostMessageW(hwndDlg, WM_NEXTDLGCTL, (WPARAM)hwndFrom, (LPARAM)TRUE);
			break;
		case IDC_REFRESH_METADATA:
			RefreshMetadata(hwndDlg);
			break;
		case 0x1234:      // all files from selected artist
		{
			wchar_t tmp[2048] = {0};
			GayStringW escaped;
			queryStrEscape(artist, escaped);

			// Keeping this here in case we want to use the ini setting later:
			//if ( g_viewnotplay )	// (BigG): Check to see if we should play or view the current tracks
			//{
				StringCchPrintfW(tmp, 2048, L"?artist = \"%s\"", escaped.Get());
				SetWindowTextW(hwndSearchGlobal, tmp);
			//}
			//else					// Otherwise do the old behavior and just play it back
			//{
			//	StringCchPrintfW(tmp, 2048, L"artist = \"%s\"", escaped.Get());
			//	main_playQuery(g_view_metaconf, tmp, 0);
			//}
			
			SetWindowTextW(hwndSearchGlobal, tmp);
		}
		break;
		case 0x1235:      // all files from selected album
		{
			wchar_t tmp[2048] = {0};
			GayStringW escaped;
			queryStrEscape(album, escaped);
			
			// Keeping this here in case we want to use the ini setting later:
			//if ( g_viewnotplay )	// (BigG): Check to see if we should play or view the current tracks
			//{
				StringCchPrintfW(tmp, 2048, L"?album = \"%s\"", escaped.Get());
				SetWindowTextW(hwndSearchGlobal, tmp);
			//}
			//else					// Otherwise do the old behavior and just play it back
			//{
			//	StringCchPrintfW(tmp, 2048, L"album = \"%s\"", escaped.Get());
			//	main_playQuery(g_view_metaconf, tmp, 0);
			//}
		}
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
			int x;
			int has = 0;
			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);
			for (x = 0; x < itemCache.Size; x ++)
			{
				if (resultlist.GetSelected(x))
				{
					if (NDE_Scanner_LocateNDEFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, itemCache.Items[x].filename))
					{
						has++;
						NDE_Scanner_Edit(s);
						db_setFieldInt(s, MAINTABLE_ID_RATING, rate);
						NDE_Scanner_Post(s);
						itemCache.Items[x].rating = rate;
						if (g_config->ReadInt(L"writeratings", 0))
						{
							wchar_t buf[64] = {0};
							if (rate > 0)
							{
								wsprintfW(buf, L"%d", rate);
							}
							else
								buf[0] = 0;
							updateFileInfo(itemCache.Items[x].filename, DB_FIELDNAME_rating, buf);
							SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
						}
					}
				}
			}
			NDE_Table_DestroyScanner(g_table, s);

			if (g_table_dirty)
			{
				g_table_dirty = 0;
				NDE_Table_Sync(g_table);
			}
			LeaveCriticalSection(&g_db_cs);

			if (has)
			{
				ListView_RedrawItems(resultlist.getwnd(), 0, itemCache.Size - 1);
				PostMessage(GetParent(hwndDlg), WM_APP + 4, (WPARAM)rate, (LPARAM)0);
			}
		}
		break;
		case ID_MEDIAWND_EXPLOREFOLDER:
			exploreItemFolder(hwndDlg);
			break;
		case ID_MEDIAWND_REMOVE_REMOVEALLDEADFILES:
			removeDeadFiles(hwndDlg);
			break;
		case ID_MEDIAWND_REMOVE_PHYSICALLYREMOVESELECTEDITEMS:
			RecycleSelectedItems();
			break;
		default:
		{
			if (cloud_hmenu && (r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_UPPER)) // deals with cloud specific menus
			{
				// 0 = no change
				// 1 = add cloud
				// 2 = add local
				// 4 = removed
				int mode = 0;
				WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS, (intptr_t)r, (intptr_t)&mode);
				int n = resultlist.GetSelectionMark();
				if (n != -1)
				{
					switch (mode)
					{
						case 1:
							setCloudValue(&itemCache.Items[n], L"5");
						break;

						case 2:
							setCloudValue(&itemCache.Items[n], L"0");
						break;

						case 4:
							setCloudValue(&itemCache.Items[n], L"4");
						break;
					}
					InvalidateRect(resultlist.getwnd(), NULL, TRUE);
				}
				break;
			}

			if (s.mode == 2)
			{
				s.menu_id = r;
				if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU) == (LRESULT)-1)
				{
					// build my data.
					s.mode = 3;
					s.data_type = ML_TYPE_ITEMRECORDLISTW;
					itemRecordListW myObj = {0, };
					copyFilesToItemCacheW(&myObj); // does not dupe strings
					s.data = (void*) & myObj;
					LRESULT result = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM) & s, IPC_LIBRARY_SENDTOMENU);
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
					_aligned_free(myObj.Items);
				}
			}
			break;
		}
	}

	if (s.mode)
	{
		s.mode = 4;
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU); // cleanup
	}
	sendto_hmenu = 0;
	DestroyMenu(cloud_hmenu);
	cloud_hmenu = 0;
	DestroyMenu(globmenu);
	UpdateWindow(hwndFrom);
	EatKeyboard();
}

static BOOL ListView_OnFindItem(HWND hwndDlg, NMLVFINDITEMW *pfi, LRESULT *pResult, UINT uMsg)
{
	if (bgThread_Handle) return FALSE;

	int i = pfi->iStart;
	if (i >= itemCache.Size) i = 0;

	int cnt = itemCache.Size - i;
	if (pfi->lvfi.flags & LVFI_WRAP) cnt += i;

	int by = g_view_metaconf->ReadInt(L"mv_sort_by", MEDIAVIEW_COL_ARTIST);

	while (cnt-- > 0)
	{
		itemRecordW *thisitem = itemCache.Items + i;
		wchar_t tmp[128] = {0};
		wchar_t *name = 0;

		switch (by)
		{
			case MEDIAVIEW_COL_FILENAME:
				name = thisitem->filename + wcslen(thisitem->filename);
				while (name >= thisitem->filename && *name != L'/' && *name != L'\\') name--;
				break;
			case MEDIAVIEW_COL_FULLPATH:
				name = thisitem->filename;
				break;
			case MEDIAVIEW_COL_EXTENSION:
				name = PathFindExtensionW(thisitem->filename);
				if (name && *name)
					name++;
				break;
			case MEDIAVIEW_COL_TITLE: name = thisitem->title; break;
			case MEDIAVIEW_COL_COMMENT: name = thisitem->comment; break;
			case MEDIAVIEW_COL_ARTIST: name = thisitem->artist; break;
			case MEDIAVIEW_COL_ALBUM: name = thisitem->album; break;
			case MEDIAVIEW_COL_GENRE: name = thisitem->genre; break;
			case MEDIAVIEW_COL_YEAR:
				tmp[0] = 0;
				if (thisitem->year >= 0) StringCchPrintfW(tmp, 128,  L"%04d", thisitem->year);
				name = tmp;
				break;
			case MEDIAVIEW_COL_TRACK:
				tmp[0] = 0;
				if (thisitem->track > 0)
				{
					if (thisitem->tracks > 0) StringCchPrintfW(tmp, 128, L"%d/%d", thisitem->track, thisitem->tracks);
					else StringCchPrintfW(tmp, 128, L"%d", thisitem->track);
				}
				name = tmp;
				break;
			case MEDIAVIEW_COL_LENGTH:
				tmp[0] = 0;
				if (thisitem->length >= 0) StringCchPrintfW(tmp, 128, L"%d:%02d", thisitem->length / 60, thisitem->length % 60);
				name = tmp;
				break;
			case MEDIAVIEW_COL_RATING:
				tmp[0] = 0;
				if (thisitem->rating > 0) StringCchPrintfW(tmp, 128, L"%d", thisitem->rating);
				name = tmp;
				break;
			case MEDIAVIEW_COL_CLOUD:
			{
				tmp[0] = 0;
				wchar_t *x = getRecordExtendedItem_fast(thisitem, extended_fields.cloud);
				if (x && *x) StringCchPrintfW(tmp, 128, L"%d", x);
				name = tmp;
				break;
			}
			case MEDIAVIEW_COL_PLAYCOUNT:
				tmp[0] = 0;
				if (thisitem->playcount > 0) StringCchPrintfW(tmp, 128, L"%d", thisitem->playcount);
				name = tmp;
				break;
			case MEDIAVIEW_COL_BITRATE:
				if (thisitem->bitrate > 0)
					StringCchPrintfW(name = tmp, 128, L"%d%s", thisitem->bitrate, WASABI_API_LNGSTRINGW(IDS_KBPS));
				break;
			case MEDIAVIEW_COL_BPM:
				if (thisitem->bpm > 0)
					StringCchPrintfW(name = tmp, 128, L"%d", thisitem->bpm);
				break;
			case MEDIAVIEW_COL_TYPE:
				name = WASABI_API_LNGSTRINGW(thisitem->type ? IDS_VIDEO : IDS_AUDIO);
				break;
			case MEDIAVIEW_COL_DISC:
				tmp[0] = 0;
				if (thisitem->disc > 0)
				{
					if (thisitem->discs > 0)
						StringCchPrintfW(tmp, 128, L"%d/%d", thisitem->disc, thisitem->discs);
					else
						StringCchPrintfW(tmp, 128, L"%d", thisitem->disc);
				}
				name = tmp;
				break;
			case MEDIAVIEW_COL_ALBUMARTIST:
				name = thisitem->albumartist;
				break;
			case MEDIAVIEW_COL_PUBLISHER:
				name = thisitem->publisher;
				break;
			case MEDIAVIEW_COL_COMPOSER:
				name = thisitem->composer;
				break;
			case MEDIAVIEW_COL_ALBUMGAIN:
				name = thisitem->replaygain_album_gain;
				break;
			case MEDIAVIEW_COL_TRACKGAIN:
				name = thisitem->replaygain_track_gain;
				break;
			case MEDIAVIEW_COL_FILESIZE:
				tmp[0] = 0;
				if (thisitem->filesize > 0) StringCchPrintfW(tmp, 128, L"%d", thisitem->filesize);
				name = tmp;
				break;
			case MEDIAVIEW_COL_FILETIME:
				tmp[0] = 0;
				if (thisitem->filetime > 0) StringCchPrintfW(tmp, 128,  L"%d", thisitem->filetime);
				name = tmp;
				break;
			case MEDIAVIEW_COL_LASTUPD:
				tmp[0] = 0;
				if (thisitem->lastupd > 0) StringCchPrintfW(tmp, 128, L"%d", thisitem->lastupd);
				name = tmp;
				break;
			case MEDIAVIEW_COL_DATEADDED:
			{
				tmp[0] = 0;
				wchar_t *x = getRecordExtendedItem_fast(thisitem,extended_fields.dateadded);
				if (x && *x) StringCchPrintfW(tmp, 128, L"%d", x);
				name = tmp;
				break;
			}
			case MEDIAVIEW_COL_LASTPLAY:
				tmp[0] = 0;
				if (thisitem->lastplay > 0) StringCchPrintfW(tmp, 128, L"%d", thisitem->lastplay);
				name = tmp;
				break;
			case MEDIAVIEW_COL_ISPODCAST:
			{
				wchar_t * t = getRecordExtendedItem_fast(thisitem, extended_fields.ispodcast);
				name = WASABI_API_LNGSTRINGW(t && wcscmp(t,L"1") ? IDS_PODCAST : IDS_NON_PODCAST);
			}
			break;
			case MEDIAVIEW_COL_PODCASTCHANNEL:
				name = getRecordExtendedItem_fast(thisitem,extended_fields.podcastchannel);
				break;
			case MEDIAVIEW_COL_PODCASTPUBDATE:
			{
				tmp[0] = 0;
				wchar_t *x = getRecordExtendedItem_fast(thisitem,extended_fields.podcastpubdate);
				if (x && *x) StringCchPrintfW(tmp, 128, L"%d", x);
				name = tmp;
			}
			break;
			case MEDIAVIEW_COL_CATEGORY:
				name = thisitem->category;
				break;
			case MEDIAVIEW_COL_DIRECTOR:
				name = getRecordExtendedItem_fast(thisitem,extended_fields.director);
				break;
			case MEDIAVIEW_COL_PRODUCER:
				name = getRecordExtendedItem_fast(thisitem,extended_fields.producer);
				break;
			case MEDIAVIEW_COL_DIMENSION:
			{
				tmp[0] = 0;
				wchar_t *w = getRecordExtendedItem_fast(thisitem,extended_fields.width);
				wchar_t *h = getRecordExtendedItem_fast(thisitem,extended_fields.height);
				if (w && *w && h && *h) StringCchPrintfW(tmp, 128, L"%dx%d", _wtoi(w), _wtoi(h));
				name = tmp;
				break;
			}
		}

		if (!name) name = L"";
		else SKIP_THE_AND_WHITESPACEW(name)

		if (pfi->lvfi.flags & (4 | LVFI_PARTIAL))
		{
			if (!StrCmpNIW(name, pfi->lvfi.psz, wcslen(pfi->lvfi.psz)))
			{
				*pResult = i;
				return TRUE;
			}
		}
		else if (pfi->lvfi.flags & LVFI_STRING)
		{
			if (!lstrcmpiW(name, pfi->lvfi.psz))
			{
				*pResult = i;
				return TRUE;
			}
		}
		else
		{
			*pResult = i;
			return TRUE;
		}
		if (++i == itemCache.Size) i = 0;
	}
	*pResult = i;
	return TRUE;
}
static BOOL ListView_OnGetDispInfo(HWND hwndDlg, NMLVDISPINFOW* pdi, UINT uMsg)
{
	LVITEMW *pItem;
	itemRecordW *pRec;

	pItem = &pdi->item;

	if (bgThread_Handle)
	{
		if (0 == pItem->iItem && 0 == pItem->iSubItem && (LVIF_TEXT & pItem->mask))
		{
			static char bufpos = 0;
			static int buflen = 17;
			static wchar_t buffer[64];//L"Scanning  _\0/-\\|";
			if (!buffer[0])
			{
				WASABI_API_LNGSTRINGW_BUF(IDS_SCANNING_PLAIN,buffer,54);
				StringCchCatW(buffer,64,L"  _");
				StringCchCatW(buffer+(buflen=lstrlenW(buffer)+1),64,L"/-\\|");
				buflen+=4;
			}
			int pos = buflen - 5;;
			buffer[pos - 1] = buffer[pos + (bufpos++&3) + 1];
			pItem->pszText = buffer;
		}
		return FALSE;
	}

	if (pItem->iItem < 0 || pItem->iItem >= itemCache.Size) return FALSE;

	pRec = itemCache.Items + pItem->iItem;

	if (LVIF_TEXT & pItem->mask)
	{
		switch (columnOrder[pItem->iSubItem])
		{
			case MEDIAVIEW_COL_FILENAME:      // show filename (Tho we'll show without the path cause paths are gay)
				pItem->pszText = PathFindFileNameW(pRec->filename);
				break;
			case MEDIAVIEW_COL_FULLPATH:      // show filename (Tho we'll show without the path cause paths are gay)
				pItem->pszText = pRec->filename;
				break;
			case MEDIAVIEW_COL_EXTENSION:
				pItem->pszText = PathFindExtensionW(pRec->filename);
				if (pItem->pszText && *pItem->pszText) pItem->pszText++;
				break;
			case MEDIAVIEW_COL_TITLE:
				pItem->pszText = pRec->title;
				break;
			case MEDIAVIEW_COL_COMMENT:
				pItem->pszText = pRec->comment;
				break;
			case MEDIAVIEW_COL_ALBUM:
				pItem->pszText = pRec->album;
				break;
			case MEDIAVIEW_COL_ARTIST:
				pItem->pszText = pRec->artist;
				break;
			case MEDIAVIEW_COL_TRACK:
				if (pRec->track > 0)
				{
					if (pRec->tracks > 0) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d/%d", pRec->track, pRec->tracks);
					else StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d", pRec->track);
				}
				break;
			case MEDIAVIEW_COL_GENRE:
				pItem->pszText = pRec->genre;
				break;
			case MEDIAVIEW_COL_YEAR:
				if (pRec->year >= 0) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%04d", pRec->year);
				break;
			case MEDIAVIEW_COL_LENGTH:
				if (pRec->length >= 0) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d:%02d", pRec->length / 60, pRec->length % 60);
				break;
			case MEDIAVIEW_COL_RATING:
				if (0 == pItem->iSubItem && ratingBackText[0]) pItem->pszText = ratingBackText;
				else if (pRec->rating > 0 && pRec->rating <= 5) _itow(pRec->rating, pItem->pszText, 10);
				break;
			case MEDIAVIEW_COL_CLOUD:
			{
				wchar_t *t = getRecordExtendedItem_fast(pRec, extended_fields.cloud);
				if (t && *t) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d", _wtoi(t));
				break;
			}
			case MEDIAVIEW_COL_PLAYCOUNT:
				if (pRec->playcount >= 0) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d", pRec->playcount);
				else pItem->pszText = L"0";
				break;
			case MEDIAVIEW_COL_DISC:
				if (pRec->disc > 0)
				{
					if (pRec->discs > 0) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d/%d", pRec->disc, pRec->discs);
					else StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d", pRec->disc);
				}
				break;
			case MEDIAVIEW_COL_ALBUMARTIST:
				pItem->pszText = pRec->albumartist;
				break;
			case MEDIAVIEW_COL_PUBLISHER:
				pItem->pszText = pRec->publisher;
				break;
			case MEDIAVIEW_COL_COMPOSER:
				pItem->pszText = pRec->composer;
				break;
			case MEDIAVIEW_COL_ALBUMGAIN:
				pItem->pszText = pRec->replaygain_album_gain;
				break;
			case MEDIAVIEW_COL_TRACKGAIN:
				pItem->pszText = pRec->replaygain_track_gain;
				break;
			case MEDIAVIEW_COL_TYPE:
				pItem->pszText = WASABI_API_LNGSTRINGW((pRec->type) ? IDS_VIDEO : IDS_AUDIO);
				break;
			case MEDIAVIEW_COL_BITRATE:
				if (pRec->bitrate > 0) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d%s", pRec->bitrate, WASABI_API_LNGSTRINGW(IDS_KBPS));
				break;
			case MEDIAVIEW_COL_BPM:
				if (pRec->bpm > 0) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%d", pRec->bpm);
				break;
			case MEDIAVIEW_COL_FILESIZE:
				if (pRec->filesize != -1) WASABI_API_LNG->FormattedSizeString(pItem->pszText, pItem->cchTextMax, pRec->filesize);
				break;
			case MEDIAVIEW_COL_FILETIME:
				MakeDateString(pRec->filetime, pItem->pszText, pItem->cchTextMax);
				break;
			case MEDIAVIEW_COL_LASTUPD:
				MakeDateString(pRec->lastupd, pItem->pszText, pItem->cchTextMax);
				break;
			case MEDIAVIEW_COL_DATEADDED:
			{
				wchar_t * t = getRecordExtendedItem_fast(pRec,extended_fields.dateadded);
				if (t && *t) MakeDateString(_wtoi64(t), pItem->pszText, pItem->cchTextMax);
				break;
			}
			case MEDIAVIEW_COL_LASTPLAY:
				if (pRec->lastplay > 0) MakeDateString(pRec->lastplay, pItem->pszText, pItem->cchTextMax);
				break;
			case MEDIAVIEW_COL_ISPODCAST:
			{
				wchar_t *t = getRecordExtendedItem_fast(pRec, extended_fields.ispodcast);
				pItem->pszText = WASABI_API_LNGSTRINGW((t && t[0] == L'1' && !t[1]) ? IDS_PODCAST : IDS_NON_PODCAST);
				break;
			}
			case MEDIAVIEW_COL_PODCASTCHANNEL:
				pItem->pszText = getRecordExtendedItem_fast(pRec, extended_fields.podcastchannel);
				break;
			case MEDIAVIEW_COL_PODCASTPUBDATE:
			{
				wchar_t * t = getRecordExtendedItem_fast(pRec,extended_fields.podcastpubdate);
				if (t && *t) MakeDateString(_wtoi64(t), pItem->pszText, pItem->cchTextMax);
				break;
			}
			case MEDIAVIEW_COL_CATEGORY:
				pItem->pszText = pRec->category;
				break;
			case MEDIAVIEW_COL_DIRECTOR:
				pItem->pszText = getRecordExtendedItem_fast(pRec, extended_fields.director);
				break;
			case MEDIAVIEW_COL_PRODUCER:
				pItem->pszText = getRecordExtendedItem_fast(pRec, extended_fields.producer);
				break;
			case MEDIAVIEW_COL_DIMENSION:
			{
				wchar_t *w = getRecordExtendedItem_fast(pRec,extended_fields.width);
				wchar_t *h = getRecordExtendedItem_fast(pRec,extended_fields.height);
				if (w && *w && h && *h) StringCchPrintfW(pItem->pszText, pItem->cchTextMax, L"%dx%d", _wtoi(w), _wtoi(h));
				break;
			}
		}
		if (!pItem->pszText)  pItem->pszText = L"";
	}
	return FALSE;
}
static BOOL ListView_OnColumnClick(HWND hwndDlg, NMLISTVIEW *pnmv)
{
	int l_sc = g_view_metaconf->ReadInt(L"mv_sort_by", MEDIAVIEW_COL_ARTIST);
	int l_sd = g_view_metaconf->ReadInt(L"mv_sort_dir", 0);
	if (columnOrder[pnmv->iSubItem] == l_sc) l_sd = !l_sd;
	else
	{
		/* JF> I like this better when the direction doesnt get reset every
		time you choose a new column. To revert to old behavior, uncomment
		this line:
		l_sd=0;
		*/
		l_sc = columnOrder[pnmv->iSubItem];
	}

	g_view_metaconf->WriteInt(L"mv_sort_by", l_sc);
	g_view_metaconf->WriteInt(L"mv_sort_dir", l_sd);
	MLSkinnedListView_DisplaySort(pnmv->hdr.hwndFrom, pnmv->iSubItem, !l_sd);
	sortResults(g_view_metaconf, &itemCache);
	resultlist.SetVirtualCount(0);
	resultlist.SetVirtualCount(itemCache.Size); // TODO: we could set a limit here
	ListView_RedrawItems(pnmv->hdr.hwndFrom, 0, itemCache.Size - 1);
	return FALSE;
}
static BOOL ListView_OnBeginDrag(HWND hwndDlg, NMLISTVIEW *pnmv)
{
	switch (columnOrder[pnmv->iSubItem])
	{
		case MEDIAVIEW_COL_RATING:
			ratingColumn.hwndList = pnmv->hdr.hwndFrom;
			ratingColumn.fStyle = RCS_DEFAULT;
			ratingColumn.iItem = pnmv->iItem;
			ratingColumn.iSubItem = pnmv->iSubItem;
			ratingColumn.value = ((UINT)pnmv->iItem < (UINT)itemCache.Size) ? itemCache.Items[pnmv->iItem].rating : 0;
			MLRatingColumn_BeginDrag(plugin.hwndLibraryParent, &ratingColumn);
		break;
	}

	SetCapture(hwndDlg);
	return FALSE;
}

static BOOL ListView_OnReturn(HWND hwndDlg, NMHDR *pnmh)
{
	SendMessage(hwndDlg, WM_COMMAND, ((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^(!!g_config->ReadInt(L"enqueuedef", 0)))
	            ? IDC_BUTTON_ENQUEUE : IDC_BUTTON_PLAY, 0);
	return FALSE;
}

static BOOL ListView_OnCustomDraw(HWND hwndDlg, NMLVCUSTOMDRAW *plvcd, LRESULT *pResult)
{
	static BOOL bDrawFocus;
	static RECT rcView;
	static RATINGCOLUMNPAINT ratingColumnPaint;
	static CLOUDCOLUMNPAINT cloudColumnPaint;

	*pResult = CDRF_DODEFAULT;

	switch (plvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult |= CDRF_NOTIFYITEMDRAW;
			CopyRect(&rcView, &plvcd->nmcd.rc);

			ratingColumnPaint.fStyle = RCS_DEFAULT;
			ratingColumnPaint.hwndList = plvcd->nmcd.hdr.hwndFrom;
			ratingColumnPaint.hdc = plvcd->nmcd.hdc;
			ratingColumnPaint.prcView = &rcView;

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
			switch (columnOrder[plvcd->iSubItem])
			{
				case MEDIAVIEW_COL_RATING:
					if (bgThread_Handle || (0 == plvcd->iSubItem && 0 == plvcd->nmcd.rc.right)) break;
					ratingColumnPaint.iItem = plvcd->nmcd.dwItemSpec;
					ratingColumnPaint.iSubItem = plvcd->iSubItem;
					ratingColumnPaint.value = (plvcd->nmcd.dwItemSpec >= 0 && plvcd->nmcd.dwItemSpec < (UINT)itemCache.Size) ? itemCache.Items[plvcd->nmcd.dwItemSpec].rating : 0;
					ratingColumnPaint.prcItem = &plvcd->nmcd.rc;
					ratingColumnPaint.rgbBk = plvcd->clrTextBk;
					ratingColumnPaint.rgbFg = plvcd->clrText;

					if (MLRatingColumn_Paint(plugin.hwndLibraryParent, &ratingColumnPaint))
					{
						*pResult = CDRF_SKIPDEFAULT;
						return TRUE;
					}
				break;

				case MEDIAVIEW_COL_CLOUD:
					if (bgThread_Handle || (0 == plvcd->iSubItem && 0 == plvcd->nmcd.rc.right)) break;

					int icon = 4;
					wchar_t *t = getRecordExtendedItem_fast(&itemCache.Items[plvcd->nmcd.dwItemSpec], extended_fields.cloud);
					if (t && *t) icon = _wtoi(t);

					cloudColumnPaint.iItem = plvcd->nmcd.dwItemSpec;
					cloudColumnPaint.iSubItem = plvcd->iSubItem;
					cloudColumnPaint.value = icon;
					cloudColumnPaint.prcItem = &plvcd->nmcd.rc;
					cloudColumnPaint.rgbBk = plvcd->clrTextBk;
					cloudColumnPaint.rgbFg = plvcd->clrText;

					if (MLCloudColumn_Paint(plugin.hwndLibraryParent, &cloudColumnPaint))
					{
						*pResult = CDRF_SKIPDEFAULT;
						return TRUE;
					}
				break;
			}
		break;
	}
	return FALSE;
}

static BOOL ListView_OnClick(HWND hwnDlg, NMITEMACTIVATE *pnmitem)
{
	if (bgThread_Handle) return FALSE;

	if (pnmitem->iItem != -1)
	{
		switch (columnOrder[pnmitem->iSubItem])
		{
			case MEDIAVIEW_COL_RATING:
				ratingColumn.hwndList =  pnmitem->hdr.hwndFrom;
				ratingColumn.ptAction = pnmitem->ptAction;
				ratingColumn.bRedrawNow = TRUE;
				ratingColumn.fStyle = RCS_DEFAULT;
				if (!MLRatingColumn_Click(plugin.hwndLibraryParent, &ratingColumn)) return FALSE;
				SetRating(ratingColumn.iItem, ratingColumn.value, ratingColumn.hwndList);
			break;

			case MEDIAVIEW_COL_CLOUD:
			{
				RECT itemRect = {0};
				if (pnmitem->iSubItem)
					ListView_GetSubItemRect(pnmitem->hdr.hwndFrom, pnmitem->iItem, pnmitem->iSubItem, LVIR_BOUNDS, &itemRect);
				else
				{
					ListView_GetItemRect(pnmitem->hdr.hwndFrom, pnmitem->iItem, &itemRect, LVIR_BOUNDS);
					itemRect.right = itemRect.left + ListView_GetColumnWidth(pnmitem->hdr.hwndFrom, pnmitem->iSubItem);
				}

				MapWindowPoints(pnmitem->hdr.hwndFrom, HWND_DESKTOP, (POINT*)&itemRect, 2);

				HMENU cloud_menu = CreatePopupMenu();
				WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_GET_CLOUD_STATUS, (intptr_t)itemCache.Items[pnmitem->iItem].filename, (intptr_t)&cloud_menu);
				if (cloud_menu)
				{
					int r = DoTrackPopup(cloud_menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, itemRect.right, itemRect.top, pnmitem->hdr.hwndFrom, NULL);
					if (r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_UPPER)
					{
						// 0 = no change
						// 1 = adding to cloud
						// 2 = added locally
						// 4 = removed
						int mode = 0;	// deals with cloud specific menus
						WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS, (intptr_t)r, (intptr_t)&mode);
						switch (mode)
						{
							case 1:
								setCloudValue(&itemCache.Items[pnmitem->iItem], L"5");
							break;

							case 2:
								setCloudValue(&itemCache.Items[pnmitem->iItem], L"4");
							break;

							case 4:
								setCloudValue(&itemCache.Items[pnmitem->iItem], L"4");
							break;
						}

						InvalidateRect(resultlist.getwnd(), NULL, TRUE);
					}
					DestroyMenu(cloud_menu);
				}
			}
			break;
		}
	}
	return FALSE;
}

static BOOL ListView_OnHotTrack(HWND hwndDlg, NMLISTVIEW *pnmlv, LRESULT *pResult)
{
	UINT iItem;

	if (bgThread_Handle)
	{
		pnmlv->iItem = -1;
		*pResult = TRUE;
		return TRUE;
	}

	if (-1 == pnmlv->iItem && 0 == pnmlv->iSubItem)
	{
		LVHITTESTINFO lvhit;
		lvhit.pt = pnmlv->ptAction;
		SendMessageW(pnmlv->hdr.hwndFrom, LVM_HITTEST, 0, (LPARAM)&lvhit);
		iItem = lvhit.iItem;
	}
	else iItem = pnmlv->iItem;

	switch (columnOrder[pnmlv->iSubItem])
	{
		case MEDIAVIEW_COL_RATING:
			ratingColumn.hwndList = pnmlv->hdr.hwndFrom;
			ratingColumn.fStyle = RCS_DEFAULT;
			ratingColumn.iItem = iItem;
			ratingColumn.iSubItem = pnmlv->iSubItem;
			ratingColumn.value = (iItem < (UINT)itemCache.Size) ? itemCache.Items[iItem].rating : 0;
			ratingColumn.ptAction = pnmlv->ptAction;
			ratingColumn.bRedrawNow = TRUE;
			MLRatingColumn_Track(plugin.hwndLibraryParent, &ratingColumn);
		break;
	}

	// LVS_EX_ONECLICKACTIVATE enabled - make listview select nothing
	pnmlv->iItem = -1;
	*pResult = TRUE;
	return TRUE;
}

/////////// Dialog Messages / Notifications
static void Dialog_OnDisplayChange(HWND hwndDlg)
{
	INT i;
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST2);

	for (i = 0; -1 != columnOrder[i] && MEDIAVIEW_COL_RATING != columnOrder[i] && MEDIAVIEW_COL_CLOUD != columnOrder[i]; i++);
	if (-1 != columnOrder[i])
	{
		if (hwndList)
		{
			INT w = (INT)SendMessageW(hwndList, LVM_GETCOLUMNWIDTH, i, 0L);
			SendMessageW(hwndList, LVM_SETCOLUMNWIDTH, i, (LPARAM)w);
		}
	}
	LayoutWindows(hwndDlg, TRUE);
}

static wchar_t tt_buf[256] = {L""};
static int last_item = -1, last_icon = -1;
LRESULT pmp_listview(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

				int cloudcol = (int)GetPropW(hwnd, L"pmp_list_info");
				if (lvh.iItem != -1 && lvh.iSubItem == cloudcol)
				{
					RECT r = {0};
					if (lvh.iSubItem)
						ListView_GetSubItemRect(hwnd, lvh.iItem, lvh.iSubItem, LVIR_BOUNDS, &r);
					else
					{
						ListView_GetItemRect(hwnd, lvh.iItem, &r, LVIR_BOUNDS);
						r.right = r.left + ListView_GetColumnWidth(hwnd, cloudcol);
					}

					MapWindowPoints(hwnd, HWND_DESKTOP, (LPPOINT)&r, 2);
					SetWindowPos(l->hwndFrom, HWND_TOPMOST, r.right, r.top + 2, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
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

				int cloudcol = (int)GetPropW(hwnd, L"pmp_list_info");
				if (lvh.iItem != -1 && lvh.iSubItem == cloudcol)
				{
					LPNMTTDISPINFOW lpnmtdi = (LPNMTTDISPINFOW)lParam;
					int icon = 4;
					wchar_t *t = getRecordExtendedItem_fast(&itemCache.Items[lvh.iItem], extended_fields.cloud);
					if (t && *t) icon = _wtoi(t);

					if (last_item == lvh.iItem && last_icon == icon)
					{
						lpnmtdi->lpszText = tt_buf;
						return 0;
					}

					if (icon == 4)
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_UPLOAD_TO_SOURCE, tt_buf, ARRAYSIZE(tt_buf));
					}
					else if (icon == 5)
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_UPLOADING_TO_SOURCE, tt_buf, ARRAYSIZE(tt_buf));
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

									nx_string_t *out_devicenames = 0;
									size_t num_names = mlplugin->MessageProc(0x405, (INT_PTR)itemCache.Items[lvh.iItem].filename, (INT_PTR)&out_devicenames, 0);
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
										free(out_devicenames);
								}
							}
						}
					}
					last_item = lvh.iItem;
					last_icon = icon;
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

void Dialog_UpdateButtonText(HWND hwndDlg, int _enqueuedef)
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

BOOL Dialog_ButtonPopupMenu(HWND hwndDlg, int buttonId, HMENU menu, int flags=0)
{
	RECT r;
	HWND buttonHWND = GetDlgItem(hwndDlg, buttonId);
	GetWindowRect(buttonHWND, &r);
	UpdateMenuItems(hwndDlg, menu, IDR_VIEW_ACCELERATORS);
	MLSkinnedButton_SetDropDownState(buttonHWND, TRUE);
	UINT tpmFlags = TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN;
	if (!(flags & BPM_WM_COMMAND)) tpmFlags |= TPM_RETURNCMD;
	int x = DoTrackPopup(menu, tpmFlags, r.left, r.top, hwndDlg, NULL);
	if ((flags & BPM_ECHO_WM_COMMAND) && x)
		SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(x, 0), 0);
	MLSkinnedButton_SetDropDownState(buttonHWND, FALSE);
	return x;
}

static void Dialog_Play(HWND hwndDlg, HWND from, UINT idFrom)
{
	HMENU listMenu = GetSubMenu(g_context_menus2, 0);
	int count = GetMenuItemCount(listMenu);
	if (count > 2)
	{
		for (int i = 2; i < count; i++)
		{
			DeleteMenu(listMenu, 2, MF_BYPOSITION);
		}
	}

	Dialog_ButtonPopupMenu(hwndDlg, idFrom, listMenu, BPM_WM_COMMAND);
}

static INT_PTR Dialog_OnInit(HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
	// benski> this is just going here because it's very likely to get called. will only get compiled in debug mode.  
	assert(sizeof(extra_idsW) / sizeof(*extra_idsW) == sizeof(extra_strsW) / sizeof(*extra_strsW));

	g_displaysearch = !(BOOL)lParam;

	// Set the hwnd for the search to a global only if the multipane view hasnt set it yet, as it takes precedence to which box gets populated with the query
	//if (!IsWindow(hwndSearchGlobal))
		hwndSearchGlobal = GetDlgItem(hwndDlg, IDC_QUICKSEARCH);		

	HACCEL accel = WASABI_API_LOADACCELERATORSW(IDR_VIEW_ACCELERATORS);
	if (accel)
		WASABI_API_APP->app_addAccelerators(hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD);

	if (!view.play)
	{
		SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view);
	}

	HWND hwndList;
	FLICKERFIX ff;
	INT index;
	INT ffcl[] = {	IDC_CLEAR,
	                IDC_BUTTON_PLAY,
	                IDC_BUTTON_ENQUEUE,
	                IDC_BUTTON_MIX,
	                IDC_BUTTON_INFOTOGGLE,
					IDC_BUTTON_CREATEPLAYLIST,
	                IDC_MIXABLE,
	                IDC_MEDIASTATUS,
	             };

	m_hwnd = hwndDlg;
	m_bgupdinfoviewerflag = 0;
	columnOrder[0] = -1;
	last_item = -1;
	tt_buf[0] = 0;

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
				int64_t *out_ids = 0;
				nx_string_t *out_filenames = 0;
				size_t num_files = mlplugin->MessageProc(0x404, (INT_PTR)&out_filenames, (INT_PTR)&out_ids, 0xDEADBEEF);
				for (size_t i = 0; i < num_files; i++)
				{
					cloudFiles.push_back((wchar_t *)out_filenames[i]->string);
				}
				if (out_filenames)
				{
					free(out_filenames);
					out_filenames = 0;
				}
				if (out_ids)
				{
					free(out_ids);
					out_ids = 0;
				}

				HWND ml_pmp_window = FindWindowW(L"ml_pmp_window", NULL);
				if (IsWindow(ml_pmp_window))
				{
					SendMessage(ml_pmp_window, WM_PMP_IPC, (WPARAM)&cloudUploading, PMP_IPC_GETCLOUDTRANSFERS);
					wchar_t a[32] = {0};
					StringCchPrintfW(a, 32, L"%d", cloudUploading.size());
				}
			}
		}
	}

	EnterCriticalSection(&g_db_cs);
	if (!m_media_scanner) m_media_scanner = NDE_Table_CreateScanner(g_table);
	LeaveCriticalSection(&g_db_cs);

	itemCache.Items = 0;
	itemCache.Alloc = 0;
	itemCache.Size = 0;

	hwndList = GetDlgItem(hwndDlg, IDC_LIST2);
	if (IsWindow(hwndList))
	{
		resultlist.setwnd(hwndList);
		resultlist.ForceUnicode();

		DWORD styleEx = LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE; /*LVS_EX_ONECLICKACTIVATE - needed to hottracking work prior WinXp */
		SendMessageW(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx, styleEx);

		HWND hwndHeader = (HWND)SendMessage(hwndList, LVM_GETHEADER, 0, 0L);
		if (IsWindow(hwndHeader)) SetWindowLongPtrW(hwndHeader, GWLP_ID, IDC_LIST2HEADER);

		MLSKINWINDOW skin = {0};
		skin.hwndToSkin = hwndList;
		skin.skinType = SKINNEDWND_TYPE_LISTVIEW;
		skin.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
		MLSkinWindow(plugin.hwndLibraryParent, &skin);
	}
	else resultlist.setwnd(NULL);

	ff.mode = FFM_ERASEINPAINT;
	for (index = 0; index < sizeof(ffcl) / sizeof(INT); index++)
	{
		ff.hwnd = GetDlgItem(hwndDlg, ffcl[index]);
		SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_FLICKERFIX, (WPARAM)&ff);
	}

	if (!g_displaysearch) // disable search box
	{
		ShowWindow(GetDlgItem(hwndDlg, IDC_QUICKSEARCH), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CLEAR), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_SEARCHCAPTION), SW_HIDE);
	}

	if (!cloud_hinst || cloud_hinst == (HINSTANCE)1)
		memcpy(columnOrder, defColumnOrder, sizeof(defColumnOrder));
	else
		memcpy(columnOrder, defColumnOrderCloud, sizeof(defColumnOrderCloud));
	//Read the column order

	Dialog_OnDisplayChange(hwndDlg);

	int cloudcol = -1;
	int l = g_view_metaconf->ReadInt(L"nbcolumns", 0);
	if (l)
	{
		if (l > MAX_COLUMN_ORDER - 1) l = MAX_COLUMN_ORDER - 1;
		index = 0;
		for (; index < l; index++)
		{
			wchar_t tmp[128] = {0};
			StringCchPrintfW(tmp, 128, L"column%d", index);
			int v = g_view_metaconf->ReadInt(tmp, 0);
			if (v == MEDIAVIEW_COL_CLOUD) cloudcol = index;
			if (v < 0 || v >= MEDIAVIEW_COL_NUMS) v = 0;
			columnOrder[index] = (BYTE)v;
		}
		columnOrder[index] = -1;
	}

	if (cloudcol == -1 && !g_view_metaconf->ReadInt(L"cloud", 1))
	{
		g_view_metaconf->WriteInt(L"cloud", 1);
		for(int i = l; i != 0; i--)
		{
			columnOrder[i+1] = columnOrder[i];
			if (i == 3)
			{
				columnOrder[i] = MEDIAVIEW_COL_CLOUD;
				break;
			}
		}
	}

	if (!GetPropW(hwndList, L"pmp_list_proc")) {
		SetPropW(hwndList, L"pmp_list_proc", (HANDLE)SetWindowLongPtrW(hwndList, GWLP_WNDPROC, (LONG_PTR)pmp_listview));
	}
	initColumnsHeader(hwndList);

	char *pszTextA = (g_config->ReadInt(L"remembersearch", 0)) ? g_view_metaconf->ReadString("lastquery_utf8", "") : "";
	AutoWide queryUnicode(pszTextA, CP_UTF8);
	SetDlgItemTextW(hwndDlg, IDC_QUICKSEARCH, queryUnicode);
	KillTimer(hwndDlg, UPDATE_QUERY_TIMER_ID);
	doQuery(hwndDlg, queryUnicode, 0);

	updateInfoText(hwndDlg);

	search_oldWndProc = (WNDPROC)SetWindowLongPtrW(GetDlgItem(hwndDlg, IDC_QUICKSEARCH), GWLP_WNDPROC, (LONG_PTR)search_newWndProc);

	if (g_table && !NDE_Table_GetRecordsCount(g_table) && !g_config->ReadInt(L"noshowadddlg", 0))
	{
		SetTimer(hwndDlg, 5050, 1000, NULL);
	}

	groupBtn = g_config->ReadInt(L"groupbtn", 1);
	enqueuedef = (g_config->ReadInt(L"enqueuedef", 0) == 1);

	/// detect predixis
	predixisExist = FALSE;
	//predixis out	- begin
	//pluginMessage p = {ML_MSG_PDXS_STATUS, (INT_PTR)"test", 0, 0};
	//pszTextA = (char *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
	//predixisExist = (pszTextA != NULL && pszTextA[0] != 0);
	//predixis out  - end

	// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
	//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
	pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG(IDC_BUTTON_MIX, IDC_BUTTON_ENQUEUE), (INT_PTR)L"ml_local"};
	wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
	if (pszTextW && pszTextW[0] != 0)
	{
		// set this to be a bit different so we can just use one button and not the
		// mixable one as well (leaving that to prevent messing with the resources)
		customAllowed = TRUE;
		SetDlgItemTextW(hwndDlg, IDC_BUTTON_MIX, pszTextW);
	}
	else
		customAllowed = FALSE;

	ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON_MIX), (!customAllowed ? SW_HIDE : SW_SHOW));
	ShowWindow(GetDlgItem(hwndDlg, IDC_MIXABLE), (!(predixisExist & 1) ? SW_HIDE : SW_SHOW));

	MLSKINWINDOW m = {0};
	m.hwndToSkin = hwndDlg;
	m.skinType = SKINNEDWND_TYPE_AUTO;
	m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
	MLSkinWindow(plugin.hwndLibraryParent, &m);

	m.skinType = SKINNEDWND_TYPE_BUTTON;
	m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | (groupBtn ? SWBS_SPLITBUTTON : 0);

	const int buttonids[] = {IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_MIX};
	for (size_t i=0;i!=sizeof(buttonids)/sizeof(buttonids[0]);i++)
	{
		m.hwndToSkin = GetDlgItem(hwndDlg, buttonids[i]);
		if (IsWindow(m.hwndToSkin)) MLSkinWindow(plugin.hwndLibraryParent, &m);
	}

	m.skinType = SKINNEDWND_TYPE_AUTO;
	m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;

	const int buttonidz[] = {IDC_SEARCHCAPTION, IDC_QUICKSEARCH, IDC_MEDIASTATUS, IDC_CLEAR, IDC_BUTTON_INFOTOGGLE, IDC_BUTTON_CREATEPLAYLIST, IDC_MIXABLE};
	for (size_t i=0;i!=sizeof(buttonidz)/sizeof(buttonidz[0]);i++)
	{
		m.hwndToSkin = GetDlgItem(hwndDlg, buttonidz[i]);
		if (IsWindow(m.hwndToSkin)) MLSkinWindow(plugin.hwndLibraryParent, &m);
	}

	Dialog_UpdateButtonText(hwndDlg, enqueuedef);
	return FALSE;
}

static BOOL Dialog_OnNotify(HWND hwndDlg, INT idCtrl, NMHDR* pnmh, LRESULT *pResult)
{
	switch (pnmh->idFrom)
	{
		case IDC_LIST2:
			switch (pnmh->code)
			{
				case LVN_ITEMCHANGED:	return ListView_OnItemChanged(hwndDlg, (NMLISTVIEW*)pnmh);
				case NM_DBLCLK:			return ListView_OnDoubleClick(hwndDlg, (NMITEMACTIVATE*)pnmh);
				case LVN_ODFINDITEMA:
				case LVN_ODFINDITEMW:	return ListView_OnFindItem(hwndDlg, (NMLVFINDITEMW*)pnmh, pResult, pnmh->code);
				case LVN_GETDISPINFOA:
				case LVN_GETDISPINFOW:	return ListView_OnGetDispInfo(hwndDlg, (NMLVDISPINFOW*)pnmh, pnmh->code);
				case LVN_COLUMNCLICK:	return ListView_OnColumnClick(hwndDlg, (NMLISTVIEW*)pnmh);
				case LVN_BEGINDRAG:		return ListView_OnBeginDrag(hwndDlg, (NMLISTVIEW*)pnmh);
				case NM_RETURN:			return ListView_OnReturn(hwndDlg, pnmh);
				case NM_CUSTOMDRAW:		return ListView_OnCustomDraw(hwndDlg, (NMLVCUSTOMDRAW*)pnmh, pResult);
				case LVN_HOTTRACK:		return ListView_OnHotTrack(hwndDlg, (NMLISTVIEW*)pnmh, pResult);
				case NM_CLICK:			return ListView_OnClick(hwndDlg, (NMITEMACTIVATE*)pnmh);
			}
			break;
		case IDC_LIST2HEADER:
			switch (pnmh->code)
			{
				case NM_RCLICK:			return Header_OnRightClick(hwndDlg, pnmh, pResult);
				case HDN_ENDDRAG:		return Header_OnEndDrag(hwndDlg, (NMHEADERW*)pnmh, pResult);
				case HDN_ITEMCHANGINGA:
				case HDN_ITEMCHANGINGW:	return Header_OnItemChanging(hwndDlg, (NMHEADERW*)pnmh, pResult, pnmh->code);
			}
			break;
	}
	return FALSE;
}

static void Dialog_OnInitMenuPopup(HWND hwndDlg, HMENU  hMenu, UINT nIndex, BOOL bSysMenu)
{
	if (hMenu && hMenu == s.build_hMenu && s.mode == 1)
	{
		myMenu = TRUE;
		if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU) == (LRESULT)-1)
			s.mode = 2;
		myMenu = FALSE;
	}
	if (rate_hmenu && hMenu == rate_hmenu)
	{
		int x;
		int sel = 0;
		for (x = 0; x < itemCache.Size; x ++)
		{
			if (resultlist.GetSelected(x))
			{
				int s = itemCache.Items[x].rating;
				if (s == sel || !sel) sel = s;
				if (s != sel) break;
			}
		}
		if (-1 == sel) sel = 0;
		Menu_SetRatingValue(rate_hmenu, sel);
	}
	if (cloud_hmenu && hMenu == cloud_hmenu)
	{
		int n = resultlist.GetSelectionMark();
		if (n != -1 && !GetMenuItemCount(hMenu))
		{
			WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_GET_CLOUD_STATUS, (intptr_t)itemCache.Items[n].filename, (intptr_t)&cloud_hmenu);
		}
	}
}

static void Dialog_OnMouseMove(HWND hwndDlg, UINT nFlags, POINTS pts)
{
	if (GetCapture() == hwndDlg)
	{
		mlDropItemStruct m = {0};

		POINTSTOPOINT(m.p, pts);
		MapWindowPoints(hwndDlg, HWND_DESKTOP, (POINT*)&m.p, 1);

		if (MLRatingColumn_Drag(plugin.hwndLibraryParent, &m.p))  return;

		m.type = ML_TYPE_ITEMRECORDLIST;
		pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);

	}
}
static void Dialog_OnLButtonUp(HWND hwndDlg, UINT nFlags, POINTS pts)
{
	if (GetCapture() == hwndDlg)
	{
		mlDropItemStruct m = {0};

		ReleaseCapture();

		POINTSTOPOINT(m.p, pts);
		MapWindowPoints(hwndDlg, HWND_DESKTOP, (POINT*)&m.p, 1);

		ratingColumn.bCanceled =  FALSE;
		ratingColumn.ptAction = m.p;
		ratingColumn.bRedrawNow = TRUE;

		if (MLRatingColumn_EndDrag(plugin.hwndLibraryParent, &ratingColumn))
		{
			SetRating(ratingColumn.iItem, ratingColumn.value, ratingColumn.hwndList);
			return;
		}

		m.type = ML_TYPE_ITEMRECORDLISTW;
		m.flags = ML_HANDLEDRAG_FLAG_NOCURSOR;


		pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);

		if (m.result > 0) // try itemRecordListW
		{
			itemRecordListW myObj = {0, };
			copyFilesToItemCacheW(&myObj);
			m.flags = 0;
			m.result = 0;
			m.data = (void*) & myObj;
			pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
			_aligned_free(myObj.Items); // DO NOT empty this object, cause it doesnt own its data
		}
		else // if it didn't work, fall back to itemRecordList
		{
			m.type = ML_TYPE_ITEMRECORDLIST;
			m.result = 0;

			pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
			if (m.result > 0)
			{
				itemRecordListW myObj = {0, };
				copyFilesToItemCacheW(&myObj);

				itemRecordList objA = {0, };
				convertRecordList(&objA, &myObj);
				m.flags = 0;
				m.result = 0;
				m.data = (void*) & objA;
				pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
				emptyRecordList(&objA);
				freeRecordList(&objA);
				_aligned_free(myObj.Items); // DO NOT empty this object, cause it doesnt own its data
			}
		}
	}
}

class ItemRecordPlaylist : public ifc_playlist 
{
public:
	ItemRecordPlaylist(const itemRecordListW *_list)
	{
		list = _list;
	}

private:
	size_t GetNumItems()
	{
		return list->Size;
	}

	size_t GetItem(size_t item, wchar_t *filename, size_t filenameCch)
	{
		if (item < (size_t)list->Size && list->Items[item].filename)
		{
			StringCchCopyW(filename, filenameCch, list->Items[item].filename);
			return 1;
		}
		return 0;
	}

	size_t GetItemTitle(size_t item, wchar_t *title, size_t titleCch)
	{
		if (item < (size_t)list->Size && list->Items[item].filename)
		{
			TAG_FMT_EXT(list->Items[item].filename, itemrecordWTagFunc, ndeTagFuncFree, (void*)&list->Items[item], title, titleCch, 0);
			return 1;
		}
		return 0;
	}

	int GetItemLengthMilliseconds(size_t item)
	{
		if (item < (size_t)list->Size && list->Items[item].length>=0)
		{
			return list->Items[item].length * 1000;
		}
		return -1000;
	}

private:
	const itemRecordListW *list;
protected:
	RECVS_DISPATCH;
};

#define CBCLASS ItemRecordPlaylist
START_DISPATCH;
CB(IFC_PLAYLIST_GETNUMITEMS, GetNumItems)
CB(IFC_PLAYLIST_GETITEM, GetItem)
CB(IFC_PLAYLIST_GETITEMTITLE, GetItemTitle)
CB(IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds)
END_DISPATCH;
#undef CBCLASS


static void Dialog_OnCommand(HWND hwndDlg, UINT idCtrl, INT nCode, HWND hwndCtrl)
{
	if (GetFocus() != hwndSearchGlobal)
	{
		switch (idCtrl)
		{
			case IDC_CLEAR:
				SetDlgItemText(hwndDlg, IDC_QUICKSEARCH, L"");
				break;
			case IDC_BUTTON_INFOTOGGLE:
				updateInfoText(hwndDlg, TRUE);
				UpdateWindow(hwndDlg);
				LayoutWindows(hwndDlg, TRUE);
				break;
			case IDC_QUICKSEARCH:
				if (nCode == EN_CHANGE)
				{
					KillTimer(hwndDlg, UPDATE_QUERY_TIMER_ID);
					SetTimer(hwndDlg, UPDATE_QUERY_TIMER_ID, g_querydelay, NULL);
				}
				break;
			case ID_AUDIOWND_PLAYSELECTION:
			case ID_QUERYWND_PLAYQUERY:
			case IDC_BUTTON_PLAY:
			case ID_MEDIAWND_PLAYSELECTEDFILES:
			case ID_AUDIOWND_ENQUEUESELECTION:
			case IDC_BUTTON_ENQUEUE:
			case ID_MEDIAWND_ENQUEUESELECTEDFILES:
			case IDC_BUTTON_MIX:
			{
				if (nCode == MLBN_DROPDOWN)
				{
					Dialog_Play(hwndDlg, hwndCtrl, idCtrl);
				}
				else
				{
					int action;
					if (idCtrl == IDC_BUTTON_PLAY || idCtrl == ID_MEDIAWND_PLAYSELECTEDFILES || idCtrl == ID_AUDIOWND_PLAYSELECTION)
					{
						action = (nCode == 1) ? g_config->ReadInt(L"enqueuedef", 0) == 1 : 0;
					}
					else if (idCtrl == IDC_BUTTON_ENQUEUE || idCtrl == ID_MEDIAWND_ENQUEUESELECTEDFILES || idCtrl == ID_AUDIOWND_ENQUEUESELECTION)
					{
						action = (nCode == 1) ? g_config->ReadInt(L"enqueuedef", 0) != 1 : 1;
					}
					else
						break;

					int i, l = itemCache.Size;
					for (i = 0; i < l; i++) if (resultlist.GetSelected(i)) break;
					playFiles(action/*idCtrl == IDC_BUTTON_ENQUEUE*/, i == l);
				}
			}
			break;
			case IDC_BUTTON_CREATEPLAYLIST:
#if 0
			// TODO consider exposing this option somehow...
			if (AGAVE_API_PLAYLISTMANAGER)				// This is the old Create Playlist button code
			{		
				wchar_t fn[MAX_PATH] = {0};
				wchar_t dir[MAX_PATH] = {0};
				GetTempPathW(MAX_PATH,dir);
				GetTempFileNameW(dir,L"ml_playlist",0,fn);
				wcscat(fn,L".m3u8");

				ItemRecordPlaylist playlist(&itemCache);
				if (AGAVE_API_PLAYLISTMANAGER->Save(fn, &playlist) == PLAYLISTMANAGER_SUCCESS)
				{
					mlAddPlaylist p={sizeof(p),NULL,fn,PL_FLAGS_IMPORT,-1,-1};
					SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&p,ML_IPC_PLAYLIST_ADD);
					DeleteFileW(fn);
				}
			}
#endif
			if (AGAVE_API_PLAYLIST_GENERATOR)
			{
				const int number_selected = resultlist.GetSelectedCount();		// Total selected

				if (number_selected > 0)
				{
					itemRecordListW recordList;										// Record list
					itemRecordW *records = new itemRecordW[number_selected];		// Array of records

					int selectedRecordcounter = 0;
					for (int i = 0; i < itemCache.Size; i++)
					{
						if (resultlist.GetSelected(i))							// See if the current item is selected or not
						{
							records[selectedRecordcounter] = itemCache.Items[i];	// If its selected then add it to our itemlist
							selectedRecordcounter++;
						}
					}

					recordList.Size = selectedRecordcounter;		// Set the correct size of the record list
					recordList.Items = records;						// Set the array of records to the record list

					AGAVE_API_PLAYLIST_GENERATOR->GeneratePlaylist(hwndDlg, &recordList);	// Call the playlist API with the list of selected records

					delete [] records;								// Free up the array of records
				}
				else
				{
					wchar_t title[64] = {0};
					MessageBoxW(m_hwnd, WASABI_API_LNGSTRINGW(IDS_ERROR_PLG_SELECT_TRACKS), WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_PLAYLIST_GENERATOR,title,64), MB_OK | MB_ICONINFORMATION);
				}
			}
			break;
#if 0
			case IDC_BUTTON_MIX:
			{
				itemRecordList list;
				int i;
				int ct = 0;
				for (i = 0; i < itemCache.Size; i++)
				{
					if (resultlist.GetSelected(i))
					{
						ct++;
					}
				}
				ZeroMemory(&list, sizeof(itemRecordList));
				allocRecordList(&list, ct, 1);
				list.Size = ct;
				ct = 0;
				for (i = 0; i < itemCache.Size; i++)
				{
					if (resultlist.GetSelected(i))
					{
						convertRecord(&list.Items[ct], &itemCache.Items[i]);
						ct++;
					}
				}
				pluginMessage p = {ML_MSG_PDXS_MIX, (INT_PTR) &list, 0, 0};
				SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&p, ML_IPC_SEND_PLUGIN_MESSAGE);
				emptyRecordList(&list);
				freeRecordList(&list);
			}
			break;
#endif
			case ID_MEDIAWND_SELECTALL:
				{
					LVITEM item = {0};
					item.state = LVIS_SELECTED;
					item.stateMask = LVIS_SELECTED;
					SendMessageW(resultlist.getwnd(), LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&item);
				}
				break;
			case ID_MEDIAWND_REMOVEFROMLIBRARY:
				removeSelectedItems(0);
				break;
			case ID_EDITITEMINFOS:
				if (resultlist.GetSelectedCount() > 0)
					editInfo(hwndDlg);
				break;
			case ID_PE_ID3:
				fileInfoDialogs(hwndDlg);
				PostMessageW(hwndDlg, WM_NEXTDLGCTL, (WPARAM)resultlist.getwnd(), (LPARAM)TRUE);
				break;
			case IDC_REFRESH_METADATA:
				RefreshMetadata(hwndDlg);
				break;
			case ID_MEDIAWND_EXPLOREFOLDER:
				exploreItemFolder(hwndDlg);
				break;
		}
	}
	else
	{
		switch (idCtrl)
		{
			case IDC_QUICKSEARCH:
				if (nCode == EN_CHANGE)
				{
					KillTimer(hwndDlg, UPDATE_QUERY_TIMER_ID);
					SetTimer(hwndDlg, UPDATE_QUERY_TIMER_ID, g_querydelay, NULL);
				}
				break;
			case ID_MEDIAWND_SELECTALL:
				SendMessageW(hwndSearchGlobal, EM_SETSEL, 0, -1);
				break;
			case ID_MEDIAWND_REMOVEFROMLIBRARY:
				{
					DWORD start = -1, end = -1;
					SendMessageW(hwndSearchGlobal, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
					if (start != -1)
					{
						if (start == end)
						{
							SendMessageW(hwndSearchGlobal, EM_SETSEL, start, end + 1);
						}
						SendMessageW(hwndSearchGlobal, EM_REPLACESEL, TRUE, (LPARAM)"");
						SendMessageW(hwndSearchGlobal, EM_SETSEL, start, start);
					}
				}
				break;
		}
	}
}

static void Dialog_OnTimer(HWND hwndDlg, UINT_PTR idEvent, TIMERPROC fnTimer)
{
	switch (idEvent)
	{
		case 5050:
			KillTimer(hwndDlg, 5050);
			WASABI_API_DIALOGBOXW(IDD_NEEDADDFILES, hwndDlg, needAddFilesProc);
			PostMessage(GetParent(hwndDlg), WM_APP + 1, (WPARAM)0, (LPARAM)0);
			break;
		case 6600:
			KillTimer(hwndDlg, idEvent);
			if (m_last_selitem >= 0 && m_last_selitem < itemCache.Size)
			{
				if (predixisExist & 1)
				{
					// Only single seeds are supported currently
					if (resultlist.GetSelectedCount() == 1 && isMixable(itemCache.Items[m_last_selitem]))
					{
						SetDlgItemTextW(hwndDlg, IDC_MIXABLE, WASABI_API_LNGSTRINGW(IDS_MIXABLE));
						isMixablePresent = true;
					}
					else SetDlgItemText(hwndDlg, IDC_MIXABLE, L"");
				}
				SendMessageW(GetParent(hwndDlg), WM_SHOWFILEINFO, (WPARAM)FALSE, (LPARAM)itemCache.Items[m_last_selitem].filename);
				m_last_selitem = -1;
			}
			break;
		case 123:
			if (bgThread_Handle)
			{
				HWND hwndList;
				hwndList = resultlist.getwnd();
				if (1 != ListView_GetItemCount(hwndList)) ListView_SetItemCountEx(hwndList, 1, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
				ListView_RedrawItems(hwndList, 0, 0);
				//UpdateWindow(hwndList);
			}
			break;
		case UPDATE_QUERY_TIMER_ID:
		{
			KillTimer(hwndDlg, UPDATE_QUERY_TIMER_ID);
			wchar_t buf[2048] = {0};
			GetWindowTextW(GetDlgItem(hwndDlg, IDC_QUICKSEARCH), buf, ARRAYSIZE(buf));
			doQuery(hwndDlg, buf);
		}
		break;
		case UPDATE_RESULT_LIST_TIMER_ID:
			{
				ListView_RedrawItems(resultlist.getwnd(), 0, resultlist.GetCount() - 1);
			}
			break;
	}
}

static void Dialog_OnDestroy(HWND hwndDlg)
{
	HWND hwndList;
	INT i, j;
	wchar_t buf[2048] = {0};

	bgQuery_Stop();

	GetDlgItemTextW(hwndDlg, IDC_QUICKSEARCH, buf, ARRAYSIZE(buf));
	g_view_metaconf->WriteString("lastquery_utf8", AutoChar(buf, CP_UTF8));

	hwndList = GetDlgItem(hwndDlg, IDC_LIST2);
	if (hwndList && IsWindow(hwndList))
	{
		for (i = 0; columnOrder[i] != -1; i++)
		{
			headerColumn *cl = &columnList[columnOrder[i]];
			g_view_metaconf->WriteInt(AutoWide(cl->config_name), SendMessageW(hwndList, LVM_GETCOLUMNWIDTH, i, 0L));
		}
	}

	//Save the column order
	for (i = 0; columnOrder[i] != -1; i++);
	g_view_metaconf->WriteInt(L"nbcolumns", i);
	for (j = 0; j < i; j++)
	{
		wchar_t tmp[128] = {0};
		StringCchPrintfW(tmp, 128, L"column%d", j);
		g_view_metaconf->WriteInt(tmp, columnOrder[j]);
	}

	freeRecordList(&itemCache);
	itemCache.Items = 0;
	itemCache.Alloc = 0;
	itemCache.Size = 0;

	cloudFiles.clear();
	cloudUploading.clear();

	hwndSearchGlobal = 0;		// Set the hwnd for the search to a global to null so we know we are not in the single pane view
}

static void Dialog_OnWindowPosChanged(HWND hwndDlg, WINDOWPOS *pwp)
{
	if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & pwp->flags) || (SWP_FRAMECHANGED & pwp->flags))
	{
		LayoutWindows(hwndDlg, !(SWP_NOREDRAW & pwp->flags), 0 != (SWP_SHOWWINDOW & pwp->flags));
	}
}

static void Dialog_OnSyncHeaderOrder(HWND hwndDlg, HWND hwndHeader)
{
	LVCOLUMNW column = {0};
	wchar_t buffer[128] = {0};
	signed char tempOrder[MAX_COLUMN_ORDER] = {0};
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST2);

	if (!hwndList) return;

	CopyMemory(tempOrder, columnOrder, sizeof(tempOrder)/sizeof(signed char));

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_IMAGE;
	column.cchTextMax = sizeof(buffer) /sizeof(wchar_t);

	SendMessageW(hwndList, WM_SETREDRAW, FALSE, 0L);

	INT sort = MLSkinnedListView_GetSort(hwndList);
	INT count = (INT)SendMessageW(hwndHeader, HDM_GETITEMCOUNT, 0, 0L);
	if (count > 0)
	{
		INT index = count + 1, *pOrder = (INT*)calloc(1, sizeof(INT)*count);
		if (pOrder && SendMessageW(hwndList, LVM_GETCOLUMNORDERARRAY, count, (LPARAM)pOrder))
		{
			INT order;
			for (order = 0; order < count; order++)
			{
				column.pszText = buffer;
				if (!SendMessageW(hwndList, LVM_GETCOLUMNW, pOrder[order], (LPARAM)&column)) continue;
				column.iOrder = order;

				// update position of the cloud column icon
				if (tempOrder[pOrder[order]] == MEDIAVIEW_COL_CLOUD)
				{
					if (!cloud_hinst || cloud_hinst == (HINSTANCE)1 ||
						!SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_ACTIVE))
					{
						MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(hwndList), -1);
						SetPropW(hwndList, L"pmp_list_info", (HANDLE)-1);
						column.cx = 0;
					}
					else
					{
						MLSkinnedHeader_SetCloudColumn(ListView_GetHeader(hwndList), order);
						SetPropW(hwndList, L"pmp_list_info", (HANDLE)order);
						column.cx = 27;
						MLCloudColumn_GetWidth(plugin.hwndLibraryParent, &column.cx);
					}
				}

				SendMessageW(hwndList, LVM_INSERTCOLUMNW, index++, (LPARAM)&column);
				columnOrder[order] = tempOrder[pOrder[order]];

				if (LOWORD(sort) == pOrder[order]) MLSkinnedListView_DisplaySort(hwndList, order, HIWORD(sort));
			}
			for (order = 0; order < count; order++) SendMessageW(hwndList, LVM_DELETECOLUMN, 0, 0L);
		}

		for (index = 0; -1 != columnOrder[index] && MEDIAVIEW_COL_RATING != columnOrder[index] && MEDIAVIEW_COL_CLOUD != columnOrder[index]; index++);
		if (-1 != columnOrder[index])
		{
			INT w = (INT)SendMessageW(hwndList, LVM_GETCOLUMNWIDTH, index, 0L);
			SendMessageW(hwndList, LVM_SETCOLUMNWIDTH, index, (LPARAM)w);
		}

		if (pOrder) free(pOrder);
	}

	SendMessageW(hwndList, WM_SETREDRAW, TRUE, 0L);
}

static void Window_OnQueryFileInfo(HWND hwnd)
{
	INT index;
	HWND hwndList = GetDlgItem(hwnd, IDC_LIST2);
	index = (hwndList) ? (INT)SendMessage(hwndList, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_FOCUSED) : -1;
	SendMessageW(GetParent(hwnd), WM_SHOWFILEINFO, (WPARAM)FALSE, (LPARAM)((-1 != index && index  < itemCache.Size) ? itemCache.Items[index].filename : L""));
}

void Window_OnDropFiles(HWND hwndDlg, HDROP hdrop)
{
	wchar_t temp[1024] = {0};
	int y = DragQueryFileW(hdrop, 0xffffffff, temp, 1024);
	if (y > 0)
	{
		wchar_t **paths = (wchar_t **)calloc(y, sizeof(wchar_t *));
		int *guesses = (int *)calloc(y, sizeof(int));
		int *metas = (int *)calloc(y, sizeof(int));
		int *recs= (int *)calloc(y, sizeof(int));
		if (paths && guesses && metas && recs)
		{
			size_t count=0;
			for (int x = 0; x < y; x ++)
			{
				DragQueryFileW(hdrop, x, temp, 1024);
				int guess = -1, meta = -1, rec = 1;
				// do this for normal media drops
				PLCallBackW plCB;
				if (AGAVE_API_PLAYLISTMANAGER && PLAYLISTMANAGER_SUCCESS != AGAVE_API_PLAYLISTMANAGER->Load(temp, &plCB))
				{
					autoscan_add_directory(temp, &guess, &meta, &rec, 0);
					if (guess == -1) guess = g_config->ReadInt(L"guessmode", 0);
					if (meta == -1)	meta = g_config->ReadInt(L"usemetadata", 1);
					paths[count] = _wcsdup(temp);
					guesses[count]=guess;
					metas[count]=meta;
					recs[count]=rec;
					count++;
				}
			}
			DragFinish(hdrop);
			Scan_ScanFolders(hwndDlg, count, paths, guesses, metas, recs);
			if (IsWindow(m_curview_hwnd)) SendMessage(m_curview_hwnd, WM_APP + 1, 0, 0); //update current view
		}
		else
		{
			free(paths);
			free(guesses);
			free(metas);
			free(recs);
		}
	}
	else DragFinish(hdrop);
}


INT_PTR CALLBACK view_mediaDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam); if (a) return a;

	switch (uMsg)
	{
		case WM_INITMENUPOPUP:		Dialog_OnInitMenuPopup(hwndDlg, (HMENU)wParam, LOWORD(lParam), HIWORD(lParam)); break;
		case WM_DISPLAYCHANGE:		Dialog_OnDisplayChange(hwndDlg); break;
		case WM_INITDIALOG:			return Dialog_OnInit(hwndDlg, (HWND)wParam, lParam);
		case WM_MOUSEMOVE:			Dialog_OnMouseMove(hwndDlg, (UINT)wParam, MAKEPOINTS(lParam)); break;
		case WM_LBUTTONUP:			Dialog_OnLButtonUp(hwndDlg, (UINT)wParam, MAKEPOINTS(lParam)); break;
		case WM_DROPFILES:			Window_OnDropFiles(hwndDlg, (HDROP)wParam);
		case WM_COMMAND:			Dialog_OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_TIMER:				Dialog_OnTimer(hwndDlg, (UINT_PTR) wParam, (TIMERPROC)lParam); break;
		case WM_DESTROY:			Dialog_OnDestroy(hwndDlg); break;
		case WM_WINDOWPOSCHANGED:	Dialog_OnWindowPosChanged(hwndDlg, (WINDOWPOS*)lParam); break;
		case WM_ERASEBKGND:			return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
		case WM_CONTEXTMENU:
			Dialog_OnContextMenu(hwndDlg, (HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));		
			return 0;
		case WM_NOTIFY:
		{
			LRESULT result;
			result = 0L;
			if (Dialog_OnNotify(hwndDlg, (INT)wParam, (NMHDR*)lParam, &result))
			{
				SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)result);
				return TRUE;
			}
		}
		break;
		case IM_SYNCHEADERORDER:		Dialog_OnSyncHeaderOrder(hwndDlg, (HWND)lParam); break;
		case WM_APP + 3:      // send by bgthread
			if (wParam == 0x666) m_bgupdinfoviewerflag = 1;
			else if (wParam == 0x69)
			{
				bgQuery_Stop();

				resultlist.SetVirtualCount(0);
				resultlist.SetVirtualCount(itemCache.Size);  // TODO: we could set a limit here
				ListView_RedrawItems(resultlist.getwnd(), 0, itemCache.Size - 1);
				UpdateWindow(resultlist.getwnd());

				__int64 total_len_bytes = bg_total_len_bytes;

				int total_length_s = (int)bg_total_len_s & 0x7FFFFFFF;
				wchar_t buffer[4*64] = {0};
				wchar_t *pb[4] = {0};
				for (int i = 0; i < 4; i++) pb[i] = buffer + i * 64;


				int index(0);

				StringCchPrintfW(pb[index], 64, L"%d %s", itemCache.Size,
								 WASABI_API_LNGSTRINGW(itemCache.Size == 1 ? IDS_ITEM : IDS_ITEMS));
				index++;

				if (itemCache.Size)
				{
					int uncert = 0; //bg_total_len_s>>31;
					if (total_length_s < 60*60) StringCchPrintfW(pb[index], 64, L"[%s%u:%02u]", uncert ? L"~" : L"", total_length_s / 60, total_length_s % 60);
					else if (total_length_s < 60*60*24) StringCchPrintfW(pb[index], 64, L"[%s%u:%02u:%02u]", uncert ? L"~" : L"", total_length_s / 60 / 60, (total_length_s / 60) % 60, total_length_s % 60);
					else
					{
						wchar_t days[16] = {0};
						int total_days = total_length_s / (60 * 60 * 24);
						total_length_s -= total_days * 60 * 60 * 24;
						StringCchPrintfW(pb[index], 64,
										 WASABI_API_LNGSTRINGW(IDS_LENGTH_DURATION_STRING),
										 uncert ? L"~" : L"", total_days,
										 WASABI_API_LNGSTRINGW_BUF(total_days == 1 ? IDS_DAY : IDS_DAYS, days, 16),
										 total_length_s / 60 / 60, (total_length_s / 60) % 60, total_length_s % 60);
					}
					index++;
				}

				if (total_len_bytes)
				{
					StringCchCopyW(pb[index], 64, L"[");
					WASABI_API_LNG->FormattedSizeString(pb[index] + 1, 64, total_len_bytes);
					StringCchCatW(pb[index], 64, L"]");
					index++;
				}

				unsigned int ms = (UINT)(querytime.QuadPart * 1000 / freq.QuadPart);
				StringCchPrintfW(pb[index], 64, WASABI_API_LNGSTRINGW(IDS_IN_X_SEC), ms / 1000.0f);
				index++;

				SetStatusText(GetDlgItem(hwndDlg, IDC_MEDIASTATUS), (LPCWSTR*)pb, index);

				if (m_bgupdinfoviewerflag)
				{
					m_bgupdinfoviewerflag = 0;
					if (itemCache.Size > 0)
					{
						if (predixisExist)
						{
							if (resultlist.GetSelectedCount() == 1 && isMixable(itemCache.Items[0]))
							{
								SetDlgItemText(hwndDlg, IDC_MIXABLE, L"");
								isMixablePresent = true;
							}
							else
							{
								SetDlgItemText(hwndDlg, IDC_MIXABLE, L"");
							}
						}
						SendMessageW(GetParent(hwndDlg), WM_SHOWFILEINFO, (WPARAM)FALSE, (LPARAM)itemCache.Items[0].filename);
					}
				}
			}
			break;
		case WM_APP + 1:
			bgQuery((resultsniff_funcW)wParam, (int)lParam);
			break;
		case WM_APP + 5:
		{
			// TODO
			int done = (HIWORD(wParam) == 1);
			int code = (LOWORD(wParam));
			for (int i = 0; i < itemCache.Size; i ++)
			{
				// 0 = no change
				// 1 = add cloud
				// 2 = add local
				// 4 = removed
				if (!lstrcmpiW(itemCache.Items[i].filename, (wchar_t *)lParam))
				{
					if (!done)
					{
						setCloudValue(&itemCache.Items[i], L"5");
					}
					else
					{
						if (code == NErr_Success)
						{
							// uploaded ok
							// TODO if going to another device, will need to alter this
							setCloudValue(&itemCache.Items[i], L"0");
						}
						else
						{
							// re-query state
							setCloudValue(&itemCache.Items[i], L"0");
						}
					}
					InvalidateRect(resultlist.getwnd(), NULL, TRUE);
					break;
				}
			}
			break;
		}
		case WM_APP + 6:	// handles the ml_cloud 'first pull' announces so we
		{					// can then show the cloud column & update the cache
			initColumnsHeader(resultlist.getwnd());
			bgQuery();//(resultsniff_funcW)wParam, (int)lParam);
			break;
		}
		case WM_APP + 104:
		{
			Dialog_UpdateButtonText(hwndDlg, wParam);
			LayoutWindows(hwndDlg, TRUE);
			return 0;
		}
		case WM_PAINT:
		{
			int tab[] = { IDC_LIST2 | DCW_SUNKENBORDER, IDC_QUICKSEARCH | DCW_SUNKENBORDER};
			dialogSkinner.Draw(hwndDlg, tab, 1 + !!IsWindowVisible(GetDlgItem(hwndDlg, IDC_QUICKSEARCH)));
		}
		return 0;

		case WM_ML_CHILDIPC:
			switch (lParam)
			{
				case ML_CHILDIPC_GO_TO_SEARCHBAR:
					SendDlgItemMessage(hwndDlg, IDC_QUICKSEARCH, EM_SETSEL, 0, -1);
					SetFocus(GetDlgItem(hwndDlg, IDC_QUICKSEARCH));
					break;
				case ML_CHILDIPC_REFRESH_SEARCH:
					PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_QUICKSEARCH, EN_CHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_QUICKSEARCH));
					break;
			}
			break;

		case WM_USER + 0x201:
			offsetX = (short)LOWORD(wParam);
			offsetY = (short)HIWORD(wParam);
			g_rgnUpdate = (HRGN)lParam;
			return TRUE;
		case WM_QUERYFILEINFO: Window_OnQueryFileInfo(hwndDlg); break;
	}
	return FALSE;
}

//////////////////////////////// Customize columns dialog

static signed char edit_columnOrder[MAX_COLUMN_ORDER];

static INT_PTR CALLBACK custColumns_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND m_curlistbox_hwnd, m_availlistbox_hwnd;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		memcpy(edit_columnOrder, columnOrder, sizeof(edit_columnOrder));
		m_curlistbox_hwnd = GetDlgItem(hwndDlg, IDC_LIST1);
		m_availlistbox_hwnd = GetDlgItem(hwndDlg, IDC_LIST2);
	
		if (NULL != WASABI_API_APP)
		{
			if (NULL != m_curlistbox_hwnd)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_curlistbox_hwnd, TRUE);
			if (NULL != m_availlistbox_hwnd)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(m_availlistbox_hwnd, TRUE);
		}

	case WM_USER + 32:
	{
		for (int i = 0; edit_columnOrder[i] != -1; i++)
		{
			int c = edit_columnOrder[i];
			headerColumn *cl = &columnList[c];
			int column_id = cl->column_id;
			if (column_id == IDS_CLOUD || column_id == IDS_CLOUD_HIDDEN)
			{
				// if no cloud support at all then we hide everything
				if (!cloud_hinst || cloud_hinst == (HINSTANCE)1) continue;
				column_id = ((!cloud_hinst || cloud_hinst == (HINSTANCE)1 ||
							 !SendMessageW(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_ACTIVE)) ? IDS_CLOUD_HIDDEN : IDS_CLOUD);
			}
			int r = SendMessageW(m_curlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(column_id));
			SendMessageW(m_curlistbox_hwnd, LB_SETITEMDATA, r, c);
		}

		for (int i = 0; i < sizeof(columnList) / sizeof(headerColumn); i++)
		{
			headerColumn *cl = &columnList[i];
			int j;
			for (j = 0; edit_columnOrder[j] != -1 && edit_columnOrder[j] != i; j++);
			if (edit_columnOrder[j] == -1)
			{
				int column_id = cl->column_id;
				if (column_id == IDS_CLOUD || column_id == IDS_CLOUD_HIDDEN)
				{
					// if no cloud support at all then we hide everything
					if (!cloud_hinst || cloud_hinst == (HINSTANCE)1) continue;
					column_id = ((!cloud_hinst || cloud_hinst == (HINSTANCE)1 ||
								 !SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_ACTIVE)) ? IDS_CLOUD_HIDDEN : IDS_CLOUD);
				}
				int r = SendMessageW(m_availlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(column_id));
				SendMessageW(m_availlistbox_hwnd, LB_SETITEMDATA, r, i);
			}
		}
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DEFS:
			if (!cloud_hinst || cloud_hinst == (HINSTANCE)1)
				memcpy(edit_columnOrder, defColumnOrder, sizeof(defColumnOrder));
			else
				memcpy(edit_columnOrder, defColumnOrderCloud, sizeof(defColumnOrderCloud));
			SendMessage(m_curlistbox_hwnd, LB_RESETCONTENT, 0, 0);
			SendMessage(m_availlistbox_hwnd, LB_RESETCONTENT, 0, 0);
			SendMessage(hwndDlg, WM_USER + 32, 0, 0);
			break;
		case IDC_LIST2:
			if (HIWORD(wParam) != LBN_DBLCLK)
			{
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					int r = SendMessage(m_availlistbox_hwnd, LB_GETSELCOUNT, 0, 0) > 0;
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), r);
				}
				return 0;
			}
		case IDC_BUTTON2:
			//add column
		{
			for (int i = 0;i < SendMessage(m_availlistbox_hwnd, LB_GETCOUNT, 0, 0);i++)
			{
				if (SendMessage(m_availlistbox_hwnd, LB_GETSEL, i, 0))
				{
					int c = SendMessage(m_availlistbox_hwnd, LB_GETITEMDATA, i, 0);
					int j;
					for (j = 0;edit_columnOrder[j] != -1;j++);
					edit_columnOrder[j] = (BYTE)c;
					edit_columnOrder[j + 1] = -1;
					SendMessage(m_availlistbox_hwnd, LB_DELETESTRING, i, 0);
					i--;

					int r = SendMessageW(m_curlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(columnList[c].column_id));
					SendMessageW(m_curlistbox_hwnd, LB_SETITEMDATA, r, c);
				}
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), 0);
		}
		break;
		case IDC_LIST1:
			if (HIWORD(wParam) != LBN_DBLCLK)
			{
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
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
			for (int i = 0;i < SendMessage(m_curlistbox_hwnd, LB_GETCOUNT, 0, 0);i++)
			{
				if (SendMessage(m_curlistbox_hwnd, LB_GETSEL, i, 0))
				{
					int c = edit_columnOrder[i];
					for (int j = i;edit_columnOrder[j] != -1;j++)
					{
						edit_columnOrder[j] = edit_columnOrder[j + 1];
					}
					SendMessage(m_curlistbox_hwnd, LB_DELETESTRING, i, 0);
					i--;

					int r = SendMessageW(m_availlistbox_hwnd, LB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(columnList[c].column_id));
					SendMessageW(m_availlistbox_hwnd, LB_SETITEMDATA, r, c);
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
					BYTE c = edit_columnOrder[i - 1];
					edit_columnOrder[i - 1] = edit_columnOrder[i];
					edit_columnOrder[i] = c;

					SendMessage(m_curlistbox_hwnd, LB_DELETESTRING, i - 1, 0);
					int r = (INT)SendMessageW(m_curlistbox_hwnd, LB_INSERTSTRING, i, (LPARAM)WASABI_API_LNGSTRINGW(columnList[c].column_id));
					SendMessage(m_curlistbox_hwnd, LB_SETITEMDATA, r, c);
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
					BYTE c = edit_columnOrder[i + 1];
					edit_columnOrder[i + 1] = edit_columnOrder[i];
					edit_columnOrder[i] = c;

					SendMessage(m_curlistbox_hwnd, LB_DELETESTRING, i + 1, 0);
					int r = (INT)SendMessageW(m_curlistbox_hwnd, LB_INSERTSTRING, i, (LPARAM)WASABI_API_LNGSTRINGW(columnList[c].column_id));
					SendMessage(m_curlistbox_hwnd, LB_SETITEMDATA, r, c);
				}
			}
		}
		break;
		case IDOK:
		{
			HWND hwndList = resultlist.getwnd();
			memcpy(columnOrder, edit_columnOrder, sizeof(edit_columnOrder));
			if (hwndList)
			{
				initColumnsHeader(hwndList);
				InvalidateRect(hwndList, NULL, TRUE);
				UpdateWindow(hwndList);
			}
		}
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

			WASABI_API_APP->app_removeAccelerators(hwndDlg);
		}
		break;
	}
	return FALSE;
}

void customizeColumnsDialog(HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_CUSTCOLUMNS, hwndParent, custColumns_dialogProc);
	EatKeyboard();
}

bool isMixable(itemRecordW &song)
{
	if (!song.filename) return false;

	AutoChar charFn(song.filename);
	pluginMessage p = {ML_MSG_PDXS_STATUS, (INT_PTR)(char *)charFn, 0, 0};
	char *text = (char *)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM) & p, ML_IPC_SEND_PLUGIN_MESSAGE);
	// Analyzed/Identified = mixable
	return text && (text[0] == 'A' || text[0] == 'I');
}

void AccessingGracenoteHack(int p)
{
	if (p == 0)
	{
		GetDlgItemTextW(m_hwnd, IDC_MEDIASTATUS, oldText, 4096);
		SetDlgItemTextW(m_hwnd, IDC_MEDIASTATUS, L"Accessing Gracenote Database");
	}
	else
	{
		SetDlgItemTextW(m_hwnd, IDC_MEDIASTATUS, oldText);
	}
}