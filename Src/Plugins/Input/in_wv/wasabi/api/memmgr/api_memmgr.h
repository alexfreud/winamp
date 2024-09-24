#ifndef __API_MEMMGR_H
#define __API_MEMMGR_H

#include "../../bfc/dispatch.h"
#include "../../bfc/platform/types.h"

class NOVTABLE api_memmgr : public Dispatchable
{
protected:
	api_memmgr() {}
	~api_memmgr() {}

public:
	void *sysMalloc(size_t size);
	void sysFree(void *ptr);
	void *sysRealloc(void *ptr, size_t newsize);
	void sysMemChanged(void *ptr);

	DISPATCH_CODES
	{
		API_MEMMGR_SYSMALLOC = 0,
		API_MEMMGR_SYSFREE = 10,
		API_MEMMGR_SYSREALLOC = 20,
		API_MEMMGR_SYSMEMCHANGED = 30,
	};

	// Some helper templates to new and delete objects with the memory manager
	// you need to be cautious with Delete() and inheritance, particularly if you're dealing with a base class
	// as the pointer to the derived class might not equal to the pointer to the base class, particularly with multiple inheritance
	// e.g. class C : public A, public B {};  C c; assert((A*)&c == (B*)&c); will likely fail

	template <class Class>
		void New(Class **obj)
	{
		size_t toAlloc = sizeof(Class);
		void *mem = sysMalloc(toAlloc);
		*obj = new (mem) Class;
	}

	template <class Class>
		void Delete(Class *obj)
	{
		if (obj)
		{
			obj->~Class();
			sysFree(obj);
		}
	}
};

inline void *api_memmgr::sysMalloc(size_t size)
{
	return _call(API_MEMMGR_SYSMALLOC, (void *)NULL, size);
}

inline void api_memmgr::sysFree(void *ptr)
{
	_voidcall(API_MEMMGR_SYSFREE, ptr);
}

inline void *api_memmgr::sysRealloc(void *ptr, size_t newsize)
{
	return _call(API_MEMMGR_SYSREALLOC, (void *)NULL, ptr, newsize);
}

inline void api_memmgr::sysMemChanged(void *ptr)
{
	_voidcall(API_MEMMGR_SYSMEMCHANGED, ptr);
}


// {000CF46E-4DF6-4a43-BBE7-40E7A3EA02ED}
static const GUID memMgrApiServiceGuid =
{ 0xcf46e, 0x4df6, 0x4a43, { 0xbb, 0xe7, 0x40, 0xe7, 0xa3, 0xea, 0x2, 0xed } };

//extern api_memmgr *memmgrApi;

#endif
