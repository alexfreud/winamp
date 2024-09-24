#ifndef NULLSOFT_PLLOADEX_EXCOMPONENT_H
#define NULLSOFT_PLLOADEX_EXCOMPONENT_H

#include "../Agave/Component/ifc_wa5component.h"

class ExComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	void DeregisterServices(api_service *service);

protected:
	RECVS_DISPATCH; // all Wasabi objects implementing a Dispatchable interface require this
};

#endif