#define OEMRESOURCE

#include "./loginNotifier.h"
#include "./common.h"
#include "./loginGui.h"

#include "../api.h"
#include "../resource.h"
#include "../api_auth.h"

#include <strsafe.h>

#define NWC_LOGINNOTIFIER		L"NullsoftLoginNotifier"

#define GRADIENT_LEFT	30
#define GRADIENT_RIGHT	10

#define SPACING_TOP		2
#define SPACING_BOTTOM	2

typedef struct __LOGINNOTIFIER
{
	UINT flags;
	HBITMAP image;
	INT type;
	LPWSTR text;
	HFONT font;
	INT textHeight;
	INT aveCharWidth;
	COLORREF rgbBack;
	COLORREF rgbText;
} LOGINNOTIFIER;

#define GetNotifier(__hwnd) ((LOGINNOTIFIER*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

static LRESULT WINAPI LoginNotifier_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BOOL LoginNotifier_RegisterClass(HINSTANCE hInstance)
{

	WNDCLASSW wc;
	if (FALSE != GetClassInfo(hInstance, NWC_LOGINNOTIFIER, &wc))
		return TRUE;
	
	ZeroMemory(&wc, sizeof(wc));

	wc.lpszClassName = NWC_LOGINNOTIFIER;
	wc.lpfnWndProc = LoginNotifier_WindowProc;
	wc.style = CS_PARENTDC; 
	wc.cbWndExtra =  sizeof(LOGINNOTIFIER*);
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	return ( 0 != RegisterClassW(&wc));
}

HWND LoginNotifier_CreateWindow(UINT styleEx, UINT style, INT x, INT y, INT cx, INT cy, HWND hParent, INT controlId)
{	
	if (FALSE == LoginNotifier_RegisterClass(WASABI_API_ORIG_HINST))
		return NULL;

	return CreateWindowEx(styleEx, NWC_LOGINNOTIFIER, NULL, WS_CHILD | style, x, y, cx, cy,
					hParent, (HMENU)(INT_PTR)controlId, WASABI_API_ORIG_HINST, NULL);
	
}

