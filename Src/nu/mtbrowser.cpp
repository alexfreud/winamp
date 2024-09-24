#include "./mtbrowser.h"
#include <exdispid.h>


#define QUIT_FORCE				0x00FF

#define THREAD_QUITING			0x0001
#define THREAD_COMINIT			0x1000

#define MEMMMGR_RECALLOCSTEP		8

#define GetThreadBrowserInstance() ((HTMLContainer2*)TlsGetValue(threadStorage))

typedef struct _THREAD_START_PARAM
{
	HANDLE		hEvent;
	MTBROWSER	*pmtb;
} THREAD_START_PARAM;

typedef struct _MEMREC
{
	HANDLE		handle;
	FREEPROC	freeproc;
} MEMREC;

typedef struct _MEMMNGR
{
	CRITICAL_SECTION	cs;
	MEMREC				*pRec;
	INT					cCount;
	INT					cAlloc;
} MEMMNGR;

typedef struct _APCPARAM
{
	MEMMNGR		*pmm;
	VARIANTARG *pArgs;
	INT			cArgs;
	HWND			hwndNotify;
	UINT		uMsgNotify;
	INT			nNotifyCode;
	APCPROC		fnAPC;
	HANDLE		hThread;
} APCPARAM;

typedef struct _CALLBACKPARAM
{
	HWND		hwndNotify;
	UINT	uMsgNotify;
	BOOL	bDestroyed;
	BOOL	bQuiting;
	BOOL	bReady;
} CALLBACKPARAM;

// forward declarations



static DWORD CALLBACK BrowserThread(LPVOID param);
static HRESULT MTBrowser_Quit(HTMLContainer2 *pContainer);

static BOOL MemRec_Add(MEMMNGR *pmm, void *pMem, FREEPROC fnFreeProc);
static BOOL MemRec_Free(MEMMNGR *pmm, void *pMem);

static APCPARAM *AllocAPCParam(MTBROWSER *pmtb, INT cArgs, INT nNotifyCode, APCPROC fnAPC);
static void CALLBACK FreeAPCParam(void *pMem);
// APC 
static void CALLBACK APC_FunctionCall(ULONG_PTR param);

static void CALLBACK APC_NavigateToName(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult);
static void CALLBACK APC_SetLocation(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult);
static void CALLBACK APC_Refresh2(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult);



static DWORD threadStorage	= TLS_OUT_OF_INDEXES;

BOOL MTBrowser_Init(MTBROWSER *pmtb)
{
	if (!pmtb) return FALSE;
	ZeroMemory(pmtb, sizeof(MTBROWSER));

	if (TLS_OUT_OF_INDEXES == threadStorage)
	{
		threadStorage = TlsAlloc();
		if (TLS_OUT_OF_INDEXES == threadStorage) return FALSE;
	}
	
	return TRUE;
}

BOOL MTBrowser_Clear(MTBROWSER *pmtb)
{
	if (pmtb)
	{
		if (pmtb->hThread) CloseHandle(pmtb->hThread);
		if (pmtb->hMemMngr)
		{
			MEMMNGR *pmm;
			pmm = (MEMMNGR*)pmtb->hMemMngr;
			EnterCriticalSection(&pmm->cs);
			for(int i = 0; i < pmm->cCount; i++)
			{
				if (pmm->pRec[i].handle) 
				{
					(pmm->pRec[i].freeproc) ? pmm->pRec[i].freeproc(pmm->pRec[i].handle) : free(pmm->pRec[i].handle);
				}
			}
			pmm->cCount = 0;
			LeaveCriticalSection(&pmm->cs);
			DeleteCriticalSection(&pmm->cs);
			free(pmtb->hMemMngr);
		}
		ZeroMemory(pmtb, sizeof(MTBROWSER));
	}
	return TRUE;
}

BOOL MTBrowser_Start(MTBROWSER *pmtb, HTMLContainer2 *pContainer, UINT uMsgNotify)
{
	THREAD_START_PARAM param = {0};
	if (!pmtb || !pContainer || TLS_OUT_OF_INDEXES == threadStorage) return FALSE;
	
	pmtb->hMemMngr = calloc(1, sizeof(MEMMNGR));
	if (!pmtb->hMemMngr) return FALSE;
	
	InitializeCriticalSection(&((MEMMNGR*)pmtb->hMemMngr)->cs);

	param.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	param.pmtb = pmtb;

	pmtb->uMsgNotify = uMsgNotify;
	pmtb->pContainer	= pContainer;
	pmtb->hThread	= CreateThread(NULL, 0, BrowserThread, (LPVOID)&param, 0, &pmtb->dwThreadId);
   
	if (pmtb->hThread) 
	{	
		WaitForSingleObject(param.hEvent, INFINITE);
		SetThreadPriority(pmtb->hThread, THREAD_PRIORITY_NORMAL);
	}
		
	CloseHandle(param.hEvent);
			
	return (NULL != pmtb->hThread);
}

