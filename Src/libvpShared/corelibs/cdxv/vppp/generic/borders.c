/**************************************************************************** 
*
*   Module Title :     borders.c
*
*   Description  :     
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "postp.h"

#ifdef MAPCA
#include "eti/mm.h"
#include "eti_loopdir.h"
#endif

#ifdef MAPCA
void CopyYLeftRightBorder 
(
    UINT8 *restrict SrcPtr1,
    UINT8 *restrict SrcPtr2,
    UINT8 *restrict DestPtr1,
    UINT8 *restrict DestPtr2,
    UINT32 PlaneHeight,
    UINT32 PlaneStride
)
{
	n64 *restrict DstPtr64_1 = (n64* restrict)DestPtr1;
    n64 *restrict DstPtr64_2 = (n64* restrict)DestPtr2;
    n32  PlaneStride64 = (PlaneStride>>3);
    n32  Left, Right;
    n64  Left64, Right64;
	int i;
	
    loop_directives ( ELD_SWP_IVDEP );
    for ( i=0; i<PlaneHeight; i++ )
	{
    	Left  = SrcPtr1[0];
        Right = SrcPtr2[0];

        Left64  = hmpv_bcopyrev_64_32 ( Left, 0, 0 );
        Right64 = hmpv_bcopyrev_64_32 ( Right, 0, 0 );

    	DstPtr64_1[0] = Left64;
        DstPtr64_2[0] = Right64;

    	DstPtr64_1[1] = Left64;
        DstPtr64_2[1] = Right64;

        DstPtr64_1[2] = Left64;
        DstPtr64_2[2] = Right64;

        DstPtr64_1[3] = Left64;
        DstPtr64_2[3] = Right64;

        SrcPtr1 += PlaneStride;
        SrcPtr2 += PlaneStride;
        DstPtr64_1 += PlaneStride64;
        DstPtr64_2 += PlaneStride64;		
	}
}

void CopyUVLeftRightBorder
(
        UINT8 *restrict SrcPtr1,
        UINT8 *restrict SrcPtr2,
        UINT8 *restrict DestPtr1,
        UINT8 *restrict DestPtr2,
        UINT32 PlaneHeight,
        UINT32 PlaneStride
)
{
	n64 *restrict DstPtr64_1 = (n64* restrict)DestPtr1;
    n64 *restrict DstPtr64_2 = (n64* restrict)DestPtr2;
    n32  PlaneStride64 = (PlaneStride>>3);
    n32  Left, Right;
    n64  Left64, Right64;
	int  i;
    
    loop_directives ( ELD_SWP_IVDEP );
	for ( i=0; i<PlaneHeight; i++ )
	{
    	Left  = SrcPtr1[0];
        Right = SrcPtr2[0];

        Left64  = hmpv_bcopyrev_64_32 ( Left, 0, 0 );
        Right64 = hmpv_bcopyrev_64_32 ( Right, 0, 0 );

    	DstPtr64_1[0] = Left64;
        DstPtr64_2[0] = Right64;

    	DstPtr64_1[1] = Left64;
        DstPtr64_2[1] = Right64;

        SrcPtr1 += PlaneStride;
        SrcPtr2 += PlaneStride;
        DstPtr64_1 += PlaneStride64;
        DstPtr64_2 += PlaneStride64;		
	}
}
#endif

/****************************************************************************
 * 
 *  ROUTINE       : UpdateUMVBorder
 *
 *  INPUTS        : POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  UINT8 *DestReconPtr    : Pointer to reconstructed image.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Copies pixel values in first/last rows/columns of the
 *                  image into the UMV border in the specified reconstructed
 *                  image.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void UpdateUMVBorder ( POSTPROC_INSTANCE *pbi, UINT8 *DestReconPtr )
{
	
	INT32 i;
	INT32 PlaneHeight;
	UINT8 *SrcPtr1, *SrcPtr2;
	UINT8 *DestPtr1, *DestPtr2;

    UINT32 Border = pbi->MVBorder;
	INT32 PlaneStride = pbi->YStride;

    /***********/
    /* Y Plane */
    /***********/
	PlaneStride = pbi->YStride;
	PlaneHeight = pbi->VFragments * 8;

    // copy the left and right most columns out 
	SrcPtr1 = DestReconPtr + pbi->ReconYDataOffset;
	SrcPtr2 = SrcPtr1 + 8 * pbi->HFragments - 1;
	DestPtr1= SrcPtr1 - Border;
	DestPtr2= SrcPtr2 + 1;

#ifdef MAPCA
    CopyYLeftRightBorder ( SrcPtr1, SrcPtr2, DestPtr1,DestPtr2, PlaneHeight, PlaneStride );
#else
    for ( i=0; i<PlaneHeight; i++ )
    {
        memset ( DestPtr1, SrcPtr1[0], Border );
        memset ( DestPtr2, SrcPtr2[0], Border );
        SrcPtr1  += PlaneStride;
        SrcPtr2  += PlaneStride;
        DestPtr1 += PlaneStride;
        DestPtr2 += PlaneStride;
    }
