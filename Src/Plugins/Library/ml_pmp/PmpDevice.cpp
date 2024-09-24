#include "main.h"
#include "DeviceView.h"
#include "api__ml_pmp.h"
#include "../nu/refcount.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "DeviceCommands.h"
#include "resource1.h"
#include <strsafe.h>

// known commands
static const char *DEVICE_CMD_VIEW_OPEN	 = "view_open";
static const char *DEVICE_CMD_SYNC = "sync";
static const char *DEVICE_CMD_AUTOFILL = "autofill";
static const char *DEVICE_CMD_PLAYLIST_CREATE = "playlist_create";
static const char *DEVICE_CMD_RENAME = "rename";
static const char *DEVICE_CMD_PREFERENCES = "preferences";
static const char *DEVICE_CMD_EJECT = "eject";
static const char *DEVICE_CMD_REMOVE = "remove";
static const char *DEVICE_CMD_TRANSFER = "transfer";
static const char *DEVICE_CMD_HIDE = "hide";

extern void UpdateDevicesListView(bool softUpdate);

// we're going to share the command enum stuff for all devices

class PortableDeviceType : public ifc_devicetype
{
public:
	PortableDeviceType()
	{
	}
	const char *GetName()
	{ 
		return "portable"; 
	}

	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
	{
		return E_NOTIMPL;
	}

	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize)
	{
		if (NULL == buffer)
			return E_POINTER;

		WASABI_API_LNGSTRINGW_BUF(IDS_PORTABLE_DEVICE_TYPE, buffer, bufferSize);
		return S_OK;
	}
protected:

#define CBCLASS PortableDeviceType
	START_DISPATCH_INLINE;
	CB(API_GETNAME, GetName);
	CB(API_GETICON, GetIcon);
	CB(API_GETDISPLAYNAME, GetDisplayName);
	END_DISPATCH;
#undef CBCLASS
};

class USBDeviceConnection : public ifc_deviceconnection
{
public:
		USBDeviceConnection()
	{
	}
	const char *GetName()
	{ 
		return "usb"; 
	}

	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
	{
		if (FALSE == FormatResProtocol(MAKEINTRESOURCE(IDB_USB), 
										L"PNG", buffer, bufferSize))
		{
			return E_FAIL;
		}

		return S_OK;
	}

	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize)
	{
		if (NULL == buffer)
			return E_POINTER;

		WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_CONNECTION_USB, buffer, bufferSize);
		return S_OK;
	}
protected:

#define CBCLASS USBDeviceConnection
	START_DISPATCH_INLINE;
	CB(API_GETNAME, GetName);
	CB(API_GETICON, GetIcon);
	CB(API_GETDISPLAYNAME, GetDisplayName);
	END_DISPATCH;
#undef CBCLASS
};

static PortableDeviceType portable_device_type;
static USBDeviceConnection usb_connection;
static PortableCommand registered_commands[] = 
{
	PortableCommand(DEVICE_CMD_VIEW_OPEN, IDS_DEVICE_CMD_VIEW_OPEN, IDS_DEVICE_CMD_VIEW_OPEN_DESC),
	PortableCommand(DEVICE_CMD_SYNC, IDS_DEVICE_CMD_SYNC, IDS_DEVICE_CMD_SYNC_DESC),
	PortableCommand(DEVICE_CMD_TRANSFER, IDS_DEVICE_CMD_TRANSFER, IDS_DEVICE_CMD_TRANSFER_DESC),
	PortableCommand(DEVICE_CMD_EJECT, IDS_DEVICE_CMD_EJECT, IDS_DEVICE_CMD_EJECT_DESC),
	PortableCommand(DEVICE_CMD_REMOVE, IDS_DEVICE_CMD_REMOVE, IDS_DEVICE_CMD_REMOVE_DESC),
	PortableCommand(DEVICE_CMD_RENAME, IDS_DEVICE_CMD_RENAME, IDS_DEVICE_CMD_RENAME_DESC),
	PortableCommand(DEVICE_CMD_AUTOFILL, IDS_DEVICE_CMD_AUTOFILL, IDS_DEVICE_CMD_AUTOFILL_DESC),
	PortableCommand(DEVICE_CMD_PLAYLIST_CREATE, IDS_DEVICE_CMD_PLAYLIST_CREATE, IDS_DEVICE_CMD_PLAYLIST_CREATE_DESC),
	PortableCommand(DEVICE_CMD_PREFERENCES, IDS_DEVICE_CMD_PREFERENCES, IDS_DEVICE_CMD_PREFERENCES_DESC),
	PortableCommand(DEVICE_CMD_HIDE, IDS_DEVICE_CMD_HIDE, IDS_DEVICE_CMD_HIDE),
};

