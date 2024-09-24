#include "./ml_rating.h"
#include <commctrl.h>

#define IMAGE_PARTSCOUNT 4

BOOL MLRatingI_Draw(HDC	hdc, INT maxValue, INT value, INT trackingVal, HMLIMGLST hmlil, INT index, RECT *prc, UINT fStyle)
{
	INT ilIndex;
	if (!hdc || !hmlil || !prc)  return FALSE;

	if (RDS_OPAQUE_I & fStyle) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, prc, L"", 0, 0);

	ilIndex = MLImageListI_GetRealIndex(hmlil, index, GetBkColor(hdc), GetTextColor(hdc));

	if (-1 != ilIndex) 
	{
		INT val, count;
		static IMAGELISTDRAWPARAMS ildp = { 56/*sizeof(IMAGELISTDRAWPARAMS)*/, 0, };

		ildp.hdcDst		= hdc;
		ildp.himl		= MLImageListI_GetRealList(hmlil);
		ildp.i			= ilIndex;
		ildp.x			= prc->left;
		ildp.y			= prc->top;
		ildp.rgbBk		= CLR_DEFAULT;
		ildp.rgbFg		= CLR_DEFAULT;
		ildp.fStyle		= ILD_NORMAL;
		ildp.dwRop		= SRCCOPY;

		MLImageListI_GetImageSize(hmlil, &ildp.cx, &ildp.cy);

		val = (value < 0) ? 0 : value;

		count = ((RDS_SHOWEMPTY_I & fStyle) ? maxValue : (((RDS_HOT_I & fStyle) && trackingVal > val) ? trackingVal : val));
		if (count < 0) count = 0;

		ildp.cx	= ildp.cx/IMAGE_PARTSCOUNT;
		ildp.xBitmap = ((RDS_HOT_I & fStyle) ? (ildp.cx*2) : (RDS_INACTIVE_HOT_I & fStyle) ? ildp.cx*2 : 0) + ildp.cx;

		if(RDS_RIGHT_I & fStyle) ildp.x = prc->right - maxValue*ildp.cx;
		if(RDS_HCENTER_I & fStyle) ildp.x = prc->left + (prc->right - prc->left - maxValue*ildp.cx)/2;

		if(RDS_BOTTOM_I & fStyle) ildp.y = prc->bottom - ildp.cy;
		if(RDS_VCENTER_I & fStyle) ildp.y = prc->top + (prc->bottom - prc->top - ildp.cy)/2;

		if (ildp.y < prc->top) 
		{
			ildp.yBitmap = prc->top - ildp.y;
			ildp.y = prc->top;
		}
		else ildp.yBitmap = 0;

		if (ildp.cy > (prc->bottom - ildp.y))  ildp.cy	= prc->bottom - ildp.y;

		for (INT i = 0; ildp.x < prc->right && i < count; i++, ildp.x += ildp.cx)
		{
			if (RDS_HOT_I & fStyle)
			{
				if (i == trackingVal) ildp.xBitmap -= ildp.cx * ((val > trackingVal) ? 2 : 1);
				else if (i == val && val > trackingVal) ildp.xBitmap += ildp.cx;
			}
			else 
			{
				if (i == val) ildp.xBitmap -= ildp.cx; 
			}
			if (ildp.x < (prc->left - ildp.cx)) continue;
			if (prc->right < (ildp.x + ildp.cx)) ildp.cx = prc->right - ildp.x;
			ImageList_DrawIndirect(&ildp);	
		}
	}

	return TRUE;
}

LONG MLRatingI_HitTest(POINT pt, INT maxValue, HMLIMGLST hmlil, RECT *prc, UINT fStyle)
{
	INT imageCX, imageCY, imageX, imageY;
	WORD index, flags;

	if (!hmlil || !prc)  return MAKELONG(0,0);

	if (!PtInRect(prc, pt)) return MAKELONG(0, RHT_NOWHERE_I);

	MLImageListI_GetImageSize(hmlil, &imageCX, &imageCY);

	imageCX	= imageCX/IMAGE_PARTSCOUNT;

	imageX = prc->left;
	if(RDS_RIGHT_I & fStyle) imageX = prc->right - maxValue*imageCX;
	if(RDS_HCENTER_I & fStyle) imageX = prc->left + (prc->right - prc->left - maxValue*imageCX)/2;

	imageY = prc->top;
	if(RDS_BOTTOM_I & fStyle) imageY = prc->bottom - imageCY;
	if(RDS_VCENTER_I & fStyle) imageY = prc->top + (prc->bottom - prc->top - imageCY)/2;

	if (imageY < prc->top) imageY = prc->top;
	if (imageCY > (prc->bottom - imageY))  imageCY = prc->bottom - imageY;

	flags = 0;
	index = 0;

	if (pt.x < imageX) flags |= RHT_TOLEFT_I;
	else if (pt.x > (imageX + imageCX*maxValue)) flags |= RHT_TORIGHT_I;
	else 
	{
		flags |= RHT_ONVALUE_I;

		if (pt.y < imageY) flags |= RHT_ONVALUEABOVE_I;
		else if (pt.y > (imageY + imageCY)) flags |= RHT_ONVALUEBELOW_I;

		index = (WORD)(pt.x - imageX)/imageCX + 1;
		if (index > maxValue) index = maxValue;
	}
	return MAKELONG(index, flags);
}

BOOL MLRatingI_CalcMinRect(INT maxValue, HMLIMGLST hmlil, RECT *prc)
{
	INT imageCX, imageCY;

	if (!hmlil || !prc || !MLImageListI_GetImageSize(hmlil, &imageCX, &imageCY)) return FALSE;
	SetRect(prc, 0, 0, maxValue*imageCX/IMAGE_PARTSCOUNT, imageCY);

	return TRUE;
}