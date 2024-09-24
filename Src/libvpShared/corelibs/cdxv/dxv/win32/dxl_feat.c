#include "dkpltfrm.h"
#include "dxl_main.h"                                            

unsigned long cpuFeatures;

#define CHECK_MMX       0x800000
#define CHECK_TSC       0x10
#define CHECK_CMOV      0x8000
#define CHECK_FCMOV     0x10000

CPU_FEATURES DXL_GetCPUFeatures(void)
{
    enum CPU_FEATURES currentFeatures = NO_FEATURES;

    if(cpuFeatures & CHECK_MMX)
        currentFeatures |= MMX_SUPPORTED;
        
    return currentFeatures;
}              

