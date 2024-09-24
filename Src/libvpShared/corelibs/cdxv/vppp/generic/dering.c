/****************************************************************************
 *
 *   Module Title :     Dering.c
 *
 *   Description  :     Post-processing de-rining filter routines.
 *
 ***************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Frames
****************************************************************************/
#include "postp.h"
#include "stdlib.h"    /* to get abs() */

/****************************************************************************
*  Macros
****************************************************************************/        
#if ( defined(_MSC_VER) || defined(MAPCA) )
#define abs(x) ( (x>0) ? (x) : (-(x)) )
#endif

#define Clamp(val)  ( (val)<0 ? 0 : ((val)>255 ? 255 : (val)) )

/****************************************************************************
*  Exported Global Variables
****************************************************************************/
UINT32 DeringModifierV1[Q_TABLE_SIZE];

/*const*/ UINT32 DeringModifierV2[Q_TABLE_SIZE] =
{
    9,  9,  8,  8,  7,  7,  7,  7,
    6,  6,  6,  6,  6,  6,  6,  6, 
    6,  6,  6,  6,  6,  6,  6,  6,
    5,  5,  5,  5,  5,  5,  5,  5,
    5,  5,  5,  5,  5,  5,  5,  5,  
    4,  4,  4,  4,  4,  4,  4,  4,  
    4,  4,  4,  4,  4,  4,  4,  4,  
    3,  3,  3,  3,  2,  2,  2,  2 
};

/*const*/ UINT32 DeringModifierV3[Q_TABLE_SIZE] =
{
    9,  9,  9,  9,  8,  8,  8,  8,
    7,  7,  7,  7,  7,  7,  7,  7, 
    6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  5,  5,  5,  5,  5,  5,  
    4,  4,  4,  4,  3,  3,  3,  3,  
    2,  2,  2,  0,  0,  0,  0,  0 
};

/*const*/ INT32 SharpenModifier[Q_TABLE_SIZE] =
{  
    -12, -11, -10, -10,  -9,  -9,  -9,  -9,
     -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6, 
     -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,
     -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
     -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0
};

