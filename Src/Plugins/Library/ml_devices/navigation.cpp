#include "main.h"
#include "./navigation.h"

#include <strsafe.h>

#define DEVICES_NAVITEM_PREFIX		L"ml_devices_"

static HNAVITEM navigationRoot = NULL;
static size_t deviceHandler = 0;

static HNAVITEM
NavigationItem_Insert(HNAVITEM parent, 
					  HNAVITEM insertAfter, 
					  unsigned int mask,
					  int itemId,
					  const wchar_t *name, 
					  const wchar_t *displayName,
					  unsigned int style,
					  int image,
					  LPARAM param)
{
	NAVITEM *item;
	NAVINSERTSTRUCT nis = {0};

	nis.hInsertAfter = insertAfter;
	nis.hParent = parent;
	item = &nis.item;

	item->cbSize = sizeof(NAVITEM);
	item->id = itemId;

	if (0 != (NIMF_IMAGE & mask))
		mask |= NIMF_IMAGESEL;

	item->mask = mask;
	item->pszText = (wchar_t*)displayName;
	item->pszInvariant = (wchar_t*)name;
	item->style = style;
	item->styleMask = style;
	item->iImage = image;
	item->iSelectedImage = image;
	item->lParam = param;

	if (!wcsnicmp(L"ml_devices_all_sources", item->pszInvariant, 22))
	{
		// and this will allow us to make the cloud library
		// root do a web page or a sources view as needed...
		return NULL;
	}

	return MLNavCtrl_InsertItem(Plugin_GetLibraryWindow(), &nis);
}

static BOOL
NavigationItem_Delete(HNAVITEM item)
{
	return MLNavCtrl_DeleteItem(Plugin_GetLibraryWindow(), item);
}

static HNAVITEM 
NavigationItem_GetFromMessage(INT msg, INT_PTR param)
{
	return (msg < ML_MSG_NAVIGATION_FIRST) ? 
			MLNavCtrl_FindItemById(Plugin_GetLibraryWindow(), param) : 
			(HNAVITEM)param;
}

static HNAVITEM
NavigationItem_Find(HNAVITEM root, const wchar_t *name, BOOL allow_root = 0)
{
	NAVCTRLFINDPARAMS find = {0};
	HNAVITEM item;
	HWND libraryWindow;

	if (NULL == name)
		return NULL;

	libraryWindow = Plugin_GetLibraryWindow();
	if (NULL == libraryWindow)
		return NULL;

	find.pszName = (wchar_t*)name;
	find.cchLength = -1;
	find.compFlags = NICF_INVARIANT;
	find.fFullNameSearch = FALSE;

	item = MLNavCtrl_FindItemByName(libraryWindow, &find);
	if (NULL == item)
		return NULL;

	if (!allow_root)
	{
		// if allowed then we can look for root level items which
		// is really for getting 'cloud' devices to another group
		if (NULL != root && 
			root != MLNavItem_GetParent(libraryWindow, item))
		{
			item = NULL;
		}
	}

	return item;
}

static wchar_t *
NavigationItem_GetNameFromDeviceName(wchar_t *buffer, size_t bufferMax, const char *name)
{
	wchar_t *cursor;
	size_t length;
	BOOL allocated;

	if (NULL == name || '\0' == *name)
		return NULL;

	length = MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
	if (ARRAYSIZE(DEVICES_NAVITEM_PREFIX) > bufferMax || 
		length > (bufferMax - (ARRAYSIZE(DEVICES_NAVITEM_PREFIX) - 1)))
	{
		bufferMax = length + ARRAYSIZE(DEVICES_NAVITEM_PREFIX) - 1;
		buffer = String_Malloc(bufferMax);
		if (NULL == buffer)
			return NULL;

		allocated = TRUE;
	}
	else
		allocated = FALSE;

	if (FAILED(StringCchCopyEx(buffer, bufferMax, DEVICES_NAVITEM_PREFIX, &cursor, &length, 0)) ||
		0 == MultiByteToWideChar(CP_UTF8, 0, name, -1, cursor, (int)length))
	{
		if (FALSE != allocated)
			String_Free(buffer);

		return NULL;
	}

	return buffer;
}

