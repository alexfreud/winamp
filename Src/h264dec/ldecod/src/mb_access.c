
/*!
*************************************************************************************
* \file mb_access.c
*
* \brief
*    Functions for macroblock neighborhoods
*
*  \author
*      Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Karsten Sühring          <suehring@hhi.de>
*************************************************************************************
*/

#include "global.h"
#include "mbuffer.h"
#include "mb_access.h"

/*!
************************************************************************
* \brief
*    returns 1 if the macroblock at the given address is available
************************************************************************
*/
Boolean mb_is_available(int mbAddr, const Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	if ((mbAddr < 0) || (mbAddr > ((int)p_Vid->dec_picture->PicSizeInMbs - 1)))
		return FALSE;

	// the following line checks both: slice number and if the mb has been decoded
	if (!p_Vid->DeblockCall)
	{
		if (p_Vid->mb_data[mbAddr].slice_nr != currMB->slice_nr)
			return FALSE;
	}

	return TRUE;
}


/*!
************************************************************************
* \brief
*    Checks the availability of neighboring macroblocks of
*    the current macroblock for prediction and context determination;
************************************************************************
*/
void CheckAvailabilityOfNeighbors(Macroblock *currMB)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	const int mb_nr = currMB->mbAddrX;

	// mark all neighbors as unavailable
	currMB->mb_up   = NULL;
	currMB->mb_left = NULL;

	if (p_Vid->dec_picture->mb_aff_frame_flag)
	{
		int cur_mb_pair = mb_nr >> 1;
		currMB->mb_addr_left = 2 * (cur_mb_pair - 1);
		currMB->mb_addr_up = 2 * (cur_mb_pair - p_Vid->dec_picture->PicWidthInMbs);
		currMB->mb_addr_upper_right = 2 * (cur_mb_pair - p_Vid->dec_picture->PicWidthInMbs + 1);
		currMB->mb_addr_upper_left = 2 * (cur_mb_pair - p_Vid->dec_picture->PicWidthInMbs - 1);

		currMB->mb_avail_left = (Boolean) (mb_is_available(currMB->mb_addr_left, currMB) && ((p_Vid->PicPos[cur_mb_pair    ][0])!=0));
		currMB->mb_avail_up = (Boolean) (mb_is_available(currMB->mb_addr_up, currMB));
		currMB->mb_avail_upper_right = (Boolean) (mb_is_available(currMB->mb_addr_upper_right, currMB) && ((p_Vid->PicPos[cur_mb_pair + 1][0])!=0));
		currMB->mb_avail_upper_left = (Boolean) (mb_is_available(currMB->mb_addr_upper_left, currMB) && ((p_Vid->PicPos[cur_mb_pair    ][0])!=0));
	}
	else
	{
		currMB->mb_addr_left = mb_nr - 1; // left?
		currMB->mb_addr_up = mb_nr - p_Vid->dec_picture->PicWidthInMbs; // up?
		currMB->mb_addr_upper_right = mb_nr - p_Vid->dec_picture->PicWidthInMbs + 1; // upper right?
		currMB->mb_addr_upper_left = mb_nr - p_Vid->dec_picture->PicWidthInMbs - 1; // upper left?

		currMB->mb_avail_left = (Boolean) (mb_is_available(currMB->mb_addr_left, currMB) && ((p_Vid->PicPos[mb_nr    ][0])!=0));
		currMB->mb_avail_up = (Boolean) (mb_is_available(currMB->mb_addr_up, currMB));
		currMB->mb_avail_upper_right = (Boolean) (mb_is_available(currMB->mb_addr_upper_right, currMB) && ((p_Vid->PicPos[mb_nr + 1][0])!=0));
		currMB->mb_avail_upper_left = (Boolean) (mb_is_available(currMB->mb_addr_upper_left, currMB) && ((p_Vid->PicPos[mb_nr    ][0])!=0));
	}

	if (currMB->mb_avail_left) currMB->mb_left = &(p_Vid->mb_data[currMB->mb_addr_left]);
	if (currMB->mb_avail_up) currMB->mb_up   = &(p_Vid->mb_data[currMB->mb_addr_up]);
}


/*!
************************************************************************
* \brief
*    returns the x and y macroblock coordinates for a given MbAddress
************************************************************************
*/
void get_mb_block_pos_normal (const h264_pic_position *PicPos, int mb_addr, short *x, short *y)
{
	*x = (short) PicPos[ mb_addr ][0];
	*y = (short) PicPos[ mb_addr ][1];
}

/*!
************************************************************************
* \brief
*    returns the x and y macroblock coordinates for a given MbAddress
*    for mbaff type slices
************************************************************************
*/
void get_mb_block_pos_mbaff (const h264_pic_position *PicPos, int mb_addr, short *x, short *y)
{
	*x = (short)  PicPos[mb_addr>>1][0];
	*y = (short) ((PicPos[mb_addr>>1][1] << 1) + (mb_addr & 0x01));
}

/*!
************************************************************************
* \brief
*    returns the x and y sample coordinates for a given MbAddress
************************************************************************
*/
void get_mb_pos (VideoParameters *p_Vid, int mb_addr, const int mb_size[2], short *x, short *y)
{
	p_Vid->get_mb_block_pos(p_Vid->PicPos, mb_addr, x, y);

	(*x) = (short) ((*x) * mb_size[0]);
	(*y) = (short) ((*y) * mb_size[1]);
}


