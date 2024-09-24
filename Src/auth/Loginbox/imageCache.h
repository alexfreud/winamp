#ifndef NULLSOFT_AUTH_LOGINBOX_IMAGECACHE_HEADER
#define NULLSOFT_AUTH_LOGINBOX_IMAGECACHE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../ombrowser/ifc_omcachecallback.h"

#include <commctrl.h>

class ifc_omcachegroup;
class ifc_omcacherecord;

class LoginImageCache : public ifc_omcachecallback
{
protected:
	LoginImageCache(HWND hLoginbox);
	~LoginImageCache();

public:
	static HRESULT CreateInstance(HWND hLoginbox, LoginImageCache **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omcachecallback */
	void PathChanged(ifc_omcacherecord *record);

	void Finish();

	HRESULT GetImageListIndex(LPCWSTR pszPath, HIMAGELIST himl, UINT *index, UINT *indexActive, UINT *indexDisabled);
	HRESULT GetImageListIndexLocal(LPCWSTR pszPath, HIMAGELIST himl, UINT *index, UINT *indexActive, UINT *indexDisabled);

private:
	HRESULT InitGroup();
	HBITMAP AdjustBitmapSize(HBITMAP hBitmap, INT forceWidth, INT forceHeight);
protected:
	size_t ref;
	HWND hwnd;
	ifc_omcachegroup *group;
	CRITICAL_SECTION lock;

private:
	RECVS_DISPATCH;
};


#endif // NULLSOFT_AUTH_LOGINBOX_IMAGECACHE_HEADER