static HBITMAP LoginNotifier_CreateTypeImage(HDC hdc, INT type, INT height)
{
	INT iconIndex;
	COLORREF rgbBack = RGB(255, 255, 255);
	COLORREF rgbAlert;

	switch(type)
	{
		case NLNTYPE_INFORMATION:
			iconIndex = LoginGuiObject::iconInfo;
			rgbAlert = RGB(209, 222, 254);
			break;
		case NLNTYPE_WARNING:
			iconIndex = LoginGuiObject::iconWarning;
			rgbAlert = RGB(254, 241, 148);
			break;
		case NLNTYPE_ERROR:
			iconIndex = LoginGuiObject::iconError;
			rgbAlert = RGB(225, 105, 105);
			break;
		case NLNTYPE_QUESTION:
			iconIndex = LoginGuiObject::iconQuestion;
			rgbAlert =  RGB(209, 222, 254);
			break;
		default:
			iconIndex = LoginGuiObject::iconNone; 
			rgbAlert = GetSysColor(COLOR_3DLIGHT);			
			break;
	}

	RECT iconRect;
	HBITMAP bitmapIcons = NULL;
	if (LoginGuiObject::iconNone != iconIndex)
	{
		LoginGuiObject *loginGui;
		if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
		{
			bitmapIcons = loginGui->GetIcon(iconIndex, &iconRect);
			loginGui->Release();
		}
	}

	if (NULL == bitmapIcons)
		SetRectEmpty(&iconRect);

	INT iconWidth = iconRect.right - iconRect.left;
	INT iconHeight = iconRect.bottom - iconRect.top;
	
	INT width = GRADIENT_LEFT + GRADIENT_RIGHT;
	width += iconWidth;

	HBITMAP bitmapDst = NULL;
	HDC contextDst = CreateCompatibleDC(hdc);
	if (NULL != contextDst)
	{
		bitmapDst = CreateCompatibleBitmap(hdc, width, height);
		if (NULL != bitmapDst)
		{
			HBITMAP bitmapDstOrig = (HBITMAP)SelectObject(contextDst, bitmapDst);
			
			TRIVERTEX vertex[] = 
			{ 
				{ 0, 0, GetRValue(rgbBack) << 8, GetGValue(rgbBack) << 8, GetBValue(rgbBack) << 8, 0x0000 },
				{ GRADIENT_LEFT, height, GetRValue(rgbAlert) << 8, GetGValue(rgbAlert) << 8, GetBValue(rgbAlert) << 8, 0x0000 },
			};

			GRADIENT_RECT    gradientRect;
			gradientRect.UpperLeft  = 0;
			gradientRect.LowerRight = 1;
			
			RECT fillRect;
			SetRect(&fillRect, 0, 0, width, height);
			if (FALSE != GdiGradientFill(contextDst, vertex, ARRAYSIZE(vertex), &gradientRect, 1, GRADIENT_FILL_RECT_H))
				fillRect.left = GRADIENT_LEFT;
			
			if (fillRect.left < fillRect.right)
			{
				COLORREF rgbBackOrig = SetBkColor(contextDst, rgbAlert);
				ExtTextOut(contextDst, 0, 0, ETO_OPAQUE, &fillRect, NULL, 0, NULL);
				if (rgbBackOrig != rgbAlert)
					SetBkColor(contextDst, rgbBackOrig);
			}

			if (NULL != bitmapIcons)
			{
				HDC contextSrc = CreateCompatibleDC(hdc);
				if (NULL != contextSrc)
				{
					HBITMAP bitmapSrcOrig = (HBITMAP)SelectObject(contextSrc, bitmapIcons);
	
					BLENDFUNCTION blendFunc;
					blendFunc.AlphaFormat = AC_SRC_ALPHA;
					blendFunc.BlendFlags = 0;
					blendFunc.BlendOp = AC_SRC_OVER;
					blendFunc.SourceConstantAlpha = 255;
						
					INT iconTop = (height - iconHeight)/2 + (height - iconHeight)%2;
					if (iconTop >= 1 && (iconTop + iconHeight) < height)
					{
						GdiAlphaBlend(contextDst, GRADIENT_LEFT, iconTop, iconWidth, iconHeight, 
								contextSrc, iconRect.left, iconRect.top, iconWidth, iconHeight, blendFunc);
					}

					SelectObject(contextSrc, bitmapSrcOrig);
					DeleteDC(contextSrc);
				}
			}
			SelectObject(contextDst, bitmapDstOrig);
		}
		DeleteDC(contextDst);
	}

	return bitmapDst;
}

static INT LoginNotifier_CalcTextHeight(HWND hwnd, HDC hdc, INT *aveCharWidth)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return 0;

	HDC contextMine(NULL);
	HFONT fontOrig;
	if (NULL == hdc)
	{
		contextMine = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL == contextMine) return 0;

		if (NULL != notifier->font)
			fontOrig = (HFONT)SelectObject(contextMine, notifier->font);

		hdc = contextMine;
	}
		
	TEXTMETRIC tm;
	if (FALSE == GetTextMetrics(hdc, &tm))
	{
		tm.tmHeight = 0;
		tm.tmAveCharWidth = 0;
	}
	
	if (NULL != aveCharWidth)
	{
		*aveCharWidth = LoginBox_GetAveCharWidth(hdc);
	}


	if (NULL != contextMine)
	{
		if (NULL != notifier->font)
			SelectObject(contextMine, fontOrig);

		ReleaseDC(hwnd, contextMine);
	}
	
	

	return tm.tmHeight;
}

