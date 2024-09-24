#ifndef NULLSOFT_HTMLCONTAINERH
#define NULLSOFT_HTMLCONTAINERH

#include <oleidl.h>
#include <ocidl.h>
//#import	<system32/macromed/Flash/Flash9e.ocx> /*no_namespace, */named_guids, raw_interfaces_only, exclude("IServiceProvider")
#include "Flash9e.tlh"
#include "FlashDispInterface.h"
//#include <shlobj.h> 

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


class SWFContainer : public IOleClientSite,
			public IOleInPlaceSite,
			public IOleInPlaceFrame,
			public IOleControlSite,
			public IDispatch
{
protected:
	ULONG m_cRefs;        // ref count

	IUnknown *m_punk;        // IUnknown of contained object
	RECT m_rect;         // size of control
	
	bool bInitialized;
public:
	HWND m_hwnd;         // window handle of the container
	SWFContainer(HWND hwnd);
	virtual ~SWFContainer();

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
	STDMETHOD (Invoke) (
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);


public:
	void add(CLSID clsid);
	void remove();

	void setLocation(int x, int y, int width, int height);
	void setVisible(BOOL fVisible);
	void setFocus(BOOL fFocus);
	void setStatusWindow(HWND hwndStatus);
	bool translateKey(LPMSG pMsg);
	

	void close();

	IUnknown * getUnknown();
	void SyncSizeToWindow(HWND window);
	IConnectionPoint *GetConnectionPoint(REFIID riid);
	DWORD m_dwCookie;
	struct ShockwaveFlashObjects::IShockwaveFlash *flash;
	FlashDispInterface *externalInterface;
private:
	
};


#endif
