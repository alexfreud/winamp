/****************************************************************************
*
*   Module Title :     SystemDependant.c
*
*   Description  :     Miscellaneous system dependant functions.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <windows.h>
#include "pbdll.h"
#include "math.h"

#include "vp60dversion.h"

#include "quantize.h" 

/****************************************************************************
*  Macros
****************************************************************************/
#pragma warning(disable:4115)

#define MMX_ENABLED 1

/****************************************************************************
*  Imports
****************************************************************************/
extern unsigned int CPUFrequency;

extern void GetProcessorFlags ( INT32 *MmxEnabled, INT32 *XmmEnabled, INT32 *WmtEnabled );
extern void VP6_BuildQuantIndex_Generic ( QUANTIZER *pbi );
extern void VP6_BuildQuantIndex_ForMMX ( QUANTIZER *pbi );
extern void VP6_BuildQuantIndex_ForWMT ( QUANTIZER *pbi );

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_SetPbParam
 *
 *  INPUTS        :     PB_INSTANCE **pbi       : Pointer to decoder instance.
 *                      PB_COMMAND_TYPE Command : Command action specifier.
 *                      UINT32 *Parameter       : Command dependent value.
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *  
 *  FUNCTION      :     Generalised command interface to decoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void CCONV VP6_SetPbParam( PB_INSTANCE *pbi, PB_COMMAND_TYPE Command, uintptr_t Parameter )
{
#if defined(POSTPROCESS)
    switch ( Command )
    {
    case PBC_SET_CPUFREE:
    {
#if defined(_MSC_VER)
        double Pixels = pbi->Configuration.VideoFrameWidth * pbi->Configuration.VideoFrameHeight;
        double FreeMhz = pbi->ProcessorFrequency * Parameter / 100;
        double PixelsPerMhz = 100 * sqrt(1.0*Pixels) / FreeMhz;
#else
        double PixelsPerMhz = 100 *10;
#endif
        pbi->CPUFree = Parameter; 

        if( PixelsPerMhz > 150 )
            pbi->PostProcessingLevel = 0;
        else if( PixelsPerMhz > 100 )
            pbi->PostProcessingLevel = 8;
        else if( PixelsPerMhz > 90 )
            pbi->PostProcessingLevel = 4;
        else if( PixelsPerMhz > 80 )
            pbi->PostProcessingLevel = 5;
        else
            pbi->PostProcessingLevel = 6;
        break;
    }

    case PBC_SET_ADDNOISE:
        pbi->AddNoiseMode = Parameter;
        //SetAddNoiseMode(pbi->postproc, Parameter);
        break;

	case PBC_SET_REFERENCEFRAME:
		CopyFrame( pbi->postproc, (YUV_BUFFER_CONFIG *) Parameter, pbi->LastFrameRecon);
		CopyFrame( pbi->postproc, (YUV_BUFFER_CONFIG *) Parameter, pbi->GoldenFrame);
		break;

	case PBC_SET_POSTPROC:
        if( Parameter == 9 )                
            VP6_SetPbParam( pbi, PBC_SET_CPUFREE, 70);
        else
        {
            pbi->CPUFree = 0;
            pbi->PostProcessingLevel = Parameter;
        }
        break;

    case PBC_SET_DEINTERLACEMODE:
        pbi->DeInterlaceMode = Parameter;
        break;

    case PBC_SET_BLACKCLAMP:
        pbi->BlackClamp = Parameter;
        break;

    case PBC_SET_WHITECLAMP:
        pbi->WhiteClamp = Parameter;
        break;

    default:
        break;
    }
#endif
}
/****************************************************************************
 * 
 *  ROUTINE       : VP6_readTSC
 *
 *  INPUTS        : None.
 *
 *  OUTPUTS       : unsigned long *tsc : Pointer to returned counter value.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Reads the cpu time stamp counter.
 *
 *  SPECIAL NOTES : Since this function uses RDTSC instruction, which is 
 *					introduced in Pentium processor, this routine is only
 *					expected to work on Pentium and above.
 *
 ****************************************************************************/
#ifdef _M_AMD64 // For 64-bit apps
unsigned __int64 __rdtsc(void);
#pragma intrinsic(__rdtsc)
#define _RDTSC __rdtsc
#else // For 32-bit apps

#define _RDTSC_STACK(ts) \
	__asm rdtsc \
	__asm mov DWORD PTR [ts], eax \
	__asm mov DWORD PTR [ts+4], edx

__inline unsigned __int64 _inl_rdtsc32() {
	unsigned __int64 t;
	_RDTSC_STACK(t);
	return t;
}
#define _RDTSC _inl_rdtsc32
#endif


