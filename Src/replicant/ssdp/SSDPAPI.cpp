#include "api__ssdp.h"
#include "SSDPAPI.h"
#include "foundation/error.h"
#include "jnetlib/jnetlib.h"
#include "nx/nxsleep.h"
#include "nswasabi/AutoCharNX.h"
#include "nswasabi/ReferenceCounted.h"
#include <time.h>
#include <stdio.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef _WIN32
#define snprintf _snprintf
#endif

SSDPAPI::SSDPAPI()
{
	listener=0;
	user_agent=0;
}

SSDPAPI::~SSDPAPI()
{
	jnl_udp_release(listener);

	for (ServiceList::iterator itr=services.begin();itr!=services.end();itr++)
	{
		NXURIRelease(itr->location);
		NXStringRelease(itr->type);
		NXStringRelease(itr->usn);
	}

	for (ServiceList::iterator itr=found_services.begin();itr!=found_services.end();itr++)
	{
		NXURIRelease(itr->location);
		NXStringRelease(itr->type);
		NXStringRelease(itr->usn);
	}
}

int SSDPAPI::Initialize()
{
	user_agent=WASABI2_API_APP->GetUserAgent();
	NXThreadCreate(&ssdp_thread, SSDPThread, this);
	return NErr_Success;
}

int SSDPAPI::SSDP_RegisterService(nx_uri_t location, nx_string_t type, nx_string_t usn)
{
	nu::AutoLock auto_lock(services_lock);
	Service service = {0};
	service.location = NXURIRetain(location);
	service.type = NXStringRetain(type);
	service.usn = NXStringRetain(usn);
	services.push_back(service);

	return NErr_Success;
}

int SSDPAPI::SSDP_RegisterCallback(cb_ssdp *callback)
{
	if (!callback)
	{
		return NErr_BadParameter;
	}

	threadloop_node_t *node = thread_loop.GetAPC();
	if (!node)
	{
		return NErr_OutOfMemory;
	}

	node->func = APC_RegisterCallback;
	node->param1 = this;
	node->param2 = callback;
	callback->Retain();
	thread_loop.Schedule(node);
	return NErr_Success;
}

int SSDPAPI::SSDP_UnregisterCallback(cb_ssdp *callback)
{
	if (!callback)
	{
		return NErr_BadParameter;
	}

	threadloop_node_t *node = thread_loop.GetAPC();
	if (!node)
	{
		return NErr_OutOfMemory;
	}
	
	node->func = APC_UnregisterCallback;
	node->param1 = this;
	node->param2 = callback;
	// since we don't actually know if the callback is registered until the other thread runs
	//  we can't guarantee that we have a reference.  so we'll add one!
	callback->Retain();
	thread_loop.Schedule(node);
	return NErr_Success;
}

int SSDPAPI::SSDP_Search(nx_string_t type)
{
	if (!type)
	{
		return NErr_BadParameter;
	}

	threadloop_node_t *node = thread_loop.GetAPC();
	if (!node)
	{
		return NErr_OutOfMemory;
	}
	
	node->func = APC_Search;
	node->param1 = this;
	node->param2 = NXStringRetain(type);
	thread_loop.Schedule(node);
	return NErr_Success;
}

int SSDPAPI::FindUSN(ServiceList &service_list, const char *usn, Service *&service)
{
	for (ServiceList::iterator itr = service_list.begin(); itr != service_list.end(); itr++)
	{
		if (!NXStringKeywordCompareWithCString(itr->usn, usn))
		{
			service = &(* itr);
			return NErr_Success;
		}
	}

	size_t num_services = service_list.size();
	Service new_service = {0};
	NXStringCreateWithUTF8(&new_service.usn, usn);
	service_list.push_back(new_service);
	service = &service_list.at(num_services);

	return NErr_Empty;
}

nx_thread_return_t SSDPAPI::SSDPThread(nx_thread_parameter_t parameter)
{
	return ((SSDPAPI *)parameter)->Internal_Run();
}

void SSDPAPI::Internal_RegisterCallback(cb_ssdp *callback)
{
	callbacks.push_back(callback);
	nu::AutoLock auto_lock(services_lock);
	for (ServiceList::iterator itr=found_services.begin();itr!=found_services.end();itr++)
	{
		callback->OnServiceConnected(itr->location, itr->type, itr->usn);
	}
}

void SSDPAPI::Internal_UnregisterCallback(cb_ssdp *callback)
{
	// TODO: verify that our list actually contains the callback?
	//callbacks.eraseObject(callback);
	auto it = std::find(callbacks.begin(), callbacks.end(), callback);
	if (it != callbacks.end())
	{
		callbacks.erase(it);
	}

	callback->Release(); // release the reference retained on the other thread
	callback->Release(); // release _our_ reference
}

