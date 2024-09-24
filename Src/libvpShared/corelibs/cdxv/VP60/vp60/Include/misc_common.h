/****************************************************************************
*
*   Module Title :     MiscCommon.h
*
*   Description  :     Miscellaneous common routines header file
*
*****************************************************************************
*/
#ifndef __MISC_COMMON_H
#define __MISC_COMMON_H

#include "type_aliases.h"
#include "compdll.h"

/****************************************************************************
*  Function Prototypes
****************************************************************************/
extern double GetEstimatedBpb( CP_INSTANCE *cpi, UINT32 TargetQIndex );
extern void UpdateBpbCorrectionFactor( CP_INSTANCE *cpi, UINT32 FrameSize );
extern void UpRegulateMB( CP_INSTANCE *cpi, UINT32 RegulationQ, UINT32 SB, UINT32 MB, BOOL NoCheck );
extern void ClampAndUpdateQ ( CP_INSTANCE *cpi, UINT32 QIndex );
extern void RegulateQ( CP_INSTANCE *cpi, INT32 TargetBits );
extern void ConfigureQuality( CP_INSTANCE *cpi, UINT32 QualityValue );
extern void CopyBackExtraFrags(CP_INSTANCE *cpi);
extern void VP6_PredictFilteredBlock(PB_INSTANCE* pbi, INT16* OutputPtr, UINT32 bp);

#endif
