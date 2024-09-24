#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_NAVIGATION_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_NAVIGATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <ifc_mlnavigationcallback.h>
#include "./forceUrl.h"
#include <vector>

class ifc_omservice;
class forceUrl;
typedef LPVOID HNAVITEM;

#define ROOTSERVICE_ID		777

typedef void (CALLBACK *NavigationCallback)(Navigation* /*instance*/, ULONG_PTR /*param*/);

class Navigation : public ifc_mlnavigationcallback
{
protected:
	Navigation();
	~Navigation();

public:
	static HRESULT CreateInstance(Navigation **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_mlnavigationcallback */
	void ImageChanged(LPCWSTR pszName, INT index);

public:
	HRESULT Finish();

	BOOL ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result);
	
	HNAVITEM GetActive(ifc_omservice **serviceOut);
	HWND GetActiveView(ifc_omservice **serviceOut);

	HRESULT SelectItem(HNAVITEM hItem, LPCWSTR pszUrl);
	HRESULT DeleteItem(HNAVITEM hItem);
	HRESULT DeleteAll();

	HRESULT GenerateServiceName(LPWSTR pszBuffer, INT cchBufferMax);

	HRESULT CreatePopup(HNAVITEM hItem, HWND *hwnd);
		
	HRESULT GetService(HNAVITEM hItem, ifc_omservice **service);
	HRESULT UpdateService(ifc_omservice *service, UINT modifiedFlags);
	HNAVITEM FindService(UINT serviceId, ifc_omservice **serviceOut);
	HRESULT ShowService(UINT serviceId, LPCWSTR pszUrl);

	HNAVITEM CreateItem(ifc_omservice *service, int altMode = 0);
	HRESULT CreateItemAsync(ifc_omservice *service);
	HRESULT CreateUserService(HNAVITEM *itemOut);

protected:
	HRESULT Initialize();
	HRESULT InitializeBrowser();
	HRESULT SaveOrder();
	HRESULT Order(std::vector<ifc_omservice *> &list);
		
	HNAVITEM GetMessageItem(INT msg, INT_PTR param1);
	HNAVITEM CreateItemInt(HNAVITEM hParent, ifc_omservice *service, int altMode = 0);
	HRESULT SelectItemInt(HNAVITEM hItem, UINT serviceId, LPCWSTR pszUrl);

	HRESULT PatchIconName(LPWSTR pszIcon, UINT cchMaxIconLen, ifc_omservice *service);

	HRESULT OnCreateView(HNAVITEM hItem, HWND hParent, HWND *hView);
	HRESULT OnContextMenu(HNAVITEM hItem, HWND hHost, POINTS pts);
	HRESULT OnEndTitleEdit(HNAVITEM hItem, LPCWSTR pszNewTitle);
	HRESULT OnDeleteItem(HNAVITEM hItem);
	HRESULT OnKeyDown(HNAVITEM hItem, NMTVKEYDOWN *pnmkd);
	HRESULT OnControlDestroy();

	HRESULT PostMainThreadCallback(NavigationCallback callback, ULONG_PTR param);

protected:
	size_t ref;
	HNAVITEM hRoot;
	HWND hLibrary;
	UINT cookie;
	ForceUrl forceUrl;

protected:
	RECVS_DISPATCH;
};

#endif // NULLOSFT_ONLINEMEDIA_PLUGIN_NAVIGATION_HEADER