#include "main.h"
#include "./deviceCommand.h"

#include <strsafe.h>

DeviceCommand::DeviceCommand() 
	: ref(1), name(NULL), displayName(NULL), description(NULL)
{
	if (FAILED(DeviceIconStore::CreateInstance(&iconStore)))
		iconStore = NULL;

	InitializeCriticalSection(&lock);
}

DeviceCommand::~DeviceCommand()
{
	AnsiString_Free(name);
	String_Free(displayName);
	String_Free(description);

	if (NULL != iconStore)
		iconStore->Release();

	DeleteCriticalSection(&lock);
}

HRESULT DeviceCommand::CreateInstance(const char *name, DeviceCommand **instance)
{
	char *nameCopy;

	if (NULL == instance) 
		return E_POINTER;

	if (FALSE != IS_STRING_EMPTY(name))
		return E_INVALIDARG;

	*instance = new DeviceCommand();

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

size_t DeviceCommand::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceCommand::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceCommand::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceCommand))
		*object = static_cast<ifc_devicecommand*>(this);
	else if (IsEqualIID(interface_guid, IFC_DeviceCommandEditor))
		*object = static_cast<ifc_devicecommandeditor*>(this);
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

void DeviceCommand::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceCommand::Unlock()
{
	LeaveCriticalSection(&lock);
}

const char *DeviceCommand::GetName()
{
	return name;
}

HRESULT DeviceCommand::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
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

HRESULT DeviceCommand::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;
	
	Lock();

	hr = StringCchCopyExW(buffer, bufferSize, displayName, NULL, NULL, STRSAFE_IGNORE_NULLS);

	Unlock();

	return hr;
}

HRESULT DeviceCommand::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;
	
	Lock();

	hr = StringCchCopyExW(buffer, bufferSize, description, NULL, NULL, STRSAFE_IGNORE_NULLS);

	Unlock();

	return hr;
}

HRESULT DeviceCommand::GetIconStore(ifc_deviceiconstore **store)
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

HRESULT DeviceCommand::SetDisplayName(const wchar_t *displayName)
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

HRESULT DeviceCommand::SetDescription(const wchar_t *description)
{
	HRESULT hr;

	Lock();

	String_Free(this->description);
	this->description = String_Duplicate(description);
	
	if (NULL == this->description && NULL != description)
		hr = E_OUTOFMEMORY;
	else 
		hr = S_OK;

	Unlock();

	return hr;
}

#define CBCLASS DeviceCommand
START_MULTIPATCH;
	START_PATCH(MPIID_DEVICECOMMAND)
		M_CB(MPIID_DEVICECOMMAND, ifc_devicecommand, ADDREF, AddRef);
		M_CB(MPIID_DEVICECOMMAND, ifc_devicecommand, RELEASE, Release);
		M_CB(MPIID_DEVICECOMMAND, ifc_devicecommand, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_DEVICECOMMAND, ifc_devicecommand, API_GETNAME, GetName);
		M_CB(MPIID_DEVICECOMMAND, ifc_devicecommand, API_GETICON, GetIcon);
		M_CB(MPIID_DEVICECOMMAND, ifc_devicecommand, API_GETDISPLAYNAME, GetDisplayName);
		M_CB(MPIID_DEVICECOMMAND, ifc_devicecommand, API_GETDESCRIPTION, GetDescription);
	NEXT_PATCH(MPIID_DEVICECOMMANDEDITOR)
		M_CB(MPIID_DEVICECOMMANDEDITOR, ifc_devicecommandeditor, ADDREF, AddRef);
		M_CB(MPIID_DEVICECOMMANDEDITOR, ifc_devicecommandeditor, RELEASE, Release);
		M_CB(MPIID_DEVICECOMMANDEDITOR, ifc_devicecommandeditor, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_DEVICECOMMANDEDITOR, ifc_devicecommandeditor, API_SETDISPLAYNAME, SetDisplayName);
		M_CB(MPIID_DEVICECOMMANDEDITOR, ifc_devicecommandeditor, API_SETDESCRIPTION, SetDescription);
		M_CB(MPIID_DEVICECOMMANDEDITOR, ifc_devicecommandeditor, API_GETICONSTORE, GetIconStore);
	END_PATCH
END_MULTIPATCH;
