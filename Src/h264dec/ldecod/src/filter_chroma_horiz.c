#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"
#include <mmintrin.h>
#include <emmintrin.h>

static const byte ALPHA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,4,4,5,6,  7,8,9,10,12,13,15,17,  20,22,25,28,32,36,40,45,  50,56,63,71,80,90,101,113,  127,144,162,182,203,226,255,255} ;
static const byte  BETA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,2,2,2,3,  3,3,3, 4, 4, 4, 6, 6,   7, 7, 8, 8, 9, 9,10,10,  11,11,12,12,13,13, 14, 14,   15, 15, 16, 16, 17, 17, 18, 18} ;
static const byte CLIP_TAB[52][5]  =
{
	{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},
	{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},{ -1, 0, 0, 0, 0},
	{ -1, 0, 0, 0, 0},{ -1, 0, 0, 1, 1},{ -1, 0, 0, 1, 1},{ -1, 0, 0, 1, 1},{ -1, 0, 0, 1, 1},{ -1, 0, 1, 1, 1},{ -1, 0, 1, 1, 1},{ -1, 1, 1, 1, 1},
	{ -1, 1, 1, 1, 1},{ -1, 1, 1, 1, 1},{ -1, 1, 1, 1, 1},{ -1, 1, 1, 2, 2},{ -1, 1, 1, 2, 2},{ -1, 1, 1, 2, 2},{ -1, 1, 1, 2, 2},{ -1, 1, 2, 3, 3},
	{ -1, 1, 2, 3, 3},{ -1, 2, 2, 3, 3},{ -1, 2, 2, 4, 4},{ -1, 2, 3, 4, 4},{ -1, 2, 3, 4, 4},{ -1, 3, 3, 5, 5},{ -1, 3, 4, 6, 6},{ -1, 3, 4, 6, 6},
	{ -1, 4, 5, 7, 7},{ -1, 4, 5, 8, 8},{ -1, 4, 6, 9, 9},{ -1, 5, 7,10,10},{ -1, 6, 8,11,11},{ -1, 6, 8,13,13},{ -1, 7,10,14,14},{ -1, 8,11,16,16},
	{ -1, 9,12,18,18},{ -1,10,13,20,20},{ -1,11,15,23,23},{ -1,13,17,25,25}
};

static const int pelnum_cr[2][4] =  {{0,8,16,16}, {0,8, 8,16}};  //[dir:0=vert, 1=hor.][yuv_format]

