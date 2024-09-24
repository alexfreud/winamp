#ifndef __C_JOBMANAGER_H__
#define __C_JOBMANAGER_H__

#include <vector>

#ifdef _WIN32
#include <wtypes.h>
#include <winbase.h>  // for mutex support
#define T_MUTEX HANDLE
#else // _WIN32
#error "This won't compile under anything other than windows since I haven't implemented mutexing on anything else"
#endif // _WIN32

template<class T> class C_JOBMANAGER {
public:
	typedef int (*T_JOBHANDLER)(int state, int last_state, T *userData);
private:
	struct T_JOB {
		int state;
		int last_state;
		int suspended;
		T *userData;
	};
	struct T_HANDLER {
		int state;
		int last_state;
		T_JOBHANDLER jobHandler;
	};
	std::vector<T_JOB*> JobList;
	std::vector<T_HANDLER*> HandlerList;
	T_MUTEX mutex;

protected:
	int waitForMutex() {
#ifdef _WIN32
		if(WaitForSingleObject(mutex,INFINITE) == WAIT_OBJECT_0) return 1;
#else // _WIN32
		// insert mutex magic here
#endif // _WIN32
		return 0;
	}

    void releaseMutex() {
#ifdef _WIN32
		ReleaseMutex(mutex);
#else // _WIN32
		// insert mutex magic here
#endif // _WIN32
	}

public:
	C_JOBMANAGER() {
#ifdef _WIN32
		mutex = CreateMutex(NULL,TRUE,NULL);
		ReleaseMutex(mutex);
#else // _WIN32
		// insert mutex magic here
#endif // _WIN32
    }

    virtual ~C_JOBMANAGER() {
		waitForMutex();
#ifdef _WIN32
		CloseHandle(mutex);
		mutex = NULL;
#else // _WIN32
		// insert mutex magic here
#endif // _WIN32

		//JobList.deleteAll();
		for (auto job : JobList)
		{
			delete job;
		}
		JobList.clear();
		
		//HandlerList.deleteAll();
		for (auto handler : HandlerList)
		{
			delete handler;
		}
		HandlerList.clear();
    }

    T *operator[](int job) {
		if(!waitForMutex()) return NULL;
		T_JOB *j = JobList[job];
		T *val = NULL;
		if(j) val = j->userData;
		releaseMutex();
		return val;
    }

    virtual int AddJob(int state, T *userData, int suspended = 0, int isUserUnique = 1) {
		if(!waitForMutex()) return -1;
		int n = JobList.size();
		if(isUserUnique && n) {
			for(int i = n-1; i >= 0; i--) {
			T_JOB *item = JobList[i];
				if(item) {
					if(item->userData == userData) {
						releaseMutex();
						return -1;
					}
				}
			}
		}
		T_JOB *job = new T_JOB;
		job->last_state = OUT_DISCONNECTED;
		job->state = state;
		job->suspended = suspended;
		job->userData = userData;
		JobList.push_back(job);
		releaseMutex();
		return n;
	}

    virtual int GetJobState(int job) {
		int retval = -1;
		if(waitForMutex()) {
			int n = JobList.size();
			if(job < n && job >= 0) retval = JobList[job]->state;
			releaseMutex();
		}
		return retval;
    }

    virtual void SetJobState(int job, int state) {
		if(!waitForMutex()) return;
		int n = JobList.size();
		if(job < n && job >= 0) JobList[job]->state = state;
		releaseMutex();
    }

    virtual void SuspendJob(int job, int suspended) {
		if(!waitForMutex()) return;
		int n = JobList.size();
		if(job < n && job >= 0) JobList[job]->suspended = suspended;
		releaseMutex();
    }

    virtual void DelJob(int job) {
		if(!waitForMutex()) return;
		int n = JobList.size();
		if(job < n && job >= 0) {
			delete JobList[job];
			JobList.erase(JobList.begin() + job);
		}
		releaseMutex();
    }

    virtual void ClearJobs() {
		if(!waitForMutex()) 
			return;
		
		//JobList.deleteAll();
		for (auto job : JobList)
		{
			delete job;
		}
		JobList.clear();

		releaseMutex();
    }

    virtual void AddHandler(int state, T_JOBHANDLER jobHandler) {
		if(!waitForMutex()) return;
		int n = HandlerList.size();
		for(int i = n-1; i >= 0; i--) {
		T_HANDLER *item = HandlerList[i];
			if(item) {
				if(item->state == state) {
					releaseMutex();
					return;
				}
			}
		}
		T_HANDLER *handler = new T_HANDLER;
		handler->state = state;
		handler->jobHandler = jobHandler;
		HandlerList.push_back(handler);
		releaseMutex();
    }

    virtual void DelHandler(int state) {
		if(!waitForMutex()) return;
		int n = HandlerList.size();
		for(int i = n-1; i >= 0; i--) {
		T_HANDLER *item = HandlerList[i];
			if(item) {
				if(item->state == state) {
					delete HandlerList[i];
					HandlerList.erase(HandlerList.begin() + i);
					releaseMutex();
					return;
				}
			}
		}
		releaseMutex();
    }

    virtual void ClearHandlers() {
		if(!waitForMutex()) 
			return;
		
		//HandlerList.deleteAll();
		for (auto handler : HandlerList)
		{
			delete handler;
		}
		HandlerList.clear();

		releaseMutex();
    }

    virtual int GetNumJobs() {
		if(!waitForMutex()) return -1;
		int n = JobList.size();
		releaseMutex();
		return n;
    }

    virtual int GetNumHandlers() {
		if(!waitForMutex()) return -1;
		int n = HandlerList.size();
		releaseMutex();
		return n;
    }

    virtual void Run(int job) {
		if(!waitForMutex()) return;
		int nJ = JobList.size();
		int nH = HandlerList.size();
		if(job < nJ && job >= 0) {
		T_JOB *job_item = JobList[job];
			for(int i = nH-1; i >= 0; i--) {
			T_HANDLER *handler = HandlerList[i];
				if(handler) {
					if(handler->state == job_item->state) {
						if(!job_item->suspended) {
							int cur_state = job_item->state;
							job_item->state = handler->jobHandler(job_item->state,job_item->last_state,job_item->userData);
							job_item->last_state = cur_state;
						}
						break;
					}
				}
			}
		}
		releaseMutex();
	}
};

#endif // !__C_JOBMANAGER_H__