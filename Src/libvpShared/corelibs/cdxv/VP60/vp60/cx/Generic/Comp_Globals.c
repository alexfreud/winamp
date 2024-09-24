/****************************************************************************
*
*   Module Title :     Comp_Globals.c
*
*   Description  :     Global compressor functions & declarations.
* 
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/ 
#include <math.h>       // For Abs()
#include "compdll.h"
#include "mcomp.h" 

/****************************************************************************
*  Macros
****************************************************************************/ 
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) )

/****************************************************************************
*  Exports
****************************************************************************/
INT32 *XX_LUT;
static INT32 XSquaredTable[511];

// Motion compensation related variables
INT32 *AbsX_LUT = NULL;
static INT32 AbsXTable[511];

UINT32 (*FiltBlockBilGetSad)(UINT8 *SrcPtr,INT32 SrcStride,UINT8 *ReconPtr1,UINT8 *ReconPtr2,INT32 PixelsPerLine,INT32 ModX, INT32 ModY,UINT32 BestSoFar);
UINT32 (*GetSAD16)(UINT8 *, INT32, UINT8 *, INT32, UINT32, UINT32);
UINT32 (*GetSadHalfPixel16)(UINT8 *, INT32, UINT8 *, UINT8 *, INT32, UINT32, UINT32);
UINT32 (*GetSAD)(UINT8 *, INT32, UINT8 *, INT32, UINT32, UINT32);
UINT32 (*GetSadHalfPixel)(UINT8 *, INT32, UINT8 *, UINT8 *, INT32, UINT32, UINT32);
UINT32 (*GetInterError)( UINT8 *, INT32, UINT8 *,  UINT8 *, INT32 );
UINT32 (*GetIntraError)( UINT8 *, INT32 );
void   (*fdct_short) ( INT16 * InputData, INT16 * OutputData );
void   (*Sub8)( UINT8 *FiltPtr, UINT8 *ReconPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconStride);
void   (*Sub8_128)( UINT8 *FiltPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride );
void   (*Sub8Av2)( UINT8 *FiltPtr, UINT8 *ReconPtr1, UINT8 *ReconPtr2, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconStride );

/****************************************************************************
*  Explicit Imports
****************************************************************************/
extern unsigned int CPUFrequency;
extern void VP6_DeleteTmpBuffers(PB_INSTANCE * pbi);
extern BOOL VP6_AllocateTmpBuffers(PB_INSTANCE * pbi);
extern void VP6_VPInitLibrary(void);
extern void VP6_VPDeInitLibrary(void);
extern void FillValueTokens ( void );

/****************************************************************************
 *
 *  ROUTINE       :     EDeleteFragmentInfo
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Deletes memory allocated for member data structures.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void EDeleteFragmentInfo ( CP_INSTANCE *cpi )
{
    if( cpi->DCT_codes )
        duck_free( cpi->DCT_codes );
    cpi->DCT_codes = 0;

    if( cpi->DCTDataBuffer )
        duck_free( cpi->DCTDataBuffer);
    cpi->DCTDataBuffer = 0;

    if( cpi->quantized_list)
        duck_free( cpi->quantized_list);
    cpi->quantized_list = 0;

    if( cpi->MbBestErr )
        duck_free(cpi->MbBestErr);
    cpi->MbBestErr = 0;

#if defined FULLFRAMEFDCT
    if( cpi->FDCTCoeffs)
        duck_free(cpi->FDCTCoeffs);
    cpi->FDCTCoeffs = 0;
#endif

}

/****************************************************************************
 *
 *  ROUTINE       :     EAllocateFragmentInfo
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     BOOL: TRUE on success, FALSE if allocation failure.
 *
 *  FUNCTION      :     Allocates memory for encoder data structures.
 *
 *  SPECIAL NOTES :     Uses ROUNDUP32 to align pointers to 32-byte boundaries.
 *
 ****************************************************************************/
