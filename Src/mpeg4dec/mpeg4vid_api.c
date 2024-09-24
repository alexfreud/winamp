#include "mpeg4vid_api.h"
#include "mp4def.h"
#include "mp4dec.h"

/* TODO: we havn't implemented "end of stream" yet, but when we do
we need to flush out the buffered frames
m_decInfo->VisualObject.vFrame = m_decInfo->VisualObject.VideoObject.prevPlaneIsB ?
&m_decInfo->VisualObject.nFrame : &m_decInfo->VisualObject.cFrame;
*/

// disable post processing for now, doesn't seem to be working out too well
//#define MP4V_DO_POST_PROCESS 

#define MAX_CODED_FRAME 8000000 // TODO: benski> verify
typedef struct 
{
	mp4_Info dec;
	int initted;
	int resetting;
	int skip_b_frames;
#ifdef MP4V_DO_POST_PROCESS
	mp4_Frame ppFrame0, ppFrame1;
#endif
	__declspec(align(32)) uint8_t buffer[MAX_CODED_FRAME];
} MPEG4VideoDecoder;

static int FreeBuffers(MPEG4VideoDecoder *decoder);

void       mp4_Error(const char *str)
{
	puts(str);
}

mpeg4vid_decoder_t MPEG4Video_CreateDecoder(int filetype, int codec)
{
	MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)malloc(sizeof(MPEG4VideoDecoder));
	memset(decoder, 0, sizeof(MPEG4VideoDecoder));

	// set configuration stuff
	decoder->dec.stopOnErr = 0;
	decoder->dec.strictSyntaxCheck = 0;
	decoder->dec.VisualObject.verid = 1;
	decoder->dec.ftype = filetype;
	decoder->dec.ftype_f = codec;

	return decoder;
}

void MPEG4Video_DestroyDecoder(mpeg4vid_decoder_t d)
{
	MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)d;
	if (decoder)
	{
#ifdef MP4V_DO_POST_PROCESS
		free(decoder->ppFrame0.mid);
		free(decoder->ppFrame1.mid);
#endif
		FreeBuffers(decoder);
		free(decoder);
	}
}

void MPEG4Video_Flush(mpeg4vid_decoder_t d)
{
	MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)d;
	if (decoder && decoder->initted) 
	{
		mp4_ResetVOL(&decoder->dec);
		decoder->resetting = 1;
	}
}

static int AllocateInitFrame(mp4_Frame* pFrame)
{
	int32_t  w, h;

	w = (pFrame->mbPerRow + 2 * MP4_NUM_EXT_MB) << 4;
	h = (pFrame->mbPerCol + 2 * MP4_NUM_EXT_MB) << 4;
	pFrame->mid = malloc(w * h + (w * h >> 1));
	if (!pFrame->mid)
		return 1; // out of memory

	ippsSet_8u(0, pFrame->mid, w * h);
	ippsSet_8u(128, pFrame->mid + w * h, w * h >> 1);

	pFrame->stepY = w;
	pFrame->stepCb = w >> 1;
	pFrame->stepCr = w >> 1;

	/* benski> stuff from LockFrame... */
	pFrame->apY = pFrame->mid;
	pFrame->pY = pFrame->apY + w * 16 + 16;
	pFrame->apCb = pFrame->apY + w * h;
	w >>= 1;
	h >>= 1;
	pFrame->pCb = pFrame->apCb + w * 8 + 8;
	pFrame->apCr = pFrame->apCb + w * h;
	pFrame->pCr = pFrame->apCr + w * 8 + 8;

	return 0;
}

static int AllocateBuffers(MPEG4VideoDecoder *decoder)
{
	if (mp4_InitVOL(&decoder->dec) != MP4_STATUS_OK) 
	{
		mp4_Error("Error: No memory to allocate internal buffers\n");
		return 1;//UMC_ERR_ALLOC;
	}
	return 0;
}

