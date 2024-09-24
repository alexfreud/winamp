#ifndef NULLSOFT_BUFFERCACHEH
#define NULLSOFT_BUFFERCACHEH
#include <time.h>
#include "../nu/GrowBuf.h"

class Buffer_GrowBuf : public GrowBuf
{
public:
	Buffer_GrowBuf() : expire_time(0) {}
	time_t expire_time;
};

#endif