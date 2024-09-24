#include "main.h"
#include "..\..\General\gen_ml/config.h"
#include "resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/listview.h"
#include <time.h>
#include "..\..\General\gen_ml/gaystring.h"
#include "..\..\General\gen_ml/ml_ipc.h"
#include <malloc.h>
#include <string>
#include "../nu/AutoWide.h"
#include "../nu/AutoCharFn.h"
#include "..\..\General\gen_ml/ml.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "../nu/sort.h"
#include "../nu/menushortcuts.h"
#include "../Winamp/strutil.h"
#include "api__ml_history.h"
#include <strsafe.h>

static INT_PTR IPC_LIBRARY_SENDTOMENU;
static void history_cleanupifnecessary();
static GayStringW g_q;
static W_ListView resultlist;
static int resultSkin, customAllowed;
static HWND m_hwnd, m_headerhwnd;
static historyRecordList itemCache;
static int history_bgThread_Kill;
static HANDLE history_bgThread_Handle;
static void fileInfoDialogs(HWND hwndParent);
static int m_lv_last_topidx;
int groupBtn = 1, enqueuedef = 0;
HMENU g_context_menus2 = NULL;
static viewButtons view;

static void MakeDateStringW(__time64_t convertTime, wchar_t *dest, size_t destlen)
{
	SYSTEMTIME sysTime = {0};
	tm *newtime = _localtime64(&convertTime);

	sysTime.wYear = newtime->tm_year + 1900;
	sysTime.wMonth = newtime->tm_mon + 1;
	sysTime.wDayOfWeek = newtime->tm_wday;
	sysTime.wDay = newtime->tm_mday;
	sysTime.wHour = newtime->tm_hour;
	sysTime.wMinute = newtime->tm_min;
	sysTime.wSecond = newtime->tm_sec;

	GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sysTime, NULL, dest, (int)destlen);

	size_t dateSize = lstrlenW(dest);
	dest += dateSize;
	destlen -= dateSize;
	if (destlen)
	{
		*dest++ = ' ';
		destlen--;
	}

	GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &sysTime, NULL, dest, (int)destlen);
}

void makeFilename2(const wchar_t *filename, wchar_t *filename2, int filename2_len)
{
	filename2[0]=0;
	if (wcsstr(filename,L"~"))
	{
		WIN32_FIND_DATAW d = {0};
		HANDLE h = FindFirstFileW(filename,&d);
		if (h != INVALID_HANDLE_VALUE)
		{
			FindClose(h);
			lstrcpynW(filename2,filename,filename2_len);
			wchar_t *p=scanstr_backW(filename2,L"\\",filename2-1)+1;
			int offs=(int)(p-filename2);
	  		lstrcpynW(filename2+offs,d.cFileName,filename2_len - offs);
			if (!_wcsicmp(filename,filename2)) filename2[0]=0;
		}
	}
}

void db_setFieldString(nde_scanner_t s, unsigned char id, const wchar_t *data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_StringField_SetString(f, data);
}

void db_setFieldInt(nde_scanner_t s, unsigned char id, int data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_IntegerField_SetValue(f, data);
}

void queryStrEscape(const wchar_t *p, GayStringW &str) 
{
	if (!p || !*p) return;
	size_t l = wcslen(p);
	wchar_t *escaped = (wchar_t *)calloc((l*3+1), sizeof(wchar_t));
	if (escaped)
	{
		wchar_t *d = escaped;
		while (p && *p)
		{
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
}

int STRCMP_NULLOK(const wchar_t *pa, const wchar_t *pb)
{
	if (!pa) pa = L"";
	else SKIP_THE_AND_WHITESPACE(pa)

	if (!pb) pb = L"";
	else SKIP_THE_AND_WHITESPACE(pb)

	return _wcsicmp(pa, pb);
}

struct SortRules
{
	int by;
	int dir;
};
static int __fastcall sortFunc(const void *elem1, const void *elem2, const void *context)
{
	historyRecord *a=(historyRecord*)elem1;
	historyRecord *b=(historyRecord*)elem2;

	const SortRules *rules = (SortRules *)context;
	int use_by = rules->by;
	int use_dir = !!rules->dir;

	#define RETIFNZ(v) if ((v)<0) return use_dir?1:-1; if ((v)>0) return use_dir?-1:1;

	// this might be too slow, but it'd be nice
	int x;
	for (x = 0; x < 4; x ++)
	{
		if (use_by == HISTORY_SORT_FILENAME)
		{
			int v=STRCMP_NULLOK(a->filename,b->filename);
			RETIFNZ(v)
			return 0;
		}
		else if (use_by == HISTORY_SORT_TITLE)
		{
			int v=STRCMP_NULLOK(a->title,b->title);
			RETIFNZ(v)
			return 0;
		}
		else if (use_by == HISTORY_SORT_LASTPLAYED)
		{
			__time64_t v1=a->lastplayed;
			__time64_t v2=b->lastplayed;
			RETIFNZ(v2-v1)
			return 0;
		}
		else if (use_by == HISTORY_SORT_PLAYCOUNT)
		{
			int v1=a->playcnt;
			int v2=b->playcnt;
			RETIFNZ(v2-v1)
			use_by=HISTORYVIEW_COL_LASTPLAYED;
		}
		else if (use_by == HISTORY_SORT_LENGTH) // length -> artist -> album -> track
		{
			int v1=a->length;
			int v2=b->length;
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v2-v1)
			return 0;
		}
		else if (use_by == HISTORY_SORT_OFFSET) 
		{
			int v1=a->offset;
			int v2=b->offset;
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v2-v1)
			return 0;
		}
		else break; // no sort order?
	}
	#undef RETIFNZ
	return 0;
}

void sortResults(historyRecordList *obj, int column, int dir) // sorts the results based on the current sort mode
{
	if (obj->Size > 1) 
	{
		SortRules rules = {column, dir};
		nu::qsort(obj->Items,obj->Size,sizeof(historyRecord),&rules, sortFunc);
	}
}

// does not copy filename
void recentScannerRefToObjCacheNFN(nde_scanner_t s, historyRecordList *obj)
{
	nde_field_t f=NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_TITLE);
	if (f)
	{
		wchar_t *strval = NDE_StringField_GetString(f);
		ndestring_retain(strval);
		obj->Items[obj->Size].title = strval;
	}
	else
		obj->Items[obj->Size].title = 0;

	f=NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_LENGTH);
	obj->Items[obj->Size].length = f?NDE_IntegerField_GetValue(f):-1;
	f=NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_OFFSET);
	obj->Items[obj->Size].offset = f?NDE_IntegerField_GetValue(f):-1;
	f=NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_PLAYCOUNT);
	obj->Items[obj->Size].playcnt = f?NDE_IntegerField_GetValue(f):0;
	f=NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_LASTPLAYED);
	obj->Items[obj->Size].lastplayed = f?NDE_IntegerField_GetValue(f):0;
	obj->Size++;
}

static void freeRecentRecord(historyRecord *p)
{
	ndestring_release(p->title);
	ndestring_release(p->filename);
}

void emptyRecentRecordList(historyRecordList *obj)
{
	historyRecord *p=obj->Items;
	while (obj->Size-->0)
	{
		freeRecentRecord(p);
		p++;
	}
	obj->Size=0;
}

void freeRecentRecordList(historyRecordList *obj)
{
	emptyRecentRecordList(obj);
	free(obj->Items);
	obj->Items=0;
	obj->Alloc=obj->Size=0;
}

void allocRecentRecordList(historyRecordList *obj, int newsize, int granularity)
{
	if (newsize < obj->Alloc || newsize < obj->Size) return;

	size_t old_Alloc = obj->Alloc;
	obj->Alloc=newsize+granularity;
	historyRecord *data = (historyRecord*)realloc(obj->Items,sizeof(historyRecord)*obj->Alloc);
	if (data)
	{
		obj->Items=data;
	}
	else
	{
		data=(historyRecord*)malloc(sizeof(historyRecord)*obj->Alloc);
		if (data)
		{
			memcpy(data, obj->Items, sizeof(historyRecord)*old_Alloc);
			free(obj->Items);
			obj->Items=data;
		}
		else obj->Alloc = (int)old_Alloc;
	}
	if (!obj->Items) obj->Alloc=0;
}

