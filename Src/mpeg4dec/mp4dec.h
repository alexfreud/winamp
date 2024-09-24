/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2001-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    MPEG-4 header.
//
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//#ifdef __INTEL_COMPILER
//#include <emmintrin.h>
//#define USE_INTRINSIC_EMM
//#else
#undef USE_INTRINSIC_XMM
#undef USE_INTRINSIC_EMM
//#endif

#define USE_TABLE_INTRA_DIV

#define mp4_Div2(a) ((a) >= 0 ? ((a) >> 1) : (((a)+1) >> 1))
#define mp4_Div2Round(a) (((a) >> 1) | ((a) & 1))
#define mp4_DivRoundInf(a, b) ((((a) + (((a) >= 0) ? ((b) >> 1) : -((b) >> 1))) / (b)))
#ifndef USE_TABLE_INTRA_DIV
#define mp4_DivIntraDC(a, b) (((a) + ((b) >> 1)) / (b))
#define mp4_DivIntraAC(a, b) mp4_DivRoundInf(a, b)
#else
// tested on (-2047..2047) // (1..46)
#define mp4_DivIntraDC(a, b) (((a) * mp4_DivIntraDivisor[b] + (1 << 17)) >> 18)
#define mp4_DivIntraAC(a, b) mp4_DivIntraDC(a, b)
#endif

__INLINE int16_t mp4_Median(int16_t a, int16_t b, int16_t c)
{
    if (a > b) {
        int16_t  t = a; a = b; b = t;
    }
    return (int16_t)((b <= c) ? b : (c >= a) ? c : a);
}

__INLINE void mp4_ComputeChromaMV(const IppMotionVector *mvLuma, IppMotionVector *mvChroma)
{
    mvChroma->dx = (int16_t)mp4_Div2Round(mvLuma->dx);
    mvChroma->dy = (int16_t)mp4_Div2Round(mvLuma->dy);
}

__INLINE void mp4_ComputeChromaMVQ(const IppMotionVector *mvLuma, IppMotionVector *mvChroma)
{
    int32_t  dx, dy;

    dx = mp4_Div2(mvLuma->dx);
    dy = mp4_Div2(mvLuma->dy);
    mvChroma->dx = (int16_t)mp4_Div2Round(dx);
    mvChroma->dy = (int16_t)mp4_Div2Round(dy);
}

__INLINE void mp4_ComputeChroma4MV(const IppMotionVector mvLuma[4], IppMotionVector *mvChroma)
{
    int32_t  dx, dy, cdx, cdy, adx, ady;

    dx = mvLuma[0].dx + mvLuma[1].dx + mvLuma[2].dx + mvLuma[3].dx;
    dy = mvLuma[0].dy + mvLuma[1].dy + mvLuma[2].dy + mvLuma[3].dy;
    adx = abs(dx);
    ady = abs(dy);
    cdx = mp4_cCbCrMvRound16[adx & 15] + (adx >> 4) * 2;
    cdy = mp4_cCbCrMvRound16[ady & 15] + (ady >> 4) * 2;
    mvChroma->dx = (int16_t)((dx >= 0) ? cdx : -cdx);
    mvChroma->dy = (int16_t)((dy >= 0) ? cdy : -cdy);
}

__INLINE void mp4_ComputeChroma4MVQ(const IppMotionVector mvLuma[4], IppMotionVector *mvChroma)
{
    int32_t  dx, dy, cdx, cdy, adx, ady;

    dx = mp4_Div2(mvLuma[0].dx) + mp4_Div2(mvLuma[1].dx) + mp4_Div2(mvLuma[2].dx) + mp4_Div2(mvLuma[3].dx);
    dy = mp4_Div2(mvLuma[0].dy) + mp4_Div2(mvLuma[1].dy) + mp4_Div2(mvLuma[2].dy) + mp4_Div2(mvLuma[3].dy);
    adx = abs(dx);
    ady = abs(dy);
    cdx = mp4_cCbCrMvRound16[adx & 15] + (adx >> 4) * 2;
    cdy = mp4_cCbCrMvRound16[ady & 15] + (ady >> 4) * 2;
    mvChroma->dx = (int16_t)((dx >= 0) ? cdx : -cdx);
    mvChroma->dy = (int16_t)((dy >= 0) ? cdy : -cdy);
}

#define limitMV(dx, xmin, xmax, mvd) \
{ \
    if ((dx) < (xmin)) \
        mvd = (int16_t)(xmin); \
    else if ((dx) >= (xmax)) \
        mvd = (int16_t)(xmax); \
    else \
        mvd = (int16_t)(dx); \
}

