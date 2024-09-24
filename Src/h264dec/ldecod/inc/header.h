/*!
 *************************************************************************************
 * \file header.h
 *
 * \brief
 *    Prototypes for header.c
 *************************************************************************************
 */

#ifndef _HEADER_H_
#define _HEADER_H_

extern void FirstPartOfSliceHeader(Slice *currSlice);
extern void RestOfSliceHeader     (Slice *currSlice);

extern void dec_ref_pic_marking(VideoParameters *p_Vid, Bitstream *currStream);

extern void decode_poc(VideoParameters *p_Vid);
extern int dumppoc(VideoParameters *p_Vid);

#endif

