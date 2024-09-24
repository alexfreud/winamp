/****************************************************************************
*        
*   Module Title :     scale.c
*
*   Description  :     Image scaling functions.
*
***************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include "postp.h"

/****************************************************************************
*  Imports
****************************************************************************/
extern void UpdateUMVBorder ( POSTPROC_INSTANCE *ppi, UINT8 * DestReconPtr );

/****************************************************************************
* 
*  ROUTINE       : HorizontalLine_Copy
*
*  INPUTS        : const unsigned char *source : Pointer to source data.
*                  unsigned int sourceWidth    : Stride of source.
*                  unsigned char *dest         : Pointer to destination data.
*                  unsigned int destWidth      : Stride of destination (NOT USED).
*                   
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Copies horizontal line of pixels from source to 
*                  destination unscaled.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void HorizontalLine_Copy 
(
 const unsigned char *source,
 unsigned int sourceWidth,
 unsigned char *dest,
 unsigned int destWidth
 )
{
	(void) destWidth;
	memcpy ( dest, source, sourceWidth );
}

/****************************************************************************
* 
*  ROUTINE       : NullScale
*
*  INPUTS        : unsigned char *dest    : Pointer to destination data (NOT USED).
*                  unsigned int destPitch : Stride of destination data (NOT USED).
*                  unsigned int destWidth : Width of destination data (NOT USED).
*                  
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Null scaling function -- does nothing.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void NullScale ( unsigned char *dest, unsigned int destPitch, unsigned int destWidth )
{
	(void) destWidth;
	(void) destPitch;
	(void) dest;
	return;
}

/****************************************************************************
* 
*  ROUTINE       : HorizontalLine_4_5_Scale_C
*
*  INPUTS        : const unsigned char *source : Pointer to source data.
*                  unsigned int sourceWidth    : Stride of source.
*                  unsigned char *dest         : Pointer to destination data.
*                  unsigned int destWidth      : Stride of destination (NOT USED).
*                   
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Copies horizontal line of pixels from source to 
*                  destination scaling up by 4 to 5.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void HorizontalLine_4_5_Scale_C 
(
 const unsigned char *source,
 unsigned int sourceWidth,
 unsigned char *dest,
 unsigned int destWidth
 )
{
	unsigned i;
	unsigned int a, b, c;
	unsigned char *des = dest;
	const unsigned char *src = source;

	(void) destWidth;

	for ( i=0; i<sourceWidth-4; i+=4 )
	{
		a = src[0];
		b = src[1];
		des [0] = (UINT8) a;
		des [1] = (UINT8) (( a * 51 + 205 * b + 128) >> 8);
		c = src[2] * 154;
		a = src[3];
		des [2] = (UINT8) (( b * 102 + c + 128) >> 8);
		des [3] = (UINT8) (( c + 102 * a + 128) >> 8);
		b = src[4];
		des [4] = (UINT8) (( a * 205 + 51 * b + 128) >> 8);

		src += 4;
		des += 5;
	}

	a = src[0];
	b = src[1];
	des [0] = (UINT8) (a);
	des [1] = (UINT8) (( a * 51 + 205 * b + 128) >> 8);
	c = src[2] * 154;
	a = src[3];
	des [2] = (UINT8) (( b * 102 + c + 128) >> 8);
	des [3] = (UINT8) (( c + 102 * a + 128) >> 8);
	des [4] = (UINT8) (a);

}        

/****************************************************************************
* 
*  ROUTINE       : VerticalBand_4_5_Scale_C
*
*  INPUTS        : unsigned char *dest    : Pointer to destination data.
*                  unsigned int destPitch : Stride of destination data.
*                  unsigned int destWidth : Width of destination data.
*                  
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Scales vertical band of pixels by scale 4 to 5. The
*                  height of the band scaled is 4-pixels.
*
*  SPECIAL NOTES : The routine uses the first line of the band below 
*                  the current band.
*
****************************************************************************/
void VerticalBand_4_5_Scale_C ( unsigned char *dest, unsigned int destPitch, unsigned int destWidth )
{
	unsigned int i;
	unsigned int a, b, c, d;
	unsigned char *des = dest;

	for ( i=0; i<destWidth; i++ )
	{
		a = des [0];
		b = des [destPitch];

		des[destPitch] = (UINT8) (( a * 51 + 205 * b + 128)>>8);

		c = des[destPitch*2]*154;
		d = des[destPitch*3];

		des [destPitch*2] = (UINT8) (( b * 102 + c + 128) >> 8);
		des [destPitch*3] = (UINT8) (( c + 102 * d + 128) >> 8);

		// First line in next band
		a = des [destPitch * 5];
		des [destPitch * 4] = (UINT8) (( d * 205 + 51 * a +128)>>8);

		des ++;
	}
}

/****************************************************************************
* 
*  ROUTINE       : LastVerticalBand_4_5_Scale_C
*
*  INPUTS        : unsigned char *dest    : Pointer to destination data.
*                  unsigned int destPitch : Stride of destination data.
*                  unsigned int destWidth : Width of destination data.
*                  
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Scales last vertical band of pixels by scale 4 to 5. The
*                  height of the band scaled is 4-pixels.
*
*  SPECIAL NOTES : The routine does not have available the first line of
*                  the band below the current band, since this is the
*                  last band.
*
****************************************************************************/
void LastVerticalBand_4_5_Scale_C ( unsigned char *dest, unsigned int destPitch, unsigned int destWidth )
{
	unsigned int i;
	unsigned int a, b, c, d;
	unsigned char *des = dest;

	for ( i=0; i<destWidth; ++i )
	{
		a = des[0];
		b = des[destPitch];

		des[destPitch] = (UINT8) ((a * 51 + 205 * b + 128)>>8);

		c = des[destPitch*2]*154;
		d = des[destPitch*3];

		des [destPitch*2] = (UINT8) (( b * 102 + c + 128) >> 8);
		des [destPitch*3] = (UINT8) (( c + 102 * d + 128) >> 8);

		// No other line for interplation of this line, so ..
		des[destPitch*4] = (UINT8) d;

		des++;
	}
}

