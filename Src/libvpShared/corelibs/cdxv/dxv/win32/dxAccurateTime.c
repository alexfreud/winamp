#include "dkpltfrm.h"


static void readTSC(UINT64 *bigOne)
{
	unsigned long xhigh;
	unsigned long xlow;
	
	__asm 
	{
		
		rdtsc
			
		mov [xlow],EAX;
		mov [xhigh],edx;
		
	}
	
	*bigOne =  xhigh ;
	*bigOne <<= 32;
	*bigOne |= xlow;
	
	return;
}


void DXL_AccurateTime(UINT64 *temp)
{
	readTSC(temp);
}
