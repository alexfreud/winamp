#ifndef NULLSOFT_WINAMP_FLAG_TRACKER_HEADER
#define NULLSOFT_WINAMP_FLAG_TRACKER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class FlagTracker
{
public:
	typedef void (CALLBACK *EventHandler)(UINT /*nMarked*/, FlagTracker* /*instance*/, ULONG_PTR /*user*/);

public:
	FlagTracker();
	~FlagTracker();
public:
	void Set(UINT nFlags, UINT nMask);
	UINT Get();
	void Mark(UINT nFlags);
	
	ULONG BeginUpdate();
	ULONG EndUpdate();

	void SetEventHandler(EventHandler handler, ULONG_PTR user);

protected:
	UINT flags;
	UINT cache;
	ULONG updateRef;
	CRITICAL_SECTION lock;
	EventHandler eventHandler;
	ULONG_PTR eventParam;
};

#endif //NULLSOFT_WINAMP_MODIFY_TRACKER_HEADER