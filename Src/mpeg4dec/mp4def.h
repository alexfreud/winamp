/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2001-2008 Intel Corporation. All Rights Reserved.
//
//  Description:    MPEG-4 header.
//
*/

#pragma once

#include <bfc/platform/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../tools/staticlib/ipp_px.h"
#include "ippdefs.h"
#include "ippcore.h"
#include "ipps.h"
#include "ippi.h"
#include "ippvc.h"
//#include "vm_debug.h"
//#include "vm_thread.h"

#pragma warning(disable : 4710) // function not inlined
#pragma warning(disable : 4514) // unreferenced inline function has been removed CL
#pragma warning(disable : 4100) // unreferenced formal parameter CL

#ifdef __cplusplus
extern "C" {
#endif

//#define USE_INLINE_BITS_FUNC
#define USE_NOTCODED_STATE

#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
    #define __INLINE static __inline
#elif defined( __GNUC__ )
    #define __INLINE static __inline__
#else
    #define __INLINE static
#endif

#if defined(__INTEL_COMPILER) && !defined(_WIN32_WCE)
    #define __ALIGN16(type, name, size) \
        __declspec (align(16)) type name[size]
#else
    #if defined(_WIN64) || defined(WIN64) || defined(LINUX64)
        #define __ALIGN16(type, name, size) \
            uint8_t _a16_##name[(size)*sizeof(type)+15]; type *name = (type*)(((int64_t)(_a16_##name) + 15) & ~15)
    #else
        #define __ALIGN16(type, name, size) \
            uint8_t _a16_##name[(size)*sizeof(type)+15]; type *name = (type*)(((int32_t)(_a16_##name) + 15) & ~15)
    #endif
#endif

#define mp4_CLIP(x, min, max) if ((x) < (min)) (x) = (min); else if ((x) > (max)) (x) = (max)
#define mp4_CLIPR(x, max) if ((x) > (max)) (x) = (max)
#define mp4_SWAP(type, x, y) {mp4_Frame *t = (x); (x) = (y); (y) = t;}
#define mp4_ABS(a) ((a) >= 0 ? (a) : -(a))

/* Timer Info */
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)

#include <windows.h>

    typedef struct _mp4_Timer {
        LARGE_INTEGER  count;
        LARGE_INTEGER  start;
        LARGE_INTEGER  stop;
        int32_t         calls;
    } mp4_Timer;

    __INLINE void mp4_TimerStart(mp4_Timer *t)
    {
        QueryPerformanceCounter(&t->start);
    }

    __INLINE void mp4_TimerStop(mp4_Timer *t)
    {
        QueryPerformanceCounter(&t->stop);
        t->count.QuadPart += t->stop.QuadPart - t->start.QuadPart;
        t->calls ++;
    }

#define TIMER_FREQ_TYPE LARGE_INTEGER

    __INLINE void mp4_GetTimerFreq(TIMER_FREQ_TYPE *f)
    {
        QueryPerformanceFrequency(f);
    }

    __INLINE double mp4_GetTimerSec(mp4_Timer *t, TIMER_FREQ_TYPE f)
    {
        return (double)t->count.QuadPart / (double)f.QuadPart;
    }

#else  // LINUX

#include <time.h>

    typedef struct _mp4_Timer {
        clock_t  count;
        clock_t  start;
        clock_t  stop;
        int32_t   calls;
    } mp4_Timer;

    __INLINE void mp4_TimerStart(mp4_Timer *t)
    {
        t->start = clock();
    }

    __INLINE void mp4_TimerStop(mp4_Timer *t)
    {
        t->stop = clock();
        t->count += t->stop - t->start;
        t->calls ++;
    }

#define TIMER_FREQ_TYPE int32_t

    __INLINE void mp4_GetTimerFreq(TIMER_FREQ_TYPE *f)
    {
        *f = CLOCKS_PER_SEC;
    }

    __INLINE double mp4_GetTimerSec(mp4_Timer *t, TIMER_FREQ_TYPE f)
    {
        return (double)t->count / (double)f;
    }

#endif

/* number of exterior MB */
#define MP4_NUM_EXT_MB 1

/* Statistic Info */
typedef struct _mp4_Statistic {
    // VideoObjectLayer Info
    int32_t      nVOP;
    int32_t      nVOP_I;
    int32_t      nVOP_P;
    int32_t      nVOP_B;
    int32_t      nVOP_S;
    int32_t      nMB;
    int32_t      nMB_INTER;
    int32_t      nMB_INTER_Q;
    int32_t      nMB_INTRA;
    int32_t      nMB_INTRA_Q;
    int32_t      nMB_INTER4V;
    int32_t      nMB_DIRECT;
    int32_t      nMB_INTERPOLATE;
    int32_t      nMB_BACKWARD;
    int32_t      nMB_FORWARD;
    int32_t      nMB_NOTCODED;
    int32_t      nB_INTRA_DC;
    int32_t      nB_INTRA_AC;
    int32_t      nB_INTER_C;
    int32_t      nB_INTER_NC;
    // app Timing Info
    mp4_Timer   time_DecodeShow;    // decode + draw + file reading
    mp4_Timer   time_Decode;        // decode + file reading
    mp4_Timer   time_DecodeOnly;    // decode only
} mp4_Statistic;

__INLINE void mp4_StatisticInc(int32_t *s)
{
    *s = (*s) + 1;
}

