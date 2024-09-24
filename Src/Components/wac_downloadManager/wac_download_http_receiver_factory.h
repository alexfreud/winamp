#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_FACTORY_HTTP_RECEIVER_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_FACTORY_HTTP_RECEIVER_H

#include <string>

#include "api/service/waservicefactory.h"
#include "api/service/services.h"

namespace wa
{
	namespace Factory
	{
		class WAC_DownloadMabager_HTTPReceiver_Factory : public waServiceFactory
		{
		public:
			FOURCC      GetServiceType();
			const char *GetServiceName();
			GUID        GetGUID();

			void       *GetInterface( int global_lock );
			int         SupportNonLockingInterface();
			int         ReleaseInterface( void *ifc );

			const char *GetTestString();
			int         ServiceNotify( int msg, int param1, int param2 );

		protected:
			RECVS_DISPATCH;
		};
	}
}

#endif  //!NULLSOFT_WAC_DOWNLOAD_MANAGER_FACTORY_HTTP_RECEIVER_H