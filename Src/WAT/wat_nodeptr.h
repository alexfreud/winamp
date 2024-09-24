#pragma once
#ifndef _WA_LISTS_NODEPTR
#define _WA_LISTS_NODEPTR

namespace wa
{
	namespace lists
	{
		struct node_ptr
		{
			node_ptr* next;
			node_ptr* prev;

			node_ptr() :
				next(0), prev(0)
			{
					
			}
		};
	}
}

#endif