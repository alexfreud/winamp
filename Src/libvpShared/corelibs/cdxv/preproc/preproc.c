/****************************************************************************
*
*   Module Title :     preproc.c
*
*   Description  :     Simple pre-processor.
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/

#include "memory.h"
#include "preproc.h"

/****************************************************************************
*  Macros
****************************************************************************/
#define FRAMECOUNT 7
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) )

/****************************************************************************
*  Imports
****************************************************************************/
extern void GetProcessorFlags (int *MmxEnabled, int *XmmEnabled, int *WmtEnabled );

/****************************************************************************
*  Exported Global Variables
****************************************************************************/
void (*tempFilter)( PreProcInstance *ppi, unsigned char *s, unsigned char *d, int bytes, int strength );

#ifndef MAPCA
/****************************************************************************
 *
 *  ROUTINE       : spatialFilter_wmt
 *
 *  INPUTS        : PreProcInstance *ppi : Pointer to pre-processor instance.
 *                  unsigned char *s     : Pointer to source frame.
 *                  unsigned char *d     : Pointer to destination frame.
 *                  int width            : WIdth of images.
 *	                int height           : Height of images.
 *                  int pitch            : Stride of images.
 *	                int strength         : Strength of filter to apply.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Performs a closesness adjusted temporarl blur
 *
 *  SPECIAL NOTES : Destination frame can be same as source frame.
 *
 ****************************************************************************/
void spatialFilter_wmt
(
	PreProcInstance *ppi,
	unsigned char *s,
	unsigned char *d,
	int width,
	int height,
	int pitch,
	int strength
)
{
	int i;
	int row = 1;
	int PixelOffsets[] = 
	{ 
		-pitch-1,	-pitch,	-pitch+1,
			  -1,		 0,       +1,
		 pitch-1,	 pitch,	 pitch+1
	};
	unsigned char *frameptr = ppi->frameBuffer;
	
    __declspec(align(16)) unsigned short threes[]  = { 3, 3, 3, 3, 3, 3, 3, 3};
	__declspec(align(16)) unsigned short sixteens[]= {16,16,16,16,16,16,16,16};

	memcpy ( d, s, width );
	
    d += pitch;
	s += pitch;
	
    do
	{
		// NOTE: By doing it this way I am ensuring that pixels will always be unaligned!!!
		int col = 1;
		d[0] = s[0];
		d[width - 1] = s[width - 1];
		do
		{
			__declspec(align(16)) unsigned short counts[8];
			__declspec(align(16)) unsigned short sums[8];
			_asm 
			{
				mov			esi, s					// get the source line 
				add         esi, col				// add the column offset 
				pxor		xmm1,xmm1				// accumulator
				pxor		xmm2,xmm2				// count 
				pxor        xmm7,xmm7				// 0s for use with unpack

		        movq		xmm3, QWORD PTR [esi]   // get 8 pixels
				punpcklbw   xmm3, xmm7				// unpack to shorts
				xor			eax, eax				// neighbor iterator

NextNeighbor:
				mov			ecx, [PixelOffsets+eax*4] // get eax index pixel neighbor offset
				movq        xmm4, QWORD PTR [esi + ecx]  // get ecx index neighbor values
				punpcklbw   xmm4, xmm7				// xmm4 unpacked neighbor values
				movdqa      xmm6, xmm4              // save the pixel values
				psubsw      xmm4, xmm3              // subtracted pixel values
				pmullw		xmm4, xmm4				// square xmm4 
				movd        xmm5, strength
				psrlw       xmm4, xmm5				// should be strength
				pmullw      xmm4, threes			// 3 * modifier
				movdqa		xmm5, sixteens			// 16s
				psubusw     xmm5, xmm4				// 16 - modifiers
				movdqa		xmm4, xmm5				// save the modifiers
				pmullw      xmm4, xmm6				// multiplier values
				paddusw     xmm1, xmm4              // accumulator
				paddusw     xmm2, xmm5              // count
				inc         eax						// next neighbor
				cmp			eax,9					// there are nine neigbors
				jne         NextNeighbor

				movdqa      counts, xmm2
				psrlw       xmm2,1                  // divide count by 2 for rounding
				paddusw     xmm1,xmm2				// rounding added in

				mov			frameptr,esi

				movdqa      sums, xmm1
			}
			
			for ( i=0; i<8; i++ )
			{
				int blurvalue = sums[i] * ppi->fixedDivide[counts[i]];
				blurvalue >>= 16;
				d[col+i] = blurvalue;
			}
			col += 8;

		} while ( col<width-1 );

		d += pitch;
		s += pitch;
		++row;
    } while ( row<height-1 );

	memcpy ( d, s, width );
	__asm emms
}
#endif
/****************************************************************************
 *
 *  ROUTINE       : tempFilter_c
 *
 *  INPUTS        : PreProcInstance *ppi : Pointer to pre-processor instance.
 *                  unsigned char *s     : Pointer to source frame.
 *                  unsigned char *d     : Pointer to destination frame.
 *                  int bytes            : Number of bytes to filter.
 *	                int strength         : Strength of filter to apply.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Performs a closesness adjusted temporarl blur
 *
 *  SPECIAL NOTES : Destination frame can be same as source frame.
 *
 ****************************************************************************/
