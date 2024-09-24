#ifndef NULLSOFT_WAC_DOWNLOADS_H
#define NULLSOFT_WAC_DOWNLOADS_H

#include "../../Agave/Component/ifc_wa5component.h"


class wac_downloads : public ifc_wa5component
{
public:
    wac_downloads();

    void RegisterServices( api_service *service );
    int  RegisterServicesSafeModeOk();

    void DeregisterServices( api_service *service );

protected:
    RECVS_DISPATCH;
};

#endif // !NULLSOFT_WAC_DOWNLOADS_H
