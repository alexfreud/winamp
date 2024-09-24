#include "main.h"
#include "../replicant/nu/AutoChar.h"
#include "../replicant/nu/ns_wc.h"
#include <shlwapi.h>
#include <malloc.h> // for alloca

static wchar_t m_lastfn[2048];
static wchar_t m_lastartist[256], m_lasttitle[256], m_lastalbum[256], m_gracenotefileid[128];
static int m_lasttrack;
static int m_playcount;
static int m_ispodcast;
static int m_rating;
static int m_db_found;

void ClearTitleHookCache()
{
	memset(m_lastfn, 0, sizeof(m_lastfn));
}

BOOL IPC_HookExtInfoW(INT_PTR param)
{
	if (skipTitleInfo) // we're reading metadata and being asked to skip title hooking (so we can get a valid title for guessing, if need be)
		return false;

	extendedFileInfoStructW *p = (extendedFileInfoStructW *)param;
	// fill in our own titles from db? not for now, just let it default to hitting the tags?

	if (!g_config->ReadInt(L"newtitle", 1))
		return FALSE;

	if (NULL == p->filename || L'\0' == *p->filename ||
		NULL == p->metadata || L'\0' == *p->metadata)
	{
		return FALSE;
	}

	int which = 0;
	if (!_wcsicmp(p->metadata, DB_FIELDNAME_artist ))	which = 1;
	else if (!_wcsicmp(p->metadata, DB_FIELDNAME_title ))	which = 2;
	else if (!_wcsicmp(p->metadata, DB_FIELDNAME_album ))	which = 3;
	//else if (!_wcsicmp(p->metadata, L"tuid")) which = 4;
	else if (!_wcsicmp(p->metadata, DB_FIELDNAME_track ))	which = 5;
	else if (!_wcsicmp(p->metadata, DB_FIELDNAME_rating )) which = 6;
	else if (!_wcsicmp(p->metadata, DB_FIELDNAME_playcount )) which = 7;
	else if (!_wcsicmp(p->metadata, DB_FIELDNAME_GracenoteFileID )) which=8;
	else if (!_wcsicmp(p->metadata, DB_FIELDNAME_ispodcast )) which=9;

	if (which)
	{
		if (_wcsicmp(m_lastfn, p->filename))
		{
			if (!g_table) openDb();
			if (!g_table) return 0;

			m_lastartist[0] = 0;
			m_lasttitle[0] = 0;
			m_lastalbum[0] = 0;
			m_gracenotefileid[0]=0;
			m_lasttrack = 0;
			m_rating=-1;
			m_playcount=-1;
			m_ispodcast=-1;
			lstrcpynW(m_lastfn, p->filename, sizeof(m_lastfn)/sizeof(wchar_t));

			wchar_t filename2[MAX_PATH] = {0}; // full lfn path if set

			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);
			int found = FindFileInDatabase(s, MAINTABLE_ID_FILENAME, m_lastfn, filename2);

			if (found == 2)
				lstrcpynW(m_lastfn, filename2, sizeof(m_lastfn)/sizeof(m_lastfn));

			if (found)
			{
				nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_ARTIST);
				if (f) lstrcpynW(m_lastartist, NDE_StringField_GetString(f), sizeof(m_lastartist)/sizeof(wchar_t));
				f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_ALBUM);
				if (f) lstrcpynW(m_lastalbum, NDE_StringField_GetString(f), sizeof(m_lastalbum)/sizeof(wchar_t));
				f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_TITLE);
				if (f) lstrcpynW(m_lasttitle, NDE_StringField_GetString(f), sizeof(m_lasttitle)/sizeof(wchar_t));
				f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_TRACKNB);
				if (f) m_lasttrack = NDE_IntegerField_GetValue(f);
				f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_PLAYCOUNT);
				if (f) m_playcount = NDE_IntegerField_GetValue(f);
				f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_RATING);
				if (f) m_rating = NDE_IntegerField_GetValue(f);
				f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_ISPODCAST);
				if (f) m_ispodcast = NDE_IntegerField_GetValue(f);
				f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_GRACENOTEFILEID);
				if (f) lstrcpynW(m_gracenotefileid, NDE_StringField_GetString(f), sizeof(m_gracenotefileid)/sizeof(wchar_t));
				m_db_found = 1;
			}
			else m_db_found = 0;

			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);
		}
		if (m_db_found == 1)
		{
			switch(which)
			{
				case 1:
				if (m_lastartist[0])
				{
					lstrcpynW(p->ret, m_lastartist, p->retlen);
					return 1;
				}
				break;
				case 2:
					if (m_lasttitle[0])
				{
					lstrcpynW(p->ret, m_lasttitle, p->retlen);
					return 1;
				}
				break;
				case 3:
					if (m_lastalbum[0])
				{
					lstrcpynW(p->ret, m_lastalbum, p->retlen);
					return 1;
				}
				break;
				case 5:
				if (m_lasttrack && m_lasttrack != -1)
				{
					_snwprintf(p->ret, p->retlen, L"%d", m_lasttrack);
					return 1;
				}
				break;
				case 6:
				if (m_rating != -1)
				{
					_snwprintf(p->ret, p->retlen, L"%d", m_rating);
					return 1;
				}
				break;
				case 7:
				if (m_playcount != -1)
				{
					_snwprintf(p->ret, p->retlen, L"%d", m_playcount);
					return 1;
				}
				break;
				case 8:
				if (m_gracenotefileid[0])
				{
					lstrcpynW(p->ret, m_gracenotefileid, p->retlen);
					return 1;
				}
				break;
				case 9:
				if (m_ispodcast != -1)
				{
					_snwprintf(p->ret, p->retlen, L"%d", m_ispodcast);
					return 1;
				}
				break;
			}
		}
	}
	return FALSE;
}

