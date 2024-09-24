#include "./common.h"
#include "./loginBox.h"

#include "./loginProvider.h"
#include "./providerLoader.h"
#include "./providerEnumerator.h"
#include "./loginTemplate.h"
#include "./loginCommand.h"
#include "./loginPage.h"
#include "./loginData.h"
#include "./loginCredentials.h"
#include "./loginCurtain.h"
#include "./loginNotifier.h"
#include "./loginPopup.h"
#include "./popupAgreement.h"
#include "./popupPasscode.h"
#include "./popupMessage.h"
#include "./pageEmpty.h"
#include "./pageError.h"
#include "./animation.h"
#include "./loginStatus.h"
#include "./providerOperation.h"
#include "./loginConfig.h"
#include "./loginTab.h"
#include "./loginGui.h"

#include "./download.h"
#include "./downloadResult.h"
#include "./imageCache.h"

#include "../resource.h"

#include "../api.h"
#include "../api_auth.h"
#include "../../nu/windowsTheme.h"
#include "../../winamp/commandLink.h"
#include "../../ombrowser/ifc_omcacherecord.h"


#include <windows.h>
#include <commctrl.h>
#include <vssym32.h>
#include <shlwapi.h>
#include <strsafe.h>

#define PROVIDERLIST_URL		L"http://client.winamp.com/data/loginproviders"
//#define PROVIDERLIST_URL		L"http://dl.getdropbox.com/u/1994752/loginProviders.xml"

// max mini in dpi
#define LOGINBOX_MINWIDTH	153
#define LOGINBOX_MINHEIGHT	140
#define LOGINBOX_MAXWIDTH 	800
#define LOGINBOX_MAXHEIGHT 	420

#define NLBS_MODAL	0x00010000

typedef struct __LOGINBOXCREATEPARAM
{
	HWND				hOwner;
	api_auth			*auth;
	UINT				style;
	const GUID			*pRealm;
} LOGINBOXCREATEPARAM;


#define PROVIDEROP_UPDATE		0
#define PROVIDEROP_REMOVE		1

typedef struct __LOGINBOX
{
	api_auth		*auth;
	UINT			style;
	GUID			realm;
	RECT			gripRect;
	LoginDownloadResult		*providerUpdate;
	LoginResult		*loginResult;
	LoginStatus		*loginStatus;
	LoginProviderOperation	*providerOp;
	BOOL			agreementOk;
	RECT			buttonMargins;
	LONG			buttonHeight;
	LONG			buttonSpace;
	LONG			buttonTop;
	RECT			minmaxInfo;

	LoginImageCache		*imageCache;
} LOGINBOX;

#define TIMER_ID_CHECKIMAGES	20
#define TIMER_DELAY_CHECKIMAGES	25

typedef struct __LOGINBOXFIND
{
	HWND hwnd;
	WCHAR buffer[32];
	GUID realm;
} LOGINBOXFIND;


#define LOGINBOX_PROP		L"NullsoftAuthLoginBoxProp"
#define GetLoginBox(__hwnd) ((LOGINBOX*)GetProp(__hwnd, LOGINBOX_PROP))

#define IDC_TABFRAME		10000
#define IDC_ACTIVEPAGE		10001
#define IDC_NOTIFIER		10002
#define IDC_CURTAIN			10003
#define IDC_POPUPAGREEMENT	10004
#define IDC_POPUPPASSCODE	10005
#define IDC_POPUPPROVIDEROP	10006

static INT_PTR  CALLBACK LoginBox_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BOOL LoginBox_InitCreateParam(LOGINBOXCREATEPARAM *param, api_auth *auth, const GUID *pRealm, HWND hOwner, UINT style)
{
	if (NULL == param) 
		return FALSE;

	ZeroMemory(param, sizeof(LOGINBOXCREATEPARAM));

	param->style = style;
	param->hOwner = hOwner;
	param->pRealm = pRealm;
	param->auth = auth;

	if (NULL == auth)
		return FALSE;

	return TRUE;
}

HWND LoginBox_CreateWindow(api_auth *auth, const GUID *pRealm, HWND hOwner, UINT fStyle)
{
	LOGINBOXCREATEPARAM param;
	if (FALSE == LoginBox_InitCreateParam(&param, auth, pRealm, hOwner, (0x0000FFFF & fStyle)))
		return NULL;
	
	return WASABI_API_CREATEDIALOGPARAMW(IDD_LOGINBOX, hOwner, LoginBox_DialogProc, (LPARAM)&param);
}

INT_PTR LoginBox_Show(api_auth *auth, const GUID *pRealm, HWND hOwner, UINT fStyle)
{
	LOGINBOXCREATEPARAM param;
	if (FALSE == LoginBox_InitCreateParam(&param, auth, pRealm, hOwner, (0x0000FFFF & fStyle) | NLBS_MODAL))
		return -1;
	
	INT_PTR result = WASABI_API_DIALOGBOXPARAMW(IDD_LOGINBOX, hOwner, LoginBox_DialogProc, (LPARAM)&param);

	return result;
}

static BOOL CALLBACK LoginBox_WindowEnumerator(HWND hwnd, LPARAM lParam)
{
	LOGINBOXFIND *find = (LOGINBOXFIND*)lParam;
	
	if (0 != GetClassName(hwnd, find->buffer, ARRAYSIZE(find->buffer)) &&
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, L"#32770", -1, find->buffer, -1))
	{
		LOGINBOX *login = GetLoginBox(hwnd);
		if (NULL != login && IsEqualGUID(find->realm, login->realm)) 
		{
			find->hwnd = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

HWND LoginBox_FindActive(const GUID *pRealm)
{
	LOGINBOXFIND find;
	ZeroMemory(&find, sizeof(find));
	find.realm = (NULL == pRealm) ? GUID_NULL : *pRealm;
	
	EnumWindows(LoginBox_WindowEnumerator, (LPARAM)&find);
	
	return find.hwnd;
}

static void LoginBox_ShowNotifier(HWND hwnd, INT durationMs)
{
	if (NULL == hwnd) return;
	HWND hNotifier = GetDlgItem(hwnd, IDC_NOTIFIER);
	if (NULL == hNotifier) return;
	
	LONG targetHeight = (LONG)LoginNotifier_GetIdealHeight(hNotifier);
	RECT notifierRect;
	GetWindowRect(hNotifier, &notifierRect);
	LONG currentHeight = notifierRect.bottom - notifierRect.top;
	LONG notifierWidth = notifierRect.right - notifierRect.left;
	
	if (currentHeight < targetHeight)
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&notifierRect, 2);

		HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
		RECT pageRect;
		if (NULL != hPage) 
		{
			GetWindowRect(hPage, &pageRect);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&pageRect, 2);
		}
		
		LONG pageWidth = pageRect.right - pageRect.left;
		LONG pageHeight = pageRect.bottom - pageRect.top;

		UINT pageStyle = (NULL != hPage) ? GetWindowStyle(hPage) : 0;
		if (0 != (WS_VISIBLE & pageStyle))
			SetWindowLongPtr(hPage, GWL_STYLE, pageStyle & ~WS_VISIBLE);
		
		UINT notifierStyle = GetWindowStyle(hNotifier);
		if (0 != (WS_VISIBLE & notifierStyle))
			SetWindowLongPtr(hNotifier, GWL_STYLE, notifierStyle & ~WS_VISIBLE);

		SetWindowPos(hNotifier, NULL, 0, 0, notifierWidth, targetHeight, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOCOPYBITS);
		
			
		if (durationMs > 0)
		{
			HDC targetDc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
			if (NULL != targetDc)
		{
			HDC composeDc = CreateCompatibleDC(targetDc);
			if (NULL != composeDc)
			{
				HBITMAP composeBmp = CreateCompatibleBitmap(targetDc, 
										(notifierWidth > pageWidth) ? notifierWidth : pageWidth, 
										targetHeight + pageHeight);
				if (NULL != composeBmp)
				{
					HBITMAP composeBmpOrig = (HBITMAP)SelectObject(composeDc, composeBmp);
					
					SendMessage(hNotifier, WM_PRINTCLIENT, (WPARAM)composeDc, (LPARAM)(PRF_CLIENT | PRF_ERASEBKGND));
					
					
					ANIMATIONDATA animation;
					if (FALSE != Animation_Initialize(&animation, durationMs))
					{
						while(currentHeight++ < targetHeight)
						{
							Animation_BeginStep(&animation);
							Animation_SetWindowPos(hPage, pageRect.left, ++pageRect.top, pageWidth, --pageHeight, 
											0, composeDc, 0, targetHeight);
							
							BitBlt(targetDc, notifierRect.left, notifierRect.top, notifierWidth, currentHeight, composeDc, 0, targetHeight - currentHeight, SRCCOPY);
							if (NULL != hPage)
								BitBlt(targetDc, pageRect.left, pageRect.top, pageWidth, pageHeight, composeDc, 0, targetHeight, SRCCOPY);
							
							Animation_EndStep(&animation, targetHeight - currentHeight);
						}
					}
					
					SelectObject(composeDc, composeBmpOrig);
					DeleteObject(composeBmp);
				}
				DeleteDC(composeDc);
			}
			ReleaseDC(hwnd, targetDc);
		}
		}
		
		if (0 != (WS_VISIBLE & pageStyle))
		{
			SetWindowLongPtr(hPage, GWL_STYLE, pageStyle);
			INT delta = (targetHeight - currentHeight);
			if (delta > 0)
			{
				SetWindowPos(hPage, NULL, pageRect.left, pageRect.top + delta, pageWidth, pageHeight - delta, 
						SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);
			}
			RedrawWindow(hPage, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASENOW | RDW_ERASE);
		}

		LoginNotifier_PlayBeep(hNotifier);
		ShowWindow(hNotifier, SW_SHOWNA);
		UpdateWindow(hNotifier);
	}
}

