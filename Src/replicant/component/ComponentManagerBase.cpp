#include "ComponentManagerBase.h"
#include "foundation/error.h"
#include "nx/nxuri.h"

ComponentManagerBase::ComponentManagerBase()
{
	phase=PHASE_INITIALIZE;
	service_api=0;
	component_sync=0;
	framework_guid = INVALID_GUID;
	application_guid = INVALID_GUID;
}

int ComponentManagerBase::LateLoad(ifc_component *component)
{
	int ret;

	if (phase >= PHASE_REGISTERED)
	{
		ret = component->RegisterServices(service_api);
		if (ret != NErr_Success)
		{
			int ret2 = component->Quit(service_api);
			if (ret2 == NErr_TryAgain)
			{
				component_sync->Wait(1);
			}
			return ret;
		}
	}

	if (phase >= PHASE_LOADING)
	{
		ret = component->OnLoading(service_api);
		if (ret != NErr_Success)
		{
			int ret2 = component->Quit(service_api);
			if (ret2 == NErr_TryAgain)
			{
				component_sync->Wait(1);
			}
			return ret;
		}
	}

	if (phase >= PHASE_LOADED)
	{
		ret = component->OnLoaded(service_api);
		if (ret != NErr_Success)
		{
			int ret2 = component->Quit(service_api);
			if (ret2 == NErr_TryAgain)
			{
				component_sync->Wait(1);
			}
			return ret;
		}
	}
	return NErr_Success;
}

void ComponentManagerBase::SetServiceAPI(api_service *_service_api)
{
	this->service_api = _service_api;
	service_api->QueryInterface(&component_sync);
}

int ComponentManagerBase::Load()
{
	if (phase != PHASE_INITIALIZE)
		return NErr_Error;

	int ret;

	/* RegisterServices phase */
	for (ComponentList::iterator itr=components.begin();itr!=components.end();)
	{
		ifc_component *component = *itr;
		//ComponentList::iterator next=itr;
		//next++;
		ret = component->RegisterServices(service_api);
		if (ret != NErr_Success)
		{
			int ret2 = component->Quit(service_api);
			if (ret2 == NErr_TryAgain)
			{
				component_sync->Wait(1);
			}	
			NXURIRelease(component->component_info.filename);
			CloseComponent(component);
			itr = components.erase(itr);
		}
		else
		{
			itr++; // = next;
		}
	}

	phase = PHASE_REGISTERED;

	/* OnLoading phase */
	for (ComponentList::iterator itr=components.begin();itr!=components.end();)
	{
		ifc_component *component = *itr;
		//ComponentList::iterator next=itr;
		//next++;
		ret = component->OnLoading(service_api);
		if (ret != NErr_Success)
		{
			int ret2 = component->Quit(service_api);
			if (ret2 == NErr_TryAgain)
			{
				component_sync->Wait(1);
			}	
			NXURIRelease(component->component_info.filename);
			CloseComponent(component);
			itr = components.erase(itr);
		}
		else
		{
			itr++; // = next;
		}
	}

	phase = PHASE_LOADING;

	/* OnLoaded phase */
	for (ComponentList::iterator itr = components.begin(); itr != components.end();)
	{
		ifc_component* component = *itr;
		//ComponentList::iterator next = itr;
		//next++;
		ret = component->OnLoaded(service_api);
		if (ret != NErr_Success)
		{
			int ret2 = component->Quit(service_api);
			if (ret2 == NErr_TryAgain)
			{
				component_sync->Wait(1);
			}
			NXURIRelease(component->component_info.filename);
			CloseComponent(component);
			itr = components.erase(itr);
		}
		else
		{
			itr++; // = next;
		}
	}

	phase = PHASE_LOADED;

	return NErr_Success;
}

int ComponentManagerBase::AddComponent(ifc_component *component)
{
	int err;
	
	if (NULL == component)
		return NErr_BadParameter;
	
	if (phase > PHASE_LOADED)
		return NErr_Error;
	
	if (component->component_info.wasabi_version != wasabi2_component_version
		|| component->component_info.nx_api_version != nx_api_version
		|| component->component_info.nx_platform_guid != nx_platform_guid)
	{
		return NErr_IncompatibleVersion;
	}

	if (component->component_info.framework_guid != INVALID_GUID 
		&& framework_guid != component->component_info.framework_guid)
	{
		return NErr_IncompatibleVersion;
	}
	
	if (component->component_info.application_guid != INVALID_GUID 
		&& application_guid != component->component_info.application_guid)
	{
		return NErr_IncompatibleVersion;
	}
	
	for (ComponentList::iterator itr = components.begin(); itr != components.end(); itr++)
	{
		ifc_component *registered_component = *itr;
		if (registered_component->component_info.component_guid == component->component_info.component_guid)
			return NErr_Error;
	}
	
	err = component->Initialize(service_api);
	if (NErr_Success != err)
		return err;
	
	/* if the component was added late, we'll need to run some extra stages */
	err = LateLoad(component);
	if (NErr_Success != err)
		return err;
	
	components.push_back(component);
	
	return NErr_Success;
}

void ComponentManagerBase::SetFrameworkGUID(GUID guid)
{
	framework_guid = guid;
}

GUID ComponentManagerBase::GetFrameworkGUID()
{
	return framework_guid;
}

void ComponentManagerBase::SetApplicationGUID(GUID guid)
{
	application_guid = guid;
}

GUID ComponentManagerBase::GetApplicationGUID()
{
	return application_guid;
}