static void playFiles(int enqueue, int all)
{
	if (history_bgThread_Handle) return;

	int cnt=0;
	int l=itemCache.Size;

	for(int i=0;i<l;i++) 
	{
		if ( all || resultlist.GetSelected( i ) )
		{
			if ( !cnt )
			{
				if ( !enqueue ) SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE );
				cnt++;
			}
			enqueueFileWithMetaStructW s = { 0 };
			s.filename = itemCache.Items[ i ].filename;
			s.title    = itemCache.Items[ i ].title;
			s.ext      = NULL;
			s.length   = itemCache.Items[ i ].length;
			SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW );
		}
	}
	if (cnt)
	{
		if(!enqueue) SendMessage(plugin.hwndWinampParent, WM_WA_IPC,0,IPC_STARTPLAY);
	}
}

static int history_saveQueryToList(nde_scanner_t s, historyRecordList *obj, int user32, int *killswitch) {

	emptyRecentRecordList(obj);

	NDE_Scanner_First(s, killswitch);
	if (killswitch && *killswitch) 
	{
	    return 0;
	}

	int r;
	unsigned int total_length_s=0;
	do 
	{
		nde_field_t f = NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_FILENAME);
		if (!f) break;

		allocRecentRecordList(obj,obj->Size+1);
		if (!obj->Alloc) break;

		wchar_t *strval = NDE_StringField_GetString(f);
		ndestring_retain(strval);
		obj->Items[obj->Size].filename = strval;
		recentScannerRefToObjCacheNFN(s,obj);

		int thisl=obj->Items[obj->Size-1].length;

		if (thisl > 0) total_length_s+=thisl * obj->Items[obj->Size-1].playcnt;
		else total_length_s|=(1<<31);

		r=NDE_Scanner_Next(s, killswitch);
		if (killswitch && *killswitch) 
		{
			return 0;
		}
	} while(r);

	if (obj->Size && obj->Size < obj->Alloc - 1024)
	{
		size_t old_Alloc = obj->Alloc;
	    obj->Alloc = obj->Size;
		historyRecord *data=(historyRecord*)realloc(obj->Items,sizeof(historyRecord)*obj->Alloc);
		if (data)
		{
			obj->Items=data;
		}
		else
		{
			data=(historyRecord*)malloc(sizeof(historyRecord)*obj->Alloc);
			if (data)
			{
				memcpy(data, obj->Items, sizeof(historyRecord)*old_Alloc);
				free(obj->Items);
				obj->Items=data;
			}
			else obj->Alloc = (int)old_Alloc;
		}
	}

	if (killswitch && *killswitch) return 0;

	sortResults(obj, 
				 g_config->ReadInt(L"recent_sort_by",HISTORY_SORT_LASTPLAYED),
				 g_config->ReadInt(L"recent_sort_dir",0));

	if (killswitch && *killswitch) return 0;

	return total_length_s;
}

typedef struct
{
	int user32;
} history_bgThreadParms;

// out can never be bigger than in+1
static void parsequicksearch(wchar_t *out, const wchar_t *in) // parses a list into a list of terms that we are searching for
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

void makeQueryStringFromText(GayStringW *query, const wchar_t *text, int nf)
{
	int ispar = 0;
	if (query->Get()[0])
	{
		ispar = 1;
		query->Append(L"&(");
	}
	if (!_wcsnicmp(text, L"query:", 6)) query->Append(text + 6); // copy the query as is
	else if (text[0] == L'?') query->Append(text + 1);
	else // this is ubergay. no wait it isn't anymore. it rocks now due to the GayString
	{
		int isAny = 0;
		if (text && (*text == L'*' && text[1] == L' '))
		{
			isAny = 1;
			text += 2;
		}
		int cchText = lstrlenW(text);
		wchar_t *tmpbuf = (wchar_t*)calloc((cchText + 2), sizeof(wchar_t));
		parsequicksearch(tmpbuf, text);

		int x;
		const wchar_t *fields[5] =
		    {
		        L"filename",
		        L"title",
		        L"artist",
		        L"album",
		        L"genre",
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
					if (op && *op == L'\'') *op = L'\"';
					if (op) op++;
				}
				p[lenp - 1] = 0; // remove >
				query->Append(p + 1);
			}
			else
			{
				for (x = 0; x < (int)min(sizeof(fields) / sizeof(fields[0]), nf); x ++)
				{
					const wchar_t *field = fields[x];
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
		free(tmpbuf);
	}
	if (ispar) query->Append(L")");
}

static DWORD WINAPI history_bgThreadQueryProc(void *tmp)
{
	history_bgThreadParms *p=(history_bgThreadParms*)tmp;

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);
	NDE_Scanner_Query(s, g_q.Get());
	int total_length_s=history_saveQueryToList(s,&itemCache, p->user32, &history_bgThread_Kill);
	NDE_Table_DestroyScanner(g_table, s);
	LeaveCriticalSection(&g_db_cs);

	if (!history_bgThread_Kill) PostMessage(m_hwnd,WM_APP+3,0x69,total_length_s);
	return 0;
}

void history_bgQuery_Stop() // exported for other people to call since it is useful (eventually
{							// we should have bgQuery pass the new query info along but I'll do that soon)
	if (history_bgThread_Handle) 
	{
	    history_bgThread_Kill=1;
		WaitForSingleObject(history_bgThread_Handle,INFINITE);
		CloseHandle(history_bgThread_Handle);
		history_bgThread_Handle=0;
	}
	KillTimer(m_hwnd,123);
}

static void history_bgQuery(int user32=0) // only internal used
{
	history_bgQuery_Stop();

	SetDlgItemTextW(m_hwnd,IDC_MEDIASTATUS,WASABI_API_LNGSTRINGW(IDS_SCANNING_ELLIPSE));
	SetTimer(m_hwnd,123,200,NULL);

	DWORD id;
	static history_bgThreadParms parms;
	parms.user32=user32;
	history_bgThread_Kill=0;
	history_bgThread_Handle=CreateThread(NULL,0,history_bgThreadQueryProc,(LPVOID)&parms,0,&id);
}

static void doQuery(HWND hwndDlg, wchar_t *text, int dobg=1) {
	history_bgQuery_Stop();

	GayStringW query;  
	if (text[0]) makeQueryStringFromText(&query,text,2);

	wchar_t *parent_query=NULL;
	SendMessage(GetParent(hwndDlg),WM_APP+2,0,(LONG_PTR)&parent_query);

	g_q.Set(L"");

	if(parent_query && parent_query[0]) 
	{
	    g_q.Set(L"(");
		g_q.Append(parent_query);
		g_q.Append(L")");
	}

	if (query.Get() && query.Get()[0]) 
	{
	    if(g_q.Get()[0]) 
		{
			g_q.Append(L" & (");
			g_q.Append(query.Get());
			g_q.Append(L")");
		} 
		else g_q.Set(query.Get());
	}

	if (dobg) history_bgQuery();
}

static WNDPROC search_oldWndProc;
static DWORD WINAPI search_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if(uMsg == WM_KEYDOWN && wParam == VK_DOWN)
	{
		HWND hwndList = resultlist.getwnd();
		if (hwndList)
		{
			PostMessage(GetParent(hwndDlg), WM_NEXTDLGCTL, (WPARAM)hwndList, TRUE);
			ListView_SetItemState(hwndList,0,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
		}
	}
	return (DWORD)CallWindowProcW(search_oldWndProc,hwndDlg,uMsg,wParam,lParam);
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

static void removeSelectedItems(int isAll=0)
{
	int hasdel=0;
	history_bgQuery_Stop();

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);

	for(int i=0;i<itemCache.Size;i++) 
	{
		if(resultlist.GetSelected(i) || isAll) 
		{
			if(NDE_Scanner_LocateNDEFilename(s, HISTORYVIEW_COL_FILENAME,FIRST_RECORD,itemCache.Items[i].filename))
			{
				hasdel=1;
				NDE_Scanner_Edit(s);
				NDE_Scanner_Delete(s);
				NDE_Scanner_Post(s);
			}
		}
	}

	NDE_Table_DestroyScanner(g_table, s);

	if (!hasdel) 
	{
		LeaveCriticalSection(&g_db_cs);
		return;
	}

	NDE_Table_Sync(g_table);
	NDE_Table_Compact(g_table);
	g_table_dirty=0;
	LeaveCriticalSection(&g_db_cs);

	resultlist.Clear();
	emptyRecentRecordList(&itemCache);

	SendMessage(m_hwnd,WM_APP+1,0,0); //refresh current view
}

