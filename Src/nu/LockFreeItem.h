#pragma once
#include <windows.h>

/* This data structure holds one item, and returns you the old item when you replace it 
   it's sort of a "stack of 1" */

template <class value_t>
class LockFreeItem
{
public:
	typedef value_t *ptr_t;
	LockFreeItem() { item = 0; }
	
	value_t *GetItem()
	{
		for(;;) // keep trying until we get this right
		{
			volatile ptr_t ret;
			InterlockedExchangePointer(&ret, item);
			if (InterlockedCompareExchangePointer((volatile PVOID *)&item, 0, ret) == ret)
				return ret;
		}
	}

	// returns the previous value
	value_t *SetItem(value_t *new_item)
	{
		for(;;) // keep trying until we get this right
		{
			volatile ptr_t ret;
			InterlockedExchangePointer(&ret, item);
			if (InterlockedCompareExchangePointer((volatile PVOID *)&item, new_item, ret) == ret)
				return ret;
		}
	}

	// if there's already a value, returns what you passed in
	value_t *SetItemIfZero(value_t *new_item)
	{
		if (InterlockedCompareExchangePointer((volatile PVOID *)&item, new_item, 0) == 0)
				return 0;
		else
			return new_item;
	}

	volatile ptr_t item;
};