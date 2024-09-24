/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2001-2008 Intel Corporation. All Rights Reserved.
//
//  Description:    Decodes MPEG-4 bitstream.
//
*/

#include "mp4def.h"
#include "mp4dec.h"

#pragma warning(disable : 188)  // enumerated type mixed with another type ICL

void mp4_ResetVOL(mp4_Info *pInfo)
{
    pInfo->VisualObject.VideoObject.VOPindex = 0;
    pInfo->VisualObject.VideoObject.prevPlaneIsB = 0;
    pInfo->VisualObject.VideoObject.GroupOfVideoObjectPlane.time_code = 0;
		ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
		pInfo->VisualObject.cFrame=0;
		ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
		pInfo->VisualObject.rFrame=0;
		ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.nFrame);
		pInfo->VisualObject.nFrame=0;
		ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.sFrame);
		pInfo->VisualObject.sFrame=0;
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

static mp4_Status mp4_AllocInitFrame(mp4_Frame* pFrame, int32_t mbPerRow, int32_t mbPerCol)
{
    int32_t w, h;

    w = (mbPerRow + 2 * MP4_NUM_EXT_MB) << 4;
    h = (mbPerCol + 2 * MP4_NUM_EXT_MB) << 4;
    pFrame->stepY = w;
    pFrame->apY = ippsMalloc_8u(w * h);
    if (!pFrame->apY)
        return MP4_STATUS_NO_MEM;
    ippsSet_8u(0, pFrame->apY, w * h);
    pFrame->pY = pFrame->apY + w * 16 + 16;
    w >>= 1;
    h >>= 1;
    pFrame->stepCb = w;
    pFrame->apCb = ippsMalloc_8u(w * h);
    if (!pFrame->apCb)
        return MP4_STATUS_NO_MEM;
    ippsSet_8u(128, pFrame->apCb, w * h);
    pFrame->pCb = pFrame->apCb + w * 8 + 8;
    pFrame->stepCr = w;
    pFrame->apCr = ippsMalloc_8u(w * h);
    if (!pFrame->apCr)
        return MP4_STATUS_NO_MEM;
    pFrame->pCr = pFrame->apCr + w * 8 + 8;
    ippsSet_8u(128, pFrame->apCr, w * h);
    return MP4_STATUS_OK;
}

/*
//  mp4_Info for decoding of Video Object Layer
*/
mp4_Status mp4_InitVOL(mp4_Info* pInfo)
{
    int32_t mbPerRow, mbPerCol, specSize;

    if (pInfo->VisualObject.VideoObject.short_video_header) {
        pInfo->noBVOPs = 1;
    } else {
        if (pInfo->strictSyntaxCheck) {
            if (pInfo->VisualObject.VideoObject.random_accessible_vol)
                pInfo->noPVOPs = pInfo->noBVOPs = 1;
            if (pInfo->VisualObject.VideoObject.type_indication == MP4_VIDEO_OBJECT_TYPE_SIMPLE ||
                pInfo->VisualObject.VideoObject.type_indication == MP4_VIDEO_OBJECT_TYPE_ADVANCED_REAL_TIME_SIMPLE ||
                pInfo->VisualObject.VideoObject.VOLControlParameters.low_delay) {
                pInfo->noBVOPs = 1;
            }
        }
    }
    if (pInfo->VisualObject.VideoObject.shape == MP4_SHAPE_TYPE_RECTANGULAR)
		{
        mbPerRow = (pInfo->VisualObject.VideoObject.width + 15) >> 4;
        mbPerCol = (pInfo->VisualObject.VideoObject.height + 15) >> 4;
        // current frame
//        if (mp4_AllocInitFrame(&pInfo->VisualObject.cFrame, mbPerRow, mbPerCol) != MP4_STATUS_OK)
//            return MP4_STATUS_NO_MEM;
        if (pInfo->VisualObject.VideoObject.sprite_enable != MP4_SPRITE_STATIC)
				{
            // ref in past frame
//            if (mp4_AllocInitFrame(&pInfo->VisualObject.rFrame, mbPerRow, mbPerCol) != MP4_STATUS_OK)
//                return MP4_STATUS_NO_MEM;
            // ref in future frame
//            if (mp4_AllocInitFrame(&pInfo->VisualObject.nFrame, mbPerRow, mbPerCol) != MP4_STATUS_OK)
//                return MP4_STATUS_NO_MEM;
            // motion info (not needed for Sprites)
            pInfo->VisualObject.VideoObject.MBinfo = (mp4_MacroBlock*)ippsMalloc_8u(mbPerRow*mbPerCol*sizeof(mp4_MacroBlock));
            if (!pInfo->VisualObject.VideoObject.MBinfo) return MP4_STATUS_NO_MEM;
#ifdef USE_NOTCODED_STATE
            // not_coded MB state
            pInfo->VisualObject.VideoObject.ncState = ippsMalloc_8u(mbPerCol * mbPerRow);
            if (!pInfo->VisualObject.VideoObject.ncState) return MP4_STATUS_NO_MEM;
            pInfo->VisualObject.VideoObject.ncStateCleared = 0;
#endif
        } 
				else 
				{ // data for static sprite
            mbPerRow = pInfo->VisualObject.sFrame->mbPerRow = (pInfo->VisualObject.VideoObject.sprite_width + 15) >> 4;
            mbPerCol = pInfo->VisualObject.sFrame->mbPerCol = (pInfo->VisualObject.VideoObject.sprite_height + 15) >> 4;
//            if (mp4_AllocInitFrame(&pInfo->VisualObject.sFrame, mbPerRow, mbPerCol) != MP4_STATUS_OK)
//                return MP4_STATUS_NO_MEM;
        }
        pInfo->VisualObject.VideoObject.MacroBlockPerRow = mbPerRow;
        pInfo->VisualObject.VideoObject.MacroBlockPerCol = mbPerCol;
        pInfo->VisualObject.VideoObject.MacroBlockPerVOP = mbPerRow * mbPerCol;
        pInfo->VisualObject.VideoObject.mbns = mp4_GetMacroBlockNumberSize(pInfo->VisualObject.VideoObject.MacroBlockPerVOP);

        if (!pInfo->VisualObject.VideoObject.short_video_header) 
				{
            // Intra Prediction info (not needed for SVH)
            pInfo->VisualObject.VideoObject.IntraPredBuff.quant = ippsMalloc_8u(mbPerRow + 1);
            if (!pInfo->VisualObject.VideoObject.IntraPredBuff.quant) return MP4_STATUS_NO_MEM;
            pInfo->VisualObject.VideoObject.IntraPredBuff.block = (mp4_IntraPredBlock*)ippsMalloc_8u((mbPerRow + 1)*6*sizeof(mp4_IntraPredBlock));
            if (!pInfo->VisualObject.VideoObject.IntraPredBuff.block) return MP4_STATUS_NO_MEM;
            {
                mp4_IntraPredBlock *mbCurr = pInfo->VisualObject.VideoObject.IntraPredBuff.block;
                mp4_IntraPredBlock *mbA = mbCurr, *mbB = pInfo->VisualObject.VideoObject.IntraPredBuff.dcB, *mbC = mbCurr + 6;
                int32_t              j;

                for (j = 0; j < mbPerRow; j ++) {
                    mbCurr[0].predA = &mbA[1];  mbCurr[0].predB = &mbB[3];  mbCurr[0].predC = &mbC[2];
                    mbCurr[1].predA = &mbC[0];  mbCurr[1].predB = &mbC[2];  mbCurr[1].predC = &mbC[3];
                    mbCurr[2].predA = &mbA[3];  mbCurr[2].predB = &mbA[1];  mbCurr[2].predC = &mbC[0];
                    mbCurr[3].predA = &mbC[2];  mbCurr[3].predB = &mbC[0];  mbCurr[3].predC = &mbC[1];
                    mbCurr[4].predA = &mbA[4];  mbCurr[4].predB = &mbB[4];  mbCurr[4].predC = &mbC[4];
                    mbCurr[5].predA = &mbA[5];  mbCurr[5].predB = &mbB[5];  mbCurr[5].predC = &mbC[5];
                    mbCurr += 6;  mbA += 6;  mbC += 6;
                }
            }
            if (pInfo->VisualObject.VideoObject.data_partitioned) {
                // DataPart info
                pInfo->VisualObject.VideoObject.DataPartBuff = (mp4_DataPartMacroBlock*)ippsMalloc_8u(mbPerRow*mbPerCol*sizeof(mp4_DataPartMacroBlock));
                if (!pInfo->VisualObject.VideoObject.DataPartBuff) return MP4_STATUS_NO_MEM;
            }
            if (pInfo->VisualObject.VideoObject.interlaced) {
                // Field MV for B-VOP
                pInfo->VisualObject.VideoObject.FieldMV = (IppMotionVector*)ippsMalloc_8u(mbPerRow*mbPerCol*sizeof(IppMotionVector)*2);
                if (!pInfo->VisualObject.VideoObject.FieldMV) return MP4_STATUS_NO_MEM;
            }
            ippiQuantInvIntraGetSize_MPEG4(&specSize);
            pInfo->VisualObject.VideoObject.QuantInvIntraSpec = (IppiQuantInvIntraSpec_MPEG4*)ippsMalloc_8u(specSize);
            ippiQuantInvIntraInit_MPEG4(pInfo->VisualObject.VideoObject.quant_type ? pInfo->VisualObject.VideoObject.intra_quant_mat : NULL, pInfo->VisualObject.VideoObject.QuantInvIntraSpec, 8);
            ippiQuantInvInterGetSize_MPEG4(&specSize);
            pInfo->VisualObject.VideoObject.QuantInvInterSpec = (IppiQuantInvInterSpec_MPEG4*)ippsMalloc_8u(specSize);
            ippiQuantInvInterInit_MPEG4(pInfo->VisualObject.VideoObject.quant_type ? pInfo->VisualObject.VideoObject.nonintra_quant_mat : NULL, pInfo->VisualObject.VideoObject.QuantInvInterSpec, 8);
            ippiWarpGetSize_MPEG4(&specSize);
            pInfo->VisualObject.VideoObject.WarpSpec = (IppiWarpSpec_MPEG4*)ippsMalloc_8u(specSize);
        }
    }
    return MP4_STATUS_OK;
}

/*
//  Free memory allocated for mp4_Info
*/
mp4_Status mp4_FreeVOL(mp4_Info* pInfo)
{
    if (pInfo->VisualObject.VideoObject.shape == MP4_SHAPE_TYPE_RECTANGULAR) {
        if (pInfo->VisualObject.VideoObject.sprite_enable != MP4_SPRITE_STATIC) {
            ippsFree(pInfo->VisualObject.VideoObject.MBinfo); pInfo->VisualObject.VideoObject.MBinfo = NULL;
#ifdef USE_NOTCODED_STATE
            ippsFree(pInfo->VisualObject.VideoObject.ncState);
#endif
        } else {
        }
        if (!pInfo->VisualObject.VideoObject.short_video_header) {
            ippsFree(pInfo->VisualObject.VideoObject.IntraPredBuff.quant); pInfo->VisualObject.VideoObject.IntraPredBuff.quant = NULL;
            ippsFree(pInfo->VisualObject.VideoObject.IntraPredBuff.block); pInfo->VisualObject.VideoObject.IntraPredBuff.block = NULL;
            if (pInfo->VisualObject.VideoObject.data_partitioned) {
                ippsFree(pInfo->VisualObject.VideoObject.DataPartBuff); pInfo->VisualObject.VideoObject.DataPartBuff = NULL;
            }
            if (pInfo->VisualObject.VideoObject.interlaced) {
                ippsFree(pInfo->VisualObject.VideoObject.FieldMV); pInfo->VisualObject.VideoObject.FieldMV = NULL;
            }
            ippsFree(pInfo->VisualObject.VideoObject.QuantInvIntraSpec); pInfo->VisualObject.VideoObject.QuantInvIntraSpec = NULL;
            ippsFree(pInfo->VisualObject.VideoObject.QuantInvInterSpec); pInfo->VisualObject.VideoObject.QuantInvInterSpec = NULL;
            ippsFree(pInfo->VisualObject.VideoObject.WarpSpec); pInfo->VisualObject.VideoObject.WarpSpec = NULL;
        }

    }
    return MP4_STATUS_OK;
}