/*!
************************************************************************
* \brief
*    get neighbouring positions for non-aff coding
* \param currMB
*   current macroblock
* \param xN
*    input x position
* \param yN
*    input y position
* \param mb_size
*    Macroblock size in pixel (according to luma or chroma MB access)
* \param pix
*    returns position informations
************************************************************************
*/
void getNonAffNeighbour(const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	if (xN < 0)
	{
		if (yN < 0)
		{
			pix->mb_addr   = currMB->mb_addr_upper_left;
			pix->available = currMB->mb_avail_upper_left;
		}
		else if (yN < maxH)
		{
			pix->mb_addr  = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (xN < maxW)
	{
		if (yN<0)
		{
			pix->mb_addr   = currMB->mb_addr_up;
			pix->available = currMB->mb_avail_up;
		}
		else if (yN < maxH)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (yN < 0)
	{
		pix->mb_addr   = currMB->mb_addr_upper_right;
		pix->available = currMB->mb_avail_upper_right;
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (xN & (maxW - 1));
		pix->pos_x = (short) (pix->x + *(CurPos++) * maxW);
		pix->y     = (short) (yN & (maxH - 1));    
		pix->pos_y = (short) (pix->y + *CurPos * maxH);
	}
}

void getNonAffNeighbourXP_NoPos(const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	if (xN < 0)
	{
		if (yN < maxH)
		{
			pix->mb_addr  = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (xN < maxW)
	{
		if (yN < maxH)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (xN & (maxW - 1));
		pix->y     = (short) (yN & (maxH - 1));    
	}
}

void getNonAffNeighbourPX_NoPos(const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	if (xN < maxW)
	{
		if (yN<0)
		{
			pix->mb_addr   = currMB->mb_addr_up;
			pix->available = currMB->mb_avail_up;
		}
		else if (yN < maxH)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (yN < 0)
	{
		pix->mb_addr   = currMB->mb_addr_upper_right;
		pix->available = currMB->mb_avail_upper_right;
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (xN & (maxW - 1));
		pix->y     = (short) (yN & (maxH - 1));    
	}
}

void getNonAffNeighbourLuma(const Macroblock *currMB, int xN, int yN, PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;

	if (xN < 0)
	{
		if (yN < 0)
		{
			pix->mb_addr   = currMB->mb_addr_upper_left;
			pix->available = currMB->mb_avail_upper_left;
		}
		else if (yN < 16)
		{
			pix->mb_addr  = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (xN < 16)
	{
		if (yN<0)
		{
			pix->mb_addr   = currMB->mb_addr_up;
			pix->available = currMB->mb_avail_up;
		}
		else if (yN < 16)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (yN < 0)
	{
		pix->mb_addr   = currMB->mb_addr_upper_right;
		pix->available = currMB->mb_avail_upper_right;
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (xN & 15);
		pix->pos_x = (short) (pix->x + *(CurPos++) * 16);
		pix->y     = (short) (yN & 15);    
		pix->pos_y = (short) (pix->y + *CurPos * 16);
	}
}

void getNonAffNeighbourXPLuma(const Macroblock *currMB, int xN, int yN, PixelPos *pix) // yN >= 0
{
	VideoParameters *p_Vid = currMB->p_Vid;

	if (xN < 0)
	{
		if (yN < 16)
		{
			pix->mb_addr  = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (xN < 16)
	{
		if (yN < 16)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (xN & 15);
		pix->pos_x = (short) (pix->x + *(CurPos++) * 16);
		pix->y     = (short) (yN & 15);    
		pix->pos_y = (short) (pix->y + *CurPos * 16);
	}
}


void getNonAffNeighbourXPLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix) // yN >= 0
{
	VideoParameters *p_Vid = currMB->p_Vid;
	assert(!p_Vid->DeblockCall);
	if (xN < 0)
	{
		pix->mb_addr  = currMB->mb_addr_left;
		pix->available = currMB->mb_avail_left;
	}
	else
	{
		pix->mb_addr   = currMB->mbAddrX;
		pix->available = TRUE;
	}

	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->pos_x = (short) ((xN & 15) + *(CurPos++) * 16);
		pix->pos_y = (short) (yN + *CurPos * 16);
	}
}

void getNonAffNeighbourPPLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix) // yN >= 0, xN >= 0
{
	VideoParameters *p_Vid = currMB->p_Vid;
	assert(!p_Vid->DeblockCall);
	pix->mb_addr   = currMB->mbAddrX;
	pix->available = TRUE;

	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->pos_x = (short) ((xN & 15) + *(CurPos++) * 16);
		pix->pos_y = (short) (yN + *CurPos * 16);
	}
}


void getNonAffNeighbourXPLumaNB_NoPos(const Macroblock *currMB, int xN, int yN, PixelPos *pix) // yN >= 0
{
	assert(!currMB->p_Vid->DeblockCall);
	if (xN < 0)
	{
		pix->mb_addr  = currMB->mb_addr_left;
		pix->available = currMB->mb_avail_left;
	}
	else
	{
		pix->mb_addr   = currMB->mbAddrX;
		pix->available = TRUE;
	}


	if (pix->available)
	{
		pix->x     = (short) (xN & 15);
		pix->y     = (short) (yN);
	}
}

void getNonAffNeighbourNPLumaNB(const Macroblock *currMB, int yN, PixelPos *pix) // xN = -1, yN >= 0 && yN < 16
{
	VideoParameters *p_Vid = currMB->p_Vid;

	pix->mb_addr  = currMB->mb_addr_left;
	pix->available = currMB->mb_avail_left;

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		//pix->x     = (short) (-1 & 15);
		pix->pos_x = (short) ((-1 & 15) + *(CurPos++) * 16);
		pix->y     = (short) (yN);    
		pix->pos_y = (short) (yN + *CurPos * 16);
	}
}


void getNonAffNeighbourPXLuma(const Macroblock *currMB, int xN, int yN, PixelPos *pix) // xN is >= 0
{
	VideoParameters *p_Vid = currMB->p_Vid;

	if (xN < 16)
	{
		if (yN<0)
		{
			pix->mb_addr   = currMB->mb_addr_up;
			pix->available = currMB->mb_avail_up;
		}
		else if (yN < 16)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (yN < 0)
	{
		pix->mb_addr   = currMB->mb_addr_upper_right;
		pix->available = currMB->mb_avail_upper_right;
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (xN & 15);
		pix->pos_x = (short) (pix->x + *(CurPos++) * 16);
		pix->y     = (short) (yN & 15);    
		pix->pos_y = (short) (pix->y + *CurPos * 16);
	}
}

void getNonAffNeighbourPXLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix) // xN is >= 0
{
	VideoParameters *p_Vid = currMB->p_Vid;
	assert(!p_Vid->DeblockCall);
	if (yN<0)
	{
		pix->mb_addr   = currMB->mb_addr_up;
		pix->available = currMB->mb_avail_up;
	}
	else 
	{
		pix->mb_addr   = currMB->mbAddrX;
		pix->available = TRUE;
	}

	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->pos_x = (short) (xN + *(CurPos++) * 16);
		pix->pos_y = (short) ((yN & 15) + *CurPos * 16);
	}
}

void getNonAffNeighbourPXLumaNB_NoPos(const Macroblock *currMB, int yN, PixelPos *pix) // xN is >= 0
{
	assert(!currMB->p_Vid->DeblockCall);
	if (yN<0)
	{
		pix->mb_addr   = currMB->mb_addr_up;
		pix->available = currMB->mb_avail_up;
	}
	else 
	{
		pix->mb_addr   = currMB->mbAddrX;
		pix->available = TRUE;
	}

	if (pix->available)
	{
		pix->y     = (short) (yN & 15);    
	}
}

void getNonAffNeighbourN0Luma(const Macroblock *currMB, PixelPos *pix) // xN = -1, yN = 0
{
	VideoParameters *p_Vid = currMB->p_Vid;
	assert(p_Vid->DeblockCall == 0);
	pix->mb_addr  = currMB->mb_addr_left;
	pix->available = currMB->mb_avail_left;

	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (-1 & 15);
		pix->pos_x = (short) (pix->x + *(CurPos++) * 16);
		pix->y     = 0;
		pix->pos_y = (short) (*CurPos * 16);
	}
}


void getNonAffNeighbourN0(const Macroblock *currMB, const int mb_size[2], PixelPos *pix) // xN = -1, yN = 0
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	assert(maxH != 0);
	assert(p_Vid->DeblockCall == 0);
	pix->mb_addr  = currMB->mb_addr_left;
	pix->available = currMB->mb_avail_left;

	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (-1 & (maxW - 1));
		pix->pos_x = (short) (pix->x + *(CurPos++) * maxW);
		pix->y     = 0;
		pix->pos_y = (short) (*CurPos * maxH);
	}
}

void getNonAffNeighbour0N(const Macroblock *currMB, const int mb_size[2], PixelPos *pix) // xN = 0, yN = -1
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	assert(maxW != 0);
	pix->mb_addr   = currMB->mb_addr_up;
	pix->available = currMB->mb_avail_up;

	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = 0;
		pix->pos_x = (short) (*(CurPos++) * maxW);
		pix->y     = (short) (-1 & (maxH - 1));    
		pix->pos_y = (short) (pix->y + *CurPos * maxH);
	}
}

void getNonAffNeighbour0NLuma(const Macroblock *currMB, PixelPos *pix) // xN = 0, yN = -1
{
	VideoParameters *p_Vid = currMB->p_Vid;

	pix->mb_addr   = currMB->mb_addr_up;
	pix->available = currMB->mb_avail_up;

	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = 0;
		pix->pos_x = (short) (*(CurPos++) * 16);
		pix->y     = (short) (-1 & (16 - 1));    
		pix->pos_y = (short) (pix->y + *CurPos * 16);
	}
}


void getNonAffNeighbourNX(const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix) // xN = -1, yN full range
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];


	if (yN < 0)
	{
		pix->mb_addr   = currMB->mb_addr_upper_left;
		pix->available = currMB->mb_avail_upper_left;
	}
	else if (yN < maxH)
	{
		pix->mb_addr  = currMB->mb_addr_left;
		pix->available = currMB->mb_avail_left;
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (-1 & (maxW - 1));
		pix->pos_x = (short) (pix->x + *(CurPos++) * maxW);
		pix->y     = (short) (yN & (maxH - 1));    
		pix->pos_y = (short) (pix->y + *CurPos * maxH);
	}
}