static HNAVITEM
NavigationItem_FindFromDeviceName(HNAVITEM root, const char *name)
{
	wchar_t buffer[1], *itemName;
	HNAVITEM item;

	if (NULL == root)
		return NULL;

	itemName = NavigationItem_GetNameFromDeviceName(buffer, ARRAYSIZE(buffer), name);
	if (NULL == itemName)
		return NULL;

	item = NavigationItem_Find(root, itemName);

	if (itemName != buffer)
		String_Free(itemName);

	return item;
}

static HNAVITEM
NavigationItem_FindFromDevice(HNAVITEM root, ifc_device *device)
{
	if (NULL == device)
		return NULL;

	return NavigationItem_FindFromDeviceName(root, device->GetName());
}

static BOOL
NavigationItem_DisplayDeviceMenu(HNAVITEM item, const char *deviceName, HWND hostWindow, POINT pt)
{
	HMENU menu;
	ifc_device *device;
	unsigned int commandId;
	BOOL succeeded;
	HWND libraryWindow;

	libraryWindow = Plugin_GetLibraryWindow();

	if (NULL == item|| NULL == deviceName)
		return FALSE;

	if (NULL == WASABI_API_DEVICES || 
		S_OK != WASABI_API_DEVICES->DeviceFind(deviceName, &device))
	{
		return FALSE;
	}

	menu = CreatePopupMenu();
	if (NULL != menu)
	{
		if (0 == Menu_InsertDeviceItems(menu, 0, 100, device, DeviceCommandContext_NavigationMenu))
		{
			DestroyMenu(menu);
			menu = NULL;
		}
	}

	device->Release();

	if (NULL == menu)
		return FALSE;

	succeeded = FALSE;

	if (item == MLNavCtrl_GetSelection(libraryWindow))
	{
		commandId = Menu_FindItemByAnsiStringData(menu, "view_open");
		if ((unsigned int)-1 != commandId)
			EnableMenuItem(menu, commandId, MF_BYCOMMAND | MF_DISABLED);
	}

	commandId = Menu_TrackPopup(Plugin_GetLibraryWindow(), menu, 
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION | TPM_VERTICAL | TPM_RETURNCMD,
					pt.x, pt.y, hostWindow, NULL);

	if (0 != commandId)
	{
		const char *command;

		command = (const char*)Menu_GetItemData(menu, commandId, FALSE);
		if (NULL != command)
		{
			if (NULL != WASABI_API_DEVICES &&
				S_OK == WASABI_API_DEVICES->DeviceFind(deviceName, &device))
			{
				BOOL commandProcessed;

				commandProcessed = FALSE;

				if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, command, -1, "view_open", -1))
				{					
					succeeded = MLNavItem_Select(libraryWindow, item);
					commandProcessed = succeeded;
				}
				else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, command, -1, "rename", -1))
				{
					succeeded = MLNavItem_EditTitle(libraryWindow, item);
					commandProcessed = succeeded;
				}

				if (FALSE == commandProcessed &&
					SUCCEEDED(device->SendCommand(command, hostWindow, 0)))
				{
					succeeded = TRUE;
				}

				device->Release();
			}
		}
	}
	else
	{
		if (ERROR_SUCCESS == GetLastError())
			succeeded = TRUE;
	}

	Menu_FreeItemData(menu, 0, -1);

	return succeeded;
}

