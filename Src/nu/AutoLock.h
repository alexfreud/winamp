#pragma warning (disable:4786)
#ifndef AUTOLOCKH
#define AUTOLOCKH

#ifdef _WIN32
#include <windows.h>
#else
#include <CoreServices/CoreServices.h>
#endif

/*
NULLSOFT_LOCK_OUTPUT_STATUS turns on/off debugging output
this can be VERY useful if you are trying to find a deadlock
each time the guard is locked or unlocked, it outputs a list of
any threads using the mutex, and their function stack
*/ 
//#define NULLSOFT_LOCK_OUTPUT_STATS

#ifdef NULLSOFT_LOCK_OUTPUT_STATS

#include <string> // we save each function name as a string
#include <deque> // we make a list of the recursive function stack for each thread
#include <map> // and map
#include <iostream> // we output to std::cerr
#include <windows.h>


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
#define MANUALLOCKNAME(x) x
#define LOCKNAME(x) ,x
#define GUARDNAME(x) (x)
namespace Nullsoft
{
	namespace Utility
	{
		/* the token which represents a resource to be locked */
		class LockGuard
		{
		public:
			inline LockGuard(char *name = "Unnamed Guard") : lockName(name), owner(0)
			{
				InitializeCriticalSection(&cerr_cs);
				InitializeCriticalSection(&map_cs);
				InitializeCriticalSection(&m_cs);
			}

			inline LockGuard(DWORD spin_count,char *name = "Unnamed Guard") : lockName(name), owner(0)
			{
				InitializeCriticalSection(&cerr_cs);
				InitializeCriticalSection(&map_cs);
				InitializeCriticalSection(&m_cs);
			}

			inline ~LockGuard()
			{
				DeleteCriticalSection(&cerr_cs);
				DeleteCriticalSection(&map_cs);
				DeleteCriticalSection(&m_cs);
			}

			inline void Lock()
			{
				EnterCriticalSection(&m_cs);
			}

			inline void Unlock()
			{
				LeaveCriticalSection(&m_cs);
			}

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
					wchar_t disp[256];
					wsprintfW(disp, L"Guard: %S\r\n", lockName.c_str());
					OutputDebugStringW(disp);
					for (ThreadMap::iterator itr = threads.begin(); itr != threads.end(); itr++)
					{
						if (itr->second.empty())
							continue;


						wsprintfW(disp, L"  Thread ID: %x", itr->first);
						if (owner == itr->first)
							wcscat(disp, L" [holding the mutex] *****\r\n");
						else
							wcscat(disp, L" [blocked]\r\n");
						OutputDebugStringW(disp);
						for (FunctionStack::iterator fitr = itr->second.begin(); fitr != itr->second.end(); fitr++)
						{
							wsprintfW(disp, L"    %S();\r\n", fitr->c_str());
							OutputDebugStringW(disp);
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
		private:
			CRITICAL_SECTION m_cs;

		};

		/* an AutoLock locks a resource (represented by a LockGuard) for the duration of its lifetime */
		class AutoLock
		{
		public:
			/*
			@param functionName The function name which wants the mutex
				we pass it in as a char * even though it'll be converted to a std::string
				to reduce overhead when OUTPUT_STATS is off
			 */
			inline AutoLock(LockGuard &_guard, char *functionName = "function name not passed") : guard(&_guard)
			{
				ManualLock(functionName);
			}
			inline void ManualLock(char *functionName = "manual lock")
			{
				thisThread = GetCurrentThreadId();
				guard->In(thisThread, functionName);
				guard->Display();
				guard->Lock();
				guard->owner = thisThread;
				guard->Display();
			}

			inline void ManualUnlock()
			{
				guard->Display();
				guard->Unlock();

				InterlockedCompareExchange((LONG volatile *)&guard->owner, 0, (LONG)thisThread);
				/* above line is functionally equivalent to:
				if (guard->owner == thisThread) 
					guard->owner=0;
				*/
				guard->Out(thisThread);
				guard->Display();
			}

			inline ~AutoLock()
			{
				ManualUnlock();
			}

			LockGuard *guard;
			DWORD thisThread;
		};

	}
}


#else
#define MANUALLOCKNAME(x)
#define LOCKNAME(x)
#define GUARDNAME(x)
namespace Nullsoft
{
	namespace Utility
	{
		/* the token which represents a resource to be locked */
		class LockGuard
		{
		public:
			inline LockGuard(char *guardName = "")
			{
      #ifdef _WIN32
					InitializeCriticalSection(&m_cs);
          #else
           MPCreateCriticalRegion(&cr);
          #endif
			}
#if _WIN32_WINNT >= 0x403
			inline LockGuard(DWORD spin_count, char *guardName = "")
			{
				if (spin_count)
					InitializeCriticalSectionAndSpinCount(&m_cs, spin_count);
				else
					InitializeCriticalSection(&m_cs);
			}
#endif
			inline ~LockGuard()
			{
      #ifdef _WIN32
				DeleteCriticalSection(&m_cs);
			#else
      MPDeleteCriticalRegion(cr);
      #endif
      }
			inline void Lock()
			{
      #ifdef _WIN32
				EnterCriticalSection(&m_cs);
			#else
        MPEnterCriticalRegion(cr, kDurationForever);
      #endif
      }

			inline void Unlock()
			{
      #ifdef _WIN32
				LeaveCriticalSection(&m_cs);
			#else
        MPExitCriticalRegion(cr);
      #endif
      }
		private:
    #ifdef _WIN32
			CRITICAL_SECTION m_cs;
		#else
			MPCriticalRegionID cr;
    #endif
			LockGuard(const LockGuard &copy) { } // make copy constructor private so it can't be used
			LockGuard &operator =(const LockGuard &copy) {} // same with operator=
    };

		/* an AutoLock locks a resource (represented by a LockGuard) for the duration of its lifetime */
		class AutoLock
		{
		public:
			inline AutoLock(LockGuard &_guard) : guard(&_guard)
			{
				guard->Lock();
			}
			
			inline AutoLock(LockGuard *_guard) : guard(_guard)
			{
				guard->Lock();
			}
			inline void ManualLock()
			{
				guard->Lock();
			}

			inline void ManualUnlock()
			{
				guard->Unlock();

			}
			inline ~AutoLock()
			{
				guard->Unlock();
			}
			LockGuard *guard;
		};

		// will lock anything that implements Lock() and Unlock()
		template <class LockGuard_t>
		class AutoLockT
		{
		public:
			inline AutoLockT(LockGuard_t &_guard) : guard(&_guard)
			{
				guard->Lock();
			}
			
			inline AutoLockT(LockGuard_t *_guard) : guard(_guard)
			{
				guard->Lock();
			}
			inline void ManualLock()
			{
				guard->Lock();
			}

			inline void ManualUnlock()
			{
				guard->Unlock();

			}
			inline ~AutoLockT()
			{
				guard->Unlock();
			}
			LockGuard_t *guard;
		};
	}
}
#endif

#endif