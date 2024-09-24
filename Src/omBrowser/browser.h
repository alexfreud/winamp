#ifndef NULLSOFT_WINAMP_OMBROWSER_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/HTMLContainer2.h"
#include "./browserInternal.h"

class obj_ombrowser;
class Browser;
class ifc_omdebugconfig;
class ifc_omservice;
class ifc_travelloghelper;

typedef void (CALLBACK *BHCALLBACK)(Browser* /*browser*/);
typedef void (CALLBACK *BHNAVCOMPLETECALLBACK)(Browser* /*browser*/, IDispatch* /*pDispatch*/, VARIANT* /*URL*/);
typedef void (CALLBACK *BHCMDSTATECALLBACK)(Browser* /*browser*/, INT /*commandId*/, BOOL /*fEnabled*/);
typedef void (CALLBACK *BHTEXTCALLBACK)(Browser* /*browser*/, LPCWSTR /*pszText*/);
typedef HRESULT (CALLBACK *BHSERVICECALLBACK)(Browser* /*browser*/, ifc_omservice** /*ppService*/);
typedef LRESULT (CALLBACK *BHMSGCALLBACK)(Browser* /*browser*/, MSG* /*pMsg*/);
typedef void (CALLBACK *BHCREATEPOPUPCALLBACK)(Browser* /*browser*/, IDispatch** /*ppDisp*/, VARIANT_BOOL* /*Cancel*/);
typedef void (CALLBACK *BHBOOLCALLBACK)(Browser* /*browser*/, VARIANT_BOOL /*Visible*/);
typedef void (CALLBACK *BHCLOSECALLBACK)(Browser* /*browser*/, VARIANT_BOOL /*IsChild*/, VARIANT_BOOL* /*Cancel*/);
typedef void (CALLBACK *BHSHOWUICALLBACK)(Browser* /*browser*/, UINT /*elementId*/, VARIANT_BOOL /*fShow*/);
typedef void (CALLBACK *BHCLIENTTOHOSTCALLBACK)(Browser* /*browser*/, LONG* /*CX*/, LONG* /*CY*/);
typedef void (CALLBACK *BHFOCUSCHANGECALLBACK)(Browser* /*browser*/, VARIANT_BOOL* /*fAllow*/);
typedef void (CALLBACK *BHWINDOWPOSCALLBACK)(Browser* /*browser*/, UINT /*Flags*/, LONG /*X*/, LONG /*Y*/, LONG /*Width*/, LONG /*Height*/);


