/*!
 ************************************************************************
 *  \file
 *     loopfilter.h
 *  \brief
 *     external deblocking filter interface
 ************************************************************************
 */

#ifndef _LOOPFILTER_H_
#define _LOOPFILTER_H_

#include "global.h"
#include "mbuffer.h"

extern void DeblockPicture(VideoParameters *p_Vid, StorablePicture *p) ;


extern void EdgeLoopLumaNormal_Horiz(ColorPlane pl, VideoImage *image, const byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, StorablePicture *p);
extern void EdgeLoopLumaNormal_Horiz_sse2(ColorPlane pl, VideoImage *image, const byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, StorablePicture *p);
extern void EdgeLoopChromaNormal_Horiz(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, int uv, StorablePicture *p);

#endif //_LOOPFILTER_H_