static void removeSelectedItemOffsets(int isAll=0)
{
	int hasdel=0;
	history_bgQuery_Stop();

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);

	for(int i=0;i<itemCache.Size;i++) 
	{
		if(resultlist.GetSelected(i) || isAll) 
		{
			if(NDE_Scanner_LocateNDEFilename(s, HISTORYVIEW_COL_FILENAME,FIRST_RECORD,itemCache.Items[i].filename))
			{
				db_setFieldInt(s, HISTORYVIEW_COL_OFFSET, -1);
				NDE_Scanner_Post(s);
				hasdel=1;
				itemCache.Items[i].offset = -1;
			}
		}
	}

	NDE_Table_DestroyScanner(g_table, s);

	if (!hasdel) 
	{
		LeaveCriticalSection(&g_db_cs);
		return;
	}

	NDE_Table_Sync(g_table);
	NDE_Table_Compact(g_table);
	g_table_dirty=0;
	LeaveCriticalSection(&g_db_cs);

	resultlist.RefreshAll();
}

static void History_SaveLastQuery(HWND hwnd)
{	
	LPSTR pszQuery = NULL;
	HWND hEditbox = GetDlgItem(hwnd, IDC_QUICKSEARCH);
	if (NULL != hEditbox)
	{
		UINT cchTextMax = GetWindowTextLength(hEditbox);
		if (0 != cchTextMax)
		{
			cchTextMax++;
			LPWSTR pszText = (LPWSTR)calloc(cchTextMax, sizeof(WCHAR));
			if (NULL != pszText)
			{
				UINT cchText = GetWindowTextW(hEditbox, pszText, cchTextMax);
				if (0 != cchText)
				{
					UINT cchQuery = WideCharToMultiByte(CP_UTF8, 0, pszText, cchText, NULL, 0, NULL, NULL);
					if (0 != cchQuery)
					{
						cchQuery++;
						pszQuery = (LPSTR)calloc(cchQuery, sizeof(CHAR));
						if (NULL != pszQuery)
						{
							cchQuery = WideCharToMultiByte(CP_UTF8, 0, pszText, cchText, pszQuery, cchQuery, NULL, NULL);
							pszQuery[cchQuery] = '\0';
						}
					}
				}
				free(pszText);
			}
		}
	}
	g_config->WriteString("recent_lastquery", pszQuery);
	if (NULL != pszQuery)
		free(pszQuery);
}

void SwapPlayEnqueueInMenu(HMENU listMenu)
{
	int playPos=-1, enqueuePos=-1;
	MENUITEMINFOW playItem={sizeof(MENUITEMINFOW), 0,}, enqueueItem={sizeof(MENUITEMINFOW), 0,};

	int numItems = GetMenuItemCount(listMenu);

	for (int i=0;i<numItems;i++)
	{
		UINT id = GetMenuItemID(listMenu, i);
		if (id == ID_MEDIAWND_PLAYSELECTEDFILES)
		{
			playItem.fMask = MIIM_ID;
			playPos = i;
			GetMenuItemInfoW(listMenu, i, TRUE, &playItem);
		}
		else if (id == ID_MEDIAWND_ENQUEUESELECTEDFILES)
		{
			enqueueItem.fMask = MIIM_ID;
			enqueuePos= i;
			GetMenuItemInfoW(listMenu, i, TRUE, &enqueueItem);
		}
	}
	
	playItem.wID = ID_MEDIAWND_ENQUEUESELECTEDFILES;
	enqueueItem.wID = ID_MEDIAWND_PLAYSELECTEDFILES;		
	SetMenuItemInfoW(listMenu, playPos, TRUE, &playItem);
	SetMenuItemInfoW(listMenu, enqueuePos, TRUE, &enqueueItem);
}

void SyncMenuWithAccelerators(HWND hwndDlg, HMENU menu)
{
	HACCEL szAccel[24] = {0};
	INT c = WASABI_API_APP->app_getAccelerators(hwndDlg, szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
	AppendMenuShortcuts(menu, szAccel, c, MSF_REPLACE);
}

static HRGN g_rgnUpdate = NULL;
static int offsetX = 0, offsetY = 0;

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
		GROUP_STATUSBAR, IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM, IDC_REMOVEBOOK, IDC_MEDIASTATUS,
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
				{
					wchar_t buffer[128] = {0};
					GetDlgItemTextW(hwnd, IDC_BUTTON_PLAY, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(GetDlgItem(hwnd, IDC_BUTTON_PLAY), buffer);

					SetRect(&rg, rc.left, rc.top + WASABI_API_APP->getScaleY(2),
							rc.right - WASABI_API_APP->getScaleX(2),
							rc.top + WASABI_API_APP->getScaleY(HIWORD(idealSize)+1));
					rc.top = rg.bottom + WASABI_API_APP->getScaleY(3);
					break;
				}
				case GROUP_STATUSBAR:
				{
					wchar_t buffer[128] = {0};
					HWND ctrl = GetDlgItem(hwnd, IDC_BUTTON_PLAY);
					GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

					SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1),
							rc.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)),
							rc.right, rc.bottom);
					rc.bottom = rg.top - WASABI_API_APP->getScaleY(3);
					break;
				}
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
				SETLAYOUTPOS(pl, rg.right - width, rg.top, width, (rg.bottom - rg.top));
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
			case IDC_BUTTON_CUSTOM:
			case IDC_REMOVEBOOK:
				if (IDC_BUTTON_CUSTOM != pl->id || customAllowed)
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
			case IDC_MEDIASTATUS:
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left, (rg.bottom - rg.top));
				pl->flags |= (pl->cx > WASABI_API_APP->getScaleX(16)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				break;
			case IDC_LIST2:
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1),
							 (rg.right - rg.left) - WASABI_API_APP->getScaleX(3),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(2));
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

static void LayoutWindows2(HWND hwnd, BOOL fRedraw)
{
	RECT rc, rg;
	HRGN rgn = NULL;

	GetClientRect(hwnd, &rc);
	SetRect(&rg, 0, 0, 0, 0);

	rc.top += WASABI_API_APP->getScaleY(2);
	rc.right -= WASABI_API_APP->getScaleY(2);

	if (rc.bottom <= rc.top || rc.right <= rc.left) return;

	HWND temp = GetDlgItem(hwnd, IDC_DB_ERROR);
	GetWindowRect(temp, &rg);
	SetWindowPos(temp, NULL, WASABI_API_APP->getScaleX(20), WASABI_API_APP->getScaleY(20),
				 rc.right - rc.left - WASABI_API_APP->getScaleX(40),
				 rc.bottom - rc.top - WASABI_API_APP->getScaleY(45),
				 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);

	temp = GetDlgItem(hwnd, IDC_RESET_DB_ON_ERROR);
	GetWindowRect(temp, &rg);
	SetWindowPos(temp, NULL, ((rc.right - rc.left) - (rg.right - rg.left)) / WASABI_API_APP->getScaleX(2),
				 rc.bottom - (rg.bottom - rg.top),
				 rg.right - rg.left, rg.bottom - rg.top,
				 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);

	InvalidateRect(hwnd, NULL, TRUE);

	if (fRedraw) 
	{
		UpdateWindow(hwnd);
	}
	if (g_rgnUpdate)
	{
		GetUpdateRgn(hwnd, g_rgnUpdate, FALSE);
		if (rgn)
		{
			OffsetRgn(rgn, rc.left, rc.top);
			CombineRgn(g_rgnUpdate, g_rgnUpdate, rgn, RGN_OR);
		}
	}
	ValidateRgn(hwnd, NULL);
	if (rgn) DeleteObject(rgn);	
}

static void history_ManageButtons(HWND hwndDlg)
{
	int has_selection = resultlist.GetSelectedCount();

	const int buttonids[] = { IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM, IDC_REMOVEBOOK};
	for (size_t i = 0; i != sizeof(buttonids)/sizeof(buttonids[0]); i++)
	{
		HWND controlHWND = GetDlgItem(hwndDlg, buttonids[i]);
		EnableWindow(controlHWND, has_selection);
	}
}

void history_UpdateButtonText(HWND hwndDlg, int _enqueuedef)
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

void UpdateMenuItems(HWND hwndDlg, HMENU menu)
{
	bool swapPlayEnqueue=false;
	if (g_config->ReadInt(L"enqueuedef", 0) == 1)
	{
		SwapPlayEnqueueInMenu(menu);
		swapPlayEnqueue=true;
	}
	
	SyncMenuWithAccelerators(hwndDlg, menu);
	if (swapPlayEnqueue) SwapPlayEnqueueInMenu(menu);
}

enum
{
	BPM_ECHO_WM_COMMAND=0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND = 0x2, // just send WM_COMMAND
};