static void LoginBox_HideNotifier(HWND hwnd, INT durationMs)
{
	if (NULL == hwnd) return;
	HWND hNotifier = GetDlgItem(hwnd, IDC_NOTIFIER);
	if (NULL == hNotifier) return;

	UINT notifierStyle = GetWindowStyle(hNotifier);
	if (0 == (WS_VISIBLE & notifierStyle)) return;

	SetWindowLongPtr(hNotifier, GWL_STYLE, notifierStyle & ~WS_VISIBLE);
		
	RECT notifierRect;
	GetWindowRect(hNotifier, &notifierRect);
	LONG currentHeight = notifierRect.bottom - notifierRect.top;
	LONG startHeight = currentHeight;
	LONG notifierWidth = notifierRect.right - notifierRect.left;
	
	if (currentHeight > 0)
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&notifierRect, 2);

		HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
		RECT pageRect;
		if (NULL != hPage) 
		{
			GetWindowRect(hPage, &pageRect);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&pageRect, 2);
		}
		
		LONG pageWidth = pageRect.right - pageRect.left;
		LONG pageHeight = pageRect.bottom - pageRect.top;

		UINT pageStyle = (NULL != hPage) ? GetWindowStyle(hPage) : 0;
		if (0 != (WS_VISIBLE & pageStyle))
			SetWindowLongPtr(hPage, GWL_STYLE, pageStyle & ~WS_VISIBLE);
		
		if (durationMs > 0)
		{
			HDC targetDc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
			if (NULL != targetDc)
			{
				HDC composeDc = CreateCompatibleDC(targetDc);
				if (NULL != composeDc)
				{
					HBITMAP composeBmp = CreateCompatibleBitmap(targetDc, 
											(notifierWidth > pageWidth) ? notifierWidth : pageWidth, 
											2*startHeight + pageHeight);
					if (NULL != composeBmp)
					{
						HBITMAP composeBmpOrig = (HBITMAP)SelectObject(composeDc, composeBmp);
						
						SendMessage(hNotifier, WM_PRINTCLIENT, (WPARAM)composeDc, (LPARAM)(PRF_CLIENT | PRF_ERASEBKGND));
						
						ANIMATIONDATA animation;
						if (FALSE != Animation_Initialize(&animation, durationMs))
						{
							while(currentHeight-- > 0)
							{
								Animation_BeginStep(&animation);
								if (FALSE != Animation_SetWindowPos(hPage, pageRect.left, --pageRect.top, pageWidth, ++pageHeight, 
																		0, composeDc, 0, startHeight))
								{
									BitBlt(targetDc, pageRect.left, pageRect.top, pageWidth, pageHeight, composeDc, 0, startHeight, SRCCOPY);
								}
								
								BitBlt(targetDc, notifierRect.left, notifierRect.top, notifierWidth, currentHeight, composeDc, 0, startHeight - currentHeight, SRCCOPY);
							
								Animation_EndStep(&animation, currentHeight);
							}
						}
						
						SelectObject(composeDc, composeBmpOrig);
						DeleteObject(composeBmp);
					}
					DeleteDC(composeDc);
				}
				ReleaseDC(hwnd, targetDc);
			}
		}
		
		if (0 != (WS_VISIBLE & pageStyle))
		{
			SetWindowLongPtr(hPage, GWL_STYLE, pageStyle);
			INT delta = currentHeight;
			if (delta > 0) 
			{
				SetWindowPos(hPage, NULL, pageRect.left, pageRect.top - delta, pageWidth, pageHeight + delta, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);
			}
			RedrawWindow(hPage, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASENOW | RDW_ERASE);
		}

		SetWindowPos(hNotifier, NULL, 0, 0, notifierWidth, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOCOPYBITS);
	}
}
static HRESULT LoginBox_SaveUsername(const GUID *realm, LoginProvider *provider, LoginCredentials *credentials)
{
	if (NULL == realm || NULL == provider || NULL == credentials) 
		return E_INVALIDARG;

	HRESULT hr;
	
	LoginConfig *config;
	hr = LoginConfig::CreateInstance(&config);
	if (FAILED(hr)) return hr;
		
	RPC_CSTR pszRealm;
	if (RPC_S_OK == UuidToStringA(realm, &pszRealm))
	{
		GUID providerId;
		RPC_CSTR pszProvider;
		
		if (SUCCEEDED(provider->GetId(&providerId)) && 
			RPC_S_OK == UuidToStringA(&providerId, &pszProvider))
		{
			LPCWSTR pszUser = credentials->GetUsername();
			
			LPSTR pszUserAnsi;
			if (NULL != pszUser && L'\0' != *pszUser)
				hr = LoginBox_WideCharToMultiByte(CP_UTF8, 0, pszUser, -1, NULL, NULL, &pszUserAnsi);
			else
				pszUserAnsi = NULL;

			if (SUCCEEDED(hr))
			{
				hr = config->WriteAnsiStr((LPCSTR)pszRealm, (LPCSTR)pszProvider, pszUserAnsi);
				LoginBox_FreeAnsiString(pszUserAnsi);
			}
			RpcStringFreeA(&pszProvider);
		}
		else 
			hr = E_FAIL;

		RpcStringFreeA(&pszRealm);
	}
	else
		hr = E_FAIL;

	config->Release();
	return hr;
}

static HRESULT LoginBox_LoadUsername(const GUID *realm, LoginProvider *provider, LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	if (NULL == realm || NULL == provider)
		return E_INVALIDARG;

	HRESULT hr;

	LoginConfig *config;
	hr = LoginConfig::CreateInstance(&config);
	if (FAILED(hr)) return hr;
		
	RPC_CSTR pszRealm;
	if (RPC_S_OK == UuidToStringA(realm, &pszRealm))
	{
		GUID providerId;
		RPC_CSTR pszProvider;
		
		if (SUCCEEDED(provider->GetId(&providerId)) && 
			RPC_S_OK == UuidToStringA(&providerId, &pszProvider))
		{
			CHAR szUserAnsi[1024] = {0};
			INT cchUser = config->ReadAnsiStr((LPCSTR)pszRealm, (LPCSTR)pszProvider, NULL, szUserAnsi, ARRAYSIZE(szUserAnsi));
			
			if (0 == cchUser)
				pszBuffer[0] = L'\0';
			else
			{
				cchUser = MultiByteToWideChar(CP_UTF8, 0, szUserAnsi, cchUser, pszBuffer, cchBufferMax);
				if (0 == cchUser)
					hr = HRESULT_FROM_WIN32(GetLastError());
				else
					pszBuffer[cchUser] = L'\0';
			}
			
			RpcStringFreeA(&pszProvider);
		}
		else 
			hr = E_FAIL;

		RpcStringFreeA(&pszRealm);
	}
	else
		hr = E_FAIL;

	config->Release();
	return hr;


}
static BOOL LoginBox_SelectActivePage(HWND hwnd)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return FALSE;
	
	loginbox->agreementOk = FALSE;
	LoginBox_HideNotifier(hwnd, 0);

	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return FALSE;

	HWND hPageOld = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
	if (NULL != hPageOld)
		SetWindowLongPtr(hPageOld, GWLP_ID, 0);
	
	BOOL enableLogin = FALSE;
	
	HWND hPage = NULL;
	LoginProvider *provider = NULL;
	
	INT iIndex = LoginTab_GetCurSel(hFrame);
	if (-1 != iIndex)
	{
		NLTITEM tabItem;
		tabItem.mask = NLTIF_PARAM;
		provider = (TRUE == LoginTab_GetItem(hFrame, iIndex, &tabItem)) ? 
							(LoginProvider*)tabItem.param : NULL;

		if (NULL != provider)
		{
			LoginTemplate *pageTemplate;
			if (SUCCEEDED(provider->GetTemplate(&pageTemplate)) && NULL != pageTemplate)
			{
				hPage = pageTemplate->CreatePage(hwnd, hwnd);
				pageTemplate->Release();
			}

			LoginCommand *command;
			if (SUCCEEDED(provider->GetCommand(&command)) && NULL != command)
			{
				enableLogin = (S_OK == command->IsValid());
				command->Release();
			}
		}
	}

	if (NULL == hPage)
	{
		hPage = (-1 == iIndex) ?
				LoginPageEmpty::CreatePage(hwnd, hwnd) :
				LoginPageError::CreatePage(hwnd, hwnd);
		
	}

	if (NULL != hPage)
	{
		//if (S_OK == UxTheme_IsThemeActive())
		//	UxEnableThemeDialogTexture(hPage, ETDT_ENABLETAB);
		
		SetWindowLongPtr(hPage, GWLP_ID, IDC_ACTIVEPAGE);

		RECT rect;
		if (NULL != hPageOld && FALSE != GetWindowRect(hPageOld, &rect))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
			SetWindowPos(hPage, HWND_TOP, rect.left, rect.top, 
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);
		}
		else
		{
			SetWindowPos(hPage, HWND_TOP, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOMOVE);
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER);
		}
		
		WCHAR szUsername[512] = {0};
		if(SUCCEEDED(LoginBox_LoadUsername(&loginbox->realm, provider, szUsername, ARRAYSIZE(szUsername))) && 
			L'\0' != szUsername[0])
		{
			LoginPage_SetUsername(hPage, szUsername);
		}

		ShowWindow(hPage, SW_SHOWNORMAL);
	
		if (WS_VISIBLE == ((WS_VISIBLE | WS_DISABLED) & GetWindowStyle(hPage)))
		{
			HWND hFocus = GetFocus();
			if (hFrame != hFocus)
			{
				HWND hTarget = LoginPage_GetFirstItem(hPage);
				if (NULL == hTarget || 
					((WS_VISIBLE | WS_TABSTOP) != ((WS_VISIBLE | WS_TABSTOP | WS_DISABLED) & GetWindowStyle(hTarget))))
				{
					hTarget = GetNextDlgTabItem(hwnd, hFrame, FALSE);
				}

				if (NULL != hTarget && hTarget != hFocus)
					PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hTarget, TRUE);
			}
		}
	}
	else
	{
		enableLogin = FALSE;
	}

	HWND hButton = GetDlgItem(hwnd, IDOK);
	if (NULL != hButton)
		EnableWindow(hButton, enableLogin);

	if (NULL != hPageOld)
	{
		DestroyWindow(hPageOld);
	}
	
	return TRUE;
}

static BOOL LoginBox_RemoveImageHelper(HWND hFrame, HIMAGELIST imageList, UINT imageIndex)
{
	if (NLTM_IMAGE_NONE == imageIndex || NLTM_IMAGE_CALLBACK == imageIndex)
		return FALSE;

	UINT imageListCount = ImageList_GetImageCount(imageList);
	if (imageIndex > imageListCount)
		return FALSE;

	INT iItem = LoginTab_GetItemCount(hFrame);
	NLTITEM tab;

	while(iItem--)
	{
		tab.mask = NLTIF_IMAGE_MASK;
		if (FALSE != LoginTab_GetItem(hFrame, iItem, &tab))
		{
			tab.mask = 0;
			if (tab.iImage >= imageIndex && tab.iImage < imageListCount)
			{
				tab.iImage = (tab.iImage != imageIndex) ? --tab.iImage : NLTM_IMAGE_NONE;
				tab.mask |= NLTIF_IMAGE;
			}

			if (tab.iImageActive >= imageIndex && tab.iImageActive < imageListCount)
			{
				tab.iImageActive = (tab.iImageActive != imageIndex) ? --tab.iImageActive : NLTM_IMAGE_NONE;
				tab.mask |= NLTIF_IMAGE_ACTIVE;
			}

			if (tab.iImageDisabled >= imageIndex && tab.iImageDisabled < imageListCount)
			{
				tab.iImageDisabled = (tab.iImageDisabled != imageIndex) ? --tab.iImageDisabled : NLTM_IMAGE_NONE;
				tab.mask |= NLTIF_IMAGE_DISABLED;
			}

			if (0 != tab.mask)
				LoginTab_SetItem(hFrame, iItem, &tab);
		}
	}
	return ImageList_Remove(imageList, imageIndex);
}


