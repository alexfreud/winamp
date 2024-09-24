/****************************************************************************
*
*   Module Title :     SystemDependant.c
*
*   Description  :     Miscellaneous system dependant functions
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
* 
*   1.19 YWX 15-Jun-01 added function pointer setups for new deblocking filter
*   1.18 YWX 26-Apr-01 Fixed the cpu frequency detection bug caused by Sleep()
*   1.17 JBX 22-Mar-01 Merged with new vp4-mapca bitstream
*   1.16 JBB 26-Jan-01 Cleaned out unused function
*   1.15 YWX 08-dec-00 Added WMT PostProcessor and 
*                        moved function declarations into _head files
*   1.14 JBB 30 NOV 00 Version number changes 
*   1.13 YWX 03-Nov-00 Optimized postprocessor filters
*   1.12 YWX 02-Nov-00 Added new loopfilter function pointers
*   1.11 YWX 19-Oct-00 Added 1-2 Scaling functions pointers
*   1.10 jbb 16 oct 00 added ifdefs to insure version code
*   1.09 YWX 04-Oct-00 Added function pointers for scaling 
*   1.08 YWX 06 Sep 00 Added function pointers for new deringing filter 
*                      using frag baseed Q Value.
*   1.07 JBB 21 Aug 00 New More Blurry in high variance area deringer
*	1.06 YWX 2  Aug 00 Added function pointers for postprocess  
*	1.05 YWX 15/05/00  Added functions to check processor frequency
*					   and more function pointers for postprocessor
*	1.04 YWX 08/05/00  Added function pointers setup for postprocess
*   1.03 SJL 20/04/00  Added ability to enable the new dequant code.
*   1.02 SJL 22/03/00  Function pointers for the loop filter.
*   1.01 JBB 21/03/00  More Function Pointers for optimized playback
*   1.00 PGW 12/10/99  Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "pbdll.h" 
#pragma warning(disable:4115)
#include <windows.h>

extern void GetProcessorFlags(INT32 *MmxEnabled, INT32 *XmmEnabled, INT32 *WmtEnabled);

//extern void ReadTokens_c(PB_INSTANCE *pbi, INT32 * HuffIndices );
extern void  (*VP5_BuildQuantIndex)( QUANTIZER * pbi);

extern void UnPackVideo_C(PB_INSTANCE *pbi);
extern void UnPackVideo2(PB_INSTANCE *pbi);

extern void VP5_BuildQuantIndex_Generic(QUANTIZER *pbi);
extern void VP5_BuildQuantIndex_ForMMX(QUANTIZER *pbi);
extern void VP5_BuildQuantIndex_ForWMT(QUANTIZER *pbi);


//extern void ReadTokens_mmx(PB_INSTANCE *pbi, INT32 * HuffIndices );
extern void UnPackVideoMMX_LL (PB_INSTANCE *pbi);
extern void ClearMmx(void);
extern void CopyBlockMMX(unsigned char *src, unsigned char *dest, unsigned int srcstride);
//extern void ReadTokensPredict_c( PB_INSTANCE *pbi, UINT32 BlockSize, UINT32 Hpos );

/****************************************************************************
*  Explicit imports
*****************************************************************************
*/
extern unsigned int CPUFrequency;

//extern MmxEnabled;          // Is MMX enabled flag


/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
 

/****************************************************************************
*  Module statics.
*****************************************************************************
*/        

              
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/


/****************************************************************************
*  Functions
*****************************************************************************
*/