/****************************************************************************
* 
*  ROUTINE       : Scale _4_5_2D
*
*  INPUTS        : POSTPROC_INSTANCE *ppi      : Pointer to post-processor instance (NOT USED).
*                  const unsigned char *source : Pointer to source image.
*                  unsigned int sourcePitch    : Stride of source image.
*                  unsigned int sourceWidth    : Width of source image.
*                  unsigned int sourceHeight   : Height of source image (NOT USED).
*                  unsigned char *dest         : Pointer to destination image.
*                  unsigned int destPitch      : Stride of destination image.
*                  unsigned int destWidth      : Width of destination image.
*                  unsigned int destHeight     : Height of destination image.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Two-dimensional 4 to 5 scaling up of an image.
*
*  SPECIAL NOTES : None.
*
****************************************************************************/
void Scale_4_5_2D
(
 POSTPROC_INSTANCE *ppi,
 const unsigned char *source,
 unsigned int sourcePitch,
 unsigned int sourceWidth,
 unsigned int sourceHeight,
 unsigned char *dest,
 unsigned int destPitch,
 unsigned int destWidth,
 unsigned int destHeight
 )
{
	unsigned i, k;
	const unsigned int srcBandHeight  = 4;
	const unsigned int destBandHeight = 5;

	(void) sourceHeight;
	(void) ppi;

	HorizontalLine_4_5_Scale ( source, sourceWidth, dest, destWidth );

	// Except last band
	for ( k=0; k<destHeight/destBandHeight-1; k++ )
	{
		// scale one band horizontally 
		for ( i=1; i<srcBandHeight; i++ )
		{
			HorizontalLine_4_5_Scale ( source+i*sourcePitch,
				sourceWidth,
				dest+i*destPitch,
				destWidth);
		}

		// first line of next band
		HorizontalLine_4_5_Scale ( source+srcBandHeight*sourcePitch,
			sourceWidth,
			dest+destBandHeight*destPitch,
			destWidth );

		// Vertical scaling is in place       
		VerticalBand_4_5_Scale ( dest, destPitch, destWidth );

		// move to the next band
		source += srcBandHeight  * sourcePitch;
		dest   += destBandHeight * destPitch;
	}

	// scale one band horizontally 
	for ( i=1; i<srcBandHeight; i++ )
	{
		HorizontalLine_4_5_Scale ( source+i*sourcePitch,
			sourceWidth,
			dest+i*destPitch,
			destWidth );
	}

	// Vertical scaling is in place       
	LastVerticalBand_4_5_Scale ( dest, destPitch, destWidth );
}


/****************************************************************************
* 
*  ROUTINE       : HorizontalLine_3_5_Scale_C
*
*  INPUTS        : const unsigned char *source : Pointer to source data.
*                  unsigned int sourceWidth    : Stride of source.
*                  unsigned char *dest         : Pointer to destination data.
*                  unsigned int destWidth      : Stride of destination (NOT USED).
*                   
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Copies horizontal line of pixels from source to 
*                  destination scaling up by 3 to 5.
*
*  SPECIAL NOTES : None. 
*
*
****************************************************************************/
void HorizontalLine_3_5_Scale_C 
(
 const unsigned char *source,
 unsigned int sourceWidth,
 unsigned char *dest,
 unsigned int destWidth
 )
{
	unsigned int i;
	unsigned int a, b, c;
	unsigned char *des = dest;
	const unsigned char *src = source;

	(void) destWidth;

	for ( i=0; i<sourceWidth-3; i+=3 )
	{
		a = src[0];
		b = src[1];
		des [0] = (UINT8) (a);
		des [1] = (UINT8) (( a * 102 + 154 * b + 128 ) >> 8);

		c = src[2] ;
		des [2] = (UINT8) (( b * 205 + c * 51 + 128 ) >> 8);
		des [3] = (UINT8) (( b * 51 + c * 205 + 128 ) >> 8);

		a = src[3];
		des [4] = (UINT8) (( c * 154 + a * 102 + 128 ) >> 8);

		src += 3;
		des += 5;
	}

	a = src[0];
	b = src[1];
	des [0] = (UINT8) (a);

	des [1] = (UINT8) (( a * 102 + 154 * b + 128 ) >> 8);
	c = src[2] ;
	des [2] = (UINT8) (( b * 205 + c * 51 + 128 ) >> 8);
	des [3] = (UINT8) (( b * 51 + c * 205 + 128 ) >> 8);

	des [4] = (UINT8) (c);
}        

/****************************************************************************
* 
*  ROUTINE       : VerticalBand_3_5_Scale_C
*
*  INPUTS        : unsigned char *dest    : Pointer to destination data.
*                  unsigned int destPitch : Stride of destination data.
*                  unsigned int destWidth : Width of destination data.
*                  
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Scales vertical band of pixels by scale 3 to 5. The
*                  height of the band scaled is 3-pixels.
*
*  SPECIAL NOTES : The routine uses the first line of the band below 
*                  the current band.
*
****************************************************************************/
void VerticalBand_3_5_Scale_C ( unsigned char *dest, unsigned int destPitch, unsigned int destWidth )
{
	unsigned int i;
	unsigned int a, b, c;
	unsigned char *des = dest;

	for ( i=0; i<destWidth; i++ )
	{
		a = des [0];
		b = des [destPitch];      
		des [destPitch] =  (UINT8) (( a * 102 + 154 * b + 128 ) >> 8);

		c = des[destPitch*2];
		des [destPitch*2] = (UINT8) (( b * 205 + c * 51 + 128 ) >> 8);
		des [destPitch*3] = (UINT8) (( b * 51 + c * 205 + 128 ) >> 8);

		// First line in next band...
		a = des [destPitch * 5];
		des [destPitch * 4] = (UINT8) (( c * 154 + a * 102 + 128 ) >> 8);

		des++;
	}
}

