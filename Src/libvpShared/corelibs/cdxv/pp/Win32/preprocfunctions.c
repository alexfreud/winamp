/****************************************************************************
*
*   Module Title :     PreProcOptFunctions.c
*
*   Description  :     MMX or otherwise processor specific 
*                      optimised versions of pre-processor functions
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.04 YWX 30-Nov-00 Added support for WMT cpu
*   1.03 PGW 24 Jul 00 Added Column SAD function.
*   1.02 YX  06/04/00  Optimized get row sad for xmm
*   1.01 PGW 12/07/99  Changes to reduce uneccessary dependancies. 
*   1.00 PGW 14/06/99  Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "preproc.h"
#include "cpuidlib.h"
#pragma warning( disable : 4799 )  // Disable no emms instruction warning!

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

/****************************************************************************
*  Imports.
*****************************************************************************
*/   
    
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Exported Functions 
*****************************************************************************
*/              

/****************************************************************************
*  Module Statics
*****************************************************************************
*/  


/****************************************************************************
*  Forward References
*****************************************************************************
*/  

UINT32 MmxRowSAD( UINT8 * Src1, UINT8 * Src2 );
extern UINT32 XmmRowSAD( UINT8 * Src1, UINT8 * Src2 );

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
void MachineSpecificConfig(PP_INSTANCE *ppi)
{
    UINT32 FeatureFlags = 0;
    BOOL   CPUID_Supported = TRUE;   // Is the CPUID instruction supported

    BOOL   TestMmx = TRUE;

    
    PROCTYPE CPUType = findCPUId();
	switch(CPUType)
	{
	case X86    :
	case PPRO   :
	case C6X86  :
	case C6X86MX:
	case AMDK5  :
	case MACG3	:
	case MAC68K	:
		ppi->MmxEnabled = FALSE;
		ppi->XmmEnabled = FALSE;
		break;
	case PII	:   
	case AMDK63D:
	case AMDK6  :
	case PMMX	:   
		ppi->MmxEnabled = TRUE;
		ppi->XmmEnabled = FALSE;
		break;
	case XMM    :
    case WMT    :
		ppi->MmxEnabled = TRUE;
		ppi->XmmEnabled = TRUE;
		break;
	}

	
	//To test We force the cpu type here
	//ppi->MmxEnabled = FALSE;
	//ppi->XmmEnabled = FALSE;

    // If MMX supported then set to use MMX versions of functions else 
    // use original 'C' versions.
	if (ppi->XmmEnabled)
	{
		ppi->RowSAD=XmmRowSAD;
        ppi->ColSAD = ScalarColSAD;
	}
	else if ( ppi->MmxEnabled )
    {
        ppi->RowSAD = MmxRowSAD;
        ppi->ColSAD = ScalarColSAD;
    }
    else
    {
        ppi->RowSAD = ScalarRowSAD;
        ppi->ColSAD = ScalarColSAD;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     MmxRowSAD
 *
 *  INPUTS        :     UINT8 * NewDataPtr	(New Data)
 *						UINT8 * RefDataPtr
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     Highest of two S.A.D. values.
 * 
 *
 *  FUNCTION      :     Calculates the sum of the absolute differences for two groups of
 *                      four pixels and returns the larger of the two.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT32 MmxRowSAD( UINT8 * NewDataPtr, UINT8  * RefDataPtr )
{
    UINT32 SadValue;
    UINT32 SadValue1;
	UINT32 AbsValues[2];

    // MMX code for calculating absolute difference values	
__asm
	{
		pxor        mm6, mm6					; Blank mmx6
		pxor        mm7, mm7					; Blank mmx6

		mov         eax,dword ptr [NewDataPtr]	; Load base addresses
		mov         ebx,dword ptr [RefDataPtr]

        // Calculate eight ABS difference values.
		movq		mm0, [eax]					; Copy eight bytes to mm0
		movq		mm1, [ebx]					; Copy eight bytes to mm1
		movq		mm2, mm0					; Take copy of MM0

		psubusb		mm0, mm1					; A-B to MM0
		psubusb		mm1, mm2					; B-A to MM1
		por			mm0, mm1					; OR MM0 and MM1 gives abs differences in MM0

		movq		mm1, mm0					; keep a copy

		// Sum together the low four bytes and the high four bytes
		punpcklbw   mm0, mm6					; unpack low four bytes to higher precision
		punpckhbw   mm1, mm7					; unpack high four bytes to higher precision
		movq        mm2, mm0                    ; take a copy
		movq        mm3, mm1                    ; take a copy
		punpcklwd   mm0, mm6					; unpack low two words to higher precision
		punpcklwd   mm1, mm7					; unpack low two words to higher precision
		punpckhwd   mm2, mm6					; unpack high low two words to higher precision
		punpckhwd   mm3, mm7					; unpack high low two words to higher precision
		
		paddd       mm0, mm2                    ; Accumulate intermediate results
		paddd       mm1, mm3                    ; Accumulate intermediate results
		movq        mm2, mm0                    ; take a copy
		movq        mm3, mm1                    ; take a copy
		punpckhdq   mm0, mm6					; Unpack and accumulate again
		punpckhdq   mm1, mm7					; Unpack and accumulate again
		punpckldq   mm2, mm6
		punpckldq   mm3, mm7
		paddd       mm0, mm2                    ; Accumulate final result
		paddd       mm1, mm3                    ; Accumulate final result

		// Interleave the two SAD results
		punpckldq   mm0, mm1

        // Write back the abs values
        movq        dword ptr [AbsValues], mm0  
    }
    
    SadValue = AbsValues[0];
    SadValue1 = AbsValues[1];
    SadValue = (SadValue > SadValue1) ? SadValue : SadValue1;

    return SadValue;
}

/****************************************************************************
 * 
 *  ROUTINE       :     ClearMmxState()
 *
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Clears down the MMX state
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ClearMmxState(PP_INSTANCE *ppi)
{
    if ( ppi->MmxEnabled )
    {
        __asm
	    {
            emms									; Clear the MMX state.
        }
    }
}
