#ifndef __WASABI_API_DEPENDENTVIEWER_H
#define __WASABI_API_DEPENDENTVIEWER_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
class ifc_dependent;

class NOVTABLE ifc_dependentviewer : public Dispatchable
{
protected:
	ifc_dependentviewer() {}
	~ifc_dependentviewer() {}
public:
	// item calls when it changes or disappears, or whatever
	int dependentViewer_callback(ifc_dependent *item, const GUID *classguid, int cb, intptr_t param1 = 0, intptr_t param2 = 0, void *ptr = NULL, size_t ptrlen = 0);

	DISPATCH_CODES
	{
	    DEPENDENTVIEWER_CALLBACK = 10,
	};
};

inline int ifc_dependentviewer::dependentViewer_callback(ifc_dependent *item, const GUID *classguid, int cb, intptr_t param1 , intptr_t param2 , void *ptr , size_t ptrlen)
{
	return _call(DEPENDENTVIEWER_CALLBACK, (int)0, item, classguid, cb, param1, param2, ptr, ptrlen);
}

typedef ifc_dependentviewer api_dependentviewer;

#endif
