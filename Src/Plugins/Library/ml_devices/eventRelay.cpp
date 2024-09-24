#include "main.h"
#include "./eventRelay.h"
#include <vector>

#define EVENT_RELAY_WINDOW_CLASS		L"NullsoftEventRelay"

typedef struct EventHandler
{
	size_t cookie;
	DeviceEventCallbacks callbacks;
	void *user;
} EventHandler;

typedef std::vector<EventHandler*> EventHandlerList;

typedef struct EventRelay
{
	EventHandlerList		handlerList;
	DeviceManagerHandler	*managerHandler;
	DeviceHandler			*deviceHandler;
} EventRelay;

#define EVENTRELAY(_hwnd) ((EventRelay*)(LONGX86)GetWindowLongPtrW((_hwnd), 0))
#define EVENTRELAY_RET_VOID(_self, _hwnd) {(_self) = EVENTRELAY((_hwnd)); if (NULL == (_self)) return;}
#define EVENTRELAY_RET_VAL(_self, _hwnd, _error) {(_self) = EVENTRELAY((_hwnd)); if (NULL == (_self)) return (_error);}


static LRESULT CALLBACK 
EventRelay_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);


static ATOM 
EventRelay_GetClassAtom(HINSTANCE instance)
{
	WNDCLASSEXW klass;
	ATOM klassAtom;

	klassAtom = (ATOM)GetClassInfoExW(instance, EVENT_RELAY_WINDOW_CLASS, &klass);
	if (0 != klassAtom)
		return klassAtom;

	memset(&klass, 0, sizeof(klass));
	klass.cbSize = sizeof(klass);
	klass.style = 0;
	klass.lpfnWndProc = EventRelay_WindowProc;
	klass.cbClsExtra = 0;
	klass.cbWndExtra = sizeof(EventRelay*);
	klass.hInstance = instance;
	klass.hIcon = NULL;
	klass.hCursor = NULL;
	klass.hbrBackground = NULL;
	klass.lpszMenuName = NULL;
	klass.lpszClassName = EVENT_RELAY_WINDOW_CLASS;
	klass.hIconSm = NULL;
	klassAtom = RegisterClassExW(&klass);
	
	return klassAtom;
}

HWND
EventRelay_CreateWindow()
{
	HINSTANCE instance;
	ATOM klassAtom;
	HWND hwnd;

	instance = GetModuleHandleW(NULL);
	klassAtom = EventRelay_GetClassAtom(instance);
	if (0 == klassAtom)
		return NULL;

	hwnd = CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_NOPARENTNOTIFY, 
							MAKEINTATOM(klassAtom), 
							NULL, 
							WS_OVERLAPPED,
							0, 0, 0, 0,
							HWND_MESSAGE, 
							NULL, 
							instance, 
							NULL);

	return hwnd;
}


static size_t
EventRelay_GenerateCookie(EventRelay *self)
{
	size_t cookie;
	EventHandler *handler;


	if (NULL == self)
		return 0;

	cookie = self->handlerList.size() + 1;

	for(;;)
	{
		size_t index = self->handlerList.size();
		while(index--)
		{
			handler = self->handlerList[index];
			if (cookie == handler->cookie)
			{
				cookie++;
				break;
			}
		}

		if (((size_t)-1) == index)
			return cookie;
	}

	return cookie;
}

static EventHandler *
EventRelay_CreateEventHandler(EventRelay *self, DeviceEventCallbacks *callbacks, void *user)
{
	EventHandler *handler;
	size_t cookie;

	if (NULL == self || NULL == callbacks)
		return NULL;

	cookie = EventRelay_GenerateCookie(self);
	if (0 == cookie)
		return NULL;

	handler = (EventHandler*)malloc(sizeof(EventHandler));
	if (NULL == handler)
		return NULL;

	handler->user = user;
	handler->cookie  = cookie;
	handler->callbacks.deviceCb = callbacks->deviceCb;
	handler->callbacks.typeCb = callbacks->typeCb;
	handler->callbacks.connectionCb = callbacks->connectionCb;
	handler->callbacks.commandCb = callbacks->commandCb;
	handler->callbacks.discoveryCb = callbacks->discoveryCb;
	
	return handler;
}

static void
EventRelay_DestroyEventHandler(EventHandler *handler)
{
	if (NULL == handler)
		return;

	free(handler);
}

