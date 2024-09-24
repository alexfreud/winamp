#define GUID_DEFINE
#include "../../winamp/setup/svc_setup.h"
#undef GUID_DEFINE

#include "./setupPage.h"

#include "../api__ml_online.h"


static HRESULT Setup_RegisterPage()
{	
	HRESULT hr;
	svc_setup *setupSvc;
	SetupPage *page;


	if (FAILED(WasabiApi_LoadDefaults()) ||
		NULL == OMBROWSERMNGR ||
		NULL == OMSERVICEMNGR ||
		NULL == OMUTILITY) 
	{
		return E_UNEXPECTED;
	}
	
	setupSvc = QueryWasabiInterface(svc_setup, UID_SVC_SETUP);
	if (NULL == setupSvc) 
		return E_POINTER;

	page = SetupPage::CreateInstance();
	if (NULL == page) 
		hr = E_OUTOFMEMORY;
	else
	{
		// try to insert before 'feedback' (if present)
		// otherwise dump at the end of the pages list.
		int index = 0xFFFFF;
		if (FAILED(setupSvc->GetPageCount(&index)))
			index = 0xFFFFF;
		else if (index > 0 && index == 3) 
			index--;

		hr = setupSvc->InsertPage(page, &index);
		if (SUCCEEDED(hr))
			setupSvc->AddJob((ifc_setupjob*)page);
		
		page->Release();
	}
	
	ReleaseWasabiInterface(UID_SVC_SETUP, setupSvc);
	
	return hr;
}

EXTERN_C _declspec(dllexport) BOOL RegisterSetup(HINSTANCE hInstance, api_service *waServices)
{
	// check the current date and if past November 30th 2013
	// then we will prevent the online page from being shown
	time_t now = time(0);
	struct tm *tn = localtime(&now);
	tn->tm_sec = tn->tm_min = tn->tm_hour = 0;

	if (mktime(tn) >= 1387497600)
		return FALSE;

	if (FAILED(WasabiApi_Initialize(hInstance, waServices)))
		return FALSE;

	BOOL result = SUCCEEDED(Setup_RegisterPage());
	WasabiApi_Release();
	return result;
}