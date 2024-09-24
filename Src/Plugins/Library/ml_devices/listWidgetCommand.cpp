#include "main.h"
#include "./listWidgetInternal.h"

#define REQUEST_STRING   ((wchar_t*)1)
#define REQUEST_IMAGE   ((DeviceImage*)1)

typedef struct ListWidgetCommand
{
	char *name;
	ListWidgetCommandState state;
	wchar_t *title;
	wchar_t *description;
	DeviceImage *imageLarge;
	DeviceImage *imageSmall;
	RECT rect;
} ListWidgetCommand;

ListWidgetCommand *
ListWidget_CreateCommand(const char *name, BOOL primary, BOOL disabled)
{
	ListWidgetCommand *self;

	if (NULL == name)
		return NULL;

	self = (ListWidgetCommand*)malloc(sizeof(ListWidgetCommand));
	if (NULL == self)
		return NULL;
		
	ZeroMemory(self, sizeof(ListWidgetCommand));

	self->name = AnsiString_Duplicate(name);
	self->state = ListWidgetCommandState_Normal;

	if (NULL != self->name)
	{
		if (FALSE != primary)
			self->state |= ListWidgetCommandState_Primary;

		if (FALSE != disabled)
			self->state |= ListWidgetCommandState_Disabled;

	}
	else
	{
		ListWidget_DestroyCommand(self);
		return NULL;
	}
	
	self->title = REQUEST_STRING;
	self->description = REQUEST_STRING;
	self->imageLarge = REQUEST_IMAGE;
	self->imageSmall = REQUEST_IMAGE;

	return self;
}

void
ListWidget_DestroyCommand(ListWidgetCommand *command)
{
	if (NULL == command)
		return;

	AnsiString_Free(command->name);

	if (NULL != command->imageLarge && REQUEST_IMAGE != command->imageLarge)
		DeviceImage_Release(command->imageLarge);

	if (NULL != command->imageSmall && REQUEST_IMAGE != command->imageSmall)
		DeviceImage_Release(command->imageSmall);
	
	if (REQUEST_STRING != command->title)
		String_Free(command->title);
	
	if (REQUEST_STRING != command->description)
		String_Free(command->description);
	
	free(command);
}

size_t 
ListWigdet_GetDeviceCommands(ListWidgetCommand **buffer, size_t bufferMax, ifc_device *device)
{
	size_t count;
	ifc_devicesupportedcommandenum *enumerator;
	ifc_devicesupportedcommand *command;
	DeviceCommandFlags flags;
	ListWidgetCommand *widgetCommand;
	BOOL primaryFound;

	if (NULL == buffer || bufferMax < 1 || NULL == device)
		return 0;

	if (FAILED(device->EnumerateCommands(&enumerator, DeviceCommandContext_View)))
		return 0;

	count = 0;
	primaryFound = FALSE;

	while(S_OK == enumerator->Next(&command, 1, NULL))
	{
		if(FAILED(command->GetFlags(&flags)))
			flags = DeviceCommandFlag_None;
		
		if (0 == (DeviceCommandFlag_Hidden & flags))
		{
			widgetCommand = ListWidget_CreateCommand(command->GetName(),
							(0 != (DeviceCommandFlag_Primary & flags)),
							(0 != (DeviceCommandFlag_Disabled & flags)));
			if (NULL != widgetCommand)
			{
				if (0 != (DeviceCommandFlag_Primary & flags))
				{
					if (count == bufferMax)
					{
						ListWidget_DestroyCommand(buffer[count-1]);
						count--;
					}

					if (count > 0)
						MoveMemory(&buffer[1], buffer, sizeof(ListWidgetCommand*) * count);
					
					buffer[0] = widgetCommand;
					primaryFound = TRUE;
					count++;
				}
				else
				{
					if (count < bufferMax)
					{
						buffer[count] = widgetCommand;
						count++;
					}
				}
			}
		}
		command->Release();

		if (count == bufferMax && FALSE != primaryFound)
			break;
	}
	
	enumerator->Release();

	return count;
}

void
ListWidget_DestroyAllCommands(ListWidgetCommand **buffer, size_t bufferMax)
{
	if (NULL == buffer)
		return;
	
	while(bufferMax--)
		ListWidget_DestroyCommand(buffer[bufferMax]);
}



const char *
ListWidget_GetCommandName(ListWidgetCommand *command)
{
	return (NULL != command) ? command->name : NULL;
}

const wchar_t *
ListWidget_GetCommandTitle(ListWidgetCommand *command)
{
	if (NULL == command || NULL == command->title)
		return NULL;

	if (REQUEST_STRING == command->title)
	{
		ifc_devicecommand *info;

		command->title = NULL;

		if (NULL != WASABI_API_DEVICES &&
			S_OK == WASABI_API_DEVICES->CommandFind(command->name, &info))
		{
			wchar_t buffer[512] = {0};

			if (SUCCEEDED(info->GetDisplayName(buffer, ARRAYSIZE(buffer))))
				command->title = String_Duplicate(buffer);

			info->Release();
		}
	}

	return command->title;
}

