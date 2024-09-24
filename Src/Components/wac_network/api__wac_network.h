#ifndef NULLSOFT_WAC_NETWORK_API_H
#define NULLSOFT_WAC_NETWORK_API_H

#include "netinc.h"
#include "wac_network_dns.h"

wa::Components::WAC_Network_AsyncDNS *GetGlobalDNS();
void                                  DestroyGlobalDNS();

#include "api/service/api_service.h"

#include "api/application/api_application.h"
#define WASABI_API_APP applicationApi

#include "../Agave/Config/api_config.h"

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#include "api/syscb/api_syscb.h"
#define WASABI_API_SYSCB sysCallbackApi

#endif // !NULLSOFT_WAC_NETWORK_API_H

