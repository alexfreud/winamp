#pragma once
#include <api/service/svcs/svc_skinfilter.h>
#include <api/service/waservicefactory.h>
class GammaFilter : public svc_skinFilter
{
public:
  int filterBitmap(uint8_t *bits, int w, int h, int bpp, const wchar_t *element_id, const wchar_t *forcegroup=NULL);
  ARGB32 filterColor(ARGB32 color, const wchar_t *element_id, const wchar_t *forcegroup=NULL);
  static const char *getServiceName() { return "Gamma skin filter"; }
protected:
	RECVS_DISPATCH;
};

class GammaFilterFactory : public waServiceFactory
{
public:
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);
	
protected:
	RECVS_DISPATCH;
};


