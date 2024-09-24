#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_MAIN_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_MAIN_H

#include "wac_downloadManager_global.h"

#include "api__wac_downloadManager.h"

//#include "wac_downloadManager.h"
//#include "wac_download_http_receiver.h"

#include "bfc/dispatch.h"

#include "../../Winamp/Singleton.h"
#include "../../Agave/Component/ifc_wa5component.h"


namespace wa
{
    namespace Components
    {
        class WAC_DownloadManager_Component : public ifc_wa5component
        {
        public:
            WAC_DownloadManager_Component()                                     {}

            void RegisterServices( api_service *service );
            int  RegisterServicesSafeModeOk()                         { return 1; }

            void DeregisterServices( api_service *service );

        protected:
            RECVS_DISPATCH;
        };
    }
}

#endif // !NULLSOFT_WAC_DOWNLOAD_MANAGER_MAIN_H