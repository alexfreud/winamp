#ifndef __WASABI_API_DEPENDENT_H
#define __WASABI_API_DEPENDENT_H

#include <bfc/dispatch.h>
#include "api_dependentviewer.h"

// we define some common event codes here. if your object needs more send
// them as parameters to OBJECTSPECIFIC

// some system-level codes for cb param. You can implement your own events
// with DEPCB_EVENT and the parameters
namespace DependentCB
{
	enum {
	    DEPCB_NOP = 0,
	    DEPCB_DELETED = 100, 		// object being deleted
	    DEPCB_EVENT = 1000, 			// object-specific event. use param1 etc to send your messages
	};
};

class NOVTABLE ifc_dependent : public Dispatchable
{
protected:
	ifc_dependent() {}
	~ifc_dependent() {}
public:
	void dependent_regViewer(ifc_dependentviewer *viewer, int add) ;
	void *dependent_getInterface(const GUID *classguid);

	DISPATCH_CODES
	{
		API_DEPENDENT_REGVIEWER = 10,
		API_DEPENDENT_GETINTERFACE = 20,
	};
};

inline void ifc_dependent::dependent_regViewer(api_dependentviewer *viewer, int add)
	{
		_voidcall(API_DEPENDENT_REGVIEWER, viewer, add);
	}
	inline void *ifc_dependent::dependent_getInterface(const GUID *classguid)
	{
		return _call(API_DEPENDENT_GETINTERFACE, (void *)0, classguid);
	}


// this is a helper for dependent_getInterface
#define HANDLEGETINTERFACE(x) { \
		if (*classguid == *x::depend_getClassGuid()) return static_cast<x *>(this); \
	}

typedef ifc_dependent api_dependent;

#endif