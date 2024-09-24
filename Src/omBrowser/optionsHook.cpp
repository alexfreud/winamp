#include "main.h"
#include "./optionsHook.h"
#include "./options.h"

#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"

#include <strsafe.h>

OptionsConfigHook::OptionsConfigHook(HWND hTarget) 
	: ref(1), hwnd(hTarget)
{
}

OptionsConfigHook::~OptionsConfigHook() 
{
}

HRESULT OptionsConfigHook::CreateInstance(HWND hTarget, OptionsConfigHook **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	if (NULL == hTarget || FALSE == IsWindow(hTarget))
		return E_INVALIDARG;

	*instance = new OptionsConfigHook(hTarget);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t OptionsConfigHook::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OptionsConfigHook::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OptionsConfigHook::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	else if (IsEqualIID(interface_guid, IFC_OmConfigCallback))
		*object = static_cast<ifc_omconfigcallback*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT OptionsConfigHook::ValueChanged(const GUID *configUid, UINT valueId, ULONG_PTR value)
{
	if (NULL == configUid) 
		return E_UNEXPECTED;

	BOMCONFIGCHANGED configData;
	configData.configUid = configUid;
	configData.valueId = valueId;
	configData.value = value;
	
	DWORD_PTR result;
	SendMessageTimeout(hwnd, BOM_CONFIGCHANGED, 0, (LPARAM)&configData, SMTO_ABORTIFHUNG | SMTO_NORMAL, 2000, &result);
	return S_OK;
}

#define CBCLASS OptionsConfigHook
START_DISPATCH;
  CB(ADDREF, AddRef);
  CB(RELEASE, Release);
  CB(QUERYINTERFACE, QueryInterface);
  CB(API_VALUECHANGED, ValueChanged);
END_DISPATCH;
#undef CBCLASS


