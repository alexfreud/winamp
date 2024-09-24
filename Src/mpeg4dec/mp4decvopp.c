/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2001-2008 Intel Corporation. All Rights Reserved.
//
//  Description:    Decodes P-VOPs
//
*/

#include "mp4def.h"
#include "mp4dec.h"

/*
// used for short_viseo_header
*/
/*static*/ mp4_Status mp4_PredictDecodeMV(mp4_Info *pInfo, mp4_MacroBlock *MBcurr, int32_t frGOB, int32_t y, int32_t x)
{
	IppMotionVector *mvLeft, *mvTop, *mvRight, *mvCurr;
	int32_t           mbInRow = pInfo->VisualObject.VideoObject.MacroBlockPerRow;

	mvCurr = MBcurr[0].mv;
	mvLeft  = MBcurr[-1].mv;
	mvTop   = MBcurr[-mbInRow].mv;
	mvRight = MBcurr[-mbInRow+1].mv;
	if (y == frGOB && x == 0) {
		mvCurr->dx = mvCurr->dy = 0;
	} else if (x == 0) {
		mvCurr->dx = mp4_Median(0, mvTop[0].dx, mvRight[0].dx);
		mvCurr->dy = mp4_Median(0, mvTop[0].dy, mvRight[0].dy);
	} else if (y == frGOB) {
		MBcurr->mv[0] = mvLeft[0];
	} else if (x == mbInRow - 1) {
		mvCurr->dx = mp4_Median(0, mvLeft[0].dx, mvTop[0].dx);
		mvCurr->dy = mp4_Median(0, mvLeft[0].dy, mvTop[0].dy);
	} else {
		mvCurr->dx = mp4_Median(mvLeft[0].dx, mvTop[0].dx, mvRight[0].dx);
		mvCurr->dy = mp4_Median(mvLeft[0].dy, mvTop[0].dy, mvRight[0].dy);
	}
	return mp4_DecodeMV(pInfo, mvCurr, 1);
}

/*
//  decode MPEG-4 PVOP
*/
mp4_Status mp4_DecodeVOP_P(mp4_Info* pInfo)
{
	__ALIGN16(int16_t, coeffMB, 64*6);
	int32_t          stepYr, stepYc, stepCbr, stepCbc, stepCrr, stepCrc, stepFc[6];
	int32_t          i, j, nmb, nmbgob, dx, dy, mbCurr, mbInVideoPacket, colNum, rowNum, mbPerRow, mbPerCol;
	uint8_t		*pYr, *pCbr, *pCrr;
	uint8_t           *pYc, *pCbc, *pCrc, *pFc[6];
	int32_t          mb_not_coded, mb_type, cbpc, cbpy, ac_pred_flag, cbpyPrev;
	int32_t          dcVLC, quant, quantPred, rvlc, scan, obmc_disable, rt, quarter_sample;
	IppiRect        limitRectL, limitRectC;
	IppMotionVector mvCur[4], mvPrev[4], mvTmp[4], mvCbCr;
	mp4_MacroBlock *pMBinfo;
	mp4_Status      sts;
#ifdef USE_NOTCODED_STATE
	uint8_t          *ncState = pInfo->VisualObject.VideoObject.ncState;
#endif
	if (!pInfo->VisualObject.cFrame)
		pInfo->VisualObject.cFrame = CreateFrame(&pInfo->VisualObject);
	mbPerRow = pInfo->VisualObject.VideoObject.MacroBlockPerRow;
	mbPerCol = pInfo->VisualObject.VideoObject.MacroBlockPerCol;
	
	stepYc = pInfo->VisualObject.cFrame->stepY;
	stepYr = pInfo->VisualObject.rFrame->stepY;
	stepCbc = pInfo->VisualObject.cFrame->stepCb;
	stepCbr = pInfo->VisualObject.rFrame->stepCb;
	stepCrc = pInfo->VisualObject.cFrame->stepCr;
	stepCrr = pInfo->VisualObject.rFrame->stepCr;
	pYc = pInfo->VisualObject.cFrame->pY;
	pCbc = pInfo->VisualObject.cFrame->pCb;
	pCrc = pInfo->VisualObject.cFrame->pCr;
	pYr = pInfo->VisualObject.rFrame->pY;
	pCbr = pInfo->VisualObject.rFrame->pCb;
	pCrr = pInfo->VisualObject.rFrame->pCr;
	stepFc[0] = stepFc[1] = stepFc[2] = stepFc[3] = stepYc; stepFc[4] = stepCbc; stepFc[5] = stepCrc;
	// Bounding rectangle for MV limitation
	limitRectL.x = - 16 * MP4_NUM_EXT_MB;
	limitRectL.y = - 16 * MP4_NUM_EXT_MB;
	limitRectL.width = pInfo->VisualObject.VideoObject.width + 16 * 2 * MP4_NUM_EXT_MB;
	limitRectL.height = pInfo->VisualObject.VideoObject.height + 16 * 2 * MP4_NUM_EXT_MB;
	pMBinfo = pInfo->VisualObject.VideoObject.MBinfo;
	// warning "variable may be used without having been initialized"
	cbpc = cbpy = mb_type = 0;
	nmb = pInfo->VisualObject.VideoObject.MacroBlockPerVOP;
	mbCurr = colNum = rowNum = 0;
	sts = MP4_STATUS_OK;
	// decode short_video_header P-VOP
	if (pInfo->VisualObject.VideoObject.short_video_header)
	{
		int32_t           frGOB;
		IppMotionVector  mvCur;

		quant = pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.vop_quant;
		nmbgob = 0;
		pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number = 0;
		frGOB = 0;
		for (;;)
		{
			do 
			{
				mb_not_coded = mp4_GetBit(pInfo);
				if (mb_not_coded)
					break;
				if (mp4_DecodeMCBPC_P(pInfo, &mb_type, &cbpc, 1) != MP4_STATUS_OK)
					goto Err_1;
			} while (mb_type == IPPVC_MB_STUFFING);
			if (mb_not_coded)
			{
#ifdef USE_NOTCODED_STATE
				if (!(*ncState)) 
				{
					*ncState = 1;
#endif
					ippiCopy16x16_8u_C1R(pYr, stepYr, pYc, stepYc);
					ippiCopy8x8_8u_C1R(pCbr, stepCbr, pCbc, stepCbc);
					ippiCopy8x8_8u_C1R(pCrr, stepCrr, pCrc, stepCrc);
#ifdef USE_NOTCODED_STATE
				}
#endif
				pMBinfo->mv[0].dx = pMBinfo->mv[0].dy = 0;
				mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_NOTCODED);
			}
			else 
			{
#ifdef USE_NOTCODED_STATE
				// clear not_coded MB state
				*ncState = 0;
#endif
				if (mb_type == IPPVC_MBTYPE_INTER4V || mb_type == IPPVC_MBTYPE_INTER4V_Q)
				{
					mp4_Error("Error: 4MV shall not be used for short_video_header");
					goto Err_1;
				}
				if (mp4_DecodeCBPY_P(pInfo, &cbpy, mb_type) != MP4_STATUS_OK)
					goto Err_1;
				if (mb_type == IPPVC_MBTYPE_INTER_Q || mb_type == IPPVC_MBTYPE_INTRA_Q)
					mp4_UpdateQuant(pInfo, quant);
				if (mb_type >= IPPVC_MBTYPE_INTRA)
				{
					pFc[0] = pYc; pFc[1] = pYc + 8; pFc[2] = pYc + 8 * stepYc; pFc[3] = pYc + 8 * stepYc + 8; pFc[4] = pCbc; pFc[5] = pCrc;
					if (mp4_DecodeIntraMB_SVH(pInfo, (cbpy << 2) + cbpc, quant, pFc, stepFc) != MP4_STATUS_OK)
						goto Err_1;
					pMBinfo->mv[0].dx = pMBinfo->mv[0].dy = 0;
				}
				else 
				{
					if (mp4_PredictDecodeMV(pInfo, pMBinfo, frGOB, rowNum, colNum) != MP4_STATUS_OK)
						goto Err_1;
					mp4_LimitMV(&pMBinfo->mv[0], &mvCur, &limitRectL, colNum * 16, rowNum * 16, 16);
					mp4_ComputeChromaMV(&mvCur, &mvCbCr);

					// decode and MC blocks
					if (cbpy)
					{
						mp4_DecodeMCInterBlock_SVH(pInfo, quant, cbpy & 8, pYr, pYc, stepYr, coeffMB, &mvCur, Err_1);
						mp4_DecodeMCInterBlock_SVH(pInfo, quant, cbpy & 4, pYr+8, pYc+8, stepYr, coeffMB, &mvCur, Err_1);
						mp4_DecodeMCInterBlock_SVH(pInfo, quant, cbpy & 2, pYr+stepYr*8, pYc+stepYc*8, stepYr, coeffMB, &mvCur, Err_1);
						mp4_DecodeMCInterBlock_SVH(pInfo, quant, cbpy & 1, pYr+8+stepYr*8, pYc+8+stepYc*8, stepYr, coeffMB, &mvCur, Err_1);
					}
					else 
					{
						mp4_Copy16x16HP_8u(pYr, stepYr, pYc, stepYc, &mvCur, 0);
						mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
						mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
						mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
						mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
					}
					mp4_DecodeMCInterBlock_SVH(pInfo, quant, cbpc & 2, pCbr, pCbc, stepCbr, coeffMB, &mvCbCr, Err_1);
					mp4_DecodeMCInterBlock_SVH(pInfo, quant, cbpc & 1, pCrr, pCrc, stepCrr, coeffMB, &mvCbCr, Err_1);
				}
			}
			pMBinfo ++;
#ifdef USE_NOTCODED_STATE
			ncState ++;
#endif
			colNum ++;
			if (colNum == mbPerRow) 
			{
				colNum = 0;
				rowNum ++;
				if (rowNum == mbPerCol)
					break;
				pYc += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
				pCbc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
				pCrc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
				pYr += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYr << 4) - stepYr;
				pCbr += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbr << 3) - stepCbr;
				pCrr += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrr << 3) - stepCrr;
			}
			else
			{
				pYc += 16; pCrc += 8; pCbc += 8;
				pYr += 16; pCrr += 8; pCbr += 8;
			}
			nmbgob ++;
			if (nmbgob == pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.num_macroblocks_in_gob && pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number < (pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.num_gobs_in_vop - 1)) 
			{
ErrRet_1:
				if (mp4_CheckDecodeGOB_SVH(pInfo) != MP4_STATUS_OK)
					goto Err_1;
				if (!pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_header_empty) 
				{
					quant = pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.quant_scale;
					i = pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number * pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.num_rows_in_gob;
					frGOB = i;
					mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum, (i - rowNum) * mbPerRow - colNum);
					rowNum = i;
					colNum = 0;
					pYc = pInfo->VisualObject.cFrame->pY + i * stepYc * 16;  pCbc = pInfo->VisualObject.cFrame->pCb + i * stepCbc * 8;  pCrc = pInfo->VisualObject.cFrame->pCr + i * stepCrc * 8;
					pYr = pInfo->VisualObject.rFrame->pY + i * stepYr * 16;  pCbr = pInfo->VisualObject.rFrame->pCb + i * stepCbr * 8;  pCrr = pInfo->VisualObject.rFrame->pCr + i * stepCrr * 8;
					pMBinfo = pInfo->VisualObject.VideoObject.MBinfo + i * mbPerRow;
#ifdef USE_NOTCODED_STATE
					ncState = pInfo->VisualObject.VideoObject.ncState + i * mbPerRow;
#endif
				}
				nmbgob = 0;
			}
		}
		mp4_AlignBits(pInfo);
		return sts;
