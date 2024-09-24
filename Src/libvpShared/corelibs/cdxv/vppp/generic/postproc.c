/*************************************************************************** 
 *
 *   Module Title :     PostProc.c
 *
 *   Description  :     Post Processing
 *
 ***************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "postp.h"
#include "duck_mem.h"
#include "stdlib.h"
#include <math.h>
#include <stddef.h>
/****************************************************************************
*  Macros
****************************************************************************/              
#define Clamp255(x)	(unsigned char) ( (x) < 0 ? 0 : ( (x) <= 255 ? (x) : 255 ) )
// TODO: benski> need better checks for other compilers

#if defined(_M_AMD64) || defined(__LP64__)
#define ROUNDUP32(X) ( ( ( (uintptr_t) X ) + 31 )&( 0xFFFFFFFFFFFFFFE0 ) )
#else //#elif //defined(_M_IX86) 
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) )
#endif

/****************************************************************************
*  Imports
****************************************************************************/              
extern void SimpleDeblockFrame(POSTPROC_INSTANCE *ppi, UINT8* SrcBuffer, UINT8* DestBuffer);
extern void UpdateUMVBorder( POSTPROC_INSTANCE *ppi, UINT8 * DestReconPtr);
extern void PostProcMachineSpecificConfig(UINT32 );

extern void DeringFrame(POSTPROC_INSTANCE *ppi, UINT8 *Src, UINT8 *Dst);
extern void DeringFrameInterlaced(POSTPROC_INSTANCE *ppi, UINT8 *Src, UINT8 *Dst);
extern void DeblockFrame(POSTPROC_INSTANCE *ppi, UINT8 *SourceBuffer, UINT8 *DestinationBuffer);
extern void DeblockFrameUsing7TapFilter(POSTPROC_INSTANCE *ppi, UINT8 *SourceBuffer, UINT8 *DestinationBuffer);
extern void DeblockFrameInterlaced(POSTPROC_INSTANCE *ppi, UINT8 *SourceBuffer, UINT8 *DestinationBuffer);

extern UINT32 DeringModifierV1[ Q_TABLE_SIZE ];
extern UINT32 DeringModifierV2[ Q_TABLE_SIZE ];

extern UINT32 *DCQuantScaleV2;
extern UINT32 *DCQuantScaleUV;
extern UINT32 *DCQuantScaleV1;

extern UINT32  LoopFilterLimitValuesVp4[Q_TABLE_SIZE];
extern UINT32  LoopFilterLimitValuesVp5[Q_TABLE_SIZE];
extern UINT32  LoopFilterLimitValuesVp6[Q_TABLE_SIZE];

extern UINT32 DeblockLimitValuesVp4[Q_TABLE_SIZE];
extern UINT32 DeblockLimitValuesVp5[Q_TABLE_SIZE];
extern UINT32 DeblockLimitValuesVp6[Q_TABLE_SIZE];

extern UINT32 *LoopFilterLimitValuesV2;

extern UINT32  *DeblockLimitValuesV2;