#define IS_IMAGEINDEX_VALID(__imageIndex)  (((INT)(__imageIndex)) >= 0)
static BOOL LoginBox_RemoveImageHelper2(HWND hFrame, NLTITEM *tab)
{
	if (NULL == tab) return FALSE;

	HIMAGELIST imageList = LoginTab_GetImageList(hFrame);
	if (NULL == imageList) return FALSE;
	
	if (IS_IMAGEINDEX_VALID(tab->iImageActive) && 
		FALSE != LoginBox_RemoveImageHelper(hFrame, imageList, tab->iImageActive))
	{
		if (IS_IMAGEINDEX_VALID(tab->iImage))
		{
			if (tab->iImage > tab->iImageActive) tab->iImage--;
			else if (tab->iImage == tab->iImageActive) tab->iImage = NLTM_IMAGE_CALLBACK;
		}

		if (IS_IMAGEINDEX_VALID(tab->iImageDisabled))
		{
			if (tab->iImageDisabled > tab->iImageActive) tab->iImageDisabled--;
			else if (tab->iImageDisabled == tab->iImageActive) tab->iImageDisabled = NLTM_IMAGE_CALLBACK;
		}

	}

	if (IS_IMAGEINDEX_VALID(tab->iImage) && 
		FALSE != LoginBox_RemoveImageHelper(hFrame, imageList, tab->iImageActive))
	{
		if (IS_IMAGEINDEX_VALID(tab->iImageActive))
		{
			if (tab->iImageActive > tab->iImage) tab->iImageActive--;
			else if (tab->iImageActive == tab->iImage) tab->iImageActive = NLTM_IMAGE_CALLBACK;
		}

		if (IS_IMAGEINDEX_VALID(tab->iImageDisabled))
		{
			if (tab->iImageDisabled > tab->iImage) tab->iImageDisabled--;
			else if (tab->iImageDisabled == tab->iImage) tab->iImageDisabled = NLTM_IMAGE_CALLBACK;
		}

	}

	if (IS_IMAGEINDEX_VALID(tab->iImageDisabled) && 
		FALSE != LoginBox_RemoveImageHelper(hFrame, imageList, tab->iImageDisabled))
	{
		if (IS_IMAGEINDEX_VALID(tab->iImageActive))
		{
			if (tab->iImageActive > tab->iImageDisabled) tab->iImageActive--;
			else if (tab->iImageActive == tab->iImageDisabled) tab->iImageActive = NLTM_IMAGE_CALLBACK;
		}

		if (IS_IMAGEINDEX_VALID(tab->iImage))
		{
			if (tab->iImage > tab->iImageDisabled) tab->iImage--;
			else if (tab->iImage == tab->iImageDisabled) tab->iImage = NLTM_IMAGE_CALLBACK;
		}

	}
	return TRUE;
}
static INT LoginBox_AppendMultipleTabs(HWND hwnd, LoginProviderEnumerator *enumerator, const GUID *filterOut, INT *filteredIndex)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return -1;
	
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return -1;

	if (NULL == enumerator)
		return 0;

	GUID testId;
	INT baseIndex = LoginTab_GetItemCount(hwnd);
	INT count = 0;
	NLTITEM tab;
	WCHAR szTitle[256] = {0};
	
	tab.mask = NLTIF_TEXT | NLTIF_PARAM | NLTIF_IMAGE_MASK;
	
	LoginProvider *provider;
	while(S_OK == enumerator->Next(1, &provider, NULL))
	{
		if (NULL != filterOut && SUCCEEDED(provider->GetId(&testId)) && FALSE != IsEqualGUID(testId, *filterOut))
		{
			if (NULL != filteredIndex)
				*filteredIndex = baseIndex + count;
		}
		else
		{
			if (FAILED(provider->GetName(szTitle, ARRAYSIZE(szTitle))))
				szTitle[0] = L'\0';
						
			tab.param = (LPARAM)provider;
			tab.pszText = (LPWSTR)szTitle;
			tab.iImage = NLTM_IMAGE_CALLBACK; 
			tab.iImageActive = NLTM_IMAGE_CALLBACK;
			tab.iImageDisabled = NLTM_IMAGE_CALLBACK;

			if (-1 != LoginTab_InsertItem(hFrame, baseIndex + count, &tab))
			{
				provider->AddRef();
				count++;
			}
		}
		
		provider->Release();
	}

	return count;
}

static INT LoginBox_InsertTab(HWND hwnd, INT index, LoginProvider *provider)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return -1;

	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return -1;

	if (NULL == provider)
		return -1;

	INT count = LoginTab_GetItemCount(hFrame);
	if (index > count) index = count;

	NLTITEM tab;
	WCHAR szTitle[256] = {0};

	if (FAILED(provider->GetName(szTitle, ARRAYSIZE(szTitle))))
		szTitle[0] = L'\0';
		
	tab.mask = NLTIF_TEXT | NLTIF_PARAM | NLTIF_IMAGE_MASK;
	tab.param = (LPARAM)provider;
	tab.pszText = (LPWSTR)szTitle;
	tab.iImage = NLTM_IMAGE_CALLBACK;
	tab.iImageActive = NLTM_IMAGE_CALLBACK;
	tab.iImageDisabled = NLTM_IMAGE_CALLBACK;

	index = LoginTab_InsertItem(hFrame, index, &tab);
	if (-1 != index)
		provider->AddRef();
	
	return index;
}

static INT LoginBox_FindTabByProviderId(HWND hwnd, const GUID *providerId)
{
	if (NULL == providerId)
		return -1;

	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return -1;

	NLTITEM tab;
	tab.mask = NLTIF_PARAM;
	INT count = LoginTab_GetItemCount(hFrame);
	GUID testId;
	for (INT i =0; i < count; i++)
	{
		if (FALSE != LoginTab_GetItem(hFrame, i, &tab))
		{
			if (NULL != tab.param)
			{
				LoginProvider *provider = (LoginProvider*)tab.param;
				if (SUCCEEDED(provider->GetId(&testId)) && FALSE != IsEqualGUID(testId, *providerId))
					return i;
			}
		}
	}
	return -1;
}

static void LoginBox_DeleteAllTabs(HWND hwnd)
{
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return;

	LoginTab_DeleteAllItems(hFrame);
}

static BOOL LoginBox_ReplaceProvider(HWND hwnd, const GUID *sourceId, LoginProvider *target)
{
	if (NULL == sourceId || NULL == target)
		return FALSE;

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return FALSE;

	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return FALSE;

	GUID testId;

	BOOL resultOk = FALSE;

	NLTITEM tab;
	tab.mask = NLTIF_PARAM | NLTIF_IMAGE_MASK;
	INT count = LoginTab_GetItemCount(hFrame);
	for (INT i =0; i < count; i++)
	{
		if (FALSE != LoginTab_GetItem(hFrame, i, &tab))
		{
			LoginProvider *provider = (LoginProvider*)tab.param;
			if (NULL != provider && SUCCEEDED(provider->GetId(&testId)) && IsEqualGUID(*sourceId, testId))
			{

				tab.mask = NLTIF_TEXT | NLTIF_PARAM | NLTIF_IMAGE_MASK;
				tab.param = (LPARAM)target;

				WCHAR szTitle[256] = {0};
				if (FAILED(target->GetName(szTitle, ARRAYSIZE(szTitle))))
					szTitle[0] = L'\0';
								
				tab.pszText = szTitle;
								
				LoginBox_RemoveImageHelper2(hFrame, &tab);
				
				tab.iImage = NLTM_IMAGE_CALLBACK;
				tab.iImageActive = NLTM_IMAGE_CALLBACK;
				tab.iImageDisabled = NLTM_IMAGE_CALLBACK;
				
				if (TRUE == LoginTab_SetItem(hFrame,i, &tab))
				{					
					target->AddRef();
					resultOk = TRUE;
					
					if (i == LoginTab_GetCurSel(hFrame))
						LoginBox_SelectActivePage(hwnd);
					
					provider->Release();
				}
				
				break;
			}
		}
	}
	return resultOk;
}

static BOOL LoginBox_DeleteProvider(HWND hwnd, const GUID *providerId)
{
	if (NULL == providerId)
		return FALSE;

	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return FALSE;

	GUID testId;
	BOOL resultOk = FALSE;

	NLTITEM tab;
	tab.mask = NLTIF_PARAM;
	INT count = LoginTab_GetItemCount(hFrame);
	for (INT i =0; i < count; i++)
	{
		if (FALSE != LoginTab_GetItem(hFrame, i, &tab))
		{
			LoginProvider *provider = (LoginProvider*)tab.param;
			if (NULL != provider && SUCCEEDED(provider->GetId(&testId)) && IsEqualGUID(*providerId, testId))
			{
				BOOL updateSelection = (i == LoginTab_GetCurSel(hFrame));

				if (TRUE == LoginTab_DeleteItem(hFrame, i))
				{
					resultOk = TRUE;
					if (FALSE != updateSelection)
					{
						LoginTab_SetCurSel(hFrame, 0);
						LoginBox_SelectActivePage(hwnd);
					}
				}
				break;
			}
		}
	}
	return resultOk;
}

static BOOL LoginBox_CenterOver(HWND hwnd, HWND hOwner)
{
	RECT rect, rectOwner;

	if (NULL == hwnd || FALSE == GetWindowRect(hwnd, &rect)) 
		return FALSE;
	
	if(NULL == hOwner || FALSE == GetWindowRect(hOwner, &rectOwner))
	{
		HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		if (NULL == hMonitor) return FALSE;
		
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		if (FALSE == GetMonitorInfo(hMonitor, &mi)) return FALSE;
		CopyRect(&rectOwner, &mi.rcWork);
	}
	
	LONG x = rectOwner.left + ((rectOwner.right - rectOwner.left) - (rect.right - rect.left))/2;
	LONG y = rectOwner.top + ((rectOwner.bottom - rectOwner.top) - (rect.bottom - rect.top))/2;

	return SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}

static void LoginBox_UpdateMargins(HWND hwnd)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return; 

	RECT rect;
	SetRect(&rect, 4, 14, 0, 0);
	MapDialogRect(hwnd, &rect);
	
	loginbox->buttonHeight = rect.top;
	loginbox->buttonSpace = rect.left;

	HWND hControl = GetDlgItem(hwnd, IDOK);
	if (NULL != hControl)
	{
		loginbox->buttonHeight = LoginBox_GetWindowTextHeight(hControl, 3);
	}

	if (loginbox->buttonHeight < 23)
		loginbox->buttonHeight = 23;

	SetRect(&rect, 2, 8, 8, 6);
	MapDialogRect(hwnd, &rect);
	CopyRect(&loginbox->buttonMargins, &rect);


	SetRect(&loginbox->minmaxInfo, LOGINBOX_MINWIDTH, LOGINBOX_MINHEIGHT, LOGINBOX_MAXWIDTH, LOGINBOX_MAXHEIGHT);
	MapDialogRect(hwnd, &loginbox->minmaxInfo);
}

static BOOL LoginBox_GetGripSize(HWND hwnd, HDC hdc, SIZE *gripSize)
{
	BOOL gripSizeOk = FALSE;
	if (SUCCEEDED(UxTheme_LoadLibrary()) && FALSE != UxIsAppThemed())
	{
		UXTHEME hTheme = UxOpenThemeData(hwnd, L"Scrollbar");
		if (NULL != hTheme)
		{
	        if (SUCCEEDED(UxGetThemePartSize(hTheme, hdc, SBP_SIZEBOX, SZB_RIGHTALIGN, NULL, TS_TRUE, gripSize)))
				gripSizeOk = TRUE;

			UxCloseThemeData(hTheme);
		}
	}
	
	if (FALSE == gripSizeOk)
	{
		gripSize->cx = GetSystemMetrics(SM_CXVSCROLL);
		gripSize->cy = GetSystemMetrics(SM_CYHSCROLL);
	}

	return TRUE;
}


static void LoginBox_LayoutGrip(HWND hwnd)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return;

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{
		SIZE gripSize;
		if (FALSE != LoginBox_GetGripSize(hwnd, hdc, &gripSize))
		{					
			GetClientRect(hwnd, &loginbox->gripRect);
			loginbox->gripRect.left = loginbox->gripRect.right - gripSize.cx;
			loginbox->gripRect.top = loginbox->gripRect.bottom - gripSize.cy;

			LONG test;
			test = gripSize.cx - MulDiv(gripSize.cx, 2, 3);
			if (loginbox->buttonMargins.right < test)
				loginbox->buttonMargins.right = test;

			test = gripSize.cy - MulDiv(gripSize.cy, 2, 3);
			if (loginbox->buttonMargins.bottom < test)
				loginbox->buttonMargins.bottom = test;
		}
		ReleaseDC(hwnd, hdc);
	}
}