static void LoginNotifier_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return;

	RECT rect;
	GetClientRect(hwnd, &rect);

	if (NULL == notifier->image)
		notifier->image = LoginNotifier_CreateTypeImage(hdc, notifier->type, rect.bottom - rect.top);
	
	INT imageWidth = 0;
	if (NULL != notifier->image)
	{		
		BITMAP bm;
		if (sizeof(BITMAP) == GetObject(notifier->image, sizeof(BITMAP), &bm))
		{
			HDC contextSrc = CreateCompatibleDC(hdc);	
			if (NULL != contextSrc)
			{
				HBITMAP bitmapSrcOrig = (HBITMAP)SelectObject(contextSrc, notifier->image);
				if (FALSE != BitBlt(hdc, rect.left, rect.top, bm.bmWidth, bm.bmHeight, contextSrc, 0, 0, SRCCOPY))
					imageWidth = bm.bmWidth;
				SelectObject(contextSrc, bitmapSrcOrig);
				DeleteDC(contextSrc);
			}
		}
	}

	
	SetBkColor(hdc, notifier->rgbBack);
	SetTextColor(hdc, notifier->rgbText);

	HFONT fontOrig;
	if (NULL != notifier->font) 
		fontOrig = (HFONT)SelectObject(hdc, notifier->font);

	LPCWSTR text = notifier->text;
	INT cchText = (NULL != text) ? lstrlenW(text) : 0;

	if (-1 == notifier->textHeight)
		notifier->textHeight  = LoginNotifier_CalcTextHeight(hwnd, hdc, &notifier->aveCharWidth);

	RECT textRect;
	CopyRect(&textRect, &rect);
	textRect.left += imageWidth;
	
	INT textOffsetY = (textRect.bottom - textRect.top) - notifier->textHeight;
	textOffsetY = textOffsetY/2 + textOffsetY%2;

	if (textOffsetY < SPACING_TOP) textOffsetY = SPACING_TOP;
		
	INT textAlignOrig = SetTextAlign(hdc, TA_TOP | TA_LEFT);
	ExtTextOut(hdc, textRect.left + notifier->aveCharWidth+2, textRect.top + textOffsetY, ETO_CLIPPED | ETO_OPAQUE, &textRect, text, cchText, NULL);

	if (textAlignOrig != (TA_TOP | TA_LEFT))
		SetTextAlign(hdc, textAlignOrig);

	if (NULL != notifier->font) 
		SelectObject(hdc, fontOrig);
}

static void LoginNotifier_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return;

	RECT rect;
	GetClientRect(hwnd, &rect);

	if (NULL != notifier->image)
	{		
		BITMAP bm;
		if (sizeof(BITMAP) != GetObject(notifier->image, sizeof(BITMAP), &bm) ||
			bm.bmHeight != rect.bottom - rect.top)
		{
			DeleteObject(notifier->image);
			notifier->image = NULL;
		}
	}

	InvalidateRect(hwnd, NULL, TRUE);

}