BOOL history_ButtonPopupMenu(HWND hwndDlg, int buttonId, HMENU menu, int flags=0)
{
	RECT r;
	HWND buttonHWND = GetDlgItem(hwndDlg, buttonId);

	GetWindowRect(buttonHWND, &r);
	UpdateMenuItems(hwndDlg, menu);
	MLSkinnedButton_SetDropDownState(buttonHWND, TRUE);

	UINT tpmFlags = TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN;
	if (!(flags & BPM_WM_COMMAND))
		tpmFlags |= TPM_RETURNCMD;
	int x = Menu_TrackPopup(plugin.hwndLibraryParent, menu, tpmFlags, r.left, r.top, hwndDlg, NULL);
	if ((flags & BPM_ECHO_WM_COMMAND) && x)
		SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(x, 0), 0);
	MLSkinnedButton_SetDropDownState(buttonHWND, FALSE);
	return x;
}

static void history_Play(HWND hwndDlg, HWND from, UINT idFrom)
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

	history_ButtonPopupMenu(hwndDlg, idFrom, listMenu, BPM_WM_COMMAND);
}

BOOL CALLBACK view_historyDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	BOOL a = (BOOL)dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam); if (a) return a;

	static HMENU sendto_hmenu;
	static librarySendToMenuStruct s;

	switch(uMsg)
	{
		case WM_INITMENUPOPUP:
			if (wParam && (HMENU)wParam == s.build_hMenu && s.mode==1)
			{
				if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU)==0xffffffff)
				s.mode=2;
			}
			return 0;

		case WM_DISPLAYCHANGE:
		{
			ListView_SetTextColor(resultlist.getwnd(), dialogSkinner.Color(WADLG_ITEMFG));
			ListView_SetBkColor(resultlist.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
			ListView_SetTextBkColor(resultlist.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
			resultlist.SetFont(dialogSkinner.GetFont());
			UpdateWindow(hwndDlg);
			LayoutWindows(hwndDlg, TRUE);
			return 0;
		}

		case WM_CONTEXTMENU:
		{
			HWND hwndFrom = (HWND)wParam;
			if (hwndFrom != resultlist.getwnd())
				return 0;

			int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
			POINT pt = {x, y};

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
				return 0;
			}

			HMENU g_context_menus=WASABI_API_LOADMENU(IDR_CONTEXTMENUS);
			HMENU menu=GetSubMenu(g_context_menus,0);
			sendto_hmenu=GetSubMenu(menu,2);

			UpdateMenuItems(hwndDlg, menu);

			s.mode = 0;
			s.hwnd = 0;
			s.build_hMenu = 0;

			IPC_LIBRARY_SENDTOMENU = (INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&"LibrarySendToMenu",IPC_REGISTER_WINAMP_IPCMESSAGE);
			if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)0,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
			{
				s.mode = 1;
				s.hwnd = hwndDlg;
				s.data_type = ML_TYPE_FILENAMESW;
				s.ctx[1] = 1;
				s.build_hMenu = sendto_hmenu;
			}

			UINT menustate = 0;
			int n=resultlist.GetSelectedCount();
			if(n == 0)
			{
				menustate = MF_BYCOMMAND|MF_GRAYED;
			}
			else
			{
				menustate = MF_BYCOMMAND|MF_ENABLED;
			}
			
			EnableMenuItem(menu,ID_PE_ID3,menustate);
			EnableMenuItem(menu,ID_MEDIAWND_PLAYSELECTEDFILES,menustate);
			EnableMenuItem(menu,ID_MEDIAWND_ENQUEUESELECTEDFILES,menustate);
			EnableMenuItem(menu,ID_MEDIAWND_EXPLOREFOLDER,menustate);
			EnableMenuItem(menu,ID_MEDIAWND_REMOVEFROMLIBRARY,menustate);
			EnableMenuItem(menu,ID_MEDIAWND_REMOVEOFFSETFROMLIBRARY,menustate);
			EnableMenuItem(menu, 2, MF_BYPOSITION|(n==0?MF_GRAYED:MF_ENABLED));

			int r = Menu_TrackPopup(plugin.hwndLibraryParent, menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, x, y, hwndDlg, NULL);
			if(!SendMessage(hwndDlg,WM_COMMAND,r,0))
			{
				if (s.mode == 2)
				{
					s.menu_id = r;
					if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
					{
						// build my data.
						s.mode=3;
						s.data_type=ML_TYPE_FILENAMESW;

						//std::vector<wchar_t> sendStr;
						std::wstring sendStr;

						int l=resultlist.GetCount();
						for(int i=0;i<l;i++) 
						{
							if (resultlist.GetSelected(i)) 
							{
								// HAKAN: why (len + 1) ?
								//sendStr.append(itemCache.Items[i].filename, wcslen(itemCache.Items[i].filename)+1);
								sendStr.append(itemCache.Items[i].filename, wcslen(itemCache.Items[i].filename));
							}
						}
						// HAKAN: No need to add trailing zero
						//sendStr.push_back(0);
						
						s.data = (void*)sendStr.c_str();

						if(SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU)!=1)
						{
							s.mode=3;
							s.data_type=ML_TYPE_FILENAMES;
	
							//std::vector<char> sendStrA;
							std::string sendStrA;

							int l=resultlist.GetCount();
							for(int i=0;i<l;i++) 
							{
								if (resultlist.GetSelected(i)) 
								{
									// HAKAN: why (len + 1) ?
									//sendStrA.append(AutoCharFn(itemCache.Items[i].filename), strlen(AutoCharFn(itemCache.Items[i].filename))+1);
									sendStrA.append(AutoCharFn(itemCache.Items[i].filename), strlen(AutoCharFn(itemCache.Items[i].filename)));
								}
							}
							// HAKAN: No need to add trailing zero
							//sendStrA.push_back(0);
							
							s.data = (void*)sendStrA.c_str();
	
							SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU);
						}
					}
				}
			}

			if (s.mode) 
			{
				s.mode=4;
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU); // cleanup
			}

			sendto_hmenu=0;
			DestroyMenu(g_context_menus);
			Sleep(100);
			MSG msg = {0};
			while(PeekMessage(&msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE)); //eat return
		}
			return 0;

		case WM_INITDIALOG:
		{
			m_hwnd=hwndDlg;
			g_q.Set(L"");

			HACCEL accel = WASABI_API_LOADACCELERATORSW(IDR_VIEW_ACCELERATORS);
			if (accel)
				WASABI_API_APP->app_addAccelerators(hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD);

			if (!view.play)
			{
				SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view);
			}

			history_cleanupifnecessary();

			itemCache.Items=0;
			itemCache.Alloc=0;
			itemCache.Size=0;

			resultlist.setwnd(GetDlgItem(hwndDlg,IDC_LIST2));
			resultlist.ForceUnicode();
			resultSkin = (int)(INT_PTR)resultlist.getwnd(); //Might be unsafe

			groupBtn = g_config->ReadInt(L"groupbtn", 1);
			enqueuedef = (g_config->ReadInt(L"enqueuedef", 0) == 1);

			// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
			//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
			pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG(IDC_BUTTON_CUSTOM, IDC_BUTTON_ENQUEUE), (INT_PTR)L"ml_history"};
			wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
			if (pszTextW && pszTextW[0] != 0)
			{
				// set this to be a bit different so we can just use one button and not the
				// mixable one as well (leaving that to prevent messing with the resources)
				customAllowed = TRUE;
				SetDlgItemTextW(hwndDlg, IDC_BUTTON_CUSTOM, pszTextW);
			}
			else
				customAllowed = FALSE;

			MLSKINWINDOW m = {0};
			m.skinType = SKINNEDWND_TYPE_LISTVIEW;
			m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
			m.hwndToSkin = resultlist.getwnd();
			MLSkinWindow(mediaLibrary.library, &m);

			m.skinType = SKINNEDWND_TYPE_BUTTON;
			m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | (groupBtn ? SWBS_SPLITBUTTON : 0);

			FLICKERFIX ff = {0, FFM_ERASEINPAINT};
			const int buttonids[] = {IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM};
			for (size_t i=0;i!=sizeof(buttonids)/sizeof(buttonids[0]);i++)
			{
				m.hwndToSkin = ff.hwnd = GetDlgItem(hwndDlg, buttonids[i]);
				if (IsWindow(m.hwndToSkin))
				{
					MLSkinWindow(plugin.hwndLibraryParent, &m);
					SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_FLICKERFIX, (WPARAM)&ff);
				}
			}

			INT ffcl[] = {IDC_REMOVEBOOK, IDC_CLEAR, IDC_MEDIASTATUS, IDC_QUICKSEARCH, IDC_SEARCHCAPTION};
			m.skinType = SKINNEDWND_TYPE_AUTO;
			m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
			for (INT index = 0; index < sizeof(ffcl) / sizeof(INT); index++)
			{
				ff.hwnd = GetDlgItem(hwndDlg, ffcl[index]);
				if (IsWindow(ff.hwnd))
				{
					SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_FLICKERFIX, (WPARAM)&ff);
					m.hwndToSkin = ff.hwnd;
					MLSkinWindow(plugin.hwndLibraryParent, &m);
				}
			}

			ListView_SetTextColor(resultlist.getwnd(),dialogSkinner.Color(WADLG_ITEMFG));
			ListView_SetBkColor(resultlist.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
			ListView_SetTextBkColor(resultlist.getwnd(),dialogSkinner.Color(WADLG_ITEMBG));

			resultlist.SetFont(dialogSkinner.GetFont());

			resultlist.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_LAST_PLAYED), g_config->ReadInt(L"recent_col_lp", 111));
			resultlist.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_PLAY_COUNT), g_config->ReadInt(L"recent_col_count", 70));
			resultlist.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_TITLE), g_config->ReadInt(L"recent_col_title", 238));
			resultlist.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_LENGTH), g_config->ReadInt(L"recent_col_len", 50));
			resultlist.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_FILENAME), g_config->ReadInt(L"recent_col_filename", 285));
			resultlist.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_OFFSET), g_config->ReadInt(L"recent_col_offset", 80));

			m_headerhwnd=ListView_GetHeader(resultlist.getwnd());

			{
				char *query="";
				if (g_config->ReadInt(L"remembersearch",0)) query = g_config->ReadString("recent_lastquery", "");
				AutoWide queryUnicode(query, CP_UTF8);
				SetDlgItemTextW(hwndDlg,IDC_QUICKSEARCH,queryUnicode);
				KillTimer(hwndDlg,UPDATE_QUERY_TIMER_ID);
				doQuery(hwndDlg,queryUnicode,0);
			}

			{
				int l_sc=g_config->ReadInt(L"recent_sort_by", HISTORY_SORT_LASTPLAYED);
				int l_sd=g_config->ReadInt(L"recent_sort_dir", 0);
				mediaLibrary.ListViewSort(resultSkin, l_sc, l_sd);
				mediaLibrary.ListViewShowSort(resultSkin, TRUE);
			}

			history_ManageButtons(hwndDlg);
			history_UpdateButtonText(hwndDlg, enqueuedef == 1);

			search_oldWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(GetDlgItem(hwndDlg,IDC_QUICKSEARCH),GWLP_WNDPROC,(LONG_PTR)search_newWndProc);
			break;
		}

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags));
			}
			return 0;

		case WM_USER + 0x201:
			offsetX = (short)LOWORD(wParam);
			offsetY = (short)HIWORD(wParam);
			g_rgnUpdate = (HRGN)lParam;
			return TRUE;

		case WM_MOUSEMOVE:
			if (GetCapture()==hwndDlg)
			{
				POINT p;
				p.x=GET_X_LPARAM(lParam);
				p.y=GET_Y_LPARAM(lParam);
				ClientToScreen(hwndDlg,&p);
				mlDropItemStruct m={0};
				m.type=ML_TYPE_FILENAMESW;
				m.p=p;

				pluginHandleIpcMessage(ML_IPC_HANDLEDRAG,(WPARAM)&m);
				break;
			}
			break;

		case WM_LBUTTONUP:
			if (GetCapture()==hwndDlg)
			{
				ReleaseCapture();
				POINT p;
				p.x=GET_X_LPARAM(lParam);
				p.y=GET_Y_LPARAM(lParam);
				ClientToScreen(hwndDlg,&p);
				mlDropItemStruct m={0};
				m.type=ML_TYPE_FILENAMESW;
				m.p=p;
				m.flags=ML_HANDLEDRAG_FLAG_NOCURSOR;

				pluginHandleIpcMessage(ML_IPC_HANDLEDRAG,(WPARAM)&m);

				if (m.result>0)
				{
					size_t buf_size=4096;
					wchar_t *buf=(wchar_t*)calloc(buf_size, sizeof(wchar_t));
					int buf_pos=0;

					int l=resultlist.GetCount();
					for(int i=0;i<l;i++) 
					{
						if (resultlist.GetSelected(i)) 
						{
							const wchar_t *tbuf = itemCache.Items[i].filename;
							size_t cchFilename = wcslen(tbuf) + 1;
							size_t newsize=buf_pos + cchFilename; 
							if (newsize < buf_size)
							{
								size_t old_buf_size = buf_size;
								buf_size=newsize+4096;
								wchar_t *reallocated_buf = (wchar_t*)realloc(buf,buf_size*sizeof(wchar_t));
								if (reallocated_buf)
								{
									buf = reallocated_buf;
								}
								else
								{
									wchar_t *newbuf = (wchar_t*)malloc(buf_size*sizeof(wchar_t));
									if (!newbuf) // out of memory? well we can at least send what we've got
										break;
									memcpy(newbuf, buf, old_buf_size);
									free(buf);				
									buf = newbuf;
								}
							}
							StringCchCopyNW(buf+buf_pos,buf_size-buf_pos,tbuf,cchFilename);
							buf_pos=(int)newsize;
						}
					}

					if (buf_pos)
					{
						buf[buf_pos]=0;
						m.flags=0;
						m.result=0;
						m.data=(void*)buf;
						pluginHandleIpcMessage(ML_IPC_HANDLEDROP,(WPARAM)&m);
					}
      
					free(buf);
				}      
			}
			break;

		case WM_COMMAND:
			if (GetFocus() != GetDlgItem(hwndDlg, IDC_QUICKSEARCH))
			{
				switch(LOWORD(wParam))
				{
					case IDC_REMOVEBOOK:
						removeSelectedItems(0);
						break;
					case IDC_CLEAR:
						SetDlgItemText(hwndDlg,IDC_QUICKSEARCH,TEXT(""));
						break;
					case IDC_QUICKSEARCH:
						if (HIWORD(wParam) == EN_CHANGE)
						{
							KillTimer(hwndDlg,UPDATE_QUERY_TIMER_ID);
							SetTimer(hwndDlg,UPDATE_QUERY_TIMER_ID,100,NULL);
						}
						break;
					case IDC_BUTTON_PLAY:
					case ID_MEDIAWND_PLAYSELECTEDFILES: 
					case IDC_BUTTON_ENQUEUE:
					case ID_MEDIAWND_ENQUEUESELECTEDFILES:
					case IDC_BUTTON_CUSTOM:
					{
						if (HIWORD(wParam) == MLBN_DROPDOWN)
						{
							history_Play(hwndDlg, (HWND)lParam, LOWORD(wParam));
						}
						else
						{
							int action;
							if (LOWORD(wParam) == IDC_BUTTON_PLAY || LOWORD(wParam) == ID_MEDIAWND_PLAYSELECTEDFILES)
							{
								action = (HIWORD(wParam) == 1) ? g_config->ReadInt(L"enqueuedef", 0) == 1 : 0;
							}
							else if (LOWORD(wParam) == IDC_BUTTON_ENQUEUE || LOWORD(wParam) == ID_MEDIAWND_ENQUEUESELECTEDFILES)
							{
								action = (HIWORD(wParam) == 1) ? g_config->ReadInt(L"enqueuedef", 0) != 1 : 1;
							}
							else
								break;

							playFiles(action, 0);
						}
						break;
					}
					case ID_MEDIAWND_SELECTALL:
						ListView_SetItemState(resultlist.getwnd(), -1, LVIS_SELECTED, LVIS_SELECTED);
						break;
					case ID_MEDIAWND_REMOVEFROMLIBRARY:
						removeSelectedItems(0);
						break;
					case ID_MEDIAWND_REMOVEOFFSETFROMLIBRARY:
						removeSelectedItemOffsets(0);
						break;
					case ID_MEDIAWND_EXPLOREFOLDER:
						exploreItemFolder(hwndDlg);
						break;
					case ID_PE_ID3:
						fileInfoDialogs(hwndDlg);
						break;
				}
			}
			else
			{
				switch(LOWORD(wParam))
				{
					case IDC_QUICKSEARCH:
						if (HIWORD(wParam) == EN_CHANGE)
						{
							KillTimer(hwndDlg,UPDATE_QUERY_TIMER_ID);
							SetTimer(hwndDlg,UPDATE_QUERY_TIMER_ID,100,NULL);
						}
						break;
					case ID_MEDIAWND_SELECTALL:
						SendDlgItemMessageW(hwndDlg, IDC_QUICKSEARCH, EM_SETSEL, 0, -1);
						break;
					case ID_MEDIAWND_REMOVEFROMLIBRARY:
						{
							DWORD start = -1, end = -1;
							SendDlgItemMessageW(hwndDlg, IDC_QUICKSEARCH, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
							if (start != -1)
							{
								if (start == end)
								{
									SendDlgItemMessageW(hwndDlg, IDC_QUICKSEARCH, EM_SETSEL, start, end + 1);
								}
								SendDlgItemMessageW(hwndDlg, IDC_QUICKSEARCH, EM_REPLACESEL, TRUE, (LPARAM)L"");
								SendDlgItemMessageW(hwndDlg, IDC_QUICKSEARCH, EM_SETSEL, start, start);
							}
						}
						break;
				}
			}
			break;

		case WM_TIMER:
			if (wParam == 123)
			{
				if (history_bgThread_Handle) 
				{
					ListView_SetItemCount(resultlist.getwnd(),1);
					ListView_RedrawItems(resultlist.getwnd(),0,0);
				}
			}
			else if (wParam == UPDATE_QUERY_TIMER_ID)
			{
				KillTimer(hwndDlg,UPDATE_QUERY_TIMER_ID);
				wchar_t text[512] = {0};
				GetWindowTextW(GetDlgItem(hwndDlg,IDC_QUICKSEARCH),text,511-1);
				text[511]=0;
				doQuery(hwndDlg,text);
			}
			return 0;

		case WM_APP+3: // sent by bgthread
			if (wParam == 0x69) 
			{
				history_bgQuery_Stop();

				ListView_SetItemCount(resultlist.getwnd(),0);
				ListView_SetItemCount(resultlist.getwnd(),itemCache.Size);
				if (itemCache.Size>0) ListView_RedrawItems(resultlist.getwnd(),0,itemCache.Size-1);

				if (m_lv_last_topidx)
				{
					ListView_EnsureVisible(resultlist.getwnd(),m_lv_last_topidx,FALSE);
					m_lv_last_topidx=0;
				}

				unsigned int total_plays=0;
				int x;
				for (x = 0; x < itemCache.Size; x ++)
				{
					total_plays += itemCache.Items[x].playcnt;
				}

				int total_length_s = (int)lParam & 0x7FFFFFFF;
				int uncert=(int)(lParam>>31);
				wchar_t buf[1024] = {0}, itemStr[16] = {0}, playStr[16] = {0};

				StringCchPrintfW(buf, 1024,
				                 L"%d %s, %u %s ",
								 itemCache.Size,
								 WASABI_API_LNGSTRINGW_BUF(itemCache.Size==1?IDS_ITEM:IDS_ITEMS,itemStr,16),
								 total_plays,
								 WASABI_API_LNGSTRINGW_BUF(total_plays==1?IDS_PLAY:IDS_PLAYS,playStr,16));

				if (total_length_s < 60*60)
				{
					StringCchPrintfW(buf+wcslen(buf), 64, L"[%s%u:%02u]",
									 uncert ? L"~" : L"", total_length_s / 60,
									 total_length_s % 60);
				}
				else if (total_length_s < 60*60*24)
				{
					StringCchPrintfW(buf+wcslen(buf), 64, L"[%s%u:%02u:%02u]",
									 uncert ? L"~" : L"", total_length_s / 60 / 60,
									 (total_length_s / 60) % 60, total_length_s % 60);
				}
				else
				{
					wchar_t days[16] = {0};
					int total_days = total_length_s / (60 * 60 * 24);
					total_length_s -= total_days * 60 * 60 * 24;
					StringCchPrintfW(buf+wcslen(buf), 64,
					                 L"[%s%u %s+%u:%02u:%02u]",
					                 uncert ? L"~" : L"", total_days,
					                 WASABI_API_LNGSTRINGW_BUF(total_days == 1 ? IDS_DAY : IDS_DAYS, days, 16),
					                 total_length_s / 60 / 60, (total_length_s / 60) % 60, total_length_s % 60);
				}

				SetDlgItemTextW(hwndDlg,IDC_MEDIASTATUS,buf);
			}
			break;

		case WM_DESTROY:
			if (resultlist.getwnd())
			{
				g_config->WriteInt(L"recent_col_lp", resultlist.GetColumnWidth(0));
				g_config->WriteInt(L"recent_col_count", resultlist.GetColumnWidth(1));
				g_config->WriteInt(L"recent_col_title", resultlist.GetColumnWidth(2));
				g_config->WriteInt(L"recent_col_len", resultlist.GetColumnWidth(3));
				g_config->WriteInt(L"recent_col_filename", resultlist.GetColumnWidth(4));
				g_config->WriteInt(L"recent_col_offset", resultlist.GetColumnWidth(5));
			}

			History_SaveLastQuery(hwndDlg);

			if (g_table_dirty && g_table)
			{
				EnterCriticalSection(&g_db_cs);
				NDE_Table_Sync(g_table);
				g_table_dirty=0;
				LeaveCriticalSection(&g_db_cs);
			}

			emptyRecentRecordList(&itemCache);
			free(itemCache.Items);
			itemCache.Items=0;
			itemCache.Alloc=0;
			itemCache.Size=0;

			m_hwnd=0;
			g_q.Set(L"");
			WASABI_API_APP->app_removeAccelerators(hwndDlg);
			break;

		case WM_PAINT:
		{
			int tab[] = {IDC_QUICKSEARCH|DCW_SUNKENBORDER, IDC_LIST2|DCW_SUNKENBORDER};
			dialogSkinner.Draw(hwndDlg, tab, 1 + !!IsWindowVisible(GetDlgItem(hwndDlg, IDC_QUICKSEARCH)));
		}
			return 0;

		case WM_APP+1:
			history_bgQuery((int)lParam);
			break;

		case WM_ML_CHILDIPC:
			if(lParam == ML_CHILDIPC_GO_TO_SEARCHBAR)
			{
				SendDlgItemMessage(hwndDlg, IDC_QUICKSEARCH, EM_SETSEL, 0, -1);
				SetFocus(GetDlgItem(hwndDlg,IDC_QUICKSEARCH));
			}
			else if (lParam == ML_CHILDIPC_REFRESH_SEARCH)
			{
				PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_QUICKSEARCH, EN_CHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_QUICKSEARCH));
			}
			break;

		case WM_ERASEBKGND:
			return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT

		case WM_NOTIFY:
		{
			LPNMHDR l=(LPNMHDR)lParam;
			if (l->idFrom==IDC_LIST2) // media view
			{
				if (l->code == NM_DBLCLK)
				{
					playFiles((!!g_config->ReadInt(L"enqueuedef", 0)) ^ (!!(GetAsyncKeyState(VK_SHIFT)&0x8000)),0);
				}
				else if (l->code == LVN_ODFINDITEMW) // yay we find an item (for kb shortcuts)
				{
					if (history_bgThread_Handle) return 0;
					NMLVFINDITEMW *t = (NMLVFINDITEMW *)lParam;
					int i=t->iStart;
					if (i >= itemCache.Size) i=0;

					int cnt=itemCache.Size-i;
					if (t->lvfi.flags & LVFI_WRAP) cnt+=i;

					int by = g_config->ReadInt(L"recent_sort_by", HISTORY_SORT_LASTPLAYED);

					while (cnt-->0)
					{
						historyRecord *thisitem=itemCache.Items + i;
						wchar_t tmp[128] = {0};
						const wchar_t *name=0;

						switch (by)
						{
							case HISTORYVIEW_COL_FILENAME:
								name=thisitem->filename;
								if (!wcsstr(name,L"://"))
								{
									while (name && *name) name++;
									while (name && name >= thisitem->filename && *name != '/' && *name != '\\') name--;
									if (name) name++;
								}
								break;
							case HISTORYVIEW_COL_TITLE: name=thisitem->title; break;
							case HISTORYVIEW_COL_LASTPLAYED: 
								tmp[0]=0;
								if (thisitem->lastplayed > 0)
								{
									__time64_t timev = thisitem->lastplayed;
									MakeDateStringW(timev, tmp, ARRAYSIZE(tmp));
								}
								name=tmp;
								break;
							case HISTORYVIEW_COL_PLAYCOUNT: 
								StringCchPrintfW(tmp,128,L"%u",thisitem->playcnt);
								name=tmp;
								break;
							case HISTORYVIEW_COL_LENGTH:
								tmp[0]=0;
								if (thisitem->length >= 0) StringCchPrintfW(tmp,128,L"%d:%02d",thisitem->length/60,thisitem->length%60);
								name=tmp;
								break;
							case HISTORYVIEW_COL_OFFSET:
								tmp[0]=0;
								if (thisitem->offset > 0) StringCchPrintfW(tmp,128,L"%d:%02d",(thisitem->offset/1000)/60,(thisitem->offset/1000)%60);
								name=tmp;
								break;
						}

						if (!name) name=L"";
						else SKIP_THE_AND_WHITESPACE(name)

						if (t->lvfi.flags & (4|LVFI_PARTIAL))
						{
							if (!_wcsnicmp(name,t->lvfi.psz,lstrlenW(t->lvfi.psz)))
							{
								SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,i);
								return 1;
							}
						}
						else if (t->lvfi.flags & LVFI_STRING)
						{
							if (!_wcsicmp(name,t->lvfi.psz))
							{
								SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,i);
								return 1;
							}
						}
						else
						{
							SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,-1);
							return 1;
						}
						if (++i == itemCache.Size) i=0;
					}
					SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,-1);
					return 1;
				}
				else if (l->code == LVN_GETDISPINFOW)
				{
					NMLVDISPINFOW *lpdi = (NMLVDISPINFOW*) lParam;
					int item=lpdi->item.iItem;

					if (history_bgThread_Handle) 
					{
						if (!item && lpdi->item.iSubItem == 0 && lpdi->item.mask & LVIF_TEXT)
						{
							static char bufpos;
							static char chars[4]={'/','-','\\','|'};
							StringCchPrintfW(lpdi->item.pszText,lpdi->item.cchTextMax,
											 L"%s %c",WASABI_API_LNGSTRINGW(IDS_SCANNING),
											 chars[bufpos++&3]);
							return 0;
						}
						}

						if (item < 0 || item >= itemCache.Size) return 0;

						historyRecord *thisitem = itemCache.Items + item;

						if (lpdi->item.mask & (LVIF_TEXT|/*LVIF_IMAGE*/0)) // we can always do images too :)
						{
							if (lpdi->item.mask & LVIF_TEXT)
							{
								wchar_t tmpbuf[128] = {0};
								const wchar_t *nameptr=0;
								switch (lpdi->item.iSubItem)
								{
									case HISTORYVIEW_COL_FILENAME:
									nameptr=thisitem->filename;
									if (!wcsstr(nameptr,L"://"))
									{
										while (nameptr && *nameptr) nameptr++;
										while (nameptr && nameptr >= thisitem->filename && *nameptr != L'/' && *nameptr != L'\\') nameptr--;
										if (nameptr) nameptr++;
									}
									break;
								case HISTORYVIEW_COL_TITLE: nameptr=thisitem->title; break;
								case HISTORYVIEW_COL_LASTPLAYED: 
									if (thisitem->lastplayed > 0)
									{
										__time64_t timev = thisitem->lastplayed;
										MakeDateStringW(timev, tmpbuf, ARRAYSIZE(tmpbuf));
									}
									nameptr=tmpbuf;
									break;
								case HISTORYVIEW_COL_PLAYCOUNT:
									StringCchPrintfW(tmpbuf,128,L"%u",thisitem->playcnt);
									nameptr=tmpbuf;
									break;
								case HISTORYVIEW_COL_LENGTH:
									if (thisitem->length >= 0)
										StringCchPrintfW(tmpbuf,128,L"%d:%02d",thisitem->length/60,thisitem->length%60);
									nameptr=tmpbuf;
									break;
								case HISTORYVIEW_COL_OFFSET:
									if (thisitem->offset > 0)
										StringCchPrintfW(tmpbuf,128,L"%d:%02d",(thisitem->offset/1000)/60,(thisitem->offset/1000)%60);
									nameptr=tmpbuf;
									break;
							}
							if (nameptr) lstrcpynW(lpdi->item.pszText,nameptr,lpdi->item.cchTextMax);
							else lpdi->item.pszText[0]=0;
						}
						// if(lpdi->item.mask & LVIF_IMAGE)
					} // bother
					return 0;
				} // LVN_GETDISPINFO
				else if (l->code == LVN_COLUMNCLICK)
				{
					NMLISTVIEW *p=(NMLISTVIEW*)lParam;
					int l_sc=g_config->ReadInt(L"recent_sort_by",HISTORY_SORT_LASTPLAYED);
					int l_sd=g_config->ReadInt(L"recent_sort_dir",0);
					if (p->iSubItem == l_sc) l_sd=!l_sd;
					else { l_sd=0; l_sc=p->iSubItem; }

					g_config->WriteInt(L"recent_sort_by",l_sc);
					g_config->WriteInt(L"recent_sort_dir",l_sd);

					mediaLibrary.ListViewSort(resultSkin, l_sc, l_sd);
					sortResults(&itemCache,
								 g_config->ReadInt(L"recent_sort_by",HISTORY_SORT_LASTPLAYED),
								 g_config->ReadInt(L"recent_sort_dir",0));
					ListView_SetItemCount(resultlist.getwnd(),0);
					ListView_SetItemCount(resultlist.getwnd(),itemCache.Size);
					ListView_RedrawItems(resultlist.getwnd(),0,itemCache.Size-1);
				}
				else if (l->code == LVN_BEGINDRAG)
				{
					SetCapture(hwndDlg);
				}
				else if (l->code == LVN_ITEMCHANGED)
				{
					history_ManageButtons(hwndDlg);
				}
			}
		}
		break;

		case WM_APP + 104:
		{
			history_UpdateButtonText(hwndDlg, (int)wParam);
			LayoutWindows(hwndDlg, TRUE);
			return 0;
		}
	}
	return FALSE;
}

