/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2001-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    Decodes B-VOPs
//
*/


#include "mp4def.h"
#include "mp4dec.h"

/*
//  Purpose:  decode MPEG-4 BVOP
*/
mp4_Status mp4_DecodeVOP_B(mp4_Info* pInfo)
{
    __ALIGN16(int16_t, coeffMB, 64*6);
    __ALIGN16(uint8_t, tmpMB, 64*4);
    uint32_t          code;
    uint8_t          *pYc, *pCbc, *pCrc, *pYp, *pCbp, *pCrp, *pYn, *pCbn, *pCrn, *pc, *pr, *pn;
    int32_t          stepYp, stepYc, stepYn, stepCbp, stepCbc, stepCbn, stepCrp, stepCrc, stepCrn, mbPerRow, mbPerCol;
    int32_t          dx, dy, TRB, TRD, quant, mbCurr, mbInVideoPacket, colNum, rowNum;
    IppiRect        limitRectL, limitRectC;
    int32_t          quarter_sample, modb, mb_type, cbpb, dct_type, field_prediction, rvlc = 0, scan;
    int32_t          mb_ftfr, mb_fbfr, mb_btfr, mb_bbfr, fcode_forward, fcode_backward;
    mp4_MacroBlock *pMBinfo;
    mp4_Status      sts;

    sts = MP4_STATUS_OK;

		if (!pInfo->VisualObject.cFrame)
			pInfo->VisualObject.cFrame = CreateFrame(&pInfo->VisualObject);
    stepYc = pInfo->VisualObject.cFrame->stepY;
    stepYp = pInfo->VisualObject.rFrame->stepY;
    stepYn = pInfo->VisualObject.nFrame->stepY;
    stepCbc = pInfo->VisualObject.cFrame->stepCb;
    stepCbp = pInfo->VisualObject.rFrame->stepCb;
    stepCbn = pInfo->VisualObject.nFrame->stepCb;
    stepCrc = pInfo->VisualObject.cFrame->stepCr;
    stepCrp = pInfo->VisualObject.rFrame->stepCr;
    stepCrn = pInfo->VisualObject.nFrame->stepCr;
    pYc = pInfo->VisualObject.cFrame->pY;
    pCbc = pInfo->VisualObject.cFrame->pCb;
    pCrc = pInfo->VisualObject.cFrame->pCr;
    pYp = pInfo->VisualObject.rFrame->pY;
    pCbp = pInfo->VisualObject.rFrame->pCb;
    pCrp = pInfo->VisualObject.rFrame->pCr;
    pYn = pInfo->VisualObject.nFrame->pY;
    pCbn = pInfo->VisualObject.nFrame->pCb;
    pCrn = pInfo->VisualObject.nFrame->pCr;
    quarter_sample = pInfo->VisualObject.VideoObject.quarter_sample;
    scan = pInfo->VisualObject.VideoObject.VideoObjectPlane.alternate_vertical_scan_flag ? IPPVC_SCAN_VERTICAL : IPPVC_SCAN_ZIGZAG;
    fcode_forward = pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_forward;
    fcode_backward = pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_backward;
    // Bounding rectangles for MV limitation
    limitRectL.x = - 16 * MP4_NUM_EXT_MB;
    limitRectL.y = - 16 * MP4_NUM_EXT_MB;
    limitRectL.width = pInfo->VisualObject.VideoObject.width + 16 * 2 * MP4_NUM_EXT_MB;
    limitRectL.height = pInfo->VisualObject.VideoObject.height + 16 * 2 * MP4_NUM_EXT_MB;
    limitRectC.x = -8 * MP4_NUM_EXT_MB;
    limitRectC.y = -8 * MP4_NUM_EXT_MB;
    limitRectC.width = (pInfo->VisualObject.VideoObject.width >> 1) + 8 * 2 * MP4_NUM_EXT_MB;
    limitRectC.height = (pInfo->VisualObject.VideoObject.height >> 1) + 8 * 2 * MP4_NUM_EXT_MB;
    quant = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant;
    mbPerRow = pInfo->VisualObject.VideoObject.MacroBlockPerRow;
    mbPerCol = pInfo->VisualObject.VideoObject.MacroBlockPerCol;
    mbCurr = colNum = rowNum = 0;
    TRD = pInfo->VisualObject.VideoObject.TRD;
    TRB = pInfo->VisualObject.VideoObject.TRB;
    pMBinfo = pInfo->VisualObject.VideoObject.MBinfo;
// decode interlaced B-VOP
    if (pInfo->VisualObject.VideoObject.interlaced) {
        IppMotionVector  mvCbCrF, mvCbCrB, mvForwT, mvBackT, mvForwB, mvBackB, mvForw[4], mvBack[4], mvCbCrFFT, mvCbCrFFB, mvCbCrBFT, mvCbCrBFB, *mvField = pInfo->VisualObject.VideoObject.FieldMV;

        // warning "variable may be used without having been initialized"
        mvCbCrF.dx = mvCbCrF.dy = mvCbCrB.dx = mvCbCrB.dy = mvCbCrFFT.dx = mvCbCrFFT.dy = mvCbCrFFB.dx = mvCbCrFFB.dy = mvCbCrBFT.dx = mvCbCrBFT.dy = mvCbCrBFB.dx = mvCbCrBFB.dy = 0;
        mb_ftfr = mb_fbfr = mb_btfr = mb_bbfr = 0;
        for (;;) {
            mbInVideoPacket = 0;
            // reset MV predictors at new VideoPacket
            mvForwT.dx = mvForwT.dy = mvBackT.dx = mvBackT.dy = mvForwB.dx = mvForwB.dy = mvBackB.dx = mvBackB.dy = 0;
            // decode B-VOP macroblocks
            for (;;) {
                if (pMBinfo->not_coded) {
                    ippiCopy16x16_8u_C1R(pYp, stepYp, pYc, stepYc);
                    ippiCopy8x8_8u_C1R(pCbp, stepCbp, pCbc, stepCbc);
                    ippiCopy8x8_8u_C1R(pCrp, stepCrp, pCrc, stepCrc);
                    mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_NOTCODED);
                } else {
                    cbpb = 0;
                    if (mp4_GetBit(pInfo)) {
                        modb = 2;
                        mb_type = IPPVC_MBTYPE_DIRECT;
                    } else {
                        modb = mp4_GetBit(pInfo);
                        // decode mb_type
                        code = mp4_ShowBits9(pInfo, 4);
                        if (code != 0) {
                            mb_type = mp4_BVOPmb_type[code].code;
                            mp4_FlushBits(pInfo, mp4_BVOPmb_type[code].len);
                        } else {
                            mp4_Error("Error when decode mb_type of B-VOP macroblock");
                            goto Err_1;
                        }
                        if (modb == 0)
                            cbpb = mp4_GetBits9(pInfo, 6);
                        if (mb_type != IPPVC_MBTYPE_DIRECT && cbpb != 0)
                            mp4_UpdateQuant_B(pInfo, quant);
                    }
                    dct_type = 0;
                    field_prediction = 0;
                    if (cbpb != 0)
                        dct_type = mp4_GetBit(pInfo);
                    if (mb_type != IPPVC_MBTYPE_DIRECT) {
                        field_prediction = mp4_GetBit(pInfo);
                        if (field_prediction) {
                            if (mb_type != IPPVC_MBTYPE_BACKWARD) {
                                mb_ftfr = mp4_GetBit(pInfo);
                                mb_fbfr = mp4_GetBit(pInfo);
                            }
                            if (mb_type != IPPVC_MBTYPE_FORWARD) {
                                mb_btfr = mp4_GetBit(pInfo);
                                mb_bbfr = mp4_GetBit(pInfo);
                            }
                        }
                    }
                    // coordinates of current MB for limitation
                    dx = colNum * 16;
                    dy = rowNum * 16;
                    if (mb_type == IPPVC_MBTYPE_FORWARD) {
                        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_FORWARD);
                        if (!field_prediction) {
                            if (mp4_DecodeMV(pInfo, &mvForwT, fcode_forward) != MP4_STATUS_OK)
                                goto Err_1;
                            if (quarter_sample) {
                                mp4_LimitMVQ(&mvForwT, &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_ComputeChromaMVQ(&mvForw[0], &mvCbCrF);
                                mp4_Copy16x16QP_8u(pYp, stepYp, pYc, stepYc, &mvForw[0], 0);
                            } else {
                                mp4_LimitMV(&mvForwT, &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_ComputeChromaMV(&mvForw[0], &mvCbCrF);
                                mp4_Copy16x16HP_8u(pYp, stepYp, pYc, stepYc, &mvForw[0], 0);
                            }
                            //mvForw[1] = mvForw[2] = mvForw[3] = mvForw[0];
                            mvForwB = mvForwT;
                        } else {
                            mvForwT.dy = (int16_t)mp4_Div2(mvForwT.dy);
                            if (mp4_DecodeMV(pInfo, &mvForwT, fcode_forward) != MP4_STATUS_OK)
                                goto Err_1;
                            mvForwB.dy = (int16_t)mp4_Div2(mvForwB.dy);
                            if (mp4_DecodeMV(pInfo, &mvForwB, fcode_forward) != MP4_STATUS_OK)
                                goto Err_1;
                            if (quarter_sample) {
                                mp4_LimitFMVQ(&mvForwT, &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYp+(mb_ftfr ? stepYp : 0), stepYp*2, pYc, stepYc*2, &mvForw[0], 0);
                                mvForw[0].dx = (int16_t)mp4_Div2(mvForw[0].dx);
                                mvForw[0].dy = (int16_t)(mp4_Div2(mvForw[0].dy*2) >> 1);
                                mp4_LimitFMVQ(&mvForwB, &mvForw[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYp+(mb_fbfr ? stepYp : 0), stepYp*2, pYc+stepYc, stepYc*2, &mvForw[2], 0);
                                mvForw[2].dx = (int16_t)mp4_Div2(mvForw[2].dx);
                                mvForw[2].dy = (int16_t)(mp4_Div2(mvForw[2].dy*2) >> 1);
                            } else {
                                mp4_LimitFMV(&mvForwT, &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYp+(mb_ftfr ? stepYp : 0), stepYp*2, pYc, stepYc*2, &mvForw[0], 0);
                                mp4_LimitFMV(&mvForwB, &mvForw[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYp+(mb_fbfr ? stepYp : 0), stepYp*2, pYc+stepYc, stepYc*2, &mvForw[2], 0);
                            }
                            mvForwT.dy <<= 1;
                            mvForwB.dy <<= 1;
                            //mvForw[1] = mvForw[0];
                            //mvForw[3] = mvForw[2];
                            mp4_ComputeChromaMV(&mvForw[0], &mvCbCrFFT);
                            mp4_ComputeChromaMV(&mvForw[2], &mvCbCrFFB);
                        }
                        if (mp4_DecodeInterMB(pInfo, coeffMB, quant, cbpb, scan) != MP4_STATUS_OK)
                            goto Err_1;
                        if (!dct_type) {
                            mp4_AddResidual(cbpb & 32, pYc, stepYc, coeffMB);
                            mp4_AddResidual(cbpb & 16, pYc+8, stepYc, coeffMB+64);
                            mp4_AddResidual(cbpb & 8, pYc+stepYc*8, stepYc, coeffMB+128);
                            mp4_AddResidual(cbpb & 4, pYc+stepYc*8+8, stepYc, coeffMB+192);
                        } else {
                            mp4_AddResidual(cbpb & 32, pYc, stepYc*2, coeffMB);
                            mp4_AddResidual(cbpb & 16, pYc+8, stepYc*2, coeffMB+64);
                            mp4_AddResidual(cbpb & 8, pYc+stepYc, stepYc*2, coeffMB+128);
                            mp4_AddResidual(cbpb & 4, pYc+stepYc+8, stepYc*2, coeffMB+192);
                        }
                        if (!field_prediction) {
                            mp4_MC_HP(cbpb & 2, pCbp, stepCbp, pCbc, stepCbc, coeffMB+256, &mvCbCrF, 0);
                            mp4_MC_HP(cbpb & 1, pCrp, stepCrp, pCrc, stepCrc, coeffMB+320, &mvCbCrF, 0);
                        } else {
                            mp4_Copy8x4HP_8u(pCbp+(mb_ftfr ? stepCbp : 0), stepCbp*2, pCbc, stepCbc*2, &mvCbCrFFT, 0);
                            mp4_Copy8x4HP_8u(pCrp+(mb_ftfr ? stepCrp : 0), stepCrp*2, pCrc, stepCrc*2, &mvCbCrFFT, 0);
                            mp4_Copy8x4HP_8u(pCbp+(mb_fbfr ? stepCbp : 0), stepCbp*2, pCbc+stepCbc, stepCbc*2, &mvCbCrFFB, 0);
                            mp4_Copy8x4HP_8u(pCrp+(mb_fbfr ? stepCrp : 0), stepCrp*2, pCrc+stepCrc, stepCrc*2, &mvCbCrFFB, 0);
                            mp4_AddResidual(cbpb & 2, pCbc, stepCbc, coeffMB+256);
                            mp4_AddResidual(cbpb & 1, pCrc, stepCrc, coeffMB+320);
                        }
                    } else if (mb_type == IPPVC_MBTYPE_BACKWARD) {
                        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_BACKWARD);
                        if (!field_prediction) {
                            if (mp4_DecodeMV(pInfo, &mvBackT, fcode_backward) != MP4_STATUS_OK)
                                goto Err_1;
                            if (quarter_sample) {
                                mp4_LimitMVQ(&mvBackT, &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_ComputeChromaMVQ(&mvBack[0], &mvCbCrB);
                                mp4_Copy16x16QP_8u(pYn, stepYn, pYc, stepYc, &mvBack[0], 0);
                            } else {
                                mp4_LimitMV(&mvBackT, &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_ComputeChromaMV(&mvBack[0], &mvCbCrB);
                                mp4_Copy16x16HP_8u(pYn, stepYn, pYc, stepYc, &mvBack[0], 0);
                            }
                            //mvBack[1] = mvBack[2] = mvBack[3] = mvBack[0];
                            mvBackB = mvBackT;
                        } else {
                            mvBackT.dy = (int16_t)mp4_Div2(mvBackT.dy);
                            if (mp4_DecodeMV(pInfo, &mvBackT, fcode_backward) != MP4_STATUS_OK)
                                goto Err_1;
                            mvBackB.dy = (int16_t)mp4_Div2(mvBackB.dy);
                            if (mp4_DecodeMV(pInfo, &mvBackB, fcode_backward) != MP4_STATUS_OK)
                                goto Err_1;
                            if (quarter_sample) {
                                mp4_LimitFMVQ(&mvBackT, &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYn+(mb_btfr ? stepYn : 0), stepYn*2, pYc, stepYc*2, &mvBack[0], 0);
                                mvBack[0].dx = (int16_t)mp4_Div2(mvBack[0].dx);
                                mvBack[0].dy = (int16_t)(mp4_Div2(mvBack[0].dy*2) >> 1);
                                mp4_LimitFMVQ(&mvBackB, &mvBack[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYn+(mb_bbfr ? stepYn : 0), stepYn*2, pYc+stepYc, stepYc*2, &mvBack[2], 0);
                                mvBack[2].dx = (int16_t)mp4_Div2(mvBack[2].dx);
                                mvBack[2].dy = (int16_t)(mp4_Div2(mvBack[2].dy*2) >> 1);
                            } else {
                                mp4_LimitFMV(&mvBackT, &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYn+(mb_btfr ? stepYn : 0), stepYn*2, pYc, stepYc*2, &mvBack[0], 0);
                                mp4_LimitFMV(&mvBackB, &mvBack[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYn+(mb_bbfr ? stepYn : 0), stepYn*2, pYc+stepYc, stepYc*2, &mvBack[2], 0);
                            }
                            mvBackT.dy <<= 1;
                            mvBackB.dy <<= 1;
                            //mvBack[1] = mvBack[0];
                            //mvBack[3] = mvBack[2];
                            mp4_ComputeChromaMV(&mvBack[0], &mvCbCrBFT);
                            mp4_ComputeChromaMV(&mvBack[2], &mvCbCrBFB);
                        }
                        if (mp4_DecodeInterMB(pInfo, coeffMB, quant, cbpb, scan) != MP4_STATUS_OK)
                            goto Err_1;
                        if (!dct_type) {
                            mp4_AddResidual(cbpb & 32, pYc, stepYc, coeffMB);
                            mp4_AddResidual(cbpb & 16, pYc+8, stepYc, coeffMB+64);
                            mp4_AddResidual(cbpb & 8, pYc+stepYc*8, stepYc, coeffMB+128);
                            mp4_AddResidual(cbpb & 4, pYc+stepYc*8+8, stepYc, coeffMB+192);
                        } else {
                            mp4_AddResidual(cbpb & 32, pYc, stepYc*2, coeffMB);
                            mp4_AddResidual(cbpb & 16, pYc+8, stepYc*2, coeffMB+64);
                            mp4_AddResidual(cbpb & 8, pYc+stepYc, stepYc*2, coeffMB+128);
                            mp4_AddResidual(cbpb & 4, pYc+stepYc+8, stepYc*2, coeffMB+192);
                        }
                        if (!field_prediction) {
                            mp4_MC_HP(cbpb & 2, pCbn, stepCbn, pCbc, stepCbc, coeffMB+256, &mvCbCrB, 0);
                            mp4_MC_HP(cbpb & 1, pCrn, stepCrn, pCrc, stepCrc, coeffMB+320, &mvCbCrB, 0);
                        } else {
                            mp4_Copy8x4HP_8u(pCbn+(mb_btfr ? stepCbn : 0), stepCbn*2, pCbc, stepCbc*2, &mvCbCrBFT, 0);
                            mp4_Copy8x4HP_8u(pCrn+(mb_btfr ? stepCrn : 0), stepCrn*2, pCrc, stepCrc*2, &mvCbCrBFT, 0);
                            mp4_Copy8x4HP_8u(pCbn+(mb_bbfr ? stepCbn : 0), stepCbn*2, pCbc+stepCbc, stepCbc*2, &mvCbCrBFB, 0);
                            mp4_Copy8x4HP_8u(pCrn+(mb_bbfr ? stepCrn : 0), stepCrn*2, pCrc+stepCrc, stepCrc*2, &mvCbCrBFB, 0);
                            mp4_AddResidual(cbpb & 2, pCbc, stepCbc, coeffMB+256);
                            mp4_AddResidual(cbpb & 1, pCrc, stepCrc, coeffMB+320);
                        }
                    } else if (mb_type == IPPVC_MBTYPE_INTERPOLATE) {
                        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTERPOLATE);
                        if (!field_prediction) {
                            if (mp4_DecodeMV(pInfo, &mvForwT, fcode_forward) != MP4_STATUS_OK)
                                goto Err_1;
                            if (mp4_DecodeMV(pInfo, &mvBackT, fcode_backward) != MP4_STATUS_OK)
                                goto Err_1;
                            if (quarter_sample) {
                                mp4_LimitMVQ(&mvForwT, &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_ComputeChromaMVQ(&mvForw[0], &mvCbCrF);
                                mp4_Copy16x16QP_8u(pYp, stepYp, pYc, stepYc, &mvForw[0], 0);
                                mp4_LimitMVQ(&mvBackT, &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_ComputeChromaMVQ(&mvBack[0], &mvCbCrB);
                                mp4_Copy16x16QP_8u(pYn, stepYn, tmpMB, 16, &mvBack[0], 0);
                            } else {
                                mp4_LimitMV(&mvForwT, &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_ComputeChromaMV(&mvForw[0], &mvCbCrF);
                                mp4_Copy16x16HP_8u(pYp, stepYp, pYc, stepYc, &mvForw[0], 0);
                                mp4_LimitMV(&mvBackT, &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_ComputeChromaMV(&mvBack[0], &mvCbCrB);
                                mp4_Copy16x16HP_8u(pYn, stepYn, tmpMB, 16, &mvBack[0], 0);
                            }
                            //mvForw[1] = mvForw[2] = mvForw[3] = mvForw[0];
                            mvForwB = mvForwT;
                            //mvBack[1] = mvBack[2] = mvBack[3] = mvBack[0];
                            mvBackB = mvBackT;
                        } else {
                            mvForwT.dy = (int16_t)mp4_Div2(mvForwT.dy);
                            if (mp4_DecodeMV(pInfo, &mvForwT, fcode_forward) != MP4_STATUS_OK)
                                goto Err_1;
                            mvForwB.dy = (int16_t)mp4_Div2(mvForwB.dy);
                            if (mp4_DecodeMV(pInfo, &mvForwB, fcode_forward) != MP4_STATUS_OK)
                                goto Err_1;
                            mvBackT.dy = (int16_t)mp4_Div2(mvBackT.dy);
                            if (mp4_DecodeMV(pInfo, &mvBackT, fcode_backward) != MP4_STATUS_OK)
                                goto Err_1;
                            mvBackB.dy = (int16_t)mp4_Div2(mvBackB.dy);
                            if (mp4_DecodeMV(pInfo, &mvBackB, fcode_backward) != MP4_STATUS_OK)
                                goto Err_1;
                            if (quarter_sample) {
                                mp4_LimitFMVQ(&mvForwT, &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYp+(mb_ftfr ? stepYp : 0), stepYp*2, pYc, stepYc*2, &mvForw[0], 0);
                                mvForw[0].dx = (int16_t)mp4_Div2(mvForw[0].dx);
                                mvForw[0].dy = (int16_t)(mp4_Div2(mvForw[0].dy*2) >> 1);
                                mp4_LimitFMVQ(&mvForwB, &mvForw[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYp+(mb_fbfr ? stepYp : 0), stepYp*2, pYc+stepYc, stepYc*2, &mvForw[2], 0);
                                mvForw[2].dx = (int16_t)mp4_Div2(mvForw[2].dx);
                                mvForw[2].dy = (int16_t)(mp4_Div2(mvForw[2].dy*2) >> 1);
                                mp4_LimitFMVQ(&mvBackT, &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYn+(mb_btfr ? stepYn : 0), stepYn*2, tmpMB, 32, &mvBack[0], 0);
                                mvBack[0].dx = (int16_t)mp4_Div2(mvBack[0].dx);
                                mvBack[0].dy = (int16_t)(mp4_Div2(mvBack[0].dy*2) >> 1);
                                mp4_LimitFMVQ(&mvBackB, &mvBack[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYn+(mb_bbfr ? stepYn : 0), stepYn*2, tmpMB+16, 32, &mvBack[2], 0);
                                mvBack[2].dx = (int16_t)mp4_Div2(mvBack[2].dx);
                                mvBack[2].dy = (int16_t)(mp4_Div2(mvBack[2].dy*2) >> 1);
                            } else {
                                mp4_LimitFMV(&mvForwT, &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYp+(mb_ftfr ? stepYp : 0), stepYp*2, pYc, stepYc*2, &mvForw[0], 0);
                                mp4_LimitFMV(&mvForwB, &mvForw[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYp+(mb_fbfr ? stepYp : 0), stepYp*2, pYc+stepYc, stepYc*2, &mvForw[2], 0);
                                mp4_LimitFMV(&mvBackT, &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYn+(mb_btfr ? stepYn : 0), stepYn*2, tmpMB, 32, &mvBack[0], 0);
                                mp4_LimitFMV(&mvBackB, &mvBack[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYn+(mb_bbfr ? stepYn : 0), stepYn*2, tmpMB+16, 32, &mvBack[2], 0);
                            }
                            mvForwT.dy <<= 1;
                            mvForwB.dy <<= 1;
                            mvBackT.dy <<= 1;
                            mvBackB.dy <<= 1;
                            //mvForw[1] = mvForw[0];
                            //mvForw[3] = mvForw[2];
                            //mvBack[1] = mvBack[0];
                            //mvBack[3] = mvBack[2];
                            mp4_ComputeChromaMV(&mvForw[0], &mvCbCrFFT);
                            mp4_ComputeChromaMV(&mvForw[2], &mvCbCrFFB);
                            mp4_ComputeChromaMV(&mvBack[0], &mvCbCrBFT);
                            mp4_ComputeChromaMV(&mvBack[2], &mvCbCrBFB);
                        }
                        ippiAverage16x16_8u_C1IR(tmpMB, 16, pYc, stepYc);
                        if (mp4_DecodeInterMB(pInfo, coeffMB, quant, cbpb, scan) != MP4_STATUS_OK)
                            goto Err_1;
                        if (!dct_type) {
                            mp4_AddResidual(cbpb & 32, pYc, stepYc, coeffMB);
                            mp4_AddResidual(cbpb & 16, pYc+8, stepYc, coeffMB+64);
                            mp4_AddResidual(cbpb & 8, pYc+stepYc*8, stepYc, coeffMB+128);
                            mp4_AddResidual(cbpb & 4, pYc+stepYc*8+8, stepYc, coeffMB+192);
                        } else {
                            mp4_AddResidual(cbpb & 32, pYc, stepYc*2, coeffMB);
                            mp4_AddResidual(cbpb & 16, pYc+8, stepYc*2, coeffMB+64);
                            mp4_AddResidual(cbpb & 8, pYc+stepYc, stepYc*2, coeffMB+128);
                            mp4_AddResidual(cbpb & 4, pYc+stepYc+8, stepYc*2, coeffMB+192);
                        }
                        if (!field_prediction) {
                            mp4_Copy8x8HP_8u(pCbp, stepCbp, pCbc, stepCbc, &mvCbCrF, 0);
                            mp4_Copy8x8HP_8u(pCrp, stepCrp, pCrc, stepCrc, &mvCbCrF, 0);
                            mp4_Copy8x8HP_8u(pCbn, stepCbn, tmpMB, 8, &mvCbCrB, 0);
                            mp4_Copy8x8HP_8u(pCrn, stepCrn, tmpMB+64, 8, &mvCbCrB, 0);
                        } else {
                            mp4_Copy8x4HP_8u(pCbp+(mb_ftfr ? stepCbp : 0), stepCbp*2, pCbc, stepCbc*2, &mvCbCrFFT, 0);
                            mp4_Copy8x4HP_8u(pCrp+(mb_ftfr ? stepCrp : 0), stepCrp*2, pCrc, stepCrc*2, &mvCbCrFFT, 0);
                            mp4_Copy8x4HP_8u(pCbp+(mb_fbfr ? stepCbp : 0), stepCbp*2, pCbc+stepCbc, stepCbc*2, &mvCbCrFFB, 0);
                            mp4_Copy8x4HP_8u(pCrp+(mb_fbfr ? stepCrp : 0), stepCrp*2, pCrc+stepCrc, stepCrc*2, &mvCbCrFFB, 0);
                            mp4_Copy8x4HP_8u(pCbn+(mb_btfr ? stepCbn : 0), stepCbn*2, tmpMB, 16, &mvCbCrBFT, 0);
                            mp4_Copy8x4HP_8u(pCrn+(mb_btfr ? stepCrn : 0), stepCrn*2, tmpMB+64, 16, &mvCbCrBFT, 0);
                            mp4_Copy8x4HP_8u(pCbn+(mb_bbfr ? stepCbn : 0), stepCbn*2, tmpMB+8, 16, &mvCbCrBFB, 0);
                            mp4_Copy8x4HP_8u(pCrn+(mb_bbfr ? stepCrn : 0), stepCrn*2, tmpMB+64+8, 16, &mvCbCrBFB, 0);
                        }
                        ippiAverage8x8_8u_C1IR(tmpMB, 8, pCbc, stepCbc);
                        ippiAverage8x8_8u_C1IR(tmpMB+64, 8, pCrc, stepCrc);
                        mp4_AddResidual(cbpb & 2, pCbc, stepCbc, coeffMB+256);
                        mp4_AddResidual(cbpb & 1, pCrc, stepCrc, coeffMB+320);
                    } else { // IPPVC_MBTYPE_DIRECT
                        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_DIRECT);
                        field_prediction = pMBinfo->field_info & 1;
                        if (!field_prediction) {
                            // frame direct mode
                            if (mp4_DecodeMV_Direct(pInfo, pMBinfo->mv, mvForw, mvBack, TRB, TRD, modb, pMBinfo->type) != MP4_STATUS_OK)
                                goto Err_1;
                            if (quarter_sample) {
                                mp4_ComputeChroma4MVQ(mvForw, &mvCbCrF);
                                mp4_Limit4MVQ(mvForw, mvForw, &limitRectL, dx, dy, 8);
                                mp4_ComputeChroma4MVQ(mvBack, &mvCbCrB);
                                mp4_Limit4MVQ(mvBack, mvBack, &limitRectL, dx, dy, 8);
                                pc = pYc;
                                pr = pYp;
                                pn = pYn;
                                mp4_Copy8x8QP_8u(pr, stepYp, pc, stepYc, &mvForw[0], 0);
                                mp4_Copy8x8QP_8u(pn, stepYn, tmpMB, 16, &mvBack[0], 0);
                                mp4_Copy8x8QP_8u(pr+8, stepYp, pc+8, stepYc, &mvForw[1], 0);
                                mp4_Copy8x8QP_8u(pn+8, stepYn, tmpMB+8, 16, &mvBack[1], 0);
                                pc = pYc + stepYc * 8;
                                pr = pYp + stepYp * 8;
                                pn = pYn + stepYn * 8;
                                mp4_Copy8x8QP_8u(pr, stepYp, pc, stepYc, &mvForw[2], 0);
                                mp4_Copy8x8QP_8u(pn, stepYn, tmpMB+128, 16, &mvBack[2], 0);
                                mp4_Copy8x8QP_8u(pr+8, stepYp, pc+8, stepYc, &mvForw[3], 0);
                                mp4_Copy8x8QP_8u(pn+8, stepYn, tmpMB+136, 16, &mvBack[3], 0);
                            } else {
                                mp4_ComputeChroma4MV(mvForw, &mvCbCrF);
                                mp4_Limit4MV(mvForw, mvForw, &limitRectL, dx, dy, 8);
                                mp4_ComputeChroma4MV(mvBack, &mvCbCrB);
                                mp4_Limit4MV(mvBack, mvBack, &limitRectL, dx, dy, 8);
                                pc = pYc;
                                pr = pYp;
                                pn = pYn;
                                mp4_Copy8x8HP_8u(pr, stepYp, pc, stepYc, &mvForw[0], 0);
                                mp4_Copy8x8HP_8u(pn, stepYn, tmpMB, 16, &mvBack[0], 0);
                                mp4_Copy8x8HP_8u(pr+8, stepYp, pc+8, stepYc, &mvForw[1], 0);
                                mp4_Copy8x8HP_8u(pn+8, stepYn, tmpMB+8, 16, &mvBack[1], 0);
                                pc = pYc + stepYc * 8;
                                pr = pYp + stepYp * 8;
                                pn = pYn + stepYn * 8;
                                mp4_Copy8x8HP_8u(pr, stepYp, pc, stepYc, &mvForw[2], 0);
                                mp4_Copy8x8HP_8u(pn, stepYn, tmpMB+128, 16, &mvBack[2], 0);
                                mp4_Copy8x8HP_8u(pr+8, stepYp, pc+8, stepYc, &mvForw[3], 0);
                                mp4_Copy8x8HP_8u(pn+8, stepYn, tmpMB+136, 16, &mvBack[3], 0);
                            }
                            mp4_LimitMV(&mvCbCrF, &mvCbCrF, &limitRectC, dx >> 1, dy >> 1, 8);
                            mp4_LimitMV(&mvCbCrB, &mvCbCrB, &limitRectC, dx >> 1, dy >> 1, 8);
                        } else {
                            mb_ftfr = (pMBinfo->field_info >> 1) & 1;
                            mb_fbfr = (pMBinfo->field_info >> 2) & 1;
                            if (mp4_DecodeMV_DirectField(pInfo, mb_ftfr, mb_fbfr, &mvField[0], &mvField[1], &mvForw[0], &mvForw[2], &mvBack[0], &mvBack[2], TRB, TRD, modb) != MP4_STATUS_OK)
                                goto Err_1;
                            if (quarter_sample) {
                                mp4_LimitFMVQ(&mvForw[0], &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_LimitFMVQ(&mvForw[2], &mvForw[2], &limitRectL, dx, dy, 16);
                                mp4_LimitFMVQ(&mvBack[0], &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_LimitFMVQ(&mvBack[2], &mvBack[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8QP_8u(pYp+(mb_ftfr ? stepYp : 0), stepYp*2, pYc, stepYc*2, &mvForw[0], 0);
                                mp4_Copy16x8QP_8u(pYp+(mb_fbfr ? stepYp : 0), stepYp*2, pYc+stepYc, stepYc*2, &mvForw[2], 0);
                                mp4_Copy16x8QP_8u(pYn/*+stepYn*mb_btfr*/, stepYn*2, tmpMB, 32, &mvBack[0], 0);
                                mp4_Copy16x8QP_8u(pYn+stepYn/**mb_bbfr*/, stepYn*2, tmpMB+16, 32, &mvBack[2], 0);
                                mvForw[0].dx = (int16_t)mp4_Div2(mvForw[0].dx);
                                mvForw[0].dy = (int16_t)(mp4_Div2(mvForw[0].dy*2) >> 1);
                                mvForw[2].dx = (int16_t)mp4_Div2(mvForw[2].dx);
                                mvForw[2].dy = (int16_t)(mp4_Div2(mvForw[2].dy*2) >> 1);
                                mvBack[0].dx = (int16_t)mp4_Div2(mvBack[0].dx);
                                mvBack[0].dy = (int16_t)(mp4_Div2(mvBack[0].dy*2) >> 1);
                                mvBack[2].dx = (int16_t)mp4_Div2(mvBack[2].dx);
                                mvBack[2].dy = (int16_t)(mp4_Div2(mvBack[2].dy*2) >> 1);
                            } else {
                                mp4_LimitFMV(&mvForw[0], &mvForw[0], &limitRectL, dx, dy, 16);
                                mp4_LimitFMV(&mvForw[2], &mvForw[2], &limitRectL, dx, dy, 16);
                                mp4_LimitFMV(&mvBack[0], &mvBack[0], &limitRectL, dx, dy, 16);
                                mp4_LimitFMV(&mvBack[2], &mvBack[2], &limitRectL, dx, dy, 16);
                                mp4_Copy16x8HP_8u(pYp+(mb_ftfr ? stepYp : 0), stepYp*2, pYc, stepYc*2, &mvForw[0], 0);
                                mp4_Copy16x8HP_8u(pYp+(mb_fbfr ? stepYp : 0), stepYp*2, pYc+stepYc, stepYc*2, &mvForw[2], 0);
                                mp4_Copy16x8HP_8u(pYn/*+stepYn*mb_btfr*/, stepYn*2, tmpMB, 32, &mvBack[0], 0);
                                mp4_Copy16x8HP_8u(pYn+stepYn/**mb_bbfr*/, stepYn*2, tmpMB+16, 32, &mvBack[2], 0);
                            }
                            mp4_ComputeChromaMV(&mvForw[0], &mvCbCrFFT);
                            mp4_ComputeChromaMV(&mvForw[2], &mvCbCrFFB);
                            mp4_ComputeChromaMV(&mvBack[0], &mvCbCrBFT);
                            mp4_ComputeChromaMV(&mvBack[2], &mvCbCrBFB);
                        }
                        ippiAverage16x16_8u_C1IR(tmpMB, 16, pYc, stepYc);
                        if (mp4_DecodeInterMB(pInfo, coeffMB, quant, cbpb, scan) != MP4_STATUS_OK)
                            goto Err_1;
                        if (!dct_type) {
                            mp4_AddResidual(cbpb & 32, pYc, stepYc, coeffMB);
                            mp4_AddResidual(cbpb & 16, pYc+8, stepYc, coeffMB+64);
                            mp4_AddResidual(cbpb & 8, pYc+stepYc*8, stepYc, coeffMB+128);
                            mp4_AddResidual(cbpb & 4, pYc+stepYc*8+8, stepYc, coeffMB+192);
                        } else {
                            mp4_AddResidual(cbpb & 32, pYc, stepYc*2, coeffMB);
                            mp4_AddResidual(cbpb & 16, pYc+8, stepYc*2, coeffMB+64);
                            mp4_AddResidual(cbpb & 8, pYc+stepYc, stepYc*2, coeffMB+128);
                            mp4_AddResidual(cbpb & 4, pYc+stepYc+8, stepYc*2, coeffMB+192);
                        }
                        if (!field_prediction) {
                            mp4_Copy8x8HP_8u(pCbp, stepCbp, pCbc, stepCbc, &mvCbCrF, 0);
                            mp4_Copy8x8HP_8u(pCrp, stepCrp, pCrc, stepCrc, &mvCbCrF, 0);
                            mp4_Copy8x8HP_8u(pCbn, stepCbn, tmpMB, 8, &mvCbCrB, 0);
                            mp4_Copy8x8HP_8u(pCrn, stepCrn, tmpMB+64, 8, &mvCbCrB, 0);
                        } else {
                            mp4_Copy8x4HP_8u(pCbp+(mb_ftfr ? stepCbp : 0), stepCbp*2, pCbc, stepCbc*2, &mvCbCrFFT, 0);
                            mp4_Copy8x4HP_8u(pCrp+(mb_ftfr ? stepCrp : 0), stepCrp*2, pCrc, stepCrc*2, &mvCbCrFFT, 0);
                            mp4_Copy8x4HP_8u(pCbp+(mb_fbfr ? stepCbp : 0), stepCbp*2, pCbc+stepCbc, stepCbc*2, &mvCbCrFFB, 0);
                            mp4_Copy8x4HP_8u(pCrp+(mb_fbfr ? stepCrp : 0), stepCrp*2, pCrc+stepCrc, stepCrc*2, &mvCbCrFFB, 0);
                            mp4_Copy8x4HP_8u(pCbn/*+(mb_btfr ? stepCbn : 0)*/, stepCbn*2, tmpMB, 16, &mvCbCrBFT, 0);
                            mp4_Copy8x4HP_8u(pCrn/*+(mb_btfr ? stepCrn : 0)*/, stepCrn*2, tmpMB+64, 16, &mvCbCrBFT, 0);
                            mp4_Copy8x4HP_8u(pCbn+/*(mb_bbfr ? */stepCbn/* : 0)*/, stepCbn*2, tmpMB+8, 16, &mvCbCrBFB, 0);
                            mp4_Copy8x4HP_8u(pCrn+/*(mb_bbfr ? */stepCrn/* : 0)*/, stepCrn*2, tmpMB+64+8, 16, &mvCbCrBFB, 0);
                        }
                        ippiAverage8x8_8u_C1IR(tmpMB, 8, pCbc, stepCbc);
                        ippiAverage8x8_8u_C1IR(tmpMB+64, 8, pCrc, stepCrc);
                        mp4_AddResidual(cbpb & 2, pCbc, stepCbc, coeffMB+256);
                        mp4_AddResidual(cbpb & 1, pCrc, stepCrc, coeffMB+320);
                    }
                }
                //mbCurr ++;
                mbInVideoPacket ++;
                colNum ++;
                pMBinfo ++;
                mvField += 2;
                if (colNum == mbPerRow) {
                    colNum = 0;
                    rowNum ++;
                    if (rowNum == mbPerCol)
                        return sts;
                    pYc += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                    pCbc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
                    pCrc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
                    pYp += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYp << 4) - stepYp;
                    pCbp += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbp << 3) - stepCbp;
                    pCrp += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrp << 3) - stepCrp;
                    pYn += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYn << 4) - stepYn;
                    pCbn += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbn << 3) - stepCbn;
                    pCrn += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrn << 3) - stepCrn;
                    // reset MV predictors at new row
                    mvForwT.dx = mvForwT.dy = mvBackT.dx = mvBackT.dy = mvForwB.dx = mvForwB.dy = mvBackB.dx = mvBackB.dy = 0;
                } else {
                    pYc += 16; pCrc += 8; pCbc += 8;
                    pYp += 16; pCrp += 8; pCbp += 8;
                    pYn += 16; pCrn += 8; pCbn += 8;
                }
                if (!pInfo->VisualObject.VideoObject.resync_marker_disable) {
                    int32_t  found;
ErrRet_1:
                    if (mp4_CheckDecodeVideoPacket(pInfo, &found) == MP4_STATUS_OK) {
                        if (found) {
                            quant = pInfo->VisualObject.VideoObject.VideoObjectPlane.quant_scale;
                            mbCurr = pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num;
                            mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum, mbCurr - rowNum * mbPerRow - colNum);
                            rowNum = mbCurr / mbPerRow;
                            colNum = mbCurr % mbPerRow;
                            pYc = pInfo->VisualObject.cFrame->pY + (rowNum * stepYc + colNum) * 16; pCbc = pInfo->VisualObject.cFrame->pCb + (rowNum * stepCbc + colNum) * 8; pCrc = pInfo->VisualObject.cFrame->pCr + (rowNum * stepCrc + colNum) * 8;
                            pYp = pInfo->VisualObject.rFrame->pY + (rowNum * stepYp + colNum) * 16; pCbp = pInfo->VisualObject.rFrame->pCb + (rowNum * stepCbp + colNum) * 8; pCrp = pInfo->VisualObject.rFrame->pCr + (rowNum * stepCrp + colNum) * 8;
                            pYn = pInfo->VisualObject.nFrame->pY + (rowNum * stepYn + colNum) * 16; pCbn = pInfo->VisualObject.nFrame->pCb + (rowNum * stepCbn + colNum) * 8; pCrn = pInfo->VisualObject.nFrame->pCr + (rowNum * stepCrn + colNum) * 8;
                            pMBinfo = pInfo->VisualObject.VideoObject.MBinfo + mbCurr;
                            break;
                        }
                    } else
                        goto Err_1;
                }
            }
        }
Err_1:
        sts = MP4_STATUS_ERROR;
        if (pInfo->stopOnErr)
            return sts;
        if (pInfo->VisualObject.VideoObject.resync_marker_disable || !mp4_SeekResyncMarker(pInfo)) 
				{
            mp4_CopyMacroBlocks(pInfo->VisualObject.rFrame, pInfo->VisualObject.cFrame, mbPerRow, rowNum, colNum,  pInfo->VisualObject.VideoObject.MacroBlockPerVOP - rowNum * mbPerRow - colNum);
            return sts;
        }
        goto ErrRet_1;
    }
// decode usual B-VOP
    for (;;) {
        IppMotionVector mvCbCrF, mvCbCrB, mvForw, mvBack, mvForwLim, mvBackLim;

        mbInVideoPacket = 0;
        // reset MV predictors at new VideoPacket
        mvForw.dx = mvForw.dy = mvBack.dx = mvBack.dy = 0;
        // decode B-VOP macroblocks
        for (;;) {
            if (pMBinfo->not_coded) {
                ippiCopy16x16_8u_C1R(pYp, stepYp, pYc, stepYc);
                ippiCopy8x8_8u_C1R(pCbp, stepCbp, pCbc, stepCbc);
                ippiCopy8x8_8u_C1R(pCrp, stepCrp, pCrc, stepCrc);
                mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_NOTCODED);
            } else {
                cbpb = 0;
                if (mp4_GetBit(pInfo)) {
                    modb = 2;
                    mb_type = IPPVC_MBTYPE_DIRECT;
                } else {
                    modb = mp4_GetBit(pInfo);
                    // decode mb_type
                    code = mp4_ShowBits9(pInfo, 4);
                    if (code != 0) {
                        mb_type = mp4_BVOPmb_type[code].code;
                        mp4_FlushBits(pInfo, mp4_BVOPmb_type[code].len);
                    } else {
                        mp4_Error("Error when decode mb_type of B-VOP macroblock");
                        goto Err_2;
                    }
                    if (modb == 0)
                        cbpb = mp4_GetBits9(pInfo, 6);
                    if (mb_type != IPPVC_MBTYPE_DIRECT && cbpb != 0)
                        mp4_UpdateQuant_B(pInfo, quant);
                }
                // coordinates of current MB for limitation
                dx = colNum * 16;
                dy = rowNum * 16;
                if (mb_type == IPPVC_MBTYPE_FORWARD) {
                    mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_FORWARD);
                    if (mp4_DecodeMV(pInfo, &mvForw, fcode_forward) != MP4_STATUS_OK)
                        goto Err_2;
                    if (quarter_sample) {
                        mp4_LimitMVQ(&mvForw, &mvForwLim, &limitRectL, dx, dy, 16);
                        mp4_ComputeChromaMVQ(&mvForwLim, &mvCbCrF);
                        mp4_Copy16x16QP_8u(pYp, stepYp, pYc, stepYc, &mvForwLim, 0);
                        mp4_DecodeReconBlockInter_MPEG4(cbpb & 32, pYc, stepYc, Err_2);
                        mp4_DecodeReconBlockInter_MPEG4(cbpb & 16, pYc+8, stepYc, Err_2);
                        mp4_DecodeReconBlockInter_MPEG4(cbpb & 8, pYc+8*stepYc, stepYc, Err_2);
                        mp4_DecodeReconBlockInter_MPEG4(cbpb & 4, pYc+8*stepYc+8, stepYc, Err_2);
                    } else {
                        mp4_LimitMV(&mvForw, &mvForwLim, &limitRectL, dx, dy, 16);
                        mp4_ComputeChromaMV(&mvForwLim, &mvCbCrF);
                        if (cbpb & 60) {
                            mp4_DecodeMCBlockInter_MPEG4(cbpb & 32, pYp, stepYp, pYc, stepYc, mvForwLim, 0, Err_2);
                            mp4_DecodeMCBlockInter_MPEG4(cbpb & 16, pYp+8, stepYp, pYc+8, stepYc, mvForwLim, 0, Err_2);
                            mp4_DecodeMCBlockInter_MPEG4(cbpb & 8, pYp+8*stepYp, stepYp, pYc+8*stepYc, stepYc, mvForwLim, 0, Err_2);
                            mp4_DecodeMCBlockInter_MPEG4(cbpb & 4, pYp+8*stepYp+8, stepYp, pYc+8*stepYc+8, stepYc, mvForwLim, 0, Err_2);
                        } else {
                            mp4_Copy16x16HP_8u(pYp, stepYp, pYc, stepYc, &mvForwLim, 0);
                            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
                            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
                            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
                            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
                        }
                    }
                    mp4_DecodeMCBlockInter_MPEG4(cbpb & 2, pCbp, stepCbp, pCbc, stepCbc, mvCbCrF, 0, Err_2);
                    mp4_DecodeMCBlockInter_MPEG4(cbpb & 1, pCrp, stepCrp, pCrc, stepCrc, mvCbCrF, 0, Err_2);
                } else if (mb_type == IPPVC_MBTYPE_BACKWARD) {
                    mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_BACKWARD);
                    if (mp4_DecodeMV(pInfo, &mvBack, fcode_backward) != MP4_STATUS_OK)
                        goto Err_2;
                    if (quarter_sample) {
                        mp4_LimitMVQ(&mvBack, &mvBackLim, &limitRectL, dx, dy, 16);
                        mp4_ComputeChromaMVQ(&mvBackLim, &mvCbCrB);
                        mp4_Copy16x16QP_8u(pYn, stepYn, pYc, stepYc, &mvBackLim, 0);
                        mp4_DecodeReconBlockInter_MPEG4(cbpb & 32, pYc, stepYc, Err_2);
                        mp4_DecodeReconBlockInter_MPEG4(cbpb & 16, pYc+8, stepYc, Err_2);
                        mp4_DecodeReconBlockInter_MPEG4(cbpb & 8, pYc+8*stepYc, stepYc, Err_2);
                        mp4_DecodeReconBlockInter_MPEG4(cbpb & 4, pYc+8*stepYc+8, stepYc, Err_2);
                    } else {
                        mp4_LimitMV(&mvBack, &mvBackLim, &limitRectL, dx, dy, 16);
                        mp4_ComputeChromaMV(&mvBackLim, &mvCbCrB);
                        if (cbpb & 60) {
                            mp4_DecodeMCBlockInter_MPEG4(cbpb & 32, pYn, stepYn, pYc, stepYc, mvBackLim, 0, Err_2);
                            mp4_DecodeMCBlockInter_MPEG4(cbpb & 16, pYn+8, stepYn, pYc+8, stepYc, mvBackLim, 0, Err_2);
                            mp4_DecodeMCBlockInter_MPEG4(cbpb & 8, pYn+8*stepYn, stepYp, pYc+8*stepYc, stepYc, mvBackLim, 0, Err_2);
                            mp4_DecodeMCBlockInter_MPEG4(cbpb & 4, pYn+8*stepYn+8, stepYp, pYc+8*stepYc+8, stepYc, mvBackLim, 0, Err_2);
                        } else {
                            mp4_Copy16x16HP_8u(pYn, stepYn, pYc, stepYc, &mvBackLim, 0);
                            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
                            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
                            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
                            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
                        }
                    }
                    mp4_DecodeMCBlockInter_MPEG4(cbpb & 2, pCbn, stepCbn, pCbc, stepCbc, mvCbCrB, 0, Err_2);
                    mp4_DecodeMCBlockInter_MPEG4(cbpb & 1, pCrn, stepCrn, pCrc, stepCrc, mvCbCrB, 0, Err_2);
                } 
								else if (mb_type == IPPVC_MBTYPE_INTERPOLATE)
								{
                    mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTERPOLATE);
                    if (mp4_DecodeMV(pInfo, &mvForw, fcode_forward) != MP4_STATUS_OK)
                        goto Err_2;
                    if (mp4_DecodeMV(pInfo, &mvBack, fcode_backward) != MP4_STATUS_OK)
                        goto Err_2;
                    if (quarter_sample)
										{
                        mp4_LimitMVQ(&mvForw, &mvForwLim, &limitRectL, dx, dy, 16);
                        mp4_ComputeChromaMVQ(&mvForwLim, &mvCbCrF);
                        mp4_Copy16x16QP_8u(pYp, stepYp, pYc, stepYc, &mvForwLim, 0);
                        mp4_LimitMVQ(&mvBack, &mvBackLim, &limitRectL, dx, dy, 16);
                        mp4_ComputeChromaMVQ(&mvBackLim, &mvCbCrB);
                        mp4_Copy16x16QP_8u(pYn, stepYn, tmpMB, 16, &mvBackLim, 0);
                    }
										else
										{
                        mp4_LimitMV(&mvForw, &mvForwLim, &limitRectL, dx, dy, 16);
                        mp4_ComputeChromaMV(&mvForwLim, &mvCbCrF);
                        mp4_Copy16x16HP_8u(pYp, stepYp, pYc, stepYc, &mvForwLim, 0);
                        mp4_LimitMV(&mvBack, &mvBackLim, &limitRectL, dx, dy, 16);
                        mp4_ComputeChromaMV(&mvBackLim, &mvCbCrB);
                        mp4_Copy16x16HP_8u(pYn, stepYn, tmpMB, 16, &mvBackLim, 0);
                    }
                    ippiAverage16x16_8u_C1IR(tmpMB, 16, pYc, stepYc);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 32, pYc, stepYc, Err_2);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 16, pYc+8, stepYc, Err_2);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 8, pYc+8*stepYc, stepYc, Err_2);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 4, pYc+8*stepYc+8, stepYc, Err_2);
                    mp4_Copy8x8HP_8u(pCbp, stepCbp, pCbc, stepCbc, &mvCbCrF, 0);
                    mp4_Copy8x8HP_8u(pCbn, stepCbn, tmpMB, 8, &mvCbCrB, 0);
                    ippiAverage8x8_8u_C1IR(tmpMB, 8, pCbc, stepCbc);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 2, pCbc, stepCbc, Err_2);
                    mp4_Copy8x8HP_8u(pCrp, stepCrp, pCrc, stepCrc, &mvCbCrF, 0);
                    mp4_Copy8x8HP_8u(pCrn, stepCrn, tmpMB, 8, &mvCbCrB, 0);
                    ippiAverage8x8_8u_C1IR(tmpMB, 8, pCrc, stepCrc);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 1, pCrc, stepCrc, Err_2);
                }
								else 
								{ // IPPVC_MBTYPE_DIRECT
                    IppMotionVector mvForw[4], mvBack[4], mvForwLim[4], mvBackLim[4];

                    mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_DIRECT);
                    //f MVs of collocated block of recently decoded I or P frame used in Direct mode
                    if (mp4_DecodeMV_Direct(pInfo, pMBinfo->mv, mvForw, mvBack, TRB, TRD, modb, pMBinfo->type) != MP4_STATUS_OK)
                        goto Err_2;
                    if (quarter_sample)
										{
                        mp4_Limit4MVQ(mvForw, mvForwLim, &limitRectL, dx, dy, 8);
                        mp4_ComputeChroma4MVQ(mvForw, &mvCbCrF);
                        mp4_Limit4MVQ(mvBack, mvBackLim, &limitRectL, dx, dy, 8);
                        mp4_ComputeChroma4MVQ(mvBack, &mvCbCrB);
                        mp4_Copy8x8QP_8u(pYp, stepYp, pYc, stepYc, &mvForwLim[0], 0);
                        mp4_Copy8x8QP_8u(pYn, stepYn, tmpMB, 16, &mvBackLim[0], 0);
                        mp4_Copy8x8QP_8u(pYp+8, stepYp, pYc+8, stepYc, &mvForwLim[1], 0);
                        mp4_Copy8x8QP_8u(pYn+8, stepYn, tmpMB+8, 16, &mvBackLim[1], 0);
                        mp4_Copy8x8QP_8u(pYp+8*stepYp, stepYp, pYc+8*stepYc, stepYc, &mvForwLim[2], 0);
                        mp4_Copy8x8QP_8u(pYn+8*stepYn, stepYn, tmpMB+8*16, 16, &mvBackLim[2], 0);
                        mp4_Copy8x8QP_8u(pYp+8*stepYp+8, stepYp, pYc+8*stepYc+8, stepYc, &mvForwLim[3], 0);
                        mp4_Copy8x8QP_8u(pYn+8*stepYn+8, stepYn, tmpMB+8*16+8, 16, &mvBackLim[3], 0);
                    } 
										else 
										{
                        if (pMBinfo->type == IPPVC_MBTYPE_INTER4V) {
                            mp4_Limit4MV(mvForw, mvForwLim, &limitRectL, dx, dy, 8);
                            mp4_ComputeChroma4MV(mvForw, &mvCbCrF);
                            mp4_Limit4MV(mvBack, mvBackLim, &limitRectL, dx, dy, 8);
                            mp4_ComputeChroma4MV(mvBack, &mvCbCrB);
                            mp4_Copy8x8HP_8u(pYp, stepYp, pYc, stepYc, &mvForwLim[0], 0);
                            mp4_Copy8x8HP_8u(pYn, stepYn, tmpMB, 16, &mvBackLim[0], 0);
                            mp4_Copy8x8HP_8u(pYp+8, stepYp, pYc+8, stepYc, &mvForwLim[1], 0);
                            mp4_Copy8x8HP_8u(pYn+8, stepYn, tmpMB+8, 16, &mvBackLim[1], 0);
                            mp4_Copy8x8HP_8u(pYp+8*stepYp, stepYp, pYc+8*stepYc, stepYc, &mvForwLim[2], 0);
                            mp4_Copy8x8HP_8u(pYn+8*stepYn, stepYn, tmpMB+8*16, 16, &mvBackLim[2], 0);
                            mp4_Copy8x8HP_8u(pYp+8*stepYp+8, stepYp, pYc+8*stepYc+8, stepYc, &mvForwLim[3], 0);
                            mp4_Copy8x8HP_8u(pYn+8*stepYn+8, stepYn, tmpMB+8*16+8, 16, &mvBackLim[3], 0);
                        } 
												else 
												{
                            mp4_LimitMV(mvForw, mvForwLim, &limitRectL, dx, dy, 16);
                            mp4_ComputeChromaMV(mvForwLim, &mvCbCrF);
                            mp4_LimitMV(mvBack, mvBackLim, &limitRectL, dx, dy, 16);
                            mp4_ComputeChromaMV(mvBackLim, &mvCbCrB);
                            mp4_Copy16x16HP_8u(pYp, stepYp, pYc, stepYc, &mvForwLim[0], 0);
                            mp4_Copy16x16HP_8u(pYn, stepYn, tmpMB, 16, &mvBackLim[0], 0);
                        }
                    }
                    ippiAverage16x16_8u_C1IR(tmpMB, 16, pYc, stepYc);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 32, pYc, stepYc, Err_2);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 16, pYc+8, stepYc, Err_2);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 8, pYc+8*stepYc, stepYc, Err_2);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 4, pYc+8*stepYc+8, stepYc, Err_2);
                    mp4_LimitMV(&mvCbCrF, &mvCbCrF, &limitRectC, dx >> 1, dy >> 1, 8);
                    mp4_LimitMV(&mvCbCrB, &mvCbCrB, &limitRectC, dx >> 1, dy >> 1, 8);
                    mp4_Copy8x8HP_8u(pCbp, stepCbp, pCbc, stepCbc, &mvCbCrF, 0);
                    mp4_Copy8x8HP_8u(pCbn, stepCbn, tmpMB, 8, &mvCbCrB, 0);
                    ippiAverage8x8_8u_C1IR(tmpMB, 8, pCbc, stepCbc);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 2, pCbc, stepCbc, Err_2);
                    mp4_Copy8x8HP_8u(pCrp, stepCrp, pCrc, stepCrc, &mvCbCrF, 0);
                    mp4_Copy8x8HP_8u(pCrn, stepCrn, tmpMB, 8, &mvCbCrB, 0);
                    ippiAverage8x8_8u_C1IR(tmpMB, 8, pCrc, stepCrc);
                    mp4_DecodeReconBlockInter_MPEG4(cbpb & 1, pCrc, stepCrc, Err_2);
                }
            }
            //mbCurr ++;
            mbInVideoPacket ++;
            colNum ++;
            pMBinfo ++;
            if (colNum == mbPerRow)
						{
                colNum = 0;
                rowNum ++;
                if (rowNum == mbPerCol)
                    return sts;
                pYc += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYc << 4) - stepYc;
                pCbc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbc << 3) - stepCbc;
                pCrc += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrc << 3) - stepCrc;
                pYp += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYp << 4) - stepYp;
                pCbp += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbp << 3) - stepCbp;
                pCrp += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrp << 3) - stepCrp;
                pYn += (2 * MP4_NUM_EXT_MB + 1) * 16 + (stepYn << 4) - stepYn;
                pCbn += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCbn << 3) - stepCbn;
                pCrn += (2 * MP4_NUM_EXT_MB + 1) * 8 + (stepCrn << 3) - stepCrn;
                // reset MV predictors at new row
                mvForw.dx = mvForw.dy = mvBack.dx = mvBack.dy = 0;
            } 
						else 
						{
                pYc += 16; pCrc += 8; pCbc += 8;
                pYp += 16; pCrp += 8; pCbp += 8;
                pYn += 16; pCrn += 8; pCbn += 8;
            }
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
                        pYc = pInfo->VisualObject.cFrame->pY + (rowNum * stepYc + colNum) * 16; pCbc = pInfo->VisualObject.cFrame->pCb + (rowNum * stepCbc + colNum) * 8; pCrc = pInfo->VisualObject.cFrame->pCr + (rowNum * stepCrc + colNum) * 8;
                        pYp = pInfo->VisualObject.rFrame->pY + (rowNum * stepYp + colNum) * 16; pCbp = pInfo->VisualObject.rFrame->pCb + (rowNum * stepCbp + colNum) * 8; pCrp = pInfo->VisualObject.rFrame->pCr + (rowNum * stepCrp + colNum) * 8;
                        pYn = pInfo->VisualObject.nFrame->pY + (rowNum * stepYn + colNum) * 16; pCbn = pInfo->VisualObject.nFrame->pCb + (rowNum * stepCbn + colNum) * 8; pCrn = pInfo->VisualObject.nFrame->pCr + (rowNum * stepCrn + colNum) * 8;
                        pMBinfo = pInfo->VisualObject.VideoObject.MBinfo + mbCurr;
                        break;
                    }
                }
								else
                    goto Err_2;
            }
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
}