static BOOL LoginNotifier_SetNotification(HWND hwnd, INT type, LPCWSTR pszText, BOOL fInvalidate)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return FALSE;

	if (type != notifier->type)
	{
		notifier->type = type;
		if (NULL != notifier->image)
		{
			DeleteObject(notifier->image);
			notifier->image = NULL;
		}
	}

	LoginBox_FreeString(notifier->text);

	BOOL resultCode = TRUE;

	if (NULL == pszText)
	{
		notifier->text = NULL;
	}
	else
	{
		if (IS_INTRESOURCE(pszText))
		{
			INT stringId;
			switch((INT_PTR)pszText)
			{
				case AUTH_SUCCESS:				stringId = IDS_ERR_SUCCESS; break;
				case AUTH_404:					stringId = IDS_ERR_404; break;
				case AUTH_TIMEOUT:				stringId = IDS_ERR_TIMEOUT; break;
				case AUTH_NOHTTP:				stringId = IDS_ERR_NOHTTP; break;
				case AUTH_NOPARSER:				stringId = IDS_ERR_NOPARSER; break;
				case AUTH_CONNECTIONRESET:		stringId = IDS_ERR_CONNECTIONRESET; break;
				case AUTH_ERROR_PARSING_XML:	stringId = IDS_ERR_PARSING_XML; break;
				case AUTH_NOT_AUTHORIZED:		stringId = IDS_ERR_NOT_AUTHORIZED; break;
				case AUTH_SECURID:				stringId = IDS_ERR_SECURID; break;
				case AUTH_ABORT:				stringId = IDS_ERR_ABORT; break;
				case AUTH_INVALIDCRED:			stringId = IDS_ERR_INVALIDCRED; break;
				case AUTH_UNCONFIRMED:			stringId = IDS_ERR_UNCONFIRMED; break;
				case AUTH_UNEXPECTED:			stringId = IDS_ERR_UNEXPECTED; break;
				case AUTH_INVALIDPASSCODE:		stringId = IDS_ERR_PASSCODE_INVALID; break;
				case AUTH_USERNAME_EMPTY:		stringId = IDS_ERR_USERNAME_EMPTY; break;
				case AUTH_USERNAME_TOOSHORT:	stringId = IDS_ERR_USERNAME_TOOSHORT; break;
				case AUTH_USERNAME_TOOLONG:		stringId = IDS_ERR_USERNAME_TOOLONG; break;
				case AUTH_USERNAME_BADFORMAT:	stringId = IDS_ERR_USERNAME_BADFORMAT; break;
				case AUTH_PASSWORD_EMPTY:		stringId = IDS_ERR_PASSWORD_EMPTY; break;
				case AUTH_PASSWORD_TOOSHORT:	stringId = IDS_ERR_PASSWORD_TOOSHORT; break;
				case AUTH_PASSWORD_TOOLONG:		stringId = IDS_ERR_PASSWORD_TOOLONG; break;
				case AUTH_PASSWORD_BADFORMAT:	stringId = IDS_ERR_PASSWORD_BADFORMAT; break;
				case AUTH_PASSCODE_EMPTY:		stringId = IDS_ERR_PASSCODE_EMPTY; break;
				case AUTH_PASSCODE_TOOSHORT:	stringId = IDS_ERR_PASSCODE_TOOSHORT; break;
				case AUTH_PASSCODE_TOOLONG:		stringId = IDS_ERR_PASSCODE_TOOLONG; break;
				case AUTH_PASSCODE_BADFORMAT:	stringId = IDS_ERR_PASSCODE_BADFORMAT; break;
				default:						stringId = IDS_ERR_UNKNOWN; break;
			}
			
			WCHAR szBuffer[2048] = {0};
			LPWSTR cursor = szBuffer;
			size_t remaining = ARRAYSIZE(szBuffer);
			WASABI_API_LNGSTRINGW_BUF(IDS_LOGIN_FAILURE, cursor, remaining);
			size_t len = lstrlen(cursor);
			cursor += len;
			remaining -= len;
			if (cursor != szBuffer)
				StringCchCopyEx(cursor, remaining, L": ", &cursor, &remaining, 0);
			WASABI_API_LNGSTRINGW_BUF(stringId, cursor, remaining);

			notifier->text = LoginBox_CopyString(szBuffer); 
			if (NULL == notifier->text)
				resultCode = FALSE;
		}
		else
		{
			notifier->text = LoginBox_CopyString(pszText);
			if (NULL == notifier->text)
				resultCode = FALSE;
		}
	}

	if (FALSE != fInvalidate)
		InvalidateRect(hwnd, NULL, TRUE);

	return resultCode;
}
static LRESULT LoginNotifier_OnCreate(HWND hwnd, CREATESTRUCT* pcs)
{
	LOGINNOTIFIER *notifier = (LOGINNOTIFIER*)calloc(1, sizeof(LOGINNOTIFIER));
	if (NULL != notifier)
	{
		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)notifier) && ERROR_SUCCESS != GetLastError())
		{
			free(notifier);
			notifier = NULL;
		}
	}

	if (NULL == notifier)
		return -1;
	
	notifier->textHeight = -1;
	notifier->rgbBack = RGB(247, 247, 247);
	notifier->rgbText = RGB(0, 0, 0);

	return 0;
}

