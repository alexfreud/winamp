#include "SysCallbacks.h"
#include "syscb/ifc_syscallback.h"

using namespace nu;

SysCallbacks::SysCallbacks() 
{
	reentry=0;
	inCallback=false;
}

//note: it's OK to add in the middle of an issueCallback
//because new callbacks go at the end of the list
//and the lockguard prevents list corruption
int SysCallbacks::SysCallbacks_RegisterCallback(ifc_sysCallback *cb)
{
	AutoLock lock(callbackGuard LOCKNAME("SysCallbacks::syscb_registerCallback"));
	callbacks.push_back(cb); 
	return 0;
}

int SysCallbacks::SysCallbacks_UnregisterCallback(ifc_sysCallback *cb)
{
	AutoLock lock(callbackGuard LOCKNAME("SysCallbacks::syscb_deregisterCallback"));
	if (inCallback)
		deleteMeAfterCallbacks.push_back(cb);
	else
	{
		//callbacks.eraseAll(cb);
		auto it = callbacks.begin();
		while (it != callbacks.end())
		{
			if (*it != cb)
			{
				it++;
				continue;
			}

			it = callbacks.erase(it);
		}
	}
	return 0;
}

int SysCallbacks::SysCallbacks_IssueCallback( GUID eventtype, int msg, intptr_t param1, intptr_t param2 )
{
	AutoLock lock( callbackGuard LOCKNAME( "SysCallbacks::syscb_issueCallback" ) );
	reentry++;
	inCallback = true;

	for ( ifc_sysCallback *l_call_back : callbacks )
	{
		//if (!deleteMeAfterCallbacks.contains(callbacks[i]) && callbacks[i]->GetEventType() == eventtype)
		if ( deleteMeAfterCallbacks.end() == std::find( deleteMeAfterCallbacks.begin(), deleteMeAfterCallbacks.end(), l_call_back ) && l_call_back->GetEventType() == eventtype )
			l_call_back->Notify( msg, param1, param2 );
	}
	inCallback = false;
	reentry--;
	if ( reentry == 0 )
	{
		for ( ifc_sysCallback *l_delete_me_after_call_back : deleteMeAfterCallbacks )
		{
			//callbacks.eraseAll(deleteMeAfterCallbacks[i]);
			auto it = callbacks.begin();
			while ( it != callbacks.end() )
			{
				if ( *it != l_delete_me_after_call_back )
				{
					it++;
					continue;
				}

				it = callbacks.erase( it );
			}
		}
		deleteMeAfterCallbacks.clear();
	}
	return 0;
}

ifc_sysCallback *SysCallbacks::SysCallbacks_Enum( GUID eventtype, size_t n )
{
	AutoLock lock( callbackGuard LOCKNAME( "SysCallbacks::syscb_enum" ) );
	// TODO: maybe check !deleteMeAfterCallbacks.contains(callbacks[i])
	for ( ifc_sysCallback *callback : callbacks )
	{
		if ( callback->GetEventType() == eventtype )
		{
			if ( n-- == 0 )
			{
				// benski> don't be fooled.  most objects don't actually support reference counting
				callback->Retain();
				return callback;
			}
		}
	}
	return 0;
}