void nukeHistory(HWND hwndDlg)
{
	wchar_t titleStr[32] = {0};
	if (MessageBoxW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_REMOVE_ALL_HISTORY),
		WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRMATION,titleStr,32),
		MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		closeDb();

		wchar_t tmp[MAX_PATH] = {0};
		StringCchPrintfW(tmp, MAX_PATH, L"%s\\recent.dat", g_tableDir);
		DeleteFileW(tmp);
		StringCchPrintfW(tmp, MAX_PATH, L"%s\\recent.idx", g_tableDir);
		DeleteFileW(tmp);

		openDb();

		// trigger a refresh of the current view
		PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
	}
}

BOOL CALLBACK view_errorinfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	BOOL a= (BOOL)dialogSkinner.Handle(hwndDlg,uMsg,wParam,lParam); if (a) return a;
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			SetWindowText(GetDlgItem(hwndDlg, IDC_DB_ERROR),
						  (wchar_t*)WASABI_API_LOADRESFROMFILEW(TEXT("TEXT"), MAKEINTRESOURCE((nde_error ? IDR_NDE_ERROR : IDR_DB_ERROR)), 0));

			if (nde_error)
				DestroyWindow(GetDlgItem(hwndDlg, IDC_RESET_DB_ON_ERROR));

			FLICKERFIX ff;
			INT index;
			INT ffcl[] = {	IDC_DB_ERROR,
							IDC_RESET_DB_ON_ERROR,
						 };

			ff.mode = FFM_ERASEINPAINT;
			for (index = 0; index < (sizeof(ffcl) / sizeof(INT)); index++)
			{
				ff.hwnd = GetDlgItem(hwndDlg, ffcl[index]);
				if (IsWindow(ff.hwnd))
				{
					SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_FLICKERFIX, (WPARAM)&ff);
				}
			}

			MLSKINWINDOW m = {0};
			m.skinType = SKINNEDWND_TYPE_DIALOG;
			m.hwndToSkin = hwndDlg;
			m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
			MLSkinWindow(plugin.hwndLibraryParent, &m);
		}
		return TRUE;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_RESET_DB_ON_ERROR)
			{
				nukeHistory(hwndDlg);
			}
		break;

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows2(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags));
			}
		return 0;

		case WM_USER+66:
			if (wParam == -1) 
			{
				LayoutWindows2(hwndDlg, TRUE);
			}
		return TRUE;

		case WM_USER + 0x201:
			offsetX = (short)LOWORD(wParam);
			offsetY = (short)HIWORD(wParam);
			g_rgnUpdate = (HRGN)lParam;
		return TRUE;

		case WM_PAINT:
		{
			dialogSkinner.Draw(hwndDlg, 0, 0);
		}
		return 0;

		case WM_ERASEBKGND:
		return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
	}
	return FALSE;
}

