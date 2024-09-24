/****************************************************************************
 *
 *   Module Title :     simpledeblock.c
 *
 *   Description  :     Simple deblocking filter.
 *
 ***************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "postp.h"

/****************************************************************************
*  Macros
****************************************************************************/        
#if ( defined(_MSC_VER) || defined(MAPCA) )
#define abs(x) ( (x>0) ? (x) : (-(x)) )
#endif

/****************************************************************************
*  Imports
****************************************************************************/              
extern UINT32 *DeblockLimitValuesV2;

/****************************************************************************
*  Module Statics
****************************************************************************/
static const UINT32 DeblockLimitValuesV1[Q_TABLE_SIZE] =
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
 
/****************************************************************************
 * 
 *  ROUTINE       : FilterHoriz_Simple2_C
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi  : Pointer to post-processing instance (NOT USED).
 *                  UINT8 *PixelPtr         : Pointer to four pixels that straddle the edge.
 *                  INT32 LineLength        : Stride of the image being filtered.
 *                  INT32 *BoundingValuePtr : Pointer to array of bounding values.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a loop filter to the vertical edge by applying
 *                  the filter horizontally to each of the 8-rows of the 
 *                  block edge.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void FilterHoriz_Simple2_C
(
    POSTPROC_INSTANCE *ppi, 
    UINT8 *PixelPtr, 
    INT32 LineLength, 
    INT32 *BoundingValuePtr
)
{
	INT32 j;
	INT32 x,y,z;
	INT32 FiltVal;
    UINT8 *LimitTable = &LimitVal_VP31[VAL_RANGE];
	
    (void) ppi;

	for ( j=0; j<8; j++ )
	{            
		y = PixelPtr[2]-PixelPtr[1];

		if ( !y ) continue;

		x = PixelPtr[1]-PixelPtr[0];
		z = PixelPtr[3]-PixelPtr[2];
		
		FiltVal = 2 * y + z - x;
        FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[1] = LimitTable[(INT32)PixelPtr[1] + FiltVal];
		PixelPtr[2] = LimitTable[(INT32)PixelPtr[2] - FiltVal];

		FiltVal >>= 1;
		FiltVal *= ((x|z)==0);

        PixelPtr[0] = LimitTable[(INT32)PixelPtr[0] + FiltVal];
		PixelPtr[3] = LimitTable[(INT32)PixelPtr[3] - FiltVal];
		
		PixelPtr += LineLength;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterVert_Simple2_C
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi  : Pointer to post-processing instance (NOT USED).
 *                  UINT8 *PixelPtr         : Pointer to four pixels that straddle the edge.
 *                  INT32 LineLength        : Stride of the image being filtered.
 *                  INT32 *BoundingValuePtr : Pointer to array of bounding values.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a loop filter to the horizontal edge by applying
 *                  the filter vertically to each of the 8-columns of the 
 *                  block edge.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void FilterVert_Simple2_C
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
	
    (void) ppi;

	for ( j=0; j<8; j++ )
	{            
        INT32 UseHighVariance;

        FiltVal = ( ((INT32)PixelPtr[0]*3) - ((INT32)PixelPtr[-LineLength]*3) );

        UseHighVariance = abs ( PixelPtr[-(2*LineLength)] - PixelPtr[-LineLength] ) > 1 ||
                          abs ( PixelPtr[0] - PixelPtr[LineLength]) > 1;

        if ( UseHighVariance )
		    FiltVal += ((INT32)PixelPtr[-(2*LineLength)]) - ((INT32)PixelPtr[LineLength]);

		FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[-LineLength] = LimitTable[(INT32)PixelPtr[-LineLength] + FiltVal];
		PixelPtr[          0] = LimitTable[(INT32)PixelPtr[          0] - FiltVal];
		
        if ( !UseHighVariance )
        {
            FiltVal >>=1;
            PixelPtr[-2*LineLength] = LimitTable[(INT32)PixelPtr[-2*LineLength] + FiltVal];
            PixelPtr[   LineLength] = LimitTable[(INT32)PixelPtr[   LineLength] - FiltVal];
        }

        PixelPtr++;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterHoriz_Simple_C
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi  : Pointer to post-processing instance (NOT USED).
 *                  UINT8 *PixelPtr         : Pointer to four pixels that straddle the edge.
 *                  INT32 LineLength        : Stride of the image being filtered.
 *                  INT32 *BoundingValuePtr : Pointer to array of bounding values.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a loop filter to the vertical edge by applying
 *                  the filter horizontally to each of the 8-rows of the 
 *                  block edge.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void FilterHoriz_Simple_C
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
	
    (void) ppi;

	for ( j=0; j<8; j++ )
	{            
        INT32 UseHighVariance;
		
		FiltVal = (PixelPtr[2]*3) - (PixelPtr[1]*3);

        UseHighVariance = abs(PixelPtr[0] - PixelPtr[1]) > 1 ||
                          abs(PixelPtr[2] - PixelPtr[3]) > 1;

        if ( UseHighVariance )
            FiltVal += PixelPtr[0] - PixelPtr[3];

        FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[1] = LimitTable[(INT32)PixelPtr[1] + FiltVal];
		PixelPtr[2] = LimitTable[(INT32)PixelPtr[2] - FiltVal];

        if ( !UseHighVariance )
        {
            FiltVal >>= 1;
            PixelPtr[0] = LimitTable[(INT32)PixelPtr[0] + FiltVal];
		    PixelPtr[3] = LimitTable[(INT32)PixelPtr[3] - FiltVal];
        }
		
		PixelPtr += LineLength;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterVert_Simple_C
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi  : Pointer to post-processing instance (NOT USED).
 *                  UINT8 *PixelPtr         : Pointer to four pixels that straddle the edge.
 *                  INT32 LineLength        : Stride of the image being filtered.
 *                  INT32 *BoundingValuePtr : Pointer to array of bounding values.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a loop filter to the horizontal edge by applying
 *                  the filter vertically to each of the 8-columns of the 
 *                  block edge.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void FilterVert_Simple_C
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
	
    (void) ppi;

	for ( j=0; j<8; j++ )
	{            
        INT32 UseHighVariance;

        FiltVal = ( ((INT32)PixelPtr[0]*3) - ((INT32)PixelPtr[-LineLength]*3) );

        UseHighVariance = abs(PixelPtr[-(2*LineLength)] - PixelPtr[-LineLength]) > 1 ||
                          abs(PixelPtr[0] - PixelPtr[LineLength]) > 1;

        if ( UseHighVariance )
		    FiltVal += ((INT32)PixelPtr[-(2*LineLength)]) - ((INT32)PixelPtr[LineLength]);
        
		FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[-LineLength] = LimitTable[(INT32)PixelPtr[-LineLength] + FiltVal];
		PixelPtr[          0] = LimitTable[(INT32)PixelPtr[          0] - FiltVal];
		
        if ( !UseHighVariance )
        {
            FiltVal >>=1;
            PixelPtr[-2*LineLength] = LimitTable[(INT32)PixelPtr[-2*LineLength] + FiltVal];
            PixelPtr[   LineLength] = LimitTable[(INT32)PixelPtr[   LineLength] - FiltVal];
        }

        PixelPtr++;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : SimpleDeblockFrame
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi  : Pointer to post-processing instance.
 *                  UINT8 *SrcBuffer        : Pointer to image to be deblocked.
 *                  UINT8 *DestBuffer       : Pointer to image to hold deblocked image.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Simple deblocker.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void SimpleDeblockFrame ( POSTPROC_INSTANCE *ppi, UINT8 *SrcBuffer, UINT8 *DestBuffer )
{
	INT32 j, m, n;
	INT32 RowStart;
	INT32 NextRow;
    INT32 FLimit; 
    INT32 QIndex;
    INT32 *BoundingValuePtr;
    INT32 LineLength  = 0;
	INT32 FragsAcross = ppi->HFragments;	
	INT32 FragsDown   = ppi->VFragments;

	QIndex = ppi->FrameQIndex;

    // Encoder version specific clause
	if ( ppi->Vp3VersionNo >= 2 )
		FLimit = DeblockLimitValuesV2[QIndex];
	else
		FLimit = DeblockLimitValuesV1[QIndex];
     
    BoundingValuePtr = SetupDeblockValueArray ( ppi, FLimit );

	for ( j=0; j<3; j++ )
	{
		switch ( j )
		{
		case 0: // Y
			FragsAcross = ppi->HFragments;
			FragsDown   = ppi->VFragments;
			LineLength  = ppi->YStride;
			RowStart    = ppi->ReconYDataOffset;
			break;
		case 1: // U
			FragsAcross = ppi->HFragments >> 1;
			FragsDown   = ppi->VFragments >> 1;
			LineLength  = ppi->UVStride;
			RowStart    = ppi->ReconUDataOffset;
			break;
		case 2:	// V
			FragsAcross = ppi->HFragments >> 1;
			FragsDown   = ppi->VFragments >> 1;
			LineLength  = ppi->UVStride;
			RowStart    = ppi->ReconVDataOffset;
			break;
		}

		NextRow = LineLength * 8;

		/*************/
		/* First Row */
		/*************/

		memcpy ( &DestBuffer[RowStart], &SrcBuffer[RowStart], 8*LineLength );

        /* First Column -- Skip */

        /* Remaining Columns */
		for ( n=1; n<FragsAcross; n++ )  // Filter Left edge always
			FilterHoriz_Simple ( ppi, &DestBuffer[RowStart+n*8-2], LineLength, BoundingValuePtr );
		
		RowStart += NextRow;

		//**************/
		// Middle Rows */
		//**************/
		for ( m=1; m<FragsDown; m++ )
		{
			n = 0;
		
		    memcpy ( &DestBuffer[RowStart], &SrcBuffer[RowStart], 8*LineLength );

			/* First column */
			FilterVert_Simple ( ppi, &DestBuffer[RowStart+n*8], LineLength, BoundingValuePtr );
			 
			/* Middle columns */
			for ( n=1; n<FragsAcross; n++ )
			{
				// Filter Left edge always
				FilterHoriz_Simple ( ppi, &DestBuffer[RowStart+n*8-2], LineLength, BoundingValuePtr );
				
				// TopRow is always done
				FilterVert_Simple ( ppi, &DestBuffer[RowStart+n*8], LineLength, BoundingValuePtr );
			}
			
			RowStart += NextRow;
		}
	}	
}