static LRESULT
EventRelay_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{	
	EventRelay *self;
	ifc_deviceobjectenum *enumerator;
	ifc_deviceobject *object;
	ifc_device *device;
	
	if (NULL == WASABI_API_DEVICES)
		return -1;

	self = new EventRelay();
	if (NULL == self)
		return -1;
	
	self->deviceHandler = NULL;
	self->managerHandler = NULL;

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)self) && ERROR_SUCCESS != GetLastError())
		return -1;
	
	if (FAILED(DeviceHandler::CreateInstance(&self->deviceHandler)))
		return -1;

	self->deviceHandler->SetRelayWindow(hwnd);

	if (SUCCEEDED(WASABI_API_DEVICES->DeviceEnumerate(&enumerator)))
	{
		while(S_OK == enumerator->Next(&object, 1, NULL))
		{
			if (SUCCEEDED(object->QueryInterface(IFC_Device, (void**)&device)))
			{
				self->deviceHandler->Advise(device);
				device->Release();
			}
			object->Release();
		}
		enumerator->Release();
	}

	if (FAILED(DeviceManagerHandler::CreateInstance(&self->managerHandler)))
		return -1;

	self->managerHandler->SetRelayWindow(hwnd);
	if (FAILED(self->managerHandler->Advise(WASABI_API_DEVICES)))
		return -1;
		
	return 0;
}

static void
EventRelay_OnDestroy(HWND hwnd)
{
	EventRelay *self;
	MSG msg;

	self = EVENTRELAY(hwnd);
	SetWindowLongPtr(hwnd, 0, 0);

	if (NULL == self)
		return;

	size_t index = self->handlerList.size();
	while(index--)
	{
		EventHandler *handler = self->handlerList[index];
		EventRelay_DestroyEventHandler(handler);
	}

	if (NULL != self->managerHandler)
	{
		self->managerHandler->SetRelayWindow(NULL);

		if (NULL != WASABI_API_DEVICES)
			self->managerHandler->Unadvise(WASABI_API_DEVICES);

		self->managerHandler->Release();
	}

	if (NULL != self->deviceHandler)
	{
		self->deviceHandler->SetRelayWindow(NULL);

		if (NULL != WASABI_API_DEVICES)
		{
			ifc_deviceobjectenum *enumerator;
			ifc_deviceobject *object;
			ifc_device *device;

			if (SUCCEEDED(WASABI_API_DEVICES->DeviceEnumerate(&enumerator)))
			{
				while(S_OK == enumerator->Next(&object, 1, NULL))
				{
					if (SUCCEEDED(object->QueryInterface(IFC_Device, (void**)&device)))
					{
						self->deviceHandler->Unadvise(device);
						device->Release();
					}
					object->Release();
				}
				enumerator->Release();
			}
		}

		self->deviceHandler->Release();
	}

	delete self;

	// finish pumping messages
	while(FALSE != PeekMessage(&msg, hwnd, EVENTRELAY_WM_FIRST, EVENTRELAY_WM_LAST, PM_REMOVE))
	{
		EventRelay_WindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}
}

static LRESULT
EventRelay_OnRegisterHandler(HWND hwnd, DeviceEventCallbacks *callbacks, void *user)
{
	EventRelay *self;
	EventHandler *handler;

	EVENTRELAY_RET_VAL(self, hwnd, 0);

	handler = EventRelay_CreateEventHandler(self, callbacks, user);
	if(NULL == handler) 
		return 0;

	self->handlerList.push_back(handler);
	return (LRESULT)handler->cookie;
}

static LRESULT
EventRelay_OnUnregisterHandler(HWND hwnd, size_t cookie)
{
	EventRelay *self;

	EVENTRELAY_RET_VAL(self, hwnd, FALSE);

	size_t index = self->handlerList.size();
	while(index--)
	{
		EventHandler *handler = self->handlerList[index];
		if (handler->cookie == cookie)
		{
			self->handlerList.erase(self->handlerList.begin() + index);
			EventRelay_DestroyEventHandler(handler);
			return TRUE;
		}
	}

	return FALSE;
}

