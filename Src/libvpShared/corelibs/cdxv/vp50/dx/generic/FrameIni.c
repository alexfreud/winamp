/****************************************************************************
*
*   Module Title :     FrameIni.c
*
*   Description  :     Video CODEC playback module
*
*    AUTHOR      :     JimBankoski
*
*****************************************************************************
*   Revision History
*
*   1.21 YWX 06-Nov-01 Changed to align the MB coeffs buffer memory
*   1.20 JBB 13-Jun-01 VP4 Code Clean Out
*	1.19 AWG 11-Jun-01 Added support for DCT16
*   1.18 JBB 24-May-01 Fixed Memory Allocation problem and frame recon prob
*   1.17 JBB 09-Apr-01 CPUFree persistence
*	1.16 SJL 05-Apr-01 Fixed MAC compile errors.
*	1.15 JBB 23-Mar-01 New DC prediction
*   1.14 JBX 22-Mar-01 Merged with vp4-mapca bitstream
*   1.13 JBB 30 NOV 00 Version number changes 
*   1.12 JBB 15-NOV-00 cleaned out ifdefs
*   1.11 JBB 17-oct-00 Ifdefs around version information
*   1.10 YWX 17-Oct-00 Added Initialization of block coordinates for 
*                      new loop filtering strategy
*   1.09 YWX 11-Oct-00 Added LastFrameNoMvRecon and LastFrameNoMvReconAlloc 
*   1.08 SJL 25 Aug 00 Fixed Mac compile error
*   1.08 JBB 24 Aug 00 Removed extraneous definition of load and decode
*   1.07 SJL 16 Aug 00 Fixed Mac compile error
*   1.06 JBB 28 jul 00 Added fragment variance array for post processor
*   1.05 JBB 27Jul00   Added checks on Mallocs
*   1.04 SJL 24Jul00   Changed Frees to DUCK_FREE for Mac utilization
*	1.03 YWX 08/05/00  Added #if defined(POSTPROCESS) for postprocess 
*	1.02 JBB 05/05/00  Added Post Processing Buffer & Block Quality Buffers
*   1.01 YWX 06/04/00  Alligned more buffers for speed
*	1.00 JBB 27/01/99  Globals Removed, use of PB_INSTANCE, common between 
*                      compressor and decompressor
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#include "pbdll.h"
#include "stdlib.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
                
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/  

/****************************************************************************
*  Imports 
*****************************************************************************
*/  

/****************************************************************************
*  Module Static Variables
*****************************************************************************
*/  

static const struct 
{
	INT32 row;
	INT32 col;
} NearMacroBlocks[12] = 
{
	{ -1, 0 },
	{ 0, -1 },
	{ -1, -1 },
	{ -1, 1 },
	{ -2, 0 },
	{ 0, -2 },
	{ -1, -2 },
	{ -2, -1 },
	{ -2, 1 },
	{ -1, 2 },
	{ -2, -2 },
	{ -2, 2 }
};

/****************************************************************************
*  Forward References
*****************************************************************************
*/  
void InitializeFragCoordinates(PB_INSTANCE *pbi);
/****************************************************************************
*  Explicit Imports
*****************************************************************************
*/


#include "duck_mem.h"