void getNonAffNeighbourNXLuma(const Macroblock *currMB, int yN, PixelPos *pix) // xN = -1, yN full range
{
	VideoParameters *p_Vid = currMB->p_Vid;


	if (yN < 0)
	{
		pix->mb_addr   = currMB->mb_addr_upper_left;
		pix->available = currMB->mb_avail_upper_left;
	}
	else if (yN < 16)
	{
		pix->mb_addr  = currMB->mb_addr_left;
		pix->available = currMB->mb_avail_left;
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (-1 & (16 - 1));
		pix->pos_x = (short) (pix->x + *(CurPos++) * 16);
		pix->y     = (short) (yN & (16 - 1));    
		pix->pos_y = (short) (pix->y + *CurPos * 16);
	}
}

void getNonAffNeighbourNP(const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix) // xN < 0, yN >= 0
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	if (yN < maxH)
	{
		pix->mb_addr  = currMB->mb_addr_left;
		pix->available = currMB->mb_avail_left;
		if (pix->available)
		{
			const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
			pix->pos_x = (short) ((-1 & (maxW - 1)) + *(CurPos++) * maxW);
			pix->pos_y = (short) (yN + *CurPos * maxH);
		}
	}
	else
	{
		pix->available = FALSE;
	}
}

void getNonAffNeighbourNPChromaNB(const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix) // xN < 0, yN >= 0
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	pix->mb_addr  = currMB->mb_addr_left;
	pix->available = currMB->mb_avail_left;
	if (pix->available)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->pos_x = (short) ((-1 & (maxW - 1)) + *(CurPos++) * maxW);
		pix->pos_y = (short) (yN + *CurPos * maxH);
	}
}

void getNonAffNeighbour0X(const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix) // xN is guaranteed to be zero
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	if (0 < maxW)
	{
		if (yN<0)
		{
			pix->mb_addr   = currMB->mb_addr_up;
			pix->available = currMB->mb_avail_up;
		}
		else if (yN < maxH)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (yN < 0)
	{
		pix->mb_addr   = currMB->mb_addr_upper_right;
		pix->available = currMB->mb_avail_upper_right;
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = 0;
		pix->pos_x = (short) (*(CurPos++) * maxW);
		pix->y     = (short) (yN & (maxH - 1));    
		pix->pos_y = (short) (pix->y + *CurPos * maxH);
	}
}

void getNonAffNeighbour0XLuma(const Macroblock *currMB, int yN, PixelPos *pix) // xN is guaranteed to be zero
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = 16, maxH = 16;

	if (yN<0)
	{
		pix->mb_addr   = currMB->mb_addr_up;
		pix->available = currMB->mb_avail_up;
	}
	else if (yN < 16)
	{
		pix->mb_addr   = currMB->mbAddrX;
		pix->available = TRUE;
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = 0;
		pix->pos_x = (short) (*(CurPos++) * maxW);
		pix->y     = (short) (yN & (maxH - 1));    
		pix->pos_y = (short) (pix->y + *CurPos * maxH);
	}
}

void getNonAffNeighbourX0(const Macroblock *currMB, int xN, const int mb_size[2], PixelPos *pix) // xN is full range, yN is 0
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW = mb_size[0], maxH = mb_size[1];

	if (xN < 0)
	{
		if (0 < maxH)
		{
			pix->mb_addr  = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else if (xN < maxW)
	{
		if (0 < maxH)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
		}
		else
		{
			pix->available = FALSE;
		}
	}
	else
	{
		pix->available = FALSE;
	}

	if (pix->available || p_Vid->DeblockCall && pix->mb_addr && p_Vid)
	{
		const int *CurPos = &p_Vid->PicPos[ pix->mb_addr ][0];
		pix->x     = (short) (xN & (maxW - 1));
		pix->pos_x = (short) (pix->x + *(CurPos++) * maxW);
		pix->y     = 0;
		pix->pos_y = (short) (*CurPos * maxH);
	}
}

/*!
************************************************************************
* \brief
*    get neighboring positions for aff coding
* \param currMB
*   current macroblock
* \param xN
*    input x position
* \param yN
*    input y position
* \param mb_size
*    Macroblock size in pixel (according to luma or chroma MB access)
* \param pix
*    returns position informations
************************************************************************
*/
void getAffNeighbour(const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW, maxH;
	int yM = -1;

	maxW = mb_size[0];
	maxH = mb_size[1];

	// initialize to "not available"
	pix->available = FALSE;

	if(yN > (maxH - 1))
	{
		return;
	}
	if (xN > (maxW - 1) && yN >= 0 && yN < maxH)
	{
		return;
	}

	if (xN < 0)
	{
		if (yN < 0)
		{
			if(!currMB->mb_field)
			{
				// frame
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_left  + 1;
					pix->available = currMB->mb_avail_upper_left;
					yM = yN;
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_left;
					pix->available = currMB->mb_avail_left;
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							yM = yN;
						}
						else
						{
							(pix->mb_addr)++;
							yM = (yN + maxH) >> 1;
						}
					}
				}
			}
			else
			{
				// field
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_left;
					pix->available = currMB->mb_avail_upper_left;
					if (currMB->mb_avail_upper_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_upper_left].mb_field)
						{
							(pix->mb_addr)++;
							yM = 2 * yN;
						}
						else
						{
							yM = yN;
						}
					}
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_upper_left+1;
					pix->available = currMB->mb_avail_upper_left;
					yM = yN;
				}
			}
		}
		else
		{ // xN < 0 && yN >= 0
			if (yN <maxH)
			{
				if (!currMB->mb_field)
				{
					// frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								yM = yN;
							}
							else
							{
								(pix->mb_addr)+= ((yN & 0x01) != 0);
								yM = yN >> 1;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								(pix->mb_addr)++;
								yM = yN;
							}
							else
							{
								(pix->mb_addr)+= ((yN & 0x01) != 0);
								yM = (yN + maxH) >> 1;
							}
						}
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr  = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								if (yN < (maxH >> 1))
								{
									yM = yN << 1;
								}
								else
								{
									(pix->mb_addr)++;
									yM = (yN << 1 ) - maxH;
								}
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr  = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								if (yN < (maxH >> 1))
								{
									yM = (yN << 1) + 1;
								}
								else
								{
									(pix->mb_addr)++;
									yM = (yN << 1 ) + 1 - maxH;
								}
							}
							else
							{
								(pix->mb_addr)++;
								yM = yN;
							}
						}
					}
				}
			}
		}
	}
	else
	{ // xN >= 0
		if (xN >= 0 && xN < maxW)
		{
			if (yN<0)
			{
				if (!currMB->mb_field)
				{
					//frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						//top
						pix->mb_addr  = currMB->mb_addr_up;
						// for the deblocker if the current MB is a frame and the one above is a field
						// then the neighbor is the top MB of the pair
						if (currMB->mb_avail_up)
						{
							if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
								pix->mb_addr  += 1;
						}

						pix->available = currMB->mb_avail_up;
						yM = yN;
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mbAddrX - 1;
						pix->available = TRUE;
						yM = yN;
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_up;
						pix->available = currMB->mb_avail_up;
						if (currMB->mb_avail_up)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
							{
								(pix->mb_addr)++;
								yM = 2* yN;
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_up + 1;
						pix->available = currMB->mb_avail_up;
						yM = yN;
					}
				}
			}
			else
			{
				// yN >=0
				// for the deblocker if this is the extra edge then do this special stuff
				if (yN == 0 && p_Vid->DeblockCall == 2)
				{
					pix->mb_addr  = currMB->mb_addr_up + 1;
					pix->available = TRUE;
					yM = yN - 1;
				}

				else if ((yN <maxH))
				{
					pix->mb_addr   = currMB->mbAddrX;
					pix->available = TRUE;
					yM = yN;
				}
			}
		}
		else
		{ // xN >= maxW
			if(yN < 0)
			{
				if (!currMB->mb_field)
				{
					// frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr  = currMB->mb_addr_upper_right + 1;
						pix->available = currMB->mb_avail_upper_right;
						yM = yN;
					}
					else
					{
						// bottom
						pix->available = FALSE;
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_upper_right;
						pix->available = currMB->mb_avail_upper_right;
						if (currMB->mb_avail_upper_right)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_upper_right].mb_field)
							{
								(pix->mb_addr)++;
								yM = 2* yN;
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_upper_right + 1;
						pix->available = currMB->mb_avail_upper_right;
						yM = yN;
					}
				}
			}
		}
	}
	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (xN & (maxW - 1));
		pix->y = (short) (yM & (maxH - 1));
		get_mb_pos(p_Vid, pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = pix->pos_x + pix->x;
		pix->pos_y = pix->pos_y + pix->y;
	}
}

