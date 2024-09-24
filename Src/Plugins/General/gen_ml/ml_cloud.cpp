#include "./ml_cloud.h"
#include <commctrl.h>

BOOL MLCloudI_Draw( HDC hdc, INT value, HMLIMGLST hmlil, INT index, RECT *prc )
{
	if ( !hdc || !hmlil || !prc )
		return FALSE;

	INT ilIndex = MLImageListI_GetRealIndex(hmlil, index, GetBkColor(hdc), GetTextColor(hdc));

	if ( -1 != ilIndex )
	{
		INT val = ( value < 0 ) ? 0 : value;
		static IMAGELISTDRAWPARAMS ildp = { 56/*sizeof(IMAGELISTDRAWPARAMS)*/, 0, };

		ildp.hdcDst = hdc;
		ildp.himl   = MLImageListI_GetRealList( hmlil );
		ildp.i      = ilIndex;
		ildp.x      = prc->left;
		ildp.y      = prc->top;
		ildp.rgbBk  = CLR_DEFAULT;
		ildp.rgbFg  = CLR_DEFAULT;
		ildp.fStyle = ILD_NORMAL;
		ildp.dwRop  = SRCCOPY;

		MLImageListI_GetImageSize( hmlil, &ildp.cx, &ildp.cy );

		ildp.xBitmap = 0;

		if ( ildp.y < prc->top )
		{
			ildp.yBitmap = prc->top - ildp.y;
			ildp.y       = prc->top;
		}
		else
			ildp.yBitmap = 0;

		if ( ildp.cy > ( prc->bottom - ildp.y ) )
			ildp.cy = prc->bottom - ildp.y;

		if ( !val )
			ildp.xBitmap -= ildp.cx;

		if ( !( ildp.x < ( prc->left - ildp.cx ) ) )
		{
			if ( prc->right < ( ildp.x + ildp.cx ) )
				ildp.cx = prc->right - ildp.x;

			ImageList_DrawIndirect( &ildp );
		}
	}

	return TRUE;
}

BOOL MLCloudI_CalcMinRect( HMLIMGLST hmlil, RECT *prc )
{
	INT imageCX, imageCY;

	if ( !hmlil || !prc || !MLImageListI_GetImageSize( hmlil, &imageCX, &imageCY ) )
		return FALSE;

	SetRect( prc, 0, 0, imageCX + 2, imageCY );

	return TRUE;
}