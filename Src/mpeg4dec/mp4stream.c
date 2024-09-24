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

#ifndef USE_INLINE_BITS_FUNC

uint32_t mp4_ShowBits(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bufptr;
    uint32_t tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8) | (ptr[3]);
    tmp <<= pInfo->bitoff;
    tmp >>= 32 - n;
    return tmp;
}

uint32_t mp4_ShowBit(mp4_Info* pInfo)
{
    uint32_t tmp = pInfo->bufptr[0];
    tmp >>= 7 - pInfo->bitoff;
    return (tmp & 1);
}

uint32_t mp4_ShowBits9(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bufptr;
    uint32_t tmp = (ptr[0] <<  8) | ptr[1];
    tmp <<= (pInfo->bitoff + 16);
    tmp >>= 32 - n;
    return tmp;
}

void mp4_FlushBits(mp4_Info* pInfo, int32_t n)
{
    n = n + pInfo->bitoff;
    pInfo->bufptr += n >> 3;
    pInfo->bitoff = n & 7;
}

uint32_t mp4_GetBits(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bufptr;
    uint32_t tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8) | (ptr[3]);
    tmp <<= pInfo->bitoff;
    tmp >>= 32 - n;
    n = n + pInfo->bitoff;
    pInfo->bufptr += n >> 3;
    pInfo->bitoff = n & 7;
    return tmp;
}

uint32_t mp4_GetBits9(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bufptr;
    uint32_t tmp = (ptr[0] << 8) | ptr[1];
    tmp <<= (pInfo->bitoff + 16);
    tmp >>= 32 - n;
    n = n + pInfo->bitoff;
    pInfo->bufptr += n >> 3;
    pInfo->bitoff = n & 7;
    return tmp;
}

void mp4_AlignBits(mp4_Info* pInfo)
{
    if (pInfo->bitoff > 0) {
        pInfo->bitoff = 0;
        (pInfo->bufptr)++;
    }
}

void mp4_AlignBits7F(mp4_Info* pInfo)
{
    if (pInfo->bitoff > 0) {
        pInfo->bitoff = 0;
        (pInfo->bufptr)++;
    } else {
        if (*pInfo->bufptr == 0x7F)
            (pInfo->bufptr)++;
    }
}

uint32_t mp4_ShowBitsAlign(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bitoff ? (pInfo->bufptr + 1) : pInfo->bufptr;
    uint32_t tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8) | (ptr[3]);
    tmp >>= 32 - n;
    return tmp;
}

uint32_t mp4_ShowBitsAlign7F(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bitoff ? (pInfo->bufptr + 1) : pInfo->bufptr;
    uint32_t tmp;
    if (!pInfo->bitoff) {
        if (*ptr == 0x7F)
            ptr ++;
    }
    tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8) | (ptr[3]);
    tmp >>= 32 - n;
    return tmp;
}

#endif

uint8_t* mp4_FindStartCodePtr(mp4_Info* pInfo)
{
    int32_t  i, len = pInfo->buflen - (pInfo->bufptr - pInfo->buffer);
    uint8_t*  ptr = pInfo->bufptr;
    for (i = 0; i < len - 3; i++) {
        if (ptr[i] == 0 && ptr[i + 1] == 0 && ptr[i + 2] == 1) {
            return ptr + i + 3;
        }
    }
    return NULL;
}

uint8_t* mp4_FindStartCodeOrShortPtr(mp4_Info* pInfo)
{
    int32_t  i, len = pInfo->buflen - (pInfo->bufptr - pInfo->buffer);
    uint8_t*  ptr = pInfo->bufptr;
    for (i = 0; i < len - 3; i++) {
        if (ptr[i] == 0 && ptr[i + 1] == 0 && ptr[i + 2] == 1) {
            return ptr + i + 3;
        }
        // short_video_header
        if (ptr[i] == 0 && ptr[i + 1] == 0 && (ptr[i + 2] & 0xFC) == 0x80) {
            return ptr + i;
        }
    }
    return NULL;
}

int32_t mp4_SeekStartCodePtr(mp4_Info* pInfo)
{
    uint8_t* ptr;

    if (pInfo->bitoff) {
        pInfo->bufptr ++;
        pInfo->bitoff = 0;
    }
    ptr = mp4_FindStartCodePtr(pInfo);
    if (ptr) {
        pInfo->bufptr = ptr;
        return 1;
    } else {
        pInfo->bufptr = pInfo->buffer + (pInfo->buflen > 3 ? pInfo->buflen - 3 : 0);
        return 0;
    }
}

int32_t mp4_SeekStartCodeOrShortPtr(mp4_Info* pInfo)
{
    uint8_t* ptr;

    if (pInfo->bitoff) {
        pInfo->bufptr ++;
        pInfo->bitoff = 0;
    }
    ptr = mp4_FindStartCodeOrShortPtr(pInfo);
    if (ptr) {
        pInfo->bufptr = ptr;
        return 1;
    } else {
        pInfo->bufptr = pInfo->buffer + (pInfo->buflen > 3 ? pInfo->buflen - 3 : 0);
        return 0;
    }
}

int32_t mp4_SeekStartCodeValue(mp4_Info* pInfo, uint8_t code)
{
    while (mp4_SeekStartCodePtr(pInfo)) {
        if (*(pInfo->bufptr) == code) {
            (pInfo->bufptr) ++;
            return 1;
        }
    }
    return 0;
}