static void LoginNotifier_OnDestroy(HWND hwnd)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);
	if (NULL == notifier) return;

	if (NULL != notifier->image)
		DeleteObject(notifier->image);

	free(notifier);
}

static void LoginNotifier_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
		return;

	LoginNotifier_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags));
}


static void LoginNotifier_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			LoginNotifier_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void LoginNotifier_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	LoginNotifier_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void LoginNotifier_OnSetFont(HWND hwnd, HFONT hFont, BOOL fRedraw)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return;

	notifier->font = hFont;

	notifier->textHeight = -1;

	if (NULL != fRedraw)
		InvalidateRect(hwnd, NULL, FALSE);
}

static LRESULT LoginNotifier_OnGetFont(HWND hwnd)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	return (NULL != notifier) ? (LRESULT)notifier->font : NULL;
}

static LRESULT LoginNotifier_OnSetText(HWND hwnd, LPCWSTR pszText)
{
	return LoginNotifier_SetNotification(hwnd, NLNTYPE_INFORMATION, pszText, TRUE);
}

static LRESULT LoginNotifier_OnGetText(HWND hwnd, LPWSTR pszBuffer, size_t cchBufferMax)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	LPCWSTR pszText = (NULL != notifier) ? notifier->text : NULL;

	if (NULL == pszBuffer) 
		return 0;

	size_t remaining;
	StringCchCopyEx(pszBuffer, cchBufferMax, pszText, NULL, &remaining, STRSAFE_IGNORE_NULLS);
	return (cchBufferMax - remaining);
}

static LRESULT LoginNotifier_OnGetTextLength(HWND hwnd)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	return (NULL != notifier && NULL != notifier->text) ? lstrlen(notifier->text) : 0;
}

static LRESULT LoginNotifier_OnNotify(HWND hwnd, INT type, LPCWSTR pszText)
{
	return LoginNotifier_SetNotification(hwnd, type, pszText, TRUE);
}

static LRESULT LoginNotifier_OnGetIdealHeight(HWND hwnd)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return 0;

	if (-1 == notifier->textHeight)
		notifier->textHeight  = LoginNotifier_CalcTextHeight(hwnd, NULL, &notifier->aveCharWidth);
	
	
	INT iconHeight(0);
	LoginGuiObject *loginGui;
	if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
	{
		loginGui->GetIconDimensions(NULL, &iconHeight);
		loginGui->Release();
	}
		
	INT height = (notifier->textHeight > iconHeight) ? 
					notifier->textHeight : iconHeight;

	height += SPACING_TOP + SPACING_BOTTOM;
	return height;
}

static LRESULT LoginNotifier_OnGetIdealSize(HWND hwnd, SIZE *sizeOut)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier || NULL == sizeOut) return FALSE;

	if (-1 == notifier->textHeight)
		notifier->textHeight  = LoginNotifier_CalcTextHeight(hwnd, NULL, &notifier->aveCharWidth);
	
	INT iconWidth(0), iconHeight(0);
	LoginGuiObject *loginGui;
	if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
	{
		loginGui->GetIconDimensions(&iconWidth, &iconHeight);
		loginGui->Release();
	}
	
	sizeOut->cy = (notifier->textHeight > iconHeight) ? 
			notifier->textHeight : 
			iconHeight;

	sizeOut->cy += SPACING_TOP + SPACING_BOTTOM;

	sizeOut->cx = (0 != iconWidth) ? 
			(GRADIENT_LEFT + GRADIENT_RIGHT + iconWidth) : 
			0;

	INT cchText = (NULL != notifier->text) ? lstrlen(notifier->text) : 0;
	
	BOOL resultOk = TRUE;

	if (0 != cchText)
	{
		sizeOut->cx += notifier->aveCharWidth+2;

		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_WINDOW | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			HFONT fontOrig = (HFONT)SelectObject(hdc, notifier->font);
			
			SIZE textSize;
			if (FALSE != GetTextExtentPoint32W(hdc, notifier->text, cchText, &textSize))
				sizeOut->cx += textSize.cx;
			else
				resultOk = FALSE;
			
			SelectObject(hdc, fontOrig);
			ReleaseDC(hwnd, hdc);
		}
		else
			resultOk = FALSE;
	}
	return resultOk;
}