BOOL EAllocateFragmentInfo ( CP_INSTANCE *cpi )
{
    // De-allocate existing memory
    EDeleteFragmentInfo(cpi);

    // Allocate new memory
    cpi->DCT_codes = duck_memalign(32, 64*sizeof(INT16), DMEM_GENERAL);
    if(!cpi->DCT_codes) { EDeleteFragmentInfo(cpi); return FALSE; }

    cpi->quantized_list = duck_memalign(32, 64*sizeof(Q_LIST_ENTRY), DMEM_GENERAL);
    if(!cpi->quantized_list) { EDeleteFragmentInfo(cpi); return FALSE; }

    cpi->DCTDataBuffer = duck_memalign(32, 64*sizeof(INT16), DMEM_GENERAL);
    if(!cpi->DCTDataBuffer) { EDeleteFragmentInfo(cpi); return FALSE; }

    cpi->MbBestErr = (UINT32 *) duck_memalign(32, cpi->pb.MacroBlocks * sizeof(UINT32), DMEM_GENERAL);
    if(!cpi->MbBestErr) { EDeleteFrameInfo(cpi); return FALSE; }

#if defined FULLFRAMEFDCT
    cpi->FDCTCoeffs= (Q_LIST_ENTRY(*)[64]) duck_memalign(32, sizeof(Q_LIST_ENTRY)*64* cpi->pb.UnitFragments , DMEM_GENERAL);
    if(!cpi->FDCTCoeffs) {EDeleteFragmentInfo(cpi); return FALSE;}
#endif


    return TRUE;
}

/****************************************************************************
 *
 *  ROUTINE       :     EDeleteFrameInfo
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Deletes memory allocated for frame buffers.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void EDeleteFrameInfo ( CP_INSTANCE *cpi )
{
    if(cpi->yuv0ptr)
        duck_free(cpi->yuv0ptr);
    cpi->yuv0ptr = 0;

    if(cpi->yuv1ptr)
        duck_free(cpi->yuv1ptr);
    cpi->yuv1ptr = 0;

    if( cpi->CoeffTokens )
        duck_free(cpi->CoeffTokens);
    cpi->CoeffTokens = 0;

    if( cpi->OutputBuffer2 )
        duck_free(cpi->OutputBuffer2);
    cpi->OutputBuffer2 = 0;
}

/****************************************************************************
 *
 *  ROUTINE       :     EAllocateFrameInfo
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     BOOL: TRUE on success, FALSE if allocation failure.
 *
 *  FUNCTION      :     Allocates memory for frame buffers.
 *
 *  SPECIAL NOTES :     Uses ROUNDUP32 to align pointers to 32-byte boundaries.
 *
 ****************************************************************************/
BOOL EAllocateFrameInfo ( CP_INSTANCE *cpi )
{
    int FrameSize = cpi->pb.ReconYPlaneSize + 2 * cpi->pb.ReconUVPlaneSize;

    // De-allocate existing memory
    EDeleteFrameInfo ( cpi );

    // Allocate frame buffers aligned to 32-byte boundaries
    cpi->yuv0ptr = duck_memalign(32, FrameSize*sizeof(YUV_BUFFER_ENTRY), DMEM_GENERAL);
    if(!cpi->yuv0ptr) { EDeleteFrameInfo(cpi); return FALSE; }

    cpi->yuv1ptr = duck_memalign(32, FrameSize*sizeof(YUV_BUFFER_ENTRY), DMEM_GENERAL);
    if(!cpi->yuv1ptr) { EDeleteFrameInfo(cpi); return FALSE; }

    cpi->CoeffTokens = duck_memalign(32, FrameSize*sizeof(TOKENEXTRA), DMEM_GENERAL);
    if(!cpi->CoeffTokens) { EDeleteFrameInfo(cpi); return FALSE; }

    // Allocate the temporary output buffer for packed dct data
    cpi->OutputBuffer2 = duck_memalign(32, FrameSize, DMEM_GENERAL);
    if(!cpi->OutputBuffer2) { EDeleteFrameInfo(cpi); return FALSE; }

    return TRUE;
}