__INLINE void mp4_LimitMV(const IppMotionVector *pSrcMV, IppMotionVector *pDstMV, const IppiRect *limitRect, int32_t x, int32_t y, int32_t size)
{
    limitMV(pSrcMV->dx, (limitRect->x - x) << 1, (limitRect->x - x + limitRect->width  - size) << 1, pDstMV->dx);
    limitMV(pSrcMV->dy, (limitRect->y - y) << 1, (limitRect->y - y + limitRect->height - size) << 1, pDstMV->dy);
}

__INLINE void mp4_LimitMVQ(const IppMotionVector *pSrcMV, IppMotionVector *pDstMV, const IppiRect *limitRect, int32_t x, int32_t y, int32_t size)
{
    limitMV(pSrcMV->dx, (limitRect->x - x) << 2, (limitRect->x - x + limitRect->width  - size) << 2, pDstMV->dx);
    limitMV(pSrcMV->dy, (limitRect->y - y) << 2, (limitRect->y - y + limitRect->height - size) << 2, pDstMV->dy);
}

__INLINE void mp4_Limit4MV(const IppMotionVector *pSrcMV, IppMotionVector *pDstMV, const IppiRect *limitRect, int32_t x, int32_t y, int32_t size)
{
    mp4_LimitMV(&pSrcMV[0], &pDstMV[0], limitRect, x       , y,        size);
    mp4_LimitMV(&pSrcMV[1], &pDstMV[1], limitRect, x + size, y,        size);
    mp4_LimitMV(&pSrcMV[2], &pDstMV[2], limitRect, x       , y + size, size);
    mp4_LimitMV(&pSrcMV[3], &pDstMV[3], limitRect, x + size, y + size, size);
}

__INLINE void mp4_Limit4MVQ(const IppMotionVector *pSrcMV, IppMotionVector *pDstMV, const IppiRect *limitRect, int32_t x, int32_t y, int32_t size)
{
    mp4_LimitMVQ(&pSrcMV[0], &pDstMV[0], limitRect, x       , y,        size);
    mp4_LimitMVQ(&pSrcMV[1], &pDstMV[1], limitRect, x + size, y,        size);
    mp4_LimitMVQ(&pSrcMV[2], &pDstMV[2], limitRect, x       , y + size, size);
    mp4_LimitMVQ(&pSrcMV[3], &pDstMV[3], limitRect, x + size, y + size, size);
}

__INLINE void mp4_LimitFMV(const IppMotionVector *pSrcMV, IppMotionVector *pDstMV, const IppiRect *limitRect, int32_t x, int32_t y, int32_t size)
{
    limitMV(pSrcMV->dx, (limitRect->x - x) << 1, (limitRect->x - x + limitRect->width  - size) << 1, pDstMV->dx);
    limitMV(pSrcMV->dy << 1, (limitRect->y - y) << 1, (limitRect->y - y + limitRect->height - size) << 1, pDstMV->dy);
    pDstMV->dy >>= 1;
}

__INLINE void mp4_LimitFMVQ(const IppMotionVector *pSrcMV, IppMotionVector *pDstMV, const IppiRect *limitRect, int32_t x, int32_t y, int32_t size)
{
    limitMV(pSrcMV->dx, (limitRect->x - x) << 2, (limitRect->x - x + limitRect->width  - size) << 2, pDstMV->dx);
    limitMV(pSrcMV->dy << 1, (limitRect->y - y) << 2, (limitRect->y - y + limitRect->height - size) << 2, pDstMV->dy);
    pDstMV->dy >>= 1;
}

#define MP4_MV_OFF_HP(dx, dy, step) \
    (((dx) >> 1) + (step) * ((dy) >> 1))

#define MP4_MV_ACC_HP(dx, dy) \
    ((((dy) & 1) << 1) + ((dx) & 1))

#define MP4_MV_OFF_QP(dx, dy, step) \
    (((dx) >> 2) + (step) * ((dy) >> 2))

#define MP4_MV_ACC_QP(dx, dy) \
    ((((dy) & 3) << 2) + ((dx) & 3))

#define mp4_Copy8x4HP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy8x4HP_8u_C1R(pSrc + MP4_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, MP4_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define mp4_Copy8x8HP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy8x8HP_8u_C1R(pSrc + MP4_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, MP4_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define mp4_Copy16x8HP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy16x8HP_8u_C1R(pSrc + MP4_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, MP4_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define mp4_Copy16x16HP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy16x16HP_8u_C1R(pSrc + MP4_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, MP4_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define mp4_Copy8x8QP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy8x8QP_MPEG4_8u_C1R(pSrc + MP4_MV_OFF_QP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, MP4_MV_ACC_QP((mv)->dx, (mv)->dy), rc)

