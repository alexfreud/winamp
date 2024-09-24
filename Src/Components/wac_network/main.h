#ifndef NULLSOFT_WAC_NETWORK_MAIN_H
#define NULLSOFT_WAC_NETWORK_MAIN_H

#include <QtCore>

#include "wac_network_global.h"

#include "api__wac_network.h"
//#include "ServiceBuild.h"


#include "../Agave/Component/ifc_wa5component.h"

namespace wa
{
    namespace Components
    {
        class WAC_Network : public ifc_wa5component
        {
        public:
            WAC_Network()                                             {}

            void RegisterServices( api_service *service );
            int  RegisterServicesSafeModeOk()                         { return 1; }

            void DeregisterServices( api_service *service );

        protected:
            RECVS_DISPATCH;
        };
    }
}

#endif // !NULLSOFT_WAC_NETWORK_MAIN_H