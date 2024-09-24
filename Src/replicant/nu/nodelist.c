#include "nodelist.h"

void nodelist_init(nodelist_t nodelist)
{
	nodelist->head=0;
	nodelist->tail=0;
}

void nodelist_push_back(nodelist_t nodelist, queue_node_t *item)
{
	if (!nodelist->head)
	{
		nodelist->head=item;
		nodelist->tail=item;
	}
	else
	{
		nodelist->tail->Next=item;
		nodelist->tail=item;
	}
	item->Next = 0;
}

void nodelist_push_front(nodelist_t nodelist, queue_node_t *item)
{
	if (!nodelist->head)
	{
		nodelist->head=item;
		nodelist->tail=item;
		item->Next=0;
	}
	else
	{
		item->Next = nodelist->head;
		nodelist->head = item;
	}
}

queue_node_t *nodelist_pop_front(nodelist_t nodelist)
{
	queue_node_t *ret;
	if (!nodelist->head)
		return 0;

	ret = nodelist->head;
	nodelist->head = nodelist->head->Next;
	ret->Next = 0; // so we don't confuse anyone

	if (ret == nodelist->tail)
	{
		nodelist->tail = 0;
	}
	return ret;
}

void nodelist_push_back_list(nodelist_t nodelist, queue_node_t *item)
{
	if (!nodelist->head)
	{
		nodelist->head=item;
	}
	else
	{
		nodelist->tail->Next=item;
	}

	while (item->Next)
		item = item->Next;

	nodelist->tail = item;
}