void history_cleanupifnecessary()
{
	if (!g_table) return;

	time_t now=time(NULL);

	// if we've done it in the last 8 hours, don't do it again!
	if (now < g_config->ReadInt(L"recent_limitlt",0) + 8*60*60) return;

	// time to cleanup 
	int limit_d=g_config->ReadInt(L"recent_limitd",1);
	int limit_dn=g_config->ReadInt(L"recent_limitnd",30);

	if (!limit_d || limit_dn < 1) return;

	g_config->WriteInt(L"recent_limitlt",(int)now);

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);
	wchar_t str[512] = {0};
	StringCchPrintfW(str,512,L"lastplay < [%d days ago]",limit_dn);
	NDE_Scanner_Query(s, str);
	NDE_Scanner_First(s);
	for (;;)
	{
		if (!NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_LASTPLAYED)) break;

		NDE_Scanner_Edit(s);
		NDE_Scanner_Delete(s);
		NDE_Scanner_Post(s);
		g_table_dirty++;
	}

	NDE_Table_DestroyScanner(g_table, s);
	if (g_table_dirty)
	{
		NDE_Table_Sync(g_table);
		g_table_dirty=0;
		NDE_Table_Compact(g_table);
	}
	LeaveCriticalSection(&g_db_cs);
}

void history_onFile(const wchar_t *fn, int offset)
{
	if (!fn || fn && !*fn) return;

	int isstream=!!wcsstr(fn,L"://");
	if (isstream)
	{
		if (g_config->ReadInt(L"recent_track",1)&2) return;
	}
	else 
	{
		if (!(g_config->ReadInt(L"recent_track",1)&1)) return;
	}

	if (!g_table && !openDb()) return;

	const wchar_t *filename = fn;

	int was_querying=0;
	if (history_bgThread_Handle)
	{
		history_bgQuery_Stop();
		was_querying=1;
	}
	KillTimer(m_hwnd,123);

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);

	wchar_t filename2[2048] = {0}; // full lfn path if set
	makeFilename2(filename,filename2,ARRAYSIZE(filename2));
	int found=0;

	if (filename2[0])
	{
		if (NDE_Scanner_LocateFilename(s, HISTORYVIEW_COL_FILENAME, FIRST_RECORD, filename2)) found = 2;
	}
	if (!found)
	{
		if (NDE_Scanner_LocateFilename(s, HISTORYVIEW_COL_FILENAME, FIRST_RECORD, filename)) found = 1;
	}

	int cnt=0;
	if (found)
	{
		NDE_Scanner_Edit(s);
		if (found == 1 && filename2[0]) db_setFieldString(s,HISTORYVIEW_COL_FILENAME,filename2); // if we have a better filename, update it
		nde_field_t f = NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_PLAYCOUNT);
		cnt = f?NDE_IntegerField_GetValue(f):0;
	}
	else
	{
		NDE_Scanner_New(s);
		db_setFieldString(s, HISTORYVIEW_COL_FILENAME, filename2[0] ? filename2 : filename);

		int plidx= (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETLISTPOS); 
		const wchar_t *ft=(const wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, plidx, IPC_GETPLAYLISTTITLEW);
		if (!ft || (INT_PTR)ft == 1) ft=fn;
		const wchar_t *ftp=ft;
		int length= (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 1, IPC_GETOUTPUTTIME);	
		  
		if (*ftp == '[' && (ftp=wcsstr(ftp,L"]")))
		{
			ftp++;
			while (ftp && *ftp == ' ') ftp++;
			if (ftp && !*ftp) ftp=ft;
		}
		else ftp=ft;
	
		db_setFieldInt(s, HISTORYVIEW_COL_LENGTH, length);
		db_setFieldString(s, HISTORYVIEW_COL_TITLE, ftp);
	}
	
	if (offset >= 0) 
	{
		db_setFieldInt(s, HISTORYVIEW_COL_OFFSET, (offset > 0 ? offset : -1));
	}
	else
	{
		db_setFieldInt(s, HISTORYVIEW_COL_LASTPLAYED, (int)time(NULL));
		db_setFieldInt(s, HISTORYVIEW_COL_PLAYCOUNT, cnt+1);
	}

	NDE_Scanner_Post(s);

	NDE_Table_DestroyScanner(g_table, s);
	NDE_Table_Sync(g_table);
	g_table_dirty++;

	// changed to save the history when updated to prevent it being
	// lost but retains the 8hr cleanup, etc which is otherwise run
	if (g_table_dirty > 100)
	{
		history_cleanupifnecessary();
	}
	if (g_table_dirty)
	{
		// and to keep existing behaviour for the dirty count, we
		// ensure that even on saving we maintain the dirty count
		closeDb(false);
		openDb();
	}
	LeaveCriticalSection(&g_db_cs);

	if (was_querying)
		history_bgQuery();

	if (IsWindow(m_hwnd))
	{
		m_lv_last_topidx=ListView_GetTopIndex(resultlist.getwnd());
		SendMessage(m_hwnd,WM_APP+1,0,0);
	}
}