static int InsideInit(MPEG4VideoDecoder *decoder)
{
	int status;
	uint32_t code;
	int h_vo_found = 0, h_vos_found = 0;

	for (;;)
	{
		if (!mp4_SeekStartCodeOrShortPtr(&decoder->dec)) 
		{
			mp4_Error("Error: Can't find Visual Object or Video Object start codes or short_video_start_marker\n");
			return 1;//UMC_ERR_SYNC;
		}
		// check short_video_start_marker
		if (mp4_IsShortCode(&decoder->dec)) 
		{
			if ((mp4_Parse_VideoObject(&decoder->dec)) != MP4_STATUS_OK)
				return 1;//UMC_ERR_INVALID_STREAM;
			break;
		}
		code = mp4_GetBits(&decoder->dec, 8);
		if (!h_vos_found && code == MP4_VISUAL_OBJECT_SEQUENCE_SC) 
		{
			h_vos_found = 1;
			if ((mp4_Parse_VisualObjectSequence(&decoder->dec)) != MP4_STATUS_OK)
				return 1;//UMC_ERR_INVALID_STREAM;
		}

		if (!h_vo_found && code == MP4_VISUAL_OBJECT_SC) 
		{
			h_vo_found = 1;
			if ((mp4_Parse_VisualObject(&decoder->dec)) != MP4_STATUS_OK)
				return 1;//UMC_ERR_INVALID_STREAM;
		}
		// some streams can start with video_object_layer
		if ((int32_t)code >= MP4_VIDEO_OBJECT_LAYER_MIN_SC && code <= MP4_VIDEO_OBJECT_LAYER_MAX_SC) {
			decoder->dec.bufptr -= 4;
			if ((mp4_Parse_VideoObject(&decoder->dec)) != MP4_STATUS_OK)
				return 1;//UMC_ERR_INVALID_STREAM;
			break;
		}
	}

	status = AllocateBuffers(decoder);
	if (status)
		return status;


#if 0 // don't really care about this, but might be useful code for later?
	// set profile/level info
	decoder->profile = decoder->dec.profile_and_level_indication >> 4;
	decoder->level = decoder->dec.profile_and_level_indication & 15;
	if (decoder->profile == MPEG4_PROFILE_SIMPLE)
		if (decoder->level == 8)
			decoder->level = MPEG4_LEVEL_0;
	if (decoder->profile == MPEG4_PROFILE_ADVANCED_SIMPLE) {
		if (decoder->level == 7)
			decoder->level = MPEG4_LEVEL_3B;
		if (decoder->level > 7) {
			decoder->profile = MPEG4_PROFILE_FGS;
			decoder->level -= 8;
		}
	}
#endif
	decoder->initted = 1;

	return 0;
}

static int FreeBuffers(MPEG4VideoDecoder *decoder)
{
	int status = 0;

		ReleaseFrame(&decoder->dec.VisualObject, decoder->dec.VisualObject.cFrame);
		decoder->dec.VisualObject.cFrame=0;
		ReleaseFrame(&decoder->dec.VisualObject, decoder->dec.VisualObject.rFrame);
		decoder->dec.VisualObject.rFrame=0;
		ReleaseFrame(&decoder->dec.VisualObject, decoder->dec.VisualObject.nFrame);
		decoder->dec.VisualObject.nFrame=0;
		ReleaseFrame(&decoder->dec.VisualObject, decoder->dec.VisualObject.sFrame);
		decoder->dec.VisualObject.sFrame=0;
		FreeCache(&decoder->dec.VisualObject);
	
	mp4_FreeVOL(&decoder->dec);
	return status;
}