#define mp4_Copy16x8QP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy16x8QP_MPEG4_8u_C1R(pSrc + MP4_MV_OFF_QP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, MP4_MV_ACC_QP((mv)->dx, (mv)->dy), rc)

#define mp4_Copy16x16QP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy16x16QP_MPEG4_8u_C1R(pSrc + MP4_MV_OFF_QP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, MP4_MV_ACC_QP((mv)->dx, (mv)->dy), rc)

#define mp4_Add8x8HP_16s8u(pSrc, srcStep, pResid, pDst, dstStep, mv, rc) \
    ippiAdd8x8HP_16s8u_C1RS(pResid, 16, pSrc + MP4_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, MP4_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define mp4_Add8x8_16s8u(pSrcDst, pResid, srcDstStep) \
    ippiAdd8x8_16s8u_C1IRS(pResid, 16, pSrcDst, srcDstStep)


#define mp4_UpdateQuant(pInfo, quant) \
{ \
    quant += mp4_dquant[mp4_GetBits9(pInfo, 2)]; \
    mp4_CLIP(quant, 1, (1 << pInfo->VisualObject.VideoObject.quant_precision) - 1); \
}

#define mp4_UpdateQuant_B(pInfo, quant) \
if (mp4_GetBit(pInfo) != 0) { \
    quant += (mp4_GetBit(pInfo) == 0) ? -2 : 2; \
    mp4_CLIP(quant, 1, (1 << pInfo->VisualObject.VideoObject.quant_precision) - 1); \
}

__INLINE void mp4_Set8x8_8u(uint8_t *p, int32_t step, uint8_t v)
{
#if defined(USE_INTRINSIC_XMM) || defined(USE_INTRINSIC_EMM)
    __m64 _p_v = _mm_set1_pi8(v);
    *(__m64*)p = _p_v;
    *(__m64*)(p+step) = _p_v;
    p += 2 * step;
    *(__m64*)p = _p_v;
    *(__m64*)(p+step) = _p_v;
    p += 2 * step;
    *(__m64*)p = _p_v;
    *(__m64*)(p+step) = _p_v;
    p += 2 * step;
    *(__m64*)p = _p_v;
    *(__m64*)(p+step) = _p_v;
    _mm_empty();
#else
    uint32_t val;

    val = v + (v << 8);
    val += val << 16;
    ((uint32_t*)p)[0] = val; ((uint32_t*)p)[1] = val; p += step;
    ((uint32_t*)p)[0] = val; ((uint32_t*)p)[1] = val; p += step;
    ((uint32_t*)p)[0] = val; ((uint32_t*)p)[1] = val; p += step;
    ((uint32_t*)p)[0] = val; ((uint32_t*)p)[1] = val; p += step;
    ((uint32_t*)p)[0] = val; ((uint32_t*)p)[1] = val; p += step;
    ((uint32_t*)p)[0] = val; ((uint32_t*)p)[1] = val; p += step;
    ((uint32_t*)p)[0] = val; ((uint32_t*)p)[1] = val; p += step;
    ((uint32_t*)p)[0] = val; ((uint32_t*)p)[1] = val;
#endif
}

__INLINE void mp4_Set16x16_8u(uint8_t *p, int32_t step, uint8_t val)
{
    int32_t i, j;

    for (i = 0; i < 16; i ++) {
        for (j = 0; j < 16; j ++)
            p[j] = val;
        p += step;
    }
}

#if defined(USE_INTRINSIC_XMM) || defined(USE_INTRINSIC_EMM)

#define mp4_Zero4MV(mv) \
    memset(mv, 0, 4 * sizeof(IppMotionVector));

#if defined(USE_INTRINSIC_XMM)

#define mp4_Zero64_16s(pDst) \
{ \
    __m64 _p_zero = _mm_setzero_si64(); \
    ((__m64*)(pDst))[0] = _p_zero; \
    ((__m64*)(pDst))[1] = _p_zero; \
    ((__m64*)(pDst))[2] = _p_zero; \
    ((__m64*)(pDst))[3] = _p_zero; \
    ((__m64*)(pDst))[4] = _p_zero; \
    ((__m64*)(pDst))[5] = _p_zero; \
    ((__m64*)(pDst))[6] = _p_zero; \
    ((__m64*)(pDst))[7] = _p_zero; \
    ((__m64*)(pDst))[8] = _p_zero; \
    ((__m64*)(pDst))[9] = _p_zero; \
    ((__m64*)(pDst))[10] = _p_zero; \
    ((__m64*)(pDst))[11] = _p_zero; \
    ((__m64*)(pDst))[12] = _p_zero; \
    ((__m64*)(pDst))[13] = _p_zero; \
    ((__m64*)(pDst))[14] = _p_zero; \
    ((__m64*)(pDst))[15] = _p_zero; \
    _m_empty(); \
}

#define mp4_Set64_16s(val, pDst) \
{ \
    __m64 _p_val = _mm_set1_pi16((int16_t)(val)); \
    ((__m64*)(pDst))[0] = _p_val; \
    ((__m64*)(pDst))[1] = _p_val; \
    ((__m64*)(pDst))[2] = _p_val; \
    ((__m64*)(pDst))[3] = _p_val; \
    ((__m64*)(pDst))[4] = _p_val; \
    ((__m64*)(pDst))[5] = _p_val; \
    ((__m64*)(pDst))[6] = _p_val; \
    ((__m64*)(pDst))[7] = _p_val; \
    ((__m64*)(pDst))[8] = _p_val; \
    ((__m64*)(pDst))[9] = _p_val; \
    ((__m64*)(pDst))[10] = _p_val; \
    ((__m64*)(pDst))[11] = _p_val; \
    ((__m64*)(pDst))[12] = _p_val; \
    ((__m64*)(pDst))[13] = _p_val; \
    ((__m64*)(pDst))[14] = _p_val; \
    ((__m64*)(pDst))[15] = _p_val; \
    _m_empty(); \
}

#elif defined(USE_INTRINSIC_EMM)

#define mp4_Zero64_16s(pDst) \
{ \
    __m128i _p_val = _mm_setzero_si128(); \
    ((__m128i*)(pDst))[0] = _p_val; \
    ((__m128i*)(pDst))[1] = _p_val; \
    ((__m128i*)(pDst))[2] = _p_val; \
    ((__m128i*)(pDst))[3] = _p_val; \
    ((__m128i*)(pDst))[4] = _p_val; \
    ((__m128i*)(pDst))[5] = _p_val; \
    ((__m128i*)(pDst))[6] = _p_val; \
    ((__m128i*)(pDst))[7] = _p_val; \
}

#define mp4_Set64_16s(val, pDst) \
{ \
    __m128i _p_val = _mm_set1_epi16((int16_t)(val)); \
    ((__m128i*)(pDst))[0] = _p_val; \
    ((__m128i*)(pDst))[1] = _p_val; \
    ((__m128i*)(pDst))[2] = _p_val; \
    ((__m128i*)(pDst))[3] = _p_val; \
    ((__m128i*)(pDst))[4] = _p_val; \
    ((__m128i*)(pDst))[5] = _p_val; \
    ((__m128i*)(pDst))[6] = _p_val; \
    ((__m128i*)(pDst))[7] = _p_val; \
}

#endif

#else

#define mp4_Zero4MV(mv) \
    (mv)[0].dx = (mv)[0].dy = (mv)[1].dx = (mv)[1].dy = (mv)[2].dx = (mv)[2].dy = (mv)[3].dx = (mv)[3].dy = 0

#define mp4_Zero64_16s(pDst) \
{ \
    int32_t  i; \
    for (i = 0; i < 32; i += 8) { \
        ((uint32_t*)(pDst))[i] = 0; \
        ((uint32_t*)(pDst))[i+1] = 0; \
        ((uint32_t*)(pDst))[i+2] = 0; \
        ((uint32_t*)(pDst))[i+3] = 0; \
        ((uint32_t*)(pDst))[i+4] = 0; \
        ((uint32_t*)(pDst))[i+5] = 0; \
        ((uint32_t*)(pDst))[i+6] = 0; \
        ((uint32_t*)(pDst))[i+7] = 0; \
    } \
}

#define mp4_Set64_16s(val, pDst) \
{ \
    int32_t  i; \
    uint32_t  v; \
    v = ((val) << 16) + (Ipp16u)(val); \
    for (i = 0; i < 32; i += 8) { \
        ((uint32_t*)(pDst))[i] = v; \
        ((uint32_t*)(pDst))[i+1] = v; \
        ((uint32_t*)(pDst))[i+2] = v; \
        ((uint32_t*)(pDst))[i+3] = v; \
        ((uint32_t*)(pDst))[i+4] = v; \
        ((uint32_t*)(pDst))[i+5] = v; \
        ((uint32_t*)(pDst))[i+6] = v; \
        ((uint32_t*)(pDst))[i+7] = v; \
    } \
}

#endif

#define mp4_MC_HP(pat, pRef, stepRef, pCur, stepCur, coeffMB, mv, rc) \
{ \
    if (pat) { \
        mp4_Add8x8HP_16s8u(pRef, stepRef, coeffMB, pCur, stepCur, mv, rc); \
    } else { \
        mp4_Copy8x8HP_8u(pRef, stepRef, pCur, stepCur, mv, rc); \
    } \
}

#define mp4_AddResidual(pat, pc, stepc, coeffMB) \
{ \
    if (pat) { \
        mp4_Add8x8_16s8u(pc, coeffMB, stepc); \
    } \
}

#define mp4_DCTInvCoeffsIntraMB(coeffMB, lnz, pFc, stepFc) \
{ \
    int32_t  i; \
    for (i = 0; i < 6; i ++) { \
        if (lnz[i] > 0) \
            ippiDCT8x8Inv_16s8u_C1R(&coeffMB[i*64], pFc[i], stepFc[i]); \
        else { \
            int k = (coeffMB[i*64] + 4) >> 3; \
            mp4_CLIP(k, 0, 255); \
            mp4_Set8x8_8u(pFc[i], stepFc[i], (uint8_t)k); \
        } \
    } \
}

#define mp4_ReconstructCoeffsIntraMB_SVH(pInfo, coeffMB, lnz, pat, quant, err) \
{ \
    int32_t  i, pm = 32; \
    for (i = 0; i < 6; i ++) { \
        if (ippiReconstructCoeffsIntra_H263_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeffMB+i*64, &lnz[i], pat & pm, quant, 0, IPPVC_SCAN_ZIGZAG, 0) != ippStsNoErr) { \
            mp4_Error("Error: decoding coefficients of Intra block"); \
            goto err; \
        } \
        if (pat & pm) { \
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_AC); \
        } else { \
            mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTRA_DC); \
        } \
        pm >>= 1; \
    } \
}