mp4_Status mp4_DecodeMVD(mp4_Info *pInfo, int32_t *mvdx, int32_t *mvdy, int32_t fcode)
{
    const mp4_VLC1 *pTab;
    int32_t          mvd, sign;
    uint32_t          code;
    int32_t          factor = fcode - 1;

    /* decode MVDx */
    code = mp4_ShowBits(pInfo, 12);
    if (code >= 128)
        pTab = mp4_MVD_B12_2 + ((code - 128) >> 5);
    else if (code >= 2)
        pTab = mp4_MVD_B12_1 + (code - 2);
    else {
        mp4_Error("Error: decoding MVD");
        return MP4_STATUS_ERROR;
    }
    mvd = pTab->code;
    mp4_FlushBits(pInfo, pTab->len);
    if (mvd) {
        sign = mp4_GetBit(pInfo);
        if (factor) {
            code = mp4_GetBits9(pInfo, factor);
            mvd = ((mvd - 1) << factor) + code + 1;
        }
        if (sign)
            mvd = -mvd;
    }
    *mvdx = mvd;
    /* decode MVDy */
    code = mp4_ShowBits(pInfo, 12);
    if (code >= 128)
        pTab = mp4_MVD_B12_2 + ((code - 128) >> 5);
    else if (code >= 2)
        pTab = mp4_MVD_B12_1 + (code - 2);
    else {
        mp4_Error("Error: decoding MVD");
        return MP4_STATUS_ERROR;
    }
    mvd = pTab->code;
    mp4_FlushBits(pInfo, pTab->len);
    if (mvd) {
        sign = mp4_GetBit(pInfo);
        if (factor) {
            code = mp4_GetBits9(pInfo, factor);
            mvd = ((mvd - 1) << factor) + code + 1;
        }
        if (sign)
            mvd = -mvd;
    }
    *mvdy = mvd;
    return MP4_STATUS_OK;
}

mp4_Status mp4_DecodeMV(mp4_Info *pInfo, IppMotionVector *mv, int32_t fcode)
{
    int32_t  mvdx, mvdy, range, range2, dx, dy;

    if (mp4_DecodeMVD(pInfo, &mvdx, &mvdy, fcode) != MP4_STATUS_OK)
        return MP4_STATUS_ERROR;
    range = 16 << fcode;
    range2 = range + range;
    dx = mv->dx + mvdx;
    if (dx < -range)
        dx += range2;
    else if (dx >= range)
        dx -= range2;
    mv->dx = (int16_t)dx;
    dy = mv->dy + mvdy;
    if (dy < -range)
        dy += range2;
    else if (dy >= range)
        dy -= range2;
    mv->dy = (int16_t)dy;
    return MP4_STATUS_OK;
}