// when using Full Statistic, FPS is less
#ifdef MP4_FULL_STAT
#define mp4_StatisticInc_(s) mp4_StatisticInc(s)
#define mp4_TimerStart_(t) mp4_TimerStart(t)
#define mp4_TimerStop_(t) mp4_TimerStop(t)
#else
#define mp4_StatisticInc_(s)
#define mp4_TimerStart_(t)
#define mp4_TimerStop_(t)
#endif

/* status codes */
typedef enum {
    MP4_STATUS_OK           =  0,   // no error
    MP4_STATUS_NO_MEM       = -1,   // out of memory
    MP4_STATUS_FILE_ERROR   = -2,   // file error
    MP4_STATUS_NOTSUPPORT   = -3,   // not supported mode
    MP4_STATUS_PARSE_ERROR  = -4,   // fail in parse MPEG-4 stream
    MP4_STATUS_ERROR        = -5    // unknown/unspecified error
} mp4_Status;

/* MPEG-4 start code values */
// ISO/IEC 14496-2: table 6-3
enum {
    MP4_VIDEO_OBJECT_MIN_SC       = 0x00,
    MP4_VIDEO_OBJECT_MAX_SC       = 0x1F,
    MP4_VIDEO_OBJECT_LAYER_MIN_SC = 0x20,
    MP4_VIDEO_OBJECT_LAYER_MAX_SC = 0x2F,
    MP4_FGS_BP_MIN_SC             = 0x40,
    MP4_FGS_BP_MAX_SC             = 0x5F,
    MP4_VISUAL_OBJECT_SEQUENCE_SC = 0xB0,
    MP4_VISUAL_OBJECT_SEQUENCE_EC = 0xB1,
    MP4_USER_DATA_SC              = 0xB2,
    MP4_GROUP_OF_VOP_SC           = 0xB3,
    MP4_VIDEO_SESSION_ERROR_SC    = 0xB4,
    MP4_VISUAL_OBJECT_SC          = 0xB5,
    MP4_VIDEO_OBJECT_PLANE_SC     = 0xB6,
    MP4_SLICE_SC                  = 0xB7,
    MP4_EXTENSION_SC              = 0xB8,
    MP4_FGS_VOP_SC                = 0xB9,
    MP4_FBA_OBJECT_SC             = 0xBA,
    MP4_FBA_OBJECT_PLANE_SC       = 0xBB,
    MP4_MESH_OBJECT_SC            = 0xBC,
    MP4_MESH_OBJECT_PLANE_SC      = 0xBD,
    MP4_STILL_TEXTURE_OBJECT_SC   = 0xBE,
    MP4_TEXTURE_SPATIAL_LAYER_SC  = 0xBF,
    MP4_TEXTURE_SNR_LAYER_SC      = 0xC0,
    MP4_TEXTURE_TILE_SC           = 0xC1,
    MP4_TEXTURE_SHAPE_LAYER_SC    = 0xC2,
    MP4_STUFFING_SC               = 0xC3
};

/* MPEG-4 code values */
// ISO/IEC 14496-2:2004 table 6-6
enum {
    MP4_VISUAL_OBJECT_TYPE_VIDEO     = 1,
    MP4_VISUAL_OBJECT_TYPE_TEXTURE   = 2,
    MP4_VISUAL_OBJECT_TYPE_MESH      = 3,
    MP4_VISUAL_OBJECT_TYPE_FBA       = 4,
    MP4_VISUAL_OBJECT_TYPE_3DMESH    = 5
};

// ISO/IEC 14496-2:2004 table 6-7
enum {
    MP4_VIDEO_FORMAT_COMPONENT      = 0,
    MP4_VIDEO_FORMAT_PAL            = 1,
    MP4_VIDEO_FORMAT_NTSC           = 2,
    MP4_VIDEO_FORMAT_SECAM          = 3,
    MP4_VIDEO_FORMAT_MAC            = 4,
    MP4_VIDEO_FORMAT_UNSPECIFIED    = 5
};

// ISO/IEC 14496-2:2004 table 6-8..10
enum {
    MP4_VIDEO_COLORS_FORBIDDEN         = 0,
    MP4_VIDEO_COLORS_ITU_R_BT_709      = 1,
    MP4_VIDEO_COLORS_UNSPECIFIED       = 2,
    MP4_VIDEO_COLORS_RESERVED          = 3,
    MP4_VIDEO_COLORS_ITU_R_BT_470_2_M  = 4,
    MP4_VIDEO_COLORS_ITU_R_BT_470_2_BG = 5,
    MP4_VIDEO_COLORS_SMPTE_170M        = 6,
    MP4_VIDEO_COLORS_SMPTE_240M        = 7,
    MP4_VIDEO_COLORS_GENERIC_FILM      = 8
};