Err_1:
		sts = MP4_STATUS_ERROR;
		if (pInfo->stopOnErr)
			return sts;
		if (!mp4_SeekGOBMarker(pInfo))
		{
			mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum,  pInfo->VisualObject.VideoObject.MacroBlockPerVOP - rowNum * mbPerRow - colNum);
			return sts;
		}
		goto ErrRet_1;
	}
	rt = pInfo->VisualObject.VideoObject.VideoObjectPlane.rounding_type;
	quarter_sample = pInfo->VisualObject.VideoObject.quarter_sample;
	obmc_disable = pInfo->VisualObject.VideoObject.obmc_disable;
	rvlc = pInfo->VisualObject.VideoObject.reversible_vlc;
	scan = pInfo->VisualObject.VideoObject.VideoObjectPlane.alternate_vertical_scan_flag ? IPPVC_SCAN_VERTICAL : IPPVC_SCAN_ZIGZAG;
	// Bounding rectangles for MV limitation
	limitRectC.x = -8 * MP4_NUM_EXT_MB;
	limitRectC.y = -8 * MP4_NUM_EXT_MB;
	limitRectC.width = (pInfo->VisualObject.VideoObject.width >> 1) + 8 * 2 * MP4_NUM_EXT_MB;
	limitRectC.height = (pInfo->VisualObject.VideoObject.height >> 1) + 8 * 2 * MP4_NUM_EXT_MB;
	quant = quantPred = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant;
	// warning "variable may be used without having been initialized"
	ac_pred_flag = cbpyPrev = 0;
	mvCbCr.dx = mvCbCr.dy = 0;
	//ippsZero_8u((uint8_t*)pInfo->VisualObject.VideoObject.MBinfo, nmb * sizeof(mp4_MacroBlock));
	// decode data_partitioned P-VOP
	if (pInfo->VisualObject.VideoObject.data_partitioned) {
		for (;;) {
			mp4_DataPartMacroBlock *pMBdp;
			int32_t  x, y;

			x = colNum;
			y = rowNum;
			// reset Intra prediction buffer on new Video_packet
			mp4_ResetIntraPredBuffer(pInfo);
			pMBdp = &pInfo->VisualObject.VideoObject.DataPartBuff[mbCurr];
			pMBinfo = &pInfo->VisualObject.VideoObject.MBinfo[mbCurr];
			mbInVideoPacket = 0;
			// decode not_coded/mb_type/cbpc/MV part
			for (;;) {
				mb_not_coded = mp4_GetBit(pInfo);
				if (mb_not_coded) {
					mb_type = IPPVC_MBTYPE_INTER;
					mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_NOTCODED);
				} else {
					if (mp4_DecodeMCBPC_P(pInfo, &mb_type, &cbpc, 1) != MP4_STATUS_OK)
						goto Err_2;
				}
				if (mb_type != IPPVC_MB_STUFFING) {
					if (mbInVideoPacket == nmb - mbCurr) {
						mp4_Error("DC Marker missed");
						goto Err_2;
					}
					pMBinfo->validPred = 1;
					if (!mb_not_coded && (mb_type <= IPPVC_MBTYPE_INTER4V)) {
						if (mb_type != IPPVC_MBTYPE_INTER4V) {
							if (mp4_PredictDecode1MV(pInfo, pMBinfo, y, x) != MP4_STATUS_OK)
								goto Err_2;
							pMBinfo->mv[1] = pMBinfo->mv[2] = pMBinfo->mv[3] = pMBinfo->mv[0];
						} else {
							if (mp4_PredictDecode4MV(pInfo, pMBinfo, y, x) != MP4_STATUS_OK)
								goto Err_2;
						}
					} else {
						mp4_Zero4MV(pMBinfo->mv);
					}
					pMBinfo->not_coded = (uint8_t)mb_not_coded;
					pMBinfo->type = (uint8_t)mb_type;
					pMBinfo ++;
					pMBdp->pat = (uint8_t)cbpc;
					pMBdp ++;
					mbInVideoPacket ++;
					x ++;
					if (x == mbPerRow) {
						x = 0;
						y ++;
					}
				}
				if (mp4_ShowBits(pInfo, 17) == MP4_MV_MARKER) {
					mp4_GetBits(pInfo, 17);
					break;
				}
			}
			pMBdp = &pInfo->VisualObject.VideoObject.DataPartBuff[mbCurr];
			pMBinfo = &pInfo->VisualObject.VideoObject.MBinfo[mbCurr];
			// decode ac_pred_flag/cbpy/dquant/IntraDC part
			for (i = 0; i < mbInVideoPacket; i ++) {
				if (!pMBinfo->not_coded) {
					mb_type = pMBinfo->type;
					if (mb_type >= IPPVC_MBTYPE_INTRA)
						pMBdp->ac_pred_flag = (uint8_t)mp4_GetBit(pInfo);
					if (mp4_DecodeCBPY_P(pInfo, &cbpy, mb_type) != MP4_STATUS_OK)
						goto Err_2;
					pMBdp->pat = (uint8_t)((cbpy << 2) + pMBdp->pat);
					quantPred = quant;
					if (mb_type == IPPVC_MBTYPE_INTER_Q || mb_type == IPPVC_MBTYPE_INTRA_Q)
						mp4_UpdateQuant(pInfo, quant);
					pMBdp->quant = (uint8_t)quant;
					if (i == 0)
						quantPred = quant;
					// decode DC coefficient of Intra blocks
					dcVLC = (quantPred < mp4_DC_vlc_Threshold[pInfo->VisualObject.VideoObject.VideoObjectPlane.intra_dc_vlc_thr]) ? 1 : 0;
					if ((mb_type >= IPPVC_MBTYPE_INTRA) && dcVLC) {
						for (j = 0; j < 6; j ++) {
							if (ippiDecodeDCIntra_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, &pMBdp->dct_dc[j], j < 4 ? IPPVC_BLOCK_LUMA : IPPVC_BLOCK_CHROMA) != ippStsNoErr)
								goto Err_2;
						}
					}
				}
				pMBdp ++;
				pMBinfo ++;
			}
			if (mbCurr + mbInVideoPacket < nmb)
				pMBinfo->type = IPPVC_MBTYPE_INTRA;  // for OBMC set first MB of the next videopacket as invalid for right MV
			pMBdp = &pInfo->VisualObject.VideoObject.DataPartBuff[mbCurr];
			pMBinfo = &pInfo->VisualObject.VideoObject.MBinfo[mbCurr];
			// decode coeffs and reconstruct macroblocks
			for (i = 0; i < mbInVideoPacket; i ++) {
				if (colNum == 0) {
					// reset B-prediction blocks on new row
					mp4_ResetIntraPredBblock(pInfo);
				}
				quant = pMBdp->quant;
				mb_type = pMBinfo->type;
				if (mb_type >= IPPVC_MBTYPE_INTRA) {
					quantPred = (i == 0) ? quant : pMBdp[-1].quant;
					dcVLC = (quantPred < mp4_DC_vlc_Threshold[pInfo->VisualObject.VideoObject.VideoObjectPlane.intra_dc_vlc_thr]) ? 1 : 0;
					pFc[0] = pYc; pFc[1] = pYc + 8; pFc[2] = pYc + 8 * stepYc; pFc[3] = pYc + 8 * stepYc + 8; pFc[4] = pCbc; pFc[5] = pCrc;
					if (mp4_DecodeIntraMB_DP(pInfo, pMBdp->dct_dc, colNum, pMBdp->pat, quant, dcVLC, pMBdp->ac_pred_flag, pFc, stepFc) != MP4_STATUS_OK)
						goto Err_2;
				} else {
					mp4_UpdateIntraPredBuffInvalid(pInfo, colNum);
					mb_not_coded = pMBinfo->not_coded;
					if (!mb_not_coded) {
						dx = colNum * 16;
						dy = rowNum * 16;
						cbpy = pMBdp->pat >> 2;
						cbpc = pMBdp->pat & 3;
						if (mb_type == IPPVC_MBTYPE_INTER4V) {
							if (quarter_sample) {
								mp4_Limit4MVQ(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 8);
								mp4_ComputeChroma4MVQ(pMBinfo->mv, &mvCbCr);
							} else {
								mp4_Limit4MV(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 8);
								mp4_ComputeChroma4MV(pMBinfo->mv, &mvCbCr);
							}
							mp4_LimitMV(&mvCbCr, &mvCbCr, &limitRectC, dx >> 1, dy >> 1, 8);
						} else {
							if (quarter_sample) {
								mp4_LimitMVQ(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 16);
								mp4_ComputeChromaMVQ(mvCur, &mvCbCr);
							} else {
								mp4_LimitMV(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 16);
								mp4_ComputeChromaMV(mvCur, &mvCbCr);
							}
							mvCur[1] = mvCur[2] = mvCur[3] = mvCur[0];
						}
					}
					if (obmc_disable) {
						if (mb_not_coded) {
							ippiCopy16x16_8u_C1R(pYr, stepYr, pYc, stepYc);
							ippiCopy8x8_8u_C1R(pCbr, stepCbr, pCbc, stepCbc);
							ippiCopy8x8_8u_C1R(pCrr, stepCrr, pCrc, stepCrc);
						} else {
							if (quarter_sample) {
								if (mb_type == IPPVC_MBTYPE_INTER4V) {
									mp4_Copy8x8QP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
									mp4_Copy8x8QP_8u(pYr+8, stepYr, pYc+8, stepYc, &mvCur[1], rt);
									mp4_Copy8x8QP_8u(pYr+8*stepYr, stepYr, pYc+8*stepYc, stepYc, &mvCur[2], rt);
									mp4_Copy8x8QP_8u(pYr+8*stepYr+8, stepYr, pYc+8*stepYc+8, stepYc, &mvCur[3], rt);
								} else
									mp4_Copy16x16QP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
								mp4_DecodeReconBlockInter_MPEG4(cbpy & 8, pYc, stepYc, Err_2);
								mp4_DecodeReconBlockInter_MPEG4(cbpy & 4, pYc+8, stepYc, Err_2);
								mp4_DecodeReconBlockInter_MPEG4(cbpy & 2, pYc+8*stepYc, stepYc, Err_2);
								mp4_DecodeReconBlockInter_MPEG4(cbpy & 1, pYc+8*stepYc+8, stepYc, Err_2);
							} else {
								if (cbpy || mb_type == IPPVC_MBTYPE_INTER4V) {
									mp4_DecodeMCBlockInter_MPEG4(cbpy & 8, pYr, stepYr, pYc, stepYc, mvCur[0], rt, Err_2);
									mp4_DecodeMCBlockInter_MPEG4(cbpy & 4, pYr+8, stepYr, pYc+8, stepYc, mvCur[1], rt, Err_2);
									mp4_DecodeMCBlockInter_MPEG4(cbpy & 2, pYr+8*stepYr, stepYr, pYc+8*stepYc, stepYc, mvCur[2], rt, Err_2);
									mp4_DecodeMCBlockInter_MPEG4(cbpy & 1, pYr+8*stepYr+8, stepYr, pYc+8*stepYc+8, stepYc, mvCur[3], rt, Err_2);
								} else {
									mp4_Copy16x16HP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
									mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
									mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
									mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
									mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
								}
							}
							mp4_DecodeMCBlockInter_MPEG4(cbpc & 2, pCbr, stepCbr, pCbc, stepCbc, mvCbCr, rt, Err_2);
							mp4_DecodeMCBlockInter_MPEG4(cbpc & 1, pCrr, stepCrr, pCrc, stepCrc, mvCbCr, rt, Err_2);
						}
					} else {
						if (mb_not_coded) {
							cbpy = 0;
							mp4_Zero4MV(mvCur);
							ippiCopy8x8_8u_C1R(pCbr, stepCbr, pCbc, stepCbc);
							ippiCopy8x8_8u_C1R(pCrr, stepCrr, pCrc, stepCrc);
						} else {
							if (mp4_DecodeInterMB(pInfo, coeffMB, quant, (cbpy << 2) + cbpc, scan) != MP4_STATUS_OK)
								goto Err_2;
							mp4_MC_HP(cbpc & 2, pCbr, stepCbr, pCbc, stepCbc, coeffMB+256, &mvCbCr, rt);
							mp4_MC_HP(cbpc & 1, pCrr, stepCrr, pCrc, stepCrc, coeffMB+320, &mvCbCr, rt);
						}
						mp4_OBMC(pInfo, pMBinfo, mvCur, colNum, rowNum, limitRectL, pYc, stepYc, pYr, stepYr, cbpy, coeffMB, 0);
					}
				}
				pMBinfo ++;
				pMBdp ++;
				colNum ++;
				if (colNum == mbPerRow) {
					colNum = 0;
					rowNum ++;
					if (rowNum == mbPerCol)
						return sts;
					pYc += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
					pCbc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
					pCrc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
					pYr += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYr << 4) - stepYr;
					pCbr += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbr << 3) - stepCbr;
					pCrr += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrr << 3) - stepCrr;
				} 
				else
				{
					pYc += 16; pCrc += 8; pCbc += 8;
					pYr += 16; pCrr += 8; pCbr += 8;
				}
			}
			//mbCurr += mbInVideoPacket;
			if (!pInfo->VisualObject.VideoObject.resync_marker_disable)
			{
				int32_t  found;
ErrRet_2:
				if (mp4_CheckDecodeVideoPacket(pInfo, &found) == MP4_STATUS_OK) 
				{
					if (found)
					{
						quant = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant_scale;
						mbInVideoPacket = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num - mbCurr;
						mbCurr = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num;
						mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum, mbCurr - rowNum * mbPerRow - colNum);
						rowNum = mbCurr / mbPerRow;
						colNum = mbCurr % mbPerRow;
						pYc = pInfo->VisualObject.cFrame->pY + (rowNum * stepYc + colNum) * 16; pCbc = pInfo->VisualObject.cFrame->pCb + (rowNum * stepCbc + colNum) * 8; pCrc = pInfo->VisualObject.cFrame->pCr + (rowNum * stepCrc + colNum) * 8;
						pYr = pInfo->VisualObject.rFrame->pY + (rowNum * stepYr + colNum) * 16; pCbr = pInfo->VisualObject.rFrame->pCb + (rowNum * stepCbr + colNum) * 8; pCrr = pInfo->VisualObject.rFrame->pCr + (rowNum * stepCrr + colNum) * 8;
						pMBinfo = pInfo->VisualObject.VideoObject.MBinfo + mbCurr;
					}
					else
						goto Err_2;
				} 
				else
					goto Err_2;
			}
			// mark MBs the previous videopacket as invalid for prediction
			for (i = 1; i <= IPP_MIN(mbInVideoPacket, mbPerRow + 1); i ++)
				pMBinfo[-i].validPred = 0;
		}
