#include "api__gen_ml.h"
#include "main.h"
#include "./skinnedprogressbar.h"
#include "./skinning.h"
#include "../winamp/wa_dlg.h"
#include "./colors.h"
#include <strsafe.h>

SkinnedProgressBar::SkinnedProgressBar(void) 
	: SkinnedWnd(FALSE), skinCursor(NULL)
{
}

SkinnedProgressBar::~SkinnedProgressBar(void)
{
}

BOOL SkinnedProgressBar::Attach(HWND hProgressBar)
{
	if(!__super::Attach(hProgressBar)) return FALSE;
	
	SetType(SKINNEDWND_TYPE_PROGRESSBAR);
	return TRUE;
}

void SkinnedProgressBar::OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{
	skinCursor = (0 != (SWS_USESKINCURSORS & style)) ? 
					(HCURSOR)SENDWAIPC(plugin.hwndParent, IPC_GETSKINCURSORS, WACURSOR_NORMAL) : NULL;

	HFONT hfOld = (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L);

	__super::OnSkinChanged(bNotifyChildren, bRedraw);

	if (hfOld != (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L))
		CallPrevWndProc(TTM_UPDATE, 0, 0L);
}

void SkinnedProgressBar::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if (NULL == hdc) return;

	int radius = (ps.rcPaint.bottom - ps.rcPaint.top - WASABI_API_APP->getScaleY(1)) / WASABI_API_APP->getScaleY(2);
	int radius2 = WASABI_API_APP->getScaleY(5);

	RECT r;
	CopyRect(&r, &ps.rcPaint);
	// fill the background accordingly
	FillRect(hdc, &r, (HBRUSH)MlStockObjects_Get(WNDBCK_BRUSH));

	r.right -= WASABI_API_APP->getScaleX(1);
	r.bottom -= WASABI_API_APP->getScaleY(1);
	OffsetRect(&r, 1, 1);
	HPEN oldPen = SelectPen(hdc, MlStockObjects_Get(HILITE_PEN));
	HBRUSH oldBrush = SelectBrush(hdc, MlStockObjects_Get(WNDBCK_BRUSH));
	// draw the border edge offset to add a 'shadow'
    RoundRect(hdc, r.left, r.top, r.right, r.bottom, radius2, radius);

	ps.rcPaint.bottom -= WASABI_API_APP->getScaleY(1);

	SelectObject(hdc, MlStockObjects_Get(ITEMBCK_PEN));
	SelectObject(hdc, MlStockObjects_Get(ITEMBCK_BRUSH));
	// draw the 'empty' part of the bar with a slight overlap of the 'shadow'
    RoundRect(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, radius2, radius);

	int pos = (DWORD)CallPrevWndProc(PBM_GETPOS, 0, 0L);
	if (pos > 0)
	{
		int range = (DWORD)CallPrevWndProc(PBM_GETRANGE, 0, 0L);
		int val = (((ps.rcPaint.right) * pos) / range);

		if (val > 0)
		{
			if ((ps.rcPaint.left + val + radius) > ps.rcPaint.right)
			{
				SelectObject(hdc, MlStockObjects_Get(ITEMTEXT_PEN));
				SelectObject(hdc, MlStockObjects_Get(ITEMTEXT_BRUSH));
				// draws the 'filled' part of the bar with a slight overlap of the 'shadow'
				RoundRect(hdc, ps.rcPaint.left - WASABI_API_APP->getScaleX(10), ps.rcPaint.top,
						  val + (radius * WASABI_API_APP->getScaleY(2)) - WASABI_API_APP->getScaleY(10), ps.rcPaint.bottom, radius2, radius);
			}
			else
			{
				ps.rcPaint.right = ps.rcPaint.left + val;
				FillRect(hdc, &ps.rcPaint, (HBRUSH)MlStockObjects_Get(ITEMTEXT_BRUSH));
			}
		}

		// not keen on this at all but it sorts out the progress overlap
		SelectObject(hdc, MlStockObjects_Get(WNDBCK_PEN));
		MoveToEx(hdc, ps.rcPaint.left, ps.rcPaint.bottom, NULL);
		LineTo(hdc, ps.rcPaint.left + ((radius)/ 2), ps.rcPaint.bottom);
		LineTo(hdc, ps.rcPaint.left, ps.rcPaint.bottom - ((radius)/ WASABI_API_APP->getScaleY(2)));
		LineTo(hdc, ps.rcPaint.left, ps.rcPaint.bottom);

		MoveToEx(hdc, ps.rcPaint.left, ps.rcPaint.top, NULL);
		LineTo(hdc, ps.rcPaint.left, ps.rcPaint.top + ((radius)/ WASABI_API_APP->getScaleY(2)));

		MoveToEx(hdc, ps.rcPaint.left, ps.rcPaint.top, NULL);
		LineTo(hdc, ps.rcPaint.left + ((radius)/ WASABI_API_APP->getScaleY(2)), ps.rcPaint.top);
	}

	SelectPen(hdc, oldPen);
	SelectBrush(hdc, oldBrush);
}

UINT SkinnedProgressBar::GetBorderType(void)
{
	return BORDER_NONE;
}

LRESULT SkinnedProgressBar::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_PAINT:
			OnPaint();
			return 0;
		case WM_SETCURSOR:
			if (NULL != skinCursor)
			{
				if (skinCursor != GetCursor())
					SetCursor(skinCursor);
				return TRUE;
			}
			break;
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}