#define mp4_ReconstructCoeffsInterMB_SVH(pInfo, coeffMB, lnz, pat, quant, err) \
{ \
    if (pat) { \
        int32_t  i, pm = 32; \
        for (i = 0; i < 6; i ++) { \
            if (pat & pm) { \
                if (ippiReconstructCoeffsInter_H263_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeffMB+i*64, &lnz[i], quant, 0) != ippStsNoErr) { \
                    mp4_Error("Error: decoding coefficients of Inter block"); \
                    goto err; \
                } \
                mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_C); \
            } else { \
                mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
            } \
            pm >>= 1; \
        } \
    } else { \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
    } \
}

#define mp4_DCTInvCoeffsInterMB_SVH(coeffMB, lastNZ, pat)  \
if (pat) { \
    int32_t   i, lnz, pm = 32; \
    int16_t *coeff = coeffMB; \
    for (i = 0; i < 6; i ++) { \
        if ((pat) & pm) { \
            lnz = lastNZ[i]; \
            if (lnz != 0) { \
                if ((lnz <= 4) && (coeff[16] == 0)) \
                    ippiDCT8x8Inv_2x2_16s_C1I(coeff); \
                else if ((lnz <= 13) && (coeff[32] == 0)) \
                    ippiDCT8x8Inv_4x4_16s_C1I(coeff); \
                else \
                    ippiDCT8x8Inv_16s_C1I(coeff); \
            } else \
                mp4_Set64_16s((int16_t)((coeff[0] + 4) >> 3), coeff); \
        } \
        pm >>= 1; \
        coeff += 64; \
    } \
}

#define mp4_DecodeMCInterBlock_SVH(pInfo, quant, pat, pRef, pCur, step, coeffMB, mv, err) \
{ \
    if (pat) { \
        int32_t lnz; \
        if (ippiReconstructCoeffsInter_H263_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeffMB, &lnz, quant, 0) != ippStsNoErr) { \
            mp4_Error("Error: decoding coefficients of Inter block"); \
            goto err; \
        } \
        if (lnz != 0) { \
            if ((lnz <= 4) && (coeffMB[16] == 0)) \
                ippiDCT8x8Inv_2x2_16s_C1I(coeffMB); \
            else if ((lnz <= 13) && (coeffMB[32] == 0)) \
                ippiDCT8x8Inv_4x4_16s_C1I(coeffMB); \
            else \
                ippiDCT8x8Inv_16s_C1I(coeffMB); \
        } else { \
            mp4_Set64_16s((int16_t)((coeffMB[0] + 4) >> 3), coeffMB); \
        } \
        mp4_Add8x8HP_16s8u(pRef, step, coeffMB, pCur, step, mv, 0); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_C); \
    } else { \
        mp4_Copy8x8HP_8u(pRef, step, pCur, step, mv, 0); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
    } \
}

// reset Intra prediction buffer on new Video_packet
#define mp4_ResetIntraPredBuffer(pInfo) \
{ \
    mp4_IntraPredBlock  *b = pInfo->VisualObject.VideoObject.IntraPredBuff.dcB; \
    int32_t              i; \
    b[3].dct_dc = b[4].dct_dc = b[5].dct_dc = -1; \
    b = pInfo->VisualObject.VideoObject.IntraPredBuff.block; \
    for (i = 0; i <= pInfo->VisualObject.VideoObject.MacroBlockPerRow; i ++) { \
        b[i*6+0].dct_dc = b[i*6+1].dct_dc = b[i*6+2].dct_dc = b[i*6+3].dct_dc = b[i*6+4].dct_dc = b[i*6+5].dct_dc = -1; \
    } \
}

