/****************************************************************************
*
*   Module Title :     MComp.h
*
*   Description  :     Video CODEC: motion compensation module header .
*
****************************************************************************/
#ifndef __INC_MCOMP_H
#define __INC_MCOMP_H

#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "type_aliases.h"
#include "codec_common.h"
#include "compdll.h"

/****************************************************************************
*  Exports
****************************************************************************/
extern INT32 *AbsX_LUT;

/****************************************************************************
*  Functions
****************************************************************************/
extern void InitMotionCompensation ( CP_INSTANCE *cpi);
extern UINT32 GetIntraErrorC ( UINT8 * DataPtr, INT32 SourceStride );
extern UINT32 GetMBIntraError ( CP_INSTANCE *cpi );
extern UINT32 GetMBInterError ( CP_INSTANCE *cpi, UINT8 * SrcPtr, UINT8 * RefPtr, MOTION_VECTOR *MV, UINT32 * );
extern UINT32 GetMBMVInterError ( CP_INSTANCE *cpi, CODING_MODE	Mode, UINT8 * RefFramePtr, MOTION_VECTOR *MV, UINT32 *TempErrors );
extern UINT32 GetMBMVExhaustiveSearch ( CP_INSTANCE *cpi, CODING_MODE Mode, UINT8 * RefFramePtr, MOTION_VECTOR *MV, UINT32 * );

extern UINT32 GetBMVExhaustiveSearch ( CP_INSTANCE* cpi, UINT8* RefFramePtr, MOTION_VECTOR* MV, UINT32);
extern UINT32 GetBMVSearch ( CP_INSTANCE* cpi, UINT8* RefFramePtr, MOTION_VECTOR* MV, UINT32 );

extern UINT32 GetMBFrameVerticalVariance ( CP_INSTANCE* cpi );
extern UINT32 GetMBFieldVerticalVariance ( CP_INSTANCE* cpi );
extern UINT32 FindMvViaDiamondSearch
(
    CP_INSTANCE *cpi,
    CODING_MODE Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT8 **BestBlockPtr,
    UINT32 BlockSize
);
extern UINT32 FindMvVia3StepSearch
(
    CP_INSTANCE *cpi,
    CODING_MODE Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT8 **BestBlockPtr,
    UINT32 BlockSize
);

extern void FindBestFractionalPixelStep
(
    CP_INSTANCE *cpi,
	CODING_MODE	Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT32 BlockSize,
    UINT32 *MinError,
	UINT8  BitShift
);
extern void SkipFractionalPixelStep
(
    CP_INSTANCE *cpi,
	CODING_MODE	Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT32 BlockSize,
    UINT32 *MinError,
	UINT8  BitShift
);

#endif
