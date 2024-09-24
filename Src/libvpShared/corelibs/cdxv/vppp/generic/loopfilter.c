/****************************************************************************
*
*   Module Title :     loopfilter.c
*
*   Description  :     Loop filter functions.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "postp.h"

/****************************************************************************
*  Macros
****************************************************************************/
#define Mod8(x) ( (x) & 7 )

/****************************************************************************
*  Exports
****************************************************************************/
UINT32 LoopFilterLimitValuesV1[Q_TABLE_SIZE] = 
{
    30, 25, 20, 20, 15, 15, 14, 14,
    13, 13, 12, 12, 11, 11, 10, 10, 
     9,  9,  8,  8,  7,  7,  7,  7,
     6,  6,  6,  6,  5,  5,  5,  5,
     4,  4,  4,  4,  3,  3,  3,  3,  
     2,  2,  2,  2,  2,  2,  2,  2,  
     0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0 
};

UINT32 *LoopFilterLimitValuesV2;

UINT32 LoopFilterLimitValuesVp4[Q_TABLE_SIZE] =
{
    30, 25, 20, 20, 15, 15, 14, 14,
    13, 13, 12, 12, 11, 11, 10, 10, 
     9,  9,  8,  8,  7,  7,  7,  7,
     6,  6,  6,  6,  5,  5,  5,  5,
     4,  4,  4,  4,  3,  3,  3,  3,  
     2,  2,  2,  2,  2,  2,  2,  2,  
     2,  2,  2,  2,  2,  2,  2,  2,  
     1,  1,  1,  1,  1,  1,  1,  1 
};

UINT32 LoopFilterLimitValuesVp5[Q_TABLE_SIZE] = 
{
    14, 14, 13, 13, 12, 12, 10, 10, 
	10, 10,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  
	 8,  8,  8,  8,  7,  7,  7,  7,	
	 7,  7,  6,  6,  6,  6,  6,  6,	
	 5,  5,  5,  5,  4,  4,  4,  4,  
     4,  4,  4,  3,  3,  3,  3,  2 
};

UINT32 LoopFilterLimitValuesVp6[Q_TABLE_SIZE] = 
{ 
    14, 14, 13, 13, 12, 12, 10, 10, 
	10, 10,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  
	 8,  8,  8,  8,  7,  7,  7,  7,	
	 7,  7,  6,  6,  6,  6,  6,  6,	
	 5,  5,  5,  5,  4,  4,  4,  4,  
     4,  4,  4,  3,  3,  3,  3,  2 
};

/****************************************************************************
 * 
 *  ROUTINE       : SetupBoundingValueArray_Generic
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi : Pointer to post-processor instance.
 *                  INT32 FLimit           : Value to use as limit.
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : INT32: Pointer to LUT position 0 (cast to UINT32)
 *
 *  FUNCTION      : Set up the bounding value array.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
INT32 *SetupBoundingValueArray_Generic ( POSTPROC_INSTANCE *ppi, INT32 FLimit )
{
    INT32 i;
    INT32 *BoundingValuePtr;

    BoundingValuePtr = &ppi->FiltBoundingValue[256];

    // Set up the bounding value array
    memset ( ppi->FiltBoundingValue, 0, (512*sizeof(*ppi->FiltBoundingValue)) );
    for ( i=0; i<FLimit; i++ )
    {
        BoundingValuePtr[-i-FLimit] = (-FLimit+i);
        BoundingValuePtr[-i]        = -i;
        BoundingValuePtr[i]         = i;
        BoundingValuePtr[i+FLimit]  = FLimit-i;
    }

    return BoundingValuePtr;
}

/****************************************************************************
 * 
 *  ROUTINE       : SetupLoopFilter
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi : Pointer to post-processor instance.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Initializes LUTs and function pointer for loop filter.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void SetupLoopFilter ( POSTPROC_INSTANCE *ppi )
{
    INT32 FLimit; 

    FLimit = LoopFilterLimitValuesV2[ppi->FrameQIndex];

    if ( ppi->Vp3VersionNo >= 2 )
        ppi->BoundingValuePtr = SetupBoundingValueArray_Generic(ppi, FLimit);
    else
        ppi->BoundingValuePtr = SetupBoundingValueArray ( ppi, FLimit );
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterHoriz_Generic
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi  : Pointer to post-processor instance.
 *                  UINT8 *PixelPtr         : Pointer to Pointer to input data.
 *                  INT32 LineLength        : Stride of input data.
 *                  INT32 *BoundingValuePtr : Pointer to array of bounding values.
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies the 4-tap loop-filter across vertical edge,
 *                  i.e. filter is applied horizontally.
 *
 *  SPECIAL NOTES : 4-Tap filter used is (1, -3, 3, -1).
 *
 ****************************************************************************/
