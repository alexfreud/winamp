#pragma once
#include "foundation/dispatch.h"
#include "foundation/atomics.h"
#include <new>
#include "nx/nxstring.h"
#include "nx/nxuri.h"

#define REFERENCE_COUNT_AS(x) size_t Retain() { return x::Retain();	}	size_t Release() { return x::Release(); }
template <class t>
class ReferenceCounted : public t
{
public:
	ReferenceCounted() { reference_count = 1; }	
protected:
	/* Dispatchable implementation */

	size_t WASABICALL Dispatchable_Retain()
	{
		return nx_atomic_inc(&reference_count);
	}

	size_t WASABICALL Dispatchable_Release()
	{
		if (!reference_count) 
			return reference_count; 
		size_t r = nx_atomic_dec(&reference_count); 
		if (!r)
		{
#if defined(__ARM_ARCH_7A__)
	__asm__ __volatile__ ("dmb" : : : "memory");
#endif
			delete(this);
		}
		return r; 
	}
	size_t reference_count;
};

template <class t>
class ReferenceCountedObject
{
public:
	ReferenceCountedObject()
	{
		ptr = new (std::nothrow) ReferenceCounted<t>;
	};

	~ReferenceCountedObject()
	{
		if (ptr)
			ptr->Release();
	}

	operator t *()
	{
		return ptr;
	}

	t *operator ->()
	{
		return ptr;
	}

	t *ptr;
};

template <class t>
class ReferenceCountedPointer
{
public:
	ReferenceCountedPointer()
	{
		ptr = 0;
	};

	ReferenceCountedPointer(t *new_ptr)
	{
		ptr = new_ptr;
	};

	~ReferenceCountedPointer()
	{
		if (ptr)
			ptr->Release();
	}

	operator t *()
	{
		return ptr;
	}

	t *operator ->()
	{
		return ptr;
	}

	t **operator &()
	{
		// if there's something already in here, we need to release it first
		if (ptr)
			ptr->Release();
		ptr=0;
		return &ptr;
	}

	t *operator =(t *new_ptr)
	{
		if (ptr)
			ptr->Release();
		ptr=0;
		ptr = new_ptr;
		return ptr;
	}

	t *ptr;
};

class ReferenceCountedNXString
{
public:
	ReferenceCountedNXString()
	{
		ptr = 0;
	};

	~ReferenceCountedNXString()
	{
		NXStringRelease(ptr);		
	}

	operator nx_string_t()
	{
		return ptr;
	}

	nx_string_t *operator &()
	{
		// if there's something already in here, we need to release it first
		if (ptr)
			NXStringRelease(ptr);
		ptr=0;
		return &ptr;
	}

	nx_string_t operator ->()
	{
		return ptr;
	}

	nx_string_t ptr;
};

class ReferenceCountedNXURI
{
public:
	ReferenceCountedNXURI()
	{
		ptr = 0;
	};

	~ReferenceCountedNXURI()
	{
		NXURIRelease(ptr);		
	}

	operator nx_uri_t()
	{
		return ptr;
	}

	nx_uri_t *operator &()
	{
		// if there's something already in here, we need to release it first
		if (ptr)
			NXURIRelease(ptr);
		ptr=0;
		return &ptr;
	}

	nx_uri_t operator ->()
	{
		return ptr;
	}
	nx_uri_t ptr;
};