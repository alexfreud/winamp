/****************************************************************************
*
*   Module Title :     vputil_if.h
*
*   Description  :     Codec utilities header file.
*
****************************************************************************/
#ifndef __VPUTIL_IF_H
#define __VPUTIL_IF_H

/****************************************************************************
*  Header Files
****************************************************************************/
#include "codec_common_interface.h"

/****************************************************************************
*  Exports
****************************************************************************/
extern void InitVPUtil ( void );

extern void (*ReconIntra)
( 
 INT16 *tmpBuffer, 
 UINT8 *ReconPtr, 
 UINT16 *ChangePtr, 
 UINT32 LineStep 
);

extern void (*ReconInter)
( 
 INT16 * tmpBuffer, 
 UINT8 * ReconPtr, 
 UINT8 * RefPtr, 
 INT16 * ChangePtr, 
 UINT32 LineStep 
);

extern void (*ReconInterHalfPixel2)
( 
 INT16 * tmpBuffer, 
 UINT8  * ReconPtr, 
 UINT8  * RefPtr1, 
 UINT8 * RefPtr2, 
 INT16  * ChangePtr, 
 UINT32 LineStep 
);

extern void (*idct[65])
(
 INT16 *InputData, 
 INT16 *QuantMatrix, 
 INT16 *OutputData 
);

extern void (*idctc[65])
( 
 INT16 *InputData, 
 INT16 *QuantMatrix, 
 INT16 * OutputData 
);

extern void (*ClearSysState) ( void );

extern void (*ReconBlock)
(
 INT16 *SrcBlock,
 INT16 *ReconRefPtr, 
 UINT8 *DestBlock, 
 UINT32 LineStep
);

extern void (*SubtractBlock)
( 
 UINT8 *SrcBlock, 
 INT16 *DestPtr, 
 UINT32 LineStep 
);

extern void (*UnpackBlock)
( 
 UINT8 *ReconPtr, 
 INT16 *ReconRefPtr, 
 UINT32 ReconPixelsPerLine
);

extern void (*AverageBlock)
( 
 UINT8 *ReconPtr1, 
 UINT8 *ReconPtr2, 
 UINT16 *ReconRefPtr, 
 UINT32 ReconPixelsPerLine
);

extern void (*CopyBlock)
(
 unsigned char *src, 
 unsigned char *dest, 
 unsigned int srcstride
);

extern void (*fdct_short)
( 
 INT16 * InputData, 
 INT16 * OutputData 
);

extern void (*Copy12x12)
(
 const unsigned char *src, 
 unsigned char *dest, 
 unsigned int srcstride,
 unsigned int deststride
);

extern void (*FilterBlockBil_8)
( 
 UINT8 *ReconPtr1, 
 UINT8 *ReconPtr2, 
 UINT8 *ReconRefPtr, 
 UINT32 ReconPixelsPerLine, 
 INT32 ModX, 
 INT32 ModY 
);

extern void (*FilterBlock)
( 
 UINT8 *ReconPtr1, 
 UINT8 *ReconPtr2, 
 UINT16 *ReconRefPtr, 
 UINT32 PixelsPerLine, 
 INT32 ModX, 
 INT32 ModY, 
 BOOL UseBicubic, 
 UINT8 BicubicAlpha
);

extern UINT32 (*FiltBlockBilGetSad)
(
 UINT8 *SrcPtr,
 INT32 SrcStride,
 UINT8 *ReconPtr1,
 UINT8 *ReconPtr2,
 INT32 PixelsPerLine,
 INT32 ModX, 
 INT32 ModY,
 UINT32 BestSoFar
);

#endif