static void LoginBox_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return;

	RECT clientRect;
	if (NULL == hwnd || FALSE == GetClientRect(hwnd, &clientRect)) 
		return;

	UINT swpFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE == fRedraw) swpFlags |= SWP_NOREDRAW;

	const INT szButtons[] = { IDOK, IDCANCEL, };
	const INT szControls[] = { IDC_STATUS, IDC_TABFRAME, IDC_NOTIFIER, IDC_ACTIVEPAGE, IDC_CURTAIN, };

	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szButtons) + ARRAYSIZE(szControls));
	if (NULL == hdwp) return;

	RECT rect;

	HRGN rgnUpdate(NULL), rgn(NULL);
	if (FALSE != fRedraw)
	{
		rgnUpdate = CreateRectRgn(0, 0, 0, 0);
		rgn = CreateRectRgn(0,0,0,0);

		if (FALSE == IsRectEmpty(&loginbox->gripRect))
		{
			SetRectRgn(rgn, loginbox->gripRect.left, loginbox->gripRect.top, 
					loginbox->gripRect.right, loginbox->gripRect.bottom);
			CombineRgn(rgnUpdate, rgnUpdate, rgn, RGN_OR);
		}

		if (loginbox->buttonTop > clientRect.top)
		{
			SetRectRgn(rgn, clientRect.left, loginbox->buttonTop, clientRect.right, loginbox->buttonTop + 1);
			CombineRgn(rgnUpdate, rgnUpdate, rgn, RGN_OR);
		}
	}

	LoginBox_LayoutGrip(hwnd);

	CopyRect(&rect, &clientRect);
	rect.left += loginbox->buttonMargins.left;
	rect.top += loginbox->buttonMargins.top;
	rect.right -= loginbox->buttonMargins.right;
	rect.bottom -= loginbox->buttonMargins.bottom;

	hdwp = LoginBox_LayoutButtonBar(hdwp, hwnd, szButtons, ARRAYSIZE(szButtons), &rect, 
				loginbox->buttonHeight, loginbox->buttonSpace, fRedraw, &rect);

	loginbox->buttonTop = rect.top - loginbox->buttonMargins.top;

	if (NULL != rgn && loginbox->buttonTop > clientRect.top)
	{
		SetRectRgn(rgn, clientRect.left, loginbox->buttonTop, clientRect.right, loginbox->buttonTop + 1);
		CombineRgn(rgnUpdate, rgnUpdate, rgn, RGN_OR);
	}

	LONG statusRight = rect.left - loginbox->buttonSpace;
	LONG pageTop = clientRect.top;
	LONG pageBottom = loginbox->buttonTop;

	UINT flags;

	if (NULL != rgn)
	{
		SetRectRgn(rgn, loginbox->gripRect.left, loginbox->gripRect.top, 
				loginbox->gripRect.right, loginbox->gripRect.bottom);
		CombineRgn(rgnUpdate, rgnUpdate, rgn, RGN_OR);
	}

	for (INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		HWND hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL == hControl || FALSE == GetWindowRect(hControl, &rect)) continue;
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
		flags = swpFlags;
		
		switch(szControls[i])
		{
			case IDC_STATUS:
				if (NULL != rgn && 0 != (WS_VISIBLE & GetWindowStyle(hControl)))
				{
					SetRectRgn(rgn, rect.left, rect.top, rect.right, rect.bottom);
					CombineRgn(rgnUpdate, rgnUpdate, rgn, RGN_OR);
				}
				rect.top = rect.bottom - rect.top;
				rect.bottom = clientRect.bottom - loginbox->buttonMargins.bottom;
				rect.top = rect.bottom - rect.top;
				rect.left = clientRect.left + loginbox->buttonMargins.left;
				rect.right = statusRight;
				flags |= SWP_NOREDRAW | SWP_NOCOPYBITS;
				if (NULL != rgn && 0 != (WS_VISIBLE & GetWindowStyle(hControl)))
				{
					SetRectRgn(rgn, rect.left, rect.top, rect.right, rect.bottom);
					CombineRgn(rgnUpdate, rgnUpdate, rgn, RGN_OR);
				}

				break;

			case IDC_TABFRAME:
				rect.left = clientRect.left;
				rect.top = clientRect.top;
				rect.right = clientRect.right;
				rect.bottom = rect.top + LoginTab_GetIdealHeight(hControl);
				pageTop = rect.bottom;
				break;

			case IDC_NOTIFIER:
				rect.bottom = pageTop + (rect.bottom - rect.top);
				rect.top = pageTop;
				rect.left = clientRect.left;
				rect.right = clientRect.right;
				if (0 != (WS_VISIBLE & GetWindowStyle(hControl)))
					pageTop = rect.bottom;
				break;

			case IDC_ACTIVEPAGE:
			case IDC_CURTAIN:
				rect.left = clientRect.left;
				rect.top = pageTop;
				rect.right = clientRect.right;
				rect.bottom = (pageBottom > pageTop) ? pageBottom : pageTop;
				break;
		}

		hdwp = DeferWindowPos(hdwp, hControl, NULL, rect.left, rect.top, 
				rect.right - rect.left, rect.bottom - rect.top, flags);
		
		if (NULL == hdwp) break;
	}

	if (NULL != hdwp)
	{
		EndDeferWindowPos(hdwp);
	}

	if (NULL != rgnUpdate)
	{
		RedrawWindow(hwnd, NULL, rgnUpdate, RDW_INVALIDATE | RDW_ALLCHILDREN);
		DeleteObject(rgnUpdate);
	}
	
	if (NULL != rgn)
		DeleteObject(rgn);
}

static BOOL LoginBox_DrawResizeGrip(HWND hwnd, HDC hdc, const RECT *gripRect, HBRUSH brushBk)
{
	BOOL gripDrawOk = FALSE;

	if (SUCCEEDED(UxTheme_LoadLibrary()) && FALSE != UxIsAppThemed())
	{
		UXTHEME hTheme = UxOpenThemeData(hwnd, L"Scrollbar");
		if (NULL != hTheme)
		{		    				
			if (UxIsThemeBackgroundPartiallyTransparent(hTheme, SBP_SIZEBOX, SZB_RIGHTALIGN))
			{
				UXTHEME windowTheme = UxOpenThemeData(hwnd, L"Window");
				if (NULL != windowTheme)
				{
					if (UxIsThemeBackgroundPartiallyTransparent(windowTheme, WP_DIALOG, 0))
						UxDrawThemeParentBackground(hwnd, hdc, gripRect);
					
					//UxDrawThemeBackground(windowTheme, hdc, WP_DIALOG, 0, &gripRect, &gripRect);
					FillRect(hdc, gripRect, brushBk);
					UxCloseThemeData(windowTheme);
				}
			}

			if (SUCCEEDED(UxDrawThemeBackground(hTheme, hdc, SBP_SIZEBOX, SZB_RIGHTALIGN, gripRect, gripRect)))
				gripDrawOk = TRUE;

			UxCloseThemeData(hTheme);
		}
	}
	
	if (FALSE == gripDrawOk)
		gripDrawOk = DrawFrameControl(hdc, (RECT*)gripRect, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);

	return gripDrawOk;
}

static void LoginBox_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return;

	if (FALSE != fErase)
	{
		HBRUSH brushBk = (HBRUSH)SendMessage(hwnd, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hwnd);
		if (NULL == brushBk)
		{
			brushBk = GetSysColorBrush(COLOR_3DFACE);
		}

		HRGN regionFill = CreateRectRgnIndirect(prcPaint);
		HRGN regionClip = NULL;

		if (RectInRegion(regionFill, &loginbox->gripRect) && 
			FALSE != LoginBox_DrawResizeGrip(hwnd, hdc, &loginbox->gripRect, brushBk))
		{
			regionClip = CreateRectRgnIndirect(&loginbox->gripRect);
			if (NULL != regionClip)
				CombineRgn(regionFill, regionFill, regionClip, RGN_DIFF);
		}

		if (loginbox->buttonTop > 0)
		{
			RECT lineRect;
			GetClientRect(hwnd, &lineRect);
			lineRect.top = loginbox->buttonTop;
			lineRect.bottom = loginbox->buttonTop + 1;

			if (RectInRegion(regionFill, &lineRect))
			{
				COLORREF rgbLine  = GetSysColor(COLOR_3DLIGHT);
				if (rgbLine == GetSysColor(COLOR_3DFACE))
					rgbLine = ColorAdjustLuma(rgbLine, -50, TRUE);

				COLORREF rgbOrig = SetBkColor(hdc, rgbLine);
				if (0 != ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &lineRect, NULL, 0, NULL))
				{
					if (NULL == regionClip)	regionClip = CreateRectRgnIndirect(&lineRect);
					else SetRectRgn(regionClip, lineRect.left, lineRect.top, lineRect.right, lineRect.bottom);

					if (NULL != regionClip)
						CombineRgn(regionFill, regionFill, regionClip, RGN_DIFF);
					
				}
				if (rgbOrig != rgbLine) SetBkColor(hdc, rgbOrig);
			}
		}

		if (NULL != regionFill)
		{
			FillRgn(hdc, regionFill, brushBk);
			DeleteObject(regionFill);
		}

		if (NULL != regionClip)
			DeleteObject(regionClip);
	}
}


static LoginProvider *LoginBox_GetActiveProviderInternal(HWND hwnd)
{
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return NULL;

	INT iSelected = LoginTab_GetCurSel(hFrame);
	if (-1 == iSelected) return NULL;
	
	NLTITEM tab;
	tab.mask = NLTIF_PARAM;
	return (TRUE == LoginTab_GetItem(hFrame, iSelected, &tab)) ? 
			(LoginProvider*)tab.param : NULL;
}

static void CALLBACK LoginBox_LoginCompletedEvent(LoginResult *result)
{
	HWND hLoginbox;
	if (SUCCEEDED(result->GetUser((void**)&hLoginbox)) && NULL != hLoginbox)
		PostMessage(hLoginbox, NLBM_LOGINCOMPLETED, 0, (LPARAM)result);
}

static void Loginbox_LockTabFrame(HWND hwnd, BOOL fLock)
{
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return;
	
	UINT frameStyle = GetWindowStyle(hFrame);
	if (FALSE != fLock)
		frameStyle &= ~WS_TABSTOP;
	else
		frameStyle |= WS_TABSTOP;
	
	SetWindowLongPtr(hFrame, GWL_STYLE, frameStyle);

	LoginTab_LockSelection(hFrame, fLock);
}

static BOOL LoginBox_EnableLoginMode(HWND hwnd, BOOL fEnable, BOOL fValidateTos)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return FALSE;

	const static INT szControls[] = { IDC_ACTIVEPAGE, IDOK, };
		
	BOOL resultCode = TRUE;

	HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
	HWND hCurtain = GetDlgItem(hwnd, IDC_CURTAIN);
	
	if (FALSE != LoginPopup::AnyPopup(hwnd))
		return FALSE;

	if (FALSE != fEnable)
	{
		HWND hPopup = NULL;
		
		if (NULL == hCurtain)
		{
			hCurtain = LoginCurtain_CreateWindow(hwnd, (NULL != hPage) ? hPage : hwnd);
			if (NULL != hCurtain)
				SetWindowLongPtr(hCurtain, GWLP_ID, IDC_CURTAIN);
		}
				
		if (FALSE != fValidateTos && TRUE != loginbox->agreementOk)
		{
		
			LoginProvider *provider = LoginBox_GetActiveProviderInternal(hwnd);
			hPopup = LoginPopupAgreement::CreatePopup((NULL != hCurtain) ? hCurtain : hwnd, provider);
			if (NULL != hPopup)
			{
				SetWindowLongPtr(hPopup, GWLP_ID, IDC_POPUPAGREEMENT);
				
				if (NULL != loginbox->loginStatus)
				{
					WCHAR szBuffer[128] = {0};
					WASABI_API_LNGSTRINGW_BUF(IDS_STATUS_AGREEMENT_REQUIRED, szBuffer, ARRAYSIZE(szBuffer));
					if (L'\0' != szBuffer[0])
					{
						UINT statusCookie = loginbox->loginStatus->Add(SysAllocString(szBuffer));
						if (-1 != statusCookie && FALSE == SetProp(hPopup, L"StatusCookie", (HANDLE)(UINT_PTR)(statusCookie + 1)))
							loginbox->loginStatus->Remove(statusCookie);
					}
				}
				

				SetWindowPos(hPopup, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				ShowWindow(hPopup, SW_SHOW);
				if (NULL != hCurtain)
					SetWindowPos(hCurtain, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED );
				
				resultCode = FALSE;

			}
			else
			{
				LoginBox_EnableLoginMode(hwnd, FALSE, FALSE);
				return FALSE;
			}
				
		
		}

		if (NULL != hCurtain)
		{
			ShowWindow(hCurtain, SW_SHOW);
			UpdateWindow(hCurtain);
		}
		
		if (NULL != hPopup && IsWindowVisible(hPopup) && IsWindowEnabled(hPopup))
		{
			HWND hRoot = GetAncestor(hPopup, GA_ROOT);
			if (NULL != hRoot)
				SendMessage(hRoot, WM_NEXTDLGCTL, (WPARAM)hPopup, TRUE);
			LoginPopup_PlayBeep(hPopup);
		}

		Loginbox_LockTabFrame(hwnd, TRUE);
		for (INT i = 0; i < ARRAYSIZE(szControls); i++)
		{
			HWND hControl = GetDlgItem(hwnd, szControls[i]);
			if (NULL != hControl) 
			{	
				EnableWindow(hControl, FALSE);
			}
		}


	}
	else
	{		
		Loginbox_LockTabFrame(hwnd, FALSE);
		for (INT i = 0; i < ARRAYSIZE(szControls); i++)
		{
			HWND hControl = GetDlgItem(hwnd, szControls[i]);
			if (NULL != hControl) EnableWindow(hControl, TRUE);
		}
		
		if (NULL != hPage && IsWindowVisible(hPage) && IsWindowEnabled(hPage))
		{
			HWND hRoot = GetAncestor(hPage, GA_ROOT);
			if (NULL != hRoot)
				SendMessage(hRoot, WM_NEXTDLGCTL, (WPARAM)hPage, TRUE);
		}

		if (NULL != hCurtain)
		{
			DestroyWindow(hCurtain);
		}
	}

	return resultCode;
}

