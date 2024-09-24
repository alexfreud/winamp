#include "./setupfactory.h"
#include <api/service/waservicefactorybase.h>
#include <api/service/services.h>
#include "api.h"
#include "gen.h"
#include "main.h"

#define GUID_DEFINE
#include "./setup.h"
#undef GUID_DEFINE

//#include "./spage_lang.h"
//#include "./spage_connect.h"
#include "./spage_skin.h"
#include "./spage_assoc.h"
//#include "./spage_feedback.h"

#include "./sjob_register.h"
#include <shlwapi.h>

class setup_factory : public waServiceFactory
{
public:
	setup_factory();
	virtual ~setup_factory();
public:
	int AddRef();
	int Release();
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);

private:
	int ref;
	svc_setup *psvcSetup;
protected:
	RECVS_DISPATCH;
};


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

BOOL Setup_RegisterService(void)
{
	setup_factory *psf = new setup_factory();
	WASABI_API_SVC->service_register(psf);
	return (0 != psf->Release());
}

int Setup_RegisterDefault(void)
{
	waServiceFactory *psf = WASABI_API_SVC->service_getServiceByGuid(UID_SVC_SETUP);
	if (!psf) return 0;

	svc_setup *pSvc = (svc_setup*)psf->getInterface();
	if (pSvc)
	{
		int index = 0;
		ifc_setuppage *pp;
		ifc_setupjob *pj;
	//	pp = new setup_page_lang();
	//	if (pp) { pSvc->InsertPage(pp, &index); pp->Release(); }

		pp = new setup_page_skin();
		if (pp) { pSvc->InsertPage(pp, &++index); pp->Release(); }

//		pp = new setup_page_connect();
//		if (pp) { pSvc->InsertPage(pp, &++index); pp->Release(); }

		pp = new setup_page_assoc();
		if (pp) { pSvc->InsertPage(pp, &++index); pp->Release(); }

		// disabled for 5.66
//		pp = new setup_page_feedback();
//		if (pp) { pSvc->InsertPage(pp, &++index); pp->Release(); }

		pj = new setup_job_register();
		if (pj) { pSvc->AddJob(pj); pj->Release(); }

		pSvc->Release();
		return 1;
	}

	return 0;
}

int Setup_RegisterPlugins(void)
{
	wchar_t dirstr[MAX_PATH] = {0};
	WIN32_FIND_DATAW d = {0};
	PathCombineW(dirstr, PLUGINDIR, L"GEN_*.DLL");

	HANDLE h = FindFirstFileW(dirstr,&d);
	if (h != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			wchar_t temp[MAX_PATH] = {0};
			PathCombineW(temp, PLUGINDIR, d.cFileName);
			HINSTANCE hLib = LoadLibraryW(temp);
			if (hLib)
			{
				winampGeneralPurposePluginGetter pr = (winampGeneralPurposePluginGetter) GetProcAddress(hLib,"winampGetGeneralPurposePlugin");
				if (pr)
				{
					Plugin_RegisterSetup fn = (Plugin_RegisterSetup)GetProcAddress(hLib, "RegisterSetup");
					if (NULL == fn || FALSE == fn(hLib, WASABI_API_SVC))
					{
						winampGeneralPurposePlugin *plugin = pr();
						if (plugin && (plugin->version == GPPHDR_VER || plugin->version == GPPHDR_VER_U))
						{
							char desc[128] = {0};
							lstrcpynA(desc, plugin->description, sizeof(desc));
							if (desc[0] && !memcmp(desc, "nullsoft(", 9))
							{
								// we'll let this leak for all 3rd party plug-ins as some crash during
								// setup when we try to unload the plug-in e.g gen_Wake_up_call.dll
								FreeModule(hLib);
							}
						}
					}
				}
			}
		} while (FindNextFileW(h,&d));
		FindClose(h);
	}
	return 1;
}

#ifdef __cplusplus
}
#endif // __cplusplus



setup_factory::setup_factory() : ref(1), psvcSetup(NULL)
{
}

setup_factory::~setup_factory()
{
	if (NULL != psvcSetup)
	{
		psvcSetup->Release();
	}
}

int setup_factory::AddRef(void)
{
	return ++ref;
}

int setup_factory::Release(void)
{	
	if (1 == ref) 
	{
		delete(this);
		return 0;
	}
	return --ref;
}

FOURCC setup_factory::GetServiceType() 
{ 
	return WaSvc::UNIQUE;  
}

const char *setup_factory::GetServiceName() 
{ 
	return "Setup Service"; 
}

GUID setup_factory::GetGUID() 
{ 
	return UID_SVC_SETUP; 
}

int setup_factory::SupportNonLockingInterface()
{ 
	return 1; 
}

const char *setup_factory::GetTestString() 
{ 
	return NULL; 
}

int setup_factory::ServiceNotify(int msg, int param1, int param2)
{ 
	switch(msg)
	{
		case SvcNotify::ONREGISTERED:
			AddRef();
			break;
		case SvcNotify::ONDEREGISTERED:
			Release();
			break;
	}
	return 1; 
}

void *setup_factory::GetInterface(int global_lock)
{	
	if (NULL == psvcSetup) 
	{
		psvcSetup = WASetup::CreateInstance();
		if (NULL == psvcSetup)
			return NULL;
	}

	psvcSetup->AddRef();
	return psvcSetup;
}

int setup_factory::ReleaseInterface(void *ifc)
{
	if (ifc == psvcSetup && NULL != psvcSetup)
	{
		if (0 == psvcSetup->Release())
			psvcSetup = NULL;
	}
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS setup_factory
START_DISPATCH
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH

