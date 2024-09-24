#include "main.h"
#include "api__in_cdda.h"

#include "cddbinterface.h"
#include "cddb.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../winamp/wa_ipc.h"
#include "../nu/ns_wc.h"
#include ".\cddbevnt.h"
#include ".\cddbui.h"
#include ".\grabwnd.h"
#include <api/application/api_application.h>
#include <atlbase.h>
#include "../nde/ndestring.h"
#include "../Winamp/buildtype.h"
#include <commctrl.h>
#include <strsafe.h>

// {C0A565DC-0CFE-405a-A27C-468B0C8A3A5C}
static const GUID internetConfigGroupGUID =
  { 0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c } };

TRACKINFO::~TRACKINFO()
{
	Reset();
}

void TRACKINFO::Reset()
{
ndestring_release(artist);
		artist=0;
		ndestring_release(title);
		title=0;
		ndestring_release(tagID);
		tagID=0;
		ndestring_release(composer);
		composer=0;
		ndestring_release(conductor);
		conductor=0;
		ndestring_release(extData);
		extData=0;
		ndestring_release(remixing);
		remixing=0;
		ndestring_release(isrc);
		isrc=0;
}

TRACKINFO::TRACKINFO(const TRACKINFO &copy)
{
	operator =(copy);
}

TRACKINFO &TRACKINFO::operator =(const TRACKINFO &copy)
{
	ndestring_retain(artist=copy.artist);
	ndestring_retain(title=copy.title);
	ndestring_retain(tagID=copy.tagID);
	ndestring_retain(composer=copy.composer);
	ndestring_retain(conductor=copy.conductor);
	ndestring_retain(extData=copy.extData);
	ndestring_retain(remixing=copy.remixing);
	ndestring_retain(isrc=copy.isrc);
	return *this;
}

void DINFO::Reset()
{
	ndestring_release(title);
	title=0;
	ndestring_release(artist);
	artist=0;
	ndestring_release(tuid);
	tuid=0;
	ndestring_release(year);
	year=0;
	ndestring_release(genre);
	genre=0;
	ndestring_release(label);
	label=0;
	ndestring_release(notes);
	notes=0;
	ndestring_release(conductor);
	conductor=0;
	ndestring_release(composer);
	composer=0;
	ndestring_release(remixing);
	remixing=0;
	discnum=0;
	numdiscs=0;	
}

DINFO::~DINFO()
{
	Reset();
}

DINFO::DINFO(const DINFO &copy)
{
	operator =(copy);
}

DINFO &DINFO::operator =(const DINFO &copy)
{
	ndestring_retain(title=copy.title);
	ndestring_retain(artist=copy.artist);
	ndestring_retain(tuid=copy.tuid);
	ndestring_retain(year=copy.year);
	ndestring_retain(genre=copy.genre);
	ndestring_retain(label=copy.label);
	ndestring_retain(notes=copy.notes);
	compilation=copy.compilation;
	discnum=copy.discnum;
	numdiscs=copy.numdiscs;
	ntracks=copy.ntracks;
	ndestring_retain(conductor=copy.conductor);
	ndestring_retain(composer=copy.composer);
	ndestring_retain(remixing=copy.remixing);
	for (int i=0;i<sizeof(tracks)/sizeof(*tracks);i++)
		tracks[i]=copy.tracks[i];
	
	CDDBID=copy.CDDBID;
	memcpy(pnFrames, copy.pnFrames, sizeof(pnFrames));
	nDiscLength=copy.nDiscLength;

	populated=copy.populated;
	return *this;
}


#define TM_INVOKERESULTS			(WM_APP + 222)
#define TM_UNINITTHREAD			(WM_APP + 2)

#define CENTER_PARENT			(-32000)
#define CENTER_LEFT			0x0001
#define CENTER_TOP			0x0002

#define WASABI_WND_CLASSW   L"BaseWindow_RootWnd"

#define INVARIANT_LCID		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

typedef struct _MEDIALOOKUP
{
	HRESULT		result;
	HWND			hwndInfo;
	CDDB_CB		callback;
	ULONG_PTR	user;
	UINT		flags;
	BSTR		bstrTOC;
	DWORD		threadId;
} MEDIALOOKUP;

typedef struct _THREADDATA
{
	LONG	ref;
	HHOOK	hhook;
	BOOL	uninitCom;
} THREADDATA;

CRITICAL_SECTION lockThread;

typedef struct _INVOKEDATA
{
	MEDIALOOKUP lookup;
	ICddbDisc	*pDisc;
	HANDLE		evntDone;
	DWORD		*pdwAutoCloseDelay;
} INVOKEDATA;

static DWORD tlsSlot = TLS_OUT_OF_INDEXES;
#ifndef IGNORE_API_GRACENOTE
ICDDBControl *pCDDBControl=0;
static CDBBEventManager eventMngr;
#endif
static MEDIALOOKUP g_lookup = {0, 0, };
static HANDLE evntBusy = NULL;
static LONG evntBusyRef = 0;
static HWND hwndProgressListener = NULL;
static ICddbDisc *pSubmitDisc = NULL;
static POINT g_lastcddbpos = { CENTER_PARENT, CENTER_PARENT}; // center


static HRESULT SetupCDDB(BOOL resetError);
static void CALLBACK Cddb_OnCommandCompleted(LONG lCommandCode, HRESULT hCommandResult, VARIANT *pCommandData, UINT_PTR user);
static void CALLBACK Cddb_OnCommandProgress(LONG lCommandCode, LONG lProgressCode, LONG lBytesDone, LONG lBytesTotal, UINT_PTR user);
#ifndef IGNORE_API_GRACENOTE
static void Cddb_OnMediaLookupCompleted(HRESULT hr, CDDBMatchCode matchCode, MEDIALOOKUP *pLookup);
static void Cddb_OnGetFullDiscInfoCompleted(HRESULT hr, ICddbDisc *pDisc, MEDIALOOKUP *pLookup);
static void Cddb_OnSubmitDiscCompleted(HRESULT result, MEDIALOOKUP *pLookup);

static HRESULT Cddb_DoFuzzyMatchDlg(HWND hwndCaller, UINT flags, ICddbDiscs *pDiscs, LONG *plVal);
static HRESULT Cddb_DoSubmitNewDlg(HWND hwndCaller, LPCWSTR pszTOC, UINT flags, ICddbDisc **ppDisc);

#define CLEARLOOKUP()	SecureZeroMemory(&g_lookup, sizeof(MEDIALOOKUP))
#define IS_BUSY(__timeout) (evntBusy && (WAIT_OBJECT_0 == WaitForSingleObject(evntBusy, (__timeout))))
#define SET_BUSY(__enable) ((__enable) ? (SetEvent(evntBusy)) : ResetEvent(evntBusy))

void ShutDownCDDB()
{
	if (pCDDBControl)
	{
		eventMngr.Unadvise(pCDDBControl);
		pCDDBControl->Release();
		pCDDBControl=0;
	}
}
#endif

static HWND GetAdaptedParent(HWND hwndParent)
{
	if (!hwndParent || !IsWindow(hwndParent)) hwndParent = line.hMainWindow;
	if (!hwndParent || !IsWindow(hwndParent)) hwndParent = GetDesktopWindow();
	if (hwndParent == line.hMainWindow) 
	{
		hwndParent = (HWND)SendMessageW(line.hMainWindow, WM_WA_IPC, 0, IPC_GETDIALOGBOXPARENT);
		if (!IsWindow(hwndParent))  hwndParent = line.hMainWindow;
	}
	return hwndParent;
}

