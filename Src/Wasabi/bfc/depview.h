#ifndef _DEPVIEW_H
#define _DEPVIEW_H

#include <bfc/depend.h>
#include <map>

// this handles classes that use the getDependencyPtr() method instead of a
// public cast

template <class VT>
class NOVTABLE DependentViewerTPtr : private DependentViewerI
{
protected:
	DependentViewerTPtr(VT *first = NULL)
	{
		_addViewItem(first);
	}

	void viewer_addViewItem(VT *item) { _addViewItem(item); }
	void viewer_delViewItem(VT *item) { _delViewItem(item); }
	using DependentViewerI::viewer_delAllItems;

	VT *viewer_enumViewItem(int which)
	{
		return lookup(DependentViewerI::viewer_enumViewItem(which));
	}
	using DependentViewerI::viewer_getNumItems;
	int viewer_haveItem(VT *item)
	{
		return DependentViewerI::viewer_haveItem(item->getDependencyPtr());
	}

	// item you were looking at is gone: WARNING: pointer no longer valid!
	virtual int viewer_onItemDeleted(VT *item) { return 1; }
	// item you are looking at issuing an event
	virtual int viewer_onEvent(VT *item, int event, intptr_t param2, void *ptr, size_t ptrlen) { return 1; }

protected:
	virtual int viewer_onItemDeleted(ifc_dependent *item)
	{
		return viewer_onItemDeleted(lookup(item));
	}
	virtual int viewer_onEvent(ifc_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen)
	{
		if (*classguid != *VT::depend_getClassGuid()) return 0;	// filter namespace
		return viewer_onEvent(lookup(item), event, param, ptr, ptrlen);
	}

private:
	VT *lookup(ifc_dependent *d)
	{
		if (d == NULL) 
			return NULL;
		
		VT *vt = NULL;
		//int r = ptrmap.getItem(d, &vt);
		auto it = ptrmap.find(d);
		if (it != ptrmap.end())
		{
			vt = it->second;
		}

		//ASSERT(r);
		ASSERT(vt->getDependencyPtr() == d);
		return vt;
	}
	void _addViewItem(VT *item)
	{
		if (item == NULL) 
			return ;
		ifc_dependent *d = item->getDependencyPtr();
		if (d == NULL) 
			return ;
		
		ptrmap.insert({ d, item });
		
		DependentViewerI::viewer_addViewItem(d);
	}
	void _delViewItem(VT *item)
	{
		if (item == NULL) return ;
		ifc_dependent *dep = NULL;
		
		//ptrmap.reverseGetItem(item, &dep);
		//int r = ptrmap.reverseDelItem(item);
		for (auto &ptr : ptrmap)
		{
			if (ptr.second == item)
			{
				dep = ptr.first;
				ptrmap.erase(dep);
				break;
			}
		}

		//ASSERT(r);
		DependentViewerI::viewer_delViewItem(dep);
	}

	std::map<ifc_dependent *, VT *> ptrmap;
};

#endif