static ifc_devicecommand * _cdecl
Devices_RegisterCommand(const char *name, void *user)
{
	for(size_t i = 0; i < sizeof(registered_commands)/sizeof(*registered_commands); i++)
	{
		if (name == registered_commands[i].GetName())
		{
			return &registered_commands[i];
		}
	}
	return NULL;
}

void Devices_Init()
{
	if (AGAVE_API_DEVICEMANAGER)
	{
		/* register 'portable' device type */
		ifc_devicetype *type = &portable_device_type;
		AGAVE_API_DEVICEMANAGER->TypeRegister(&type, 1);

		/* register 'usb' connection type */
		ifc_deviceconnection *connection = &usb_connection;
		AGAVE_API_DEVICEMANAGER->ConnectionRegister(&connection, 1);


		/* register commands */
		const char *commands[sizeof(registered_commands)/sizeof(*registered_commands)];
		for(size_t i = 0; i < sizeof(registered_commands)/sizeof(*registered_commands); i++)
		{
			commands[i] = registered_commands[i].GetName();
		}
		AGAVE_API_DEVICEMANAGER->CommandRegisterIndirect(commands, sizeof(registered_commands)/sizeof(*registered_commands), Devices_RegisterCommand, NULL);
	}	
}

int DeviceView::QueryInterface(GUID interface_guid, void **object)
{
	if (interface_guid == IFC_Device)
	{
		AddRef();
		*object = (ifc_device *)this;
		return 0;
	}
	return 1;
}

const char *DeviceView::GetName()
{
	return name;
}

HRESULT DeviceView::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	buffer[0]=0;
	dev->extraActions(DEVICE_GET_ICON, width, height, (intptr_t)buffer);
	if (buffer[0] == 0)
		return E_NOTIMPL;
	else
		return S_OK;

	return E_NOTIMPL;
}

HRESULT DeviceView::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	// TODO sometimes this is erroring on loading
	dev->getPlaylistName(0, buffer, bufferSize);
	return S_OK;
}

const char *DeviceView::GetType()
{
	return "portable";
}

const char *DeviceView::GetDisplayType()
{
	return display_type;
}

const char *DeviceView::GetConnection()
{
	return connection_type;
}

BOOL DeviceView::GetHidden()
{
	return FALSE;
}

HRESULT DeviceView::GetTotalSpace(uint64_t *size)
{
	UpdateSpaceInfo(FALSE, TRUE);
	*size = dev->getDeviceCapacityTotal();
	return S_OK;
}

HRESULT DeviceView::GetUsedSpace(uint64_t *size)
{
	if (NULL == size)
		return E_POINTER;

	UpdateSpaceInfo(TRUE, TRUE);
	*size = usedSpace;

	return S_OK;
}

BOOL DeviceView::GetAttached()
{
	return TRUE; // ml_pmp devices are by default attached
}

HRESULT DeviceView::Attach(HWND hostWindow)
{
	return E_NOTIMPL;
}

HRESULT DeviceView::Detach(HWND hostWindow)
{
	return E_NOTIMPL;
}