static BOOL CalculatePopUpPos(RECT *prcParent, RECT *prcPopUp, UINT flags) 
{
	LONG x, y;
	MONITORINFO mi;
	if (!prcPopUp) return FALSE;

	if (prcParent)
	{
		x = prcParent->left;
		y = prcParent->top;

		OffsetRect(prcPopUp, -prcPopUp->left, -prcPopUp->top);
		OffsetRect(prcParent, -prcParent->left, -prcParent->top);
		if (CENTER_LEFT & flags) x += (prcParent->right - prcPopUp->right)/2;
		if (CENTER_TOP & flags) y += (prcParent->bottom - prcPopUp->bottom )/2;
		
		SetRect(prcPopUp, x, y, x + prcPopUp->right, y + prcPopUp->bottom);
	}
	mi.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor = MonitorFromRect(prcPopUp, MONITOR_DEFAULTTONULL);
	if (!hMonitor)
	{
		OffsetRect(prcPopUp, -prcPopUp->left, -prcPopUp->top);
		hMonitor = MonitorFromRect(prcPopUp, MONITOR_DEFAULTTONEAREST);

		if(GetMonitorInfo(hMonitor, &mi))
		{
			x = mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - prcPopUp->right)/2;
			if(x < mi.rcWork.left) x = mi.rcWork.left;
			y = mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) - prcPopUp->bottom)/2;
			if(y < mi.rcWork.top) y = mi.rcWork.top;
			SetRect(prcPopUp, x, y, x + prcPopUp->right, y + prcPopUp->bottom);
		}
	}
	else
	{
		if(GetMonitorInfo(hMonitor, &mi))
		{
			if (prcPopUp->right > mi.rcWork.right)
			{
				OffsetRect(prcPopUp, mi.rcWork.right - prcPopUp->right, 0);
			}
		}
	}
	return TRUE;
}