static void
EventRelay_OnNotifyDevice(HWND hwnd, ifc_device *device, DeviceEvent eventId)
{
	ReplyMessage(0);
	
	if (NULL != device)
	{
		EventRelay *self;
		self = EVENTRELAY(hwnd);
		
		if (NULL != self)
		{
			switch(eventId)
			{
				case Event_DeviceAdded:
					if (NULL != self->deviceHandler)
						self->deviceHandler->Advise(device);
					break;

				case Event_DeviceRemoved:
					if (NULL != self->deviceHandler)
						self->deviceHandler->Unadvise(device);
					break;
			}

			size_t index = self->handlerList.size();
			while(index--)
			{
				EventHandler *handler = self->handlerList[index];
				if (NULL != handler->callbacks.deviceCb)
					handler->callbacks.deviceCb(device, eventId, handler->user);
			}
		}

		device->Release();
	}
}

static void
EventRelay_OnNotifyDiscovery(HWND hwnd, api_devicemanager *manager, DeviceDiscoveryEvent eventId)
{
	ReplyMessage(0);
	
	if (NULL != manager)
	{
		EventRelay *self;
		self = EVENTRELAY(hwnd);
		
		if (NULL != self)
		{
			size_t index = self->handlerList.size();
			while(index--)
			{
				EventHandler *handler = self->handlerList[index];
				if (NULL != handler->callbacks.discoveryCb)
					handler->callbacks.discoveryCb(manager, eventId, handler->user);
			}
		}

		manager->Release();
	}
}

static void
EventRelay_OnNotifyType(HWND hwnd, ifc_devicetype *type, DeviceTypeEvent eventId)
{
	ReplyMessage(0);
	
	if (NULL != type)
	{
		EventRelay *self;
		self = EVENTRELAY(hwnd);
		
		if (NULL != self)
		{
			size_t index = self->handlerList.size();
			while(index--)
			{
				EventHandler *handler = self->handlerList[index];
				if (NULL != handler->callbacks.typeCb)
					handler->callbacks.typeCb(type, eventId, handler->user);
			}
		}

		type->Release();
	}
}

static void
EventRelay_OnNotifyConnection(HWND hwnd, ifc_deviceconnection *connection, DeviceConnectionEvent eventId)
{
	ReplyMessage(0);
	
	if (NULL != connection)
	{
		EventRelay *self;
		self = EVENTRELAY(hwnd);
		
		if (NULL != self)
		{
			size_t index = self->handlerList.size();
			while(index--)
			{
				EventHandler *handler = self->handlerList[index];
				if (NULL != handler->callbacks.connectionCb)
					handler->callbacks.connectionCb(connection, eventId, handler->user);
			}
		}

		connection->Release();
	}
}

static void
EventRelay_OnNotifyCommand(HWND hwnd, ifc_devicecommand *command, DeviceCommandEvent eventId)
{
	ReplyMessage(0);
	
	if (NULL != command)
	{
		EventRelay *self;
		self = EVENTRELAY(hwnd);
		
		if (NULL != self)
		{
			size_t index = self->handlerList.size();
			while(index--)
			{
				EventHandler *handler = self->handlerList[index];
				if (NULL != handler->callbacks.commandCb)
					handler->callbacks.commandCb(command, eventId, handler->user);
			}
		}

		command->Release();
	}
}

static LRESULT CALLBACK 
EventRelay_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:		return EventRelay_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:	EventRelay_OnDestroy(hwnd); return 0;

		case EVENTRELAY_WM_REGISTER_HANDLER: return EventRelay_OnRegisterHandler(hwnd, (DeviceEventCallbacks*)lParam, (void*)wParam);
		case EVENTRELAY_WM_UNREGISTER_HANDLER: return EventRelay_OnUnregisterHandler(hwnd, (size_t)lParam);
		case EVENTRELAY_WM_NOTIFY_DEVICE:		EventRelay_OnNotifyDevice(hwnd, (ifc_device*)lParam, (DeviceEvent)wParam); return 0;
		case EVENTRELAY_WM_NOTIFY_DISCOVERY:	EventRelay_OnNotifyDiscovery(hwnd, (api_devicemanager*)lParam, (DeviceDiscoveryEvent)wParam); return 0;
		case EVENTRELAY_WM_NOTIFY_TYPE:			EventRelay_OnNotifyType(hwnd, (ifc_devicetype*)lParam, (DeviceTypeEvent)wParam); return 0;
		case EVENTRELAY_WM_NOTIFY_CONNECTION:	EventRelay_OnNotifyConnection(hwnd, (ifc_deviceconnection*)lParam, (DeviceConnectionEvent)wParam); return 0;
		case EVENTRELAY_WM_NOTIFY_COMMAND:		EventRelay_OnNotifyCommand(hwnd, (ifc_devicecommand*)lParam, (DeviceCommandEvent)wParam); return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