void tempFilter_c
(
	PreProcInstance *ppi,
	unsigned char *s,
	unsigned char *d,
	int bytes,
	int strength
)
{
	int byte = 0;
	unsigned char *frameptr = ppi->frameBuffer;

	if ( ppi->frame == 0 )
	{
		do
		{
			int frame = 0;
			do
			{
				*frameptr = s[byte];
				++frameptr;
				++frame;
			} while ( frame < FRAMECOUNT );
			
			d[byte] = s[byte];
			
			++byte;
		} while ( byte < bytes );
	}
	else
	{
		int modifier;
		int offset = (ppi->frame % FRAMECOUNT);

		do
		{
			int accumulator = 0;
			int count = 0;
			int frame = 0;
			
			frameptr[offset] = s[byte];

			do
			{
				int pixelValue = *frameptr;
				
				modifier   = s[byte];		
				modifier  -= pixelValue;
				modifier  *= modifier;
				modifier >>= strength;
				modifier  *= 3;

				if(modifier > 16)
					modifier = 16;
				
				modifier = 16 - modifier;
				
				accumulator += modifier * pixelValue;
				
				count += modifier;
				
				frameptr++;
				
				++frame;
			} while ( frame < FRAMECOUNT );
			
			accumulator += (count >> 1);
			accumulator *= ppi->fixedDivide[count];
			accumulator >>= 16;

			d[byte] = accumulator;
			
			++byte;
		} while ( byte < bytes );
	}
	++ppi->frame;
}
#ifndef MAPCA
/****************************************************************************
 *
 *  ROUTINE       : tempFilter_wmt
 *
 *  INPUTS        : PreProcInstance *ppi : Pointer to pre-processor instance.
 *                  unsigned char *s     : Pointer to source frame.
 *                  unsigned char *d     : Pointer to destination frame.
 *                  int bytes            : Number of bytes to filter.
 *	                int strength         : Strength of filter to apply.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Performs a closesness adjusted temporarl blur
 *
 *  SPECIAL NOTES : Destination frame can be same as source frame.
 *
 ****************************************************************************/
void tempFilter_wmt
(
	PreProcInstance *ppi,
	unsigned char *s,
	unsigned char *d,
	int bytes,
	int strength
)
{
	int byte = 0;
	unsigned char * frameptr = ppi->frameBuffer;

    __declspec(align(16)) unsigned short threes[]  ={ 3, 3, 3, 3, 3, 3, 3, 3};
	__declspec(align(16)) unsigned short sixteens[]={16,16,16,16,16,16,16,16};

	if ( ppi->frame == 0 )
	{
		do
		{
			int i;
			int frame = 0;
			
			do
			{
			    for ( i=0; i<8; i++ )
				{
					*frameptr = s[byte+i];
					++frameptr;
				}
				++frame;
			} while ( frame < FRAMECOUNT );
			
		    for ( i=0; i<8; i++ )
				d[byte+i] = s[byte+i];

			byte += 8;
			
		} while ( byte < bytes );
    }
	else
	{
		int i;
		int offset2 = (ppi->frame % FRAMECOUNT);
		
        do
		{
			__declspec(align(16)) unsigned short counts[8];
			__declspec(align(16)) unsigned short sums[8];
			int accumulator = 0;
			int count = 0;
			int frame = 0;
			_asm 
			{
        		mov         eax,offset2	
				mov			edi,s					// source pixels
				pxor		xmm1,xmm1				// accumulator

				pxor        xmm7,xmm7

				mov         esi,frameptr			// accumulator
				pxor		xmm2,xmm2				// count 

		        movq		xmm3, QWORD PTR [edi]       

				movq		QWORD PTR [esi+8*eax],xmm3					

				punpcklbw   xmm3, xmm2				// xmm3 source pixels
				mov			ecx,  FRAMECOUNT

NextFrame:
				movq        xmm4, QWORD PTR [esi]   // get frame buffer values
				punpcklbw   xmm4, xmm7				// xmm4 frame buffer pixels
				movdqa      xmm6, xmm4              // save the pixel values
				psubsw      xmm4, xmm3              // subtracted pixel values
				pmullw		xmm4, xmm4				// square xmm4 
				movd        xmm5, strength
				psrlw       xmm4, xmm5				// should be strength
				pmullw      xmm4, threes			// 3 * modifier
				movdqa		xmm5, sixteens			// 16s
				psubusw     xmm5, xmm4				// 16 - modifiers
				movdqa		xmm4, xmm5				// save the modifiers
				pmullw      xmm4, xmm6				// multiplier values
				paddusw     xmm1, xmm4              // accumulator
				paddusw     xmm2, xmm5              // count
				add         esi, 8					// next frame
				dec         ecx						// next set of eight pixels
				jnz         NextFrame

				movdqa      counts, xmm2
				psrlw       xmm2,1                  // divide count by 2 for rounding
				paddusw     xmm1,xmm2				// rounding added in

				mov			frameptr,esi

				movdqa      sums, xmm1
			}
			
			for ( i=0; i<8; i++ )
			{
				int blurvalue = sums[i] * ppi->fixedDivide[counts[i]];
				blurvalue >>= 16;
				d[i] = blurvalue;
			}
			s += 8;
			d += 8;
			byte += 8;
		} while ( byte < bytes );
	}
	++ppi->frame;
	__asm emms
}

