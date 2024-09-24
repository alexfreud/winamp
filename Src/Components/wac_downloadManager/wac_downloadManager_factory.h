#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_FACTORY_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_FACTORY_H

#include "api__wac_downloadManager.h"

#include "api/service/waservicefactory.h"
#include "api/service/services.h"

namespace wa
{
	namespace Components
	{
		class WAC_DownloadManagerFactory : public waServiceFactory
		{
		public:
			FOURCC      GetServiceType();
			const char *GetServiceName();
			GUID        GetGUID();
			const char *GetTestString();

			void       *GetInterface( int global_lock );
			int         ReleaseInterface( void *ifc );

			int         SupportNonLockingInterface();
			int         ServiceNotify( int msg, int param1, int param2 );

			HRESULT     Register( api_service *p_service );
			HRESULT     Unregister( api_service *p_service );


		protected:
			RECVS_DISPATCH;
		};
	}
}

#endif // !NULLSOFT_WAC_DOWNLOAD_MANAGER_FACTORY_H
