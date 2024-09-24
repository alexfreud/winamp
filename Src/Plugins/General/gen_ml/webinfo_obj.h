#ifndef NULLSOFT_MLDISC_MINIINFO_HEADER
#define NULLSOFT_MLDISC_MINIINFO_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../nu/HTMLContainer2.h"

#define HOMEPAGE_NOTLOADED	0x00
#define HOMEPAGE_LOADING	0x01
#define HOMEPAGE_FAILED		0x02
#define HOMEPAGE_LOADED		0x03

class WebFileInfo;

WebFileInfo* CreateWebFileInfo(HWND hwndParent, IDispatch *pDispWA);

class WebFileInfo : public HTMLContainer2, public IDropTarget
{

protected:
	WebFileInfo(HWND hwndParent, IDispatch *pDispWA);
	~WebFileInfo(void);

public:
	HRESULT InvokeFileInfo(LPCWSTR pszFileName);
	HRESULT NavigateToPage(void);
	HRESULT DisplayMessage(LPCWSTR pszMessage, BOOL bPostIt);
	HRESULT UpdateColors(void);

	// IUnknown 
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	// IDropTarget 
    STDMETHOD (DragEnter)(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    STDMETHOD (DragOver)(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    STDMETHOD (DragLeave)(void);
    STDMETHOD (Drop)(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	STDMETHOD (GetDropTarget)(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget);

protected:
	STDMETHOD (GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
	STDMETHOD (ShowContextMenu)(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);
	STDMETHOD (ShowMessage)(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult);
	STDMETHOD (TranslateAccelerator)(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID);

	virtual void OnBeforeNavigate(IDispatch *pDispatch, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel);
	virtual void OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
	virtual void OnNavigateError(IDispatch *pDispatch, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel);
	virtual void OnDocumentReady(IDispatch *pDispatch, VARIANT *URL);

	virtual COLORREF OnGetHostBkColor(void);
	virtual DWORD OnGetHostInfoFlags(void);
	virtual OLECHAR* OnGetHostCSS(void);
	virtual DWORD OnGetDownlodFlags(void);
	virtual LPCWSTR OnGetUserAgent(void);
	virtual DWORD GetContainerStyle(void);

protected:
	IDispatch	*pDispWA;
	BSTR		bstrMessage;
	BSTR		bstrFileName;
	INT			nHomePage;
	INT			nDragMode;
private:
	friend WebFileInfo *CreateWebFileInfo(HWND hwndParent, IDispatch *pDispWA);
};

#endif //NULLSOFT_MLDISC_MINIINFO_HEADER