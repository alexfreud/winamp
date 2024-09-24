#pragma once
#include "foundation/atomics.h"
#include "lfitem.h"
/* This data structure holds one item, and returns you the old item when you replace it 
   it's sort of a "stack of 1" */

template <class value_t>
class LockFreeItem
{
public:
	typedef value_t *ptr_t;
	LockFreeItem() 
	{
	lfitem_init(&item);
	}
	
	value_t *GetItem()
	{
		return (value_t *)lfitem_get(&item);
	}

	// returns the previous value
	value_t *SetItem(value_t *new_item)
	{
		return (value_t *)lfitem_set(&item, new_item);
	}

	// if there's already a value, returns what you passed in
	value_t *SetItemIfZero(value_t *new_item)
	{
		return (value_t *)lfitem_set_if_zero(&item, new_item);
	}

	lfitem_value_t item;
};
