#include "cpucount.h"

#ifdef _WIN32
#include <windows.h>

int cpucount() throw() 
{
	SYSTEM_INFO sysinfo = {0};
	::GetSystemInfo(&sysinfo);

	return sysinfo.dwNumberOfProcessors;
}
#endif

#ifdef __APPLE_CC__
#import <sys/param.h>
#import <sys/sysctl.h>

int cpucount() throw() 
{ 
	int count = 0;
	size_t size = sizeof(count);

	if (sysctlbyname("hw.ncpu",&count,&size,NULL,0)) return 1;

	return count; 
}
#endif

#if (defined PLATFORM_LINUX || defined PLATFORM_ARMv6 || defined PLATFORM_ARMv7)
#include <unistd.h>
int cpucount() throw() 
{ 
	return sysconf(_SC_NPROCESSORS_ONLN);
}
#endif

#ifdef PLATFORM_BSD
#include <sys/param.h>
#include <sys/sysctl.h>
#ifndef HW_AVAILCPU
#define HW_AVAILCPU 25
#endif

int cpucount() throw() 
{ 
	int numCPU = 1;
	int mib[4] = {0};
	size_t len = 0; 

	/* set the mib for hw.ncpu */
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

	/* get the number of CPUs from the system */
	sysctl(mib, 2, &numCPU, &len, NULL, 0);

	if( numCPU < 1 ) 
	{
		mib[1] = HW_NCPU;
		sysctl( mib, 2, &numCPU, &len, NULL, 0 );

		if( numCPU < 1 )
		{
			numCPU = 1;
		}
	}
	return numCPU; 
}
#endif
