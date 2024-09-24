#include "main.h"
#include "./flagTracker.h"

FlagTracker::FlagTracker() 
	: flags(0), cache(0), updateRef(0), eventHandler(NULL), eventParam(0)
{
	InitializeCriticalSection(&lock);
}

FlagTracker::~FlagTracker()
{
	DeleteCriticalSection(&lock);
}

void FlagTracker::Set(UINT nFlags, UINT nMask)
{
	EnterCriticalSection(&lock);

	nFlags &= nMask;

	UINT clearMask = ~(nFlags ^ nMask);
	flags &= clearMask;
	cache &= clearMask;

	if (0 != nFlags)
	{
		Mark(nFlags);
	}

	LeaveCriticalSection(&lock);
}

UINT FlagTracker::Get()
{
	return flags;
}

void FlagTracker::Mark(UINT nFlags)
{
	EnterCriticalSection(&lock);

	flags |= nFlags;

	if (0 == updateRef)
	{		
		if (NULL != eventHandler && 0 != nFlags)
		{
			eventHandler(nFlags, this, eventParam);
		}
	}
	else
	{
		cache |= nFlags;
	}
	LeaveCriticalSection(&lock);
}

ULONG FlagTracker::BeginUpdate()
{
	EnterCriticalSection(&lock);
    ULONG r = InterlockedIncrement((LONG*)&updateRef);
	LeaveCriticalSection(&lock);
	return r;
}

ULONG FlagTracker::EndUpdate()
{
	EnterCriticalSection(&lock);

	ULONG r;
	if (0 == updateRef) 
	{
		r = 0;
	}
	else
	{
		r = InterlockedDecrement((LONG*)&updateRef);
		if (0 == r && 0 != cache)
		{
			Mark(cache);
			cache = 0;
		}
	}
	LeaveCriticalSection(&lock);
	return r;
}

void FlagTracker::SetEventHandler(EventHandler handler, ULONG_PTR user)
{
	EnterCriticalSection(&lock);

	eventHandler = handler;
	eventParam = user;

	LeaveCriticalSection(&lock);
}