static void LoginBox_PerformLogin(HWND hwnd, LoginData *loginData)
{
	LoginBox_HideNotifier(hwnd, 150);

	if (FALSE == LoginBox_EnableLoginMode(hwnd, TRUE, TRUE))
		return;

	INT authError;

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) authError = AUTH_UNEXPECTED;
	else
	{
		HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
		if (NULL == hPage) authError = AUTH_UNEXPECTED;
		else
		{
			LoginProvider *provider = LoginBox_GetActiveProviderInternal(hwnd);
			if (NULL == provider) authError = AUTH_UNEXPECTED;
			else
			{
				LoginCommand *command;
				if (FAILED(provider->GetCommand(&command))) authError = AUTH_UNEXPECTED;
				else
				{			
					if (NULL == loginData)
					{			
						if (FALSE == LoginPage_GetData(hPage, &loginData))
						loginData = NULL;
					}
					else
						loginData->AddRef();
			
					HRESULT hr = command->BeginLogin(loginData, LoginBox_LoginCompletedEvent, hwnd, &loginbox->loginResult);
					authError = (SUCCEEDED(hr)) ? AUTH_SUCCESS : AUTH_UNEXPECTED;
					command->Release();

					if (NULL != loginData)
					loginData->Release();
				}
			}
		}
	}

	if (AUTH_SUCCESS != authError)
	{
		LoginBox_EnableLoginMode(hwnd, FALSE, FALSE);
		if (AUTH_ABORT != authError)
		{
			HWND hNotifier = GetDlgItem(hwnd, IDC_NOTIFIER);
			if (NULL != hNotifier)
			{
				LoginNotifier_Notify(hNotifier, NLNTYPE_ERROR, MAKEINTRESOURCE(authError));
				LoginBox_ShowNotifier(hwnd, 200);
			}
		}
	}
}

static INT LoginBox_SetCredentials(HWND hwnd, LoginCredentials *credentials)
{
	if (NULL == credentials)
		return AUTH_UNEXPECTED;

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox || NULL == loginbox->auth)
		return AUTH_UNEXPECTED;

	INT result = loginbox->auth->SetCredentials(credentials->GetRealm(), credentials->GetSessionKey(), 
					credentials->GetToken(), credentials->GetUsername(), credentials->GetExpiration());
	
	return result;
}

static INT LoginBox_RequestPasscode(HWND hwnd, LoginResult *loginResult)
{
	LoginData *loginData;
	if (NULL == loginResult || FAILED(loginResult->GetLoginData(&loginData)) || NULL == loginData)
		return AUTH_UNEXPECTED;
	
	HWND hCurtain = GetDlgItem(hwnd, IDC_CURTAIN);
	HWND hPopup = LoginPopupPasscode::CreatePopup((NULL != hCurtain) ? hCurtain : hwnd, loginData);
	if (NULL != hPopup)
	{
		SetWindowLongPtr(hPopup, GWLP_ID, IDC_POPUPPASSCODE);

		SetWindowPos(hPopup, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		ShowWindow(hPopup, SW_SHOW);

		if (NULL != hCurtain)
			SetWindowPos(hCurtain, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED );

		if (IsWindowVisible(hPopup) && IsWindowEnabled(hPopup))
		{
			HWND hRoot = GetAncestor(hPopup, GA_ROOT);
			if (NULL != hRoot)
				SendMessage(hRoot, WM_NEXTDLGCTL, (WPARAM)hPopup, TRUE);
			LoginPopup_PlayBeep(hPopup);
		}

	}

	loginData->Release();

	return (NULL != hPopup) ? AUTH_SUCCESS : AUTH_UNEXPECTED;
}

static BOOL LoginBox_EndDialog(HWND hwnd, INT authResult)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	
	if (0 != (NLBS_MODAL & loginbox->style))
	{
		EndDialog(hwnd, authResult);
	}
	else
	{
		DestroyWindow(hwnd);
	}
	return TRUE;
}

static BOOL LoginBox_ReloadProviders(HWND hwnd, INT *loaded, INT *prefVisible)
{
	if (NULL != loaded)
		*loaded = NULL;

	WCHAR szPath[MAX_PATH] = {0};
	if (FAILED(LoginBox_GetConfigPath(szPath, FALSE)))	
		return FALSE;

	if (FALSE == PathAppend(szPath, L"loginProviders.xml"))
		return FALSE;

	LoginProviderEnumerator *enumerator;
	LoginProviderLoader loader;

	if (FAILED(loader.ReadXml(szPath,&enumerator, prefVisible)))
		return FALSE;
	

	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL != hFrame) 
	{
		UINT frameStyle = GetWindowStyle(hFrame);
		if (0 != (WS_VISIBLE & frameStyle)) 
			SetWindowLongPtr(hFrame, GWL_STYLE, frameStyle & ~WS_VISIBLE);

		LoginBox_DeleteAllTabs(hwnd);
		INT r = LoginBox_AppendMultipleTabs(hwnd, enumerator, NULL, NULL);
		if (NULL != loaded)
			*loaded = r;

		if (0 != (WS_VISIBLE & frameStyle))
		{
			frameStyle = GetWindowStyle(hFrame);
			if (0 == (WS_VISIBLE & frameStyle))
				SetWindowLongPtr(hFrame, GWL_STYLE, frameStyle | WS_VISIBLE);
		}
	}

	enumerator->Release();
	return TRUE;
}


static void LoginBox_PerformProviderOp(HWND hwnd)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox || NULL == loginbox->providerOp)
		return;

	if (FALSE == LoginBox_EnableLoginMode(hwnd, TRUE, FALSE))
		return;

	HWND hCurtain = GetDlgItem(hwnd, IDC_CURTAIN);

	LPCWSTR pszMessage;
	switch(loginbox->providerOp->GetCode())
	{
		case LoginProviderOperation::operationReplace:
			pszMessage = MAKEINTRESOURCE(IDS_PROVIDER_CHANGED); break;
		case LoginProviderOperation::operationDelete:
			pszMessage = MAKEINTRESOURCE(IDS_PROVIDER_REMOVED); break;
		default:
			pszMessage = NULL;
			break;
	}
	HWND hPopup = LoginPopupMessage::CreatePopup((NULL != hCurtain) ? hCurtain : hwnd, 
					MAKEINTRESOURCE(IDS_PROVIDERUPDATE_TITLE), pszMessage, 
					LoginPopupMessage::typeContinue | LoginPopupMessage::iconWarning, NULL, NULL);
	if (NULL != hPopup)
	{
		SetWindowLongPtr(hPopup, GWLP_ID, IDC_POPUPPROVIDEROP);

		SetWindowPos(hPopup, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		ShowWindow(hPopup, SW_SHOW);

		if (NULL != hCurtain)
			SetWindowPos(hCurtain, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED );
		
		if (IsWindowVisible(hPopup) && IsWindowEnabled(hPopup))
		{
			HWND hRoot = GetAncestor(hPopup, GA_ROOT);
			if (NULL != hRoot)
				SendMessage(hRoot, WM_NEXTDLGCTL, (WPARAM)hPopup, TRUE);
			LoginPopup_PlayBeep(hPopup);
		}
	}
	else
	{
		LoginBox_EnableLoginMode(hwnd, FALSE, FALSE);
	}
}
static INT_PTR LoginBox_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{	
	LOGINBOXCREATEPARAM *createParam = (LOGINBOXCREATEPARAM*)param;
	LOGINBOX *login = (LOGINBOX*)calloc(1, sizeof(LOGINBOX));
	if (NULL == login) 
	{
		return 0;
	}

	SetProp(hwnd, LOGINBOX_PROP, login);

	if (NULL != createParam)
	{
		login->style = createParam->style;
		login->realm = (NULL == createParam->pRealm) ? GUID_NULL : *createParam->pRealm;
		login->auth = createParam->auth;
	}
	
	if (NULL != login->auth)
		login->auth->AddRef();

	LoginGuiObject::InitializeThread();

	HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);

	HWND hFrame = LoginTab_CreateWindow(0, L"Select provider:", 
					WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE, 0, 0, 0, 0, hwnd, IDC_TABFRAME);
	
	if (NULL != hFrame)
	{
		HIMAGELIST imageList = ImageList_Create(40, 40, ILC_COLOR32, 7*3, 3*3);
		if (NULL != imageList)
			LoginTab_SetImageList(hFrame, imageList);
	}

	HWND hNotifier = LoginNotifier_CreateWindow(0, WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 0, 0, hwnd, IDC_NOTIFIER);
	if (NULL != hNotifier) 
	{
		if (NULL != hFont)
			SendMessage(hNotifier, WM_SETFONT, (WPARAM)hFont, 0L);
		SetWindowPos(hNotifier, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	}
	
	HWND hStatus = GetDlgItem(hwnd, IDC_STATUS);
	if (NULL != hStatus)
	{
		LoginStatus::CreateInstance(hStatus, &login->loginStatus);
	}

	LoginGuiObject *loginGui;
	if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
	{
		HFONT fontButton = loginGui->GetTextFont();
		if (NULL != fontButton)
		{
			INT szButtons[] = {IDOK, IDCANCEL, };
			for (INT i = 0; i < ARRAYSIZE(szButtons); i++)
			{
				HWND hButton = GetDlgItem(hwnd, szButtons[i]);
				if(NULL != hButton) 
					SNDMSG(hButton, WM_SETFONT, (WPARAM)fontButton, 0L);
			}
		}
		loginGui->Release();
	}
	INT loaded, prefVisible, prefWidth;

	prefWidth = 0;
	if (FALSE == LoginBox_ReloadProviders(hwnd, &loaded, &prefVisible))
	{
		loaded = 0;
		prefVisible = 0;
	}

	if (NULL != hFrame && prefVisible > 0)
		prefWidth = LoginTab_GetIdealWidth(hFrame, prefVisible);

	if (NULL != hFrame)
	{
		INT activeId = -1;
		LoginConfig *config;
		if (SUCCEEDED(LoginConfig::CreateInstance(&config)))
		{
			RPC_CSTR pszRealm;
			if (RPC_S_OK == UuidToStringA(&login->realm, &pszRealm))
			{
				CHAR szProvider[128] = {0};
				config->ReadAnsiStr((LPCSTR)pszRealm, "lastProvider", NULL, szProvider, ARRAYSIZE(szProvider));
				if (NULL != szProvider && L'\0' != *szProvider)
				{
					GUID providerId;
					if (RPC_S_OK == UuidFromStringA((RPC_CSTR)szProvider, &providerId))
						activeId = LoginBox_FindTabByProviderId(hwnd, &providerId);
				}
				RpcStringFreeA(&pszRealm);
			}
			
			config->Release();
		}

		if (-1 == activeId)
			activeId = 0;

		LoginTab_SetCurSel(hFrame, activeId);
		LoginBox_SelectActivePage(hwnd);

	}

	LoginBox_UpdateMargins(hwnd);

	if (0 != prefWidth)
	{
		RECT rect;
		
		GetClientRect(hwnd, &rect);
		prefWidth -= rect.right - rect.left;

		GetWindowRect(hwnd, &rect);
		prefWidth += rect.right - rect.left;
		
		
		if (prefWidth < login->minmaxInfo.left) prefWidth = login->minmaxInfo.left;
		else if (prefWidth > login->minmaxInfo.right) prefWidth = login->minmaxInfo.right;

		SetWindowPos(hwnd, NULL, 0, 0, prefWidth, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
		
	}
	
	LoginBox_UpdateLayout(hwnd, FALSE);	
	
	if (NULL != createParam && 0 == (LBS_NOCENTEROWNER & createParam->style))
		LoginBox_CenterOver(hwnd, createParam->hOwner);

	SendMessage(hwnd, DM_REPOSITION, 0, 0L);
	
	PostMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);
	PostMessage(hwnd, NLBM_UPDATEPROVIDERS, (0 == loaded), 0L);

	return 0;
}

