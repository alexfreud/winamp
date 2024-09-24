/****************************************************************************
*
*   Module Title :     SystemDependant.c
*
*   Description  :     Miscellaneous system dependant functions.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "compdll.h"

/****************************************************************************
*  Macros
****************************************************************************/
#define MMX_ENABLED 1

/****************************************************************************
*  Imports
****************************************************************************/
// Functions that should only be used in assembly versions of the code
extern unsigned long VP6_GetProcessorFrequency();
extern void GetProcessorFlags(INT32 *MmxEnabled, INT32 *XmmEnabled, INT32 *WmtEnabled);

extern UINT32 ComputeBlockReconError ( CP_INSTANCE *cpi, UINT32 bp  );
extern UINT32 GetSumAbsDiffs16(UINT8 * SrcPtr,INT32 SourceStride,UINT8  * RefPtr,INT32 ReconStride,UINT32 ErrorSoFar,UINT32 BestSoFar);
extern UINT32 GetHalfPixelSumAbsDiffs16(UINT8 * SrcPtr,INT32 SourceStride,UINT8 * RefPtr,UINT8 * RefPtr2,INT32 ReconStride,UINT32 ErrorSoFar,UINT32 BestSoFar);
extern UINT32 WmtGetSumAbsDiffs16(UINT8 * SrcPtr,INT32 SourceStride,UINT8  * RefPtr,INT32 ReconStride,UINT32 ErrorSoFar,UINT32 BestSoFar);
extern UINT32 WmtGetHalfPixelSumAbsDiffs16(UINT8 * SrcPtr,INT32 SourceStride,UINT8 * RefPtr,UINT8 * RefPtr2,INT32 ReconStride,UINT32 ErrorSoFar,UINT32 BestSoFar);

extern UINT32 GetIntraErrorC( UINT8* DataPtr, INT32 SourceStride);
extern UINT32 GetInterErr(  UINT8 * NewDataPtr, INT32 SourceStride, UINT8 * RefDataPtr1,  UINT8 * RefDataPtr2, INT32 RefStride );
extern UINT32 GetSumAbsDiffs( UINT8 * NewDataPtr, INT32 SourceStride, UINT8  * RefDataPtr, INT32 RefStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );
extern UINT32 GetHalfPixelSumAbsDiffs( UINT8 * SrcData, INT32 SourceStride, UINT8 * RefDataPtr1, UINT8 * RefDataPtr2, INT32 RefStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );

extern UINT32 MmxGetSAD( UINT8 * NewDataPtr, INT32 SourceStride, UINT8  * RefDataPtr, INT32 RefStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );
extern UINT32 MmxGetHalfPixelSAD( UINT8 * SrcData, INT32 SourceStride, UINT8 * RefDataPtr1, UINT8 * RefDataPtr2, INT32 RefStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );
extern UINT32 MmxGetInterErr(  UINT8 * NewDataPtr, INT32 SourceStride, UINT8 * RefDataPtr1,  UINT8 * RefDataPtr2, INT32 RefStride );
extern UINT32 MmxGetIntraError( UINT8* DataPtr, INT32 SourceStride);
extern void   MmxSUB8( UINT8 *FiltPtr, UINT8 *ReconPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconSourceStride );
extern void   MmxSUB8_128( UINT8 *FiltPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride );
extern void   MmxSUB8AV2( UINT8 *FiltPtr, UINT8 *ReconPtr1, UINT8 *ReconPtr2, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconSourceStride );

extern UINT32 WmtComputeBlockReconError ( CP_INSTANCE *cpi, UINT32 bp  );
extern void   WmtSUB8( UINT8 *FiltPtr, UINT8 *ReconPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconSourceStride );
extern void   WmtSUB8_128( UINT8 *FiltPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride );
extern void   WmtSUB8AV2( UINT8 *FiltPtr, UINT8 *ReconPtr1, UINT8 *ReconPtr2, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconSourceStride );

extern UINT32 XmmGetInterErr(  UINT8 * NewDataPtr, INT32 SourceStride, UINT8 * RefDataPtr1,  UINT8 * RefDataPtr2, INT32 RefStride );
extern UINT32 XMMGetSAD( UINT8 * NewDataPtr, INT32 SourceStride, UINT8  * RefDataPtr, INT32 RefStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );
extern UINT32 WmtGetIntraError( UINT8* DataPtr, INT32 SourceStride);
extern UINT32 WmtGetHalfPixelSAD( UINT8 * SrcData, INT32 SourceStride, UINT8 * RefDataPtr1, UINT8 * RefDataPtr2, INT32 RefStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );
extern UINT32 WmtGetInterErr(  UINT8 * NewDataPtr, INT32 SourceStride, UINT8 * RefDataPtr1,  UINT8 * RefDataPtr2, INT32 RefStride );

extern void VP6_quantize_c( QUANTIZER *pbi, INT16 * DCT_block, Q_LIST_ENTRY * quantized_list, UINT8 bp );
extern void VP6_quantize_wmt( QUANTIZER *pbi, INT16 * DCT_block, Q_LIST_ENTRY * quantized_list, UINT8 bp );
extern void VP6_quantize_mmx( QUANTIZER *pbi, INT16 * DCT_block, Q_LIST_ENTRY * quantized_list, UINT8 bp );


extern UINT32 GetMBFrameVerticalVariance( CP_INSTANCE *cpi);
extern UINT32 MmxGetMBFrameVertVar( CP_INSTANCE *cpi);
extern UINT32 WmtGetMBFrameVertVar( CP_INSTANCE *cpi);