// ISO/IEC 14496-2:2004 table 6-11
enum {
    MP4_VIDEO_OBJECT_TYPE_SIMPLE                     = 1,
    MP4_VIDEO_OBJECT_TYPE_SIMPLE_SCALABLE            = 2,
    MP4_VIDEO_OBJECT_TYPE_CORE                       = 3,
    MP4_VIDEO_OBJECT_TYPE_MAIN                       = 4,
    MP4_VIDEO_OBJECT_TYPE_NBIT                       = 5,
    MP4_VIDEO_OBJECT_TYPE_2DTEXTURE                  = 6,
    MP4_VIDEO_OBJECT_TYPE_2DMESH                     = 7,
    MP4_VIDEO_OBJECT_TYPE_SIMPLE_FACE                = 8,
    MP4_VIDEO_OBJECT_TYPE_STILL_SCALABLE_TEXTURE     = 9,
    MP4_VIDEO_OBJECT_TYPE_ADVANCED_REAL_TIME_SIMPLE  = 10,
    MP4_VIDEO_OBJECT_TYPE_CORE_SCALABLE              = 11,
    MP4_VIDEO_OBJECT_TYPE_ADVANCED_CODING_EFFICIENCY = 12,
    MP4_VIDEO_OBJECT_TYPE_ADVANCED_SCALABLE_TEXTURE  = 13,
    MP4_VIDEO_OBJECT_TYPE_SIMPLE_FBA                 = 14,
    MP4_VIDEO_OBJECT_TYPE_SIMPLE_STUDIO              = 15,
    MP4_VIDEO_OBJECT_TYPE_CORE_STUDIO                = 16,
    MP4_VIDEO_OBJECT_TYPE_ADVANCED_SIMPLE            = 17,
    MP4_VIDEO_OBJECT_TYPE_FINE_GRANULARITY_SCALABLE  = 18
};

// ISO/IEC 14496-2:2004 table 6.17 (maximum defined video_object_layer_shape_extension)
#define MP4_SHAPE_EXT_NUM 13
// ISO/IEC 14496-2:2004 table 6-14
enum {
    MP4_ASPECT_RATIO_FORBIDDEN  = 0,
    MP4_ASPECT_RATIO_1_1        = 1,
    MP4_ASPECT_RATIO_12_11      = 2,
    MP4_ASPECT_RATIO_10_11      = 3,
    MP4_ASPECT_RATIO_16_11      = 4,
    MP4_ASPECT_RATIO_40_33      = 5,
    MP4_ASPECT_RATIO_EXTPAR     = 15
};

// ISO/IEC 14496-2:2004 table 6-15
#define MP4_CHROMA_FORMAT_420    1
// ISO/IEC 14496-2:2004 table 6-16
enum {
    MP4_SHAPE_TYPE_RECTANGULAR  = 0,
    MP4_SHAPE_TYPE_BINARY       = 1,
    MP4_SHAPE_TYPE_BINARYONLY   = 2,
    MP4_SHAPE_TYPE_GRAYSCALE    = 3
};

// ISO/IEC 14496-2:2004 table 6-19
#define MP4_SPRITE_STATIC   1
#define MP4_SPRITE_GMC      2
// ISO/IEC 14496-2:2004 table 6-24
enum {
    MP4_VOP_TYPE_I  = 0,
    MP4_VOP_TYPE_P  = 1,
    MP4_VOP_TYPE_B  = 2,
    MP4_VOP_TYPE_S  = 3
};

// ISO/IEC 14496-2:2004 table 6-26
enum {
    MP4_SPRITE_TRANSMIT_MODE_STOP   = 0,
    MP4_SPRITE_TRANSMIT_MODE_PIECE  = 1,
    MP4_SPRITE_TRANSMIT_MODE_UPDATE = 2,
    MP4_SPRITE_TRANSMIT_MODE_PAUSE  = 3
};

// ISO/IEC 14496-2:2004 table 7-3
enum {
    MP4_BAB_TYPE_MVDSZ_NOUPDATE  = 0,
    MP4_BAB_TYPE_MVDSNZ_NOUPDATE = 1,
    MP4_BAB_TYPE_TRANSPARENT     = 2,
    MP4_BAB_TYPE_OPAQUE          = 3,
    MP4_BAB_TYPE_INTRACAE        = 4,
    MP4_BAB_TYPE_MVDSZ_INTERCAE  = 5,
    MP4_BAB_TYPE_MVDSNZ_INTERCAE = 6
};

#define MP4_DC_MARKER  0x6B001 // 110 1011 0000 0000 0001
#define MP4_MV_MARKER  0x1F001 //   1 1111 0000 0000 0001

