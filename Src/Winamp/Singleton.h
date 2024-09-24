#ifndef __WASABI_SINGLETON_H
#define __WASABI_SINGLETON_H

#include "api/service/waservicefactoryi.h"
#include "api/service/services.h"

template <class api_t, class impl_t>
class Singleton : public waServiceFactoryI
{
public:
	Singleton() : impl( 0 ), orig( 0 )                                   {}

	Singleton( impl_t *new_impl, bool existing ) : impl( 0 ), orig( 0 )
	{
		if ( existing )
			Register( new_impl );
		else
			RegisterNew( new_impl );
	}

	Singleton( impl_t *&new_impl ) : impl( 0 ), orig( 0 )                { RegisterNew( new_impl ); }

	~Singleton()                                                         { Deregister(); }

	void Register( impl_t *new_impl )
	{
		impl = new_impl;
		WASABI_API_SVC->service_register( this );
	}

	void RegisterNew( impl_t *&new_impl )
	{
		if ( !new_impl )
		{
			orig     = &new_impl;
			new_impl = new impl_t;
		}

		impl = new_impl;
		WASABI_API_SVC->service_register( this );
	}

	void Deregister()
	{
		if ( impl )
		{
			WASABI_API_SVC->service_deregister( this );
			if ( orig )
			{
				delete impl;
				*orig = 0;
			}

			impl = 0;
		}
	}

	impl_t *impl;
	impl_t **orig;

	FOURCC         svc_serviceType()                                     { return WaSvc::UNIQUE; }                 // TODO: make this a (defaulted) template parameter
	const char    *svc_getServiceName()                                  { return impl_t::getServiceName(); }
	GUID           svc_getGuid()                                         { return impl_t::getServiceGuid(); }      // GUID per service factory, can be INVALID_GUID
	void          *svc_getInterface( int global_lock = TRUE )            { return static_cast<api_t *>( impl ); }
	int            svc_supportNonLockingGetInterface()                   { return TRUE; }
	int            svc_releaseInterface( void *ifc )                     { return 0; }
	CfgItem       *svc_getCfgInterface()                                 { return NULL; }
	const wchar_t *svc_getTestString()                                   { return 0; }
	int            svc_notify( int msg, int param1 = 0, int param2 = 0 ) { return 0; }
};


template <class api_t, class impl_t>
class Singleton2 : public waServiceFactoryI
{
public:
	Singleton2() : impl( 0 ), orig( 0 )                                  {}

	Singleton2( impl_t *new_impl, bool existing ) : impl( 0 ), orig( 0 )
	{
		if ( existing )
			Register( new_impl );
		else
			RegisterNew( new_impl );
	}

	Singleton2( impl_t *&new_impl ) : impl( 0 ), orig( 0 )               { RegisterNew( new_impl ); }

	~Singleton2()                                                        { Deregister(); }

	void Register( impl_t *new_impl )
	{
		impl = new_impl;
		WASABI_API_SVC->service_register( this );
	}

	void RegisterNew( impl_t *&new_impl )
	{
		if ( !new_impl )
		{
			orig     = &new_impl;
			new_impl = new impl_t;
		}

		impl = new_impl;
		WASABI_API_SVC->service_register( this );
	}

	void Deregister()
	{
		if ( impl )
		{
			WASABI_API_SVC->service_deregister( this );
			if ( orig )
			{
				delete impl;
				*orig = 0;
			}

			impl = 0;
		}
	}

	impl_t *impl;
	impl_t **orig;

	FOURCC         svc_serviceType()                                     { return WaSvc::UNIQUE; }                 // TODO: make this a (defaulted) template parameter
	const char    *svc_getServiceName()                                  { return impl_t::getServiceName(); }
	GUID           svc_getGuid()                                         { return impl_t::getServiceGuid(); }      // GUID per service factory, can be INVALID_GUID
	void          *svc_getInterface( int global_lock = TRUE )            { return static_cast<api_t *>( impl ); }
	int            svc_supportNonLockingGetInterface()                   { return TRUE; }
	int            svc_releaseInterface( void *ifc )                     { return 0; }
	CfgItem       *svc_getCfgInterface()                                 { return NULL; }
	const wchar_t *svc_getTestString()                                   { return 0; }
	int            svc_notify( int msg, int param1 = 0, int param2 = 0 ) { return 0; }
};

#endif