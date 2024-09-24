#include "./loginCurtain.h"
#include "./loginPopup.h"
#include "./common.h"
#include "./graphics.h"
#include "./imageLoader.h"
#include "../resource.h"
#include "../api.h"


#define NWC_LOGINCURTAIN		L"NullsoftLoginCurtain"

typedef struct __LOGINCURTAINCREATEPARAM
{
	HWND	owner;
} LOGINCURTAINCREATEPARAM;

#define NLPF_IMAGEINVALID		0x00000001

typedef struct __LOGINCURTAIN
{
	HWND owner;
	HBITMAP bkImage;
	UINT flags;
	UINT childCount;
} LOGINCURTAIN;

typedef struct __DRAWBORDERPARAM
{
	HWND hParent;
	RECT rect;
	HDC hdc;
	HDC hdcSrc;
	HBITMAP bitmapFrame;
	HBITMAP bitmapOrig;
	BLENDFUNCTION blendFunc;
} DRAWBORDERPARAM;


typedef struct __UPDATEPOSPARAM
{
	HWND hParent;
	RECT clientRect;
	RECT childRect;
	HDWP hdwp;
	UINT childCount;
} UPDATEPOSPARAM;

typedef struct __EXCLUDERGNPARAM
{
	HWND hParent;
	HRGN exclude;
	HRGN tmp;
	RECT rect;
} EXCLUDERGNPARAM;

#define BACKGROUND_ALPHA	200

#define GetCurtain(__hwnd) ((LOGINCURTAIN*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

static LRESULT WINAPI LoginCurtain_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



static BOOL LoginCurtain_RegisterClass(HINSTANCE hInstance)
{

	WNDCLASSW wc;
	if (FALSE != GetClassInfo(hInstance, NWC_LOGINCURTAIN, &wc))
		return TRUE;
	
	ZeroMemory(&wc, sizeof(wc));

	wc.lpszClassName = NWC_LOGINCURTAIN;
	wc.lpfnWndProc = LoginCurtain_WindowProc;
	wc.style = CS_PARENTDC; 
	wc.cbWndExtra =  sizeof(LOGINCURTAIN*);
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	
	return ( 0 != RegisterClassW(&wc));
}

HWND LoginCurtain_CreateWindow(HWND hParent, HWND hOwner)
{	
	if (FALSE == LoginCurtain_RegisterClass(WASABI_API_ORIG_HINST))
		return NULL;

	RECT rect;
	if (FALSE == GetClientRect(hOwner, &rect))
		SetRectEmpty(&rect);
	else
		MapWindowPoints(hOwner, hParent, (POINT*)&rect, 2);

	LOGINCURTAINCREATEPARAM createParam;
	createParam.owner = hOwner;
		
	return CreateWindowEx(WS_EX_CONTROLPARENT, NWC_LOGINCURTAIN, NULL, 
					WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL, 
					rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
					hParent, NULL, WASABI_API_ORIG_HINST, &createParam);
	
}

static BOOL CALLBACK LoginCurtain_ExcludeChildRgnCallback(HWND hwnd, LPARAM lParam)
{
	EXCLUDERGNPARAM *param = (EXCLUDERGNPARAM*)lParam;
	if (NULL == param) return FALSE;

	RECT *r = &param->rect;

	if (0 == (WS_VISIBLE & GetWindowStyle(hwnd)) ||
		param->hParent != (HWND)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT) ||
		FALSE == GetWindowRect(hwnd, r))
		return TRUE;

	MapWindowPoints(HWND_DESKTOP, param->hParent, (POINT*)r, 2);

	if (NULL == param->tmp)
	{
		param->tmp = CreateRectRgn(0, 0, 0, 0);
		if (NULL == param->tmp) return FALSE;
	}

	if (NULL == param->exclude)
	{
		param->exclude = CreateRectRgn(0, 0, 0, 0);
		if (NULL == param->exclude) return FALSE;
	}

	INT regionType = GetWindowRgn(hwnd, param->tmp);
	switch(regionType)
	{
		case NULLREGION:
			SetRectRgn(param->tmp, r->left, r->top, r->right, r->bottom);
			CombineRgn(param->exclude, param->exclude, param->tmp, RGN_OR);
			break;
		case SIMPLEREGION:
		case COMPLEXREGION:	
			OffsetRgn(param->tmp, r->left, r->top);
			CombineRgn(param->exclude, param->exclude, param->tmp, RGN_OR);
			break;
	}

	return TRUE;
}

