/****************************************************************************
 *        
 *   Module Title :     clamp.c
 *
 *   Description  :     Image pixel value clamping routines.
 *
 ***************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include "postp.h"

/****************************************************************************
 * 
 *  ROUTINE       : ClampLevels_C
 *
 *  INPUTS        : POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  INT32  BlackClamp,	   : Number of levels to clamp up from 0.
 *                  INT32  WhiteClamp,	   : Number of levels to clamp down from 255.
 *                  UINT8 *Src,			   : Pointer to input image to be clamped.
 *                  UINT8 *Dst			   : Pointer to clamped image.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Clamps the pixel values in the input image at each
 *                  end of the 8-bit range. 
 *
 *  SPECIAL NOTES : BlackClamp/WhiteClamp are the number.of levels to
 *                  clamp at either end of the range. In particular, it
 *                  should be noted that WhiteClamp is _not_ the level
 *                  to clamp to at the high end of the range.
 *
 ****************************************************************************/
void ClampLevels_C
( 
    POSTPROC_INSTANCE *pbi,
    INT32  BlackClamp,
    INT32  WhiteClamp,
    UINT8 *Src,		
    UINT8 *Dst		
)
{
	int i;
	int row,col;
	unsigned char clamped[256];

    int	width  = pbi->HFragments*8;
	int	height = pbi->VFragments*8;
	UINT8 *SrcPtr  = Src + pbi->ReconYDataOffset;
	UINT8 *DestPtr = Dst + pbi->ReconYDataOffset;
	UINT32 LineLength = pbi->YStride;

	// set up clamping table so we can avoid ifs while clamping
	for ( i=0; i<256; i++ )
	{
		clamped[i] = i;
		if ( i<BlackClamp )
			clamped[i] = BlackClamp;

		if ( i>(255-WhiteClamp) )
			clamped[i] = 255-WhiteClamp;
	}

    // clamping is for Y only!
	for ( row=0 ; row<height; row++ )
	{
		for ( col=0; col<width; col++ )
			SrcPtr[col] = clamped[DestPtr[col]];
		SrcPtr  += LineLength;
		DestPtr += LineLength;
    }
}