// ISO/IEC 14496-2:2004 table G.1
enum {
    MP4_SIMPLE_PROFILE_LEVEL_1                     = 0x01,
    MP4_SIMPLE_PROFILE_LEVEL_2                     = 0x02,
    MP4_SIMPLE_PROFILE_LEVEL_3                     = 0x03,
    MP4_SIMPLE_PROFILE_LEVEL_0                     = 0x08,
    MP4_SIMPLE_SCALABLE_PROFILE_LEVEL_0            = 0x10,
    MP4_SIMPLE_SCALABLE_PROFILE_LEVEL_1            = 0x11,
    MP4_SIMPLE_SCALABLE_PROFILE_LEVEL_2            = 0x12,
    MP4_CORE_PROFILE_LEVEL_1                       = 0x21,
    MP4_CORE_PROFILE_LEVEL_2                       = 0x22,
    MP4_MAIN_PROFILE_LEVEL_2                       = 0x32,
    MP4_MAIN_PROFILE_LEVEL_3                       = 0x33,
    MP4_MAIN_PROFILE_LEVEL_4                       = 0x34,
    MP4_NBIT_PROFILE_LEVEL_2                       = 0x42,
    MP4_SCALABLE_TEXTURE_PROFILE_LEVEL_1           = 0x51,
    MP4_SIMPLE_FACE_ANIMATION_PROFILE_LEVEL_1      = 0x61,
    MP4_SIMPLE_FACE_ANIMATION_PROFILE_LEVEL_2      = 0x62,
    MP4_SIMPLE_FBA_PROFILE_LEVEL_1                 = 0x63,
    MP4_SIMPLE_FBA_PROFILE_LEVEL_2                 = 0x64,
    MP4_BASIC_ANIMATED_TEXTURE_PROFILE_LEVEL_1     = 0x71,
    MP4_BASIC_ANIMATED_TEXTURE_PROFILE_LEVEL_2     = 0x72,
    MP4_HYBRID_PROFILE_LEVEL_1                     = 0x81,
    MP4_HYBRID_PROFILE_LEVEL_2                     = 0x82,
    MP4_ADVANCED_REAL_TIME_SIMPLE_PROFILE_LEVEL_1  = 0x91,
    MP4_ADVANCED_REAL_TIME_SIMPLE_PROFILE_LEVEL_2  = 0x92,
    MP4_ADVANCED_REAL_TIME_SIMPLE_PROFILE_LEVEL_3  = 0x93,
    MP4_ADVANCED_REAL_TIME_SIMPLE_PROFILE_LEVEL_4  = 0x94,
    MP4_CORE_SCALABLE_PROFILE_LEVEL_1              = 0xA1,
    MP4_CORE_SCALABLE_PROFILE_LEVEL_2              = 0xA2,
    MP4_CORE_SCALABLE_PROFILE_LEVEL_3              = 0xA3,
    MP4_ADVANCED_CODING_EFFICIENCY_PROFILE_LEVEL_1 = 0xB1,
    MP4_ADVANCED_CODING_EFFICIENCY_PROFILE_LEVEL_2 = 0xB2,
    MP4_ADVANCED_CODING_EFFICIENCY_PROFILE_LEVEL_3 = 0xB3,
    MP4_ADVANCED_CODING_EFFICIENCY_PROFILE_LEVEL_4 = 0xB4,
    MP4_ADVANCED_CORE_PROFILE_LEVEL_1              = 0xC1,
    MP4_ADVANCED_CORE_PROFILE_LEVEL_2              = 0xC2,
    MP4_ADVANCED_SCALABLE_TEXTURE_PROFILE_LEVEL_1  = 0xD1,
    MP4_ADVANCED_SCALABLE_TEXTURE_PROFILE_LEVEL_2  = 0xD2,
    MP4_ADVANCED_SCALABLE_TEXTURE_PROFILE_LEVEL_3  = 0xD3,
    MP4_SIMPLE_STUDIO_PROFILE_LEVEL_1              = 0xE1,
    MP4_SIMPLE_STUDIO_PROFILE_LEVEL_2              = 0xE2,
    MP4_SIMPLE_STUDIO_PROFILE_LEVEL_3              = 0xE3,
    MP4_SIMPLE_STUDIO_PROFILE_LEVEL_4              = 0xE4,
    MP4_CORE_STUDIO_PROFILE_LEVEL_1                = 0xE5,
    MP4_CORE_STUDIO_PROFILE_LEVEL_2                = 0xE6,
    MP4_CORE_STUDIO_PROFILE_LEVEL_3                = 0xE7,
    MP4_CORE_STUDIO_PROFILE_LEVEL_4                = 0xE8,
    MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_0            = 0xF0,
    MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_1            = 0xF1,
    MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_2            = 0xF2,
    MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_3            = 0xF3,
    MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_4            = 0xF4,
    MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_5            = 0xF5,
    MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_3B           = 0xF7,
    MP4_FGS_PROFILE_LEVEL_0                        = 0xF8,
    MP4_FGS_PROFILE_LEVEL_1                        = 0xF9,
    MP4_FGS_PROFILE_LEVEL_2                        = 0xFA,
    MP4_FGS_PROFILE_LEVEL_3                        = 0xFB,
    MP4_FGS_PROFILE_LEVEL_4                        = 0xFC,
    MP4_FGS_PROFILE_LEVEL_5                        = 0xFD
};

/* Frame Info */
typedef struct _mp4_Frame {
    uint8_t*      apY;        // allocated with border
    uint8_t*      apCb;       // allocated with border
    uint8_t*      apCr;       // allocated with border
    int32_t      stepY;
    int32_t      stepCr;
    int32_t      stepCb;
    uint8_t*      pY;         // real pointer
    uint8_t*      pCb;        // real pointer
    uint8_t*      pCr;        // real pointer
    int32_t      type;
    int64_t      time;
    int32_t      mbPerRow;   // info for realloc VOP with Shape
    int32_t      mbPerCol;
    uint8_t*      apB;        // for binary mask
    uint8_t*      pB;
    uint8_t*      apA[3];     // for aux components
    uint8_t*      pA[3];
    uint8_t*      mid;
		uint64_t	timestamp; // user-provided timestamp
		int QP;
		unsigned int reference_count;
		int outputted; // for debugging purposes
		int sprite; // 0 - frame, 1 - sprite
		struct _mp4_Frame *next; // benski> for linked list of display & free frames
} mp4_Frame;

/* Block Info for Intra Prediction */
typedef struct _mp4_IntraPredBlock {
    struct _mp4_IntraPredBlock  *predA;
    struct _mp4_IntraPredBlock  *predB;
    struct _mp4_IntraPredBlock  *predC;
    int16_t      dct_acA[8];
    int16_t      dct_acC[8];
    int16_t      dct_dc;
} mp4_IntraPredBlock;

/* Buffer for Intra Prediction */
typedef struct _mp4_IntraPredBuff {
    uint8_t               *quant;     // quant buffer;
    mp4_IntraPredBlock  dcB[6];     // blocks for Left-Top DC only
    mp4_IntraPredBlock  *block;
} mp4_IntraPredBuff;

