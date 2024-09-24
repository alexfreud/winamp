#ifndef refPtr_H_
#define refPtr_H_

#include "threading/thread.h"
#ifdef WIN32
#include <crtdbg.h>
#else
#include <assert.h>
#endif

#ifndef WIN32
#ifndef _ASSERTE
#define _ASSERTE(x) assert(x)
#endif
#endif

/*
	Intrusive reference counted pointer template.
	
	The class to be reference counted must implement

		void refIncrement() const  // increment the reference count
		int refDecrement() const // decrement the reference count and return the new value.

	Yes, the methods are const. This provides maximum flexibility.
	Obviously the reference counted values must be mutable for this to work.

	Two predefined bases are provided.

		refCountBase - non locking base for single threaded scenarios
		refCountBaseMT - locking base for multi threaded scenarios

*/

// base class for single threaded reference count objects
class refCountBase
{
template <class T> friend class refPtr;
public:
	/// return the number of references to this object (useful for debugging)
	int refCount() const { return m_refCount; }

protected:
	refCountBase() : m_refCount(0) {}
	virtual ~refCountBase()
	{
		// make sure nobody has deleted this directly with delete
		_ASSERTE(m_refCount == 0);
	}

private:
	mutable int m_refCount;

	void refIncrement() const  { _ASSERTE(m_refCount >= 0); ++m_refCount; }

	int refDecrement() const { _ASSERTE(m_refCount > 0); return --m_refCount; }
};

// base class for multi threaded reference count objects
class refCountBaseMT
{
template <class T> friend class refPtr;
public:
	/// return the number of references to this object (useful for debugging)
	int refCount() const 
	{
		stackLock sl(m_lock);
		return m_refCount; 
	}

protected:
	refCountBaseMT() : m_refCount(0) {}
	virtual ~refCountBaseMT()
	{
		// make sure nobody has deleted this directly with delete
		_ASSERTE(m_refCount == 0);
	}

private:
	mutable int m_refCount;
	mutable AOL_namespace::mutex m_lock;

	void refIncrement() const 
	{
		stackLock sl(m_lock);
		_ASSERTE(m_refCount >= 0);
		++m_refCount; 
	}

	int refDecrement() const 
	{
		stackLock sl(m_lock);
		_ASSERTE(m_refCount > 0);
		return --m_refCount; 
	}
};

template <class T> class refPtr
{
public:
	typedef T *pointer_t;

	// construction
	refPtr() : m_object(NULL) {}

	refPtr(const refPtr<T> &rhs) : m_object(rhs.m_object)
	{
		if (m_object)
			m_object->refIncrement();
	}

	refPtr(T *object) : m_object(object)
	{
		if (m_object)
			m_object->refIncrement();
	}

	~refPtr()
	{
		if (m_object && (m_object->refDecrement() == 0))
			delete m_object;
	}

	// asignment
	refPtr<T> &operator=(const refPtr<T> &rhs)
	{
		if (m_object != rhs.m_object)
		{
			if (m_object && (m_object->refDecrement() == 0))
				delete m_object;
			m_object = rhs.m_object;
			if (m_object)
				m_object->refIncrement();
		}
		return *this;
	}

	/// test if pointers are the same
	inline bool operator==(const refPtr<T> &rhs) const
	{
		return m_object == rhs.m_object;
	}

	inline bool operator==(void *nl) const
	{
		return m_object == nl;
	}

	/// test if pointers are not the same
	inline bool operator!=(const refPtr<T> &rhs) const
	{
		return m_object != rhs.m_object;
	}

	inline bool operator!=(void *nl) const
	{
		return m_object != nl;
	}

	// dereferencing
	T *operator->() const { _ASSERTE(m_object); return (T *)m_object; }
	T &operator*() const { _ASSERTE(m_object); return *((T *)m_object); }
	operator T *() const { return (T *)m_object; }

	/// explicit get - do NOT delete this pointer!
	T *get() const { return (T *)m_object; }

	/* these member templates allows conversion of this smart pointer to
		other smart pointers in the parent hierarchy. This simulates up-casting
		the pointer to a base */
	template <class newType>
		operator refPtr<newType>() { return refPtr<newType>((T *)m_object); }

	template <class newType>
		operator const refPtr<newType>() const { return refPtr<newType>((T *)m_object); }

private:
	T *m_object;
};

#endif
