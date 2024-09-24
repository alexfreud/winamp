#ifndef NULLSOFT_WINAMP_OMSERVICE_IMPLEMENTATION_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_IMPLEMENTATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omservice.h"
#include "./ifc_omservicedetails.h"
#include "./ifc_omserviceeditor.h"
#include "./ifc_omservicecopier.h"
#include "./ifc_omservicecommand.h"
#include "./ifc_omservicehostext.h"
#include "./ifc_omserviceeventmngr.h"
#include "./flagTracker.h"
#include <vector>
#include <bfc/multipatch.h>

#define MPIID_OMSVC				10
#define MPIID_OMSVCDETAILS		20
#define MPIID_OMSVCEDITOR		30
#define MPIID_OMSVCCOPIER		40
#define MPIID_OMSVCCOMMAND		50
#define MPIID_OMSVCEVENTMNGR	60
#define MPIID_OMSVCHOSTEXT		70

class OmService :	public MultiPatch<MPIID_OMSVC, ifc_omservice>,
					public MultiPatch<MPIID_OMSVCDETAILS, ifc_omservicedetails>,
					public MultiPatch<MPIID_OMSVCEDITOR, ifc_omserviceeditor>,
					public MultiPatch<MPIID_OMSVCCOPIER, ifc_omservicecopier>,
					public MultiPatch<MPIID_OMSVCCOMMAND, ifc_omservicecommand>,
					public MultiPatch<MPIID_OMSVCEVENTMNGR, ifc_omserviceeventmngr>,
					public MultiPatch<MPIID_OMSVCHOSTEXT, ifc_omservicehostext>
{
protected:
	OmService(UINT serviceId, ifc_omservicehost *serviceHost);
	~OmService();

public:
	static HRESULT CreateInstance(UINT serviceId, ifc_omservicehost *serviceHost, OmService **serviceOut);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omservice */
	unsigned int GetId();
	HRESULT GetName(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetUrl(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetUrlDirect(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetIcon(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetExternal(IDispatch **ppDispatch);
	HRESULT GetRating(UINT *rating);
	HRESULT GetVersion(UINT *version);
	HRESULT GetFlags(UINT *flags);
	HRESULT SetAddress(LPCWSTR pszAddress);
	HRESULT GetAddress(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetGeneration(UINT *generation);
	HRESULT UpdateFlags(UINT flags);

	/* ifc_omservicedetails */
	HRESULT GetDescription(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetAuthorFirst(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetAuthorLast(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetUpdated(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetPublished(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetThumbnail(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetScreenshot(LPWSTR pszBuffer, UINT cchBufferMax);

	/* ifc_omserviceeditor */
	HRESULT SetName(LPCWSTR pszName, BOOL utf8);
	HRESULT SetUrl(LPCWSTR pszUrl, BOOL utf8);
	HRESULT SetIcon(LPCWSTR pszPath, BOOL utf8);
	HRESULT SetRating(UINT nRating);
	HRESULT SetVersion(UINT nVersion);
	HRESULT SetGeneration(UINT nGeneration);
	HRESULT SetFlags(UINT nFlags, UINT nMask);
	HRESULT SetDescription(LPCWSTR pszDescription, BOOL utf8);
	HRESULT SetAuthorFirst(LPCWSTR pszName, BOOL utf8);
	HRESULT SetAuthorLast(LPCWSTR pszName, BOOL utf8);
	HRESULT SetUpdated(LPCWSTR pszDate, BOOL utf8);
	HRESULT SetPublished(LPCWSTR pszDate, BOOL utf8);
	HRESULT SetThumbnail(LPCWSTR pszPath, BOOL utf8);
	HRESULT SetScreenshot(LPCWSTR pszPath, BOOL utf8);
	HRESULT SetModified(UINT nFlags, UINT nMask);
	HRESULT GetModified(UINT *pFlags);
	HRESULT BeginUpdate();
	HRESULT EndUpdate();

	/* ifc_omservicecopier */
	HRESULT CopyTo(ifc_omservice *service, UINT *modifiedFlags);

	/* ifc_omservicecommand */
	HRESULT QueryState(HWND hBrowser, const GUID *commandGroup, UINT commandId);
	HRESULT Exec(HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg);

	/* ifc_omserviceeventmngr */
	HRESULT RegisterEventHandler(ifc_omserviceevent *handler);
	HRESULT UnregisterEventHandler(ifc_omserviceevent *handler);
	HRESULT Signal_ServiceChange(UINT modifiedFlags);
	HRESULT Signal_CommandStateChange(const GUID *commandGroup, UINT commandId);

	/* ifc_omservicehostext */
	HRESULT GetHost(ifc_omservicehost **ppHost);
	HRESULT SetHost(ifc_omservicehost *serviceHost);

public:
	HRESULT SetId(UINT serviceId);

private:
	static void CALLBACK ModifiedEvent(UINT nMark, FlagTracker *instance, ULONG_PTR user);
	void UnregisterAllEventHandlers();
	HRESULT StringCchCopyExMT(LPTSTR pszDest, size_t cchDest, LPCTSTR pszSrc, LPTSTR *ppszDestEnd, size_t *pcchRemaining, DWORD dwFlags);

protected:
	typedef std::vector<ifc_omserviceevent*> EventList;

protected:
	ULONG	ref;
	UINT	id;
	LPWSTR	name;
	LPWSTR	url;
	LPWSTR	icon;
	UINT	rating;
	UINT	version;
	UINT	flags;
	UINT	generation;

	LPWSTR	description;
	LPWSTR	authorFirst;
	LPWSTR	authorLast;
	LPWSTR	published;
	LPWSTR	updated;
	LPWSTR	thumbnail;
	LPWSTR	screenshot;
	FlagTracker modified;

	LPWSTR address;
	ifc_omservicehost *host;

	CRITICAL_SECTION lock;

	EventList eventList;
	CRITICAL_SECTION eventLock;

protected:
	RECVS_MULTIPATCH;
};

#endif //NULLSOFT_WINAMP_OMSERVICE_IMPLEMENTATION_HEADER