static void LoginBox_OnDestroy(HWND hwnd)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	RemoveProp(hwnd, LOGINBOX_PROP);

	if (NULL == loginbox)
		return;

	LoginConfig *config;
	if (SUCCEEDED(LoginConfig::CreateInstance(&config)))
	{
		RPC_CSTR pszRealm, pszProvider;
		if (RPC_S_OK == UuidToStringA(&loginbox->realm, &pszRealm))
		{
			GUID providerId;
			LoginProvider *activeProvider = LoginBox_GetActiveProviderInternal(hwnd);
			if (NULL == activeProvider || FAILED(activeProvider->GetId(&providerId)) ||
				RPC_S_OK != UuidToStringA(&providerId, &pszProvider))
			{
				pszProvider = NULL;
			}

			config->WriteAnsiStr((LPCSTR)pszRealm, "lastProvider", (LPCSTR)pszProvider);
			if (NULL != pszProvider)
				RpcStringFreeA(&pszProvider);

			RpcStringFreeA(&pszRealm);
		}
		
		config->Release();
	}

	if (NULL != loginbox->imageCache)
	{
		loginbox->imageCache->Finish();
		loginbox->imageCache->Release();
		loginbox->imageCache = NULL;
	}
	
	LoginBox_DeleteAllTabs(hwnd);

	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL != hFrame)
	{
		HIMAGELIST imageList = LoginTab_SetImageList(hFrame, NULL);
		if (NULL != imageList)
		{		
			ImageList_Destroy(imageList);
		}
	}

	if (NULL != loginbox->loginResult)
	{
		LoginResult *result = loginbox->loginResult;
		result->RequestAbort(TRUE);
		HANDLE resultAborted;
		if (SUCCEEDED(result->GetWaitHandle(&resultAborted)))
		{
			WaitForSingleObjectEx(resultAborted, INFINITE, TRUE);
			CloseHandle(resultAborted);
		}

		loginbox->loginResult = NULL;
		result->Release();
	}
	
	if (NULL != loginbox->auth)
		loginbox->auth->Release();

	if (NULL != loginbox->providerUpdate)
	{
		UINT state;
		if (FAILED(loginbox->providerUpdate->GetState(&state)) || 
			LoginDownloadResult::stateCompleted != state)
		{
			loginbox->providerUpdate->RequestAbort(TRUE);
			HANDLE completed;
			if (SUCCEEDED(loginbox->providerUpdate->GetWaitHandle(&completed)) && NULL != completed)
			{
				WaitForSingleObject(completed, INFINITE);
				CloseHandle(completed);
			}
		}
		loginbox->providerUpdate->Release();
	}

	if (NULL != loginbox->providerOp)
		loginbox->providerOp->Release();

	if (NULL != loginbox->loginStatus)
	{
		loginbox->loginStatus->DetachWindow();
		loginbox->loginStatus->Release();
	}

	LoginGuiObject::UninitializeThread();
	
	free(loginbox);
}

static void LoginBox_OnGetMinMaxInfo(HWND hwnd, MINMAXINFO *mmi)
{
	if (NULL == mmi) return;

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL != loginbox)
	{
		mmi->ptMinTrackSize.x = loginbox->minmaxInfo.left;
		mmi->ptMinTrackSize.y = loginbox->minmaxInfo.top;
		mmi->ptMaxTrackSize.x = loginbox->minmaxInfo.right;
		mmi->ptMaxTrackSize.y = loginbox->minmaxInfo.bottom;
	}
}

static void LoginBox_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return;

	if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) 
	{
		LoginBox_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags));   
	}
}

static void LoginBox_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{
		case IDOK:
			LoginBox_PerformLogin(hwnd, NULL);
			break;
		case IDCANCEL:
			LoginBox_EndDialog(hwnd, AUTH_ABORT);
			break;
		case ID_LOGINBOX_UPDATEPROVIDERS:
			LoginBox_UpdateProviders(hwnd, TRUE);
			break;
		case ID_LOGINTAB_RESETORDER:
			LoginTab_ResetOrder(GetDlgItem(hwnd, IDC_TABFRAME));
			break;
	}

}

static void LoginBox_OnTabSelected(HWND hwnd, HWND hFrame)
{
	LoginBox_SelectActivePage(hwnd);
}

static void LoginBox_OnTabDeleted(HWND hwnd, HWND hFrame, INT iItem)
{
	if (NULL == hFrame || iItem < 0) return;

	NLTITEM tab;
	tab.mask = NLTIF_PARAM | NLTIF_IMAGE_MASK;
	if (FALSE != LoginTab_GetItem(hFrame, iItem, &tab))
	{
		LoginBox_RemoveImageHelper2(hFrame, &tab);

		LoginProvider *provider = (LoginProvider*)tab.param;
		if (NULL != provider)
			provider->Release();

	}
}

static BOOL LoginBox_OnTabDeleteAll(HWND hwnd, HWND hFrame)
{
	if (NULL == hFrame) return FALSE;

	HIMAGELIST imageList = LoginTab_GetImageList(hFrame);
	if (NULL != imageList)
		ImageList_RemoveAll(imageList);

	NLTITEM tab;
	tab.mask = NLTIF_PARAM;

	INT iItem = LoginTab_GetItemCount(hFrame);
	while (iItem--)
	{
		if (FALSE != LoginTab_GetItem(hFrame, iItem, &tab))
		{
		
			LoginProvider *provider = (LoginProvider*)tab.param;
			if (NULL != provider)
				provider->Release();
		}
	}

	return TRUE;
}
static void LoginBox_OnTabRClick(HWND hwnd, HWND hFrame, POINT pt)
{
	HMENU hMenu = WASABI_API_LOADMENUW(IDR_MENU_TABCONTEXT);
	HMENU hContext = (NULL != hMenu) ? GetSubMenu(hMenu, 0) : NULL;
	if (NULL == hContext) return;
	
	MapWindowPoints(hFrame, HWND_DESKTOP, &pt, 1);

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	UINT fEnable = (NULL != loginbox && NULL == loginbox->providerUpdate) ? MF_ENABLED : MF_DISABLED;
	EnableMenuItem(hContext, ID_LOGINBOX_UPDATEPROVIDERS, fEnable | MF_BYCOMMAND);

	TrackPopupMenuEx(hContext, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_VERPOSANIMATION, pt.x, pt.y, hwnd, NULL);
	
}
static void LoginBox_OnTabHelp(HWND hwnd, HWND hFrame, NMLOGINTABHELP *plth)
{
	if (NULL == plth || NULL == plth->param)
		return;
	LoginProvider *provider = (LoginProvider*)plth->param;

	WCHAR szBuffer[2048] = {0};
	if (SUCCEEDED(provider->GetDescription(szBuffer, ARRAYSIZE(szBuffer))))
		plth->bstrHelp = SysAllocString(szBuffer);
}
static void LoginBox_OnTabImage(HWND hwnd, HWND hFrame, NMLOGINTABIMAGE *plti)
{
	if (NULL == plti || NULL == plti->imageList)
		return;

	if (NLTM_IMAGE_CALLBACK == plti->iImage)
	{
		plti->maskUpdate = NLTIF_IMAGE_MASK;
		plti->iImage = NLTM_IMAGE_NONE;
		plti->iImageActive = NLTM_IMAGE_NONE;
		plti->iImageDisabled = NLTM_IMAGE_NONE;

		LOGINBOX *loginbox = GetLoginBox(hwnd);
		if (NULL != loginbox)
		{
			if (NULL == loginbox->imageCache)
				LoginImageCache::CreateInstance(hwnd, &loginbox->imageCache);
			
			LoginProvider *provider = (LoginProvider*)plti->param;
			if (NULL != provider && NULL != loginbox->imageCache)
			{
				WCHAR szImage[4096] = {0};
				if (SUCCEEDED(provider->GetImagePath(szImage, ARRAYSIZE(szImage))))
				{
					loginbox->imageCache->GetImageListIndex(szImage, plti->imageList, 
							&plti->iImage, &plti->iImageActive, &plti->iImageDisabled);
				}
			}
		}
		return;
	}

	if (0 != (NLTIF_IMAGE_ACTIVE & plti->maskRequest))
	{
		plti->iImageActive = plti->iImage;
		plti->maskUpdate |= NLTIF_IMAGE_ACTIVE;
	}

	if (0 != (NLTIF_IMAGE_DISABLED & plti->maskRequest))
	{
		plti->iImageDisabled = plti->iImage;
		plti->maskUpdate |= NLTIF_IMAGE_DISABLED;

	}
}
static void LoginBox_OnPopupAgreementResult(HWND hwnd, NLPNRESULT *pnr)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return;

	HWND hCurtain = GetDlgItem(hwnd, IDC_CURTAIN);
	UINT curtainStyle = (NULL != hCurtain) ? GetWindowStyle(hCurtain) : 0;
	if (0 != (WS_VISIBLE & curtainStyle))
		SetWindowLongPtr(hCurtain, GWL_STYLE, curtainStyle & ~WS_VISIBLE);

	UINT statusCookie = (UINT)(UINT_PTR)GetProp(pnr->hdr.hwndFrom, L"StatusCookie");
	RemoveProp(pnr->hdr.hwndFrom, L"StatusCookie");
	if (0 != statusCookie)
		loginbox->loginStatus->Remove(statusCookie - 1);

	DestroyWindow(pnr->hdr.hwndFrom);
	

	if (IDOK == pnr->exitCode)
	{
		loginbox->agreementOk = TRUE;
		LoginBox_PerformLogin(hwnd, NULL); // this will restore curtain visibility
	}
	else
	{
		loginbox->agreementOk = FALSE;
			
		RECT invalidRect;
		if (NULL != hCurtain)
		{		
			GetWindowRect(hCurtain, &invalidRect);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&invalidRect, 2);
		}
		else
			SetRectEmpty(&invalidRect);

		LoginBox_EnableLoginMode(hwnd, FALSE, FALSE);
		if (FALSE == IsRectEmpty(&invalidRect))
			RedrawWindow(hwnd, &invalidRect, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW);

		LoginBox_PerformProviderOp(hwnd);
	}
}

