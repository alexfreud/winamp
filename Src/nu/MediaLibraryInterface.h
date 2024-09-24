
#ifndef MEDIALIBRARYINTERFACEH
#define MEDIALIBRARYINTERFACEH

#include <windows.h>
#include <tchar.h>

#include "..\Plugins\General\gen_ml/ml.h"
#include "../winamp/wa_ipc.h"

#ifndef REPLICANT_COMPAT
#include <vector>
#endif


class MediaLibraryInterface
{
public:
	MediaLibraryInterface() : httpRetrieveFile( 0 )                  {}

	// children communicate back to the media library by SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,param,ML_IPC_X);
	int            AddTreeImage( int resourceId ); // returns index of the image.
	int            AddTreeImage( int resourceId, int imageIndex, BMPFILTERPROC filter );
	int            AddTreeImageBmp( int resourceId );
	void           AddTreeItem( MLTREEITEM &newItem );
	void           AddTreeItem( MLTREEITEMW &newItem );
	void           SetTreeItem( MLTREEITEM &item );
	void           SetTreeItem( MLTREEITEMW &item );
	void           RemoveTreeItem( INT_PTR treeId );
	void           SelectTreeItem( INT_PTR treeId );
	void           UpdateTreeItem( MLTREEITEMINFO &newItem );
	void           UpdateTreeItem( MLTREEITEMINFOW &newItem );
	INT_PTR        GetSelectedTreeItem( void );
	void           RenameTreeId( INT_PTR treeId, const wchar_t *newName );

	INT_PTR        GetChildId( INT_PTR id );
	INT_PTR        GetNextId( INT_PTR id );

	void           AddToSendTo( char description[], INT_PTR context, INT_PTR unique );
	void           AddToSendTo( wchar_t description[], INT_PTR context, INT_PTR unique );
	void           BranchSendTo( INT_PTR context );
	void           AddToBranchSendTo( const wchar_t description[], INT_PTR context, INT_PTR unique );
	void           EndBranchSendTo( const wchar_t description[], INT_PTR context );

	const char    *GetIniDirectory();
	const wchar_t *GetIniDirectoryW();
	const char    *GetWinampIni();
	const wchar_t *GetWinampIniW();
	void           BuildPath( const wchar_t *pathEnd, wchar_t *path, size_t numChars );


	int            SkinList( HWND list );
	void           UnskinList( int token );
	void           ListViewShowSort( int token, BOOL show );
	void           ListViewSort( int token, int columnIndex, BOOL ascending );
	void           ListSkinUpdateView( int listSkin );

	void           OpenURL( const wchar_t *url )                     { SendMessage( winamp, WM_WA_IPC, (WPARAM)url, IPC_OPEN_URL ); }

	int            SkinComboBox( HWND comboBox );
	void           UnskinComboBox( int token );
	
	void          *GetWADLGFunc( int num );
	
	#ifndef REPLICANT_COMPAT
	void           PlayStreams( std::vector<const wchar_t *> &urls, bool force = false );
	#endif
	
	void           PlayStream( wchar_t *url, bool force = false );
	void           PlayFile( const wchar_t *url );
	void           EnqueueFile( const wchar_t *url );
	void           EnqueueStream( wchar_t *url, bool force = true );
	void           GetFileInfo( const char *filename, char *title, int titleCch, int *length );
	void           GetFileInfo( const wchar_t *filename, wchar_t *title, int titleCch, int *length );

	void           ClearPlaylist()                                       { SendMessage( winamp, WM_WA_IPC, 0, IPC_DELETE ); }
	void           SwitchToPluginView( int itemId )                      { SendMessage( library, WM_ML_IPC, itemId, ML_IPC_SETCURTREEITEM ); }
	int            GetWinampVersion()                                    { return (int)(INT_PTR)SendMessage( winamp, WM_WA_IPC, 0, IPC_GETVERSION ); }
	
	HWND           library         = 0;
	HWND           winamp          = 0;
	
	HINSTANCE      instance        = 0;
	
	const char    *iniDirectory    = 0;
	const wchar_t *iniDirectoryW   = 0;
	