/****************************************************************************
 * 
 *  ROUTINE       :     readTSC
 *
 *  INPUTS        :     None
 *                   
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     read the cpu time stamp counter
 *
 *  SPECIAL NOTES :     Since this function uses RDTSC instruction, which is 
 *						introduced in Pentium processor, so this routine is 
 *						expected to work on Pentium and above.
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void VP5_readTSC(unsigned long *tsc)
{
	int time;
	
	__asm 
	{
        pushad
        cpuid
		rdtsc
		mov time,eax
        popad
	}

	*tsc=time;
	return;
}


/****************************************************************************
 * 
 *  ROUTINE       :     VP5_GetProcessorFrequency()
 *
 *  INPUTS        :     None
 *                   
 *
 *  OUTPUTS       :     The Frequency in MHZ
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Check the Processor's working freqency 
 *
 *  SPECIAL NOTES :     This function should only be used here. Limited tests 
 *						has verified it works till 166MHz Pentium with MMX. 
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
unsigned long VP5_GetProcessorFrequency()
{

	LARGE_INTEGER pf;						//Performance Counter Frequencey
	LARGE_INTEGER startcount, endcount;		
	unsigned long tsc1, tsc2;

	//If the cpu does not support the high resolution counter, return 0
    unsigned long time1, time2;
	unsigned long cpufreq=0;				
    unsigned long Nearest66Mhz, Nearest50Mhz;
    unsigned long Delta66, Delta50;

	if( QueryPerformanceFrequency(&pf))
	{
		
		// read the counter and TSC at start
		QueryPerformanceCounter(&startcount);
		VP5_readTSC(&tsc1);
		// delay for 10 ms to get enough accuracy
        time1 = timeGetTime();
        time2 = time1;

        while( time2 < time1+5 )
            time2 = timeGetTime();

		//read the counter and TSC at end
		QueryPerformanceCounter(&endcount);
		VP5_readTSC(&tsc2);
		
		//calculate the frequency
		cpufreq = (unsigned long )((double)( tsc2 - tsc1 ) 
			* (double)pf.LowPart 
			/ (double) ( endcount.LowPart - startcount.LowPart ) 
			/ 1000000);

	}
   
    Nearest66Mhz = ((cpufreq * 3 + 100)/200 * 200) / 3;
    Delta66 = abs(Nearest66Mhz - cpufreq);
    Nearest50Mhz = ((cpufreq + 25)/50 *50);
    Delta50 = abs(Nearest50Mhz - cpufreq);

    if(Delta50 < Delta66)
        cpufreq = Nearest50Mhz;
    else
    {
    
        cpufreq = Nearest66Mhz;
        if(cpufreq == 666)
            cpufreq = 667;
    }
    return cpufreq;

}


/****************************************************************************
 * 
 *  ROUTINE       :     MachineSpecificConfig
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Checks for machine specifc features such as MMX support 
 *                      sets approipriate flags and function pointers.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
#define MMX_ENABLED 1
void VP5_DMachineSpecificConfig(void)
{
	INT32 MmxEnabled;
	INT32 XmmEnabled; 
	INT32 WmtEnabled;

	GetProcessorFlags( &MmxEnabled,&XmmEnabled,&WmtEnabled);
	

	// If MMX supported then set to use MMX versions of functions else 
    // use original 'C' versions.

	if(WmtEnabled)		//Willamette
	{
		VP5_BuildQuantIndex = VP5_BuildQuantIndex_ForWMT;
	}
	else if ( MmxEnabled )
    {
		VP5_BuildQuantIndex = VP5_BuildQuantIndex_ForMMX;
    }
    else
    {
		VP5_BuildQuantIndex = VP5_BuildQuantIndex_Generic;
    }
	
//	ReadTokens = ReadTokensPredict_c;

}

// Issues a warning message
void VP5_IssueWarning( char * WarningMessage )
{
    // Issue the warning messge
    MessageBox(NULL, WarningMessage, NULL, MB_ICONEXCLAMATION | MB_TASKMODAL );
}

// Pause/Sleep for a X milliseconds
void VP5_PauseProcess( unsigned int SleepMs )
{
    Sleep( SleepMs );
}

char * VP5_SytemGlobalAlloc( unsigned int Size )  
{
    return GlobalAlloc( GPTR, Size );  
}

void VP5_SystemGlobalFree( char * MemPtr )
{
    GlobalFree( (HGLOBAL) MemPtr );
}



/****************************************************************************
 * 
 *  ROUTINE       :     VP5_SetPbParam
 *
 *  INPUTS        :     PB_COMMAND_TYPE Command
 *                      char *          Parameter
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *  
 *  FUNCTION      :     Generalised command interface to decoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CCONV VP5_SetPbParam( PB_INSTANCE *pbi, PB_COMMAND_TYPE Command, UINT32 Parameter )
{

#if defined(POSTPROCESS)
    switch ( Command )
    {
    case PBC_SET_CPUFREE:
    {
        
        double Pixels = pbi->Configuration.VideoFrameWidth * pbi->Configuration.VideoFrameHeight;
        double FreeMhz = pbi->ProcessorFrequency * Parameter / 100;
        double PixelsPerMhz = 100 * sqrt(1.0*Pixels) / FreeMhz;
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
	case PBC_SET_REFERENCEFRAME:
        CopyFrame( pbi->postproc, (YUV_BUFFER_CONFIG *) Parameter, pbi->LastFrameRecon);
		CopyFrame( pbi->postproc, (YUV_BUFFER_CONFIG *) Parameter, pbi->GoldenFrame);
		break;
	
	case PBC_SET_POSTPROC:
        if( Parameter == 9 )                
        {
            VP5_SetPbParam( pbi, PBC_SET_CPUFREE, 70);
        }
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
