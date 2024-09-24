#include "main.h"
#include "../winamp/wa_ipc.h"
#include <api/service/waServiceFactory.h>
#include "../../Agave/Config/api_config.h"

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

static api_config *configApi=0;

api_config *GetConfigAPI()
{
	if (mikmod.service && !configApi)
	{
		waServiceFactory *sf = (waServiceFactory *)mikmod.service->service_getServiceByGuid(AgaveConfigGUID);
		configApi = (api_config *)sf->getInterface();
	}

	return configApi;
}
extern "C"
int GetSampleSizeFlag()
{
	api_config *config = GetConfigAPI();
	int bits=16;
	if (config)
		bits = config->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);

	switch(bits)
	{
	case 8:
		return 0;
	case 24:
		return DMODE_24BITS;
	case 16:
	default:
		return DMODE_16BITS;
	}
		
}

extern "C"
int GetNumChannels()
{
		api_config *config = GetConfigAPI();
	bool mono=false;
	if (config)
		mono = config->GetBool(playbackConfigGroupGUID, L"mono", false);

	if (mono)
		return 1;
	else
		return 2;
		
}

extern "C"
int AllowSurround()
{
	api_config *config = GetConfigAPI();
	bool surround=true;
	if (config)
		surround= config->GetBool(playbackConfigGroupGUID, L"surround", true);

	return surround?1:0;		
}

extern "C"
int GetThreadPriorityConfig()
{
	api_config *config = GetConfigAPI();
	int priority=THREAD_PRIORITY_HIGHEST;
	if (config)
		priority = config->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST);

		return priority;
}