/****************************************************************************
* 
*  ROUTINE       : LastVerticalBand_3_5_Scale_C
*
*  INPUTS        : unsigned char *dest    : Pointer to destination data.
*                  unsigned int destPitch : Stride of destination data.
*                  unsigned int destWidth : Width of destination data.
*                  
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Scales last vertical band of pixels by scale 3 to 5. The
*                  height of the band scaled is 3-pixels.
*
*  SPECIAL NOTES : The routine does not have available the first line of
*                  the band below the current band, since this is the
*                  last band.
*
****************************************************************************/
void LastVerticalBand_3_5_Scale_C ( unsigned char *dest, unsigned int destPitch, unsigned int destWidth )
{
	unsigned int i;
	unsigned int a, b, c;
	unsigned char *des = dest;

	for ( i=0; i<destWidth; ++i )
	{
		a = des [0];
		b = des [destPitch];

		des [ destPitch ] =  (UINT8) (( a * 102 + 154 * b + 128 ) >> 8);

		c = des[destPitch*2];
		des [destPitch*2] = (UINT8) (( b * 205 + c * 51 + 128 ) >> 8);
		des [destPitch*3] = (UINT8) (( b * 51 + c * 205 + 128 ) >> 8);

		// No other line for interplation of this line, so ..
		des [ destPitch * 4 ] = (UINT8) (c) ;

		des++;
	}
}

/****************************************************************************
* 
*  ROUTINE       : Scale _3_5_2D
*
*  INPUTS        : POSTPROC_INSTANCE *ppi      : Pointer to post-processor instance (NOT USED).
*                  const unsigned char *source : Pointer to source image.
*                  unsigned int sourcePitch    : Stride of source image.
*                  unsigned int sourceWidth    : Width of source image.
*                  unsigned int sourceHeight   : Height of source image (NOT USED).
*                  unsigned char *dest         : Pointer to destination image.
*                  unsigned int destPitch      : Stride of destination image.
*                  unsigned int destWidth      : Width of destination image.
*                  unsigned int destHeight     : Height of destination image.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Two-dimensional 3 to 5 scaling up of an image.
*
*  SPECIAL NOTES : None.
* 
****************************************************************************/
void Scale_3_5_2D
( 
 POSTPROC_INSTANCE *ppi,
 const unsigned char *source,
 unsigned int sourcePitch,
 unsigned int sourceWidth,
 unsigned int sourceHeight,
 unsigned char *dest,
 unsigned int destPitch,
 unsigned int destWidth,
 unsigned int destHeight
 )
{
	// define the constants for a 3->5 scale up
	const unsigned int srcBandHeight = 3;
	const unsigned int destBandHeight = 5;
	unsigned int i, k;

	(void) ppi;
	(void) sourceHeight;

	HorizontalLine_3_5_Scale ( source, sourceWidth, dest, destWidth );

	// Except last band
	for ( k=0; k<destHeight/destBandHeight-1; k++ )
	{
		// scale one band horizontally 
		for ( i=1; i<srcBandHeight; i++ )
		{
			HorizontalLine_3_5_Scale ( source+i*sourcePitch,
				sourceWidth,
				dest+i*destPitch,
				destWidth );
		}

		// First line of next band
		HorizontalLine_3_5_Scale ( source+srcBandHeight*sourcePitch,
			sourceWidth,
			dest+destBandHeight*destPitch,
			destWidth );

		// Vertical scaling is in place       
		VerticalBand_3_5_Scale ( dest, destPitch, destWidth );

		// move to the next band
		source += srcBandHeight  * sourcePitch;
		dest   += destBandHeight * destPitch;
	}

	// scale one band horizontally 
	for ( i=1; i<srcBandHeight; i++ )
	{
		HorizontalLine_3_5_Scale ( source+i*sourcePitch,
			sourceWidth,
			dest+i*destPitch,
			destWidth );
	}

	// Vertical scaling is in place       
	LastVerticalBand_3_5_Scale ( dest, destPitch, destWidth );
}

/****************************************************************************
* 
*  ROUTINE       : HorizontalLine_1_2_Scale_C
*
*  INPUTS        : const unsigned char *source : Pointer to source data.
*                  unsigned int sourceWidth    : Stride of source.
*                  unsigned char *dest         : Pointer to destination data.
*                  unsigned int destWidth      : Stride of destination (NOT USED).
*                   
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Copies horizontal line of pixels from source to 
*                  destination scaling up by 1 to 2.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void HorizontalLine_1_2_Scale_C 
(
 const unsigned char *source,
 unsigned int sourceWidth,
 unsigned char *dest,
 unsigned int destWidth
 )
{
	unsigned int i;
	unsigned int a, b;
	unsigned char *des = dest;
	const unsigned char *src = source;

	(void) destWidth;

	for ( i=0; i<sourceWidth-1; i+=1 )
	{
		a = src[0];
		b = src[1];
		des [0] = (UINT8) (a);
		des [1] = (UINT8) (( a + b + 1 ) >> 1);
		src += 1;
		des += 2;
	}

	a = src[0];
	des [0] = (UINT8) (a);
	des [1] = (UINT8) (a);
}        

/****************************************************************************
* 
*  ROUTINE       : VerticalBand_1_2_Scale_C
*
*  INPUTS        : unsigned char *dest    : Pointer to destination data.
*                  unsigned int destPitch : Stride of destination data.
*                  unsigned int destWidth : Width of destination data.
*                  
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Scales vertical band of pixels by scale 1 to 2. The
*                  height of the band scaled is 1-pixel.
*
*  SPECIAL NOTES : The routine uses the first line of the band below 
*                  the current band.
*
****************************************************************************/
void VerticalBand_1_2_Scale_C ( unsigned char *dest, unsigned int destPitch, unsigned int destWidth )
{
	unsigned int i;
	unsigned int a, b;
	unsigned char *des = dest;

	for ( i=0; i<destWidth; i++ )
	{
		a = des [0];
		b = des [destPitch * 2];

		des[destPitch] = (UINT8) ((a + b + 1 )>>1);

		des++;
	}
}

