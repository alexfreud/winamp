#define OEMRESOURCE

#include "./loginPage.h"
#include "./loginData.h"
#include "./loginBox.h"
#include "./loginProvider.h"
#include "./loginGui.h"
#include "./common.h"
#include "../api.h"

#include "../../nu/windowsTheme.h"

#include <vssym32.h>
#include <vsstyle.h>
#include <windows.h>

#define IDC_TITLE			9999
#define IDC_HELPLINK		9998

typedef struct __LOGINPAGECREATEPARAM
{
	LOGINPAGECREATOR fnCreator;
	LPARAM  lParam;
	HWND hLoginbox;
} LOGINPAGECREATEPARAM;

LoginPage::LoginPage(HWND hwnd, HWND hLoginbox)
{
	this->hwnd = hwnd;
	this->hLoginbox = hLoginbox;
}

LoginPage::~LoginPage()
{
	
}

HWND LoginPage::CreatePage(HWND hLoginbox, LPCWSTR pszTemplate, HWND hParent, LPARAM param, LOGINPAGECREATOR fnCreator)
{
	if (NULL == hLoginbox || NULL == hParent)
		return NULL;

	if (NULL == pszTemplate || NULL == fnCreator)
		return NULL;

	LOGINPAGECREATEPARAM createParam;
	createParam.fnCreator = fnCreator;
	createParam.lParam = param;
	createParam.hLoginbox = hLoginbox;

	return WASABI_API_CREATEDIALOGPARAMW((INT)(INT_PTR)pszTemplate, hParent, LoginPage_DialogProc, (LPARAM)&createParam);
}

void LoginPage::UpdateColors()
{
	rgbTitle = RGB(0, 51, 153);
	rgbSecondaryText = GetSysColor(COLOR_WINDOWTEXT);
	rgbText = GetSysColor(COLOR_WINDOWTEXT);
	rgbBack = GetSysColor(COLOR_WINDOW);
	hbrBack = GetSysColorBrush(COLOR_WINDOW);

	if (SUCCEEDED(UxTheme_LoadLibrary()) && FALSE != UxIsAppThemed())
	{
		UXTHEME hTheme = UxOpenThemeData(hwnd, L"TEXTSTYLE");
		if (NULL != hTheme)
		{
			UxGetThemeColor(hTheme, TEXT_MAININSTRUCTION, 0, TMT_TEXTCOLOR, &rgbTitle);
			UxGetThemeColor(hTheme, TEXT_BODYTEXT, 0, TMT_TEXTCOLOR, &rgbText);
			UxGetThemeColor(hTheme, TEXT_SECONDARYTEXT, 0, TMT_TEXTCOLOR, &rgbSecondaryText);
			UxCloseThemeData(hTheme);
		}
	}
}

void LoginPage::UpdateMargins()
{
	SetRect(&margins, 14, 7, 7, 7);
	MapDialogRect(hwnd, &margins);

	RECT controlRect;
	HWND hControl = GetDlgItem(hwnd, IDC_HELPLINK);
	if (NULL != hControl && GetWindowRect(hControl, &controlRect))
	{
	
		INT t = (controlRect.right - controlRect.left) + 1;
		if (margins.right < t) margins.right = t;

		t = (controlRect.bottom - controlRect.top) + 1;
		if (margins.top < t) margins.top = t;
	}
}

