#pragma once
#include "nx/nxuri.h"
#include "service/api_service.h"
#include "component/ifc_component.h"
#include "nu/PtrDeque.h"
#include "component/ifc_component_sync.h"

class ComponentManagerBase
{
public:
	void SetServiceAPI(api_service *service_api);
	int Load();
protected:
	ComponentManagerBase();
	int LateLoad(ifc_component *mod);
	enum Phase
	{
		PHASE_INITIALIZE=0, /* components are still being added */
		PHASE_REGISTERED=1, /* RegisterServices() has been called on all components */
		PHASE_LOADING=2, /* OnLoading() has been called on all components */
		PHASE_LOADED=3, /* OnLoaded() has been called on all components */
	};
	Phase phase;
	typedef nu::PtrDeque<ifc_component> ComponentList;
	ComponentList components;
	api_service *service_api;
	ifc_component_sync *component_sync;
private:
	/* your implementation needs to override this.  You should call FreeLibrary(component->component_info.hModule); or dlclose(component->component_info.dl_handle); or similar */
	virtual void CloseComponent(ifc_component *component)=0;
};