// reset B-prediction blocks on new row
#define mp4_ResetIntraPredBblock(pInfo) \
{ \
    pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[3].dct_dc = \
    pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[4].dct_dc = \
    pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[5].dct_dc = -1; \
}

// mark current MB as invalid for Intra prediction and rotate buffer
#define mp4_UpdateIntraPredBuffInvalid(pInfo, colNum) \
{ \
    mp4_IntraPredBlock  *b = &pInfo->VisualObject.VideoObject.IntraPredBuff.block[colNum*6+6]; \
    pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[3].dct_dc = b[3].dct_dc; \
    pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[4].dct_dc = b[4].dct_dc; \
    pInfo->VisualObject.VideoObject.IntraPredBuff.dcB[5].dct_dc = b[5].dct_dc; \
    b[0].dct_dc = b[1].dct_dc = b[2].dct_dc = b[3].dct_dc = b[4].dct_dc = b[5].dct_dc = -1; \
    /* pInfo->VisualObject.VideoObject.IntraPredBuff.quant[colNum+1] = (uint8_t)quant; */ \
}


/* 2x2 and 4x4 DCT decision suitable for Classical Zigzag Scan only */
#define mp4_DecodeMCBlockInter_MPEG4(pat, pr, stepr, pc, stepc, mv, rt, err) \
{ \
    if (pat) { \
        int32_t lnz; \
        if (ippiReconstructCoeffsInter_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeffMB, &lnz, rvlc, scan, pInfo->VisualObject.VideoObject.QuantInvInterSpec, quant) != ippStsNoErr) { \
            mp4_Error("Error: decoding coefficients of Inter block"); \
            goto err; \
        } \
        if (pInfo->VisualObject.VideoObject.quant_type == 0 || (coeffMB[63] == 0)) { \
            if (lnz != 0) { \
                if ((lnz <= 4) && (coeffMB[16] == 0)) \
                    ippiDCT8x8Inv_2x2_16s_C1I(coeffMB); \
                else if ((lnz <= 13) && (coeffMB[32] == 0)) \
                    ippiDCT8x8Inv_4x4_16s_C1I(coeffMB); \
                else \
                    ippiDCT8x8Inv_16s_C1I(coeffMB); \
            } else { \
                mp4_Set64_16s((int16_t)((coeffMB[0] + 4) >> 3), coeffMB); \
            } \
        } else { \
            ippiDCT8x8Inv_16s_C1I(coeffMB); \
        } \
        mp4_Add8x8HP_16s8u(pr, stepr, coeffMB, pc, stepc, &mv, rt); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_C); \
    } else { \
        mp4_Copy8x8HP_8u(pr, stepr, pc, stepc, &mv, rt); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
    } \
}

/* 2x2 and 4x4 DCT decision suitable for Classical Zigzag Scan only */
#define mp4_DecodeReconBlockInter_MPEG4(pat, pc, stepc, err) \
{ \
    if (pat) { \
        int32_t lnz; \
        if (ippiReconstructCoeffsInter_MPEG4_1u16s(&pInfo->bufptr, &pInfo->bitoff, coeffMB, &lnz, rvlc, scan, pInfo->VisualObject.VideoObject.QuantInvInterSpec, quant) != ippStsNoErr) { \
            mp4_Error("Error: decoding coefficients of Inter block"); \
            goto err; \
        } \
        if (pInfo->VisualObject.VideoObject.quant_type == 0 || (coeffMB[63] == 0)) { \
            if (lnz != 0) { \
                if ((lnz <= 4) && (coeffMB[16] == 0)) \
                    ippiDCT8x8Inv_2x2_16s_C1I(coeffMB); \
                else if ((lnz <= 13) && (coeffMB[32] == 0)) \
                    ippiDCT8x8Inv_4x4_16s_C1I(coeffMB); \
                else \
                    ippiDCT8x8Inv_16s_C1I(coeffMB); \
                } else { \
                    mp4_Set64_16s((int16_t)((coeffMB[0] + 4) >> 3), coeffMB); \
                } \
            } else { \
                ippiDCT8x8Inv_16s_C1I(coeffMB); \
        } \
        mp4_Add8x8_16s8u(pc, coeffMB, stepc); \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_C); \
    } else { \
        mp4_StatisticInc_(&pInfo->VisualObject.Statistic.nB_INTER_NC); \
    } \
}