static HBITMAP LoginPage_GetHelpBitmap(HWND hwnd, HBRUSH hbrBack, INT *pWidth, INT *pHeight)
{
	LoginGuiObject *loginGui;
	if (FAILED(LoginGuiObject::QueryInstance(&loginGui)))
		return NULL;
	
	
	RECT rectSrc;
	HBITMAP hbmpSrc = loginGui->GetIcon(LoginGuiObject::iconQuestion, &rectSrc);
	HBITMAP hbmpDst = NULL;
	if (NULL != hbmpSrc)
	{		
		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if( NULL != hdc)
		{
			HDC hdcDst = CreateCompatibleDC(hdc);
			HDC hdcSrc = CreateCompatibleDC(hdc);
			if (NULL != hdcDst && NULL != hdcSrc)
			{				
				INT imageWidth = rectSrc.right - rectSrc.left;
				INT imageHeight = rectSrc.bottom - rectSrc.top;

				BITMAPINFOHEADER header;
				ZeroMemory(&header, sizeof(BITMAPINFOHEADER));

				header.biSize = sizeof(BITMAPINFOHEADER);
				header.biCompression = BI_RGB;
				header.biBitCount = 24;
				header.biPlanes = 1;
				header.biWidth = imageWidth;
				header.biHeight = -imageHeight;
				void *pixelData;

				hbmpDst = CreateDIBSection(hdc, (LPBITMAPINFO)&header, DIB_RGB_COLORS, (void**)&pixelData, NULL, 0);
				if (NULL != hbmpDst)
				{
					HBITMAP hbmpDstOrig  = (HBITMAP)SelectObject(hdcDst, hbmpDst);
					HBITMAP hbmpSrcOrig  = (HBITMAP)SelectObject(hdcSrc, hbmpSrc);

					RECT fillRect;
					SetRect(&fillRect, 0, 0, imageWidth, imageHeight);
					FillRect(hdcDst, &fillRect, hbrBack);

					BLENDFUNCTION bf;
					bf.BlendOp = AC_SRC_OVER;
					bf.BlendFlags = 0;
					bf.SourceConstantAlpha = 255;
					bf.AlphaFormat = AC_SRC_ALPHA;

					GdiAlphaBlend(hdcDst, 0, 0, imageWidth, imageHeight, 
								hdcSrc, rectSrc.left, rectSrc.top, imageWidth, imageHeight, bf);
					
					SelectObject(hdcDst, hbmpDstOrig);
					SelectObject(hdcSrc, hbmpSrcOrig);
					
					if (NULL != pWidth) *pWidth = imageWidth;
					if (NULL != pHeight) *pHeight = imageHeight;
				}
			}

			if (NULL != hdcDst) DeleteDC(hdcDst);
			if (NULL != hdcSrc) DeleteDC(hdcSrc);
			ReleaseDC(hwnd, hdc);
		}
	}
	loginGui->Release();
	return hbmpDst;
}

void LoginPage::UpdateLayout(BOOL fRedraw)
{
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	UINT flags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE == fRedraw) flags |= SWP_NOREDRAW;

	const INT szControls[] = { IDC_HELPLINK, IDC_TITLE};
	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szControls));

	RECT rect;
	INT cx, cy, x, y;

	for (INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		HWND hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL == hControl || FALSE == GetWindowRect(hControl, &rect)) continue;
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
		x = rect.left;
		y = rect.top;
		cx = rect.right - rect.left;
		cy = rect.bottom - rect.top;

		switch(szControls[i])
		{
			case IDC_HELPLINK:
				x = clientRect.right - cx - 1;
				if (x < clientRect.left + 1) x = clientRect.left + 1;
				y = clientRect.top + 1;
				break;
			case IDC_TITLE:
				x = clientRect.left + margins.left;
				y = clientRect.top + margins.top;
				cx = clientRect.right - margins.right - x;
				cy = 0;
				
				LoginBox_GetWindowTextSize(hControl, cx, &cx, &cy);
				
				if ((cx + x) > (clientRect.right - margins.right)) 
					cx = clientRect.right - margins.right - x;
				if ((cy + y) > (clientRect.bottom - margins.bottom))
					cy = clientRect.bottom - margins.bottom - y;
					break;
		}
		
		hdwp = DeferWindowPos(hdwp, hControl, NULL, x, y, cx, cy, flags);
		if (NULL == hdwp) break;
	}

	if (NULL != hdwp)
		EndDeferWindowPos(hdwp);
}

