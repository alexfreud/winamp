// feedback.cpp : Defines the entry point for the application.
//
#include ".\main.h"
#include <commctrl.h>
#include <strsafe.h>
Settings settings;

int WINAPI wWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow) 
{
	wchar_t *argv[2] = {0};
	int argc = 0;
	if (lpszCmdLine && wcslen(lpszCmdLine) >0) argc = ParseCommandLine(lpszCmdLine, NULL);
	if (argc != 1)
	{
		MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMS),0};
		msgbx.lpszText = L"Winamp Error Reporter\nCopyright © 2005-2014 Winamp SA";
		msgbx.lpszCaption = L"About...";
		msgbx.lpszIcon = MAKEINTRESOURCEW(ICON_XP);
		msgbx.hInstance = GetModuleHandle(0);
		msgbx.dwStyle = MB_USERICON;
		MessageBoxIndirectW(&msgbx);
		return 0;
	}

	ParseCommandLine(lpszCmdLine, argv);
	settings.SetPath(argv[0]);

	if (!settings.Load())
	{
		MessageBox(NULL, L"Unable to load settings.", L"Error", MB_OK);
		return 0;
	}

	INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_WIN95_CLASSES};
    InitCommonControlsEx(&icex);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DLG_SILENT), NULL, (DLGPROC)SilentDlgProc, (LPARAM)hInstance);
	return 0;
}

static int ParseCommandLine(wchar_t *cmdline, wchar_t **argv)
{
	wchar_t *bufp;
    int argc;
    argc = 0;
    for ( bufp = cmdline; *bufp; )
	{
		/* Skip leading whitespace */
		while ( isspace(*bufp) ) ++bufp;
		/* Skip over argument */
		if ( *bufp == L'"' )
		{
				++bufp;
				if ( *bufp )
				{
						if ( argv ) argv[argc] = bufp;
						++argc;
				}
				/* Skip over word */
				while ( *bufp && (*bufp != L'"') ) ++bufp;
			}
		else 
		{
			if ( *bufp ) 
			{
					if ( argv ) argv[argc] = bufp;
					++argc;
			}
			/* Skip over word */
			while ( *bufp && ! isspace(*bufp) ) ++bufp;
		}
		if ( *bufp ) 
		{
				if ( argv ) *bufp = L'\0';
				++bufp;
		}
    }
    if ( argv ) argv[argc] = NULL;
    return(argc);
}