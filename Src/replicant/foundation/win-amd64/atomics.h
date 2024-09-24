/*

 Win64 (amd64) implementation

*/

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
	return (size_t)_InterlockedIncrement64((volatile LONGLONG *)addr);
}

NX_ATOMIC_INLINE static size_t nx_atomic_dec(volatile size_t *addr)
{
	return (size_t)_InterlockedDecrement64((volatile LONGLONG *)addr);
}

NX_ATOMIC_INLINE static size_t nx_atomic_dec_release(volatile size_t *addr)
{
	return (size_t)_InterlockedDecrement64((volatile LONGLONG *)addr);
}

NX_ATOMIC_INLINE static void nx_atomic_write(size_t value, volatile size_t *addr)
{
	InterlockedExchange64((volatile LONG64 *)addr, value);
}

NX_ATOMIC_INLINE static void nx_atomic_write_pointer(void *value, void* volatile *addr)
{
	InterlockedExchangePointer(addr, value);
}

NX_ATOMIC_INLINE static size_t nx_atomic_add(size_t value, volatile size_t* addr)
{
	return (size_t)InterlockedExchangeAdd64 ((volatile LONGLONG *)addr, (LONGLONG)value);
}

NX_ATOMIC_INLINE static size_t nx_atomic_sub(size_t value, volatile size_t* addr)
{
	return (size_t)InterlockedExchangeAdd64((volatile LONGLONG *)addr, -(LONGLONG)value);
}

NX_ATOMIC_INLINE static void *nx_atomic_swap_pointer(void *value, void* volatile *addr)
{
	return InterlockedExchangePointer(addr, value);
}

NX_ATOMIC_INLINE static int nx_atomic_cmpxchg_pointer(void *oldvalue, void *newvalue, void* volatile *addr)
{
	return InterlockedCompareExchangePointer(addr, newvalue, oldvalue) == oldvalue;
}
/*
NX_ATOMIC_INLINE static int nx_atomic_cmpxchg2(size_t *oldvalue, size_t *newvalue, volatile size_t *addr)
{
	return InterlockedCompare64Exchange128((LONG64 volatile *)addr, (LONG64)newvalue[1], (LONG64)newvalue[0], (LONG64)oldvalue[0]) == oldvalue[0];
}
*/
#if 0
NX_ATOMIC_INLINE static size_t atomic_increment(volatile size_t *val)
{
	return (size_t)InterlockedIncrement((volatile LONG *)val);
}

NX_ATOMIC_INLINE static size_t atomic_decrement(volatile size_t *val)
{
		return (size_t)InterlockedDecrement((volatile LONG *)val);
}

NX_ATOMIC_INLINE static void atomic_add(volatile size_t *val, size_t add)
{
	InterlockedExchangeAdd64((volatile LONGLONG *)val, (LONGLONG)add);
}

NX_ATOMIC_INLINE static void atomic_sub(volatile size_t *val, size_t sub)
{
	InterlockedExchangeAdd64((volatile LONGLONG *)val, -((LONGLONG)sub));
}

NX_ATOMIC_INLINE static void *atomic_exchange_pointer(void* volatile *target, void *value)
{
	return InterlockedExchangePointer(target, value);
}

NX_ATOMIC_INLINE static bool atomic_compare_exchange_pointer(void* volatile *destination, void *exchange, void *compare)
{
	return InterlockedCompareExchangePointer(destination, exchange, compare) == compare;
}

NX_ATOMIC_INLINE static void atomic_write(volatile size_t *dest, size_t src)
{
	InterlockedExchange64((volatile LONG64 *)dest, src);
}

#endif