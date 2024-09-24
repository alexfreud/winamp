
/*!
 **************************************************************************
 * \file defines.h
 *
 * \brief
 *    Header file containing some useful global definitions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Detlev Marpe
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *   
 *
 * \date
 *    21. March 2001
 **************************************************************************
 */


#ifndef H264_DEFINES_H_
#define H264_DEFINES_H_
#pragma once

#if defined _DEBUG
# define TRACE           0      //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#else
# define TRACE           0      //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#endif

#define JM                  "16.1 (FRExt)"
#define VERSION             "16.1"
#define EXT_VERSION         "(FRExt)"

#define DUMP_DPB                  0    //!< Dump DPB info for debug purposes
#define PAIR_FIELDS_IN_OUTPUT     0    //!< Pair field pictures for output purposes
#define IMGTYPE                   0    //!< Define imgpel size type. 0 implies byte (cannot handle >8 bit depths) and 1 implies unsigned short
#define ENABLE_FIELD_CTX          1    //!< Enables Field mode related context types for CABAC
#define ENABLE_HIGH444_CTX        1    //!< Enables High 444 profile context types for CABAC. 
#define ZEROSNR                   0    //!< PSNR computation method
#define ENABLE_OUTPUT_TONEMAPPING 1    //!< enable tone map the output if tone mapping SEI present


#include "typedefs.h"


#define H264_MEMORY_ALIGNMENT      32

//#define MAX_NUM_SLICES 150
#define MAX_NUM_SLICES 50
#define MAX_REFERENCE_PICTURES 32               //!< H.264 allows 32 fields
#define MAX_CODED_FRAME_SIZE 8000000         //!< bytes for one frame

//AVC Profile IDC definitions
#define BASELINE         66      //!< YUV 4:2:0/8  "Baseline"
#define MAIN             77      //!< YUV 4:2:0/8  "Main"
#define EXTENDED         88      //!< YUV 4:2:0/8  "Extended"
#define FREXT_HP        100      //!< YUV 4:2:0/8 "High"
#define FREXT_Hi10P     110      //!< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422     122      //!< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444     244      //!< YUV 4:4:4/14 "High 4:4:4"
#define FREXT_CAVLC444   44      //!< YUV 4:4:4/14 "CAVLC 4:4:4"


#define FILE_NAME_SIZE  255
#define INPUT_TEXT_SIZE 1024

#if (ENABLE_HIGH444_CTX == 1)
# define NUM_BLOCK_TYPES 22  
#else
# define NUM_BLOCK_TYPES 10
#endif


//#define _LEAKYBUCKET_

#define BLOCK_SHIFT            2
#define BLOCK_SIZE             4
#define BLOCK_SIZE_8x8         8
#define SMB_BLOCK_SIZE         8
#define BLOCK_PIXELS          16
#define MB_BLOCK_SIZE         16
#define MB_PIXELS            256 // MB_BLOCK_SIZE * MB_BLOCK_SIZE
#define MB_PIXELS_SHIFT        8 // log2(MB_BLOCK_SIZE * MB_BLOCK_SIZE)
#define MB_BLOCK_SHIFT         4
#define BLOCK_MULTIPLE         4 // (MB_BLOCK_SIZE/BLOCK_SIZE)
#define MB_BLOCK_PARTITIONS   16 // (BLOCK_MULTIPLE * BLOCK_MULTIPLE)
#define BLOCK_CONTEXT         64 // (4 * MB_BLOCK_PARTITIONS)

// These variables relate to the subpel accuracy supported by the software (1/4)
#define BLOCK_SIZE_SP      16  // BLOCK_SIZE << 2
#define BLOCK_SIZE_8x8_SP  32  // BLOCK_SIZE8x8 << 2

//  Available MB modes
enum {
  PSKIP        =  0,
  BSKIP_DIRECT =  0,
  P16x16       =  1,
  P16x8        =  2,
  P8x16        =  3,
  SMB8x8       =  4,
  SMB8x4       =  5,
  SMB4x8       =  6,
  SMB4x4       =  7,
  P8x8         =  8,
  I4MB         =  9,
  I16MB        = 10,
  IBLOCK       = 11,
  SI4MB        = 12,
  I8MB         = 13,
  IPCM         = 14,
  MAXMODE      = 15
} ;//MBModeTypes;

// number of intra prediction modes
#define NO_INTRA_PMODE  9

// Direct Mode types
enum {
  DIR_TEMPORAL = 0, //!< Temporal Direct Mode
  DIR_SPATIAL  = 1 //!< Spatial Direct Mode
} ;//DirectModes;

// CAVLC block types
enum {
  LUMA              =  0,
  LUMA_INTRA16x16DC =  1,
  LUMA_INTRA16x16AC =  2,
  CB                =  3,
  CB_INTRA16x16DC   =  4,
  CB_INTRA16x16AC   =  5,
  CR                =  8,
  CR_INTRA16x16DC   =  9,
  CR_INTRA16x16AC   = 10
} ;//CAVLCBlockTypes;