static BOOL
NavigationItem_DisplayRootMenu(HNAVITEM item, HWND hostWindow, POINT pt)
{
	HMENU pluginMenu, menu;
	BOOL succeeded;

	if (NULL == item)
		return FALSE;

	pluginMenu = Plugin_LoadMenu();
	if (NULL == pluginMenu)
		return FALSE;

	succeeded = FALSE;

	menu = GetSubMenu(pluginMenu, 0);
	if (NULL != menu)
	{
		HWND libraryWindow;
		HNAVITEM selectedItem;

		libraryWindow = Plugin_GetLibraryWindow();

		selectedItem = MLNavCtrl_GetSelection(libraryWindow);

		EnableMenuItem(menu, ID_VIEW_OPEN, MF_BYCOMMAND | ((item != selectedItem) ? MF_ENABLED : MF_DISABLED));
		SetMenuDefaultItem(menu, ID_VIEW_OPEN, FALSE);

		unsigned int commandId = Menu_TrackPopup(Plugin_GetLibraryWindow(), menu, 
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION | TPM_VERTICAL | TPM_RETURNCMD,
					pt.x, pt.y, hostWindow, NULL);

		if (0 != commandId)
		{
			switch(commandId)
			{
				case ID_VIEW_OPEN:
					MLNavItem_Select(libraryWindow, item);
					break;
				case ID_DISCOVERY_BEGIN:
					Plugin_BeginDiscovery();
					break;
				case ID_PLUGIN_HELP:
					Plugin_ShowHelp();
					break;
			}
		}
		else
		{
			if (ERROR_SUCCESS == GetLastError())
				succeeded = TRUE;
		}
	}

	DestroyMenu(pluginMenu);

	return succeeded;
}

static BOOL 
Navigation_GetDeviceTitle(ifc_device *device, wchar_t *buffer, size_t bufferMax)
{
	size_t read, write;
	
	if (NULL == device ||
		FAILED(device->GetDisplayName(buffer, bufferMax)))
	{
		return FALSE;
	}
		
	for(read = 0, write = 0;; read++)
	{
		if (read == bufferMax)
			return FALSE;

		if (L'\r' == buffer[read])
			continue;

		if (L'\n' == buffer[read] || 
			L'\t' == buffer[read] ||
			L'\b' == buffer[read])
		{
			buffer[write] = L' ';
		}
		
		buffer[write] = buffer[read];
		if (L'\0' == buffer[read])
			break;

		write++;
	}

	return TRUE;
}

static BOOL
Navigation_DeviceAdd(HNAVITEM root, ifc_device *device)
{
	HNAVITEM item, insertAfter = NCI_LAST;
	wchar_t nameBuffer[256] = {0}, *itemName;
	wchar_t title[512] = {0};
	int iconIndex;

	if (NULL == device)
		return FALSE;

	itemName = NavigationItem_GetNameFromDeviceName(nameBuffer, ARRAYSIZE(nameBuffer), device->GetName());
	if (NULL == itemName)
		return FALSE;

	if (NULL != NavigationItem_Find(root, itemName))
	{
		if (itemName != nameBuffer)
			String_Free(itemName);

		return FALSE;
	}

	if (FALSE == Navigation_GetDeviceTitle(device, title, ARRAYSIZE(title)))
		title[0] = L'\0';

	iconIndex = NavigationIcons_GetDeviceIconIndex(device);

	// filter the cloud devices to their own group
	if (!lstrcmpiA(device->GetConnection(), "cloud"))
	{
		HNAVITEM cloud = NavigationItem_Find(navigationRoot, L"cloud_sources", TRUE);
		if (cloud != NULL)
		{
			root = cloud;
			HNAVITEM transfers = NavigationItem_Find(0, L"cloud_transfers", TRUE);

			// to maintain some specific orders, we need to alter the insert position
			if (!wcsnicmp(L"ml_devices_hss", itemName, 14))
			{
				insertAfter = transfers;
				if (!insertAfter) insertAfter = NCI_LAST;
			}
			else if (!wcsnicmp(L"ml_devices_local_desktop", itemName, 24))
			{
				insertAfter = NavigationItem_Find(0, L"ml_devices_hss", TRUE);
				if (!insertAfter) insertAfter = NCI_LAST;
			}

			// when adding children, change from the cloud source to the open/closed arrow
			HWND libraryWindow = Plugin_GetLibraryWindow();
			if (NULL != libraryWindow && transfers)
			{
				NAVITEM itemInfo = {0};
				itemInfo.hItem = cloud;
				itemInfo.cbSize = sizeof(itemInfo);
				itemInfo.mask = NIMF_IMAGE | NIMF_IMAGESEL;
				itemInfo.iImage = itemInfo.iSelectedImage = -1;
				MLNavItem_SetInfo(libraryWindow, &itemInfo);
			}
		}
	}

	item = NavigationItem_Insert(root, insertAfter, 
								 NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL | NIMF_STYLE,
								 0, itemName, title, NIS_ALLOWEDIT, iconIndex, 0L);

	if (NULL == item)
		NavigationIcons_ReleaseIconIndex(iconIndex);

	if (itemName != nameBuffer)
		String_Free(itemName);

	if (NULL == item)
		return FALSE;

	device->SetNavigationItem(item);

	return TRUE;
}

