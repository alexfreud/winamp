#include "dec_api.h"
#include "global.h"
#include "nalu.h"
#include "image.h"
#include "meminput.h"
#include "output.h"
#include "fmo.h"
#include "erc_api.h"
#include "parset.h"
#include "memcache.h"
#include "block.h"
#include "optim.h"
#include "mc_prediction.h"
#include "vlc.h"
#include <stddef.h> // for offsetof

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

OptimizedFunctions opt;

DecoderParams *alloc_decoder();
void Configure(VideoParameters *p_Vid, InputParameters *p_Inp);
void malloc_slice(InputParameters *p_Inp, VideoParameters *p_Vid);
void init        (VideoParameters *p_Vid);
void free_slice  (Slice *currSlice);
void free_img( VideoParameters *p_Vid);

int sse2_flag = 0, mmx_flag=0, sse_flag=0, sse3_flag=0, sse4_1_flag=0;
int H264_Init()
{
	int flags_edx, flags_ecx;
	#ifdef H264_IPP
	ippStaticInit();
	#endif

#ifdef _M_IX86
	 _asm {
		 mov eax, 1
			 cpuid
			 mov flags_edx, edx
			 mov flags_ecx, ecx
	 }
	 mmx_flag = flags_edx & 0x00800000;
	 sse_flag = flags_edx & 0x02000000;
	 sse2_flag = flags_edx & 0x04000000;
	 sse3_flag = flags_ecx & 0x00000001;
	 sse4_1_flag= flags_ecx & (1 << 19);
	 
#elif defined(_M_X64)
	 sse2_flag = 1;
#endif

#ifdef _M_IX86
	 /* if you get any compile errors here, you need to change biari.asm */
	 BUILD_BUG_ON(offsetof(TextureInfoContexts, map_contexts) != 436);
	 BUILD_BUG_ON(offsetof(TextureInfoContexts, last_contexts) != 3252);
	 BUILD_BUG_ON(offsetof(TextureInfoContexts, one_contexts) != 6068);	 
	 BUILD_BUG_ON(offsetof(TextureInfoContexts, abs_contexts) != 6508);	 	 
	 
	 BUILD_BUG_ON(offsetof(Macroblock, p_Slice) != 0);
	 BUILD_BUG_ON(offsetof(Macroblock, p_Vid) != 4);	 
 	 BUILD_BUG_ON(offsetof(Macroblock, qp) != 60);	 
	 BUILD_BUG_ON(offsetof(Macroblock, qpc) != 64);	 
	 BUILD_BUG_ON(offsetof(Macroblock, qp_scaled) != 72);	 	 
	 BUILD_BUG_ON(offsetof(Macroblock, cbp_blk) != 248);	 
	 BUILD_BUG_ON(offsetof(Macroblock, mb_field) != 344);
	 BUILD_BUG_ON(offsetof(Macroblock, read_and_store_CBP_block_bit) != 400);	 
	  
	 BUILD_BUG_ON(offsetof(Slice, tex_ctx) != 100);	
	 BUILD_BUG_ON(offsetof(Slice, mb_rec) != 1696);	
	 BUILD_BUG_ON(offsetof(Slice, mb_pred) != 928);	
	 BUILD_BUG_ON(offsetof(Slice, coeff) != 15632);	 
	 BUILD_BUG_ON(offsetof(Slice, coeff_ctr) != 15760);	 
	 BUILD_BUG_ON(offsetof(Slice, pos) != 15764);	 
	 BUILD_BUG_ON(offsetof(Slice, cof) != 2464);	 
	 BUILD_BUG_ON(offsetof(Slice, last_dquant) != 88);	 
	 BUILD_BUG_ON(offsetof(Slice, mot_ctx) != 96);	 
	 BUILD_BUG_ON(offsetof(Slice, slice_type) != 64);	 
	 
	 
	 BUILD_BUG_ON(offsetof(StorablePicture, structure) != 0);
	 BUILD_BUG_ON(offsetof(StorablePicture, chroma_qp_offset) != 158688);	 	 
	 BUILD_BUG_ON(offsetof(StorablePicture, motion) != 158524);
	 BUILD_BUG_ON(offsetof(StorablePicture, plane_images) != 158512);
	 BUILD_BUG_ON(offsetof(StorablePicture, imgY) != 158512);

	 
	 BUILD_BUG_ON(offsetof(VideoParameters, structure) != 697200);
	 BUILD_BUG_ON(offsetof(VideoParameters, bitdepth_chroma_qp_scale) != 697456);
	 BUILD_BUG_ON(offsetof(VideoParameters, dec_picture) != 698192);

	 BUILD_BUG_ON(offsetof(DecodingEnvironment, Dcodestrm_len) != 16);
	 BUILD_BUG_ON(offsetof(DecodingEnvironment, Dcodestrm) != 12);
	 BUILD_BUG_ON(offsetof(DecodingEnvironment, DbitsLeft) != 8);
	 BUILD_BUG_ON(offsetof(DecodingEnvironment, Dvalue) != 4);
	 BUILD_BUG_ON(offsetof(DecodingEnvironment, Drange) != 0);

	 BUILD_BUG_ON(sizeof(BiContextType) != 4);
	 BUILD_BUG_ON(offsetof(BiContextType, state) != 0);
	 BUILD_BUG_ON(offsetof(BiContextType, MPS) != 2);

	BUILD_BUG_ON(offsetof(OptimizedFunctions, copy_image_data_16x16_stride) != 32);
#endif

	 if (sse2_flag)
	 {
		 //opt.itrans4x4 = itrans4x4_mmx;
		 opt.itrans8x8 = itrans8x8_sse2;
		 opt.weighted_mc_prediction16x16 = weighted_mc_prediction16x16_sse2;
		 opt.weighted_mc_prediction16x8 = weighted_mc_prediction16x8_sse2;
		 opt.weighted_mc_prediction8x8 = weighted_mc_prediction8x8_sse2;

		 opt.weighted_bi_prediction16x16 = weighted_bi_prediction16x16_sse2;
		 opt.weighted_bi_prediction16x8 = weighted_bi_prediction16x8_sse2;
		 opt.weighted_bi_prediction8x8 = weighted_bi_prediction8x8_sse2;
		 
		 opt.bi_prediction8x8 = bi_prediction8x8_sse2;
		 opt.copy_image_data_16x16_stride = copy_image_data_16x16_stride_sse;
		 opt.code_from_bitstream_2d_5_4 = code_from_bitstream_2d_5_4_sse2;
		 opt.code_from_bitstream_2d_17_4 = code_from_bitstream_2d_17_4_sse2;
		 opt.code_from_bitstream_2d_16_1 = code_from_bitstream_2d_16_1_sse2;
	 }
	 else if (sse_flag && mmx_flag)
	 {
		 //opt.itrans4x4 = itrans4x4_mmx;
		 opt.itrans8x8 = itrans8x8_c;//itrans8x8_mmx;

		 opt.weighted_mc_prediction16x16 = weighted_mc_prediction16x16_ipp;
		 opt.weighted_mc_prediction16x8 = weighted_mc_prediction16x8_ipp;
		 opt.weighted_mc_prediction8x8 = weighted_mc_prediction8x8_ipp;

		 opt.weighted_bi_prediction16x16 = weighted_bi_prediction16x16_ipp;
		 opt.weighted_bi_prediction16x8 = weighted_bi_prediction16x8_ipp;
		 opt.weighted_bi_prediction8x8 = weighted_bi_prediction8x8_ipp;
		 
		 opt.bi_prediction8x8 = bi_prediction8x8_ipp;
		 opt.copy_image_data_16x16_stride = copy_image_data_16x16_stride_sse;
		 opt.code_from_bitstream_2d_5_4 = code_from_bitstream_2d_5_4_c;
		 opt.code_from_bitstream_2d_17_4 = code_from_bitstream_2d_17_4_c;
		 opt.code_from_bitstream_2d_16_1 = code_from_bitstream_2d_16_1_c;
	 }
	 else
		 return 0;

	 return 1;
}

