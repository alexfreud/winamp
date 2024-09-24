#ifndef DIALOGSKINNERH
#define DIALOGSKINNERH

#include "MediaLibraryInterface.h"
#include "../winamp/wa_dlg.h"

COLORREF GetHTMLColor( int color );

class DialogSkinner
{
	typedef HBITMAP( *BitmapFunc )( );
	typedef int ( *ColorFunc )( int idx ); // pass this an index, returns a RGB value (passing 0 or > 3 returns NULL)
	typedef int ( *HandleFunc )( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	typedef void ( *DrawFunc )( HWND hwndDlg, int *tab, int tabsize ); // each entry in tab would be the id | DCW_*
	
public:
	DialogSkinner()
	{}

	int Color( int index )
	{
		if ( !color )
			color = (ColorFunc)mediaLibrary.GetWADLGFunc( 1 );

		return color( index );
	}

	RGBQUAD GetRGB( int index )
	{
		COLORREF color = Color( index );

		RGBQUAD rgb;
		rgb.rgbReserved = 0;
		rgb.rgbBlue     = GetBValue( color );
		rgb.rgbGreen    = GetGValue( color );
		rgb.rgbRed      = GetRValue( color );

		return rgb;
	}

	INT_PTR Handle( HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		if ( !handle )
			handle = (HandleFunc)mediaLibrary.GetWADLGFunc( 2 );

		return handle( dlg, msg, wParam, lParam );
	}

	void Draw( HWND dlg, int *tab, int tabSize )
	{
		if ( !draw )
			draw = (DrawFunc)mediaLibrary.GetWADLGFunc( 3 );

		draw( dlg, tab, tabSize );
	}

	HFONT GetFont()
	{
		return (HFONT)mediaLibrary.GetWADLGFunc( 66 );
	}

	HBITMAP GetBitmap()
	{
		if ( !bitmap )
			bitmap = (BitmapFunc)mediaLibrary.GetWADLGFunc( 4 );

		return bitmap();
	}

	ColorFunc  color  = 0;
	HandleFunc handle = 0;
	DrawFunc   draw   = 0;
	BitmapFunc bitmap = 0;
};

extern DialogSkinner dialogSkinner;

#endif