#define LOAD_LINE_EPI16(reg, ptr) { reg = _mm_loadl_epi64((__m128i *)(ptr));	reg = _mm_unpacklo_epi8(reg, xmm_zero); }
static void FilterChroma8_Horiz_sse2(int inc_dim, imgpel *SrcPtrP, imgpel *SrcPtrQ, const byte Strength[16], const byte *ClipTab, int Alpha, int Beta, int bitdepth_scale, int max_imgpel_value)
{
	__m128i xmm_L1, xmm_L0, xmm_R0, xmm_R1;
	__m128i xmm_strength;
	__m128i xmm_absdiff, xmm_diff, xmm_acc;
	__m128i xmm_127, xmm_zero;
	__m128i xmm_alpha, xmm_beta;

			int match;
			xmm_zero = _mm_setzero_si128();
			xmm_strength = _mm_load_si128((__m128i *)Strength);
			xmm_127 = _mm_set1_epi8(127);
			xmm_strength = _mm_adds_epu8(xmm_strength, xmm_127);
			xmm_strength = _mm_srai_epi16(xmm_strength, 15); // shift so it's all 0xFFFF or 0x0000

			LOAD_LINE_EPI16(xmm_R0, SrcPtrQ);
			LOAD_LINE_EPI16(xmm_L0, SrcPtrP);

			xmm_alpha = _mm_set1_epi16((uint16_t)Alpha);

			// if ( abs( R0 - L0 ) < Alpha ) 
			xmm_diff=_mm_subs_epu16(xmm_R0, xmm_L0);
			xmm_absdiff =_mm_subs_epu16(xmm_L0, xmm_R0);
			xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
			xmm_absdiff = _mm_cmplt_epi16(xmm_absdiff, xmm_alpha);
			xmm_strength = _mm_and_si128(xmm_strength, xmm_absdiff);
			match = _mm_movemask_epi8(xmm_strength);
			if (match == 0)
				return;

			LOAD_LINE_EPI16(xmm_R1, SrcPtrQ+inc_dim);

			xmm_beta = _mm_set1_epi16((uint16_t)Beta);

			// if ( abs(R0 - R1) < Beta )  
			xmm_diff=_mm_subs_epu16(xmm_R0, xmm_R1);
			xmm_absdiff =_mm_subs_epu16(xmm_R1, xmm_R0);
			xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
			xmm_absdiff = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
			xmm_strength = _mm_and_si128(xmm_strength, xmm_absdiff);
			match = _mm_movemask_epi8(xmm_strength);
			if (match == 0)
				return;

			LOAD_LINE_EPI16(xmm_L1, SrcPtrP-inc_dim);

			// if ( abs(L0 - L1) < Beta )
			xmm_diff=_mm_subs_epu16(xmm_L0, xmm_L1);
			xmm_absdiff =_mm_subs_epu16(xmm_L1, xmm_L0);
			xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
			xmm_absdiff = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
			xmm_strength = _mm_and_si128(xmm_strength, xmm_absdiff);
			match = _mm_movemask_epi8(xmm_strength);
			if (match == 0)
				return;

			if (Strength[0] == 4) // if strong filter is in use, ALL strengths will be 4
			{
				// *SrcPtrP = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
				__m128i xmm_2  = _mm_set1_epi16(2);

				xmm_acc = xmm_L1;
				xmm_acc = _mm_slli_epi16(xmm_acc, 1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_L0);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_R1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_2);
				xmm_acc = _mm_srai_epi16(xmm_acc, 2);
				xmm_acc = _mm_and_si128(xmm_acc, xmm_strength);
				xmm_L0  = _mm_andnot_si128(xmm_strength, xmm_L0);
				xmm_L0  = _mm_or_si128(xmm_L0, xmm_acc);
				xmm_L0 = _mm_packus_epi16(xmm_L0, xmm_L0);
				_mm_storel_epi64((__m128i *)(SrcPtrP), xmm_L0);

				// *SrcPtrQ = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
				xmm_acc = xmm_R1;
				xmm_acc = _mm_slli_epi16(xmm_acc, 1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_R0);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_L1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_2);
				xmm_acc = _mm_srai_epi16(xmm_acc, 2);
				xmm_acc = _mm_and_si128(xmm_acc, xmm_strength);
				xmm_R0  = _mm_andnot_si128(xmm_strength, xmm_R0);
				xmm_R0  = _mm_or_si128(xmm_R0, xmm_acc);
				xmm_R0 = _mm_packus_epi16(xmm_R0, xmm_R0);
				_mm_storel_epi64((__m128i *)(SrcPtrQ), xmm_R0);
			}
			else
			{
				int C0 = ClipTab[ Strength[0] ] * bitdepth_scale + 1;
				int C1 = ClipTab[ Strength[4] ] * bitdepth_scale + 1;
				int C2 = ClipTab[ Strength[8] ] * bitdepth_scale + 1;
				int C3 = ClipTab[ Strength[12] ] * bitdepth_scale + 1;
				__m128i xmm_tc0 = _mm_setr_epi16(C0, C0, C1, C1, C2, C2, C3, C3); // TODO: benski> probably a better way to do this.
				__m128i xmm_negative_tc0 = _mm_sub_epi16(xmm_zero, xmm_tc0);
				__m128i xmm_4 = _mm_set1_epi16(4);
				//int dif = iClip3( -tc0, tc0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );
				xmm_acc = xmm_R0;
				xmm_acc = _mm_sub_epi16(xmm_acc, xmm_L0);
				xmm_acc = _mm_slli_epi16(xmm_acc, 2);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_L1);
				xmm_acc = _mm_sub_epi16(xmm_acc, xmm_R1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_4);
				xmm_acc = _mm_srai_epi16(xmm_acc, 3);
				xmm_acc = _mm_min_epi16(xmm_acc, xmm_tc0);
				xmm_acc = _mm_max_epi16(xmm_acc, xmm_negative_tc0);
				xmm_acc = _mm_and_si128(xmm_acc, xmm_strength);

				// *SrcPtrP = (imgpel) iClip1 ( max_imgpel_value, L0 + dif) ;
				xmm_L0 = _mm_add_epi16(xmm_L0, xmm_acc);
				xmm_L0 = _mm_packus_epi16(xmm_L0, xmm_L0);
				_mm_storel_epi64((__m128i *)(SrcPtrP), xmm_L0);

				// *SrcPtrQ = (imgpel) iClip1 ( max_imgpel_value, R0 - dif) ;	
				xmm_R0 = _mm_sub_epi16(xmm_R0, xmm_acc);
				xmm_R0 = _mm_packus_epi16(xmm_R0, xmm_R0);
				_mm_storel_epi64((__m128i *)(SrcPtrQ), xmm_R0);
			}
	
	
}

