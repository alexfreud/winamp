/****************************************************************************
*
*   Module Title :     xprintf.cpp
*
*   Description  :     Display a printf style message on the current video frame.    
*
****************************************************************************/						

/****************************************************************************
*  Header Files
****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include "xprintf.h"

/****************************************************************************
 * 
 *  ROUTINE       : xprintf
 *
 *  INPUTS        : const PB_INSTANCE *ppbi : Pointer to decoder instance.
 *                  long nPixel             : Offset into buffer to write text.
 *                  const char *format      : Format string for print.
 *                  ...                     : Variable length argument list.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : int: Size (in bytes) of the formatted text.
 *
 *  FUNCTION      : Display a printf style message on the current video frame.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
int vp6_xprintf ( const PB_INSTANCE *ppbi, long nPixel, const char *format, ... )
{
    BOOL bRC;
    va_list arglist;
	HFONT hfont, hfonto;

    int rc = 0;
    long nSizeY = ppbi->HFragments * 8;
    long nStride = ppbi->Configuration.YStride;
    char szFormatted[256] = "";
    UINT8 *pDest = &ppbi->PostProcessBuffer[nPixel];

    //  Format text
    va_start ( arglist, format );
    _vsnprintf ( szFormatted, sizeof(szFormatted), format, arglist );
    va_end ( arglist );

#if defined (_WIN32_WCE)
#else
    //  Set up temporary bitmap
    HDC hdcMemory   = NULL;
    HBITMAP hbmTemp = NULL;
    HBITMAP hbmOrig = NULL;

    RECT rect;
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = 8 * strlen(szFormatted);
    rect.bottom = 8;

    hdcMemory = CreateCompatibleDC ( NULL );
    if ( hdcMemory == NULL )
        goto Exit;

    hbmTemp = CreateBitmap ( rect.right, rect.bottom, 1, 1, NULL );
    if ( hbmTemp == NULL )
        goto Exit;

    hbmOrig = static_cast<HBITMAP>(SelectObject(hdcMemory, hbmTemp));
    if ( !hbmOrig )
        goto Exit;

    //  Write text into bitmap
    //  font?
	hfont = CreateFont ( 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "" ); 
	if ( hfont == NULL )
		goto Exit;

    hfonto = static_cast<HFONT>(SelectObject(hdcMemory, hbmTemp));
	if ( !hfonto )
		goto Exit;

	SelectObject ( hdcMemory, hfont );
    SetTextColor ( hdcMemory, 1 );
    SetBkColor ( hdcMemory, 0 );
    SetBkMode ( hdcMemory, TRANSPARENT );

    bRC = BitBlt ( hdcMemory, rect.left, rect.top, rect.right, rect.bottom, hdcMemory, rect.left, rect.top, BLACKNESS );
    if ( !bRC )
        goto Exit;

    bRC = ExtTextOut ( hdcMemory, 0, 0, ETO_CLIPPED, &rect, szFormatted, strlen(szFormatted), NULL );
    if ( !bRC )
        goto Exit;

    //  Copy bitmap to video frame
    long x;
    long y;

    for ( y=rect.top; y<rect.bottom; ++y )
    {
        for ( x=rect.left; x<rect.right; ++x )
        {
            if ( GetPixel( hdcMemory, x, rect.bottom - 1 - y ) )
                pDest[x] = 255;
        }
        pDest += nStride;
    }

    rc = strlen ( szFormatted );

Exit:

    if ( hbmTemp != NULL )
    {
        if ( hbmOrig != NULL )
        {
            SelectObject ( hdcMemory, hbmOrig );
        }
        DeleteObject ( hbmTemp );
    }
    if ( hfont != NULL )
    {
        if ( hfonto != NULL )
            SelectObject ( hdcMemory, hfonto );
        DeleteObject ( hfont );
    }

    if ( hdcMemory != NULL )
        DeleteDC ( hdcMemory );
    hdcMemory = 0;
#endif

    return rc;
}
