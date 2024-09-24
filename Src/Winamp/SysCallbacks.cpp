/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author: Ben Allison benski@nullsoft.com
** Created:
**/
#include "main.h"
#include "SysCallbacks.h"
#include <api/syscb/callbacks/syscb.h>
#include "../nu/AutoLock.h"
using namespace Nullsoft::Utility;

SysCallbacks::SysCallbacks()
{
	reentry=0;
	inCallback=false;
}

//note: it's OK to add in the middle of an issueCallback
//because new callbacks go at the end of the list
//and the lockguard prevents list corruption
int SysCallbacks::syscb_registerCallback(SysCallback *cb, void *param)
{
	AutoLock lock(callbackGuard LOCKNAME("SysCallbacks::syscb_registerCallback"));
	int event_type = cb->getEventType();
	//CallbackList *&callbacks = callback_map[event_type];
	//if (!callbacks)
	//	callbacks = new CallbackList;
	//callbacks->push_back(cb); 
	EventMap::iterator find = callback_map.find(event_type);
	CallbackList* callbacks = 0;
	if (find != callback_map.end())
	{
		callbacks = find->second;
	}
	else
	{
		callbacks = new CallbackList();
		callback_map.insert({ event_type, callbacks });
	}

	if (callbacks)
	{
		callbacks->push_back(cb);
	}
	return 1;
}

int SysCallbacks::syscb_deregisterCallback(SysCallback *cb)
{
	AutoLock lock(callbackGuard LOCKNAME("SysCallbacks::syscb_deregisterCallback"));
	if (inCallback)
		deleteMeAfterCallbacks.push_back(cb);
	else
	{
		int event_type = cb->getEventType();
		EventMap::iterator find = callback_map.find(event_type);
		if (find != callback_map.end())
		{
			CallbackList *callbacks = find->second;
			if (callbacks)
			{
				//callbacks->eraseAll(cb);
				auto it = callbacks->begin();
				while ( it != callbacks->end())
				{
					if (*it != cb)
					{
						it++;
						continue;
					}

					it = callbacks->erase(it);
				}
			}
		}
	}
	return 1;
}

int SysCallbacks::syscb_issueCallback(int event_type, int msg, intptr_t param1 , intptr_t param2)
{
	AutoLock lock(callbackGuard LOCKNAME("SysCallbacks::syscb_issueCallback"));
	reentry++;
	inCallback=true;
	EventMap::iterator find = callback_map.find(event_type);
	if (find != callback_map.end())
	{
		CallbackList *callbacks = find->second;
		if (callbacks)
		{
			for (size_t i=0;i<callbacks->size();i++)
			{
				SysCallback *callback = callbacks->at(i);
				//if (!deleteMeAfterCallbacks.contains(callback))
				//	callback->notify(msg, param1, param2);
				bool found = false;
				for (auto obj : deleteMeAfterCallbacks)
				{
					if (obj == callback)
					{
						found = true;
						break;
					}
				}

				if (!found)
					callback->notify(msg, param1, param2);
			}
		}
	}

	inCallback=false;
	reentry--;
	if (reentry==0)
	{
		for ( SysCallback *l_sys_call_back : deleteMeAfterCallbacks )
		{
			for (EventMap::iterator itr=callback_map.begin(); itr != callback_map.end(); itr++)
			{
				CallbackList *callbacks = itr->second;
				if (callbacks)
				{
					//callbacks->eraseAll(cb);
					auto it = callbacks->begin();
					while (it != callbacks->end())
					{
						if (*it != l_sys_call_back )
						{
							it++;
							continue;
						}

						it = callbacks->erase(it);
					}
				}
			}
		}
		deleteMeAfterCallbacks.clear();
	}
	return 1;
}

SysCallback *SysCallbacks::syscb_enum(int event_type, size_t n)
{
	AutoLock lock(callbackGuard LOCKNAME("SysCallbacks::syscb_enum"));
	// TODO: maybe check !deleteMeAfterCallbacks.contains(callbacks[i])
	if (event_type)
	{
		EventMap::iterator find = callback_map.find(event_type);
		if (find != callback_map.end())
		{
			CallbackList *callbacks = find->second;
			if (callbacks)
			{
				if (n <= callbacks->size())
				{
					SysCallback *callback = callbacks->at(n);
					if (callback)
						callback->AddRef(); 	// benski> don't be fooled.  most objects don't actually support reference counting
					return callback;

				}
			}
		}
	}
	else
	{
		// enumerates ALL syscallbacks
		for (EventMap::iterator itr=callback_map.begin(); itr != callback_map.end(); itr++)
		{
			CallbackList *callbacks = itr->second;
			if (callbacks)
			{
				if (n >= callbacks->size())
				{
					n-=callbacks->size();
				}
				else
				{
					SysCallback *callback = callbacks->at(n);
					if (callback)
						callback->AddRef(); 	// benski> don't be fooled.  most objects don't actually support reference counting
					return callback;
				}
			}
		}
	}
	return 0;
}


#define CBCLASS SysCallbacks
START_DISPATCH;
CB(API_SYSCB_SYSCB_REGISTERCALLBACK, syscb_registerCallback)
CB(API_SYSCB_SYSCB_DEREGISTERCALLBACK, syscb_deregisterCallback)
CB(API_SYSCB_SYSCB_ISSUECALLBACK, syscb_issueCallback)
CB(API_SYSCB_SYSCB_ENUM, syscb_enum)
END_DISPATCH;
#undef CBCLASS