uint8_t* mp4_FindShortVideoStartMarkerPtr(mp4_Info* pInfo)
{
    int32_t  i, len = pInfo->buflen - (pInfo->bufptr - pInfo->buffer);
    uint8_t*  ptr = pInfo->bufptr;
    for (i = 0; i < len - 3; i++) {
        if (ptr[i] == 0 && ptr[i + 1] == 0 && (ptr[i + 2] & (~3)) == 0x80) {
            return ptr + i + 2;
        }
    }
    return NULL;
}

int32_t mp4_SeekShortVideoStartMarker(mp4_Info* pInfo)
{
    uint8_t* ptr;

    if (pInfo->bitoff) {
        pInfo->bufptr ++;
        pInfo->bitoff = 0;
    }
    ptr = mp4_FindShortVideoStartMarkerPtr(pInfo);
    if (ptr) {
        pInfo->bufptr = ptr;
        return 1;
    } else {
        pInfo->bufptr = pInfo->buffer + (pInfo->buflen > 3 ? pInfo->buflen - 3 : 0);
        return 0;
    }
}

//changed pInfo->len on pInfo->buflen!!!
int32_t mp4_SeekGOBMarker(mp4_Info* pInfo)
{
    for (; pInfo->bufptr < pInfo->buffer + pInfo->buflen - 2; pInfo->bufptr ++) {
        if (pInfo->bufptr[0] == 0) {
            pInfo->bitoff = 0;
            if (pInfo->bufptr[0] == 0 && pInfo->bufptr[1] == 0 && (pInfo->bufptr[2] & (~3)) == 0x80)
                return 0;
            pInfo->bufptr --;
            for (pInfo->bitoff = 1; pInfo->bitoff <= 7; pInfo->bitoff ++) {
                if (mp4_ShowBits(pInfo, 17) == 1)
                    return 1;
            }
            pInfo->bufptr ++;
            for (pInfo->bitoff = 0; pInfo->bitoff <= 7; pInfo->bitoff ++) {
                if (mp4_ShowBits(pInfo, 17) == 1)
                    return 1;
            }
            pInfo->bufptr ++;
        }
    }
    return 0;
}

int32_t mp4_SeekResyncMarker(mp4_Info* pInfo)
{
    int32_t  rml;

    if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coding_type == MP4_VOP_TYPE_I)
        rml = 17;
    else if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coding_type == MP4_VOP_TYPE_B)
        rml = 16 + IPP_MAX(pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_forward, pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_backward);
    else
        rml = 16 + pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_forward;
    pInfo->bitoff = 0;
    for (; pInfo->bufptr < pInfo->buffer + pInfo->buflen - 2; pInfo->bufptr ++) {
        if (pInfo->bufptr[0] == 0) {
            if (pInfo->bufptr[0] == 0 && pInfo->bufptr[1] == 0 && pInfo->bufptr[2] == 1)
                return 0;
            if (mp4_ShowBits(pInfo, rml) == 1) {
                // check stuffing bits
                pInfo->bufptr --;
                pInfo->bitoff = 7;
                while (pInfo->bitoff > 0 && mp4_ShowBit(pInfo))
                    pInfo->bitoff --;
                if (pInfo->bitoff == 0 && mp4_ShowBit(pInfo)) {
                    // stuffing bits are invalid
                    pInfo->bufptr[0] = 0x7f;
                }
                return 1;
            }
            pInfo->bufptr += 2;
        }
    }
    return 0;
}

int32_t mp4_FindResyncMarker(mp4_Info* pInfo)
{
    int32_t  rml;

    if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coding_type == MP4_VOP_TYPE_I)
        rml = 17;
    else if (pInfo->VisualObject.VideoObject.VideoObjectPlane.coding_type == MP4_VOP_TYPE_B)
        rml = 16 + IPP_MAX(pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_forward, pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_backward);
    else
        rml = 16 + pInfo->VisualObject.VideoObject.VideoObjectPlane.fcode_forward;
    pInfo->bitoff = 0;
    for (; pInfo->bufptr < pInfo->buffer + pInfo->buflen - 2; pInfo->bufptr ++) {
        if (pInfo->bufptr[0] == 0)
        {
            if (pInfo->bufptr[0] == 0 && pInfo->bufptr[1] == 0 && pInfo->bufptr[2] == 1)
                return 0;
            if (mp4_ShowBits(pInfo, rml) == 1)
            {
                return rml;
            }
        }
    }
    return 0;
}

int mp4_IsStartCodeOrShort(mp4_Info* pInfo)
{
    if (pInfo->bufptr[0] == 0 && pInfo->bufptr[1] == 0 && (pInfo->bufptr[2] == 1 || ((pInfo->bufptr[2] & 0xFC) == 0x80)))
        return 1;
    return 0;
}


int mp4_IsStartCodeValue(mp4_Info* pInfo, int min, int max)
{
    if (pInfo->bufptr[0-3] == 0 && pInfo->bufptr[1-3] == 0 && pInfo->bufptr[2-3] == 1)
        if (pInfo->bufptr[3-3] >= min && pInfo->bufptr[3-3] <= max)
            return 1;
    return 0;
}


int mp4_IsShortCode(mp4_Info* pInfo)
{
    if (pInfo->bufptr[0] == 0 && pInfo->bufptr[1] == 0 && ((pInfo->bufptr[2] & 0xFC) == 0x80))
        return 1;
    return 0;
}