Err_2:
		sts = MP4_STATUS_ERROR;
		if (pInfo->stopOnErr)
			return sts;
		if (pInfo->VisualObject.VideoObject.resync_marker_disable || !mp4_SeekResyncMarker(pInfo)) 
		{
			mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum,  pInfo->VisualObject.VideoObject.MacroBlockPerVOP - rowNum * mbPerRow - colNum);
			return sts;
		}
		goto ErrRet_2;
	}
	// decode interlaced P-VOP
	if (pInfo->VisualObject.VideoObject.interlaced)
	{
		int32_t          stepY, pYoff23, field_prediction, dct_type, dct_typePrev, mb_ftfr, mb_fbfr;
		IppMotionVector mvCbCrT, mvCbCrB, *mvField = pInfo->VisualObject.VideoObject.FieldMV;

		// warning "variable may be used without having been initialized"
		dct_type = dct_typePrev = mb_ftfr = mb_fbfr = 0;
		mvCbCrT.dx = mvCbCrT.dy = mvCbCrB.dx = mvCbCrB.dy = 0;
		for (;;) {
			// reset Intra prediction buffer on new Video_packet
			mp4_ResetIntraPredBuffer(pInfo);
			mbInVideoPacket = 0;
			// decode blocks
			for (;;)
			{
				if (colNum == 0) 
				{
					// reset B-prediction blocks on new row
					mp4_ResetIntraPredBblock(pInfo);
				}
				mb_not_coded = mp4_GetBit(pInfo);
				mb_type = IPPVC_MBTYPE_INTER;
				if (!mb_not_coded) 
				{
					if (mp4_DecodeMCBPC_P(pInfo, &mb_type, &cbpc, 1) != MP4_STATUS_OK)
						goto Err_3;
				}
				if (mb_type != IPPVC_MB_STUFFING)
				{
					if (!mb_not_coded)
					{
						if (mb_type >= IPPVC_MBTYPE_INTRA)
							ac_pred_flag = mp4_GetBit(pInfo);
						if (mp4_DecodeCBPY_P(pInfo, &cbpy, mb_type) != MP4_STATUS_OK)
							goto Err_3;
						quantPred = quant;
						if (mb_type == IPPVC_MBTYPE_INTER_Q || mb_type == IPPVC_MBTYPE_INTRA_Q)
							mp4_UpdateQuant(pInfo, quant);
						dct_type = 0;
						field_prediction = 0;
						if (mb_type >= IPPVC_MBTYPE_INTRA || (cbpy + cbpc) != 0)
							dct_type = mp4_GetBit(pInfo);
						if (mb_type == IPPVC_MBTYPE_INTER || mb_type == IPPVC_MBTYPE_INTER_Q) {
							field_prediction = mp4_GetBit(pInfo);
							if (field_prediction) {
								mb_ftfr = mp4_GetBit(pInfo);
								mb_fbfr = mp4_GetBit(pInfo);
							}
						}
					}
					else 
					{
						mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_NOTCODED);
						mp4_Zero4MV(pMBinfo->mv);
						field_prediction = 0;
					}
					pMBinfo->validPred = 1;
					pMBinfo->not_coded = (uint8_t)mb_not_coded;
					pMBinfo->type = (uint8_t)mb_type;
					pMBinfo->field_info = (uint8_t)(field_prediction + (mb_ftfr << 1) + (mb_fbfr << 2));
					if (mb_type >= IPPVC_MBTYPE_INTRA) 
					{
						if (mbInVideoPacket == 0)
							quantPred = quant;
						dcVLC = (quantPred < mp4_DC_vlc_Threshold[pInfo->VisualObject.VideoObject.VideoObjectPlane.intra_dc_vlc_thr]) ? 1 : 0;
						if (dct_type) 
						{
							stepY = stepYc * 2;
							pYoff23 = stepYc;
						}
						else 
						{
							stepY = stepYc;
							pYoff23 = 8 * stepYc;
						}
						stepFc[0] = stepFc[1] = stepFc[2] = stepFc[3] = stepY;
						pFc[0] = pYc; pFc[1] = pYc + 8; pFc[2] = pYc + pYoff23; pFc[3] = pYc + pYoff23 + 8; pFc[4] = pCbc; pFc[5] = pCrc;
						if (mp4_DecodeIntraMB(pInfo, colNum, (cbpy << 2) + cbpc, quant, dcVLC, ac_pred_flag, pFc, stepFc) != MP4_STATUS_OK)
							goto Err_3;
						mp4_Zero4MV(pMBinfo->mv);
#ifdef USE_NOTCODED_STATE
						// clear not_coded MB state
						*ncState = 0;
#endif
					} 
					else 
					{
						mp4_UpdateIntraPredBuffInvalid(pInfo, colNum);
						if (!mb_not_coded)
						{
							dx = colNum * 16;
							dy = rowNum * 16;
							if (!field_prediction)
							{
								if (mb_type != IPPVC_MBTYPE_INTER4V)
								{
									if (mp4_PredictDecode1MV(pInfo, pMBinfo, rowNum, colNum) != MP4_STATUS_OK)
										goto Err_3;
									pMBinfo->mv[1] = pMBinfo->mv[2] = pMBinfo->mv[3] = pMBinfo->mv[0];
									if (quarter_sample)
									{
										mp4_LimitMVQ(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 16);
										mp4_ComputeChromaMVQ(mvCur, &mvCbCr);
									}
									else 
									{
										mp4_LimitMV(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 16);
										mp4_ComputeChromaMV(mvCur, &mvCbCr);
									}
									mvCur[1] = mvCur[2] = mvCur[3] = mvCur[0];
								}
								else 
								{
									if (mp4_PredictDecode4MV(pInfo, pMBinfo, rowNum, colNum) != MP4_STATUS_OK)
										goto Err_3;
									if (quarter_sample)
									{
										mp4_Limit4MVQ(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 8);
										mp4_ComputeChroma4MVQ(pMBinfo->mv, &mvCbCr);
									}
									else 
									{
										mp4_Limit4MV(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 8);
										mp4_ComputeChroma4MV(pMBinfo->mv, &mvCbCr);
									}
									mp4_LimitMV(&mvCbCr, &mvCbCr, &limitRectC, dx >> 1, dy >> 1, 8);
								}
							} 
							else 
							{
								IppMotionVector  mvFT, mvFB;
								if (mp4_PredictDecodeFMV(pInfo, pMBinfo, rowNum, colNum, &mvFT, &mvFB) != MP4_STATUS_OK)
									goto Err_3;
								mvField[0] = mvFT; mvField[1] = mvFB;
								if (quarter_sample) 
								{
									mp4_LimitFMVQ(&mvFT, &mvCur[0], &limitRectL, dx, dy, 16);
									mp4_LimitFMVQ(&mvFB, &mvCur[2], &limitRectL, dx, dy, 16);
									mvTmp[0].dx = (int16_t)mp4_Div2(mvCur[0].dx);
									mvTmp[0].dy = (int16_t)(mp4_Div2(mvCur[0].dy << 1) >> 1);
									mvTmp[2].dx = (int16_t)mp4_Div2(mvCur[2].dx);
									mvTmp[2].dy = (int16_t)(mp4_Div2(mvCur[2].dy << 1) >> 1);
									mp4_ComputeChromaMV(&mvTmp[0], &mvCbCrT);
									mp4_ComputeChromaMV(&mvTmp[2], &mvCbCrB);
								} 
								else
								{
									mp4_LimitFMV(&mvFT, &mvCur[0], &limitRectL, dx, dy, 16);
									mp4_LimitFMV(&mvFB, &mvCur[2], &limitRectL, dx, dy, 16);
									mp4_ComputeChromaMV(&mvCur[0], &mvCbCrT);
									mp4_ComputeChromaMV(&mvCur[2], &mvCbCrB);
								}
							}
						}
					}
					if (!obmc_disable)
					{
						// OBMC for previous MB
						if (colNum > 0)
							if (pMBinfo[-1].type < IPPVC_MBTYPE_INTRA && !(pMBinfo[-1].field_info & 1))
								mp4_OBMC(pInfo, pMBinfo - 1, mvPrev, colNum - 1, rowNum, limitRectL, pYc - 16, stepYc, pYr - 16, stepYr, cbpyPrev, coeffMB, dct_typePrev);
						if (mb_type < IPPVC_MBTYPE_INTRA && !field_prediction)
						{
							if (mb_not_coded) 
							{
								cbpyPrev = 0;
								mp4_Zero4MV(mvPrev);
								ippiCopy8x8_8u_C1R(pCbr, stepCbr, pCbc, stepCbc);
								ippiCopy8x8_8u_C1R(pCrr, stepCrr, pCrc, stepCrc);
							}
							else 
							{
								cbpyPrev = cbpy;
								dct_typePrev = dct_type;
								mvPrev[0] = mvCur[0]; mvPrev[1] = mvCur[1]; mvPrev[2] = mvCur[2]; mvPrev[3] = mvCur[3];
								if (mp4_DecodeInterMB(pInfo, coeffMB, quant, (cbpy << 2) + cbpc, scan) != MP4_STATUS_OK)
									goto Err_3;
								mp4_MC_HP(cbpc & 2, pCbr, stepCbr, pCbc, stepCbc, coeffMB+256, &mvCbCr, rt);
								mp4_MC_HP(cbpc & 1, pCrr, stepCrr, pCrc, stepCrc, coeffMB+320, &mvCbCr, rt);
							}
							// OBMC current MB if it is the last in the row
							if (colNum == mbPerRow - 1)
								mp4_OBMC(pInfo, pMBinfo, mvPrev, colNum, rowNum, limitRectL, pYc, stepYc, pYr, stepYr, cbpyPrev, coeffMB, dct_typePrev);
						}
					}
					if (mb_type < IPPVC_MBTYPE_INTRA && (obmc_disable || field_prediction)) 
					{
						if (mb_not_coded)
						{
#ifdef USE_NOTCODED_STATE
							if (!(*ncState)) 
							{
								*ncState = 1;
#endif
								ippiCopy16x16_8u_C1R(pYr, stepYr, pYc, stepYc);
								ippiCopy8x8_8u_C1R(pCbr, stepCbr, pCbc, stepCbc);
								ippiCopy8x8_8u_C1R(pCrr, stepCrr, pCrc, stepCrc);
#ifdef USE_NOTCODED_STATE
							}
#endif
						}
						else 
						{
#ifdef USE_NOTCODED_STATE
							// clear not_coded MB state
							*ncState = 0;
#endif
							if (mp4_DecodeInterMB(pInfo, coeffMB, quant, (cbpy << 2) + cbpc, scan) != MP4_STATUS_OK)
								goto Err_3;
							if (quarter_sample)
							{
								if (!field_prediction)
								{
									if (mb_type == IPPVC_MBTYPE_INTER4V) 
									{
										mp4_Copy8x8QP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
										mp4_Copy8x8QP_8u(pYr+8, stepYr, pYc+8, stepYc, &mvCur[1], rt);
										mp4_Copy8x8QP_8u(pYr+stepYr*8, stepYr, pYc+stepYc*8, stepYc, &mvCur[2], rt);
										mp4_Copy8x8QP_8u(pYr+8+stepYr*8, stepYr, pYc+8+stepYc*8, stepYc, &mvCur[3], rt);
									} 
									else
										mp4_Copy16x16QP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
								}
								else 
								{
									mp4_Copy16x8QP_8u(pYr+stepYr*mb_ftfr, stepYr*2, pYc, stepYc*2, &mvCur[0], rt);
									mp4_Copy16x8QP_8u(pYr+stepYr*mb_fbfr, stepYr*2, pYc+stepYc, stepYc*2, &mvCur[2], rt);
								}
							}
							else 
							{
								if (!field_prediction) 
								{
									if (mb_type == IPPVC_MBTYPE_INTER4V)
									{
										mp4_Copy8x8HP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
										mp4_Copy8x8HP_8u(pYr+8, stepYr, pYc+8, stepYc, &mvCur[1], rt);
										mp4_Copy8x8HP_8u(pYr+stepYr*8, stepYr, pYc+stepYc*8, stepYc, &mvCur[2], rt);
										mp4_Copy8x8HP_8u(pYr+8+stepYr*8, stepYr, pYc+8+stepYc*8, stepYc, &mvCur[3], rt);
									}
									else 
									{
										mp4_Copy16x16HP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
									}
								} 
								else 
								{
									mp4_Copy16x8HP_8u(pYr+stepYr*mb_ftfr, stepYr*2, pYc, stepYc*2, &mvCur[0], rt);
									mp4_Copy16x8HP_8u(pYr+stepYr*mb_fbfr, stepYr*2, pYc+stepYc, stepYc*2, &mvCur[2], rt);
								}
							}
							if (!dct_type)
							{
								mp4_AddResidual(cbpy & 8, pYc, stepYc, coeffMB);
								mp4_AddResidual(cbpy & 4, pYc+8, stepYc, coeffMB+64);
								mp4_AddResidual(cbpy & 2, pYc+stepYc*8, stepYc, coeffMB+128);
								mp4_AddResidual(cbpy & 1, pYc+stepYc*8+8, stepYc, coeffMB+192);
							} else {
								mp4_AddResidual(cbpy & 8, pYc, stepYc*2, coeffMB);
								mp4_AddResidual(cbpy & 4, pYc+8, stepYc*2, coeffMB+64);
								mp4_AddResidual(cbpy & 2, pYc+stepYc, stepYc*2, coeffMB+128);
								mp4_AddResidual(cbpy & 1, pYc+stepYc+8, stepYc*2, coeffMB+192);
							}
							if (!field_prediction) {
								mp4_MC_HP(cbpc & 2, pCbr, stepCbr, pCbc, stepCbc, coeffMB+256, &mvCbCr, rt);
								mp4_MC_HP(cbpc & 1, pCrr, stepCrr, pCrc, stepCrc, coeffMB+320, &mvCbCr, rt);
							} else {
								mp4_Copy8x4HP_8u(pCbr+(mb_ftfr ? stepCbr : 0), stepCbr*2, pCbc, stepCbc*2, &mvCbCrT, rt);
								mp4_Copy8x4HP_8u(pCrr+(mb_ftfr ? stepCrr : 0), stepCrr*2, pCrc, stepCrc*2, &mvCbCrT, rt);
								mp4_Copy8x4HP_8u(pCbr+(mb_fbfr ? stepCbr : 0), stepCbr*2, pCbc+stepCbc, stepCbc*2, &mvCbCrB, rt);
								mp4_Copy8x4HP_8u(pCrr+(mb_fbfr ? stepCrr : 0), stepCrr*2, pCrc+stepCrc, stepCrc*2, &mvCbCrB, rt);
								mp4_AddResidual(cbpc & 2, pCbc, stepCbc, coeffMB+256);
								mp4_AddResidual(cbpc & 1, pCrc, stepCrc, coeffMB+320);
							}
						}
					}
#ifdef USE_NOTCODED_STATE
					ncState ++;
#endif
					pMBinfo ++;
					mvField += 2;
					mbInVideoPacket ++;
					colNum ++;
					if (colNum == mbPerRow) {
						colNum = 0;
						rowNum ++;
						if (rowNum == mbPerCol) {
							// skip stuffing
							while (mp4_ShowBits(pInfo, 10) == 1)
								mp4_FlushBits(pInfo, 10);
							return sts;
						}
						pYc += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
						pCbc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
						pCrc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
						pYr += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYr << 4) - stepYr;
						pCbr += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbr << 3) - stepCbr;
						pCrr += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrr << 3) - stepCrr;
					} else {
						pYc += 16; pCrc += 8; pCbc += 8;
						pYr += 16; pCrr += 8; pCbr += 8;
					}
				}
				if (!pInfo->VisualObject.VideoObject.resync_marker_disable) {
					int32_t  found;
ErrRet_3:
					if (mp4_CheckDecodeVideoPacket(pInfo, &found) == MP4_STATUS_OK) {
						if (found) {
							mbInVideoPacket = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num - mbCurr;
							quant = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant_scale;
							mbCurr = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num;
							mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum, mbCurr - rowNum * mbPerRow - colNum);
							rowNum = mbCurr / mbPerRow;
							colNum = mbCurr % mbPerRow;
							pYc = pInfo->VisualObject.cFrame->pY + (rowNum * stepYc + colNum) * 16; pCbc = pInfo->VisualObject.cFrame->pCb + (rowNum * stepCbc + colNum) * 8; pCrc = pInfo->VisualObject.cFrame->pCr + (rowNum * stepCrc + colNum) * 8;
							pYr = pInfo->VisualObject.rFrame->pY + (rowNum * stepYr + colNum) * 16; pCbr = pInfo->VisualObject.rFrame->pCb + (rowNum * stepCbr + colNum) * 8; pCrr = pInfo->VisualObject.rFrame->pCr + (rowNum * stepCrr + colNum) * 8;
							pMBinfo = pInfo->VisualObject.VideoObject.MBinfo + mbCurr;
#ifdef USE_NOTCODED_STATE
							ncState = pInfo->VisualObject.VideoObject.ncState + mbCurr;
#endif
							mvField = pInfo->VisualObject.VideoObject.FieldMV + 2 * mbCurr;
							break;
						}
					} else
						goto Err_3;
				}
			}
			// mark MBs the previous videopacket as invalid for prediction
			for (i = 1; i <= IPP_MIN(mbInVideoPacket, mbPerRow + 1); i ++) {
				pMBinfo[-i].validPred = 0;
			}
		}