static void LoginNotifier_OnSetBkColor(HWND hwnd, COLORREF rgbColor)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return;
	
	notifier->rgbBack = rgbColor;
}

static LRESULT LoginNotifier_OnGetBkColor(HWND hwnd)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return RGB(255, 0, 255);

	return notifier->rgbBack;
}

static void LoginNotifier_OnSetTextColor(HWND hwnd, COLORREF rgbColor)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return;
	
	notifier->rgbText = rgbColor;
}

static LRESULT LoginNotifier_OnGetTextColor(HWND hwnd)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return RGB(255, 0, 255);

	return notifier->rgbText;
}
static void LoginNotifier_OnPlayBeep(HWND hwnd)
{
	LOGINNOTIFIER *notifier = GetNotifier(hwnd);
	if (NULL == notifier) return;

	UINT beepType;
	switch(notifier->type)
	{
		case NLNTYPE_INFORMATION:	beepType = MB_ICONASTERISK; break;
		case NLNTYPE_WARNING:		beepType = MB_ICONEXCLAMATION; break;
		case NLNTYPE_ERROR:			beepType = MB_ICONHAND; break;
		case NLNTYPE_QUESTION:		beepType = MB_ICONQUESTION; break;
		default: return;
	}
	LoginBox_MessageBeep(beepType);
}

static LRESULT WINAPI LoginNotifier_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return LoginNotifier_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			LoginNotifier_OnDestroy(hwnd); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_PAINT:				LoginNotifier_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		LoginNotifier_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_PRINT:				return 0;
		
		case WM_WINDOWPOSCHANGED:	LoginNotifier_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_SETFONT:			LoginNotifier_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); return 0;
		case WM_GETFONT:			return LoginNotifier_OnGetFont(hwnd);
		case WM_SETTEXT:			return LoginNotifier_OnSetText(hwnd, (LPCWSTR)lParam);
		case WM_GETTEXT:			return LoginNotifier_OnGetText(hwnd, (LPWSTR)lParam, (INT)wParam);
		case WM_GETTEXTLENGTH:		return LoginNotifier_OnGetTextLength(hwnd);

		case NLNM_NOTIFY:			return LoginNotifier_OnNotify(hwnd, (INT)wParam, (LPCWSTR)lParam);
		case NLNM_GETIDEALHEIGHT:	return LoginNotifier_OnGetIdealHeight(hwnd);
		case NLNM_SETBKCOLOR:		LoginNotifier_OnSetBkColor(hwnd, (COLORREF)lParam); return 0;
		case NLNM_GETBKCOLOR:		return LoginNotifier_OnGetBkColor(hwnd);
		case NLNM_SETTEXTCOLOR:		LoginNotifier_OnSetTextColor(hwnd, (COLORREF)lParam); return 0;
		case NLNM_GETTEXTCOLOR:		return LoginNotifier_OnGetTextColor(hwnd);
		case NLNM_PLAYBEEP:			LoginNotifier_OnPlayBeep(hwnd); return 0;
		case NLNM_GETIDEALSIZE:		return LoginNotifier_OnGetIdealSize(hwnd, (SIZE*)lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

