#pragma once

/* This data structure holds one item, and returns you the old item when you replace it 
   it's sort of a "stack of 1" */

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct lfitem_struct_t 
	{
		void * volatile item;
	}  lfitem_value_t, *lfitem_t;


	void lfitem_init(lfitem_t item);
	void *lfitem_get(lfitem_t item);
	void *lfitem_set(lfitem_t item, const void *value);
	void *lfitem_set_if_zero(lfitem_t item, void *value);

#ifdef __cplusplus
}
#endif
