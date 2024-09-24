#pragma once
#include "../../foundation/types.h"
#include <Windows.h>
#include <intrin.h>

#ifdef __cplusplus
#define NX_ATOMIC_INLINE inline
#else
#define NX_ATOMIC_INLINE
#endif

NX_ATOMIC_INLINE static size_t nx_atomic_inc(volatile size_t *addr)
{
	return (size_t)_InterlockedIncrement((volatile LONG *)addr);
}

NX_ATOMIC_INLINE static size_t nx_atomic_dec(volatile size_t *addr)
{
	return (size_t)_InterlockedDecrement((volatile LONG *)addr);
}

NX_ATOMIC_INLINE static size_t nx_atomic_dec_release(volatile size_t *addr)
{
	return (size_t)_InterlockedDecrement((volatile LONG *)addr);
}

NX_ATOMIC_INLINE static void nx_atomic_write(size_t value, volatile size_t *addr)
{
	InterlockedExchange((LONG *)addr, value);
}

NX_ATOMIC_INLINE static void nx_atomic_write_pointer(void *value, void* volatile *addr)
{
	InterlockedExchangePointer(addr, value);
}

NX_ATOMIC_INLINE static size_t nx_atomic_add(size_t value, volatile size_t* addr)
{
	return (size_t)InterlockedExchangeAdd((volatile LONG *)addr, (LONG)value);
}

NX_ATOMIC_INLINE static size_t nx_atomic_sub(size_t value, volatile size_t* addr)
{
	return (size_t)InterlockedExchangeAdd((volatile LONG *)addr, -(LONG)value);
}

NX_ATOMIC_INLINE static void *nx_atomic_swap_pointer(const void *value, void* volatile *addr)
{
	return InterlockedExchangePointer(addr, (PVOID)value);
}

NX_ATOMIC_INLINE static int nx_atomic_cmpxchg_pointer(void *oldvalue, void *newvalue, void* volatile *addr)
{
	return InterlockedCompareExchangePointer(addr, newvalue, oldvalue) == oldvalue;
}

#pragma intrinsic(_InterlockedCompareExchange64)
NX_ATOMIC_INLINE static int nx_atomic_cmpxchg2(int64_t oldvalue, int64_t newvalue, volatile int64_t *addr)
{
	return _InterlockedCompareExchange64(addr, newvalue, oldvalue) == oldvalue;
}
