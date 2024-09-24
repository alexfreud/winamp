#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUP_SERVICE_PANEL_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUP_SERVICE_PANEL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <bfc/multipatch.h>
#include <ifc_omserviceevent.h>
#include <ifc_omcachecallback.h>

class ifc_omservice;

#define MPIID_SERVICEEVENT		10
#define MPIID_CACHECALLBACK		20


class ServicePanel : public MultiPatch<MPIID_SERVICEEVENT, ifc_omserviceevent>,
					public MultiPatch<MPIID_CACHECALLBACK, ifc_omcachecallback>
{
protected:	
	ServicePanel(LPCWSTR pszName, ifc_omservice *service);
	~ServicePanel();

public:
	static HWND CreateInstance(HWND hParent, LPCWSTR pszName, ifc_omservice *service, ServicePanel **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omserviceevent */
	void ServiceChange(ifc_omservice *service, unsigned int modifiedFlags);

	/* ifc_omcachecallback */
	void PathChanged(ifc_omcacherecord *record);

protected:
	void Attach(HWND hwnd);
	void Detach();
	
	void UpdateName();
	void UpdateDescription();
	void UpdateMeta();
	void UpdateThumbnail();

	HFONT PickTitleFont(LPCWSTR pszTitle, INT cchTitle, INT maxWidth);
	LPCWSTR FormatDate(LPCWSTR pszDate, LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT GetFullName(LPWSTR pszBuffer, UINT cchBufferMax);
	
	INT_PTR OnInitDialog(HWND hFocus, LPARAM lParam);
	void OnDestroy();
	INT_PTR OnDialogColor(HDC hdc, HWND hControl);
	INT_PTR OnStaticColor(HDC hdc, HWND hControl);
	INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnGetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT LoadLocalThumbnail(LPCWSTR pszPath);

private:
	friend static INT_PTR WINAPI ServicePanel_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	size_t ref;
	HWND hwnd;
	LPWSTR name;
	ifc_omservice *service;
	ifc_omcacherecord *thumbnailCache;
	HFONT fontTitle;
	HFONT fontMeta;

private:
	RECVS_MULTIPATCH;
};


#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUP_SERVICE_PANEL_HEADER