#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
template <class ifc_t>
class Countable : public ifc_t
{
public:
	Countable() 
	{
		ref_count=1;
	}

	// this needs to be done like this otherwise the destructor doesn't get called properly (we don't want virtual destructor for various reasons)
#define REFERENCE_COUNT_IMPLEMENTATION 	size_t AddRef() { return InterlockedIncrement((LONG*)&ref_count);	}\
	size_t Release() { if (!ref_count) return ref_count; LONG r = InterlockedDecrement((LONG*)&ref_count); if (!r) delete(this); return r; }
protected:
	size_t ref_count;
};

#define REFERENCE_COUNTED CB(ADDREF, AddRef); CB(RELEASE, Release);

