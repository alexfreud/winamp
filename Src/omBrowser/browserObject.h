#ifndef NULLSOFT_WINAMP_OMBROWSER_OBJECT_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_OBJECT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./obj_ombrowser.h"
#include "./ifc_ombrowserwndmngr.h"
#include "./ifc_ombrowsereventmngr.h"
#include <bfc/multipatch.h>
#include <vector>

#define MPIID_OMBROWSER				10
#define MPIID_OMBROWSERWNDMNGR		20
#define MPIID_OMBROWSEREVENTMNGR	30

class OmBrowserWndRecord;

class OmBrowserObject : public MultiPatch<MPIID_OMBROWSER, obj_ombrowser>,
						public MultiPatch<MPIID_OMBROWSERWNDMNGR, ifc_ombrowserwndmngr>,
						public MultiPatch<MPIID_OMBROWSEREVENTMNGR, ifc_ombrowsereventmngr>
{
public:
	typedef enum
	{
		flagFinishing = 0x0001,
	} Falgs;
protected:
	OmBrowserObject();
	~OmBrowserObject();

public:
	static HRESULT CreateInstance(OmBrowserObject **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* obj_ombrowser */
	HRESULT Initialize(LPCWSTR pszName, HWND hwndWinamp);
	HRESULT Finish(void);
	HRESULT RegisterWinampHook(ifc_winamphook *hook, UINT *cookieOut);
	HRESULT UnregisterWinampHook(UINT cookie);
	HRESULT GetConfig(const GUID *configIfc, void **configOut);
	HRESULT GetSessionId(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT GetClientId(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT GetRegistry(ifc_ombrowserregistry **registryOut);
	HRESULT CreateView(ifc_omservice *service, HWND hParent, LPCWSTR forceUrl, UINT viewStyle, HWND *hView);
	HRESULT CreatePopup(ifc_omservice *service, INT x, INT y, INT cx, INT cy, HWND hOwner, LPCWSTR forceUrl, UINT viewStyle, HWND *hWindow);
	HRESULT ShowOptions(HWND hOwner, UINT style, BROWSEROPTIONSCALLBACK callback, ULONG_PTR user);
	HRESULT IsFinishing(void);
	HRESULT GetClass(ifc_ombrowserclass **instance);
	HRESULT GetVersion(int *major, int *minor);
	HRESULT GetIEVersion(int *major, int *minor, int *build, int *subbuild);

	/* ifc_ombrowserwndmngr */
	HRESULT RegisterWindow(HWND hwnd, const GUID *windowType);
	HRESULT UnregisterWindow(HWND hwnd);
	HRESULT Enumerate(const GUID *windowType, UINT *serviceIdFilter, ifc_ombrowserwndenum **enumerator);

	/* ifc_ombrowsereventmngr */
	HRESULT RegisterEventHandler(ifc_ombrowserevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_ombrowserevent *eventHandler);
	HRESULT Signal_WindowCreate(HWND hwnd, const GUID *windowType);
	HRESULT Signal_WindowClose(HWND hwnd, const GUID *windowType);

protected:
	LPCWSTR GetConfigFileInt(void);

protected:
	typedef struct __WindowRecord
	{
		HWND hwnd;
		GUID type;
	} WindowRecord;

	typedef std::vector<OmBrowserWndRecord*> WindowList;
	typedef std::vector<ifc_ombrowserevent*> EventList;

protected:
	ULONG ref;
	UINT flags;
	WindowList windowList;
	ifc_ombrowserclass *browserClass;
	EventList eventList;
	CRITICAL_SECTION lock;

protected:
	RECVS_MULTIPATCH;
};

#endif //NULLSOFT_WINAMP_OMBROWSER_OBJECT_HEADER