#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_TEST_SUITE_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_TEST_SUITE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <vector>
#include "./device.h"

class TestSuite
{
public:
	TestSuite();
	~TestSuite();

public:
	BOOL AddDevice(Device *device);
	size_t GetDeviceCount();
	Device *GetDevice(size_t index);
	Device *GetRandomDevice();
	Device *CreateDeviceCopy(Device *source);
	Device *GetDeviceByName(const char *name);
	BOOL RegisterDevices(api_devicemanager *manager);
	BOOL UnregisterDevices(api_devicemanager *manager);
		
	BOOL AddType(ifc_devicetype *type);
	size_t GetTypeCount();
	ifc_devicetype *GetType(size_t index);
	BOOL RegisterTypes(api_devicemanager *manager);
	BOOL UnregisterTypes(api_devicemanager *manager);

	BOOL AddConnection(ifc_deviceconnection *connection);
	size_t GetConnectionCount();
	ifc_deviceconnection *GetConnection(size_t index);
	BOOL RegisterConnections(api_devicemanager *manager);
	BOOL UnregisterConnections(api_devicemanager *manager);

	BOOL AddCommand(ifc_devicecommand *command);
	size_t GetCommandCount();
	ifc_devicecommand *GetCommand(size_t index);
	BOOL RegisterCommands(api_devicemanager *manager);
	BOOL UnregisterCommands(api_devicemanager *manager);

	BOOL SetIconBase(const wchar_t *path);
	BOOL SetConnectList(char **devices, size_t count);

private:
	typedef std::vector<Device*> DeviceList;
	typedef std::vector<ifc_devicetype*> TypeList;
	typedef std::vector<ifc_deviceconnection*> ConnectionList;
	typedef std::vector<ifc_devicecommand*> CommandList;

	typedef std::vector<char*> NameList;

private:
	DeviceList deviceList;
	TypeList typeList;
	ConnectionList connectionList;
	CommandList commandList;
	NameList insertList;
	

};

#endif // _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_TEST_SUITE_HEADER