BOOL LoginPage::GetPageRect(RECT *prc)
{
	if (NULL == prc || FALSE == GetClientRect(hwnd, prc))
		return FALSE;
	
	prc->left += margins.left;
	prc->top += margins.top;
	prc->right -= margins.right;
	prc->bottom -= margins.bottom;

	HWND hTitle = GetDlgItem(hwnd, IDC_TITLE);
	if (NULL != hTitle)
	{
		UINT titleStyle = GetWindowStyle(hTitle);
		if (0 != (WS_VISIBLE & titleStyle))
		{
			RECT titleRect;
			if (FALSE != GetWindowRect(hTitle, &titleRect))
			{
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&titleRect, 2);
				
				titleRect.bottom += GetTitleSpacing();
											
				if (titleRect.bottom > prc->top)
				{
					prc->top = titleRect.bottom;
					if (prc->top > prc->bottom)
						prc->top = prc->bottom;
				}
			}
		}
	}
	return TRUE;
}

INT LoginPage::GetTitleSpacing()
{
	HWND hTitle = GetDlgItem(hwnd, IDC_TITLE);
	if (NULL == hTitle) return 0;

	HFONT fontTitle = (HFONT)SendMessage(hTitle, WM_GETFONT, 0, 0L);

	HDC hdc = GetDCEx(hTitle, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return 0;
		
	HFONT fontOrig = (HFONT)SelectObject(hdc, fontTitle);

	TEXTMETRIC tm;
	if (FALSE == GetTextMetrics(hdc, &tm))
		tm.tmHeight = 0;
		
	SelectObject(hdc, fontOrig);
	ReleaseDC(hTitle, hdc);

	return tm.tmHeight;
}


BOOL LoginPage::IsHelpAvailable()
{
	LoginProvider *provider;
	if (NULL == hLoginbox || 
		FALSE == LoginBox_GetActiveProvider(hLoginbox, &provider) ||
		NULL == provider)
	{
		return FALSE;
	}

	WCHAR szBuffer[8192] = {0};
	
	HRESULT hr = provider->GetHelpLink(szBuffer, ARRAYSIZE(szBuffer));
	provider->Release();

	if (FAILED(hr) || L'\0' == szBuffer[0])
		return FALSE;

	return TRUE;
}

BOOL LoginPage::ShowHelp()
{
	LoginProvider *provider;
	if (NULL == hLoginbox || 
		FALSE == LoginBox_GetActiveProvider(hLoginbox, &provider) ||
		NULL == provider)
	{
		return FALSE;
	}

	WCHAR szBuffer[8192] = {0};
	
	HRESULT hr = provider->GetHelpLink(szBuffer, ARRAYSIZE(szBuffer));
	provider->Release();

	if (FAILED(hr) || L'\0' == szBuffer[0])
		return FALSE;

	return LoginBox_OpenUrl(hwnd, szBuffer, TRUE);
}

BOOL LoginPage::SetLabelText(INT controlId, LPCWSTR pszText)
{
	HWND hLabel = GetDlgItem(hwnd, controlId);
	if (NULL == hLabel) return FALSE;
	
	LPWSTR pszTemp = NULL;
	if (NULL != pszText && L'\0' != *pszText)
	{
		INT cchLabel = lstrlenW(pszText);
		if (cchLabel > 0 && L':' != pszText[cchLabel-1])
		{
			pszTemp = LoginBox_MallocString(cchLabel + 2);
			if (NULL != pszTemp)
			{
				CopyMemory(pszTemp, pszText, sizeof(WCHAR) * cchLabel);
				pszTemp[cchLabel] = L':';
				pszTemp[cchLabel + 1] = L'\0';
				pszText = pszTemp;
			}
		}
	}
	
	BOOL result = SetWindowText(hLabel, pszText);

	if (NULL != pszTemp)
		LoginBox_FreeString(pszTemp);

	return result;
}

BOOL LoginPage::OnInitDialog(HWND hFocus, LPARAM param)
{
	HWND hControl = CreateWindowEx(WS_EX_NOPARENTNOTIFY, L"Static", NULL, 
					WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
					0, 0, 100, 24, hwnd, (HMENU)IDC_TITLE, NULL, 0L);

	if (NULL != hControl)
	{
		HFONT fontTitle = NULL;
		LoginGuiObject *loginGui;
		if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
		{
			fontTitle = loginGui->GetTitleFont();
			loginGui->Release();
		}

		if (NULL == fontTitle)
			fontTitle = (HFONT)SNDMSG(hwnd, WM_GETFONT, 0, 0L);

		if (NULL != fontTitle)
			SNDMSG(hControl, WM_SETFONT, (WPARAM)fontTitle, 0L);
	}

	INT imageWidth, imageHeight;
	HBITMAP bitmapHelp = LoginPage_GetHelpBitmap(hwnd, hbrBack, &imageWidth, &imageHeight);
	if (NULL != bitmapHelp)
	{
		UINT controlStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
							SS_BITMAP | SS_NOTIFY;

		if (FALSE != IsHelpAvailable())
			controlStyle |= WS_VISIBLE;

		hControl = CreateWindowEx(WS_EX_NOPARENTNOTIFY, L"Static", NULL, controlStyle,
					0, 0, 0, 0, hwnd, (HMENU)IDC_HELPLINK, NULL, 0L);
		
		HBITMAP bitmapSelected = NULL;

		if (NULL != hControl)
		{
			bitmapSelected = (HBITMAP)SNDMSG(hControl, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmapHelp);
			if (NULL != bitmapSelected)
				DeleteObject(bitmapSelected);
			bitmapSelected = (HBITMAP)SNDMSG(hControl, STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0L);
		}

		if (bitmapSelected != bitmapHelp)
			DeleteObject(bitmapHelp);
	}
	

	UpdateMargins();
	UpdateColors();
	UpdateLayout(FALSE);
	PostMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);
	
	return FALSE;
}