mp4_Status mp4_Decode4MV(mp4_Info *pInfo, IppMotionVector *mv, int32_t fcode)
{
    int32_t  i, mvdx, mvdy, range, range2, dx, dy;

    range = 16 << fcode;
    range2 = range + range;
    for (i = 0; i < 4; i ++) {
        if (mp4_DecodeMVD(pInfo, &mvdx, &mvdy, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
        dx = mv[i].dx + mvdx;
        if (dx < -range)
            dx += range2;
        else if (dx >= range)
            dx -= range2;
        mv[i].dx = (int16_t)dx;
        dy = mv[i].dy + mvdy;
        if (dy < -range)
            dy += range2;
        else if (dy >= range)
            dy -= range2;
        mv[i].dy = (int16_t)dy;
    }
    return MP4_STATUS_OK;
}

mp4_Status mp4_DecodeMV_Direct(mp4_Info *pInfo, IppMotionVector mvC[4], IppMotionVector mvForw[4], IppMotionVector mvBack[4], int32_t TRB, int32_t TRD, int32_t modb, int32_t comb_type)
{
    int32_t  mvdx, mvdy, i;

    if (modb == 2) {
        if (comb_type != IPPVC_MBTYPE_INTER4V) {
            mvForw[0].dx = mvForw[1].dx = mvForw[2].dx = mvForw[3].dx = (int16_t)((TRB * mvC[0].dx) / TRD);
            mvForw[0].dy = mvForw[1].dy = mvForw[2].dy = mvForw[3].dy = (int16_t)((TRB * mvC[0].dy) / TRD);
            mvBack[0].dx = mvBack[1].dx = mvBack[2].dx = mvBack[3].dx = (int16_t)(((TRB - TRD) * mvC[0].dx) / TRD);
            mvBack[0].dy = mvBack[1].dy = mvBack[2].dy = mvBack[3].dy = (int16_t)(((TRB - TRD) * mvC[0].dy) / TRD);
        } else
            for (i = 0; i < 4; i ++) {
                mvForw[i].dx = (int16_t)((TRB * mvC[i].dx) / TRD);
                mvForw[i].dy = (int16_t)((TRB * mvC[i].dy) / TRD);
                mvBack[i].dx = (int16_t)(((TRB - TRD) * mvC[i].dx) / TRD);
                mvBack[i].dy = (int16_t)(((TRB - TRD) * mvC[i].dy) / TRD);
            }
    } else {
        if (mp4_DecodeMVD(pInfo, &mvdx, &mvdy, 1) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
        if (comb_type != IPPVC_MBTYPE_INTER4V) {
            mvForw[0].dx = mvForw[1].dx = mvForw[2].dx = mvForw[3].dx = (int16_t)((TRB * mvC[0].dx) / TRD + mvdx);
            mvForw[0].dy = mvForw[1].dy = mvForw[2].dy = mvForw[3].dy = (int16_t)((TRB * mvC[0].dy) / TRD + mvdy);
            if (mvdx == 0)
                mvBack[0].dx = mvBack[1].dx = mvBack[2].dx = mvBack[3].dx = (int16_t)(((TRB - TRD) * mvC[0].dx) / TRD);
            else
                mvBack[0].dx = mvBack[1].dx = mvBack[2].dx = mvBack[3].dx = (int16_t)(mvForw[0].dx - mvC[0].dx);
            if (mvdy == 0)
                mvBack[0].dy = mvBack[1].dy = mvBack[2].dy = mvBack[3].dy = (int16_t)(((TRB - TRD) * mvC[0].dy) / TRD);
            else
                mvBack[0].dy = mvBack[1].dy = mvBack[2].dy = mvBack[3].dy = (int16_t)(mvForw[0].dy - mvC[0].dy);
        } else
            for (i = 0; i < 4; i++) {
                mvForw[i].dx = (int16_t)((TRB * mvC[i].dx) / TRD + mvdx);
                mvForw[i].dy = (int16_t)((TRB * mvC[i].dy) / TRD + mvdy);
                if (mvdx == 0)
                    mvBack[i].dx = (int16_t)(((TRB - TRD) * mvC[i].dx) / TRD);
                else
                    mvBack[i].dx = (int16_t)(mvForw[i].dx - mvC[i].dx);
                if (mvdy == 0)
                    mvBack[i].dy = (int16_t)(((TRB - TRD) * mvC[i].dy) / TRD);
                else
                    mvBack[i].dy = (int16_t)(mvForw[i].dy - mvC[i].dy);
            }
    }
    return MP4_STATUS_OK;
}

mp4_Status mp4_DecodeMV_DirectField(mp4_Info *pInfo, int32_t mb_ftfr, int32_t mb_fbfr, IppMotionVector *mvTop, IppMotionVector *mvBottom, IppMotionVector *mvForwTop, IppMotionVector *mvForwBottom, IppMotionVector *mvBackTop, IppMotionVector *mvBackBottom, int32_t TRB, int32_t TRD, int32_t modb)
{
    // field direct mode
    int32_t  TRDt, TRDb, TRBt, TRBb, deltaTop, deltaBottom, mvdx, mvdy;

    deltaTop = mb_ftfr;
    deltaBottom = mb_fbfr - 1;
    if (pInfo->VisualObject.VideoObject.VideoObjectPlane.top_field_first) {
        deltaTop = -deltaTop;
        deltaBottom = -deltaBottom;
    }
    TRDt = mp4_DivRoundInf(TRD, pInfo->VisualObject.VideoObject.Tframe) * 2 + deltaTop;
    TRDb = mp4_DivRoundInf(TRD, pInfo->VisualObject.VideoObject.Tframe) * 2 + deltaBottom;
    TRBt = mp4_DivRoundInf(TRB, pInfo->VisualObject.VideoObject.Tframe) * 2 + deltaTop;
    TRBb = mp4_DivRoundInf(TRB, pInfo->VisualObject.VideoObject.Tframe) * 2 + deltaBottom;
    if (modb == 2) {
        // delta == 0
        mvdx = mvdy = 0;
    } else {
        if (mp4_DecodeMVD(pInfo, &mvdx, &mvdy, 1) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
    }
    mvForwTop->dx = (int16_t)((TRBt * mvTop->dx) / TRDt + mvdx);
    if (mvdx == 0)
        mvBackTop->dx = (int16_t)(((TRBt - TRDt) * mvTop->dx) / TRDt);
    else
        mvBackTop->dx = (int16_t)(mvForwTop->dx - mvTop->dx);
    mvForwTop->dy = (int16_t)((TRBt * mvTop->dy * 2) / TRDt + mvdy);
    if (mvdy == 0)
        mvBackTop->dy = (int16_t)(((TRBt - TRDt) * mvTop->dy * 2) / TRDt);
    else
        mvBackTop->dy = (int16_t)(mvForwTop->dy - mvTop->dy * 2);
    mvForwBottom->dx = (int16_t)((TRBb * mvBottom->dx) / TRDb + mvdx);
    if (mvdx == 0)
        mvBackBottom->dx = (int16_t)(((TRBb - TRDb) * mvBottom->dx) / TRDb);
    else
        mvBackBottom->dx = (int16_t)(mvForwBottom->dx - mvBottom->dx);
    mvForwBottom->dy = (int16_t)((TRBb * mvBottom->dy * 2) / TRDb + mvdy);
    if (mvdy == 0)
        mvBackBottom->dy = (int16_t)(((TRBb - TRDb) * mvBottom->dy * 2) / TRDb);
    else
        mvBackBottom->dy = (int16_t)(mvForwBottom->dy - mvBottom->dy * 2);
    mvForwTop->dy >>= 1;
    mvBackTop->dy >>= 1;
    mvForwBottom->dy >>= 1;
    mvBackBottom->dy >>= 1;
    return MP4_STATUS_OK;
}

/*static*/ void mp4_ExpandFrameReplicate(uint8_t *pSrcDstPlane, int32_t frameWidth, int32_t frameHeight, int32_t expandPels, int32_t step)
{
    uint8_t   *pDst1, *pDst2, *pSrc1, *pSrc2;
    int32_t  i, j;
    uint32_t  t1, t2;

    pDst1 = pSrcDstPlane + step * expandPels;
    pDst2 = pDst1 + frameWidth + expandPels;
    if (expandPels == 8) {
        for (i = 0; i < frameHeight; i ++) {
            t1 = pDst1[8] + (pDst1[8] << 8);
            t2 = pDst2[-1] + (pDst2[-1] << 8);
            t1 = (t1 << 16) + t1;
            t2 = (t2 << 16) + t2;
            ((uint32_t*)pDst1)[0] = t1;
            ((uint32_t*)pDst1)[1] = t1;
            ((uint32_t*)pDst2)[0] = t2;
            ((uint32_t*)pDst2)[1] = t2;
            pDst1 += step;
            pDst2 += step;
        }
    } else if (expandPels == 16) {
        for (i = 0; i < frameHeight; i ++) {
            t1 = pDst1[16] + (pDst1[16] << 8);
            t2 = pDst2[-1] + (pDst2[-1] << 8);
            t1 = (t1 << 16) + t1;
            t2 = (t2 << 16) + t2;
            ((uint32_t*)pDst1)[0] = t1;
            ((uint32_t*)pDst1)[1] = t1;
            ((uint32_t*)pDst1)[2] = t1;
            ((uint32_t*)pDst1)[3] = t1;
            ((uint32_t*)pDst2)[0] = t2;
            ((uint32_t*)pDst2)[1] = t2;
            ((uint32_t*)pDst2)[2] = t2;
            ((uint32_t*)pDst2)[3] = t2;
            pDst1 += step;
            pDst2 += step;
        }
    } else {
        for (i = 0; i < frameHeight; i ++) {
            ippsSet_8u(pDst1[expandPels], pDst1, expandPels);
            ippsSet_8u(pDst2[-1], pDst2, expandPels);
            pDst1 += step;
            pDst2 += step;
        }
    }
    pDst1 = pSrcDstPlane;
    pSrc1 = pSrcDstPlane + expandPels * step;
    pDst2 = pSrc1 + frameHeight * step;
    pSrc2 = pDst2 - step;
    j = frameWidth + 2 * expandPels;
    for (i = 0; i < expandPels; i ++) {
        ippsCopy_8u(pSrc1, pDst1, j);
        ippsCopy_8u(pSrc2, pDst2, j);
        pDst1 += step;
        pDst2 += step;
    }
}

void mp4_PadFrame(mp4_Info* pInfo)
{
#if 0
    /*
    //  padding VOP (for not complete blocks padd by
    //      0 for DivX(tm) 5.0 AVI streams
    //      128 for QuickTime(tm) MP4 streams
    //      replication for other
    */
    int32_t  wL, hL, wC, hC, i;

    //if (pInfo->VisualObject.VideoObject.short_video_header)
    //    return;
    wL = pInfo->VisualObject.VideoObject.width;
    hL = pInfo->VisualObject.VideoObject.height;
    wC = pInfo->VisualObject.VideoObject.width >> 1;
    hC = pInfo->VisualObject.VideoObject.height >> 1;
    if ((pInfo->VisualObject.VideoObject.width & 15 || pInfo->VisualObject.VideoObject.height & 15) &&
        ((pInfo->ftype == 1 && pInfo->ftype_f == 0) || (pInfo->ftype == 2 && pInfo->ftype_f == 1))) {
        uint8_t     pad = (uint8_t)(pInfo->ftype == 1 ? 128 : 0);

        if (pInfo->VisualObject.VideoObject.width & 15) {
            uint8_t *p;
            // pad one col
            p = pInfo->VisualObject.cFrame.pY + pInfo->VisualObject.VideoObject.width;
            for (i = 0; i < pInfo->VisualObject.VideoObject.height; i ++) {
                *p = pad;
                p += pInfo->VisualObject.cFrame.stepY;
            }
            p = pInfo->VisualObject.cFrame.pCb + (pInfo->VisualObject.VideoObject.width >> 1);
            for (i = 0; i < pInfo->VisualObject.VideoObject.height >> 1; i ++) {
                *p = pad;
                p += pInfo->VisualObject.cFrame.stepCb;
            }
            p = pInfo->VisualObject.cFrame.pCr + (pInfo->VisualObject.VideoObject.width >> 1);
            for (i = 0; i < pInfo->VisualObject.VideoObject.height >> 1; i ++) {
                *p = pad;
                p += pInfo->VisualObject.cFrame.stepCr;
            }
            wL ++;
            wC ++;
        }
        if (pInfo->VisualObject.VideoObject.height & 15) {
            // pad one row
            ippsSet_8u(pad, pInfo->VisualObject.cFrame.pY + pInfo->VisualObject.cFrame.stepY * pInfo->VisualObject.VideoObject.height, pInfo->VisualObject.VideoObject.width);
            ippsSet_8u(pad, pInfo->VisualObject.cFrame.pCb + pInfo->VisualObject.cFrame.stepCb * (pInfo->VisualObject.VideoObject.height >> 1), pInfo->VisualObject.VideoObject.width >> 1);
            ippsSet_8u(pad, pInfo->VisualObject.cFrame.pCr + pInfo->VisualObject.cFrame.stepCr * (pInfo->VisualObject.VideoObject.height >> 1), pInfo->VisualObject.VideoObject.width >> 1);
            hL ++;
            hC ++;
        }
    }
#else
    /*
    //  padding VOP for not complete blocks
    //  replication from macroblock boundary for DIVX and MP4 and from frame boundary for other
    */
    int32_t  wL, hL, wC, hC;

    if ((pInfo->ftype == 1 && pInfo->ftype_f == 0) || (pInfo->ftype == 2 && pInfo->ftype_f == 1)) {
        wL = pInfo->VisualObject.VideoObject.MacroBlockPerRow * 16;
        hL = pInfo->VisualObject.VideoObject.MacroBlockPerCol * 16;
    } else {
        wL = pInfo->VisualObject.VideoObject.width;
        hL = pInfo->VisualObject.VideoObject.height;
    }
    wC = wL >> 1;
    hC = hL >> 1;
#endif
    mp4_ExpandFrameReplicate(pInfo->VisualObject.cFrame->apY, wL, hL, 16 * MP4_NUM_EXT_MB, pInfo->VisualObject.cFrame->stepY);
    mp4_ExpandFrameReplicate(pInfo->VisualObject.cFrame->apCb, wC, hC, 8 * MP4_NUM_EXT_MB, pInfo->VisualObject.cFrame->stepCb);
    mp4_ExpandFrameReplicate(pInfo->VisualObject.cFrame->apCr, wC, hC, 8 * MP4_NUM_EXT_MB, pInfo->VisualObject.cFrame->stepCr);

}

mp4_Status mp4_DecodeVideoObjectPlane(mp4_Info* pInfo)
{
    mp4_Status  status = MP4_STATUS_OK;
    Ipp64s      vop_time;

    // set VOP time
    if (pInfo->VisualObject.VideoObject.short_video_header) 
		{
        vop_time = pInfo->VisualObject.VideoObject.vop_sync_time + pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.temporal_reference * 1001;
        if (pInfo->VisualObject.cFrame && pInfo->VisualObject.cFrame->time > vop_time)
				{
            pInfo->VisualObject.VideoObject.vop_sync_time += 256 * 1001;
            vop_time += 256 * 1001;
        }
    } 
		else 
		{
        if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coding_type == MP4_VOP_TYPE_B)
				{
            vop_time = pInfo->VisualObject.VideoObject.vop_sync_time_b + pInfo->VisualObject.VideoObject.VideoObjectPlane.modulo_time_base * pInfo->VisualObject.VideoObject.vop_time_increment_resolution + pInfo->VisualObject.VideoObject.VideoObjectPlane.time_increment;
        } 
				else 
				{
            if (pInfo->VisualObject.VideoObject.GroupOfVideoObjectPlane.time_code > pInfo->VisualObject.VideoObject.vop_sync_time)
                pInfo->VisualObject.VideoObject.vop_sync_time = pInfo->VisualObject.VideoObject.GroupOfVideoObjectPlane.time_code;
            vop_time = pInfo->VisualObject.VideoObject.vop_sync_time + pInfo->VisualObject.VideoObject.VideoObjectPlane.modulo_time_base * pInfo->VisualObject.VideoObject.vop_time_increment_resolution + pInfo->VisualObject.VideoObject.VideoObjectPlane.time_increment;
            if (pInfo->VisualObject.VideoObject.vop_sync_time_b < pInfo->VisualObject.VideoObject.vop_sync_time)
                pInfo->VisualObject.VideoObject.vop_sync_time_b = pInfo->VisualObject.VideoObject.vop_sync_time;
            if (pInfo->VisualObject.VideoObject.VideoObjectPlane.modulo_time_base != 0) {
                pInfo->VisualObject.VideoObject.vop_sync_time = vop_time - pInfo->VisualObject.VideoObject.VideoObjectPlane.time_increment;
            }
        }
    }
//  if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coded || vop_time != pInfo->VisualObject.rFrame.time) {
    if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coded ||
        ( (!pInfo->VisualObject.cFrame || vop_time != pInfo->VisualObject.cFrame->time) &&
         (!pInfo->VisualObject.rFrame || vop_time != pInfo->VisualObject.rFrame->time) &&
         (!pInfo->VisualObject.nFrame || vop_time != pInfo->VisualObject.nFrame->time)
				 )) 
		{
        switch (pInfo->VisualObject.VideoObject.VideoObjectPlane.coding_type) 
				{
        case MP4_VOP_TYPE_I :
            // set new video frame
            if (pInfo->VisualObject.VideoObject.sprite_enable == MP4_SPRITE_STATIC)
						{
							// TODO: verify
										ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.sFrame);
										pInfo->VisualObject.sFrame = pInfo->VisualObject.cFrame;
										pInfo->VisualObject.cFrame = 0;
            } 
						else 
						{
                if (pInfo->noPVOPs) 
								{
										DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
										pInfo->VisualObject.cFrame=0;
                }
								else if (pInfo->noBVOPs) 
								{
																		DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
																	ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
													pInfo->VisualObject.rFrame = pInfo->VisualObject.cFrame;
													pInfo->VisualObject.cFrame = 0;
                } 
								else 
								{
                    if (pInfo->VisualObject.VideoObject.VOPindex > 0) 
										{
                        if (pInfo->VisualObject.VideoObject.prevPlaneIsB) 
												{
													DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
													pInfo->VisualObject.cFrame = 0;
													DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.nFrame);
																					ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
													pInfo->VisualObject.rFrame = pInfo->VisualObject.nFrame;
													pInfo->VisualObject.nFrame = 0;
													pInfo->VisualObject.VideoObject.prevPlaneIsB=0;
                        } 
												else 
												{
													DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
													ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
													pInfo->VisualObject.rFrame = pInfo->VisualObject.cFrame;
													pInfo->VisualObject.cFrame = 0;
                        }

                    }
                }
            }
            if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coded)
						{
                if (pInfo->VisualObject.VideoObject.shape == MP4_SHAPE_TYPE_RECTANGULAR) 
								{
                    status = mp4_DecodeVOP_I(pInfo);
                } //f else
                    //f status = mp4_DecodeVOP_I_Shape(pInfo);
                if (pInfo->VisualObject.VideoObject.sprite_enable == MP4_SPRITE_STATIC && pInfo->VisualObject.VideoObject.VOPindex == 0)
								{
									// TODO: verify (was: mp4_SWAP(mp4_Frame, pInfo->VisualObject.sFrame, pInfo->VisualObject.cFrame);
										ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.sFrame);
										pInfo->VisualObject.sFrame = pInfo->VisualObject.cFrame;
										pInfo->VisualObject.cFrame = 0;                    
                    mp4_ExpandFrameReplicate(pInfo->VisualObject.sFrame->apY, pInfo->VisualObject.VideoObject.sprite_width, pInfo->VisualObject.VideoObject.sprite_height, 16, pInfo->VisualObject.sFrame->stepY);
                    mp4_ExpandFrameReplicate(pInfo->VisualObject.sFrame->apCb, pInfo->VisualObject.VideoObject.sprite_width >> 1, pInfo->VisualObject.VideoObject.sprite_height >> 1, 8, pInfo->VisualObject.sFrame->stepCb);
                    mp4_ExpandFrameReplicate(pInfo->VisualObject.sFrame->apCr, pInfo->VisualObject.VideoObject.sprite_width >> 1, pInfo->VisualObject.VideoObject.sprite_height >> 1, 8, pInfo->VisualObject.sFrame->stepCr);
                } else {
                    mp4_PadFrame(pInfo);
                }
                // set past and future time for B-VOP
                pInfo->VisualObject.VideoObject.rTime = pInfo->VisualObject.VideoObject.nTime;
                pInfo->VisualObject.VideoObject.nTime = vop_time;
#ifdef USE_NOTCODED_STATE
                // Clear not_coded MB state
                if ((pInfo->VisualObject.VideoObject.sprite_enable != MP4_SPRITE_STATIC) && pInfo->VisualObject.VideoObject.obmc_disable && !pInfo->VisualObject.VideoObject.ncStateCleared) {
                    ippsZero_8u(pInfo->VisualObject.VideoObject.ncState, pInfo->VisualObject.VideoObject.MacroBlockPerVOP);
                    pInfo->VisualObject.VideoObject.ncStateCleared = 1;
                }
#endif
            }
            mp4_StatisticInc(&pInfo->VisualObject.Statistic.nVOP_I);
            break;
        case MP4_VOP_TYPE_P :
            // set new video frame
            if (pInfo->noBVOPs) 
						{
          				DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
																	ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
													pInfo->VisualObject.rFrame = pInfo->VisualObject.cFrame;
													pInfo->VisualObject.cFrame = 0;
            }
						else 
						{
                if (pInfo->VisualObject.VideoObject.VOPindex > 0) 
								{
                    if (pInfo->VisualObject.VideoObject.prevPlaneIsB) 
										{
											DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
											ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
											pInfo->VisualObject.cFrame=0;
											DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.nFrame);
											ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
											pInfo->VisualObject.rFrame = pInfo->VisualObject.nFrame;
											pInfo->VisualObject.nFrame = 0;
                      pInfo->VisualObject.VideoObject.prevPlaneIsB = 0;
                    }
										else 
										{
											if (pInfo->VisualObject.cFrame)
											{
												int w, h;
												DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
												ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
												pInfo->VisualObject.rFrame = pInfo->VisualObject.cFrame;
												pInfo->VisualObject.cFrame=0;

												pInfo->VisualObject.cFrame = CreateFrame(&pInfo->VisualObject);
												w = (pInfo->VisualObject.cFrame->mbPerRow + 2 * MP4_NUM_EXT_MB) << 4;
												h = (pInfo->VisualObject.cFrame->mbPerCol + 2 * MP4_NUM_EXT_MB) << 4;
												memcpy(pInfo->VisualObject.cFrame->mid, pInfo->VisualObject.rFrame->mid, w * h + (w * h >> 1));
											}
                    }

                }
            }
            if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coded)
						{
                status = mp4_DecodeVOP_P(pInfo);
                mp4_PadFrame(pInfo);
                // set past and future time for B-VOP
                pInfo->VisualObject.VideoObject.rTime = pInfo->VisualObject.VideoObject.nTime;
                pInfo->VisualObject.VideoObject.nTime = vop_time;
            }
            mp4_StatisticInc(&pInfo->VisualObject.Statistic.nVOP_P);
