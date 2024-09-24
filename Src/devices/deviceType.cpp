#include "main.h"
#include "./deviceType.h"

#include <strsafe.h>

DeviceType::DeviceType() 
	: ref(1), name(NULL), displayName(NULL)
{
	if (FAILED(DeviceIconStore::CreateInstance(&iconStore)))
		iconStore = NULL;

	InitializeCriticalSection(&lock);
}

DeviceType::~DeviceType()
{
	AnsiString_Free(name);
	String_Free(displayName);
	if (NULL != iconStore)
		iconStore->Release();

	DeleteCriticalSection(&lock);
}

HRESULT DeviceType::CreateInstance(const char *name, DeviceType **instance)
{
	char *nameCopy;

	if (NULL == instance) 
		return E_POINTER;

	if (FALSE != IS_STRING_EMPTY(name))
		return E_INVALIDARG;

	*instance = new DeviceType();

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	nameCopy = AnsiString_Duplicate(name);
	if (NULL == nameCopy)
	{
		(*instance)->Release();
		return E_OUTOFMEMORY;
	}

	(*instance)->name = nameCopy;
	
	return S_OK;
}

size_t DeviceType::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceType::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceType::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceType))
		*object = static_cast<ifc_devicetype*>(this);
	else if (IsEqualIID(interface_guid, IFC_DeviceTypeEditor))
		*object = static_cast<ifc_devicetypeeditor*>(this);
	else if (IsEqualIID(interface_guid, IFC_DeviceObject))
		*object = static_cast<ifc_deviceobject*>(this);
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

void DeviceType::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceType::Unlock()
{
	LeaveCriticalSection(&lock);
}

const char *DeviceType::GetName()
{
	return name;
}

HRESULT DeviceType::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	HRESULT hr;
	
	if (NULL == buffer)
		return E_POINTER;
	
	Lock();

	if (NULL == iconStore)
		hr = E_UNEXPECTED;
	else
		hr = iconStore->Get(buffer, bufferSize, width, height);

	Unlock();

	return hr;
}

HRESULT DeviceType::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;
	
	Lock();

	hr = StringCchCopyExW(buffer, bufferSize, displayName, NULL, NULL, STRSAFE_IGNORE_NULLS);

	Unlock();

	return hr;
}

HRESULT DeviceType::GetIconStore(ifc_deviceiconstore **store)
{
	HRESULT hr;

	if (NULL == store)
		return E_POINTER;

	Lock();

	if (NULL == iconStore)
		hr = E_UNEXPECTED;
	else
	{
		iconStore->AddRef();
		*store = iconStore;
		hr = S_OK;
	}

	Unlock();

	return hr;
}

HRESULT DeviceType::SetDisplayName(const wchar_t *displayName)
{
	HRESULT hr;

	Lock();

	String_Free(this->displayName);
	this->displayName = String_Duplicate(displayName);
	
	if (NULL == this->displayName && NULL != displayName)
		hr = E_OUTOFMEMORY;
	else 
		hr = S_OK;

	Unlock();

	return hr;
}

#define CBCLASS DeviceType
START_MULTIPATCH;
	START_PATCH(MPIID_DEVICETYPE)
		M_CB(MPIID_DEVICETYPE, ifc_devicetype, ADDREF, AddRef);
		M_CB(MPIID_DEVICETYPE, ifc_devicetype, RELEASE, Release);
		M_CB(MPIID_DEVICETYPE, ifc_devicetype, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_DEVICETYPE, ifc_devicetype, API_GETNAME, GetName);
		M_CB(MPIID_DEVICETYPE, ifc_devicetype, API_GETICON, GetIcon);
		M_CB(MPIID_DEVICETYPE, ifc_devicetype, API_GETDISPLAYNAME, GetDisplayName);
	NEXT_PATCH(MPIID_DEVICETYPEEDITOR)
		M_CB(MPIID_DEVICETYPEEDITOR, ifc_devicetypeeditor, ADDREF, AddRef);
		M_CB(MPIID_DEVICETYPEEDITOR, ifc_devicetypeeditor, RELEASE, Release);
		M_CB(MPIID_DEVICETYPEEDITOR, ifc_devicetypeeditor, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_DEVICETYPEEDITOR, ifc_devicetypeeditor, API_SETDISPLAYNAME, SetDisplayName);
		M_CB(MPIID_DEVICETYPEEDITOR, ifc_devicetypeeditor, API_GETICONSTORE, GetIconStore);
	END_PATCH
END_MULTIPATCH;