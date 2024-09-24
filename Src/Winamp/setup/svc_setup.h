#ifndef NULLSOFT_WINAMP_SETUP_SERVICE_HEADER
#define NULLSOFT_WINAMP_SETUP_SERVICE_HEADER

#include <bfc/dispatch.h>
#include "./ifc_setuppage.h"
#include "./ifc_setupjob.h"

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef __cplusplus
}
#endif // __cplusplus

class api_service;
typedef BOOL (__cdecl *Plugin_RegisterSetup)(HINSTANCE /*hInstance*/, api_service* /*serviceApi*/);

class __declspec(novtable) svc_setup : public Dispatchable
{
protected:
	svc_setup(void) {};
	~svc_setup(void) {};
public:
	HRESULT InsertPage(ifc_setuppage *pPage, int* pIndex);
	HRESULT RemovePage(int index);
	HRESULT GetPageCount(int* pCount);
	HRESULT GetPage(int index, ifc_setuppage **pPage);
	HRESULT GetActiveIndex(int*pIndex);
	HRESULT Start(HWND hwndWinamp);
	HRESULT AddJob(ifc_setupjob *pJob);
	HRESULT RemoveJob(ifc_setupjob *pJob);
	HRESULT CreateStatusWnd(HWND *phwndStatus);
	HRESULT Save(HWND hwndStatus);
	HRESULT ExecJobs(HWND hwndStatus);
	HRESULT GetWinampWnd(HWND *phwndWinamp); // this call will return S_OK only after setup->Start()

public:
	DISPATCH_CODES
	{
		API_SETUP_INSERT_PAGE = 10,
		API_SETUP_REMOVE_PAGE = 20,
		API_SETUP_GET_PAGE_COUNT= 30,
		API_SETUP_GET_PAGE= 40,
		API_SETUP_GET_ACTIVE_INDEX = 50,
		API_SETUP_START = 60,
		API_SETUP_ADD_JOB = 70,
		API_SETUP_REMOVE_JOB = 80,
		API_SETUP_CREATE_STATUSWND = 90,
		API_SETUP_SAVE = 100,
		API_SETUP_EXECJOBS = 110,
		API_SETUP_GETWINAMPWND = 120,
		
	};
};

inline HRESULT svc_setup::InsertPage(ifc_setuppage *pPage, int* pIndex)
{
	return _call(API_SETUP_INSERT_PAGE, E_NOTIMPL, pPage, pIndex);
}

inline HRESULT svc_setup::RemovePage(int index)
{
	return _call(API_SETUP_REMOVE_PAGE, E_NOTIMPL, index);
}

inline HRESULT svc_setup::GetPageCount(int* pCount)
{
	return _call(API_SETUP_GET_PAGE_COUNT, E_NOTIMPL, pCount);
}

inline HRESULT svc_setup::GetPage(int index, ifc_setuppage **pPage)
{
	return _call(API_SETUP_GET_PAGE, E_NOTIMPL, index, pPage);
}
inline HRESULT svc_setup::GetActiveIndex(int*pIndex)
{
	return _call(API_SETUP_GET_ACTIVE_INDEX, E_NOTIMPL, pIndex);
}
inline HRESULT svc_setup::Start(HWND hwndWinamp)
{
	return _call(API_SETUP_START, E_NOTIMPL, hwndWinamp);
}

inline HRESULT svc_setup::AddJob(ifc_setupjob *pJob)
{
	return _call(API_SETUP_ADD_JOB, E_NOTIMPL, pJob);
}

inline HRESULT svc_setup::RemoveJob(ifc_setupjob *pJob)
{
	return _call(API_SETUP_REMOVE_JOB, E_NOTIMPL, pJob);
}

inline HRESULT svc_setup::CreateStatusWnd(HWND *phwndStatus)
{
	return _call(API_SETUP_CREATE_STATUSWND, E_NOTIMPL, phwndStatus);
}

inline HRESULT svc_setup::Save(HWND hwndStatus)
{
	return _call(API_SETUP_SAVE, E_NOTIMPL, hwndStatus);
}

inline HRESULT svc_setup::ExecJobs(HWND hwndStatus)
{
	return _call(API_SETUP_EXECJOBS, E_NOTIMPL, hwndStatus);
}

inline HRESULT svc_setup::GetWinampWnd(HWND *phwndWinamp)
{
	return _call(API_SETUP_GETWINAMPWND, E_NOTIMPL, phwndWinamp);
}



// {A7281DEB-9DA8-4ee9-93A8-2E0B8F5F1805}
static const GUID UID_SVC_SETUP = { 0xa7281deb, 0x9da8, 0x4ee9, { 0x93, 0xa8, 0x2e, 0xb, 0x8f, 0x5f, 0x18, 0x5 } };


#endif //NULLSOFT_WINAMP_SETUP_SERVICE_HEADER