static void LoginCurtain_ExcludeChildren(HWND hwnd, HDC hdc)
{
	EXCLUDERGNPARAM param;
	param.hParent = hwnd;
	param.exclude = NULL;
	param.tmp = NULL;
	EnumChildWindows(hwnd, LoginCurtain_ExcludeChildRgnCallback, (LPARAM)&param);
	if (NULL != param.exclude)
	{
		ExtSelectClipRgn(hdc, param.exclude, RGN_DIFF);
		DeleteObject(param.exclude);
	}

	if (NULL != param.tmp)
		DeleteObject(param.tmp);
}

static BOOL CALLBACK LoginCurtain_DrawWindowBorderCallback(HWND hwnd, LPARAM lParam)
{
	DRAWBORDERPARAM *param = (DRAWBORDERPARAM*)lParam;
	if (NULL == param) return FALSE;

	RECT *r = &param->rect;

	if (0 == (WS_VISIBLE & GetWindowStyle(hwnd)) ||
		param->hParent != (HWND)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT) ||
		FALSE == GetWindowRect(hwnd, r))
		return TRUE;

	MapWindowPoints(HWND_DESKTOP, param->hParent, (POINT*)r, 2);

	if (NULL == param->bitmapFrame)
	{
		INT frameHeight, frameWidth;
		param->bitmapFrame = ImageLoader_LoadBitmap(WASABI_API_ORIG_HINST, MAKEINTRESOURCE(IDR_POPUPBORDER_IMAGE), 
										TRUE, &frameWidth, &frameHeight);
		if (NULL == param->bitmapFrame)
			return FALSE;

		param->hdcSrc = CreateCompatibleDC(param->hdc);
		if (NULL == param->hdcSrc) return FALSE;

		param->bitmapOrig = (HBITMAP)SelectObject(param->hdcSrc, param->bitmapFrame);

		param->blendFunc.AlphaFormat = AC_SRC_ALPHA;
		param->blendFunc.BlendFlags = 0;
		param->blendFunc.BlendOp = AC_SRC_OVER;
		param->blendFunc.SourceConstantAlpha = 255;
	}
	
	r->left -= 14;
	r->top -= 13;
	r->right += 17;
	r->bottom += 19;

	GdiAlphaBlend(param->hdc, r->left, r->top, 26, 26, param->hdcSrc, 0, 0, 26, 26, param->blendFunc);
	GdiAlphaBlend(param->hdc, r->right - 28, r->top, 28, 26, param->hdcSrc, 27, 0, 28, 26, param->blendFunc);
	GdiAlphaBlend(param->hdc, r->left, r->bottom - 31, 26, 31, param->hdcSrc, 0, 27, 26, 31, param->blendFunc);
	GdiAlphaBlend(param->hdc, r->right - 28, r->bottom - 31, 28, 31, param->hdcSrc, 27, 27, 28, 31, param->blendFunc);

	LONG l = (r->right - r->left  - (26 + 28));
	GdiAlphaBlend(param->hdc, r->left + 26, r->top, l, 26, param->hdcSrc, 25, 0, 1, 26, param->blendFunc);
	GdiAlphaBlend(param->hdc, r->left + 26, r->bottom - 31, l, 31, param->hdcSrc, 25, 27, 1, 31, param->blendFunc);

	l = (r->bottom - r->top - (26 + 31));
	GdiAlphaBlend(param->hdc, r->left, r->top + 26, 26, l, param->hdcSrc, 0, 28, 26, 1, param->blendFunc);
	GdiAlphaBlend(param->hdc, r->right - 28, r->top + 26, 28, l, param->hdcSrc, 27, 28, 28, 1, param->blendFunc);

	return TRUE;
}

static void LoginCurtain_DrawChildBorders(HWND hwnd, HDC hdc)
{
	DRAWBORDERPARAM param;
	ZeroMemory(&param, sizeof(param));
	param.hdc = hdc;
	param.hParent = hwnd;

	EnumChildWindows(hwnd, LoginCurtain_DrawWindowBorderCallback, (LPARAM)&param);

	if (NULL != param.hdcSrc)
	{
		SelectObject(param.hdcSrc, param.bitmapOrig);
		DeleteDC(param.hdcSrc);
	}

	if (NULL != param.bitmapFrame)
		DeleteObject(param.bitmapFrame);
}