const wchar_t *
ListWidget_GetCommandDescription(ListWidgetCommand *command)
{
	if (NULL == command || NULL == command->description)
		return NULL;

	if (REQUEST_STRING == command->description)
	{
		ifc_devicecommand *info;

		command->description = NULL;

		if (NULL != WASABI_API_DEVICES &&
			S_OK == WASABI_API_DEVICES->CommandFind(command->name, &info))
		{
			wchar_t buffer[1024] = {0};

			if (SUCCEEDED(info->GetDescription(buffer, ARRAYSIZE(buffer))))
				command->description = String_Duplicate(buffer);

			info->Release();
		}
	}
	return command->description;
}

HBITMAP
ListWidget_GetCommandLargeBitmap(WidgetStyle *style, ListWidgetCommand *command, int width, int height)
{
	if (NULL == style || NULL == command || NULL == command->imageLarge)
		return NULL;
	
	if (REQUEST_IMAGE == command->imageLarge)
	{
		ifc_devicecommand *info;

		command->imageLarge = NULL;

		if (NULL != WASABI_API_DEVICES &&
			S_OK == WASABI_API_DEVICES->CommandFind(command->name, &info))
		{
			wchar_t buffer[MAX_PATH * 2] = {0};

			if (FAILED(info->GetIcon(buffer, ARRAYSIZE(buffer), width, height)))
				buffer[0] = L'\0';
		
			info->Release();

			if (L'\0' != buffer[0])
			{
				command->imageLarge = DeviceImageCache_GetImage(Plugin_GetImageCache(), buffer, 
								width, height, NULL, NULL);

			}
		}
	}

	return DeviceImage_GetBitmap(command->imageLarge, DeviceImage_Normal);
}

HBITMAP
ListWidget_GetCommandSmallBitmap(WidgetStyle *style, ListWidgetCommand *command, int width, int height)
{
	if (NULL == style || NULL == command || NULL == command->imageSmall)
		return NULL;
	
	if (REQUEST_IMAGE == command->imageSmall)
	{
		ifc_devicecommand *info;

		command->imageSmall = NULL;

		if (NULL != WASABI_API_DEVICES &&
			S_OK == WASABI_API_DEVICES->CommandFind(command->name, &info))
		{
			wchar_t buffer[MAX_PATH * 2] = {0};

			if (FAILED(info->GetIcon(buffer, ARRAYSIZE(buffer), width, height)))
				buffer[0] = L'\0';
		
			info->Release();

			if (L'\0' != buffer[0])
			{
				command->imageSmall = DeviceImageCache_GetImage(Plugin_GetImageCache(), buffer, 
								width, height, NULL, NULL);

			}
		}
	}

	return DeviceImage_GetBitmap(command->imageSmall, DeviceImage_Normal);
}

BOOL
ListWidget_ResetCommandImages(ListWidgetCommand *command)
{
	if (NULL == command)
		return FALSE;
	
	if (REQUEST_IMAGE != command->imageLarge)
	{
		if (NULL != command->imageLarge)
			DeviceImage_Release(command->imageLarge);
		command->imageLarge = REQUEST_IMAGE;
	}

	if (REQUEST_IMAGE != command->imageSmall)
	{
		if (NULL != command->imageSmall)
			DeviceImage_Release(command->imageSmall);
		command->imageSmall = REQUEST_IMAGE;
	}

	return TRUE;
}

BOOL
ListWidget_GetCommandRect(ListWidgetCommand *command, RECT *rect)
{
	if (NULL == command || NULL == rect)
		return FALSE;

	return CopyRect(rect, &command->rect);
}

BOOL
ListWidget_SetCommandRect(ListWidgetCommand *command, const RECT *rect)
{
	if (NULL == command || NULL == rect)
		return FALSE;

	return CopyRect(&command->rect, rect);
}

BOOL
ListWidget_GetCommandRectEqual(ListWidgetCommand *command, const RECT *rect)
{
	if (NULL == command || NULL == rect)
		return FALSE;

	return EqualRect(&command->rect, rect);
}

BOOL
ListWidget_GetCommandPrimary(ListWidgetCommand *command)
{
	return (NULL == command || 
			(0 != (ListWidgetCommandState_Primary & command->state)));
}


BOOL
ListWidget_GetCommandDisabled(ListWidgetCommand *command)
{
	return (NULL == command || 
			(0 != (ListWidgetCommandState_Disabled & command->state)));
}


BOOL
ListWidget_GetCommandPressed(ListWidgetCommand *command)
{
	return (NULL == command || 
			(0 != (ListWidgetCommandState_Pressed & command->state)));
}

BOOL
ListWidget_EnableCommand(ListWidgetCommand *command, BOOL enable)
{
	if (NULL == command)
		return FALSE;

	if ((FALSE == enable) == (0 != (ListWidgetCommandState_Disabled & command->state)))
		return FALSE;
	
	if (FALSE == enable)
		command->state |= ListWidgetCommandState_Disabled;
	else
		command->state &= ~ListWidgetCommandState_Disabled;

	return TRUE;
}

BOOL
ListWidget_SetCommandPressed(ListWidgetCommand *command, BOOL pressed)
{
	if (NULL == command)
		return FALSE;

	if ((FALSE == pressed) == (0 == (ListWidgetCommandState_Pressed & command->state)))
		return FALSE;
	
	if (FALSE == pressed)
		command->state &= ~ListWidgetCommandState_Pressed;
	else
		command->state |= ListWidgetCommandState_Pressed;

	return TRUE;
}