static BOOL SetPopUpPos(HWND hwnd, UINT flags)
{
	RECT rc, rw;

	if (!hwnd) return FALSE;

	HWND hwndParent = GetParent(hwnd);
	hwndParent = GetAdaptedParent(hwndParent);

	if (GetClientRect(hwndParent, &rc) && GetWindowRect(hwnd, &rw))
	{
		HWND hwndFound;
		wchar_t szClass[2048] = {0};
		UINT flags = CENTER_LEFT;

		MapWindowPoints(hwndParent, HWND_DESKTOP, (POINT*)&rc, 2);

		if (hwndParent == line.hMainWindow ||
				(GetClassNameW(hwndParent, szClass, sizeof(szClass)/sizeof(wchar_t)) &&
				CSTR_EQUAL == CompareStringW(INVARIANT_LCID, 0, szClass, -1, WASABI_WND_CLASSW, -1) &&
				NULL != (hwndFound = FindWindowEx(NULL, hwndParent, WASABI_WND_CLASSW, NULL)))
				&& IsWindowVisible(hwndFound))
		{ 
			OffsetRect(&rc, (rc.right - rc.left + 4), 0); 
			flags &= ~ CENTER_LEFT; 
		}
		if (CalculatePopUpPos(&rc, &rw, flags))
		{
			SetWindowPos(hwnd, HWND_TOP, rw.left, rw.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}
	return TRUE;
}

static void Cddb_ProcessResult(MEDIALOOKUP *pLookup, ICddbDisc *pDisc, DWORD *pdwAutoCloseDelay)
{
	if (pLookup && pLookup->callback) pLookup->callback(pLookup->result, pDisc, pdwAutoCloseDelay, pLookup->user);
}

static void CALLBACK Execute_ProcessResult(INVOKEDATA *pData)
{
	#ifndef IGNORE_API_GRACENOTE
	INVOKEDATA data;

	if (!pData) return;

	CopyMemory(&data, &pData->lookup, sizeof(INVOKEDATA));
	if (data.pDisc) 
		data.pDisc->AddRef();

	Cddb_ProcessResult(&data.lookup, data.pDisc, data.pdwAutoCloseDelay);
	if (data.pDisc) 
		data.pDisc->Release();
	if (data.evntDone) 
		SetEvent(data.evntDone);
	#endif
}

static LRESULT CALLBACK HookMessageProc(int code, WPARAM wParam, LPARAM lParam)
{
	THREADDATA *pData;
	
	if (TLS_OUT_OF_INDEXES == tlsSlot) return 0;
	pData = (THREADDATA*)TlsGetValue(tlsSlot);
	if (!pData) return 0;

	if (code < 0) return CallNextHookEx(pData->hhook, code, wParam, lParam);

	if (NULL == ((MSG*)lParam)->hwnd) // thread message
	{
		switch(((MSG*)lParam)->message)
		{
			case TM_INVOKERESULTS:
				Execute_ProcessResult((INVOKEDATA*)((MSG*)lParam)->lParam);
				return TRUE;
			case TM_UNINITTHREAD:
				Cddb_UninitializeThread();
				return TRUE;
		}
	}

	return CallNextHookEx(pData->hhook, code, wParam, lParam);
}

void Cddb_Initialize(void)
{
	InitializeCriticalSection(&lockThread);
}

void Cddb_Uninitialize(void)
{
	DeleteCriticalSection(&lockThread);
}

static HRESULT Cddb_IsThreadInitialized(void)
{
	THREADDATA *pData;
	if (TLS_OUT_OF_INDEXES == tlsSlot) return E_OUTOFMEMORY;
	pData = (THREADDATA*)TlsGetValue(tlsSlot);
	return (pData) ? S_OK : S_FALSE;
}

static HANDLE GetMainThreadHandle(void)
{
	HANDLE hThread = NULL;
	api_application *pApp = NULL;
	waServiceFactory *sf = NULL;

	if (!line.service) return NULL;
	if (NULL == (sf = line.service->service_getServiceByGuid(applicationApiServiceGuid))) return NULL;
	if (NULL == (pApp = (api_application*)sf->getInterface())) {  return NULL; }

	hThread = pApp->main_getMainThreadHandle();
	sf->releaseInterface(pApp);

	return hThread;
}

static HRESULT Cddb_InvokeProcessResult(MEDIALOOKUP *pLookup, ICddbDisc *pDisc, DWORD *pdwAutoCloseDelay)
{
	HRESULT hr = E_FAIL;
	#ifndef IGNORE_API_GRACENOTE
	INVOKEDATA data;
	if (!pLookup) return E_INVALIDARG;
	CopyMemory(&data.lookup, pLookup, sizeof(MEDIALOOKUP));
	data.pDisc = pDisc;
	data.pdwAutoCloseDelay = pdwAutoCloseDelay;
	if (NULL != pDisc)
		pDisc->AddRef();

	if (!data.lookup.threadId) data.lookup.threadId = GetWindowThreadProcessId(line.hMainWindow, NULL);
	if (data.lookup.threadId != GetCurrentThreadId())
	{
		data.evntDone = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (data.evntDone)
		{
			if (PostThreadMessage(data.lookup.threadId, TM_INVOKERESULTS, 0, (LPARAM)&data))
			{
				MSG msg;
				msg.message = NULL;

				for(;;)
				{
					DWORD status = MsgWaitForMultipleObjectsEx(1, &data.evntDone, 
								INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);

					if (status == WAIT_OBJECT_0+1)
					{
						while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
						{
							if (WM_QUIT == msg.message) 
							{
								break;
							}
							if (0 == CallMsgFilter(&msg, 0)) 
							{
								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
						}
					} 
					else if (status == WAIT_OBJECT_0) 	
					{ 
						break; 
					}
				}

				if (WM_QUIT == msg.message) 
					PostQuitMessage((int)msg.wParam);
				hr = S_OK;
			}
			CloseHandle(data.evntDone);
		}
	}
	else
	{
		Cddb_ProcessResult(pLookup, pDisc, pdwAutoCloseDelay);
		hr = S_OK;
	}

	if (NULL != pDisc)
		pDisc->Release();
	#endif
	return hr;
}

#ifndef IGNORE_API_GRACENOTE
bool GetRole(ICddbTrack *track, BSTR roleId, BSTR *str)
{
	if (!roleId || !*roleId)
		return false;

	if (!track)
		return false;

	ICddbCreditsPtr credits;
	track->get_Credits(&credits);
	if (credits)
	{
		long creditCount;
		credits->get_Count(&creditCount);
		for (long c = 0;c < creditCount;c++)
		{
			ICddbCreditPtr credit;
			credits->GetCredit(c + 1, &credit);
			if (credit)
			{
				BSTR thisRole;
				credit->get_Id(&thisRole);
				if (!wcscmp(thisRole, roleId))
				{
					credit->get_Name(str);
					return true;
				}
			}
		}
	}
	return false;
}

bool GetRole(ICddbDisc *track, BSTR roleId, BSTR *str)
{
	if (!roleId || !*roleId)
		return false;

	if (!track)
		return false;

	ICddbCreditsPtr credits;
	track->get_Credits(&credits);
	if (credits)
	{
		long creditCount;
		credits->get_Count(&creditCount);
		for (long c = 0;c < creditCount;c++)
		{
			ICddbCreditPtr credit;
			credits->GetCredit(c + 1, &credit);
			if (credit)
			{
				BSTR thisRole;
				credit->get_Id(&thisRole);
				if (!wcscmp(thisRole, roleId))
				{
					credit->get_Name(str);
					return true;
				}
			}
		}
	}
	return false;
}
#endif

HRESULT Cddb_InitializeThread(void)
{
	HRESULT hr = S_OK;
	EnterCriticalSection(&lockThread);

	if (!evntBusy) 
	{
		evntBusyRef = 0;
		evntBusy = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == evntBusy)
			hr = E_UNEXPECTED;
	}

	if (SUCCEEDED(hr)) 
	{
		evntBusyRef++;

		if (TLS_OUT_OF_INDEXES == tlsSlot &&
			TLS_OUT_OF_INDEXES == (tlsSlot = TlsAlloc())) 
			hr = E_OUTOFMEMORY;

		if (SUCCEEDED(hr))
		{
			THREADDATA *pData = (THREADDATA*)TlsGetValue(tlsSlot);
			if (pData) 
			{
				pData->ref++; 
			}
			else
			{
				pData = (THREADDATA*)calloc(1, sizeof(THREADDATA));
				if (!pData)
				{
					hr = E_OUTOFMEMORY;
				}
				if (SUCCEEDED(hr))
				{
					pData->hhook = SetWindowsHookExW(WH_MSGFILTER, HookMessageProc, NULL, GetCurrentThreadId());
					if (!pData->hhook || !TlsSetValue(tlsSlot, pData))
					{
						if (pData->hhook) UnhookWindowsHookEx(pData->hhook);
						free(pData);
						pData = NULL;
						hr = E_FAIL;
					}
					else 
					{
						pData->ref = 1;
						pData->uninitCom = (S_OK == CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
						hr = S_OK;
					}
				}
			}
		}
	}
	LeaveCriticalSection(&lockThread);
	return hr;
}

HRESULT Cddb_UninitializeThread(void)
{
	EnterCriticalSection(&lockThread);

	if (TLS_OUT_OF_INDEXES != tlsSlot)
	{
		THREADDATA *pData = (THREADDATA*)TlsGetValue(tlsSlot);
		if (NULL != pData &&
			pData->ref && 0 == --pData->ref)
		{
			TlsSetValue(tlsSlot, NULL);
			if (pData->hhook) 
				UnhookWindowsHookEx(pData->hhook);
			
			if (pData->uninitCom)
				CoUninitialize();

			free(pData);
		}
	}

	if (evntBusyRef && 0 == --evntBusyRef) 
	{
		CloseHandle(evntBusy);
		evntBusy = NULL;
	}
	
	LeaveCriticalSection(&lockThread);
	return S_OK;
}

LPCWSTR Cddb_CalculateTOC(DINFO *pDisc, LPWSTR pszTOC, size_t cchTOC)
{
	LPWSTR p;
    if (!pszTOC || !pDisc || !cchTOC) return NULL;
	pszTOC[0] = 0x00;
	p = pszTOC;
	for (int x = 0; x < pDisc->ntracks + 1; x++)
	{
		if (S_OK != StringCchPrintfExW(p, cchTOC, &p, &cchTOC, 
						STRSAFE_NULL_ON_FAILURE, L"%lu ", pDisc->pnFrames[x])) 	return NULL;
	}
	if (p != pszTOC) *(p - 1) = 0x00;
	return pszTOC;
}

// 2 author (???)
// 3 composer
// 6 lyricist
// 7 publisher (how is this different from label?)
// 9 songwriter (music & lyrics)
// 10 conductor/arranger
// 11 Arranger
// 12 Conductor
// 13 Director
// 72 Engineer
// 74 Mastering
// 75 Mastering Location
// 76 Mixing
// 77 Mixing Location
// 78 Producer
// 79 Programming (???)
// 80 Recording Location
// 147 remixer
#ifndef IGNORE_API_GRACENOTE
void GetDiscInfo(ICddbDiscPtr pDisc, DINFO *ps)
{
	CComBSTR str, disc_artist, disc_composer, disc_conductor, disc_remixing;
	BSTR composerRole=L"3", conductorRole=L"12", remixingRole=L"147";

	/*
	for (int i=0;i<100;i++)
	{
		wchar_t id[256] = {0};
		_itow(i, id, 10);
	ICddbRolePtr role;
	pCDDBControl->GetRoleInfo(id, &role);
	if (role)
	{
		BSTR name, description;
		role->get_Name(&name);
		role->get_Description(&description);
		wchar_t str[4096] = {0};
		wsprintf(str, L"ID: %s\r\nName: %s\r\nDescription: %s\r\n", id, name, description);
		MessageBoxW(NULL, str, L"CDDB Role", MB_OK);
	}
	}
	*/

	ps->Reset();
	if (pDisc == NULL) // yikes!
		return;
	ICddbDisc2Ptr pDisc2;
	pDisc->QueryInterface(&pDisc2);

	ICddbDisc2_5Ptr pDisc2_5;
	pDisc->QueryInterface(&pDisc2_5);

	if (GetRole(pDisc, conductorRole, &disc_conductor) && disc_conductor && disc_conductor.m_str[0])
		ps->conductor = ndestring_wcsdup(disc_conductor.m_str);

	if (GetRole(pDisc, composerRole, &disc_composer) && disc_composer && disc_composer.m_str[0])
				ps->composer = ndestring_wcsdup(disc_composer.m_str);

	if (GetRole(pDisc, remixingRole, &disc_remixing) && disc_remixing && disc_remixing.m_str[0])
		ps->remixing = ndestring_wcsdup(disc_remixing.m_str);

	if (SUCCEEDED(pDisc->get_Artist(&disc_artist)) && disc_artist && disc_artist.m_str[0])
		ps->artist = ndestring_wcsdup(disc_artist.m_str);

	if (SUCCEEDED(pDisc->get_Year(&str)) && str && str.m_str[0])
		ps->year = ndestring_wcsdup(str.m_str);

	if (pDisc2_5 == NULL
	    || (FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(3, &str))
	        && FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(2, &str))
	        && FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(1, &str))
	        && FAILED(pDisc2_5->get_V2GenreStringPrimary(&str)))
	   )
	{
		pDisc->get_GenreId(&str);
		ICddbGenre *poop = 0;
	 if (SUCCEEDED(pCDDBControl->GetGenreInfo(str, &poop)) && poop)
		{
			poop->get_Name(&str);
			poop->Release();
		}
		else
			str.Empty();
	}
	if (str && str.m_str[0])
		ps->genre = ndestring_wcsdup(str.m_str);

	if (SUCCEEDED(pDisc->get_Title(&str)) && str && str.m_str[0])
		ps->title = ndestring_wcsdup(str.m_str);

	if (SUCCEEDED(pDisc->get_TitleUId(&str)) && str && str.m_str[0])
		ps->tuid = ndestring_wcsdup(str.m_str);

	if (SUCCEEDED(pDisc->get_Label(&str)) && str && str.m_str[0])
		ps->label = ndestring_wcsdup(str.m_str);

	if (SUCCEEDED(pDisc->get_Notes(&str)) && str && str.m_str[0])
		ps->notes = ndestring_wcsdup(str.m_str);

	long val;
	if (SUCCEEDED(pDisc->get_Compilation(&val)))
		ps->compilation = !!val;

	if (SUCCEEDED(pDisc->get_TotalInSet(&str)) && str && str.m_str[0])
		ps->numdiscs = _wtoi(str.m_str);

	if (SUCCEEDED(pDisc->get_NumberInSet(&str)) && str && str.m_str[0])
		ps->discnum = _wtoi(str.m_str);

	for (int x = 0; x < ps->ntracks; x ++)
	{
		TRACKINFO &trackInfo = ps->tracks[x];
		ICddbTrack *t;
		ICddbTrack2_5Ptr track2_5;
		pDisc->GetTrack(x + 1, &t);
		if (!t) break;

		t->QueryInterface(&track2_5);

		if (SUCCEEDED(t->get_Artist(&str)) && str && str.m_str[0] && (!disc_artist || !disc_artist.m_str[0] || wcscmp(str.m_str, disc_artist.m_str)))
			trackInfo.artist = ndestring_wcsdup(str.m_str);

		if (SUCCEEDED(t->get_Title(&str)) && str && str.m_str[0])
			trackInfo.title = ndestring_wcsdup(str.m_str);

		if (SUCCEEDED(t->get_ISRC(&str)) && str && str.m_str[0])
			trackInfo.isrc = ndestring_wcsdup(str.m_str);

		if (SUCCEEDED(pCDDBControl->GetDiscTagId(pDisc, x + 1, &str)) && str && str.m_str[0])
			trackInfo.tagID = ndestring_wcsdup(str.m_str);

		if (GetRole(t, conductorRole, &str) && str && str.m_str[0] && (!disc_conductor || !disc_conductor.m_str[0] || wcscmp(str.m_str, disc_conductor.m_str)))
			trackInfo.conductor = ndestring_wcsdup(str.m_str);

		if (GetRole(t, composerRole, &str) && str && str.m_str[0] && (!disc_composer || !disc_composer.m_str[0] || wcscmp(str.m_str, disc_composer.m_str)))
			trackInfo.composer = ndestring_wcsdup(str.m_str);

		if (GetRole(t, remixingRole, &str) && str && str.m_str[0] && (!disc_remixing || !disc_remixing.m_str[0] || wcscmp(str.m_str, disc_remixing.m_str)))
			trackInfo.remixing = ndestring_wcsdup(str.m_str);

			if (track2_5 != NULL && (SUCCEEDED(track2_5->get_ExtDataSerialized(&str)) && str && str.m_str[0]) // try track first
				|| (pDisc2_5 != NULL && SUCCEEDED(pDisc2_5->get_ExtDataSerialized(&str)) && str && str.m_str[0])) // then disc
				trackInfo.extData = ndestring_wcsdup(str.m_str);

		t->Release();
	}

	ps->populated = true;
}
#endif

