#ifndef NULLSOFT_FACTORY_WAC_NETWORK_CONNECTION_H
#define NULLSOFT_FACTORY_WAC_NETWORK_CONNECTION_H

#include <string>

#include "api/service/services.h"
#include "api/service/waservicefactory.h"


class JNL_ConnectionFactory : public waServiceFactory
{
public:
	FOURCC       GetServiceType();
	const char  *GetServiceName();
	GUID         GetGUID();
	const char  *GetTestString();

	void        *GetInterface( int global_lock );
	int          ReleaseInterface( void *ifc );

	int          SupportNonLockingInterface();
	int          ServiceNotify( int msg, int param1, int param2 );

protected:
	RECVS_DISPATCH;
};


//
//namespace wa
//{
//	namespace Components
//	{
//		class WAC_ConnectionFactory : public waServiceFactory
//		{
//			//public:
//			////WAC_ConnectionFactory()                                          {}
//			////~WAC_ConnectionFactory()                                         {}
//
//			//FOURCC      GetServiceType();
//			//const char *GetServiceName();
//			//const char *GetTestString();
//			//GUID        GetGUID();
//
//			//void       *GetInterface( int p_global_lock );
//			//int         ReleaseInterface( void *p_ifc );
//
//			//int         SupportNonLockingInterface();
//			//int         ServiceNotify( int p_msg, int p_param1, int p_param2 );
//
//
//			//HRESULT     Register( api_service *p_service );
//			//HRESULT     Unregister( api_service *p_service );
//
//
//			//protected:
//			//RECVS_DISPATCH;
//		};
//	}
//}

#endif // !NULLSOFT_FACTORY_WAC_NETWORK_CONNECTION_H