/* MacroBlock Info Data Partitioned mode */
typedef struct _mp4_DataPartMacroBlock {
    int16_t          dct_dc[6];
    uint8_t           type;
    uint8_t           not_coded;
    uint8_t           mcsel;
    uint8_t           ac_pred_flag;
    uint8_t           pat;
    uint8_t           quant;
} mp4_DataPartMacroBlock;

/* MacroBlock Info for Motion */
typedef struct _mp4_MacroBlock {
    IppMotionVector mv[4];
    uint8_t        validPred;     // for MV pred, OBMC
    uint8_t        type;          // for OBMC, BVOP
    uint8_t        not_coded;     // for OBMC, BVOP
    uint8_t        field_info;    // for Interlaced BVOP Direct mode
} mp4_MacroBlock;

/* Group Of Video Object Plane Info */
typedef struct _mp4_GroupOfVideoObjectPlane {
    int64_t      time_code;
    int32_t      closed_gov;
    int32_t      broken_link;
} mp4_GroupOfVideoObjectPlane;

/* Video Object Plane Info */
typedef struct _mp4_VideoObjectPlane {
    int32_t      coding_type;
    int32_t      modulo_time_base;
    int32_t      time_increment;
    int32_t      coded;
    int32_t      id;                             // verid != 1 (newpred)
    int32_t      id_for_prediction_indication;   // verid != 1 (newpred)
    int32_t      id_for_prediction;              // verid != 1 (newpred)
    int32_t      rounding_type;
    int32_t      reduced_resolution;             // verid != 1
    int32_t      vop_width;
    int32_t      vop_height;
    int32_t      vop_horizontal_mc_spatial_ref;
    int32_t      vop_vertical_mc_spatial_ref;
    int32_t      background_composition;
    int32_t      change_conv_ratio_disable;
    int32_t      vop_constant_alpha;
    int32_t      vop_constant_alpha_value;
    int32_t      intra_dc_vlc_thr;
    int32_t      top_field_first;
    int32_t      alternate_vertical_scan_flag;
    int32_t      sprite_transmit_mode;
    int32_t      warping_mv_code_du[4];
    int32_t      warping_mv_code_dv[4];
    int32_t      brightness_change_factor;
    int32_t      quant;
    int32_t      alpha_quant[3];
    int32_t      fcode_forward;
    int32_t      fcode_backward;
    int32_t      shape_coding_type;
    int32_t      load_backward_shape;
    int32_t      ref_select_code;
    int32_t      dx;
    int32_t      dy;
    int32_t      quant_scale;
    int32_t      macroblock_num;
    int32_t      vop_id;
    int32_t      vop_id_for_prediction_indication;
    int32_t      vop_id_for_prediction;
} mp4_VideoObjectPlane;

/* mp4_ComplexityEstimation Info */
typedef struct _mp4_ComplexityEstimation {
    int32_t      estimation_method;
    int32_t      shape_complexity_estimation_disable;
    int32_t      opaque;
    int32_t      transparent;
    int32_t      intra_cae;
    int32_t      inter_cae;
    int32_t      no_update;
    int32_t      upsampling;
    int32_t      texture_complexity_estimation_set_1_disable;
    int32_t      intra_blocks;
    int32_t      inter_blocks;
    int32_t      inter4v_blocks;
    int32_t      not_coded_blocks;
    int32_t      texture_complexity_estimation_set_2_disable;
    int32_t      dct_coefs;
    int32_t      dct_lines;
    int32_t      vlc_symbols;
    int32_t      vlc_bits;
    int32_t      motion_compensation_complexity_disable;
    int32_t      apm;
    int32_t      npm;
    int32_t      interpolate_mc_q;
    int32_t      forw_back_mc_q;
    int32_t      halfpel2;
    int32_t      halfpel4;
    int32_t      version2_complexity_estimation_disable;     // verid != 1
    int32_t      sadct;                                      // verid != 1
    int32_t      quarterpel;                                 // verid != 1
    int32_t      dcecs_opaque;
    int32_t      dcecs_transparent;
    int32_t      dcecs_intra_cae;
    int32_t      dcecs_inter_cae;
    int32_t      dcecs_no_update;
    int32_t      dcecs_upsampling;
    int32_t      dcecs_intra_blocks;
    int32_t      dcecs_inter_blocks;
    int32_t      dcecs_inter4v_blocks;
    int32_t      dcecs_not_coded_blocks;
    int32_t      dcecs_dct_coefs;
    int32_t      dcecs_dct_lines;
    int32_t      dcecs_vlc_symbols;
    int32_t      dcecs_vlc_bits;
    int32_t      dcecs_apm;
    int32_t      dcecs_npm;
    int32_t      dcecs_interpolate_mc_q;
    int32_t      dcecs_forw_back_mc_q;
    int32_t      dcecs_halfpel2;
    int32_t      dcecs_halfpel4;
    int32_t      dcecs_sadct;                                // verid != 1
    int32_t      dcecs_quarterpel;                           // verid != 1
} mp4_ComplexityEstimation;

/* mp4_Scalability Info */
typedef struct _mp4_ScalabilityParameters {
    int32_t      dummy;
} mp4_ScalabilityParameters;

/* VOLControlParameters Info */
typedef struct _mp4_VOLControlParameters {
    int32_t      chroma_format;
    int32_t      low_delay;
    int32_t      vbv_parameters;
    int32_t      bit_rate;
    int32_t      vbv_buffer_size;
    int32_t      vbv_occupancy;
} mp4_VOLControlParameters;

