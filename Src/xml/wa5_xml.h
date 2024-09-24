#ifndef __WASABI_WA5_XML_H
#define __WASABI_WA5_XML_H

#include "../Agave/Component/ifc_wa5component.h"

class WA5_XML : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};
#endif