extern UINT32 GetMBFieldVerticalVariance( CP_INSTANCE *cpi);
extern UINT32 MmxGetMBFieldVertVar( CP_INSTANCE *cpi);
extern UINT32 WmtGetMBFieldVertVar( CP_INSTANCE *cpi);

extern UINT32 FiltBlockBilGetSad_C(UINT8 *SrcPtr,INT32 SrcStride,UINT8 *ReconPtr1,UINT8 *ReconPtr2,INT32 PixelsPerLine,INT32 ModX, INT32 ModY,UINT32 BestSoFar);
extern UINT32 FiltBlockBilGetSad_mmx(UINT8 *SrcPtr,INT32 SrcStride,UINT8 *ReconPtr1,UINT8 *ReconPtr2,INT32 PixelsPerLine,INT32 ModX, INT32 ModY,UINT32 BestSoFar);
extern UINT32 FiltBlockBilGetSad_wmt(UINT8 *SrcPtr,INT32 SrcStride,UINT8 *ReconPtr1,UINT8 *ReconPtr2,INT32 PixelsPerLine,INT32 ModX, INT32 ModY,UINT32 BestSoFar);

/****************************************************************************
 * 
 *  ROUTINE       : MachineSpecificConfig
 *
 *  INPUTS        : None.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Checks for machine specifc features such as MMX support 
 *                  sets appropriate flags and function pointers.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void CMachineSpecificConfig( void )
{
	INT32 MmxEnabled;
	INT32 XmmEnabled; 
	INT32 WmtEnabled;

	GetProcessorFlags( &MmxEnabled, &XmmEnabled, &WmtEnabled);

	GetSAD          = GetSumAbsDiffs;
	GetSadHalfPixel = GetHalfPixelSumAbsDiffs;
	GetInterError   = GetInterErr;

	if( WmtEnabled )
    {
        GetSAD16           = WmtGetSumAbsDiffs16;
        GetSadHalfPixel16  = WmtGetHalfPixelSumAbsDiffs16; 

		GetSAD             = XMMGetSAD; 
		GetSadHalfPixel    = WmtGetHalfPixelSAD;  		
        GetInterError      = WmtGetInterErr;
		GetIntraError      = WmtGetIntraError;
        Sub8               = WmtSUB8;
        Sub8_128           = WmtSUB8_128;
        Sub8Av2            = WmtSUB8AV2;
		VP6_quantize       = VP6_quantize_wmt;
        GetMBFrameVertVar  = WmtGetMBFrameVertVar;
        GetMBFieldVertVar  = WmtGetMBFieldVertVar;
        FiltBlockBilGetSad = FiltBlockBilGetSad_wmt;
        GetBlockReconErr   = WmtComputeBlockReconError;

    }
    else if ( XmmEnabled )
	{
        GetSAD16           = GetSumAbsDiffs16;
        GetSadHalfPixel16  = GetHalfPixelSumAbsDiffs16; 

        GetSAD             = XMMGetSAD;
		GetSadHalfPixel    = MmxGetHalfPixelSAD;  		
        GetInterError      = MmxGetInterErr;
		GetIntraError      = MmxGetIntraError;
        Sub8               = MmxSUB8;
        Sub8_128           = MmxSUB8_128;
        Sub8Av2            = MmxSUB8AV2;
		VP6_quantize       = VP6_quantize_mmx;
        GetMBFrameVertVar  = MmxGetMBFrameVertVar;
        GetMBFieldVertVar  = MmxGetMBFieldVertVar;
        FiltBlockBilGetSad = FiltBlockBilGetSad_mmx;
        GetBlockReconErr   = ComputeBlockReconError;

	}
	else if ( MmxEnabled )
    {
        GetSAD16           = GetSumAbsDiffs16;
        GetSadHalfPixel16  = GetHalfPixelSumAbsDiffs16; 

        GetSAD             = MmxGetSAD;
        GetSadHalfPixel    = MmxGetHalfPixelSAD;
        GetInterError      = MmxGetInterErr;
		GetIntraError      = MmxGetIntraError;
        Sub8               = MmxSUB8;
        Sub8_128           = MmxSUB8_128;
        Sub8Av2            = MmxSUB8AV2;
		VP6_quantize       = VP6_quantize_mmx;
        GetMBFrameVertVar  = MmxGetMBFrameVertVar;
        GetMBFieldVertVar  = MmxGetMBFieldVertVar;
        FiltBlockBilGetSad = FiltBlockBilGetSad_mmx;
        GetBlockReconErr   = ComputeBlockReconError;

    }
    else
    {
        GetSAD16           = GetSumAbsDiffs16;
        GetSadHalfPixel16  = GetHalfPixelSumAbsDiffs16; 

        GetSAD             = GetSumAbsDiffs;
        GetSadHalfPixel    = GetHalfPixelSumAbsDiffs;
        GetInterError      = GetInterErr;
		GetIntraError      = GetIntraErrorC;
		fdct_short         = fdct_short_C;
		VP6_quantize       = VP6_quantize_c;
        Sub8               = SUB8;
        Sub8_128           = SUB8_128;
        Sub8Av2            = SUB8AV2;
        GetMBFrameVertVar  = GetMBFrameVerticalVariance;
        GetMBFieldVertVar  = GetMBFieldVerticalVariance;
        FiltBlockBilGetSad = FiltBlockBilGetSad_C;
        GetBlockReconErr   = ComputeBlockReconError;

    }

}

