#ifndef NULLSOFT_FACTORY_WAC_BROWSER_H
#define NULLSOFT_FACTORY_WAC_BROWSER_H

#include <string>

#include "api__wac_browser.h"

#include "api/service/services.h"
#include "api/service/waservicefactory.h"

namespace wa
{
	namespace Components
	{
		class WAC_BrowserFactory : public waServiceFactory
		{
		public:
			//WAC_BrowserFactory()                                      {}
			//~WAC_BrowserFactory()                                     {}

			FOURCC      GetServiceType();
			const char *GetServiceName();
			const char *GetTestString();
			GUID        GetGUID();

			void       *GetInterface( int p_global_lock );
			int         ReleaseInterface( void *p_ifc );

			int         SupportNonLockingInterface();
			int         ServiceNotify( int p_msg, int p_param1, int p_param2 );


			HRESULT     Register( api_service *p_service );
			HRESULT     Unregister( api_service *p_service );


		protected:
			RECVS_DISPATCH;
		};
	}
}

#endif // !NULLSOFT_FACTORY_WAC_BROWSER_H