void getAffNeighbourNX(const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW, maxH;
	int yM = -1;
	int xN = -1;

	maxW = mb_size[0];
	maxH = mb_size[1];

	// initialize to "not available"
	pix->available = FALSE;

	if(yN > (maxH - 1))
	{
		return;
	}
	if (xN > (maxW - 1) && yN >= 0 && yN < maxH)
	{
		return;
	}

	if (xN < 0)
	{
		if (yN < 0)
		{
			if(!currMB->mb_field)
			{
				// frame
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_left  + 1;
					pix->available = currMB->mb_avail_upper_left;
					yM = yN;
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_left;
					pix->available = currMB->mb_avail_left;
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							yM = yN;
						}
						else
						{
							(pix->mb_addr)++;
							yM = (yN + maxH) >> 1;
						}
					}
				}
			}
			else
			{
				// field
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_left;
					pix->available = currMB->mb_avail_upper_left;
					if (currMB->mb_avail_upper_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_upper_left].mb_field)
						{
							(pix->mb_addr)++;
							yM = 2 * yN;
						}
						else
						{
							yM = yN;
						}
					}
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_upper_left+1;
					pix->available = currMB->mb_avail_upper_left;
					yM = yN;
				}
			}
		}
		else
		{ // xN < 0 && yN >= 0
			if (yN <maxH)
			{
				if (!currMB->mb_field)
				{
					// frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								yM = yN;
							}
							else
							{
								(pix->mb_addr)+= ((yN & 0x01) != 0);
								yM = yN >> 1;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								(pix->mb_addr)++;
								yM = yN;
							}
							else
							{
								(pix->mb_addr)+= ((yN & 0x01) != 0);
								yM = (yN + maxH) >> 1;
							}
						}
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr  = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								if (yN < (maxH >> 1))
								{
									yM = yN << 1;
								}
								else
								{
									(pix->mb_addr)++;
									yM = (yN << 1 ) - maxH;
								}
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr  = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								if (yN < (maxH >> 1))
								{
									yM = (yN << 1) + 1;
								}
								else
								{
									(pix->mb_addr)++;
									yM = (yN << 1 ) + 1 - maxH;
								}
							}
							else
							{
								(pix->mb_addr)++;
								yM = yN;
							}
						}
					}
				}
			}
		}
	}
	else
	{ // xN >= 0
		if (xN >= 0 && xN < maxW)
		{
			if (yN<0)
			{
				if (!currMB->mb_field)
				{
					//frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						//top
						pix->mb_addr  = currMB->mb_addr_up;
						// for the deblocker if the current MB is a frame and the one above is a field
						// then the neighbor is the top MB of the pair
						if (currMB->mb_avail_up)
						{
							if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
								pix->mb_addr  += 1;
						}

						pix->available = currMB->mb_avail_up;
						yM = yN;
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mbAddrX - 1;
						pix->available = TRUE;
						yM = yN;
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_up;
						pix->available = currMB->mb_avail_up;
						if (currMB->mb_avail_up)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
							{
								(pix->mb_addr)++;
								yM = 2* yN;
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_up + 1;
						pix->available = currMB->mb_avail_up;
						yM = yN;
					}
				}
			}
			else
			{
				// yN >=0
				// for the deblocker if this is the extra edge then do this special stuff
				if (yN == 0 && p_Vid->DeblockCall == 2)
				{
					pix->mb_addr  = currMB->mb_addr_up + 1;
					pix->available = TRUE;
					yM = yN - 1;
				}

				else if ((yN <maxH))
				{
					pix->mb_addr   = currMB->mbAddrX;
					pix->available = TRUE;
					yM = yN;
				}
			}
		}
		else
		{ // xN >= maxW
			if(yN < 0)
			{
				if (!currMB->mb_field)
				{
					// frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr  = currMB->mb_addr_upper_right + 1;
						pix->available = currMB->mb_avail_upper_right;
						yM = yN;
					}
					else
					{
						// bottom
						pix->available = FALSE;
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_upper_right;
						pix->available = currMB->mb_avail_upper_right;
						if (currMB->mb_avail_upper_right)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_upper_right].mb_field)
							{
								(pix->mb_addr)++;
								yM = 2* yN;
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_upper_right + 1;
						pix->available = currMB->mb_avail_upper_right;
						yM = yN;
					}
				}
			}
		}
	}
	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (xN & (maxW - 1));
		pix->y = (short) (yM & (maxH - 1));
		get_mb_pos(p_Vid, pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = pix->pos_x + pix->x;
		pix->pos_y = pix->pos_y + pix->y;
	}
}

