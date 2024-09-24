#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPPAGE_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPPAGE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <bfc/multipatch.h>

#include "../../winamp/setup/ifc_setuppage.h"
#include "../../winamp/setup/ifc_setupjob.h"

#include "./setupGroupList.h"

class SetupListboxLabel;

#define ID_KNOWNGROUP			0
#define ID_FEATUREDGROUP		1

#define MPIID_SETUPPAGE		10
#define MPIID_SETUPJOB		20

#define SPM_FIRST			(WM_APP + 2)
#define SPM_UPDATELIST		(SPM_FIRST + 0)

class SetupPage :	public MultiPatch<MPIID_SETUPPAGE, ifc_setuppage>,
					public MultiPatch<MPIID_SETUPJOB, ifc_setupjob>
{
protected:
	typedef enum
	{
		flagInitWasabi = 0x00000001,
	};

protected:
	SetupPage();
	virtual ~SetupPage();

public:
	static SetupPage* CreateInstance();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_setuppage */
	HRESULT GetName(bool bShort, const wchar_t **pszName);
	HRESULT Save(HWND hText);
	HRESULT CreateView(HWND hParent, HWND *phwnd);
	HRESULT Revert(void);
	HRESULT IsDirty(void);
	HRESULT Validate(void);

	/* ifc_setupjob */
	HRESULT Execute(HWND hwndText); 
	HRESULT Cancel(HWND hwndText);
	HRESULT IsCancelSupported(void);

public:
	BOOL AttachWindow(HWND hAttach);
	void DetachWindow();

	void ListboxSelectionChanged();
	BOOL UpdateListAsync(INT groupId);

protected:
	HRESULT InitializeServices();

private:
	size_t ref;
	HWND hwnd;
	LPWSTR name;
	LPWSTR title;
	SetupGroupList *groupList;
	HANDLE completeEvent;
	BOOL servicesInitialized;
	
protected:
	RECVS_MULTIPATCH;
};

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPPAGE_HEADER