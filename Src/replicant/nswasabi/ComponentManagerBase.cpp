#include "ComponentManagerBase.h"
#include "foundation/error.h"
#include "nx/nxuri.h"

ComponentManagerBase::ComponentManagerBase()
{
	phase=PHASE_INITIALIZE;
	service_api=0;
	component_sync=0;
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

void ComponentManagerBase::SetServiceAPI(api_service *service_api)
{
	this->service_api = service_api;
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
		ComponentList::iterator next=itr;
		next++;
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
			components.erase(component);

		}
		itr=next;
	}

	phase = PHASE_REGISTERED;

	/* OnLoading phase */
	for (ComponentList::iterator itr=components.begin();itr!=components.end();)
	{
		ifc_component *component = *itr;
		ComponentList::iterator next=itr;
		next++;
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
			components.erase(component);

		}
		itr=next;
	}

	phase = PHASE_LOADING;

	/* OnLoaded phase */
	for (ComponentList::iterator itr=components.begin();itr!=components.end();)
	{
		ifc_component *component = *itr;
		ComponentList::iterator next=itr;
		next++;
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
			components.erase(component);
		}
		itr=next;
	}

	phase = PHASE_LOADED;

	return NErr_Success;
}