static BOOL
Navigation_DeviceRemove(HNAVITEM root, ifc_device *device)
{
	HNAVITEM item;
			
	item = NavigationItem_FindFromDevice(root, device);
	if (NULL == item)
		return FALSE;

	return NavigationItem_Delete(item);
}

static BOOL
Navigation_DeviceTitleChanged(HNAVITEM root, ifc_device *device)
{
	NAVITEM itemInfo;
	wchar_t buffer[1024] = {0};

	itemInfo.hItem = NavigationItem_FindFromDevice(root, device);
	if (NULL == itemInfo.hItem)
		return FALSE;

	if (FALSE == Navigation_GetDeviceTitle(device, buffer, ARRAYSIZE(buffer)))
		return FALSE;
	
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.pszText = buffer;
	itemInfo.cchTextMax = -1;
	itemInfo.mask = NIMF_TEXT;

	return MLNavItem_SetInfo(Plugin_GetLibraryWindow(), &itemInfo);
}

static BOOL
Navigation_DeviceIconChanged(HNAVITEM root, ifc_device *device)
{
	NAVITEM itemInfo;
	int iconIndex;
	HWND libraryWindow;

	if (NULL == root || NULL == device)
		return FALSE;

	libraryWindow = Plugin_GetLibraryWindow();

	itemInfo.hItem = NavigationItem_FindFromDevice(root, device);
	if (NULL == itemInfo.hItem)
		return FALSE;

	iconIndex = NavigationIcons_GetDeviceIconIndex(device);

	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.mask = NIMF_IMAGE;

	if (FALSE == MLNavItem_GetInfo(libraryWindow, &itemInfo))
	{
		NavigationIcons_ReleaseIconIndex(iconIndex);
		return FALSE;
	}

	if (itemInfo.iImage == iconIndex)
	{
		NavigationIcons_ReleaseIconIndex(iconIndex);
		return TRUE;
	}

	NavigationIcons_ReleaseIconIndex(itemInfo.iImage);
	itemInfo.mask = NIMF_IMAGE | NIMF_IMAGESEL;
	itemInfo.iImage = iconIndex;
	itemInfo.iSelectedImage = iconIndex;

	if (FALSE == MLNavItem_SetInfo(libraryWindow, &itemInfo))
	{
		NavigationIcons_ReleaseIconIndex(iconIndex);
		return FALSE;
	}

	return TRUE;
}