void getAffNeighbourNXLuma(const Macroblock *currMB, int yN, PixelPos *pix)
{
	const int mb_size[2]={16,16};
	getAffNeighbourNX(currMB, yN, mb_size, pix);
}
void getAffNeighbourN0(const Macroblock *currMB, const int mb_size[2], PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW, maxH;
	int yM = -1;
	int xN = -1;
	int yN=0;

	maxW = mb_size[0];
	maxH = mb_size[1];

	// initialize to "not available"
	pix->available = FALSE;

	if(yN > (maxH - 1))
	{
		return;
	}
	if (xN > (maxW - 1) && yN >= 0 && yN < maxH)
	{
		return;
	}

	if (xN < 0)
	{
		if (yN < 0)
		{
			if(!currMB->mb_field)
			{
				// frame
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_left  + 1;
					pix->available = currMB->mb_avail_upper_left;
					yM = yN;
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_left;
					pix->available = currMB->mb_avail_left;
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							yM = yN;
						}
						else
						{
							(pix->mb_addr)++;
							yM = (yN + maxH) >> 1;
						}
					}
				}
			}
			else
			{
				// field
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_left;
					pix->available = currMB->mb_avail_upper_left;
					if (currMB->mb_avail_upper_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_upper_left].mb_field)
						{
							(pix->mb_addr)++;
							yM = 2 * yN;
						}
						else
						{
							yM = yN;
						}
					}
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_upper_left+1;
					pix->available = currMB->mb_avail_upper_left;
					yM = yN;
				}
			}
		}
		else
		{ // xN < 0 && yN >= 0
			if (yN <maxH)
			{
				if (!currMB->mb_field)
				{
					// frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								yM = yN;
							}
							else
							{
								(pix->mb_addr)+= ((yN & 0x01) != 0);
								yM = yN >> 1;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								(pix->mb_addr)++;
								yM = yN;
							}
							else
							{
								(pix->mb_addr)+= ((yN & 0x01) != 0);
								yM = (yN + maxH) >> 1;
							}
						}
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr  = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								if (yN < (maxH >> 1))
								{
									yM = yN << 1;
								}
								else
								{
									(pix->mb_addr)++;
									yM = (yN << 1 ) - maxH;
								}
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr  = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								if (yN < (maxH >> 1))
								{
									yM = (yN << 1) + 1;
								}
								else
								{
									(pix->mb_addr)++;
									yM = (yN << 1 ) + 1 - maxH;
								}
							}
							else
							{
								(pix->mb_addr)++;
								yM = yN;
							}
						}
					}
				}
			}
		}
	}
	else
	{ // xN >= 0
		if (xN >= 0 && xN < maxW)
		{
			if (yN<0)
			{
				if (!currMB->mb_field)
				{
					//frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						//top
						pix->mb_addr  = currMB->mb_addr_up;
						// for the deblocker if the current MB is a frame and the one above is a field
						// then the neighbor is the top MB of the pair
						if (currMB->mb_avail_up)
						{
							if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
								pix->mb_addr  += 1;
						}

						pix->available = currMB->mb_avail_up;
						yM = yN;
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mbAddrX - 1;
						pix->available = TRUE;
						yM = yN;
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_up;
						pix->available = currMB->mb_avail_up;
						if (currMB->mb_avail_up)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
							{
								(pix->mb_addr)++;
								yM = 2* yN;
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_up + 1;
						pix->available = currMB->mb_avail_up;
						yM = yN;
					}
				}
			}
			else
			{
				// yN >=0
				// for the deblocker if this is the extra edge then do this special stuff
				if (yN == 0 && p_Vid->DeblockCall == 2)
				{
					pix->mb_addr  = currMB->mb_addr_up + 1;
					pix->available = TRUE;
					yM = yN - 1;
				}

				else if ((yN <maxH))
				{
					pix->mb_addr   = currMB->mbAddrX;
					pix->available = TRUE;
					yM = yN;
				}
			}
		}
		else
		{ // xN >= maxW
			if(yN < 0)
			{
				if (!currMB->mb_field)
				{
					// frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr  = currMB->mb_addr_upper_right + 1;
						pix->available = currMB->mb_avail_upper_right;
						yM = yN;
					}
					else
					{
						// bottom
						pix->available = FALSE;
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_upper_right;
						pix->available = currMB->mb_avail_upper_right;
						if (currMB->mb_avail_upper_right)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_upper_right].mb_field)
							{
								(pix->mb_addr)++;
								yM = 2* yN;
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_upper_right + 1;
						pix->available = currMB->mb_avail_upper_right;
						yM = yN;
					}
				}
			}
		}
	}
	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (xN & (maxW - 1));
		pix->y = (short) (yM & (maxH - 1));
		get_mb_pos(p_Vid, pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = pix->pos_x + pix->x;
		pix->pos_y = pix->pos_y + pix->y;
	}
}


void getAffNeighbourLuma(const Macroblock *currMB, int xN, int yN, PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	const int maxW=16, maxH=16;
	int yM = -1;

	// initialize to "not available"
	pix->available = FALSE;

	if(yN > (maxH - 1))
	{
		return;
	}
	if (xN > (maxW - 1) && yN >= 0 && yN < maxH)
	{
		return;
	}

	if (xN < 0)
	{
		if (yN < 0)
		{
			if(!currMB->mb_field)
			{
				// frame
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_left  + 1;
					pix->available = currMB->mb_avail_upper_left;
					yM = yN;
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_left;
					pix->available = currMB->mb_avail_left;
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							yM = yN;
						}
						else
						{
							(pix->mb_addr)++;
							yM = (yN + maxH) >> 1;
						}
					}
				}
			}
			else
			{
				// field
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_left;
					pix->available = currMB->mb_avail_upper_left;
					if (currMB->mb_avail_upper_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_upper_left].mb_field)
						{
							(pix->mb_addr)++;
							yM = 2 * yN;
						}
						else
						{
							yM = yN;
						}
					}
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_upper_left+1;
					pix->available = currMB->mb_avail_upper_left;
					yM = yN;
				}
			}
		}
		else
		{ // xN < 0 && yN >= 0
			if (yN <maxH)
			{
				if (!currMB->mb_field)
				{
					// frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								yM = yN;
							}
							else
							{
								(pix->mb_addr)+= ((yN & 0x01) != 0);
								yM = yN >> 1;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								(pix->mb_addr)++;
								yM = yN;
							}
							else
							{
								(pix->mb_addr)+= ((yN & 0x01) != 0);
								yM = (yN + maxH) >> 1;
							}
						}
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr  = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								if (yN < (maxH >> 1))
								{
									yM = yN << 1;
								}
								else
								{
									(pix->mb_addr)++;
									yM = (yN << 1 ) - maxH;
								}
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr  = currMB->mb_addr_left;
						pix->available = currMB->mb_avail_left;
						if (currMB->mb_avail_left)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
							{
								if (yN < (maxH >> 1))
								{
									yM = (yN << 1) + 1;
								}
								else
								{
									(pix->mb_addr)++;
									yM = (yN << 1 ) + 1 - maxH;
								}
							}
							else
							{
								(pix->mb_addr)++;
								yM = yN;
							}
						}
					}
				}
			}
		}
	}
	else
	{ // xN >= 0
		if (xN >= 0 && xN < maxW)
		{
			if (yN<0)
			{
				if (!currMB->mb_field)
				{
					//frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						//top
						pix->mb_addr  = currMB->mb_addr_up;
						// for the deblocker if the current MB is a frame and the one above is a field
						// then the neighbor is the top MB of the pair
						if (currMB->mb_avail_up)
						{
							if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
								pix->mb_addr  += 1;
						}

						pix->available = currMB->mb_avail_up;
						yM = yN;
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mbAddrX - 1;
						pix->available = TRUE;
						yM = yN;
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_up;
						pix->available = currMB->mb_avail_up;
						if (currMB->mb_avail_up)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
							{
								(pix->mb_addr)++;
								yM = 2* yN;
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_up + 1;
						pix->available = currMB->mb_avail_up;
						yM = yN;
					}
				}
			}
			else
			{
				// yN >=0
				// for the deblocker if this is the extra edge then do this special stuff
				if (yN == 0 && p_Vid->DeblockCall == 2)
				{
					pix->mb_addr  = currMB->mb_addr_up + 1;
					pix->available = TRUE;
					yM = yN - 1;
				}

				else if (yN <maxH)
				{
					pix->mb_addr   = currMB->mbAddrX;
					pix->available = TRUE;
					yM = yN;
				}
			}
		}
		else
		{ // xN >= maxW
			if(yN < 0)
			{
				if (!currMB->mb_field)
				{
					// frame
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr  = currMB->mb_addr_upper_right + 1;
						pix->available = currMB->mb_avail_upper_right;
						yM = yN;
					}
					else
					{
						// bottom
						pix->available = FALSE;
					}
				}
				else
				{
					// field
					if ((currMB->mbAddrX & 0x01) == 0)
					{
						// top
						pix->mb_addr   = currMB->mb_addr_upper_right;
						pix->available = currMB->mb_avail_upper_right;
						if (currMB->mb_avail_upper_right)
						{
							if(!p_Vid->mb_data[currMB->mb_addr_upper_right].mb_field)
							{
								(pix->mb_addr)++;
								yM = 2* yN;
							}
							else
							{
								yM = yN;
							}
						}
					}
					else
					{
						// bottom
						pix->mb_addr   = currMB->mb_addr_upper_right + 1;
						pix->available = currMB->mb_avail_upper_right;
						yM = yN;
					}
				}
			}
		}
	}
	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (xN & (maxW - 1));
		pix->y = (short) (yM & (maxH - 1));
		get_mb_block_pos_mbaff(p_Vid->PicPos, pix->mb_addr, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = 16*pix->pos_x + pix->x;
		pix->pos_y = 16*pix->pos_y + pix->y;
	}
}


void getAffNeighbourPXLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix)
{ // xN >= 0, yN < 16, xN < 16
	VideoParameters *p_Vid = currMB->p_Vid;
	const int maxW=16, maxH=16;
	int yM = -1;

	// initialize to "not available"
	pix->available = FALSE;

	if (yN<0)
	{
		if (!currMB->mb_field)
		{
			//frame
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				//top
				pix->mb_addr  = currMB->mb_addr_up;
				// for the deblocker if the current MB is a frame and the one above is a field
				// then the neighbor is the top MB of the pair
				if (currMB->mb_avail_up)
				{
					if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
						pix->mb_addr  += 1;
				}

				pix->available = currMB->mb_avail_up;
				yM = yN;
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mbAddrX - 1;
				pix->available = TRUE;
				yM = yN;
			}
		}
		else
		{
			// field
			pix->available = currMB->mb_avail_up;
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				// top
				pix->mb_addr   = currMB->mb_addr_up;

				if (currMB->mb_avail_up)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
					{
						(pix->mb_addr)++;
						yM = 2* yN;
					}
					else
					{
						yM = yN;
					}
				}
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mb_addr_up + 1;

				yM = yN;
			}
		}
	}
	else
	{
		// yN >=0
		// for the deblocker if this is the extra edge then do this special stuff
		if (yN == 0 && p_Vid->DeblockCall == 2)
		{
			pix->mb_addr  = currMB->mb_addr_up + 1;
			pix->available = TRUE;
			yM = yN - 1;
		}
		else
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
			yM = yN;
		}
	}

	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (xN);
		pix->y = (short) (yM & (maxH - 1));
		get_mb_block_pos_mbaff(p_Vid->PicPos, pix->mb_addr, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = 16*pix->pos_x + pix->x;
		pix->pos_y = 16*pix->pos_y + pix->y;
	}
}

void getAffNeighbourPXLumaNB_NoPos(const Macroblock *currMB, int yN, PixelPos *pix)
{ // xN >= 0, yN < 16, xN < 16, DeblockCall == 0
	VideoParameters *p_Vid = currMB->p_Vid;
	int yM = -1;

	// initialize to "not available"
	pix->available = FALSE;

	if (yN<0)
	{
		if (!currMB->mb_field)
		{
			//frame
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				//top
				pix->mb_addr  = currMB->mb_addr_up;
				// for the deblocker if the current MB is a frame and the one above is a field
				// then the neighbor is the top MB of the pair
				if (currMB->mb_avail_up)
				{
					pix->mb_addr  += 1;
				}

				pix->available = currMB->mb_avail_up;
				yM = yN;
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mbAddrX - 1;
				pix->available = TRUE;
				yM = yN;
			}
		}
		else
		{
			// field
			pix->available = currMB->mb_avail_up;
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				// top
				pix->mb_addr   = currMB->mb_addr_up;

				if (currMB->mb_avail_up)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
					{
						(pix->mb_addr)++;
						yM = 2* yN;
					}
					else
					{
						yM = yN;
					}
				}
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mb_addr_up + 1;

				yM = yN;
			}
		}
	}
	else
	{
		// yN >=0
		pix->mb_addr   = currMB->mbAddrX;
		pix->available = TRUE;
		yM = yN;
	}

	if (pix->available)
	{
		pix->y = (short) (yM & 15);
	}
}


void getAffNeighbourXPLuma(const Macroblock *currMB, int xN, int yN, PixelPos *pix)
{ // yN >= 0
	VideoParameters *p_Vid = currMB->p_Vid;
	const int maxW=16, maxH=16;
	int yM = -1;

	// initialize to "not available"
	pix->available = FALSE;

	if(yN > (maxH - 1))
	{
		return;
	}
	if (xN > (maxW - 1)  && yN < maxH)
	{
		return;
	}

	if (xN < 0)
	{
			if (!currMB->mb_field)
			{
				// frame
				pix->mb_addr   = currMB->mb_addr_left;
				pix->available = currMB->mb_avail_left;
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							yM = yN;
						}
						else
						{
							(pix->mb_addr)+= ((yN & 0x01) != 0);
							yM = yN >> 1;
						}
					}
				}
				else
				{
					// bottom
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							(pix->mb_addr)++;
							yM = yN;
						}
						else
						{
							(pix->mb_addr)+= ((yN & 0x01) != 0);
							yM = (yN + maxH) >> 1;
						}
					}
				}
			}
			else
			{
				pix->mb_addr  = currMB->mb_addr_left;
				pix->available = currMB->mb_avail_left;
				// field
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							if (yN < (maxH >> 1))
							{
								yM = yN << 1;
							}
							else
							{
								(pix->mb_addr)++;
								yM = (yN << 1 ) - maxH;
							}
						}
						else
						{
							yM = yN;
						}
					}
				}
				else
				{
					// bottom
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							if (yN < (maxH >> 1))
							{
								yM = (yN << 1) + 1;
							}
							else
							{
								(pix->mb_addr)++;
								yM = (yN << 1 ) + 1 - maxH;
							}
						}
						else
						{
							(pix->mb_addr)++;
							yM = yN;
						}
					}
				}
			}
	}
	else if (xN < maxW)
	{ // xN >= 0
		// yN >=0
		// for the deblocker if this is the extra edge then do this special stuff
		if (yN == 0 && p_Vid->DeblockCall == 2)
		{
			pix->mb_addr  = currMB->mb_addr_up + 1;
			pix->available = TRUE;
			yM = yN - 1;
		}

		else if (yN <maxH)
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
			yM = yN;
		}
	}

	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (xN & (maxW - 1));
		pix->y = (short) (yM & (maxH - 1));
		get_mb_block_pos_mbaff(p_Vid->PicPos, pix->mb_addr, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = 16*pix->pos_x + pix->x;
		pix->pos_y = 16*pix->pos_y + pix->y;
	}
}


void getAffNeighbourPPLumaNB(const Macroblock *currMB, int xN, int yN, PixelPos *pix)
{ 
	VideoParameters *p_Vid = currMB->p_Vid;

	// xN >= 0
	// yN >=0
	pix->mb_addr   = currMB->mbAddrX;
	pix->available = TRUE;

	pix->x = (short) (xN & (16 - 1));
	pix->y = (short) (yN & (16 - 1));
	get_mb_block_pos_mbaff(p_Vid->PicPos, pix->mb_addr, &(pix->pos_x), &(pix->pos_y));
	pix->pos_x = 16*pix->pos_x + pix->x;
	pix->pos_y = 16*pix->pos_y + pix->y;
}

void getAffNeighbourNPLuma(const Macroblock *currMB, int yN, PixelPos *pix)
{ // yN >= 0
	VideoParameters *p_Vid = currMB->p_Vid;
	const int maxW=16, maxH=16;
	int yM = -1;

	// initialize to "not available"
	pix->available = FALSE;

	if(yN > (maxH - 1))
	{
		return;
	}

	if (yN <maxH)
	{
		if (!currMB->mb_field)
		{
			// frame
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				// top
				pix->mb_addr   = currMB->mb_addr_left;
				pix->available = currMB->mb_avail_left;
				if (currMB->mb_avail_left)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
					{
						yM = yN;
					}
					else
					{
						(pix->mb_addr)+= ((yN & 0x01) != 0);
						yM = yN >> 1;
					}
				}
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mb_addr_left;
				pix->available = currMB->mb_avail_left;
				if (currMB->mb_avail_left)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
					{
						(pix->mb_addr)++;
						yM = yN;
					}
					else
					{
						(pix->mb_addr)+= ((yN & 0x01) != 0);
						yM = (yN + maxH) >> 1;
					}
				}
			}
		}
		else
		{
			// field
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				// top
				pix->mb_addr  = currMB->mb_addr_left;
				pix->available = currMB->mb_avail_left;
				if (currMB->mb_avail_left)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
					{
						if (yN < (maxH >> 1))
						{
							yM = yN << 1;
						}
						else
						{
							(pix->mb_addr)++;
							yM = (yN << 1 ) - maxH;
						}
					}
					else
					{
						yM = yN;
					}
				}
			}
			else
			{
				// bottom
				pix->mb_addr  = currMB->mb_addr_left;
				pix->available = currMB->mb_avail_left;
				if (currMB->mb_avail_left)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
					{
						if (yN < (maxH >> 1))
						{
							yM = (yN << 1) + 1;
						}
						else
						{
							(pix->mb_addr)++;
							yM = (yN << 1 ) + 1 - maxH;
						}
					}
					else
					{
						(pix->mb_addr)++;
						yM = yN;
					}
				}
			}
		}
	}


	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (-1 & (maxW - 1));
		pix->y = (short) (yM & (maxH - 1));
		get_mb_block_pos_mbaff(p_Vid->PicPos, pix->mb_addr, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = 16*pix->pos_x + pix->x;
		pix->pos_y = 16*pix->pos_y + pix->y;
	}
}

