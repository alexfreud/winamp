#ifndef NULLSOFT_SYSCALLBACKSH
#define NULLSOFT_SYSCALLBACKSH

#include <api/syscb/api_syscb.h>
#include "../nu/AutoLock.h"
#include <map>
#include <vector>

class SysCallbacks : public api_syscb
{
public:
	static const char *getServiceName() { return "System Callback API"; }
	static const GUID getServiceGuid() { return syscbApiServiceGuid; }
public:
	SysCallbacks();
    int syscb_registerCallback(SysCallback *cb, void *param = NULL);
    int syscb_deregisterCallback(SysCallback *cb);
    int syscb_issueCallback(int eventtype, int msg, intptr_t param1 = 0, intptr_t param2 = 0);
		SysCallback *syscb_enum(int eventtype, size_t n); 

protected:
	RECVS_DISPATCH;
private:
	Nullsoft::Utility::LockGuard callbackGuard;
	typedef std::vector<SysCallback*> CallbackList;
	typedef std::map<int, CallbackList*> EventMap;
	EventMap callback_map;
	CallbackList deleteMeAfterCallbacks;
	bool inCallback;
	volatile int reentry;
};

extern SysCallbacks *sysCallbacks;

#endif