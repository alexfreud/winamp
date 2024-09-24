/****************************************************************************
*        
*   Module Title :     decodemv.h
*
*   Description  :     Functions for decoding modes and motionvectors 
*
****************************************************************************/
#ifndef __INC_DECODEMV_H
#define __INC_DECODEMV_H

#ifndef STRICT
#define STRICT              /* Strict type checking */
#endif

/****************************************************************************
*  Module statics
****************************************************************************/        
#define MV_NODES	17

/****************************************************************************
*  Exports
****************************************************************************/        
extern const UINT8 DefaultMvShortProbs[2][7];
extern const UINT8 VP6_MvUpdateProbs[2][MV_NODES];
extern const UINT8 DefaultMvLongProbs[2][LONG_MV_BITS];
extern const UINT8 DefaultIsShortProbs[2];
extern const UINT8 DefaultSignProbs[2];

extern void VP6_FindNearestandNextNearest(PB_INSTANCE* pbi, UINT32 MBrow, UINT32 MBcol, UINT8 Frame, int *type);
extern void VP6_ConfigureMvEntropyDecoder( PB_INSTANCE *pbi, UINT8 FrameType );
extern void VP6_decodeMotionVector(	PB_INSTANCE *pbi,	MOTION_VECTOR *mv,	CODING_MODE Mode );

#endif