/****************************************************************************
* 
*  ROUTINE       : LastVerticalBand_1_2_Scale_C
*
*  INPUTS        : unsigned char *dest    : Pointer to destination data.
*                  unsigned int destPitch : Stride of destination data.
*                  unsigned int destWidth : Width of destination data.
*                  
*  OUTPUTS       : None.
*
*  RETURNS       : void
*
*  FUNCTION      : Scales last vertical band of pixels by scale 1 to 2. The
*                  height of the band scaled is 1-pixel.
*
*  SPECIAL NOTES : The routine does not have available the first line of
*                  the band below the current band, since this is the
*                  last band.
*
****************************************************************************/
void LastVerticalBand_1_2_Scale_C ( unsigned char *dest, unsigned int destPitch, unsigned int destWidth )
{
	unsigned int i;
	unsigned char *des = dest;

	for ( i=0; i<destWidth; ++i )
	{
		des[destPitch] = des[0];
		des++;
	}
}

/****************************************************************************
* 
*  ROUTINE       : Scale1D_c
*
*  INPUTS        : const unsigned char *source : Pointer to data to be scaled.
*                  int sourceStep              : Number of pixels to step on in source.
*                  unsigned int sourceScale    : Scale for source.
*                  unsigned int sourceLength   : Length of source (UNUSED).
*                  unsigned char *dest         : Pointer to output data array.
*                  int destStep                : Number of pixels to step on in destination.
*                  unsigned int destScale      : Scale for destination.
*                  unsigned int destLength     : Length of destination.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Performs linear interpolation in one dimension.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void Scale1D_c
( 
 const unsigned char *source,
 int sourceStep,
 unsigned int sourceScale,
 unsigned int sourceLength,
 unsigned char *dest,
 int destStep,
 unsigned int destScale,
 unsigned int destLength
 )
{
	unsigned int i;
	unsigned int roundValue = destScale / 2;
	unsigned int leftModifier = destScale;
	unsigned int rightModifier = 0;
	unsigned char leftPixel = *source;
	unsigned char rightPixel = *( source + sourceStep );

	(void) sourceLength;

	// These asserts are needed if there are boundary issues...
	//assert ( destScale > sourceScale );
	//assert ( (sourceLength-1) * destScale >= (destLength-1) * sourceScale );

	for ( i=0; i<destLength*destStep; i+=destStep )
	{
		dest[i] = (INT8)((leftModifier*leftPixel + rightModifier*rightPixel + roundValue) / destScale);

		rightModifier += sourceScale;

		while ( rightModifier > destScale )
		{
			rightModifier -= destScale;
			source += sourceStep;
			leftPixel = *source;
			rightPixel = *( source + sourceStep );
		}

		leftModifier = destScale - rightModifier;
	}
}

/****************************************************************************
* 
*  ROUTINE       : Scale1D_2t1_i
*
*  INPUTS        : const unsigned char *source : Pointer to data to be scaled.
*                  int sourceStep              : Number of pixels to step on in source.
*                  unsigned int sourceScale    : Scale for source (UNUSED).
*                  unsigned int sourceLength   : Length of source (UNUSED).
*                  unsigned char *dest         : Pointer to output data array.
*                  int destStep                : Number of pixels to step on in destination.
*                  unsigned int destScale      : Scale for destination (UNUSED).
*                  unsigned int destLength     : Length of destination.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Performs 2-to-1 interpolated scaling.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void Scale1D_2t1_i
( 
 const unsigned char *source,
 int sourceStep,
 unsigned int sourceScale,
 unsigned int sourceLength,
 unsigned char *dest,
 int destStep,
 unsigned int destScale,
 unsigned int destLength
 )
{
	unsigned int i, j;
	unsigned int temp;

	(void) sourceLength;
	(void) sourceScale;
	(void) destScale;

	sourceStep *= 2;
	dest[0] = source[0];
	for ( i=destStep, j=sourceStep; i<destLength*destStep; i+=destStep, j+=sourceStep )
	{
		temp = 8;
		temp += 3 * source[j-sourceStep];
		temp += 10 * source[j];
		temp += 3 * source[j+sourceStep];
		temp >>= 4;
		dest[i] = (INT8) (temp);
	}
}

/****************************************************************************
* 
*  ROUTINE       : Scale1D_2t1_ps
*
*  INPUTS        : const unsigned char *source : Pointer to data to be scaled.
*                  int sourceStep              : Number of pixels to step on in source.
*                  unsigned int sourceScale    : Scale for source (UNUSED).
*                  unsigned int sourceLength   : Length of source (UNUSED).
*                  unsigned char *dest         : Pointer to output data array.
*                  int destStep                : Number of pixels to step on in destination.
*                  unsigned int destScale      : Scale for destination (UNUSED).
*                  unsigned int destLength     : Length of destination.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Performs 2-to-1 point subsampled scaling.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void Scale1D_2t1_ps
( 
 const unsigned char *source,
 int sourceStep,
 unsigned int sourceScale,
 unsigned int sourceLength,
 unsigned char *dest,
 int destStep,
 unsigned int destScale,
 unsigned int destLength
 )
{
	unsigned int i, j;

	(void) sourceLength;
	(void) sourceScale;
	(void) destScale;

	sourceStep *= 2;
	j = 0;
	for ( i=0; i<destLength*destStep; i+=destStep, j+=sourceStep )
		dest[i] = source[j];
}

/****************************************************************************
* 
*  ROUTINE       : Scale2D
*
*  INPUTS        : const unsigned char *source  : Pointer to data to be scaled.
*                  int sourcePitch              : Stride of source image.
*                  unsigned int sourceWidth     : Width of input image.
*                  unsigned int sourceHeight    : Height of input image.
*                  unsigned char *dest          : Pointer to output data array.
*                  int destPitch                : Stride of destination image.
*                  unsigned int destWidth       : Width of destination image.
*                  unsigned int destHeight      : Height of destination image.
*                  unsigned char *tempArea      : Pointer to temp work area.
*                  unsigned char tempAreaHeight : Height of temp work area.
*                  unsigned int hscale          : Horizontal scale factor numerator.
*                  unsigned int hratio          : Horizontal scale factor denominator.
*                  unsigned int vscale          : Vertical scale factor numerator.
*                  unsigned int vratio          : Vertical scale factor denominator.
*                  unsigned int interlaced      : Interlace flag.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Performs 2-tap linear interpolation in two dimensions.
*
*  SPECIAL NOTES : Expansion is performed one band at a time to help with 
*                  caching.
*
****************************************************************************/
void Scale2D
( 
 const unsigned char *source,
 int sourcePitch,
 unsigned int sourceWidth,
 unsigned int sourceHeight,
 unsigned char *dest,
 int destPitch,
 unsigned int destWidth,
 unsigned int destHeight,
 unsigned char *tempArea,
 unsigned char tempAreaHeight,
 unsigned int hscale,
 unsigned int hratio,
 unsigned int vscale,
 unsigned int vratio,
 unsigned int interlaced
 )
{
	unsigned int i, j, k;
	unsigned int bands;
	unsigned int destBandHeight;
	unsigned int sourceBandHeight;

	typedef void (*Scale1D)( const unsigned char *source,int sourceStep,unsigned int sourceScale,unsigned int sourceLength,
		unsigned char *dest,int destStep,unsigned int destScale,unsigned int destLength);

	Scale1D Scale1Dv = Scale1D_c;
	Scale1D Scale1Dh = Scale1D_c;

	if ( hscale==2 && hratio==1 )
		Scale1Dh = Scale1D_2t1_ps;

	if ( vscale==2 && vratio==1 )
	{
		if ( interlaced )
			Scale1Dv = Scale1D_2t1_ps;
		else
			Scale1Dv = Scale1D_2t1_i;
	}

	if ( sourceHeight == destHeight )
	{
		// for each band of the image
		for ( k=0; k<destHeight; k++ )
		{ 
			Scale1Dh ( source, 1, hscale, sourceWidth+1, dest, 1, hratio, destWidth );
			source += sourcePitch;
			dest   += destPitch;
		}
		return;
	}

	if ( destHeight > sourceHeight )
	{
		destBandHeight   = tempAreaHeight - 1;
		sourceBandHeight = destBandHeight * sourceHeight / destHeight;
	}
	else
	{
		sourceBandHeight = tempAreaHeight - 1;
		destBandHeight   = sourceBandHeight * vratio / vscale;
	}

	// first row needs to be done so that we can stay one row ahead for vertical zoom
	Scale1Dh ( source, 1, hscale, sourceWidth+1, tempArea, 1, hratio, destWidth );

	// for each band of the image
	bands = (destHeight + destBandHeight - 1)/ destBandHeight;
	for ( k=0; k<bands; k++ )
	{
		// scale one band horizontally 
		for ( i=1; i<sourceBandHeight+1; i++ )
		{
			if ( k*sourceBandHeight+i < sourceHeight )
			{
				Scale1Dh ( source+i*sourcePitch, 1, hscale, sourceWidth+1,
					tempArea+i*destPitch, 1, hratio, destWidth );
			}
			else  //  Duplicate the last row 
			{
				// copy tempArea row 0 over from last row in the past
				memcpy ( tempArea+i*destPitch, tempArea+(i-1)*destPitch, destPitch );
			}
		}

		// scale one band vertically 
		for ( j=0; j<destWidth; j++ )
		{
			Scale1Dv ( &tempArea[j], destPitch, vscale, sourceBandHeight+1,
				&dest[j], destPitch, vratio, destBandHeight );
		}

		// copy tempArea row 0 over from last row in the past
		memcpy ( tempArea, tempArea+sourceBandHeight*destPitch, destPitch );

		// move to the next band
		source += sourceBandHeight * sourcePitch;
		dest   += destBandHeight * destPitch;
	}
}