#ifdef USE_NOTCODED_STATE
            pInfo->VisualObject.VideoObject.ncStateCleared = 0;
#endif
            break;
        case MP4_VOP_TYPE_B :
            status = MP4_STATUS_OK;
						            // after reset it is need to skip first B-frames
						//if (pInfo->VisualObject.VideoObject.VOPindex >= 2)
							//DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);

            if (!pInfo->VisualObject.VideoObject.prevPlaneIsB)
						{
							ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.nFrame);
							pInfo->VisualObject.nFrame = pInfo->VisualObject.cFrame;
							pInfo->VisualObject.cFrame = 0;
              pInfo->VisualObject.VideoObject.prevPlaneIsB = 1;
            }
						else 
						{
								DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
								ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
								pInfo->VisualObject.cFrame=0;
						}
						if (pInfo->VisualObject.VideoObject.VOPindex < 2)
						{
								// if we don't have both reference frames
								if (pInfo->ftype == 2)
								{
									//repeat last frame for AVI timing reasons
									DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.nFrame);
									if (pInfo->VisualObject.nFrame)
										pInfo->VisualObject.nFrame->outputted = 0;
								}
								mp4_StatisticInc(&pInfo->VisualObject.Statistic.nVOP_B);
								break;
						}

            // set Tframe for direct interlaced mode
            if (!pInfo->VisualObject.VideoObject.Tframe)
                pInfo->VisualObject.VideoObject.Tframe = (int32_t)(vop_time - pInfo->VisualObject.rFrame->time);
            if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coded) 
						{
                pInfo->VisualObject.VideoObject.TRB = (int32_t)(vop_time - pInfo->VisualObject.VideoObject.rTime);
                pInfo->VisualObject.VideoObject.TRD = (int32_t)(pInfo->VisualObject.VideoObject.nTime - pInfo->VisualObject.VideoObject.rTime);
                // defense from bad streams when B-VOPs are before Past and/or Future
                if (pInfo->VisualObject.VideoObject.TRB <= 0)
                    pInfo->VisualObject.VideoObject.TRB = 1;
                if (pInfo->VisualObject.VideoObject.TRD <= 0)
                    pInfo->VisualObject.VideoObject.TRD = 2;
                if (pInfo->VisualObject.VideoObject.TRD <= pInfo->VisualObject.VideoObject.TRB) 
								{
                    pInfo->VisualObject.VideoObject.TRB = 1;
                    pInfo->VisualObject.VideoObject.TRD = 2;
                }
                if (pInfo->VisualObject.VideoObject.Tframe >= pInfo->VisualObject.VideoObject.TRD)
                    pInfo->VisualObject.VideoObject.Tframe = pInfo->VisualObject.VideoObject.TRB;
                status = mp4_DecodeVOP_B(pInfo);
            }
            mp4_StatisticInc(&pInfo->VisualObject.Statistic.nVOP_B);
            break;
        case MP4_VOP_TYPE_S :
            // set new video frame
            if (pInfo->VisualObject.VideoObject.sprite_enable == MP4_SPRITE_GMC) 
						{
                if (pInfo->VisualObject.VideoObject.VOPindex > 0) 
								{
                    if (pInfo->VisualObject.VideoObject.prevPlaneIsB) 
										{
																						DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
											ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
											pInfo->VisualObject.cFrame=0;
											DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.nFrame);
																	ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
													pInfo->VisualObject.rFrame = pInfo->VisualObject.nFrame;
													pInfo->VisualObject.nFrame = 0;
                        pInfo->VisualObject.VideoObject.prevPlaneIsB = 0;
                    }
										else 
										{
												DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
															ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
													pInfo->VisualObject.rFrame = pInfo->VisualObject.cFrame;
													pInfo->VisualObject.cFrame = 0;
                    }
                }
            }
						else
						{
								DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
								ReleaseFrame(&pInfo->VisualObject, pInfo->VisualObject.cFrame);
								pInfo->VisualObject.cFrame=0;
						}

            if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coded) 
						{
                status = mp4_DecodeVOP_S(pInfo);
                if (pInfo->VisualObject.VideoObject.sprite_enable == MP4_SPRITE_GMC)
								{
                    mp4_PadFrame(pInfo);
                    // set past and future time for B-VOP
                    pInfo->VisualObject.VideoObject.rTime = pInfo->VisualObject.VideoObject.nTime;
                    pInfo->VisualObject.VideoObject.nTime = vop_time;
                }
#ifdef USE_NOTCODED_STATE
                // Clear not_coded MB state
                if ((pInfo->VisualObject.VideoObject.sprite_enable != MP4_SPRITE_STATIC) && pInfo->VisualObject.VideoObject.obmc_disable && !pInfo->VisualObject.VideoObject.ncStateCleared) 
								{
                    ippsZero_8u(pInfo->VisualObject.VideoObject.ncState, pInfo->VisualObject.VideoObject.MacroBlockPerVOP);
                    pInfo->VisualObject.VideoObject.ncStateCleared = 1;
                }
#endif
            }
            mp4_StatisticInc(&pInfo->VisualObject.Statistic.nVOP_S);
            break;
        }
        if (!pInfo->VisualObject.VideoObject.VideoObjectPlane.coded)
				{
						//pInfo->VisualObject.rFrame->outputted = 0;
						//pInfo->VisualObject.cFrame = pInfo->VisualObject.rFrame;
						//pInfo->VisualObject.rFrame=0;
						//DisplayFrame(&pInfo->VisualObject, pInfo->VisualObject.rFrame);
            //ippsCopy_8u(pInfo->VisualObject.rFrame->apY, pInfo->VisualObject.cFrame->apY, pInfo->VisualObject.cFrame->stepY * ((pInfo->VisualObject.VideoObject.MacroBlockPerCol + 2) << 4));
            //ippsCopy_8u(pInfo->VisualObject.rFrame->apCb, pInfo->VisualObject.cFrame->apCb, pInfo->VisualObject.cFrame->stepCb * ((pInfo->VisualObject.VideoObject.MacroBlockPerCol + 2) << 3));
            //ippsCopy_8u(pInfo->VisualObject.rFrame->apCr, pInfo->VisualObject.cFrame->apCr, pInfo->VisualObject.cFrame->stepCr * ((pInfo->VisualObject.VideoObject.MacroBlockPerCol + 2) << 3));
#ifdef USE_NOTCODED_STATE
            ippsSet_8u(1, pInfo->VisualObject.VideoObject.ncState, pInfo->VisualObject.VideoObject.MacroBlockPerVOP);
            pInfo->VisualObject.VideoObject.ncStateCleared = 0;
#endif
        }
        mp4_StatisticInc(&pInfo->VisualObject.Statistic.nVOP);
    }
		if (pInfo->VisualObject.cFrame)
		{
			// save current VOP type
			pInfo->VisualObject.cFrame->type = pInfo->VisualObject.VideoObject.VideoObjectPlane.coding_type;
			// save current VOP time
			pInfo->VisualObject.cFrame->time = vop_time;
		}
    return status;
}

/*
//  Intra DC and AC reconstruction for SVH macroblock
*/
mp4_Status mp4_DecodeIntraMB_SVH(mp4_Info *pInfo, int32_t pat, int32_t quant, uint8_t *pR[], int32_t stepR[])
{
    __ALIGN16(int16_t, coeff, 64);
    int32_t  blockNum, pm = 32, lnz;

    for (blockNum = 0; blockNum < 6; blockNum ++) {
        if (ippiReconstructCoeffsIntra_H263_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeff, &lnz, pat & pm, quant, 0, IPPVC_SCAN_ZIGZAG, 0) != ippStsNoErr) {
            mp4_Error("Error: decoding coefficients of Inter block");
            return MP4_STATUS_ERROR;
        }
        if (lnz > 0) {
            ippiDCT8x8Inv_16s8u_C1R(coeff, pR[blockNum], stepR[blockNum]);
        } else {
            mp4_Set8x8_8u(pR[blockNum], stepR[blockNum], (uint8_t)((coeff[0] + 4) >> 3));
        }
        if (pat & pm) {
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_AC);
        } else {
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_DC);
        }
        pm >>= 1;
    }
    return MP4_STATUS_OK;
}


mp4_Status mp4_DecodeInterMB_SVH(mp4_Info *pInfo, int16_t *coeffMB, int32_t quant, int32_t pat)
{
    int32_t   i, lnz, pm = 32;
    int16_t *coeff = coeffMB;

    for (i = 0; i < 6; i ++) {
        if ((pat) & pm) {
            if (ippiReconstructCoeffsInter_H263_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeff, &lnz, quant, 0) != ippStsNoErr) {
                mp4_Error("Error: decoding coefficients of Inter block");
                return MP4_STATUS_ERROR;
            }
            if (lnz != 0) {
                if ((lnz <= 4) && (coeff[16] == 0))
                    ippiDCT8x8Inv_2x2_16s_C1I(coeff);
                else if ((lnz <= 13) && (coeff[32] == 0))
                    ippiDCT8x8Inv_4x4_16s_C1I(coeff);
                else
                    ippiDCT8x8Inv_16s_C1I(coeff);
            } else {
                mp4_Set64_16s((int16_t)((coeff[0] + 4) >> 3), coeff);
            }
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_C);
        } else {
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
        }
        pm >>= 1;
        coeff += 64;
    }
    return MP4_STATUS_OK;
}

