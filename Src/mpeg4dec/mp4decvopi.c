/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2001-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    Decodes I-VOPs
//
*/

#include "mp4def.h"
#include "mp4dec.h"

/*
//  decode mcbpc and set MBtype and ChromaPattern
*/
/*static*/ mp4_Status mp4_DecodeMCBPC_I(mp4_Info* pInfo, int32_t *mbType, int32_t *mbPattern)
{
    uint32_t      code;
    int32_t      type, pattern, fb;

    code = mp4_ShowBits9(pInfo, 9);
    if (code == 1) {
        type = IPPVC_MB_STUFFING;
        pattern = 0;
        fb = 9;
    } else if (code >= 64) {
        type = IPPVC_MBTYPE_INTRA;
        pattern = code >> 6;
        if (pattern >= 4) {
            pattern = 0;
            fb = 1;
        } else
            fb = 3;
    } else {
        type = IPPVC_MBTYPE_INTRA_Q;
        pattern = code >> 3;
        if (pattern >= 4) {
            pattern = 0;
            fb = 4;
        } else if (code >= 8) {
            fb = 6;
        } else {
            mp4_Error("Error: decoding MCBPC");
            return MP4_STATUS_ERROR;
        }
    }
    mp4_FlushBits(pInfo, fb);
    *mbType = type;
    *mbPattern = pattern;
    if (type == IPPVC_MBTYPE_INTRA)
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTRA);
    else if (type == IPPVC_MBTYPE_INTRA_Q)
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTRA_Q);
    return MP4_STATUS_OK;
}


