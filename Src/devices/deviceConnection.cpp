#include "main.h"
#include "./deviceConnection.h"

#include <strsafe.h>

DeviceConnection::DeviceConnection() 
	: ref(1), name(NULL), displayName(NULL)
{
	if (FAILED(DeviceIconStore::CreateInstance(&iconStore)))
		iconStore = NULL;

	InitializeCriticalSection(&lock);
}

DeviceConnection::~DeviceConnection()
{
	AnsiString_Free(name);
	String_Free(displayName);
	if (NULL != iconStore)
		iconStore->Release();

	DeleteCriticalSection(&lock);
}

HRESULT DeviceConnection::CreateInstance(const char *name, DeviceConnection **instance)
{
	char *nameCopy;

	if (NULL == instance) 
		return E_POINTER;

	if (FALSE != IS_STRING_EMPTY(name))
		return E_INVALIDARG;

	*instance = new DeviceConnection();

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

size_t DeviceConnection::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceConnection::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceConnection::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceConnection))
		*object = static_cast<ifc_deviceconnection*>(this);
	else if (IsEqualIID(interface_guid, IFC_DeviceConnectionEditor))
		*object = static_cast<ifc_deviceconnectioneditor*>(this);
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

void DeviceConnection::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceConnection::Unlock()
{
	LeaveCriticalSection(&lock);
}

const char *DeviceConnection::GetName()
{
	return name;
}

HRESULT DeviceConnection::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
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

HRESULT DeviceConnection::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;
	
	Lock();

	hr = StringCchCopyExW(buffer, bufferSize, displayName, NULL, NULL, STRSAFE_IGNORE_NULLS);

	Unlock();

	return hr;
}

HRESULT DeviceConnection::GetIconStore(ifc_deviceiconstore **store)
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

HRESULT DeviceConnection::SetDisplayName(const wchar_t *displayName)
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

#define CBCLASS DeviceConnection
START_MULTIPATCH;
	START_PATCH(MPIID_DEVICECONNECTION)
		M_CB(MPIID_DEVICECONNECTION, ifc_deviceconnection, ADDREF, AddRef);
		M_CB(MPIID_DEVICECONNECTION, ifc_deviceconnection, RELEASE, Release);
		M_CB(MPIID_DEVICECONNECTION, ifc_deviceconnection, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_DEVICECONNECTION, ifc_deviceconnection, API_GETNAME, GetName);
		M_CB(MPIID_DEVICECONNECTION, ifc_deviceconnection, API_GETICON, GetIcon);
		M_CB(MPIID_DEVICECONNECTION, ifc_deviceconnection, API_GETDISPLAYNAME, GetDisplayName);
	NEXT_PATCH(MPIID_DEVICECONNECTIONEDITOR)
		M_CB(MPIID_DEVICECONNECTIONEDITOR, ifc_deviceconnectioneditor, ADDREF, AddRef);
		M_CB(MPIID_DEVICECONNECTIONEDITOR, ifc_deviceconnectioneditor, RELEASE, Release);
		M_CB(MPIID_DEVICECONNECTIONEDITOR, ifc_deviceconnectioneditor, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_DEVICECONNECTIONEDITOR, ifc_deviceconnectioneditor, API_SETDISPLAYNAME, SetDisplayName);
		M_CB(MPIID_DEVICECONNECTIONEDITOR, ifc_deviceconnectioneditor, API_GETICONSTORE, GetIconStore);
	END_PATCH
END_MULTIPATCH;