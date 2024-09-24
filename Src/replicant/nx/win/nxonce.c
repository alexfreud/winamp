#include "nxonce.h"
#include "foundation/error.h"
#if 0 && _WIN32_WINNT >= 0x600

void NXOnce(nx_once_t once, int (NX_ONCE_API *init_fn)(nx_once_t, void *, void **), void *param)
{
	InitOnceExecuteOnce(once, (PINIT_ONCE_FN)init_fn, param, 0);
}

void NXOnceInit(nx_once_t once)
{
	InitOnceInitialize(once);
}
#else
/* this ONLY works because of the strict(ish) memory ordering of the AMD64/x86 processors. 
 Don't use this implementation for a processor that has loose memory ordering restriction (e.g. ARM, PowerPC)
 see http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf
 */
void NXOnce(nx_once_t once, int (NX_ONCE_API *init_fn)(nx_once_t, void *, void **), void *param)
{
    if (once->status)
        return;
    
    EnterCriticalSection(&once->critical_section);
    if (once->status)
    {
        LeaveCriticalSection(&once->critical_section);
        return;
    }
    
    init_fn(once, param, 0);
    // benski> not important for the x86, but on processors with weak memory-order on stores, once->status might set to 1 BEFORE all stores from init_fn complete!
    once->status = 1;
    LeaveCriticalSection(&once->critical_section);
}

void NXOnceInit(nx_once_t once)
{
	once->status=0;
	InitializeCriticalSection(&once->critical_section);	
}

#endif