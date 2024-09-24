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
*   1.20 YWX 06-Nov-01 Configuration Baseline for C only version
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "pbdll.h" 

extern void GetProcessorFlags(INT32 *MmxEnabled, INT32 *XmmEnabled, INT32 *WmtEnabled);

//extern void ReadTokens_c(PB_INSTANCE *pbi, INT32 * HuffIndices );
extern void  (*VP5_BuildQuantIndex)( QUANTIZER * pbi);

extern void UnPackVideo_C(PB_INSTANCE *pbi);
extern void UnPackVideo2(PB_INSTANCE *pbi);

extern void VP5_BuildQuantIndex_Generic(QUANTIZER *pbi);

/****************************************************************************
*  Explicit imports
*****************************************************************************
*/

extern unsigned int CPUFrequency;

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
 *  ROUTINE       :     GetProcessorFrequency()
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

    return 0;

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
void VP5_DMachineSpecificConfig(void)
{
    VP5_BuildQuantIndex = VP5_BuildQuantIndex_Generic;
}

// Issues a warning message
void VP5_IssueWarning( char * WarningMessage )
{
	(void) WarningMessage;
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
        
        double PixelsPerMhz = 100 *10;
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