static int PostProcess(MPEG4VideoDecoder *decoder, mp4_Frame *inout)
{
#ifdef MP4V_DO_POST_PROCESS
	int      w, h, i, j, k, QP, width, height;
	IppiSize size;
	Ipp8u    *pSrc[3], *pDst[3];
	int      srcPitch[3], dstPitch[3], threshold[6];
	int m_DeblockingProcPlane[3] = {1,1,1}; // TODO: configurable
	int m_DeringingProcPlane[3] = {1,1,1}; // TODO: configurable
	int32_t     m_DeblockingTHR1 = 2; // TODO
	int32_t m_DeblockingTHR2 = 6; // TODO
	QP = inout->QP;
	width = (decoder->dec.VisualObject.VideoObject.width + 7) & (~7);
	height = (decoder->dec.VisualObject.VideoObject.height + 7) & (~7);
	if (m_DeblockingProcPlane[0] || m_DeblockingProcPlane[1] || m_DeblockingProcPlane[2]) {
		pSrc[0] = inout->pY;
		srcPitch[0] = inout->stepY;
		pDst[0] = decoder->ppFrame0.pY;
		dstPitch[0] = decoder->ppFrame0.stepY;
		pSrc[1] = inout->pCb;
		srcPitch[1] = inout->stepCb;
		pDst[1] = decoder->ppFrame0.pCb;
		dstPitch[1] = decoder->ppFrame0.stepCb;
		pSrc[2] = inout->pCr;
		srcPitch[2] = inout->stepCr;
		pDst[2] = decoder->ppFrame0.pCr;
		dstPitch[2] = decoder->ppFrame0.stepCr;
		for (k = 0; k < 3; k ++) {
			if (m_DeblockingProcPlane[k]) {
				size.height = 8;
				if (k == 0) {
					size.width = width;
					h = height >> 3;
					w = width >> 3;
				} else {
					size.width = width >> 1;
					h = height >> 4;
					w = width >> 4;
				}
				for (i = 0; i < h; i ++) {
					ippiCopy_8u_C1R(pSrc[k], srcPitch[k], pDst[k], dstPitch[k], size);
					if (i > 0) {
						for (j = 0; j < w; j ++)
							ippiFilterDeblocking8x8HorEdge_MPEG4_8u_C1IR(pDst[k] + 8 * j, dstPitch[k], QP, m_DeblockingTHR1, m_DeblockingTHR2);
						for (j = 1; j < w; j ++)
							ippiFilterDeblocking8x8VerEdge_MPEG4_8u_C1IR(pDst[k] - 8 * dstPitch[k] + 8 * j, dstPitch[k], QP, m_DeblockingTHR1, m_DeblockingTHR2);
					}
					if (i == h - 1) {
						for (j = 1; j < w; j ++)
							ippiFilterDeblocking8x8VerEdge_MPEG4_8u_C1IR(pDst[k] + 8 * j, dstPitch[k], QP, m_DeblockingTHR1, m_DeblockingTHR2);
					}
					pSrc[k] += srcPitch[k] * 8;
					pDst[k] += dstPitch[k] * 8;
				}
			} else {
				if (k == 0) {
					size.width = width;
					size.height = height;
				} else {
					size.width = width >> 1;
					size.height = height >> 1;
				}
				ippiCopy_8u_C1R(pSrc[k], srcPitch[k], pDst[k], dstPitch[k], size);
			}
		}
		*inout = decoder->ppFrame0;
	}
	if (m_DeringingProcPlane[0] || m_DeringingProcPlane[1] || m_DeringingProcPlane[2]) {
		pSrc[0] = inout->pY;
		srcPitch[0] = inout->stepY;
		pDst[0] = decoder->ppFrame1.pY;
		dstPitch[0] = decoder->ppFrame1.stepY;
		if (!m_DeringingProcPlane[0]) {
			size.width = width;
			size.height = height;
			ippiCopy_8u_C1R(pSrc[0], srcPitch[0], pDst[0], dstPitch[0], size);
		}
		pSrc[1] = inout->pCb;
		srcPitch[1] = inout->stepCb;
		pDst[1] = decoder->ppFrame1.pCb;
		dstPitch[1] = decoder->ppFrame1.stepCb;
		if (!m_DeringingProcPlane[1]) {
			size.width = width >> 1;
			size.height = height >> 1;
			ippiCopy_8u_C1R(pSrc[1], srcPitch[1], pDst[1], dstPitch[1], size);
		}
		pSrc[2] = inout->pCr;
		srcPitch[2] = inout->stepCr;
		pDst[2] = decoder->ppFrame1.pCr;
		dstPitch[2] = decoder->ppFrame1.stepCr;
		if (!m_DeringingProcPlane[2]) {
			size.width = width >> 1;
			size.height = height >> 1;
			ippiCopy_8u_C1R(pSrc[2], srcPitch[2], pDst[2], dstPitch[2], size);
		}
		h = inout->mbPerCol;
		w = inout->mbPerRow;
		for (i = 0; i < h; i ++) {
			for (j = 0; j < w; j ++) {
				ippiFilterDeringingThreshold_MPEG4_8u_P3R(pSrc[0]+ 16 * j, srcPitch[0], pSrc[1] + 8 * j, srcPitch[1], pSrc[2] + 8 * j, srcPitch[2], threshold);
				// copy border macroblocks
				if (i == 0 || i == h - 1 || j == 0 || j == w - 1) {
					if (m_DeringingProcPlane[0])
						ippiCopy16x16_8u_C1R(pSrc[0] + 16 * j, srcPitch[0], pDst[0] + 16 * j, dstPitch[0]);
					if (m_DeringingProcPlane[1])
						ippiCopy8x8_8u_C1R(pSrc[1] + 8 * j, srcPitch[1], pDst[1] + 8 * j, dstPitch[1]);
					if (m_DeringingProcPlane[2])
						ippiCopy8x8_8u_C1R(pSrc[2] + 8 * j, srcPitch[2], pDst[2] + 8 * j, dstPitch[2]);
				}
				if (m_DeringingProcPlane[0]) {
					if (i != 0 && j != 0)
						ippiFilterDeringingSmooth8x8_MPEG4_8u_C1R(pSrc[0] + 16 * j, srcPitch[0], pDst[0] + 16 * j, dstPitch[0], QP, threshold[0]);
					if (i != 0 && j != w - 1)
						ippiFilterDeringingSmooth8x8_MPEG4_8u_C1R(pSrc[0] + 16 * j + 8, srcPitch[0], pDst[0] + 16 * j + 8, dstPitch[0], QP, threshold[1]);
					if (i != h - 1 && j != 0)
						ippiFilterDeringingSmooth8x8_MPEG4_8u_C1R(pSrc[0] + 16 * j + 8 * srcPitch[0], srcPitch[0], pDst[0] + 16 * j + 8 * dstPitch[0], dstPitch[0], QP, threshold[2]);
					if (i != h - 1 && j != w - 1)
						ippiFilterDeringingSmooth8x8_MPEG4_8u_C1R(pSrc[0] + 16 * j + 8 * srcPitch[0] + 8, srcPitch[0], pDst[0] + 16 * j + 8 * dstPitch[0] + 8, dstPitch[0], QP, threshold[3]);
				}
				if (i != 0 && j != 0 && i != h - 1 && j != w - 1) {
					if (m_DeringingProcPlane[1])
						ippiFilterDeringingSmooth8x8_MPEG4_8u_C1R(pSrc[1] + 8 * j, srcPitch[1], pDst[1] + 8 * j, dstPitch[1], QP, threshold[4]);
					if (m_DeringingProcPlane[2])
						ippiFilterDeringingSmooth8x8_MPEG4_8u_C1R(pSrc[2] + 8 * j, srcPitch[2], pDst[2] + 8 * j, dstPitch[2], QP, threshold[5]);
				}
			}
			pSrc[0] += srcPitch[0] * 16;
			pDst[0] += dstPitch[0] * 16;
			pSrc[1] += srcPitch[1] * 8;
			pDst[1] += dstPitch[1] * 8;
			pSrc[2] += srcPitch[2] * 8;
			pDst[2] += dstPitch[2] * 8;
		}
		*inout = decoder->ppFrame1;
	}
#endif
	return 0;
}