int retrieve_offset(const wchar_t *fn)
{
	int offset = -1;
	
	int isstream=!!wcsstr(fn,L"://");
	if (isstream)
	{
		if (g_config->ReadInt(L"recent_track",1)&2) return offset;
	}
	else 
	{
		if (!(g_config->ReadInt(L"recent_track",1)&1)) return offset;
	}

	if (!g_table && !openDb()) return offset;

	const wchar_t *filename=fn;

	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);

	wchar_t filename2[2048] = {0}; // full lfn path if set
	makeFilename2(filename,filename2,ARRAYSIZE(filename2));
	int found=0;

	if (filename2[0])
	{
		if (NDE_Scanner_LocateFilename(s, HISTORYVIEW_COL_FILENAME,FIRST_RECORD,filename2)) found=2;
	}
	if (!found)
	{
		if (NDE_Scanner_LocateFilename(s, HISTORYVIEW_COL_FILENAME,FIRST_RECORD,filename)) found=1;
	}

	if (found)
	{
		nde_field_t f = NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_OFFSET);
		offset = f?NDE_IntegerField_GetValue(f):-1;
	}

	NDE_Table_DestroyScanner(g_table, s);

	LeaveCriticalSection(&g_db_cs);

	return offset;
}

void fileInfoDialogs(HWND hwndParent)
{
	history_bgQuery_Stop();
	int l=resultlist.GetCount(),i;
	int needref=0;
	for(i=0;i<l;i++)
	{
		if(!resultlist.GetSelected(i)) continue;
		historyRecord *song=(historyRecord *)itemCache.Items + i;
		if (!song->filename || !song->filename[0]) continue;

		infoBoxParamW p;
		p.filename=song->filename;
		p.parent=hwndParent;
		if (SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&p,IPC_INFOBOXW)) break;
		needref=1;

		EnterCriticalSection(&g_db_cs);
		nde_scanner_t s= NDE_Table_CreateScanner(g_table);
		if (NDE_Scanner_LocateNDEFilename(s, HISTORYVIEW_COL_FILENAME,FIRST_RECORD,song->filename)) 
		{
			wchar_t ft[1024]={0};
			basicFileInfoStructW bi = {0};
			bi.filename=p.filename;
			bi.length=-1;
			bi.title=ft;
			bi.titlelen=ARRAYSIZE(ft);
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&bi,IPC_GET_BASIC_FILE_INFOW);

			db_setFieldInt(s,HISTORYVIEW_COL_LENGTH,bi.length);
			db_setFieldString(s,HISTORYVIEW_COL_TITLE,ft);

			NDE_Scanner_Post(s);
		}
		NDE_Table_DestroyScanner(g_table, s);
		g_table_dirty++;
		LeaveCriticalSection(&g_db_cs);
	}

	if (g_table_dirty && g_table)
	{
		EnterCriticalSection(&g_db_cs);
		NDE_Table_Sync(g_table);
		g_table_dirty=0;
		LeaveCriticalSection(&g_db_cs);
	}

	if (needref)
	{
	    SendMessage(hwndParent,WM_TIMER,UPDATE_QUERY_TIMER_ID,0);
	}

	MSG msg;
	while(PeekMessage(&msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE)); //eat return
}