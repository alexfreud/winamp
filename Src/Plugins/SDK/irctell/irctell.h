#ifndef _GENERIC_H
#define _GENERIC_H


#include <api/syscb/callbacks/corecb.h>
#include <api/syscb/callbacks/svccb.h>
#include "../Agave/Component/ifc_wa5component.h"
#include <bfc/multipatch.h>

enum {	patch_wa5,	patch_core,	patch_svc};
class WACIrctell :public MultiPatch<patch_wa5, ifc_wa5component>,
	public MultiPatch<patch_core, CoreCallback>,
	public MultiPatch<patch_svc, SysCallback>
{

public:
	void RegisterServices(api_service *service);
	void DeregisterServices(api_service *service);

	int ccb_notify(int msg, int param1=0, int param2=0);

	FOURCC getEventType();
  int notify(int msg, int param1 = 0, int param2 = 0);

protected:
	RECVS_MULTIPATCH;
};

#endif