void MPEG4Video_DecodeFrame(mpeg4vid_decoder_t d, const void *buffer, size_t bufferlen, uint64_t time_code)
{
	int status=0;
	int code;
	MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)d;

	if (bufferlen > (MAX_CODED_FRAME-32))
		return;

	if (buffer)
	{
		memcpy(decoder->buffer, buffer, bufferlen);
		memset(decoder->buffer+bufferlen, 0, 32);
		decoder->dec.bitoff = 0;
		decoder->dec.bufptr = decoder->dec.buffer = (uint8_t *)decoder->buffer;
		decoder->dec.buflen = bufferlen;
	}

	if (!decoder->initted) // initialize off the first packet
	{
		status = InsideInit(decoder);
		if (status)
			return;
	}

	for (;;)
	{
		// Seeking the VOP start_code, and then begin the vop decoding
		if (decoder->dec.VisualObject.VideoObject.short_video_header) 
		{
			if (!mp4_SeekShortVideoStartMarker(&decoder->dec)) 
			{
				mp4_Error("Error: Failed seeking short_video_start_marker\n");
				status = 1;//UMC_ERR_SYNC;
				break;
			}
		} 
		else 
		{
			for (;;)
			{
				if (!mp4_SeekStartCodePtr(&decoder->dec)) {
					mp4_Error("Error: Failed seeking GOV or VOP Start Code");
					status = 1;//UMC_ERR_SYNC;
					break;
				}
				code = decoder->dec.bufptr[0];
				decoder->dec.bufptr++;
				// parse repeated VOS, VO and VOL headers because stream may be glued from different streams
				if (code == MP4_VISUAL_OBJECT_SEQUENCE_SC) 
				{
					if (mp4_Parse_VisualObjectSequence(&decoder->dec) != MP4_STATUS_OK) 
					{
						status = 1;//UMC_ERR_INVALID_STREAM;
						break;
					}
				}
				else if (code == MP4_VISUAL_OBJECT_SC) 
				{
					if (mp4_Parse_VisualObject(&decoder->dec) != MP4_STATUS_OK) 
					{
						status = 1;//UMC_ERR_INVALID_STREAM;
						break;
					}
				} 
				else if (code >= MP4_VIDEO_OBJECT_LAYER_MIN_SC && code <= MP4_VIDEO_OBJECT_LAYER_MAX_SC) 
				{
					// save parameters which can affect on reinit
					Ipp32s interlaced = decoder->dec.VisualObject.VideoObject.interlaced;
					Ipp32s data_partitioned = decoder->dec.VisualObject.VideoObject.data_partitioned;
					Ipp32s sprite_enable = decoder->dec.VisualObject.VideoObject.sprite_enable;
					Ipp32s width = decoder->dec.VisualObject.VideoObject.width;
					Ipp32s height = decoder->dec.VisualObject.VideoObject.height;

					// in repeated headers check only VOL header
					decoder->dec.bufptr -= 4;

					if (mp4_Parse_VideoObject(&decoder->dec) != MP4_STATUS_OK) {
						status = 1;//UMC_ERR_INVALID_STREAM;
						break;
					}
					// realloc if something was changed
					if (interlaced != decoder->dec.VisualObject.VideoObject.interlaced ||
						data_partitioned != decoder->dec.VisualObject.VideoObject.data_partitioned ||
						sprite_enable != decoder->dec.VisualObject.VideoObject.sprite_enable ||
						width != decoder->dec.VisualObject.VideoObject.width ||
						height != decoder->dec.VisualObject.VideoObject.height) 
					{
						// TODO: return a code so we can return MP4_VIDEO_OUTPUT_FORMAT_CHANGED
						if (decoder->dec.strictSyntaxCheck) 
						{
							mp4_Error("Error: Repeated VOL header is different from previous");
							status = 1;//UMC_ERR_INVALID_STREAM;
							break;
						}

						//							UnlockBuffers();
						status = FreeBuffers(decoder);
						if (status)
							break;

						status = AllocateBuffers(decoder);
						if (status)
							break;

						// free buffers for MPEG-4 post-processing
#ifdef MP4V_DO_POST_PROCESS
						free(decoder->ppFrame0.mid);
						decoder->ppFrame0.mid = 0;

						free(decoder->ppFrame1.mid);
						decoder->ppFrame1.mid = 0;
#endif

						// TODO: clear cache and display frames
					}
					// reinit quant matrix
					ippiQuantInvIntraInit_MPEG4(decoder->dec.VisualObject.VideoObject.quant_type ? decoder->dec.VisualObject.VideoObject.intra_quant_mat : NULL, decoder->dec.VisualObject.VideoObject.QuantInvIntraSpec, 8);
					ippiQuantInvInterInit_MPEG4(decoder->dec.VisualObject.VideoObject.quant_type ? decoder->dec.VisualObject.VideoObject.nonintra_quant_mat : NULL, decoder->dec.VisualObject.VideoObject.QuantInvInterSpec, 8);
				}
				else if (code == MP4_GROUP_OF_VOP_SC) 
				{
					if (mp4_Parse_GroupOfVideoObjectPlane(&decoder->dec) != MP4_STATUS_OK) 
					{
						status = 1;//UMC_ERR_INVALID_STREAM;
						break;
					}
				} 
				else if (decoder->dec.bufptr[-1] == MP4_VIDEO_OBJECT_PLANE_SC) 
				{
					break;
				}
			}
			if (status)
				break;

		}

		// parse VOP header
		if ((mp4_Parse_VideoObjectPlane(&decoder->dec)) != MP4_STATUS_OK) 
		{
			//status = UMC_WRN_INVALID_STREAM;
			status = 0;
			break;
		}

		if (decoder->resetting && decoder->dec.VisualObject.VideoObject.VideoObjectPlane.coding_type != MP4_VOP_TYPE_I) 
		{
			//UnlockBuffers();
			//return UMC_ERR_NOT_ENOUGH_DATA;
//			decoder->dec.VisualObject.vFrame = NULL;
			status = 1;//UMC_ERR_NOT_ENOUGH_DATA;
			break;
		}
		if (decoder->dec.VisualObject.VideoObject.VideoObjectPlane.coding_type == MP4_VOP_TYPE_B)
		{
			if (decoder->skip_b_frames)
			{
				decoder->dec.bufptr = decoder->dec.buffer+ decoder->dec.buflen;
				status = 1;//UMC_ERR_NOT_ENOUGH_DATA;
				break;
			} 
		}

		// decode VOP
		if ((mp4_DecodeVideoObjectPlane(&decoder->dec)) != MP4_STATUS_OK) 
		{
			status = 1;//UMC_WRN_INVALID_STREAM;
		}
		if (decoder->dec.VisualObject.cFrame)
		{
		decoder->dec.VisualObject.cFrame->timestamp = time_code;
		decoder->dec.VisualObject.cFrame->QP = decoder->dec.VisualObject.VideoObject.short_video_header ? decoder->dec.VisualObject.VideoObject.VideoObjectPlaneH263.vop_quant : decoder->dec.VisualObject.VideoObject.VideoObjectPlane.quant;
		}

		// after reset it is need to skip first B-frames
		if (decoder->dec.VisualObject.VideoObject.VOPindex < 2 && decoder->dec.VisualObject.VideoObject.VideoObjectPlane.coding_type == MP4_VOP_TYPE_B) 
		{
			status = 1;//UMC_ERR_NOT_ENOUGH_DATA;
			break;
		}

		// do not count not_coded P frames with same vop_time as reference (in AVI)

		if (decoder->dec.VisualObject.VideoObject.VideoObjectPlane.coded ||
			(
			decoder->dec.VisualObject.cFrame && 
			 (!decoder->dec.VisualObject.rFrame || (decoder->dec.VisualObject.rFrame->time != decoder->dec.VisualObject.cFrame->time)) &&
			 (!decoder->dec.VisualObject.nFrame || (decoder->dec.VisualObject.nFrame->time != decoder->dec.VisualObject.cFrame->time))
			 )
			)
		{
			decoder->dec.VisualObject.VideoObject.VOPindex++;
		}
		else
		{
			status = 1;//UMC_ERR_NOT_ENOUGH_DATA;
			break;
		}
		if (decoder->resetting && decoder->dec.VisualObject.VideoObject.VideoObjectPlane.coding_type == MP4_VOP_TYPE_I)
		{
			// TODO: ???m_time_reset = (Ipp32s)decoder->dec.VisualObject.cFrame.time;
			//decoder->dec.VisualObject.vFrame = NULL;
			decoder->resetting = 0;
		}

	}
	//	return status;
}