nx_thread_return_t SSDPAPI::Internal_Run()
{	
	addrinfo *addr = 0;
	jnl_dns_resolve_now("239.255.255.250", 1900, &addr, SOCK_DGRAM);
	int ret = jnl_udp_create_multicast_listener(&listener, "239.255.255.250", 1900);
	if (ret != NErr_Success || !addr)
		return (nx_thread_return_t)ret;

	jnl_httpu_request_t httpu;
	ret = jnl_httpu_request_create(&httpu);
	if (ret != NErr_Success)
	{

		return (nx_thread_return_t)ret;
	}

	time_t last_notify = 0;

	for (;;)
	{
		time_t now = time(0);
		if ((now - last_notify) > 15)
		{
			Internal_Notify(0, addr->ai_addr, (socklen_t)addr->ai_addrlen);
			if (ret != NErr_Success)
			{
				// TODO: ?	
			}
			last_notify = time(0);
#if 0
#if defined(_DEBUG) && defined(_WIN32)
			FILE *f = fopen("c:/services.txt", "wb");
			if (f)
			{
				for (ServiceList::iterator itr=found_services.begin();itr!=found_services.end();itr++)
				{
	
					fprintf(f, "-----\r\n");
					fprintf(f, "ST: %s\r\n", AutoCharPrintfUTF8(itr->type));
					fprintf(f, "Location: %s\r\n", AutoCharPrintfUTF8(itr->location));
					fprintf(f, "USN: %s\r\n", AutoCharPrintfUTF8(itr->usn));
				}
				fclose(f);
			}
#elif defined(__ANDROID__)
				for (ServiceList::iterator itr=found_services.begin();itr!=found_services.end();itr++)
				{
	
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "-----\r\n");
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "ST: %s\r\n", AutoCharPrintfUTF8(itr->type));
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "Location: %s\r\n", AutoCharPrintfUTF8(itr->location));
					__android_log_print(ANDROID_LOG_INFO, "libreplicant", "USN: %s\r\n", AutoCharPrintfUTF8(itr->usn));
				}
#endif
#endif
		}

		thread_loop.Step(100);

		for (;;)
		{
			size_t bytes_received=0;

			jnl_udp_run(listener, 0, 8192, 0, &bytes_received);
			if (bytes_received)
			{
				jnl_httpu_request_process(httpu, listener);
				const char *method = jnl_httpu_request_get_method(httpu);
				if (method)
				{
					if (!strcmp(method, "NOTIFY"))
					{

						unsigned int max_age=0;
						const char *cache_control = jnl_httpu_request_get_header(httpu, "cache-control");
						if (cache_control)
						{
							if (strncasecmp(cache_control, "max-age=", 8) == 0)
							{
								max_age = strtoul(cache_control+8, 0, 10);
							}
						}
						const char *location = jnl_httpu_request_get_header(httpu, "location");
						const char *nt = jnl_httpu_request_get_header(httpu, "nt");
						const char *nts = jnl_httpu_request_get_header(httpu, "nts");
						const char *usn = jnl_httpu_request_get_header(httpu, "usn");

						// a hack to work with older android wifi sync protocol
						if (!usn)
							usn = jnl_httpu_request_get_header(httpu, "id");
						if (usn && nt && location && nts && !strcmp(nts, "ssdp:alive"))
						{
							Service *service = 0;
							int ret = FindUSN(found_services, usn, service);
							if (ret == NErr_Success) // found an existing one
							{
								// update the last seen time and expiration date
								// TODO: should we double check that the location didn't change?
								bool changed=false;
								ReferenceCountedNXString old_location;
								
								if (service->location)
									NXURIGetNXString(&old_location, service->location);
								if (!service->location || NXStringKeywordCompareWithCString(old_location, location))
								{
									NXURIRelease(service->location);
									NXURICreateWithUTF8(&service->location, location);
									changed=true;
								}

								service->last_seen = time(0);
								if (max_age)
									service->expiry = service->last_seen + max_age;
								else
									service->expiry = (time_t)-1;

								if (changed)
								{
									// notify clients
									for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
									{
										(*itr)->OnServiceDisconnected(service->usn);
										(*itr)->OnServiceConnected(service->location, service->type, service->usn);
									}
								}
							}
							else if (ret == NErr_Empty) // new one
							{
								NXURICreateWithUTF8(&service->location, location);
								NXStringCreateWithUTF8(&service->type, nt);
								service->last_seen = time(0);
								if (max_age)
									service->expiry = service->last_seen + max_age;
								else							
									service->expiry = (time_t)-1;

								// notify clients
								for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
								{
									(*itr)->OnServiceConnected(service->location, service->type, service->usn);
								}
							}
						}
					}
					else if (!strcmp(method, "M-SEARCH"))
					{
						sockaddr *addr; socklen_t addr_length;
						jnl_udp_get_address(listener, &addr, &addr_length);
						const char *st = jnl_httpu_request_get_header(httpu, "st");
						Internal_Notify(st, addr, addr_length);
					}
				}	
			}
			else
			{
				break;
			}
		}

		// check for expirations