static HBITMAP LoginCurtain_CreateBkImage(HWND hwnd, HDC hdc, HBITMAP hBitmap, HWND hOwner)
{
	RECT rect;
	if (FALSE == GetClientRect(hwnd, &rect))
		return NULL;

	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	BITMAP bi;
	
	if (NULL == hBitmap ||
		sizeof(BITMAP) != GetObject(hBitmap, sizeof(BITMAP), &bi) || 
		bi.bmWidth < width || bi.bmHeight < height)
	{
		if (NULL != hBitmap)
			DeleteObject(hBitmap);

		BITMAPINFOHEADER header;
		ZeroMemory(&header, sizeof(BITMAPINFOHEADER));

		header.biSize = sizeof(BITMAPINFOHEADER);
		header.biCompression = BI_RGB;
		header.biBitCount = 32;
		header.biPlanes = 1;
		header.biWidth = (width + 32);
		header.biHeight = -(height + 32);
		
		bi.bmBitsPixel = header.biBitCount;
		bi.bmWidth = header.biWidth;
		bi.bmHeight = header.biHeight;
		bi.bmPlanes = header.biPlanes;

		hBitmap = CreateDIBSection(NULL, (LPBITMAPINFO)&header, DIB_RGB_COLORS, (void**)&bi.bmBits, NULL, 0);
		if (NULL == hBitmap) return NULL;
	}
	else
	{
		bi.bmHeight = -bi.bmHeight;
	}

	HBRUSH bkBrush = (HBRUSH)SendMessage(hOwner, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hwnd);
	
	HBITMAP hbo = (HBITMAP)SelectObject(hdc, hBitmap);

	if (NULL == hOwner)
	{
		if (NULL != bkBrush)
			FillRect(hdc, &rect, bkBrush);
		else
			ExtTextOut(hdc, 0, 0, OPAQUE, &rect, NULL, 0, NULL);
	}
	else
	{
		if (FALSE == LoginBox_PrintWindow(hOwner, hdc, 0))
			SendMessage(hOwner, WM_PRINT, (WPARAM)hdc, (LPARAM) (PRF_NONCLIENT | PRF_CLIENT | PRF_CHILDREN | PRF_ERASEBKGND));

		COLORREF rgbBlend = GetDCBrushColor(hdc);
		if (CLR_INVALID == rgbBlend) 
			rgbBlend = GetBkColor(hdc);
		rgbBlend |= (BACKGROUND_ALPHA << 24);
		Image_ColorOverEx((BYTE*)bi.bmBits,  bi.bmWidth, bi.bmHeight, 0, 0, width, height, bi.bmBitsPixel, FALSE, rgbBlend);
	}

	LoginCurtain_DrawChildBorders(hwnd, hdc);
	
	SelectObject(hdc, hbo);
	return hBitmap;
}

static void LoginCurtain_EraseBkGround(HWND hwnd, HDC hdc, const RECT *prcPaint)
{
	LOGINCURTAIN  *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return;
	
	HDC hdcSrc = CreateCompatibleDC(hdc);
	if (NULL == hdcSrc) return;

	
	if (NULL == curtain->bkImage || 0 != (NLPF_IMAGEINVALID & curtain->flags))
	{
		HRGN clipRegion = CreateRectRgn(0,0,0,0);
		if (NULL != clipRegion)
		{
			INT regionType = GetClipRgn(hdc, clipRegion);
			if (1 == regionType)
				SelectClipRgn(hdcSrc, clipRegion);
			DeleteObject(clipRegion);
		}
		curtain->bkImage = LoginCurtain_CreateBkImage(hwnd, hdcSrc, curtain->bkImage, curtain->owner);
		curtain->flags &= ~NLPF_IMAGEINVALID;
	}

	if (NULL != curtain->bkImage)
	{
		HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcSrc, curtain->bkImage);
		BitBlt(hdc, prcPaint->left, prcPaint->top, 
			prcPaint->right - prcPaint->left, prcPaint->bottom - prcPaint->top, 
			hdcSrc, prcPaint->left, prcPaint->top, SRCCOPY);

		SelectObject(hdcSrc, hbmpOld);
	}

	DeleteDC(hdcSrc);

}

static void LoginCurtain_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	if (FALSE != fErase)
	{
		LoginCurtain_ExcludeChildren(hwnd, hdc);
		LoginCurtain_EraseBkGround(hwnd, hdc, prcPaint);
	}
}

