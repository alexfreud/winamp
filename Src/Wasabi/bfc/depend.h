#ifndef _DEPEND_H
#define _DEPEND_H

#include <bfc/platform/platform.h>
#include <bfc/common.h>
#include <bfc/ptrlist.h>

// a pair of classes to implement data dependency. a viewer can register
// a list of things it wants to know about

// WARNING: this file is still under development. check back for changes
// in subsequent SDK releases. over time it is going to become more generic


class ifc_dependent;

#include <api/dependency/api_dependentviewer.h>

// inherit from this one
class /*NOVTABLE */DependentViewerI : public api_dependentviewer
{
protected:
	/**
	 @param classguid	If set, incoming events are restricted to those in that GUID namespace. Can be NULL.
	*/
	DependentViewerI();
	DependentViewerI(const DependentViewerI &dep);
	virtual ~DependentViewerI();
	// copy
	DependentViewerI &operator =(const DependentViewerI &dep);

	// derived classes call this on themselves when they want to view a new item
	// everything else gets handled automagically
	void viewer_addViewItem(ifc_dependent *item);
	void viewer_delViewItem(ifc_dependent *item);
	void viewer_delAllItems();
	void viewer_delAllOfClass(const GUID *guid); //only works if dependent has implemented dependent_getInterface() for the GUID

	// call this whenever you need to know what you're looking at
	ifc_dependent *viewer_enumViewItem(int which);
	int viewer_getNumItems();
	// returns TRUE if item is in our list
	int viewer_haveItem(ifc_dependent *item);

	// convenience callback methods

	// item you were looking at is gone: WARNING: pointer no longer valid!
	virtual int viewer_onItemDeleted(ifc_dependent *item) { return 1; }
	// item you are looking at issuing an event
	virtual int viewer_onEvent(ifc_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen) { return 1; }

private:
	// don't override this; override the individual convenience callbacks
	virtual int dependentViewer_callback(ifc_dependent *item, const GUID *classguid, int cb, intptr_t param1 = 0, intptr_t param2 = 0, void *ptr = NULL, size_t ptrlen = 0);
	typedef PtrList <ifc_dependent> DependentList;
	DependentList * viewed_items;
protected:
	RECVS_DISPATCH;
};

template <class VT>
class NOVTABLE DependentViewerT : private DependentViewerI
{
protected:
	DependentViewerT() { }
	DependentViewerT(VT *first)
	{
		if (first) viewer_addViewItem(first);
	}

public:
	using DependentViewerI::viewer_addViewItem;
	using DependentViewerI::viewer_delViewItem;
	using DependentViewerI::viewer_delAllItems;
	VT *viewer_enumViewItem(int which)
	{
		return static_cast<VT*>(DependentViewerI::viewer_enumViewItem(which));
	}
	using DependentViewerI::viewer_getNumItems;
	using DependentViewerI::viewer_haveItem;

	// spiffy callbacks to override
	// item you were looking at is gone: WARNING: pointer no longer valid!
	virtual int viewer_onItemDeleted(VT *item) { return 1; }
	// item you are looking at issuing an event (filtered by class guid of VT)
	virtual int viewer_onEvent(VT *item, int event, intptr_t param2, void *ptr, size_t ptrlen) { return 1; }

private:
	virtual int viewer_onItemDeleted(ifc_dependent *item)
	{
		return viewer_onItemDeleted(static_cast<VT*>(item));
	}
	virtual int viewer_onEvent(ifc_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen)
	{
		if (*classguid != *VT::depend_getClassGuid()) return 0;	// filter namespace
		return viewer_onEvent(static_cast<VT*>(item), event, param, ptr, ptrlen);
	}
};

// ------------------------------------------------------------

#include <api/dependency/api_dependent.h>

class NOVTABLE DependentI : public ifc_dependent
{
protected:
	DependentI(const GUID *class_guid = NULL);
	DependentI(const DependentI &dep);
	virtual ~DependentI();

public:
	// copy
	DependentI& operator =(const ifc_dependent &dep);

protected:
	// override this to catch when viewers register and deregister
	virtual void dependent_onRegViewer(api_dependentviewer *viewer, int add) {}
	// override this to help people cast you to various classes
	virtual void *dependent_getInterface(const GUID *classguid) { return NULL; }

	// call this on yourself to send an event
	void dependent_sendEvent(const GUID *classguid, int event, intptr_t param = 0, void *ptr = NULL, size_t ptrlen = 0, api_dependentviewer *viewer = NULL);

private:
	virtual void dependent_regViewer(api_dependentviewer *viewer, int add);
	void sendViewerCallbacks(const GUID *classguid, int msg, intptr_t param1 = 0, intptr_t param2 = 0, void *ptr = NULL, size_t ptrlen = 0, api_dependentviewer *viewer = NULL);
	typedef PtrList<api_dependentviewer> ViewerList;
	ViewerList *viewers;
	GUID class_guid;
protected:
	RECVS_DISPATCH;
};


// use like MyClass *myobj = dynamic_guid_cast<MyClass>(some_dependent_ptr);
// MyClass::getClassGuid() must be defined
// MyClass::dependent_getInterface() must be defined too
template <class T>
class dynamic_guid_cast
{
public:
	dynamic_guid_cast(ifc_dependent *_dp, const GUID *_g) : dp(_dp), g(_g) { }
	operator T*()
	{
		return (*g == *T::depend_getClassGuid()) ? static_cast<T*>(dp->dependent_getInterface(g)) : NULL;
	}
private:
	ifc_dependent *dp;
	const GUID *g;
};

#endif
