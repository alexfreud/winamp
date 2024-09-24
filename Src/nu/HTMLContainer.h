#ifndef NULLSOFT_HTMLCONTAINERH
#define NULLSOFT_HTMLCONTAINERH

#include <ocidl.h>
#include <mshtmhst.h>
#include <mshtmdid.h>
#include <shlobj.h> 
/**************************************************************************
   class definitions
**************************************************************************/


#ifndef DOCHOSTUIFLAG_HOST_NAVIGATES
#define DOCHOSTUIFLAG_HOST_NAVIGATES 0x02000000
#endif 
#ifndef DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION
#define DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION 0x04000000
#endif 
#ifndef DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL
#define DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL 0x08000000
#endif 
#ifndef DOCHOSTUIFLAG_USE_WINDOWED_SELECTCONTROL
#define DOCHOSTUIFLAG_USE_WINDOWED_SELECTCONTROL 0x10000000
#endif 
#ifndef DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE
#define DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE 0x20000000
#endif 



class HTMLContainer : public IOleClientSite,
			public IOleInPlaceSite,
			public IOleInPlaceFrame,
			public IOleControlSite,
			public IDocHostUIHandler,
			public IDispatch
{
protected:
	ULONG m_cRefs;        // ref count

	IUnknown *m_punk;        // IUnknown of contained object
	RECT m_rect;         // size of control
	
	bool bInitialized;
public:
	HWND m_hwnd;         // window handle of the container
	HTMLContainer();
	HTMLContainer(HWND hwnd);
	virtual ~HTMLContainer();

public:
	// *** IUnknown Methods ***
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	// *** IOleInPlaceUIWindow Methods ***
	STDMETHOD (GetBorder)(LPRECT lprectBorder);
	STDMETHOD (RequestBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
	STDMETHOD (SetBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
	STDMETHOD (SetActiveObject)(IOleInPlaceActiveObject * pActiveObject,
	                            LPCOLESTR lpszObjName);
	// *** IOleClientSite Methods ***
	STDMETHOD (SaveObject)();
	STDMETHOD (GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER *ppMk);
	STDMETHOD (GetContainer)(LPOLECONTAINER *ppContainer);
	STDMETHOD (ShowObject)();
	STDMETHOD (OnShowWindow)(BOOL fShow);
	STDMETHOD (RequestNewObjectLayout)();

	// *** IOleWindow Methods ***
	STDMETHOD (GetWindow) (HWND * phwnd);
	STDMETHOD (ContextSensitiveHelp) (BOOL fEnterMode);

	// *** IOleInPlaceSite Methods ***
	STDMETHOD (CanInPlaceActivate) (void);
	STDMETHOD (OnInPlaceActivate) (void);
	STDMETHOD (OnUIActivate) (void);
	STDMETHOD (GetWindowContext) (IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD (Scroll) (SIZE scrollExtent);
	STDMETHOD (OnUIDeactivate) (BOOL fUndoable);
	STDMETHOD (OnInPlaceDeactivate) (void);
	STDMETHOD (DiscardUndoState) (void);
	STDMETHOD (DeactivateAndUndo) (void);
	STDMETHOD (OnPosRectChange) (LPCRECT lprcPosRect);


	// *** IOleInPlaceFrame Methods ***
	STDMETHOD (InsertMenus)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
	STDMETHOD (SetMenu)(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
	STDMETHOD (RemoveMenus)(HMENU hmenuShared);
	STDMETHOD (SetStatusText)(LPCOLESTR pszStatusText);
	STDMETHOD (EnableModeless)(BOOL fEnable);
	STDMETHOD (TranslateAccelerator)(LPMSG lpmsg, WORD wID);

	// *** IOleControlSite Methods ***
	STDMETHOD (OnControlInfoChanged)(void);
	STDMETHOD (LockInPlaceActive)(BOOL fLock);
	STDMETHOD (GetExtendedControl)(IDispatch **ppDisp);
	STDMETHOD (TransformCoords)(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags);
	STDMETHOD (TranslateAccelerator)(LPMSG pMsg, DWORD grfModifiers);
	STDMETHOD (OnFocus)(BOOL fGotFocus);
	STDMETHOD (ShowPropertyFrame)(void);

	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	// *** IDocHostUIHandler Methods ***
	STDMETHOD (ShowContextMenu)(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);
	STDMETHOD (GetHostInfo)(DOCHOSTUIINFO __RPC_FAR *pInfo);
	STDMETHOD (ShowUI)(DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc);
	STDMETHOD (HideUI)(void);
	STDMETHOD (UpdateUI)(void);
	STDMETHOD (OnDocWindowActivate)(BOOL fActivate);
	STDMETHOD (OnFrameWindowActivate)(BOOL fActivate);
	STDMETHOD (ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow);
	STDMETHOD (TranslateAccelerator)(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID);
	STDMETHOD (GetOptionKeyPath)(LPOLESTR __RPC_FAR *pchKey, DWORD dw);
	STDMETHOD (GetDropTarget)(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget);
	STDMETHOD (GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
	STDMETHOD (TranslateUrl)(DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut);
	STDMETHOD (FilterDataObject)(IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet);
	//	STDMETHOD (EnableModeless)(BOOL fEnable);

public:
	void add(CLSID clsid);
	void remove();

	void setLocation(int x, int y, int width, int height);
	void setVisible(BOOL fVisible);
	void setFocus(BOOL fFocus);
	void setStatusWindow(HWND hwndStatus);
	bool translateKey(LPMSG pMsg);
	
	
	virtual void OnBeforeNavigate(void) {}		// deprecated
	virtual void OnNavigateError(void)	{}		// deprecated
	virtual void OnNavigateComplete(void) {}		// deprecated

	virtual void OnBeforeNavigate(IDispatch *pDispatch, LPCWSTR pszURL, DWORD dwFlags, LPCWSTR pszTargetFrameName, VARIANT *vtPostData, LPCWSTR pszHeaders, VARIANT_BOOL *Cancel);
	virtual void OnNavigateError(IDispatch *pDispatch, LPCWSTR pszURL, LPCWSTR pszTargetFrameName, INT nStatusCode, VARIANT_BOOL *Cancel);
	virtual void OnNavigateComplete(IDispatch *pDispatch, LPCWSTR pszURL);
	virtual void OnDocumentComplete(IDispatch *pDisp, LPCWSTR pszURL);
	virtual void OnDownloadBegin(void);
	virtual void OnDownloadComplete(void);
	virtual void OnFileDownload(VARIANT_BOOL *ActiveDocument, VARIANT_BOOL *Cancel);
	virtual void OnNewWindow2(IDispatch **ppDisp, VARIANT_BOOL *Cancel);
	virtual void OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, LPCWSTR pszUrlContext, LPCWSTR pszUrl);
	virtual void OnProgressChange(long Progress, long ProgressMax);
	virtual void OnStatusTextChange(LPCWSTR pszText);
	
	void close();
	virtual BOOL SetHostCSS(LPCWSTR pszHostCSS);
	virtual HWND GetHostHWND(void);
	DWORD SetDownloadFlags(DWORD dwFlags);
	DWORD SetHostInfoFlags(DWORD dwFlags);

	IDispatch *getDispatch();
	IUnknown * getUnknown();
	IWebBrowser2 *m_pweb;
	void SyncSizeToWindow(HWND window);
	IConnectionPoint *GetConnectionPoint(REFIID riid);



	DWORD m_dwCookie;
private:
	wchar_t		*pszHostCSS;
	DWORD		dwDownloadFlags;
	DWORD		dwHostInfoFlags;
};


#endif