__INLINE int32_t mp4_GetMacroBlockNumberSize(int32_t nmb)
{
    int32_t  nb = 0;
    nmb --;
    do {
        nmb >>= 1;
        nb ++;
    } while (nmb);
    return nb;
}

__INLINE int32_t mp4_GetConvRatio(mp4_Info* pInfo)
{
    if (mp4_GetBit(pInfo) == 1)
        return 0;
    else
        return (mp4_GetBit(pInfo) == 0 ? 2 : 4);
}

//  decode cbpy for Inter nontransparent MB
__INLINE mp4_Status mp4_DecodeCBPY_P(mp4_Info* pInfo, int32_t *yPattern, int32_t mbType)
{
    uint32_t      code;

    code = mp4_ShowBits9(pInfo, 6);
    if (mbType < IPPVC_MBTYPE_INTRA)
        *yPattern = 15 - mp4_cbpy4[code].code;
    else
        *yPattern = mp4_cbpy4[code].code;
    if (mp4_cbpy4[code].len == 255) {
        mp4_Error("Error: decoding CBPY");
        return MP4_STATUS_ERROR;
    } else {
        mp4_FlushBits(pInfo, mp4_cbpy4[code].len);
        return MP4_STATUS_OK;
    }
}

//  decode cbpy for Intra nontransparent MB

__INLINE mp4_Status mp4_DecodeCBPY_I(mp4_Info* pInfo, int32_t *yPattern)
{
    uint32_t      code;

    code = mp4_ShowBits9(pInfo, 6);
    *yPattern = mp4_cbpy4[code].code;
    if (mp4_cbpy4[code].len == 255) {
        mp4_Error("Error: decoding CBPY");
        return MP4_STATUS_ERROR;
    } else {
        mp4_FlushBits(pInfo, mp4_cbpy4[code].len);
        return MP4_STATUS_OK;
    }
}

