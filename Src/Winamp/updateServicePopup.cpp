#include "main.h"
#include "./updateService.h"
#include "./api.h"

#include "../omBrowser/obj_ombrowser.h"
#include "../omBrowser/ifc_omservice.h"
#include "../omBrowser/browserPopup.h"
#include "../omBrowser/browserHost.h"

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define SVCPOPUP_PROP		L"Winamp_UpdateSvcWndProp"

typedef struct __SVCPOPUP
{
	UpdateService *service;
	WNDPROC		originalProc;
} SVCPOPUP;

static LRESULT WINAPI ServicePopup_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


HRESULT ServiceSubclass_Attach(HWND hwnd, UpdateService *service)
{
	SVCPOPUP *psp = (SVCPOPUP*)calloc(1, sizeof(SVCPOPUP));
	if (NULL == psp) return E_OUTOFMEMORY;

	psp->service = service;
	if (NULL != service) service->AddRef();

	psp->originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ServicePopup_WindowProc);
	if (NULL == psp->originalProc || FALSE == SetPropW(hwnd, SVCPOPUP_PROP, psp))
	{
		if (NULL != psp->originalProc) 
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)psp->originalProc);
		
		if (NULL != psp->service)
			service->Release();

		free(psp);

		return E_FAIL;
	}
	return S_OK;
}

static void ServicePopup_Dettach(HWND hwnd, SVCPOPUP *psp)
{
	RemovePropW(hwnd, SVCPOPUP_PROP);
	if (NULL == psp) return;
	
	if (NULL != psp->originalProc)
		SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)psp->originalProc);
		
	if (NULL != psp->service)
	{
		psp->service->Finish();
		psp->service->Release();
	}
	free(psp);
}

static void ServicePopup_OnBrowserNotify(HWND hwnd, UpdateService *service, NMHDR *pnmh)
{
	switch(pnmh->code)
	{
		case NBHN_DOCUMENTREADY:
			if (0 == (WS_VISIBLE & GetWindowLongPtrW(hwnd, GWL_STYLE)))
				ShowWindowAsync(hwnd, SW_SHOWNORMAL);
			break;
	}

}

static LRESULT WINAPI ServicePopup_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SVCPOPUP *psp = (SVCPOPUP*)GetPropW(hwnd, SVCPOPUP_PROP);
	if (NULL == psp || NULL == psp->originalProc)
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
		case WM_NCDESTROY:
		case WM_DESTROY:
			{
				WNDPROC proc = psp->originalProc;
				ServicePopup_Dettach(hwnd, psp);
				if (NULL != proc)
					CallWindowProcW(proc, hwnd, uMsg, wParam, lParam);
			}
			return 0;

		case WM_NOTIFY:
			switch(wParam)
			{
				case 0x1000 /*IDC_BROWSER*/:
					ServicePopup_OnBrowserNotify(hwnd, psp->service, (NMHDR*)lParam);
					break;
			}
			break;
	}

	return CallWindowProcW(psp->originalProc, hwnd, uMsg, wParam, lParam);
}


