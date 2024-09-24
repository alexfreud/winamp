#include "./layout.h"

#include <windows.h>

BOOL Layout_Initialize( HWND hwnd, const INT *itemList, INT itemCount, LAYOUTITEM *layout )
{
	if ( NULL == itemList || NULL == layout )
		return FALSE;

	LAYOUTITEM *item;

	for ( INT i = 0; i < itemCount; i++ )
	{
		item       = &layout[ i ];
		item->hwnd = GetDlgItem( hwnd, itemList[ i ] );
		if ( item->hwnd == NULL )
			continue;

		if ( FALSE == GetWindowRect( item->hwnd, &item->rect ) )
			SetRectEmpty( &item->rect );
		else
			MapWindowPoints( HWND_DESKTOP, hwnd, (POINT *)&item->rect, 2 );

		item->cx    = item->rect.right - item->rect.left;
		item->cy    = item->rect.bottom - item->rect.top;
		item->x     = item->rect.left;
		item->y     = item->rect.top;
		item->flags = 0;
	}

	return TRUE;
}

BOOL Layout_Perform( HWND hwnd, LAYOUTITEM *layout, INT layoutCount, BOOL fRedraw )
{
	HDWP hdwp, hdwpTemp;
	hdwp = BeginDeferWindowPos( layoutCount );
	if ( hdwp == NULL )
		return FALSE;

	UINT baseFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	if ( fRedraw == FALSE )
		baseFlags |= ( SWP_NOREDRAW | SWP_NOCOPYBITS );

	LAYOUTITEM *item;
	for ( INT i = 0; i < layoutCount; i++ )
	{
		item = &layout[ i ];
		if ( item->hwnd == NULL )
			continue;

		UINT flags = baseFlags | ( item->flags & ~( SWP_HIDEWINDOW | SWP_SHOWWINDOW ) );
		if ( item->x == item->rect.left && item->y == item->rect.top )
			flags |= SWP_NOMOVE;

		if ( item->cx == ( item->rect.right - item->rect.left ) && item->cy == ( item->rect.bottom - item->rect.top ) )
			flags |= SWP_NOSIZE;

		if ( ( SWP_HIDEWINDOW & item->flags ) != 0 )
		{
			UINT windowStyle = (UINT)GetWindowLongPtr( item->hwnd, GWL_STYLE );
			if ( ( WS_VISIBLE & windowStyle ) != 0 )
			{
				SetWindowLongPtr( item->hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE );
				if ( FALSE != fRedraw )
				{
					RedrawWindow( hwnd, &item->rect, NULL, RDW_INVALIDATE | RDW_ERASE );
				}
			}
		}

		if ( ( SWP_NOSIZE | SWP_NOMOVE ) != ( ( SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED ) & flags ) )
		{
			hdwpTemp = DeferWindowPos( hdwp, item->hwnd, NULL, item->x, item->y, item->cx, item->cy, flags );
			if ( hdwpTemp == NULL )
				break;

			hdwp = hdwpTemp;
		}
	}

	BOOL result = ( hdwp != NULL ) ? EndDeferWindowPos( hdwp ) : FALSE;

	for ( INT i = 0; i < layoutCount; i++ )
	{
		item = &layout[ i ];
		if ( NULL != item->hwnd && 0 != ( SWP_SHOWWINDOW & item->flags ) )
		{
			UINT windowStyle = (UINT)GetWindowLongPtr( item->hwnd, GWL_STYLE );
			if ( 0 == ( WS_VISIBLE & windowStyle ) )
			{
				SetWindowLongPtr( item->hwnd, GWL_STYLE, windowStyle | WS_VISIBLE );
				if ( FALSE != fRedraw )
					RedrawWindow( item->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN );
			}
		}
	}

	return result;
}

static void Layout_SetItemVisibility( const RECT *rect, LAYOUTITEM *item )
{
	if ( NULL == item || NULL == item->hwnd )
		return;

	BOOL outsider = ( item->cx <= 0 || item->cy <= 0 ||
		item->x >= rect->right || item->y >= rect->bottom ||
		( item->x + item->cx ) < rect->left || ( item->y + item->cy ) < rect->top );

	UINT windowStyle = (UINT)GetWindowLongPtr( item->hwnd, GWL_STYLE );
	if ( 0 == ( WS_VISIBLE & windowStyle ) )
	{
		if ( !outsider )
		{
			item->flags |= SWP_SHOWWINDOW;
		}
	}
	else
	{
		if ( outsider )
		{
			item->flags |= SWP_HIDEWINDOW;
		}
	}
}

BOOL Layout_SetVisibility( const RECT *rect, LAYOUTITEM *layout, INT layoutCount )
{
	if ( NULL == rect || NULL == layout )
		return FALSE;

	for ( INT i = 0; i < layoutCount; i++ )
	{
		Layout_SetItemVisibility( rect, &layout[ i ] );
	}

	return TRUE;
}

BOOL Layout_SetVisibilityEx( const RECT *rect, const INT *indexList, INT indexCount, LAYOUTITEM *layout )
{
	if ( NULL == rect || NULL == indexList || NULL == layout )
		return FALSE;

	for ( INT i = 0; i < indexCount; i++ )
	{
		Layout_SetItemVisibility( rect, &layout[ indexList[ i ] ] );
	}

	return TRUE;
}

BOOL Layout_GetValidRgn( HRGN validRgn, POINTS parrentOffset, const RECT *validRect, LAYOUTITEM *layout, INT layoutCount )
{
	if ( NULL == validRgn )
		return FALSE;

	SetRectRgn( validRgn, 0, 0, 0, 0 );

	if ( NULL == layout )
		return FALSE;

	HRGN rgn = CreateRectRgn( 0, 0, 0, 0 );
	if ( NULL == rgn )
		return FALSE;

	LAYOUTITEM *item;
	LONG l, t, r, b;

	for ( INT i = 0; i < layoutCount; i++ )
	{
		item = &layout[ i ];
		if ( NULL != item->hwnd && 0 == ( ( SWP_HIDEWINDOW | SWP_SHOWWINDOW ) & item->flags ) )
		{
			l = item->x + parrentOffset.x;
			t = item->y + parrentOffset.y;
			r = l + item->cx;
			b = t + item->cy;
			if ( 0 != ( SWP_NOREDRAW & item->flags ) || ( l == item->rect.left && t == item->rect.top && r == item->rect.right && b == item->rect.bottom ) )
			{
				if ( NULL != validRect )
				{
					if ( l < validRect->left )
						l = validRect->left;

					if ( t < validRect->top )
						t = validRect->top;

					if ( r > validRect->right )
						r = validRect->right;

					if ( b > validRect->bottom )
						b = validRect->bottom;
				}

				if ( l < r && t < b )
				{
					SetRectRgn( rgn, l, t, r, b );
					CombineRgn( validRgn, validRgn, rgn, RGN_OR );

					if ( NULLREGION != GetUpdateRgn( item->hwnd, rgn, FALSE ) )
					{
						OffsetRgn( rgn, parrentOffset.x, parrentOffset.y );
						CombineRgn( validRgn, validRgn, rgn, RGN_DIFF );
					}
				}
			}
		}
	}

	DeleteObject( rgn );

	return TRUE;
}