/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include <windowsx.h>

#include "Main.h"
#include "resource.h"
#include "Options.h"
#include "SkinCOM.h"
#include "ExternalCOM.h"
#include "../nu/combobox.h"
#include "../nu/ns_wc.h"
#include <malloc.h>

static int CALLBACK EnumFontsProc( LOGFONT *lplf, TEXTMETRIC *lptm, DWORD dwType, LPARAM lpData )
{
	ComboBox_AddString( (HWND)lpData, lplf->lfFaceName );
	return TRUE;
}


void UpdatePlaylistFontSizeText( void )
{
	HWND plpref = GetDlgItem( prefs_hwnd, IDC_RECT );
	plpref = GetWindow( plpref, GW_HWNDNEXT );
	if ( IsWindow( plpref ) )
		SetDlgItemInt( plpref, IDC_PLFONTSIZE, config_pe_fontsize, 0 );
}

void UpdateManualAdvanceState( void )
{
	HWND plpref = GetDlgItem( prefs_hwnd, IDC_RECT );
	plpref = GetWindow( plpref, GW_HWNDNEXT );
	if ( IsWindow( plpref ) )
		CheckDlgButton( plpref, IDC_MANUALPLAYLISTADVANCE, config_pladv ? 0 : 1 );
}

// shuffle tab procedure
INT_PTR CALLBACK PlaybackOptionsProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static int pl_prefs_init;
	hi helpinfo[] = {
		{IDC_PREFS_SHUFFLE_MORPH_RATE,IDS_P_O_SMS},
		{IDC_DEFEXT,IDS_P_O_DEFEXT},
		{IDC_RFL,IDS_P_O_RFL},

		{IDC_MANUALPLAYLISTADVANCE,IDS_P_O_MPA},
		{IDC_PLNUMS,IDS_P_DISP_TNUMS},
		{IDC_PLFONTSIZE,IDS_P_DISP_PLFONT},
	};

	DO_HELP();

	if ( uMsg == WM_INITDIALOG )
	{
		pl_prefs_init = 0;
		SetDlgItemTextA( hwndDlg, IDC_DEFEXT, config_defext );
		SendDlgItemMessage( hwndDlg, IDC_DEFEXT, EM_LIMITTEXT, sizeof( config_defext ), 0 );

		CheckDlgButton( hwndDlg, IDC_RFL, ( config_rofiob & 1 ) ? 1 : 0 );
		CheckDlgButton( hwndDlg, IDC_MANUALPLAYLISTADVANCE, config_pladv ? 0 : 1 );
		SendMessageW( GetDlgItem( hwndDlg, IDC_PREFS_SHUFFLE_MORPH_RATE ), TBM_SETRANGEMAX, 0, 50 );
		SendMessageW( GetDlgItem( hwndDlg, IDC_PREFS_SHUFFLE_MORPH_RATE ), TBM_SETRANGEMIN, 0, 0 );
		SendMessageW( GetDlgItem( hwndDlg, IDC_PREFS_SHUFFLE_MORPH_RATE ), TBM_SETPOS, 1, config_shuffle_morph_rate );
		CheckDlgButton( hwndDlg, IDC_PLNUMS, config_shownumsinpl ? 1 : 0 );
		CheckDlgButton( hwndDlg, IDC_PLZEROPAD, config_zeropadplnum ? 1 : 0 );
		EnableWindow( GetDlgItem( hwndDlg, IDC_PLZEROPAD ), config_shownumsinpl );

		SendDlgItemMessage( hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG( 999, 1 ) );
		SetDlgItemInt( hwndDlg, IDC_PLFONTSIZE, config_pe_fontsize, 0 );

		SendDlgItemMessageW( hwndDlg, IDC_PLDIRECTION, CB_ADDSTRING, 0, (LPARAM)getStringW( IDS_P_PLDIRECTION_AUTO, NULL, 0 ) );
		SendDlgItemMessageW( hwndDlg, IDC_PLDIRECTION, CB_ADDSTRING, 0, (LPARAM)getStringW( IDS_P_PLDIRECTION_L2R, NULL, 0 ) );
		SendDlgItemMessageW( hwndDlg, IDC_PLDIRECTION, CB_ADDSTRING, 0, (LPARAM)getStringW( IDS_P_PLDIRECTION_R2L, NULL, 0 ) );

		SendDlgItemMessage( hwndDlg, IDC_PLDIRECTION, CB_SETCURSEL, config_pe_direction, 0 );

		HWND fontcombo = GetDlgItem( hwndDlg, IDC_CUSTOMFONT );
		HDC dc = GetDC( NULL );
		EnumFonts( dc, NULL, (FONTENUMPROC)EnumFontsProc, (LPARAM)fontcombo );
		ReleaseDC( NULL, dc );

		// select the font, but fall back to Arial if it doesn't exist
		ComboBox combobox( fontcombo );
		if ( combobox.SelectString( playlist_custom_fontW ) == CB_ERR )
		{
			StringCbCopyW( playlist_custom_fontW, sizeof( playlist_custom_fontW ), FALLBACK_FONT );
			combobox.SelectString( playlist_custom_fontW );
		}
		CheckDlgButton( hwndDlg, IDC_NOCUSTOMFONT, config_custom_plfont ? 0 : 1 );
		if ( !config_custom_plfont )
		{
			EnableWindow( GetDlgItem( hwndDlg, IDC_CUSTOMFONT ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_STATIC_CUSTOMFONT ), FALSE );
		}

		//WI(plscrollsize);
		//WI(plmw2xscroll);
		SetDlgItemInt( hwndDlg, IDC_PLSCROLL, config_plscrollsize, 0 );
		SendDlgItemMessage( hwndDlg, IDC_SPIN3, UDM_SETRANGE, 0, MAKELONG( 100, 1 ) );
		CheckDlgButton( hwndDlg, IDC_MOUSE_SCROLL_DOUBLE_LINES, config_plmw2xscroll ? 1 : 0 );
		pl_prefs_init = 1;
	}
	else if ( uMsg == WM_COMMAND )
		switch ( LOWORD( wParam ) )
		{
			case IDC_PLDIRECTION:
				if ( HIWORD( wParam ) == CBN_SELCHANGE )
				{
					int l = SendDlgItemMessage( hwndDlg, IDC_PLDIRECTION, CB_GETCURSEL, 0, 0 );
					if ( l != CB_ERR )
						config_pe_direction = l;

					InvalidateRect( hPLWindow, NULL, FALSE );
				}
				return 0;

			case IDC_NOCUSTOMFONT:
				config_custom_plfont = !IsDlgButtonChecked( hwndDlg, IDC_NOCUSTOMFONT );
				EnableWindow( GetDlgItem( hwndDlg, IDC_CUSTOMFONT ), IsDlgButtonChecked( hwndDlg, IDC_NOCUSTOMFONT ) ? 0 : 1 );
				EnableWindow( GetDlgItem( hwndDlg, IDC_STATIC_CUSTOMFONT ), IsDlgButtonChecked( hwndDlg, IDC_NOCUSTOMFONT ) ? 0 : 1 );
				draw_reinit_plfont( 1 );
				InvalidateRect( hPLWindow, NULL, FALSE );
				JSAPI1_SkinChanged();
				break;

			case IDC_CUSTOMFONT:
				if ( HIWORD( wParam ) == CBN_SELCHANGE )
				{
					HWND fontcombo;
					int idx, len;
					wchar_t *t;
					fontcombo = GetDlgItem( hwndDlg, IDC_CUSTOMFONT );
					ComboBox combobox( fontcombo );
					idx = combobox.GetSelection();
					len = combobox.GetTextLen( idx );
					t = (wchar_t *)_malloca( ( len + 1 ) * sizeof( wchar_t ) );
					combobox.GetText( idx, t );
					t[ len ] = 0;
					StringCbCopyW( playlist_custom_fontW, sizeof( playlist_custom_fontW ), t );
					WideCharToMultiByteSZ( CP_ACP, 0, playlist_custom_fontW, -1, playlist_custom_font, 128, 0, 0 );
					_freea( t );
					draw_reinit_plfont( 1 );
					InvalidateRect( hPLWindow, NULL, FALSE );
					JSAPI1_SkinChanged();
				}
				break;

			case IDC_PLNUMS:
			{
				int t = config_shownumsinpl;
				config_shownumsinpl = IsDlgButtonChecked( hwndDlg, IDC_PLNUMS ) ? 1 : 0;
				if ( config_shownumsinpl != t )
				{
					_w_i( "shownumsinpl", config_shownumsinpl );
					draw_reinit_plfont( 1 );
					if ( config_pe_open )
						InvalidateRect( hPLWindow, NULL, FALSE );
				}

				EnableWindow( GetDlgItem( hwndDlg, IDC_PLZEROPAD ), config_shownumsinpl );
			}
			break;

			case IDC_PLZEROPAD:
			{
				config_zeropadplnum = IsDlgButtonChecked( hwndDlg, IDC_PLZEROPAD ) ? 1 : 0;
				InvalidateRect( hPLWindow, NULL, FALSE );
			}
			break;

			case IDC_PLFONTSIZE:
				if ( HIWORD( wParam ) == EN_CHANGE && pl_prefs_init )
				{
					int s = 0, t = GetDlgItemInt( hwndDlg, IDC_PLFONTSIZE, &s, 0 );
					if ( t < 2 )
						t = 11;

					if ( t != config_pe_fontsize && s )
					{
						config_pe_fontsize = t;
						if ( hMainWindow )
						{
							draw_reinit_plfont( 1 );
							InvalidateRect( hPLWindow, NULL, FALSE );
						}

						JSAPI1_SkinChanged();
					}
				}
				break;

			case IDC_DEFEXT:
				if ( HIWORD( wParam ) == EN_CHANGE )
				{
					GetWindowTextA( GetDlgItem( hwndDlg, IDC_DEFEXT ), config_defext, sizeof( config_defext ) );
				}
				break;

			case IDC_RFL:
				config_rofiob &= ~1;
				config_rofiob |= IsDlgButtonChecked( hwndDlg, IDC_RFL ) ? 1 : 0;
				break;

			case IDC_MANUALPLAYLISTADVANCE:
			{
				int manadv = IsDlgButtonChecked( hwndDlg, IDC_MANUALPLAYLISTADVANCE ) ? 0 : 1;
				if ( manadv != config_pladv )
					SendMessageW( hMainWindow, WM_COMMAND, WINAMP_FILE_MANUALPLADVANCE, 0 );

				break;
			}

			case IDC_MOUSE_SCROLL_DOUBLE_LINES:
				config_plmw2xscroll = IsDlgButtonChecked( hwndDlg, IDC_MOUSE_SCROLL_DOUBLE_LINES ) ? 1 : 0;
				break;

			case IDC_PLSCROLL:
				if ( HIWORD( wParam ) == EN_CHANGE && pl_prefs_init )
				{
					int s = 0, t = GetDlgItemInt( hwndDlg, IDC_PLSCROLL, &s, 0 );
					if ( t < 1 )
						t = 1;

					if ( t > 16 )
						t = 16;

					if ( t != config_plscrollsize && s )
						config_plscrollsize = t;
				}
				break;

			case IDC_SHUFFLE_HELP:
			{
				wchar_t title[ 64 ] = { 0 };
				MessageBoxW( hwndDlg, getStringW( IDS_SHUFFLE_MORPH_INFO, NULL, 0 ),
					getStringW( IDS_SHUFFLE_MORPH_RATE, title, 64 ), 0 );
			}
			break;
		}
	else if ( uMsg == WM_HSCROLL )
	{
		HWND swnd = (HWND)lParam;
		if ( swnd == GetDlgItem( hwndDlg, IDC_PREFS_SHUFFLE_MORPH_RATE ) )
		{
			config_shuffle_morph_rate = (unsigned char)SendMessageW( GetDlgItem( hwndDlg, IDC_PREFS_SHUFFLE_MORPH_RATE ), TBM_GETPOS, 0, 0 );
		}
	}
	else if ( uMsg == WM_DESTROY )
	{
		pl_prefs_init = 0;
	}

	const int controls[] =
	{
		IDC_PREFS_SHUFFLE_MORPH_RATE,
	};

	if ( FALSE != DirectMouseWheel_ProcessDialogMessage( hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE( controls ) ) )
		return TRUE;

	return FALSE;
} //shuffle