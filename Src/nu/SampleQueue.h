#pragma once
#include <bfc/platform/types.h>
#include <deque>
#include "../nu/AutoLock.h"

template <class SampleData>
class SampleQueue
{
public:
	void PushFree(SampleData *new_sample)
	{
		queue_guard.Lock();
		free_queue.push_front(new_sample);
		queue_guard.Unlock();
	}

	void PushProcessed(SampleData *new_sample)
	{
		queue_guard.Lock();
		processed_queue.push_front(new_sample);
		queue_guard.Unlock();
	}

	// will return 0 if none ready
	SampleData *PopProcessed()
	{
		SampleData *sample=0;
		queue_guard.Lock();
		if (!processed_queue.empty())
		{
			sample = processed_queue.back();
			processed_queue.pop_back();
		}
		queue_guard.Unlock();
		return sample;
	}

	SampleData *PopFree()
	{
		SampleData *sample=0;
		queue_guard.Lock();
		if (!free_queue.empty())
		{
			sample = free_queue.back();
			free_queue.pop_back();
		}
		queue_guard.Unlock();
		if (!sample)
			sample = new SampleData;
		return sample;
	}

	void Trim()
	{
		queue_guard.Lock();
		//free_queue.deleteAll();
		auto it_f = free_queue.begin();
		while (it_f != free_queue.end())
		{
			SampleData* p = *it_f;
			delete p;
			it_f = free_queue.erase(it_f);
		}
			
		//processed_queue.deleteAll();
		auto it_p = processed_queue.begin();
		while (it_p != processed_queue.end())
		{
			SampleData* p = *it_p;
			delete p;
			it_p = processed_queue.erase(it_p);
		}
		queue_guard.Unlock();
	}

private:
	std::deque<SampleData*> free_queue;
	std::deque<SampleData*> processed_queue;

	Nullsoft::Utility::LockGuard queue_guard;
};