HRESULT DeviceView::EnumerateCommands(ifc_devicesupportedcommandenum **enumerator, DeviceCommandContext context)
{
	DeviceCommandInfo commands[32];
	size_t count;

	if (NULL == enumerator)
		return E_POINTER;

	count = 0;

	LinkedQueue * txQueue = getTransferQueue(this);
	if (txQueue == NULL)
		return E_POINTER;

	//	return E_NOTIMPL;
	if (context == DeviceCommandContext_View)
	{
		if (0 == txQueue->GetSize())
		{
			SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_SYNC, DeviceCommandFlag_Primary);
			SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_AUTOFILL, DeviceCommandFlag_None);
			SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_PLAYLIST_CREATE, DeviceCommandFlag_None);
			SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_PREFERENCES, DeviceCommandFlag_None);
			SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_EJECT, DeviceCommandFlag_None);
			SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_REMOVE, DeviceCommandFlag_None);
		}
	}
	else
	{
		SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_VIEW_OPEN, DeviceCommandFlag_Primary);

		DeviceCommandFlags flags = DeviceCommandFlag_None;
		if (0 != txQueue->GetSize())
			flags |= DeviceCommandFlag_Disabled;

		if (0 != dev->extraActions(DEVICE_SYNC_UNSUPPORTED,0,0,0))
			flags |= DeviceCommandFlag_Disabled;
		SetDeviceCommandInfo(&commands[count++], (!isCloudDevice ? DEVICE_CMD_SYNC : DEVICE_CMD_TRANSFER), flags | DeviceCommandFlag_Group);
		if (!isCloudDevice) SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_AUTOFILL, flags);

		flags = DeviceCommandFlag_None;
		if (0 == dev->extraActions(DEVICE_PLAYLISTS_UNSUPPORTED,0,0,0))
		{
			// TODO remove once we've got cloud playlists implemented
			if (!isCloudDevice)
				SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_PLAYLIST_CREATE, flags | DeviceCommandFlag_Group);
		}

		// adds a specific menu item to hide the 'local library' source
		if (isCloudDevice)
		{
			char name[128] = {0};
			if (dev->extraActions(DEVICE_GET_UNIQUE_ID, (intptr_t)name, sizeof(name), 0))
			{
				if (!strcmp(name, "local_desktop"))
				{
					flags = DeviceCommandFlag_None | DeviceCommandFlag_Group;
					SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_HIDE, flags);
				}
			}
		}

		bool has_rename = true;
		flags = DeviceCommandFlag_None;
		if (0 == dev->extraActions(DEVICE_CAN_RENAME_DEVICE,0,0,0))
			has_rename = false;
		else
			SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_RENAME, flags | DeviceCommandFlag_Group);

		flags = (!has_rename ? DeviceCommandFlag_Group : DeviceCommandFlag_None);
		SetDeviceCommandInfo(&commands[count++], DEVICE_CMD_PREFERENCES, flags);
		if (!dev->extraActions(DEVICE_DOES_NOT_SUPPORT_REMOVE,0,0,0))
			SetDeviceCommandInfo(&commands[count++], (!isCloudDevice ? DEVICE_CMD_EJECT : DEVICE_CMD_REMOVE), flags | DeviceCommandFlag_Group);
	}

	*enumerator = new DeviceCommandEnumerator(commands, count);
	if (NULL == *enumerator)
		return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT DeviceView::SendCommand(const char *command, HWND hostWindow, ULONG_PTR param)
{
	if (!strcmp(command, DEVICE_CMD_EJECT) || !strcmp(command, DEVICE_CMD_REMOVE))
	{
		Eject();
		return S_OK;
	}
	else if (!strcmp(command, DEVICE_CMD_SYNC) || !strcmp(command, DEVICE_CMD_TRANSFER))
	{ 
		if (!this->isCloudDevice) Sync();
		else CloudSync();
		return S_OK;
	}
	else if (!strcmp(command, DEVICE_CMD_AUTOFILL))
	{
		Autofill();
		return S_OK;
	}
	else if (!strcmp(command, DEVICE_CMD_RENAME))
	{
		if (NULL != treeItem)
			MLNavItem_EditTitle(plugin.hwndLibraryParent, treeItem);
		else
			RenamePlaylist(0);
		return S_OK;
	}
	else if (!strcmp(command, DEVICE_CMD_PLAYLIST_CREATE))
	{
		CreatePlaylist();
		return S_OK;
	}
	else if (!strcmp(command, DEVICE_CMD_PREFERENCES))
	{
		SENDWAIPC(plugin.hwndWinampParent, IPC_OPENPREFSTOPAGE,(WPARAM)&devPrefsPage);
		return S_OK;
	}
	else if (!strcmp(command, DEVICE_CMD_HIDE))
	{
		static int IPC_CLOUD_HIDE_LOCAL = -1;
		if (IPC_CLOUD_HIDE_LOCAL == -1)
			IPC_CLOUD_HIDE_LOCAL = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloudLocal", IPC_REGISTER_WINAMP_IPCMESSAGE);

		SENDWAIPC(plugin.hwndWinampParent, IPC_CLOUD_HIDE_LOCAL, 0);
		return S_OK;
	}

	return E_NOTIMPL;
}