void MPEG4Video_GetPicture(mpeg4vid_decoder_t d, mp4_Frame **frame)
{
	int status = 0;
	MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)d;

	mp4_Frame *rendFrame = GetDisplayFrame(&decoder->dec.VisualObject);
	if (rendFrame)
	{
		uint64_t timestamp = rendFrame->timestamp;
#ifdef MP4V_DO_POST_PROCESS
		if (1 /* TODO: disable deblocking and deringing on request*/)
		{
			if (!decoder->ppFrame0.mid) 
			{
				decoder->ppFrame0.mbPerRow = decoder->dec.VisualObject.vFrame->mbPerRow;
				decoder->ppFrame0.mbPerCol = decoder->dec.VisualObject.vFrame->mbPerCol;
				status = AllocateInitFrame(&decoder->ppFrame0);
				if (status)
					return ;
			}
			if (!decoder->ppFrame1.mid) 
			{
				decoder->ppFrame1.mbPerRow = decoder->dec.VisualObject.vFrame->mbPerRow;
				decoder->ppFrame1.mbPerCol = decoder->dec.VisualObject.vFrame->mbPerCol;
				status = AllocateInitFrame(&decoder->ppFrame1);
				if (status)
					return ;
			}

			PostProcess(decoder, rendFrame);
		}
#endif
		//if (m_LastDecodedFrame.GetColorFormat() != YUV420) {
		//            m_LastDecodedFrame.Init(decoder->dec.VisualObject.VideoObject.width, decoder->dec.VisualObject.VideoObject.height, YUV420);
		//}

		// TODO ???  m_LastDecodedFrame.SetTime(pts);
		*frame = rendFrame;
		rendFrame->timestamp = timestamp;
	}
	else
			*frame = 0;
}