/****************************************************************************
 * 
 *  ROUTINE       : DeringBlockStrong_C
 *
 *  INPUTS        : const POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  const UINT8 *SrcPtr          : Pointer to input image.
 *                  UINT8 *DstPtr                : Pointer to output image.
 *                  const INT32 Pitch            : Stride of SrcPtr & DstPtr.
 *                  UINT32 FragQIndex            : Quantizer index to use.
 *                  UINT32 *QuantScale           :
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a strong de-ringing filter to a block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeringBlockStrong_C
( 
   const POSTPROC_INSTANCE *pbi, 
   const UINT8 *SrcPtr,
   UINT8 *DstPtr,
   const INT32 Pitch,
   UINT32 FragQIndex,
   UINT32 *QuantScale
)
{
    int B;
    int al;
    int ar;
    int au;
    int ad;
    int atot;
    int High;
    int Low;
    int TmpMod;
    int newVal;
    short UDMod[72];
    short LRMod[72];
    unsigned int j,k;

    unsigned char p;
    unsigned char pl;
    unsigned char pr;
    unsigned char pu;
    unsigned char pd;

    unsigned int rowOffset = 0;
    unsigned int round = (1<<6);
    unsigned int QValue = QuantScale[FragQIndex];
    int Sharpen = SharpenModifier[FragQIndex];

    const unsigned char *Src     = SrcPtr;
    const unsigned char *curRow  = SrcPtr;
    const unsigned char *lastRow = SrcPtr-Pitch;
    const unsigned char *nextRow = SrcPtr+Pitch;
    unsigned char *dstRow        = DstPtr;

    (void) pbi;
    Low  = 0;
    High = 3 * QValue;
    
    if ( High>32 )
        High = 32;
       
    /* Initialize the Mod Data */
    for ( k=0; k<9; k++ )
    {           
        for ( j=0; j<8; j++ )
        {
            TmpMod = 32 + QValue - (abs(Src[j]-Src[j-Pitch]));

            if ( TmpMod < -64 )
                TmpMod = Sharpen;
            else if ( TmpMod < Low )
                TmpMod = Low;
            else if ( TmpMod > High )
                TmpMod = High;
            
            UDMod[k*8+j] = (INT16)TmpMod;
        }
        Src += Pitch;
    }

    Src = SrcPtr;

    for ( k=0; k<8; k++ )
    {           
        for ( j=0; j<9; j++ )
        {
            TmpMod = 32 + QValue - (abs(Src[j]-Src[j-1]));
            
            if ( TmpMod < -64 )
                TmpMod = Sharpen;
            else if ( TmpMod < 0 )
                TmpMod = Low;
            else if ( TmpMod > High )
                TmpMod = High;

            LRMod[k*9+j] = (INT16)TmpMod;
        }
        Src += Pitch;
    }
      
    for ( k=0; k<8; k++ )
    {
        // In the case that this function called with
        // same buffer for source and destination, To 
        // keep the c and the mmx version to have 
        // consistant results, intermediate buffer is 
        // used to store the eight pixel value before 
        // writing them to destination(i.e. Overwriting 
        // souce for the speical case)
        
        // column 0 
        int newPixel[8];

        atot = 128;
        B = round;
        p = curRow[rowOffset+0];
        
        pl = curRow[rowOffset+0-1];
        al = LRMod[k*9+0];
        atot -= al;
        B += al * pl; 
        
        pu = lastRow[rowOffset+0];
        au = UDMod[k*8+0];
        atot -= au;
        B += au * pu;
        
        pd = nextRow[rowOffset+0];
        ad = UDMod[(k+1)*8+0];
        atot -= ad;
        B += ad * pd;
        
        pr = curRow[rowOffset+0+1];
        ar = LRMod[k*9+0+1];
        atot -= ar;
        B += ar * pr;
        
        newVal = ( atot * p + B) >> 7;
        
        newPixel[0] = Clamp( newVal );

        // column 1 
        atot = 128;
        B = round;
        p = curRow[rowOffset+1];
        
        pl = curRow[rowOffset+1-1];
        al = LRMod[k*9+1];
        atot -= al;
        B += al * pl; 
        
        pu = lastRow[rowOffset+1];
        au = UDMod[k*8+1];
        atot -= au;
        B += au * pu;
        
        pd = nextRow[rowOffset+1];
        ad = UDMod[(k+1)*8+1];
        atot -= ad;
        B += ad * pd;
        
        pr = curRow[rowOffset+1+1];
        ar = LRMod[k*9+1+1];
        atot -= ar;
        B += ar * pr;
        
        newVal = ( atot * p + B) >> 7;
        
        newPixel[1] = Clamp( newVal );
        
        // column 2 
        atot = 128;
        B = round;
        p = curRow[rowOffset+2];
        
        pl = curRow[rowOffset+2-1];
        al = LRMod[k*9+2];
        atot -= al;
        B += al * pl; 
        
        pu = lastRow[rowOffset+2];
        au = UDMod[k*8+2];
        atot -= au;
        B += au * pu;
        
        pd = nextRow[rowOffset+2];
        ad = UDMod[(k+1)*8+2];
        atot -= ad;
        B += ad * pd;
        
        pr = curRow[rowOffset+2+1];
        ar = LRMod[k*9+2+1];
        atot -= ar;
        B += ar * pr;
        
        newVal = ( atot * p + B) >> 7;
        
        newPixel[2] = Clamp( newVal );

        // column 3 
        atot = 128;
        B = round;
        p = curRow[rowOffset+3];
        
        pl = curRow[rowOffset+3-1];
        al = LRMod[k*9+3];
        atot -= al;
        B += al * pl; 
        
        pu = lastRow[rowOffset+3];
        au = UDMod[k*8+3];
        atot -= au;
        B += au * pu;
        
        pd = nextRow[rowOffset+3];
        ad = UDMod[(k+1)*8+3];
        atot -= ad;
        B += ad * pd;
        
        pr = curRow[rowOffset+3+1];
        ar = LRMod[k*9+3+1];
        atot -= ar;
        B += ar * pr;
        
        newVal = ( atot * p + B) >> 7;
        
        newPixel[3] = Clamp( newVal );

        // column 4 
        atot = 128;
        B = round;
        p = curRow[rowOffset+4];
        
        pl = curRow[rowOffset+4-1];
        al = LRMod[k*9+4];
        atot -= al;
        B += al * pl; 
        
        pu = lastRow[rowOffset+4];
        au = UDMod[k*8+4];
        atot -= au;
        B += au * pu;
        
        pd = nextRow[rowOffset+4];
        ad = UDMod[(k+1)*8+4];
        atot -= ad;
        B += ad * pd;
        
        pr = curRow[rowOffset+4+1];
        ar = LRMod[k*9+4+1];
        atot -= ar;
        B += ar * pr;
        
        newVal = ( atot * p + B) >> 7;
        
        newPixel[4] = Clamp( newVal );

        // column 5 
        atot = 128;
        B = round;
        p = curRow[rowOffset+5];
        
        pl = curRow[rowOffset+5-1];
        al = LRMod[k*9+5];
        atot -= al;
        B += al * pl; 
        
        pu = lastRow[rowOffset+5];
        au = UDMod[k*8+5];
        atot -= au;
        B += au * pu;
        
        pd = nextRow[rowOffset+5];
        ad = UDMod[(k+1)*8+5];
        atot -= ad;
        B += ad * pd;
        
        pr = curRow[rowOffset+5+1];
        ar = LRMod[k*9+5+1];
        atot -= ar;
        B += ar * pr;
        
        newVal = ( atot * p + B) >> 7;
        
        newPixel[5] = Clamp( newVal );
        
        // column 6 
        atot = 128;
        B = round;
        p = curRow[rowOffset+6];
        
        pl = curRow[rowOffset+6-1];
        al = LRMod[k*9+6];
        atot -= al;
        B += al * pl; 
        
        pu = lastRow[rowOffset+6];
        au = UDMod[k*8+6];
        atot -= au;
        B += au * pu;
        
        pd = nextRow[rowOffset+6];
        ad = UDMod[(k+1)*8+6];
        atot -= ad;
        B += ad * pd;
        
        pr = curRow[rowOffset+6+1];
        ar = LRMod[k*9+6+1];
        atot -= ar;
        B += ar * pr;
        
        newVal = ( atot * p + B) >> 7;
        
        newPixel[6] = Clamp( newVal );

        // column 7 
        atot = 128;
        B = round;
        p = curRow[rowOffset+7];
        
        pl = curRow[rowOffset+7-1];
        al = LRMod[k*9+7];
        atot -= al;
        B += al * pl; 
        
        pu = lastRow[rowOffset+7];
        au = UDMod[k*8+7];
        atot -= au;
        B += au * pu;
        
        pd = nextRow[rowOffset+7];
        ad = UDMod[(k+1)*8+7];
        atot -= ad;
        B += ad * pd;
        
        pr = curRow[rowOffset+7+1];
        ar = LRMod[k*9+7+1];
        atot -= ar;
        B += ar * pr;
        
        newVal = ( atot * p + B) >> 7;
        
        newPixel[7] = Clamp( newVal );

        dstRow[rowOffset+0] = (INT8)newPixel[0];
        dstRow[rowOffset+1] = (INT8)newPixel[1];
        dstRow[rowOffset+2] = (INT8)newPixel[2];
        dstRow[rowOffset+3] = (INT8)newPixel[3];
        dstRow[rowOffset+4] = (INT8)newPixel[4];
        dstRow[rowOffset+5] = (INT8)newPixel[5];
        dstRow[rowOffset+6] = (INT8)newPixel[6];
        dstRow[rowOffset+7] = (INT8)newPixel[7];
        
        rowOffset += Pitch;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : DeringBlockWeak_C
 *
 *  INPUTS        : const POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  const UINT8 *SrcPtr          : Pointer to input image.
 *                  UINT8 *DstPtr                : Pointer to output image.
 *                  const INT32 Pitch            : Stride of SrcPtr & DstPtr.
 *                  UINT32 FragQIndex            : Quantizer index to use.
 *                  UINT32 *QuantScale           :
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a weak de-ringing filter to a block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeringBlockWeak_C
( 
    const POSTPROC_INSTANCE *pbi, 
    const UINT8 *SrcPtr,
    UINT8 *DstPtr,
    const INT32 Pitch,
    UINT32 FragQIndex,
    UINT32 *QuantScale
)
{
    int B;
    int al;
    int ar;
    int au;
    int ad;
    int atot;
    int High;
    int Low;
    int newVal;
    int TmpMod;
    short UDMod[72];
    short LRMod[72];
    unsigned int j, k;
    unsigned char p;
    unsigned char pl;
    unsigned char pr;
    unsigned char pu;
    unsigned char pd;

    unsigned int rowOffset = 0;
    unsigned int round = (1<<6);
    unsigned int QValue = QuantScale[FragQIndex];
    int Sharpen = SharpenModifier[FragQIndex];
    const unsigned char *Src     = SrcPtr;
    const unsigned char *curRow  = SrcPtr;
    const unsigned char *lastRow = SrcPtr-Pitch;
    const unsigned char *nextRow = SrcPtr+Pitch;
    unsigned char *dstRow        = DstPtr;

    (void) pbi;

    Low  = 0;
    High = 3 * QValue;
    
    if ( High>24 ) 
        High = 24;
    
    /* Initialize the Mod Data */
    for ( k=0; k<9; k++ )
    {           
        for ( j=0; j<8; j++ )
        {
            TmpMod = 32 + QValue - 2*(abs(Src[j]-Src[j-Pitch]));

            if ( TmpMod < -64 )
                TmpMod = Sharpen;
            else if ( TmpMod < Low )
                TmpMod = Low;
            else if ( TmpMod > High )
                TmpMod = High;
            
            UDMod[k*8+j] = (INT16)TmpMod;
        }
        Src += Pitch;
    }

    Src = SrcPtr;

    for ( k=0; k<8; k++ )
    {           
        for ( j=0; j<9; j++ )
        {
            TmpMod = 32 + QValue - 2*(abs(Src[j]-Src[j-1]));
            
            if ( TmpMod < -64 )
                TmpMod = Sharpen;
            else if ( TmpMod < Low )
                TmpMod = Low;
            else if ( TmpMod > High )
                TmpMod = High;

            LRMod[k*9+j] = (INT16)TmpMod;
        }
        Src += Pitch;
    }

    for ( k=0; k<8; k++ )
    {
        // loop expanded for speed
        for ( j=0; j<8; j++ )
        {
            // column 0 
            atot = 128;
            B = round;
            p = curRow[rowOffset+j];
            
            pl = curRow[rowOffset+j-1];
            al = LRMod[k*9+j];
            atot -= al;
            B += al * pl;
            
            pu = lastRow[rowOffset+j];
            au = UDMod[k*8+j];
            atot -= au;
            B += au * pu;
            
            pd = nextRow[rowOffset+j];
            ad = UDMod[(k+1)*8+j];
            atot -= ad;
            B += ad * pd;
            
            pr = curRow[rowOffset+j+1];
            ar = LRMod[k*9+j+1];
            atot -= ar;
            B += ar * pr;
            
            newVal = ( atot * p + B) >> 7;
            
            dstRow[ rowOffset+j] = (INT8) Clamp( newVal );
        }
        
        rowOffset += Pitch;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : DeringBlock
 *
 *  INPUTS        : const POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  const UINT8 *SrcPtr          : Pointer to input image.
 *                  UINT8 *DstPtr                : Pointer to output image.
 *                  const INT32 Pitch            : Stride of SrcPtr & DstPtr.
 *                  UINT32 FragQIndex            : Quantizer index to use.
 *                  UINT32 *QuantScale           :
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a de-ringing filter to a block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeringBlock
( 
    const POSTPROC_INSTANCE *pbi, 
    const UINT8 *SrcPtr,
    UINT8 *DstPtr,
    const INT32 Pitch,
    UINT32 FragQIndex,
    const UINT32 *QuantScale,
    UINT32 Variance
)
{
    int B;
    int atot;
    int newVal;
    int High;
    int Low;
    int TmpMod;
    int N[8];   // neighbors
    unsigned int j, k, l;
    unsigned int QValue = QuantScale[FragQIndex];

    int Slope = 4;
    unsigned int round = (1<<7);
    const unsigned char *srcRow = SrcPtr;
    unsigned char *dstRow = DstPtr;
    int Sharpen = SharpenModifier[FragQIndex];

    if ( pbi->PostProcessingLevel > 100 )
        QValue = pbi->PostProcessingLevel - 100;

    if ( Variance > 32768)
        Slope = 4;
    else if (Variance > 2048)
        Slope = 8;

    Low  = 0;
    High = 3 * QValue;
    
    if ( High > 32 )
        High = 32;
    
    for ( k=0; k<8; k++ )
    {
        // loop expanded for speed
        for ( j=0; j<8; j++ )
        {
            // set up 8 neighbors of pixel srcRow[j]
            N[0] = srcRow[j-Pitch-1]; 
            N[1] = srcRow[j-Pitch  ]; 
            N[2] = srcRow[j-Pitch+1];
            N[3] = srcRow[j      -1];
            N[4] = srcRow[j      +1];
            N[5] = srcRow[j+Pitch-1];
            N[6] = srcRow[j+Pitch  ];
            N[7] = srcRow[j+Pitch+1];

            // column 0 
            atot = 256;
            B = round;

            for ( l=0; l<8; l++ )
            {
                TmpMod = 32 + QValue - (Slope *(abs(srcRow[j]-N[l])) >> 2);
                
                if ( TmpMod < -64 )
                    TmpMod = Sharpen;
                else if ( TmpMod < Low )
                    TmpMod = Low;
                else if ( TmpMod > High )
                    TmpMod = High;

                atot -= TmpMod;
                B += TmpMod * N[l];
            }
           
            newVal = ( atot * srcRow[j] + B) >> 8;
            
            dstRow[j] = (INT8) Clamp( newVal );
        }
        
        dstRow += Pitch;
        srcRow += Pitch;
    }
}

/***************************************************************************
 * 
 *  ROUTINE       : DiagonalBlur
 *
 *  INPUTS        : const POSTPROC_INSTANCE *pbi : Pointer to post-processor instance (NOT USED).
 *                  const UINT8 *SrcPtr          : Pointer to input image.
 *                  UINT8 *DstPtr                : Pointer to output image.
 *                  const INT32 Pitch            : Stride of SrcPtr & DstPtr.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a de-ringing filter to a block.
 *
 *  SPECIAL NOTES : None.
 *
 ***************************************************************************/
void DiagonalBlur
( 
    const POSTPROC_INSTANCE *pbi, 
    const UINT8 *SrcPtr,
    UINT8 *DstPtr,
    const INT32 Pitch
)
{
    unsigned int j, k;
    unsigned char *dstRow = DstPtr;
    const unsigned char *srcRow = SrcPtr;

	for ( k=0; k<8; k++ )
	{
		// loop expanded for speed
		for ( j=0; j<8; j++ )
		{
			int sum;
			
			sum = 16;
			sum += 8*srcRow[j];
			sum += 2*srcRow[j-2*Pitch-2]; 
			sum += 2*srcRow[j-2*Pitch+2]; 
			sum += 4*srcRow[j-Pitch  -1];
			sum += 4*srcRow[j-Pitch  +1];
			sum += 4*srcRow[j+Pitch  -1];
			sum += 4*srcRow[j+Pitch  +1];
			sum += 2*srcRow[j+2*Pitch-2];
			sum += 2*srcRow[j+2*Pitch+2];
			
			sum >>= 5;
			
			dstRow[j] = sum;
		}
		
		dstRow += Pitch;
		srcRow += Pitch;
	}
	for ( k=0; k<8; k++ )
	{
		// loop expanded for speed
		for ( j=0; j<8; j++ )
		{
			int sum;
			
			sum = 1;
			sum += 6*srcRow[j];
			sum += -1 * srcRow[j-Pitch];
			sum += -1 * srcRow[j+Pitch];
			sum += -1 * srcRow[j-1];
			sum += -1 * srcRow[j+1];
			
			sum >>= 1;
			
			if ( sum<0 )
				sum = 0;

			if ( sum>255 )
				sum = 255;

			dstRow[j] = sum;
		}
		
		dstRow += Pitch;
		srcRow += Pitch;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : DeringFrame
 *
 *  INPUTS        : POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  UINT8 *Src             : Pointer to input image.
 *                  UINT8 *Dst             : Pointer to output image.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a de-ringing filter to a frame.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeringFrame ( POSTPROC_INSTANCE *pbi, UINT8 *Src, UINT8 *Dst )
{
	UINT32 Block;
    UINT32 col, row;
	UINT32 BlocksAcross, BlocksDown;
	UINT32 *QuantScale;
	UINT32 LineLength;
	INT32  Thresh1,Thresh2,Thresh3,Thresh4;
	UINT8  *SrcPtr;     // Pointer to line of source image data
	UINT8  *DestPtr;    // Pointer to line of destination image data
	INT32  Quality = pbi->FrameQIndex;

    if ( pbi->Vp3VersionNo >= 5 )
    {
		Thresh1 = 384;                  
		Thresh2 = 6 * Thresh1;          
		Thresh3 = 5 * Thresh2/4;        
		Thresh4 = 5 * Thresh2/2;        
	}
	else
	{
		Thresh1 = 2048;
		Thresh2 = 15 * Thresh1;
		Thresh3 = 3 * Thresh2;
		Thresh4 = 4 * Thresh2;
	}

    if ( pbi->Vp3VersionNo >= 5 )
        QuantScale = DeringModifierV3;
    else if ( pbi->Vp3VersionNo >= 2 )
        QuantScale = DeringModifierV2;
    else
        QuantScale = DeringModifierV1;

	BlocksAcross = pbi->HFragments;
	BlocksDown   = pbi->VFragments;

	SrcPtr     = Src + pbi->ReconYDataOffset;
	DestPtr    = Dst + pbi->ReconYDataOffset;
	LineLength = pbi->YStride;

	Block = 0;
	
	// De-ring Y plane
	for ( row=0 ; row<BlocksDown; row++ )
	{
		for ( col=0; col<BlocksAcross; col++ )
		{
			INT32 Variance = pbi->FragmentVariances[Block]; 
			
			if ( (pbi->PostProcessingLevel>5) && (Variance > Thresh3) )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				
				if( (col > 0                && pbi->FragmentVariances[Block-1] > Thresh4 ) ||
					(col + 1 < BlocksAcross && pbi->FragmentVariances[Block+1] > Thresh4 ) ||
					(row + 1 < BlocksDown   && pbi->FragmentVariances[Block+BlocksAcross] > Thresh4) ||
					(row > 0                && pbi->FragmentVariances[Block-BlocksAcross] > Thresh4) )
				{
					DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
					DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				}		
			}
			else if ( Variance > Thresh2 )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if(Variance > Thresh1 )
			{
				DeringBlockWeak ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else
			{
				CopyBlock ( SrcPtr+8*col, DestPtr+8*col, LineLength );
            }
			
			++Block;
		}
		SrcPtr  += 8 * LineLength;
		DestPtr += 8 * LineLength;
    }
    
	// De-ring U plane
	BlocksAcross /= 2;
	BlocksDown   /= 2;
	LineLength   /= 2;

	SrcPtr  = Src + pbi->ReconUDataOffset;
	DestPtr = Dst + pbi->ReconUDataOffset;
	for ( row=0; row<BlocksDown; row++ )
	{
		for ( col=0; col<BlocksAcross; col++ )
		{
			INT32 Variance = pbi->FragmentVariances[Block]; 
			if ( pbi->Vp3VersionNo < 5)
			   Quality = pbi->FragQIndex[Block]; 
			
			if ( (pbi->PostProcessingLevel>5) && (Variance > Thresh4) )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );		
			}
			else if ( Variance > Thresh2 )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if ( Variance > Thresh1 )
			{
				DeringBlockWeak ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else
			{
				CopyBlock ( SrcPtr+8*col, DestPtr+8*col, LineLength );
			}
			
			++Block;
		}
		SrcPtr  += 8 * LineLength;
		DestPtr += 8 * LineLength;
    }

	// De-ring U plane
	SrcPtr  = Src + pbi->ReconVDataOffset;
	DestPtr = Dst + pbi->ReconVDataOffset;

	for ( row=0; row<BlocksDown; row++ )
	{
		for ( col=0; col<BlocksAcross; col++ )
		{
			INT32 Variance = pbi->FragmentVariances[Block]; 

            if ( pbi->Vp3VersionNo < 5 )
			   Quality = pbi->FragQIndex[Block]; 
			
			if ( (pbi->PostProcessingLevel>5) && (Variance > Thresh4) )            
			{
				DeringBlockStrong (pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				DeringBlockStrong (pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				DeringBlockStrong (pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				
			}
			else if ( Variance > Thresh2 )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if ( Variance > Thresh1 )
			{
				DeringBlockWeak ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else
			{
				CopyBlock ( SrcPtr+8*col, DestPtr+8*col, LineLength );
			}
			
			++Block;
		}

        SrcPtr  += 8 * LineLength;
		DestPtr += 8 * LineLength;
    }  
}

/****************************************************************************
 * 
 *  ROUTINE       : DeringFrameInterlaced
 *
 *  INPUTS        : POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  UINT8 *Src             : Pointer to input image.
 *                  UINT8 *Dst             : Pointer to output image.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a de-ringing filter to an INTERLACED frame.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeringFrameInterlaced ( POSTPROC_INSTANCE *pbi, UINT8 *Src, UINT8 *Dst )
{
	UINT32 Block;
    UINT32 col, row;
	UINT32 BlocksAcross,BlocksDown;
	UINT32 LineLength;
	UINT32 *QuantScale;
	INT32 Thresh1,Thresh2,Thresh3,Thresh4;
	UINT8  *SrcPtr;	    // Pointer to line of source image data
	UINT8  *DestPtr;    // Pointer to line of destination image data
	INT32  Quality = pbi->FrameQIndex;

    if ( pbi->Vp3VersionNo >= 5 )
    {
		Thresh1 = 384;                  
		Thresh2 = 6 * Thresh1;          
		Thresh3 = 5 * Thresh2/4;        
		Thresh4 = 5 * Thresh2/2;        
	}
	else
	{
		Thresh1 = 2048;
		Thresh2 = 15 * Thresh1;
		Thresh3 = 3 * Thresh2;
		Thresh4 = 4 * Thresh2;
	}

    if ( pbi->Vp3VersionNo >= 5 )
        QuantScale = DeringModifierV3;
    else if ( pbi->Vp3VersionNo >= 2 )
        QuantScale = DeringModifierV2;
    else
        QuantScale = DeringModifierV1;

	BlocksAcross = pbi->HFragments;
	BlocksDown   = pbi->VFragments/2;       // Y plane will be done in two passes

	SrcPtr  = Src + pbi->ReconYDataOffset;
	DestPtr = Dst + pbi->ReconYDataOffset;
	LineLength = pbi->YStride * 2;			// pitch is doubled for interlacing

	Block = 0;	

    // De-ring Y Plane: Top Field
	for ( row=0; row<BlocksDown; row++ )
	{
		for ( col=0; col<BlocksAcross; col++ )
		{
			INT32 Variance = pbi->FragmentVariances[Block]; 
			
			if ( (pbi->PostProcessingLevel>5) && (Variance > Thresh3) )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				
				if( (col > 0                && pbi->FragmentVariances[Block-1] > Thresh4 ) ||
					(col + 1 < BlocksAcross && pbi->FragmentVariances[Block+1] > Thresh4 ) ||
					(row + 1 < BlocksDown   && pbi->FragmentVariances[Block+BlocksAcross] > Thresh4) ||
					(row > 0                && pbi->FragmentVariances[Block-BlocksAcross] > Thresh4) )
				{
					DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
					DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				}
				
			}
			else if ( Variance > Thresh2 )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if ( Variance > Thresh1 )
			{
				DeringBlockWeak ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else
			{
				CopyBlock ( SrcPtr+8*col, DestPtr+8*col, LineLength );
			}
			
			++Block;
		}

        SrcPtr  += 8 * LineLength;
		DestPtr += 8 * LineLength;
    }

    // De-ring Y Plane: Bottom Field
	SrcPtr  = Src + pbi->ReconYDataOffset + pbi->YStride;
	DestPtr = Dst + pbi->ReconYDataOffset + pbi->YStride;

	for ( row=0; row<BlocksDown; row++ )
	{
		for ( col=0; col<BlocksAcross; col++ )
		{
			INT32 Variance = pbi->FragmentVariances[Block]; 
			
			if ( (pbi->PostProcessingLevel>5) && (Variance > Thresh3) )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				
				if( (col > 0                && pbi->FragmentVariances[Block-1] > Thresh4 ) ||
					(col + 1 < BlocksAcross && pbi->FragmentVariances[Block+1] > Thresh4 ) ||
					(row + 1 < BlocksDown   && pbi->FragmentVariances[Block+BlocksAcross] > Thresh4) ||
					(row > 0                && pbi->FragmentVariances[Block-BlocksAcross] > Thresh4) )
				{
					DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
					DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				}
			}
			else if ( Variance > Thresh2 )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if(Variance > Thresh1 )
			{
				DeringBlockWeak ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else
			{
				CopyBlock ( SrcPtr+8*col, DestPtr+8*col, LineLength );
			}
			
			++Block;
		}

        SrcPtr  += 8 * LineLength;
		DestPtr += 8 * LineLength;
    }

    // NOTE: BlocksDown for UV Planes is same as in Y for interlaced frame.

    // De-ring U Plane
	BlocksAcross /= 2;
	LineLength   /= 4;

	SrcPtr  = Src + pbi->ReconUDataOffset;
	DestPtr = Dst + pbi->ReconUDataOffset;
	
    for ( row=0; row<BlocksDown; row++ )
	{
		for ( col=0; col<BlocksAcross; col++ )
		{
			INT32 Variance = pbi->FragmentVariances[Block]; 
			
			if ( (pbi->PostProcessingLevel>5) && (Variance > Thresh4) )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if ( Variance > Thresh2 )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if ( Variance > Thresh1 )
			{
				DeringBlockWeak ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else
			{
				CopyBlock ( SrcPtr+8*col, DestPtr+8*col, LineLength );
			}
			
			++Block;
		}

        SrcPtr  += 8 * LineLength;
		DestPtr += 8 * LineLength;
    }

    // De-ring V Plane
	SrcPtr  = Src + pbi->ReconVDataOffset;
	DestPtr = Dst + pbi->ReconVDataOffset;

	for ( row=0; row<BlocksDown; row++ )
	{
		for ( col=0; col<BlocksAcross; col++ )
		{
			INT32 Variance = pbi->FragmentVariances[Block]; 
			
			if ( (pbi->PostProcessingLevel>5) && (Variance > Thresh4) )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if ( Variance > Thresh2 )
			{
				DeringBlockStrong ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else if ( Variance > Thresh1 )
			{
				DeringBlockWeak ( pbi, SrcPtr+8*col, DestPtr+8*col, LineLength, Quality, QuantScale );
			}
			else
			{
				CopyBlock ( SrcPtr+8*col, DestPtr+8*col, LineLength );
			}
			
			++Block;
		}

        SrcPtr  += 8 * LineLength;
		DestPtr += 8 * LineLength;
    }
}
