#ifndef _NULLSOFT_WINAMP_ML_DEVICES_EVENT_RELAY_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_EVENT_RELAY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef enum DeviceEvent
{
	Event_DeviceAdded = 1,
	Event_DeviceRemoved = 2,
	Event_DeviceIconChanged = 3,
	Event_DeviceDisplayNameChanged = 4,
	Event_DeviceAttached = 5,
	Event_DeviceDetached = 6,
	Event_DeviceHidden = 7,
	Event_DeviceShown = 8,
	Event_DeviceTotalSpaceChanged = 9,
	Event_DeviceUsedSpaceChanged = 10,
	Event_DeviceCommandChanged = 11,
	Event_DeviceActivityStarted = 12,
	Event_DeviceActivityFinished = 13,
	Event_DeviceActivityChanged = 14,
	Event_DeviceModelChanged = 15,
	Event_DeviceStatusChanged = 16,
} DeviceEvent;

typedef enum DeviceTypeEvent
{
	Event_TypeRegistered = 1,
	Event_TypeUnregistered = 2,
} DeviceTypeEvent;

typedef enum DeviceConnectionEvent
{
	Event_ConnectionRegistered = 1,
	Event_ConnectionUnregistered = 2,
} DeviceConnectionEvent;

typedef enum DeviceCommandEvent
{
	Event_CommandRegistered = 1,
	Event_CommandUnregistered = 2,
} DeviceCommandEvent;

typedef enum DeviceDiscoveryEvent
{
	Event_DiscoveryStarted = 1,
	Event_DiscoveryFinished = 2,
} DeviceDiscoveryEvent;

typedef void (*DeviceEventCb)(ifc_device *device, DeviceEvent eventId, void *user);
typedef void (*DeviceTypeEventCb)(ifc_devicetype *type, DeviceTypeEvent eventId, void *user);
typedef void (*DeviceConnectionEventCb)(ifc_deviceconnection *connection, DeviceConnectionEvent eventId, void *user);
typedef void (*DeviceCommandEventCb)(ifc_devicecommand *command, DeviceCommandEvent eventId, void *user);
typedef void (*DeviceDiscoveryEventCb)(api_devicemanager *manager, DeviceDiscoveryEvent eventId, void *user);

typedef struct DeviceEventCallbacks
{
	DeviceEventCb deviceCb;
	DeviceTypeEventCb typeCb;
	DeviceConnectionEventCb connectionCb;
	DeviceCommandEventCb commandCb;
	DeviceDiscoveryEventCb discoveryCb;
} DeviceEventCallbacks;

HWND 
EventRelay_CreateWindow();


#define EVENTRELAY_WM_FIRST					(WM_USER + 10)

#define EVENTRELAY_WM_REGISTER_HANDLER		(EVENTRELAY_WM_FIRST + 0)
#define EVENTRELAY_REGISTER_HANDLER(/*HWND*/ _hwnd, /*DeviceEventCallbacks*/ _handler, /*void* */ _user)\
		((size_t)SendMessageW((_hwnd), EVENTRELAY_WM_REGISTER_HANDLER, (WPARAM)(_user), (LPARAM)(_handler)))

#define EVENTRELAY_WM_UNREGISTER_HANDLER	(EVENTRELAY_WM_FIRST + 1)
#define EVENTRELAY_UNREGISTER_HANDLER(/*HWND*/ _hwnd, /*size_t*/ _handlerCookie)\
		((BOOL)SendMessageW((_hwnd), EVENTRELAY_WM_UNREGISTER_HANDLER, 0, (LPARAM)(_handlerCookie)))
							  

#define EVENTRELAY_WM_NOTIFY_DEVICE			(EVENTRELAY_WM_FIRST + 2)
#define EVENTRELAY_NOTIFY_DEVICE(/*HWND*/ _hwnd, /*ifc_device* */ _device, /*DeviceEvent*/ _eventId)\
		{ ifc_device *_d = (_device); if (NULL != _d && NULL != (_hwnd)) { _d->AddRef(); \
			if (FALSE == ((BOOL)PostMessageW((_hwnd), EVENTRELAY_WM_NOTIFY_DEVICE,\
			(WPARAM)(_eventId), (LPARAM)(_d)))) { _d->Release(); }}}

#define EVENTRELAY_WM_NOTIFY_DISCOVERY		(EVENTRELAY_WM_FIRST + 3)
#define EVENTRELAY_NOTIFY_DISCOVERY(/*HWND*/ _hwnd, /*api_devicemanager* */ _manager, /*DeviceDiscoveryEvent*/ _eventId)\
		{ api_devicemanager *_m = (_manager); if (NULL != _m && NULL != (_hwnd)) { _m->AddRef(); \
			if (FALSE == ((BOOL)PostMessageW((_hwnd), EVENTRELAY_WM_NOTIFY_DISCOVERY,\
			(WPARAM)(_eventId), (LPARAM)(_m)))) { _m->Release(); }}}

#define EVENTRELAY_WM_NOTIFY_TYPE			(EVENTRELAY_WM_FIRST + 4)
#define EVENTRELAY_NOTIFY_TYPE(/*HWND*/ _hwnd, /*ifc_devicetype* */ _type, /*DeviceTypeEvent*/ _eventId)\
		{ ifc_devicetype *_t = (_type); if (NULL != _t && NULL != (_hwnd)) { _t->AddRef(); \
			if (FALSE == ((BOOL)PostMessageW((_hwnd), EVENTRELAY_WM_NOTIFY_TYPE,\
			(WPARAM)(_eventId), (LPARAM)(_t)))) { _t->Release(); }}}

#define EVENTRELAY_WM_NOTIFY_CONNECTION		(EVENTRELAY_WM_FIRST + 5)
#define EVENTRELAY_NOTIFY_CONNECTION(/*HWND*/ _hwnd, /*ifc_deviceconnection* */ _connection, /*DeviceConnectionEvent*/ _eventId)\
		{ ifc_deviceconnection *_c = (_connection); if (NULL != _c && NULL != (_hwnd)) { _c->AddRef(); \
			if (FALSE == ((BOOL)PostMessageW((_hwnd), EVENTRELAY_WM_NOTIFY_CONNECTION,\
			(WPARAM)(_eventId), (LPARAM)(_c)))) { _c->Release(); }}}

#define EVENTRELAY_WM_NOTIFY_COMMAND		(EVENTRELAY_WM_FIRST + 6)
#define EVENTRELAY_NOTIFY_COMMAND(/*HWND*/ _hwnd, /*ifc_devicecommand* */ _command, /*DeviceCommandEvent*/ _eventId)\
		{ ifc_devicecommand *_c = (_command); if (NULL != _c && NULL != (_hwnd)) { _c->AddRef(); \
			if (FALSE == ((BOOL)PostMessageW((_hwnd), EVENTRELAY_WM_NOTIFY_COMMAND,\
			(WPARAM)(_eventId), (LPARAM)(_c)))) { _c->Release(); }}}

#define EVENTRELAY_WM_LAST					EVENTRELAY_WM_NOTIFY_COMMAND

#endif //_NULLSOFT_WINAMP_ML_DEVICES_EVENT_RELAY_HEADER