void MPEG4Video_ReleaseFrame(mpeg4vid_decoder_t d,  mp4_Frame *frame)
{
	MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)d;
	ReleaseFrame(&decoder->dec.VisualObject, frame);
}

static double GetAspectRatio(mp4_VideoObject *video_object)
{
	int aspect_ratio_width=1, aspect_ratio_height=1;
	switch (video_object->aspect_ratio_info) 
	{
	case MP4_ASPECT_RATIO_FORBIDDEN:
	case MP4_ASPECT_RATIO_1_1:
		aspect_ratio_width  = 1;
		aspect_ratio_height = 1;
		break;
	case MP4_ASPECT_RATIO_12_11:
		aspect_ratio_width  = 12;
		aspect_ratio_height = 11;
		break;
	case MP4_ASPECT_RATIO_10_11:
		aspect_ratio_width  = 10;
		aspect_ratio_height = 11;
		break;
	case MP4_ASPECT_RATIO_16_11:
		aspect_ratio_width  = 16;
		aspect_ratio_height = 11;
		break;
	case MP4_ASPECT_RATIO_40_33:
		aspect_ratio_width  = 40;
		aspect_ratio_height = 33;
		break;
	default:
		aspect_ratio_width  = video_object->aspect_ratio_info_par_width;
		aspect_ratio_height = video_object->aspect_ratio_info_par_height;
		break;
	}
	return (double)aspect_ratio_width / (double)aspect_ratio_height;
}

int MPEG4Video_GetOutputFormat(mpeg4vid_decoder_t d, int *width, int *height, double *aspect_ratio)
{
	MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)d;
	if (decoder && decoder->initted)
	{
		*width = decoder->dec.VisualObject.VideoObject.width;	
		*height = decoder->dec.VisualObject.VideoObject.height;
		*aspect_ratio = GetAspectRatio(&decoder->dec.VisualObject.VideoObject);
		return 0;
	}
	return 1;
}

void MPEG4Video_HurryUp(mpeg4vid_decoder_t d, int state)
{
		MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)d;
	if (decoder)
	{
		decoder->skip_b_frames = state;
	}
}
void MPEG4Video_EndOfStream(mpeg4vid_decoder_t d)
{
	MPEG4VideoDecoder *decoder = (MPEG4VideoDecoder *)d;
	if (decoder)
	{
		// TODO
	}
}