static void LoginBox_OnPopupPasscodeResult(HWND hwnd, NPPNRESULT *pnr)
{
	HWND hCurtain = GetDlgItem(hwnd, IDC_CURTAIN);
	UINT curtainStyle = (NULL != hCurtain) ? GetWindowStyle(hCurtain) : 0;
	if (0 != (WS_VISIBLE & curtainStyle))
		SetWindowLongPtr(hCurtain, GWL_STYLE, curtainStyle & ~WS_VISIBLE);

	DestroyWindow(pnr->hdr.hwndFrom);

	if (IDOK == pnr->exitCode)
	{				
		LoginBox_PerformLogin(hwnd, pnr->loginData); // this will restore curtain visibility
	}
	else
	{
		RECT invalidRect;
		if (NULL != hCurtain)
		{		
			GetWindowRect(hCurtain, &invalidRect);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&invalidRect, 2);
		}
		else
			SetRectEmpty(&invalidRect);

		LoginBox_EnableLoginMode(hwnd, FALSE, FALSE);
		if (FALSE == IsRectEmpty(&invalidRect))
			RedrawWindow(hwnd, &invalidRect, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW);

		LoginBox_PerformProviderOp(hwnd);
	}
}

static void LoginBox_OnPopupProviderOpResult(HWND hwnd, NLPNRESULT *pnr)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return;

	HWND hCurtain = GetDlgItem(hwnd, IDC_CURTAIN);
	UINT curtainStyle = (NULL != hCurtain) ? GetWindowStyle(hCurtain) : 0;
	if (0 != (WS_VISIBLE & curtainStyle))
		SetWindowLongPtr(hCurtain, GWL_STYLE, curtainStyle & ~WS_VISIBLE);
	
	DestroyWindow(pnr->hdr.hwndFrom);
				
	RECT invalidRect;
	if (NULL != hCurtain)
	{		
		GetWindowRect(hCurtain, &invalidRect);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&invalidRect, 2);
	}
	else
		SetRectEmpty(&invalidRect);

	LoginBox_EnableLoginMode(hwnd, FALSE, FALSE);
	if (FALSE == IsRectEmpty(&invalidRect))
		RedrawWindow(hwnd, &invalidRect, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW);

	if (NULL != loginbox->providerOp)
	{		
		LoginProvider *source, *target;
		GUID sourceId;
		if(SUCCEEDED(loginbox->providerOp->GetSource(&source)) && NULL != source)
		{
			if (FAILED(source->GetId(&sourceId)))
				sourceId = GUID_NULL;

			source->Release();
		
			switch(loginbox->providerOp->GetCode())
			{
				case LoginProviderOperation::operationReplace:
					if(SUCCEEDED(loginbox->providerOp->GetTarget(&target)))
					{
						LoginBox_ReplaceProvider(hwnd, &sourceId, target);
						target->Release();
					}
					break;
				case LoginProviderOperation::operationDelete:
					LoginBox_DeleteProvider(hwnd, &sourceId);
					break;
			}
		}

		loginbox->providerOp->Release();
		loginbox->providerOp = NULL;
	}
}

static LRESULT LoginBox_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	switch(controlId)
	{
		case IDC_TABFRAME:
			switch(pnmh->code)
			{
				case NLTN_SELCHANGE:
					LoginBox_OnTabSelected(hwnd, pnmh->hwndFrom);
					break;
				case NLTN_DELETEITEM:
					LoginBox_OnTabDeleted(hwnd, pnmh->hwndFrom, ((NMLOGINTAB*)pnmh)->iItem);
					break;
				case NLTN_DELETEALLITEMS:
					return LoginBox_OnTabDeleteAll(hwnd, pnmh->hwndFrom);
				case NM_RCLICK:
					LoginBox_OnTabRClick(hwnd, pnmh->hwndFrom, ((NMLOGINTABCLICK*)pnmh)->pt);
					break;
				case NLTN_GETITEMHELP:
					LoginBox_OnTabHelp(hwnd, pnmh->hwndFrom, (NMLOGINTABHELP*)pnmh);
					break;
				case NLTN_GETITEMIMAGE:
					LoginBox_OnTabImage(hwnd, pnmh->hwndFrom, (NMLOGINTABIMAGE*)pnmh);
					break;
			}
			break;
		case IDC_POPUPAGREEMENT:
			switch(pnmh->code)
			{
				case NLPN_RESULT:
					LoginBox_OnPopupAgreementResult(hwnd, (NLPNRESULT*)pnmh);
					break;
			}
			break;
		case IDC_POPUPPASSCODE:
			switch(pnmh->code)
			{
				case NPPN_RESULT:
					LoginBox_OnPopupPasscodeResult(hwnd, (NPPNRESULT*)pnmh);
					break;
			}
			break;
		case IDC_POPUPPROVIDEROP:
			switch(pnmh->code)
			{
				case NLPN_RESULT:
					LoginBox_OnPopupProviderOpResult(hwnd, (NLPNRESULT*)pnmh);
					break;
			}
			break;

	}
	return 0;
}

static INT_PTR LoginBox_OnNcHitTest(HWND hwnd, POINTS pts)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL != loginbox)
	{
		POINT pt;
		POINTSTOPOINT(pt, pts);
		MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);

		LONG offsetX = (loginbox->gripRect.right - loginbox->gripRect.left)/3;
		LONG offsetY = (loginbox->gripRect.bottom - loginbox->gripRect.top)/3;
		if (pt.x < loginbox->gripRect.right && pt.y < loginbox->gripRect.bottom &&
			pt.x > (loginbox->gripRect.left + offsetX) && pt.y > (loginbox->gripRect.top + offsetY))
		{
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, HTBOTTOMRIGHT);
			return TRUE;
		}
	}
	return FALSE;
}


static void LoginBox_OnSetFont(HWND hwnd, HFONT hFont, BOOL fRedraw)
{
	DefWindowProc(hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(fRedraw, 0));
	LoginBox_UpdateMargins(hwnd);
}
static void LoginBox_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			LoginBox_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void LoginBox_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	if (0 != (PRF_CLIENT & options))
	{
		RECT clientRect;
		if (GetClientRect(hwnd, &clientRect))
		LoginBox_Paint(hwnd, hdc, &clientRect, TRUE);
	}
}
static HBRUSH LoginBox_OnGetStaticColor(HWND hwnd, HDC hdc, HWND hControl)
{
	HBRUSH hb;
	INT_PTR controlId = (INT_PTR)GetWindowLongPtr(hControl, GWLP_ID);
	switch(controlId)
	{
		case IDC_STATUS:
			hb = (HBRUSH)SendMessage(hwnd, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hwnd);
			SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
			return hb;
	}
	return 0;
}

static BOOL LoginBox_OnGetAuthApi(HWND hwnd, api_auth **authApi)
{
	if (NULL == authApi) return FALSE;

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox || NULL == loginbox->auth)
	{
		*authApi = NULL;
		return FALSE;
	}

	*authApi = loginbox->auth;
	loginbox->auth->AddRef();

	return TRUE;
}

static BOOL LoginBox_OnGetRealm(HWND hwnd, GUID *realm)
{
	if (NULL == realm) return FALSE;

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox)
	{
		*realm = GUID_NULL;
		return FALSE;
	}

	*realm = loginbox->realm;
	return TRUE;
}

static BOOL LoginBox_OnGetActiveProvider(HWND hwnd, LoginProvider **provider)
{
	if (NULL == provider)
		return FALSE;
	
	*provider = LoginBox_GetActiveProviderInternal(hwnd);
	if (NULL == (*provider))
		return FALSE;
	
	(*provider)->AddRef();
	return TRUE;
}

static void LoginBox_OnLoginCompleted(HWND hwnd, LoginResult *result)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox || NULL == result || loginbox->loginResult != result) 
		return;

	INT authError = AUTH_ABORT;
	LoginProvider *provider = LoginBox_GetActiveProviderInternal(hwnd);
	if (NULL != provider) 
	{
		LoginCommand *command;
		if (SUCCEEDED(provider->GetCommand(&command)))
		{
			LoginCredentials *credentials;
			if (SUCCEEDED(command->EndLogin(result, &authError, &credentials)))
			{
				switch(authError)
				{
					case AUTH_SUCCESS:
						authError = LoginBox_SetCredentials(hwnd, credentials);
						LoginBox_SaveUsername(&loginbox->realm, provider, credentials);
						break;

					case AUTH_SECURID:
						authError = LoginBox_RequestPasscode(hwnd, result);
						if (AUTH_SUCCESS == authError)
							authError = AUTH_SECURID;
						break;
				}

				if (NULL != credentials)
					credentials->Release();
			}
			command->Release();
		}
	}

	
	loginbox->loginResult = NULL;
	result->Release();

	switch(authError)
	{
		case AUTH_SUCCESS:
			LoginBox_EndDialog(hwnd, AUTH_SUCCESS);
			break;
		case AUTH_SECURID:
			break;
		default:
			LoginBox_EnableLoginMode(hwnd, FALSE, FALSE);
			if (AUTH_ABORT != authError)
			{
				HWND hNotifier = GetDlgItem(hwnd, IDC_NOTIFIER);
				if (NULL != hNotifier)
				{
					LoginNotifier_Notify(hNotifier, NLNTYPE_ERROR, MAKEINTRESOURCE(authError));
					LoginBox_ShowNotifier(hwnd, 200);
				}
			}

			LoginBox_PerformProviderOp(hwnd);
			break;
	}
	
}

static BOOL LoginBox_OnGetStatus(HWND hwnd, LoginStatus **loginStatus)
{
	if (NULL == loginStatus)
		return FALSE;

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL != loginbox && NULL !=loginbox->loginStatus)
	{
		*loginStatus = loginbox->loginStatus;
		(*loginStatus)->AddRef();
		return TRUE;
	}

	return FALSE;
}
static UINT LoginBox_OnAddStatus(HWND hwnd, BSTR pszStatus)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL != loginbox && NULL !=loginbox->loginStatus)
		return loginbox->loginStatus->Add(pszStatus);

	return 0;
}

static BOOL LoginBox_OnSetStatus(HWND hwnd, UINT cookie, BSTR pszStatus)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL != loginbox && NULL !=loginbox->loginStatus)
		return loginbox->loginStatus->Set(cookie, pszStatus);

	return TRUE;
}

static void LoginBox_OnRemoveStatus(HWND hwnd, UINT cookie)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL != loginbox && NULL !=loginbox->loginStatus)
		loginbox->loginStatus->Remove(cookie);
}