/* Video Object Plane with int16_t header Info */
typedef struct _mp4_VideoObjectPlaneH263 {
    int32_t      temporal_reference;
    int32_t      split_screen_indicator;
    int32_t      document_camera_indicator;
    int32_t      full_picture_freeze_release;
    int32_t      source_format;
    int32_t      picture_coding_type;
    int32_t      vop_quant;
    int32_t      gob_number;
    int32_t      num_gobs_in_vop;
    int32_t      num_macroblocks_in_gob;
    int32_t      gob_header_empty;
    int32_t      gob_frame_id;
    int32_t      quant_scale;
    int32_t      num_rows_in_gob;
} mp4_VideoObjectPlaneH263;

/* Video Object Info */
typedef struct _mp4_VideoObject {
// iso part
    int32_t                      id;
    int32_t                      short_video_header;
    int32_t                      random_accessible_vol;
    int32_t                      type_indication;
    int32_t                      is_identifier;
    int32_t                      verid;
    int32_t                      priority;
    int32_t                      aspect_ratio_info;
    int32_t                      aspect_ratio_info_par_width;
    int32_t                      aspect_ratio_info_par_height;
    int32_t                      is_vol_control_parameters;
    mp4_VOLControlParameters    VOLControlParameters;
    int32_t                      shape;
    int32_t                      shape_extension;                // verid != 1
    int32_t                      vop_time_increment_resolution;
    int32_t                      vop_time_increment_resolution_bits;
    int32_t                      fixed_vop_rate;
    int32_t                      fixed_vop_time_increment;
    int32_t                      width;
    int32_t                      height;
    int32_t                      interlaced;
    int32_t                      obmc_disable;
    int32_t                      sprite_enable;                  // if verid != 1 (2 bit GMC is added)
    int32_t                      sprite_width;
    int32_t                      sprite_height;
    int32_t                      sprite_left_coordinate;
    int32_t                      sprite_top_coordinate;
    int32_t                      sprite_warping_points;
    int32_t                      sprite_warping_accuracy;
    int32_t                      sprite_brightness_change;
    int32_t                      low_latency_sprite_enable;
    int32_t                      sadct_disable;                  // verid != 1
    int32_t                      not_8_bit;
    int32_t                      quant_precision;
    int32_t                      bits_per_pixel;
    int32_t                      no_gray_quant_update;
    int32_t                      composition_method;
    int32_t                      linear_composition;
    int32_t                      quant_type;
    int32_t                      load_intra_quant_mat;
    uint8_t                       intra_quant_mat[64];
    int32_t                      load_nonintra_quant_mat;
    uint8_t                       nonintra_quant_mat[64];
    int32_t                      load_intra_quant_mat_grayscale[3];
    uint8_t                       intra_quant_mat_grayscale[3][64];
    int32_t                      load_nonintra_quant_mat_grayscale[3];
    uint8_t                       nonintra_quant_mat_grayscale[3][64];
    int32_t                      quarter_sample;                 // verid != 1
    int32_t                      complexity_estimation_disable;
    mp4_ComplexityEstimation    ComplexityEstimation;
    int32_t                      resync_marker_disable;
    int32_t                      data_partitioned;
    int32_t                      reversible_vlc;
    int32_t                      newpred_enable;                 // verid != 1
    int32_t                      requested_upstream_message_type;// verid != 1
    int32_t                      newpred_segment_type;           // verid != 1
    int32_t                      reduced_resolution_vop_enable;  // verid != 1
    int32_t                      scalability;
    mp4_ScalabilityParameters   ScalabilityParameters;
    mp4_GroupOfVideoObjectPlane GroupOfVideoObjectPlane;
    mp4_VideoObjectPlane        VideoObjectPlane;
    mp4_VideoObjectPlaneH263    VideoObjectPlaneH263;
// app part
    int32_t                      VOPindex;
    int32_t                      MacroBlockPerRow;
    int32_t                      MacroBlockPerCol;
    int32_t                      MacroBlockPerVOP;
    int32_t                      mbns; // num bits for MacroBlockPerVOP
    mp4_MacroBlock*             MBinfo;
    mp4_IntraPredBuff           IntraPredBuff;
    mp4_DataPartMacroBlock*     DataPartBuff;
    IppiQuantInvIntraSpec_MPEG4*  QuantInvIntraSpec;
    IppiQuantInvInterSpec_MPEG4*  QuantInvInterSpec;
    IppiWarpSpec_MPEG4*         WarpSpec;
    // for B-VOP
    int32_t                      prevPlaneIsB;
    // for interlaced B-VOP direct mode
    int32_t                      Tframe;
    IppMotionVector*            FieldMV;
    // for B-VOP direct mode
    int32_t                      TRB, TRD;
    // time increment of past and future VOP for B-VOP
    int64_t                      rTime, nTime;
    // VOP global time
    int64_t                      vop_sync_time, vop_sync_time_b;
#ifdef USE_NOTCODED_STATE
    // not_coded MB state
    uint8_t*                      ncState;
    int32_t                      ncStateCleared;
#endif
} mp4_VideoObject;

/* StillTexture Object Info */
typedef struct _mp4_StillTextureObject {
    int32_t  dummy;
} mp4_StillTextureObject;

/* Mesh Object Info */
typedef struct _mp4_MeshObject {
    int32_t  dummy;
} mp4_MeshObject;

/* Face Object Info */
typedef struct _mp4_FaceObject {
    int32_t  dummy;
} mp4_FaceObject;