void VP6_readTSC(unsigned long *tsc)
{
	LARGE_INTEGER t;
	t.QuadPart = _RDTSC();
	*tsc = t.LowPart;

	return;
}
/****************************************************************************
 * 
 *  ROUTINE       : VP6_GetProcessorFrequency
 *
 *  INPUTS        : None.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : unsigned long: The processors' frequency (in MHz).
 *
 *  FUNCTION      : Check the Processor's working freqency.
 *
 *  SPECIAL NOTES : This function should only be used here. Limited tests 
 *					have verified it works till 166MHz Pentium with MMX. 
 *
 ****************************************************************************/
unsigned long VP6_GetProcessorFrequency()
{

	LARGE_INTEGER pf;						// Performance Counter Frequency
	LARGE_INTEGER startcount, endcount;		
	unsigned long tsc1, tsc2;

	// If the cpu does not support the high resolution counter, return 0
    unsigned long time1, time2;
	unsigned long cpufreq = 0;
    unsigned long Nearest66Mhz, Nearest50Mhz;
    unsigned long Delta66, Delta50;

	if ( QueryPerformanceFrequency( &pf ) )
	{
		// read the counter and TSC at start
		QueryPerformanceCounter ( &startcount );
		VP6_readTSC ( &tsc1 );

		// delay for 10 ms to get enough accuracy
        time1 = timeGetTime();
        time2 = time1;
        while ( time2 < time1+5 )
            time2 = timeGetTime();

		// read the counter and TSC at end
		QueryPerformanceCounter ( &endcount );
		VP6_readTSC ( &tsc2 );
		
		// calculate the frequency
		cpufreq = (unsigned long )( (double)(tsc2-tsc1) 
			            * (double)pf.LowPart 
			            / (double) (endcount.LowPart - startcount.LowPart) 
			            / 1000000 );
	}
   
    Nearest66Mhz = ((cpufreq * 3 + 100)/200 * 200) / 3;
    Delta66      = abs( Nearest66Mhz - cpufreq );
    Nearest50Mhz = ((cpufreq + 25)/50 *50);
    Delta50      = abs( Nearest50Mhz - cpufreq );

    if ( Delta50 < Delta66 )
        cpufreq = Nearest50Mhz;
    else
    {
        cpufreq = Nearest66Mhz;
        if ( cpufreq == 666 )
            cpufreq = 667;
    }
    return cpufreq;
}


/****************************************************************************
 * 
 *  ROUTINE       : VP6_DMachineSpecificConfig
 *
 *  INPUTS        : None.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Checks for machine specifc features such as MMX support;
 *                  sets approipriate flags and function pointers.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void VP6_DMachineSpecificConfig ( void )
{
#if 0
	INT32 MmxEnabled;
	INT32 XmmEnabled; 
	INT32 WmtEnabled;

	GetProcessorFlags ( &MmxEnabled, &XmmEnabled, &WmtEnabled );

	// If MMX supported use MMX version of functions, else use C versions
	if ( WmtEnabled )		// Willamette
		VP6_BuildQuantIndex = VP6_BuildQuantIndex_ForWMT;
	else if ( MmxEnabled )  // MMX
		VP6_BuildQuantIndex = VP6_BuildQuantIndex_ForMMX;
    else                    // No instruction set support
#endif
		VP6_BuildQuantIndex = VP6_BuildQuantIndex_Generic;
}

/****************************************************************************
 * 
 *  ROUTINE       : VP6_IssueWarning
 *
 *  INPUTS        : char *WarningMessage : Pointer to warning message text.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Issues a warning message.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void VP6_IssueWarning ( char *WarningMessage )
{
    MessageBox ( NULL, WarningMessage, NULL, MB_ICONEXCLAMATION | MB_TASKMODAL );
}

/****************************************************************************
 * 
 *  ROUTINE       : VP6_IssueWarning
 *
 *  INPUTS        : unsigned int SleepMs : Time (in milli-seconds) to wait.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Pause/Sleep for specified time(in milli-seconds).
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void VP6_PauseProcess ( unsigned int SleepMs )
{
    Sleep ( SleepMs );
}

/****************************************************************************
 * 
 *  ROUTINE       : VP6_SytemGlobalAlloc
 *
 *  INPUTS        : unsigned int Size : Size of block of memory (in bytes).
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : char *: Pointer to allocated block of memory.
 *
 *  FUNCTION      : Allocates a block of memory of specified size.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
char *VP6_SytemGlobalAlloc ( unsigned int Size )  
{
    return GlobalAlloc( GPTR, Size );  
}

/****************************************************************************
 * 
 *  ROUTINE       : VP6_SystemGlobalFree
 *
 *  INPUTS        : char *MemPtr : Pointer to block of memory.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : De-allocates a block of memory.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void VP6_SystemGlobalFree ( char* MemPtr )
{
    GlobalFree ( (HGLOBAL)MemPtr );
}
