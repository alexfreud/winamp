#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"

void GetStrengthNormal_Horiz(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p)
{
	// dir == 1
	PixelPos pixMB;
	byte     StrValue;
	Macroblock *MbP;

	assert(NUM_SLICE_TYPES == 5); // the next line assumes this
	if (p->slice_type>=SP_SLICE) //(p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
	{ 
		// Set strength to either 3 or 4 regardless of pixel position
		StrValue = (edge == 0 && p->structure==FRAME) ? 4 : 3;
		memset(&Strength[0], (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
	}
	else
	{    
		VideoParameters *p_Vid = MbQ->p_Vid;
		int yQ = edge < 16 ? edge - 1: 0;

		p_Vid->getNeighbour0X(MbQ, yQ, p_Vid->mb_size[IS_LUMA], &pixMB);

		MbP = &(p_Vid->mb_data[pixMB.mb_addr]);

		if (!(MbP->mb_type==I4MB||MbP->mb_type==I8MB||MbP->mb_type==I16MB||MbP->mb_type==IPCM||MbQ->mb_type==I4MB||MbQ->mb_type==I8MB||MbQ->mb_type==I16MB||MbQ->mb_type==IPCM))
		{
			PicMotionParams *motion = &p->motion;
			h264_ref_t    ref_p0,ref_p1,ref_q0,ref_q1;
			int      blkP, blkQ, idx;
			int      blk_x, blk_y ;
			int posx;

			PicMotion **motion0 = motion->motion[LIST_0];
			PicMotion **motion1 = motion->motion[LIST_1];
			short    mb_x, mb_y;
			const int blk_y2 = pixMB.pos_y >> 2;
			int cbp_pq, cbp_p, cbp_q;

			posx = pixMB.pos_x >> 2;
			blkP = (pixMB.y & 0xFFFC);
			blkQ = ((yQ+1) & 0xFFFC);

			cbp_p = (int)MbQ->cbp_blk[0];
			cbp_q = (int)MbP->cbp_blk[0];
			cbp_pq = (((cbp_p >> blkQ) & 0xF) | ((cbp_q >> blkP) & 0xF));
			if (cbp_pq == 0xF)
			{
				memset(Strength, 2, 16);
				return;
				//StrValue = 2;
			}

			p_Vid->get_mb_block_pos (p_Vid->PicPos, MbQ->mbAddrX, &mb_x, &mb_y);
			mb_x <<= 2;
			mb_y <<= 2;

			blk_x  = mb_x + (blkQ  & 3);
			blk_y  = mb_y + (blkQ >> 2);

			for( idx = 0 ; idx < MB_BLOCK_SIZE ; idx += BLOCK_SIZE, posx++, blkP++, blkQ++, blk_x++, cbp_pq>>=1)
			{
				if (cbp_pq & 1)
					StrValue = 2;
				else
				{
					PicMotion *motion_p0, *motion_q0, *motion_p1, *motion_q1;

					motion_p0=&motion0[blk_y ][blk_x ];
					motion_q0=&motion0[blk_y2][posx];
					motion_p1=&motion1[blk_y ][blk_x ];
					motion_q1=&motion1[blk_y2][posx];
					// if no coefs, but vector difference >= 1 set Strength=1
					// if this is a mixed mode edge then one set of reference pictures will be frame and the
					// other will be field 								
					ref_p0 = motion_p0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p0->ref_pic_id;
					ref_q0 = motion_q0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q0->ref_pic_id;
					ref_p1 = motion_p1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p1->ref_pic_id;
					ref_q1 = motion_q1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q1->ref_pic_id;
					if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) || ((ref_p0==ref_q1) && (ref_p1==ref_q0)))
					{
						// L0 and L1 reference pictures of p0 are different; q0 as well
						if (ref_p0 != ref_p1)
						{
							// compare MV for the same reference picture
							if (ref_p0 == ref_q0)
							{
								if (ref_p0 == UNDEFINED_REFERENCE)
								{
									StrValue =  (byte) (
										(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit));
								}
								else if (ref_p1 == UNDEFINED_REFERENCE)
								{
									StrValue =  (byte) (
										(abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit));
								}
								else
								{
									StrValue =  (byte) (
										(abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit) ||
										(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit));
								}
							}
							else
							{
								StrValue =  (byte) (
									(abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
									(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
									(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
									(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit));
							}
						}
						else
						{ // L0 and L1 reference pictures of p0 are the same; q0 as well
							StrValue = (byte) (
								((abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
								(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit ) ||
								(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
								(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit))
								&&
								((abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
								(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
								(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
								(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit)));
						}
					}
					else
					{
						StrValue = 1;
					}
				}
				memset(&Strength[idx], (byte) StrValue, BLOCK_SIZE * sizeof(byte));
			}
		}
		else
		{
			// Start with Strength=3. or Strength=4 for Mb-edge
			StrValue = (edge == 0 && p->structure==FRAME) ? 4 : 3;
			memset(&Strength[0], (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
		}      
	}
}


void GetStrength_Horiz_YUV420(byte Strength[4], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p, PixelPos pixMB, Macroblock *MbP)
{
	// dir == 1
	byte     StrValue;

	assert(NUM_SLICE_TYPES == 5); // the next line assumes this
	if (p->slice_type>=SP_SLICE) //(p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
	{ 
		// Set strength to either 3 or 4 regardless of pixel position
		StrValue = (edge == 0 && p->structure==FRAME) ? 4 : 3;
		memset(&Strength[0], (byte) StrValue, 4 * sizeof(byte));
	}
	else
	{    
		VideoParameters *p_Vid = MbQ->p_Vid;
		int yQ = edge < 16 ? edge - 1: 0;

		if (!(MbP->mb_type==I4MB||MbP->mb_type==I8MB||MbP->mb_type==I16MB||MbP->mb_type==IPCM||MbQ->mb_type==I4MB||MbQ->mb_type==I8MB||MbQ->mb_type==I16MB||MbQ->mb_type==IPCM))
		{
			PicMotionParams *motion = &p->motion;
			h264_ref_t    ref_p0,ref_p1,ref_q0,ref_q1;
			int      blkP, blkQ, idx;
			int posx;

			PicMotion **motion0 = motion->motion[LIST_0];
			PicMotion **motion1 = motion->motion[LIST_1];

			const int blk_y2 = pixMB.pos_y >> 2;
			int cbp_pq, cbp_p, cbp_q;

			blkP = (pixMB.y & 0xFFFC);
			blkQ = ((yQ+1) & 0xFFFC);

			cbp_p = (int)MbQ->cbp_blk[0];
			cbp_q = (int)MbP->cbp_blk[0];
			cbp_pq = (((cbp_p >> blkQ) & 0xF) | ((cbp_q >> blkP) & 0xF));
			if (cbp_pq == 0xF)
			{
				memset(Strength, 2, 4);
				return;
				//StrValue = 2;
			}
			posx = pixMB.pos_x >> 2;
#ifdef _DEBUG
			{
				short    mb_x, mb_y;
				get_mb_block_pos_normal(p_Vid->PicPos, MbQ->mbAddrX, &mb_x, &mb_y);
				assert((mb_x << 2) == posx);
				assert(((mb_y << 2) + (blkQ >> 2)) == (blk_y2+1));
			}
#endif
			//blk_y  = mb_y + (blkQ >> 2);

			for( idx = 0 ; idx < MB_BLOCK_SIZE ; idx += BLOCK_SIZE, posx++, cbp_pq>>=1)
			{
				if (cbp_pq & 1)
					StrValue = 2;
				else
				{
					PicMotion *motion_p0, *motion_q0, *motion_p1, *motion_q1;


					motion_p0=&motion0[blk_y2+1][posx];
					motion_q0=&motion0[blk_y2][posx];
					motion_p1=&motion1[blk_y2+1][posx];
					motion_q1=&motion1[blk_y2][posx];

					// if no coefs, but vector difference >= 1 set Strength=1
					// if this is a mixed mode edge then one set of reference pictures will be frame and the
					// other will be field 								
					ref_p0 = motion_p0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p0->ref_pic_id;
					ref_q0 = motion_q0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q0->ref_pic_id;
					ref_p1 = motion_p1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p1->ref_pic_id;
					ref_q1 = motion_q1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q1->ref_pic_id;
					if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) || ((ref_p0==ref_q1) && (ref_p1==ref_q0)))
					{
						// L0 and L1 reference pictures of p0 are different; q0 as well
						if (ref_p0 != ref_p1)
						{
							// compare MV for the same reference picture
							if (ref_p0 == ref_q0)
							{
								if (ref_p0 == UNDEFINED_REFERENCE)
								{
									StrValue =  (byte) (
										(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit));
								}
								else if (ref_p1 == UNDEFINED_REFERENCE)
								{
									StrValue =  (byte) (
										(abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit));
								}
								else
								{
									StrValue =  (byte) (
										(abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit) ||
										(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit));
								}
							}
							else
							{
								StrValue =  (byte) (
									(abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
									(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
									(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
									(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit));
							}
						}
						else
						{ // L0 and L1 reference pictures of p0 are the same; q0 as well
							StrValue = (byte) (
								((abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
								(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit ) ||
								(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
								(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit))
								&&
								((abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
								(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
								(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
								(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit)));
						}
					}
					else
					{
						StrValue = 1;
					}
				}
				Strength[idx/4] = StrValue;
				//memset(&Strength[idx/4], (byte) StrValue,  sizeof(byte));
			}
		}
		else
		{
			// Start with Strength=3. or Strength=4 for Mb-edge
			StrValue = (edge == 0 && p->structure==FRAME) ? 4 : 3;
			memset(&Strength[0], (byte) StrValue, 4 * sizeof(byte));
		}      
	}
}

void GetStrengthMBAff_Horiz_YUV420(byte Strength[16], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p)
{
	// dir == 1
	short  blkP, blkQ, idx;
	short  blk_x, blk_x2, blk_y, blk_y2 ;
	h264_ref_t  ref_p0,ref_p1,ref_q0,ref_q1;
	int    xQ, yQ;
	short  mb_x, mb_y;
	Macroblock *MbP;

	PixelPos pixP;
	int dir_m1 = 0;

	PicMotionParams *motion = &p->motion;
	PicMotion **motion0 = motion->motion[LIST_0];
	PicMotion **motion1 = motion->motion[LIST_1];
	yQ = (edge < MB_BLOCK_SIZE ? edge : 1);

	for( idx = 0; idx < 16; ++idx )
	{
		VideoParameters *p_Vid = MbQ->p_Vid;
		xQ = idx;

		getAffNeighbourPXLumaNB(MbQ, xQ , yQ - 1, &pixP);
		blkQ = (short) ((yQ & 0xFFFC) + (xQ >> 2));
		blkP = (short) ((pixP.y & 0xFFFC) + (pixP.x >> 2));

		MbP = &(p_Vid->mb_data[pixP.mb_addr]);
		p_Vid->mixedModeEdgeFlag = (byte) (MbQ->mb_field != MbP->mb_field);   

		if ((p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
		{
			Strength[idx] = (edge == 0 && (((!MbP->mb_field && !MbQ->mb_field)))) ? 4 : 3;
		}
		else
		{
			// Start with Strength=3. or Strength=4 for Mb-edge
			Strength[idx] = (edge == 0 && (((!MbP->mb_field && !MbQ->mb_field)))) ? 4 : 3;

			if(  !(MbP->mb_type==I4MB || MbP->mb_type==I16MB || MbP->mb_type==I8MB || MbP->mb_type==IPCM)
				&& !(MbQ->mb_type==I4MB || MbQ->mb_type==I16MB || MbQ->mb_type==I8MB || MbQ->mb_type==IPCM) )
			{
				if( ((MbQ->cbp_blk[0] &  ((int64)1 << blkQ )) != 0) || ((MbP->cbp_blk[0] &  ((int64)1 << blkP)) != 0) )
					Strength[idx] = 2 ;
				else
				{
					// if no coefs, but vector difference >= 1 set Strength=1
					// if this is a mixed mode edge then one set of reference pictures will be frame and the
					// other will be field
					if (p_Vid->mixedModeEdgeFlag)
					{
						(Strength[idx] = 1);
					}
					else
					{
						get_mb_block_pos_mbaff(p_Vid->PicPos, MbQ->mbAddrX, &mb_x, &mb_y);
						blk_y  = (short) ((mb_y<<2) + (blkQ >> 2));
						blk_x  = (short) ((mb_x<<2) + (blkQ  & 3));
						blk_y2 = (short) (pixP.pos_y >> 2);
						blk_x2 = (short) (pixP.pos_x >> 2);
						{
							PicMotion *motion_p0, *motion_q0, *motion_p1, *motion_q1;
							motion_p0=&motion0[blk_y ][blk_x ];
							motion_q0=&motion0[blk_y2][blk_x2];
							motion_p1=&motion1[blk_y ][blk_x ];
							motion_q1=&motion1[blk_y2][blk_x2];

							ref_p0 = motion_p0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p0->ref_pic_id;
							ref_q0 = motion_q0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q0->ref_pic_id;
							ref_p1 = motion_p1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p1->ref_pic_id;
							ref_q1 = motion_q1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q1->ref_pic_id;
							if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) ||
								((ref_p0==ref_q1) && (ref_p1==ref_q0)))
							{
								Strength[idx]=0;
								// L0 and L1 reference pictures of p0 are different; q0 as well
								if (ref_p0 != ref_p1)
								{
									// compare MV for the same reference picture
									if (ref_p0==ref_q0)
									{
										Strength[idx] =  (byte) (
											(abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
											(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit) ||
											(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
											(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit));
									}
									else
									{
										Strength[idx] =  (byte) (
											(abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
											(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
											(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
											(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit));
									}
								}
								else
								{ // L0 and L1 reference pictures of p0 are the same; q0 as well

									Strength[idx] = (byte) (
										((abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit ) ||
										(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit))
										&&
										((abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
										(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit)));
								}
							}
							else
							{
								Strength[idx] = 1;
							}
						}
					}
				}
			}
		}
	}
}

static __forceinline uint8_t GetMotionStrength(PicMotion *motion0, PicMotion *motion1, int motion_stride, int mvlimit)
{
	h264_ref_t    ref_p0,ref_p1,ref_q0,ref_q1;
  
	ref_p0 = motion0[0].ref_idx < 0 ? UNDEFINED_REFERENCE : motion0[0].ref_pic_id;
	ref_p1 = motion1[0].ref_idx < 0 ? UNDEFINED_REFERENCE : motion1[0].ref_pic_id;
	ref_q0 = motion0[motion_stride].ref_idx < 0 ? UNDEFINED_REFERENCE : motion0[motion_stride].ref_pic_id;
	ref_q1 = motion1[motion_stride].ref_idx < 0 ? UNDEFINED_REFERENCE : motion1[motion_stride].ref_pic_id;

	if (ref_p0==ref_q0 && ref_p1==ref_q1)
	{
		if (ref_p0 != ref_p1)
		{
			// compare MV for the same reference picture
			if (ref_p0 == UNDEFINED_REFERENCE)
			{
				return (byte) (
					(abs( motion1[0].mv[0] - motion1[motion_stride].mv[0]) >= 4) ||
					(abs( motion1[0].mv[1] - motion1[motion_stride].mv[1]) >= mvlimit));
			}
			else if (ref_p1 == UNDEFINED_REFERENCE)
			{
				return  (byte) (
					(abs( motion0[0].mv[0] - motion0[motion_stride].mv[0]) >= 4) ||
					(abs( motion0[0].mv[1] - motion0[motion_stride].mv[1]) >= mvlimit));
			}
			else
			{
				return  (byte) (
					(abs( motion0[0].mv[0] - motion0[motion_stride].mv[0]) >= 4) ||
					(abs( motion0[0].mv[1] - motion0[motion_stride].mv[1]) >= mvlimit) ||
					(abs( motion1[0].mv[0] - motion1[motion_stride].mv[0]) >= 4) ||
					(abs( motion1[0].mv[1] - motion1[motion_stride].mv[1]) >= mvlimit));
			}
		}
		else
		{ // L0 and L1 reference pictures of p0 are the same; q0 as well
			return (byte) (
				((abs( motion0[0].mv[0] - motion0[motion_stride].mv[0]) >= 4) ||
				(abs( motion0[0].mv[1] - motion0[motion_stride].mv[1]) >= mvlimit ) ||
				(abs( motion1[0].mv[0] - motion1[motion_stride].mv[0]) >= 4) ||
				(abs( motion1[0].mv[1] - motion1[motion_stride].mv[1]) >= mvlimit))
				&&
				((abs( motion0[0].mv[0] - motion1[motion_stride].mv[0]) >= 4) ||
				(abs( motion0[0].mv[1] - motion1[motion_stride].mv[1]) >= mvlimit) ||
				(abs( motion1[0].mv[0] - motion0[motion_stride].mv[0]) >= 4) ||
				(abs( motion1[0].mv[1] - motion0[motion_stride].mv[1]) >= mvlimit)));
		}
	}
	else if (ref_p0==ref_q1 && ref_p1==ref_q0)
	{
		return  (byte) (
			(abs( motion0[0].mv[0] - motion1[motion_stride].mv[0]) >= 4) ||
			(abs( motion0[0].mv[1] - motion1[motion_stride].mv[1]) >= mvlimit) ||
			(abs( motion1[0].mv[0] - motion0[motion_stride].mv[0]) >= 4) ||
			(abs( motion1[0].mv[1] - motion0[motion_stride].mv[1]) >= mvlimit));
	}
	else
	{
		return 1;
	}
}


void GetStrength_Horiz_YUV420_All(uint8_t Strength[4][4], Macroblock *MbQ, int mvlimit, StorablePicture *p, int pos_x, int pos_y, Macroblock *MbP, int luma_transform_size_8x8_flag)
{
	// dir == 1
	assert(NUM_SLICE_TYPES == 5); // the next line assumes this
	if ((p->slice_type>=SP_SLICE) //(p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
		|| ((1 << MbQ->mb_type) & 26112))
	{ 
		// Set strength to either 3 or 4 regardless of pixel position
		*(int32_t *)(Strength[0]) = MbP?p->structure==FRAME ? 0x04040404 : 0x03030303 : 0;
		*(int32_t *)(Strength[1]) = luma_transform_size_8x8_flag?0:0x03030303;
		*(int32_t *)(Strength[2]) = 0x03030303;
		*(int32_t *)(Strength[3]) = luma_transform_size_8x8_flag?0:0x03030303;
	}
	else
	{
		PicMotionParams *motion = &p->motion;
		int motion_stride = p->size_x>>2;
		PicMotion *motion0 = &motion->motion[LIST_0][pos_y-!!MbP][pos_x];
		PicMotion *motion1 = &motion->motion[LIST_1][pos_y-!!MbP][pos_x];

		int cbp_p, cbp_q=(int)MbQ->cbp_blk[0], cbp_pq;	

		// edge 0
		if (!MbP)
		{
			*(int32_t *)(Strength[0]) = 0;
		}
		else if ((1 << MbP->mb_type) & 26112)
		{
			*(int32_t *)(Strength[0]) = p->structure==FRAME ? 0x04040404 : 0x03030303;
			motion0 += motion_stride;
			motion1 += motion_stride;
		}
		else
		{
			cbp_p=(int)MbP->cbp_blk[0];
			cbp_pq = (((cbp_p >> 12) & 0xF) | (cbp_q & 0xF));
			if (cbp_pq == 0xF)
			{
				memset(Strength[0], 2, 4);
			}
			else
			{
				if (cbp_pq & (1<<0))
					Strength[0][0] = 2;
				else
					Strength[0][0] = GetMotionStrength(&motion0[0], &motion1[0], motion_stride, mvlimit);

				if (cbp_pq & (1<<1))
					Strength[0][1] = 2;
				else
					Strength[0][1] = GetMotionStrength(&motion0[1], &motion1[1], motion_stride, mvlimit);

				if (cbp_pq & (1<<2))
					Strength[0][2] = 2;
				else
					Strength[0][2] = GetMotionStrength(&motion0[2], &motion1[2], motion_stride, mvlimit);

				if (cbp_pq & (1<<3))
					Strength[0][3] = 2;
				else
					Strength[0][3] = GetMotionStrength(&motion0[3], &motion1[3], motion_stride, mvlimit);
			}
			motion0 += motion_stride;
			motion1 += motion_stride;
		}

		// edge 1
		if (luma_transform_size_8x8_flag)
		{
			*(int32_t *)(Strength[1]) = 0;
		}
		else
		{
			cbp_pq = ((cbp_q) | (cbp_q >> 4)) & 0xF;
			if (cbp_pq == 0xF)
			{
				memset(Strength[1], 2, 4);
			}
			else
			{
				if (cbp_pq & (1<<0))
					Strength[1][0] = 2;
				else
					Strength[1][0] = GetMotionStrength(&motion0[0], &motion1[0], motion_stride, mvlimit);

				if (cbp_pq & (1<<1))
					Strength[1][1] = 2;
				else
					Strength[1][1] = GetMotionStrength(&motion0[1], &motion1[1], motion_stride, mvlimit);

				if (cbp_pq & (1<<2))
					Strength[1][2] = 2;
				else
					Strength[1][2] = GetMotionStrength(&motion0[2], &motion1[2], motion_stride, mvlimit);

				if (cbp_pq & (1<<3))
					Strength[1][3] = 2;
				else
					Strength[1][3] = GetMotionStrength(&motion0[3], &motion1[3], motion_stride, mvlimit);

			}
		}


		motion0 += motion_stride;
			motion1 += motion_stride;
		// edge 2 
		cbp_pq = (cbp_q | (cbp_q >> 4)) & 0xF0;
		if (cbp_pq == 0xF0)
		{
			memset(Strength[2], 2, 4);
		}
		else
		{
			if (cbp_pq & (0x10<<0))
				Strength[2][0] = 2;
			else
				Strength[2][0] = GetMotionStrength(&motion0[0], &motion1[0], motion_stride, mvlimit);

			if (cbp_pq & (0x10<<1))
				Strength[2][1] = 2;
			else
				Strength[2][1] = GetMotionStrength(&motion0[1], &motion1[1], motion_stride, mvlimit);

			if (cbp_pq & (0x10<<2))
				Strength[2][2] = 2;
			else
				Strength[2][2] = GetMotionStrength(&motion0[2], &motion1[2], motion_stride, mvlimit);

			if (cbp_pq & (0x10<<3))
				Strength[2][3] = 2;
			else
				Strength[2][3] = GetMotionStrength(&motion0[3], &motion1[3], motion_stride, mvlimit);
		}


		motion0 += motion_stride;
			motion1 += motion_stride;
		// edge 3
		if (luma_transform_size_8x8_flag)
		{
			*(int32_t *)(Strength[3]) = 0;
		}
		else
		{
			cbp_pq = (cbp_q | (cbp_q >> 4)) & 0xF00;
			if (cbp_pq == 0xF00)
			{
				memset(Strength[3], 2, 4);
			}
			else
			{
				if (cbp_pq & (0x100<<0))
					Strength[3][0] = 2;
				else
					Strength[3][0] = GetMotionStrength(&motion0[0], &motion1[0], motion_stride, mvlimit);

				if (cbp_pq & (0x100<<1))
					Strength[3][1] = 2;
				else
					Strength[3][1] = GetMotionStrength(&motion0[1], &motion1[1], motion_stride, mvlimit);

				if (cbp_pq & (0x100<<2))
					Strength[3][2] = 2;
				else
					Strength[3][2] = GetMotionStrength(&motion0[2], &motion1[2], motion_stride, mvlimit);

				if (cbp_pq & (0x100<<3))
					Strength[3][3] = 2;
				else
					Strength[3][3] = GetMotionStrength(&motion0[3], &motion1[3], motion_stride, mvlimit);
			}
		}
	}
}