HRESULT Cddb_GetIUIOptions(void **ppUIOptions)
{
	HRESULT hr;

	if (!ppUIOptions) return E_INVALIDARG;
	*ppUIOptions = NULL;

	hr = SetupCDDB(FALSE);
	if (SUCCEEDED(hr))
	{
		#ifndef IGNORE_API_GRACENOTE
		hr = CoCreateInstance(CLSID_CddbUIOptions, NULL, 
				CLSCTX_INPROC_SERVER, IID_ICddbUIOptions, ppUIOptions);
		#endif
	}
	return hr;
}

HRESULT Cddb_GetIControl(void **ppControl)
{
	HRESULT hr;

	if (!ppControl) return E_INVALIDARG;
	*ppControl = NULL;

	hr = SetupCDDB(FALSE);
	if(SUCCEEDED(hr))
	{
		#ifndef IGNORE_API_GRACENOTE
		pCDDBControl->AddRef();
		*ppControl = pCDDBControl;
		#endif
	}
	return hr;
}

HRESULT Cddb_GetICacheManger(void **ppCache)
{
	HRESULT hr;

	if (!ppCache) return E_INVALIDARG;
	*ppCache = NULL;

	hr = SetupCDDB(FALSE);
	if (SUCCEEDED(hr))
	{
		#ifndef IGNORE_API_GRACENOTE
		hr = CoCreateInstance(CLSID_CddbCacheManager, NULL, 
				CLSCTX_INPROC_SERVER, IID_ICddbCacheManager, ppCache);
		#endif
	}
	return hr;
}

static HRESULT SetupCDDB(BOOL resetError)
{
	static HRESULT result(S_FALSE);

	#ifndef IGNORE_API_GRACENOTE
	if (S_FALSE == result || (FAILED(result) && resetError))
	{
		if (AGAVE_API_GRACENOTE)
			pCDDBControl = AGAVE_API_GRACENOTE->GetCDDB();
		result = (NULL == pCDDBControl) ? CDDBCTLNotInitialized : S_OK;
		if (SUCCEEDED(result))
		{
			HRESULT hr;
			ICddbOptionsPtr pOptions;
			hr = pCDDBControl->GetOptions(&pOptions);
			if (SUCCEEDED(hr))
			{
				LONG lVal;
				BOOL bUpdate(FALSE);
				hr = pOptions->get_AsyncCompletion(&lVal);
				if (SUCCEEDED(hr) && !lVal) { pOptions->put_AsyncCompletion(1); bUpdate = TRUE; }
				hr = pOptions->get_ProgressEvents(&lVal);
				if (SUCCEEDED(hr) && !lVal) { pOptions->put_ProgressEvents(1); bUpdate = TRUE; }

#if defined(BETA) || defined(INTERNAL) || defined(NIGHT)
				pOptions->put_TestSubmitMode(TRUE); // BETA
#endif

				if (bUpdate) pCDDBControl->SetOptions(pOptions);
			}
			if (SUCCEEDED(eventMngr.Advise(pCDDBControl)))
			{
				eventMngr.RegisterCallback(CDDB_CB_CMDCOMPLETED, Cddb_OnCommandCompleted);
				eventMngr.RegisterCallback(CDDB_CB_CMDPROGRESS, Cddb_OnCommandProgress);
			}

		}
	}
	#endif
	return result;
}

#ifndef IGNORE_API_GRACENOTE
HRESULT Cddb_GetDiscFromCache(BSTR bstrTOC, ICddbDisc **ppDisc)
{
	HRESULT hr;
	ICddbCacheManager *pCache;

	if (!ppDisc) return E_INVALIDARG;
	*ppDisc = NULL;

	if (!bstrTOC) return CDDB_E_BADTOC;

	hr = Cddb_GetICacheManger((void**)&pCache);
	if (SUCCEEDED(hr))
	{
		hr = pCache->FetchDiscByToc(bstrTOC, ppDisc);
		pCache->Release();
	}
	
	return hr;
}
#endif

static LRESULT CALLBACK DisplayDiscInfoWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	HWND hwndListener;
	WNDPROC fnOldProc = (WNDPROC)GetPropW(hwnd, L"WNDPROC");
	switch(uMsg)
	{
		case WM_INITDIALOG:
			result = (IsWindowUnicode(hwnd)) ? CallWindowProcW(fnOldProc, hwnd, uMsg, wParam, lParam) : CallWindowProcA(fnOldProc, hwnd, uMsg, wParam, lParam);
			hwndListener = (HWND)GetPropW(hwnd, L"LISTENER");
			if (hwndListener) CddbProgressDlg_SetStatus(hwndListener, MAKEINTRESOURCEW(IDS_OPENING), -1);
			return result;
		case WM_DESTROY:
			{
				RemovePropW(hwnd, L"WNDPROC");
				hwndListener = (HWND)GetPropW(hwnd, L"LISTENER");
				if (hwndListener)
				{
					RECT rc, rw;
					if (GetWindowRect(hwndListener, &rc) && GetWindowRect(hwnd, &rw)) 
					{
						CalculatePopUpPos(&rw, &rc, CENTER_LEFT);
						SetWindowPos(hwndListener, NULL, rc.left, rc.top, 0, 0, 
							SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
					}
				}
				HWND *list = (HWND*)GetPropW(hwnd, L"WNDLIST");
				RemovePropW(hwnd, L"WNDLIST");
				if (list)
				{
					for (HWND *p = list; *p != NULL; p++) ShowWindowAsync(*p, SW_SHOWNA);
					free(list);
				}
				RemovePropW(hwnd, L"LISTENER");
				
				if (fnOldProc) SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)fnOldProc);
			}
			break;
		case WM_SHOWWINDOW:
			if (wParam)
			{
				hwndListener = (HWND)GetPropW(hwnd, L"LISTENER");
				if(hwndListener)
				{
					HWND *list = (HWND*)calloc(24, sizeof(HWND));
					if (!FindAllOwnedWindows(hwndListener, list, 24, 0) || !SetPropW(hwnd, L"WNDLIST", list)) free(list);
					else for (HWND *p = list; *p != NULL; p++) ShowWindowAsync(*p, SW_HIDE);
					ShowWindowAsync(hwndListener, SW_HIDE);
				}
			}
			break;
	}

	if (!fnOldProc)
	{
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	return (IsWindowUnicode(hwnd)) ? CallWindowProcW(fnOldProc, hwnd, uMsg, wParam, lParam) : CallWindowProcA(fnOldProc, hwnd, uMsg, wParam, lParam);
}

