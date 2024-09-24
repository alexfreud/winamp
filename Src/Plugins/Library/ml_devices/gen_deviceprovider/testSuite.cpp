#include "main.h"
#include "./testSuite.h"

#include <strsafe.h>

TestSuite::TestSuite()
{
}

TestSuite::~TestSuite()
{
	size_t index;

	index = deviceList.size();
	while(index--)
	{
		deviceList[index]->Release();
	}

	index = typeList.size();
	while(index--)
	{
		typeList[index]->Release();
	}

	index = connectionList.size();
	while(index--)
	{
		connectionList[index]->Release();
	}

	index = commandList.size();
	while(index--)
	{
		commandList[index]->Release();
	}

	index = insertList.size();
	while(index--)
	{
		AnsiString_Free(insertList[index]);
	}
}

BOOL TestSuite::AddDevice(Device *device)
{
	if (NULL == device)
		return FALSE;

	deviceList.push_back(device);
	device->AddRef();

	return TRUE;
}

size_t TestSuite::GetDeviceCount()
{
	return deviceList.size();
}

Device *TestSuite::GetDevice(size_t index)
{
	return deviceList[index];
}

static int 
String_RemoveCounter(wchar_t *string)
{
	int len;

	len = lstrlenW(string);
	if (len > 3)
	{
		int cutoff = 0;
		if (string[len-1] == L')')
		{
			WORD charType;
			cutoff = 1;
			while(len > cutoff &&
				  FALSE != GetStringTypeW(CT_CTYPE1, string + (len - 1) - cutoff, 1, &charType) &&
				  0 != (C1_DIGIT & charType))
			{
				cutoff++;
			}

			if (len > cutoff &&
				cutoff > 1 && 
				L'(' == string[len - 1 - cutoff])
			{
				cutoff++;
				if (len > cutoff &&
					L' ' == string[len - 1 - cutoff])
				{
					string[len - 1 - cutoff] = L'\0';
					len -= (cutoff + 1);
				}
			}
		}
	}

	return len;
}


static int 
AnsiString_RemoveCounter(char *string)
{
	int len;

	len = lstrlenA(string);
	if (len > 3)
	{
		int cutoff = 0;
		if (string[len-1] == ')')
		{
			WORD charType;
			cutoff = 1;
			while(len > cutoff &&
				  FALSE != GetStringTypeA(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, string + (len - 1) - cutoff, 1, &charType) &&
				  0 != (C1_DIGIT & charType))
			{
				cutoff++;
			}

			if (len > cutoff &&
				cutoff > 1 && 
				'(' == string[len - 1 - cutoff])
			{
				cutoff++;
				if (len > cutoff &&
					' ' == string[len - 1 - cutoff])
				{
					string[len - 1 - cutoff] = '\0';
					len -= (cutoff + 1);
				}
			}
		}
	}

	return len;
}