Err_3:
		sts = MP4_STATUS_ERROR;
		if (pInfo->stopOnErr)
			return sts;
		if (pInfo->VisualObject.VideoObject.resync_marker_disable || !mp4_SeekResyncMarker(pInfo)) 
		{
			mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum,  pInfo->VisualObject.VideoObject.MacroBlockPerVOP - rowNum * mbPerRow - colNum);
			return sts;
		}
		goto ErrRet_3;
	}
	// decode usual P-VOP
	{
		for (;;)
		{
			// reset Intra prediction buffer on new Video_packet
			mp4_ResetIntraPredBuffer(pInfo);
			mbInVideoPacket = 0;
			// decode blocks
			for (;;) 
			{
				if (colNum == 0) 
				{
					// reset B-prediction blocks on new row
					mp4_ResetIntraPredBblock(pInfo);
				}
				mb_not_coded = mp4_GetBit(pInfo);
				mb_type = IPPVC_MBTYPE_INTER;
				if (!mb_not_coded)
					if (mp4_DecodeMCBPC_P(pInfo, &mb_type, &cbpc, 1) != MP4_STATUS_OK)
						goto Err_4;
				if (mb_type != IPPVC_MB_STUFFING)
				{
					if (!mb_not_coded)
					{
						if (mb_type >= IPPVC_MBTYPE_INTRA)
							ac_pred_flag = mp4_GetBit(pInfo);
						if (mp4_DecodeCBPY_P(pInfo, &cbpy, mb_type) != MP4_STATUS_OK)
							goto Err_4;
						quantPred = quant;
						if (mb_type == IPPVC_MBTYPE_INTER_Q || mb_type == IPPVC_MBTYPE_INTRA_Q)
							mp4_UpdateQuant(pInfo, quant);
					}
					else
					{
						mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_NOTCODED);
						mp4_Zero4MV(pMBinfo->mv);
					}
					pMBinfo->validPred = 1;
					pMBinfo->not_coded = (uint8_t)mb_not_coded;
					pMBinfo->type = (uint8_t)mb_type;
					if (mb_type >= IPPVC_MBTYPE_INTRA) {
						if (mbInVideoPacket == 0)
							quantPred = quant;
						dcVLC = (quantPred < mp4_DC_vlc_Threshold[pInfo->VisualObject.VideoObject.VideoObjectPlane.intra_dc_vlc_thr]) ? 1 : 0;
						pFc[0] = pYc; pFc[1] = pYc + 8; pFc[2] = pYc + 8 * stepYc; pFc[3] = pYc + 8 * stepYc + 8; pFc[4] = pCbc; pFc[5] = pCrc;
						if (mp4_DecodeIntraMB(pInfo, colNum, (cbpy << 2) + cbpc, quant, dcVLC, ac_pred_flag, pFc, stepFc) != MP4_STATUS_OK)
							goto Err_4;
						mp4_Zero4MV(pMBinfo->mv);
#ifdef USE_NOTCODED_STATE
						// clear not_coded MB state
						*ncState = 0;
#endif
					} else {
						mp4_UpdateIntraPredBuffInvalid(pInfo, colNum);
						if (!mb_not_coded) {
							dx = colNum * 16;
							dy = rowNum * 16;
							if (mb_type != IPPVC_MBTYPE_INTER4V) {
								if (mp4_PredictDecode1MV(pInfo, pMBinfo, rowNum, colNum) != MP4_STATUS_OK)
									goto Err_4;
								pMBinfo->mv[1] = pMBinfo->mv[2] = pMBinfo->mv[3] = pMBinfo->mv[0];
								if (quarter_sample) {
									mp4_LimitMVQ(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 16);
									mp4_ComputeChromaMVQ(mvCur, &mvCbCr);
								} else {
									mp4_LimitMV(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 16);
									mp4_ComputeChromaMV(mvCur, &mvCbCr);
								}
								mvCur[1] = mvCur[2] = mvCur[3] = mvCur[0];
							} else {
								if (mp4_PredictDecode4MV(pInfo, pMBinfo, rowNum, colNum) != MP4_STATUS_OK)
									goto Err_4;
								if (quarter_sample) {
									mp4_Limit4MVQ(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 8);
									mp4_ComputeChroma4MVQ(pMBinfo->mv, &mvCbCr);
								} else {
									mp4_Limit4MV(pMBinfo->mv, mvCur, &limitRectL, dx, dy, 8);
									mp4_ComputeChroma4MV(pMBinfo->mv, &mvCbCr);
								}
								mp4_LimitMV(&mvCbCr, &mvCbCr, &limitRectC, dx >> 1, dy >> 1, 8);
							}
						}
						if (obmc_disable) {
							if (mb_not_coded) {
#ifdef USE_NOTCODED_STATE
								if (!(*ncState)) {
									*ncState = 1;
#endif
									ippiCopy16x16_8u_C1R(pYr, stepYr, pYc, stepYc);
									ippiCopy8x8_8u_C1R(pCbr, stepCbr, pCbc, stepCbc);
									ippiCopy8x8_8u_C1R(pCrr, stepCrr, pCrc, stepCrc);
#ifdef USE_NOTCODED_STATE
								}
#endif
							} else {
#ifdef USE_NOTCODED_STATE
								// clear not_coded MB state
								*ncState = 0;
#endif
								if (quarter_sample) {
									if (mb_type == IPPVC_MBTYPE_INTER4V) {
										mp4_Copy8x8QP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
										mp4_Copy8x8QP_8u(pYr+8, stepYr, pYc+8, stepYc, &mvCur[1], rt);
										mp4_Copy8x8QP_8u(pYr+8*stepYr, stepYr, pYc+8*stepYc, stepYc, &mvCur[2], rt);
										mp4_Copy8x8QP_8u(pYr+8*stepYr+8, stepYr, pYc+8*stepYc+8, stepYc, &mvCur[3], rt);
									} else
										mp4_Copy16x16QP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
									mp4_DecodeReconBlockInter_MPEG4(cbpy & 8, pYc, stepYc, Err_4);
									mp4_DecodeReconBlockInter_MPEG4(cbpy & 4, pYc+8, stepYc, Err_4);
									mp4_DecodeReconBlockInter_MPEG4(cbpy & 2, pYc+8*stepYc, stepYc, Err_4);
									mp4_DecodeReconBlockInter_MPEG4(cbpy & 1, pYc+8*stepYc+8, stepYc, Err_4);
								} else {
									if (cbpy || mb_type == IPPVC_MBTYPE_INTER4V) {
										mp4_DecodeMCBlockInter_MPEG4(cbpy & 8, pYr, stepYr, pYc, stepYc, mvCur[0], rt, Err_4);
										mp4_DecodeMCBlockInter_MPEG4(cbpy & 4, pYr+8, stepYr, pYc+8, stepYc, mvCur[1], rt, Err_4);
										mp4_DecodeMCBlockInter_MPEG4(cbpy & 2, pYr+8*stepYr, stepYr, pYc+8*stepYc, stepYc, mvCur[2], rt, Err_4);
										mp4_DecodeMCBlockInter_MPEG4(cbpy & 1, pYr+8*stepYr+8, stepYr, pYc+8*stepYc+8, stepYc, mvCur[3], rt, Err_4);
									} else {
										mp4_Copy16x16HP_8u(pYr, stepYr, pYc, stepYc, &mvCur[0], rt);
										mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
										mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
										mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
										mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
										//if (cbpy) {
										//    mp4_DecodeReconBlockInter_MPEG4(cbpy & 8, pYc, stepYc);
										//    mp4_DecodeReconBlockInter_MPEG4(cbpy & 4, pYc+8, stepYc);
										//    mp4_DecodeReconBlockInter_MPEG4(cbpy & 2, pYc+8*stepYc, stepYc);
										//    mp4_DecodeReconBlockInter_MPEG4(cbpy & 1, pYc+8*stepYc+8, stepYc);
										//}
									}
								}
								mp4_DecodeMCBlockInter_MPEG4(cbpc & 2, pCbr, stepCbr, pCbc, stepCbc, mvCbCr, rt, Err_4);
								mp4_DecodeMCBlockInter_MPEG4(cbpc & 1, pCrr, stepCrr, pCrc, stepCrc, mvCbCr, rt, Err_4);
							}
						}
					}
					if (!obmc_disable) {
						// OBMC for previous MB
						if (colNum > 0)
							if (pMBinfo[-1].type < IPPVC_MBTYPE_INTRA)
								mp4_OBMC(pInfo, pMBinfo - 1, mvPrev, colNum - 1, rowNum, limitRectL, pYc - 16, stepYc, pYr - 16, stepYr, cbpyPrev, coeffMB, 0);
						if (mb_type < IPPVC_MBTYPE_INTRA) {
							if (mb_not_coded) 
							{
								cbpyPrev = 0;
								mp4_Zero4MV(mvPrev);
								ippiCopy8x8_8u_C1R(pCbr, stepCbr, pCbc, stepCbc);
								ippiCopy8x8_8u_C1R(pCrr, stepCrr, pCrc, stepCrc);
							} 
							else 
							{
								cbpyPrev = cbpy;
								mvPrev[0] = mvCur[0]; mvPrev[1] = mvCur[1]; mvPrev[2] = mvCur[2]; mvPrev[3] = mvCur[3];
								if (mp4_DecodeInterMB(pInfo, coeffMB, quant, (cbpy << 2) + cbpc, scan) != MP4_STATUS_OK)
									goto Err_4;
								mp4_MC_HP(cbpc & 2, pCbr, stepCbr, pCbc, stepCbc, coeffMB+256, &mvCbCr, rt);
								mp4_MC_HP(cbpc & 1, pCrr, stepCrr, pCrc, stepCrc, coeffMB+320, &mvCbCr, rt);
							}
							// OBMC current MB if it is the last in the row
							if (colNum == mbPerRow - 1)
								mp4_OBMC(pInfo, pMBinfo, mvPrev, colNum, rowNum, limitRectL, pYc, stepYc, pYr, stepYr, cbpyPrev, coeffMB, 0);
						}
					}
#ifdef USE_NOTCODED_STATE
					ncState ++;
#endif
					pMBinfo ++;
					mbInVideoPacket ++;
					colNum ++;
					if (colNum == mbPerRow) 
					{
						colNum = 0;
						rowNum ++;
						if (rowNum == mbPerCol) 
						{
							// skip stuffing
							while (mp4_ShowBits(pInfo, 10) == 1)
								mp4_FlushBits(pInfo, 10);
							return sts;
						}
						pYc += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
						pCbc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
						pCrc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
						pYr += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYr << 4) - stepYr;
						pCbr += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbr << 3) - stepCbr;
						pCrr += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrr << 3) - stepCrr;
					}
					else 
					{
						pYc += 16; pCrc += 8; pCbc += 8;
						pYr += 16; pCrr += 8; pCbr += 8;
					}
				}
				if (!pInfo->VisualObject.VideoObject.resync_marker_disable) 
				{
					int32_t  found;
ErrRet_4:
					if (mp4_CheckDecodeVideoPacket(pInfo, &found) == MP4_STATUS_OK)
					{
						if (found) {
							mbInVideoPacket = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num - mbCurr;
							quant = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant_scale;
							mbCurr = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num;
							mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum, mbCurr - rowNum * mbPerRow - colNum);
							rowNum = mbCurr / mbPerRow;
							colNum = mbCurr % mbPerRow;
							pYc = pInfo->VisualObject.cFrame->pY + (rowNum * stepYc + colNum) * 16; pCbc = pInfo->VisualObject.cFrame->pCb + (rowNum * stepCbc + colNum) * 8; pCrc = pInfo->VisualObject.cFrame->pCr + (rowNum * stepCrc + colNum) * 8;
							pYr = pInfo->VisualObject.rFrame->pY + (rowNum * stepYr + colNum) * 16; pCbr = pInfo->VisualObject.rFrame->pCb + (rowNum * stepCbr + colNum) * 8; pCrr = pInfo->VisualObject.rFrame->pCr + (rowNum * stepCrr + colNum) * 8;
							pMBinfo = pInfo->VisualObject.VideoObject.MBinfo + mbCurr;
#ifdef USE_NOTCODED_STATE
							ncState = pInfo->VisualObject.VideoObject.ncState + mbCurr;
#endif
							break;
						}
					}
					else
						goto Err_4;
				}
			}
			// mark MBs the previous videopacket as invalid for prediction
			for (i = 1; i <= IPP_MIN(mbInVideoPacket, mbPerRow + 1); i ++)
			{
				pMBinfo[-i].validPred = 0;
			}
		}
Err_4:
		sts = MP4_STATUS_ERROR;
		if (pInfo->stopOnErr)
			return sts;
		if (pInfo->VisualObject.VideoObject.resync_marker_disable || !mp4_SeekResyncMarker(pInfo))
		{
			mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum,  pInfo->VisualObject.VideoObject.MacroBlockPerVOP - rowNum * mbPerRow - colNum);
			return sts;
		}
		goto ErrRet_4;
	}
}


