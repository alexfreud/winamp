#include "main.h"
#include "config.h"
#include "resource.h"

char config_extensions[1024] = {0};
bool config_upsample8bit=false;

static int ExtensionInList(HWND hwndDlg, int id, const char *string)
{
	return (int)SendMessageA(GetDlgItem(hwndDlg, id), LB_FINDSTRINGEXACT, 0, (LPARAM)string);
}

void FillExtensionList( HWND hwndDlg )
{
	int numTypes;
	sf_command( 0, SFC_GET_FORMAT_MAJOR_COUNT, &numTypes, sizeof( numTypes ) );
	SF_FORMAT_INFO info;
	for ( int i = 0; i < numTypes; i++ )
	{
		info.format = i;
		sf_command( 0, SFC_GET_FORMAT_MAJOR, &info, sizeof( info ) );
		if ( !_strcmpi( info.extension, "mpc" ) )
			continue;

		if ( ExtensionInList( hwndDlg, IDC_EXTENSION_LIST, info.extension ) == LB_ERR )
		{
			LRESULT index = SendMessageA( GetDlgItem( hwndDlg, IDC_EXTENSION_LIST ), LB_ADDSTRING, 0, (LPARAM)info.extension );
			if ( ExtensionExists( info.extension, config_extensions ) )
				SendMessage( GetDlgItem( hwndDlg, IDC_EXTENSION_LIST ), LB_SETSEL, TRUE, index );
		}
	}
}

void FillExtensionsEditControl( HWND hwndDlg )
{
	char extensions[ 256 ] = "";
	char temp[ 20 ] = { 0 };
	
	char *s = config_extensions;

	while ( s && *s )
	{
		lstrcpynA( temp, s, 20 );
		
		char *scan = temp;
		while ( scan && *scan && *scan != ';' )
			scan = CharNextA( scan );

		*scan = 0;

		if ( ExtensionInList( hwndDlg, IDC_EXTENSION_LIST, temp ) == LB_ERR )
		{
			if ( *extensions )
				lstrcatA( extensions, ";" );

			lstrcatA( extensions, temp );
		}

		s += lstrlenA( temp );
		if ( *s == ';' )
			s = CharNextA( s );
	}

	SetDlgItemTextA( hwndDlg, IDC_ADDITIONAL_EXTENSIONS, extensions );
}

void Preferences_Init( HWND hwndDlg )
{
	SendMessageA( GetDlgItem( hwndDlg, IDC_OUTPUTBITS ), CB_ADDSTRING, 0, (LPARAM)"16 bit" ); // TODO: string table
	SendMessageA( GetDlgItem( hwndDlg, IDC_OUTPUTBITS ), CB_ADDSTRING, 0, (LPARAM)"32 bit" ); // TODO: string table

	FillExtensionList( hwndDlg );
	FillExtensionsEditControl( hwndDlg );
}


void Preferences_OnOK( HWND hwndDlg )
{
	config_extensions[ 0 ] = 0;
	LRESULT num = SendMessage( GetDlgItem( hwndDlg, IDC_EXTENSION_LIST ), LB_GETCOUNT, 0, 0 );
	for ( int i = 0; i < num; i++ )
	{
		if ( SendMessage( GetDlgItem( hwndDlg, IDC_EXTENSION_LIST ), LB_GETSEL, i, 0 ) > 0 )
		{
			char thisExtension[ 256 ] = { 0 };
			if ( config_extensions[ 0 ] )
				lstrcatA( config_extensions, ";" );

			SendMessageA( GetDlgItem( hwndDlg, IDC_EXTENSION_LIST ), LB_GETTEXT, i, (LPARAM)thisExtension );
			lstrcatA( config_extensions, thisExtension );
		}
	}

	char additional[ 1024 ] = { 0 };
	GetDlgItemTextA( hwndDlg, IDC_ADDITIONAL_EXTENSIONS, additional, 1024 );
	if ( additional[ 0 ] )
	{
		if ( config_extensions[ 0 ] )
			lstrcatA( config_extensions, ";" );

		lstrcatA( config_extensions, additional );
	}

	SetFileExtensions( config_extensions );
}

BOOL CALLBACK PreferencesDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
			Preferences_Init( hwndDlg );
			break;
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
					Preferences_OnOK( hwndDlg );
					EndDialog( hwndDlg, 0 );
					break;
				case IDCANCEL:
					EndDialog( hwndDlg, 0 );
					break;
			}
	}

	return 0;
}
