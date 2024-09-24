#ifndef BENCHMARKH
#define BENCHMARKH


#if defined(_WIN32)
static uint64_t Benchmark()
{
	return GetTickCount64();
}

#elif defined(__ANDROID__)
#include <time.h>

// Make sure to divide by 1000000 (1 million) to get results in Milliseconds, as they are returned in nanoseconds
static uint64_t Benchmark()
{
	struct timespec ts;
	uint64_t count;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
	count=(uint64_t)ts.tv_sec*1000000000ULL + (uint64_t)ts.tv_nsec;
	return count;
}

#elif defined(__APPLE__)
#include <mach/mach_time.h>
static uint64_t Benchmark()
{
	uint64_t absoluteTime;
	static mach_timebase_info_data_t timeBase = {0,0};
	
	absoluteTime = mach_absolute_time();
	
	if (0 == timeBase.denom)
	{
		kern_return_t err = mach_timebase_info(&timeBase);
		if (0 != err)
			return 0;
	}
	uint64_t nanoTime = absoluteTime * timeBase.numer / timeBase.denom;
	return nanoTime/(1000*1000);
}
#endif

#endif