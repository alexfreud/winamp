#ifndef NULLSOFT_HTMLCONTAINERH
#define NULLSOFT_HTMLCONTAINERH

#include <ocidl.h>
#include <mshtmhst.h>
#include <mshtmdid.h>
#include <shlobj.h> 
#include <urlmon.h>
/**************************************************************************
   class definitions
**************************************************************************/


#ifndef DOCHOSTUIFLAG_HOST_NAVIGATES
#define DOCHOSTUIFLAG_HOST_NAVIGATES                 0x02000000
#endif 
#ifndef DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION
#define DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION   0x04000000
#endif 
#ifndef DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL
#define DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL   0x08000000
#endif 
#ifndef DOCHOSTUIFLAG_USE_WINDOWED_SELECTCONTROL
#define DOCHOSTUIFLAG_USE_WINDOWED_SELECTCONTROL     0x10000000
#endif 
#ifndef DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE
#define DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE 0x20000000
#endif 

class HTMLContainer2;

typedef BOOL( CALLBACK *BROWSERCB )( HTMLContainer2 *pContiner, DISPID dispId, DISPPARAMS FAR *pDispParams, LPVOID pUser ); // return TRUE to block normal processing

typedef enum tagCONTAINERSTYLE
{
	CSTYLE_NORMAL                 = 0x00, // nothing
	CSTYLE_NAVIGATE2_NOCLICKSOUND = 0x01, // prevents click sound in Nvigate2 calls
	CSTYLE_NOCLICKSOUND           = 0x02, // prevents all click sounds ( requires testing )

} CONTAINERSTYLE;


BOOL HTMLContainer2_Initialize();
BOOL HTMLContainer2_Uninitialize();

