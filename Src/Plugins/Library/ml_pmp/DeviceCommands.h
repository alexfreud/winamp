#pragma once
#include "../nu/refcount.h"
#include "../devices/ifc_devicecommand.h"
#include "../devices/ifc_devicesupportedcommand.h"
#include "../devices/ifc_devicesupportedcommandenum.h"
class PortableCommand : public ifc_devicecommand
{
public:
	PortableCommand(const char *name, int title, int description);
	const char *name;
	int title;
	int description;

	const char *GetName();
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);

	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);

	HRESULT GetDescription(wchar_t *buffer, size_t bufferSize);
RECVS_DISPATCH;
};

typedef struct DeviceCommandInfo
{
	const char *name;
	DeviceCommandFlags flags;
} DeviceCommandInfo;

BOOL SetDeviceCommandInfo(DeviceCommandInfo *info, const char *name, DeviceCommandFlags flags);

class DeviceCommand :  public Countable<ifc_devicesupportedcommand>
{
public:
	DeviceCommand(const char *name, DeviceCommandFlags flags);
	DeviceCommand(const DeviceCommandInfo *commandInfo);

public:
	const char *GetName();
	HRESULT GetFlags(DeviceCommandFlags *flags);
	REFERENCE_COUNT_IMPLEMENTATION;

public:
	const char *name;
	DeviceCommandFlags flags;
RECVS_DISPATCH;
};

class DeviceCommandEnumerator : public Countable<ifc_devicesupportedcommandenum>
{
public:
	DeviceCommandEnumerator(const DeviceCommandInfo *commandInfoList, size_t listSize);
	~DeviceCommandEnumerator();

	HRESULT Next(ifc_devicesupportedcommand **buffer, size_t bufferMax, size_t *count);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);
	REFERENCE_COUNT_IMPLEMENTATION;

private:
	size_t position;
	DeviceCommand **commands;
	size_t count;
	RECVS_DISPATCH;
};
