#include "main.h"
#include "cddb.h"
#include "../Winamp/wa_ipc.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "api__in_cdda.h"

#include <api/service/waservicefactory.h>
#include <shlwapi.h>
#include <atlbase.h>
#include "resource.h"
#include <commctrl.h>
#include <strsafe.h>

char config_use_cddb = 1;

#define MAX_CACHE_SIZE	8


typedef struct _CBDATA
{
	DINFO	*ps;
	CHAR	cLetter;
	DWORD	cddbid;
} CBDATA;

typedef struct _CACHEREC
{
	DINFO		discInfo;
	HRESULT		result;
} CACHEREC;

static CACHEREC cddb_cache[MAX_CACHE_SIZE];
static int cddb_cache_cursor = 0;

void DefaultValues(DINFO *ps)
{
	ps->Reset();
	ps->compilation = false;
	ps->discnum = 0;
	ps->numdiscs = 0;

	ps->populated = true;
}

static bool IsInternetAvailable()
{
	return !!SendMessage(line.hMainWindow, WM_WA_IPC, 0, IPC_INETAVAILABLE);
}

static DWORD failid[26] ={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define WRITEINFO(obj, field, name) { obj->get_ ## field(&str); fwprintf(fp, name L": %s\r\n", str.m_str); }
#define WRITEINFO_LONG(obj, field, name) { long val; obj->get_ ## field(&val); fwprintf(fp, name L": %d\r\n", val); }

#ifndef IGNORE_API_GRACENOTE
static void DumpInfoFile(ICddbDiscPtr disc)
{
#if 0 // keep this code, but we'll wait until a later version to implement more data
	long numTracks;
	FILE *fp = fopen("c:\\cdinfo.txt", "wt");
	//		wchar_t BOM[] = {0xFEFF, 0};
	//	fwrite(BOM, 2, 1, fp);
	CComBSTR str;

	WRITEINFO(disc, Toc, L"Disc TOC");
	WRITEINFO(disc, Artist, L"Disc Artist");
	WRITEINFO(disc, Title, L"Disc Title");
	WRITEINFO(disc, Label, L"Disc Label");
	WRITEINFO(disc, Year, L"Disc Year");
	WRITEINFO(disc, MediaId, L"Disc MediaId");
	WRITEINFO(disc, Notes, L"Disc Notes");
	WRITEINFO(disc, GenreId, L"Disc GenreId");
	WRITEINFO(disc, SecondaryGenreId, L"Disc Secondary GenreId");
	WRITEINFO(disc, RegionId, L"Disc RegionId");
	WRITEINFO(disc, Revision, L"Disc Revision");
	WRITEINFO(disc, TotalInSet, L"Disc Discs");
	WRITEINFO(disc, NumberInSet, L"Disc Disc #");
	WRITEINFO(disc, LanguageId, L"Disc Language Id");
	WRITEINFO(disc, RevisionTag, L"Disc Revision Tag");
	WRITEINFO_LONG(disc, Compilation, L"Disc Compilation");

	ICddbDisc2 *disc2;
	disc->QueryInterface(&disc2);
	WRITEINFO(disc2, ReleaseDate, L"Disc Release Date");
	WRITEINFO(disc2, RecordingDate, L"Disc Recording Date");
	WRITEINFO(disc2, ProductCode, L"Disc Product Code");
	WRITEINFO(disc2, YearComposed, L"Disc Year Composed");
	disc2->Release();

	ICddbCredits *credits;
	disc->get_Credits(&credits);
	long creditCount;
	credits->get_Count(&creditCount);
	for (long c = 0;c < creditCount;c++)
	{
		ICddbCredit *credit;
		credits->GetCredit(c + 1, &credit);
		WRITEINFO(credit, Id, L"Credit ID");
		WRITEINFO(credit, Name, L"Credit Name");
		WRITEINFO(credit, Notes, L"Credit Notes");
		credit->Release();
	}
	credits->Release();

	ICddbTracks *tracks;
	disc->get_Tracks(&tracks);
	tracks->get_Count(&numTracks);
	for (int i = 0;i < numTracks;i++)
	{
		ICddbTrack *track;
		tracks->GetTrack(i + 1, &track);
		fputws(L"-----------------\r\n", fp);
		WRITEINFO(track, Title, L"Track Title");
		WRITEINFO(track, Artist, L"Track Artist");
		WRITEINFO(track, Year, L"Track Year");
		WRITEINFO(track, Label, L"Track Label");
		WRITEINFO(track, Notes, L"Track Notes");
		WRITEINFO(track, GenreId, L"Track GenreId");
		WRITEINFO(track, SecondaryGenreId, L"Track SecondaryGenreId");
		WRITEINFO(track, Lyrics, L"Track Lyrics");
		WRITEINFO(track, BeatsPerMinute, L"Track BPM");
		WRITEINFO(track, ISRC, L"Track ISRC");

		ICddbTrack2 *track2;
		track->QueryInterface(&track2);
		WRITEINFO(track2, ReleaseDate, L"Release Date");
		WRITEINFO(track2, RecordingDate, L"Recording Date");
		WRITEINFO(track2, YearComposed, L"Year Composed");
		track2->Release();

		ICddbCredits *credits;
		track->get_Credits(&credits);
		if (credits)
		{
			long creditCount;
			credits->get_Count(&creditCount);
			for (long c = 0;c < creditCount;c++)
			{
				ICddbCredit *credit;
				credits->GetCredit(c + 1, &credit);
				WRITEINFO(credit, Id, L"Credit ID");
				WRITEINFO(credit, Name, L"Credit Name");
				WRITEINFO(credit, Notes, L"Credit Notes");
				credit->Release();
			}
			credits->Release();
		}
		track->Release();
	}

	tracks->Release();
	fclose(fp);
#endif
}
#endif

#ifdef IGNORE_API_GRACENOTE
static HRESULT CALLBACK Cddb_ResultCallback(HRESULT result, ICddbDisc *pDisc, DWORD *pdwAutoCloseDelay, ULONG_PTR user)
{
	int index;
	CACHEREC *pRec;
	CBDATA *pData = (CBDATA*)user;
	if (!pData) return E_POINTER;
	DINFO *pDI = NULL;

	for (index = 0, pRec = &cddb_cache[0]; index < MAX_CACHE_SIZE && 0 != pRec->discInfo.CDDBID; pRec++, index++)
	{
		if (pRec->discInfo.CDDBID == pData->cddbid)
		{
			pDI = &pRec->discInfo;

			break;
		}
	}
	if (!pDI) return S_OK;

	if (S_OK == result)
	{
		#ifndef IGNORE_API_GRACENOTE
		if (pDisc)
		{
			DumpInfoFile(pDisc);
			GetDiscInfo(pDisc, pDI);
			StoreDisc(pDI->CDDBID, pDisc);
			ICddbCacheManager* pCache;
			HRESULT hr = Cddb_GetICacheManger((void**)&pCache);
			if (SUCCEEDED(hr))
			{
				CComBSTR toc;
				pDisc->get_Toc(&toc);
				pCache->StoreDiscByToc(toc, pDisc);
				pCache->Release();
			}
			pDI->populated = true;
		}
		#endif
	}
	else
	{
		#ifndef IGNORE_API_GRACENOTE
		if (DoCDText(pDI, pData->cLetter))
		{
			StoreCDText(pDI->CDDBID, pData->cLetter);
			result = S_OK;
		}
		#endif
	}

	CddbCache_SetDisc(pDI, result);

	if (pData->ps->CDDBID == pDI->CDDBID)
	{
		*pData->ps = *pDI;
	}
	return S_OK;
}
#endif

void InitializeCddbCache(void)
{
	ZeroMemory(cddb_cache, sizeof(CACHEREC)*MAX_CACHE_SIZE);
	cddb_cache_cursor = 0;
}

void UninitializeCddbCache(void)
{
	ZeroMemory(cddb_cache, sizeof(CACHEREC)*MAX_CACHE_SIZE);
	cddb_cache_cursor = 0;
}

HRESULT CddbCache_SetDisc(DINFO *pDiscInfo,  HRESULT lookupResult)
{
	int index;
	CACHEREC *pRec;
	if (!pDiscInfo || 0 == pDiscInfo->CDDBID) return E_FAIL;

	for (index = 0, pRec = &cddb_cache[0]; index < MAX_CACHE_SIZE && 0 != pRec->discInfo.CDDBID; pRec++, index++)
	{
		if (pRec->discInfo.CDDBID == pDiscInfo->CDDBID) break;
	}
	if (0 == pRec->discInfo.CDDBID || MAX_CACHE_SIZE == index)
	{
		pRec = &cddb_cache[cddb_cache_cursor];
		cddb_cache_cursor++;
		if (cddb_cache_cursor >= MAX_CACHE_SIZE) cddb_cache_cursor = 0;
	}

	if (pRec)
	{
		pRec->result = lookupResult;
		pRec->discInfo = *pDiscInfo;
	}

	return S_OK;
}

int GetCDDBInfo(DINFO *ps, wchar_t device)
{
	int index;
	CACHEREC *pRec;
	//HRESULT hr;

	//wchar_t szTOC[2048] = {0};

	if (ps->populated) return 0;

	/* first, look in our memory cache */
	for (index = 0, pRec = &cddb_cache[0]; index < MAX_CACHE_SIZE && 0 != pRec->discInfo.CDDBID; pRec++, index++)
	{
		if (pRec->discInfo.CDDBID == ps->CDDBID)
		{
			if (S_OK == pRec->result)
				*ps = pRec->discInfo;
			else
				DefaultValues(ps);
			return 0; // no need to notify about changes

		}
	}

	/* look it up in the local database */
	if (QueryDINFO(ps->CDDBID, ps))
	{
		CddbCache_SetDisc(ps, S_OK);
		return 0;
	}

	/* check CDDB & CD-Text */
	/*CddbCache_SetDisc(ps, E_PENDING);
	if (Cddb_CalculateTOC(ps, szTOC, sizeof(szTOC)/sizeof(wchar_t)))
	{
		ULONG flags;
		CBDATA data;
		data.ps = ps;
		data.cLetter = device;
		data.cddbid = ps->CDDBID;
		flags = CDDB_UI_MODAL | CDDB_UI_MULTIPLE | CDDB_UI_NOMATCH;
		if (!config_use_cddb || !IsInternetAvailable()) flags |= CDDB_NOINET;
		hr = Cddb_DoLookup(szTOC, line.hMainWindow, Cddb_ResultCallback, flags, (ULONG_PTR)&data);
		if (FAILED(hr)) Cddb_DisplayResultDlg(line.hMainWindow, hr, AUTOCLOSE_NEVER, flags);
	}
	else hr = CDDB_E_BADTOC;
	if (FAILED(hr)) CddbCache_SetDisc(ps, hr);*/
	return 1;//SUCCEEDED(hr);
}