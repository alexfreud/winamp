/****************************************************************************
*
*   Module Title :     FrameIni.c
*
*   Description  :     Initialization functions.
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include "pbdll.h"
#include "duck_mem.h"
#include <intsafe.h> // TODO: make a mac version of this
/****************************************************************************
*  Module Static Variables
****************************************************************************/  
static const struct 
{
	INT32 row;
	INT32 col;
} NearMacroBlocks[12] = 
{
	{ -1,  0 },
	{  0, -1 },
	{ -1, -1 },
	{ -1,  1 },
	{ -2,  0 },
	{  0, -2 },
	{ -1, -2 },
	{ -2, -1 },
	{ -2,  1 },
	{ -1,  2 },
	{ -2, -2 },
	{ -2,  2 }
};

/****************************************************************************
* 
*  ROUTINE       :     VP6_InitMBI
*
*  INPUTS        :     PB_INSTANCE * pbi : Pointer to decoder instance.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     void
*
*  FUNCTION      :     Initialize MBI structure.
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void 
VP6_InitMBI(PB_INSTANCE *pbi)
{

	pbi->mbi.blockDxInfo[0].ZeroRunProbsBasePtr = 
		pbi->mbi.blockDxInfo[1].ZeroRunProbsBasePtr = 
		pbi->mbi.blockDxInfo[2].ZeroRunProbsBasePtr = 
		pbi->mbi.blockDxInfo[3].ZeroRunProbsBasePtr = 
		pbi->mbi.blockDxInfo[4].ZeroRunProbsBasePtr = 
		pbi->mbi.blockDxInfo[5].ZeroRunProbsBasePtr = (UINT8 *)pbi->ZeroRunProbs;

	pbi->mbi.blockDxInfo[0].AcProbsBasePtr = 
		pbi->mbi.blockDxInfo[1].AcProbsBasePtr = 
		pbi->mbi.blockDxInfo[2].AcProbsBasePtr = 
		pbi->mbi.blockDxInfo[3].AcProbsBasePtr = pbi->AcProbs + ACProbOffset(0,0,0,0);
	pbi->mbi.blockDxInfo[4].AcProbsBasePtr = 
		pbi->mbi.blockDxInfo[5].AcProbsBasePtr = pbi->AcProbs + ACProbOffset(1,0,0,0);

	pbi->mbi.blockDxInfo[0].DcProbsBasePtr = 
		pbi->mbi.blockDxInfo[1].DcProbsBasePtr = 
		pbi->mbi.blockDxInfo[2].DcProbsBasePtr = 
		pbi->mbi.blockDxInfo[3].DcProbsBasePtr = pbi->DcProbs + DCProbOffset(0,0);
	pbi->mbi.blockDxInfo[4].DcProbsBasePtr = 
		pbi->mbi.blockDxInfo[5].DcProbsBasePtr = pbi->DcProbs + DCProbOffset(1,0);

	pbi->mbi.blockDxInfo[0].DcNodeContextsBasePtr = 
		pbi->mbi.blockDxInfo[1].DcNodeContextsBasePtr = 
		pbi->mbi.blockDxInfo[2].DcNodeContextsBasePtr = 
		pbi->mbi.blockDxInfo[3].DcNodeContextsBasePtr = pbi->DcNodeContexts + DcNodeOffset(0,0,0);
	pbi->mbi.blockDxInfo[4].DcNodeContextsBasePtr = 
		pbi->mbi.blockDxInfo[5].DcNodeContextsBasePtr = pbi->DcNodeContexts + DcNodeOffset(1,0,0);

	pbi->mbi.blockDxInfo[0].dequantPtr = pbi->quantizer->dequant_coeffs[VP6_QTableSelect[0]];
	pbi->mbi.blockDxInfo[1].dequantPtr = pbi->quantizer->dequant_coeffs[VP6_QTableSelect[1]];
	pbi->mbi.blockDxInfo[2].dequantPtr = pbi->quantizer->dequant_coeffs[VP6_QTableSelect[2]];
	pbi->mbi.blockDxInfo[3].dequantPtr = pbi->quantizer->dequant_coeffs[VP6_QTableSelect[3]];
	pbi->mbi.blockDxInfo[4].dequantPtr = pbi->quantizer->dequant_coeffs[VP6_QTableSelect[4]];
	pbi->mbi.blockDxInfo[5].dequantPtr = pbi->quantizer->dequant_coeffs[VP6_QTableSelect[5]];

	pbi->mbi.blockDxInfo[0].LastDc = 
		pbi->mbi.blockDxInfo[1].LastDc = 
		pbi->mbi.blockDxInfo[2].LastDc = 
		pbi->mbi.blockDxInfo[3].LastDc = pbi->fc.LastDcY;
	pbi->mbi.blockDxInfo[4].LastDc = pbi->fc.LastDcU;
	pbi->mbi.blockDxInfo[5].LastDc = pbi->fc.LastDcV;

	pbi->mbi.blockDxInfo[0].Left = &pbi->fc.LeftY[0];
	pbi->mbi.blockDxInfo[1].Left = &pbi->fc.LeftY[0];
	pbi->mbi.blockDxInfo[2].Left = &pbi->fc.LeftY[1];
	pbi->mbi.blockDxInfo[3].Left = &pbi->fc.LeftY[1];
	pbi->mbi.blockDxInfo[4].Left = &pbi->fc.LeftU;
	pbi->mbi.blockDxInfo[5].Left = &pbi->fc.LeftV;

	pbi->mbi.blockDxInfo[0].MvShift =
		pbi->mbi.blockDxInfo[1].MvShift =
		pbi->mbi.blockDxInfo[2].MvShift =
		pbi->mbi.blockDxInfo[3].MvShift = Y_MVSHIFT;
	pbi->mbi.blockDxInfo[4].MvShift =
		pbi->mbi.blockDxInfo[5].MvShift = UV_MVSHIFT;

	pbi->mbi.blockDxInfo[0].MvModMask =
		pbi->mbi.blockDxInfo[1].MvModMask =
		pbi->mbi.blockDxInfo[2].MvModMask =
		pbi->mbi.blockDxInfo[3].MvModMask = Y_MVMODMASK;
	pbi->mbi.blockDxInfo[4].MvModMask =
		pbi->mbi.blockDxInfo[5].MvModMask = UV_MVMODMASK;

	pbi->mbi.blockDxInfo[0].CurrentReconStride =
		pbi->mbi.blockDxInfo[1].CurrentReconStride =
		pbi->mbi.blockDxInfo[2].CurrentReconStride =
		pbi->mbi.blockDxInfo[3].CurrentReconStride = pbi->Configuration.YStride;
	pbi->mbi.blockDxInfo[4].CurrentReconStride =
		pbi->mbi.blockDxInfo[5].CurrentReconStride = pbi->Configuration.UVStride;

	pbi->mbi.blockDxInfo[0].FrameReconStride =
		pbi->mbi.blockDxInfo[1].FrameReconStride =
		pbi->mbi.blockDxInfo[2].FrameReconStride =
		pbi->mbi.blockDxInfo[3].FrameReconStride = pbi->Configuration.YStride;
	pbi->mbi.blockDxInfo[4].FrameReconStride =
		pbi->mbi.blockDxInfo[5].FrameReconStride = pbi->Configuration.UVStride;

	// Default clear data area down to 0s
	memset(pbi->mbi.blockDxInfo[0].coeffsPtr, 0, 6*64*sizeof(Q_LIST_ENTRY));

	//______ compressor only ______
	pbi->mbi.blockDxInfo[0].FrameSourceStride =
		pbi->mbi.blockDxInfo[1].FrameSourceStride =
		pbi->mbi.blockDxInfo[2].FrameSourceStride =
		pbi->mbi.blockDxInfo[3].FrameSourceStride = pbi->Configuration.VideoFrameWidth;
	pbi->mbi.blockDxInfo[4].FrameSourceStride =
		pbi->mbi.blockDxInfo[5].FrameSourceStride = pbi->Configuration.VideoFrameWidth/2;

	pbi->mbi.blockDxInfo[0].CurrentSourceStride =
		pbi->mbi.blockDxInfo[1].CurrentSourceStride =
		pbi->mbi.blockDxInfo[2].CurrentSourceStride =
		pbi->mbi.blockDxInfo[3].CurrentSourceStride = pbi->Configuration.VideoFrameWidth;
	pbi->mbi.blockDxInfo[4].CurrentSourceStride =
		pbi->mbi.blockDxInfo[5].CurrentSourceStride = pbi->Configuration.VideoFrameWidth/2;

	pbi->mbi.blockDxInfo[0].Plane =
		pbi->mbi.blockDxInfo[1].Plane =
		pbi->mbi.blockDxInfo[2].Plane =
		pbi->mbi.blockDxInfo[3].Plane = 0;
	pbi->mbi.blockDxInfo[4].Plane =
		pbi->mbi.blockDxInfo[5].Plane = 1;
	//______ compressor only ______

}

/****************************************************************************
* 
*  ROUTINE       :     VP6_DeleteFragmentInfo
*
*  INPUTS        :     PB_INSTANCE * pbi : Pointer to decoder instance.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     void
*
*  FUNCTION      :     De-allocates memory associated with decoder data structures.
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void VP6_DeleteFragmentInfo ( PB_INSTANCE *pbi )
{
	// Free prior allocs if present
	if(pbi->mbi.blockDxInfo[0].coeffsPtr)
		duck_free(pbi->mbi.blockDxInfo[0].coeffsPtr);
	pbi->mbi.blockDxInfo[0].coeffsPtr = 0;

	if(	pbi->FragInfo)
		duck_free(pbi->FragInfo);
	pbi->FragInfo      = 0;

	if(	pbi->fc.AboveY)
		duck_free(pbi->fc.AboveY);
	pbi->fc.AboveY      = 0;

	if(	pbi->fc.AboveU)
		duck_free(pbi->fc.AboveU);
	pbi->fc.AboveU      = 0;

	if(	pbi->fc.AboveV)
		duck_free(pbi->fc.AboveV);
	pbi->fc.AboveV      = 0;

	if(	pbi->MBInterlaced)
		duck_free(pbi->MBInterlaced);
	pbi->MBInterlaced      = 0;

	if(	pbi->MBMotionVector)
		duck_free(pbi->MBMotionVector);
	pbi->MBMotionVector      = 0;

	if(	pbi->predictionMode)
		duck_free(pbi->predictionMode);
	pbi->predictionMode      = 0;

#ifdef DMAREADREFERENCE
	if(pbi->ReferenceBlocks)
		duck_free(pbi->ReferenceBlocks);
	pbi->ReferenceBlocks      = 0;
#endif
#ifdef DMAWRITERECON
	if(pbi->ReconstructedMBs)
		duck_free(pbi->ReconstructedMBs);
	pbi->ReconstructedMBs      = 0;
#endif
}

/****************************************************************************
* 
*  ROUTINE       :     VP6_AllocateFragmentInfo
*
*  INPUTS        :     PB_INSTANCE * pbi : Pointer to decoder instance.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     BOOL: TRUE if successful, FALSE on error.
*
*  FUNCTION      :     Initializes the Playback instance passed in.
*
*  SPECIAL NOTES :     Uses duck_memalign to ensure data structures are aligned
*                      on 32-byte boundaries to improve cache performance. 
*
****************************************************************************/
BOOL VP6_AllocateFragmentInfo ( PB_INSTANCE *pbi )
{
	// Clear any existing info
	VP6_DeleteFragmentInfo(pbi);

	pbi->mbi.blockDxInfo[0].coeffsPtr = (Q_LIST_ENTRY *) duck_memalign(32, sizeof(Q_LIST_ENTRY)*64*6, DMEM_GENERAL);
	if(!pbi->mbi.blockDxInfo[0].coeffsPtr) {VP6_DeleteFragmentInfo(pbi); return FALSE;}
	pbi->mbi.blockDxInfo[1].coeffsPtr = pbi->mbi.blockDxInfo[0].coeffsPtr + 64;
	pbi->mbi.blockDxInfo[2].coeffsPtr = pbi->mbi.blockDxInfo[1].coeffsPtr + 64;
	pbi->mbi.blockDxInfo[3].coeffsPtr = pbi->mbi.blockDxInfo[2].coeffsPtr + 64;
	pbi->mbi.blockDxInfo[4].coeffsPtr = pbi->mbi.blockDxInfo[3].coeffsPtr + 64;
	pbi->mbi.blockDxInfo[5].coeffsPtr = pbi->mbi.blockDxInfo[4].coeffsPtr + 64;

	// context allocations
	pbi->fc.AboveY = (BLOCK_CONTEXT *) duck_memalign(32, (8+pbi->HFragments) * sizeof(BLOCK_CONTEXT), DMEM_GENERAL);
	if(!pbi->fc.AboveY) { VP6_DeleteFragmentInfo(pbi); return FALSE;}

	pbi->fc.AboveU = (BLOCK_CONTEXT *) duck_memalign(32, (8+pbi->HFragments / 2) * sizeof(BLOCK_CONTEXT), DMEM_GENERAL);
	if(!pbi->fc.AboveU) { VP6_DeleteFragmentInfo(pbi); return FALSE;}

	pbi->fc.AboveV = (BLOCK_CONTEXT *) duck_memalign(32, (8+pbi->HFragments / 2) * sizeof(BLOCK_CONTEXT), DMEM_GENERAL);
	if(!pbi->fc.AboveV) { VP6_DeleteFragmentInfo(pbi); return FALSE;}

	// the encoder is the only thing using this move it to compdll
	pbi->MBInterlaced = (char *) duck_memalign(32, pbi->MacroBlocks * sizeof(char), DMEM_GENERAL);
	if(!pbi->MBInterlaced) { VP6_DeleteFragmentInfo(pbi); 	return FALSE; }

	pbi->predictionMode = (char *) duck_memalign(32, pbi->MacroBlocks * sizeof(char), DMEM_GENERAL);
	if(!pbi->predictionMode) { VP6_DeleteFragmentInfo(pbi); return FALSE;}

	pbi->MBMotionVector = (MOTION_VECTOR *) duck_memalign(32, pbi->MacroBlocks * sizeof(MOTION_VECTOR ), DMEM_GENERAL);
	if(!pbi->MBMotionVector) { VP6_DeleteFragmentInfo(pbi); return FALSE;}

	// the encoder is the only thing using this move it to compdll
	pbi->FragInfo = (FRAG_INFO *) duck_memalign(32, pbi->UnitFragments * sizeof(FRAG_INFO), DMEM_GENERAL);
	if(!pbi->FragInfo) { VP6_DeleteFragmentInfo(pbi); return FALSE;}

#ifdef DMAREADREFERENCE
	pbi->ReferenceBlocks=(UINT8(*)[192])duck_memalign(32, 6*192, DMEM_GENERAL);
	if(!pbi->ReferenceBlocks){ VP6_DeleteFragmentInfo(pbi); return FALSE;}
#endif

#ifdef DMAWRITERECON
	pbi->ReconstructedMBs = (UINT8*) duck_memalign(32, 768, DMEM_GENERAL);
	if(!pbi->ReconstructedMBs){ VP6_DeleteFragmentInfo(pbi); return FALSE;}
#endif

	return TRUE;
}

