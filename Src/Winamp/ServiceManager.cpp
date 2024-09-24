/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "ServiceManager.h"
#include <api.h>
#include <api/service/waservicefactory.h>
#include "../nu/AutoLock.h"
#include <api/syscb/callbacks/syscb.h>
#include <api/syscb/callbacks/svccb.h>

using namespace Nullsoft::Utility;


struct Counter
{
	Counter( waServiceFactory *_owner, void *_ptr ) : ptr( _ptr ), owner( _owner ) {}

	void             *ptr;
	waServiceFactory *owner;
};

ServiceManager::ServiceManager() : serviceGuard(512)
{}

int ServiceManager::service_register( waServiceFactory *svc )
{
	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_register" ) );
	FOURCC service_type = svc->getServiceType();

	// add the service to the master list
	services.push_back( svc );

	// add it to the by-type lookup
	ServiceList *&type_list = services_by_type[ service_type ];
	if ( !type_list )
		type_list = new ServiceList;

	type_list->push_back( svc );

	// send notifications
	svc->serviceNotify( SvcNotify::ONREGISTERED );
	WASABI_API_SYSCB->syscb_issueCallback( SysCallback::SERVICE, SvcCallback::ONREGISTER, (intptr_t)service_type, reinterpret_cast<intptr_t>( svc ) );

	return 1;
}

int ServiceManager::service_deregister( waServiceFactory *svc )
{
	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_deregister" ) );
	FOURCC service_type = svc->getServiceType();

	// remove it from the master list
	//services.eraseObject(svc);
	auto it = std::find( services.begin(), services.end(), svc );
	if ( it != services.end() )
		services.erase( it );

	// and from the type lookup map
	ServiceList *type_list = services_by_type[ service_type ];
	if ( type_list )
	{
		//type_list->eraseObject(svc);
		auto it = std::find( type_list->begin(), type_list->end(), svc );
		if ( it != type_list->end() )
			type_list->erase( it );
	}

	WASABI_API_SYSCB->syscb_issueCallback( SysCallback::SERVICE, SvcCallback::ONDEREGISTER, (intptr_t)service_type, reinterpret_cast<intptr_t>( svc ) );
	svc->serviceNotify( SvcNotify::ONDEREGISTERED );

	return 1;
}

size_t ServiceManager::service_getNumServices( FOURCC svc_type )
{
	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_getNumServices" ) );
	if ( svc_type )
	{
		ServiceList *type_list = services_by_type[ svc_type ];
		if ( type_list )
			return type_list->size();
		else
			return 0;
	}
	else
		return services.size();
}

waServiceFactory *ServiceManager::service_enumService( FOURCC svc_type, size_t n )
{
	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_enumService" ) );
	ServiceList *type_list = 0;
	if ( svc_type )
		type_list = services_by_type[ svc_type ];
	else
		type_list = &services;

	if ( type_list && n < type_list->size() )
		return type_list->at( n );
	else
		return 0;
}

waServiceFactory *ServiceManager::service_getServiceByGuid( GUID guid )
{
	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_getServiceByGuid" ) );
	for ( waServiceFactory *l_service : services )
	{
		if ( l_service->getGuid() == guid )
			return l_service;
	}

	return NULL;
}

int ServiceManager::service_lock( waServiceFactory *owner, void *svcptr )
{
	if ( owner == NULL || svcptr == NULL )
		return 0;

	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_lock" ) );
	locks.push_back( new Counter( owner, svcptr ) );

	return 1;
}

int ServiceManager::service_clientLock( void *svcptr )
{
	return 1;
}

int ServiceManager::service_release( void *svcptr )
{
	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_release" ) );
	for ( size_t i = 0; i != locks.size(); i++ )
	{
		if ( locks[ i ]->ptr == svcptr )
		{
			void             *ptr   = locks[ i ]->ptr;
			waServiceFactory *owner = locks[ i ]->owner;

			delete locks[ i ];

			locks.erase( locks.begin() + i );
			owner->releaseInterface( ptr );

			return 1;
		}
	}

	return 0;
}