/****************************************************************************
* 
*  ROUTINE       : ScaleFrame
*
*  INPUTS        : YUV_BUFFER_CONFIG *src       : Pointer to frame to be scaled.
*                  YUV_BUFFER_CONFIG *dst       : Pointer to buffer to hold scaled frame.
*                  unsigned char *tempArea      : Pointer to temp work area.
*                  unsigned char tempAreaHeight : Height of temp work area.
*                  unsigned int hscale          : Horizontal scale factor numerator.
*                  unsigned int hratio          : Horizontal scale factor denominator.
*                  unsigned int vscale          : Vertical scale factor numerator.
*                  unsigned int vratio          : Vertical scale factor denominator.
*                  unsigned int interlaced      : Interlace flag.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Performs 2-tap linear interpolation in two dimensions.
*
*  SPECIAL NOTES : Expansion is performed one band at a time to help with 
*                  caching.
*
****************************************************************************/
void ScaleFrame
(  
 YUV_BUFFER_CONFIG *src,
 YUV_BUFFER_CONFIG *dst,
 unsigned char *tempArea,
 unsigned char tempHeight,
 unsigned int hscale,
 unsigned int hratio,
 unsigned int vscale,
 unsigned int vratio,
 unsigned int interlaced
 )
{
	int i;
	int dw = (hscale - 1 + src->YWidth * hratio) / hscale;
	int dh = (vscale - 1 + src->YHeight * vratio) / vscale;

	// call our internal scaling routines!!
	Scale2D ( (unsigned char *) src->YBuffer, src->YStride, src->YWidth, src->YHeight,
		(unsigned char *) dst->YBuffer, dst->YStride, dw, dh,
		tempArea, tempHeight, hscale, hratio, vscale, vratio, interlaced );

	if ( dw < (int)dst->YWidth )
		for ( i=0; i<dh; i++ )
			memset ( dst->YBuffer+i*dst->YStride+dw-1, dst->YBuffer[i*dst->YStride+dw-2], dst->YWidth-dw+1 );

	if ( dh < (int)dst->YHeight )
		for ( i=dh-1; i<(int)dst->YHeight; i++ )
			memcpy(dst->YBuffer + i*dst->YStride, dst->YBuffer + (dh-2) * dst->YStride, dst->YWidth+1);

	Scale2D ( (unsigned char *) src->UBuffer,src->UVStride, src->UVWidth, src->UVHeight,
		(unsigned char *) dst->UBuffer,dst->UVStride, dw/2, dh/2,
		tempArea, tempHeight, hscale, hratio, vscale, vratio, interlaced );

	if ( dw/2 < (int)dst->UVWidth )
		for(i=0;i<dst->UVHeight;i++)
			memset(dst->UBuffer + i * dst->UVStride + dw/2 - 1, dst->UBuffer[i*dst->UVStride+dw/2-2],dst->UVWidth-dw/2 + 1);

	if ( dh/2 < (int)dst->UVHeight )
		for ( i=dh/2-1; i<(int)dst->YHeight/2; i++ )
			memcpy ( dst->UBuffer+i*dst->UVStride, dst->UBuffer+(dh/2-2)*dst->UVStride, dst->UVWidth );

	Scale2D ( (unsigned char *) src->VBuffer,src->UVStride, src->UVWidth, src->UVHeight,
		(unsigned char *) dst->VBuffer,dst->UVStride, dw/2, dh/2,
		tempArea, tempHeight, hscale, hratio, vscale, vratio, interlaced );

	if ( dw/2 < (int)dst->UVWidth )
		for ( i=0; i<dst->UVHeight; i++ )
			memset ( dst->VBuffer+i*dst->UVStride+dw/2-1, dst->VBuffer[i*dst->UVStride+dw/2-2], dst->UVWidth-dw/2+1 );

	if ( dh/2 < (int) dst->UVHeight )
		for ( i=dh/2-1; i<(int)dst->YHeight/2; i++ )
			memcpy ( dst->VBuffer+i*dst->UVStride, dst->VBuffer+(dh/2-2)*dst->UVStride, dst->UVWidth );
}

