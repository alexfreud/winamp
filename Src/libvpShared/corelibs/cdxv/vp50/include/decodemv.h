/****************************************************************************
*        
*   Module Title :     decodemode.h
*
*   Description  :     functions for decoding modes and motionvectors 
*
*   AUTHOR       :     James Bankoski
*
*****************************************************************************
*   Revision History
*
*   1.00 JBB 30OCT01  New Configuration baseline.
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#ifndef STRICT
#define STRICT              /* Strict type checking. */
#endif


#define MV_NODES	11

/****************************************************************************
*  Implicit Imports
*****************************************************************************
*/        
/****************************************************************************
*  Exported data structures and functions
*****************************************************************************
*/        

extern void FindNearestandNextNearest(PB_INSTANCE* pbi, UINT32 MBrow, UINT32 MBcol,
    MOTION_VECTORA* nearest, MOTION_VECTORA* nextnearest, UINT8 Frame, int *type);

extern void ConfigureMvEntropyDecoder( PB_INSTANCE *pbi, UINT8 FrameType );

extern void decodeMotionVector(	PB_INSTANCE *pbi,	MOTION_VECTOR *mv,	MOTION_VECTOR *nearestMv);

extern UINT8 MvUpdateProbs[2][MV_NODES];