const char *ServiceManager::service_getTypeName( FOURCC svc_type )
{
	return NULL;
}

int ServiceManager::service_unlock( void *svcptr )
{
	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_unlock" ) );
	for ( size_t i = 0; i != locks.size(); i++ )
	{
		if ( locks[ i ]->ptr == svcptr )
		{
			delete locks[ i ];
			locks.erase( locks.begin() + i );

			return 1;
		}
	}

	return 0;
}

int ServiceManager::service_isvalid( FOURCC svctype, waServiceFactory *service )
{
	AutoLock lock( serviceGuard LOCKNAME( "ServiceManager::service_isvalid" ) );
	//return !!services.contains(service);
	return ( services.end() != std::find( services.begin(), services.end(), service ) );
}

int ServiceManager::service_compactDuplicates( waServiceFactory *me )
{
	// first, find 'me'
	GUID guid = me->getGuid();

	size_t me_index = 0;

	//bool found = services.findItem(me, &me_index);
	//if (!found)
	//	return 1;
	if ( services.end() == std::find( services.begin(), services.end(), me ) )
		return 1;

	ServiceList *type_list = services_by_type[ me->getServiceType() ];

	// go in reverse order because service to compact is likely at the end
	size_t n = services.size();
	while ( n-- ) // n is guaranteed to be >0 because otherwise the 'find' loop above would have failed
	{
		waServiceFactory *n_service = services[ n ];
		if ( n != me_index && n_service != me && n_service->getGuid() == guid )
		{
			services[ me_index ] = n_service;
			services.erase( services.begin() + n );

			// fix up our by-type cache
			if ( type_list )
			{
				size_t me_index = 0;
				size_t n_index = 0;

				//----------------------------------------------------
				// Replaced with code below this commented code
				//if (type_list->findItem_reverse(n_service, &n_index) && type_list->findItem(me, &me_index))
				//{
				//	type_list->at(me_index) = n_service;
				//	type_list->eraseindex(n_index);
				//}

				bool foundService = false;
				for ( int idx = type_list->size() - 1; idx >= 0; idx-- )
				{
					if ( type_list->at( idx ) == n_service )
					{
						foundService = true;
						n_index = idx;
						break;
					}
				}

				bool foundMe = false;
				for ( int idx2 = 0; idx2 < type_list->size(); idx2++ )
				{
					if ( type_list->at( idx2 ) == me )
					{
						foundMe = true;
						me_index = idx2;
						break;
					}
				}

				if ( foundService && foundMe )
				{
					type_list->at( me_index ) = n_service;
					type_list->erase( type_list->begin() + n_index );
				}
				//--------------------------------------------------
			}

			me->serviceNotify( SvcNotify::ONDEREGISTERED );

			return 0;
		}
	}

	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ServiceManager
START_DISPATCH;
CB( API_SERVICE_SERVICE_REGISTER,         service_register );
CB( API_SERVICE_SERVICE_DEREGISTER,       service_deregister );
CB( API_SERVICE_SERVICE_GETNUMSERVICES,   service_getNumServices );
CB( API_SERVICE_SERVICE_ENUMSERVICE,      service_enumService );
CB( API_SERVICE_SERVICE_GETSERVICEBYGUID, service_getServiceByGuid );
CB( API_SERVICE_SERVICE_LOCK,             service_lock );
CB( API_SERVICE_SERVICE_CLIENTLOCK,       service_clientLock );
CB( API_SERVICE_SERVICE_RELEASE,          service_release );
CB( API_SERVICE_SERVICE_GETTYPENAME,      service_getTypeName );
CB( API_SERVICE_SERVICE_UNLOCK,           service_unlock );
CB( API_SERVICE_ISVALID,                  service_isvalid );
CB( API_SERVICE_COMPACT_DUPLICATES,       service_compactDuplicates );
END_DISPATCH;
#undef CBCLASS