extern mp4_Status mp4_DecodeMVD(mp4_Info *pInfo, int32_t *mvdx,  int32_t *mvdy, int32_t fcode);
extern mp4_Status mp4_DecodeMV(mp4_Info *pInfo, IppMotionVector *mv, int32_t fcode);
extern mp4_Status mp4_Decode4MV(mp4_Info *pInfo, IppMotionVector *mv, int32_t fcode);
extern mp4_Status mp4_DecodeMV_Direct(mp4_Info *pInfo, IppMotionVector mvC[4], IppMotionVector mvForw[4], IppMotionVector mvBack[4], int32_t TRB, int32_t TRD, int32_t modb, int32_t comb_type);
extern mp4_Status mp4_DecodeMV_DirectField(mp4_Info *pInfo, int32_t mb_ftfr, int32_t mb_fbfr, IppMotionVector *mvTop, IppMotionVector *mvBottom, IppMotionVector *mvForwTop, IppMotionVector *mvForwBottom, IppMotionVector *mvBackTop, IppMotionVector *mvBackBottom, int32_t TRB, int32_t TRD, int32_t modb);
extern mp4_Status mp4_DecodeIntraMB_SVH(mp4_Info *pInfo, int32_t pat, int32_t quant, uint8_t *pR[], int32_t stepR[]);
extern mp4_Status mp4_DecodeIntraMB_DP(mp4_Info *pInfo, int16_t dct_dc[], int32_t x, int32_t pat, int32_t quant, int32_t dcVLC, int32_t ac_pred_flag, uint8_t *pR[], int32_t stepR[]);
extern mp4_Status mp4_DecodeIntraMB(mp4_Info *pInfo, int32_t x, int32_t pat, int32_t quant, int32_t dcVLC, int32_t ac_pred_flag, uint8_t *pR[], int32_t stepR[]);
extern mp4_Status mp4_DecodeInterMB_SVH(mp4_Info *pInfo, int16_t *coeffMB, int32_t quant, int32_t pat);
extern mp4_Status mp4_DecodeInterMB(mp4_Info *pInfo, int16_t *coeffMB, int32_t quant, int32_t pat, int32_t scan);
extern mp4_Status mp4_ReconstructCoeffsIntraMB(mp4_Info *pInfo, int32_t x, int32_t pat, int32_t quant, int32_t dcVLC, int32_t ac_pred_flag, int16_t *coeff, int32_t lnz[]);
extern mp4_Status mp4_DecodeMCBPC_P(mp4_Info* pInfo, int32_t *mbType, int32_t *mbPattern, int32_t stat);
extern mp4_Status mp4_PredictDecode1MV(mp4_Info *pInfo, mp4_MacroBlock *MBcurr, int32_t y, int32_t x);
extern mp4_Status mp4_PredictDecode4MV(mp4_Info *pInfo, mp4_MacroBlock *MBcurr, int32_t y, int32_t x);
extern mp4_Status mp4_PredictDecodeFMV(mp4_Info *pInfo, mp4_MacroBlock *MBcurr, int32_t y, int32_t x, IppMotionVector *mvT, IppMotionVector *mvB);
extern mp4_Status mp4_DecodeVideoObjectPlane(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVOP_I(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVOP_P(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVOP_B(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVOP_S(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVOP_I_MT(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVOP_P_MT(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVOP_B_MT(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVOP_S_MT(mp4_Info* pInfo);
extern void mp4_DCTInvCoeffsInterMB(mp4_Info *pInfo, int16_t *coeffMB, int32_t lnz[], int32_t pat, int32_t scan);
extern void mp4_PadFrame(mp4_Info* pInfo);
extern void mp4_OBMC(mp4_Info *pInfo, mp4_MacroBlock *pMBinfo, IppMotionVector *mvCur, int32_t colNum, int32_t rowNum, IppiRect limitRectL, uint8_t *pYc, int32_t stepYc, uint8_t *pYr, int32_t stepYr, int32_t cbpy, int16_t *coeffMB, int32_t dct_type);
extern mp4_Status mp4_CheckDecodeVideoPacket(mp4_Info* pInfo, int32_t *found);
extern int32_t     mp4_CheckDecodeGOB_SVH(mp4_Info* pInfo);
extern void       mp4_CopyMacroBlocks(const mp4_Frame *rFrame, mp4_Frame *cFrame, int32_t mbPerRow, int32_t rowNum, int32_t colNum, int32_t n);

extern mp4_Status mp4_Sprite_Trajectory(mp4_Info* pInfo);
extern mp4_Status mp4_PredictDecodeMV(mp4_Info *pInfo, mp4_MacroBlock *MBcurr, int32_t frGOB, int32_t y, int32_t x);
extern mp4_Status mp4_DecodeMCBPC_I(mp4_Info* pInfo, int32_t *mbType, int32_t *mbPattern);

#ifdef FLOAT_POINT_IDCT

static void fIDCT_16s8u_C1R(int16_t *coeff, uint8_t *pR, int stepR)
{
    __ALIGN16(Ipp32f, c, 64);
    int  i, j;

    for (i = 0; i < 64; i ++)
        c[i] = coeff[i];
    ippiDCT8x8Inv_32f_C1I(c);
    for (i = 0; i < 8; i ++)
        for (j = 0; j < 8; j ++)
            pR[i*stepR+j] = c[i*8+j] < 0 ? 0 : c[i*8+j] > 255 ? 255 : (int16_t)(c[i*8+j] + 0.5f);
}

static void fIDCT_16s_C1I(int16_t *coeff)
{
    __ALIGN16(Ipp32f, c, 64);
    int  i;

    for (i = 0; i < 64; i ++)
        c[i] = coeff[i];
    ippiDCT8x8Inv_32f_C1I(c);
    for (i = 0; i < 64; i ++)
        coeff[i] = c[i] < 0 ? (int16_t)(c[i] - 0.5f) : (int16_t)(c[i] + 0.5f);
}

#define ippiDCT8x8Inv_16s8u_C1R(coeff, pR, stepR) \
    fIDCT_16s8u_C1R(coeff, pR, stepR)

#define ippiDCT8x8Inv_2x2_16s_C1I(coeff) \
    fIDCT_16s_C1I(coeff)

#define ippiDCT8x8Inv_4x4_16s_C1I(coeff) \
    fIDCT_16s_C1I(coeff)

#define ippiDCT8x8Inv_16s_C1I(coeff) \
    fIDCT_16s_C1I(coeff)

#endif

#ifdef __cplusplus
}
#endif

