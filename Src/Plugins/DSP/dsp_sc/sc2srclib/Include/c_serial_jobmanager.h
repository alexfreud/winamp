#ifndef __C_SERIAL_JOBMANAGER_H__
#define __C_SERIAL_JOBMANAGER_H__

#include "c_jobmanager.h"

template<class T> class C_SERIAL_JOBMANAGER : public C_JOBMANAGER<T> {
private:
	int currentJob;
public:
	C_SERIAL_JOBMANAGER() {
		currentJob = 0;
    }
    ~C_SERIAL_JOBMANAGER() { }
    int GetCurrentJob() { return currentJob; }
    virtual void Run(int passes = 1) {
		int numPasses = passes;
		while(numPasses-- > 0) {
			C_JOBMANAGER<T>::Run(currentJob++);
			if(currentJob > GetNumJobs()) currentJob = 0;
		}
	}
};

#endif // !__C_SERIAL_JOBMANAGER_H__