void getAffNeighbourN0Luma(const Macroblock *currMB, PixelPos *pix)
{ // xN = -1 && yN == 0
	VideoParameters *p_Vid = currMB->p_Vid;
	//const int maxW=16, maxH=16;
	int yM = -1;


	// initialize to "not available"
	pix->available = FALSE;

	if (!currMB->mb_field)
	{
		// frame
		if ((currMB->mbAddrX & 0x01) == 0)
		{
			// top
			pix->mb_addr   = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
			if (currMB->mb_avail_left)
			{
				yM = 0;
			}
		}
		else
		{
			// bottom
			pix->mb_addr   = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
			if (currMB->mb_avail_left)
			{
				if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
				{
					(pix->mb_addr)++;
					yM = 0;
				}
				else
				{
					yM = 8;
				}
			}
		}
	}
	else
	{
		// field
		if ((currMB->mbAddrX & 0x01) == 0)
		{
			// top
			pix->mb_addr  = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
			if (currMB->mb_avail_left)
			{
				yM = 0;
			}
		}
		else
		{
			// bottom
			pix->mb_addr  = currMB->mb_addr_left;
			pix->available = currMB->mb_avail_left;
			if (currMB->mb_avail_left)
			{
				if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
				{
					yM = 1;
				}
				else
				{
					(pix->mb_addr)++;
					yM = 0;
				}
			}
		}
	}


	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (-1 & 15);
		pix->y = (short) (yM & 15);
		get_mb_block_pos_mbaff(p_Vid->PicPos, pix->mb_addr, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = 16*pix->pos_x + pix->x;
		pix->pos_y = 16*pix->pos_y + pix->y;
	}
}

void getAffNeighbourX0(const Macroblock *currMB, int xN, const int mb_size[2], PixelPos *pix)
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW, maxH;
	int yM = -1;

	maxW = mb_size[0];
	maxH = mb_size[1];

	// initialize to "not available"
	pix->available = FALSE;

	if(0 > (maxH - 1))
	{
		return;
	}
	if (xN > (maxW - 1) && 0 < maxH)
	{
		return;
	}

	if (xN < 0)
	{
		if (0 <maxH)
		{
			if (!currMB->mb_field)
			{
				// frame
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_left;
					pix->available = currMB->mb_avail_left;
					if (currMB->mb_avail_left)
					{
						yM = 0;
					}
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_left;
					pix->available = currMB->mb_avail_left;
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							(pix->mb_addr)++;
							yM = 0;
						}
						else
						{
							yM = (0 + maxH) >> 1;
						}
					}
				}
			}
			else
			{
				// field
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr  = currMB->mb_addr_left;
					pix->available = currMB->mb_avail_left;
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							if (0 < (maxH >> 1))
							{
								yM = 0;
							}
							else
							{
								(pix->mb_addr)++;
								yM = (0) - maxH;
							}
						}
						else
						{
							yM = 0;
						}
					}
				}
				else
				{
					// bottom
					pix->mb_addr  = currMB->mb_addr_left;
					pix->available = currMB->mb_avail_left;
					if (currMB->mb_avail_left)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_left].mb_field)
						{
							if (0 < (maxH >> 1))
							{
								yM = 1;
							}
							else
							{
								(pix->mb_addr)++;
								yM = 1 - maxH;
							}
						}
						else
						{
							(pix->mb_addr)++;
							yM = 0;
						}
					}
				}
			}
		}

	}
	else
	{ // xN >= 0
		if (xN >= 0 && xN < maxW)
		{
			// yN >=0
			// for the deblocker if this is the extra edge then do this special stuff
			if (p_Vid->DeblockCall == 2)
			{
				pix->mb_addr  = currMB->mb_addr_up + 1;
				pix->available = TRUE;
				yM = 0 - 1;
			}

			else if (0 <maxH)
			{
				pix->mb_addr   = currMB->mbAddrX;
				pix->available = TRUE;
				yM = 0;
			}

		}
	}
	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = (short) (xN & (maxW - 1));
		pix->y = (short) (yM & (maxH - 1));
		get_mb_pos(p_Vid, pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
		pix->pos_x = pix->pos_x + pix->x;
		pix->pos_y = pix->pos_y + pix->y;
	}
}

void getAffNeighbour0X(const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix) // xN == 0, yN full range
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW, maxH;
	int yM = -1;

	maxW = mb_size[0];
	maxH = mb_size[1];

	// initialize to "not available"
	pix->available = FALSE;

	if(yN > (maxH - 1))
	{
		return;
	}
	if (0 > (maxW - 1) && yN >= 0 && yN < maxH)
	{
		return;
	}

	if (0 < maxW)
	{
		if (yN<0)
		{
			if (!currMB->mb_field)
			{
				//frame
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					//top
					pix->mb_addr  = currMB->mb_addr_up;
					// for the deblocker if the current MB is a frame and the one above is a field
					// then the neighbor is the top MB of the pair
					if (currMB->mb_avail_up)
					{
						if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
							pix->mb_addr  += 1;
					}

					pix->available = currMB->mb_avail_up;
					yM = yN;
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mbAddrX - 1;
					pix->available = TRUE;
					yM = yN;
				}
			}
			else
			{
				// field
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_up;
					pix->available = currMB->mb_avail_up;
					if (currMB->mb_avail_up)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
						{
							(pix->mb_addr)++;
							yM = 2* yN;
						}
						else
						{
							yM = yN;
						}
					}
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_up + 1;
					pix->available = currMB->mb_avail_up;
					yM = yN;
				}
			}
		}
		else
		{
			// yN >=0
			// for the deblocker if this is the extra edge then do this special stuff
			if (yN == 0 && p_Vid->DeblockCall == 2)
			{
				pix->mb_addr  = currMB->mb_addr_up + 1;
				pix->available = TRUE;
				yM = yN - 1;
			}

			else if ((yN >= 0) && (yN <maxH))
			{
				pix->mb_addr   = currMB->mbAddrX;
				pix->available = TRUE;
				yM = yN;
			}
		}
	}
	else
	{ // xN >= maxW
		if(yN < 0)
		{
			if (!currMB->mb_field)
			{
				// frame
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr  = currMB->mb_addr_upper_right + 1;
					pix->available = currMB->mb_avail_upper_right;
					yM = yN;
				}
				else
				{
					// bottom
					pix->available = FALSE;
				}
			}
			else
			{
				// field
				if ((currMB->mbAddrX & 0x01) == 0)
				{
					// top
					pix->mb_addr   = currMB->mb_addr_upper_right;
					pix->available = currMB->mb_avail_upper_right;
					if (currMB->mb_avail_upper_right)
					{
						if(!p_Vid->mb_data[currMB->mb_addr_upper_right].mb_field)
						{
							(pix->mb_addr)++;
							yM = 2* yN;
						}
						else
						{
							yM = yN;
						}
					}
				}
				else
				{
					// bottom
					pix->mb_addr   = currMB->mb_addr_upper_right + 1;
					pix->available = currMB->mb_avail_upper_right;
					yM = yN;
				}
			}
		}
	}

	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = 0;
		pix->y = (short) (yM & (maxH - 1));
		get_mb_pos(p_Vid, pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
		pix->pos_y = pix->pos_y + pix->y;
	}
}