	const char    *winampIni       = 0;
	const wchar_t *winampIniW      = 0;
	
	const char    *pluginDirectory = 0;
	
	void           AddPreferences( prefsDlgRec &prefs )               { SendMessage( winamp, WM_WA_IPC, (WPARAM)&prefs, IPC_ADD_PREFS_DLG ); }
	void           AddPreferences( prefsDlgRecW &prefs )              { SendMessage( winamp, WM_WA_IPC, (WPARAM)&prefs, IPC_ADD_PREFS_DLGW ); }
	void           InsertTreeItem( MLTREEITEM &newItem );
	void           InsertTreeItem( MLTREEITEMW &newItem );

	char           *GetProxy()
	{
		char *proxy = (char *)SendMessage( winamp, WM_WA_IPC, 0, IPC_GET_PROXY_STRING );
		if ( proxy == (char *)1 || strlen(proxy) == 0 )
			return 0;
		else
			return proxy;
	}

	void           BuildPluginPath( const TCHAR *filename, TCHAR *path, size_t pathSize );

	int ( *httpRetrieveFile )( HWND hwnd, const char *url, const char *file, const char *dlgtitle );

	IDispatch     *GetDispatchObject();
	int            GetUniqueDispatchId();
	void           ListSkinDisableHorizontalScrollbar( int listSkin );

	void           AddToMediaLibrary( const char *filename );
	void           AddToMediaLibrary( const wchar_t *filename );

	void           GoToPreferences( int id )                          { SendMessage( winamp, WM_WA_IPC, id, IPC_OPENPREFSTOPAGE ); }
	void           GoToPreferences( prefsDlgRec &prefs )              { SendMessage( winamp, WM_WA_IPC, (WPARAM)&prefs, IPC_OPENPREFSTOPAGE ); }
	void           GoToPreferences( prefsDlgRecW &prefs )             { SendMessage( winamp, WM_WA_IPC, (WPARAM)&prefs, IPC_OPENPREFSTOPAGE ); }

	const char    *GetFontName()                                      { return (const char *)SendMessage( winamp, WM_WA_IPC, 1, IPC_GET_GENSKINBITMAP ); }
	int            GetFontSize()                                      { return (int)SendMessage( winamp, WM_WA_IPC, 3, IPC_GET_GENSKINBITMAP ); }

	void           AddBookmark( char *bookmark )
	{
		COPYDATASTRUCT cds;
		cds.dwData = IPC_ADDBOOKMARK;
		cds.lpData = bookmark;
		cds.cbData = (int)strlen( bookmark ) + 1;
		
		SendMessage( winamp, WM_COPYDATA, NULL, (LPARAM)&cds );
	}

	void           AddBookmarkW( wchar_t *bookmark )
	{
		COPYDATASTRUCT cds;
		cds.dwData = IPC_ADDBOOKMARKW;
		cds.lpData = bookmark;
		cds.cbData = (int)( sizeof( wchar_t ) * wcslen( bookmark ) + sizeof( wchar_t ) );
		
		SendMessage( winamp, WM_COPYDATA, NULL, (LPARAM)&cds );
	}

	void           ShowMediaLibrary()                                 { SendMessage( library, WM_ML_IPC, 0, ML_IPC_ENSURE_VISIBLE ); }

	const char    *GetExtensionList()                                 { return (const char *)SendMessage( winamp, WM_WA_IPC, 0, IPC_GET_EXTLIST ); }
	bool           IsShowing()                                        { return !!SendMessage( library, WM_ML_IPC, 0, ML_IPC_IS_VISIBLE ); }
	void           RefreshPrefs( int prefs )                          { SendMessage( library, WM_ML_IPC, prefs, ML_IPC_REFRESH_PREFS ); }
	void           GracenoteCancelRequest()                           { SendMessage( library, WM_ML_IPC, GRACENOTE_CANCEL_REQUEST, ML_IPC_GRACENOTE ); }// TODO: should we post this?

	DWORD          AddDispatch( wchar_t *name, IDispatch *object );
};

extern MediaLibraryInterface mediaLibrary;

#endif
