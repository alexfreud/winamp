#pragma warning (disable:4786)
#ifndef NONBLOCKLOCKH
#define NONBLOCKLOCKH
#include <windows.h>


/*
NULLSOFT_LOCK_OUTPUT_STATUS turns on/off debugging output
this can be VERY useful if you are trying to find a deadlock
each time the guard is locked or unlocked, it outputs a list of
any threads using the mutex, and their function stack
*/
#define NULLSOFT_LOCK_OUTPUT_STATS



#ifdef NULLSOFT_LOCK_OUTPUT_STATS

#include <string> // we save each function name as a string
#include <deque> // we make a list of the recursive function stack for each thread
#include <map> // and map
#include <iostream> // we output to std::cerr
#include <windows.h>
#endif

/*****
Description:
This class uses scoping to wrap a critical section (lightweight in-process mutex)
The constructor enters the mutex and the destructor leaves it.  This allows it to
take advantage of automatic scoping in C++, because C++ automatically calls the destructor
when an object leaves scope. 

This is _especially_ useful when you have multiple return paths, since you don't have to
repeat mutex-leaving code.

To use:
Make a LockGuard for a resource you want to protect.  The guard is shared, so make it part 
of your class, or a global, or whatever.  The LockGuard is essentially a "token", equivalent
to your mutex handle or critical section handle.

Make an AutoLock object on the stack to lock.  It will unlock automatically when the object
leaves scope.  

Note: You'll want to make an object on the stack - don't use a heap object (new/delete)
unless you have weird requirements and know what you are doing.

Example:

class MyClass
{
LockGuard fileGuard;
fstream file;
void DumpSomeData() // 
{
AutoLock lock(fileGuard);
file << GetData();
}

void CALLBACK NewData() // potentially called by another thread
{
AutoLock lock(fileGuard)
file << newData;
}
};


Tip: You can use "false scoping" to tweak the mutex lifetime, for example:

void DoStuff()
{
a = GetData();
{ // false scope
AutoLock lock(dataGuard);
DoCalculationsWith(a);
} // mutex will release here
SetData(a);
}

Tip: A common mistake is making a temporary object.
i.e.
CORRECT:  AutoLock lock(fileGuard); // an AutoLock object called "lock" is put on the stack
INCORRECT: AutoLock(fileGuard); // An unnamed temporary is created which will be destroyed IMMEDIATELY

*******/

namespace Nullsoft
{
	namespace Utility
	{
		class NonBlockLock;
		/* the token which represents a resource to be locked */
		class NonBlockLockGuard
		{
		public:
			friend class NonBlockLock;
			inline NonBlockLockGuard(char *name = "Unnamed Guard")
			{
#ifdef NULLSOFT_LOCK_OUTPUT_STATS
				lockName = name;
				InitializeCriticalSection(&cerr_cs);
				InitializeCriticalSection(&map_cs);
#endif
				event=CreateEvent(NULL, FALSE, TRUE, NULL);
				ownerThread=-1;
				InitializeCriticalSection(&threads_cs);
			}
			inline ~NonBlockLockGuard()
			{
#ifdef NULLSOFT_LOCK_OUTPUT_STATS
				DeleteCriticalSection(&cerr_cs);
				DeleteCriticalSection(&map_cs);
#endif
				CloseHandle(event);
				DeleteCriticalSection(&threads_cs);
			}
		private:
			inline bool Lock()
			{
				HRESULT hr;
				EnterCriticalSection(&threads_cs);
				hr=WaitForSingleObject(event, 0);
				if (hr == WAIT_TIMEOUT && ownerThread==GetCurrentThreadId())
				{
					LeaveCriticalSection(&threads_cs);
					return false;
				}
				else if (hr == WAIT_OBJECT_0)
				{
					ownerThread=GetCurrentThreadId();
					LeaveCriticalSection(&threads_cs);
					return true;
				}
				LeaveCriticalSection(&threads_cs);

				do
				{
					EnterCriticalSection(&threads_cs);
					if (WaitForSingleObject(event, 3)==WAIT_OBJECT_0)
					{
						ownerThread=GetCurrentThreadId();
						LeaveCriticalSection(&threads_cs);
						break;
					}
					else
					{
						LeaveCriticalSection(&threads_cs);
						MSG msg;
					while(PeekMessage(&msg, NULL, 0, 0, 1))
					{
						//TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					Sleep(3);
					}
				} while(true);
				return true;

			}

			inline void Unlock()
			{
				//LeaveCriticalSection(&m_cs);

				EnterCriticalSection(&threads_cs);
				ownerThread=-1;
				SetEvent(event);
				LeaveCriticalSection(&threads_cs);
			}

#ifdef NULLSOFT_LOCK_OUTPUT_STATS

			int ThreadCount()
			{
				EnterCriticalSection(&map_cs);
				int count = 0;
				for (ThreadMap::iterator itr = threads.begin(); itr != threads.end(); itr++)
				{
					if (!itr->second.empty())
						count++;
				}

				LeaveCriticalSection(&map_cs);
				return count;
			}

			void Display()
			{
				EnterCriticalSection(&map_cs);
				EnterCriticalSection(&cerr_cs);

				if (ThreadCount() > 1 && owner)
				{

					std::cerr << "Guard: " << lockName << std::endl;
					for (ThreadMap::iterator itr = threads.begin(); itr != threads.end(); itr++)
					{
						if (itr->second.empty())
							continue;

						std::cerr << "  Thread ID: " << std::hex << itr->first << std::dec;
						if (owner == itr->first)
							std::cerr << " [holding the mutex] *****";
						else
							std::cerr << " [blocked]";
						std::cerr << std::endl;
						for (FunctionStack::iterator fitr = itr->second.begin(); fitr != itr->second.end(); fitr++)
						{
							std::cerr << "    " << *fitr << "();" << std::endl;
						}

					}
				}
				LeaveCriticalSection(&cerr_cs);
				LeaveCriticalSection(&map_cs);
			}

			void In(DWORD thread, char *functionName)
			{
				EnterCriticalSection(&map_cs);
				threads[thread].push_back(functionName);
				LeaveCriticalSection(&map_cs);
			}

			void Out(DWORD thread)
			{
				EnterCriticalSection(&map_cs);
				threads[thread].pop_back();
				LeaveCriticalSection(&map_cs);
			}
			std::string lockName;
			CRITICAL_SECTION cerr_cs, map_cs;
			typedef std::deque<std::string> FunctionStack; // this typedef reduce ugly c++ <>::<>::<> overkill
			typedef std::map<DWORD, FunctionStack> ThreadMap;
			ThreadMap threads;

			DWORD owner;
#endif
		private:
			//CRITICAL_SECTION m_cs;
			CRITICAL_SECTION threads_cs;
			HANDLE event;
			DWORD ownerThread;



		};