BOOL IPC_HookExtInfo(INT_PTR param)
{
	extendedFileInfoStruct *p = (extendedFileInfoStruct*)param;
	// fill in our own titles from db? not for now, just let it default to hitting the tags?
	if (!g_config->ReadInt(L"newtitle", 1))
		return FALSE;

	if (NULL == p->filename || '\0' == *p->filename ||
		NULL == p->metadata || '\0' == *p->metadata)
	{
		return FALSE;
	}

	extendedFileInfoStructW pW = {0};
	AutoWide wideFn(p->filename), wideMetadata(p->metadata);
	pW.filename = wideFn;
	pW.metadata = wideMetadata;
	pW.retlen = p->retlen;
	pW.ret = (wchar_t *)alloca(pW.retlen * sizeof(wchar_t));

	if (IPC_HookExtInfoW((INT_PTR)&pW))
	{
		WideCharToMultiByteSZ(CP_ACP, 0, pW.ret, -1, p->ret, p->retlen, 0, 0);
		return 1;
	}
	return 0;

}

BOOL IPC_HookTitleInfo(INT_PTR param)
{
	if (skipTitleInfo) // we're reading metadata and being asked to skip title hooking (so we can get a valid title for guessing, if need be)
		return false;

	waHookTitleStructW *hts = (waHookTitleStructW*)param;

	if (NULL != hts->filename && 
		!StrStrW(hts->filename, L"://") && 
		g_config->ReadInt(L"newtitle", 1))
	{
		if (!g_table) openDb();
		if (!g_table) return 0;
		wchar_t filename2[MAX_PATH] = {0}; // full lfn path if set
		EnterCriticalSection(&g_db_cs);

		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		int found=FindFileInDatabase(s, MAINTABLE_ID_FILENAME, hts->filename, filename2);

		if (found)
		{
			nde_field_t length = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_LENGTH);
			int l = -1;
			if (length)
				l = NDE_IntegerField_GetValue(length);
			if (l > 0)
				hts->length = l;
			else hts->length = -1;

			if (hts->title)
			{
				TAG_FMT_EXT(hts->filename, fieldTagFunc, ndeTagFuncFree, (void*)s, hts->title, 2048, 1);
			}
		}

		NDE_Table_DestroyScanner(g_table, s);
		LeaveCriticalSection(&g_db_cs);

		if (found) return 1;
	} // not http://
	return FALSE;
}

DWORD doGuessProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// call the old wndproc, and if it produces empty results or no results, try to guess
	extendedFileInfoStruct *p = (extendedFileInfoStruct*)wParam;

	LRESULT ret = CallWindowProc(wa_oldWndProc, hwndDlg, uMsg, wParam, lParam);
	if (NULL != p && 
		(!ret || !p->ret[0]) && 
		g_config->ReadInt(L"guessmode", 0) != 2)
	{
		int which = 0;

		if (NULL != p->metadata)
		{
			if (!_stricmp(p->metadata, "artist"))	which = 1;
			else if (!_stricmp(p->metadata, "title"))	which = 2;
			else if (!_stricmp(p->metadata, "album"))	which = 3;
			else if (!_stricmp(p->metadata, "track"))	which = 5;
			else which = 0;
		}
		else
			which = 0;

		if (which)
		{
			static char m_lastfn[2048];
			if (NULL != p->filename && _strnicmp(m_lastfn, p->filename, sizeof(m_lastfn)))
			{
				m_db_found = 2;
				m_lastartist[0] = 0;
				m_lasttitle[0] = 0;
				m_lastalbum[0] = 0;
				m_lasttrack = 0;
				StringCbCopyA(m_lastfn, sizeof(m_lastfn), p->filename);

				int tn = 0;
				wchar_t *artist = 0, *album = 0, *title = 0;
				wchar_t *guessbuf = guessTitles(AutoWide(m_lastfn), &tn, &artist, &album, &title);

				if (guessbuf)
				{
					if (artist) StringCbCopyW(m_lastartist, sizeof(m_lastartist), artist);
					if (album) StringCbCopyW(m_lastalbum, sizeof(m_lastalbum), album);
					if (title) StringCbCopyW(m_lasttitle, sizeof(m_lasttitle), title); 
					m_lasttrack = tn;
					free(guessbuf);
				}
			}
			if (m_db_found == 2)
			{
				if (which == 1 && m_lastartist[0])
				{
					WideCharToMultiByteSZ(CP_ACP, 0, m_lastartist, -1, p->ret, p->retlen, 0, 0);
					return 1;
				}
				if (which == 2 && m_lasttitle[0])
				{
					WideCharToMultiByteSZ(CP_ACP, 0, m_lasttitle, -1, p->ret, p->retlen, 0, 0);
					return 1;
				}
				if (which == 3 && m_lastalbum[0])
				{
					WideCharToMultiByteSZ(CP_ACP, 0, m_lastalbum, -1, p->ret, p->retlen, 0, 0);
					return 1;
				}
				if (which == 5 && m_lasttrack)
				{
					_snprintf(p->ret, p->retlen, "%d", m_lasttrack);
					return 1;
				}
				if (which == 6 && m_rating != -1)
				{
					_snprintf(p->ret, p->retlen, "%d", m_rating);
					return 1;
				}
				if (which == 7 && m_playcount != -1)
				{
					_snprintf(p->ret, p->retlen, "%d", m_playcount);
					return 1;
				}
			}
		}
	}
	return (int)ret;
}