/*
//  decode IVOP
*/
mp4_Status mp4_DecodeVOP_I(mp4_Info* pInfo)
{
    int32_t  quant, quantPred, dcVLC, mb_type, cbpc, cbpy, ac_pred_flag;
    int32_t  i, j, nmb, nmbgob, stepYc, stepCbc, stepCrc, stepFc[6], mbCurr, mbInVideoPacket, colNum, rowNum, mbPerRow, mbPerCol;
    uint8_t  *pFc[6];
    mp4_Status sts;

				if (pInfo->VisualObject.cFrame)
		{
			DebugBreak();
		}
		pInfo->VisualObject.cFrame = CreateFrame(&pInfo->VisualObject);
    stepYc = pInfo->VisualObject.cFrame->stepY;
    stepCbc = pInfo->VisualObject.cFrame->stepCb;
    stepCrc = pInfo->VisualObject.cFrame->stepCr;
    mbPerRow = pInfo->VisualObject.VideoObject.MacroBlockPerRow;
    mbPerCol = pInfo->VisualObject.VideoObject.MacroBlockPerCol;
    stepFc[0] = stepFc[1] = stepFc[2] = stepFc[3] = stepYc; stepFc[4] = stepCbc; stepFc[5] = stepCrc;
    pFc[0] = pInfo->VisualObject.cFrame->pY; pFc[1] = pInfo->VisualObject.cFrame->pY + 8;
    pFc[2] = pInfo->VisualObject.cFrame->pY + 8 * stepYc; pFc[3] = pInfo->VisualObject.cFrame->pY + 8 * stepYc + 8;
    pFc[4] = pInfo->VisualObject.cFrame->pCb; pFc[5] = pInfo->VisualObject.cFrame->pCr;
    nmb = pInfo->VisualObject.VideoObject.MacroBlockPerVOP;
    mbCurr = colNum = rowNum = 0;
    sts = MP4_STATUS_OK;
// decode short_video_header I-VOP
    if (pInfo->VisualObject.VideoObject.short_video_header) 
		{
        quant = pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.vop_quant;
        nmbgob = 0;
        pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number = 0;
        for (;;)
				{
            do
						{
                if (mp4_DecodeMCBPC_I(pInfo, &mb_type, &cbpc) != MP4_STATUS_OK)
                    goto Err_1;
            } while (mb_type == IPPVC_MB_STUFFING);
            if (mp4_DecodeCBPY_I(pInfo, &cbpy) != MP4_STATUS_OK)
                goto Err_1;
            if (mb_type == IPPVC_MBTYPE_INTRA_Q)
                mp4_UpdateQuant(pInfo, quant);
            if (mp4_DecodeIntraMB_SVH(pInfo, (cbpy << 2) + cbpc, quant, pFc, stepFc) != MP4_STATUS_OK)
                goto Err_1;
            colNum ++;
            if (colNum == mbPerRow) {
                colNum = 0;
                rowNum ++;
                if (rowNum == mbPerCol)
                    break;
                pFc[0] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                pFc[1] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                pFc[2] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                pFc[3] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                pFc[4] += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
                pFc[5] += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
            } 
						else
						{
                pFc[0] += 16; pFc[1] += 16; pFc[2] += 16; pFc[3] += 16; pFc[4] += 8; pFc[5] += 8;
            }
            nmbgob ++;
            if (nmbgob == pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.num_macroblocks_in_gob && pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number < (pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.num_gobs_in_vop - 1)) {
ErrRet_1:
                if (mp4_CheckDecodeGOB_SVH(pInfo) != MP4_STATUS_OK)
                    goto Err_1;
                if (!pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_header_empty)
								{
                    quant = pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.quant_scale;
                    i = pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number * pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.num_rows_in_gob;
                    mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum, (i - rowNum) * mbPerRow - colNum);
                    rowNum = i;
                    colNum = 0;
                    pFc[0] = pInfo->VisualObject.cFrame->pY + i * stepYc * 16; pFc[1] = pFc[0] + 8; pFc[2] = pFc[0] + 8 * stepYc; pFc[3] = pFc[2] + 8;
                    pFc[4] = pInfo->VisualObject.cFrame->pCb + i * stepCbc * 8; pFc[5] = pInfo->VisualObject.cFrame->pCr + i * stepCrc * 8;
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
    quant = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant;
    if (pInfo->VisualObject.VideoObject.sprite_enable != MP4_SPRITE_STATIC)
        ippsZero_8u((uint8_t*)pInfo->VisualObject.VideoObject.MBinfo, nmb * sizeof(mp4_MacroBlock));
// decode data_partitioned I-VOP
    if (pInfo->VisualObject.VideoObject.data_partitioned) 
		{
        mp4_DataPartMacroBlock *pMBdp;

        for (;;) 
				{
            // reset Intra prediction buffer on new Video_packet
            mp4_ResetIntraPredBuffer(pInfo);
            mbInVideoPacket = 0;
            pMBdp = &pInfo->VisualObject.VideoObject.DataPartBuff[mbCurr];
            // decode mb_type/cbpc/dquant/DC part
            for (;;)
						{
                if (mp4_DecodeMCBPC_I(pInfo, &mb_type, &cbpc) != MP4_STATUS_OK)
                    goto Err_2;
                if (mb_type != IPPVC_MB_STUFFING) 
								{
                    if (mbInVideoPacket == nmb - mbCurr) 
										{
                        mp4_Error("DC Marker missed");
                        goto Err_2;
                    }
                    quantPred = quant;
                    if (mb_type == IPPVC_MBTYPE_INTRA_Q)
                        mp4_UpdateQuant(pInfo, quant);
                    if (mbInVideoPacket == 0)
                        quantPred = quant;
                    dcVLC = (quantPred < mp4_DC_vlc_Threshold[pInfo->VisualObject.VideoObject.VideoObjectPlane.intra_dc_vlc_thr]) ? 1 : 0;
                    if (dcVLC) {
                        for (i = 0; i < 6; i ++) {
                            if (ippiDecodeDCIntra_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, &pMBdp->dct_dc[i], (i < 4) ? IPPVC_BLOCK_LUMA : IPPVC_BLOCK_CHROMA) != ippStsNoErr)
                                goto Err_2;
                        }
                    }
                    pMBdp->quant = (uint8_t)quant;
                    pMBdp->type = (uint8_t)mb_type;
                    pMBdp->pat = (uint8_t)cbpc;
                    pMBdp ++;
                    mbInVideoPacket ++;
                }
                if (mp4_ShowBits(pInfo, 19) == MP4_DC_MARKER)
								{
                    mp4_GetBits(pInfo, 19);
                    break;
                }
            }
            pMBdp = &pInfo->VisualObject.VideoObject.DataPartBuff[mbCurr];
            // decode ac_pred_flag/cbpy part
            for (i = 0; i < mbInVideoPacket; i ++) 
						{
                pMBdp[i].ac_pred_flag = (uint8_t)mp4_GetBit(pInfo);
                if (mp4_DecodeCBPY_I(pInfo, &cbpy) != MP4_STATUS_OK)
								{
                    if (pInfo->stopOnErr)
                        goto Err_2;
                    for (j = i + 1; j < mbInVideoPacket; j ++)
                        pMBdp[j].ac_pred_flag = 1;
                    break;
                }
                pMBdp[i].pat = (uint8_t)((cbpy << 2) + pMBdp[i].pat);
            }
            // decode AC part and reconstruct macroblocks
            for (i = 0; i < mbInVideoPacket; i ++) {
                if (colNum == 0) {
                    // reset B-prediction blocks on new row
                    mp4_ResetIntraPredBblock(pInfo);
                }
                quant = pMBdp[i].quant;
                quantPred = (i == 0) ? quant : pMBdp[i-1].quant;
                dcVLC = (quantPred < mp4_DC_vlc_Threshold[pInfo->VisualObject.VideoObject.VideoObjectPlane.intra_dc_vlc_thr]) ? 1 : 0;
                ac_pred_flag = pMBdp[i].ac_pred_flag;
                if (mp4_DecodeIntraMB_DP(pInfo, pMBdp[i].dct_dc, colNum, pMBdp[i].pat, quant, dcVLC, ac_pred_flag, pFc, stepFc) != MP4_STATUS_OK)
                    //if (!pInfo->VisualObject.VideoObject.reversible_vlc)
                        goto Err_2;
                    //else
                    //    goto Err_RVLC;
                colNum ++;
                if (colNum == mbPerRow) 
								{
                    colNum = 0;
                    rowNum ++;
                    if (rowNum == mbPerCol)
                        return sts;
                    pFc[0] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                    pFc[1] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                    pFc[2] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                    pFc[3] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                    pFc[4] += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
                    pFc[5] += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
                }
								else
								{
                    pFc[0] += 16; pFc[1] += 16; pFc[2] += 16; pFc[3] += 16; pFc[4] += 8; pFc[5] += 8;
                }
            }
            mbCurr += mbInVideoPacket;
            if (!pInfo->VisualObject.VideoObject.resync_marker_disable)
						{
                int32_t  found;
ErrRet_2:
                if (mp4_CheckDecodeVideoPacket(pInfo, &found) == MP4_STATUS_OK) 
								{
                    if (found) 
										{
                        quant = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant_scale;
                        mbCurr = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num;
                        mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum, mbCurr - rowNum * mbPerRow - colNum);
                        rowNum = mbCurr / mbPerRow;
                        colNum = mbCurr % mbPerRow;
                        pFc[0] = pInfo->VisualObject.cFrame->pY + (rowNum * stepYc + colNum) * 16; pFc[1] = pFc[0] + 8; pFc[2] = pFc[0] + stepYc * 8; pFc[3] = pFc[2] + 8;
                        pFc[4] = pInfo->VisualObject.cFrame->pCb + (rowNum * stepCbc + colNum) * 8; pFc[5] = pInfo->VisualObject.cFrame->pCr + (rowNum * stepCrc + colNum) * 8;
                    } 
										else
                        goto Err_2;
                }
								else
                    goto Err_2;
            }
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
#if 0
Err_RVLC:
        {
            uint8_t *sBufptr = pInfo->bufptr;
            int32_t sBitoff = pInfo->bitoff;

            pInfo->bitoff --;
            if (pInfo->bitoff == -1) {
                pInfo->bitoff = 7;
                pInfo->bufptr --;
            }
            // decode AC part and reconstruct macroblocks
            for (j = mbInVideoPacket - 1; j >= i; j --) {
                int32_t  lnz, quantPred, dcVLC;

                quantPred = ((j == 0) ? pMBdp[j].quant : pMBdp[j-1].quant);
                dcVLC = (quantPred < mp4_DC_vlc_Threshold[pInfo->VisualObject.VideoObject.VideoObjectPlane.intra_dc_vlc_thr]) ? 1 : 0;
                if (ippiDecodeCoeffsIntraRVLCBack_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, pDCTdp[j*64*6], &lnz, dcVLC) != ippStsNoErr)
                    break;
            }
            pInfo->bufptr = sBufptr;
            pInfo->bitoff = sBitoff;
        }
        goto ErrRet_2;
#endif
    }
// decode not data partitioned I-VOP
    else {
        int32_t  stepY = stepYc, dct_type = 0, pYoff23 = 8 * stepYc;
        for (;;) {
            // reset Intra prediction buffer on new Video_packet
            mp4_ResetIntraPredBuffer(pInfo);
            mbInVideoPacket = 0;
            // decode blocks
            for (;;) {
                if (colNum == 0) {
                    // reset B-prediction blocks on new row
                    mp4_ResetIntraPredBblock(pInfo);
                }
                if (mp4_DecodeMCBPC_I(pInfo, &mb_type, &cbpc) != MP4_STATUS_OK)
                    goto Err_3;
                if (mb_type != IPPVC_MB_STUFFING) {
                    ac_pred_flag = mp4_GetBit(pInfo);
                    if (mp4_DecodeCBPY_I(pInfo, &cbpy) != MP4_STATUS_OK)
                        goto Err_3;
                    quantPred = quant;
                    if (mb_type == IPPVC_MBTYPE_INTRA_Q)
                        mp4_UpdateQuant(pInfo, quant);
                    if (mbInVideoPacket == 0)
                        quantPred = quant;
                    dcVLC = (quantPred < mp4_DC_vlc_Threshold[pInfo->VisualObject.VideoObject.VideoObjectPlane.intra_dc_vlc_thr]) ? 1 : 0;
                    if (pInfo->VisualObject.VideoObject.interlaced) {
                        dct_type = mp4_GetBit(pInfo);
                        if (dct_type) {
                            stepY = stepYc * 2;
                            pYoff23 = stepYc;
                        } else {
                            stepY = stepYc;
                            pYoff23 = 8 * stepYc;
                        }
                        stepFc[0] = stepFc[1] = stepFc[2] = stepFc[3] = stepY;
                    }
                    pFc[2] = pFc[0] + pYoff23; pFc[3] = pFc[1] + pYoff23;
                    if (mp4_DecodeIntraMB(pInfo, colNum, (cbpy << 2) + cbpc, quant, dcVLC, ac_pred_flag, pFc, stepFc) != MP4_STATUS_OK)
                        goto Err_3;
                    mbInVideoPacket ++;
                    colNum ++;
                    if (colNum == mbPerRow) {
                        colNum = 0;
                        rowNum ++;
                        if (rowNum == mbPerCol) {
                            // skip stuffing
                            while (mp4_ShowBits9(pInfo, 9) == 1)
                                mp4_FlushBits(pInfo, 9);
                            return sts;
                        }
                        pFc[0] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                        pFc[1] += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                        pFc[4] += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
                        pFc[5] += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
                    } else {
                        pFc[0] += 16; pFc[1] += 16; pFc[4] += 8; pFc[5] += 8;
                    }
                }
                if (!pInfo->VisualObject.VideoObject.resync_marker_disable)
								{
                    int32_t  found;
ErrRet_3:
                    if (mp4_CheckDecodeVideoPacket(pInfo, &found) == MP4_STATUS_OK) 
										{
                        if (found) 
												{
                            quant = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant_scale;
                            mbCurr = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num;
                            mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum, mbCurr - rowNum * mbPerRow - colNum);
                            rowNum = mbCurr / mbPerRow;
                            colNum = mbCurr % mbPerRow;
                            pFc[0] = pInfo->VisualObject.cFrame->pY + (rowNum * stepYc + colNum) * 16; pFc[1] = pFc[0] + 8;
                            pFc[4] = pInfo->VisualObject.cFrame->pCb + (rowNum * stepCbc + colNum) * 8; pFc[5] = pInfo->VisualObject.cFrame->pCr + (rowNum * stepCrc + colNum) * 8;
                            break;
                        }
                    } 
										else
                        goto Err_3;
                }
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
}