/* video_signal_type Info */
typedef struct _mp4_VideoSignalType {
    int32_t  is_video_signal_type;
    int32_t  video_format;
    int32_t  video_range;
    int32_t  is_colour_description;
    int32_t  colour_primaries;
    int32_t  transfer_characteristics;
    int32_t  matrix_coefficients;
} mp4_VideoSignalType;

/* Visual Object Info */
typedef struct _mp4_VisualObject {
    int32_t                  is_identifier;
    int32_t                  verid;
    int32_t                  priority;
    int32_t                  type;
    mp4_VideoSignalType     VideoSignalType;
    mp4_VideoObject         VideoObject;
    mp4_StillTextureObject  StillTextureObject;
    mp4_MeshObject          MeshObject;
    mp4_FaceObject          FaceObject;
    mp4_Frame              *sFrame;  // static sprite
    mp4_Frame              *cFrame;  // current TODO make pointer
    mp4_Frame              *rFrame;  // reference in past TODO make pointer
    mp4_Frame              *nFrame;  // reference in future TODO make pointer
    int32_t                  frameCount;
    int32_t                  frameInterval;
    int32_t                  frameScale;
    mp4_Statistic           Statistic;
		mp4_Frame *frame_cache; // linked list of available frames (malloc cache)
		mp4_Frame *sprite_cache; // linked list of available sprite (malloc cache)
		mp4_Frame *display_frames; // linked list of display frames
} mp4_VisualObject;

/* Full Info */
typedef struct _mp4_Info {
    int32_t      ftype;          // 0 - raw, 1 - mp4, 2 - avi
    int32_t      ftype_f;        // ftype == 1 (0 - QuickTime(tm)), ftype == 2 (0 - DivX(tm) v. < 5, XVID, 1 - DivX(tm) v. >= 5)
    uint8_t*      buffer;         /* buffer header for saving MPEG-4 stream */
    size_t    buflen;         /* total buffer length */
    size_t    len;            /* valid data in buffer */
    uint8_t*      bufptr;         /* current frame, point to header or data */
    int32_t      bitoff;         /* mostly point to next frame header or PSC */
    int32_t      profile_and_level_indication;
    mp4_VisualObject    VisualObject;
    int32_t      stopOnErr;
    int         strictSyntaxCheck;
    int         noPVOPs;
    int         noBVOPs;
} mp4_Info;

/* bitstream functions */
extern uint8_t*     mp4_FindStartCodePtr(mp4_Info* pInfo);
extern uint8_t*     mp4_FindStartCodeOrShortPtr(mp4_Info* pInfo);
extern int32_t     mp4_SeekStartCodePtr(mp4_Info* pInfo);
extern int32_t     mp4_SeekStartCodeOrShortPtr(mp4_Info* pInfo);
extern int32_t     mp4_SeekStartCodeValue(mp4_Info* pInfo, uint8_t code);
extern uint8_t*     mp4_FindShortVideoStartMarkerPtr(mp4_Info* pInfo);
extern int32_t     mp4_SeekShortVideoStartMarker(mp4_Info* pInfo);
extern int32_t     mp4_SeekGOBMarker(mp4_Info* pInfo);
extern int32_t     mp4_SeekResyncMarker(mp4_Info* pInfo);
extern int32_t     mp4_FindResyncMarker(mp4_Info* pInfo);
extern int        mp4_IsStartCodeOrShort(mp4_Info* pInfo);
extern int        mp4_IsStartCodeValue(mp4_Info* pInfo, int min, int max);
extern int        mp4_IsShortCode(mp4_Info* pInfo);

/* tables */
typedef struct _mp4_VLC1 {
    uint8_t  code;
    uint8_t  len;
} mp4_VLC1;

extern const uint8_t mp4_DefaultIntraQuantMatrix[];
extern const uint8_t mp4_DefaultNonIntraQuantMatrix[];
extern const uint8_t mp4_ClassicalZigzag[];
extern const uint8_t mp4_DCScalerLuma[];
extern const uint8_t mp4_DCScalerChroma[];
extern const uint8_t mp4_cCbCrMvRound16[];
extern const uint8_t mp4_cCbCrMvRound12[];
extern const uint8_t mp4_cCbCrMvRound8[];
extern const uint8_t mp4_cCbCrMvRound4[];
extern const Ipp8s mp4_dquant[];
extern const mp4_VLC1 mp4_cbpy1[];
extern const mp4_VLC1 mp4_cbpy2[];
extern const mp4_VLC1 mp4_cbpy3[];
extern const mp4_VLC1 mp4_cbpy4[];
extern const mp4_VLC1* mp4_cbpy_t[];
extern const uint8_t mp4_cbpy_b[];
extern const int32_t mp4_DC_vlc_Threshold[];
extern const uint8_t mp4_PVOPmb_type[];
extern const uint8_t mp4_PVOPmb_cbpc[];
extern const uint8_t mp4_PVOPmb_bits[];
extern const mp4_VLC1 mp4_BVOPmb_type[];
extern const mp4_VLC1 mp4_MVD_B12_1[];
extern const mp4_VLC1 mp4_MVD_B12_2[];
extern const int32_t mp4_H263_width[];
extern const int32_t mp4_H263_height[];
extern const int32_t mp4_H263_mbgob[];
extern const int32_t mp4_H263_gobvop[];
extern const int32_t mp4_H263_rowgob[];
extern const uint8_t mp4_aux_comp_count[];
extern const uint8_t mp4_aux_comp_is_alpha[];
extern const uint8_t mp4_BABtypeIntra[][3];
extern const int32_t mp4_DivIntraDivisor[];

