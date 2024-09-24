/****************************************************************************
*
*   Module Title :     Timer.C
*
*   Description  :     Video CODEC timer module
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*   
*   1.01 PGW 09/07/99  Added code to support profile timing
*   1.00 PGW 14/06/99  Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/					  

#define STRICT              /* Strict type checking. */
#define INC_WIN_HEADER      1
#include <windows.h>

#include "type_aliases.h"
#include <mmsystem.h> 

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
                
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/  


/****************************************************************************
*  Module Static Variables
*****************************************************************************
*/              

// Used for calculation of elapsed time
UINT32 LastCPUTime;

/****************************************************************************
 * 
 *  ROUTINE       :     MyInitTimer
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises the timer mechanism.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void MyInitTimer( void )
{
}


/****************************************************************************
 * 
 *  ROUTINE       :     MyGetTime
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Time in ms since startup.
 *
 *  FUNCTION      :     Provides a model independant interface for getting times.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT32 MyGetTime( void )	  
{
/* Use different timing mechanisms for win32 and win16. 
*  The win16 method is accurate to 1ms whilst the Win32 is not garauteed to better than 16ms
*/
    return timeGetTime();
}

/****************************************************************************
 * 
 *  ROUTINE       :     MyGetElapsedCpuTime
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     CPU cycles since last call
 *
 *  FUNCTION      :     Calculate the CPU cycles elapsed since the last call
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT32 MyGetElapsedCpuTime( void )	  
{
    UINT32 CurrCPUTime[2];                          // Full 64 bit CPU time
    UINT32 CurrentCpuTime;                          // modified 32 bit current time
    UINT32 ElapsedTime;

__asm
	{
        rdtsc                                       ; Get CPU time into EDX:EAX

        mov         dword ptr [CurrCPUTime], eax    ; Save to a global
        mov         dword ptr [CurrCPUTime+4], edx   
    }

    // Save CurrCPUTime to LastCPUTime
    CurrCPUTime[0] = (CurrCPUTime[0] >> 8);
    CurrCPUTime[1] = (CurrCPUTime[1] & 0x000000FF) << 24;
    CurrentCpuTime = CurrCPUTime[0] | CurrCPUTime[1];

    // Check for wrapp around
    if ( CurrentCpuTime >= LastCPUTime )
    {
        ElapsedTime =  CurrentCpuTime - LastCPUTime;
    }
    else
    {
        ElapsedTime =  (LastCPUTime - CurrentCpuTime) + 0xFFFF;
    }
    LastCPUTime = CurrentCpuTime;

    return ElapsedTime;
}