again:
		nu::AutoLock auto_lock(services_lock);
		for (ServiceList::iterator itr=found_services.begin();itr!=found_services.end();itr++)
		{
			if (itr->expiry < now)
			{
				for (CallbackList::iterator itr2=callbacks.begin();itr2!=callbacks.end();itr2++)
				{
					(*itr2)->OnServiceDisconnected(itr->usn);
				}
				
				NXURIRelease(itr->location);
				NXStringRelease(itr->usn);
				NXStringRelease(itr->type);
				found_services.erase(itr);
				goto again; 
			}
		}

	}

	return 0;
}

static char notify_request[] = "NOTIFY * HTTP/1.1\r\n";
static char notify_host[] = "HOST: 239.255.255.250:1900\r\n";
static char notify_cache_control[] = "CACHE-CONTROL:max-age=600\r\n";
static char notify_nts[] = "NTS:ssdp:alive\r\n";

static char search_request[] = "NOTIFY * HTTP/1.1\r\n";
static char search_man[] = "MAN:\"ssdp:discover\"\r\n";

ns_error_t SSDPAPI::Internal_Notify(const char *st, sockaddr *addr, socklen_t addr_length)
{
	jnl_udp_set_peer_address(listener, addr, addr_length);
	nu::AutoLock auto_lock(services_lock);
	for (ServiceList::iterator itr=services.begin();itr!=services.end();itr++)
	{
		if (st && NXStringKeywordCompareWithCString(itr->type, st))
			continue;

		jnl_udp_send(listener, notify_request, strlen(notify_request));
		jnl_udp_send(listener, notify_host, strlen(notify_host));
		jnl_udp_send(listener, notify_cache_control, strlen(notify_cache_control));
		jnl_udp_send(listener, notify_nts, strlen(notify_nts));
		char header[512] = {0};
		snprintf(header, 511, "LOCATION:%s\r\n", AutoCharPrintfUTF8(itr->location));
		header[511]=0;
		jnl_udp_send(listener, header, strlen(header));
		if (itr->usn)
		{
			snprintf(header, 511, "USN:%s\r\n", AutoCharPrintfUTF8(itr->usn));
			header[511]=0;
			jnl_udp_send(listener, header, strlen(header));
		}
		if (itr->type)
		{
			snprintf(header, 511, "NT:%s\r\n", AutoCharPrintfUTF8(itr->type));
			header[511]=0;
			jnl_udp_send(listener, header, strlen(header));
		}
		if (user_agent)
		{
			snprintf(header, 511, "SERVER:%s\r\n",user_agent);
			header[511]=0;
			jnl_udp_send(listener, header, strlen(header));
		}
		jnl_udp_send(listener, "\r\n", 2);

		size_t bytes_sent=0;
		do
		{
			jnl_udp_run(listener, 8192, 0, &bytes_sent, 0); // TODO: error check
			if (bytes_sent == 0)
				NXSleep(100);
		} while (bytes_sent == 0);
	}
	return NErr_Success;
}

void SSDPAPI::Internal_Search(nx_string_t st)
{
	addrinfo *addr;
	jnl_dns_resolve_now("239.255.255.250", 1900, &addr, SOCK_DGRAM);
	jnl_udp_set_peer_address(listener, addr->ai_addr, (socklen_t)addr->ai_addrlen);
	jnl_udp_send(listener, notify_request, strlen(notify_request));

	jnl_udp_send(listener, search_request, strlen(search_request));
	jnl_udp_send(listener, notify_host, strlen(notify_host));
	jnl_udp_send(listener, search_man, strlen(search_man));
	char header[512] = {0};
	sprintf(header, "ST:%s\r\n", AutoCharPrintfUTF8(st));
	header[511]=0;
	jnl_udp_send(listener, header, strlen(header));
	size_t bytes_sent=0;
	do
	{
		jnl_udp_run(listener, 8192, 0, &bytes_sent, 0); // TODO: error check
		if (bytes_sent == 0)
			NXSleep(100);
	} while (bytes_sent == 0);
}