/*
//  Intra DC and AC reconstruction for macroblock
*/
mp4_Status mp4_DecodeIntraMB(mp4_Info *pInfo, int32_t x, int32_t pat, int32_t quant, int32_t dcVLC, int32_t ac_pred_flag, uint8_t *pR[], int32_t stepR[])
{
    __ALIGN16(int16_t, coeff, 64);
    int32_t      blockNum, lnz, predDir, scan, dc, dcA, dcB, dcC, dcP, k, nz, predQuantA, predQuantC, dcScaler, pm = 32;
    int16_t      *predAcA, *predAcC, sDC = 0;
    mp4_IntraPredBlock *bCurr;

    for (blockNum = 0; blockNum < 6; blockNum ++) {
        // find prediction direction
        bCurr = &pInfo->VisualObject.VideoObject.IntraPredBuff.block[6*x+blockNum];
        dcA = bCurr->predA->dct_dc >= 0 ? bCurr->predA->dct_dc : 1024;
        dcB = bCurr->predB->dct_dc >= 0 ? bCurr->predB->dct_dc : 1024;
        dcC = bCurr->predC->dct_dc >= 0 ? bCurr->predC->dct_dc : 1024;
        if (mp4_ABS(dcA - dcB) < mp4_ABS(dcB - dcC)) {
            predDir = IPPVC_SCAN_HORIZONTAL;
            dcP = dcC;
        } else {
            predDir = IPPVC_SCAN_VERTICAL;
            dcP = dcA;
        }
        scan = IPPVC_SCAN_ZIGZAG;
        if (pInfo->VisualObject.VideoObject.VideoObjectPlane.alternate_vertical_scan_flag)
            scan = IPPVC_SCAN_VERTICAL;
        else if (ac_pred_flag)
            scan = predDir;
        // decode coeffs
        if (dcVLC) {
            if (ippiDecodeDCIntra_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeff, (blockNum < 4) ? IPPVC_BLOCK_LUMA : IPPVC_BLOCK_CHROMA) != ippStsNoErr) {
                mp4_Error("Error: decoding DC coefficient of Intra block");
                return MP4_STATUS_ERROR;
            }
        }
        if (pat & pm) {
            if (ippiDecodeCoeffsIntra_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeff, &lnz, 0, dcVLC, scan) != ippStsNoErr) {
                mp4_Error("Error: decoding coefficients of Intra block");
                return MP4_STATUS_ERROR;
            }
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_AC);
        } else {
            if (dcVLC)
                sDC = coeff[0];
            mp4_Zero64_16s(coeff);
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_DC);
            lnz = 0;
            if (dcVLC)
                coeff[0] = sDC;
        }
        // predict DC
        dcScaler = (blockNum < 4) ? mp4_DCScalerLuma[quant] : mp4_DCScalerChroma[quant];
        dc = coeff[0] + mp4_DivIntraDC(dcP, dcScaler);
        mp4_CLIP(dc, -2048, 2047);
        coeff[0] = (int16_t)dc;
        // predict AC
        nz = 0;
        if (ac_pred_flag) {
            if (predDir == IPPVC_SCAN_HORIZONTAL && (bCurr->predC->dct_dc >= 0)) {
                predAcC = bCurr->predC->dct_acC;
                predQuantC = (blockNum == 2 || blockNum == 3) ? quant : pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x+1];
                if (predQuantC == quant)
                    for (k = 1; k < 8; k ++) {
                        coeff[k] = (int16_t)(coeff[k] + predAcC[k]);
                        if (coeff[k]) {
                            mp4_CLIP(coeff[k], -2048, 2047);
                            nz = 1;
                        }
                    }
                else
                    for (k = 1; k < 8; k ++) {
                        coeff[k] = (int16_t)(coeff[k] + mp4_DivIntraAC(predAcC[k] * predQuantC, quant));
                        if (coeff[k]) {
                            mp4_CLIP(coeff[k], -2048, 2047);
                            nz = 1;
                        }
                    }
            } else if (predDir == IPPVC_SCAN_VERTICAL && (bCurr->predA->dct_dc >= 0)) {
                predAcA = bCurr->predA->dct_acA;
                predQuantA = (blockNum == 1 || blockNum == 3) ? quant : pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x];
                if (predQuantA == quant)
                    for (k = 1; k < 8; k ++) {
                        coeff[k*8] = (int16_t)(coeff[k*8] + predAcA[k]);
                        if (coeff[k*8]) {
                            mp4_CLIP(coeff[k*8], -2048, 2047);
                            nz = 1;
                        }
                    }
                else
                    for (k = 1; k < 8; k ++) {
                        coeff[k*8] = (int16_t)(coeff[k*8] + mp4_DivIntraAC(predAcA[k] * predQuantA, quant));
                        if (coeff[k*8]) {
                            mp4_CLIP(coeff[k*8], -2048, 2047);
                            nz = 1;
                        }
                    }
            }
        }
        // copy predicted AC for future Prediction
        for (k = 1; k < 8; k ++) {
            bCurr[6].dct_acC[k] = coeff[k];
            bCurr[6].dct_acA[k] = coeff[k*8];
        }
        if ((nz | lnz) || (pInfo->VisualObject.VideoObject.quant_type == 1)) {
            ippiQuantInvIntra_MPEG4_16s_C1I(coeff, 63, pInfo->VisualObject.VideoObject.QuantInvIntraSpec, quant, (blockNum < 4) ? IPPVC_BLOCK_LUMA : IPPVC_BLOCK_CHROMA);
            ippiDCT8x8Inv_16s8u_C1R(coeff, pR[blockNum], stepR[blockNum]);
        } else {
            k = coeff[0] * dcScaler;
            mp4_CLIP(k, -2048, 2047);
            coeff[0] = (int16_t)k;
            k = (k + 4) >> 3;
            mp4_CLIP(k, 0, 255);
            mp4_Set8x8_8u(pR[blockNum], stepR[blockNum], (uint8_t)k);
        }
        // copy DC for future Prediction
        if (blockNum >= 3)
            pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[blockNum].dct_dc = bCurr[6].dct_dc;
        bCurr[6].dct_dc = coeff[0];
        // copy quant
        if (blockNum == 5)
            pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x+1] = (uint8_t)quant;
        pm >>= 1;
    }
    return MP4_STATUS_OK;
}

/*
//  Intra DC and AC reconstruction for DP macroblock
*/
mp4_Status mp4_DecodeIntraMB_DP(mp4_Info *pInfo, int16_t dct_dc[], int32_t x, int32_t pat, int32_t quant, int32_t dcVLC, int32_t ac_pred_flag, uint8_t *pR[], int32_t stepR[])
{
    __ALIGN16(int16_t, coeff, 64);
    int32_t      blockNum, lnz, predDir, scan, dc, dcA, dcB, dcC, dcP, k, nz, predQuantA, predQuantC, dcScaler, pm = 32;
    int16_t      *predAcA, *predAcC;
    mp4_IntraPredBlock *bCurr;

    for (blockNum = 0; blockNum < 6; blockNum ++) {
        // find prediction direction
        bCurr = &pInfo->VisualObject.VideoObject.IntraPredBuff.block[6*x+blockNum];
        dcA = bCurr->predA->dct_dc >= 0 ? bCurr->predA->dct_dc : 1024;
        dcB = bCurr->predB->dct_dc >= 0 ? bCurr->predB->dct_dc : 1024;
        dcC = bCurr->predC->dct_dc >= 0 ? bCurr->predC->dct_dc : 1024;
        if (mp4_ABS(dcA - dcB) < mp4_ABS(dcB - dcC)) {
            predDir = IPPVC_SCAN_HORIZONTAL;
            dcP = dcC;
        } else {
            predDir = IPPVC_SCAN_VERTICAL;
            dcP = dcA;
        }
        scan = (ac_pred_flag) ? predDir : IPPVC_SCAN_ZIGZAG;
        // decode coeffs
        if (pat & pm) {
            if (ippiDecodeCoeffsIntra_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeff, &lnz, pInfo->VisualObject.VideoObject.reversible_vlc, dcVLC, scan) != ippStsNoErr) {
                mp4_Error("Error: decoding coefficients of Intra block");
                return MP4_STATUS_ERROR;
            }
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_AC);
        } else {
            mp4_Zero64_16s(coeff);
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_DC);
            lnz = 0;
        }
        if (dcVLC)
            coeff[0] = dct_dc[blockNum];
        // predict DC
        dcScaler = (blockNum < 4) ? mp4_DCScalerLuma[quant] : mp4_DCScalerChroma[quant];
        dc = coeff[0] + mp4_DivIntraDC(dcP, dcScaler);
        mp4_CLIP(dc, -2048, 2047);
        coeff[0] = (int16_t)dc;
        // predict AC
        nz = 0;
        if (ac_pred_flag) {
            if (predDir == IPPVC_SCAN_HORIZONTAL && (bCurr->predC->dct_dc >= 0)) {
                predAcC = bCurr->predC->dct_acC;
                predQuantC = (blockNum == 2 || blockNum == 3) ? quant : pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x+1];
                if (predQuantC == quant)
                    for (k = 1; k < 8; k ++) {
                        coeff[k] = (int16_t)(coeff[k] + predAcC[k]); // clip ??
                        if (coeff[k]) {
                            mp4_CLIP(coeff[k], -2048, 2047);
                            nz = 1;
                        }
                    }
                else
                    for (k = 1; k < 8; k ++) {
                        coeff[k] = (int16_t)(coeff[k] + mp4_DivIntraAC(predAcC[k] * predQuantC, quant));
                        if (coeff[k]) {
                            mp4_CLIP(coeff[k], -2048, 2047);
                            nz = 1;
                        }
                    }
            } else if (predDir == IPPVC_SCAN_VERTICAL && (bCurr->predA->dct_dc >= 0)) {
                predAcA = bCurr->predA->dct_acA;
                predQuantA = (blockNum == 1 || blockNum == 3) ? quant : pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x];
                if (predQuantA == quant)
                    for (k = 1; k < 8; k ++) {
                        coeff[k*8] = (int16_t)(coeff[k*8] + predAcA[k]);
                        if (coeff[k*8]) {
                            mp4_CLIP(coeff[k*8], -2048, 2047);
                            nz = 1;
                        }
                    }
                else
                    for (k = 1; k < 8; k ++) {
                        coeff[k*8] = (int16_t)(coeff[k*8] + mp4_DivIntraAC(predAcA[k] * predQuantA, quant));
                        if (coeff[k*8]) {
                            mp4_CLIP(coeff[k*8], -2048, 2047);
                            nz = 1;
                        }
                    }
            }
        }
        // copy predicted AC for future Prediction
        for (k = 1; k < 8; k ++) {
            bCurr[6].dct_acC[k] = coeff[k];
            bCurr[6].dct_acA[k] = coeff[k*8];
        }
        if ((nz | lnz) || (pInfo->VisualObject.VideoObject.quant_type == 1)) {
            ippiQuantInvIntra_MPEG4_16s_C1I(coeff, 63, pInfo->VisualObject.VideoObject.QuantInvIntraSpec, quant, (blockNum < 4) ? IPPVC_BLOCK_LUMA : IPPVC_BLOCK_CHROMA);
            ippiDCT8x8Inv_16s8u_C1R(coeff, pR[blockNum], stepR[blockNum]);
        } else {
            k = coeff[0] * dcScaler;
            mp4_CLIP(k, -2048, 2047);
            coeff[0] = (int16_t)k;
            k = (k + 4) >> 3;
            mp4_CLIP(k, 0, 255);
            mp4_Set8x8_8u(pR[blockNum], stepR[blockNum], (uint8_t)k);
        }
        // copy DC for future Prediction
        if (blockNum >= 3)
            pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[blockNum].dct_dc = bCurr[6].dct_dc;
        bCurr[6].dct_dc = coeff[0];
        // copy quant
        if (blockNum == 5)
            pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x+1] = (uint8_t)quant;
        pm >>= 1;
    }
    return MP4_STATUS_OK;
}

