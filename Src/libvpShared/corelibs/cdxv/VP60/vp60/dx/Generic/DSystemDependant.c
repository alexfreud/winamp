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
#include "pbdll.h"
extern void VP6_BuildQuantIndex_Generic ( QUANTIZER *pbi );

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
 *  ROUTINE       :     VP6_GetProcessorFrequency()
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     unsigned long: clock speed of the host processor.
 *
 *  FUNCTION      :     Get the Processor's working freqency. 
 *
 *  SPECIAL NOTES :     Stub function--always returns value 0. 
 *
 ****************************************************************************/
unsigned long VP6_GetProcessorFrequency ( void )
{
    return 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DMachineSpecificConfig
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Sets up pointers to platform dependant functions.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_DMachineSpecificConfig ( void )
{
    VP6_BuildQuantIndex = VP6_BuildQuantIndex_Generic;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_IssueWarning
 *
 *  INPUTS        :     char *WarningMessage : Message to be issued.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Issues a warning message.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_IssueWarning ( char *WarningMessage )
{
	(void) WarningMessage;
}