static void CALLBACK OnGrabbed_DisplayDiscInfo(HWND hwnd, CREATESTRUCT *lpcs, HWND *phwndInsertAfter, ULONG_PTR user)
{
	WNDPROC oldProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)DisplayDiscInfoWndProc);
	if (oldProc)
	{
		SetPropW(hwnd, L"WNDPROC", oldProc);
		SetPropW(hwnd, L"LISTENER", (HANDLE)user);
	}
	if(user)
	{
		RECT rc, rw;
		if (GetClientRect((HWND)user, &rc))
		{
			MapWindowPoints((HWND)user, HWND_DESKTOP, (POINT*)&rc, 2);
			SetRect(&rw, lpcs->x, lpcs->y, lpcs->x + lpcs->cx, lpcs->y + lpcs->cy);
			CalculatePopUpPos(&rc, &rw, CENTER_LEFT);
			lpcs->x = rw.left;
			lpcs->y = rw.top;
		}
	}
	else ShowWindow((HWND)user, SW_HIDE);
}

void Cddb_GetResultText(HRESULT hr, LPWSTR pszResult, INT cchResult, LPWSTR  pszReason, INT cchReason)
{
	INT nResult, nReason;

	nResult = (S_OK == hr) ? IDS_SUCCESS : IDS_NOT_FOUND;
    nReason = 0;

	if (FAILED(hr)) 
	{
		switch(hr)
		{
			#ifndef IGNORE_API_GRACENOTE
			case CDDBCTLNotInitialized:	nReason = IDS_CDDB_NOT_INSTALLED; break;
			case CDDBCTLBusy:			nReason = IDS_CDDB_E_BUSY; break;
			case CDDB_E_BADTOC:			nReason = IDS_CDDB_E_BADTOC; break;
			case E_ABORT:
			case CDDBTRNCancelled:
										nReason = IDS_CDDB_E_ABORT; break;
			#endif
			default:					nReason = IDS_CDDB_E_FAIL; break;
		}
	}

	if (pszReason && cchReason) 
	{
		if(nReason)
		{	INT len;
			WASABI_API_LNGSTRINGW_BUF(IDS_REASON, pszReason, cchReason);
			len = lstrlenW(pszReason);
			cchReason -= len;
			WASABI_API_LNGSTRINGW_BUF(nReason, pszReason + len, cchReason);
		}
		else pszReason[0] = 0x00;
	}

	if (pszResult && cchResult)
	{
		WASABI_API_LNGSTRINGW_BUF(nResult, pszResult, cchResult);
	}
}

static HRESULT Cddb_FinishLookup(MEDIALOOKUP *pLookup, ICddbDisc *pDisc)
{
	HRESULT hr = E_FAIL;
	#ifndef IGNORE_API_GRACENOTE
	MEDIALOOKUP lookup_cpy;
	DWORD delay = AUTOCLOSE_NOW;

	if (!pLookup) hr = E_INVALIDARG;
	if (!evntBusy) hr = E_FAIL;
	else hr = S_OK;

	if (NULL != pDisc)
		pDisc->AddRef();

	if (NULL != pLookup)
	{
		CopyMemory(&lookup_cpy, pLookup, sizeof(MEDIALOOKUP));
		SecureZeroMemory(pLookup, sizeof(MEDIALOOKUP));
	}
	else
		SecureZeroMemory(&lookup_cpy, sizeof(MEDIALOOKUP));

	if (SUCCEEDED(hr))
	{
		HRESULT hrInvoke;
		
		if(S_OK != lookup_cpy.result) 
		{
			if (NULL != pDisc)
				pDisc->Release();

			pDisc = NULL;
		}

		if (lookup_cpy.hwndInfo)
		{
			CddbProgressDlg_SetExtendedMode(lookup_cpy.hwndInfo, FALSE);
			CddbProgressDlg_EnableAbortButton(lookup_cpy.hwndInfo, FALSE);
			CddbProgressDlg_SetStatus(lookup_cpy.hwndInfo, MAKEINTRESOURCEW(IDS_PROCESSING), -1);
		}

		if (FAILED(lookup_cpy.result))
			delay = 5000;

		hrInvoke = Cddb_InvokeProcessResult(&lookup_cpy, pDisc, &delay);
		if (FAILED(hrInvoke) && S_OK == lookup_cpy.result) 
			lookup_cpy.result = hrInvoke;
	}

	SET_BUSY(FALSE);

	if (SUCCEEDED(hr))
	{
		if (lookup_cpy.hwndInfo) 
		{
			RECT rc;
			if (GetWindowRect(lookup_cpy.hwndInfo, &rc))
			{
				g_lastcddbpos.x = rc.left;
				g_lastcddbpos.y = rc.top;
			}

			lookup_cpy.result = Cddb_DisplayResultDlg(lookup_cpy.hwndInfo, lookup_cpy.result, delay,
														CDDB_UI_USE_PARENT | lookup_cpy.flags);
			if (delay > 10 && 0 == (CDDB_UI_RESULT_MODAL & lookup_cpy.flags) || FAILED(lookup_cpy.result)) 
			{
				CddbProgressDlg_ExitModal(lookup_cpy.hwndInfo, lookup_cpy.result, FALSE);
			}
		}
		hr = lookup_cpy.result;
	}

	if (NULL != lookup_cpy.bstrTOC)
		SysFreeString(lookup_cpy.bstrTOC);

	if (lookup_cpy.threadId == GetCurrentThreadId())
		Cddb_UninitializeThread();
	else
	{
		if (PostThreadMessage(lookup_cpy.threadId, TM_UNINITTHREAD, 0, 0L))
		{
		/*	MSG msg;
			DWORD status;
			msg.message = NULL;
			for(;;)
			{
				status = MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
				if (status == WAIT_OBJECT_0+1)
				{						
					while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
					{
						if (WM_QUIT == msg.message) break;
						if (!CallMsgFilter(&msg, 0)) DispatchMessage(&msg);
					}
				} 
				else if (status == WAIT_OBJECT_0)
				{ 
					break; 
				}
			}
			if (WM_QUIT == msg.message) PostQuitMessage((int)msg.wParam);*/
		}
	}

	if (NULL != pDisc)
		pDisc->Release();
	#endif
	return hr;
}

static void CALLBACK CddbProgressDlg_OnAbort(HWND hwndDlg, BSTR bstrUser)
{
	#ifndef IGNORE_API_GRACENOTE
	ICDDBControl *pControl;
	HRESULT hr = Cddb_GetIControl((void**)&pControl);
	if (SUCCEEDED(hr))
	{
		LONG lVal;
		pControl->Cancel(&lVal);
		CddbProgressDlg_EnableAbortButton(hwndDlg, FALSE);
		pControl->Release();
	}

	//CddbProgressDlg_Completed(hwndDlg, L"Aborted !!!", 4600);
	#endif
}

typedef struct _MODALDATA
{
	ICddbDisc		*pDisc;
	UINT			flags;
	BSTR			bstrTOC;
	
} MODALDATA;