class Browser : public HTMLContainer2, 
				public IDropTarget, 
				public IProtectFocus,
				public IHTMLOMWindowServices,
				public INewWindowManager
{
public:
	typedef enum
	{
		commandNone = 0,
		commandBack = 1,
		commandForward = 2,
		commandStop = 3,
		commandRefresh = 4,
		commandRefreshCompletely = 5,
	} Commands;

	typedef enum
	{
		commandStateSupported = 1,
		commandStateEnabled = 2,
		commandStateLatched = 4,
	} CommandStates;

	typedef enum 
	{
		flagUiDisableScroll	= 0x00000001,
		flagUiDisableContextMenu	 = 0x00000002,
		flagUiDialogMode = 0x00000004,
		flagUiDisableHostCss = 0x00000008,
	} UiFlags;

protected:
	Browser(obj_ombrowser *browserMngr, HWND winampWindow, HWND hParent);
	~Browser();

public:
	static Browser *CreateInstance(obj_ombrowser *browserManager, HWND winampWindow, HWND hParent);

public:

	/*** IUnknown ***/
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	STDMETHOD (QueryInterface)(REFIID, LPVOID*);
	
	/*** IDropTarget ***/
	STDMETHOD (DragEnter)(IDataObject *, DWORD, POINTL, DWORD*);
    STDMETHOD (DragOver)(DWORD, POINTL, DWORD*);
    STDMETHOD (DragLeave)(void);
    STDMETHOD (Drop)(IDataObject*, DWORD, POINTL, DWORD*);

	STDMETHOD (GetDropTarget)(IDropTarget*, IDropTarget **);
	
	STDMETHOD (GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
	STDMETHOD (ShowContextMenu)(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);
	STDMETHOD (ShowMessage)(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult);


	// *** IDocHostUIHandler ***
	STDMETHOD (TranslateAccelerator)(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID);

	/*** IDocHostUIHandler2 ***/
	STDMETHOD (GetOverrideKeyPath)(LPOLESTR __RPC_FAR *pchKey, DWORD dw);

	/*** IOleCommandTarget ***/
	STDMETHOD (Exec)(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);
	
	/*** IServiceProvider ***/
	STDMETHOD (QueryService)(REFGUID guidService, REFIID riid, void **ppv);

	/*** IProtectFocus ***/
	STDMETHOD (AllowFocusChange)(BOOL *pfAllow);

	/*** IHTMLOMWindowServices ***/
	STDMETHOD (moveTo)(LONG x, LONG y);
    STDMETHOD (moveBy)(LONG x, LONG y);        
    STDMETHOD (resizeTo)(LONG x, LONG y);        
	STDMETHOD (resizeBy)(LONG x, LONG y);

	/*** INewWindowManager ***/
	STDMETHOD (EvaluateNewWindow)(LPCWSTR pszUrl, LPCWSTR pszName, LPCWSTR pszUrlContext, LPCWSTR pszFeatures, BOOL fReplace, DWORD dwFlags, DWORD dwUserActionTime);


	STDMETHOD (Initialize)(BOOL fRegisterAsBrowser);
	STDMETHOD (Finish)(void);

	HRESULT SetExternal(IDispatch *pDispatch);
	HRESULT SendCommand(INT commandId);
	HRESULT QueryCommandState(INT commandId, INT *commandState);
	UINT GetSecueLockIcon() { return secureLockIcon; }

	BOOL TranslateKey(LPMSG pMsg);

	/*Events*/
	BHNAVCOMPLETECALLBACK  EventDocumentReady;
	BHNAVCOMPLETECALLBACK  EventNavigateComplete;
	BHCALLBACK             EventDownloadBegin;
	BHCALLBACK             EventDownloadComplete;
	BHCALLBACK             EventContainerDestroyed;
	BHCMDSTATECALLBACK     EventCommandStateChange;
	BHTEXTCALLBACK         EventStatusChange;
	BHTEXTCALLBACK         EventTitleChange;
    BHCALLBACK             EventSecureLockIconChange;
	BHCREATEPOPUPCALLBACK  EventCreatePopup;
	BHBOOLCALLBACK         EventVisible;
	BHBOOLCALLBACK         EventSetResizable;
	BHCLOSECALLBACK        EventWindowClosing;
	BHSHOWUICALLBACK       EventShowUiElement;
	BHCLIENTTOHOSTCALLBACK EventClientToHost;
	BHWINDOWPOSCALLBACK    EventSetWindowPos;
	BHFOCUSCHANGECALLBACK  EventFocusChange;
	BHBOOLCALLBACK         EventSetFullscreen;
	BHCALLBACK             EventClosePopup;

	BHSERVICECALLBACK CallbackGetOmService;
	BHMSGCALLBACK CallbackRedirectKey;

	HRESULT GetExternalName(LPWSTR pszBuffer, INT cchBufferMax);

	void SetUiFlags(UINT flags, UINT mask);
	UINT GetUiFlags(UINT mask);

	HRESULT ToggleFullscreen();
	HRESULT GetTravelLog(ifc_travelloghelper **travelLog);

protected:

	void OnBeforeNavigate(IDispatch *pDispatch, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel);
	void OnDownloadBegin(void);
	void OnDownloadComplete(void);
	void OnNavigateComplete(IDispatch *pDispatch, VARIANT *URL);
	void OnDocumentReady(IDispatch *pDispatch, VARIANT *URL);
	void OnNavigateError(IDispatch *pDispatch, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel);
	void OnCommandStateChange(LONG commandId, VARIANT_BOOL Enable);
	void OnStatusTextChange(LPCWSTR pszText);
	void OnSetSecureLockIcon(UINT secureLockIcon);
	void OnNavigateCancelled(LPCWSTR pszUrl, VARIANT_BOOL *Cancel);
	void OnNewWindow2(IDispatch **ppDisp, VARIANT_BOOL *Cancel);
	void OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
	void OnTitleChange(BSTR pszText);
	void OnVisibleChange(VARIANT_BOOL fVisible);
	void OnWindowClosing(VARIANT_BOOL IsChildWindow, VARIANT_BOOL *Cancel);
	void OnShowUiElement(UINT elementId, VARIANT_BOOL fShow);
	void OnWindowSetResizable(VARIANT_BOOL Enable);
	void OnEnableFullscreen(VARIANT_BOOL Enable);
	void OnClientToHostWindow(LONG *CX, LONG *CY);
	void OnSetWindowPos(UINT Flags, LONG X, LONG Y, LONG CX, LONG CY);

	virtual COLORREF OnGetHostBkColor(void);
	virtual DWORD OnGetHostInfoFlags(void);
	virtual OLECHAR* OnGetHostCSS(void);
	virtual DWORD OnGetDownlodFlags(void);
	virtual LPCWSTR OnGetUserAgent(void);

	HRESULT FormatErrorParam(LPWSTR pszBuffer, INT cchBufferMax, UINT errorCode, LPCWSTR pszUrl);

	HANDLE InitializePopupHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void DeletePopupHook(HANDLE hHook);
	void InitializeMenuPopup(HWND hwnd, HMENU hMenu, INT iPos, BOOL fWindowMenu);

	BOOL InputLangChangeRequest(HWND hwnd, UINT flags, HKL hkl);
	void InputLangChange(UINT charset, HKL hkl);
	void OnClosePopupInternal();

	HRESULT GetDebugConfig(ifc_omdebugconfig **debugConfig);
	HRESULT GetErrorPageName(LPWSTR pszBuffer, HRESULT cchBufferMax, UINT errorCode, BOOL fCancel);

private:
	typedef enum
	{
		navigationForwardEnabled = 0x0001,
		navigationBackEnabled = 0x0002,
		navigationActive = 0x0004,
	} navigationState;

private:
	obj_ombrowser *browserManager;
	IDispatch	*externalDisp;
	IDropTargetHelper *pDropTargetHerlper;
	UINT navigationState;
	UINT secureLockIcon;
	WCHAR szDone[64];
	LPWSTR pszUserAgent;
	UINT uiFlags;
};

#ifdef _DEBUG
	void BrowserDebug_PrintRefs(Browser *browser);
#else
	#define BrowserDebug_PrintRefs(x)
#endif //_DEBUG

#endif //NULLSOFT_WINAMP_OMBROWSER_HEADER