/****************************************************************************
 *
 *  ROUTINE       :     DeleteCPInstance
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     CP_INSTANCE **cpi  : Pointer to pointer to encoder instance.    
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Deletes memory allocated for encoder instance and sets
 *                      encoder instance pointer to NULL.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void DeleteCPInstance ( CP_INSTANCE **cpi )
{
    if ( *cpi != NULL )
    {
		DeletePreProc ( &(*cpi)->preproc );
        VP6_DeleteTmpBuffers ( &(*cpi)->pb );
        duck_free ( *cpi );
        *cpi = NULL;
    }
}

/****************************************************************************
 *
 *  ROUTINE       :     CreateCPInstance
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     CP_INSTANCE *: Pointer to new encoder instance or NULL.
 *
 *  FUNCTION      :     Creates and initializes an encoder instance.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
CP_INSTANCE *CreateCPInstance ( void )
{
    UINT32  i;
    CP_INSTANCE *cpi;

    // Allocate encoder data structure
    int cpi_size = sizeof( CP_INSTANCE );
    cpi = duck_malloc ( cpi_size, DMEM_GENERAL );
    if ( !cpi )
        return NULL;

    // Initialize
    memset ( (unsigned char *)cpi, 0, cpi_size );
    
    // Allocate decoder buffers
    if ( !VP6_AllocateTmpBuffers(&cpi->pb) )
    {
        DeleteCPInstance(&cpi);
        return NULL;
    }

    // Initialise Configuration structure to legal values
    cpi->Configuration.BaseQ                = 32;
    cpi->Configuration.FirstFrameQ          = 32;
    cpi->Configuration.WorstQuality         = 32;
    cpi->Configuration.ActiveWorstQuality   = 8;
	cpi->Configuration.ActiveBestQuality    = Q_TABLE_SIZE - 4;
    cpi->Configuration.OutputFrameRate      = 30;
    cpi->Configuration.TargetBandwidth      = 100*1024;

    cpi->MVChangeFactor                 = 14;
    cpi->FourMvChangeFactor             = 8;
    cpi->ExhaustiveSearchThresh         = 2500;
    cpi->MinImprovementForFourMV        = 100;
    cpi->FourMVThreshold                = 10000;
    cpi->IntraThresh                    = 25;
    cpi->InterTripOutThresh             = 5000;
    cpi->BpbCorrectionFactor            = 1.0;
	cpi->KeyFrameBpbCorrectionFactor    = 1.0;
    cpi->GoldenFrameEnabled             = TRUE;
    cpi->InterPrediction                = TRUE;
    cpi->MotionCompensation             = TRUE;
    cpi->ThreshMapThreshold             = 5;
    cpi->QuickCompress                  = TRUE;
	cpi->RdOpt                          = 0;
    cpi->PreProcFilterLevel             = 4;
	cpi->FixedQ                         = -1;
    cpi->pb.idct                        = idctc;
	cpi->pb.ProcessorFrequency          = CPUFrequency;

    memset ( cpi->pb.DcProbs, 0, sizeof(cpi->pb.DcProbs) );
    memset ( cpi->pb.AcProbs, 0, sizeof(cpi->pb.AcProbs) );

    cpi->nExperimentals = 0;
    for ( i=0; i<C_SET_EXPERIMENTAL_MAX-C_SET_EXPERIMENTAL_MIN+1; i++ )
        cpi->Experimental[i] = 0;

	// Access pointers to MV cost array
	cpi->EstMvCostPtrX = &cpi->EstMVCost[0][MV_ENTROPY_TOKENS / 2];
	cpi->EstMvCostPtrY = &cpi->EstMVCost[1][MV_ENTROPY_TOKENS / 2];

    return cpi;
}

/****************************************************************************
 *
 *  ROUTINE       :     VPEInitLibrary
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Fully initializes the playback library.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void VPEInitLibrary ( void )
{
    int i;

    // Initialise the decompressor
    VP6_VPInitLibrary();
    CMachineSpecificConfig();

    // Prepare Abs difference lookup table
    AbsX_LUT = &AbsXTable[255];
    for ( i=(-255); i<=255; i++ )
        AbsX_LUT[i] = abs(i);

    // Prepare table of squared error values
    XX_LUT = &XSquaredTable[255];
    for ( i=(-255); i<=255; i++ )
        XX_LUT[i] = i*i;

    // Prepare table of tokens for fast look-up
    FillValueTokens();
}

/****************************************************************************
 *
 *  ROUTINE       :     VPEDeInitLibrary
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     De-initializes the playback library.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void VPEDeInitLibrary ( void )
{
    VP6_VPDeInitLibrary();
}
