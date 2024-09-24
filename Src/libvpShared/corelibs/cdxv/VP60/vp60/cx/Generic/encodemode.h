/****************************************************************************
*        
*   Module Title :     encodemode.h
*
*   Description  :     Functions for encoding modes and Motion Vectors.
*
****************************************************************************/
#ifndef __INC_ENCODEMODE_H
#define __INC_ENCODEMODE_H

#ifndef STRICT
#define STRICT              /* Strict type checking */
#endif

/****************************************************************************
*  Exports
****************************************************************************/        
extern void encodeModeAndMotionVector(CP_INSTANCE* cpi, UINT32 MBrow, UINT32 MBcol);
extern void UpdateModeProbs(CP_INSTANCE *cpi);
extern UINT32 modeCost(CP_INSTANCE *cpi,UINT32 i,UINT32 j,CODING_MODE mode);
extern UINT32 blockModeCost(CP_INSTANCE *cpi,UINT32 i,UINT32 j,CODING_MODE mode);
extern void BuildModeCostEstimates( CP_INSTANCE *cpi );

#endif