void getAffNeighbour0XLuma(const Macroblock *currMB, int yN, PixelPos *pix) // xN == 0, yN full range
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW, maxH;
	int yM = -1;

	maxW = 16;
	maxH = 16;

	// initialize to "not available"
	pix->available = FALSE;

	if(yN > 15)
	{
		return;
	}

	if (yN<0)
	{
		if (!currMB->mb_field)
		{
			//frame
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				//top
				pix->mb_addr  = currMB->mb_addr_up;
				// for the deblocker if the current MB is a frame and the one above is a field
				// then the neighbor is the top MB of the pair
				if (currMB->mb_avail_up)
				{
					if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
						pix->mb_addr  += 1;
				}

				pix->available = currMB->mb_avail_up;
				yM = yN;
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mbAddrX - 1;
				pix->available = TRUE;
				yM = yN;
			}
		}
		else
		{
			// field
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				// top
				pix->mb_addr   = currMB->mb_addr_up;
				pix->available = currMB->mb_avail_up;
				if (currMB->mb_avail_up)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
					{
						(pix->mb_addr)++;
						yM = 2* yN;
					}
					else
					{
						yM = yN;
					}
				}
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mb_addr_up + 1;
				pix->available = currMB->mb_avail_up;
				yM = yN;
			}
		}
	}
	else
	{
		// yN >=0
		// for the deblocker if this is the extra edge then do this special stuff
		if (yN == 0 && p_Vid->DeblockCall == 2)
		{
			pix->mb_addr  = currMB->mb_addr_up + 1;
			pix->available = TRUE;
			yM = yN - 1;
		}

		else if ((yN >= 0) && (yN <maxH))
		{
			pix->mb_addr   = currMB->mbAddrX;
			pix->available = TRUE;
			yM = yN;
		}
	}

	if (pix->available || p_Vid->DeblockCall)
	{
		const int mb_size[2] = {16,16};
		pix->x = 0;
		pix->y = (short) (yM & (maxH - 1));
		get_mb_pos(p_Vid, pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
		pix->pos_y = pix->pos_y + pix->y;
	}
}



void getAffNeighbour0N(const Macroblock *currMB, const int mb_size[2], PixelPos *pix) // xN == 0, yN = -1
{
	VideoParameters *p_Vid = currMB->p_Vid;
	int maxW, maxH;
	int yM = -1;

	maxW = mb_size[0];
	maxH = mb_size[1];

	// initialize to "not available"
	pix->available = FALSE;

	if (0 < maxW)
	{
		if (!currMB->mb_field)
		{
			//frame
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				//top
				pix->mb_addr  = currMB->mb_addr_up;
				// for the deblocker if the current MB is a frame and the one above is a field
				// then the neighbor is the top MB of the pair
				if (currMB->mb_avail_up)
				{
					if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
						pix->mb_addr  += 1;
				}

				pix->available = currMB->mb_avail_up;
				yM = -1;
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mbAddrX - 1;
				pix->available = TRUE;
				yM = -1;
			}
		}
		else
		{
			// field
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				// top
				pix->mb_addr   = currMB->mb_addr_up;
				pix->available = currMB->mb_avail_up;
				if (currMB->mb_avail_up)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
					{
						(pix->mb_addr)++;
						yM = -2;
					}
					else
					{
						yM = -1;
					}
				}
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mb_addr_up + 1;
				pix->available = currMB->mb_avail_up;
				yM = -1;
			}
		}
	}
	else
	{ // xN >= maxW
		if (!currMB->mb_field)
		{
			// frame
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				// top
				pix->mb_addr  = currMB->mb_addr_upper_right + 1;
				pix->available = currMB->mb_avail_upper_right;
				yM = -1;
			}
			else
			{
				// bottom
				pix->available = FALSE;
			}
		}
		else
		{
			// field
			if ((currMB->mbAddrX & 0x01) == 0)
			{
				// top
				pix->mb_addr   = currMB->mb_addr_upper_right;
				pix->available = currMB->mb_avail_upper_right;
				if (currMB->mb_avail_upper_right)
				{
					if(!p_Vid->mb_data[currMB->mb_addr_upper_right].mb_field)
					{
						(pix->mb_addr)++;
						yM = -2;
					}
					else
					{
						yM = -1;
					}
				}
			}
			else
			{
				// bottom
				pix->mb_addr   = currMB->mb_addr_upper_right + 1;
				pix->available = currMB->mb_avail_upper_right;
				yM = -1;
			}
		}
	}

	if (pix->available || p_Vid->DeblockCall)
	{
		pix->x = 0;
		pix->y = (short) (yM & (maxH - 1));
		get_mb_pos(p_Vid, pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
		pix->pos_y = pix->pos_y + pix->y;
	}
}

void getAffNeighbour0NLuma(const Macroblock *currMB, PixelPos *pix) // xN == 0, yN = -1
{
	VideoParameters *p_Vid = currMB->p_Vid;
	const int maxW=16, maxH=16;
	int yM = -1;


	// initialize to "not available"
	pix->available = FALSE;

	if (!currMB->mb_field)
	{
		//frame
		if ((currMB->mbAddrX & 0x01) == 0)
		{
			//top
			pix->mb_addr  = currMB->mb_addr_up;
			// for the deblocker if the current MB is a frame and the one above is a field
			// then the neighbor is the top MB of the pair
			if (currMB->mb_avail_up)
			{
				if (!(p_Vid->DeblockCall == 1 && (p_Vid->mb_data[currMB->mb_addr_up]).mb_field))
					pix->mb_addr  += 1;
			}

			pix->available = currMB->mb_avail_up;
			yM = -1;
		}
		else
		{
			// bottom
			pix->mb_addr   = currMB->mbAddrX - 1;
			pix->available = TRUE;
			yM = -1;
		}
	}
	else
	{
		// field
		if ((currMB->mbAddrX & 0x01) == 0)
		{
			// top
			pix->mb_addr   = currMB->mb_addr_up;
			pix->available = currMB->mb_avail_up;
			if (currMB->mb_avail_up)
			{
				if(!p_Vid->mb_data[currMB->mb_addr_up].mb_field)
				{
					(pix->mb_addr)++;
					yM = -2;
				}
				else
				{
					yM = -1;
				}
			}
		}
		else
		{
			// bottom
			pix->mb_addr   = currMB->mb_addr_up + 1;
			pix->available = currMB->mb_avail_up;
			yM = -1;
		}
	}

	if (pix->available || p_Vid->DeblockCall)
	{
		const int mb_size[2] = {16,16};
		pix->x = 0;
		pix->y = (short) (yM & (maxH - 1));
		get_mb_pos(p_Vid, pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
		pix->pos_y = pix->pos_y + pix->y;
	}
}


/*!
************************************************************************
* \brief
*    get neighboring 4x4 block
* \param currMB
*   current macroblock
* \param block_x
*    input x block position
* \param block_y
*    input y block position
* \param mb_size
*    Macroblock size in pixel (according to luma or chroma MB access)
* \param pix
*    returns position informations
************************************************************************
*/
void get4x4Neighbour(const Macroblock *currMB, int block_x, int block_y, const int mb_size[2], PixelPos *pix)
{
	currMB->p_Vid->getNeighbour(currMB, block_x, block_y, mb_size, pix);

	if (pix->available)
	{
		pix->x >>= 2;
		pix->y >>= 2;
		pix->pos_x >>= 2;
		pix->pos_y >>= 2;
	}
}

void get4x4NeighbourLuma(const Macroblock *currMB, int block_x, int block_y, PixelPos *pix)
{
	currMB->p_Vid->getNeighbourLuma(currMB, block_x, block_y, pix);

	if (pix->available)
	{
		pix->x >>= 2;
		pix->y >>= 2;
		pix->pos_x >>= 2;
		pix->pos_y >>= 2;
	}
}
