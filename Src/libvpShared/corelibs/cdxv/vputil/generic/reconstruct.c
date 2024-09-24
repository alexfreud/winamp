/****************************************************************************
*
*   Module Title :     Reconstruct.c
*
*   Description  :     Block reconstruction functions.
*
****************************************************************************/
#define STRICT              // Strict type checking 

/****************************************************************************
*  Header Files
****************************************************************************/
#include "reconstruct.h"
#include "codec_common.h"

/****************************************************************************
 * 
 *  ROUTINE       : SatUnsigned8
 *
 *  INPUTS        : INT16 *DataBlock      : Pointer to 8x8 input block.
 *                  UINT32 ResultLineStep : Stride of output block.
 *                  UINT32 DataLineStep   : Stride of input block.
 *
 *  OUTPUTS       : UINT8 *ResultPtr      : Pointer to 8x8 output block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Saturates the input data to 8 bits unsigned and stores
 *                  in the output buffer.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void SatUnsigned8 ( UINT8 *ResultPtr, INT16 *DataBlock, UINT32 ResultLineStep, UINT32 DataLineStep )
{
    INT32 i;
       
     // Partly expanded loop
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        ResultPtr[0] = (char) LIMIT(DataBlock[0]);
        ResultPtr[1] = (char) LIMIT(DataBlock[1]);
        ResultPtr[2] = (char) LIMIT(DataBlock[2]);
        ResultPtr[3] = (char) LIMIT(DataBlock[3]);
        ResultPtr[4] = (char) LIMIT(DataBlock[4]);
        ResultPtr[5] = (char) LIMIT(DataBlock[5]);
        ResultPtr[6] = (char) LIMIT(DataBlock[6]);
        ResultPtr[7] = (char) LIMIT(DataBlock[7]);

        DataBlock += DataLineStep;
        ResultPtr += ResultLineStep;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : ScalarReconIntra
 *
 *  INPUTS        : INT16 *TmpDataBuffer : Pointer to 8x8 temporary buffer for internal use.
 *                  UINT16 *ChangePtr    : Pointer to 8x8 intra prediction block.
 *                  UINT32 LineStep      : Stride of reconstruction block.
 *
 *  OUTPUTS       : UINT8 *ReconPtr      : Pointer to 8x8 block to hold reconstructed block.
 *
 *  RETURNS       : None
 *
 *  FUNCTION      : Reconstructs an intra block.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void ScalarReconIntra ( INT16 *TmpDataBuffer, UINT8 *ReconPtr, UINT16 *ChangePtr, UINT32 LineStep )
{
    UINT32 i;
	INT16 *TmpDataPtr = TmpDataBuffer;

    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
   	{	
        TmpDataPtr[0] = (INT16) ( ChangePtr[0] + 128 );
        TmpDataPtr[1] = (INT16) ( ChangePtr[1] + 128 );
        TmpDataPtr[2] = (INT16) ( ChangePtr[2] + 128 );
        TmpDataPtr[3] = (INT16) ( ChangePtr[3] + 128 );
        TmpDataPtr[4] = (INT16) ( ChangePtr[4] + 128 );
        TmpDataPtr[5] = (INT16) ( ChangePtr[5] + 128 );
        TmpDataPtr[6] = (INT16) ( ChangePtr[6] + 128 );
        TmpDataPtr[7] = (INT16) ( ChangePtr[7] + 128 );

        TmpDataPtr += BLOCK_HEIGHT_WIDTH;
        ChangePtr  += BLOCK_HEIGHT_WIDTH;
    }

    // Saturate the output to unsigned 8 bit values in recon buffer
    SatUnsigned8 ( ReconPtr, TmpDataBuffer, LineStep, BLOCK_HEIGHT_WIDTH );
}

/****************************************************************************
 * 
 *  ROUTINE       : ScalarReconInter
 *
 *  INPUTS        : INT16 *TmpDataBuffer : Pointer to 8x8 temporary buffer for internal use.
 *                  UINT8 *RefPtr        : Pointer to 8x8 reference block.
 *                  INT16 *ChangePtr     : Pointer to 8x8 inter prediction error block.
 *                  UINT32 LineStep      : Stride of reference and output blocks.
 *
 *  OUTPUTS       : UINT8 *ReconPtr      : Pointer to 8x8 block to hold reconstructed block.
 *
 *  RETURNS       : None
 *
 *  FUNCTION      : Reconstructs an inter-coded block by adding a prediction
 *                  error to a reference block in the previous frame 
 *                  reconstruction buffer.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void ScalarReconInter ( INT16 *TmpDataBuffer, UINT8 *ReconPtr, UINT8 *RefPtr, INT16 *ChangePtr, UINT32 LineStep )
{
    UINT32 i;
	INT16 *TmpDataPtr = TmpDataBuffer;

    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
   	{	
		// Form each row
   	    TmpDataPtr[0] = (INT16)(RefPtr[0] + ChangePtr[0]);
   	    TmpDataPtr[1] = (INT16)(RefPtr[1] + ChangePtr[1]);
   	    TmpDataPtr[2] = (INT16)(RefPtr[2] + ChangePtr[2]);
   	    TmpDataPtr[3] = (INT16)(RefPtr[3] + ChangePtr[3]);
   	    TmpDataPtr[4] = (INT16)(RefPtr[4] + ChangePtr[4]);
   	    TmpDataPtr[5] = (INT16)(RefPtr[5] + ChangePtr[5]);
   	    TmpDataPtr[6] = (INT16)(RefPtr[6] + ChangePtr[6]);
   	    TmpDataPtr[7] = (INT16)(RefPtr[7] + ChangePtr[7]);

        // Next row of Block
		ChangePtr  += BLOCK_HEIGHT_WIDTH;
        TmpDataPtr += BLOCK_HEIGHT_WIDTH;
        RefPtr     += LineStep; 
    }

    // Saturate the output to unsigned 8 bit values in recon buffer
    SatUnsigned8 ( ReconPtr, TmpDataBuffer, LineStep, BLOCK_HEIGHT_WIDTH );
}

/****************************************************************************
 * 
 *  ROUTINE       : ScalarReconInterHalfPixel2
 *
 *  INPUTS        : INT16 *TmpDataBuffer : Pointer to 8x8 temporary buffer for internal use.
 *                  UINT8 *RefPtr1       : Pointer to first 8x8 reference block.
 *                  UINT8 *RefPtr2       : Pointer to second 8x8 reference block.
 *                  INT16 *ChangePtr     : Pointer to 8x8 inter prediction error block.
 *                  UINT32 LineStep      : Stride of reference blocks.
 *
 *  OUTPUTS       : UINT8 *ReconPtr      : Pointer to 8x8 block to hold reconstructed block.
 *
 *  RETURNS       : None
 *
 *  FUNCTION      : Reconstructs an inter-coded block by adding a prediction
 *                  error to a reference block computed by averaging the two
 *                  specified reference blocks. The two reference blocks are
 *                  those that bracket the 1/2-pixel accuracy motion vector.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void ScalarReconInterHalfPixel2 
(
    INT16 *TmpDataBuffer,
    UINT8 *ReconPtr, 
    UINT8 *RefPtr1,
    UINT8 *RefPtr2, 
    INT16 *ChangePtr,
    UINT32 LineStep 
)
{
    UINT32  i;
	INT16 *TmpDataPtr = TmpDataBuffer;

    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
   	{	
		// Form each row
        TmpDataPtr[0] = (INT16)( (((INT32)RefPtr1[0] + (INT32)RefPtr2[0]) >> 1) + ChangePtr[0] );
   	    TmpDataPtr[1] = (INT16)( (((INT32)RefPtr1[1] + (INT32)RefPtr2[1]) >> 1) + ChangePtr[1] );
   	    TmpDataPtr[2] = (INT16)( (((INT32)RefPtr1[2] + (INT32)RefPtr2[2]) >> 1) + ChangePtr[2] );
   	    TmpDataPtr[3] = (INT16)( (((INT32)RefPtr1[3] + (INT32)RefPtr2[3]) >> 1) + ChangePtr[3] );
   	    TmpDataPtr[4] = (INT16)( (((INT32)RefPtr1[4] + (INT32)RefPtr2[4]) >> 1) + ChangePtr[4] );
   	    TmpDataPtr[5] = (INT16)( (((INT32)RefPtr1[5] + (INT32)RefPtr2[5]) >> 1) + ChangePtr[5] );
   	    TmpDataPtr[6] = (INT16)( (((INT32)RefPtr1[6] + (INT32)RefPtr2[6]) >> 1) + ChangePtr[6] );
   	    TmpDataPtr[7] = (INT16)( (((INT32)RefPtr1[7] + (INT32)RefPtr2[7]) >> 1) + ChangePtr[7] );

        // Next row of Block
		ChangePtr  += BLOCK_HEIGHT_WIDTH;
        TmpDataPtr += BLOCK_HEIGHT_WIDTH;
        RefPtr1    += LineStep; 
        RefPtr2    += LineStep; 
    }

    // Saturate the output to unsigned 8 bit values in recon buffer
    SatUnsigned8( ReconPtr, TmpDataBuffer, LineStep, BLOCK_HEIGHT_WIDTH );
}

/****************************************************************************
 * 
 *  ROUTINE       : ReconBlock_C
 *  
 *  INPUTS        : INT16 *SrcBlock    : Pointer to 8x8 prediction error.
 *					INT16 *ReconRefPtr : Pointer to 8x8 block prediction.
 *                  UINT32 LineStep    : Stride of output block.
 *
 *  OUTPUTS       : UINT8 *DestBlock   : Pointer to 8x8 reconstructed block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Reconstrut a block by adding the prediction error
 *                  block to the source block and clipping values.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void ReconBlock_C ( INT16 *SrcBlock, INT16 *ReconRefPtr, UINT8 *DestBlock, UINT32 LineStep )
{
    UINT32 i;
    INT16 *SrcBlockPtr = SrcBlock;

    // For each block row
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        SrcBlock[0] = (INT16)(SrcBlock[0] + ReconRefPtr[0]);
        SrcBlock[1] = (INT16)(SrcBlock[1] + ReconRefPtr[1]);
        SrcBlock[2] = (INT16)(SrcBlock[2] + ReconRefPtr[2]);
        SrcBlock[3] = (INT16)(SrcBlock[3] + ReconRefPtr[3]);
        SrcBlock[4] = (INT16)(SrcBlock[4] + ReconRefPtr[4]);
        SrcBlock[5] = (INT16)(SrcBlock[5] + ReconRefPtr[5]);
        SrcBlock[6] = (INT16)(SrcBlock[6] + ReconRefPtr[6]);
        SrcBlock[7] = (INT16)(SrcBlock[7] + ReconRefPtr[7]);
        
        // Next row...
        SrcBlock    += BLOCK_HEIGHT_WIDTH;
        ReconRefPtr += BLOCK_HEIGHT_WIDTH;
    }

    // Saturate the output to unsigned 8 bit values in recon buffer
    SatUnsigned8( DestBlock, SrcBlockPtr, LineStep, BLOCK_HEIGHT_WIDTH );
}