void LoginPage::OnDestroy()
{
	HWND hControl = GetDlgItem(hwnd, IDC_HELPLINK);
	if (NULL != hControl)
	{
		HBITMAP bitmapSelected = (HBITMAP)SNDMSG(hControl, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, 0L);
		if (NULL != bitmapSelected)
			DeleteObject(bitmapSelected);
	}
}

void LoginPage::OnWindowPosChanged(const WINDOWPOS *pwp)
{
	if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) 
		UpdateLayout(0 == (SWP_NOREDRAW & pwp->flags));
}

void LoginPage::OnCommand(UINT commandId, UINT eventType, HWND hControl)
{
	switch(commandId)
	{
		case IDC_HELPLINK:
			switch(eventType)
			{
				case STN_CLICKED:
					ShowHelp();
					break;
			}
			break;
	}
}

BOOL LoginPage::OnNotify(UINT controlId, const NMHDR *pnmh)
{
	return FALSE;
}

BOOL LoginPage::OnGetLoginData(LoginData **ppLoginData)
{
	if (FAILED(LoginData::CreateInstance(NULL, hwnd, hLoginbox, ppLoginData)))
		return FALSE;

	return TRUE;
}

void LoginPage::OnUpdateStateChange(BOOL updateActive)
{
}

BOOL LoginPage::OnSetUsername(LPCWSTR pszUsername)
{
	return FALSE;
}

BOOL LoginPage::OnSetPassword(LPCWSTR pszPassword)
{
	return FALSE;
}

HWND LoginPage::OnGetFirstItem()
{
	return NULL;
}

BOOL LoginPage::OnSetTitle(LPCWSTR pszTitle)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TITLE);
	if (NULL == hControl) return FALSE;
	
	BOOL result = (BOOL)SNDMSG(hControl, WM_SETTEXT, 0, (LPARAM)pszTitle);
	if (FALSE != result)
		UpdateLayout(TRUE);

	return result;
}

HBRUSH LoginPage::OnGetStaticColor(HDC hdc, HWND hControl)
{		
	INT controlId = (INT)GetWindowLongPtr(hControl,  GWLP_ID);
	switch(controlId)
	{
		case IDC_TITLE:
			SetTextColor(hdc, rgbTitle);
			break;
		default:
			SetTextColor(hdc, rgbText);
			break;
	}

	SetBkColor(hdc, rgbBack);
	return hbrBack;
}

HBRUSH LoginPage::OnGetDialogColor(HDC hdc, HWND hControl)
{		
	SetTextColor(hdc, rgbText);
	SetBkColor(hdc, rgbBack);
	return hbrBack;
}