BOOL MTBrowser_Kill(MTBROWSER *pmtb, UINT nTerminateDelay)
{
	MSG msg;
	if (pmtb)
	{
		pmtb->bQuiting = TRUE;
		if (pmtb && pmtb->hThread) 
		{
			PostThreadMessage(pmtb->dwThreadId, WM_QUIT, QUIT_FORCE, 0L);
			while(WAIT_TIMEOUT == WaitForSingleObject(pmtb->hThread, 0))
			{
				DWORD dwStatus;
				dwStatus = MsgWaitForMultipleObjectsEx(1, &pmtb->hThread, nTerminateDelay, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
				if (WAIT_OBJECT_0 + 1  == dwStatus || WAIT_OBJECT_0 == dwStatus)
				{
					while (PeekMessageW(&msg, NULL, 0xc0d6, 0xc0d6, PM_NOREMOVE)) if (NULL == msg.hwnd) DispatchMessageW(&msg);
				}
				else if (WAIT_TIMEOUT == dwStatus)
				{
					TerminateThread(pmtb->hThread, 3);
					break;
				}
			}
			CloseHandle(pmtb->hThread);
			pmtb->hThread = NULL;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL MTBrowser_QuitAPC(MTBROWSER *pmtb)
{
	if (!pmtb) return FALSE;
	if (pmtb->bQuiting) return TRUE;
	pmtb->bQuiting = (pmtb->dwThreadId && PostThreadMessage(pmtb->dwThreadId, WM_QUIT, 0, 0L));
	return pmtb->bQuiting;
}
BOOL MTBrowser_NavigateToNameAPC(MTBROWSER *pmtb, LPCWSTR pszURL, UINT fFlags)
{
	HAPC hAPC;
	VARIANTARG *pArgs;

	hAPC = MTBrowser_InitializeAPC(pmtb, 2, MTBC_APC_NAVIGATE, APC_NavigateToName, &pArgs);
	if (!hAPC) return FALSE;
	
	pArgs[0].vt		= VT_BSTR;
	pArgs[0].bstrVal= SysAllocString(pszURL);
	pArgs[1].vt		= VT_I4;
	pArgs[1].intVal	= fFlags;
	return MTBrowser_CallAPC(hAPC);
}

BOOL MTBrowser_SetLocationAPC(MTBROWSER *pmtb, RECT *pRect)
{
	HAPC hAPC;
	VARIANTARG *pArgs;

	hAPC = MTBrowser_InitializeAPC(pmtb, 4, MTBC_APC_SETLOCATION, APC_SetLocation, &pArgs);
	if (!hAPC) return FALSE;
	
	pArgs[0].vt		= VT_I4;
	pArgs[0].intVal	= pRect->left;
	pArgs[1].vt		= VT_I4;
	pArgs[1].intVal	= pRect->top;
	pArgs[2].vt		= VT_I4;
	pArgs[2].intVal	= pRect->right - pRect->left;
	pArgs[3].vt		= VT_I4;
	pArgs[3].intVal	= pRect->bottom - pRect->top;
	return MTBrowser_CallAPC(hAPC);
}

BOOL MTBrowser_Refresh2APC(MTBROWSER *pmtb, INT nRefreshMode)
{
	HAPC hAPC;
	VARIANTARG *pArgs;

	hAPC = MTBrowser_InitializeAPC(pmtb, 1, MTBC_APC_REFRESH2, APC_Refresh2, &pArgs);
	if (!hAPC) return FALSE;
	
	pArgs[0].vt		= VT_I4;
	pArgs[0].intVal	= nRefreshMode;
	return MTBrowser_CallAPC(hAPC);
}
HAPC MTBrowser_InitializeAPC(MTBROWSER *pmtb, INT nCount, UINT nCmdCode, APCPROC fnAPC, VARIANTARG **pArgs)
{
	APCPARAM *pParam;
	if (!pmtb || pmtb->bQuiting || !pmtb->hThread) return FALSE;
	pParam = AllocAPCParam(pmtb, nCount, nCmdCode, fnAPC);
	if (pParam && pArgs) *pArgs = pParam->pArgs;
	return (HAPC) pParam;
}

BOOL MTBrowser_CallAPC(HAPC hAPC)
{
	BOOL result;
	if (!hAPC) return FALSE;

	result = QueueUserAPC(APC_FunctionCall, ((APCPARAM*)hAPC)->hThread, (ULONG_PTR)hAPC);
	if (!result) MemRec_Free(((APCPARAM*)hAPC)->pmm, hAPC);

	return result;
}

BOOL MTBrowser_AddMemRec(MTBROWSER *pmtb, void *pMem, FREEPROC fnFreeProc)
{
	return (pmtb && pmtb->hMemMngr) ? MemRec_Add((MEMMNGR*)pmtb->hMemMngr, pMem, fnFreeProc) : FALSE;
}

BOOL MTBrowser_FreeMemRec(MTBROWSER *pmtb, void *pMem)
{
	return (pmtb && pmtb->hMemMngr) ? MemRec_Free((MEMMNGR*)pmtb->hMemMngr, pMem) : FALSE;
}


static BOOL MemRec_Add(MEMMNGR *pmm, void *pMem, FREEPROC fnFreeProc)
{
	if (!pmm || !pMem) return FALSE;
	
	EnterCriticalSection(&pmm->cs);

	if (pmm->cCount == pmm->cAlloc)
	{
		LPVOID pData;
		pData = realloc(pmm->pRec, sizeof(MEMREC)*(pmm->cCount + MEMMMGR_RECALLOCSTEP));
		if (!pData) 
		{
			LeaveCriticalSection(&pmm->cs);
			return FALSE;
		}
		pmm->pRec = (MEMREC*)pData;
		pmm->cAlloc = pmm->cCount + MEMMMGR_RECALLOCSTEP;
	}
	pmm->pRec[pmm->cCount].handle		= pMem;
	pmm->pRec[pmm->cCount].freeproc	= fnFreeProc;
	pmm->cCount++;

	LeaveCriticalSection(&pmm->cs);
	return TRUE;
}

static BOOL MemRec_Free(MEMMNGR *pmm, void *pMem)
{
	INT index;
	MEMREC	rec;

	if (!pmm || !pMem) return FALSE;
	
	EnterCriticalSection(&pmm->cs);

	ZeroMemory(&rec, sizeof(MEMREC));

	for(index = 0; index < pmm->cCount; index++)
	{
		if (pmm->pRec[index].handle == pMem) 
		{
			rec.freeproc= pmm->pRec[index].freeproc;
			rec.handle	= pmm->pRec[index].handle;
			break;
		}
	}

	if (index < (pmm->cCount -1)) MoveMemory(&pmm->pRec[index], &pmm->pRec[index + 1], sizeof(MEMREC)*(pmm->cCount - index - 1));
	if (index < pmm->cCount) pmm->cCount--;
	
	if(rec.handle)  
	{
		(rec.freeproc) ? rec.freeproc(rec.handle) : free(rec.handle);
	}
	LeaveCriticalSection(&pmm->cs);

	return (NULL != rec.handle);
}

#define POSTNOTIFY(_hwnd, _msg, _code, _param) ((_msg && IsWindow(_hwnd)) ? PostMessageW(_hwnd, _msg, (WPARAM)_code, (LPARAM)_param) : FALSE)
static BOOL CALLBACK OnBrowserEvent(HTMLContainer2 *pContainer, DISPID dispId, DISPPARAMS FAR *pDispParams, LPVOID pUser)
{
	CALLBACKPARAM *pcb;
	pcb = (CALLBACKPARAM*)pUser;
	if (!pcb) return FALSE;

	switch(dispId)
	{
		case DISPID_DESTRUCTOR:
			pcb->bDestroyed = TRUE;
			PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0L);
			break;
		case DISPID_NAVIGATECOMPLETE2:
			if (pcb->bReady) 
			{

			}
			else
			{ 
				DWORD_PTR dwRedrawOFF;
				HWND hwndParent;
				hwndParent = pContainer->GetParentHWND();
				if (hwndParent) SendMessageTimeout(hwndParent, WM_SETREDRAW, FALSE, 0L, SMTO_NORMAL, 100, &dwRedrawOFF);
				pContainer->SetLocation(-2000, 0, 100, 100);
				if (hwndParent) SendMessageTimeout(hwndParent, WM_SETREDRAW, TRUE, 0L, SMTO_NORMAL, 100, &dwRedrawOFF);
			}
			break;
		case DISPID_DOCUMENTCOMPLETE:
			{
				HRESULT hr;
				IUnknown  *pUnk, *pUnkDisp;
				BOOL bDocReady;
				bDocReady = FALSE;
				hr = pContainer->GetIUnknown(&pUnk);
				if (SUCCEEDED(hr))
				{
					hr = pDispParams->rgvarg[1].pdispVal->QueryInterface(IID_IUnknown, (void**)&pUnkDisp);
					if (SUCCEEDED(hr)) pUnkDisp->Release();
					if (pUnk == pUnkDisp) bDocReady = TRUE;
					if (!pcb->bReady && pDispParams->rgvarg[0].pvarVal->bstrVal && 
							0 == lstrcmpW(L"about:blank", pDispParams->rgvarg[0].pvarVal->bstrVal))
					{
						pcb->bReady = TRUE;
						POSTNOTIFY(pcb->hwndNotify, pcb->uMsgNotify, MTBC_READY, (LPARAM)pContainer);
						break;
					}
				}
				if (pcb->bReady) POSTNOTIFY(pcb->hwndNotify, pcb->uMsgNotify, MTBC_DOCUMENTCOMPLETE, bDocReady);
			}

	}
	return pcb->bQuiting;
}

static DWORD CALLBACK BrowserThread(LPVOID param)
{
	HRESULT hr;
	HTMLContainer2 *pInstance;
	DWORD dwInterval;
	UINT uState;
	MSG msg;
	CALLBACKPARAM cbParam;

	uState = 0;
	TlsSetValue(threadStorage, 0);

	pInstance = ((THREAD_START_PARAM*)param)->pmtb->pContainer;
	
	ZeroMemory(&cbParam, sizeof(CALLBACKPARAM));

	cbParam.uMsgNotify = ((THREAD_START_PARAM*)param)->pmtb->uMsgNotify;
	cbParam.hwndNotify = pInstance->GetParentHWND();

	pInstance->RegisterBrowserEventCB(OnBrowserEvent, (LPVOID)&cbParam);
	
	((THREAD_START_PARAM*)param)->pmtb->hwndNotify = cbParam.hwndNotify;

	SetEvent(((THREAD_START_PARAM*)param)->hEvent);
		
	hr = OleInitialize(NULL);//hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		uState |= THREAD_COMINIT;
		hr = pInstance->Initialize();
		if(SUCCEEDED(hr)) TlsSetValue(threadStorage, pInstance);
		else 
		{
			pInstance->Finish();
			pInstance->Release();
		}
	}
		
	// force message queue
	PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
		
	if (FAILED(hr) || !IsWindow(cbParam.hwndNotify)) 
	{
		TlsSetValue(threadStorage, 0);
		pInstance->Finish();
		pInstance->Release();
	}
	else
	{
		pInstance->NavigateToName(L"about:blank", 0);
	}
	
	
	dwInterval = INFINITE;
	while (!cbParam.bDestroyed)
	{
  		DWORD dwStatus = MsgWaitForMultipleObjectsEx(0, NULL, dwInterval, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
		if (WAIT_OBJECT_0 == dwStatus) 
		{
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) 
			{
				if (WM_QUIT == msg.message) 
				{ 
					cbParam.bQuiting = TRUE;
					TlsSetValue(threadStorage, 0);
					if (QUIT_FORCE == msg.wParam) 
					{
						if (!cbParam.bDestroyed)
						{
							pInstance->Finish();
							while(pInstance->Release() > 0);
						}
						break;
					}
					else if (0 == (THREAD_QUITING & uState) && SUCCEEDED(MTBrowser_Quit(pInstance))) 
					{
						uState |= THREAD_QUITING;
						dwInterval = 200;
					}
					POSTNOTIFY(cbParam.hwndNotify, cbParam.uMsgNotify, MTBC_APC_QUIT, (LPARAM)(THREAD_QUITING & uState));
				} 
				else if ((WM_KEYFIRST > msg.message || WM_KEYLAST < msg.message || 
							(THREAD_QUITING & uState) || !pInstance->TranslateKey(&msg)) &&
							!IsDialogMessageW(cbParam.hwndNotify, &msg))
				{									
					TranslateMessage(&msg);	
					DispatchMessageW(&msg);
				}
			}
		}
		else if (WAIT_TIMEOUT == dwStatus) // quiting - check readystatus
		{ 
			if (!cbParam.bDestroyed)
			{
				READYSTATE state;
				IWebBrowser2 *pWeb2;
				hr = pInstance->GetIWebBrowser2(&pWeb2);
				
				if (SUCCEEDED(hr))
				{
					hr = pWeb2->get_ReadyState(&state);
					pWeb2->Release();
				}
				else state = READYSTATE_UNINITIALIZED;
				if (FAILED(hr) || READYSTATE_UNINITIALIZED == state || READYSTATE_INTERACTIVE <= state)
				{
					pInstance->Finish();
					pInstance->Release();
				}
			}
		}
	}
	
	if (THREAD_COMINIT & uState) OleUninitialize();//CoUninitialize();
	POSTNOTIFY(cbParam.hwndNotify, cbParam.uMsgNotify, MTBC_DESTROYED, (LPARAM)pInstance);
	
	return 0;
}

static HRESULT MTBrowser_Quit(HTMLContainer2 *pContainer)
{
	HRESULT hr;
	IWebBrowser2	 *pWeb2;
	
	hr = pContainer->GetIWebBrowser2(&pWeb2);
	if (SUCCEEDED(hr))
	{	
		hr = pWeb2->Stop();
		pWeb2->Release();
		if (SUCCEEDED(hr)) hr = pContainer->NavigateToName(L"about:blank", navNoHistory | navNoReadFromCache | navNoWriteToCache);
	}
	return hr;
}

static APCPARAM* AllocAPCParam(MTBROWSER *pmtb, INT cArgs, INT nNotifyCode, APCPROC fnAPC)
{
	if (!pmtb || !pmtb->hMemMngr || !fnAPC) return NULL;

	APCPARAM *pParam = (APCPARAM*)calloc(1, sizeof(APCPARAM));
	if (!pParam) return NULL;

	if (cArgs)
	{
		INT i;
		pParam->pArgs = (VARIANTARG*)calloc(cArgs, sizeof(VARIANTARG));
		if (!pParam->pArgs)
		{
			free(pParam);
			return NULL;
		}
		for (i = 0; i < cArgs; i++) VariantInit(&pParam->pArgs[i]);
	}
	else pParam->pArgs = NULL;

	pParam->cArgs = cArgs;
	pParam->pmm			= (MEMMNGR*)pmtb->hMemMngr;
	pParam->hwndNotify	= pmtb->hwndNotify;
	pParam->uMsgNotify	= pmtb->uMsgNotify;
	pParam->nNotifyCode = nNotifyCode;
	pParam->fnAPC		= fnAPC;
	pParam->hThread		= pmtb->hThread;

	if (!MemRec_Add(pParam->pmm, pParam, FreeAPCParam)) 
	{
		FreeAPCParam(pParam);
		pParam = NULL;
	}
	
	return pParam;
}

static void CALLBACK FreeAPCParam(void *pMem)
{
	if (pMem)
	{
		APCPARAM* pParam;
		pParam = (APCPARAM*)pMem;
		if (pParam->pArgs) 
		{
			for(INT i = 0; i < pParam->cArgs; i++) VariantClear(&pParam->pArgs[i]);
			free(pParam->pArgs);
		}
		free(pMem);
	}
}

static void CALLBACK APC_FunctionCall(ULONG_PTR param)
{
	LPARAM result;
	APCPARAM *pParam;
	pParam = (APCPARAM*)param;
	if (!pParam) return;

	HTMLContainer2 *pContainer;
	pContainer = GetThreadBrowserInstance();
	if (pContainer)
	{
		result = 0L;
		if (pParam->fnAPC) pParam->fnAPC(pContainer, pParam->pArgs, pParam->cArgs, &result);
		if (pParam->nNotifyCode && pParam->hwndNotify && IsWindow(pParam->hwndNotify))
		{
			PostMessageW(pParam->hwndNotify, pParam->uMsgNotify, pParam->nNotifyCode, result);
		}
	}
	MemRec_Free(pParam->pmm, pParam);
	
}

static void CALLBACK APC_NavigateToName(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	*pResult = (2 == cArgs) ? pContainer->NavigateToName(pArgs[0].bstrVal, pArgs[1].intVal) : E_INVALIDARG;
}

static void CALLBACK APC_SetLocation(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	*pResult = (4 == cArgs) ? pContainer->SetLocation(pArgs[0].intVal, pArgs[1].intVal, pArgs[2].intVal, pArgs[3].intVal) : 	E_INVALIDARG;
}


static void CALLBACK APC_Refresh2(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	if (1 != cArgs) *pResult = E_INVALIDARG;
	else
	{
		HRESULT hr;
		IWebBrowser2 *pWeb2;
		hr = pContainer->GetIWebBrowser2(&pWeb2);
		if (SUCCEEDED(hr))
		{
			hr = pWeb2->Refresh2(&pArgs[0]);
			pWeb2->Release();
		}
		*pResult = (LPARAM)hr;
	}
	
}