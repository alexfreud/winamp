/****************************************************************************
*
*   Module Title :     PB_Globals.c
*
*   Description  :     Video CODEC Demo: playback dll global declarations
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "pbdll.h"
#include "duck_mem.h"

/****************************************************************************
*  Module Statics
****************************************************************************/ 
static UINT32 VP6_DCQuantScaleP[Q_TABLE_SIZE];

/****************************************************************************
*  Imports
****************************************************************************/ 
extern unsigned long VP6_GetProcessorFrequency();

/****************************************************************************
*  Exports
****************************************************************************/
unsigned int CPUFrequency;      // Process Frequency

// Truth table to indicate if the given mode uses motion estimation
BOOL VP6_ModeUsesMC[MAX_MODES] = { FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE };

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DeleteTmpBuffers
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     De-allocate buffers used during decoing.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_DeleteTmpBuffers ( PB_INSTANCE *pbi )
{ 
	if ( pbi->ReconDataBuffer[0] )
		duck_free(pbi->ReconDataBuffer[0]);
	if ( pbi->LoopFilteredBlock )
		duck_free(pbi->LoopFilteredBlock);
	if ( pbi->TmpDataBuffer )
		duck_free(pbi->TmpDataBuffer);
	if ( pbi->TmpReconBuffer )
		duck_free(pbi->TmpReconBuffer);
	if ( pbi->ScaleBuffer )
		duck_free(pbi->ScaleBuffer);

    
	pbi->ReconDataBuffer[0]     = 0;
	pbi->LoopFilteredBlock      = 0;
	pbi->TmpDataBuffer          = 0;
	pbi->TmpReconBuffer         = 0;
    pbi->ScaleBuffer            = 0;
}


/****************************************************************************
 * 
 *  ROUTINE       :     VP6_AllocateTmpBuffers
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     BOOL: Always TRUE.
 * 
 *  FUNCTION      :     Allocates buffers required during decoding.
 *
 *  SPECIAL NOTES :     Uses ROUNDUP32 to ensure that buffers are aligned
 *                      on 32-byte boundaries to improve cache performance.
 *
 ****************************************************************************/
BOOL VP6_AllocateTmpBuffers ( PB_INSTANCE *pbi )
{

	// clear any existing info
	VP6_DeleteTmpBuffers ( pbi ); 

	pbi->ReconDataBuffer[0]      = (INT16 *)duck_memalign(32, 6*64*sizeof(INT16), DMEM_GENERAL);
	if ( !pbi->ReconDataBuffer[0] )      { VP6_DeleteTmpBuffers(pbi); return FALSE;};
    pbi->ReconDataBuffer[1] = pbi->ReconDataBuffer[0] + 64;
    pbi->ReconDataBuffer[2] = pbi->ReconDataBuffer[1] + 64;
    pbi->ReconDataBuffer[3] = pbi->ReconDataBuffer[2] + 64;
    pbi->ReconDataBuffer[4] = pbi->ReconDataBuffer[3] + 64;
    pbi->ReconDataBuffer[5] = pbi->ReconDataBuffer[4] + 64;

    pbi->TmpDataBuffer        = (INT16 *)duck_memalign(32, 64 * sizeof(INT16), DMEM_GENERAL);
    if ( !pbi->TmpDataBuffer )        { VP6_DeleteTmpBuffers(pbi); return FALSE;};

	pbi->LoopFilteredBlock        = (UINT8 *)duck_memalign(32, 256 * sizeof(UINT8), DMEM_GENERAL);
    if ( !pbi->LoopFilteredBlock )        { VP6_DeleteTmpBuffers(pbi); return FALSE;};

    pbi->TmpReconBuffer       = (INT16 *)duck_memalign(32, 64 * sizeof(INT16), DMEM_GENERAL);
    if ( !pbi->TmpReconBuffer )       { VP6_DeleteTmpBuffers(pbi); return FALSE;};

    return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DeletePBInstance
 *
 *  INPUTS        :     PB_INSTANCE **pbi : Pointer to the pointer to the 
 *                                          decoder instance.
 *
 *  OUTPUTS       :     PB_INSTANCE **pbi : Pointer to the pointer to the 
 *                                          decoder instance. Set to 0 on exit.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     De-allocates the decoder instance data structure.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_DeletePBInstance ( PB_INSTANCE **pbi )
{
    if ( *pbi )
    {
        // Delete any other dynamically allocaed temporary buffers
		VP6_DeleteTmpBuffers(*pbi);
		VP6_DeleteQuantizer(&(*pbi)->quantizer);
        DeletePostProcInstance(&(*pbi)->postproc);
    }

    // dealoocate and reset pointer to NULL
	duck_free ( *pbi );
	*pbi = 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_CreatePBInstance
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     PB_INSTANCE *: Pointer to allocated decoder instance.
 *
 *  FUNCTION      :     Allocates space for and initializes decoder instance.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
PB_INSTANCE *VP6_CreatePBInstance ( void )
{
	PB_INSTANCE *pbi = 0;
    CONFIG_TYPE ConfigurationInit = { 0,0,0,0,8,8,0,0,0,0,0,0,0,0 };
	int pbi_size = sizeof(PB_INSTANCE);

    pbi = (PB_INSTANCE *) duck_malloc ( pbi_size, DMEM_GENERAL );
    if ( !pbi )
        return 0;

	// initialize whole structure to 0
	memset ( (unsigned char *)pbi, 0, pbi_size );
	memcpy ( (void *)&pbi->Configuration, (void *)&ConfigurationInit, sizeof(CONFIG_TYPE) );

	if ( !VP6_AllocateTmpBuffers(pbi) )
    {
        duck_free(pbi);
        return 0;
    }

	pbi->CPUFree = 70;
    pbi->idct    = idct;

	// Initialise Entropy related data structures.
	memset( pbi->DcProbs, 0, sizeof(pbi->DcProbs) );
	memset( pbi->AcProbs, 0, sizeof(pbi->AcProbs) );

	return pbi;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_VPInitLibrary
 *
 *  INPUTS        :     None.
 *  
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Fully initializes the playback library.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_VPInitLibrary(void)
{
	int i;

#if !defined(__POWERPC__)
    CPUFrequency = VP6_GetProcessorFrequency();
#endif


    VP6_DMachineSpecificConfig();

    for ( i=0 ; i<Q_TABLE_SIZE; i++ )
    {
		INT32 dcScale = VP6_DcQuant[i]/2 + 2;
		VP6_DCQuantScaleP[i] = dcScale;
    }

    InitPostProcessing (
		VP6_DCQuantScaleP,
		VP6_DCQuantScaleP,
		VP6_DCQuantScaleP,
		CURRENT_DECODE_VERSION );

	InitVPUtil(); 
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_VPDeInitLibrary
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     De-initializes the playback library.
 *
 *  SPECIAL NOTES :     Currently nothing to be done. 
 *
 ****************************************************************************/
void VP6_VPDeInitLibrary(void)
{
}
