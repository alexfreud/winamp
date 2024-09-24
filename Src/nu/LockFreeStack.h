#pragma once
#include <windows.h>

/* lock free stack object 
multiple threads can push and pop without locking
note that order is not guaranteed.  that is, if Thread 1 calls Insert before Thread 2, Thread 2's item might still make it in before.

since it uses malloc/free for the linked list nodes, it might not be considered truely lock-free.

use LockFreeStack2 if you can assure that you have a 'next' pointer in your class/struct
*/
template <class value_t>
class LockFreeStackNode
{
public:
	LockFreeStackNode(value_t _data)
	{
		data = _data;
		next = 0;
	}
	value_t data;

	LockFreeStackNode *next;
};

template <class value_t>
class LockFreeStack
{
public:
	LockFreeStack()
	{
		head = 0;
	}

	void push(value_t data)
	{
		LockFreeStackNode<value_t> *new_head = new LockFreeStackNode<value_t>(data);
		LockFreeStackNode<value_t> *old_head = 0;
		do
		{
			old_head = (LockFreeStackNode<value_t> *)head;
			new_head->next = old_head;
		} while (InterlockedCompareExchangePointer((volatile PVOID *)&head, new_head, old_head) != old_head);
	}

	value_t pop()
	{
		LockFreeStackNode<value_t> *new_head = 0, *old_head = 0;
		do
		{
			old_head = (LockFreeStackNode<value_t> *)head;
			if (old_head)
			{
				new_head = old_head->next;
			}	
			else
			{
				new_head = 0;
			}
		} while (InterlockedCompareExchangePointer((volatile PVOID *)&head, new_head, old_head) != old_head);
		value_t ret = old_head?old_head->data:0;
		delete old_head;
		return ret;
	}

	bool pop_ref(value_t *val)
	{
		LockFreeStackNode<value_t> *new_head = 0, *old_head = 0;
		do
		{
			old_head = (LockFreeStackNode<value_t> *)head;
			if (old_head)
			{
				new_head = old_head->next;
			}			
			else
			{
				new_head = 0;
			}
		} while (InterlockedCompareExchangePointer((volatile PVOID *)&head, new_head, old_head) != old_head);
		if (old_head)
		{
			*val = old_head->data;
			delete old_head;
			return true;
		}

		return false;
	}

	volatile LockFreeStackNode<value_t> *head;
};

template <class value_t>
class LockFreeStack2
{
public:
	LockFreeStack()
	{
		head = 0;
	}

	void push(value_t *data)
	{
		value_t *old_head = 0;
		do
		{
			old_head = head;
			data->next = old_head;
		} while (InterlockedCompareExchangePointer((volatile PVOID *)&head, data, old_head) != old_head);
	}

	value_t *pop()
	{
		value_t *new_head = 0, *old_head = 0;
		do
		{
			old_head = head;
			if (old_head)
			{
				new_head = old_head->next;
			}
			else
			{
				new_head = 0;
			}
		} while (InterlockedCompareExchangePointer((volatile PVOID *)&head, new_head, old_head) != old_head);
		return old_head;
	}

	bool pop_ref(value_t **val)
	{
		value_t *new_head = 0, *old_head = 0;
		do
		{
			old_head = head;
			if (old_head)
			{
				new_head = old_head->next;
			}
			else
			{
				new_head = 0;
			}
		} while (InterlockedCompareExchangePointer((volatile PVOID *)&head, new_head, old_head) != old_head);
		if (old_head)
		{
			*val = old_head;
			return true;
		}

		return false;
	}

	volatile value_t *head;
};