/****************************************************************************
 *
 *  ROUTINE       : tempFilter_mmx
 *
 *  INPUTS        : PreProcInstance *ppi : Pointer to pre-processor instance.
 *                  unsigned char *s     : Pointer to source frame.
 *                  unsigned char *d     : Pointer to destination frame.
 *                  int bytes            : Number of bytes to filter.
 *	                int strength         : Strength of filter to apply.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Performs a closesness adjusted temporarl blur
 *
 *  SPECIAL NOTES : Destination frame can be same as source frame.
 *
 ****************************************************************************/
void tempFilter_mmx
(
	PreProcInstance *ppi,
	unsigned char *s,
	unsigned char *d,
	int bytes,
	int strength
)
{
	int byte = 0;
	unsigned char *frameptr = ppi->frameBuffer;
	
    __declspec(align(16)) unsigned short threes[]  ={ 3, 3, 3, 3};
	__declspec(align(16)) unsigned short sixteens[]={16,16,16,16};

	if ( ppi->frame == 0 )
	{
		do
		{
			int i;
			int frame = 0;
			
			do
			{
			    for ( i=0; i<4; i++ )
				{
					*frameptr = s[byte+i];
					++frameptr;
				}
				++frame;
			} while ( frame < FRAMECOUNT );
			
		    for ( i=0; i<4; i++ )
				d[byte+i] = s[byte+i];

			byte += 4;
			
		} while ( byte < bytes );
	}
	else
	{
		int i;
		int offset2 = (ppi->frame % FRAMECOUNT);
		do
		{
			__declspec(align(16)) unsigned short counts[8];
			__declspec(align(16)) unsigned short sums[8];
			int accumulator = 0;
			int count = 0;
			int frame = 0;
			_asm 
			{

				mov         eax,offset2	
				mov			edi,s					// source pixels
				pxor		mm1,mm1				    // accumulator
				pxor        mm7,mm7

				mov         esi,frameptr			// accumulator
				pxor		mm2,mm2				    // count 

		        movd		mm3, DWORD PTR [edi]       
				movd		DWORD PTR [esi+4*eax],mm3					

				punpcklbw   mm3, mm2				// mm3 source pixels
				mov			ecx,  FRAMECOUNT

NextFrame:
				movd        mm4, DWORD PTR [esi]    // get frame buffer values
				punpcklbw   mm4, mm7				// mm4 frame buffer pixels
				movq	    mm6, mm4                // save the pixel values
				psubsw      mm4, mm3                // subtracted pixel values
				pmullw		mm4, mm4				// square mm4 
				movd        mm5, strength
				psrlw       mm4, mm5				// should be strength
				pmullw      mm4, threes			    // 3 * modifier
				movq		mm5, sixteens			// 16s
				psubusw     mm5, mm4				// 16 - modifiers
				movq		mm4, mm5				// save the modifiers
				pmullw      mm4, mm6				// multiplier values
				paddusw     mm1, mm4                // accumulator
				paddusw     mm2, mm5                // count
				add         esi, 4					// next frame
				dec         ecx						// next set of eight pixels
				jnz         NextFrame

				movq        counts, mm2
				psrlw       mm2,1                   // divide count by 2 for rounding
				paddusw     mm1,mm2				    // rounding added in

				mov			frameptr,esi

				movq        sums, mm1

			}
			
			for ( i=0; i<4; i++ )
			{
				int blurvalue = sums[i] * ppi->fixedDivide[counts[i]];
				blurvalue >>= 16;
				d[i] = blurvalue;
			}
			s += 4;
			d += 4;
			byte += 4;
		} while ( byte < bytes );
	}
	++ppi->frame;
	__asm emms
}
#endif
/****************************************************************************
 *
 *  ROUTINE       : DeletePreProc
 *
 *  INPUTS        : PreProcInstance *ppi : Pointer to pre-processor instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Deletes a pre-processing instance.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeletePreProc ( PreProcInstance *ppi )
{				   
    if ( ppi->frameBufferAlloc )
        duck_free ( ppi->frameBufferAlloc );
    ppi->frameBufferAlloc = 0;
    ppi->frameBuffer      = 0;

    if( ppi->fixedDivideAlloc )
        duck_free ( ppi->fixedDivideAlloc );
    ppi->fixedDivideAlloc = 0;
    ppi->fixedDivide      = 0;
}

/****************************************************************************
 *
 *  ROUTINE       : InitPreProc
 *
 *  INPUTS        : PreProcInstance *ppi : Pointer to pre-processor instance.
 *                  int FrameSize        : Number of bytes in one frame.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : int: 1 if successful, 0 if failed.
 *
 *  FUNCTION      : Initializes prepprocessor instance.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
int InitPreProc ( PreProcInstance *ppi, int FrameSize )
{
	int i;
	int MmxEnabled;
	int XmmEnabled; 
	int WmtEnabled;
#ifndef MAPCA
	GetProcessorFlags ( &MmxEnabled, &XmmEnabled, &WmtEnabled );

	if ( WmtEnabled )
		tempFilter = tempFilter_wmt;
	else if ( MmxEnabled )
		tempFilter = tempFilter_mmx;
	else 
#endif
		tempFilter = tempFilter_c;

	DeletePreProc ( ppi );

	ppi->frameBufferAlloc = duck_malloc ( 32+FrameSize*7*sizeof(unsigned char), DMEM_GENERAL );
    if ( !ppi->frameBufferAlloc ) { DeletePreProc( ppi ); return 0; }
    ppi->frameBuffer = (unsigned char *) ROUNDUP32( ppi->frameBufferAlloc );

	ppi->fixedDivideAlloc = duck_malloc ( 32+255*sizeof(unsigned int), DMEM_GENERAL );
    if ( !ppi->fixedDivideAlloc ) { DeletePreProc( ppi ); return 0; }
    ppi->fixedDivide = (unsigned int *) ROUNDUP32( ppi->fixedDivideAlloc );

	for ( i=1; i<255; i++ )
		ppi->fixedDivide[i] = 0x10000 / i;
	return 1;
}

/****************************************************************************
 *
 *  ROUTINE       : spatialFilter_c
 *
 *  INPUTS        : PreProcInstance *ppi : Pointer to pre-processor instance.
 *                  unsigned char *s     : Pointer to source frame.
 *                  unsigned char *d     : Pointer to destination frame.
 *                  int width            : Width of images.
 *	                int height           : Height of images.
 *                  int pitch            : Stride of images.
 *	                int strength         : Strength of filter to apply.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Performs a closesness adjusted temporal blur.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void spatialFilter_c
(
	PreProcInstance *ppi,
	unsigned char *s,
	unsigned char *d,
	int width,
	int height,
	int pitch,
	int strength
)
{
	int modifier;
	int byte = 0;
	int row = 1;
	int PixelOffsets[9];
	
	
	PixelOffsets[0] = -pitch - 1;
	PixelOffsets[1] = -pitch;
	PixelOffsets[2] = -pitch + 1;
	PixelOffsets[3] =		 - 1;
	PixelOffsets[4] =		   0;
	PixelOffsets[5] =		 + 1;
	PixelOffsets[6] =  pitch - 1;
	PixelOffsets[7] =  pitch    ;
	PixelOffsets[8] =  pitch + 1;
	
	memcpy ( d, s, width );

    d += pitch;
	s += pitch;
	
    do
	{
		int col = 1;
		
        d[0] = s[0];
		d[width - 1] = s[width - 1];
		
        do
		{
			int accumulator = 0;
			int count = 0;
			int neighbor = 0;
		
            do
			{
				int pixelValue = s[ col + PixelOffsets[neighbor] ];
				
				modifier = s[col];
				modifier -= pixelValue;
				modifier *= modifier;
				modifier >>= strength;
				modifier *= 3;
				
				if(modifier > 16)
					modifier = 16;
				
				modifier = 16 - modifier;
				
				accumulator += modifier * pixelValue;
				
				count += modifier;
				
				neighbor++;
			} while ( neighbor < sizeof(PixelOffsets)/sizeof(int) );
			
			accumulator += (count >> 1);
			accumulator *= ppi->fixedDivide[count];
			accumulator >>= 16;
			
			d[col] = accumulator;
			
			++col;

		} while ( col < width-1 );

		d += pitch;
		s += pitch;

		++row;
		
	} while ( row < height-1 );
	
    memcpy ( d, s, width );
}