/****************************************************************************
 * 
 *  ROUTINE       :     DeleteFragmentInfo
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
void VP5_DeleteFragmentInfo(PB_INSTANCE * pbi)
{

	// free prior allocs if present
#ifndef MAPCA
    if( pbi->mbi.CoeffsAlloc)
        duck_free(pbi->mbi.CoeffsAlloc);
    pbi->mbi.CoeffsAlloc = 0;
    pbi->mbi.Coeffs=0;
#endif

    if(	pbi->FragInfoAlloc)
		duck_free(pbi->FragInfoAlloc);
    pbi->FragInfoAlloc = 0;
    pbi->FragInfo = 0;

	if(	pbi->fc.AboveYAlloc)
		duck_free(pbi->fc.AboveYAlloc);
    pbi->fc.AboveYAlloc = 0;
    pbi->fc.AboveY = 0;

	if(	pbi->fc.AboveUAlloc)
		duck_free(pbi->fc.AboveUAlloc);
    pbi->fc.AboveUAlloc = 0;
    pbi->fc.AboveU = 0;

	if(	pbi->fc.AboveVAlloc)
		duck_free(pbi->fc.AboveVAlloc);
    pbi->fc.AboveVAlloc = 0;
    pbi->fc.AboveV = 0;

	if(	pbi->MBInterlacedAlloc)
		duck_free(pbi->MBInterlacedAlloc);
    pbi->MBInterlacedAlloc = 0;
    pbi->MBInterlaced = 0;

	if(	pbi->MBMotionVectorAlloc)
		duck_free(pbi->MBMotionVectorAlloc);
    pbi->MBMotionVectorAlloc = 0;
    pbi->MBMotionVector = 0;

	if(	pbi->predictionModeAlloc)
		duck_free(pbi->predictionModeAlloc);
    pbi->predictionModeAlloc = 0;
    pbi->predictionMode = 0;

#ifdef MAPCA
    if(pbi->ReferenceBlocksAlloc)
        duck_free(pbi->ReferenceBlocksAlloc);
    pbi->ReferenceBlocksAlloc = 0;
    pbi->ReferenceBlocks = 0;

    if(pbi->ReconstructedMBsAlloc)
			duck_free(pbi->ReconstructedMBsAlloc);
	pbi->ReconstructedMBsAlloc=0;	
    pbi->ReconstructedMBs =0;
#endif



}


/****************************************************************************
 * 
 *  ROUTINE       :     AllocateFragmentInfo
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
BOOL VP5_AllocateFragmentInfo(PB_INSTANCE * pbi)
{

	// clear any existing info
	VP5_DeleteFragmentInfo(pbi);
#ifndef MAPCA
    pbi->mbi.CoeffsAlloc = (Q_LIST_ENTRY(*)[72]) duck_malloc(32 + sizeof(Q_LIST_ENTRY)*72*6, DMEM_GENERAL);
    if(!pbi->mbi.CoeffsAlloc) {VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->mbi.Coeffs = (Q_LIST_ENTRY(*)[72])ROUNDUP32(pbi->mbi.CoeffsAlloc);
#endif
	// context allocations
    pbi->fc.AboveYAlloc = (BLOCK_CONTEXTA *) duck_malloc(32 + (8+pbi->HFragments) * sizeof(BLOCK_CONTEXT), DMEM_GENERAL);
    if(!pbi->fc.AboveYAlloc) { VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->fc.AboveY = (BLOCK_CONTEXTA *) ROUNDUP32(pbi->fc.AboveYAlloc);

    pbi->fc.AboveUAlloc = (BLOCK_CONTEXTA *) duck_malloc(32 + (8+pbi->HFragments / 2) * sizeof(BLOCK_CONTEXT), DMEM_GENERAL);
    if(!pbi->fc.AboveUAlloc) { VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->fc.AboveU = (BLOCK_CONTEXTA *) ROUNDUP32(pbi->fc.AboveUAlloc);

    pbi->fc.AboveVAlloc = (BLOCK_CONTEXTA *) duck_malloc(32 + (8+pbi->HFragments / 2) * sizeof(BLOCK_CONTEXT), DMEM_GENERAL);
    if(!pbi->fc.AboveVAlloc) { VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->fc.AboveV = (BLOCK_CONTEXTA *) ROUNDUP32(pbi->fc.AboveVAlloc);


	// the encoder is the only thing using this move it to compdll
    pbi->MBInterlacedAlloc = (char *) duck_malloc(32+pbi->MacroBlocks * sizeof(char), DMEM_GENERAL);
    if(!pbi->MBInterlacedAlloc) { VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->MBInterlaced = (char *) ROUNDUP32(pbi->MBInterlacedAlloc );

    pbi->predictionModeAlloc = (char *) duck_malloc(32+pbi->MacroBlocks * sizeof(char), DMEM_GENERAL);
    if(!pbi->predictionModeAlloc) { VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->predictionMode = (char *) ROUNDUP32(pbi->predictionModeAlloc );

    pbi->MBMotionVectorAlloc = (MOTION_VECTORA *) duck_malloc(32+pbi->MacroBlocks * sizeof(MOTION_VECTORA ), DMEM_GENERAL);
    if(!pbi->MBMotionVectorAlloc) { VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->MBMotionVector = (MOTION_VECTORA  *) ROUNDUP32(pbi->MBMotionVectorAlloc );


	// the encoder is the only thing using this move it to compdll
    pbi->FragInfoAlloc = (FRAG_INFO *) duck_malloc(32+pbi->UnitFragments * sizeof(FRAG_INFO), DMEM_GENERAL);
    if(!pbi->FragInfoAlloc) { VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->FragInfo = (FRAG_INFO *) ROUNDUP32(pbi->FragInfoAlloc );


#ifdef MAPCA
    pbi->ReferenceBlocksAlloc=(UINT8(*)[192])duck_malloc(32 + 6*192, DMEM_GENERAL);
    if(!pbi->ReferenceBlocksAlloc){ VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->ReferenceBlocks = (UINT8(*)[192])ROUNDUP32(pbi->ReferenceBlocksAlloc); 

    pbi->ReconstructedMBsAlloc = (UINT8*) duck_malloc(32 + 768, DMEM_GENERAL);
    if(!pbi->ReconstructedMBsAlloc){ VP5_DeleteFragmentInfo(pbi); return FALSE;}
    pbi->ReconstructedMBs = (UINT8*) ROUNDUP32(pbi->ReconstructedMBsAlloc);
#endif

    return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       :     DeleteFrameInfo
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
void VP5_DeleteFrameInfo(PB_INSTANCE * pbi)
{
	if(pbi->ThisFrameReconAlloc )
		duck_free(pbi->ThisFrameReconAlloc );
	if(pbi->GoldenFrameAlloc)
		duck_free(pbi->GoldenFrameAlloc);
	if(pbi->LastFrameReconAlloc)
		duck_free(pbi->LastFrameReconAlloc);
	if(pbi->PostProcessBufferAlloc)
		duck_free(pbi->PostProcessBufferAlloc);

	pbi->ThisFrameReconAlloc = 0;
	pbi->GoldenFrameAlloc = 0;
	pbi->LastFrameReconAlloc = 0;
	pbi->PostProcessBufferAlloc = 0;

	pbi->ThisFrameRecon = 0;
	pbi->GoldenFrame = 0;
	pbi->LastFrameRecon = 0;
	pbi->PostProcessBufferAlloc = 0;

}


/****************************************************************************
 * 
 *  ROUTINE       :     AllocateFrameInfo
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
BOOL VP5_AllocateFrameInfo(PB_INSTANCE * pbi, unsigned int FrameSize)
{

	// clear any existing info
	VP5_DeleteFrameInfo(pbi);

	// allocate frames

	// (JBB+YX ) Added 2 extra lines to framebuffer so that copy12x12
	// doesn't fail when we have a large motion vector in V 
	// on the last v block.  Note : We never use these pixels
	// anyway so this doesn't hurt anything

	pbi->ThisFrameReconAlloc = (UINT8 *)duck_malloc(32+pbi->Configuration.YStride+FrameSize*sizeof(YUV_BUFFER_ENTRY), DMEM_GENERAL);
    if(!pbi->ThisFrameReconAlloc) { VP5_DeleteFrameInfo(pbi); return FALSE;}

	pbi->GoldenFrameAlloc = (UINT8 *)duck_malloc(32+pbi->Configuration.YStride+FrameSize*sizeof(YUV_BUFFER_ENTRY ), DMEM_GENERAL);
    if(!pbi->GoldenFrameAlloc) { VP5_DeleteFrameInfo(pbi); return FALSE;}

	pbi->LastFrameReconAlloc = (UINT8 *)duck_malloc(32+pbi->Configuration.YStride+FrameSize*sizeof(YUV_BUFFER_ENTRY), DMEM_GENERAL);
    if(!pbi->LastFrameReconAlloc) { VP5_DeleteFrameInfo(pbi); return FALSE;}

	pbi->PostProcessBufferAlloc = (UINT8 *)duck_malloc(32+pbi->Configuration.YStride+FrameSize*sizeof(YUV_BUFFER_ENTRY), DMEM_GENERAL);
    if(!pbi->PostProcessBufferAlloc) { VP5_DeleteFrameInfo(pbi); return FALSE;}


	// adjust up to the next 32 byte boundary
	pbi->ThisFrameRecon = (unsigned char *) ROUNDUP32(pbi->ThisFrameReconAlloc );
	pbi->GoldenFrame = (unsigned char *) ROUNDUP32(pbi->GoldenFrameAlloc );
	pbi->LastFrameRecon = (unsigned char *) ROUNDUP32(pbi->LastFrameReconAlloc );
	pbi->PostProcessBuffer = (unsigned char *) ROUNDUP32( pbi->PostProcessBufferAlloc );

    return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP5_InitFrameDetails
 *
 *  INPUTS        :     Nonex.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises the frame details.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL VP5_InitFrameDetails(PB_INSTANCE *pbi)
{
	int FrameSize;
	UINT32 i;

	if(pbi->CPUFree > 0 )
		VP5_SetPbParam( pbi, PBC_SET_CPUFREE, pbi->CPUFree );

    /* Set the frame size etc. */                                                        
    pbi->YPlaneSize = pbi->Configuration.VideoFrameWidth * pbi->Configuration.VideoFrameHeight; 
    pbi->UVPlaneSize = pbi->YPlaneSize / 4;  
    pbi->HFragments = pbi->Configuration.VideoFrameWidth / pbi->Configuration.HFragPixels;
    pbi->VFragments = pbi->Configuration.VideoFrameHeight / pbi->Configuration.VFragPixels;
    pbi->UnitFragments = ((pbi->VFragments * pbi->HFragments)*3)/2;
	pbi->YPlaneFragments = pbi->HFragments * pbi->VFragments;
	pbi->UVPlaneFragments = pbi->YPlaneFragments / 4;

    pbi->Configuration.YStride = (pbi->Configuration.VideoFrameWidth + STRIDE_EXTRA);
    pbi->Configuration.UVStride = pbi->Configuration.YStride / 2;
    pbi->ReconYPlaneSize = pbi->Configuration.YStride * (pbi->Configuration.VideoFrameHeight + STRIDE_EXTRA);
    pbi->ReconUVPlaneSize = pbi->ReconYPlaneSize / 4;
	FrameSize = pbi->ReconYPlaneSize + 2 * pbi->ReconUVPlaneSize;

    pbi->YDataOffset = 0;
    pbi->UDataOffset = pbi->YPlaneSize;
    pbi->VDataOffset = pbi->YPlaneSize + pbi->UVPlaneSize;
    pbi->ReconYDataOffset = 0;//(pbi->Configuration.YStride * UMV_BORDER) + UMV_BORDER;
    pbi->ReconUDataOffset = pbi->ReconYPlaneSize;// + (pbi->Configuration.UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2);
    pbi->ReconVDataOffset = pbi->ReconYPlaneSize + pbi->ReconUVPlaneSize;// + (pbi->Configuration.UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2);

	// Image dimensions in Macro-Blocks
	pbi->MBRows  = 4+(pbi->Configuration.VideoFrameHeight/16)  + ( pbi->Configuration.VideoFrameHeight%16 ? 1 : 0 );
	pbi->MBCols  = 4+(pbi->Configuration.VideoFrameWidth/16)  + ( pbi->Configuration.VideoFrameWidth%16 ? 1 : 0 );
	pbi->MacroBlocks = pbi->MBRows * pbi->MBCols;


	for(i=0;i<12;i++)
	{
		pbi->mvNearOffset[i] = MBOffset(NearMacroBlocks[i].row, NearMacroBlocks[i].col);
	}
