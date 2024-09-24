#pragma once

/* 
Simple Reader/Writer lock.  Lets unlimited readers through but a writer will lock exclusively
not meant for high-throughput uses
this is useful when writes are very infrequent
*/
#include <bfc/platform/types.h>
#include <windows.h>

typedef size_t simple_rwlock_t;
static const size_t simple_rwlock_writer_active = 1; // writer active flag
static const size_t simple_rwlock_reader_increment= 2; // to adjust reader count

static inline void simple_rwlock_write_lock(simple_rwlock_t *lock)
{
	while (InterlockedCompareExchangePointer((PVOID volatile*)lock, (PVOID)simple_rwlock_writer_active, 0))
	{
		// nop
	}
}

static inline void simple_rwlock_write_unlock(simple_rwlock_t *lock)
{
#ifdef _WIN64
	InterlockedExchangeAdd64((LONGLONG  volatile*)lock, -simple_rwlock_writer_active);
#else
	InterlockedExchangeAdd((LONG volatile*)lock, -simple_rwlock_writer_active);
#endif
}


static inline void simple_rwlock_read_lock(simple_rwlock_t *lock)
{
	InterlockedExchangeAdd((LONG volatile*)lock, simple_rwlock_reader_increment);
	while ((*lock & simple_rwlock_writer_active))
	{
		// nope
	}
}

static inline void simple_rwlock_read_unlock(simple_rwlock_t *lock)
{
	#ifdef _WIN64
	InterlockedExchangeAdd64((LONGLONG  volatile*)lock, -simple_rwlock_reader_increment);
#else
	InterlockedExchangeAdd((LONG volatile*)lock, -simple_rwlock_reader_increment);
#endif
}