// CABAC block types
enum {
  LUMA_16DC     =   0,
  LUMA_16AC     =   1,
  LUMA_8x8      =   2,
  LUMA_8x4      =   3,
  LUMA_4x8      =   4,
  LUMA_4x4      =   5,
  CHROMA_DC     =   6,
  CHROMA_AC     =   7,
  CHROMA_DC_2x4 =   8,
  CHROMA_DC_4x4 =   9,
  CB_16DC       =  10,
  CB_16AC       =  11,
  CB_8x8        =  12,
  CB_8x4        =  13,
  CB_4x8        =  14,
  CB_4x4        =  15,
  CR_16DC       =  16,
  CR_16AC       =  17,
  CR_8x8        =  18,
  CR_8x4        =  19,
  CR_4x8        =  20,
  CR_4x4        =  21
} ;//CABACBlockTypes;

// Macro defines
#define Q_BITS          15
#define DQ_BITS          6
#define Q_BITS_8        16
#define DQ_BITS_8        6 

//#define IS_INTRA(MB)    ((MB)->mb_type==I4MB  || (MB)->mb_type==I16MB ||(MB)->mb_type==IPCM || (MB)->mb_type==I8MB || (MB)->mb_type==SI4MB)
#define IS_INTRA(MB)    (!!((1 << (MB)->mb_type) & ((1<<I4MB) | (1<<I16MB) | (1<<IPCM) | (1<<I8MB) | (1<<SI4MB))))
#define IS_I16MB(MB)    ((MB)->mb_type==I16MB  || (MB)->mb_type==IPCM)

#define IS_INTER(MB)    (!IS_INTRA(MB))
//#define IS_INTER(MB)    ((MB)->mb_type!=SI4MB && (MB)->mb_type!=I4MB  && (MB)->mb_type!=I16MB && (MB)->mb_type!=I8MB  && (MB)->mb_type!=IPCM)
#define IS_INTERMV(MB)  ((MB)->mb_type!=I4MB  && (MB)->mb_type!=I16MB && (MB)->mb_type!=I8MB  && (MB)->mb_type!=0 && (MB)->mb_type!=IPCM)
#define IS_DIRECT(MB)   ((MB)->mb_type==0     && (currSlice->slice_type == B_SLICE ))
#define IS_SKIP(MB)     ((MB)->mb_type==0     && (currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE))

#define TOTRUN_NUM       15
#define RUNBEFORE_NUM     7
#define RUNBEFORE_NUM_M1  6

// Quantization parameter range
#define MIN_QP          0
#define MAX_QP          51
// 4x4 intra prediction modes 
enum {
  VERT_PRED            = 0,
  HOR_PRED             = 1,
  DC_PRED              = 2,
  DIAG_DOWN_LEFT_PRED  = 3,
  DIAG_DOWN_RIGHT_PRED = 4,
  VERT_RIGHT_PRED      = 5,
  HOR_DOWN_PRED        = 6,
  VERT_LEFT_PRED       = 7,
  HOR_UP_PRED          = 8
} ;//I4x4PredModes;

// 16x16 intra prediction modes
enum {
  VERT_PRED_16   = 0,
  HOR_PRED_16    = 1,
  DC_PRED_16     = 2,
  PLANE_16       = 3
} ;//I16x16PredModes;

// 8x8 chroma intra prediction modes
enum {
  DC_PRED_8     =  0,
  HOR_PRED_8    =  1,
  VERT_PRED_8   =  2,
  PLANE_8       =  3
} ;//I8x8PredModes;

enum {
  EOS = 1,    //!< End Of Sequence
  SOP = 2,    //!< Start Of Picture
  SOS = 3     //!< Start Of Slice
};

// MV Prediction types
enum {
  MVPRED_MEDIAN   = 0,
  MVPRED_L        = 1,
  MVPRED_U        = 2,
  MVPRED_UR       = 3
} ;//MVPredTypes;

enum {
  DECODING_OK     = 0,
  SEARCH_SYNC     = 1,
  PICTURE_DECODED = 2
};

#define  LAMBDA_ACCURACY_BITS         16
#define INVALIDINDEX  (-135792468)

#define RC_MAX_TEMPORAL_LEVELS   5

//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix

#define MAX_PLANE       3
#define IS_INDEPENDENT(IMG)           ((IMG)->separate_colour_plane_flag)
#define IS_FREXT_PROFILE(profile_idc) ( profile_idc>=FREXT_HP || profile_idc == FREXT_CAVLC444 )
#define HI_INTRA_ONLY_PROFILE (((p_Vid->active_sps->profile_idc>=FREXT_Hi10P)&&(p_Vid->active_sps->constrained_set3_flag))||(p_Vid->active_sps->profile_idc==FREXT_CAVLC444)) 

enum
{
	VUI_AR_UNDEFINED = 0,
	VUI_AR_SQUARE = 1, // 1:1
	VUI_AR_12_11 = 2, // 12:11
	VUI_AR_10_11 = 3, // 10:11
	VUI_AR_16_11 = 4, // 16:11
	VUI_AR_40_33 = 5, // 40:33
	VUI_AR_24_11 = 6, // 24:11
	VUI_AR_20_11 = 7, // 20:11
	VUI_AR_32_11 = 8, // 32:11
	VUI_AR_80_33 = 9, // 80:33
	VUI_AR_18_11 = 10, // 18:11
	VUI_AR_15_11 = 11, // 15:11
	VUI_AR_64_33 = 12, // 64:33
VUI_AR_160_99 = 13, // 160:99
VUI_AR_4_3 = 14, // 4:3
VUI_AR_3_2 = 15, // 3:2
VUI_AR_2_1 = 16, // 2:1
	

	VUI_EXTENDED_SAR = 255,
};
#endif