HRESULT DeviceView::GetCommandFlags(const char *command, DeviceCommandFlags *flags)
{
	return E_NOTIMPL;
}

HRESULT DeviceView::GetActivity(ifc_deviceactivity **activity)
{
	LinkedQueue * txQueue = getTransferQueue();
	if (txQueue == NULL || txQueue->GetSize() == 0)
	{
		*activity = 0;
		return S_FALSE;
	}
	AddRef();
	*activity = this;
	return S_OK;
}

HRESULT DeviceView::Advise(ifc_deviceevent *handler)
{
	event_handlers.push_back(handler);
	return S_OK;
}

HRESULT DeviceView::Unadvise(ifc_deviceevent *handler)
{
	//event_handlers.eraseObject(handler);
	auto it = std::find(event_handlers.begin(), event_handlers.end(), handler);
	if (it != event_handlers.end())
	{
		event_handlers.erase(it);
	}

	return S_OK;
}

extern C_ItemList devices;
extern void UpdateDevicesListView(bool softupdate);
void DeviceView::SetNavigationItem(void *navigationItem)
{
	if (navigationItem)
		RegisterViews((HNAVITEM)navigationItem);
}

BOOL DeviceView::GetActive()
{
	LinkedQueue * txQueue = getTransferQueue();
	if (txQueue == NULL || txQueue->GetSize() == 0)
		return FALSE;
	return TRUE; 
}

BOOL DeviceView::GetCancelable()
{
	return FALSE;
}

HRESULT DeviceView::GetProgress(unsigned int *percentCompleted)
{
	LinkedQueue * txQueue = getTransferQueue();
	LinkedQueue * finishedTX = getFinishedTransferQueue();
	int txProgress = getTransferProgress();
	int size = (txQueue ? txQueue->GetSize() : 0);
	double num = (100.0 * (double)size) - (double)txProgress;
	double total = (double)100 * size + 100 * (finishedTX ? finishedTX->GetSize() : 0);

	double percent = (0 != total) ? (((total - num) * 100) / total) : 0;

	*percentCompleted = (unsigned int)percent;
	return S_OK;
}

HRESULT DeviceView::Activity_GetDisplayName(wchar_t *buffer, size_t bufferMax)
{
	WASABI_API_LNGSTRINGW_BUF(IDS_TRANSFERRING, buffer, bufferMax);
	return S_OK;
}

HRESULT DeviceView::Activity_GetStatus(wchar_t *buffer, size_t bufferMax)
{
	WASABI_API_LNGSTRINGW_BUF(IDS_TRANSFERRING_DESC, buffer, bufferMax);
	return S_OK;
}

HRESULT DeviceView::Cancel(HWND hostWindow)
{
//	threadKillswitch = 1; // TODO: i think this is how to do it
	//transferContext.WaitForKill();
	return S_OK;
}

HRESULT DeviceView::GetDropSupported(unsigned int dataType)
{
	if (dataType == ML_TYPE_ITEMRECORDLISTW 
		|| dataType == ML_TYPE_ITEMRECORDLIST 
		|| dataType == ML_TYPE_PLAYLIST 
		|| dataType == ML_TYPE_PLAYLISTS 
		|| dataType == ML_TYPE_FILENAMES 
		|| dataType == ML_TYPE_FILENAMESW) 
		return S_OK;
	return E_FAIL;
}

