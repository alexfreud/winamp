#include "main.h"
#include "DeviceCommands.h"

#include <strsafe.h>
PortableCommand::PortableCommand(const char *name,  int title, int description) 
	: name(name), title(title), description(description)
{
}

const char *PortableCommand::GetName()
{ 
	return name; 
}

HRESULT PortableCommand::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return E_NOTIMPL;
}

HRESULT PortableCommand::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	if (NULL == buffer)
		return E_POINTER;

	WASABI_API_LNGSTRINGW_BUF(title, buffer, bufferSize);
	return S_OK;
}

HRESULT PortableCommand::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	if (NULL == buffer)
		return E_POINTER;

	WASABI_API_LNGSTRINGW_BUF(description, buffer, bufferSize);
	return S_OK;
}

#define CBCLASS PortableCommand
START_DISPATCH;
CB(API_GETNAME, GetName);
CB(API_GETICON, GetIcon);
CB(API_GETDISPLAYNAME, GetDisplayName);
CB(API_GETDESCRIPTION, GetDescription);
END_DISPATCH;
#undef CBCLASS


BOOL SetDeviceCommandInfo(DeviceCommandInfo *info, const char *name, DeviceCommandFlags flags)
{
	if (NULL == info)
		return FALSE;

	info->name = name;
	info->flags = flags;
	return TRUE;
}

/* -------------- */
DeviceCommand::DeviceCommand(const char *name, DeviceCommandFlags flags)
: name(name), flags(flags)
{
}

DeviceCommand::DeviceCommand(const DeviceCommandInfo *commandInfo)
: name(commandInfo->name), flags(commandInfo->flags)
{
}

const char *DeviceCommand::GetName() 
{
	return name; 
}

HRESULT DeviceCommand::GetFlags(DeviceCommandFlags *flags) 
{
	*flags = this->flags; 
	return 0; 
}


#define CBCLASS DeviceCommand
START_DISPATCH;
REFERENCE_COUNTED;
CB(API_GETNAME, GetName);
CB(API_GETFLAGS, GetFlags);
END_DISPATCH;
#undef CBCLASS



DeviceCommandEnumerator::DeviceCommandEnumerator(const DeviceCommandInfo *commandInfoList, size_t listSize) 
	: position(0), commands(NULL), count(0)
{
	if (NULL != commandInfoList && 
		0 != listSize)
	{
		commands = (DeviceCommand**)calloc(listSize, sizeof(DeviceCommand*));
		if (NULL != commands)
		{
			for(count = 0; count < listSize; count++)
			{
				commands[count] = new DeviceCommand(&commandInfoList[count]);
			}
		}
	}
}

DeviceCommandEnumerator::~DeviceCommandEnumerator()
{
	if (NULL != commands)
	{
		while(count--)
			commands[count]->Release();
		
		free(commands);
	}
		
}

HRESULT DeviceCommandEnumerator::Next(ifc_devicesupportedcommand **buffer, size_t bufferMax, size_t *fetched)
{
	size_t available, copied, index;
	DeviceCommand **source;

	if (NULL == buffer)
		return E_POINTER;
	
	if (0 == bufferMax) 
		return E_INVALIDARG;

	if (position >= count)
	{
		if (NULL != fetched) 
			*fetched = 0;

		return S_FALSE;
	}

	available = count - position;
	copied = ((available > bufferMax) ? bufferMax : available);
	
	source = commands + position;
	CopyMemory(buffer, source, copied * sizeof(ifc_devicesupportedcommand*));
    
	for(index = 0; index < copied; index++)
		buffer[index]->AddRef();
	
	position += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT DeviceCommandEnumerator::Reset(void)
{
	position=0;
	return S_OK;
}

HRESULT DeviceCommandEnumerator::Skip(size_t count)
{
	position += count;
	if (position > this->count)
		position = this->count;
	return (position < this->count) ? S_OK : S_FALSE;
}

HRESULT DeviceCommandEnumerator::GetCount(size_t *count)
{ 
	if (NULL == count)
		return E_POINTER;
	
	*count = this->count;

	return S_OK;
}


#define CBCLASS DeviceCommandEnumerator
START_DISPATCH;
CB(API_NEXT, Next);
CB(API_RESET, Reset);
CB(API_SKIP, Skip);
CB(API_GETCOUNT, GetCount);
REFERENCE_COUNTED;
END_DISPATCH;
#undef CBCLASS