static BOOL CALLBACK LoginCurtain_UpdateChildPosCallback(HWND hwnd, LPARAM lParam)
{
	UPDATEPOSPARAM *param = (UPDATEPOSPARAM*)lParam;
	if (NULL == param) return FALSE;

	if (param->hParent != (HWND)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT))
		return TRUE;

	if (NULL == param->hdwp)
	{
		param->hdwp = BeginDeferWindowPos(param->childCount);
		param->childCount = 0;
		
		if (NULL == param->hdwp) return FALSE;
	}

	LONG prevWidth(0), prevHeight(0);
	if (FALSE != GetWindowRect(hwnd, &param->childRect))
	{
		prevWidth = param->childRect.right - param->childRect.left;
		prevHeight = param->childRect.bottom - param->childRect.top;
	}
	

	if (FALSE != LoginPopup_UpdateWindowPos(hwnd, &param->clientRect, &param->childRect))
	{
		param->hdwp = DeferWindowPos(param->hdwp, hwnd, NULL, param->childRect.left, param->childRect.top, 
								param->childRect.right - param->childRect.left, param->childRect.bottom - param->childRect.top, 
								SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

		if (NULL == param->hdwp) return FALSE;
		
		LONG width = param->childRect.right - param->childRect.left;
		LONG height = param->childRect.bottom - param->childRect.top; 
		if (width != prevWidth || height != prevHeight)
		{
			HRGN clipRgn = CreateRoundRectRgn(0, 0, width, height, 9, 9);
			SetWindowRgn(hwnd, clipRgn, FALSE);
		}
	}
	param->childCount++;
	return TRUE;
}

static void LoginCurtain_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	LOGINCURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return;

	curtain->flags |= NLPF_IMAGEINVALID;

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	UPDATEPOSPARAM param;
	param.hParent = hwnd;
	param.hdwp = 0;
	param.childCount = curtain->childCount;

	CopyRect(&param.clientRect, &clientRect);
	param.clientRect.left += 15;
	param.clientRect.top += 14;
	param.clientRect.right -= 18;
	param.clientRect.bottom -= 20;

	EnumChildWindows(hwnd, LoginCurtain_UpdateChildPosCallback, (LPARAM)&param);
	curtain->childCount = param.childCount;
	if (NULL != param.hdwp)
		EndDeferWindowPos(param.hdwp);

	if (FALSE != fRedraw)
	{
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
	}
}

static LRESULT LoginCurtain_OnCreate(HWND hwnd, CREATESTRUCT* pcs)
{
	LOGINCURTAIN *curtain = (LOGINCURTAIN*)calloc(1, sizeof(LOGINCURTAIN));
	if (NULL != curtain)
	{
		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)curtain) && ERROR_SUCCESS != GetLastError())
		{
			free(curtain);
			curtain = NULL;
		}
	}

	if (NULL == curtain)
		return -1;

	LOGINCURTAINCREATEPARAM *createParam = (LOGINCURTAINCREATEPARAM*)pcs->lpCreateParams;
	if (NULL != createParam)
	{
		curtain->owner = createParam->owner;
	}

	SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOREDRAW);

	return 0;
}

static void LoginCurtain_OnDestroy(HWND hwnd)
{
	LOGINCURTAIN *curtain = GetCurtain(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);
	if (NULL == curtain) return;

	if (NULL != curtain->bkImage)
		DeleteObject(curtain->bkImage);

	free(curtain);
}

static void LoginCurtain_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
		return;

	LoginCurtain_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags));
}


static void LoginCurtain_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			LoginCurtain_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void LoginCurtain_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	LoginCurtain_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void LoginCurtain_OnParentNotify(HWND hwnd, INT eventId, INT childId, LPARAM eventParam)
{
	switch(eventId)
	{
		case WM_CREATE:
			{
				HWND hChild = (HWND)eventParam;
				RECT rect;
				GetWindowRect(hChild, &rect);
				HRGN clipRgn = CreateRoundRectRgn(0, 0, rect.right - rect.left, rect.bottom - rect.top, 9, 9);
				SetWindowRgn(hChild, clipRgn, FALSE);
			}
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED );
			break;

		case WM_DESTROY:
			{
				HWND hChild = (HWND)eventParam;
				SetWindowLongPtr(hChild, GWLP_ID, 0);
			}
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
			break;
	}
}

static LRESULT LoginCurtain_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	HWND hAncestor = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hAncestor)
		return SendMessage(hAncestor, WM_NOTIFY, (WPARAM)controlId, (LPARAM)pnmh);
	return 0;
}

static LRESULT WINAPI LoginCurtain_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return LoginCurtain_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			LoginCurtain_OnDestroy(hwnd); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_PAINT:				LoginCurtain_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		LoginCurtain_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_WINDOWPOSCHANGED:	LoginCurtain_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_PARENTNOTIFY:		LoginCurtain_OnParentNotify(hwnd, LOWORD(wParam), HIWORD(wParam), lParam); return 0;
		case WM_NOTIFY:				return LoginCurtain_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