static void IntraStrongFilter_Chroma8_Horiz_YUV420_sse2(int inc_dim, imgpel *SrcPtrP, int Alpha, int Beta)
{
	__m128i xmm_L1, xmm_L0, xmm_R0, xmm_R1;
	__m128i xmm_strength;
	__m128i xmm_absdiff, xmm_diff, xmm_acc;
	__m128i  xmm_zero;
	__m128i xmm_alpha, xmm_beta;
__m128i xmm_2;

			int match;
			xmm_zero = _mm_setzero_si128();

			LOAD_LINE_EPI16(xmm_L0, SrcPtrP);
			LOAD_LINE_EPI16(xmm_R0, SrcPtrP+inc_dim);

			xmm_alpha = _mm_set1_epi16((uint16_t)Alpha);

			// if ( abs( R0 - L0 ) < Alpha ) 
			xmm_diff=_mm_subs_epu16(xmm_R0, xmm_L0);
			xmm_absdiff =_mm_subs_epu16(xmm_L0, xmm_R0);
			xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
			xmm_strength = _mm_cmplt_epi16(xmm_absdiff, xmm_alpha);
			match = _mm_movemask_epi8(xmm_strength);
			if (match == 0)
				return;

			LOAD_LINE_EPI16(xmm_R1, SrcPtrP+2*inc_dim);

			xmm_beta = _mm_set1_epi16((uint16_t)Beta);

			// if ( abs(R0 - R1) < Beta )  
			xmm_diff=_mm_subs_epu16(xmm_R0, xmm_R1);
			xmm_absdiff =_mm_subs_epu16(xmm_R1, xmm_R0);
			xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
			xmm_absdiff = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
			xmm_strength = _mm_and_si128(xmm_strength, xmm_absdiff);
			match = _mm_movemask_epi8(xmm_strength);
			if (match == 0)
				return;

			LOAD_LINE_EPI16(xmm_L1, SrcPtrP-inc_dim);

			// if ( abs(L0 - L1) < Beta )
			xmm_diff=_mm_subs_epu16(xmm_L0, xmm_L1);
			xmm_absdiff =_mm_subs_epu16(xmm_L1, xmm_L0);
			xmm_absdiff =_mm_or_si128(xmm_absdiff, xmm_diff);
			xmm_absdiff = _mm_cmplt_epi16(xmm_absdiff, xmm_beta);
			xmm_strength = _mm_and_si128(xmm_strength, xmm_absdiff);
			match = _mm_movemask_epi8(xmm_strength);
			if (match == 0)
				return;

			
				// *SrcPtrP = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
				xmm_2  = _mm_set1_epi16(2);

				xmm_acc = xmm_L1;
				xmm_acc = _mm_slli_epi16(xmm_acc, 1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_L0);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_R1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_2);
				xmm_acc = _mm_srai_epi16(xmm_acc, 2);
				xmm_acc = _mm_and_si128(xmm_acc, xmm_strength);
				xmm_L0  = _mm_andnot_si128(xmm_strength, xmm_L0);
				xmm_L0  = _mm_or_si128(xmm_L0, xmm_acc);
				xmm_L0 = _mm_packus_epi16(xmm_L0, xmm_L0);
				_mm_storel_epi64((__m128i *)(SrcPtrP), xmm_L0);

				// *SrcPtrQ = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
				xmm_acc = xmm_R1;
				xmm_acc = _mm_slli_epi16(xmm_acc, 1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_R0);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_L1);
				xmm_acc = _mm_add_epi16(xmm_acc, xmm_2);
				xmm_acc = _mm_srai_epi16(xmm_acc, 2);
				xmm_acc = _mm_and_si128(xmm_acc, xmm_strength);
				xmm_R0  = _mm_andnot_si128(xmm_strength, xmm_R0);
				xmm_R0  = _mm_or_si128(xmm_R0, xmm_acc);
				xmm_R0 = _mm_packus_epi16(xmm_R0, xmm_R0);
				_mm_storel_epi64((__m128i *)(SrcPtrP+inc_dim), xmm_R0);
			
	
	
}


