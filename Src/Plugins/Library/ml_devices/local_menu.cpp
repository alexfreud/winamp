#include "main.h"
#include "./local_menu.h"

unsigned int
Menu_InsertDeviceItems(HMENU menu, int position, unsigned int baseId, 
					   ifc_device *device, DeviceCommandContext context)
{
	unsigned int count, separator;
	MENUITEMINFO itemInfo = {0};
	wchar_t itemName[512] = {0};

	ifc_devicecommand *commandInfo;
	ifc_devicesupportedcommandenum *enumerator;
	ifc_devicesupportedcommand *command;
	DeviceCommandFlags commandFlags;

	if (NULL == device || NULL == menu)
		return 0;

	if (FAILED(device->EnumerateCommands(&enumerator, context)))
		return 0;

	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_DATA | MIIM_ID | MIIM_FTYPE | MIIM_STATE | MIIM_STRING;
		
	count = 0;
	separator = 0;

	while(S_OK == enumerator->Next(&command, 1, NULL))
	{
		if(SUCCEEDED(command->GetFlags(&commandFlags)))
		{
			if (0 != (DeviceCommandFlag_Group & commandFlags) && 
				separator != count)
			{
				itemInfo.fType = MFT_SEPARATOR;
				itemInfo.fState = MFS_ENABLED;
				itemInfo.wID = 0xFFFE;
				itemInfo.dwItemData = NULL;
				if (0 != InsertMenuItem(menu, position + count, TRUE, &itemInfo))
				{
					count++;
					separator = count;
				}
			}
			if (0 == (DeviceCommandFlag_Hidden & commandFlags))
			{
				if (S_OK == WASABI_API_DEVICES->CommandFind(command->GetName(), &commandInfo))
				{
					if (SUCCEEDED(commandInfo->GetDisplayName(itemName, ARRAYSIZE(itemName))))
					{						
						itemInfo.dwItemData = (ULONG_PTR)AnsiString_Duplicate(command->GetName());
						if (NULL != itemInfo.dwItemData)
						{
							itemInfo.fType = MFT_STRING;
							itemInfo.dwTypeData = itemName;
							itemInfo.fState = 0;
							itemInfo.wID = baseId + count;

							if (0 == (DeviceCommandFlag_Disabled & commandFlags))
								itemInfo.fState |= MFS_ENABLED;
							else
								itemInfo.fState |= (MFS_DISABLED | MFS_GRAYED);
														
							if (0 != (DeviceCommandFlag_Primary & commandFlags))
								itemInfo.fState |= MFS_DEFAULT;

							if (0 != InsertMenuItem(menu, position + count, TRUE, &itemInfo))
								count++;
							else
								AnsiString_Free((char*)itemInfo.dwItemData);
						}

					}
					commandInfo->Release();
				}
			}
		}
		command->Release();
	}
	

	enumerator->Release();
	
	return count;
}

unsigned int 
Menu_FreeItemData(HMENU menu, unsigned int start, int count)
{
	unsigned int processed;
	MENUITEMINFO itemInfo;

	if (NULL == menu)
		return 0;
	
	if (count < 0 )
		count = GetMenuItemCount(menu);

	if (start > (unsigned int)count)
		return 0;
	
	count -= start;
	processed = 0;

	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_DATA;

	while(count-- && 
		  FALSE != GetMenuItemInfo(menu, start + processed, TRUE, &itemInfo))
	{
		AnsiString_Free((char*)itemInfo.dwItemData);
		processed++;
	}
	
	return processed;
}

ULONG_PTR
Menu_GetItemData(HMENU menu, unsigned int item, BOOL byPosition)
{
	MENUITEMINFO itemInfo;

	if (NULL == menu)
		return 0;

	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_DATA;

	if (FALSE != GetMenuItemInfo(menu, item, byPosition, &itemInfo))
		return itemInfo.dwItemData;
	
	return 0;
}

unsigned int
Menu_FindItemByData(HMENU menu, Menu_FindItemByDataCb callback, void *user)
{
	int index, count;
	MENUITEMINFO itemInfo;

	if (NULL == menu || NULL == callback)
		return -1;

	count = GetMenuItemCount(menu);
	if (0 == count)
		return -1;

	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_DATA | MIIM_ID;

	for (index = 0; index < count; index++)
	{
		if (FALSE != GetMenuItemInfo(menu, index, TRUE, &itemInfo) &&
			FALSE != callback(itemInfo.dwItemData, user))
		{
			return itemInfo.wID;
		}
	}
	
	return -1;

}

static BOOL
Menu_FindItemByAnsiStringDataCb(ULONG_PTR param, void *user)
{
	const char *string1, *string2;
	
	string1 = (const char*)param;
	string2 = (const char*)user;
	
	if (NULL == string1 || NULL == string2)
		return (string1 == string2);

	return (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, string1, -1, string2, -1));
}

unsigned int
Menu_FindItemByAnsiStringData(HMENU menu, const char *string)
{
	return Menu_FindItemByData(menu, Menu_FindItemByAnsiStringDataCb, (void*)string);
}