HRESULT DeviceView::Drop(void *data, unsigned int dataType)
{
	return (HRESULT)TransferFromML(dataType,data,E_FAIL,S_OK);
}

HRESULT DeviceView::SetDisplayName(const wchar_t *displayName, bool force = 0)
{
	if((0 == force && 0 == dev->extraActions(DEVICE_CAN_RENAME_DEVICE,0,0,0)))
		return E_FAIL;

	dev->setPlaylistName(0, displayName);	
	free(devPrefsPage.name);
	devPrefsPage.name = _wcsdup(displayName);
	SENDWAIPC(plugin.hwndWinampParent, IPC_UPDATE_PREFS_DLGW, (WPARAM)&devPrefsPage);

	DevicePropertiesChanges();
	UpdateDevicesListView(false);

	OnNameChanged(displayName);

	return S_OK;
}

HRESULT DeviceView::GetModel(wchar_t *buffer, size_t bufferSize)
{
	if (NULL == buffer)
		return E_POINTER;

	buffer[0] = L'\0';

	if(0 == dev->extraActions(DEVICE_GET_MODEL, (intptr_t)buffer, bufferSize, 0))
		return E_NOTIMPL;

	return S_OK;
}

HRESULT DeviceView::GetStatus(wchar_t *buffer, size_t bufferSize)
{
	return E_NOTIMPL;
}

#define CBCLASS DeviceView
START_MULTIPATCH;
START_PATCH(PATCH_IFC_DEVICE)
M_CB(PATCH_IFC_DEVICE, ifc_device, QUERYINTERFACE, QueryInterface);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETNAME, GetName);
M_CB(PATCH_IFC_DEVICE, ifc_device, ifc_deviceobject::API_GETDISPLAYNAME, GetDisplayName);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETICON, GetIcon);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETTYPE, GetType);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETDISPLAYTYPE, GetDisplayType);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETCONNECTION, GetConnection);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETTOTALSPACE, GetTotalSpace);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETUSEDSPACE, GetUsedSpace);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_ENUMERATECOMMANDS, EnumerateCommands);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_SENDCOMMAND, SendCommand);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETATTACHED, GetAttached);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_ATTACH, Attach);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_DETACH, Detach);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_ADVISE, Advise);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_UNADVISE, Unadvise);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_CREATEVIEW, CreateView);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETACTIVITY, GetActivity);
M_VCB(PATCH_IFC_DEVICE, ifc_device, API_SETNAVIGATIONITEM, SetNavigationItem);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETDROPSUPPORTED, GetDropSupported);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_DROP, Drop);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_SETDISPLAYNAME, SetDisplayName);
M_CB(PATCH_IFC_DEVICE, ifc_device, API_GETMODEL, GetModel);
M_CB(PATCH_IFC_DEVICE, ifc_device, ifc_device::API_GETSTATUS, GetStatus);
M_CB(PATCH_IFC_DEVICE, ifc_device, ADDREF, AddRef);
M_CB(PATCH_IFC_DEVICE, ifc_device, RELEASE, Release);
NEXT_PATCH(PATCH_IFC_DEVICEACTIVITY)
M_CB(PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity, API_GETACTIVE, GetActive);
M_CB(PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity, API_GETCANCELABLE, GetCancelable);
M_CB(PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity, API_GETPROGRESS, GetProgress);
M_CB(PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity, ifc_deviceactivity::API_GETDISPLAYNAME, Activity_GetDisplayName);
M_CB(PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity, ifc_deviceactivity::API_GETSTATUS, Activity_GetStatus);
M_CB(PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity, API_CANCEL, Cancel);
M_CB(PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity, ADDREF, AddRef);
M_CB(PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity, RELEASE, Release);
END_PATCH
END_MULTIPATCH;
#undef CBCLASS