h264_decoder_t H264_CreateDecoder()
{
	DecoderParams *decoder=alloc_decoder();

	if (decoder)
	{
		InputParameters *p_Inp = decoder->p_Inp;
		Configure(decoder->p_Vid, p_Inp);
		p_Inp->intra_profile_deblocking = 1;

		initBitsFile(decoder->p_Vid);

		malloc_slice(decoder->p_Inp, decoder->p_Vid);
		init_old_slice(decoder->p_Vid->old_slice);

		init(decoder->p_Vid);

		init_out_buffer(decoder->p_Vid);

		decoder->p_Vid->current_mb_nr = -4711;     // initialized to an impossible value for debugging -- correct value is taken from slice header

	}
	return decoder;
}

void H264_DestroyDecoder(h264_decoder_t d)
{
	DecoderParams *decoder = (DecoderParams *)d;
	if (decoder)
	{
		free_slice(decoder->p_Vid->currentSlice);
		FmoFinit(decoder->p_Vid);

		free_global_buffers(decoder->p_Vid);
		flush_dpb(decoder->p_Vid);

#if (PAIR_FIELDS_IN_OUTPUT)
		flush_pending_output(decoder->p_Vid);
#endif

		out_storable_pictures_destroy(decoder->p_Vid);

		ercClose(decoder->p_Vid, decoder->p_Vid->erc_errorVar);

		CleanUpPPS(decoder->p_Vid);
		free_dpb(decoder->p_Vid);
		uninit_out_buffer(decoder->p_Vid);
		image_cache_flush(&decoder->p_Vid->image_cache[0]);
		image_cache_flush(&decoder->p_Vid->image_cache[1]);
		motion_cache_flush(&decoder->p_Vid->motion_cache);
		FreeNALU(decoder->p_Vid->nalu);
		free (decoder->p_Inp);
		free_img (decoder->p_Vid);
		free(decoder);
	}
}