#ifndef MAPCA
	ChangePostProcConfiguration(pbi->postproc, &pbi->Configuration);
#endif
	if(!VP5_AllocateFragmentInfo(pbi))
        return FALSE;

	if(!VP5_AllocateFrameInfo(pbi, FrameSize))
    {
        VP5_DeleteFragmentInfo(pbi);
        return FALSE;
    }

	// We have a differently output size than our scaling provides
	if( pbi->ScaleBuffer == 0 && pbi->OutputWidth &&
		(pbi->Configuration.VideoFrameWidth != pbi->OutputWidth ||
		pbi->Configuration.VideoFrameHeight != pbi->OutputHeight ) )
	{
		// we add 32 to outputwidth to insure that we have enough to overscale (ie scale to a size that's bigger 
		// than our output size) we do this now even though we don't use it so that we don't have to check border conditions
		pbi->ScaleBufferAlloc = (UINT8 *) 
			duck_malloc(32 + 3 * 
			(pbi->OutputWidth + 32) * 
			(pbi->OutputHeight + 32)* 
			sizeof(YUV_BUFFER_ENTRY) / 2, DMEM_GENERAL);  
		
		pbi->ScaleBuffer = (UINT8 *) ROUNDUP32(pbi->ScaleBufferAlloc );                  
	}
	
	// this is just so the post processor will work !!
	for(i=0;i<pbi->UnitFragments;i++)
		pbi->FragInfo[i].DisplayFragment = 1;


    return TRUE;

}

/****************************************************************************
 * 
 *  ROUTINE       :     InitialiseConfiguration
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Sets up the default starting pbi->Configuration.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_InitialiseConfiguration(PB_INSTANCE *pbi)
{  

    // IDCT table initialisation
    //InitDctTables();

    pbi->Configuration.HFragPixels = 8;
    pbi->Configuration.VFragPixels = 8;
} 