static void
Navigation_AddExistingDevices(HNAVITEM root)
{
	ifc_device *device;
	ifc_deviceobject *object;
	ifc_deviceobjectenum *enumerator;

	if (NULL == WASABI_API_DEVICES)
		return;

	if (FAILED(WASABI_API_DEVICES->DeviceEnumerate(&enumerator)))
		return;

	while(S_OK == enumerator->Next(&object, 1, NULL))
	{
		if (SUCCEEDED(object->QueryInterface(IFC_Device, (void**)&device)))
		{
			if (FALSE == device->GetHidden() && 
				FALSE != device->GetAttached())
			{
				Navigation_DeviceAdd(root, device);
			}
			device->Release();
		}
		object->Release();
	}
	enumerator->Release();
}

static void
Navigation_DeviceCb(ifc_device *device, DeviceEvent eventId, void *user)
{
	HNAVITEM rootItem;
	rootItem = (HNAVITEM)user;
	
	switch(eventId)
	{
		case Event_DeviceAdded:
			if (FALSE != device->GetAttached())
				Navigation_DeviceAdd(rootItem, device);
			break;
		case Event_DeviceRemoved:
			if (FALSE != device->GetAttached())
				Navigation_DeviceRemove(rootItem, device);
			break;
		case Event_DeviceHidden:
			if (FALSE != device->GetAttached())
				Navigation_DeviceRemove(rootItem, device);
			break;
		case Event_DeviceShown:
			if (FALSE != device->GetAttached())
				Navigation_DeviceAdd(rootItem, device);
			break;
		case Event_DeviceAttached:
			Navigation_DeviceAdd(rootItem, device);
			break;
		case Event_DeviceDetached:
			Navigation_DeviceRemove(rootItem, device);
			break;
		case Event_DeviceDisplayNameChanged:
			Navigation_DeviceTitleChanged(rootItem, device);
			break;
		case Event_DeviceIconChanged:
			Navigation_DeviceIconChanged(rootItem, device);
			break;
	}
}

static BOOL
Navigation_RegisterDeviceHandler(HNAVITEM root)
{
	HWND eventRelay;
	DeviceEventCallbacks callbacks;

	if (0 != deviceHandler)
		return FALSE;

	eventRelay = Plugin_GetEventRelayWindow();
	if (NULL == eventRelay)
		return FALSE;

	ZeroMemory(&callbacks, sizeof(callbacks));
	callbacks.deviceCb = Navigation_DeviceCb;

	deviceHandler = EVENTRELAY_REGISTER_HANDLER(eventRelay, &callbacks, root); 
	return (0 != deviceHandler);
}

BOOL 
Navigation_Initialize(void)
{
	HWND libraryWindow;
	wchar_t buffer[128] = {0};

	if (NULL != navigationRoot)
		return FALSE;

	libraryWindow = Plugin_GetLibraryWindow();

	MLNavCtrl_BeginUpdate(libraryWindow, NUF_LOCK_TOP);

	WASABI_API_LNGSTRINGW_BUF(IDS_DEVICES_NAVIGATION_NODE, buffer, ARRAYSIZE(buffer));

	navigationRoot = NavigationItem_Insert(NULL, NULL, 
						NIMF_ITEMID | NIMF_TEXT | NIMF_STYLE | NIMF_TEXTINVARIANT | 
						NIMF_PARAM | NIMF_IMAGE | NIMF_IMAGESEL,
						ML_TREEVIEW_ID_DEVICES,
						DEVICES_NAVITEM_PREFIX L"root",
						buffer,
						NIS_HASCHILDREN | NIS_ALLOWCHILDMOVE | NIS_DEFAULTIMAGE,
						-1, 
						-1L);

	if (NULL == navigationRoot)
		return FALSE;

	Navigation_AddExistingDevices(navigationRoot);
	Navigation_RegisterDeviceHandler(navigationRoot);

	MLNavCtrl_EndUpdate(libraryWindow);

	return TRUE;
}

