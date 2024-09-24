#include "./skinneddivider.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"

SkinnedDivider::SkinnedDivider(void) : SkinnedWnd(FALSE)
{
	callback = NULL;
	userParam = 0L;
	clickoffs = 0;
}

SkinnedDivider::~SkinnedDivider(void)
{
}

BOOL SkinnedDivider::Attach(HWND hwndDivider)
{
	if(!SkinnedWnd::Attach(hwndDivider)) return FALSE;
	SetType(SKINNEDWND_TYPE_DIVIDER);
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return TRUE;
}

BOOL SkinnedDivider::OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult)
{
	switch(msg)
	{
		case ML_IPC_SKINNEDDIVIDER_SETCALLBACK: 
				if ( 0 != param)
				{
					callback = ((MLDIVIDERCALLBACK*)param)->fnCallback;
					userParam = ((MLDIVIDERCALLBACK*)param)->userParam;
				}
				else { callback = NULL; userParam= 0L;}
				return TRUE;
	}
	return __super::OnMediaLibraryIPC(msg, param, pResult);
}

void SkinnedDivider::OnPaint(void)
{
	PAINTSTRUCT ps;
	RECT rc, rh;
	HDC hdc = BeginPaint(hwnd, &ps);

	GetClientRect(hwnd, &rc);

	SetBkColor(hdc, WADlg_getColor(WADLG_WNDBG));
	if (SWDIV_NOHILITE & style)
	{
		ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rc, L"", 0, NULL);
	}
	else
	{
		int l;
		HPEN pen = (HPEN)MlStockObjects_Get(HILITE_PEN),
			 penOld = (HPEN)SelectObject(hdc, pen);

		if (SWDIV_VERT & style)
		{
			l = (rc.right - rc.left)/2 - 1;
			SetRect(&rh, ps.rcPaint.left, ps.rcPaint.top, l, ps.rcPaint.bottom);	
			if (rh.left < rh.right) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rh, L"", 0, 0);
			SetRect(&rh, l + 1, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);	
			if (rh.left < rh.right) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rh, L"", 0, 0);
			
			MoveToEx(hdc, l, rc.top, NULL);
			LineTo(hdc, l, rc.bottom);
		}
		else
		{
			l = (rc.bottom - rc.top)/2 - 1;

			SetRect(&rh, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, l);	
			if (rh.top < rh.bottom) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rh, L"", 0, 0);
			SetRect(&rh, ps.rcPaint.left, l + 1, ps.rcPaint.right, ps.rcPaint.bottom);	
			if (rh.left < rh.right) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rh, L"", 0, 0);

			MoveToEx(hdc, rc.left, l, NULL);
			LineTo(hdc, rc.right, l);
		}

		SelectObject(hdc, penOld);
	}
	EndPaint(hwnd, &ps);
}

LRESULT SkinnedDivider::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_NCHITTEST: return HTCLIENT;
		case WM_ERASEBKGND: return 0;
		case WM_PAINT: OnPaint(); return 0;

		case WM_LBUTTONDOWN:
			clickoffs = (SWDIV_VERT & style) ? LOWORD(lParam) : HIWORD(lParam);
			SetCapture(hwnd);
			break;
		case WM_LBUTTONUP:
			ReleaseCapture();
			clickoffs = 0;
			break;

		case WM_SETCURSOR:
			SetCursor(LoadCursor(NULL, (SWDIV_VERT & style) ? IDC_SIZEWE : IDC_SIZENS));
			return TRUE;

		case WM_MOUSEMOVE:
			if (GetCapture() == hwnd)
			{
				RECT rw;
				BOOL vert = (SWDIV_VERT & style);
				GetWindowRect(hwnd, &rw);
				GetCursorPos(((LPPOINT)&rw) + 1);
				(SWDIV_VERT & style) ? rw.right -= clickoffs : rw.bottom -= clickoffs;

				if ((vert && rw.left != rw.right) || (!vert && rw.top != rw.bottom))
				{
					MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), ((LPPOINT)&rw) + 1, 1);
					if (callback) callback(hwnd, (vert) ? rw.right : rw.bottom, userParam);
				}
			}
		break;

	}
	return __super::WindowProc(uMsg, wParam, lParam);
}