/****************************************************************************
* 
*  ROUTINE       :     VP6_DeleteFrameInfo
*
*  INPUTS        :     PB_INSTANCE * pbi : Pointer to decoder instance.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     void
*
*  FUNCTION      :     De-allocate memory associated with frame level data
*                      structures.
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void VP6_DeleteFrameInfo ( PB_INSTANCE *pbi )
{
	if(pbi->ThisFrameRecon )
		duck_free(pbi->ThisFrameRecon );
	if(pbi->GoldenFrame)
		duck_free(pbi->GoldenFrame);
	if(pbi->LastFrameRecon)
		duck_free(pbi->LastFrameRecon);
	if(pbi->PostProcessBuffer)
		duck_free(pbi->PostProcessBuffer);

	pbi->ThisFrameRecon         = 0;
	pbi->GoldenFrame            = 0;
	pbi->LastFrameRecon         = 0;
	pbi->PostProcessBuffer      = 0;
}

/****************************************************************************
* 
*  ROUTINE       :     VP6_AllocateFrameInfo
*
*  INPUTS        :     PB_INSTANCE * pbi      : Pointer to decoder instance.
*                      unsigned int FrameSize : Size of the YUV frame in bytes.
*
*  OUTPUTS       :     None
*
*  RETURNS       :     BOOL: TRUE if successful, FALSE on error.
*
*  FUNCTION      :     Initializes the Playback instance passed in
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
BOOL VP6_AllocateFrameInfo ( PB_INSTANCE *pbi, unsigned int FrameSize )
{
	// clear any existing info
	VP6_DeleteFrameInfo(pbi);

	// Allocate frame buffers:
	// Added 2 extra lines to framebuffer so that copy12x12 doesn't fail
	// when we have a large motion vector in V on the last v block.  
	// Note : We never use these pixels anyway so this doesn't hurt.
	pbi->ThisFrameRecon = (UINT8 *)duck_memalign(32, pbi->Configuration.YStride+FrameSize*sizeof(YUV_BUFFER_ENTRY), DMEM_GENERAL);
	if(!pbi->ThisFrameRecon) { VP6_DeleteFrameInfo(pbi); return FALSE;}

	pbi->GoldenFrame = (UINT8 *)duck_memalign(32, pbi->Configuration.YStride+FrameSize*sizeof(YUV_BUFFER_ENTRY ), DMEM_GENERAL);
	if(!pbi->GoldenFrame) { VP6_DeleteFrameInfo(pbi); return FALSE;}

	pbi->LastFrameRecon = (UINT8 *)duck_memalign(32, pbi->Configuration.YStride+FrameSize*sizeof(YUV_BUFFER_ENTRY), DMEM_GENERAL);
	if(!pbi->LastFrameRecon) { VP6_DeleteFrameInfo(pbi); return FALSE;}

	pbi->PostProcessBuffer = (UINT8 *)duck_memalign(32, pbi->Configuration.YStride+FrameSize*sizeof(YUV_BUFFER_ENTRY), DMEM_GENERAL);
	if(!pbi->PostProcessBuffer) { VP6_DeleteFrameInfo(pbi); return FALSE;}

	return TRUE;
}

/****************************************************************************
* 
*  ROUTINE       :     VP6_InitFrameDetails
*
*  INPUTS        :     PB_INSTANCE * pbi : Pointer to decoder instance.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     BOOL: TRUE on success, FALSE on failure.
*
*  FUNCTION      :     Initialises various details about the frame.
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
BOOL VP6_InitFrameDetails ( PB_INSTANCE *pbi )
{
	UINT32 i;
	int FrameSize;

	if ( pbi->CPUFree > 0 )
		VP6_SetPbParam( pbi, PBC_SET_CPUFREE, pbi->CPUFree );

	/* Set the frame size etc. */                                                        
	if (UIntMult(pbi->Configuration.VideoFrameWidth, pbi->Configuration.VideoFrameHeight, &pbi->YPlaneSize) == S_OK)
	{
		pbi->UVPlaneSize = pbi->YPlaneSize / 4;  
		pbi->HFragments = pbi->Configuration.VideoFrameWidth / pbi->Configuration.HFragPixels;
		pbi->VFragments = pbi->Configuration.VideoFrameHeight / pbi->Configuration.VFragPixels;
		if (UIntMult(pbi->VFragments, pbi->HFragments, &pbi->YPlaneFragments) == S_OK && 
			UIntMult(pbi->YPlaneFragments, 3, &pbi->UnitFragments) == S_OK)
		{
			pbi->UnitFragments /= 2;
			pbi->UVPlaneFragments = pbi->YPlaneFragments / 4;

			pbi->Configuration.YStride = (pbi->Configuration.VideoFrameWidth + STRIDE_EXTRA);
			pbi->Configuration.UVStride = pbi->Configuration.YStride / 2;

			if (UIntMult(pbi->Configuration.YStride, pbi->Configuration.VideoFrameHeight + STRIDE_EXTRA, &pbi->ReconYPlaneSize) == S_OK)
			{
				pbi->ReconUVPlaneSize = pbi->ReconYPlaneSize / 4;

				FrameSize = pbi->ReconYPlaneSize + 2 * pbi->ReconUVPlaneSize;

				pbi->YDataOffset = 0;
				pbi->UDataOffset = pbi->YPlaneSize;
				pbi->VDataOffset = pbi->YPlaneSize + pbi->UVPlaneSize;
				pbi->ReconYDataOffset = 0;
				pbi->ReconUDataOffset = pbi->ReconYPlaneSize;
				pbi->ReconVDataOffset = pbi->ReconYPlaneSize + pbi->ReconUVPlaneSize;

				// Image dimensions in Macro-Blocks
				pbi->MBRows  = (2*BORDER_MBS)+(pbi->Configuration.VideoFrameHeight/16)  + ( pbi->Configuration.VideoFrameHeight%16 ? 1 : 0 );
				pbi->MBCols  = (2*BORDER_MBS)+(pbi->Configuration.VideoFrameWidth/16)  + ( pbi->Configuration.VideoFrameWidth%16 ? 1 : 0 );
				pbi->MacroBlocks = pbi->MBRows * pbi->MBCols;

				for( i=0; i<12; i++ )
					pbi->mvNearOffset[i] = MBOffset(NearMacroBlocks[i].row, NearMacroBlocks[i].col);

				ChangePostProcConfiguration(pbi->postproc, &pbi->Configuration);

				if ( !VP6_AllocateFragmentInfo(pbi) )
					return FALSE;

				if ( !VP6_AllocateFrameInfo(pbi, FrameSize) )
				{
					VP6_DeleteFragmentInfo(pbi);
					return FALSE;
				}

				// We have a differently output size than our scaling provides
				if ( pbi->ScaleBuffer == 0 && pbi->OutputWidth &&
					(pbi->Configuration.VideoFrameWidth != pbi->OutputWidth ||
					pbi->Configuration.VideoFrameHeight != pbi->OutputHeight ) )
				{
					// Add 32 to outputwidth to ensure that we have enough to overscale 
					// (ie scale to a size that's bigger than our output size). Do this
					// now even though we don't use it so we don't have to check border conditions.
					pbi->ScaleBuffer = (UINT8 *) 
						duck_malloc(32 + 3 * 
						(pbi->OutputWidth + 32) * 
						(pbi->OutputHeight + 32)* 
						sizeof(YUV_BUFFER_ENTRY) / 2, DMEM_GENERAL);

				}


				VP6_InitMBI(pbi);

				return TRUE;
			}
		}
	}
	return FALSE;
}

/****************************************************************************
* 
*  ROUTINE       :     VP6_InitialiseConfiguration
*
*  INPUTS        :     PB_INSTANCE * pbi : Pointer to decoder instance.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     void
*
*  FUNCTION      :     Sets the base size of a coding block (8x8).
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void VP6_InitialiseConfiguration ( PB_INSTANCE *pbi )
{  
	pbi->Configuration.HFragPixels = 8;
	pbi->Configuration.VFragPixels = 8;
} 
