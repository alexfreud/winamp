#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"

void GetStrengthNormal_Vert(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p)
{
	// dir == 0
	PixelPos pixP, pixMB;
	byte     StrValue;
	Macroblock *MbP;

	if ((p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
	{ 
		// Set strength to either 3 or 4 regardless of pixel position
		StrValue = (edge == 0) ? 4 : 3;
		memset(&Strength[0], (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
	}
	else
	{    
		VideoParameters *p_Vid = MbQ->p_Vid;
		int xQ = edge - 1;
		int yQ = 0;

		p_Vid->getNeighbourX0(MbQ, xQ, p_Vid->mb_size[IS_LUMA], &pixMB);
		pixP = pixMB;
		MbP = &(p_Vid->mb_data[pixP.mb_addr]);

		if (!(MbP->mb_type==I4MB||MbP->mb_type==I8MB||MbP->mb_type==I16MB||MbP->mb_type==IPCM||MbQ->mb_type==I4MB||MbQ->mb_type==I8MB||MbQ->mb_type==I16MB||MbQ->mb_type==IPCM))
		{
			PicMotionParams *motion = &p->motion;
			h264_ref_t    ref_p0,ref_p1,ref_q0,ref_q1;
			int      blkP, blkQ, idx;
			int      blk_x, blk_x2, blk_y, blk_y2 ;

			PicMotion **motion0 = motion->motion[LIST_0];
			PicMotion **motion1 = motion->motion[LIST_1];
			short    mb_x, mb_y;

			p_Vid->get_mb_block_pos (p_Vid->PicPos, MbQ->mbAddrX, &mb_x, &mb_y);
			mb_x <<= 2;
			mb_y <<= 2;

			xQ ++;

			for( idx = 0 ; idx < MB_BLOCK_SIZE ; idx += BLOCK_SIZE )
			{
				yQ = idx;

				blkQ = (yQ & 0xFFFC) + (xQ >> 2);
				blkP = (idx & 0xFFFC) + (pixP.x >> 2);

				if( ((MbQ->cbp_blk[0] & ((int64)1 << blkQ )) != 0) || ((MbP->cbp_blk[0] & ((int64)1 << blkP)) != 0) )
					StrValue = 2;
				else
				{
					// if no coefs, but vector difference >= 1 set Strength=1
					// if this is a mixed mode edge then one set of reference pictures will be frame and the
					// other will be field    
					PicMotion *motion_p0, *motion_q0, *motion_p1, *motion_q1;
					blk_y  = mb_y + (blkQ >> 2);
					blk_x  = mb_x + (blkQ  & 3);
					blk_y2 = (pixMB.pos_y + idx) >> 2;
					blk_x2 = pixMB.pos_x >> 2;

					motion_p0=&motion0[blk_y ][blk_x ];
					motion_q0=&motion0[blk_y2][blk_x2];
					motion_p1=&motion1[blk_y ][blk_x ];
					motion_q1=&motion1[blk_y2][blk_x2];
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
			StrValue = (edge == 0) ? 4 : 3;
			memset(&Strength[0], (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
		}      
	}
}

void GetStrength_Vert_YUV420(uint8_t Strength[4], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p, PixelPos pixMB, Macroblock *MbP)
{
	// dir == 0
	int i;
	uint8_t     StrValue;

	if ((p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
	{ 
		// Set strength to either 3 or 4 regardless of pixel position
		StrValue = (edge == 0) ? 4 : 3;
		for (i=0;i<4;i++)
		{
			Strength[i]=StrValue;
		}
	}
	else
	{    
		VideoParameters *p_Vid = MbQ->p_Vid;
		if (!(MbP->mb_type==I4MB||MbP->mb_type==I8MB||MbP->mb_type==I16MB||MbP->mb_type==IPCM||MbQ->mb_type==I4MB||MbQ->mb_type==I8MB||MbQ->mb_type==I16MB||MbQ->mb_type==IPCM))
		{
			PicMotionParams *motion = &p->motion;
			h264_ref_t    ref_p0,ref_p1,ref_q0,ref_q1;
			int      blkP, blkQ, idx;
			int      blk_x2, blk_y, blk_y2 ;

			PicMotion **motion0 = motion->motion[LIST_0];
			PicMotion **motion1 = motion->motion[LIST_1];
			short    mb_x, mb_y;
			const int cbp_p=(int)MbP->cbp_blk[0], cbp_q=(int)MbQ->cbp_blk[0];

			get_mb_block_pos_normal(p_Vid->PicPos, MbQ->mbAddrX, &mb_x, &mb_y);
			mb_x <<= 2;
			mb_y <<= 2;

			mb_x += edge;
			blkQ = edge;
			blkP = pixMB.x >> 2;
			blk_x2 = pixMB.pos_x >> 2;

			for( idx = 0 ; idx < BLOCK_SIZE ; idx++,blkQ+=BLOCK_SIZE, blkP+=BLOCK_SIZE)
			{
				if (_bittest(&cbp_p, blkP) || _bittest(&cbp_q, blkQ))
					StrValue = 2;
				else
				{
					// if no coefs, but vector difference >= 1 set Strength=1
					// if this is a mixed mode edge then one set of reference pictures will be frame and the
					// other will be field          
					PicMotion *motion_p0, *motion_q0, *motion_p1, *motion_q1;
					blk_y  = mb_y + idx;
					blk_y2 = (pixMB.pos_y >> 2) + idx;

					motion_p0=&motion0[blk_y ][mb_x ];
					motion_q0=&motion0[blk_y2][blk_x2];
					motion_p1=&motion1[blk_y ][mb_x ];
					motion_q1=&motion1[blk_y2][blk_x2];
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
				Strength[idx] = StrValue;
			}
		}
		else
		{
			// Start with Strength=3. or Strength=4 for Mb-edge
			StrValue = (edge == 0) ? 4 : 3;
			for (i=0;i<4;i++)
			{
				Strength[i]=StrValue;
			}
		}      
	}
}

// assumes YUV420, MB Aff
void GetStrength_MBAff_Vert_YUV420(byte Strength[16], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p)
{
	// dir == 0
	if ((p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) 
		|| (MbQ->mb_type==I4MB || MbQ->mb_type==I16MB || MbQ->mb_type==I8MB || MbQ->mb_type==IPCM))
	{
		memset(Strength,(edge == 0) ? 4 : 3, 16); 
	}
	else
	{
		short  blkP, blkQ, idx;
		short  blk_x, blk_x2, blk_y, blk_y2 ;
		h264_ref_t  ref_p0,ref_p1,ref_q0,ref_q1;
		int    xQ, yQ;
		short  mb_x, mb_y;
		Macroblock *MbP;

		PixelPos pixP;

		PicMotionParams *motion = &p->motion;
		PicMotion **motion0 = motion->motion[LIST_0];
		PicMotion **motion1 = motion->motion[LIST_1];
		xQ = edge;
		for( idx = 0; idx < 16; ++idx )
		{
			VideoParameters *p_Vid = MbQ->p_Vid;

			yQ = idx;
			getAffNeighbourXPLuma(MbQ, xQ - 1, yQ, &pixP);
			blkQ = (short) ((yQ & 0xC) + (xQ >> 2)); // blkQ changes once every 4 loop iterations
			blkP = (short) ((pixP.y & 0xFFFC) + (pixP.x >> 2));

			MbP = &(p_Vid->mb_data[pixP.mb_addr]);
			p_Vid->mixedModeEdgeFlag = (byte) (MbQ->mb_field != MbP->mb_field);   

			// Start with Strength=3. or Strength=4 for Mb-edge
			Strength[idx] = (edge == 0) ? 4 : 3;

			if(  !(MbP->mb_type==I4MB || MbP->mb_type==I16MB || MbP->mb_type==I8MB || MbP->mb_type==IPCM))
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

static __forceinline uint8_t GetMotionStrength(PicMotion *motion0, PicMotion *motion1, int mvlimit)
{
	uint8_t StrValue;
	h264_ref_t    ref_p0,ref_p1,ref_q0,ref_q1;

	ref_p0 = motion0[0].ref_idx < 0 ? UNDEFINED_REFERENCE : motion0[0].ref_pic_id;
	ref_p1 = motion1[0].ref_idx < 0 ? UNDEFINED_REFERENCE : motion1[0].ref_pic_id;
	ref_q0 = motion0[1].ref_idx < 0 ? UNDEFINED_REFERENCE : motion0[1].ref_pic_id;
	ref_q1 = motion1[1].ref_idx < 0 ? UNDEFINED_REFERENCE : motion1[1].ref_pic_id;

	if (ref_p0==ref_q0 && ref_p1==ref_q1)
	{
		if (ref_p0 != ref_p1)
		{
			// compare MV for the same reference picture
			if (ref_p0 == UNDEFINED_REFERENCE)
			{
				StrValue =  (byte) (
					(abs( motion1[0].mv[0] - motion1[1].mv[0]) >= 4) ||
					(abs( motion1[0].mv[1] - motion1[1].mv[1]) >= mvlimit));
			}
			else if (ref_p1 == UNDEFINED_REFERENCE)
			{
				StrValue =  (byte) (
					(abs( motion0[0].mv[0] - motion0[1].mv[0]) >= 4) ||
					(abs( motion0[0].mv[1] - motion0[1].mv[1]) >= mvlimit));
			}
			else
			{
				StrValue =  (byte) (
					(abs( motion0[0].mv[0] - motion0[1].mv[0]) >= 4) ||
					(abs( motion0[0].mv[1] - motion0[1].mv[1]) >= mvlimit) ||
					(abs( motion1[0].mv[0] - motion1[1].mv[0]) >= 4) ||
					(abs( motion1[0].mv[1] - motion1[1].mv[1]) >= mvlimit));
			}
		}
		else
		{ // L0 and L1 reference pictures of p0 are the same; q0 as well
			StrValue = (byte) (
				((abs( motion0[0].mv[0] - motion0[1].mv[0]) >= 4) ||
				(abs( motion0[0].mv[1] - motion0[1].mv[1]) >= mvlimit ) ||
				(abs( motion1[0].mv[0] - motion1[1].mv[0]) >= 4) ||
				(abs( motion1[0].mv[1] - motion1[1].mv[1]) >= mvlimit))
				&&
				((abs( motion0[0].mv[0] - motion1[1].mv[0]) >= 4) ||
				(abs( motion0[0].mv[1] - motion1[1].mv[1]) >= mvlimit) ||
				(abs( motion1[0].mv[0] - motion0[1].mv[0]) >= 4) ||
				(abs( motion1[0].mv[1] - motion0[1].mv[1]) >= mvlimit)));
		}
	}
	else if (ref_p0==ref_q1 && ref_p1==ref_q0)
	{
		StrValue =  (byte) (
			(abs( motion0[0].mv[0] - motion1[1].mv[0]) >= 4) ||
			(abs( motion0[0].mv[1] - motion1[1].mv[1]) >= mvlimit) ||
			(abs( motion1[0].mv[0] - motion0[1].mv[0]) >= 4) ||
			(abs( motion1[0].mv[1] - motion0[1].mv[1]) >= mvlimit));
	}
	else
	{
		StrValue = 1;
	}
	return StrValue;
}

void GetStrength_Vert_YUV420_All(uint8_t Strength[4][4], Macroblock *MbQ, int mvlimit, StorablePicture *p, int pos_x, int pos_y, Macroblock *MbP, int luma_transform_size_8x8_flag)
{
	// dir == 0
	if ((p->slice_type>=SP_SLICE) //(p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
		|| ((1 << MbQ->mb_type) & 26112))
	{ 
		// Set strength to either 3 or 4 regardless of pixel position
		*(int32_t *)(Strength[0]) = MbP?0x04040404:0;
		*(int32_t *)(Strength[1]) = luma_transform_size_8x8_flag?0:0x03030303;
		*(int32_t *)(Strength[2]) = 0x03030303;
		*(int32_t *)(Strength[3]) = luma_transform_size_8x8_flag?0:0x03030303;
	}
	else
	{ 
		PicMotionParams *motion = &p->motion;
		int motion_stride = p->size_x >> 2;
		PicMotion *motion0 = &motion->motion[LIST_0][pos_y][pos_x];
		PicMotion *motion1 = &motion->motion[LIST_1][pos_y][pos_x];
		int cbp_q=(int)MbQ->cbp_blk[0];		

		// edge 0 
		if (!MbP)
		{
			*(int32_t *)(Strength[0]) = 0;
		}
		else if ((1 << MbP->mb_type) & 26112)
		{
			*(int32_t *)(Strength[0]) = 0x04040404;
		}
		else
		{
			int cbp_p = (int)MbP->cbp_blk[0];
			if( ((cbp_q & (1 << 0 )) != 0) || ((cbp_p & (1 << (3))) != 0) )
				Strength[0][0] = 2;
			else
				Strength[0][0] = GetMotionStrength(&motion0[0-1], &motion1[0-1], mvlimit);

			if( ((cbp_q & (1 << 4 )) != 0) || ((cbp_p & (1 << (4 + 3))) != 0) )
				Strength[0][1] = 2;
			else
				Strength[0][1] = GetMotionStrength(&motion0[motion_stride-1], &motion1[motion_stride-1], mvlimit);

			if( ((cbp_q & (1 << 8 )) != 0) || ((cbp_p & (1 << (8 + 3))) != 0) )
				Strength[0][2] = 2;
			else
				Strength[0][2] = GetMotionStrength(&motion0[2*motion_stride-1], &motion1[2*motion_stride-1], mvlimit);

			if( ((cbp_q & (1 << 12 )) != 0) || ((cbp_p & (1 << (12 + 3))) != 0) )
				Strength[0][3] = 2;
			else
				Strength[0][3] = GetMotionStrength(&motion0[3*motion_stride-1], &motion1[3*motion_stride-1], mvlimit);
		}

		// edge 1 
		if (luma_transform_size_8x8_flag)
		{
			*(int32_t *)(Strength[1]) = 0;
		}
		else
		{
			if (cbp_q & (3 << 0))
				Strength[1][0] = 2;
			else
				Strength[1][0] = GetMotionStrength(&motion0[0], &motion1[0], mvlimit);

			if (cbp_q & (3 << 4))
				Strength[1][1] = 2;
			else
				Strength[1][1] = GetMotionStrength(&motion0[1*motion_stride], &motion1[1*motion_stride], mvlimit);

			if (cbp_q & (3 << 8))
				Strength[1][2] = 2;
			else
				Strength[1][2] = GetMotionStrength(&motion0[2*motion_stride], &motion1[2*motion_stride], mvlimit);

			if (cbp_q & (3 << 12))
				Strength[1][3] = 2;
			else
				Strength[1][3] = GetMotionStrength(&motion0[3*motion_stride], &motion1[3*motion_stride], mvlimit);
		}

		// edge 2
		if (cbp_q & (6 << 0))
			Strength[2][0] = 2;
		else
			Strength[2][0] = GetMotionStrength(&motion0[1], &motion1[1], mvlimit);

		if (cbp_q & (6 << 4))
			Strength[2][1] = 2;
		else
			Strength[2][1] = GetMotionStrength(&motion0[motion_stride+1], &motion1[motion_stride+1], mvlimit);

		if (cbp_q & (6 << 8))
			Strength[2][2] = 2;
		else
			Strength[2][2] = GetMotionStrength(&motion0[2*motion_stride+1], &motion1[2*motion_stride+1], mvlimit);

		if (cbp_q & (6 << 12))
			Strength[2][3] = 2;
		else
			Strength[2][3] = GetMotionStrength(&motion0[3*motion_stride+1], &motion1[3*motion_stride+1], mvlimit);

		// edge 3
		if (luma_transform_size_8x8_flag)
		{
			*(int32_t *)(Strength[3]) = 0;
		}
		else
		{
			if (cbp_q & (0xC << 0))
				Strength[3][0] = 2;
			else
				Strength[3][0] = GetMotionStrength(&motion0[2], &motion1[2], mvlimit);

			if (cbp_q & (0xC << 4))
				Strength[3][1] = 2;
			else
				Strength[3][1] = GetMotionStrength(&motion0[motion_stride+2], &motion1[motion_stride+2], mvlimit);

			if (cbp_q & (0xC << 8))
				Strength[3][2] = 2;
			else
				Strength[3][2] = GetMotionStrength(&motion0[2*motion_stride+2], &motion1[2*motion_stride+2], mvlimit);

			if (cbp_q & (0xC << 12))
				Strength[3][3] = 2;
			else
				Strength[3][3] = GetMotionStrength(&motion0[3*motion_stride+2], &motion1[3*motion_stride+2], mvlimit);
		}
	}
}