/*
//  Intra DC and AC reconstruction for macroblock (w/o iDCT)
*/
mp4_Status mp4_ReconstructCoeffsIntraMB(mp4_Info *pInfo, int32_t x, int32_t pat, int32_t quant, int32_t dcVLC, int32_t ac_pred_flag, int16_t *coeffMB, int32_t lastNZ[])
{
    int32_t      blockNum, lnz, predDir, scan, dc, dcA, dcB, dcC, dcP, k, nz, predQuantA, predQuantC, dcScaler, pm = 32;
    int16_t      *predAcA, *predAcC, sDC = 0, *coeff = coeffMB;
    mp4_IntraPredBlock *bCurr;

    for (blockNum = 0; blockNum < 6; blockNum ++) {
        // find prediction direction
        bCurr = &pInfo->VisualObject.VideoObject.IntraPredBuff.block[6*x+blockNum];
        dcA = bCurr->predA->dct_dc >= 0 ? bCurr->predA->dct_dc : 1024;
        dcB = bCurr->predB->dct_dc >= 0 ? bCurr->predB->dct_dc : 1024;
        dcC = bCurr->predC->dct_dc >= 0 ? bCurr->predC->dct_dc : 1024;
        if (mp4_ABS(dcA - dcB) < mp4_ABS(dcB - dcC)) {
            predDir = IPPVC_SCAN_HORIZONTAL;
            dcP = dcC;
        } else {
            predDir = IPPVC_SCAN_VERTICAL;
            dcP = dcA;
        }
        scan = IPPVC_SCAN_ZIGZAG;
        if (pInfo->VisualObject.VideoObject.VideoObjectPlane.alternate_vertical_scan_flag)
            scan = IPPVC_SCAN_VERTICAL;
        else if (ac_pred_flag)
            scan = predDir;
        // decode coeffs
        if (dcVLC) {
            if (ippiDecodeDCIntra_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeff, (blockNum < 4) ? IPPVC_BLOCK_LUMA : IPPVC_BLOCK_CHROMA) != ippStsNoErr) {
                mp4_Error("Error: decoding DC coefficient of Intra block");
                return MP4_STATUS_ERROR;
            }
        }
        if (pat & pm) {
            if (ippiDecodeCoeffsIntra_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeff, &lnz, 0, dcVLC, scan) != ippStsNoErr) {
                mp4_Error("Error: decoding coefficients of Intra block");
                return MP4_STATUS_ERROR;
            }
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_AC);
        } else {
            if (dcVLC)
                sDC = coeff[0];
            mp4_Zero64_16s(coeff);
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_DC);
            lnz = 0;
            if (dcVLC)
                coeff[0] = sDC;
        }
        // predict DC
        dcScaler = (blockNum < 4) ? mp4_DCScalerLuma[quant] : mp4_DCScalerChroma[quant];
        dc = coeff[0] + mp4_DivIntraDC(dcP, dcScaler);
        mp4_CLIP(dc, -2048, 2047);
        coeff[0] = (int16_t)dc;
        // predict AC
        nz = 0;
        if (ac_pred_flag) {
            if (predDir == IPPVC_SCAN_HORIZONTAL && (bCurr->predC->dct_dc >= 0)) {
                predAcC = bCurr->predC->dct_acC;
                predQuantC = (blockNum == 2 || blockNum == 3) ? quant : pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x+1];
                if (predQuantC == quant)
                    for (k = 1; k < 8; k ++) {
                        coeff[k] = (int16_t)(coeff[k] + predAcC[k]);
                        if (coeff[k]) {
                            mp4_CLIP(coeff[k], -2048, 2047);
                            nz = 1;
                        }
                    }
                else
                    for (k = 1; k < 8; k ++) {
                        coeff[k] = (int16_t)(coeff[k] + mp4_DivIntraAC(predAcC[k] * predQuantC, quant));
                        if (coeff[k]) {
                            mp4_CLIP(coeff[k], -2048, 2047);
                            nz = 1;
                        }
                    }
            } else if (predDir == IPPVC_SCAN_VERTICAL && (bCurr->predA->dct_dc >= 0)) {
                predAcA = bCurr->predA->dct_acA;
                predQuantA = (blockNum == 1 || blockNum == 3) ? quant : pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x];
                if (predQuantA == quant)
                    for (k = 1; k < 8; k ++) {
                        coeff[k*8] = (int16_t)(coeff[k*8] + predAcA[k]);
                        if (coeff[k*8]) {
                            mp4_CLIP(coeff[k*8], -2048, 2047);
                            nz = 1;
                        }
                    }
                else
                    for (k = 1; k < 8; k ++) {
                        coeff[k*8] = (int16_t)(coeff[k*8] + mp4_DivIntraAC(predAcA[k] * predQuantA, quant));
                        if (coeff[k*8]) {
                            mp4_CLIP(coeff[k*8], -2048, 2047);
                            nz = 1;
                        }
                    }
            }
        }
        // copy predicted AC for future Prediction
        for (k = 1; k < 8; k ++) {
            bCurr[6].dct_acC[k] = coeff[k];
            bCurr[6].dct_acA[k] = coeff[k*8];
        }
        if ((nz | lnz) || (pInfo->VisualObject.VideoObject.quant_type == 1)) {
            ippiQuantInvIntra_MPEG4_16s_C1I(coeff, 63, pInfo->VisualObject.VideoObject.QuantInvIntraSpec, quant, (blockNum < 4) ? IPPVC_BLOCK_LUMA : IPPVC_BLOCK_CHROMA);
            lnz = 63;
        } else {
            k = coeff[0] * dcScaler;
            mp4_CLIP(k, -2048, 2047);
            coeff[0] = (int16_t)k;
        }
        // copy DC for future Prediction
        if (blockNum >= 3)
            pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[blockNum].dct_dc = bCurr[6].dct_dc;
        bCurr[6].dct_dc = coeff[0];
        // copy quant
        if (blockNum == 5)
            pInfo->VisualObject.VideoObject.IntraPredBuff.quant[x+1] = (uint8_t)quant;
        lastNZ[blockNum] = lnz;
        pm >>= 1;
        coeff += 64;
    }
    return MP4_STATUS_OK;
}

mp4_Status mp4_DecodeInterMB(mp4_Info *pInfo, int16_t *coeffMB, int32_t quant, int32_t pat, int32_t scan)
{
    int32_t   i, lnz, pm = 32;
    int16_t *coeff = coeffMB;

    for (i = 0; i < 6; i ++) {
        if ((pat) & pm) {
            if (ippiReconstructCoeffsInter_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeff, &lnz, pInfo->VisualObject.VideoObject.reversible_vlc, scan, pInfo->VisualObject.VideoObject.QuantInvInterSpec, quant) != ippStsNoErr) {
                mp4_Error("Error: decoding coefficients of Inter block");
                return MP4_STATUS_ERROR;
            }
            if (pInfo->VisualObject.VideoObject.quant_type == 0 || (coeff[63] == 0)) {
                if (lnz != 0) {
                    if (scan == IPPVC_SCAN_ZIGZAG) {
                        if ((lnz <= 4) && (coeff[16] == 0))
                            ippiDCT8x8Inv_2x2_16s_C1I(coeff);
                        else if ((lnz <= 13) && (coeff[32] == 0))
                            ippiDCT8x8Inv_4x4_16s_C1I(coeff);
                        else
                            ippiDCT8x8Inv_16s_C1I(coeff);
                    } else {  // IPPVC_SCAN_VERTICAL
                        if ((lnz <= 5) && (coeff[16] == 0) && (coeff[24] == 0))
                            ippiDCT8x8Inv_2x2_16s_C1I(coeff);
                        else if (lnz <= 9)
                            ippiDCT8x8Inv_4x4_16s_C1I(coeff);
                        else
                            ippiDCT8x8Inv_16s_C1I(coeff);
                    }
                } else {
                    mp4_Set64_16s((int16_t)((coeff[0] + 4) >> 3), coeff);
                }
            } else {
                ippiDCT8x8Inv_16s_C1I(coeff);
            }
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_C);
        } else {
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC);
        }
        pm >>= 1;
        coeff += 64;
    }
    return MP4_STATUS_OK;
}

void mp4_DCTInvCoeffsInterMB(mp4_Info *pInfo, int16_t *coeffMB, int32_t lastNZ[], int32_t pat, int32_t scan)
{
    int32_t   i, lnz, pm = 32;
    int16_t *coeff = coeffMB;

    for (i = 0; i < 6; i ++) {
        if ((pat) & pm) {
            if (pInfo->VisualObject.VideoObject.quant_type == 0 || (coeff[63] == 0)) {
                lnz = lastNZ[i];
                if (lnz != 0) {
                    if (scan == IPPVC_SCAN_ZIGZAG) {
                        if ((lnz <= 4) && (coeff[16] == 0))
                            ippiDCT8x8Inv_2x2_16s_C1I(coeff);
                        else if ((lnz <= 13) && (coeff[32] == 0))
                            ippiDCT8x8Inv_4x4_16s_C1I(coeff);
                        else
                            ippiDCT8x8Inv_16s_C1I(coeff);
                    } else {  // IPPVC_SCAN_VERTICAL
                        if ((lnz <= 5) && (coeff[16] == 0) && (coeff[24] == 0))
                            ippiDCT8x8Inv_2x2_16s_C1I(coeff);
                        else if (lnz <= 9)
                            ippiDCT8x8Inv_4x4_16s_C1I(coeff);
                        else
                            ippiDCT8x8Inv_16s_C1I(coeff);
                    }
                } else {
                    mp4_Set64_16s((int16_t)((coeff[0] + 4) >> 3), coeff);
                }
            } else {
                ippiDCT8x8Inv_16s_C1I(coeff);
            }
        }
        pm >>= 1;
        coeff += 64;
    }
}

/*
//  decode mcbpc and set MBtype and ChromaPattern
*/
mp4_Status mp4_DecodeMCBPC_P(mp4_Info* pInfo, int32_t *mbType, int32_t *mbPattern, int32_t stat)
{
    uint32_t      code;
    int32_t      type, pattern;

    code = mp4_ShowBits9(pInfo, 9);
    if (code >= 256) {
        type = IPPVC_MBTYPE_INTER;
        pattern = 0;
        mp4_FlushBits(pInfo, 1);
    } else {
        type = mp4_PVOPmb_type[code];
        pattern = mp4_PVOPmb_cbpc[code];
        mp4_FlushBits(pInfo, mp4_PVOPmb_bits[code]);
    }
    if (code == 0) {
        mp4_Error("Error: decoding MCBPC");
        return MP4_STATUS_ERROR;
    }
    *mbType = type;
    *mbPattern = pattern;
    if (stat) {
        if (type == IPPVC_MBTYPE_INTER)
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTER);
        else if (type == IPPVC_MBTYPE_INTER_Q)
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTER_Q);
        else if (type == IPPVC_MBTYPE_INTRA)
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTRA);
        else if (type == IPPVC_MBTYPE_INTRA_Q)
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTRA_Q);
        else if (type == IPPVC_MBTYPE_INTER4V)
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nMB_INTER4V);
    }
    return MP4_STATUS_OK;
}

