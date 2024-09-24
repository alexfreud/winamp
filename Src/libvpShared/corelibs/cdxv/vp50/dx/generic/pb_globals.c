/****************************************************************************
*
*   Module Title :     PB_Globals.c
*
*   Description  :     Video CODEC Demo: playback dll global declarations
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.21 JBB 13 Jun 01 VP4 Code Clean Out
*	1.20 AWG 08-Jun-01 Added support for DCT16
*   1.19 JBB 01-MAY-01 VP5 Functionality (set up coefftoband array)
*   1.18 YWX 26-Apr 01 Added global "CPUFrequency" and its initializing
* 			in VPInitlibrary()		
*   1.17 JBB 06 Apr 01 new cpu free variable initialized
*   1.16 SJL 30 Mar 01 Added #if defined(POSTPROCESS) around InitPostProcessing();
*   1.15 PGW 25 Jan 01 Add code to create and destroy MV huffman trees.
*   1.15 JBB 26 Jan 01 No need to destroy huffman trees
*   1.14 JBB 22 Aug 00 Ansi C conversion
*   1.13 JBB 21 Aug 00 New More Blurry in high variance area deringer
*	1.12 YWX 2  Aug 00 Removed redundant kernel modifiers
*   1.11 JBB 27 Jul 00 Moved kernel modifiers to pbi mallocs -> duck_malloc
*                      for scott added malloc checks
*	1.10 YWX 15/05/00  change the initialization of PostProcessLevel
*	1.09 JBB 27/01/99  Globals Removed, use of PB_INSTANCE, added PB_Instance
*                      allocation and deletion funcitons
*   1.08 PGW 17/12/99  Draw dib functionality removed.
*   1.07 PGW 16/12/99  Added support for VP3 version id.
*   1.06 PGW 15/12/99  Added key frame type variable
*   1.05 PGW 22/11/99  Changes relating to restructuring of block map stuff.
*   1.04 PGW 14/10/99  Changes to reduce uneccessary dependancies. 
*   1.05 PGW 06/09/99  DivBySix changed to UINT8 [].
*   1.04 PGW 24/08/99  Removed of EOF token and assosciated data sturctures etc.
*                      Deleted COrderList[].
*   1.03 PGW 15/07/99  Added bit extraction variables.
*   1.02 PGW 14/07/99  Changes to interface to idct and reconstruction functions.
*                      Added ModeUsesMC[] truth table. Added (*ReconIntra) funtion
*                      pointer.
*   1.01 PGW 09/07/99  Added code to support profile timing
*   1.00 PGW 22/06/99  Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "pbdll.h"
#include "duck_mem.h"

/****************************************************************************
*  Explicit imports
*****************************************************************************
*/ 

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
 
/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
extern unsigned long VP5_GetProcessorFrequency();
                
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/
//extern Q_LIST_ENTRY VP5_DcScaleFactorTableV1[ Q_TABLE_SIZE ] ; 
extern Q_LIST_ENTRY VP5_DcQuant[ Q_TABLE_SIZE ];

UINT32 DCQuantScaleP[Q_TABLE_SIZE];

//****************************************************************
// Function Pointers now library globals!
//****************************************************************

// Process Frequency
unsigned int CPUFrequency;

// Truth table to indicate if the given mode uses motion estimation
BOOL VP5_ModeUsesMC[MAX_MODES] = { FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE };

/****************************************************************************
 * 
 *  ROUTINE       :     DeleteTmpBuffers
 *
 *
 *  INPUTS        :     Instance of PB to be initialized
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_DeleteTmpBuffers(PB_INSTANCE * pbi)
{

	if(pbi->ReconDataBufferAlloc)
		duck_free(pbi->ReconDataBufferAlloc);
	if(pbi->LoopFilteredBlockAlloc)
		duck_free(pbi->LoopFilteredBlockAlloc);
	if(pbi->TmpDataBufferAlloc)
		duck_free(pbi->TmpDataBufferAlloc);
	if(pbi->TmpReconBufferAlloc)
		duck_free(pbi->TmpReconBufferAlloc);
	if(pbi->ScaleBufferAlloc)
		duck_free(pbi->ScaleBufferAlloc);

	pbi->ReconDataBufferAlloc=0;
	pbi->TmpDataBufferAlloc = 0;
	pbi->TmpReconBufferAlloc = 0;
    pbi->ScaleBufferAlloc = 0;
    pbi->ScaleBuffer = 0;
	pbi->ReconDataBuffer=0;
	pbi->TmpDataBuffer = 0;
	pbi->TmpReconBuffer = 0;

	pbi->LoopFilteredBlockAlloc = 0;
	pbi->LoopFilteredBlock = 0;

}


/****************************************************************************
 * 
 *  ROUTINE       :     AllocateTmpBuffers
 *
 *
 *  INPUTS        :     Instance of PB to be initialized
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) )
BOOL VP5_AllocateTmpBuffers(PB_INSTANCE * pbi)
{

	// clear any existing info
	VP5_DeleteTmpBuffers(pbi);
#ifdef MAPCA
    pbi->ReconDataBufferAlloc      = (INT16 (*)[64])duck_malloc(32+64*sizeof(INT16)*6, DMEM_GENERAL);
	if(!pbi->ReconDataBufferAlloc)      { VP5_DeleteTmpBuffers(pbi); return FALSE;};
    pbi->ReconDataBuffer           = (INT16 (*)[64])ROUNDUP32(pbi->ReconDataBufferAlloc);
#else
	// Adjust the position of all of our temporary
	pbi->ReconDataBufferAlloc      = (INT16 *)duck_malloc(32+64*sizeof(INT16), DMEM_GENERAL);
	if(!pbi->ReconDataBufferAlloc)      { VP5_DeleteTmpBuffers(pbi); return FALSE;};
    pbi->ReconDataBuffer           = (INT16 *)ROUNDUP32(pbi->ReconDataBufferAlloc);
#endif
    
	pbi->TmpDataBufferAlloc        = (INT16 *)duck_malloc(32 + 64 * sizeof(INT16), DMEM_GENERAL);
    if(!pbi->TmpDataBufferAlloc)        { VP5_DeleteTmpBuffers(pbi); return FALSE;};
    pbi->TmpDataBuffer             = (INT16 *)ROUNDUP32(pbi->TmpDataBufferAlloc);

	pbi->LoopFilteredBlockAlloc        = (UINT8 *)duck_malloc(32 + 256 * sizeof(UINT8), DMEM_GENERAL);
    if(!pbi->LoopFilteredBlockAlloc)        { VP5_DeleteTmpBuffers(pbi); return FALSE;};
    pbi->LoopFilteredBlock             = (UINT8 *)ROUNDUP32(pbi->LoopFilteredBlockAlloc);

    pbi->TmpReconBufferAlloc       = (INT16 *)duck_malloc(32 + 64 * sizeof(INT16), DMEM_GENERAL);
    if(!pbi->TmpReconBufferAlloc)       { VP5_DeleteTmpBuffers(pbi); return FALSE;};
    pbi->TmpReconBuffer            = (INT16 *)ROUNDUP32(pbi->TmpReconBufferAlloc);


    return TRUE;

}

/****************************************************************************
 * 
 *  ROUTINE       :     DeletePBInstance
 *
 *
 *  INPUTS        :     Instance of PB to be deleted
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     frees the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_DeletePBInstance(PB_INSTANCE **pbi)
{
	// clear any existing info
    if(*pbi)
    {
        // Delete the motion vector huffman trees.
        //DestroyMvTrees(*pbi);

        // Delete any other dynamically allocaed temporary buffers
		VP5_DeleteTmpBuffers(*pbi);
		VP5_DeleteQuantizer(&(*pbi)->quantizer);
#ifndef MAPCA
        DeletePostProcInstance(&(*pbi)->postproc);
#endif
    }

	duck_free(*pbi);
	*pbi=0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     CreatePBInstance
 *
 *
 *  INPUTS        :     Instance of PB to be initialized
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
PB_INSTANCE * VP5_CreatePBInstance(void)
{
	PB_INSTANCE *pbi=0;
	CONFIG_TYPE ConfigurationInit = 
	{
		0,0,0,0,
	    8,8,
	};


	int pbi_size = sizeof(PB_INSTANCE);
	pbi=(PB_INSTANCE *) duck_malloc(pbi_size, DMEM_GENERAL);
    if(!pbi)
    {
        return 0;
    }

	// initialize whole structure to 0
	memset((unsigned char *) pbi, 0, sizeof(PB_INSTANCE));
	
	memcpy((void *) &pbi->Configuration, (void *) &ConfigurationInit, sizeof(CONFIG_TYPE));

	if(!VP5_AllocateTmpBuffers(pbi))
    {
        duck_free(pbi);
        return 0;
    }


	pbi->KeyFrameType = DCT_KEY_FRAME;
	pbi->CPUFree = 70;
#ifndef MAPCA
    pbi->idct = idct;
#endif

	// Initialise Entropy related data structures.
	memset( pbi->DcProbs, 0, sizeof(pbi->DcProbs) );
	memset( pbi->AcProbs, 0, sizeof(pbi->AcProbs) );


	return pbi;
}




/****************************************************************************
 * 
 *  ROUTINE       :     VPInitLibrary
 *
 *
 *  INPUTS        :     init VP library
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Fully initializes the playback library
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_VPInitLibrary(void)
{
	int i;
#if !defined(__POWERPC__)
    CPUFrequency = VP5_GetProcessorFrequency();
#endif


    VP5_DMachineSpecificConfig();

    for( i = 0 ; i < Q_TABLE_SIZE; i++)
    {
		INT32 dcScale;

//		if(i<4)
//			dcScale = ((6-i) * VP5_DcQuant[i]/4);
//		else 
			dcScale = VP5_DcQuant[i]/2;

		DCQuantScaleP[i] =  dcScale;

    }

#ifndef MAPCA
    InitPostProcessing(
		DCQuantScaleP,
		DCQuantScaleP,
		DCQuantScaleP,
		CURRENT_DECODE_VERSION);
	InitVPUtil(); 
#else
    VP5_InitPostProcess();
#endif
}

/*********************************************************/


/****************************************************************************
 * 
 *  ROUTINE       :     VPDeinitLibrary
 *
 *
 *  INPUTS        :     init VP library
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Fully initializes the playback library
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_VPDeInitLibrary(void)
{
#ifdef MAPCA
    VP5_ClosePostProcess();
#endif


}



