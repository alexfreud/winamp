#ifndef NULLSOFT_WAC_NETWORK_H
#define NULLSOFT_WAC_NETWORK_H

#include "../../Agave/Component/ifc_wa5component.h"


class wac_network : public ifc_wa5component
{
public:
    wac_network();

    void RegisterServices( api_service *service );
    int  RegisterServicesSafeModeOk();

    void DeregisterServices( api_service *service );

protected:
    RECVS_DISPATCH;
};

#endif // !NULLSOFT_WAC_NETWORK_H
