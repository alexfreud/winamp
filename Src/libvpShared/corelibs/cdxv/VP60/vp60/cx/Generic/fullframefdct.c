/****************************************************************************
*
*   Module Title :     FullFrameFDCT.c 
*
*   Description  :     Compressor functions for block order transmittal
*
*   AUTHOR       :     Paul Wilkins
*
****************************************************************************/
#define STRICT               /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "compdll.h"
#include "misc_common.h"
#include "decodemode.h"
#include "decodemv.h"
#include "quantize.h"
extern void PredictBlock ( CP_INSTANCE *cpi, BLOCK_POSITION bp, UINT32 MBrow, UINT32 MBcol );
extern void PredictDCE( CP_INSTANCE *cpi, BLOCK_POSITION bp);

#if defined FULLFRAMEFDCT
#endif