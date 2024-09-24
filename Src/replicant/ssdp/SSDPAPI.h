#pragma once
#include "nx/nxstring.h"
#include "nx/nxuri.h"
#include <vector>
#include "nu/AutoLock.h"
#include "nx/nxthread.h"
#include "foundation/error.h"
#include "jnetlib/jnetlib.h"
#include "cb_ssdp.h"
#include "nu/ThreadLoop.h"
#include "api_ssdp.h"
#include "nswasabi/ServiceName.h"

class SSDPAPI : public api_ssdp
{
public:
	SSDPAPI();
	~SSDPAPI();
	WASABI_SERVICE_NAME("SSDP API");
	int Initialize();

	int WASABICALL SSDP_RegisterService(nx_uri_t location, nx_string_t st, nx_string_t usn);
	int WASABICALL SSDP_RegisterCallback(cb_ssdp *callback);
	int WASABICALL SSDP_UnregisterCallback(cb_ssdp *callback);
	int WASABICALL SSDP_Search(nx_string_t st);
private:
		struct Service
	{
		nx_uri_t location;
		nx_string_t type;
		nx_string_t usn;
		time_t last_seen;
		time_t expiry;
	};

	// TODO: map it off of USN
	typedef std::vector<Service> ServiceList;
	typedef std::vector<cb_ssdp*> CallbackList;
	nx_thread_return_t Internal_Run();
	ns_error_t Internal_Notify(const char *st, sockaddr *addr, socklen_t length);
	void Internal_RegisterCallback(cb_ssdp *callback);
	void Internal_UnregisterCallback(cb_ssdp *callback);
	void Internal_Search(nx_string_t st);
	static nx_thread_return_t NXTHREADCALL SSDPThread(nx_thread_parameter_t parameter);
	int FindUSN(ServiceList &service_list, const char *usn, Service *&service);

	ThreadLoop thread_loop;
	CallbackList callbacks;
	ServiceList services;
	ServiceList found_services;
	nu::LockGuard services_lock;
	nx_thread_t ssdp_thread;
	jnl_udp_t listener;
	const char *user_agent;



	static void APC_RegisterCallback(void *_this, void *_callback, double) { ((SSDPAPI *)_this)->Internal_RegisterCallback((cb_ssdp *)_callback); }
	static void APC_UnregisterCallback(void *_this, void *_callback, double) { ((SSDPAPI *)_this)->Internal_UnregisterCallback((cb_ssdp *)_callback); }
	static void APC_Search(void *_this, void *st, double) { ((SSDPAPI *)_this)->Internal_Search((nx_string_t)st); }
};
