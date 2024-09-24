#pragma once
#include "nu/lfmpscq.h"
#include "nu/LockFreeLIFO.h"
#include <windows.h>

struct threadloop_node_t : public queue_node_t
{
	void (*func)(void *param1, void *param2, double real_value);

	void *param1;
	void *param2;
	double real_value;
};

class ThreadLoop
{
public:
	ThreadLoop();
	threadloop_node_t *GetAPC(); // returns a node for you to fill out
	void Schedule(threadloop_node_t *apc);
	void Run();
	void Kill();
private:
	void RefillCache();

	HANDLE procedure_notification;
	HANDLE kill_switch;
	mpscq_t procedure_queue;

	/* Memory cache to be able to run APCs without having the memory manager lock 
	we'll allocate 100 at a time (#defined by PROCEDURE_CACHE_SEED)
	and allocate new ones only if the cache is empty (which unfortunately will lock)
	cache_bases holds the pointers we've allocated (to free on destruction of this object)
	and procedure_cache holds the individual pointers */
	static lifo_t procedure_cache;
	static lifo_t cache_bases;
};