static BOOL LoginBox_ApplyProvidersUpdate(HWND hwnd, LoginProviderEnumerator *providerEnum)
{
	if (NULL == providerEnum)
		return FALSE;
			
	GUID activeId(GUID_NULL);
	INT activeIndex(-1);
	LoginProviderOperation *providerOp(NULL);
	LoginProvider *providerActive = LoginBox_GetActiveProviderInternal(hwnd);
	if (NULL != providerActive)
	{		
		providerActive->GetId(&activeId);

		if (S_OK != LoginProviderOperation::CreateFromUpdate(providerActive, providerEnum, &providerOp))
			providerOp = NULL;
	}
			
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL != hFrame) 
	{
		UINT frameStyle = GetWindowStyle(hFrame);
		if (0 != (WS_VISIBLE & frameStyle)) 
			SetWindowLongPtr(hFrame, GWL_STYLE, frameStyle & ~WS_VISIBLE);

		LoginBox_DeleteAllTabs(hwnd);
		providerEnum->Reset();
		
		if(NULL != providerOp)
			LoginBox_AppendMultipleTabs(hwnd, providerEnum, &activeId, &activeIndex);
		else
			LoginBox_AppendMultipleTabs(hwnd, providerEnum, NULL, NULL);
		
		LoginProvider *provider;
		if (NULL != providerOp && SUCCEEDED(providerOp->GetSource(&provider)))
		{
			LoginBox_InsertTab(hwnd, activeIndex, provider);
			provider->Release();
		}
		
		activeIndex = (FALSE == IsEqualGUID(activeId, GUID_NULL)) ?
			LoginBox_FindTabByProviderId(hwnd, &activeId) : -1;

		if (-1 != activeIndex) 
		{
			LoginTab_SetCurSel(hFrame, activeIndex);
		}
		else
		{
			LoginTab_SetCurSel(hFrame, 0);
			LoginBox_SelectActivePage(hwnd);
		}
								
		if (0 != (WS_VISIBLE & frameStyle))
		{
			frameStyle = GetWindowStyle(hFrame);
			if (0 == (WS_VISIBLE & frameStyle))
				SetWindowLongPtr(hFrame, GWL_STYLE, frameStyle | WS_VISIBLE);

			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
			UpdateWindow(hFrame);
		}
	}

	if (NULL != providerOp)
	{
		LOGINBOX *loginbox = GetLoginBox(hwnd);
		if (NULL != loginbox)
		{
			if (NULL != loginbox->providerOp)
				loginbox->providerOp->Release();
			loginbox->providerOp = providerOp;
			providerOp->AddRef();

		}
		providerOp->Release();
	}
	LoginBox_PerformProviderOp(hwnd);

	return TRUE;
}

static void LoginBox_OnProvidersUpdated(HWND hwnd, PROVIDERUPDATERESULT *result)
{
	if (NULL == result)
		return;

	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox) return;
	

	BOOL dataIdentical;
	LoginProviderEnumerator *providerEnum;
	HRESULT resultCode = result->errorCode;

	if (SUCCEEDED(resultCode))
	{
		providerEnum = result->enumerator;
		if(NULL != providerEnum) providerEnum->AddRef();

		dataIdentical = result->dataIdentical;
	}
	else
	{	
		providerEnum = NULL;
		dataIdentical = FALSE;
	}

	BOOL storedUpdate = (NULL != loginbox->providerUpdate && 
						loginbox->providerUpdate == result->downloader);

	ReplyMessage(0);

	if (SUCCEEDED(resultCode))
	{		
		BOOL updateOk;

		if(FALSE == dataIdentical)
			updateOk = LoginBox_ApplyProvidersUpdate(hwnd, providerEnum);
		else
			updateOk = TRUE;
		
		if(FALSE != updateOk)
		{
			LoginConfig *config;
			if (SUCCEEDED(LoginConfig::CreateInstance(&config)))
			{
				UINT ts = LoginBox_GetCurrentTime();
				config->WriteInt("LoginBox", "lastCheck", ts);
			
				LPSTR lang;
				if (FAILED(LoginBox_GetCurrentLang(&lang)))
					lang = NULL;
										
				config->WriteAnsiStr("LoginBox", "lastLang", lang);
				LoginBox_FreeAnsiString(lang);
				
				config->Release();
			}
		}

	}

	if (NULL != providerEnum)
		providerEnum->Release();

	if (FALSE != storedUpdate)
	{
		loginbox->providerUpdate->Release();
		loginbox->providerUpdate = NULL;
	
		HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
		if (NULL != hPage)
			LoginPage_UpdateStateChange(hPage, FALSE);
	}
}

static void CALLBACK LoginBox_DownloadCompleted(LoginDownloadResult *result, void *data)
{
	HWND hLoginbox = (HWND)data;
	if(NULL == result || FALSE == IsWindow(hLoginbox)) return;

	UINT type;
	if (FAILED(result->GetType(&type)) ||  LoginDownloadResult::typeProviderList != type)
		return;

	BSTR fileName = NULL;
	LoginDownload download;
	HRESULT hr = download.End(result, &fileName);
	
	LoginProviderEnumerator *enumerator = NULL;
	
	if (SUCCEEDED(hr))
	{		
		if (S_FALSE != hr)
		{
			LoginProviderLoader loader;
			hr = loader.ReadXml(fileName, &enumerator, NULL);
			if (FAILED(hr))
				enumerator = NULL;
		}
	}

	LoginBox_ProvidersUpdated(hLoginbox, result, hr, (S_FALSE == hr), enumerator);

	if (NULL != enumerator)
			enumerator->Release();

	SysFreeString(fileName);
		
}

static BOOL LoginBox_OnUpdateProviders(HWND hwnd, BOOL forceUpdate)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	if (NULL == loginbox || NULL != loginbox->providerUpdate)
		return FALSE;

	if (FALSE == forceUpdate)
	{
		LoginConfig *config;
		if (SUCCEEDED(LoginConfig::CreateInstance(&config)))
		{
			BOOL checkRequired = TRUE;
			UINT ts = LoginBox_GetCurrentTime();
			UINT lastCheck = config->ReadInt("LoginBox", "lastCheck", 0);

			if ((ts - lastCheck) < (60*60*24)) // check once 24 hours
			{
				checkRequired = FALSE;
				
				LPSTR lang;
				if (SUCCEEDED(LoginBox_GetCurrentLang(&lang)))
				{
					CHAR lastLang[32] = {0};
					config->ReadAnsiStr("LoginBox", "lastLang", NULL, lastLang, ARRAYSIZE(lastLang));
					checkRequired = (NULL == lang || NULL == lastLang) ?
						((NULL == lang) != (NULL == lastLang)) :
						(CSTR_EQUAL != CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, lang, -1, lastLang, -1));

					LoginBox_FreeAnsiString(lang);
				}
			}
		
			config->Release();

			if (FALSE  == checkRequired)
				return FALSE;
		}
	}

	WCHAR szUrl[4096] = {0};
	LPWSTR cursor = szUrl;
	size_t remaining = ARRAYSIZE(szUrl);
	
	HRESULT hr;
	hr = StringCchCopyEx(cursor, remaining, PROVIDERLIST_URL, &cursor, &remaining, 0);
	if (SUCCEEDED(hr))
	{
		if (NULL != WASABI_API_LNG)
		{
			LPCWSTR lang = WASABI_API_LNG->GetLanguageIdentifier(LANG_LANG_CODE);
			if (NULL != lang && L'\0' != *lang)
			{
				hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, 0, L"?lang=%s", lang);
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
		if (NULL != hPage)
			LoginPage_UpdateStateChange(hPage, TRUE);

		LoginDownload download;
		hr = download.Begin(szUrl, LoginDownloadResult::typeProviderList, LoginBox_DownloadCompleted, 
						hwnd, loginbox->loginStatus, &loginbox->providerUpdate);
		if (FAILED(hr))
		{
			loginbox->providerUpdate = NULL;
			if (NULL != hPage)
				LoginPage_UpdateStateChange(hPage, FALSE);
		}
	}

	return SUCCEEDED(hr);
}

static BOOL LoginBox_OnGetUpdateState(HWND hwnd)
{
	LOGINBOX *loginbox = GetLoginBox(hwnd);
	return (NULL != loginbox && NULL != loginbox->providerUpdate);
}

static void LoginBox_OnImageCached(HWND hwnd, IMAGECACHERESULT *result)
{
	LoginImageCache *cache = NULL;
	ifc_omcacherecord *record = NULL;
	if (NULL != result)
	{
		cache = result->imageCache;
		record = result->cacheRecord;
	}

	if (NULL != cache) cache->AddRef();
	if (NULL != record) record->AddRef();

	ReplyMessage(0);

	if (NULL != cache && NULL != record)
	{
		WCHAR szName[4096] = {0};
		if (SUCCEEDED(record->GetName(szName, ARRAYSIZE(szName))))
		{
			HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
			if (NULL != hFrame)
			{
				NLTITEM tab;
				tab.mask = NLTIF_PARAM;
				INT count = LoginTab_GetItemCount(hFrame);
				for (INT i = 0; i < count; i++)
				{
					if (FALSE != LoginTab_GetItem(hFrame, i, &tab))
					{
						WCHAR szImage[4096] = {0};
						LoginProvider *provider = (LoginProvider*)tab.param;
						if (NULL != provider && 
							SUCCEEDED(provider->GetImagePath(szImage, ARRAYSIZE(szImage))) &&
							S_OK == LoginBox_IsStrEqInvI(szImage, szName))
						{
			
							
							LoginBox_RemoveImageHelper2(hFrame, &tab);

							tab.mask = NLTIF_IMAGE_MASK;
							tab.iImage = NLTM_IMAGE_CALLBACK;
							tab.iImageActive = NLTM_IMAGE_CALLBACK;
							tab.iImageDisabled = NLTM_IMAGE_CALLBACK;
							LoginTab_SetItem(hFrame, i, &tab);
							tab.mask = NLTIF_PARAM;
						}
					}
				}
			}
		}

	}

	if (NULL != cache)  cache->Release();
	if (NULL != record)	record->Release();
}

static INT_PTR CALLBACK LoginBox_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return LoginBox_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:			LoginBox_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	LoginBox_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:			LoginBox_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_NOTIFY:				MSGRESULT(hwnd, LoginBox_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam));
		case WM_GETMINMAXINFO:		LoginBox_OnGetMinMaxInfo(hwnd, (MINMAXINFO*)lParam); return TRUE;
		case WM_ERASEBKGND:			MSGRESULT(hwnd, 0);
		case WM_PAINT:				LoginBox_OnPaint(hwnd); return TRUE;
		case WM_PRINTCLIENT:		LoginBox_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return TRUE;
		case WM_CTLCOLORSTATIC:		return (INT_PTR)LoginBox_OnGetStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_NCHITTEST:			return LoginBox_OnNcHitTest(hwnd, MAKEPOINTS(lParam));
		case WM_SETFONT:			LoginBox_OnSetFont(hwnd, (HFONT)wParam, LOWORD(lParam)); return TRUE;
		case WM_SIZE:				MSGRESULT(hwnd, 0);
		
		case NLBM_GETAUTHAPI:		MSGRESULT(hwnd, LoginBox_OnGetAuthApi(hwnd, (api_auth**)lParam));
		case NLBM_GETREALM:			MSGRESULT(hwnd, LoginBox_OnGetRealm(hwnd, (GUID*)lParam));
		case NLBM_GETACTIVEPROVIDER:MSGRESULT(hwnd, LoginBox_OnGetActiveProvider(hwnd, (LoginProvider**)lParam));
		case NLBM_GETSTATUS:		MSGRESULT(hwnd, LoginBox_OnGetStatus(hwnd, (LoginStatus**)lParam));
		case NLBM_ADDSTATUS:		MSGRESULT(hwnd, LoginBox_OnAddStatus(hwnd, (BSTR)lParam));
		case NLBM_SETSTATUS:		MSGRESULT(hwnd, LoginBox_OnSetStatus(hwnd, (UINT)wParam, (BSTR)lParam));
		case NLBM_REMOVESTATUS:		LoginBox_OnRemoveStatus(hwnd, (UINT)wParam);
		case NLBM_UPDATEPROVIDERS:	MSGRESULT(hwnd, LoginBox_OnUpdateProviders(hwnd, (BOOL)wParam));
		case NLBM_GETUPDATESTATE:	MSGRESULT(hwnd, LoginBox_OnGetUpdateState(hwnd));
		case NLBM_LOGINCOMPLETED:	LoginBox_OnLoginCompleted(hwnd, (LoginResult*)lParam); return TRUE;
		case NLBM_PROVIDERSUPDATED:	LoginBox_OnProvidersUpdated(hwnd, (PROVIDERUPDATERESULT*)lParam); return TRUE;
		case NLBM_IMAGECACHED:		LoginBox_OnImageCached(hwnd, (IMAGECACHERESULT*)lParam); return TRUE;
	}

	return FALSE;
}