void H264_DecodeFrame(h264_decoder_t d, const void *buffer, size_t bufferlen, uint64_t time_code)
{
	DecoderParams *decoder = (DecoderParams *)d;
	int ret;
	memory_input_t *mem_input = decoder->p_Vid->mem_input;
	mem_input->user_buffer=buffer;
	mem_input->user_buffer_size=bufferlen;
	mem_input->user_buffer_read=0;
	__try
	{
		ret = decode_one_frame(decoder->p_Vid, time_code);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		int x;
		x=0;
	}
#ifdef _M_IX86
		_mm_empty();
#endif
}

void H264_GetPicture(h264_decoder_t d, StorablePicture **pic)
{
	DecoderParams *decoder = (DecoderParams *)d;
	if (pic)
	{
		out_storable_picture_get(decoder->p_Vid, pic);
	}
}

static double GetAspectRatio(const vui_seq_parameters_t *vui)
{
	int aspect_ratio_width=1, aspect_ratio_height=1;

	if (vui->aspect_ratio_info_present_flag) 
	{
		switch(vui->aspect_ratio_idc)
		{
		case VUI_AR_UNDEFINED:
		case VUI_AR_SQUARE:
			aspect_ratio_width  = 1;
			aspect_ratio_height = 1;
			break;
		case VUI_AR_12_11:
			aspect_ratio_width  = 12;
			aspect_ratio_height = 11;
			break;
		case VUI_AR_10_11:
			aspect_ratio_width  = 10;
			aspect_ratio_height = 11;
			break;
		case VUI_AR_16_11:
			aspect_ratio_width  = 16;
			aspect_ratio_height = 11;
			break;
		case VUI_AR_40_33:
			aspect_ratio_width  = 40;
			aspect_ratio_height = 33;
			break;
		case VUI_AR_24_11:
			aspect_ratio_width  = 24;
			aspect_ratio_height = 11;
			break;
		case VUI_AR_20_11:
			aspect_ratio_width  = 20;
			aspect_ratio_height = 11;
			break;
		case VUI_AR_32_11:
			aspect_ratio_width  = 32;
			aspect_ratio_height = 11;
			break;
		case VUI_AR_80_33:
			aspect_ratio_width  = 80;
			aspect_ratio_height = 33;
			break;
		case VUI_AR_18_11:
			aspect_ratio_width  = 18;
			aspect_ratio_height = 11;
			break;
		case VUI_AR_15_11:
			aspect_ratio_width  = 15;
			aspect_ratio_height = 11;
			break;
		case VUI_AR_64_33:
			aspect_ratio_width  = 64;
			aspect_ratio_height = 33;
			break;
		case VUI_AR_160_99:
			aspect_ratio_width  = 160;
			aspect_ratio_height = 99;
			break;
		case VUI_AR_4_3:
			aspect_ratio_width  = 4;
			aspect_ratio_height = 3;
			break;
		case VUI_AR_3_2:
			aspect_ratio_width  = 3;
			aspect_ratio_height = 2;
			break;;
		case VUI_AR_2_1:
			aspect_ratio_width  = 2;
			aspect_ratio_height = 1;
			break;;
		case VUI_EXTENDED_SAR:
		default:
			aspect_ratio_width  = vui->sar_width;
			aspect_ratio_height = vui->sar_height;
			break;
		}
	}
	return (double)aspect_ratio_width / (double)aspect_ratio_height;
}