mp4_Status mp4_PredictDecode1MV(mp4_Info *pInfo, mp4_MacroBlock *MBcurr, int32_t y, int32_t x)
{
    IppMotionVector *mvLeft, *mvTop, *mvRight, *mvCurr;
    int32_t           mbInRow = pInfo->VisualObject.VideoObject.MacroBlockPerRow;
    int32_t           fcode = pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_forward;
    int32_t           resync_marker_disable = pInfo->VisualObject.VideoObject.resync_marker_disable;

    // block 0
    mvCurr = MBcurr[0].mv;
    mvLeft  = MBcurr[-1].mv;
    mvTop   = MBcurr[-mbInRow].mv;
    mvRight = MBcurr[-mbInRow+1].mv;
    if (resync_marker_disable) {
        if ((y | x) == 0) {
            mvCurr[0].dx = mvCurr[0].dy = 0;
        } else if (x == 0) {
            mvCurr[0].dx = mp4_Median(0, mvTop[2].dx, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(0, mvTop[2].dy, mvRight[2].dy);
        } else if (y == 0) {
            mvCurr[0] = mvLeft[1];
        } else if (x == mbInRow - 1) {
            mvCurr[0].dx = mp4_Median(0, mvLeft[1].dx, mvTop[2].dx);
            mvCurr[0].dy = mp4_Median(0, mvLeft[1].dy, mvTop[2].dy);
        } else {
            mvCurr[0].dx = mp4_Median(mvLeft[1].dx, mvTop[2].dx, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(mvLeft[1].dy, mvTop[2].dy, mvRight[2].dy);
        }
    } else {
        int32_t   validLeft, validTop, validRight;

        if (x > 0)
            validLeft = MBcurr[-1].validPred;
        else
            validLeft = 0;
        if (y > 0)
            validTop = MBcurr[-mbInRow].validPred;
        else
            validTop = 0;
        if ((y > 0) && (x < mbInRow - 1))
            validRight = MBcurr[-mbInRow+1].validPred;
        else
            validRight = 0;
        switch ((validLeft << 2) | (validTop << 1) | validRight) {
        case 7:
            mvCurr[0].dx = mp4_Median(mvLeft[1].dx, mvTop[2].dx, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(mvLeft[1].dy, mvTop[2].dy, mvRight[2].dy);
            break;
        case 6:
            mvCurr[0].dx = mp4_Median(mvLeft[1].dx, mvTop[2].dx, 0);
            mvCurr[0].dy = mp4_Median(mvLeft[1].dy, mvTop[2].dy, 0);
            break;
        case 5:
            mvCurr[0].dx = mp4_Median(mvLeft[1].dx, 0, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(mvLeft[1].dy, 0, mvRight[2].dy);
            break;
        case 4:
            mvCurr[0] = mvLeft[1];
            break;
        case 3:
            mvCurr[0].dx = mp4_Median(0, mvTop[2].dx, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(0, mvTop[2].dy, mvRight[2].dy);
            break;
        case 2:
            mvCurr[0] = mvTop[2];
            break;
        case 1:
            mvCurr[0] = mvRight[2];
            break;
        default:
            mvCurr[0].dx = mvCurr[0].dy = 0;
            break;
        }
    }
    return mp4_DecodeMV(pInfo, mvCurr, fcode);
}

mp4_Status mp4_PredictDecode4MV(mp4_Info *pInfo, mp4_MacroBlock *MBcurr, int32_t y, int32_t x)
{
    IppMotionVector *mvLeft, *mvTop, *mvRight, *mvCurr;
    int32_t           mbInRow = pInfo->VisualObject.VideoObject.MacroBlockPerRow;
    int32_t           fcode = pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_forward;
    int32_t           resync_marker_disable = pInfo->VisualObject.VideoObject.resync_marker_disable;

    mvCurr = MBcurr[0].mv;
    mvLeft  = MBcurr[-1].mv;
    mvTop   = MBcurr[-mbInRow].mv;
    mvRight = MBcurr[-mbInRow+1].mv;
    if (resync_marker_disable) {
        // block 0
        if ((y | x) == 0) {
            mvCurr[0].dx = mvCurr[0].dy = 0;
        } else if (x == 0) {
            mvCurr[0].dx = mp4_Median(0, mvTop[2].dx, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(0, mvTop[2].dy, mvRight[2].dy);
        } else if (y == 0) {
            mvCurr[0] = mvLeft[1];
        } else if (x == mbInRow - 1) {
            mvCurr[0].dx = mp4_Median(0, mvLeft[1].dx, mvTop[2].dx);
            mvCurr[0].dy = mp4_Median(0, mvLeft[1].dy, mvTop[2].dy);
        } else {
            mvCurr[0].dx = mp4_Median(mvLeft[1].dx, mvTop[2].dx, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(mvLeft[1].dy, mvTop[2].dy, mvRight[2].dy);
        }
        if (mp4_DecodeMV(pInfo, mvCurr, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
        // block 1
        if (y == 0) {
            mvCurr[1] = mvCurr[0];
        } else if (x == mbInRow - 1) {
            mvCurr[1].dx = mp4_Median(mvCurr[0].dx, mvTop[3].dx, 0);
            mvCurr[1].dy = mp4_Median(mvCurr[0].dy, mvTop[3].dy, 0);
        } else {
            mvCurr[1].dx = mp4_Median(mvCurr[0].dx, mvTop[3].dx, mvRight[2].dx);
            mvCurr[1].dy = mp4_Median(mvCurr[0].dy, mvTop[3].dy, mvRight[2].dy);
        }
        if (mp4_DecodeMV(pInfo, mvCurr+1, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
        // block 2
        if (x == 0) {
            mvCurr[2].dx = mp4_Median(0, mvCurr[0].dx, mvCurr[1].dx);
            mvCurr[2].dy = mp4_Median(0, mvCurr[0].dy, mvCurr[1].dy);
        } else {
            mvCurr[2].dx = mp4_Median(mvLeft[3].dx, mvCurr[0].dx, mvCurr[1].dx);
            mvCurr[2].dy = mp4_Median(mvLeft[3].dy, mvCurr[0].dy, mvCurr[1].dy);
        }
        if (mp4_DecodeMV(pInfo, mvCurr+2, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
        // block 3
        mvCurr[3].dx = mp4_Median(mvCurr[2].dx, mvCurr[0].dx, mvCurr[1].dx);
        mvCurr[3].dy = mp4_Median(mvCurr[2].dy, mvCurr[0].dy, mvCurr[1].dy);
        if (mp4_DecodeMV(pInfo, mvCurr+3, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
    } else {
        int32_t   validLeft, validTop, validRight;

        if (x > 0)
            validLeft = MBcurr[-1].validPred;
        else
            validLeft = 0;
        if (y > 0)
            validTop = MBcurr[-mbInRow].validPred;
        else
            validTop = 0;
        if ((y > 0) && (x < mbInRow - 1))
            validRight = MBcurr[-mbInRow+1].validPred;
        else
            validRight = 0;
        // block 0
        switch ((validLeft << 2) | (validTop << 1) | validRight) {
        case 7:
            mvCurr[0].dx = mp4_Median(mvLeft[1].dx, mvTop[2].dx, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(mvLeft[1].dy, mvTop[2].dy, mvRight[2].dy);
            break;
        case 6:
            mvCurr[0].dx = mp4_Median(mvLeft[1].dx, mvTop[2].dx, 0);
            mvCurr[0].dy = mp4_Median(mvLeft[1].dy, mvTop[2].dy, 0);
            break;
        case 5:
            mvCurr[0].dx = mp4_Median(mvLeft[1].dx, 0, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(mvLeft[1].dy, 0, mvRight[2].dy);
            break;
        case 4:
            mvCurr[0] = mvLeft[1];
            break;
        case 3:
            mvCurr[0].dx = mp4_Median(0, mvTop[2].dx, mvRight[2].dx);
            mvCurr[0].dy = mp4_Median(0, mvTop[2].dy, mvRight[2].dy);
            break;
        case 2:
            mvCurr[0] = mvTop[2];
            break;
        case 1:
            mvCurr[0] = mvRight[2];
            break;
        default:
            mvCurr[0].dx = mvCurr[0].dy = 0;
            break;
        }
        if (mp4_DecodeMV(pInfo, mvCurr, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
        // block 1
        switch ((validTop << 1) | validRight) {
        case 3:
            mvCurr[1].dx = mp4_Median(mvCurr[0].dx, mvTop[3].dx, mvRight[2].dx);
            mvCurr[1].dy = mp4_Median(mvCurr[0].dy, mvTop[3].dy, mvRight[2].dy);
            break;
        case 2:
            mvCurr[1].dx = mp4_Median(mvCurr[0].dx, mvTop[3].dx, 0);
            mvCurr[1].dy = mp4_Median(mvCurr[0].dy, mvTop[3].dy, 0);
            break;
        case 1:
            mvCurr[1].dx = mp4_Median(mvCurr[0].dx, 0, mvRight[2].dx);
            mvCurr[1].dy = mp4_Median(mvCurr[0].dy, 0, mvRight[2].dy);
            break;
        default:
            mvCurr[1] = mvCurr[0];
            break;
        }
        if (mp4_DecodeMV(pInfo, mvCurr+1, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
        // block 2
        if (validLeft) {
            mvCurr[2].dx = mp4_Median(mvLeft[3].dx, mvCurr[0].dx, mvCurr[1].dx);
            mvCurr[2].dy = mp4_Median(mvLeft[3].dy, mvCurr[0].dy, mvCurr[1].dy);
        } else {
            mvCurr[2].dx = mp4_Median(0, mvCurr[0].dx, mvCurr[1].dx);
            mvCurr[2].dy = mp4_Median(0, mvCurr[0].dy, mvCurr[1].dy);
        }
        if (mp4_DecodeMV(pInfo, mvCurr+2, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
        // block 3
        mvCurr[3].dx = mp4_Median(mvCurr[2].dx, mvCurr[0].dx, mvCurr[1].dx);
        mvCurr[3].dy = mp4_Median(mvCurr[2].dy, mvCurr[0].dy, mvCurr[1].dy);
        if (mp4_DecodeMV(pInfo, mvCurr+3, fcode) != MP4_STATUS_OK)
            return MP4_STATUS_ERROR;
    }
    return MP4_STATUS_OK;
}

mp4_Status mp4_PredictDecodeFMV(mp4_Info *pInfo, mp4_MacroBlock *MBcurr, int32_t y, int32_t x, IppMotionVector *mvT, IppMotionVector *mvB)
{
    IppMotionVector  mvLeft = {0, 0}, mvTop = {0, 0}, mvRight = {0, 0}, mvPred;
    int32_t           mbInRow = pInfo->VisualObject.VideoObject.MacroBlockPerRow;
    int32_t           fcode = pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_forward;
    int32_t           resync_marker_disable = pInfo->VisualObject.VideoObject.resync_marker_disable;

    if (x > 0) {
        mvLeft = MBcurr[-1].mv[1];
        mvLeft.dy = (int16_t)mp4_Div2(mvLeft.dy);
    }
    if (y > 0) {
        mvTop = MBcurr[-mbInRow].mv[2];
        mvTop.dy = (int16_t)mp4_Div2(mvTop.dy);
        if (x < (mbInRow - 1)) {
            mvRight = MBcurr[-mbInRow+1].mv[2];
            mvRight.dy = (int16_t)mp4_Div2(mvRight.dy);
        }
    }
    if (resync_marker_disable) {
        if ((y | x) == 0) {
            mvPred.dx = mvPred.dy = 0;
        } else if (x == 0) {
            mvPred.dx = mp4_Median(0, mvTop.dx, mvRight.dx);
            mvPred.dy = mp4_Median(0, mvTop.dy, mvRight.dy);
        } else if (y == 0) {
            mvPred = mvLeft;
        } else if (x == mbInRow - 1) {
            mvPred.dx = mp4_Median(0, mvLeft.dx, mvTop.dx);
            mvPred.dy = mp4_Median(0, mvLeft.dy, mvTop.dy);
        } else {
            mvPred.dx = mp4_Median(mvLeft.dx, mvTop.dx, mvRight.dx);
            mvPred.dy = mp4_Median(mvLeft.dy, mvTop.dy, mvRight.dy);
        }
    } else {
        int32_t   validLeft, validTop, validRight;

        if (x > 0)
            validLeft = MBcurr[-1].validPred;
        else
            validLeft = 0;
        if (y > 0)
            validTop = MBcurr[-mbInRow].validPred;
        else
            validTop = 0;
        if ((y > 0) && (x < mbInRow - 1))
            validRight = MBcurr[-mbInRow+1].validPred;
        else
            validRight = 0;
        switch ((validLeft << 2) | (validTop << 1) | validRight) {
        case 7:
            mvPred.dx = mp4_Median(mvLeft.dx, mvTop.dx, mvRight.dx);
            mvPred.dy = mp4_Median(mvLeft.dy, mvTop.dy, mvRight.dy);
            break;
        case 6:
            mvPred.dx = mp4_Median(mvLeft.dx, mvTop.dx, 0);
            mvPred.dy = mp4_Median(mvLeft.dy, mvTop.dy, 0);
            break;
        case 5:
            mvPred.dx = mp4_Median(mvLeft.dx, 0, mvRight.dx);
            mvPred.dy = mp4_Median(mvLeft.dy, 0, mvRight.dy);
            break;
        case 4:
            mvPred = mvLeft;
            break;
        case 3:
            mvPred.dx = mp4_Median(0, mvTop.dx, mvRight.dx);
            mvPred.dy = mp4_Median(0, mvTop.dy, mvRight.dy);
            break;
        case 2:
            mvPred = mvTop;
            break;
        case 1:
            mvPred = mvRight;
            break;
        default:
            mvPred.dx = mvPred.dy = 0;
            break;
        }
    }
    *mvT = mvPred;
    if (mp4_DecodeMV(pInfo, mvT, fcode) != MP4_STATUS_OK)
        return MP4_STATUS_ERROR;
    *mvB = mvPred;
    if (mp4_DecodeMV(pInfo, mvB, fcode) != MP4_STATUS_OK)
        return MP4_STATUS_ERROR;
    // update MV buffer for future prediction
    MBcurr->mv[0].dx = MBcurr->mv[1].dx = MBcurr->mv[2].dx = MBcurr->mv[3].dx = (int16_t)mp4_Div2Round(mvT->dx + mvB->dx);
    MBcurr->mv[0].dy = MBcurr->mv[1].dy = MBcurr->mv[2].dy = MBcurr->mv[3].dy = (int16_t)(mvT->dy + mvB->dy);
    return MP4_STATUS_OK;
}

void mp4_OBMC(mp4_Info *pInfo, mp4_MacroBlock *pMBinfo, IppMotionVector *mvCur, int32_t colNum, int32_t rowNum, IppiRect limitRectL, uint8_t *pYc, int32_t stepYc, uint8_t *pYr, int32_t stepYr, int32_t cbpy, int16_t *coeffMB, int32_t dct_type)
{
    IppMotionVector mvOBMCL, mvOBMCU, mvOBMCR, mvOBMCB, *mvLeft, *mvUpper, *mvRight;
    int32_t  mbPerRow = pInfo->VisualObject.VideoObject.MacroBlockPerRow, dx, dy, rt;

    // get Right MV
    if (colNum == mbPerRow - 1)
        mvRight = &mvCur[1];
    else if (pMBinfo[1].type >= IPPVC_MBTYPE_INTRA)
        mvRight = &mvCur[1];
    else
        mvRight = pMBinfo[1].mv;
    // get Left MV
    if (colNum == 0)
        mvLeft = mvCur - 1;
    else if (pMBinfo[-1].type >= IPPVC_MBTYPE_INTRA)
        mvLeft = mvCur - 1;
    else
        mvLeft = pMBinfo[-1].mv;
    // get Upper MV
    if (rowNum == 0)
        mvUpper = mvCur - 2;
    else if (pMBinfo[-mbPerRow].type >= IPPVC_MBTYPE_INTRA)
        mvUpper = mvCur - 2;
    else
        mvUpper = pMBinfo[-mbPerRow].mv;
    dx = colNum * 16;
    dy = rowNum * 16;
    rt = pInfo->VisualObject.VideoObject.VideoObjectPlane.rounding_type;
    if (pInfo->VisualObject.VideoObject.quarter_sample) {
        mp4_LimitMVQ(&mvLeft[1], &mvOBMCL, &limitRectL, dx, dy, 8);
        mp4_LimitMVQ(&mvUpper[2], &mvOBMCU, &limitRectL, dx, dy, 8);
        mp4_LimitMVQ(&mvCur[1], &mvOBMCR, &limitRectL, dx, dy, 8);
        mp4_LimitMVQ(&mvCur[2], &mvOBMCB, &limitRectL, dx, dy, 8);
        ippiOBMC8x8QP_MPEG4_8u_C1R(pYr, stepYr, pYc, stepYc, &mvCur[0], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvOBMCB, rt);
        mp4_LimitMVQ(&mvCur[0], &mvOBMCL, &limitRectL, dx+8, dy, 8);
        mp4_LimitMVQ(&mvUpper[3], &mvOBMCU, &limitRectL, dx+8, dy, 8);
        mp4_LimitMVQ(&mvRight[0], &mvOBMCR, &limitRectL, dx+8, dy, 8);
        mp4_LimitMVQ(&mvCur[3], &mvOBMCB, &limitRectL, dx+8, dy, 8);
        ippiOBMC8x8QP_MPEG4_8u_C1R(pYr+8, stepYr, pYc+8, stepYc, &mvCur[1], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvOBMCB, rt);
        mp4_LimitMVQ(&mvLeft[3], &mvOBMCL, &limitRectL, dx, dy+8, 8);
        mp4_LimitMVQ(&mvCur[0], &mvOBMCU, &limitRectL, dx, dy+8, 8);
        mp4_LimitMVQ(&mvCur[3], &mvOBMCR, &limitRectL, dx, dy+8, 8);
        ippiOBMC8x8QP_MPEG4_8u_C1R(pYr+stepYr*8, stepYr, pYc+stepYc*8, stepYc, &mvCur[2], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvCur[2], rt);
        mp4_LimitMVQ(&mvCur[2], &mvOBMCL, &limitRectL, dx+8, dy+8, 8);
        mp4_LimitMVQ(&mvCur[1], &mvOBMCU, &limitRectL, dx+8, dy+8, 8);
        mp4_LimitMVQ(&mvRight[2], &mvOBMCR, &limitRectL, dx+8, dy+8, 8);
        ippiOBMC8x8QP_MPEG4_8u_C1R(pYr+8+stepYr*8, stepYr, pYc+8+stepYc*8, stepYc, &mvCur[3], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvCur[3], rt);
    } else {
        mp4_LimitMV(&mvLeft[1], &mvOBMCL, &limitRectL, dx, dy, 8);
        mp4_LimitMV(&mvUpper[2], &mvOBMCU, &limitRectL, dx, dy, 8);
        mp4_LimitMV(&mvCur[1], &mvOBMCR, &limitRectL, dx, dy, 8);
        mp4_LimitMV(&mvCur[2], &mvOBMCB, &limitRectL, dx, dy, 8);
        ippiOBMC8x8HP_MPEG4_8u_C1R(pYr, stepYr, pYc, stepYc, &mvCur[0], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvOBMCB, rt);
        mp4_LimitMV(&mvCur[0], &mvOBMCL, &limitRectL, dx+8, dy, 8);
        mp4_LimitMV(&mvUpper[3], &mvOBMCU, &limitRectL, dx+8, dy, 8);
        mp4_LimitMV(&mvRight[0], &mvOBMCR, &limitRectL, dx+8, dy, 8);
        mp4_LimitMV(&mvCur[3], &mvOBMCB, &limitRectL, dx+8, dy, 8);
        ippiOBMC8x8HP_MPEG4_8u_C1R(pYr+8, stepYr, pYc+8, stepYc, &mvCur[1], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvOBMCB, rt);
        mp4_LimitMV(&mvLeft[3], &mvOBMCL, &limitRectL, dx, dy+8, 8);
        mp4_LimitMV(&mvCur[0], &mvOBMCU, &limitRectL, dx, dy+8, 8);
        mp4_LimitMV(&mvCur[3], &mvOBMCR, &limitRectL, dx, dy+8, 8);
        ippiOBMC8x8HP_MPEG4_8u_C1R(pYr+stepYr*8, stepYr, pYc+stepYc*8, stepYc, &mvCur[2], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvCur[2], rt);
        mp4_LimitMV(&mvCur[2], &mvOBMCL, &limitRectL, dx+8, dy+8, 8);
        mp4_LimitMV(&mvCur[1], &mvOBMCU, &limitRectL, dx+8, dy+8, 8);
        mp4_LimitMV(&mvRight[2], &mvOBMCR, &limitRectL, dx+8, dy+8, 8);
        ippiOBMC8x8HP_MPEG4_8u_C1R(pYr+8+stepYr*8, stepYr, pYc+8+stepYc*8, stepYc, &mvCur[3], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvCur[3], rt);
    }
    if (!dct_type) {
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
}


void mp4_CopyMacroBlocks(const mp4_Frame *rFrame, mp4_Frame *cFrame, int32_t mbPerRow, int32_t rowNum, int32_t colNum, int32_t n)
{
    int32_t  i, stepYr, stepYc, stepCbr, stepCbc, stepCrr, stepCrc;
    uint8_t   *pYc, *pCbc, *pCrc, *pYr, *pCbr, *pCrr;

    if (n <= 0)
        return;
    stepYc = cFrame->stepY;
    stepCbc = cFrame->stepCb;
    stepCrc = cFrame->stepCr;
    stepYr = rFrame->stepY;
    stepCbr = rFrame->stepCb;
    stepCrr = rFrame->stepCr;
    pYc = cFrame->pY + (rowNum * stepYc + colNum) * 16;
    pCbc = cFrame->pCb + (rowNum * stepCbc + colNum) * 8;
    pCrc = cFrame->pCr + (rowNum * stepCrc + colNum) * 8;
    pYr = rFrame->pY + (rowNum * stepYr + colNum) * 16;
    pCbr = rFrame->pCb + (rowNum * stepCbr + colNum) * 8;
    pCrr = rFrame->pCr + (rowNum * stepCrr + colNum) * 8;
    for (i = rowNum * mbPerRow + colNum; i < rowNum * mbPerRow + colNum + n; i ++) {
        ippiCopy16x16_8u_C1R(pYr, stepYr, pYc, stepYc);
        ippiCopy8x8_8u_C1R(pCbr, stepCbr, pCbc, stepCbc);
        ippiCopy8x8_8u_C1R(pCrr, stepCrr, pCrc, stepCrc);
        if ((i + 1) % mbPerRow == 0) {
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
}


// added by benski
mp4_Frame *CreateFrame(mp4_VisualObject *object)
{
	if (object->frame_cache)
	{
		mp4_Frame *ret = object->frame_cache;
		object->frame_cache = ret->next;
		ret->next = 0;
		ret->reference_count = 1;
		return ret;
	}
	else
	{
		mp4_Frame *ret = (mp4_Frame *)calloc(1, sizeof(mp4_Frame));
		ret->reference_count = 1;
		ret->sprite = 0;
		ret->mbPerRow = (object->VideoObject.width + 15) >> 4;
		ret->mbPerCol = (object->VideoObject.height + 15) >> 4;
		AllocateInitFrame(ret);
		return ret;
	}
}

mp4_Frame *CreateSpriteFrame(mp4_VisualObject *object)
{
	if (object->sprite_cache)
	{
		mp4_Frame *ret = object->sprite_cache;
		object->sprite_cache = ret->next;
		ret->next = 0;
		ret->reference_count = 1;
		return ret;
	}
	else
	{
		mp4_Frame *ret = (mp4_Frame *)calloc(1, sizeof(mp4_Frame));
		ret->reference_count = 1;
		ret->sprite = 1;
		ret->mbPerRow = (object->VideoObject.sprite_width + 15)  >> 4;
		ret->mbPerCol = (object->VideoObject.sprite_height + 15)  >> 4;
		AllocateInitFrame(ret);
		return ret;
	}
}

/* to delete
free(decoder->dec.VisualObject.cFrame.mid);
decoder->dec.VisualObject.cFrame.mid = 0;
*/
void ReleaseFrame(mp4_VisualObject *object, mp4_Frame *frame)
{
	if (frame && --frame->reference_count == 0)
	{
		if (frame->outputted == 0)
		{
//			DebugBreak();
		}
		frame->outputted = 0;
		if (frame->sprite)
		{
			frame->next = object->sprite_cache;
			object->sprite_cache = frame;
		}
		else
		{
			frame->next = object->frame_cache;
			object->frame_cache = frame;
		}
	}
}

mp4_Frame *GetDisplayFrame(mp4_VisualObject *object)
{
	mp4_Frame *ret = object->display_frames;
	if (ret)
	{
		object->display_frames = ret->next;
		ret->next = 0;
	}
	return ret;
}

void DisplayFrame(mp4_VisualObject *object, mp4_Frame *frame)
{
	if (frame)
	{
		mp4_Frame *tail = object->display_frames;
		if (frame->outputted)
		{
			DebugBreak();
		}
		frame->outputted = 1;
		frame->reference_count++;
		if (tail)
		{
			while (tail->next)
			{
				tail = tail->next;
			}
			tail->next = frame;
			frame->next = 0;
		}
		else
		{
			object->display_frames = frame;
			frame->next = 0;
		}
	}
}

void FreeCache(mp4_VisualObject *object)
{
	while (object->display_frames)
	{
		mp4_Frame *frame = object->display_frames;
		object->display_frames = frame->next;
		free(frame->mid);
		free(frame);
	}

	while (object->frame_cache)
	{
		mp4_Frame *frame = object->frame_cache;
		object->frame_cache = frame->next;
		free(frame->mid);
		free(frame);
	}

	while (object->sprite_cache)
	{
		mp4_Frame *frame = object->frame_cache;
		object->frame_cache = frame->next;
		free(frame->mid);
		free(frame);
	}
}
