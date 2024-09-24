#include "main.h"
#include "./supportedCommand.h"

#include <strsafe.h>

DeviceSupportedCommand::DeviceSupportedCommand() 
	: ref(1), name(NULL), flags(DeviceCommandFlag_None)
{
}

DeviceSupportedCommand::~DeviceSupportedCommand()
{
	AnsiString_Free(name);
}

HRESULT DeviceSupportedCommand::CreateInstance(const char *name, DeviceSupportedCommand **instance)
{
	DeviceSupportedCommand *self;

	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;
	
	self = new DeviceSupportedCommand();
	if (NULL == self) 
		return E_OUTOFMEMORY;

	self->name = AnsiString_Duplicate(name);
		
	*instance = self;
	return S_OK;
}

size_t DeviceSupportedCommand::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceSupportedCommand::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceSupportedCommand::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceSupportedCommand))
		*object = static_cast<ifc_devicesupportedcommand*>(this);
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

const char *DeviceSupportedCommand::GetName()
{
	return name;
}

HRESULT DeviceSupportedCommand::GetFlags(DeviceCommandFlags *flagsOut)
{
	if (NULL == flagsOut)
		return E_POINTER;
	
	*flagsOut = flags;
	
	return S_OK;
}

HRESULT DeviceSupportedCommand::SetFlags(DeviceCommandFlags mask, DeviceCommandFlags value)
{
	DeviceCommandFlags temp;
	temp = (flags & mask) | (mask & value);

	if (temp == flags)
		return S_FALSE;

	flags = temp;
	
	return S_OK;
}

#define CBCLASS DeviceSupportedCommand
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_GETFLAGS, GetFlags)
END_DISPATCH;
#undef CBCLASS