/****************************************************************************
* 
*  ROUTINE       : Fast_4_5_Scale
*
*  INPUTS        : POSTPROC_INSTANCE *ppi       : Pointer to post-processor instance (NOT USED).
*                  UINT8 *FrameBuffer           : Pointer to source image.
*                  YUV_BUFFER_CONFIG *YuvConfig : Pointer to destination image.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Scales up image by factor of 5/4, creating 5 output
*                  samples for every 4 input samples horizontally & 
*                  vertically.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void Fast_4_5_Scale ( POSTPROC_INSTANCE *ppi, UINT8 *FrameBuffer, YUV_BUFFER_CONFIG *YuvConfig )
{
	// check that width and height are valid please..!
	int h = ppi->Configuration.VideoFrameHeight;
	int w = ppi->Configuration.VideoFrameWidth;
	int nh = YuvConfig->YHeight;
	int nw = YuvConfig->YWidth;

	Scale_4_5_2D ( ppi, &FrameBuffer[ppi->ReconYDataOffset], w+32, w, h,
		(UINT8 *)YuvConfig->YBuffer, nw, nw, nh );
	w  >>= 1;
	h  >>= 1;
	nw >>= 1;
	nh >>= 1;

	Scale_4_5_2D ( ppi, &FrameBuffer[ppi->ReconUDataOffset], w+16, w, h,
		(UINT8 *)YuvConfig->UBuffer, nw, nw, nh );

	Scale_4_5_2D ( ppi, &FrameBuffer[ppi->ReconVDataOffset], w+16, w, h,
		(UINT8 *)YuvConfig->VBuffer, nw, nw, nh );
}

/****************************************************************************
* 
*  ROUTINE       : Fast_3_5_Scale
*
*  INPUTS        : POSTPROC_INSTANCE *ppi       : Pointer to post-processor instance (NOT USED).
*                  UINT8 *FrameBuffer           : Pointer to source image.
*                  YUV_BUFFER_CONFIG *YuvConfig : Pointer to destination image.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Scales up image by factor of 5/3, creating 5 output
*                  samples for every 3 input samples horizontally & 
*                  vertically.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void Fast_3_5_Scale ( POSTPROC_INSTANCE *ppi, UINT8 *FrameBuffer, YUV_BUFFER_CONFIG *YuvConfig )
{
	// check that width and height are valid please..!
	int h = ppi->Configuration.VideoFrameHeight;
	int w = ppi->Configuration.VideoFrameWidth;
	int nh = YuvConfig->YHeight;
	int nw = YuvConfig->YWidth;

	Scale_3_5_2D ( ppi, &FrameBuffer[ppi->ReconYDataOffset], w+32, w, h,
		(UINT8 *)YuvConfig->YBuffer, nw, nw, nh );
	w  >>= 1;
	h  >>= 1;
	nw >>= 1;
	nh >>= 1;

	Scale_3_5_2D ( ppi, &FrameBuffer[ppi->ReconUDataOffset], w+16, w, h,
		(UINT8 *)YuvConfig->UBuffer, nw, nw, nh );

	Scale_3_5_2D ( ppi, &FrameBuffer[ppi->ReconVDataOffset], w+16, w, h,
		(UINT8 *)YuvConfig->VBuffer, nw, nw, nh );
}

/****************************************************************************
* 
*  ROUTINE       : AnyRatio_2D_Scale
*
*  INPUTS        : POSTPROC_INSTANCE *ppi      : Pointer to post-processor instance (NOT USED).
*                  const unsigned char *source : Pointer to source image.
*                  unsigned int sourcePitch    : Stride of source image.
*                  unsigned int sourceWidth    : Width of source image.
*                  unsigned int sourceHeight   : Height of source image (NOT USED).
*                  unsigned char *dest         : Pointer to destination image.
*                  unsigned int destPitch      : Stride of destination image.
*                  unsigned int destWidth      : Width of destination image.
*                  unsigned int destHeight     : Height of destination image.
*
*  OUTPUTS       : None.
*
*  RETURNS       : int: 1 if image scaled, 0 if image could not be scaled.
*	
*  FUNCTION      : Scale the image with changing apect ratio.
*
*  SPECIAL NOTES : This scaling is a bi-linear scaling. Need to re-work the 
*                  whole function for new scaling algorithm.
*
****************************************************************************/
int AnyRatio_2D_Scale
(
 POSTPROC_INSTANCE *ppi, 
 const unsigned char *source,
 unsigned int sourcePitch,
 unsigned int sourceWidth,
 unsigned int sourceHeight,
 unsigned char *dest,
 unsigned int destPitch,
 unsigned int destWidth,
 unsigned int destHeight
 )
{
	unsigned int i, k, max_k;
	unsigned int srcBandHeight  = 0;
	unsigned int destBandHeight = 0;

	// suggested scale factors
	int hs = ppi->Configuration.HScale;
	int hr = ppi->Configuration.HRatio;
	int vs = ppi->Configuration.VScale;
	int vr = ppi->Configuration.VRatio;

	// assume the ratios are scalable instead of should be centered
	int RatioScalable = 1;

	void (*HorizLineScale) ( const unsigned char *, unsigned int, unsigned char *, unsigned int) = NULL;
	void (*VertBandScale) ( unsigned char *, unsigned int, unsigned int) = NULL;
	void (*LastVertBandScale) ( unsigned char *, unsigned int, unsigned int) = NULL;

	(void) ppi;

	// find out the ratio for each direction
	switch ( hr*10/hs )
	{
	case 8:
		// 4-5 Scale in Width direction
		HorizLineScale = HorizontalLine_4_5_Scale;   
		break;
	case 6:
		// 3-5 Scale in Width direction
		HorizLineScale = HorizontalLine_3_5_Scale;
		break;
	case 5: 
		// 1-2 Scale in Width direction
		HorizLineScale = HorizontalLine_1_2_Scale;
		break;
	case 10:
		// no scale in Width direction
		HorizLineScale = HorizontalLine_Copy;
		break;
	default:
		// The ratio is not acceptable now
		// throw("The ratio is not acceptable for now!");
		RatioScalable = 0;
		break;
	}

	switch ( vr*10/vs )
	{
	case 8:
		// 4-5 Scale in vertical direction
		VertBandScale     = VerticalBand_4_5_Scale;
		LastVertBandScale = LastVerticalBand_4_5_Scale;
		srcBandHeight     = 4;
		destBandHeight    = 5;
		break;
	case 6:
		// 3-5 Scale in vertical direction
		VertBandScale     = VerticalBand_3_5_Scale;
		LastVertBandScale = LastVerticalBand_3_5_Scale;
		srcBandHeight     = 3;
		destBandHeight    = 5;
		break;
	case 5:
		// 1-2 Scale in vertical direction
		VertBandScale     = VerticalBand_1_2_Scale;
		LastVertBandScale = LastVerticalBand_1_2_Scale;
		srcBandHeight     = 1;
		destBandHeight    = 2;
		break;
	case 10:
		// no scale in Width direction
		VertBandScale     = NullScale;
		LastVertBandScale = NullScale;
		srcBandHeight     = 4;
		destBandHeight    = 4;
		break;
	default:
		// The ratio is not acceptable now
		// throw("The ratio is not acceptable for now!");
		RatioScalable = 0;
		break;
	}

	if ( RatioScalable == 0 )
		return RatioScalable;

	HorizLineScale ( source, sourceWidth, dest, destWidth );

	// except last band
	max_k = (destHeight+destBandHeight-1)/destBandHeight;
	if (max_k)
	{
		for ( k=0; k<max_k-1; k++ )
		{
			// scale one band horizontally 
			for ( i=1; i<srcBandHeight; i++ )
			{
				HorizLineScale ( source+i*sourcePitch,
					sourceWidth,
					dest+i*destPitch,
					destWidth );
			}

			// first line of next band
			HorizLineScale ( source+srcBandHeight*sourcePitch,
				sourceWidth,
				dest+destBandHeight*destPitch,
				destWidth );

			// Vertical scaling is in place       
			VertBandScale ( dest, destPitch, destWidth );

			// Next band...
			source += srcBandHeight  * sourcePitch;
			dest   += destBandHeight * destPitch;
		}

		// scale one band horizontally 
		for ( i=1; i<srcBandHeight; i++ )
		{
			HorizLineScale ( source+i*sourcePitch,
				sourceWidth,
				dest+i*destPitch,
				destWidth );
		}

		// Vertical scaling is in place       
		LastVertBandScale ( dest, destPitch, destWidth );
	}
	return RatioScalable;
}