BOOL LoginPage::OnSetCursor(HWND hTarget, INT hitCode, INT uMsg)
{
	HWND hControl = GetDlgItem(hwnd, IDC_HELPLINK);
	if (hControl == hTarget && NULL != hControl)
	{
		UINT controlStyle = GetWindowStyle(hControl);
		if (WS_VISIBLE == ((WS_VISIBLE | WS_DISABLED) & controlStyle))
		{
			HCURSOR hCursor = LoadCursor(NULL, IDC_HAND);
			if (NULL != hCursor)
			{
				SetCursor(hCursor);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL LoginPage::OnHelp(HELPINFO *phi)
{
	return ShowHelp();
}

void LoginPage::OnThemeChanged()
{
	UpdateColors();
}

void LoginPage::OnSysColorChanged()
{
		UpdateColors();
}

INT_PTR LoginPage::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:			return OnInitDialog((HWND)wParam, lParam);
		case WM_DESTROY:			OnDestroy(); return TRUE;
		case WM_NOTIFY:				MSGRESULT(hwnd, OnNotify((INT)wParam, (NMHDR*)lParam));
		case WM_COMMAND:			OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_WINDOWPOSCHANGED:	OnWindowPosChanged((WINDOWPOS*)lParam); return TRUE;
		case WM_SIZE:				return TRUE;
		case WM_CTLCOLORSTATIC:		return (INT_PTR)OnGetStaticColor((HDC)wParam, (HWND)lParam);
		case WM_CTLCOLORDLG:		return (INT_PTR)OnGetDialogColor((HDC)wParam, (HWND)lParam);
		case WM_SETCURSOR:			
			if (FALSE != OnSetCursor((HWND)wParam, LOWORD(lParam), HIWORD(lParam)))
				MSGRESULT(hwnd, TRUE);
			break;
		case WM_HELP:				
			if (FALSE != OnHelp((HELPINFO*)lParam))
				MSGRESULT(hwnd, TRUE);
			break;

		case WM_THEMECHANGED:		OnThemeChanged(); return TRUE;
		case WM_SYSCOLORCHANGE:		OnSysColorChanged(); return TRUE;

		case NLPM_GETLOGINDATA:		MSGRESULT(hwnd, OnGetLoginData((LoginData**)lParam));
		case NLPM_UPDATESTATECHANGE: OnUpdateStateChange((BOOL)lParam); return TRUE;
		case NLPM_SETUSERNAME:		MSGRESULT(hwnd, OnSetUsername((LPCWSTR)lParam));
		case NLPM_SETPASSWORD:		MSGRESULT(hwnd, OnSetPassword((LPCWSTR)lParam));
		case NLPM_GETFIRSTITEM:		MSGRESULT(hwnd, OnGetFirstItem());
		case NLPM_SETTITLE:			MSGRESULT(hwnd, OnSetTitle((LPCWSTR)lParam));
		
	}

	return FALSE;
}

static INT_PTR CALLBACK LoginPage_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static ATOM LOGINPAGE_PROP = 0;
	LoginPage *page = (LoginPage*)GetProp(hwnd, MAKEINTATOM(LOGINPAGE_PROP));

	if (NULL == page) 
	{
		switch(uMsg)
		{
			case WM_INITDIALOG:
				if (0 == LOGINPAGE_PROP)
				{
					LOGINPAGE_PROP = GlobalAddAtomW(L"NullsoftLoginPageProp");
					if (0 == LOGINPAGE_PROP)
						return 0;
				}

				if (NULL != lParam)
				{
					LOGINPAGECREATEPARAM *create = (LOGINPAGECREATEPARAM*)lParam;
					lParam = create->lParam;
					if (SUCCEEDED(create->fnCreator(hwnd, create->hLoginbox, &page)))
					{
						if (FALSE == SetProp(hwnd, MAKEINTATOM(LOGINPAGE_PROP), (HANDLE)page))
						{
							delete(page);
							page = NULL;
						}
					}
				}

				if (NULL != page)
					return page->DialogProc(uMsg, wParam, lParam);

				break;
		}
		return 0;
	}

	INT_PTR result = page->DialogProc(uMsg, wParam, lParam);

	if (WM_DESTROY == uMsg)
	{
		RemoveProp(hwnd, MAKEINTATOM(LOGINPAGE_PROP));
		delete(page);
	}

	return result;
}