		/* an AutoLock locks a resource (represented by a LockGuard) for the duration of its lifetime */
		class NonBlockLock
		{
		public:
			/*
			@param functionName The function name which wants the mutex
			we pass it in as a char * even though it'll be converted to a std::string
			to reduce overhead when OUTPUT_STATS is off
			*/
			inline NonBlockLock(NonBlockLockGuard &_guard, char *functionName = "function name not passed") : guard(&_guard), owner(false)
			{
#ifdef NULLSOFT_LOCK_OUTPUT_STATS
				thisThread = GetCurrentThreadId();
				guard->In(thisThread, functionName);
				guard->Display();
#endif

				owner=guard->Lock();

#ifdef NULLSOFT_LOCK_OUTPUT_STATS
				guard->owner = thisThread;
				guard->Display();
#endif

			}

			inline void ManualLock(char *functionName = "manual lock")
			{
#ifdef NULLSOFT_LOCK_OUTPUT_STATS
				thisThread = GetCurrentThreadId();
				guard->In(thisThread,functionName);
				guard->Display();
#endif

				owner=guard->Lock();

#ifdef NULLSOFT_LOCK_OUTPUT_STATS
				guard->owner = thisThread;
				guard->Display();
#endif
			}

			inline void ManualUnlock()
			{
				#ifdef NULLSOFT_LOCK_OUTPUT_STATS
				guard->Display();
#endif
				if (owner)
					guard->Unlock();

				owner=false;
#ifdef NULLSOFT_LOCK_OUTPUT_STATS

				InterlockedCompareExchange((LONG *)(void *)&guard->owner, 0, (LONG)thisThread);
				/* above line is functionally equivalent to:
				if (guard->owner == thisThread) 
				guard->owner=0;
				*/
				guard->Out(thisThread);
				guard->Display();
#endif
			}

			inline ~NonBlockLock()
			{
#ifdef NULLSOFT_LOCK_OUTPUT_STATS
				guard->Display();
#endif
				if (owner)
					guard->Unlock();
#ifdef NULLSOFT_LOCK_OUTPUT_STATS

				InterlockedCompareExchange((LONG *)(void *)&guard->owner, 0, (LONG)thisThread);
				/* above line is functionally equivalent to:
				if (guard->owner == thisThread) 
				guard->owner=0;
				*/
				guard->Out(thisThread);
				guard->Display();
#endif

			}

			NonBlockLockGuard *guard;
			bool owner;
#ifdef NULLSOFT_LOCK_OUTPUT_STATS
			DWORD thisThread;
#endif

		};


	}
}

#endif