Device *TestSuite::CreateDeviceCopy(Device *source)
{
	const char *name;
	size_t index, counter;
	char buffer[1024];
	wchar_t *displayName;
	BOOL found;
	Device *destination;
	int length;

	if (NULL == source)
		return NULL;
	
	found = FALSE;
	counter = 0;

	name = source->GetName();
	StringCbCopyA(buffer, sizeof(buffer), name);
	length = AnsiString_RemoveCounter(buffer);

	for (;;)
	{
		
		if (0 != counter)
			StringCbPrintfA(buffer + length, sizeof(buffer) - length, " (%d)", counter);

		found = TRUE;

		index = deviceList.size();
		while(index--)
		{
			if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, deviceList[index]->GetName(), -1, buffer, -1))
			{
				found = FALSE;
				break;
			}
		}

		if (FALSE != found)
			break;
		else
			counter++;
	}

	if (FAILED(Device::CreateInstance(buffer, source->GetType(), source->GetConnection(), &destination)))
		return NULL;

	source->CopyTo(destination);
		
	source->GetDisplayName((wchar_t*)buffer, sizeof(buffer)/sizeof(wchar_t));
	displayName = (wchar_t*)buffer;
	length = String_RemoveCounter(displayName);
	
	if (0 != counter)
		StringCchPrintf(displayName + length, sizeof(buffer)/sizeof(wchar_t) -  length, L" (%d)", counter);
	
	destination->SetDisplayName(displayName);
			

	return destination;

}
Device *TestSuite::GetRandomDevice()
{
	size_t index;
	Device *device;
	LARGE_INTEGER perfCounter;

	if (0 == deviceList.size())
		return NULL;

	if (FALSE != QueryPerformanceCounter(&perfCounter))
		srand(perfCounter.LowPart);
	else
		srand(GetTickCount());

	index = (size_t)((double)rand()/(RAND_MAX + 1) * deviceList.size());
	device = deviceList[index];

	if (S_OK == device->IsConnected())
	{
		size_t search;

		for(search = index + 1; search < deviceList.size(); search++)
		{
			device = deviceList[search];
			if (S_FALSE == device->IsConnected())
				return device;
		}

		search = index;
		while(search--)
		{
			device = deviceList[search];
			if (S_FALSE == device->IsConnected())
				return device;
		}

		device = CreateDeviceCopy(deviceList[index]);
		if (NULL != device)
		{
			size_t totalSpace, usedSpace;
			totalSpace = (size_t)((double)rand()/(RAND_MAX + 1) * 1000);
			usedSpace = (size_t)((double)rand()/(RAND_MAX + 1) * totalSpace);

			device->SetTotalSpace(totalSpace);
			device->SetUsedSpace(usedSpace);

			device->Disconnect();
			device->Detach(NULL);
			AddDevice(device);
		}
	}

	return device;
}
	
BOOL TestSuite::AddType(ifc_devicetype *type)
{
	if (NULL == type)
		return FALSE;

	typeList.push_back(type);
	type->AddRef();

	return TRUE;
}

size_t TestSuite::GetTypeCount()
{
	return typeList.size();
}

ifc_devicetype *TestSuite::GetType(size_t index)
{
	return typeList[index];
}


BOOL TestSuite::RegisterTypes(api_devicemanager *manager)
{
	if (NULL == manager)
		return FALSE;

	if (0 != typeList.size())
		manager->TypeRegister((ifc_devicetype**)typeList.begin(), typeList.size());

	return TRUE;
}

BOOL TestSuite::UnregisterTypes(api_devicemanager *manager)
{
	size_t index;
	if (NULL == manager)
		return FALSE;

	index = typeList.size();
	while(index--)
	{
		manager->TypeUnregister(typeList[index]->GetName());
	}

	return TRUE;
}

BOOL TestSuite::SetIconBase(const wchar_t *path)
{
	size_t index;
	Device *device;
	ifc_devicetype *type;
	ifc_devicetypeeditor *typeEditor;
	ifc_deviceiconstore *iconStore;
	ifc_deviceconnection *connection;
	ifc_deviceconnectioneditor *connectionEditor;
	ifc_devicecommand *command;
	ifc_devicecommandeditor *commandEditor;

	index = deviceList.size();
	while(index--)
	{
		device = deviceList[index];
		device->SetIconBase(path);
	}

	index = typeList.size();
	while(index--)
	{
		type = typeList[index];
		if (SUCCEEDED(type->QueryInterface(IFC_DeviceTypeEditor, (void**)&typeEditor)))
		{
			if(SUCCEEDED(typeEditor->GetIconStore(&iconStore)))
			{
				iconStore->SetBasePath(path);
				iconStore->Release();
			}
			typeEditor->Release();
		}
	}

	index = commandList.size();
	while(index--)
	{
		command = commandList[index];
		if (SUCCEEDED(command->QueryInterface(IFC_DeviceCommandEditor, (void**)&commandEditor)))
		{
			if(SUCCEEDED(commandEditor->GetIconStore(&iconStore)))
			{
				iconStore->SetBasePath(path);
				iconStore->Release();
			}
			commandEditor->Release();
		}
	}

	index = connectionList.size();
	while(index--)
	{
		connection = connectionList[index];
		if (SUCCEEDED(connection->QueryInterface(IFC_DeviceConnectionEditor, (void**)&connectionEditor)))
		{
			if(SUCCEEDED(connectionEditor->GetIconStore(&iconStore)))
			{
				iconStore->SetBasePath(path);
				iconStore->Release();
			}
			connectionEditor->Release();
		}
	}


	return S_OK;
}