const FrameFormat *H264_GetOutputFormat(h264_decoder_t d, double *aspect_ratio)
{
	DecoderParams *decoder = (DecoderParams *)d;
	if (decoder && decoder->p_Inp)
	{
		if (decoder->p_Vid->active_sps)
			*aspect_ratio = GetAspectRatio(&decoder->p_Vid->active_sps->vui_seq_parameters);

		return &decoder->p_Inp->output;
	}
	else
		return 0;
}

void H264_Flush(h264_decoder_t d)
{
	DecoderParams *decoder = (DecoderParams *)d;
	if (decoder && decoder->p_Vid)
	{
		StorablePicture *pic=0;
		exit_picture(decoder->p_Vid,  &decoder->p_Vid->dec_picture);
		if (pic)
			free_storable_picture(decoder->p_Vid, pic);
		pic=0;

		decoder->p_Vid->frame_num = 0;
		decoder->p_Vid->pre_frame_num = INT_MIN;
		decoder->p_Vid->PreviousFrameNum=0;
		decoder->p_Vid->PreviousFrameNumOffset = 0;
		decoder->p_Vid->PrevPicOrderCntLsb = 0;
		decoder->p_Vid->PrevPicOrderCntMsb = 0;
		flush_dpb(decoder->p_Vid);

		do
		{
			pic=0;
			out_storable_picture_get(decoder->p_Vid, &pic);
			if (pic)
				free_storable_picture(decoder->p_Vid, pic);
		} while (pic);
		decoder->p_Vid->mem_input->resetting = 1;
	}
}

void H264_FreePicture(h264_decoder_t d, StorablePicture *p)
{
	DecoderParams *decoder = (DecoderParams *)d;
	if (decoder && decoder->p_Vid && p)
	{
		free_storable_picture(decoder->p_Vid, p);
	}
}

void H264_EndOfStream(h264_decoder_t d)
{
	DecoderParams *decoder = (DecoderParams *)d;
	if (decoder && decoder->p_Vid)
	{
		if (decoder->p_Vid->dec_picture)
		exit_picture(decoder->p_Vid, &decoder->p_Vid->dec_picture);
		else
			flush_dpb(decoder->p_Vid);
	}
}

void H264_HurryUp(h264_decoder_t d, int state)
{
	DecoderParams *decoder = (DecoderParams *)d;
	if (decoder && decoder->p_Vid)
	{
		memory_input_t *mem_input = decoder->p_Vid->mem_input;
		if (mem_input)
			mem_input->skip_b_frames = state;
	}
}