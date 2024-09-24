
/*!
 *************************************************************************************
 * \file mb_access.h
 *
 * \brief
 *    Functions for macroblock neighborhoods
 *
 * \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>  
 *************************************************************************************
 */

#ifndef _MB_ACCESS_H_
#define _MB_ACCESS_H_

extern void CheckAvailabilityOfNeighbors(Macroblock *currMB);

/* MB Aff */
extern void getAffNeighbour         (const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix);
extern void getAffNeighbourLuma     (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getAffNeighbourXPLuma   (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getAffNeighbourPPLumaNB (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getAffNeighbourNPLuma   (const Macroblock *currMB, int yN, PixelPos *pix);
extern void getAffNeighbourN0Luma   (const Macroblock *currMB, PixelPos *pix);
extern void getAffNeighbourNXLuma   (const Macroblock *currMB, int xN, PixelPos *pix);
extern void getAffNeighbour0X       (const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix);
extern void getAffNeighbour0XLuma   (const Macroblock *currMB, int yN, PixelPos *pix);
extern void getAffNeighbour0N       (const Macroblock *currMB, const int mb_size[2], PixelPos *pix);
extern void getAffNeighbourX0       (const Macroblock *currMB, int xN, const int mb_size[2], PixelPos *pix);
extern void getAffNeighbourNX       (const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix);
extern void getAffNeighbourN0       (const Macroblock *currMB, const int mb_size[2], PixelPos *pix);
extern void getAffNeighbour0NLuma   (const Macroblock *currMB, PixelPos *pix);
extern void getAffNeighbourPXLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getAffNeighbourPXLumaNB_NoPos(const Macroblock *currMB, int yN, PixelPos *pix);
/* normal */
extern void getNonAffNeighbour      (const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbourXP_NoPos(const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbourPX_NoPos(const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbourLuma  (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getNonAffNeighbourXPLuma(const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getNonAffNeighbourPXLuma(const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getNonAffNeighbourN0Luma(const Macroblock *currMB, PixelPos *pix);
extern void getNonAffNeighbourNXLuma(const Macroblock *currMB, int yN, PixelPos *pix);
extern void getNonAffNeighbourN0    (const Macroblock *currMB, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbour0N    (const Macroblock *currMB, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbourNX    (const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbourNP    (const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbourNPChromaNB(const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbour0X    (const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbour0XLuma(const Macroblock *currMB, int yN, PixelPos *pix);
extern void getNonAffNeighbourX0    (const Macroblock *currMB, int xN, const int mb_size[2], PixelPos *pix);
extern void getNonAffNeighbour0NLuma(const Macroblock *currMB, PixelPos *pix);
extern void getNonAffNeighbourNPLumaNB(const Macroblock *currMB, int yN, PixelPos *pix);
extern void getNonAffNeighbourXPLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getNonAffNeighbourPPLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getNonAffNeighbourXPLumaNB_NoPos(const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getNonAffNeighbourPXLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix);
extern void getNonAffNeighbourPXLumaNB_NoPos(const Macroblock *currMB, int yN, PixelPos *pix);
extern void get4x4Neighbour         (const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix);
extern void get4x4NeighbourLuma     (const Macroblock *currMB, int block_x, int block_y, PixelPos *pix);
extern Boolean mb_is_available      (int mbAddr, const Macroblock *currMB);
extern void get_mb_pos              (VideoParameters *p_Vid, int mb_addr, const int mb_size[2], short *x, short *y);
extern void get_mb_block_pos_normal (const h264_pic_position *PicPos, int mb_addr, short *x, short *y);
extern void get_mb_block_pos_mbaff  (const h264_pic_position *PicPos, int mb_addr, short *x, short *y);


#endif
