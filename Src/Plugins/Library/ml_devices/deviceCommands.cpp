#include "main.h"
#include "deviceCommands.h"


typedef struct DeviceCommandInfo
{ 
	const char		*name;
	unsigned int	title;
	unsigned int	description;
	unsigned int 	smallIcon;
	unsigned int	largeIcon;
} DeviceCommandInfo;


static DeviceCommandInfo registeredCommands[] = 
{
	{ "sync", 
	  IDS_SYNC_COMMAND_TITLE, 
	  IDS_SYNC_COMMAND_DESC, 
	  IDR_SYNC_COMMAND_SMALL_IMAGE, 
	  IDR_SYNC_COMMAND_LARGE_IMAGE },
	{ "cancel_sync", 
	  IDS_CANCEL_SYNC_COMMAND_TITLE, 
	  IDS_CANCEL_SYNC_COMMAND_DESC, 
	  IDR_CANCEL_SYNC_COMMAND_SMALL_IMAGE, 
	  IDR_CANCEL_SYNC_COMMAND_LARGE_IMAGE },
	{ "attach", 
	  IDS_ATTACH_COMMAND_TITLE, 
	  IDS_ATTACH_COMMAND_DESC, 
	  IDR_ATTACH_COMMAND_SMALL_IMAGE, 
	  IDR_ATTACH_COMMAND_LARGE_IMAGE },
	{ "detach", 
	  IDS_DETACH_COMMAND_TITLE, 
	  IDS_DETACH_COMMAND_DESC, 
	  IDR_DETACH_COMMAND_SMALL_IMAGE, 
	  IDR_DETACH_COMMAND_LARGE_IMAGE },
	{ "eject", 
	  IDS_EJECT_COMMAND_TITLE, 
	  IDS_EJECT_COMMAND_DESC, 
	  IDR_EJECT_COMMAND_SMALL_IMAGE, 
	  IDR_EJECT_COMMAND_LARGE_IMAGE },
	{ "rename", 
	  IDS_RENAME_COMMAND_TITLE, 
	  IDS_RENAME_COMMAND_DESC, 
	  IDR_RENAME_COMMAND_SMALL_IMAGE, 
	  IDR_RENAME_COMMAND_LARGE_IMAGE },
	{ "view_open", 
	  IDS_VIEW_OPEN_COMMAND_TITLE, 
	  IDS_VIEW_OPEN_COMMAND_DESC, 
	  IDR_VIEW_OPEN_COMMAND_SMALL_IMAGE, 
	  IDR_VIEW_OPEN_COMMAND_LARGE_IMAGE },
	{ "preferences", 
	  IDS_PREFERENCES_COMMAND_TITLE, 
	  IDS_PREFERENCES_COMMAND_DESC, 
	  IDR_PREFERENCES_COMMAND_SMALL_IMAGE, 
	  IDR_PREFERENCES_COMMAND_LARGE_IMAGE },
	{ "playlist_create", 
	  IDS_PLAYLIST_CREATE_COMMAND_TITLE, 
	  IDS_PLAYLIST_CREATE_COMMAND_DESC, 
	  IDR_PLAYLIST_CREATE_COMMAND_SMALL_IMAGE, 
	  IDR_PLAYLIST_CREATE_COMMAND_LARGE_IMAGE },
};

static ifc_devicecommand * _cdecl
DeviceCommands_RegisterCommandCb(const char *name, void *user)
{
	size_t index;
	wchar_t buffer[2048] = {0};
	DeviceCommandInfo *commandInfo;
	ifc_devicecommand *command;
	ifc_devicecommandeditor *editor;

	commandInfo = NULL;
	for(index = 0; index < ARRAYSIZE(registeredCommands); index++)
	{
		if (name == registeredCommands[index].name)
		{
			commandInfo = &registeredCommands[index];
			break;
		}
	}

	if (NULL == commandInfo)
		return NULL;
	
	if (NULL == WASABI_API_DEVICES)
		return NULL;

	if (FAILED(WASABI_API_DEVICES->CreateCommand(commandInfo->name, &command)))
		return NULL;
	
	if (FAILED(command->QueryInterface(IFC_DeviceCommandEditor, (void**)&editor)))
	{
		command->Release();
		return NULL;
	}

	if (0 != commandInfo->title)
	{
		WASABI_API_LNGSTRINGW_BUF(commandInfo->title, buffer, ARRAYSIZE(buffer));
		editor->SetDisplayName(buffer);
	}

	if (0 != commandInfo->description)
	{
		WASABI_API_LNGSTRINGW_BUF(commandInfo->description, buffer, ARRAYSIZE(buffer));
		editor->SetDescription(buffer);
	}

	if (0 != commandInfo->smallIcon || 0 != commandInfo->largeIcon)
	{
		ifc_deviceiconstore *iconStore;
		if (SUCCEEDED(editor->GetIconStore(&iconStore)))
		{
			if (0 != commandInfo->smallIcon)
			{
				if (FALSE != Plugin_GetResourceString(MAKEINTRESOURCE(commandInfo->smallIcon), RT_RCDATA, buffer, ARRAYSIZE(buffer)))
					iconStore->Add(buffer, 16, 16, TRUE);
			}

			if (0 != commandInfo->largeIcon)
			{
				if (FALSE != Plugin_GetResourceString(MAKEINTRESOURCE(commandInfo->largeIcon), RT_RCDATA, buffer, ARRAYSIZE(buffer)))
					iconStore->Add(buffer, 43, 24, TRUE);
			}
			
			iconStore->Release();
		}
	}

	editor->Release();
	return command;
}

BOOL
DeviceCommands_Register()
{
	const char *commands[ARRAYSIZE(registeredCommands)];
	size_t index;

	if (NULL == WASABI_API_DEVICES)
		return FALSE;

	for(index = 0; index < ARRAYSIZE(commands); index++)
	{
		commands[index] = registeredCommands[index].name;
	}
		
	WASABI_API_DEVICES->CommandRegisterIndirect(commands, ARRAYSIZE(commands), DeviceCommands_RegisterCommandCb, NULL);
	
	return TRUE;
}


BOOL
DeviceCommand_GetSupported(ifc_device *device, const char *name, DeviceCommandContext context, 
							ifc_devicesupportedcommand **commandOut)
{
	ifc_devicesupportedcommandenum *enumerator;
	ifc_devicesupportedcommand *command;
	BOOL foundCommand;

	if (NULL == device ||
		NULL == name || 
		FAILED(device->EnumerateCommands(&enumerator, context)))
	{
		return FALSE;
	}
		
	foundCommand = FALSE;

	while (S_OK == enumerator->Next(&command, 1, NULL))
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, name, -1, command->GetName(), -1))
		{
			foundCommand = TRUE;
			
			if (NULL != commandOut)
				*commandOut = command;
			else
				command->Release();

			break;
		}
		command->Release();
	}

	enumerator->Release();
	return foundCommand;	
}

BOOL
DeviceCommand_GetEnabled(ifc_device *device, const char *name, DeviceCommandContext context)
{
	ifc_devicesupportedcommand *command;
	DeviceCommandFlags flags;
	
	if (FALSE == DeviceCommand_GetSupported(device, name, context, &command))
		return FALSE;

	if (FAILED(command->GetFlags(&flags)))
		flags = DeviceCommandFlag_Disabled;

	command->Release();

	return (0 == (DeviceCommandFlag_Disabled & flags));
}