#endif

    // Now copy the top and bottom source lines into each line of the respective borders
	SrcPtr1 = DestReconPtr + Border * PlaneStride;
	SrcPtr2 = SrcPtr1 + (pbi->VFragments * 8 * PlaneStride)- PlaneStride;
	DestPtr1= DestReconPtr;
	DestPtr2= SrcPtr2 + PlaneStride;
    for ( i=0; i<(INT32)Border; i++ )
    {
        memcpy ( DestPtr1, SrcPtr1, PlaneStride );
        memcpy ( DestPtr2, SrcPtr2, PlaneStride );
        DestPtr1 += PlaneStride;
        DestPtr2 += PlaneStride;
    }

	PlaneStride = pbi->UVStride;
	PlaneHeight = pbi->VFragments * 4;

    /***********/
    /* U Plane */
    /***********/

    // copy the left and right most columns out 
	SrcPtr1 = DestReconPtr + pbi->ReconUDataOffset;
	SrcPtr2 = SrcPtr1 + 4 * pbi->HFragments - 1;
	DestPtr1= SrcPtr1 - Border/2;
	DestPtr2= SrcPtr2 + 1;

#ifdef MAPCA
    CopyUVLeftRightBorder ( SrcPtr1, SrcPtr2, DestPtr1,DestPtr2, PlaneHeight, PlaneStride );
#else
    for ( i=0; i<PlaneHeight; i++ )
    {
        memset ( DestPtr1, SrcPtr1[0], Border/2 );
        memset ( DestPtr2, SrcPtr2[0], Border/2 );
        SrcPtr1  += PlaneStride;
        SrcPtr2  += PlaneStride;
        DestPtr1 += PlaneStride;
        DestPtr2 += PlaneStride;
    }
#endif

    // Now copy the top and bottom source lines into each line of the respective borders
	SrcPtr1 = DestReconPtr + pbi->ReconUDataOffset - Border/2;
	SrcPtr2 = SrcPtr1 + (pbi->VFragments * 4 * PlaneStride)- PlaneStride;
	DestPtr1= SrcPtr1 - Border/2*PlaneStride;
	DestPtr2= SrcPtr2 + PlaneStride;
    for ( i=0; i<(INT32)(Border/2); i++ )
    {
        memcpy ( DestPtr1, SrcPtr1, PlaneStride );
        memcpy ( DestPtr2, SrcPtr2, PlaneStride );
        DestPtr1 += PlaneStride;
        DestPtr2 += PlaneStride;
    }

    /***********/
    /* V Plane */
    /***********/
    
    // copy the left and right most columns out 
	SrcPtr1 = DestReconPtr + pbi->ReconVDataOffset;
	SrcPtr2 = SrcPtr1 + 4 * pbi->HFragments - 1;
	DestPtr1= SrcPtr1 - Border/2;
	DestPtr2= SrcPtr2 + 1;

#ifdef MAPCA
    CopyUVLeftRightBorder ( SrcPtr1, SrcPtr2, DestPtr1,DestPtr2, PlaneHeight, PlaneStride );
#else
    for ( i=0; i<PlaneHeight; i++ )
    {
        memset ( DestPtr1, SrcPtr1[0], Border/2 );
        memset ( DestPtr2, SrcPtr2[0], Border/2 );
        SrcPtr1  += PlaneStride;
        SrcPtr2  += PlaneStride;
        DestPtr1 += PlaneStride;
        DestPtr2 += PlaneStride;
    }
#endif

    // Now copy the top and bottom source lines into each line of the respective borders
	SrcPtr1 = DestReconPtr + pbi->ReconVDataOffset - Border/2;
	SrcPtr2 = SrcPtr1 + (pbi->VFragments * 4 * PlaneStride)- PlaneStride;
	DestPtr1= SrcPtr1 - Border/2*PlaneStride;
	DestPtr2= SrcPtr2 + PlaneStride;
    for ( i=0; i<(INT32)(Border/2); i++ )
    {
        memcpy ( DestPtr1, SrcPtr1, PlaneStride );
        memcpy ( DestPtr2, SrcPtr2, PlaneStride );
        DestPtr1 += PlaneStride;
        DestPtr2 += PlaneStride;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : CopyFrame
 *
 *  INPUTS        : POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  YUV_BUFFER_CONFIG *b   : Pointer to source image.
 *                  UINT8 *DestReconPtr    : Pointer to destination image.
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Copies the source image into the destination image and
 *                  updates the destination's UMV borders.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void CopyFrame ( POSTPROC_INSTANCE *pbi, YUV_BUFFER_CONFIG *b, UINT8 *DestReconPtr )
{
	int row;
	unsigned char *source, *dest;

	source = (unsigned char *) b->YBuffer;
	dest = DestReconPtr + pbi->ReconYDataOffset;
	for ( row=0; row<b->YHeight; row++ )
	{
		memcpy ( dest, source, b->YWidth );
		source += b->YStride;
		dest   += pbi->YStride;
	}

    source = (unsigned char *) b->UBuffer;
	dest = DestReconPtr + pbi->ReconUDataOffset;
	for ( row=0; row<b->UVHeight; row++ )
	{
		memcpy ( dest, source, b->UVWidth );
		source += b->UVStride;
		dest   += pbi->UVStride;
	}

    source = (unsigned char *)  b->VBuffer;
	dest = DestReconPtr + pbi->ReconVDataOffset;
	for ( row=0; row<b->UVHeight; row++ )
	{
		memcpy ( dest, source, b->UVWidth );
		source += b->UVStride;
		dest   += pbi->UVStride;
	}

	UpdateUMVBorder ( pbi, DestReconPtr );
}
