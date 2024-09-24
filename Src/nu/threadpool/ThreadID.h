#pragma once
#include <windows.h>
#include "ThreadFunctions.h"
#include "threadpool_types.h"
#include <vector>


class ThreadID : private ThreadFunctions
{
public:
	static DWORD CALLBACK thread_func_stub(LPVOID param);
	ThreadID(ThreadFunctions *t_f, HANDLE killswitch, HANDLE global_functions_semaphore, ThreadPoolTypes::HandleList &inherited_handles, volatile LONG *thread_count, HANDLE _max_load_event, int _reserved, int _com_type);
	~ThreadID();
	void Kill();

	/* Try and Wait must be paired!!! */
	bool TryAddHandle(HANDLE new_handle);
	void WaitAddHandle(HANDLE new_handle);
	void AddHandle(HANDLE new_handle);
	
	/* Try and Wait must be paired!!! */
	bool TryRemoveHandle(HANDLE handle); 
	void WaitRemoveHandle(HANDLE handle);
	void RemoveHandle(HANDLE handle);
	
	using ThreadFunctions::QueueFunction;
	bool IsReserved() const;
	bool IsReleased() const;
	bool CanRunCOM(int flags) const; 
	void Reserve(); // re-reserves a released thread
	void Release(); // release a reversed thread
private:
	void RemoveHandle_Internal(HANDLE handle);
	DWORD CALLBACK ThreadFunction();

	int reserved;
	ThreadFunctions *global_functions;
	volatile LONG *num_threads_available;
	int com_type;
	bool released;

	ThreadFunctions local_functions;

	// list of handles we're waiting on
	typedef std::vector<HANDLE> HandleList;
	HandleList wait_handles;
	CRITICAL_SECTION handle_lock;

	// handles we create/own
	HANDLE threadHandle;
	HANDLE wakeHandle;

	// handles given to us
	HANDLE max_load_event;

};