BOOL TestSuite::SetConnectList(char **devices, size_t count)
{
	size_t index;
	char *name;

	index = insertList.size();
	if (index > 0)
	{
		while(index--)
		{
			name = insertList[index];
			AnsiString_Free(name);
		}
		insertList.clear();
	}

	for(index = 0; index < count; index++)
	{
		name = AnsiString_Duplicate(devices[index]);
		if (NULL != name)
			insertList.push_back(name);
	}

	return TRUE;
}

Device *TestSuite::GetDeviceByName(const char *name)
{
	size_t index;
	Device *device;

	for (index = 0; index < deviceList.size(); index++)
	{
		device = deviceList[index];
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, name, -1, device->GetName(), -1))
			return device;
	}

	return NULL;
}

BOOL TestSuite::RegisterDevices(api_devicemanager *manager)
{
	size_t index;
	const char *name;
	Device *device;

	if (NULL == manager)
		return FALSE;
	
	for (index = 0; index < insertList.size(); index++)
	{
		name = insertList[index];
		device = GetDeviceByName(name);
		if (NULL != device)
		{
			manager->DeviceRegister((ifc_device**)&device, 1);
			device->Connect();
		}
	}
	
	return TRUE;
}

BOOL TestSuite::UnregisterDevices(api_devicemanager *manager)
{
	size_t index;
	Device *device;

	if (NULL == manager)
		return FALSE;
	
	index = deviceList.size();
	while(index--)
	{
		device = deviceList[index];
		if (S_OK == device->IsConnected())
		{	
			device->Disconnect();
			manager->DeviceUnregister(device->GetName());
		}
	}
	
	return TRUE;
}

BOOL TestSuite::AddConnection(ifc_deviceconnection *connection)
{
	if (NULL == connection)
		return FALSE;

	connectionList.push_back(connection);
	connection->AddRef();

	return TRUE;
}

size_t TestSuite::GetConnectionCount()
{
	return connectionList.size();
}

ifc_deviceconnection *TestSuite::GetConnection(size_t index)
{
	return connectionList[index];
}

BOOL TestSuite::RegisterConnections(api_devicemanager *manager)
{
	if (NULL == manager)
		return FALSE;

	if (0 != connectionList.size())
		manager->ConnectionRegister((ifc_deviceconnection**)connectionList.begin(), connectionList.size());

	return TRUE;
}

BOOL TestSuite::UnregisterConnections(api_devicemanager *manager)
{
	size_t index;
	if (NULL == manager)
		return FALSE;

	index = connectionList.size();
	while(index--)
	{
		manager->ConnectionUnregister(connectionList[index]->GetName());
	}

	return TRUE;
}


BOOL TestSuite::AddCommand(ifc_devicecommand *command)
{
	if (NULL == command)
		return FALSE;

	commandList.push_back(command);
	command->AddRef();

	return TRUE;
}

size_t TestSuite::GetCommandCount()
{
	return commandList.size();
}

ifc_devicecommand *TestSuite::GetCommand(size_t index)
{
	return commandList[index];
}

BOOL TestSuite::RegisterCommands(api_devicemanager *manager)
{
	if (NULL == manager)
		return FALSE;

	if (0 != commandList.size())
		manager->CommandRegister((ifc_devicecommand**)commandList.begin(), commandList.size());

	return TRUE;
}

BOOL TestSuite::UnregisterCommands(api_devicemanager *manager)
{
	size_t index;
	if (NULL == manager)
		return FALSE;

	index = commandList.size();
	while(index--)
	{
		manager->CommandUnregister(commandList[index]->GetName());
	}

	return TRUE;
}