#include "main.h"

//#include "wac_network_connection_factory.h"
#include "ServiceBuild.h"

#include <openssl/ssl.h>

#include "bfc/platform/export.h"



#include "netinc.h"
#include "api__wac_network.h"

#include "wac_network_http_receiver_factory.h"
#include "wac_network_connection_factory.h"
#include "wac_network_ssl_connection_factory.h"
#include "wac_network_dns_factory.h"
#include "wac_network_web_server_factory.h"
#include "util.h"

#include "api/syscb/api_syscb.h"
#include "wac_network_ssl_connection.h"

#include <openssl/ssl.h>

#include "bfc/platform/export.h"
#include "../nu/nonewthrow.c"


JNL_HTTPGetFactory    HTTPGetService;
JNL_ConnectionFactory connectionService;
JNL_AsyncDNSFactory   dnsService;
JNL_WebServFactory    webserverService;




api_service     *WASABI_API_SVC        = Q_NULLPTR;
api_application *WASABI_API_APP        = Q_NULLPTR;
api_config      *AGAVE_API_CONFIG      = Q_NULLPTR;
api_threadpool  *WASABI_API_THREADPOOL = Q_NULLPTR;
api_syscb       *WASABI_API_SYSCB      = Q_NULLPTR;


static wa::Components::WAC_Network_AsyncDNS *global_dns = NULL;

wa::Components::WAC_Network_AsyncDNS *GetGlobalDNS()
{
	if ( !global_dns )
	{
		if ( JNL::open_socketlib() )
			return NULL;

		global_dns = new wa::Components::WAC_Network_AsyncDNS;
	}

	return global_dns;
}

void DestroyGlobalDNS()
{
	if ( global_dns )
	{
		delete global_dns;
		global_dns = NULL;

		JNL::close_socketlib();
	}
}


#ifdef USE_SSL
#include <wincrypt.h>
#include <openssl/rand.h>

static HCRYPTPROV GetKeySet()
{
	HCRYPTPROV   hCryptProv;
	LPCWSTR UserName = L"WinampKeyContainer";  // name of the key container

	if ( CryptAcquireContext(
		&hCryptProv,               // handle to the CSP
		UserName,                  // container name
		NULL,                      // use the default provider
		PROV_RSA_FULL,             // provider type
		0 ) )                        // flag values
	{
		return hCryptProv;
	}
	else if ( CryptAcquireContext(
		&hCryptProv,
		UserName,
		NULL,
		PROV_RSA_FULL,
		CRYPT_NEWKEYSET ) )
	{
		return hCryptProv;
	}
	else
		return 0;

}

JNL_SSL_ConnectionFactory sslConnectionService;

void InitSSL()
{
	SSL_load_error_strings();
	SSL_library_init();

	HCRYPTPROV hCryptProv = GetKeySet();
	if ( hCryptProv )
	{
		BYTE pbData[ 8 * sizeof( unsigned long ) ] = { 0 };
		if ( CryptGenRandom( hCryptProv, 8 * sizeof( unsigned long ), pbData ) )
		{
			RAND_seed( pbData, 16 );
		}

		CryptReleaseContext( hCryptProv, 0 );
	}

	sslContext = SSL_CTX_new( SSLv23_client_method() );

	SSL_CTX_set_verify( sslContext, SSL_VERIFY_NONE, NULL );
	//	SSL_CTX_set_session_cache_mode(sslContext, SSL_SESS_CACHE_OFF);
}

void QuitSSL()
{
	SSL_CTX_free( sslContext );
}

#endif  // !USE_SSL



wa::Components::WAC_Network _wac_network;


void wa::Components::WAC_Network::RegisterServices( api_service *p_service )
{
	WASABI_API_SVC = p_service;

	ServiceBuild( WASABI_API_SYSCB,      syscbApiServiceGuid );
	ServiceBuild( WASABI_API_APP,        applicationApiServiceGuid );
	ServiceBuild( AGAVE_API_CONFIG,      AgaveConfigGUID );
	ServiceBuild( WASABI_API_THREADPOOL, ThreadPoolGUID );


	WASABI_API_SVC->service_register( &HTTPGetService );
	WASABI_API_SVC->service_register( &connectionService );

#ifdef USE_SSL
	WASABI_API_SVC->service_register( &sslConnectionService );
#endif

	WASABI_API_SVC->service_register( &dnsService );
	WASABI_API_SVC->service_register( &webserverService );
	//dlMgrFactory.Register( &dlMgr );
}

void wa::Components::WAC_Network::DeregisterServices( api_service *p_service )
{
	Q_UNUSED( p_service )

	ServiceRelease( WASABI_API_SYSCB,      syscbApiServiceGuid );

	ServiceRelease( WASABI_API_APP,        applicationApiServiceGuid );
	ServiceRelease( AGAVE_API_CONFIG,      AgaveConfigGUID );


	//dlMgr.Kill();
	//dlMgrFactory.Deregister();
	DestroyGlobalDNS();
	p_service->service_deregister( &HTTPGetService );
	p_service->service_deregister( &connectionService );

#ifdef USE_SSL
	p_service->service_deregister( &sslConnectionService );
#endif

	p_service->service_deregister( &dnsService );

#ifdef USE_SSL
	QuitSSL();
#endif

	ServiceRelease( WASABI_API_THREADPOOL, ThreadPoolGUID );
}


extern "C" WAC_NETWORK_EXPORT ifc_wa5component * GetWinamp5SystemComponent()
{
	return &_wac_network;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS wa::Components::WAC_Network
START_DISPATCH;
VCB( API_WA5COMPONENT_REGISTERSERVICES,           RegisterServices )
CB(  API_WA5COMPONENT_REGISTERSERVICES_SAFE_MODE, RegisterServicesSafeModeOk )
VCB( API_WA5COMPONENT_DEREEGISTERSERVICES,        DeregisterServices )
END_DISPATCH;
#undef CBCLASS