class HTMLContainer2 : public IOleClientSite,
	public IOleInPlaceSite,
	public IOleInPlaceFrame,
	public IOleControlSite,
	public IDocHostUIHandler2,
	public IDocHostShowUI,
	public IOleCommandTarget,
	public IServiceProvider,
	public IDispatch
{

	public:
	typedef enum
	{
		uiToolbar    = 1,
		uiStatusbar  = 2,
		uiMenubar    = 3,
		uiAddressbar = 4,
	} uiElement;

	typedef enum
	{
		wndLeft     = 0x0001,
		wndTop      = 0x0002,
		wndWidth    = 0x0004,
		wndHeight   = 0x0008,
		wndRelative = 0x0010,
	} windowPosFlags;

	typedef enum
	{
		msgNavigate2      = 0,
		msgNavigateToName = 1,
	} redirectedMessage;

	protected:
	HTMLContainer2( HWND waWindow, HWND hwndParent );
	virtual ~HTMLContainer2( void );

	public:
	// *** IUnknown Methods ***
	STDMETHOD( QueryInterface )( REFIID riid, PVOID *ppvObject );
	STDMETHOD_( ULONG, AddRef )( void );
	STDMETHOD_( ULONG, Release )( void );

	protected:
	// *** IOleInPlaceUIWindow Methods ***
	STDMETHOD( GetBorder )( LPRECT lprectBorder );
	STDMETHOD( RequestBorderSpace )( LPCBORDERWIDTHS lpborderwidths );
	STDMETHOD( SetBorderSpace )( LPCBORDERWIDTHS lpborderwidths );
	STDMETHOD( SetActiveObject )( IOleInPlaceActiveObject *pActiveObject,
								  LPCOLESTR lpszObjName );
	// *** IOleClientSite Methods ***
	STDMETHOD( SaveObject )( );
	STDMETHOD( GetMoniker )( DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER *ppMk );
	STDMETHOD( GetContainer )( LPOLECONTAINER *ppContainer );
	STDMETHOD( ShowObject )( );
	STDMETHOD( OnShowWindow )( BOOL fShow );
	STDMETHOD( RequestNewObjectLayout )( );

	// *** IOleWindow Methods ***
	STDMETHOD( GetWindow ) ( HWND *phwnd );
	STDMETHOD( ContextSensitiveHelp ) ( BOOL fEnterMode );

	// *** IOleInPlaceSite Methods ***
	STDMETHOD( CanInPlaceActivate ) ( void );
	STDMETHOD( OnInPlaceActivate ) ( void );
	STDMETHOD( OnUIActivate ) ( void );
	STDMETHOD( GetWindowContext ) ( IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo );
	STDMETHOD( Scroll ) ( SIZE scrollExtent );
	STDMETHOD( OnUIDeactivate ) ( BOOL fUndoable );
	STDMETHOD( OnInPlaceDeactivate ) ( void );
	STDMETHOD( DiscardUndoState ) ( void );
	STDMETHOD( DeactivateAndUndo ) ( void );
	STDMETHOD( OnPosRectChange ) ( LPCRECT lprcPosRect );

	// *** IOleInPlaceFrame Methods ***
	STDMETHOD( InsertMenus )( HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths );
	STDMETHOD( SetMenu )( HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject );
	STDMETHOD( RemoveMenus )( HMENU hmenuShared );
	STDMETHOD( SetStatusText )( LPCOLESTR pszStatusText );
	STDMETHOD( EnableModeless )( BOOL fEnable );
	STDMETHOD( TranslateAccelerator )( LPMSG lpmsg, WORD wID );

	// *** IOleControlSite Methods ***
	STDMETHOD( OnControlInfoChanged )( void );
	STDMETHOD( LockInPlaceActive )( BOOL fLock );
	STDMETHOD( GetExtendedControl )( IDispatch **ppDisp );
	STDMETHOD( TransformCoords )( POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags );
	STDMETHOD( TranslateAccelerator )( LPMSG pMsg, DWORD grfModifiers );
	STDMETHOD( OnFocus )( BOOL fGotFocus );
	STDMETHOD( ShowPropertyFrame )( void );

	// *** IDispatch Methods ***
	STDMETHOD( GetIDsOfNames )( REFIID riid, OLECHAR FAR *FAR *rgszNames, unsigned int cNames, LCID lcid, DISPID FAR *rgdispid );
	STDMETHOD( GetTypeInfo )( unsigned int itinfo, LCID lcid, ITypeInfo FAR *FAR *pptinfo );
	STDMETHOD( GetTypeInfoCount )( unsigned int FAR *pctinfo );
	STDMETHOD( Invoke )( DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR *pexecinfo, unsigned int FAR *puArgErr );

	// *** IDocHostUIHandler Methods ***
	STDMETHOD( ShowContextMenu )( DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved );
	STDMETHOD( GetHostInfo )( DOCHOSTUIINFO __RPC_FAR *pInfo );
	STDMETHOD( ShowUI )( DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc );
	STDMETHOD( HideUI )( void );
	STDMETHOD( UpdateUI )( void );
	STDMETHOD( OnDocWindowActivate )( BOOL fActivate );
	STDMETHOD( OnFrameWindowActivate )( BOOL fActivate );
	STDMETHOD( ResizeBorder )( LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow );
	STDMETHOD( TranslateAccelerator )( LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID );
	STDMETHOD( GetOptionKeyPath )( LPOLESTR __RPC_FAR *pchKey, DWORD dw );
	STDMETHOD( GetDropTarget )( IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget );
	STDMETHOD( GetExternal )( IDispatch __RPC_FAR *__RPC_FAR *ppDispatch );
	STDMETHOD( TranslateUrl )( DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut );
	STDMETHOD( FilterDataObject )( IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet );
	//	STDMETHOD (EnableModeless)(BOOL fEnable);

		// *** IDocHostUIHandler2 Methods ***
	STDMETHOD( GetOverrideKeyPath )( LPOLESTR __RPC_FAR *pchKey, DWORD dw );

	// *** IDocHostShowUI ***
	STDMETHOD( ShowHelp )( HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit );
	STDMETHOD( ShowMessage )( HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult );

	/*** IOleCommandTarget ***/
	STDMETHOD( QueryStatus )( const GUID *pguidCmdGroup, ULONG cCmds, OLECMD *prgCmds, OLECMDTEXT *pCmdText );
	STDMETHOD( Exec )( const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut );

	/*** IServiceProvider ***/
	STDMETHOD( QueryService )( REFGUID guidService, REFIID riid, void **ppv );

	public:
	STDMETHOD( Initialize )( void );
	STDMETHOD( Finish )( void );

	STDMETHOD( UnadviseBrowserEvents )( void );

	STDMETHOD( SetLocation )( int x, int y, int width, int height );
	STDMETHOD( SetFocus )( BOOL fFocused );
	virtual BOOL TranslateKey( LPMSG pMsg );

	HWND GetHostHWND( void );
	HWND GetParentHWND( void );

	HRESULT NavigateEx( IWebBrowser2 *pWeb2, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers );
	HRESULT Navigate2( VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers );

	HRESULT PostNavigate2( VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers ); // navigate using postmessage API
	HRESULT NavigateToName( LPCWSTR pszUrl, UINT fFlags );
	HRESULT NavigateToNameEx( IWebBrowser2 *pWeb2, LPCWSTR pszUrl, UINT fFlags );
	HRESULT PostNavigateToName( LPCWSTR pszUrl, UINT fFlags );
	HRESULT WriteHTML( LPCWSTR pszHTML );
	HRESULT WriteDocument( BSTR data ); // if succeeded will free bstr
	HRESULT InvokeScriptFunction( LPCWSTR pszFuncName, LCID lcid, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, UINT FAR *puArgErr );

	HRESULT GetIDispatch( IDispatch **pDisp );
	HRESULT GetIUnknown( IUnknown **pUnk );
	HRESULT GetIWebBrowser2( IWebBrowser2 **pWeb2 );

	// Registers cursors to use with browser// set hCurToUse = NULL to remove
	// hCurToUse will be destryoed using DestroyCursor, make sure that this is not shared resource (use CopyCursor)
	STDMETHOD( RegisterBrowserCursor )( INT nSysCurID, HCURSOR hCurToUse );

	// used by MTBrowser
	BROWSERCB RegisterBrowserEventCB( BROWSERCB fnBrowserCB, LPVOID pUserData );

	static HRESULT InternetSetFeatureEnabled( INTERNETFEATURELIST FeatureEntry, DWORD dwFlags, BOOL fEnable );
	static HRESULT InternetIsFeatureEnabled( INTERNETFEATURELIST FeatureEntry, DWORD dwFlags );

	protected:
	virtual void OnBeforeNavigate( IDispatch *pDispatch, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel );

	virtual void OnNavigateError( IDispatch *pDispatch, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel ){}
	virtual void OnNavigateComplete( IDispatch *pDispatch, VARIANT *URL )                                                                  {}
	virtual void OnDocumentComplete( IDispatch *pDispatch, VARIANT *URL )                                                                  {}
	virtual void OnDocumentReady( IDispatch *pDispatch, VARIANT *URL )                                                                     {} // top frame OnDocumentComplete
	virtual void OnDownloadBegin( void )                                                                                                   {}
	virtual void OnDownloadComplete( void )                                                                                                {}
	virtual void OnFileDownload( VARIANT_BOOL *ActiveDocument, VARIANT_BOOL *Cancel )                                                      {}
	virtual void OnNewWindow2( IDispatch **ppDisp, VARIANT_BOOL *Cancel )                                                                  {}
	virtual void OnNewWindow3( IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl )                {}
	virtual void OnProgressChange( long Progress, long ProgressMax )                                                                       {}
	virtual void OnStatusTextChange( LPCWSTR pszText )                                                                                     {}
	virtual void OnCommandStateChange( LONG commandId, VARIANT_BOOL Enable )                                                               {}
	virtual void OnSetSecureLockIcon( UINT secureLockIcon )                                                                                {}
	virtual void OnNavigateCancelled( LPCWSTR pszUrl, VARIANT_BOOL *Cancel )                                                               {}
	virtual void OnTitleChange( BSTR pszText )                                                                                             {}
	virtual void OnVisibleChange( VARIANT_BOOL fVisible )                                                                                  {}
	virtual void OnWindowClosing( VARIANT_BOOL IsChildWindow, VARIANT_BOOL *Cancel )                                                       {}
	virtual void OnShowUiElement( UINT elementId, VARIANT_BOOL fSHow )                                                                     {}
	virtual void OnWindowSetResizable( VARIANT_BOOL Enable )                                                                               {}
	virtual void OnClientToHostWindow( LONG *CX, LONG *CY )                                                                                {}
	virtual void OnSetWindowPos( UINT flags, LONG x, LONG y, LONG cx, LONG cy )                                                            {}
	virtual void OnEnableFullscreen( VARIANT_BOOL Enable )                                                                                 {}

	virtual COLORREF OnGetHostBkColor( void );
	virtual DWORD OnGetHostInfoFlags( void );
	virtual OLECHAR *OnGetHostCSS( void );			// use CoTaskMemAlloc to allocate string
	virtual OLECHAR *OnGetHostNamespace( void );	// use CoTaskMemAlloc to allocate string
	virtual DWORD OnGetDownlodFlags( void );
	virtual LPCWSTR OnGetUserAgent( void );

	virtual DWORD GetContainerStyle( void );

	BOOL ValidateURLHost( LPCWSTR pszUrl );

	HRESULT IsFrameset( IWebBrowser2 *pWeb2 );
	HRESULT GetFramesCount( IWebBrowser2 *pWeb2, INT *frameCount );

	HRESULT GetAppVersion( BSTR *p );
	HRESULT GetUserAgent( BSTR *p );

	virtual HANDLE InitializePopupHook( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )                                               { return NULL; }
	virtual void DeletePopupHook( HANDLE hHoook )                                                                                          {}
	virtual void InitializeMenuPopup( HWND hwnd, HMENU hMenu, INT iPos, BOOL fWindowMenu )                                                 {}

	virtual void ProcessRedirectedMessage( HWND hwnd, UINT messageId, LPARAM param );
	HRESULT PostRedirectMessage( UINT messageId, LPARAM param );

	virtual BOOL InputLangChangeRequest( HWND hwnd, UINT flags, HKL hkl )                                                                  { return FALSE; }
	virtual void InputLangChange( UINT charset, HKL hkl )                                                                                  {}
	virtual void OnClosePopupInternal()                                                                                                    {}

	private:
	///  helpers
	friend static HRESULT HTMLContainer2_OnShowUiElementHelper( HTMLContainer2 *instance, UINT elementId, DISPPARAMS *pDispParams );

	protected:
	LONG		ref;				// ref count
	IUnknown *pUnk;				// IUnknown of contained object
	RECT		rect;				// 
	HWND		hParent;			// window handle of the container

	private:
	DWORD		dwCookie;
	DWORD		dwFlags;
	BROWSERCB	fnBrwoserCB;
	LPVOID		userData;
	BOOL		bNavigating;
	VOID *hCursors;
	INT			nCursors;
	BOOL		ensureChakraLoaded;
	HWND		winampWindow;

	friend static LRESULT SubclassControl_WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

#endif