void 
Navigation_Uninitialize(void)
{
	if (0 != deviceHandler)
	{
		HWND eventRelay;
		eventRelay = Plugin_GetEventRelayWindow();
		if (NULL != eventRelay)
		{
			EVENTRELAY_UNREGISTER_HANDLER(eventRelay, deviceHandler);
		}
		deviceHandler = NULL;
	}

	if (NULL != navigationRoot)
	{
		NavigationItem_Delete(navigationRoot);
		navigationRoot = FALSE;
	}

	NavigationIcons_ClearCache();
}

BOOL 
Navigation_SelectDevice(const char *name)
{
	HNAVITEM item;

	item = NavigationItem_FindFromDeviceName(navigationRoot, name);
	if (NULL == item)
		return FALSE;

	return MLNavItem_Select(Plugin_GetLibraryWindow(), item);
}

BOOL 
Navigation_EditDeviceTitle(const char *name)
{
	HNAVITEM item;

	item = NavigationItem_FindFromDeviceName(navigationRoot, name);
	if (NULL == item)
		return FALSE;

	return MLNavItem_EditTitle(Plugin_GetLibraryWindow(), item);
}

static void
Navigation_DestroyCb()
{
}

static BOOL
NavigationItem_IsMine(HNAVITEM item, ifc_device **device)
{
	NAVITEM info;
	wchar_t buffer[128] = {0};
	INT nameLength;
	INT prefixLength;

	if (NULL == item)
		return FALSE;

	if (item == navigationRoot)
	{
		if (NULL  != device)
			*device = NULL;
		return TRUE;
	}

	info.cbSize = sizeof(NAVITEM);
	info.hItem = item;
	info.mask = NIMF_TEXTINVARIANT;
	info.cchInvariantMax = ARRAYSIZE(buffer);
	info.pszInvariant = buffer;

	if (FALSE == MLNavItem_GetInfo(Plugin_GetLibraryWindow(), &info))
		return FALSE;

	// to maintain some specific orders, we need to alter the insert position
	BOOL swappped = FALSE;
	if (!wcsnicmp(L"cloud_sources", info.pszInvariant, 13))
	{
		// and this will allow us to make the cloud library
		// root do a web page or a sources view as needed...
		lstrcpynW(info.pszInvariant, L"ml_devices_all_sources", ARRAYSIZE(buffer));
		swappped = TRUE;
	}

	nameLength = (NULL != info.pszInvariant) ? lstrlen(info.pszInvariant) : 0;
	prefixLength = ARRAYSIZE(DEVICES_NAVITEM_PREFIX) - 1;

	if (nameLength <= prefixLength)
		return FALSE;

	if (CSTR_EQUAL != CompareString(CSTR_INVARIANT, 0, 
						DEVICES_NAVITEM_PREFIX, prefixLength, info.pszInvariant, prefixLength))
	{
		return FALSE;
	}

	if (NULL != device)
	{
		char name[ARRAYSIZE(buffer)] = {0};
		nameLength = WideCharToMultiByte(CP_UTF8, 0, 
							info.pszInvariant + prefixLength, nameLength - prefixLength, 
							name, ARRAYSIZE(name), NULL, NULL);
		name[nameLength] = '\0';

		if (0 == nameLength ||
			NULL == WASABI_API_DEVICES ||
			S_OK != WASABI_API_DEVICES->DeviceFind(name, device))
		{
			*device = NULL;
			if (swappped) return FALSE;
		}
	}
	return TRUE;
}

static HWND
Navigation_CreateViewErrorWidget(HWND hostWindow, void *user)
{
	return InfoWidget_CreateWindow(WIDGET_TYPE_VIEW_ERROR, 
								   MAKEINTRESOURCE(IDS_INFOWIDGET_TITLE),
								   MAKEINTRESOURCE(IDS_CREATE_DEVICE_VIEW_FAILED),
								   NULL,
								   hostWindow, 0, 0, 0, 0, FALSE, 0);
}