// project functions
extern void       mp4_Error(const char *str);
//#define mp4_Error(str) puts(str)
extern mp4_Status mp4_InitVOL(mp4_Info *pInfo);
extern mp4_Status mp4_FreeVOL(mp4_Info *pInfo);
extern void       mp4_ResetVOL(mp4_Info *pInfo);
//extern void       mp4_ShowFrame(mp4_Frame *frame);
#define mp4_ShowFrame(frame)
extern mp4_Status mp4_Parse_VisualObjectSequence(mp4_Info* pInfo);
extern mp4_Status mp4_Parse_VisualObject(mp4_Info* pInfo);
extern mp4_Status mp4_Parse_VideoObject(mp4_Info* pInfo);
extern mp4_Status mp4_Parse_GroupOfVideoObjectPlane(mp4_Info* pInfo);
extern mp4_Status mp4_Parse_VideoObjectPlane(mp4_Info* pInfo);
extern mp4_Status mp4_DecodeVideoObjectPlane(mp4_Info* pInfo);

#ifndef USE_INLINE_BITS_FUNC
extern uint32_t mp4_ShowBits(mp4_Info* pInfo, int32_t n);
extern uint32_t mp4_ShowBit(mp4_Info* pInfo);
extern uint32_t mp4_ShowBits9(mp4_Info* pInfo, int32_t n);
extern void   mp4_FlushBits(mp4_Info* pInfo, int32_t n);
extern uint32_t mp4_GetBits(mp4_Info* pInfo, int32_t n);
//extern uint32_t mp4_GetBit(mp4_Info* pInfo);
extern uint32_t mp4_GetBits9(mp4_Info* pInfo, int32_t n);
extern void   mp4_AlignBits(mp4_Info* pInfo);
extern void   mp4_AlignBits7F(mp4_Info* pInfo);
extern uint32_t mp4_ShowBitsAlign(mp4_Info* pInfo, int32_t n);
extern uint32_t mp4_ShowBitsAlign7F(mp4_Info* pInfo, int32_t n);
#else
__INLINE uint32_t mp4_ShowBits(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bufptr;
    uint32_t tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8) | (ptr[3]);
    tmp <<= pInfo->bitoff;
    tmp >>= 32 - n;
    return tmp;
}

__INLINE uint32_t mp4_ShowBit(mp4_Info* pInfo)
{
    uint32_t tmp = pInfo->bufptr[0];
    tmp >>= 7 - pInfo->bitoff;
    return (tmp & 1);
}

__INLINE uint32_t mp4_ShowBits9(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bufptr;
    uint32_t tmp = (ptr[0] <<  8) | ptr[1];
    tmp <<= (pInfo->bitoff + 16);
    tmp >>= 32 - n;
    return tmp;
}

__INLINE void mp4_FlushBits(mp4_Info* pInfo, int32_t n)
{
    n = n + pInfo->bitoff;
    pInfo->bufptr += n >> 3;
    pInfo->bitoff = n & 7;
}

__INLINE uint32_t mp4_GetBits(mp4_Info* pInfo, int32_t n)
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

__INLINE uint32_t mp4_GetBits9(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bufptr;
    uint32_t tmp = (ptr[0] <<  8) | ptr[1];
    tmp <<= (pInfo->bitoff + 16);
    tmp >>= 32 - n;
    n = n + pInfo->bitoff;
    pInfo->bufptr += n >> 3;
    pInfo->bitoff = n & 7;
    return tmp;
}

__INLINE void mp4_AlignBits(mp4_Info* pInfo)
{
    if (pInfo->bitoff > 0) {
        pInfo->bitoff = 0;
        (pInfo->bufptr)++;
    }
}

__INLINE void mp4_AlignBits7F(mp4_Info* pInfo)
{
    if (pInfo->bitoff > 0) {
        pInfo->bitoff = 0;
        (pInfo->bufptr)++;
    } else {
        if (*pInfo->bufptr == 0x7F)
            (pInfo->bufptr)++;
    }
}

__INLINE uint32_t mp4_ShowBitsAlign(mp4_Info* pInfo, int32_t n)
{
    uint8_t* ptr = pInfo->bitoff ? (pInfo->bufptr + 1) : pInfo->bufptr;
    uint32_t tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8) | (ptr[3]);
    tmp >>= 32 - n;
    return tmp;
}

__INLINE uint32_t mp4_ShowBitsAlign7F(mp4_Info* pInfo, int32_t n)
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

#endif // USE_INLINE_BITS_FUNC

__INLINE uint32_t mp4_GetBit(mp4_Info* pInfo)
{
    uint32_t tmp = pInfo->bufptr[0];
    if (pInfo->bitoff != 7) {
        tmp >>= 7 - pInfo->bitoff;
        pInfo->bitoff ++;
    } else {
        pInfo->bitoff = 0;
        pInfo->bufptr ++;
    }
    return (tmp & 1);
}

__INLINE int32_t mp4_GetMarkerBit(mp4_Info* pInfo) {
    if (!mp4_GetBit(pInfo)) {
        mp4_Error("Error: wrong marker bit");
        return 0;
    }
    return 1;
}

// added by benski
extern mp4_Frame *CreateFrame(mp4_VisualObject *object);
extern void ReleaseFrame(mp4_VisualObject *object, mp4_Frame *frame);
extern mp4_Frame *GetDisplayFrame(mp4_VisualObject *object);
extern void DisplayFrame(mp4_VisualObject *object, mp4_Frame *frame);
extern void FreeCache(mp4_VisualObject *object);

#ifdef __cplusplus
}
#endif