#ifndef IGNORE_API_GRACENOTE
HRESULT Cddb_DisplayDiscInfo(ICddbDisc *pDisc, CDDBUIFlags *pUIFlags, HWND hwndParent)
{
	HRESULT hr;
	ICDDBControl *pControl;
	BOOL bManual = FALSE;
	hr = Cddb_GetIControl((void**)&pControl);
	if (FAILED(hr)) return hr;

	if (hwndParent)
	{
		if (!BeginGrabCreateWindow(L"#32770", NULL, NULL, OnGrabbed_DisplayDiscInfo, (ULONG_PTR)hwndParent)) 
		{
			ShowWindow(hwndParent, SW_HIDE);
			bManual = TRUE;
		}
	}

	hr = pControl->DisplayDiscInfo(pDisc, *pUIFlags, pUIFlags);
	if (hwndParent)
	{
		EndGrabCreateWindow();
		if (bManual) ShowWindow(hwndParent, SW_SHOW);
	}

	return hr;
}

static void CALLBACK CddbProgressDlg_OnSubmitNew(HWND hwndDlg, BSTR bstrUser)
{
	HRESULT hr;
	ICDDBControl *pControl;
	CDDBUIFlags	uiflags;
	MODALDATA *pData;
	wchar_t szText[256] = {0};

	pData = (MODALDATA*)CddbProgressDlg_GetUserData(hwndDlg);
	if (!pData) 
	{
		CddbProgressDlg_ExitModal(hwndDlg, E_INVALIDARG, TRUE);
		return;
	}

	CddbProgressDlg_ShowButton1(hwndDlg, NULL, NULL, NULL);

	SetWindowText(hwndDlg, WASABI_API_LNGSTRINGW_BUF(IDS_SUBMITDISC_TITLE, szText, sizeof(szText)/sizeof(wchar_t)));
	CddbProgressDlg_Initialize(hwndDlg, MAKEINTRESOURCEW(IDS_QUERYING), CddbProgressDlg_OnAbort, NULL);
	CddbProgressDlg_SetStatus(hwndDlg, MAKEINTRESOURCEW(IDS_INITIALIZING), -1);

	hr = Cddb_GetIControl((void**)&pControl);
	if (SUCCEEDED(hr))
	{
		hr = pControl->GetSubmitDisc(pData->bstrTOC, NULL, NULL, &pData->pDisc);
		pControl->Release();
	}

	if (!pData->pDisc)
	{
		CddbProgressDlg_ExitModal(hwndDlg, hr, TRUE);
		return;
	}

	uiflags =  (CDDBUIFlags)(UI_SUBMITNEW | UI_EDITMODE);
	hr = Cddb_DisplayDiscInfo(pData->pDisc, &uiflags, hwndDlg);

	if (SUCCEEDED(hr) && (0 == ((uiflags & (UI_OK | UI_DATA_CHANGED)) == (UI_OK | UI_DATA_CHANGED))))
		hr = S_FALSE;

	if (S_OK != hr)
	{
		pData->pDisc->Release();
		pData->pDisc = NULL;
	}
	CddbProgressDlg_ExitModal(hwndDlg, hr, TRUE);
	return;
}

static void CALLBACK CddbProgressDlg_OnAcceptMatch(HWND hwndDlg, BSTR bstrUser)
{
	MODALDATA *pData;
	pData = (MODALDATA*)CddbProgressDlg_GetUserData(hwndDlg);
	if (!pData) 
	{
		CddbProgressDlg_ExitModal(hwndDlg, E_INVALIDARG, TRUE);
		return;
	}

	pData->pDisc = (ICddbDisc*)(LONG_PTR)(LONG)(CddbProgressDlg_GetSelRecordIndex(hwndDlg) + 1);
	CddbProgressDlg_ExitModal(hwndDlg, S_OK, TRUE);
	return;
}
#endif

HRESULT Cddb_DoLookup(LPCWSTR pszTOC, HWND hwndParent, CDDB_CB callback, UINT flags, ULONG_PTR user)
{
	#ifndef IGNORE_API_GRACENOTE
	HRESULT hr;

	ICDDBControl *pControl;
	CDDBMatchCode	matchCode;

	if (!callback) return E_INVALIDARG;

	if (IS_BUSY(0))
	{
		return  (CSTR_EQUAL == CompareStringW(INVARIANT_LCID, 0, g_lookup.bstrTOC, -1, pszTOC, -1)) ? E_PENDING : CDDBCTLBusy;
	}

	hr = Cddb_InitializeThread();
	if (FAILED(hr)) return hr;	

	SET_BUSY(TRUE);
	CLEARLOOKUP();

	g_lookup.callback	= callback;
	g_lookup.user		= user;
	g_lookup.flags		= flags;
	g_lookup.threadId	= GetCurrentThreadId();
	g_lookup.bstrTOC	= SysAllocString(pszTOC);
	eventMngr.SetUserParam((ULONG_PTR)&g_lookup);

	if (0 == (CDDB_NOCACHE & flags))
	{
		ICddbDisc *pDisc;
		g_lookup.result = Cddb_GetDiscFromCache(g_lookup.bstrTOC, &pDisc);
		if (CDDBTRNDataStoreNotCached != g_lookup.result) 
		{
			Cddb_FinishLookup(&g_lookup, pDisc);
			if (pDisc) pDisc->Release();
			return S_OK;
		}
	}

	if (CDDB_NOINET == (CDDB_NOINET & flags)) 
	{
		g_lookup.result = S_FALSE;
		Cddb_FinishLookup(&g_lookup, NULL);
		return S_OK;
	}

	hwndParent = GetAdaptedParent(hwndParent);

	g_lookup.hwndInfo = CddbProgressDlg_Create(hwndParent, SW_HIDE);
	hwndProgressListener = g_lookup.hwndInfo;

	CddbProgressDlg_Initialize(g_lookup.hwndInfo, MAKEINTRESOURCEW(IDS_QUERYING), CddbProgressDlg_OnAbort, NULL);
    CddbProgressDlg_SetStatus(g_lookup.hwndInfo, MAKEINTRESOURCEW(IDS_INITIALIZING), -1);

	if (g_lookup.hwndInfo)
	{
		SetPopUpPos(g_lookup.hwndInfo, CENTER_LEFT);
		ShowWindow(g_lookup.hwndInfo, SW_SHOWNA);
	}

	g_lookup.result = Cddb_GetIControl((void**)&pControl);
	if (FAILED(g_lookup.result)) 
	{
		Cddb_FinishLookup(&g_lookup, NULL);	
		return S_OK;
	}

	g_lookup.result = pControl->LookupMediaByToc(g_lookup.bstrTOC, TRUE, &matchCode);

	pControl->Release();
	if (FAILED(g_lookup.result))
	{
		Cddb_FinishLookup(&g_lookup, NULL);	
		return S_OK;
	}

	if (CDDB_UI_MODAL & flags)
	{
		RECT rc;

		hr = CddbProgressDlg_DoModal(g_lookup.hwndInfo, &rc);
		if (SUCCEEDED(hr))
		{
			SET_BUSY(FALSE);
			CLEARLOOKUP();
			
			g_lastcddbpos.x = rc.left;
			g_lastcddbpos.y = rc.top;
		}
	}
	return S_OK;
	#else
	return E_FAIL;
	#endif
}