static HWND
NavigationItem_CreateViewCb(HNAVITEM item, HWND parentWindow)
{
	HWND hwnd;
	ifc_device *device;

	if (NULL == item ||
		FALSE == NavigationItem_IsMine(item, &device))
		return NULL;

	if (NULL != device)
	{
		hwnd = device->CreateView(parentWindow);
		device->Release();

		if (NULL == hwnd)
		{
			hwnd = WidgetHost_Create(0, 0, 0, 0, 0, parentWindow, Navigation_CreateViewErrorWidget, NULL);
		}
	}
	else
	{
		hwnd = ManagerView_CreateWindow(parentWindow);
	}
	return hwnd;
}

static BOOL
NavigationItem_ShowContextMenuCb(HNAVITEM item, HWND hostWindow, POINTS pts)
{
	POINT pt;
	ifc_device *device;
		
	if (NULL == item ||
		FALSE == NavigationItem_IsMine(item, &device))
		return FALSE;

	POINTSTOPOINT(pt, pts);
	
	if (item != navigationRoot)
	{
		if (NULL != device)
		{
			char *deviceName = AnsiString_Duplicate(device->GetName());
			device->Release();
			device = NULL;

			if (NULL != deviceName)
			{
				NavigationItem_DisplayDeviceMenu(item, deviceName, hostWindow, pt);
				AnsiString_Free(deviceName);
			}
		}
	}
	else
	{
		NavigationItem_DisplayRootMenu(item, hostWindow, pt);
	}

	if (NULL != device)
		device->Release();

	return TRUE;
}

static BOOL
NavigationItem_ShowHelpCb(HNAVITEM item, HWND hostWindow, POINTS pts)
{		
	if (NULL == item ||
		FALSE == NavigationItem_IsMine(item, NULL))
	{
		return FALSE;
	}

	Plugin_ShowHelp();
	return TRUE;
}

static BOOL
NavigationItem_DeleteCb(HNAVITEM item)
{
	ifc_device *device;

	if (NULL == item ||
		FALSE == NavigationItem_IsMine(item, &device))
		return FALSE;

	if (NULL != device)
	{
		device->SetNavigationItem(NULL);
		device->Release();
	}

	if (item != navigationRoot)
	{
		NAVITEM itemInfo;
		HWND libraryWindow;

		libraryWindow = Plugin_GetLibraryWindow();

		itemInfo.cbSize = sizeof(itemInfo);
		itemInfo.mask = NIMF_IMAGE;
		itemInfo.hItem = item;

		if (FALSE != MLNavItem_GetInfo(libraryWindow, &itemInfo) && 
			-1 != itemInfo.iImage)
		{
			NavigationIcons_ReleaseIconIndex(itemInfo.iImage);
		}
	}

	return TRUE;
}

static BOOL
NavigationItem_KeyDownCb(HNAVITEM item, NMTVKEYDOWN *keyData)
{
	ifc_device *device;

	if (NULL == keyData ||
		NULL == item ||
		FALSE == NavigationItem_IsMine(item, &device))
	{
		return FALSE;
	}

	if (NULL == device)
		return TRUE;

	switch(keyData->wVKey)
	{
		case VK_F2:
			MLNavItem_EditTitle(Plugin_GetLibraryWindow(), item);
			break;
	}
	
	device->Release();
	return TRUE;
}

static int
NavigationItem_DropTargetCb(HNAVITEM item, unsigned int dataType, void *data)
{
	ifc_device *device;
	int result;

	if (NULL == item ||
		FALSE == NavigationItem_IsMine(item, &device))
	{
		return 0;
	}
		
	if (NULL == device)
		return -1;

	result = -1;
	
	if (NULL == data)
	{
		if (S_OK == device->GetDropSupported(dataType))
			result = 1;
	}
	else
	{
		if (SUCCEEDED(device->Drop(data, dataType)))
			result = 1;
	}
	
	device->Release();

	return result;
}

