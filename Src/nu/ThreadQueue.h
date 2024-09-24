#pragma once
#include "RingBuffer.h"
#include <semaphore.h>

class ThreadQueue
{
public:
	ThreadQueue();
	~ThreadQueue();
	void Queue(const void *);
	// Get() blocks until there's something in the queue
	void *Get();
	// return value is same as sem_wait
	// delay is in nanoseconds
	int Wait(long delay, void **val);
	// kind of like sem_trywait
	int Try(void **val);
private:
	// TODO: need to use something safer than RingBuffer, preferably a lock-free linked list so we can grow unlimited
	 RingBuffer buffer;
	sem_t event;
};

