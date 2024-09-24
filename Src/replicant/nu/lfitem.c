#include "nu/lfitem.h"
#include "foundation/atomics.h"

void lfitem_init(lfitem_t item)
{
	item->item=0;
}

void *lfitem_get(lfitem_t item)
{
	return nx_atomic_swap_pointer(0, &(item->item));
}

void *lfitem_set(lfitem_t item, const void *value)
{
	return nx_atomic_swap_pointer((void *)value, &(item->item));
}

void *lfitem_set_if_zero(lfitem_t item, void *value)
{
	if (nx_atomic_cmpxchg_pointer(0, value, &(item->item)) == 0)
		return 0;
	else
		return (void *)value;
}