void FilterHoriz_Generic
(
    POSTPROC_INSTANCE *ppi, 
    UINT8 *PixelPtr, 
    INT32 LineLength, 
    INT32 *BoundingValuePtr
)
{
	INT32 j;
	INT32 FiltVal;
    UINT8 *LimitTable = &LimitVal_VP31[VAL_RANGE];
	(void)ppi;

	for ( j=0; j<8; j++ )
	{            
		FiltVal = PixelPtr[0] - (PixelPtr[1]*3) + (PixelPtr[2]*3) - PixelPtr[3];
		FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[1] = LimitTable[(INT32)PixelPtr[1] + FiltVal];
		PixelPtr[2] = LimitTable[(INT32)PixelPtr[2] - FiltVal];
		
		PixelPtr += LineLength;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterVert_Generic
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi  : Pointer to post-processor instance.
 *                  UINT8 *PixelPtr         : Pointer to Pointer to input data.
 *                  INT32 LineLength        : Stride of input data.
 *                  INT32 *BoundingValuePtr : Pointer to array of bounding values.
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies the 4-tap loop-filter across horizontal edge,
 *                  i.e. filter is applied vertically.
 *
 *  SPECIAL NOTES : 4-Tap filter used is (1, -3, 3, -1).
 *
 ****************************************************************************/
void FilterVert_Generic 
( 
    POSTPROC_INSTANCE *ppi,
    UINT8 *PixelPtr,
    INT32 LineLength,
    INT32 *BoundingValuePtr
)
{
	INT32 j;
	INT32 FiltVal;
    UINT8 * LimitTable = &LimitVal_VP31[VAL_RANGE];
	(void)ppi;

	for ( j=0; j<8; j++ )
	{            
		FiltVal =   (INT32)PixelPtr[-(2 * LineLength)]
			      - ((INT32)PixelPtr[- LineLength] * 3)
			      + ((INT32)PixelPtr[0] * 3)
			      -  (INT32)PixelPtr[LineLength];
		
		FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[-LineLength] = LimitTable[(INT32)PixelPtr[-LineLength] + FiltVal];
		PixelPtr[0]           = LimitTable[(INT32)PixelPtr[0] - FiltVal];
		
		PixelPtr++;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : Bound
 *
 *  INPUTS        : UINT32 FLimit  : Limit to use in computing bounding value.
 *                  INT32  FiltVal : Value to have bounds applied to.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : INT32: 
 *
 *  FUNCTION      : Computes a bounded Filtval based on specified Flimit.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
 
#if defined (_WIN32_WCE)
INT32 Bound ( UINT32 FLimit, INT32 FiltVal )
#else
INLINE INT32 Bound ( UINT32 FLimit, INT32 FiltVal )
#endif
{
    INT32 Clamp;
    INT32 FiltSign;
    INT32 NewSign;

    Clamp = 2 * FLimit;

    // Next 3 lines are fast way to find abs...
    FiltSign = (FiltVal >> 31);         // Sign extension makes FiltSign all 0's or all 1's
    FiltVal ^= FiltSign;                // FiltVal is then 1's complement of value if -ve
    FiltVal -= FiltSign;                // Filtval = abs Filtval

    FiltVal *= (FiltVal < Clamp);       // clamp filter value to 2 times limit

    FiltVal -= FLimit;                  // subtract limit value 
    
    // Next 3 lines are fast way to find abs...
    NewSign = (FiltVal >> 31);          // Sign extension makes NewSign all 0's or all 1's
    FiltVal ^= NewSign;                 // FiltVal is then 1's complement of value if -ve
    FiltVal -= NewSign;                 // FiltVal = abs FiltVal

    FiltVal = FLimit - FiltVal;         // flimit - abs (filtVal - flimit)
    
    FiltVal += FiltSign;                // convert back to signed value
    FiltVal ^= FiltSign;            
    
    return FiltVal;
}

/****************************************************************************
 * 
 *  ROUTINE       : FilteringHoriz_8_C
 *
 *  INPUTS        : UINT32 QValue : Current quatizer level.
 *                  UINT8 *Src    : Pointer to data to be filtered.
 *                  INT32 Pitch   : Pitch of input data.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies horizontal filter across vertical edge inside
 *                  block with Q-dependent limits.
 *
 *  SPECIAL NOTES : 4-Tap filter used is (1, -3, 3, -1).
 *
 ****************************************************************************/                       
void FilteringHoriz_8_C ( UINT32 QValue, UINT8 *Src, INT32 Pitch )
{    
    INT32 j;
	INT32 FiltVal;
    UINT32 FLimit;
    UINT8 *LimitTable = &LimitVal_VP31[VAL_RANGE];

    FLimit = LoopFilterLimitValuesV2[QValue];

	for ( j=0; j<8; j++ )
	{            
        // Apply 4-tap filter with rounding...
		FiltVal =  ( Src[-2] - 
			        (Src[-1] * 3) +
			        (Src[ 0] * 3) - 
			         Src[ 1] + 4 ) >> 3;

        FiltVal = Bound ( FLimit, FiltVal );

		Src[-1] = LimitTable[(INT32)Src[-1] + FiltVal];
		Src[ 0] = LimitTable[(INT32)Src[ 0] - FiltVal];
		
        Src += Pitch;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilteringVert_8_C
 *
 *  INPUTS        : UINT32 QValue : Current quatizer level.
 *                  UINT8 *Src    : Pointer to data to be filtered.
 *                  INT32 Pitch   : Pitch of input data.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies vertical filter across horizontal edge inside
 *                  block with Q-dependent limits.
 *
 *  SPECIAL NOTES : 4-Tap filter used is (1, -3, 3, -1).
 *
 ****************************************************************************/                       
void FilteringVert_8_C ( UINT32 QValue, UINT8 *Src, INT32 Pitch )
{    
    INT32 j;
	INT32 FiltVal;
    UINT32 FLimit;
    UINT8 *LimitTable = &LimitVal_VP31[VAL_RANGE];

    FLimit = LoopFilterLimitValuesV2[QValue];

	for ( j=0; j<8; j++ )
	{            
        // Apply 4-tap filter with rounding...
  		FiltVal = (  (INT32)Src[-(2 * Pitch)] - 
        		    ((INT32)Src[-Pitch] * 3)  + 
		        	((INT32)Src[0] * 3 )      - 
			         (INT32)Src[Pitch] + 4 ) >> 3;

        FiltVal = Bound( FLimit, FiltVal);

		Src[-Pitch] = LimitTable[(INT32)Src[-Pitch] + FiltVal];
		Src[     0] = LimitTable[(INT32)Src[     0] - FiltVal];
	
        Src++;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilteringHoriz_12_C
 *
 *  INPUTS        : UINT32 QValue : Current quatizer level.
 *                  UINT8 *Src    : Pointer to data to be filtered.
 *                  INT32 Pitch   : Pitch of input data.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies vertical filter across horizontal edge inside
 *                  block with Q-dependent limits.
 *
 *  SPECIAL NOTES : 4-Tap filter used is (1, -3, 3, -1).
 *
 ****************************************************************************/                       
void FilteringHoriz_12_C ( UINT32 QValue, UINT8 *Src, INT32 Pitch )
{    
    INT32  j;
	INT32  FiltVal;
    UINT32 FLimit;
    UINT8 *LimitTable = &LimitVal_VP31[VAL_RANGE];

    FLimit = LoopFilterLimitValuesV2[QValue];

	for ( j=0; j<12; j++ )
	{            
        // Apply 4-tap filter with rounding...
		FiltVal =  ( Src[-2]      - 
			        (Src[-1] * 3) +
			        (Src[ 0] * 3) - 
			         Src[1]  + 4) >> 3;

        FiltVal = Bound ( FLimit, FiltVal );

		Src[-1] = LimitTable[(INT32)Src[-1] + FiltVal];
		Src[ 0] = LimitTable[(INT32)Src[ 0] - FiltVal];
		
        Src += Pitch;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilteringVert_12_C
 *
 *  INPUTS        : UINT32 QValue : Current quatizer level.
 *                  UINT8 *Src    : Pointer to data to be filtered.
 *                  INT32 Pitch   : Pitch of input data.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies vertical filter across horizontal edge inside
 *                  block with Q-dependent limits.
 *
 *  SPECIAL NOTES : 4-Tap filter used is (1, -3, 3, -1).
 *
 ****************************************************************************/                       
void FilteringVert_12_C ( UINT32 QValue, UINT8 *Src, INT32 Pitch )
{    
    INT32  j;
	INT32  FiltVal;
    UINT32 FLimit;
    UINT8 *LimitTable = &LimitVal_VP31[VAL_RANGE];

    FLimit = LoopFilterLimitValuesV2[QValue];

	for ( j=0; j<12; j++ )
	{            
 		FiltVal = ( (INT32)Src[- (2 * Pitch)] - 
        		   ((INT32)Src[- Pitch] * 3)  + 
		           ((INT32)Src[0] * 3)        - 
			        (INT32)Src[Pitch] + 4 ) >> 3;

        FiltVal = Bound ( FLimit, FiltVal );

		Src[-Pitch] = LimitTable[(INT32)Src[-Pitch] + FiltVal];
		Src[     0] = LimitTable[(INT32)Src[     0] - FiltVal];
	
        Src++;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : ApplyReconLoopFilter
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi     : Pointer to post-processor instance.
 *                  INT32  FrameQIndex         : Q index for the frame.
 *                  UINT8  *LastFrameRecon     : Pointer to last frame reconstruction buffer.
 *                  UINT8  *PostProcessBuffer  : Pointer to last post-processing buffer.
 *                  UINT8  *FragInfo           : Pointer to list of coded blocks.
 *                  UINT32 FragInfoElementSize : Size of each element.
 *                  UINT32 FragInfoCodedMask   : Mask to get at whether fragment is coded.
 *             
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a loop filter to the edge pixels of coded blocks.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void ApplyReconLoopFilter
(
    POSTPROC_INSTANCE *ppi,
    INT32		       FrameQIndex,
    UINT8		      *LastFrameRecon,
    UINT8		      *PostProcessBuffer,
    UINT8		      *FragInfo,
    UINT32             FragInfoElementSize,
    UINT32		       FragInfoCodedMask
)
{
	int j, m, n;
	UINT32 nextRow;
	UINT8 *rowStart;
    INT32 *BoundingValuePtr;

    INT32 i = 0;
    INT32 FLimit = 0; 
	int FromFragment = 0;
    INT32 LineLength = 0;
    INT32 LineFragments = 0;
	int FragsAcross = ppi->HFragments;	
	int FragsDown   = ppi->VFragments;

	// variables passed in per frame
	ppi->FrameQIndex 		 = FrameQIndex;
	ppi->LastFrameRecon      = LastFrameRecon;
	ppi->PostProcessBuffer 	 = PostProcessBuffer;
	ppi->FragInfo 			 = FragInfo;
	ppi->FragInfoElementSize = FragInfoElementSize;
	ppi->FragInfoCodedMask	 = FragInfoCodedMask;

    FLimit = LoopFilterLimitValuesV1[ppi->FrameQIndex];
    if ( FLimit == 0 )
        return;
        
    BoundingValuePtr = SetupBoundingValueArray ( ppi, FLimit );
 
	for ( j=0; j<3; j++ )
	{
		switch ( j )
		{
		case 0: // Y
			FromFragment  = 0;
			FragsAcross   = ppi->HFragments;
			FragsDown     = ppi->VFragments;
			LineLength    = ppi->YStride;
			LineFragments = ppi->HFragments;
			rowStart      = ppi->LastFrameRecon + ppi->ReconYDataOffset;
			break;
		case 1: // U
			FromFragment  = ppi->YPlaneFragments;
			FragsAcross   = ppi->HFragments >> 1;
			FragsDown     = ppi->VFragments >> 1;
			LineLength    = ppi->UVStride;
			LineFragments = ppi->HFragments / 2;
			rowStart      = ppi->LastFrameRecon + ppi->ReconUDataOffset;
			break;
		case 2:	// V
			FromFragment  = ppi->YPlaneFragments + ppi->UVPlaneFragments;
			FragsAcross   = ppi->HFragments >> 1;
			FragsDown     = ppi->VFragments >> 1;
			LineLength    = ppi->UVStride;
			LineFragments = ppi->HFragments / 2;
			rowStart      = ppi->LastFrameRecon + ppi->ReconVDataOffset;
			break;
		}
		
        nextRow = 8*LineLength;
		i = FromFragment;
		n = 0;

		/*************/
		/* First Row */
		/*************/

		/* First column */
		
		// only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
		if ( blockCoded ( i ) )
		{
			// Filter right hand border only if the block to the right is not coded
			if ( !blockCoded ( i + 1 ) )
				FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
			
			// Bottom done if next row set
			if ( !blockCoded (i + LineFragments) )
				FilterVert ( ppi, rowStart+8*n+nextRow, LineLength, BoundingValuePtr );
		}
		
		i++;
		
		/* Middle columns */
		for ( n=1; n<FragsAcross-1; n++, i++ )
		{
			if ( blockCoded( i ))
			{
				// Filter Left edge always
				FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
				
				// Filter right hand border only if the block to the right is not coded
				if ( !blockCoded( i + 1 ) )
					FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
				
				// Bottom done if next row set
				if( !blockCoded( i + LineFragments) )
					FilterVert(ppi, rowStart + 8*n + nextRow, LineLength, BoundingValuePtr);
			}
		}
		
		// Last Column
		if ( blockCoded( i ) )
		{
			// Filter Left edge always
			FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
			
			// Bottom done if next row set
			if( !blockCoded (i + LineFragments) )
				FilterVert(ppi, rowStart + 8*n + nextRow, LineLength, BoundingValuePtr);
		}
		
		i++;
		rowStart += nextRow;
		n = 0;

		/***************/
		/* Middle Rows */
		/***************/
		for ( m=1; m<FragsDown-1; m++ )
		{
			/* First column */
			n=0;
			
			// only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
			if( blockCoded( i ) )
			{
				// TopRow is always done
				FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );

				// Filter right hand border only if the block to the right is not coded
				if ( !blockCoded ( i + 1 ) )
					FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
				
				// Bottom done if next row set
				if ( !blockCoded (i + LineFragments) )
					FilterVert ( ppi, rowStart + 8*n + nextRow, LineLength, BoundingValuePtr );
			}

			i++;

			/* Middle columns */
			for ( n=1; n<FragsAcross-1; n++, i++ )
			{
				if ( blockCoded ( i ) )
				{
					// Filter Left edge always
					FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
					
					// TopRow is always done
					FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
					
					// Filter right hand border only if the block to the right is not coded
					if ( !blockCoded ( i + 1 ) )
						FilterHoriz ( ppi, rowStart + 8*n + 6 , LineLength, BoundingValuePtr );
					
					// Bottom done if next row set
					if ( !blockCoded (i + LineFragments) )
						FilterVert ( ppi, rowStart+8*n+nextRow, LineLength, BoundingValuePtr );
				}
			}

			/* Last Column */
			if ( blockCoded ( i ) )
			{
				// Filter Left edge always
				FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
				
				// TopRow is always done
				FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
				
				// Bottom done if next row set
				if ( !blockCoded (i + LineFragments) )
					FilterVert ( ppi, rowStart + 8*n + nextRow, LineLength, BoundingValuePtr );
            }

			i++;
			rowStart += nextRow;
		}
	}
		
	//***********/
	// Last Row */
	//***********/
	
	/* First Column */
	n = 0;
	
    // only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
	if ( blockCoded ( i ) )
	{
		// TopRow is always done
		FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
		
		// Filter right hand border only if the block to the right is not coded
		if ( !blockCoded ( i + 1 ) )
			FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
	}
	
	i++;
	
	/* middle columns */
	for ( n=1; n<FragsAcross-1; n++, i++ )
	{
		if ( blockCoded ( i ) )
		{
			// Filter Left edge always
			FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
			
			// TopRow is always done
			FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
			
			// Filter right hand border only if the block to the right is not coded
			if ( !blockCoded( i + 1 ) )
				FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
		}
	}
	
	/* Last Column */
	if ( blockCoded ( i ) )
	{
		// Filter Left edge always
		FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
		
		// TopRow is always done
		FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
	}
}


/****************************************************************************
 * 
 *  ROUTINE       : LoopFilter
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi     : Pointer to post-processor instance.
 *                  INT32  FrameQIndex         : Q index for the frame.
 *                  UINT8  *LastFrameRecon     : Pointer to last frame reconstruction buffer.
 *                  UINT8  *PostProcessBuffer  : Pointer to last post-processing buffer.
 *                  UINT8  *FragInfo           : Pointer to list of coded blocks.
 *                  UINT32 FragInfoElementSize : Size of each element.
 *                  UINT32 FragInfoCodedMask   : Mask to get at whether fragment is coded.
 *             
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a loop filter to the edge pixels of coded blocks.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void LoopFilter
(
    POSTPROC_INSTANCE *ppi,
    INT32		       FrameQIndex,
    UINT8		      *LastFrameRecon,
    UINT8		      *PostProcessBuffer,
    UINT8		      *FragInfo,
    UINT32             FragInfoElementSize,
    UINT32		       FragInfoCodedMask
)
{
	int j, m, n;
	UINT32 nextRow;
	UINT8 *rowStart;
    INT32 *BoundingValuePtr;

    INT32 i = 0;
    INT32 FLimit = 0; 
	int   FromFragment = 0;
    INT32 LineLength = 0;
    INT32 LineFragments = 0;
	int   FragsDown   = ppi->VFragments;
	int   FragsAcross = ppi->HFragments;	
	
    // variables passed in per frame
	ppi->FrameQIndex 			= FrameQIndex;
	ppi->LastFrameRecon 		= LastFrameRecon;
	ppi->PostProcessBuffer 		= PostProcessBuffer;
	ppi->FragInfo 				= FragInfo;
	ppi->FragInfoElementSize 	= FragInfoElementSize;
	ppi->FragInfoCodedMask		= FragInfoCodedMask;

    FLimit = LoopFilterLimitValuesV1[ppi->FrameQIndex];
    if ( FLimit == 0 )
        return;
	
    BoundingValuePtr = SetupBoundingValueArray ( ppi, FLimit );
	
	for ( j=0; j<3; j++ )
	{
		switch ( j )
		{
		case 0: // Y
			FromFragment  = 0;
			FragsAcross   = ppi->HFragments;
			FragsDown     = ppi->VFragments;
			LineLength    = ppi->YStride;
			LineFragments = ppi->HFragments;
			rowStart      = ppi->LastFrameRecon + ppi->ReconYDataOffset;
			break;
		case 1: // U
			FromFragment  = ppi->YPlaneFragments;
			FragsAcross   = ppi->HFragments >> 1;
			FragsDown     = ppi->VFragments >> 1;
			LineLength    = ppi->UVStride;
			LineFragments = ppi->HFragments / 2;
			rowStart      = ppi->LastFrameRecon + ppi->ReconUDataOffset;
			break;
		case 2:	// V
			FromFragment  = ppi->YPlaneFragments + ppi->UVPlaneFragments;
			FragsAcross   = ppi->HFragments >> 1;
			FragsDown     = ppi->VFragments >> 1;
			LineLength    = ppi->UVStride;
			LineFragments = ppi->HFragments / 2;
			rowStart      = ppi->LastFrameRecon + ppi->ReconVDataOffset;
			break;
		}
		
        nextRow = 8*LineLength;
		i = FromFragment;
		n = 0;
		
		//************/
		// First Row */
		//************/
		
		/* First Column */
		
		// only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
		if ( blockCoded ( i ) )
		{
			// Filter right hand border only if the block to the right is not coded
			if ( !blockCoded ( i + 1 ) )
				FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
			
			// Bottom done if next row set
			if( !blockCoded (i + LineFragments) )
				FilterVert ( ppi, rowStart+8*n+nextRow, LineLength, BoundingValuePtr );
		}
		
		i++;
		
		/* Middle columns */
		for ( n=1; n<FragsAcross-1; n++, i++ )
		{
			if ( blockCoded ( i ) )
			{
				// Filter Left edge always
				FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
				
				// Filter right hand border only if the block to the right is not coded
				if ( !blockCoded ( i + 1 ) )
					FilterHoriz(ppi, rowStart + 8*n +6 , LineLength, BoundingValuePtr);
				
				// Bottom done if next row set
				if( !blockCoded (i + LineFragments) )
					FilterVert ( ppi, rowStart+8*n+nextRow, LineLength, BoundingValuePtr );
			}
			
		}
		
		/* Last Column */
		if ( blockCoded ( i ) )
		{
			// Filter Left edge always
			FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
			
			// Bottom done if next row set
			if( !blockCoded (i + LineFragments) )
				FilterVert ( ppi, rowStart+8*n+nextRow, LineLength, BoundingValuePtr );
		}
		
		i++;
		rowStart += nextRow;
		n = 0;
		
		//**************/
		// Middle Rows */
		//**************/
		for ( m=1; m<FragsDown-1; m++ )
		{
			/* First column */
			n = 0;
			
			// only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
			if ( blockCoded ( i ) )
			{
				// TopRow is always done
				FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
				
				// Filter right hand border only if the block to the right is not coded
				if ( !blockCoded ( i + 1 ) )
					FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
				
				// Bottom done if next row set
				if( !blockCoded (i + LineFragments) )
					FilterVert(ppi, rowStart + 8*n + nextRow, LineLength, BoundingValuePtr);
			}
			
			i++;
			
			/* Middle columns */
			for ( n=1; n<FragsAcross-1; n++, i++ )
			{
				if ( blockCoded ( i ) )
				{
					// Filter Left edge always
					FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
					
					// TopRow is always done
					FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
					
					// Filter right hand border only if the block to the right is not coded
					if ( !blockCoded ( i + 1 ) )
						FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
					
					// Bottom done if next row set
					if( !blockCoded (i + LineFragments) )
						FilterVert ( ppi, rowStart+8*n+nextRow, LineLength, BoundingValuePtr );
				}
			}
			
			/* Last Column */
			if ( blockCoded ( i ) )
			{
				// Filter Left edge always
				FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
				
				// TopRow is always done
				FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
				
				// Bottom done if next row set
				if( !blockCoded (i + LineFragments) )
					FilterVert ( ppi, rowStart+8*n+nextRow, LineLength, BoundingValuePtr );
			}
			
			i++;
			rowStart += nextRow;
		}
		
		//***********/
		// Last Row */
		//***********/
		
		/* First column */
		n = 0;
		
        // only do 2 prediction if fragment coded and on non intra or if all fragments are intra 
		if ( blockCoded ( i ) )
		{
			// TopRow is always done
			FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
			
			// Filter right hand border only if the block to the right is not coded
			if ( !blockCoded ( i + 1 ) )
				FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
		}
		
		i++;
		
		/* Middle columns */
		for ( n=1; n<FragsAcross-1; n++, i++ )
		{
			if ( blockCoded ( i ) )
			{
				// Filter Left edge always
				FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
				
				// TopRow is always done
				FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
				
				// Filter right hand border only if the block to the right is not coded
				if ( !blockCoded ( i + 1 ) )
					FilterHoriz ( ppi, rowStart+8*n+6, LineLength, BoundingValuePtr );
			}
			
		}
		
		/* Last Column */
		if ( blockCoded ( i ) )
		{
			// Filter Left edge always
			FilterHoriz ( ppi, rowStart+8*n-2, LineLength, BoundingValuePtr );
			
			// TopRow is always done
			FilterVert ( ppi, rowStart+8*n, LineLength, BoundingValuePtr );
		}
		
		i++;

	}
}
