#pragma once
#ifndef MT_stl_H_
#define MT_stl_H_

#include <queue>
#include "thread.h"

/***** STL like containers that have Multithread features *******/

template <class T>
class MT_queue
{
	std::queue<T>	m_container;
	mutable AOL_namespace::mutex	m_mutex;
	conditionVariable	m_conditionVariable;

public:
	typedef T value_type;
	typedef T message_t;
	typedef typename std::queue<T>::size_type size_type;

	MT_queue(){}

	size_type size() const
	{
		stackLock sl(m_mutex);
		return m_container.size();
	}

	bool empty() const
	{
		stackLock sl(m_mutex);
		return m_container.empty();
	}

	// note, it is important that this is T and not T&. If it was
	// T& and we did something like this
	//
	// T &f = mtq.front();
	//
	// Then f would be able to manipulate a queue entry outside the
	// protection of the lock. We avoid this by returning T (a copy)
	// instead.
	T front() const	
	{
		stackLock sl(m_mutex);
		return m_container.front();
	}

	void has_front(T &t,bool &has) const
	{
		stackLock sl(m_mutex);
		has = false;
		if (!m_container.empty())
		{
			has = true;
			t = m_container.front();
		}
	}

	void push(const T &t) // push_back
	{
		stackLock sl(m_mutex);
		m_container.push(t);
		m_conditionVariable.signal();
	}

	void pop() // pop_front
	{
		stackLock sl(m_mutex);
		if (!m_container.empty())
		{
			m_container.pop();
		}
	}

	std::queue<T> getAll()
	{
		stackLock sl(m_mutex);

		std::queue<T> result = m_container;
		m_container.clear();
		return result;
	}

	void clear()
	{
		stackLock sl(m_mutex);
		m_container.clear();
	}

	// blocking retrieval. Will block until something is in the
	// queue to get
	T get()
	{
		stackLock sl(m_mutex);

		while (m_container.empty())
		{
			m_conditionVariable.wait(m_mutex);
		}

		T t = m_container.front();
		m_container.pop();

		return t;
	}
};

#endif
