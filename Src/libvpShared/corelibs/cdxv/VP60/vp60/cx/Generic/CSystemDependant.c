/****************************************************************************
*
*   Module Title :     SystemDependant.c
*
*   Description  :     Miscellaneous system dependant functions
*
****************************************************************************/
#define STRICT              /* Strict type checking. */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <string.h>
#include <time.h>
#include <stdlib.h>   
#include <stdio.h>  

#include "pbdll.h"
#include "compdll.h"
#include "mcomp.h"
#include "quantize.h"
#include "resource.h"    /* Resource IDs. */  

/****************************************************************************
*  Explicit imports
****************************************************************************/
#if defined(POSTPROCESS)
extern void FDct1d4 (INT16 *InputData, INT16 * OutputData);
extern void IDct4( INT16 *InputData, INT16 *OutputData);
#endif 

extern UINT32 ComputeBlockReconError ( CP_INSTANCE *cpi, UINT32 bp);
extern UINT32 GetSumAbsDiffs16(UINT8 * SrcPtr,INT32 SourceStride,UINT8  * RefPtr,INT32 ReconStride,UINT32 ErrorSoFar,UINT32 BestSoFar);
extern UINT32 GetHalfPixelSumAbsDiffs16(UINT8 * SrcPtr,INT32 SourceStride,UINT8 * RefPtr,UINT8 * RefPtr2,INT32 ReconStride,UINT32 ErrorSoFar,UINT32 BestSoFar);
extern UINT32 GetIntraErrorC( UINT8* DataPtr, INT32 SourceStride);
extern UINT32 GetInterErr(  UINT8 * NewDataPtr, INT32 SourceStride, UINT8 * RefDataPtr1,  UINT8 * RefDataPtr2, INT32 RefStride );
extern UINT32 GetSumAbsDiffs( UINT8 * NewDataPtr, INT32 SourceStride, UINT8  * RefDataPtr, INT32 RefStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );
extern UINT32 GetHalfPixelSumAbsDiffs( UINT8 * SrcData, INT32 SourceStride, UINT8 * RefDataPtr1, UINT8 * RefDataPtr2, INT32 RefStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );
extern void VP6_quantize_c( QUANTIZER *pbi, INT16 * DCT_block, Q_LIST_ENTRY * quantized_list, UINT8 bp );
extern UINT32 GetMBFieldVerticalVariance( CP_INSTANCE *cpi);
extern UINT32 FiltBlockBilGetSad_C(UINT8 *SrcPtr,INT32 SrcStride,UINT8 *ReconPtr1,UINT8 *ReconPtr2,INT32 PixelsPerLine,INT32 ModX, INT32 ModY,UINT32 BestSoFar);


/****************************************************************************
 * 
 *  ROUTINE       :     CMachineSpecificConfig
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Sets function pointers to vanilla "C" implementations.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void CMachineSpecificConfig ( void )
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