/****************************************************************************
* 
*  ROUTINE       : AnyRatioFrameScale
*
*  INPUTS        : POSTPROC_INSTANCE *ppi       : Pointer to post-processor instance (NOT USED).
*                  UINT8 *FrameBuffer           : Pointer to source image.
*                  YUV_BUFFER_CONFIG *YuvConfig : Pointer to destination image.
*                  INT32 YOffset                : Offset from start of buffer to Y samples.
*                  INT32 UVOffset               : Offset from start of buffer to UV samples.
*
*  OUTPUTS       : None.
*
*  RETURNS       : int: 1 if image scaled, 0 if image could not be scaled.
*	
*  FUNCTION      : Scale the image with changing apect ratio.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
int AnyRatioFrameScale
( 
 POSTPROC_INSTANCE *ppi, 
 UINT8 *FrameBuffer, 
 YUV_BUFFER_CONFIG *YuvConfig,
 INT32 YOffset,
 INT32 UVOffset
 )
{
	int i;
	int ew;
	int eh;

	// suggested scale factors
	int hs = ppi->Configuration.HScale;
	int hr = ppi->Configuration.HRatio;
	int vs = ppi->Configuration.VScale;
	int vr = ppi->Configuration.VRatio;

	int RatioScalable = 1;

	int sw = (ppi->Configuration.ExpandedFrameWidth * hr + hs - 1)/hs;
	int sh = (ppi->Configuration.ExpandedFrameHeight * vr + vs - 1)/vs;
	int dw = ppi->Configuration.ExpandedFrameWidth;
	int dh = ppi->Configuration.ExpandedFrameHeight;

	if ( hr == 3 )
		ew = (sw+2)/3 * 3 * hs / hr;
	else
		ew = (sw+7)/8 * 8 * hs / hr;

	if ( vr == 3 )
		eh = (sh+2)/3 * 3 * vs / vr;
	else
		eh = (sh+7)/8 * 8 * vs / vr;

	RatioScalable = AnyRatio_2D_Scale ( ppi, &FrameBuffer[ppi->ReconYDataOffset], 
		ppi->Configuration.VideoFrameWidth +ppi->MVBorder*2, sw, sh,
		(UINT8 *) YuvConfig->YBuffer + YOffset, YuvConfig->YStride, dw, dh);

	for ( i=0; i<eh; i++ )
		memset ( YuvConfig->YBuffer+YOffset+i*YuvConfig->YStride+dw, 0, ew-dw );

	for ( i=dh; i<eh; i++ )
		memset ( YuvConfig->YBuffer+YOffset+i*YuvConfig->YStride, 0, ew );

	if ( RatioScalable==0 )
		return RatioScalable;

	sw = (sw+1)>>1;
	sh = (sh+1)>>1;
	dw = (dw+1)>>1;
	dh = (dh+1)>>1;

	AnyRatio_2D_Scale ( ppi, &FrameBuffer[ppi->ReconUDataOffset], ppi->Configuration.VideoFrameWidth/2+ppi->MVBorder, sw,sh,
		(UINT8 *)YuvConfig->UBuffer+UVOffset, YuvConfig->UVStride, dw, dh );

	AnyRatio_2D_Scale ( ppi, &FrameBuffer[ppi->ReconVDataOffset], ppi->Configuration.VideoFrameWidth/2+ppi->MVBorder, sw, sh,
		(UINT8 *)YuvConfig->VBuffer+UVOffset, YuvConfig->UVStride, dw, dh );

	return RatioScalable;
}

/****************************************************************************
* 
*  ROUTINE       : CenterImage
*
*  INPUTS        : POSTPROC_INSTANCE *ppi       : Pointer to post-processor instance.
*                  UINT8 *FrameBuffer           : Pointer to source image.
*                  YUV_BUFFER_CONFIG *YuvConfig : Pointer to destination image.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Centers the image without scaling in the output buffer.
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void CCONV CenterImage ( POSTPROC_INSTANCE *ppi, UINT8 *FrameBuffer, YUV_BUFFER_CONFIG *YuvConfig )
{
	UINT32 i;
	INT32 RowOffset, ColOffset;
	UINT8 *SrcDataPointer;
	UINT8 *DstDataPointer;

	// center values
	RowOffset = (YuvConfig->YHeight - ppi->Configuration.VideoFrameHeight)/2;
	ColOffset = (YuvConfig->YWidth - ppi->Configuration.VideoFrameWidth)/2;

	// Y's
	SrcDataPointer = &FrameBuffer[ppi->ReconYDataOffset];
	DstDataPointer = (UINT8 *)YuvConfig->YBuffer+RowOffset*YuvConfig->YWidth+ColOffset;
	for ( i=0; i<ppi->Configuration.VideoFrameHeight; i++ )
	{
		memcpy ( DstDataPointer, SrcDataPointer, ppi->Configuration.VideoFrameWidth );
		DstDataPointer += YuvConfig->YWidth;
		SrcDataPointer += ppi->YStride; 
	}

	// U's
	SrcDataPointer = &FrameBuffer[ppi->ReconUDataOffset];
	DstDataPointer = (UINT8 *)YuvConfig->UBuffer+RowOffset/2*YuvConfig->UVWidth+ColOffset/2;
	for ( i=0; i<ppi->Configuration.VideoFrameHeight/2; i++ )
	{
		memcpy ( DstDataPointer, SrcDataPointer, ppi->Configuration.VideoFrameWidth/2 );
		DstDataPointer += YuvConfig->UVWidth;
		SrcDataPointer += ppi->UVStride;
	}

	// V's
	SrcDataPointer = &FrameBuffer[ppi->ReconVDataOffset];
	DstDataPointer = (UINT8 *)YuvConfig->VBuffer+RowOffset/2*YuvConfig->UVWidth+ColOffset/2;
	for ( i=0; i<ppi->Configuration.VideoFrameHeight/2; i++ )
	{
		memcpy ( DstDataPointer, SrcDataPointer, ppi->Configuration.VideoFrameWidth/2 );
		DstDataPointer += YuvConfig->UVWidth;
		SrcDataPointer += ppi->UVStride;
	}
}

/****************************************************************************
* 
*  ROUTINE       : ScaleOrCenter 
*
*  INPUTS        : POSTPROC_INSTANCE *ppi       : Pointer to post-processor instance.
*                  UINT8 *FrameBuffer           : Pointer to source image.
*                  YUV_BUFFER_CONFIG *YuvConfig : Pointer to destination image.
*
*  OUTPUTS       : None.
*
*  RETURNS       : void
*	
*  FUNCTION      : Centers the image without scaling in the output buffer.
*	
*  FUNCTION      : Decides to scale or center image in scale buffer for blit
*
*  SPECIAL NOTES : None. 
*
****************************************************************************/
void CCONV ScaleOrCenter
( 
 POSTPROC_INSTANCE *ppi, 
 UINT8 *FrameBuffer,
 YUV_BUFFER_CONFIG *YuvConfig
 )
{
	if ( ppi->PostProcessingLevel ) 
		UpdateUMVBorder ( ppi, FrameBuffer );

	switch ( ppi->Configuration.ScalingMode )
	{
	case SCALE_TO_FIT:
	case MAINTAIN_ASPECT_RATIO:
		{ 
			// center values
			int row = (YuvConfig->YHeight - (int)ppi->Configuration.ExpandedFrameHeight ) / 2;
			int col = (YuvConfig->YWidth  - (int)ppi->Configuration.ExpandedFrameWidth  ) / 2;

			int YOffset  = row * YuvConfig->YWidth + col;
			int UVOffset = (row>>1) * YuvConfig->UVWidth + (col>>1);

			// perform center and scale 
			AnyRatioFrameScale ( ppi, FrameBuffer, YuvConfig, YOffset, UVOffset );

			break;
		}
		/*		
		case SCALE_TO_FIT:
		// Scale the image if the aspect ratio is scalable
		if ( AnyRatioFrameScale( ppi, FrameBuffer, YuvConfig, 0, 0 ) != 1 )
		CenterImage ( ppi, FrameBuffer, YuvConfig );
		break;
		*/		
	case CENTER:
		CenterImage ( ppi, FrameBuffer, YuvConfig );
		break;

	default:
		break;
	}
}
