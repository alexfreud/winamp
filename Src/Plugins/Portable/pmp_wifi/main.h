#pragma once
#include "../../Library/ml_pmp/pmp.h"
#include "../Winamp/wa_ipc.h"
#include <bfc/platform/types.h>
#include "modelInfo.h"
extern PMPDevicePlugin plugin;
extern int winampVersion;
extern GUID winamp_id;
extern char winamp_id_str[40];
extern char winamp_name[260];
extern wchar_t inifile[MAX_PATH];
void StopListenServer();
BOOL FormatResProtocol(const wchar_t *resourceName, const wchar_t *resourceType, wchar_t *buffer, size_t bufferMax);


// result from <device> XML data structure
struct DeviceInfo 
{
	uint64_t total_space, used_space;
	uint64_t id;

	wchar_t manufacturer[128];
	wchar_t model[128];
	wchar_t name[128];
	wchar_t product[128];
	const ModelInfo *modelInfo;
};

extern "C" void DeviceInfo_Init(DeviceInfo *device_info);
extern "C" void DeviceInfo_Copy(DeviceInfo *dest, const DeviceInfo *source);