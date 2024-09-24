/****************************************************************************
*        
*   Module Title :     encodemv.h
*
*   Description  :     functions for decoding modes and motionvectors 
*
****************************************************************************/
#ifndef __INC_ENCODEMV_H
#define __INC_ENCODEMV_H

#ifndef STRICT
#define STRICT              /* Strict type checking */
#endif

/****************************************************************************
*  Exports
****************************************************************************/        
extern void BuildandPackMvTree( CP_INSTANCE *cpi );
extern void BuildandPackMvTree2( CP_INSTANCE *cpi );
extern void BuildMVCostEstimates( CP_INSTANCE *cpi );
extern void encodeMotionVector ( CP_INSTANCE *cpi, INT32 MVectorX, INT32 MVectorY, CODING_MODE Mode );

#endif
