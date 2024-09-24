#include "main.h"
#include "./ml_cloudcolumn.h"
#include "./ml_cloud.h"
#include "api__gen_ml.h"
#include "./ml.h"
#include "./ml_IPC_0313.h"
#include "./resource.h"
#include "../winamp/gen.h"
#include "./stockobjects.h"
#include <commctrl.h>
#include <strsafe.h>

extern HMLIMGLST hmlilCloud;

#define CLOUD_IMAGELIST		hmlilCloud

#define CLOUD_LEFTPADDING	5
#define CLOUD_RIGHTPADDING	4

static INT cloudMinWidth = 27;

BOOL MLCloudColumnI_Initialize(void)
{
	RECT rc;
	cloudMinWidth = ((MLCloudI_CalcMinRect(CLOUD_IMAGELIST, &rc)) ? (rc.right - rc.left) : 0);
	return TRUE;
}

INT MLCloudColumnI_GetMinWidth(void)
{
	return cloudMinWidth + CLOUD_LEFTPADDING + CLOUD_RIGHTPADDING;
}

BOOL MLCloudColumnI_Paint(CLOUDCOLUMNPAINT_I *pRCPaint)
{
	RECT rc;

	rc.left = LVIR_BOUNDS;
	rc.top = pRCPaint->iSubItem;
	if (SendMessageW(pRCPaint->hwndList, LVM_GETSUBITEMRECT, pRCPaint->iItem, (LPARAM)&rc))
	{
		if ((rc.right - rc.left - CLOUD_LEFTPADDING - CLOUD_RIGHTPADDING) >= cloudMinWidth && 
			(rc.left + CLOUD_LEFTPADDING) < (rc.right - CLOUD_RIGHTPADDING) && rc.top < rc.bottom)
		{
			INT left;
			COLORREF rgbBkOld;

			if (rc.right <= pRCPaint->prcView->left || rc.left >= pRCPaint->prcView->right) return TRUE;

			rgbBkOld = SetBkColor(pRCPaint->hdc, pRCPaint->rgbBk);

			left = rc.left;
			if (0 == rc.left) rc.left = 3;
			ExtTextOutW(pRCPaint->hdc, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);

			INT value = pRCPaint->value;
			if (value)
			{
				COLORREF rgbFgOld = SetTextColor(pRCPaint->hdc, pRCPaint->rgbFg);
				rc.left = left + CLOUD_LEFTPADDING;
				rc.right -= CLOUD_RIGHTPADDING;
				MLCloudI_Draw(pRCPaint->hdc, value, CLOUD_IMAGELIST, (value - 1), &rc);
				if (pRCPaint->rgbFg != rgbFgOld) SetTextColor(pRCPaint->hdc, rgbFgOld);
			}

			if (pRCPaint->rgbBk != rgbBkOld) SetBkColor(pRCPaint->hdc, rgbBkOld);
			return TRUE;
		}
	}

	return FALSE;
}

INT MLCloudColumnI_GetWidth(INT width)
{
	INT minWidth = MLCloudColumnI_GetMinWidth();
	if (width < minWidth) width = minWidth;
	if (width > minWidth) width = minWidth;
	return width;
}