/****************************************************************************
*  Exports
****************************************************************************/
UINT8 LimitVal_VP31[VAL_RANGE * 3];
void  (*FilteringVert_12)(UINT32 QValue,UINT8 * Src, INT32 Pitch); 
void  (*FilteringHoriz_12)(UINT32 QValue,UINT8 * Src, INT32 Pitch); 
void  (*FilteringVert_8)(UINT32 QValue,UINT8 * Src, INT32 Pitch); 
void  (*FilteringHoriz_8)(UINT32 QValue,UINT8 * Src, INT32 Pitch); 
void  (*VerticalBand_4_5_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
void  (*LastVerticalBand_4_5_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
void  (*VerticalBand_3_5_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
void  (*LastVerticalBand_3_5_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
void  (*HorizontalLine_1_2_Scale)(const unsigned char * source,unsigned int sourceWidth,unsigned char * dest,unsigned int destWidth);
void  (*HorizontalLine_3_5_Scale)(const unsigned char * source,unsigned int sourceWidth,unsigned char * dest,unsigned int destWidth);
void  (*HorizontalLine_4_5_Scale)(const unsigned char * source,unsigned int sourceWidth,unsigned char * dest,unsigned int destWidth);
void  (*VerticalBand_1_2_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
void  (*LastVerticalBand_1_2_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
void  (*FilterHoriz_Simple)(xPB_INST ppi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
void  (*FilterVert_Simple)(xPB_INST ppi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
void  (*DeringBlockWeak)(xPB_INST, const UINT8 *, UINT8 *, INT32, UINT32, UINT32 *);
void  (*DeringBlockStrong)(xPB_INST, const UINT8 *, UINT8 *, INT32, UINT32, UINT32 *);
void  (*DeblockLoopFilteredBand)(xPB_INST, UINT8 *, UINT8 *, UINT32, UINT32, UINT32, UINT32 *);
void  (*DeblockNonFilteredBand)(xPB_INST, UINT8 *, UINT8 *, UINT32, UINT32, UINT32, UINT32 *);
void  (*DeblockNonFilteredBandNewFilter)(xPB_INST, UINT8 *, UINT8 *, UINT32, UINT32, UINT32, UINT32 *);
INT32*(*SetupBoundingValueArray)(xPB_INST ppi, INT32 FLimit);
INT32*(*SetupDeblockValueArray)(xPB_INST ppi, INT32 FLimit);
void  (*FilterHoriz)(xPB_INST ppi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
void  (*FilterVert)(xPB_INST ppi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
void  (*ClampLevels)( POSTPROC_INSTANCE *ppi,INT32 BlackClamp,	INT32 WhiteClamp, UINT8	*Src, UINT8	*Dst);
void  (*FastDeInterlace)(UINT8 *SrcPtr, UINT8 *DstPtr, INT32 Width, INT32 Height, INT32 Stride);  
void  (*PlaneAddNoise)( UINT8 *Start, UINT32 Width, UINT32 Height, INT32 Pitch, int q);

/****************************************************************************
 * 
 *  ROUTINE       : InitPostProcessing
 *
 *  INPUTS        : UINT32 *DCQuantScaleV2p :
 *	                UINT32 *DCQuantScaleUVp :
 *	                UINT32 *DCQuantScaleV1p :
 *	                UINT32 Version          : Codec version number.
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Initialise pointers to version specific data tables &
 *                  set-up LUTs.
 *
 *  SPECIAL NOTES : None
 *
 ****************************************************************************/
void InitPostProcessing
( 
	UINT32 *DCQuantScaleV2p,
	UINT32 *DCQuantScaleUVp,
	UINT32 *DCQuantScaleV1p,
	UINT32 Version
)
{
    int i;

	for ( i=0; i<VAL_RANGE*3; i++ ) 
    {
		int x = i - VAL_RANGE;
		LimitVal_VP31[i] = Clamp255 ( x );
	}

	DCQuantScaleV2 = DCQuantScaleV2p;
	DCQuantScaleUV = DCQuantScaleUVp;
	DCQuantScaleV1 = DCQuantScaleV1p;

    for ( i=0 ; i<Q_TABLE_SIZE; i++ )
        DeringModifierV1[i] = DCQuantScaleV1[i]; 

    if ( Version >= 6 )
    {
		LoopFilterLimitValuesV2 = LoopFilterLimitValuesVp6;
		DeblockLimitValuesV2    = DeblockLimitValuesVp6;
    }
	else if ( Version >= 5 )
	{
		LoopFilterLimitValuesV2 = LoopFilterLimitValuesVp5;
		DeblockLimitValuesV2    = DeblockLimitValuesVp5;
	}
	else
	{
		LoopFilterLimitValuesV2 = LoopFilterLimitValuesVp4;
		DeblockLimitValuesV2    = DeblockLimitValuesVp4;
	}
	PostProcMachineSpecificConfig ( Version );
}


/****************************************************************************
 * 
 *  ROUTINE       : DeInitPostProcessing
 *
 *  INPUTS        : None.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : De-initializes post-processing module.
 *
 *  SPECIAL NOTES : Currently this function does nothing.
 *
 ****************************************************************************/
void DeInitPostProcessing ( void )
{
	return;
}

/****************************************************************************
 * 
 *  ROUTINE       : DeletePostProcBuffers
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi : Pointer to post-processor instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : De-allocates buffers used by the post-processing module.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void DeletePostProcBuffers ( POSTPROC_INSTANCE *ppi )
{
	if ( ppi->IntermediateBufferAlloc )
		duck_free ( ppi->IntermediateBufferAlloc );
	ppi->IntermediateBufferAlloc = 0;
	ppi->IntermediateBuffer		 = 0;

	if ( ppi->IntermediateBufferAlloc )
		duck_free ( ppi->IntermediateBufferAlloc );
	ppi->IntermediateBufferAlloc = 0;
	ppi->IntermediateBuffer		 = 0;

	if ( ppi->FiltBoundingValueAlloc )
		duck_free ( ppi->FiltBoundingValueAlloc );
	ppi->FiltBoundingValueAlloc	= 0;
	ppi->FiltBoundingValue		= 0;

	if ( ppi->DeblockBoundingValueAlloc )
		duck_free ( ppi->DeblockBoundingValueAlloc );
	ppi->DeblockBoundingValueAlloc = 0;
	ppi->DeblockBoundingValue	   = 0;

	if ( ppi->FragQIndexAlloc )
		duck_free ( ppi->FragQIndexAlloc );
	ppi->FragQIndexAlloc = 0;
	ppi->FragQIndex		 = 0;

	if ( ppi->FragmentVariancesAlloc )
		duck_free ( ppi->FragmentVariancesAlloc );
	ppi->FragmentVariancesAlloc	= 0;
	ppi->FragmentVariances		= 0;

	if ( ppi->FragDeblockingFlagAlloc )
		duck_free ( ppi->FragDeblockingFlagAlloc );
	ppi->FragDeblockingFlagAlloc = 0;
	ppi->FragDeblockingFlag		 = 0;
}

/****************************************************************************
 * 
 *  ROUTINE       : AllocatePostProcBuffers
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi : Pointer to post-processor instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : INT32: TRUE: Success, FALSE Failure (Chenge to BOOL!!)
 *
 *  FUNCTION      : Allocates buffers used by the post-processing module.
 *
 *  SPECIAL NOTES : Uses ROUNDUP32 to align allocated buffers to improve
 *                  cache performance. 
 *
 ****************************************************************************/
INT32 AllocatePostProcBuffers ( POSTPROC_INSTANCE *ppi )
{
	DeletePostProcBuffers ( ppi );

    ppi->IntermediateBufferAlloc     = (UINT8*)duck_malloc ( 32 + ppi->YStride * 
            (ppi->Configuration.VideoFrameHeight + ppi->MVBorder*2) * 3/2 * sizeof(UINT8), DMEM_GENERAL);
    if ( !ppi->IntermediateBufferAlloc ) { DeletePostProcBuffers ( ppi ); return FALSE; };
    ppi->IntermediateBuffer          = (UINT8 *)ROUNDUP32 ( ppi->IntermediateBufferAlloc );

    ppi->FiltBoundingValueAlloc      = (INT32 *)duck_malloc(32+512*sizeof(INT32), DMEM_GENERAL);
    if ( !ppi->FiltBoundingValueAlloc ) { DeletePostProcBuffers ( ppi ); return FALSE; };
	ppi->FiltBoundingValue			 = (INT32 *)ROUNDUP32 ( ppi->FiltBoundingValueAlloc );

	ppi->DeblockBoundingValueAlloc   = (INT32 *)duck_malloc(32+512*sizeof(INT32), DMEM_GENERAL);
    if ( !ppi->DeblockBoundingValueAlloc ) { DeletePostProcBuffers ( ppi ); return FALSE; };
	ppi->DeblockBoundingValue		 = (INT32 *)ROUNDUP32 ( ppi->DeblockBoundingValueAlloc );

	ppi->FragQIndexAlloc			 = (INT32 *)duck_malloc(32+ppi->UnitFragments*sizeof(INT32), DMEM_GENERAL);
    if ( !ppi->FragQIndexAlloc ) { DeletePostProcBuffers ( ppi ); return FALSE; };
	ppi->FragQIndex					 = (INT32 *)ROUNDUP32 ( ppi->FragQIndexAlloc );

	ppi->FragmentVariancesAlloc      = (INT32 *)duck_malloc(32+ppi->UnitFragments*sizeof(INT32), DMEM_GENERAL);
    if ( !ppi->FragmentVariancesAlloc ) { DeletePostProcBuffers ( ppi ); return FALSE; };
	ppi->FragmentVariances			 = (INT32 *)ROUNDUP32 ( ppi->FragmentVariancesAlloc );

	ppi->FragDeblockingFlagAlloc     = (UINT8 *)duck_malloc(32+ppi->UnitFragments*sizeof(UINT8), DMEM_GENERAL);
    if ( !ppi->FragDeblockingFlagAlloc ){ DeletePostProcBuffers ( ppi ); return FALSE; };
	ppi->FragDeblockingFlag			 = (UINT8 *)ROUNDUP32 ( ppi->FragDeblockingFlagAlloc );

	return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       : ChangePostProcConfiguration
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi         : Pointer to post-processor instance.
 *                  CONFIG_TYPE *ConfigurationInit : Pointer to 
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 * 
 *  FUNCTION      : Initialize post-processor to with the setting passed in.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void ChangePostProcConfiguration ( POSTPROC_INSTANCE *ppi, CONFIG_TYPE *ConfigurationInit )
{
	memcpy ((void *)&ppi->Configuration, (void *)ConfigurationInit, sizeof(CONFIG_TYPE) );

	ppi->HFragments       = (ppi->Configuration.VideoFrameWidth >> 3);
	ppi->VFragments       = (ppi->Configuration.VideoFrameHeight>> 3);
	ppi->YStride          = ppi->Configuration.YStride;
	ppi->UVStride         = ppi->Configuration.UVStride;
	ppi->YPlaneFragments  = ppi->HFragments * ppi->VFragments;
	ppi->UVPlaneFragments = ppi->YPlaneFragments / 4;
	ppi->UnitFragments    = ppi->YPlaneFragments + 2 * ppi->UVPlaneFragments;
	ppi->MVBorder         = (ppi->YStride - 8*ppi->HFragments)/2;
	ppi->ReconYDataOffset = ppi->MVBorder * ppi->YStride + ppi->MVBorder;
	ppi->ReconYDataOffset = ppi->MVBorder * ppi->YStride + ppi->MVBorder;

	ppi->ReconUDataOffset = 
		(ppi->YStride * (ppi->Configuration.VideoFrameHeight + ppi->MVBorder*2)) 
		+ ppi->MVBorder / 2 * ppi->UVStride + ppi->MVBorder/2;

	ppi->ReconVDataOffset = 
		(ppi->YStride * (ppi->Configuration.VideoFrameHeight + ppi->MVBorder*2)) 
		+ (ppi->UVStride * (ppi->Configuration.VideoFrameHeight/2 + ppi->MVBorder)) 
		+ ppi->MVBorder/2 * ppi->UVStride +ppi->MVBorder/2;

	AllocatePostProcBuffers ( ppi );
}

/****************************************************************************
 * 
 *  ROUTINE       : CreatePostProcInstance
 *
 *  INPUTS        : CONFIG_TYPE *ConfigurationInit : Pointer to configuration.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : POSTPROC_INSTANCE *: Pointer to allocated & configured
 *                                       post-processor instance.
 * 
 *  FUNCTION      : Allocates space for and initializes a post-processor
 *                  instance.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
POSTPROC_INSTANCE *CreatePostProcInstance ( CONFIG_TYPE *ConfigurationInit )
{
	POSTPROC_INSTANCE *ppi;
	int postproc_size = sizeof ( POSTPROC_INSTANCE );

	ppi = (POSTPROC_INSTANCE *) duck_malloc ( postproc_size, DMEM_GENERAL );
    if ( !ppi )
        return 0;

	// initialize whole structure to 0
	memset ( (unsigned char *)ppi, 0, postproc_size );
	
	ChangePostProcConfiguration ( ppi, ConfigurationInit );

    ppi->AddNoiseMode = 1;

	return ppi;
}

/****************************************************************************
 * 
 *  ROUTINE       : DeletePostProcInstance
 *
 *  INPUTS        : POSTPROC_INSTANCE **ppi : Pointer-to-pointer to post-processor instance.
 *
 *  OUTPUTS       : POSTPROC_INSTANCE **ppi : Pointer-to-pointer to post-processor instance.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : Deletes post-processor instance & de-allocates memory.
 *
 *  SPECIAL NOTES : Pointer to post-processor instance is set to NULL
 *                  on exit.
 *
 ****************************************************************************/
void DeletePostProcInstance ( POSTPROC_INSTANCE **ppi )
{
    if ( *ppi )
    {
        // Delete any other dynamically allocaed temporary buffers
		DeletePostProcBuffers ( *ppi );
		duck_free ( *ppi );
		*ppi = 0;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : SetPPInterlacedMode
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi : Pointer to post-processor instance.
 *                  int Interlaced         : 0=Non-interlaced, 1=Interlaced.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : Set post-processor's Interlaced Mode to specified value.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void SetPPInterlacedMode ( POSTPROC_INSTANCE *ppi, int Interlaced )
{
	ppi->Configuration.Interlaced = Interlaced;
}

/****************************************************************************
 * 
 *  ROUTINE       : SetDeInterlaceMode
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi : Pointer to post-processor instance.
 *                  int DeInterlaceMode    : Mode to use for de-interlacing.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : Set post-processor's De-Interlace Mode to specified value.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void SetDeInterlaceMode ( POSTPROC_INSTANCE *ppi, int DeInterlaceMode )
{
	ppi->DeInterlaceMode = DeInterlaceMode;
}

/****************************************************************************
 * 
 *  ROUTINE       : SetDeInterlaceMode
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi : Pointer to post-processor instance.
 *                  int DeInterlaceMode    : Mode to use for de-interlacing.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : Set post-processor's De-Interlace Mode to specified value.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void SetAddNoiseMode(POSTPROC_INSTANCE *ppi, int AddNoiseMode)
{
	ppi->AddNoiseMode = AddNoiseMode;
}


/****************************************************************************
 * 
 *  ROUTINE       : UpdateFragQIndex
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi : Pointer to post-processor instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : Update the QIndex for each updated block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void UpdateFragQIndex ( POSTPROC_INSTANCE *ppi )
{
    UINT32 i;
    UINT32 ThisFrameQIndex;    

    // Mark coded blocks with Q-index
    ThisFrameQIndex = ppi->FrameQIndex;

    for ( i=0; i<ppi->UnitFragments; i++ )
        if ( blockCoded ( i ) )
            ppi->FragQIndex[i] = ThisFrameQIndex;
}




/****************************************************************************
 * 
 *  ROUTINE       : Gaussian
 *
 *  INPUTS        : sigma ( standard deviation), mu ( mean) and x (value)
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : generate height of gaussian distribution curve with 
 *                  deviation sigma and mean mu at position x
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
double gaussian(double sigma, double mu, double x)
{
    return 1 / ( sigma * sqrt(2.0*3.14159265)) * 
        (exp(-(x-mu)*(x-mu)/(2*sigma*sigma)));

}

/****************************************************************************
 * 
 *  ROUTINE       : PlaneAddNoise_C
 *
 *  INPUTS        : UINT8 *Start    starting address of buffer to add gaussian
 *                                  noise to
 *                  UINT32 Width    width of plane
 *                  UINT32 Height   height of plane
 *                  INT32  Pitch    distance between subsequent lines of frame
 *                  INT32  q        quantizer used to determine amount of noise 
 *                                  to add
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : adds gaussian noise to a plane of pixels
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void PlaneAddNoise_C( UINT8 *Start, UINT32 Width, UINT32 Height, INT32 Pitch, int q)
{
    unsigned int i,j;
    INT32 Pitch4 = Pitch * 4;
    const int noiseAmount = 2;
    const int noiseAdder = 2 * noiseAmount + 1;

    unsigned char blackclamp[16];
    unsigned char whiteclamp[16];
    unsigned char bothclamp[16];
    char CharDist[300];
    char Rand[2048];

    double sigma;
    sigma = 1 + .8*(63-q) / 63.0;

    // set up a lookup table of 256 entries that matches 
    // a gaussian distribution with sigma determined by q.
    // 
    {
        double i,sum=0;
        int next,j;

        next=0;
        for(i=-32;i<32;i++)
        {
            int a = (int) (.5+256*gaussian(sigma,0,i));

            if(a)
            {
                for(j=0;j<a;j++)
                {
                    CharDist[next+j]=(char) i;
                }
                next = next+j;
            }

        }
        for(next=next;next<256;next++)
            CharDist[next] = 0;

    }

    // generate a line of 2048 characters following our gaussian distribution
    for(i=0;i<2048;i++)
    {
        Rand[i]=CharDist[rand() & 0xff];
    }

	for(i=0;i<16;i++)
	{
		blackclamp[i]=-CharDist[0];
		whiteclamp[i]=-CharDist[0];
		bothclamp[i]=-2*CharDist[0];
	}

    for(i=0;i<Height;i++)
    {
        UINT8* Pos = Start + i *Pitch;
        INT8*  Ref = (INT8 *) (Rand + (rand() & 0xff));  /* cast required on strict OSX-CW8 */

        for(j=0;j<Width;j++)
        {
            if(Pos[j] < -CharDist[0])
               Pos[j] = -CharDist[0];

            if(Pos[j] > 255-CharDist[0])
               Pos[j] = 255-CharDist[0];

            Pos[j]+=Ref[j];
        }
    }
}
 

/****************************************************************************
 * 
 *  ROUTINE       : PostProcess
 *
 *  INPUTS        : POSTPROC_INSTANCE *ppi     : Pointer to post-processor instance.
 *                  INT32  Vp3VersionNo        : Encoder version used to code frame.
 *                  INT32  FrameType           : Encoding method: Keyframe or non-Keyframe.
 *                  INT32  PostProcessingLevel : Level of post-processing to perform.
 *                  INT32  FrameQIndex         : Q-index used to code frame.
 *                  UINT8  *LastFrameRecon     : Pointer to last frame reconstruction buffer.
 *                  UINT8  *PostProcessBuffer  : Pointer to last post-processing buffer.
 *                  UINT8  *FragInfo           : Pointer to list of coded blocks.
 *                  UINT32 FragInfoElementSize : Size of each element.
 *                  UINT32 FragInfoCodedMask   : Mask to get at whether fragment is coded.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : Applies de-blocking and de-ringing filters to the frame.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void PostProcess
(
    POSTPROC_INSTANCE *ppi,
    INT32        Vp3VersionNo,
    INT32		 FrameType,
    INT32		 PostProcessingLevel,
    INT32		 FrameQIndex,
    UINT8		*LastFrameRecon,
    UINT8		*PostProcessBuffer,
    UINT8		*FragInfo,
    UINT32       FragInfoElementSize,
    UINT32		 FragInfoCodedMask
)
{
	int ReconUVPlaneSize;
    
    // variables passed in per frame
	ppi->Vp3VersionNo			= Vp3VersionNo;
	ppi->FrameType 				= FrameType;
	ppi->PostProcessingLevel 	= PostProcessingLevel;
	ppi->FrameQIndex 			= FrameQIndex;
	ppi->LastFrameRecon 		= LastFrameRecon;
	ppi->PostProcessBuffer 		= PostProcessBuffer;
	ppi->FragInfo 				= FragInfo;
	ppi->FragInfoElementSize 	= FragInfoElementSize;
	ppi->FragInfoCodedMask		= FragInfoCodedMask;

    switch ( ppi->PostProcessingLevel )
    {
    case 8:
        // On a slow machine, use a simpler and faster deblocking filter
		UpdateFragQIndex ( ppi );
		if(ppi->Vp3VersionNo < 2)
		{
	        DeblockFrame ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
		}
		else
		{
            if ( ppi->Configuration.Interlaced && ppi->DeInterlaceMode )
            {
                SimpleDeblockFrame ( ppi, ppi->LastFrameRecon, ppi->IntermediateBuffer );
                ReconUVPlaneSize = ppi->VFragments*2*ppi->YStride;
                memcpy ( ppi->PostProcessBuffer+ppi->ReconUDataOffset, ppi->IntermediateBuffer+ppi->ReconUDataOffset, ReconUVPlaneSize );
                memcpy ( ppi->PostProcessBuffer+ppi->ReconVDataOffset, ppi->IntermediateBuffer+ppi->ReconVDataOffset, ReconUVPlaneSize );
                FastDeInterlace ( ppi->IntermediateBuffer+ppi->ReconYDataOffset, 
                                  ppi->PostProcessBuffer+ppi->ReconYDataOffset, 
                                  ppi->HFragments*8, ppi->VFragments*8, ppi->YStride );
            }
            else
			    SimpleDeblockFrame ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
		}
        break;


    case 6:
	case 5:
	    if ( ppi->Vp3VersionNo < 5 ) 
		{
			UpdateFragQIndex ( ppi );
		}
		else
		{
			if ( ppi->Configuration.Interlaced )
			{
                if ( !ppi->DeInterlaceMode )
                {
                    DeblockFrameInterlaced ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
                    UpdateUMVBorder ( ppi, ppi->PostProcessBuffer );
                    DeringFrameInterlaced ( ppi, ppi->PostProcessBuffer, ppi->PostProcessBuffer );
                }
                else
                {
                    DeblockFrameInterlaced ( ppi, ppi->LastFrameRecon, ppi->IntermediateBuffer );
                    UpdateUMVBorder ( ppi, ppi->IntermediateBuffer );
                    DeringFrameInterlaced ( ppi, ppi->IntermediateBuffer, ppi->IntermediateBuffer );
                    
                    ReconUVPlaneSize = ppi->VFragments*2*ppi->YStride;
                    memcpy ( ppi->PostProcessBuffer+ppi->ReconUDataOffset, ppi->IntermediateBuffer+ppi->ReconUDataOffset, ReconUVPlaneSize );
                    memcpy ( ppi->PostProcessBuffer+ppi->ReconVDataOffset, ppi->IntermediateBuffer+ppi->ReconVDataOffset, ReconUVPlaneSize );
                    FastDeInterlace ( ppi->IntermediateBuffer+ppi->ReconYDataOffset, 
                                      ppi->PostProcessBuffer+ppi->ReconYDataOffset, 
                                      ppi->HFragments*8, ppi->VFragments*8, ppi->YStride);      
                }
				break;
			}
		}
		DeblockFrame ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
		UpdateUMVBorder ( ppi, ppi->PostProcessBuffer );
		DeringFrame ( ppi, ppi->PostProcessBuffer, ppi->PostProcessBuffer );

        if(ppi->AddNoiseMode&&PlaneAddNoise!=0) 
            PlaneAddNoise(ppi->PostProcessBuffer + ppi->ReconYDataOffset,ppi->HFragments*8, ppi->VFragments*8,ppi->YStride,FrameQIndex);

        break;
    case 7:
	    if ( ppi->Vp3VersionNo >= 5 ) 
		{
			if ( ppi->Configuration.Interlaced )		
			{
                if ( !ppi->DeInterlaceMode )
				    DeblockFrameInterlaced ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
                else
                {
				    DeblockFrameInterlaced ( ppi, ppi->LastFrameRecon, ppi->IntermediateBuffer );
                    ReconUVPlaneSize = ppi->VFragments*2*ppi->YStride;
                    memcpy ( ppi->PostProcessBuffer+ppi->ReconUDataOffset, ppi->IntermediateBuffer+ppi->ReconUDataOffset, ReconUVPlaneSize );
                    memcpy ( ppi->PostProcessBuffer+ppi->ReconVDataOffset, ppi->IntermediateBuffer+ppi->ReconVDataOffset, ReconUVPlaneSize );
                    FastDeInterlace ( ppi->IntermediateBuffer+ppi->ReconYDataOffset, 
                                      ppi->PostProcessBuffer+ppi->ReconYDataOffset, 
                                      ppi->HFragments*8, ppi->VFragments*8, ppi->YStride );
                }
				break;
			}
		}
		else
        {
			UpdateFragQIndex ( ppi );
        }
		DeblockFrame ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
		UpdateUMVBorder ( ppi, ppi->PostProcessBuffer );
		DeringFrame ( ppi, ppi->PostProcessBuffer, ppi->PostProcessBuffer );


        break;


    case 4:
	    if ( ppi->Vp3VersionNo >= 5 ) 
		{
			if ( ppi->Configuration.Interlaced )		
			{
                if ( !ppi->DeInterlaceMode )
				    DeblockFrameInterlaced ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
                else
                {
				    DeblockFrameInterlaced ( ppi, ppi->LastFrameRecon, ppi->IntermediateBuffer );
                    ReconUVPlaneSize = ppi->VFragments*2*ppi->YStride;
                    memcpy ( ppi->PostProcessBuffer+ppi->ReconUDataOffset, ppi->IntermediateBuffer+ppi->ReconUDataOffset, ReconUVPlaneSize );
                    memcpy ( ppi->PostProcessBuffer+ppi->ReconVDataOffset, ppi->IntermediateBuffer+ppi->ReconVDataOffset, ReconUVPlaneSize );
                    FastDeInterlace ( ppi->IntermediateBuffer+ppi->ReconYDataOffset, 
                                      ppi->PostProcessBuffer+ppi->ReconYDataOffset, 
                                      ppi->HFragments*8, ppi->VFragments*8, ppi->YStride );
                }
				break;
			}
		}
		else
        {
			UpdateFragQIndex ( ppi );
        }
        DeblockFrame ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
        //PlaneAddNoise(ppi->PostProcessBuffer + ppi->ReconYDataOffset,ppi->HFragments*8, ppi->VFragments*8,ppi->YStride,FrameQIndex);
        break;

    case 1:
        UpdateFragQIndex ( ppi );
        break;

    case 0:
        if ( ppi->Configuration.Interlaced && ppi->DeInterlaceMode )
        {
            ReconUVPlaneSize = ppi->VFragments*2*ppi->YStride;
            memcpy ( ppi->PostProcessBuffer+ppi->ReconUDataOffset, ppi->LastFrameRecon+ppi->ReconUDataOffset, ReconUVPlaneSize );
            memcpy ( ppi->PostProcessBuffer+ppi->ReconVDataOffset, ppi->LastFrameRecon+ppi->ReconVDataOffset, ReconUVPlaneSize );
            FastDeInterlace ( ppi->LastFrameRecon+ppi->ReconYDataOffset, 
                              ppi->PostProcessBuffer+ppi->ReconYDataOffset, 
                              ppi->HFragments*8, ppi->VFragments*8, ppi->YStride );
        }
        break;

    default:
        DeblockFrame ( ppi, ppi->LastFrameRecon, ppi->PostProcessBuffer );
        UpdateUMVBorder ( ppi, ppi->PostProcessBuffer );
        DeringFrame ( ppi, ppi->PostProcessBuffer, ppi->PostProcessBuffer );
        break;
    }
}