static HRESULT Cddb_DoSubmitNewDlg(HWND hwndCaller, BSTR bstrTOC, UINT flags, ICddbDisc **ppDisc)
{
	RECT  rw;
	HRESULT hr;
	HWND hwndInfo, hwndOldListener, hwndParent;
	MODALDATA data;
	wchar_t szText[256] = {0};

	if (!ppDisc) return E_INVALIDARG;

	data.pDisc = NULL;
	data.bstrTOC = bstrTOC;
	data.flags	= flags;

	hwndParent = (hwndCaller) ? GetParent(hwndCaller) : NULL;
	if (!hwndParent) hwndParent = line.hMainWindow;
	hwndInfo = CddbProgressDlg_Create(hwndParent, SW_HIDE);

	if (!hwndInfo || !CddbProgressDlg_SetUserData(hwndInfo, &data))
	{
		if (hwndInfo) 
			DestroyWindow(hwndInfo);

		if (NULL != data.bstrTOC)
			SysFreeString(data.bstrTOC);
		return E_FAIL;
	}

	SetWindowText(hwndInfo, WASABI_API_LNGSTRINGW_BUF(IDS_LOOKUPRESULT_TITLE, szText, sizeof(szText)/sizeof(wchar_t)));
	CddbProgressDlg_Initialize(hwndInfo, MAKEINTRESOURCEW(IDS_NOT_FOUND), NULL, NULL);
	CddbProgressDlg_Completed(hwndInfo, MAKEINTRESOURCEW(IDS_SUBMIT_OFFER), NULL, AUTOCLOSE_NEVER, S_OK);
	#ifndef IGNORE_API_GRACENOTE
	CddbProgressDlg_ShowButton1(hwndInfo, MAKEINTRESOURCEW(IDS_SUBMITNEW), CddbProgressDlg_OnSubmitNew, NULL);
	#endif

	hwndOldListener = hwndProgressListener;
	if (hwndCaller)
	{
		GetWindowRect(hwndCaller, &rw);
		SetWindowPos(hwndInfo, HWND_TOP, rw.left, rw.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		ShowWindowAsync(hwndCaller, SW_HIDE);
	} 
	else ShowWindow(hwndInfo, SW_SHOW);

	RECT rc;
	hr = CddbProgressDlg_DoModal(hwndInfo, &rc);

	*ppDisc = data.pDisc;

	if (hwndCaller)
	{
		SetWindowPos(hwndCaller, HWND_TOP, rc.left, rc.top, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	hwndProgressListener = hwndOldListener;
	return hr;
}

#ifndef IGNORE_API_GRACENOTE
static HRESULT Cddb_DoFuzzyMatchDlg(HWND hwndCaller, UINT flags, ICddbDiscs *pDiscs, LONG *plVal) 
{
	HRESULT hr;
	LONG count;
	ICddbDisc *pDisc(NULL);
	BSTR bstrArtist, bstrTitle;
	RECT  rw;
	HWND hwndInfo, hwndOldListener, hwndParent;
	MODALDATA data;
	wchar_t szText[256] = {0};

	data.pDisc = NULL;
	data.flags	= flags;

	if (!plVal || !pDiscs) return E_INVALIDARG;
	*plVal = 0L;

	hwndParent = (hwndCaller) ? GetParent(hwndCaller) : NULL;
	if (!hwndParent) hwndParent = line.hMainWindow;
	hwndInfo = CddbProgressDlg_Create(hwndParent, SW_HIDE);

	if (!hwndInfo || !CddbProgressDlg_SetUserData(hwndInfo, &data))
	{
		if (hwndInfo) DestroyWindow(hwndInfo);
		return E_FAIL;
	}

	hwndOldListener = hwndProgressListener;

	SetWindowText(hwndInfo, WASABI_API_LNGSTRINGW_BUF(IDS_LOOKUPRESULT_TITLE, szText, sizeof(szText)/sizeof(wchar_t)));
	CddbProgressDlg_Initialize(hwndInfo, MAKEINTRESOURCEW(IDS_FUZZYDISC_TITLE), NULL, NULL);
	CddbProgressDlg_SetStatus(hwndInfo, MAKEINTRESOURCEW(IDS_INITIALIZING), -1);

	if (hwndCaller)
	{
		GetWindowRect(hwndCaller, &rw);
		SetWindowPos(hwndInfo, HWND_TOP, rw.left, rw.top, 0, 0, SWP_NOSIZE);
		ShowWindowAsync(hwndCaller, SW_HIDE);
	}
	ShowWindow(hwndInfo, SW_SHOWNA);

	if (FAILED(pDiscs->get_Count(&count))) count = 0;
	for(long i = 1; i <= count; i++)
	{
		if (SUCCEEDED(pDiscs->GetDisc(i, &pDisc)))
		{
			if (FAILED(pDisc->get_Artist(&bstrArtist))) bstrArtist = NULL;
			if (FAILED(pDisc->get_Title(&bstrTitle))) bstrTitle = NULL;
			CddbProgressDlg_AddRecord(hwndInfo, bstrArtist, bstrTitle, NULL);
			if (bstrArtist) SysFreeString(bstrArtist);
			if (bstrTitle) SysFreeString(bstrTitle);
			pDisc->Release();
		}
	}

	CddbProgressDlg_ShowButton1(hwndInfo, MAKEINTRESOURCEW(IDS_ACCEPT), CddbProgressDlg_OnAcceptMatch, NULL);
	CddbProgressDlg_SetExtendedMode(hwndInfo, TRUE);
	CddbProgressDlg_Completed(hwndInfo, MAKEINTRESOURCEW(IDS_FOUND_MULTIPLE), NULL, AUTOCLOSE_NEVER, S_OK);

	UpdateWindow(hwndInfo);
	SetForegroundWindow(hwndInfo);

	RECT rc;
	hr = CddbProgressDlg_DoModal(hwndInfo, &rc);
	g_lastcddbpos.x = rc.left;
	g_lastcddbpos.y = rc.top;
	*plVal = ((LONG)(LONG_PTR)data.pDisc);

	if (hwndCaller)
	{
		SetWindowPos(hwndCaller, HWND_TOP, rc.left, rc.top, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	hwndProgressListener = hwndOldListener;
	return hr;
}
#endif

static void CALLBACK Cddb_OnCommandCompleted(LONG lCommandCode, HRESULT hCommandResult, VARIANT *pCommandData, UINT_PTR user)
{
	#ifndef IGNORE_API_GRACENOTE
	switch(lCommandCode)
	{
		case CMD_LookupMediaByToc: Cddb_OnMediaLookupCompleted(hCommandResult, (CDDBMatchCode)pCommandData->lVal, (MEDIALOOKUP*)user); break;
		case CMD_GetFullDiscInfo: Cddb_OnGetFullDiscInfoCompleted(hCommandResult, (ICddbDisc*)pCommandData->byref, (MEDIALOOKUP*)user); break;
		case CMD_SubmitDisc: Cddb_OnSubmitDiscCompleted(hCommandResult, (MEDIALOOKUP*)user); break;
	}
	#endif
}

static void CALLBACK Cddb_OnCommandProgress(LONG lCommandCode, LONG lProgressCode, LONG lBytesDone, LONG lBytesTotal, UINT_PTR user)
{
	#ifndef IGNORE_API_GRACENOTE
	if (hwndProgressListener)
	{
		INT ids(0), nComplete(-1);
		switch(lProgressCode)
		{
			case CMD_CONNECTING:		ids = IDS_CDDB_PROGRESS_CONNECTING; break;
			case CMD_SENDING:		ids = IDS_CDDB_PROGRESS_SENDING; nComplete = lBytesTotal?((100 * lBytesDone)/lBytesTotal):100; break;
			case CMD_RECEIVING:		ids = IDS_CDDB_PROGRESS_RECEIVING; nComplete = lBytesTotal?((100 * lBytesDone)/lBytesTotal):100; break;
			case CMD_CANCELLED:		ids = IDS_CDDB_PROGRESS_CANCELLED; break;
			case CMD_WAITING:		ids = IDS_CDDB_PROGRESS_WAITING; lBytesTotal?((100 * lBytesDone)/lBytesTotal):100; break;
			case CMD_COMPLETED:		ids = IDS_CDDB_PROGRESS_COMPLETED; break;
		}
		if (ids) CddbProgressDlg_SetStatus(hwndProgressListener, MAKEINTRESOURCEW(ids), nComplete);
	}
	#endif
}

#ifndef IGNORE_API_GRACENOTE
static void Cddb_OnMediaLookupCompleted(HRESULT result, CDDBMatchCode matchCode, MEDIALOOKUP *pLookup)
{
	ICDDBControl *pControl;
	ICddbDisc *pDisc(NULL), *pfuzzyDisc(NULL);
	ICddbDiscs *pDiscs(NULL);
	LONG lVal;
	wchar_t szText[256] = {0};
	if (!pLookup) return;

	SetWindowText(pLookup->hwndInfo, WASABI_API_LNGSTRINGW_BUF(IDS_LOOKUPRESULT_TITLE, szText, sizeof(szText)/sizeof(wchar_t)));
	pLookup->result = result;
	if (FAILED(pLookup->result))
	{
		switch(pLookup->result)
		{
			case CDDBSVCServiceError: pLookup->result = CDDB_E_BADTOC; break;//
		}
		Cddb_FinishLookup(pLookup, NULL);
		return;
	}

	pLookup->result = Cddb_GetIControl((void**)&pControl);
	if (FAILED(pLookup->result)) 
	{
		Cddb_FinishLookup(pLookup, NULL);
		return;
	}

	switch(matchCode)
	{
		case MATCH_EXACT: 
			CddbProgressDlg_SetStatus(pLookup->hwndInfo, MAKEINTRESOURCEW(IDS_FOUND_EXACT), -1);
			pLookup->result = pControl->GetMatchedDiscInfo((IUnknown**)&pDisc);
			break;
		case MATCH_MULTIPLE:
			pLookup->result = S_MULTIPLE;
			CddbProgressDlg_SetStatus(pLookup->hwndInfo, MAKEINTRESOURCEW(IDS_FOUND_MULTIPLE), -1);
			if (0 != ((CDDB_UI_MULTIPLE | CDDB_RESOLVE_MULTIPLE) & pLookup->flags))
			{
				lVal = 0;
				pLookup->result = pControl->GetMatchedDiscInfo((IUnknown**)&pDiscs);
				if (SUCCEEDED(pLookup->result))
				{
					if (0 != (CDDB_UI_MULTIPLE & pLookup->flags))
					{
						pLookup->result = Cddb_DoFuzzyMatchDlg(pLookup->hwndInfo, 0, pDiscs, &lVal);
						if (S_OK != pLookup->result) lVal = 0;
					}
					if (0 == lVal && 0 != (CDDB_RESOLVE_MULTIPLE & pLookup->flags)) lVal = 1;
					if (lVal)
					{	
						CddbProgressDlg_Initialize(pLookup->hwndInfo, MAKEINTRESOURCEW(IDS_QUERYING), CddbProgressDlg_OnAbort, NULL);
						CddbProgressDlg_SetStatus(pLookup->hwndInfo, MAKEINTRESOURCEW(IDS_INITIALIZING), -1);
						pLookup->result = pDiscs->GetDisc(lVal, &pfuzzyDisc);
						if (SUCCEEDED(pLookup->result))
						{
							if (!pfuzzyDisc) pLookup->result = S_FALSE;
							else
							{
								pLookup->result = pfuzzyDisc->IsSubmit(&lVal);
								if (FAILED(pLookup->result) || !lVal) 
								{
									pLookup->result = pControl->GetFullDiscInfo(pfuzzyDisc, TRUE, &pDisc);
									pfuzzyDisc->Release();
									if (SUCCEEDED(pLookup->result)) 
									{
										pDiscs->Release();
										pControl->Release();
										return; 
									}
								}
								else pDisc = pfuzzyDisc;
							}
						}
					}
					pDiscs->Release();
				}
			}
			break;
		case MATCH_NONE:
			pLookup->result = S_FALSE;
			CddbProgressDlg_SetStatus(pLookup->hwndInfo, MAKEINTRESOURCEW(IDS_NOT_FOUND), -1);
			if (CDDB_UI_NOMATCH & pLookup->flags)
			{
				pLookup->result = Cddb_DoSubmitNewDlg(pLookup->hwndInfo, pLookup->bstrTOC, 0, &pDisc);
				if (S_OK == pLookup->result)
				{
					LONG lVal;
					CddbProgressDlg_Initialize(pLookup->hwndInfo, MAKEINTRESOURCEW(IDS_SUBMITTING), CddbProgressDlg_OnAbort, NULL);
					CddbProgressDlg_SetStatus(pLookup->hwndInfo, MAKEINTRESOURCEW(IDS_INITIALIZING), -1);

					pSubmitDisc = pDisc;
					pLookup->result = pControl->SubmitDisc(pSubmitDisc, TRUE, &lVal);	
					if (SUCCEEDED(pLookup->result)) 
					{
						pControl->Release();
						return;
					}
					
				}
			}
			break;
	}
	pControl->Release();

	Cddb_FinishLookup(pLookup, pDisc);
	if (pDisc) pDisc->Release();
}
#endif

static void Cddb_OnGetFullDiscInfoCompleted(HRESULT result, ICddbDisc *pDisc, MEDIALOOKUP *pLookup)
{
	if (!pLookup) return;
	pLookup->result = result;
	Cddb_FinishLookup(pLookup, pDisc);
}

static void Cddb_OnSubmitDiscCompleted(HRESULT result, MEDIALOOKUP *pLookup)
{
	#ifndef IGNORE_API_GRACENOTE
	if (!pLookup) return;
	pLookup->result = result;
	if (FAILED(result))
	{
		if (pSubmitDisc) pSubmitDisc->Release();
		pSubmitDisc = NULL;
	}
	Cddb_FinishLookup(pLookup, pSubmitDisc);
	if (pSubmitDisc) 
	{
		pSubmitDisc->Release();
		pSubmitDisc = NULL;
	}
	#endif
}

HWND Cddb_GetInfoWindow(void)
{
	#ifndef IGNORE_API_GRACENOTE
	return (IS_BUSY(0)) ? g_lookup.hwndInfo  : NULL;
	#else
	return NULL;
	#endif
}

void Cdbb_DisplayInfo(LPCWSTR pszTitle, LPCWSTR pszCaption, LPCWSTR pszStatus, INT percentCompleted)
{
	HWND hwndInfo;
	hwndInfo = Cddb_GetInfoWindow();
	if (!hwndInfo) return;
	BOOL CddbProgressDlg_SetStatus(HWND hwnd, LPCWSTR pszStatus, INT nPercentCompleted);
}

HRESULT Cddb_DisplayResultDlg(HWND hwndParent, HRESULT result, DWORD dwAutoCloseDelay, UINT flags)
{
	#ifndef IGNORE_API_GRACENOTE
	HWND hwndInfo;
	wchar_t szReason[256] = {0}, szResult[256] = {0};

	if (CDDB_UI_USE_PARENT & flags) hwndInfo = hwndParent;
	else 
	{
		wchar_t szText[256] = {0};
		hwndParent = GetAdaptedParent(hwndParent);
		hwndInfo = CddbProgressDlg_Create(hwndParent, SW_HIDE);
		SetPopUpPos(hwndInfo, CENTER_LEFT);

		SetWindowText(hwndInfo, WASABI_API_LNGSTRINGW_BUF(IDS_LOOKUPRESULT_TITLE, szText, sizeof(szText)/sizeof(wchar_t)));
		CddbProgressDlg_Initialize(hwndInfo, NULL, NULL, NULL);
		CddbProgressDlg_Completed(hwndInfo, NULL, NULL, AUTOCLOSE_NEVER, S_OK);
	}
	if (!hwndInfo || !IsWindow(hwndInfo)) return E_FAIL;

	if (AUTOCLOSE_NOW == dwAutoCloseDelay && GetWindowThreadProcessId(hwndInfo, NULL) != GetCurrentThreadId()) 
	{
		dwAutoCloseDelay = 1; // DestroyWindow is not working on other thread
		ShowWindowAsync(hwndInfo, SW_HIDE); // in case we in modal loop on other
	}

	if (dwAutoCloseDelay > 10)
	{
		Cddb_GetResultText(result, szResult, sizeof(szResult)/sizeof(wchar_t), 
											szReason, sizeof(szReason)/sizeof(wchar_t));
		ShowWindow(hwndInfo, SW_SHOW);
	}

	CddbProgressDlg_Completed(hwndInfo, szResult, szReason, dwAutoCloseDelay, result);

	if (dwAutoCloseDelay > 10 && CDDB_UI_RESULT_MODAL == ((CDDB_UI_MODAL | CDDB_UI_RESULT_MODAL) & flags))
	{
		CddbProgressDlg_DoModal(hwndInfo, NULL);
	}
	#endif

	return S_OK;
}