static BOOL
NavigationItem_TitleEditBeginCb(HNAVITEM item)
{ // return TRUE to cancel ediging (only on own items!!!);
	 
	ifc_device *device;
	
	BOOL blockEditor;

	if (NULL == item ||
		FALSE == NavigationItem_IsMine(item, &device))
	{
		return FALSE;
	}
		
	if (NULL == device)
		return TRUE;
		
	blockEditor = (FALSE == DeviceCommand_GetEnabled(device, "rename", 
									DeviceCommandContext_NavigationMenu));

	device->Release();

	return blockEditor;
}


static BOOL
NavigationItem_TitleEditEndCb(HNAVITEM item, const wchar_t *title)
{
	HRESULT hr;
	ifc_device *device;
	
	if (NULL == title)
		return FALSE;

	if (NULL == item ||
		FALSE == NavigationItem_IsMine(item, &device) || 
		NULL == device)
	{
		return FALSE;
	}

	hr = device->SetDisplayName(title);
	device->Release();

	if (FAILED(hr))
	{
		HWND libraryWindow;
		wchar_t title[256] = {0}, message[1024] = {0};

		libraryWindow = Plugin_GetLibraryWindow();

		WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGEBOX_TITLE, title, ARRAYSIZE(title));
		WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGE_UNABLE_TO_RENAME, message, ARRAYSIZE(message));
		
		MessageBox(libraryWindow, message, title, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	return FALSE;
}


BOOL
Navigation_ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result)
{
	if (msg == ML_MSG_NO_CONFIG)
	{
		*result = TRUE;
		return TRUE;
	}

	if (msg < ML_MSG_TREE_BEGIN || msg > ML_MSG_TREE_END)
		return FALSE;

	HNAVITEM item;
	switch(msg)
	{
		case ML_MSG_TREE_ONCREATEVIEW: 
			item = NavigationItem_GetFromMessage(msg, param1);
			*result = (INT_PTR)NavigationItem_CreateViewCb(item, (HWND)param2);
			return TRUE;

		case ML_MSG_NAVIGATION_CONTEXTMENU:
			item = NavigationItem_GetFromMessage(msg, param1);
			*result = (INT_PTR)NavigationItem_ShowContextMenuCb(item, (HWND)param2, MAKEPOINTS(param3));
			return TRUE;

		case ML_MSG_NAVIGATION_HELP:
			item = NavigationItem_GetFromMessage(msg, param1);
			*result = (INT_PTR)NavigationItem_ShowHelpCb(item, (HWND)param2, MAKEPOINTS(param3));
			return TRUE;

		case ML_MSG_NAVIGATION_ONDELETE:
			item = NavigationItem_GetFromMessage(msg, param1);
			*result = (INT_PTR)NavigationItem_DeleteCb(item);
			return TRUE;

		case ML_MSG_TREE_ONKEYDOWN:
			item = NavigationItem_GetFromMessage(msg, param1);
			*result = (INT_PTR)NavigationItem_KeyDownCb(item, (NMTVKEYDOWN*)param2);
			return TRUE;

		case ML_MSG_TREE_ONDROPTARGET:
			item = NavigationItem_GetFromMessage(msg, param1);
			*result = (INT_PTR)NavigationItem_DropTargetCb(item, (unsigned int)param2, (void*)param3);
			return TRUE;

		case ML_MSG_NAVIGATION_ONBEGINTITLEEDIT:
			item = NavigationItem_GetFromMessage(msg, param1);
			*result = (INT_PTR)NavigationItem_TitleEditBeginCb(item);
			return TRUE;

		case ML_MSG_NAVIGATION_ONENDTITLEEDIT:
			item = NavigationItem_GetFromMessage(msg, param1);
			*result = (INT_PTR)NavigationItem_TitleEditEndCb(item, (const wchar_t*)param2);
			return TRUE;

		case ML_MSG_NAVIGATION_ONDESTROY:
			Navigation_DestroyCb();
			*result = 0L;
			return TRUE;
		
	}
	return FALSE;
}