// separate function to make it easier to unit test
static void FilterChroma8_Horiz(int inc_dim, imgpel *SrcPtrP, imgpel *SrcPtrQ, const byte Strength[16], const byte *ClipTab, int Alpha, int Beta, int bitdepth_scale, int max_imgpel_value)
{
	int pel;
	for( pel = 0 ; pel < 8 ; ++pel, SrcPtrP++, SrcPtrQ++ )
	{
		int Strng = Strength[(((pel >> 1) << 2) + (pel & 0x01))];

		if( Strng != 0)
		{
			imgpel  L0  = *SrcPtrP;
			imgpel  R0  = *SrcPtrQ;

			if ( abs( R0 - L0 ) < Alpha ) 
			{
				imgpel R1  = *(SrcPtrQ + inc_dim);
				if ( abs(R0 - R1) < Beta )  
				{
					imgpel L1  = *(SrcPtrP - inc_dim);
					if ( abs(L0 - L1) < Beta )
					{
						if( Strng == 4 )    // INTRA strong filtering
						{
							*SrcPtrP = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
							*SrcPtrQ = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
						}
						else
						{
							int tc0  = ClipTab[ Strng ] * bitdepth_scale + 1;
							int dif = iClip3( -tc0, tc0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

							*SrcPtrP = (imgpel) iClip1 ( max_imgpel_value, L0 + dif) ;
							*SrcPtrQ = (imgpel) iClip1 ( max_imgpel_value, R0 - dif) ;
						}
					}
				}
			}
		}
	}
}

void EdgeLoopChromaNormal_Horiz(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, int uv, StorablePicture *p)
{ 
	// dir == 1
	imgpel** Img = image->img;
	VideoParameters *p_Vid = MbQ->p_Vid;  

	int yQ = (edge < 16 ? edge - 1: 0);  
	PixelPos pixMB1;

	p_Vid->getNeighbour0X(MbQ, yQ, p_Vid->mb_size[IS_CHROMA], &pixMB1);

	if (pixMB1.available || (MbQ->DFDisableIdc == 0))
	{
		int      bitdepth_scale   = p_Vid->bitdepth_scale[IS_CHROMA];
		int      max_imgpel_value = p_Vid->max_pel_value_comp[uv + 1];

		int AlphaC0Offset = MbQ->DFAlphaC0Offset;
		int BetaOffset = MbQ->DFBetaOffset;
		PixelPos pixP = pixMB1;
		Macroblock *MbP = &(p_Vid->mb_data[pixP.mb_addr]);

		// Average QP of the two blocks
		int QP = (MbP->qpc[uv] + MbQ->qpc[uv] + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
		int indexB = iClip3(0, MAX_QP, QP + BetaOffset);

		int Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
		int Beta    = BETA_TABLE [indexB] * bitdepth_scale;

		if (Alpha !=0 && Beta != 0)
		{
			const int PelNum = pelnum_cr[1][p->chroma_format_idc];
			const     byte *ClipTab = CLIP_TAB[indexA];
			int       inc_dim = image->stride;
			int pel;
			PixelPos pixQ, pixMB2;    

			p_Vid->getNeighbour0X(MbQ, ++yQ, p_Vid->mb_size[IS_CHROMA], &pixMB2);
			pixQ = pixMB2;

			if (pelnum_cr[1][p->chroma_format_idc] == 8)
			{
				imgpel *SrcPtrP = &(Img[pixP.pos_y][pixP.pos_x]);
				imgpel *SrcPtrQ = &(Img[pixQ.pos_y][pixQ.pos_x]);
				if (sse2_flag)
					FilterChroma8_Horiz_sse2(inc_dim, SrcPtrP, SrcPtrQ, Strength, ClipTab, Alpha, Beta, bitdepth_scale, max_imgpel_value);
				else
					FilterChroma8_Horiz(inc_dim, SrcPtrP, SrcPtrQ, Strength, ClipTab, Alpha, Beta, bitdepth_scale, max_imgpel_value);
					
			}
			else
			{
				for( pel = 0 ; pel < PelNum ; ++pel )
				{
					int Strng = Strength[(PelNum == 8) ? (((pel >> 1) << 2) + (pel & 0x01)) : pel];

					if( Strng != 0)
					{
						imgpel *SrcPtrP = &(Img[pixP.pos_y][pixP.pos_x]);
						imgpel  L0  = *SrcPtrP;
						imgpel *SrcPtrQ = &(Img[pixQ.pos_y][pixQ.pos_x]);
						imgpel  R0  = *SrcPtrQ;

						if ( abs( R0 - L0 ) < Alpha ) 
						{
							imgpel R1  = *(SrcPtrQ + inc_dim);
							if ( abs(R0 - R1) < Beta )  
							{
								imgpel L1  = *(SrcPtrP - inc_dim);
								if ( abs(L0 - L1) < Beta )
								{
									if( Strng == 4 )    // INTRA strong filtering
									{
										*SrcPtrP = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
										*SrcPtrQ = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
									}
									else
									{
										int tc0  = ClipTab[ Strng ] * bitdepth_scale + 1;
										int dif = iClip3( -tc0, tc0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

										*SrcPtrP = (imgpel) iClip1 ( max_imgpel_value, L0 + dif) ;
										*SrcPtrQ = (imgpel) iClip1 ( max_imgpel_value, R0 - dif) ;
									}
								}
							}
						}
					}
					pixP.pos_x++;
					pixQ.pos_x++;
				}
			}
		}
	}
}


static void FilterChroma8_Horiz_sse(int p_step, imgpel *SrcPtrP, int Alpha, int Beta, const uint8_t Strength[4], const char *ClipTab)
{
	__m64 mmx_alpha_minus_one = _mm_set1_pi16(Alpha-1), mmx_beta_minus_one = _mm_set1_pi16(Beta-1);
	__m64 mmx_zero	= _mm_setzero_si64(), mmx_four=_mm_set1_pi16(4);
	__m64 mmx_minus_one;
	__m64 mmx_absdiff, mmx_diff;
	__m64 mmx_L0, mmx_L1;
	__m64 mmx_R0, mmx_R1;
	__m64 mmx_C0, mmx_negative_C0,  mmx_dif,  mmx_match;
	int match;
	int i=0;

	mmx_minus_one = _mm_set1_pi32(-1);

	STAGE:

	while (!Strength[i*2] && !Strength[i*2+1])
	{
		SrcPtrP += 4;
		if (i++ == 1) // last stage
			return;
	}

		mmx_L0 = _mm_cvtsi32_si64(*(int *)(SrcPtrP));        
		mmx_R0 = _mm_cvtsi32_si64(*(int *)(SrcPtrP+p_step));

		// abs( R0 - L0 ) < Alpha
		// MMX doesn't have unsigned compare, so we have to go to short
		mmx_L0 = _mm_unpacklo_pi8(mmx_L0, mmx_zero);
		mmx_R0 = _mm_unpacklo_pi8(mmx_R0, mmx_zero);
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_L0);
		mmx_absdiff =_mm_subs_pu16(mmx_L0, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_match = _mm_cmpgt_pi16(mmx_absdiff, mmx_alpha_minus_one); // 1's in any words we don't have to do
		mmx_match = _mm_xor_si64(mmx_match, mmx_minus_one);
		match = _mm_movemask_pi8(mmx_match); 
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;
			SrcPtrP += 4;
			goto STAGE; // start the process over from next position
		}

		// abs( R0 - R1) < Beta
		// MMX doesn't have unsigned compare, so we have to go to short
		// mmx_R0 already populated
		mmx_R1 = _mm_cvtsi32_si64(*(int *)(SrcPtrP+2*p_step));
		mmx_R1 = _mm_unpacklo_pi8(mmx_R1, mmx_zero);
		mmx_diff=_mm_subs_pu16(mmx_R0, mmx_R1);
		mmx_absdiff =_mm_subs_pu16(mmx_R1, mmx_R0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_absdiff = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_absdiff = _mm_xor_si64(mmx_absdiff, mmx_minus_one);
		mmx_match = _mm_and_si64(mmx_match, mmx_absdiff);
		match = _mm_movemask_pi8(mmx_match);
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;
				SrcPtrP += 4;
			goto STAGE; // start the process over from next position
		}

		// abs(L0 - L1) < Beta
		// MMX doesn't have unsigned compare, so we have to go to short
		// mmx_L0 already populated
		mmx_L1 = _mm_cvtsi32_si64(*(int *)(SrcPtrP-p_step));
		mmx_L1 = _mm_unpacklo_pi8(mmx_L1, mmx_zero);    
		mmx_diff=_mm_subs_pu16(mmx_L0, mmx_L1);
		mmx_absdiff =_mm_subs_pu16(mmx_L1, mmx_L0);
		mmx_absdiff =_mm_or_si64(mmx_absdiff, mmx_diff);
		mmx_absdiff = _mm_cmpgt_pi16(mmx_absdiff, mmx_beta_minus_one);
		mmx_absdiff = _mm_xor_si64(mmx_absdiff, mmx_minus_one);
		mmx_match = _mm_and_si64(mmx_match, mmx_absdiff);
		match = _mm_movemask_pi8(mmx_match);
		if (match == 0)
		{
			if (i++ == 1) // last stage
				return;
SrcPtrP += 4;
			goto STAGE; // start the process over from next position
		}

		// ok, now time to performn the actual calculation. hope it was worth it!!

		// tc0  = ClipTab[ Strng ]  + 1
		mmx_C0 = _mm_setr_pi16(ClipTab[Strength[i*2]]+1, ClipTab[Strength[i*2]]+1, ClipTab[Strength[i*2+1]]+1, ClipTab[Strength[i*2+1]]+1);
		mmx_negative_C0 = _mm_sub_pi16(mmx_zero, mmx_C0);

		// dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );
		mmx_dif = mmx_R0;
		mmx_dif = _mm_sub_pi16(mmx_dif, mmx_L0);
		mmx_dif = _mm_slli_pi16(mmx_dif, 2);
		mmx_dif = _mm_add_pi16(mmx_dif, mmx_L1);
		mmx_dif = _mm_sub_pi16(mmx_dif, mmx_R1);
		mmx_dif = _mm_add_pi16(mmx_dif, mmx_four);
		mmx_dif = _mm_srai_pi16(mmx_dif, 3);
		mmx_dif = _mm_min_pi16(mmx_dif, mmx_C0);
		mmx_dif = _mm_max_pi16(mmx_dif, mmx_negative_C0);
		mmx_dif = _mm_and_si64(mmx_dif, mmx_match);

		// L0 = (imgpel) iClip1(max_imgpel_value, L0 + dif);
		mmx_L0 = _mm_add_pi16(mmx_L0, mmx_dif);

		// R0 = (imgpel) iClip1(max_imgpel_value, R0 - dif);
		mmx_R0 = _mm_sub_pi16(mmx_R0, mmx_dif);

		// store
		mmx_R0 = _mm_packs_pu16(mmx_R0, mmx_R0); 
		mmx_L0 = _mm_packs_pu16(mmx_L0, mmx_L0); 

		*(int *)SrcPtrP = _mm_cvtsi64_si32(mmx_L0);
		*(int *)(SrcPtrP+p_step) = _mm_cvtsi64_si32(mmx_R0);

		if (i++ == 1)
			return;

		SrcPtrP += 4;
		goto STAGE; // next stage
}


void EdgeLoopChroma_Horiz_YUV420(VideoImage *image, const byte strength[4], Macroblock *MbQ, int uv, PixelPos pixMB, Macroblock *MbP)
{ 
	// dir == 1
	imgpel** Img = image->img;

	if (pixMB.available || (MbQ->DFDisableIdc == 0))
	{
		int AlphaC0Offset = MbQ->DFAlphaC0Offset;
		int BetaOffset = MbQ->DFBetaOffset;

		// Average QP of the two blocks
		int QP = (MbP->qpc[uv] + MbQ->qpc[uv] + 1) >> 1;

		int indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
		int indexB = iClip3(0, MAX_QP, QP + BetaOffset);

		int Alpha   = ALPHA_TABLE[indexA] ;
		int Beta    = BETA_TABLE [indexB] ;

		if (Alpha !=0 && Beta != 0)
		{
			const int PelNum = 8;

			int       inc_dim = image->stride;
			imgpel *SrcPtrP;


			SrcPtrP = &(Img[pixMB.pos_y>>1][pixMB.pos_x>>1]);

			if (strength[0] == 4) // if strong filter is used, all blocks will be strong
			{
				IntraStrongFilter_Chroma8_Horiz_YUV420_sse2(inc_dim, SrcPtrP,   Alpha, Beta);
			}
			else
			{
				const     byte *ClipTab = CLIP_TAB[indexA];
				FilterChroma8_Horiz_sse(inc_dim, SrcPtrP, Alpha, Beta, strength, ClipTab);
			}
		}
	}
}