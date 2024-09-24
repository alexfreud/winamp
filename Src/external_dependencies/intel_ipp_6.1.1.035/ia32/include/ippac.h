/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//    Copyright (c) 2001-2009 Intel Corporation. All Rights Reserved.
//
//          Intel(R) Integrated Performance Primitives
//                     Audio Coding (ippAC)
//
*/

#if !defined(__IPPAC_H__) || defined(_OWN_BLDPCS)
#define __IPPAC_H__

#if defined (_WIN32_WCE) && defined (_M_IX86) && defined (__stdcall)
  #define _IPP_STDCALL_CDECL
  #undef __stdcall
#endif

#include "ippdefs.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
//  Data Structures and Macro
*/

#if !defined(_OWN_BLDPCS)

/*
//  FDP
*/

struct FDPState_32f;
typedef struct FDPState_32f IppsFDPState_32f;

/*
//  FIR Block
*/

struct FIRBlockState_32f;
typedef struct FIRBlockState_32f IppsFIRBlockState_32f;

/*
//  MDCT
*/

struct MDCTFwdSpec_32f;
typedef struct MDCTFwdSpec_32f IppsMDCTFwdSpec_32f;

struct MDCTInvSpec_32f;
typedef struct MDCTInvSpec_32f IppsMDCTInvSpec_32f;

struct MDCTFwdSpec_16s;
typedef struct MDCTFwdSpec_16s IppsMDCTFwdSpec_16s;

/*
//  Filter_SBR
*/

struct IppsFilterContext_SBR_32f;
typedef struct IppsFilterContext_SBR_32f IppsFilterSpec_SBR_C_32fc;
typedef struct IppsFilterContext_SBR_32f IppsFilterSpec_SBR_C_32f;
typedef struct IppsFilterContext_SBR_32f IppsFilterSpec_SBR_R_32f;

struct IppsFilterContext_SBR_32s;
typedef struct IppsFilterContext_SBR_32s IppsFilterSpec_SBR_C_32sc;
typedef struct IppsFilterContext_SBR_32s IppsFilterSpec_SBR_R_32s;

/*
//  Filter_PQMF
*/

struct IppsFilterContext_PQMF_MP3;
typedef struct IppsFilterContext_PQMF_MP3 IppsFilterSpec_PQMF_MP3;

/*
//  Filter_DTS
*/

struct IppsFilterContext_DTS_32f;
typedef struct IppsFilterContext_DTS_32f IppsFilterSpec_DTS_32f;

/*
//  MP3 codec defines
*/

#define IPP_MP3_GRANULE_LEN   576 /* maximum number of frequency
                                     decoded lines */
#define IPP_MP3_SF_BUF_LEN    40  /* scalefactor buffer length */
#define IPP_MP3_V_BUF_LEN     512 /* V data buffers length (32-bit words) */
#define IPP_MP3_ID_MPEG2      0   /* MPEG-2 frame identifier */
#define IPP_MP3_ID_MPEG1      1   /* MPEG-1 frame identifier */

/*
//  Scalefactor band table length for short block
*/

#define IPP_MP3_SFB_TABLE_LONG_LEN    138
typedef const Ipp16s IppMP3ScaleFactorBandTableLong[IPP_MP3_SFB_TABLE_LONG_LEN];

/*
//  Scalefactor band table length for long block
*/

#define IPP_MP3_SFB_TABLE_SHORT_LEN   84
typedef const Ipp16s IppMP3ScaleFactorBandTableShort[IPP_MP3_SFB_TABLE_SHORT_LEN];

/*
//  Mixed block partition table
*/

#define IPP_MP3_MBP_TABLE_LEN   12
typedef const Ipp16s IppMP3MixedBlockPartitionTable[IPP_MP3_MBP_TABLE_LEN];

/* Example mixed block partition table

    For mixed blocks only, this partition table informs
    the Huffman decoder of how many SFBs to count for region0.


IppMP3MixedBlockPartitionTable Modified_Mbp_Table =
{
    // MPEG-2
    // 22.050 kHz
    // Long block SFBs      Short block SFBs
        6,                  2,

    // MPEG-2
    // 24 kHz
    // Long block SFBs      Short block SFBs
        6,                  2,

    // MPEG-2
    // 16 kHz
    // Long block SFBs      Short block SFBs
        6,                  2,

    // MPEG-1
    // 44.1 kHz
    // Long block SFBs      Short block SFBs
        8,                  0,

    // MPEG-1
    // 48 kHz
    // Long block SFBs      Short block SFBs
        8,                  0,

    // MPEG-1
    // 32 kHz
    // Long block SFBs      Short block SFBs
        8,                  0
};

*/


typedef enum {
  IPPAC_DTS_NONPERFECT  = 0,
  IPPAC_DTS_PERFECT = 1
} IppAC_DTS_SYNTHESIS;


typedef enum {
  IPP_CDBK_UNKNOWN  = 0,
  IPP_CDBK_STANDARD = 1
} Ipp_Cdbk_VQ_Hint;


typedef enum {
  IPPAC_PS_CONF0 = 0,
  IPPAC_PS_CONF1 = 1,
  IPPAC_PS_CONF2 = 2,
  IPPAC_PS_CONF3 = 3,
  IPPAC_PS_CONF4 = 4

} IppAC_PS_DEC_ANALYSIS;

typedef struct CdbkState_VQ_32f IppsVQCodeBookState_32f;

/*
//  MPEG-1, MPEG-2 BC header, 32 bits.
//  See ISO/IEC 11172-3, sect 2.4.1.3, 2.4.2.3, 2.4.2.4
*/

typedef struct {
  int id;              /* ID: 1 - MPEG-1, 0 - MPEG-2              */
  int layer;           /* layer index: 0x3 - Layer I
                                       0x2 - Layer II
                                       0x1 - Layer III            */
  int protectionBit;   /* CRC flag 0: CRC on, 1: CRC off          */
  int bitRate;         /* bit rate index                          */
  int samplingFreq;    /* sampling frequency index                */
  int paddingBit;      /* padding flag:
                            0 - no padding,
                            1 - padding                           */
  int privateBit;      /* private_bit, no use                     */
  int mode;            /* mono/stereo select information          */
  int modeExt;         /* extension to mode                       */
  int copyright;       /* copyright or not:
                            0 - no,
                            1 - yes                               */
  int originalCopy;    /* original bitstream or copy:
                            0 - copy,
                            1 - original                          */
  int emphasis;        /* flag indicates the type of de-emphasis
                          that shall be used                      */
  int CRCWord;         /* CRC-check word                          */
} IppMP3FrameHeader;

/*
//  MP3 side information structure, for each granule.
//  Other info main_data_begin, private_bits, scfsi are not included
//  here.
//  Please refer to reference ISO/IEC 11172-3:1993, 2.4.1.7, 2.4.2.7.
//  ISO/IEC 13818-3:1998, 2.4.1.7 ).
*/

typedef struct {
  int  part23Len;       /* the number of bits for scale factors
                           and Huffman data                       */
  int  bigVals;         /* the half number of Huffman data whose
                           maximum amplitudes are greater than 1  */
  int  globGain;        /* the quantizer step size information    */
  int  sfCompress;      /* information to select the number of
                           bits used for the transmission of the
                           scale factors                          */
  int  winSwitch;       /* flag signals that the block uses an
                           other than normal window               */
  int  blockType;       /* flag indicates the window type         */
  int  mixedBlock;      /* flag indicates type of block:
                            0 - non mixed block,
                            1 - mixed block                       */
  int  pTableSelect[3]; /* Huffman table index for the 3 regions
                           in big-values field                    */
  int  pSubBlkGain[3];  /* gain offset from the global gain for
                           one subblock                           */
  int  reg0Cnt;         /* the number of scale factor bands at
                           the boundary of the first region of
                           the big-values field                   */
  int  reg1Cnt;         /* the number of scale factor bands at
                           the boundary of the second region of
                           the big-values field                   */
  int  preFlag;         /* flag of high frequency amplification   */
  int  sfScale;         /* scale to the scale factors             */
  int  cnt1TabSel;      /* Huffman table index for the count1
                           region of quadruples                   */
} IppMP3SideInfo;

/*
//  Global Macro Definitions for AAC codec
*/

#define IPP_AAC_ELT_NUM       16          /* maximum number of elements for
                                             one program */
#define IPP_AAC_CR_ID_LEN     9           /* copyright id length */
#define IPP_AAC_LFE_ELT_NUM   (1<<2)      /* maximum Low Frequency
                                             Enhance elements number for one
                                             program */
#define IPP_AAC_DATA_ELT_NUM  (1<<3)      /* maximum data elements number
                                             for one program */
#define IPP_AAC_COMMENTS_LEN  (1<<8)      /* maximum length of the comment
                                             field in bytes */
#define IPP_AAC_TAG_LEN       4           /* length of AAC data element tag */
#define IPP_AAC_MAX_SFB       51          /* maximum scalefactor band for all
                                             sampling frequencies */
#define IPP_AAC_GROUP_NUM_MAX 8           /* max groups number for one frame */
#define IPP_AAC_SF_MAX        60          /* max number of sfb in one window */
#define IPP_AAC_PRED_SFB_MAX  41          /* maximum prediction scalefactor
                                             bands number for one frame */
#define IPP_AAC_SF_LEN        120         /* scale factor buffer length */
#define IPP_AAC_TNS_FILT_MAX  8           /* maximum filters number for one
                                             frame */
#define ADIF_ID               0x41444946  /* ASCII-"ADIF" */

/*
//  ADIF Header
*/

typedef struct {
  Ipp32u ADIFId;              /* 32-bit, "ADIF" ASCII code        */
  int copyIdPres;             /* copy id flag:
                                  0 - off,
                                  1 - on                          */
  int originalCopy;           /* copyright bit:
                                  0 - no copyright on the coded
                                      bitstream,
                                  1 - copyright protected         */
  int home;                   /* original/home bit:
                                  0 - the bitstream is a copy,
                                  1 - the bitstream is an original*/
  int bitstreamType;          /* bitstream flag:
                                  0 - constant rate bitstream,
                                  1 - variable rate bitstream     */
  int bitRate;                /* bit rate; if 0, unkown bit rate  */
  int numPrgCfgElt;           /* number of program configure
                                 elements                         */
  int pADIFBufFullness[IPP_AAC_ELT_NUM];  /* buffer fullness      */
  Ipp8u pCopyId[IPP_AAC_CR_ID_LEN];       /* copy id              */
} IppAACADIFHeader;

/*
// AAC Program Config Element
*/

typedef struct {
  int eltInsTag;              /* element instance tag             */
  int profile;                /* profile index:
                                  0 - main,
                                  1 - Low Complexity,
                                  2 - Scaleable Sampling Rate,
                                  3 - reserved                    */
  int samplingRateIndex;      /* sampling rate index              */
  int numFrontElt;            /* number of front elements         */
  int numSideElt;             /* number of side elements          */
  int numBackElt;             /* number of back elements          */
  int numLfeElt;              /* number of LFE elements           */
  int numDataElt;             /* number of data elements          */
  int numValidCcElt;          /* number of coupling channel
                                  elements                        */
  int monoMixdownPres;        /* mono mixdown flag:
                                  0 - off,
                                  1 - on                          */
  int monoMixdownEltNum;      /* number of mono mixdown elements  */
  int stereoMixdownPres;      /* stereo mixdown flag:
                                  0 - off,
                                  1 - on                          */
  int stereoMixdownEltNum;    /* number of stereo mixdown
                                  elements                        */
  int matrixMixdownIdxPres;   /* matrix mixdown flag:
                                  0 - off,
                                  1 - on                          */
  int matrixMixdownIdx;       /* identifier of the surround
                                  mixdown coefficient             */
  int pseudoSurroundEnable;   /* pseudo surround:
                                  0 - off,
                                  1 - on                          */
  int pFrontIsCpe[IPP_AAC_ELT_NUM];     /* channel pair flag for
                                           front elements         */
  int pFrontTagSel[IPP_AAC_ELT_NUM];    /* instance tag for front
                                           elements               */
  int pSideIsCpe[IPP_AAC_ELT_NUM];      /* channel pair flag for
                                           side elements          */
  int pSideTagSel[IPP_AAC_ELT_NUM];     /* instance tag for side
                                           elements               */
  int pBackIsCpe[IPP_AAC_ELT_NUM];      /* channel pair flag for
                                           back elements          */
  int pBackTagSel[IPP_AAC_ELT_NUM];     /* instance tag for back
                                           elements               */
  int pLfeTagSel[IPP_AAC_LFE_ELT_NUM];  /* channel pair flag for
                                           LFE elements           */
  int pDataTagSel[IPP_AAC_DATA_ELT_NUM];  /* instance tag for data
                                             elements             */
  int pCceIsIndSw[IPP_AAC_ELT_NUM];     /* independent flag for
                                           coupling               */
                                        /* channel elements       */
  int pCceTagSel[IPP_AAC_ELT_NUM];      /* instance tag for coupling
                                           channel elements       */
  int numComBytes;                      /* number of comment field
                                           bytes                  */
  Ipp8s pComFieldData[IPP_AAC_COMMENTS_LEN]; /* the comment buffer
                                                field             */
} IppAACPrgCfgElt;

/*
//  MPEG-2 AAC ADTS frame header See ISO/IEC 13838-7, Table 6.5
*/

typedef struct {
  /* ADTS fixed header */
  int id;                   /* ID 1                               */
  int layer;                /* layer index:
                                0x3 - Layer I,
                                0x2 - Layer II,
                                0x1 - Layer III                   */
  int protectionBit;        /* CRC flag:
                                0 - CRC on,
                                1 - CRC off                       */
  int profile;              /* profile:
                                0 - MP,
                                1 - LP,
                                2 - SSR                           */
  int samplingRateIndex;    /* sampling frequency index           */
  int privateBit;           /* private_bit, no use                */
  int chConfig;             /* channel configuration              */
  int originalCopy;         /* copyright bit:
                                0 - no copyright on the coded
                                bitstream,
                                1 - copyright protected           */
  int home;                 /* original/home bit:
                                0 - the bitstream is a copy,
                                1 - the bitstream is an original  */
  int emphasis;             /* no in ISO/IEC 13818-7, but used    */
                            /* by ISO/IEC 14490-3                 */
  /* ADTS variable header */
  int cpRightIdBit;         /* copyright id bit                   */
  int cpRightIdStart;       /* copyright id start                 */
  int frameLen;             /* frame length in bytes              */
  int ADTSBufFullness;      /* buffer fullness                    */
  int numRawBlock;          /* n of raw data blocks in the frame  */
  /* ADTS CRC error check, 16bits */
  int CRCWord;              /* CRC-check word                     */
} IppAACADTSFrameHeader;

/*
//  Channel Pair Element Structure
*/

typedef struct {
  int commonWin;          /* common window flag:
                              0 - off,
                              1 - on                        */
  int msMaskPres;         /* MS stereo mask present flag    */
  Ipp8u ppMsMask[IPP_AAC_GROUP_NUM_MAX][IPP_AAC_SF_MAX];
                          /* MS stereo flag buffer for each
                             scalefactor band               */
} IppAACChanPairElt;

/*
//  Individual Channel Side Information Structure.
*/

typedef struct {
  /* unpacked from the bitstream */
  int icsReservedBit;     /* reserved bit                         */
  int winSequence;        /* window sequence flag                 */
  int winShape;           /* window shape flag:
                              0 - sine window,
                              1 - KBD window                      */
  int maxSfb;             /* maximum effective scale factor bands */
  int sfGrouping;         /* scale factor grouping information    */
  int predDataPres;       /* prediction data present flag for one
                             frame:
                              0 - prediction off,
                              1 - prediction on                   */
  int predReset;          /* prediction reset flag:
                              0 - off,
                              1 - on                              */
  int predResetGroupNum;  /* prediction reset group number        */
  Ipp8u pPredUsed[IPP_AAC_PRED_SFB_MAX+3];
                          /* prediction flag buffer for each scale
                             factor band:
                              0 - off,
                              1 - on buffer length 44 bytes, 4-byte
                                  align                           */
  /* decoded from the above info */
  int numWinGrp;          /* window group number                  */
  int pWinGrpLen[IPP_AAC_GROUP_NUM_MAX]; /* buffer for number of
                                            windows in each group */
} IppAACIcsInfo;

/*
//  MPEG 4 Layer
*/

#define IPP_AAC_TNS_COEF_LEN  60        /* TNS coefficients buffer length */
#define IPP_AAC_MAX_LTP_SFB   40        /* maximum length of Long Term
                                           Prediction buffer for long block */
#define IPP_AAC_WIN_MAX       8         /* maximum length of Long Term
                                           Prediction buffer for short block */
#define IPP_AAC_CHAN_NUM      2         /* number of AAC channels */
#define NOISE_HCB             13        /* code of HCB noise in scalefactor code book */
#define IPP_AAC_FRAME_LEN     1024      /* size of data in one AAC frame */
#define IPP_LTP_BLOCK_LEN     (1024*3)  /* size of data in one AAC LTP block */

/*
//  AAC Channel Information Structure
*/

typedef struct {
  int tag;               /* channel tag                           */
  int id;                /* element id                            */
  int samplingRateIndex; /* sampling rate index                   */
  int predSfbMax;        /* maximum prediction scale factor bands */
  int preWinShape;       /* previous block window shape           */
  int winLen;            /* 128: if short window, 1024: others    */
  int numWin;            /* 1 for long block, 8 for short block   */
  int numSwb;            /* decided by sampling frequency and
                            block type                            */
  /* unpacking from the bitstream */
  int globGain;          /* global gain                           */
  int pulseDataPres;     /* pulse data present flag:
                             0 - off,
                             1 - on                               */
  int tnsDataPres;       /* TNS data present flag:
                             0 - off,
                             1 - on                               */
  int gainContrDataPres; /* gain control data present flag:
                             0 - off,
                             1 - on                               */
  /* Ics Info pointer */
  IppAACIcsInfo *pIcsInfo;  /* pointer to IppAACIcsInfo structure */
  /* channel pair element pointer */
  IppAACChanPairElt *pChanPairElt; /* pointer to IppAACChanPairElt
                                      structure                   */
  /* section data */
  Ipp8u pSectCb[IPP_AAC_SF_LEN];   /* section code book buffer    */
  Ipp8u pSectEnd[IPP_AAC_SF_LEN];  /* end of scale factor offset
                                      in each section             */
  int pMaxSect[IPP_AAC_GROUP_NUM_MAX];  /* maximum section number
                                           for each group         */
  /* TNS data */
  int pTnsNumFilt[IPP_AAC_GROUP_NUM_MAX];   /* TNS number filter
                                               number buffer      */
  int pTnsFiltCoefRes[IPP_AAC_GROUP_NUM_MAX]; /* TNS coefficients
                                                 resolution flag  */
  int pTnsRegionLen[IPP_AAC_TNS_FILT_MAX];  /* TNS filter length  */
  int pTnsFiltOrder[IPP_AAC_TNS_FILT_MAX];  /* TNS filter order   */
  int pTnsDirection[IPP_AAC_TNS_FILT_MAX];  /* TNS filter direction
                                               flag               */
} IppAACChanInfo;

/*
//  TNS Information Structure
*/

typedef struct {
  int tnsDataPresent;                     /* if TNS is used       */
  int pTnsNumFilt[IPP_AAC_GROUP_NUM_MAX]; /* Number of TNS filter */
  int pTnsFiltCoefRes[IPP_AAC_GROUP_NUM_MAX]; /* TNS coefficient
                                                  resolution      */
  int pTnsRegionLen[IPP_AAC_TNS_FILT_MAX]; /* TNS filter length   */
  int pTnsFiltOrder[IPP_AAC_TNS_FILT_MAX]; /* TNS filter order    */
  int pTnsDirection[IPP_AAC_TNS_FILT_MAX]; /* TNS filter direction
                                              flag                */
  int pTnsCoefCompress[IPP_AAC_GROUP_NUM_MAX]; /* TNS compress
                                                  coefficients    */
  Ipp8s pTnsFiltCoef[IPP_AAC_TNS_COEF_LEN];    /* TNS filter
                                                  coefficients    */
} IppAACTnsInfo;

/*
//  Long Term Prediction (LTP) Structure
*/

typedef struct{
  int ltpDataPresent;             /* if LTP is used               */
  int ltpLag;                     /* optimal delay from 0 to 2047 */
  Ipp16s ltpCoef;                 /* indicate the LTP coefficient */
  int pLtpLongUsed[IPP_AAC_SF_MAX+1];   /* if long block use ltp  */
  int pLtpShortUsed[IPP_AAC_WIN_MAX];   /* if short block use ltp */
  int pLtpShortLagPresent[IPP_AAC_WIN_MAX]; /* if short lag
                                               is transmitted     */
  int pLtpShortLag[IPP_AAC_WIN_MAX];        /* relative delay
                                               for short window   */
} IppAACLtpInfo;

/*
//  AAC Scalable Main Element Header
*/

typedef struct {
  int windowSequence;  /* the windows is short or long type       */
  int windowShape;     /* what window is used for the right hand,
                          part of this analysis window            */
  int maxSfb;          /* number of scale factor band transmitted */
  int sfGrouping;      /* grouping of short spectral data         */
  int numWinGrp;       /* window group number                     */
  int pWinGrpLen[IPP_AAC_GROUP_NUM_MAX]; /* length of every group */
  int msMode;          /* MS stereo flag:
                           0 - none,
                           1 - different for every sfb,
                           2 - all                                */
  Ipp8u (*ppMsMask)[IPP_AAC_SF_MAX];  /* if MS's used in one sfb,
                                         when msMode == 1         */
  IppAACTnsInfo pTnsInfo[IPP_AAC_CHAN_NUM]; /* TNS structure for
                                               two channels       */
  IppAACLtpInfo pLtpInfo[IPP_AAC_CHAN_NUM]; /* LTP structure for
                                               two channels       */
} IppAACMainHeader;

/*
// AAC Scalable Extension Element Header
*/

typedef struct{
  int msMode;  /* Mode:
                   0 - non,
                   1 - part,
                   2 - all                                        */
  int maxSfb;  /* number of scale factor band for extension layer */
  Ipp8u (*ppMsMask)[IPP_AAC_SF_MAX];        /* if MS is used      */
  IppAACTnsInfo pTnsInfo[IPP_AAC_CHAN_NUM]; /* TNS structure
                                               for Stereo         */
  int pDiffControlLr[IPP_AAC_CHAN_NUM][IPP_AAC_MAX_SFB]; /*
                                       FSS information for stereo */
} IppAACExtHeader;

/*
//  MP3 encoder
*/

#define IPP_MP3_CHANNEL_NUM           2   /* Number MP3 channels */
#define IPP_MP3_PSY_BAND_LONG_NUM    63   /* Number MP3 partition energy for long blocks */
#define IPP_MP3_PSY_BAND_SHORT_NUM   42   /* Number MP3 partition energy for short blocks */
#define FIRST_6_CW                    6   /* Number calculated lines are used for
                                             unpredictability measure */
#define IPP_MP3_BIT_RATE_FREE         0   /* Type value for free bit rate */
#define IPP_MP3_BIT_RATE_32           1   /* Type value for 32 bit rate */
#define IPP_MP3_BIT_RATE_40           2   /* Type value for 40 bit rate */
#define IPP_MP3_BIT_RATE_48           3   /* Type value for 48 bit rate */
#define IPP_MP3_BIT_RATE_56           4   /* Type value for 56 bit rate */
#define IPP_MP3_BIT_RATE_64           5   /* Type value for 64 bit rate */
#define IPP_MP3_BIT_RATE_80           6   /* Type value for 80 bit rate */
#define IPP_MP3_BIT_RATE_96           7   /* Type value for 96 bit rate */
#define IPP_MP3_BIT_RATE_112          8   /* Type value for 112 bit rate */
#define IPP_MP3_BIT_RATE_128          9   /* Type value for 128 bit rate */
#define IPP_MP3_BIT_RATE_160         10   /* Type value for 160 bit rate */
#define IPP_MP3_BIT_RATE_192         11   /* Type value for 192 bit rate */
#define IPP_MP3_BIT_RATE_224         12   /* Type value for 224 bit rate */
#define IPP_MP3_BIT_RATE_256         13   /* Type value for 256 bit rate */
#define IPP_MP3_BIT_RATE_320         14   /* Type value for 320 bit rate */

#define IPP_MP3_SAMPLE_RATE_32000     2   /* Type value for 32000 sample rate */
#define IPP_MP3_SAMPLE_RATE_44100     0   /* Type value for 44100 sample rate */
#define IPP_MP3_SAMPLE_RATE_48000     1   /* Type value for 48000 sample rate */

#define IPP_MP3_NONINTERLEAVED_PCM    1   /* Type value for non interleaved PCM */
#define IPP_MP3_INTERLEAVED_PCM       2   /* Type value for interleaved PCM */

/*
//  MP3 Huffman Table Structure
*/

typedef struct {
  int         tableSize;      /* number of rows in table          */
  int         linbits;        /* variable used for encode if the
                                 magnitude of encoded value is
                                 greater or equal to 15           */
  int         maxBitsValue;   /* maximum bit length of codewords  */
  Ipp16u      *pHcod;         /* pointer to Huffman code table    */
  Ipp8u       *pSlen;         /* pointer to Huffman length table  */
} IppMP3HuffmanTable;

/*
//  MP3 Quant Information Structure
*/

typedef struct {
  int ScaleFactorBits;                /* number of bits used to
                                         encode scale factors     */
  Ipp32s pEnergy[IPP_MP3_SF_BUF_LEN]; /* energy of each scale
                                         factor band              */
  Ipp32s pDistortion[IPP_MP3_SF_BUF_LEN];  /* allowed distortion
                                              of each scale factor
                                              band                */
  Ipp32s pXmin[IPP_MP3_SF_BUF_LEN];   /* allowed distortion of
                                         each scale factor band   */
  Ipp32s pNMR[IPP_MP3_SF_BUF_LEN];    /* Noise-Mask-Ratio array   */
  int quantizeStepSize;               /* quantization step        */
  int pSfbMaxIndex[40];               /* index array of Scalefactor
                                         band maximum values      */
  int minBits;      /* minimum bits decided by the bit reservoir
                       and used bits                              */
  int maxBits;      /* decided by estimatedBits and bit reservoir */
  int count1;       /* number of Huffman symbols whose magnitude
                       is no greater than 1                       */
  int hufSize;      /* number of bits used for Huffman symbols    */
  int pRegionCount[3];  /* pointer to the number of Hufman symbols
                           in each region for the big values part */
} IppMP3QuantInfo;

/*
//  MP3 Bit Reservoir
*/

typedef struct {
  int BitsRemaining; /* bits currently remaining in the reservoir */
  int MaxBits; /* maximum possible reservoir size, in bits,
                  determined as follows:  min(7680-avg_frame_len,
                  2^9*8), where  avg_frame_len is the average frame
                  length, in bits, including padding bits and
                  excluding side information bits                 */
} IppMP3BitReservoir;

/*
//  Psychoacoustic Model Two State
*/

typedef struct {
  Ipp64s pPrevMaskedThresholdLong[2][63];    /* long block masked
             threshold history buffer; Contains masked threshold
             estimates for the threshold calculation partitions
             associated with the two most recent long blocks      */
  Ipp64s pPrevMaskedThresholdShort[IPP_MP3_PSY_BAND_SHORT_NUM]; /*
             short block masked threshold history buffer; Contains
             masked threshold estimates for the threshold
             calculation partitions associated with the two most
             recent long blocks                                   */
  Ipp32sc pPrevFFT[2][FIRST_6_CW];        /* FFT history buffer;
             contains real and imaginary FFT components associated
             with the two most recent long blocks                 */
  Ipp32s pPrevFFTMag[2][FIRST_6_CW];      /* FFT magnitude history
             buffer; contains FFT component magnitudes associated
             with the two most recent long blocks                 */
  int nextPerceptualEntropy;              /* PE estimate for next
             granule; one granule delay provided for synchronization
             with analysis filterbank                             */
  int nextBlockType;                     /* Expected block type for
             next granule; either long (normal), short, or stop.
             Depending upon analysis results for the granule
             following the next, a long block could change to a
             start block, and a stop block could change to a short
             block. This buffer provides one granule of delay for
             synchronization with the analysis filterbank         */
  Ipp32s pNextMSRLong[IPP_MP3_SF_BUF_LEN];        /* long block MSR
             estimates for next granule. One granule delay provided
             for synchronization with analysis filterbank         */
  Ipp32s pNextMSRShort[IPP_MP3_SF_BUF_LEN];       /* short block MSR
             estimates for next granule. One granule delay provided
             for synchronization with analysis filterbank         */
} IppMP3PsychoacousticModelTwoState;

/*
//  MP3 Psychoacoustic Model Two Analysis
*/

typedef struct {
  Ipp32s pMSR[36]; /* MSRs for one granule/channel. For long blocks,
                      elements 0-20 represent the thresholds
                      associated with the 21 SFBs. For short blocks,
                      elements 0,3,6,..,33, elements 1,4,...,34, and
                      elements 2,5,...,35, respectively, represent
                      the thresholds associated with the 12 SFBs for
                      each of the 3 consecutive short blocks in one
                      granule/channel. That is, the block thresholds
                      are interleaved such that the thresholds are
                      grouped by SFB.                             */
  Ipp32s PE;        /* Estimated perceptual entropy, one
                       granule/channel                            */
} IppMP3PsychoacousticModelTwoAnalysis;

#endif /* _OWN_BLDPCS */

/*******************************************************************/

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippacGetLibVersion
//
//  Purpose:
//    Get the library version.
//
//  Parameters:
//
//  Returns:
//    The structure of information about version of ippac library.
//
//  Notes:
//    Not necessary to release the returned structure.
*/
IPPAPI(const IppLibraryVersion*, ippacGetLibVersion, (void))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPReset_32f
//
//  Purpose:
//    Resets predictors for all spectral lines.
//
//  Parameters:
//    pFDPState             Pointer to the predictor specific state structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pFDPState pointer is
//                          NULL.
//    ippStsContextMatchErr Indicates an error when the state structure is
//                          invalid.
*/
IPPAPI(IppStatus, ippsFDPReset_32f, (IppsFDPState_32f *pFDPState))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPResetSfb_32f
//
//  Purpose:
//    Resets predictor-specific information in some scale factor bands.
//
//  Parameters:
//    pSfbOffset            Pointer to the band offset vector.
//    sfbNumber             Number of bands.
//    pResetFlag            Array of flags showing whether predictors for
//                          spectral lines in a certain scale factor band
//                          need to be reset. pFDPState Pointer to the
//                          predictor specific state structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsSizeErr         Indicates an error when sfbNumber is less than
//                          or equal to 0.
//    ippStsContextMatchErr Indicates an error when the state structure is
//                          invalid.
*/
IPPAPI(IppStatus, ippsFDPResetSfb_32f, (const int* pSfbOffset, int sfbNumber, const Ipp8u* pResetFlag, IppsFDPState_32f* pFDPState))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPResetGroup_32f
//
//  Purpose:
//    Resets predictors for group of spectral lines.
//
//  Parameters:
//    resetGroupNumber      Number of the group to be reset.
//    step                  Distance between two neighboring spectral lines
//                          in the group.
//    pFDPState             Pointer to the predictor specific state structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pFDPState pointer is
//                          NULL.
//    ippStsSizeErr         Indicates an error when resetGroupNumber or step
//                          is less than or equal to 0.
//    ippStsContextMatchErr Indicates an error when the state structure is
//                          invalid.
*/
IPPAPI(IppStatus, ippsFDPResetGroup_32f, (int resetGroupNumber, int step, IppsFDPState_32f* pFDPState))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPInitAlloc_32f
//
//  Purpose:
//    Creates and initializes predictor state.
//
//  Parameters:
//    ppFDPState            Pointer to pointer to the FDP state structure to
//                          be created.
//    len                   Number of spectral lines to be processed. Audio
//                          Coding Functions 10 10-29
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the ppFDPState pointer is
//                          NULL.
//    ippStsSizeErr         Indicates an error when the len is less than
//                          or equal to 0.
//    ippStsMemAllocErr     Indicates an error when no memory is allocated.
*/
IPPAPI(IppStatus, ippsFDPInitAlloc_32f, (IppsFDPState_32f** ppFDPState, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPFree_32f
//
//  Purpose:
//    Closes FDP state.
//
//  Parameters:
//    pFDPState             Pointer to the FDP state structure to be closed.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pFDPState pointer is
//                          NULL.
//    ippStsContextMatchErr Indicates an error when the state structure is
//                          invalid.
*/
IPPAPI(IppStatus, ippsFDPFree_32f, (IppsFDPState_32f *pFDPState))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPFwd_32f
//
//  Purpose:
//    Performs frequency domain prediction procedure and calculates
//    prediction error.
//
//  Parameters:
//    pSrc                  Pointer to the input data array.
//    pDst                  Pointer to the data array to be filled with
//                          prediction errors.
//    pFDPState             Pointer to the predictor specific state structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pFDPState pointer is
//                          NULL.
//    ippStsContextMatchErr Indicates an error when the state structure is
//                          invalid.
//    ippStsMisalignedBuf   Indicates misaligned arrays. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsFDPFwd_32f, (const Ipp32f* pSrc, Ipp32f* pDst, IppsFDPState_32f* pFDPState))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPInv_32f_I
//
//  Purpose:
//    Retrieves input signal from prediction error, using frequency domain
//    prediction procedure.
//
//  Parameters:
//    pSrcDst               Pointer to the input and output data array for
//                          the in-place operation.
//    pBandsOffset          Pointer to the band offset vector.
//    predictorBandsNumber  Number of scale factor bands.
//    pPredictionUsed       Pointer to array of flags showing whether
//                          prediction will be used in certain scale factor
//                          band.
//    pFDPState             Pointer to the predictor specific state structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsSizeErr         Indicates an error when predictorBandsNumber is
//                          less than or equal to 0.
//    ippStsContextMatchErr Indicates an error when the state structure is
//                          invalid.
//    ippStsMisalignedBuf   Indicates misaligned arrays. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsFDPInv_32f_I, (Ipp32f* pSrcDst, const int* pBandOffset, int predictorBandsNumber, const Ipp8u* pPredictionUsed, IppsFDPState_32f* pFDPState))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPInit_32f
//
//  Purpose:
//    Initializes predictor state.
//
//  Parameters:
//    ppFDPState            Pointer to pointer to the FDP state structure to
//                          be initialized.
//    len                   Number of spectral lines to be processed.
//    pMemSpec              Pointer to the area for FDP state structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when ppFDPState or pMemSpec
//                          is NULL.
//    ippStsSizeErr         Indicates an error when the len is less than
//                          or equal to 0.
*/
IPPAPI(IppStatus, ippsFDPInit_32f, (IppsFDPState_32f **ppFDPState, int len, Ipp8u *pMemSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFDPGetSize_32f
//
//  Purpose:
//    Gets size of FDP state structure.
//
//  Parameters:
//    len                   Number of spectral lines to be processed.
//    pSizeState            Address of size value in bytes of the frequency
//                          domain predictor state structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSizeState pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when the length is less than
//                          or equal to 0.
*/
IPPAPI(IppStatus, ippsFDPGetSize_32f, (int len, int *pSizeState))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFIRBlockInitAlloc_32f
//
//  Purpose:
//    Initializes FIR block filter state.
//
//  Parameters:
//    pState                Pointer to the FIR block filter state structure
//                          to be created.
//    order                 Number of elements in the array containing the
//                          tap values.
//    len                   Number of input signals.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pointers to data
//                          arrays are null.
//    ippStsMemAllocErr     Indicates an error when no memory is allocated.
//    ippStsFIRLenErr       Indicates an error when order or len is less
//                          than or equal to 0.
*/
IPPAPI(IppStatus, ippsFIRBlockInitAlloc_32f,(IppsFIRBlockState_32f** pState,
int order, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFIRBlockFree_32f
//
//  Purpose:
//    Closes FIR block filter state.
//
//  Parameters:
//    pState                Pointer to the FIR block filter state structure
//                          to be closed.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pointers to data
//                          arrays are null.
//    ippStsContextMatchErr Indicates an error when the state structure is
//                          invalid.
*/
IPPAPI(IppStatus, ippsFIRBlockFree_32f,(IppsFIRBlockState_32f* pState))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsFIRBlockOne_32f
//
//  Purpose:
//    Filters vector of samples through FIR block filter.
//
//  Parameters:
//    pSrc                  Pointer to the input vector of samples to be
//                          filtered.
//    pDst                  Pointer to the vector of filtered output samples.
//    pState                Pointer to the FIR filter state structure.
//    pTaps                 Pointer to the vector of filter taps.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the state structure is
//                          invalid.
//    ippStsMisalignedBuf   Indicates misaligned arrays. Supply aligned data
//                          for better performance.
//    ippStsFIRLenErr       Indicates an error when one of following conditions is true:
//                          1)pState->len is less than or equal to 0;
//                          2)pState->order is less than or equal to 0;
//                          3)pState->queue_end is less than 0;
//                          4)pState->queue_end is greater or equal to pState->order.
*/
IPPAPI(IppStatus, ippsFIRBlockOne_32f,(Ipp32f *pSrc, Ipp32f* pDst, IppsFIRBlockState_32f* pState, Ipp32f *pTaps))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdInitAlloc_32f
//
//  Purpose:
//    Create and initialize modified discrete cosine transform specification
//    structure.
//
//  Parameters:
//    ppMDCTSpec            Pointer to pointer to MDCT specification
//                          structure to be created.
//    len                   Number of samples in MDCT. Since this set of
//                          functions was designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and , where . These values
//                          are the only values that appear in audio coding.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the ppMDCTSpec pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
//    ippStsMemAllocErr     Indicates an error when no memory is allocated.
*/
IPPAPI(IppStatus, ippsMDCTFwdInitAlloc_32f,(IppsMDCTFwdSpec_32f** ppMDCTSpec, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInvInitAlloc_32f
//
//  Purpose:
//    Create and initialize modified discrete cosine transform specification
//    structure.
//
//  Parameters:
//    ppMDCTSpec            Pointer to pointer to MDCT specification
//                          structure to be created.
//    len                   Number of samples in MDCT. Since this set of
//                          functions was designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and , where . These values
//                          are the only values that appear in audio coding.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the ppMDCTSpec pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
//    ippStsMemAllocErr     Indicates an error when no memory is allocated.
*/
IPPAPI(IppStatus, ippsMDCTInvInitAlloc_32f,(IppsMDCTInvSpec_32f** ppMDCTSpec, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdFree_32f
//
//  Purpose:
//    Closes modified discrete cosine transform specification structure.
//
//  Parameters:
//    pMDCTSpec             Pointer to the MDCT specification structure to
//                          be closed.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pMDCTSpec pointer is
//                          NULL.
*/
IPPAPI(IppStatus, ippsMDCTFwdFree_32f,(IppsMDCTFwdSpec_32f* pMDCTSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInvFree_32f
//
//  Purpose:
//    Closes modified discrete cosine transform specification structure.
//
//  Parameters:
//    pMDCTSpec             Pointer to the MDCT specification structure to
//                          be closed.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pMDCTSpec pointer is
//                          NULL.
*/
IPPAPI(IppStatus, ippsMDCTInvFree_32f,(IppsMDCTInvSpec_32f* pMDCTSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdGetBufSize_32f
//
//  Purpose:
//    Gets the size of MDCT work buffer.
//
//  Parameters:
//    pMDCTSpec             Pointer to the MDCT specification structure.
//    pSize                 Address of the MDCT work buffer size value in
//                          bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pMDCTSpec pointer or
//                          pSize value is NULL.
//    ippStsContextMatchErr Indicates an error when the specification
//                          structure pMDCTSpec is invalid.
*/
IPPAPI(IppStatus, ippsMDCTFwdGetBufSize_32f,(const IppsMDCTFwdSpec_32f* pMDCTSpec,
    int* pSize))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInvGetBufSize_32f
//
//  Purpose:
//    Gets the size of MDCT work buffer.
//
//  Parameters:
//    pMDCTSpec             Pointer to the MDCT specification structure.
//    pSize                 Address of the MDCT work buffer size value in
//                          bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pMDCTSpec pointer or
//                          pSize value is NULL.
//    ippStsContextMatchErr Indicates an error when the specification
//                          structure pMDCTSpec is invalid.
*/
IPPAPI(IppStatus, ippsMDCTInvGetBufSize_32f,(const IppsMDCTInvSpec_32f* pMDCTSpec,
    int* pSize))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwd_32f
//
//  Purpose:
//    Computes forward or inverse modified discrete cosine transform (MDCT)
//    of a signal.
//
//  Parameters:
//    pSrc                  Pointer to the input data array.
//    pDst                  Pointer to the output data array.
//    pSrcDst               Pointer to the input and output data array for
//                          the in-place operations.
//    pMDCTSpec             Pointer to the MDCT specification structure.
//    pBuffer               Pointer to the MDCT work buffer.
//    scaleFactor           Scalefactor value.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the specification
//                          structure pMDCTSpec is invalid.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsMDCTFwd_32f,(const Ipp32f* pSrc, Ipp32f* pDst,
    const IppsMDCTFwdSpec_32f* pMDCTSpec, Ipp8u* pBuffer))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInv_32f
//
//  Purpose:
//    Computes forward or inverse modified discrete cosine transform (MDCT)
//    of a signal.
//
//  Parameters:
//    pSrc                  Pointer to the input data array.
//    pDst                  Pointer to the output data array.
//    pMDCTSpec             Pointer to the MDCT specification structure.
//    pBuffer               Pointer to the MDCT work buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the specification
//                          structure pMDCTSpec is invalid.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsMDCTInv_32f,(const Ipp32f* pSrc, Ipp32f* pDst,
    const IppsMDCTInvSpec_32f* pMDCTSpec, Ipp8u* pBuffer))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwd_32f_I
//
//  Purpose:
//    Computes forward or inverse modified discrete cosine transform (MDCT)
//    of a signal.
//
//  Parameters:
//    pSrcDst               Pointer to the input and output data array for
//                          the in-place operations.
//    pMDCTSpec             Pointer to the MDCT specification structure.
//    pBuffer               Pointer to the MDCT work buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the specification
//                          structure pMDCTSpec is invalid.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsMDCTFwd_32f_I,(Ipp32f* pSrcDst, const IppsMDCTFwdSpec_32f* pMDCTSpec, Ipp8u* pBuffer))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInv_32f_I
//
//  Purpose:
//    Computes forward or inverse modified discrete cosine transform (MDCT)
//    of a signal.
//
//  Parameters:
//    pSrcDst               Pointer to the input and output data array for
//                          the in-place operations.
//    pMDCTSpec             Pointer to the MDCT specification structure.
//    pBuffer               Pointer to the MDCT work buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the specification
//                          structure pMDCTSpec is invalid.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsMDCTInv_32f_I,(Ipp32f* pSrcDst, const IppsMDCTInvSpec_32f* pMDCTSpec, Ipp8u* pBuffer))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwd_16s_Sfs
//
//  Purpose:
//    Computes forward or inverse modified discrete cosine transform (MDCT)
//    of a signal.
//
//  Parameters:
//    pSrc                  Pointer to the input data array.
//    pDst                  Pointer to the output data array.
//    pMDCTSpec             Pointer to the MDCT specification structure.
//    scaleFactor           Scalefactor value.
//    pBuffer               Pointer to the MDCT work buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the specification
//                          structure pMDCTSpec is invalid.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsMDCTFwd_16s_Sfs,
       (const Ipp16s *pSrc, Ipp16s *pDst,
        const IppsMDCTFwdSpec_16s* pMDCTSpec,
        int scaleFactor, Ipp8u* pBuffer))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdInitAlloc_16s
//
//  Purpose:
//    Create and initialize modified discrete cosine transform specification
//    structure.
//
//  Parameters:
//    ppMDCTSpec            Pointer to pointer to MDCT specification
//                          structure to be created.
//    len                   Number of samples in MDCT. Since this set of
//                          functions was designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and 2**k, where k>=5. These
//                          values are the only values that appear in audio
//                          coding.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the ppMDCTSpec pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
//    ippStsMemAllocErr     Indicates an error when no memory is allocated.
*/
IPPAPI(IppStatus, ippsMDCTFwdInitAlloc_16s, (IppsMDCTFwdSpec_16s** ppMDCTSpec, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdFree_32f
//
//  Purpose:
//    Closes modified discrete cosine transform specification structure.
//
//  Parameters:
//    pMDCTSpec             Pointer to the MDCT specification structure to
//                          be closed.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pMDCTSpec pointer is
//                          NULL.
*/
IPPAPI(IppStatus, ippsMDCTFwdFree_16s, (IppsMDCTFwdSpec_16s* pMDCTSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdGetSize_16s
//
//  Purpose:
//    Get the sizes of MDCT specification structure, MDCT initialization,
//    and MDCT work buffer.
//
//  Parameters:
//    len                   Number of samples in MDCT. Since this set of
//                          functions is designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and 2**k, where k>=5. These
//                          values are the only values that appear in audio
//                          coding.
//    pSizeSpec             Address of the MDCT specification structure size
//                          value in bytes.
//    pSizeInit             Address of the MDCT initialization buffer size
//                          value in bytes.
//    pSizeBuf              Address of the MDCT work buffer size value in
//                          bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSizeSpec,
//                          pSizeInit, or pSizeBuf pointer is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
*/
IPPAPI(IppStatus, ippsMDCTFwdGetSize_16s, (int len, int* pSizeSpec, int* pSizeInit, int* pSizeBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdGetBufSize_16s
//
//  Purpose:
//    Gets the size of MDCT work buffer.
//
//  Parameters:
//    pMDCTSpec             Pointer to the MDCT specification structure.
//    pSize                 Address of the MDCT work buffer size value in
//                          bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pMDCTSpec pointer or
//                          pSize value is NULL.
//    ippStsContextMatchErr Indicates an error when the specification
//                          structure pMDCTSpec is invalid.
*/
IPPAPI(IppStatus, ippsMDCTFwdGetBufSize_16s,
       (const IppsMDCTFwdSpec_16s *pMDCTSpec, int *pSize))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInvGetSize_32f
//
//  Purpose:
//    Get the sizes of MDCT specification structure, MDCT initialization,
//    and MDCT work buffer.
//
//  Parameters:
//    len                   Number of samples in MDCT. Since this set of
//                          functions is designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and 2**k, where k>=5. These
//                          values are the only values that appear in audio
//                          coding.
//    pSizeSpec             Address of the MDCT specification structure size
//                          value in bytes.
//    pSizeInit             Address of the MDCT initialization buffer size
//                          value in bytes.
//    pSizeBuf              Address of the MDCT work buffer size value in
//                          bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSizeSpec,
//                          pSizeInit, or pSizeBuf pointer is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
*/
IPPAPI(IppStatus, ippsMDCTInvGetSize_32f, (int len, int *pSizeSpec, int *pSizeInit, int *pSizeBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdGetSize_32f
//
//  Purpose:
//    Get the sizes of MDCT specification structure, MDCT initialization,
//    and MDCT work buffer.
//
//  Parameters:
//    len                   Number of samples in MDCT. Since this set of
//                          functions is designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and 2**k, where k>=5. These
//                          values are the only values that appear in audio
//                          coding.
//    pSizeSpec             Address of the MDCT specification structure size
//                          value in bytes.
//    pSizeInit             Address of the MDCT initialization buffer size
//                          value in bytes.
//    pSizeBuf              Address of the MDCT work buffer size value in
//                          bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSizeSpec,
//                          pSizeInit, or pSizeBuf pointer is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
*/
IPPAPI(IppStatus, ippsMDCTFwdGetSize_32f, (int len, int *pSizeSpec, int *pSizeInit, int *pSizeBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdInit_32f
//
//  Purpose:
//    Initialize modified discrete cosine transform specification structure.
//
//  Parameters:
//    ppMDCTSpec            Pointer to pointer to MDCT specification
//                          structure to be created.
//    len                   Number of samples in MDCT. Since this set of
//                          functions was designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and 2**k, where k>=5. These
//                          values are the only values that appear in audio
//                          coding.
//    pMemSpec              Pointer to the area for MDCT specification
//                          structure.
//    pMemInit              Pointer to the buffer that is used for
//                          initialization.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the ppMDCTSpec, pMemSpec
//                          or pMemInit pointer is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
*/
IPPAPI(IppStatus, ippsMDCTFwdInit_32f, (IppsMDCTFwdSpec_32f** ppMDCTSpec, int len, Ipp8u* pMemSpec, Ipp8u* pMemInit))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInvInit_32f
//
//  Purpose:
//    Initialize modified discrete cosine transform specification structure.
//
//  Parameters:
//    ppMDCTSpec            Pointer to pointer to MDCT specification
//                          structure to be created.
//    len                   Number of samples in MDCT. Since this set of
//                          functions was designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and 2**k, where k>=5. These
//                          values are the only values that appear in audio
//                          coding.
//    pMemSpec              Pointer to the area for MDCT specification
//                          structure.
//    pMemInit              Pointer to the buffer that is used for
//                          initialization.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the ppMDCTSpec, pMemSpec
//                          or pMemInit pointer is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
*/
IPPAPI(IppStatus, ippsMDCTInvInit_32f,(IppsMDCTInvSpec_32f** ppMDCTSpec, int len, Ipp8u* pMemSpec, Ipp8u* pMemInit))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwdInit_16s
//
//  Purpose:
//    Initialize modified discrete cosine transform specification structure.
//
//  Parameters:
//    ppMDCTSpec            Pointer to pointer to MDCT specification
//                          structure to be created.
//    len                   Number of samples in MDCT. Since this set of
//                          functions was designed specially for audio
//                          coding, only the following values of length are
//                          supported: 12, 36, and 2**k, where k>=5. These
//                          values are the only values that appear in audio
//                          coding.
//    pMemSpec              Pointer to the area for MDCT specification
//                          structure.
//    pMemInit              Pointer to the buffer that is used for
//                          initialization.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the ppMDCTSpec, pMemSpec
//                          or pMemInit pointer is NULL.
//    ippStsSizeErr         Indicates an error when len does not belong to
//                          the above set of admissible values.
*/
IPPAPI(IppStatus, ippsMDCTFwdInit_16s,(IppsMDCTFwdSpec_16s** ppMDCTSpec, int len, Ipp8u* pMemSpec, Ipp8u* pMemInit))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSpread_16s_Sfs
//
//  Purpose:
//    Computes spreading function.
//
//  Parameters:
//    Src1                  Input data 1.
//    Src2                  Input data 2.
//    inScaleFactor         Input value scalefactor value.
//    pDst                  Pointer to the output data vector, output data
//                          is in Q15 format.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pDst pointer is NULL.
*/
IPPAPI(IppStatus, ippsSpread_16s_Sfs,
      (Ipp16s src1, Ipp16s src2, int inScaleFactor, Ipp16s* pDst))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsInterleave_16s
//
//  Purpose:
//    Converts signal from non-interleaved to interleaved format.
//
//  Parameters:
//    pSrc                  Array of pointers to the vectors [len]
//                          containing samples for particular channels.
//    ch_num                Number of channels.
//    len                   Number of samples in each channel.
//    pDst                  Pointer to the destination vector [ch_num * len]
//                          in interleaved format.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when len is less than or
//                          equal to 0.
//    ippStsNumChannelsErr  Indicates an error when ch_num is less than or
//                          equal to 0.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsInterleave_16s, (const Ipp16s   **pSrc,
                                             int      ch_num,
                                             int      len,
                                             Ipp16s   *pDst))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsInterleave_32f
//
//  Purpose:
//    Converts signal from non-interleaved to interleaved format.
//
//  Parameters:
//    pSrc                  Array of pointers to the vectors [len]
//                          containing samples for particular channels.
//    ch_num                Number of channels.
//    len                   Number of samples in each channel.
//    pDst                  Pointer to the destination vector [ch_num * len]
//                          in interleaved format.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when len is less than or
//                          equal to 0.
//    ippStsNumChannelsErr  Indicates an error when ch_num is less than or
//                          equal to 0.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsInterleave_32f, (const Ipp32f **pSrc,
                                             int    ch_num,
                                             int    len,
                                             Ipp32f *pDst))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDeinterleave_16s
//
//  Purpose:
//    Converts signal from interleaved to non-interleaved format.
//
//  Parameters:
//    pSrc                  Pointer to vector [ch_num * len] of interleaved
//                          samples.
//    ch_num                Number of channels.
//    len                   Number of samples in each channel.
//    pDst                  Array of pointers to the vectors [len] to be
//                          filled with samples of particular channels.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when len or ch_num is less
//                          than or equal to 0.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsDeinterleave_16s, (const Ipp16s* pSrc, int ch_num,
                                              int len, Ipp16s** pDst))
/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDeinterleave_32f
//
//  Purpose:
//    Converts signal from interleaved to non-interleaved format.
//
//  Parameters:
//    pSrc                  Pointer to vector [ch_num * len] of interleaved
//                          samples.
//    ch_num                Number of channels.
//    len                   Number of samples in each channel.
//    pDst                  Array of pointers to the vectors [len] to be
//                          filled with samples of particular channels.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when len or ch_num is less
//                          than or equal to 0.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
*/
IPPAPI(IppStatus, ippsDeinterleave_32f, (const Ipp32f* pSrc, int ch_num,
                                              int len, Ipp32f** pDst))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsPow34_32f16s
//
//  Purpose:
//    Raises a vector to the power of 3/4.
//
//  Parameters:
//    pSrc                  Pointer to the input data vector [len].
//    pDst                  Pointer to the output data vector [len].
//    len                   Number of elements in the input and output
//                          vectors.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
//    ippStsSizeErr         Indicates an error when len is less than or
//                          equal to 0.
*/
IPPAPI(IppStatus, ippsPow34_32f16s, (const Ipp32f* pSrc, Ipp16s* pDst, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsPow43_16s32f
//
//  Purpose:
//    Raises a vector to the power of 4/3.
//
//  Parameters:
//    pSrc                  Pointer to the input data vector [len].
//    pDst                  Pointer to the output data vector [len].
//    len                   Number of elements in the input and output
//                          vectors.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data for
//                          better performance.
//    ippStsSizeErr         Indicates an error when len is less than or
//                          equal to 0.
*/
IPPAPI(IppStatus, ippsPow43_16s32f, (const Ipp16s* pSrc, Ipp32f* pDst, int len))

IPPAPI(IppStatus, ippsPow43Scale_16s32s_Sf, (const Ipp16s* pSrc, Ipp32s* pDst,
       const Ipp16s *pScaleFactor, const Ipp32s *pBandOffset,
       int offset, int bandsNumber, int groupLen, int scalef))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsPow34_32f
//
//  Purpose:
//    Raises a vector to the power of 3/4.
//
//  Parameters:
//    pSrc                  Pointer to the input data vector [len].
//    pDst                  Pointer to the output data vector [len].
//    len                   Number of elements in the input and output
//                          vectors.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
//    ippStsSizeErr         Indicates an error when len is less than or
//                          equal to 0.
*/
IPPAPI(IppStatus, ippsPow34_32f,(const Ipp32f* pSrc, Ipp32f* pDst, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsPow34_16s_Sfs
//
//  Purpose:
//    Raises a vector to the power of 3/4.
//
//  Parameters:
//    pSrc                  Pointer to the input data vector [len].
//    inscaleFactor         Input data scalefactor value.
//    pDst                  Pointer to the output data vector [len].
//    scaleFactor           Scalefactor value.
//    len                   Number of elements in the input and output
//                          vectors.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsMisalignedBuf   Indicates misaligned data. Supply aligned data
//                          for better performance.
//    ippStsSizeErr         Indicates an error when len is less than or
//                          equal to 0.
*/
IPPAPI(IppStatus, ippsPow34_16s_Sfs,(const Ipp16s* pSrc, int inScaleFactor, Ipp16s* pDst, int ScaleFactor, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsCalcSF_16s32f
//
//  Purpose:
//    Restores actual scale factors from the bit stream values.
//
//  Parameters:
//    pSrc                  Pointer to the input data array.
//    offset                Scale factors offset.
//    pDst                  Pointer to the output data array.
//    len                   Number of elements in the vector.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrc or pDst pointer
//                          is NULL.
//    ippStsSizeErr         Indicates an error when len is less than or
//                          equal to 0.
*/
IPPAPI(IppStatus,ippsCalcSF_16s32f, (const Ipp16s* pSrc, int offset,Ipp32f *pDst, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsScale_32f_I
//
//  Purpose:
//    Applies scale factors to spectral bands in accordance with spectral
//    bands boundaries.
//
//  Parameters:
//    pSrcDst               Pointer to the input and output data array. The
//                          size of array must be not less than pBandOffset[
//                          bands_number].
//    pSF                   Pointer to the data array containing scale
//                          factors. The size of the array must be not less
//                          than bands_number.
//    pBandOffset           Pointer to the vector of band offsets. The size
//                          of array must be not less than bands_number + 1.
//    bandsNumber           Number of bands to which scale factors are
//                          applied.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pSrcDst or
//                          pBandsOffset pointer is NULL.
//    ippStsSizeErr         Indicates an error when bandsNumber is less than
//                          or equal to 0.
*/
IPPAPI(IppStatus, ippsScale_32f_I, (Ipp32f* pSrcDst, Ipp32f* pSF, const Ipp32s* pBandOffset, int bandsNumber))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMakeFloat_16s32f
//
//  Purpose:
//    Converts mantissa and exponent arrays to float arrays.
//
//  Parameters:
//    inmant                Array of mantissas.
//    inexp                 Array of exponents.
//    size                  Number of array elements.
//    outfloat              Array of resulting float arrays.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the inmant, inexp or
//                          outfloat pointer is NULL.
//    ippStsSizeErr         Indicates an error when size is less than or
//                          equal to 0.
*/
IPPAPI(IppStatus, ippsMakeFloat_16s32f, (Ipp16s* inmant, Ipp16s* inexp, Ipp32s size, Ipp32f* outfloat))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVQCodeBookInitAlloc_32f
//
//  Purpose:
//    Creates and initializes the codebook structure.
//
//  Parameters:
//    pInputTable           Pointer to the codebook table of the size step *
//                          height containing height quantization vectors of
//                          the length step.
//    ppCodeBook            Pointer to pointer to the codebook structure to
//                          be created.
//    step                  Step to the next line in the table InputTable,
//                          quantization vector length.
//    height                Table height, number of quantization vectors.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when ppCodeBook or
//                          pInputTable pointer is NULL.
//    ippStsSizeErr         Indicates an error when the step or height is
//                          less than or equal to 0.
//    ippStsMemAllocErr     Indicates an error when no memory is allocated.
*/
IPPAPI(IppStatus, ippsVQCodeBookInitAlloc_32f, (const Ipp32f* pInputTable, IppsVQCodeBookState_32f** ppCodeBooks, int step, int height))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVQCodeBookFree_32f
//
//  Purpose:
//    Closes the IppsVQCodeBookState_32f structure created by the function
//    ippsVQCodeBookInitAlloc.
//
//  Parameters:
//    pCodeBook             Pointer to specified IppsVQCodeBookState_32f
//                          structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pCodeBook pointer is
//                          NULL.
*/
IPPAPI(IppStatus, ippsVQCodeBookFree_32f, (IppsVQCodeBookState_32f* pCodeBook))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVQCodeBookInit_32f
//
//  Purpose:
//    Initializes the codebook structure.
//
//  Parameters:
//    pInputTable           Pointer to the codebook table of the size step *
//                          height containing height quantization vectors of
//                          the length step.
//    ppCodeBook            Pointer to pointer to the codebook structure to
//                          be created.
//    step                  Step to the next line in the table InputTable,
//                          quantization vector length.
//    height                Table height, number of quantization vectors.
//    pMemSpec              Pointer to the area for the codebook structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pInputTable, ppCodeBook
//                          or pMemSpec pointer is NULL.
//    ippStsSizeErr         Indicates an error when the step or height is
//                          less than or equal to 0.
*/
IPPAPI(IppStatus, ippsVQCodeBookInit_32f, (const Ipp32f* pInputTable, IppsVQCodeBookState_32f** ppCodeBook , int step, int height, Ipp8u* pMemSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVQPreliminarySelect_32f
//
//  Purpose:
//    Selects candidates for the nearest code vector of codebooks.
//
//  Parameters:
//    pSrc                  Source vector to be quantized.
//    pWeights              Pointer to the vector of weights.
//    nDiv                  Number of fragmentations of the src and weights
//                          vectors.
//    pLengths              Pointer to an array of lengths of fragmentations.
//    pIndx                 Pointer to the output vector of indexes of the
//                          nCand minimum candidates.
//    pSign                 Pointer to the output vector of signs of nCand
//                          minimum candidates: - The value of 1 indicates
//                          that the minimal distortion appears when the
//                          norm is negative; - The value of 0 indicates
//                          that the minimal distortion appears when the
//                          norm is positive.
//    nCand                 Number of output candidates.
//    pPolbits              Pointer to the polbits flag vector.
//    pCodeBook             Pointer to the specified IppsVQCodeBookState_32f
//                          structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pCodeBook or pSrc
//                          pointer is NULL.
*/
IPPAPI(IppStatus, ippsVQPreliminarySelect_32f, (const Ipp32f* pSrc, const Ipp32f* pWeights, int nDiv, const Ipp32s* pLengths, Ipp32s* pIndx, Ipp32s* pSign, int nCand, int* polbits, IppsVQCodeBookState_32f* pCodeBook))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVQMainSelect_32f
//
//  Purpose:
//    Finds optimal indexes with minimal distortion.
//
//  Parameters:
//    pSrc                  Source vector to be quantized.
//    pWeights              Pointer to the vector of weights.
//    nDiv                  Number of fragmentations of the src and weights
//                          vectors.
//    pLengths              Pointer to an array of lengths of fragmentations.
//    nCand                 Number of input candidates.
//    ppIndexCand           Pointer to pointer to the input vectors of
//                          indexes of nCand minimum candidates.
//    ppSignCand            Pointer to pointer to the input vectors of signs
//                          of nCand minimum candidates.
//    ppIndx                Pointer to pointer to the output vectors of
//                          indexes.
//    ppSign                Pointer to pointer to the output vectors of signs.
//    ppCodeBooks           Pointer to pointer to the specified
//                          IppsVQCodeBookState_32f structures.
//    nCodeBooks            Number of codebooks.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
*/
IPPAPI(IppStatus, ippsVQMainSelect_32f, (const Ipp32f* pSrc, const Ipp32f* pWeights, const Ipp32s* pLengths, int nDiv, int nCand, Ipp32s** ppIndexCand, Ipp32s** ppSignCand, Ipp32s** ppIndx, Ipp32s** ppSign, IppsVQCodeBookState_32f** ppCodeBooks, int nCodeBooks))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVQIndexSelect_32f
//
//  Purpose:
//    Finds optimal vector set for specified number of codebooks.
//
//  Parameters:
//    pSrc                  Source vector to be quantized.
//    pWeights              Pointer to the vector of weights.
//    nDiv                  Number of fragmentations of the src and weights
//                          vectors.
//    pLengths              Pointer to an array of lengths of fragmentations.
//    nCand                 Number of input candidates.
//    ppPolbits             Indicates whether one or two norms should be
//                          used to compute the optimal vector set for the
//                          specified number of codebooks.
//    ppIndx                Pointer to pointer to the output vectors of
//                          indexes.
//    ppSign                Pointer to pointer to the output vectors of signs.
//                          - The value of 1 indicates that the minimal
//                          distortion appears when the norm is negative.
//                          - The value of 0 indicates that the minimal
//                          distortion appears when the norm is positive.
//    ppCodeBooks           Pointer to pointer to the specified
//                          IppsVQCodeBookState_32f structures.
//    nCodeBooks            Number of codebooks.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
*/
IPPAPI(IppStatus, ippsVQIndexSelect_32f, (const Ipp32f* pSrc, const Ipp32f* pWeights, int nDiv, const Ipp32s* pLengths,  int nCand, int** ppPolbits, Ipp32s** ppIndx, Ipp32s** ppSign, IppsVQCodeBookState_32f** ppCodeBooks, int nCodeBooks))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVQReconstruction_32f
//
//  Purpose:
//    Reconstructs vectors from indexes.
//
//  Parameters:
//    ppIndx                Pointer to pointer to an array of input vectors
//                          of indexes for each codebook.
//    ppSign                Pointer to pointer to an array of input vectors
//                          of signs for each codebook containing either 1
//                          or -1.
//    pLengths              Pointer to an array of lengths of partitions of
//                          output vector.
//    pDst                  Pointer to the reconstructed vector of spectrum
//                          values.
//    nDiv                  Number of partitions of output vector.
//    ppCodeBooks           Pointer to pointer to an array of pointers to
//                          specified IppsVQCodeBookState_32f structures.
//    nCodeBooks            Number of codebooks.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
*/
IPPAPI(IppStatus, ippsVQReconstruction_32f, (const Ipp32s** ppIndx, const Ipp32s** ppSign, const Ipp32s* pLength, int nDiv, int nStream, Ipp32f* pDst, IppsVQCodeBookState_32f** ppCodeBooks))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVQCodeBookGetSize_32f
//
//  Purpose:
//    Gets the size of the codebook structure.
//
//  Parameters:
//    step                  Step to the next line in the codebook table,
//                          quantization vector length.
//    height                Table height, number of quantization vectors.
//    pSizeState            Address of the codebook structure size value in
//                          bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSizeState pointer is
//                          NULL.
//    ippStsSizeErr         Indicates an error when the step or height is
//                          less than or equal to 0.
*/
IPPAPI(IppStatus, ippsVQCodeBookGetSize_32f, (int step, int height, int *pSizeState))

/****************************************************************************
 *                   MPEG4-SBR AUDIO
 ****************************************************************************/

 /*
 // DECODER PART
 */
/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterGetSize_SBR_RToC_32f,
//    ippsAnalysisFilterGetSize_SBR_RToC_32f32fc,
//    ippsAnalysisFilterGetSize_SBR_RToR_32f,
//    ippsSynthesisFilterGetSize_SBR_CToR_32f,
//    ippsSynthesisFilterGetSize_SBR_CToR_32fc32f,
//    ippsSynthesisFilterGetSize_SBR_RToR_32f
//    ippsSynthesisDownFilterGetSize_SBR_CToR_32f,
//    ippsSynthesisDownFilterGetSize_SBR_CToR_32fc32f,
//    ippsSynthesisDownFilterGetSize_SBR_RToR_32f
//
//  Purpose:
//    Returns size of QMF specification structures, init and work buffers.
//
//  Parameters:
//    pSizeSpec             Address of the FilterSpec_SBR specification
//                          structure size value in bytes.
//    pSizeInitBuf          Address size of the buffer for initialization
//                          functions in bytes.
//    pSizeWorkBuf          Address of the work buffer size value in bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
*/
IPPAPI( IppStatus, ippsAnalysisFilterGetSize_SBR_RToC_32f, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))
IPPAPI( IppStatus, ippsAnalysisFilterGetSize_SBR_RToC_32f32fc, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))
IPPAPI( IppStatus, ippsAnalysisFilterGetSize_SBR_RToR_32f, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))
IPPAPI( IppStatus, ippsSynthesisFilterGetSize_SBR_CToR_32f, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))
IPPAPI(IppStatus, ippsSynthesisFilterGetSize_SBR_CToR_32fc32f, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))
IPPAPI( IppStatus, ippsSynthesisFilterGetSize_SBR_RToR_32f, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))
IPPAPI( IppStatus, ippsSynthesisDownFilterGetSize_SBR_CToR_32f, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))
IPPAPI( IppStatus, ippsSynthesisDownFilterGetSize_SBR_CToR_32fc32f, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))
IPPAPI( IppStatus, ippsSynthesisDownFilterGetSize_SBR_RToR_32f, ( int* pSizeSpec, int* pSizeInitBuf, int* pSizeWorkBuf ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterInit_SBR_RToC_32f
//
//  Purpose:
//    Initializes analysis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsAnalysisFilterInit_SBR_RToC_32f,
       ( IppsFilterSpec_SBR_C_32f** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterInit_SBR_CToR_32fc32f
//
//  Purpose:
//    Initializes analysis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsSynthesisFilterInit_SBR_CToR_32fc32f,
       ( IppsFilterSpec_SBR_C_32fc** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterInit_SBR_RToC_32f32fc
//
//  Purpose:
//    Initializes analysis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsAnalysisFilterInit_SBR_RToC_32f32fc,
       ( IppsFilterSpec_SBR_C_32fc** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterInit_SBR_RToR_32f
//
//  Purpose:
//    Initializes analysis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsAnalysisFilterInit_SBR_RToR_32f,
       ( IppsFilterSpec_SBR_R_32f** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterInit_SBR_CToR_32fc32f
//
//  Purpose:
//    Initializes synthesis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsSynthesisFilterInit_SBR_CToR_32f,
       ( IppsFilterSpec_SBR_C_32f** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterInit_SBR_RToR_32f
//
//  Purpose:
//    Initializes synthesis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsSynthesisFilterInit_SBR_RToR_32f,
       ( IppsFilterSpec_SBR_R_32f** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisDownFilterInit_SBR_CToR_32f
//
//  Purpose:
//    Initializes synthesis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsSynthesisDownFilterInit_SBR_CToR_32f,
       ( IppsFilterSpec_SBR_C_32f** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisDownFilterInit_SBR_CToR_32fc32f
//
//  Purpose:
//    Initializes down sample synthesis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsSynthesisDownFilterInit_SBR_CToR_32fc32f,
       ( IppsFilterSpec_SBR_C_32fc** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisDownFilterInit_SBR_RToR_32f
//
//  Purpose:
//    Initializes down sample synthesis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the FilterSpec_SBR
//                          specification structure is written. Onlt the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the FilterSpec_SBR
//                          specification structure.
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsSynthesisDownFilterInit_SBR_RToR_32f,
       ( IppsFilterSpec_SBR_R_32f** ppFilterSpec, Ipp8u* pMemSpec, Ipp8u* pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilter_SBR_RToC_32f_D2L
//    ippsAnalysisFilter_SBR_RToC_32f32fc_D2L
//    ippsAnalysisFilter_SBR_RToR_32f_D2L
//
//  Purpose:
//    Splits time domain signal output from the core decoder into 32
//    subband signals.
//
//  Parameters:
//    pSrc                  Pointer to the input audio signal.
//    pDst                  array of pointers, holds subband samples.
//    pDstRe                array of pointers  that contains real parts of subband samples.
//    pDstIm                array of pointers that contains imaginary parts of subband
//                          samples.
//    pSbrTableWindowDown   Pointer to the window table that is used by decoder SBR Analysis Filter.
//    offset                Desired displacement in number of rows when the matrix pDst is calculated. offset >=0
//    NumLoop               Parameter equal to 32 if frame size of the core decoded signal is 1024
//                          30 if frame size of the core decoded signal is 960.
//    kx                    first SBR subband in the SBR range.
//    pFilterSpec           Pointer to the FilterSpec_SBR specification structure.
//    pWorkBuf              Pointer to the work buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsAnalysisFilter_SBR_RToC_32f_D2L,
       (const Ipp32f* pSrc, Ipp32f* pDstRe[], Ipp32f* pDstIm[], const Ipp32f* pSbrTableWinDown,
       int NumLoop, int offset, int kx,
       const IppsFilterSpec_SBR_C_32f* pFilterSpec, Ipp8u* pWorkBuf))
IPPAPI( IppStatus, ippsAnalysisFilter_SBR_RToC_32f32fc_D2L,
       (const Ipp32f* pSrc, Ipp32fc* pDst[], const Ipp32f* pSbrTableWinDown,
       int NumLoop, int offset, int kx,
       const IppsFilterSpec_SBR_C_32fc* pFilterSpec, Ipp8u* pWorkBuf))
IPPAPI( IppStatus, ippsAnalysisFilter_SBR_RToR_32f_D2L,
       (const Ipp32f* pSrc, Ipp32f* pDst[], const Ipp32f* pSbrTableWinDown,
       int NumLoop, int offset, int kx,
       const IppsFilterSpec_SBR_R_32f* pFilterSpec, Ipp8u* pWorkBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilter_SBR_CToR_32f_D2L
//    ippsSynthesisFilter_SBR_CToR_32fc32f_D2L
//    ippsSynthesisFilter_SBR_RToR_32f_D2L
//
//  Purpose:
//    Transforms SBR-processed subband signals into time domain samples.
//
//  Parameters:
//      pSrc       - array of pointers, holds SBR-processed subband signals.
//      pSrcRe     - array of pointers that contains real parts of SBR-processed subband signals.
//      pSrcIm     - array of pointers that contains imaginary parts of SBR-processed subband signals.
//      pDst       - Pointer to the output vector, holds time domain output samples.
//      pSbrTableWindow - Pointer to the window table that is used by SBR Analysis Filter.
//      NumLoop    - Parameter equal to 32 if frame size of the core decoded signal is 1024
//      30 if frame size of the core decoded signal is 960.
//      pFilterSpec- Pointer to the FilterSpec_SBR specification structure.
//      pWorkBuf   - Pointer to the work buffer used by the filter.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI( IppStatus, ippsSynthesisFilter_SBR_CToR_32f_D2L,
       (const Ipp32f* pSrcRe[], const Ipp32f* pSrcIm[], Ipp32f* pDst, const Ipp32f* pSbrTableWindow,
       int NumLoop, const IppsFilterSpec_SBR_C_32f* pFilterSpec, Ipp8u* pWorkBuf))
IPPAPI( IppStatus, ippsSynthesisFilter_SBR_CToR_32fc32f_D2L,
       (const Ipp32fc* pSrc[], Ipp32f* pDst, const Ipp32f* pSbrTableWindow, int NumLoop,
       const IppsFilterSpec_SBR_C_32fc* pFilterSpec, Ipp8u* pWorkBuf ))
IPPAPI( IppStatus, ippsSynthesisFilter_SBR_RToR_32f_D2L,
       (const Ipp32f* pSrc[], Ipp32f* pDst, const Ipp32f* pSbrTableWindow, int NumLoop,
       const IppsFilterSpec_SBR_R_32f* pFilterSpec, Ipp8u* pWorkBuf ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:       ippsSynthesisDownFilter_SBR_CToR_32f_D2L, ippsSynthesisDownFilter_SBR_CToR_32fc32f_D2L, ippsSynthesisDownFilter_SBR_RToR_32f_D2L
//  Purpose:    Transforms SBR-processed subband signals into time domain samples and
//              performs downsampling at the same time.
//
//  Parameters:
//      pSrc   - array of pointers, holds SBR-processed subband signals.
//      pSrcRe - array of pointers that contains real parts of SBR-processed subband signals.
//      pSrcIm - array of pointers that contains imaginary parts of SBR-processed subband signals.
//      pDst   - Pointer to the output vector, holds time domain output samples.
//      pSbrTableWindowDown  - Pointer to the window table that is used by SBR Analysis Filter.
//      NumLoop- Parameter equal to 32 if frame size of the core decoded signal is 1024
//      30 if frame size of the core decoded signal is 960.
//      pFilterSpec  - Pointer to the FilterSpec_SBR specification structure.
//      pWorkBuf     - Pointer to the work buffer used by the filter.
//
//  Return Values:     status
*/
IPPAPI( IppStatus, ippsSynthesisDownFilter_SBR_CToR_32f_D2L,
       (const Ipp32f* pSrcRe[],const Ipp32f* pSrcIm[], Ipp32f* pDst,
       const Ipp32f* pSbrTableWindowDown, int NumLoop,
       const IppsFilterSpec_SBR_C_32f* pFilterSpec, Ipp8u* pWorkBuf ))
IPPAPI( IppStatus, ippsSynthesisDownFilter_SBR_CToR_32fc32f_D2L,
       (const Ipp32fc* pSrc[], Ipp32f* pDst, const Ipp32f* pSbrTableWindowDown, int NumLoop,
       const IppsFilterSpec_SBR_C_32fc* pFilterSpec, Ipp8u* pWorkBuf ))
IPPAPI( IppStatus, ippsSynthesisDownFilter_SBR_RToR_32f_D2L,
       (const Ipp32f* pSrc[], Ipp32f* pDst, const Ipp32f* pSbrTableWindowDown, int NumLoop,
       const IppsFilterSpec_SBR_R_32f* pFilterSpec, Ipp8u* pWorkBuf ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:       ippsPredictCoef_SBR_R_32f_D2L, ippsPredictCoef_SBR_C_32fc_D2L, ippsPredictCoef_SBR_C_32f_D2L
//  Purpose:    Obtains prediction filter coefficients using covariance method.
//
//  Parameters:
//      pSrc    - array of pointers holds input real or complex matrix ([40][32]),
//                holds Low Frequency QMF-processed subband signals.
//      pSrcRe  - array of pointers ([40][32]) that contains real
//                parts of Low Frequency SBR-processed subband signals.
//      pSrcIm  - array of pointers ([40][32]) that contains
//                imaginary parts of Low Frequency SBR-processed subband signals.
//      k0      - First QMF subband in the f_master table.
//      len     - Size autocorrelation.
//      pAlpha0, pAlpha1     - Pointers to the  arrays that contain complex
//                             or real Prediction filter coefficients used by High Frequency filter.
//      pAlpha0Re, pAlpha1Re - Pointers to the  arrays that contain real parts of the
//                             corresponding complex filter coefficients.
//      pAlpha0Im, pAlpha1Im - Pointers to the arrays that contain imaginary parts of the
//                             corresponding complex filter coefficients.
//
//  Return Values:     status
*/
IPPAPI( IppStatus, ippsPredictCoef_SBR_R_32f_D2L,
       (const Ipp32f* pSrc[], Ipp32f* pAlpha0, Ipp32f* pAlpha1, int k0, int len))
IPPAPI( IppStatus, ippsPredictCoef_SBR_C_32fc_D2L,
       (const Ipp32fc* pSrc[], Ipp32fc* pAlpha0, Ipp32fc* pAlpha1, int k0, int len))
IPPAPI( IppStatus, ippsPredictCoef_SBR_C_32f_D2L,
                                            (const Ipp32f* pSrcRe[], const Ipp32f* pSrcIm[],
                                            Ipp32f* pAlpha0Re, Ipp32f* pAlpha0Im,
                                            Ipp32f* pAlpha1Re, Ipp32f* pAlpha1Im,
                                            int k0, int len))

IPPAPI( IppStatus, ippsPredictOneCoef_SBRHQ_32sc_D2L,
       (const Ipp32sc* pSrc[], Ipp32sc* pAlpha0, Ipp32sc* pAlpha1, int k, int lenCorr))

IPPAPI( IppStatus, ippsPredictOneCoef_SBRLP_32s_D2L,
       (const Ipp32s* pSrc[], Ipp32s* pAlpha0, Ipp32s* pAlpha1,
        Ipp32s* pRefCoef, int k, int lenCorr, int flag))

/****************************************************************************
 * FIXED-POINT VERSION
 *
 *
 ****************************************************************************/

 /* <-------------------- HQ MODE --------------------------> */

  /* GET SIZE */
  IPPAPI( IppStatus, ippsAnalysisFilterGetSize_SBRHQ_32s32sc, ( int* pSizeSpec ))

  IPPAPI( IppStatus, ippsSynthesisFilterGetSize_SBRHQ_32sc32s, ( int* pSizeSpec ))

  IPPAPI( IppStatus, ippsSynthesisDownFilterGetSize_SBRHQ_32sc32s, ( int* pSizeSpec))

  /* INIT */
  IPPAPI( IppStatus, ippsAnalysisFilterInit_SBRHQ_32s32sc,
       ( IppsFilterSpec_SBR_C_32sc* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisFilterInit_SBRHQ_32sc32s,
       ( IppsFilterSpec_SBR_C_32sc* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisDownFilterInit_SBRHQ_32sc32s,
       ( IppsFilterSpec_SBR_C_32sc* pFilterSpec))

  /* INITALLOC */
  IPPAPI( IppStatus, ippsAnalysisFilterInitAlloc_SBRHQ_32s32sc,
       ( IppsFilterSpec_SBR_C_32sc** ppFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisFilterInitAlloc_SBRHQ_32sc32s,
       ( IppsFilterSpec_SBR_C_32sc** ppFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisDownFilterInitAlloc_SBRHQ_32sc32s,
       ( IppsFilterSpec_SBR_C_32sc** ppFilterSpec))

  /* FREE */
  IPPAPI( IppStatus, ippsAnalysisFilterFree_SBRHQ_32s32sc,
       ( IppsFilterSpec_SBR_C_32sc* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisFilterFree_SBRHQ_32sc32s,
       ( IppsFilterSpec_SBR_C_32sc* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisDownFilterFree_SBRHQ_32sc32s,
       ( IppsFilterSpec_SBR_C_32sc* pFilterSpec))

  /* MAIN */
  IPPAPI( IppStatus, ippsAnalysisFilter_SBRHQ_32s32sc,
       (const Ipp32s* pSrc, Ipp32sc* pDst,int kx,
        const IppsFilterSpec_SBR_C_32sc* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisFilter_SBRHQ_32sc32s,
       (const Ipp32sc* pSrc, Ipp32s* pDst,
        const IppsFilterSpec_SBR_C_32sc* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisDownFilter_SBRHQ_32sc32s,
       (const Ipp32sc* pSrc, Ipp32s* pDst,
        const IppsFilterSpec_SBR_C_32sc* pFilterSpec ))

/* <-----------------------LP MODE -----------------------> */

  /* GET SIZE */
  IPPAPI( IppStatus, ippsAnalysisFilterGetSize_SBRLP_32s, ( int* pSizeSpec ))

  IPPAPI( IppStatus, ippsSynthesisFilterGetSize_SBRLP_32s, ( int* pSizeSpec ))

  IPPAPI( IppStatus, ippsSynthesisDownFilterGetSize_SBRLP_32s, ( int* pSizeSpec ))

  /* INIT */
  IPPAPI( IppStatus, ippsAnalysisFilterInit_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisFilterInit_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisDownFilterInit_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s* pFilterSpec))

  /* INITALLOC */
  IPPAPI( IppStatus, ippsAnalysisFilterInitAlloc_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s** ppFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisFilterInitAlloc_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s** ppFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisDownFilterInitAlloc_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s** ppFilterSpec))

  /* FREE */
  IPPAPI( IppStatus, ippsAnalysisFilterFree_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisFilterFree_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisDownFilterFree_SBRLP_32s,
       ( IppsFilterSpec_SBR_R_32s* pFilterSpec))

  /* MAIN */
  IPPAPI( IppStatus, ippsAnalysisFilter_SBRLP_32s,
       (const Ipp32s* pSrc, Ipp32s* pDst,int kx,
        const IppsFilterSpec_SBR_R_32s* pFilterSpec))

  IPPAPI( IppStatus, ippsSynthesisFilter_SBRLP_32s,
       (const Ipp32s* pSrc, Ipp32s* pDst,
        const IppsFilterSpec_SBR_R_32s* pFilterSpec ))

  IPPAPI( IppStatus, ippsSynthesisDownFilter_SBRLP_32s,
       (const Ipp32s* pSrc, Ipp32s* pDst,
        const IppsFilterSpec_SBR_R_32s* pFilterSpec ))

/*
// ENCODER PART
*/

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterEncGetSize_SBR_32f
//
//  Purpose:
//    Returns size of FilterSpec_SBR specification structures & init buffer.
//
//  Parameters:
//    pSizeSpec             Address of the FilterSpec_SBR specification
//                          structure size value in bytes.
//    pSizeInitBuf          Address size of the buffer for initialization
//                          functions in bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
*/
IPPAPI(IppStatus, ippsAnalysisFilterEncGetSize_SBR_32f,
       (int *pSizeSpec,
       int *pSizeInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterInitEnc_SBR_32f
//
//  Purpose:
//    Initializes analysis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to FilterSpec_SBR specification structure
//
//    pInitBuf              Pointer to the Init buffer.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier pFilterSpec is
//                          incorrect.
*/

IPPAPI(IppStatus, ippsAnalysisFilterEncInit_SBR_32f,
       (IppsFilterSpec_SBR_C_32fc  *ppFilterSpec,
       Ipp8u                      *pInitBuf))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterEncInitAlloc_SBR_32f
//
//  Purpose:
//    Allocate memory for analysis specification structure and initializes it.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the IppsFilterSpec_SBR
//                          specification structure is written.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when ppFilterSpec is NULL.
*/

IPPAPI(IppStatus, ippsAnalysisFilterEncInitAlloc_SBR_32f, (IppsFilterSpec_SBR_C_32f **ppFilterSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterEncFree_SBR_32f
//
//  Purpose:
//    Frees memory allocated for internal IppsFilterSpec_SBR structure.
//
//  Parameters:
//    pFilterSpec           Pointer to the IppsFilterSpec_SBR specification
//                          structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pFilterSpec is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/

IPPAPI(IppStatus, ippsAnalysisFilterEncFree_SBR_32f, (IppsFilterSpec_SBR_C_32f *pFilterSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterEnc_SBR_32f32fc
//
//  Purpose:
//    Splits time domain signal output from the core decoder into 32 subband
//    signals.
//
//  Parameters:
//    pSrc                  Pointer to the input audio signal.
//
//    pDst                  Pointer to the output signal
//
//    pFilterSpec           Pointer to the FilterSpec_SBR specification
//                          structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
//
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/

IPPAPI(IppStatus, ippsAnalysisFilterEnc_SBR_32f32fc,
       (const Ipp32f   *pSrc,
       Ipp32fc        *pDst,
       const IppsFilterSpec_SBR_C_32f  *pFilterSpec))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDetectTransient_SBR_32f
//
//  Purpose:
//    Transient detection is done by function.
//
//  Parameters:
//    pSrc                  pointer to the vector which stores the spectrum component of audio signal.
//
//    pInOutThr            Pointer to the threshold of deviation.
//
//    pDst                 pointer to the transient threshold
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
*/

IPPAPI( IppStatus, ippsDetectTransient_SBR_32f, (const Ipp32f* pSrc, Ipp32f* pInOutThr, Ipp32f* pDst))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsEstimateTNR_SBR_32f
//
//  Purpose:
//    Function estimates Tonality-To-Noise Ratio of audio signal.
//
//  Parameters:
//    pSrc                 pointer to the vector which stores the spectrum component of audio signal
//
//    pTNR0                Pointer to first estimation of TNR.
//
//    pTNR1                Pointer to second estimation of TNR
//
//    pMeanNrg             pointer
//
//  Returns:
//    ippStsNoErr          Indicates no error.
//    ippStsNullPtrErr     Indicates an error when any of the specified
//                         pointers is NULL.
*/

IPPAPI( IppStatus, ippsEstimateTNR_SBR_32f, (const Ipp32fc* pSrc, Ipp32f* pTNR0, Ipp32f* pTNR1, Ipp32f* pMeanNrg))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsUnpackFrameHeader_MP3
//
//  Purpose:
//    Unpacks audio frame header.
//
//  Parameters:
//    ppBitStream           Pointer to the pointer to the first byte of the
//                          MP3 frame header (*ppBitstream will be updated
//                          in the function).
//    pFrameHeader          Pointer to the MP3 frame header structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when ppBitStreamp or
//                          FrameHeader is NULL.
*/

IPPAPI( IppStatus, ippsAnalysisFilter_PS_32fc_D2, (const Ipp32fc* pSrc, Ipp32fc ppDst[32][12],
                                                   IppAC_PS_DEC_ANALYSIS config))
/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilter_PS_32fc_D2
//
//  Purpose:
//    Increases frequency resolution of the first few lower subbands by hybrid filtering
//
//  Parameters:
//    pSrc                  Pointer to the lower QMF subband
//    ppDst                 Output matrix, contains hybrid subband samples
//    config                flag describing configuration of hybrid filtering
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the specified pointers is NULL.
//    ippStsBadArgErr       Indicates an error when config isn't correct
*/

IPPAPI(IppStatus, ippsUnpackFrameHeader_MP3, (
  Ipp8u**            ppBitStream,
  IppMP3FrameHeader* pFrameHeader))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsUnpackSideInfo_MP3
//
//  Purpose:
//    Unpacks side information from input bitstream for use during decoding
//    of associated frame.
//
//  Parameters:
//    ppBitStream           Pointer to the pointer to the first byte of the
//                          side information associated with the current
//                          frame in the bit stream buffer. The function
//                          updates this parameter.
//    pFrameHeader          Pointer to the structure that contains the
//                          unpacked MP3 frame header. The header structure
//                          provides format information about the input
//                          bitstream. Both single- and dual-channel MPEG-1
//                          and MPEG-2 modes are supported.
//    pDstSideInfo          Pointer to the MP3 side information structure.
//                          The structure contains side information that
//                          applies to all granules and all channels for the
//                          current frame. One or more of the structures are
//                          placed contiguously in the buffer pointed to by
//                          pDstSideInfo in the following order:
//                          {granule 0 (channel 0, channel 1), granule 1 (channel
//                          channel 1)}.
//    pDstMainDataBegin     Pointer to the main_data_begin field.
//    pDstPrivateBits       Pointer to the private bits field.
//    pDstScfsi             Pointer to the scale factor selection
//                          information associated with the current frame.
//                          The data is organized contiguously in the buffer
//                          pointed to by pDstScfsi in the following order:
//                          {channel 0 (scfsi_band 0, scfsi_band 1, ...,
//                          scfsi_band 3), channel 1 (scfsi_band 0,
//                          scfsi_band 1, ..., [, fsi_band 3)}.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pDstSideInfo,
//                          pDstMainDataBegin, pDstPrivateBits, pDstScfsi,
//                          pFrameHeader, or ppBitStream is NULL.
//    ippStsMP3FrameHeaderErr Indicates an error when some elements in the
//                          MP3 frame header structure are invalid:
//                          pFrameHeader->id != IPP_MP3_ID_MPEG1
//                          pFrameHeader->id != IPP_MP3_ID_MPEG2
//                          pFrameHeader->layer != 1 pFrameHeader->mode < 0
//                          pFrameHeader->mode > 3.
//    ippStsMP3SideInfoErr  Indicates an error when the value of block_type
//                          is zero when window_switching_flag is set.
*/
IPPAPI(IppStatus, ippsUnpackSideInfo_MP3, (
  Ipp8u**            ppBitStream,
  IppMP3SideInfo*    pDstSideInfo,
  int*               pDstMainDataBegin,
  int*               pDstPrivateBits,
  int*               pDstScfsi,
  IppMP3FrameHeader* pFrameHeader))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsUnpackScaleFactors_MP3_1u8s
//
//  Purpose:
//    Unpacks scalefactors.
//
//  Parameters:
//    ppBitStream           Pointer to the pointer to the first bitstream
//                          buffer byte associated with the scale factors
//                          for the current frame, granule, and channel. The
//                          function updates this parameter.
//    pOffSet               Pointer to the next bit in the byte referenced by *
//                          ppBitStream. Valid within the range of 0 to 7,
//                          where 0 corresponds to the most significant bit
//                          and 7 corresponds to the least significant bit.
//                          The function updates this parameter.
//    pDstScaleFactor       Pointer to the scalefactor vector for long and/
//                          or short blocks.
//    pSideInfo             Pointer to the MP3 side information structure
//                          associated with the current granule and channel.
//    pScfsi                Pointer to scale factor selection information
//                          for the current channel.
//    pFrameHeader          Pointer to MP3 frame header structure for the
//                          current frame.
//    granule               Granule index. Can take the values
//                          of either 0 or 1.
//    channel               Channel index. Can take the values of either 0
//                          or 1.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when ppBitStream, pOffset,
//                          pDstScaleFactor, pSideInfo, pScfsi, ppBitStream,
//                          or pFrameHeader is NULL.
*/
IPPAPI(IppStatus, ippsUnpackScaleFactors_MP3_1u8s, (
  Ipp8u**            ppBitStream,
  int*               pOffset,
  Ipp8s*             pDstScaleFactor,
  IppMP3SideInfo*    pSideInfo,
  int*               pScfsi,
  IppMP3FrameHeader* pFrameHeader,
  int                granule,
  int                channel))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsHuffmanDecode_MP3_1u32s
//
//  Purpose:
//    Decodes Huffman data.
//
//  Parameters:
//    ppBitStream           Pointer to the pointer to the first bit stream
//                          byte that contains the Huffman code words
//                          associated with the current granule and channel.
//                          The function updates this parameter.
//    pOffset               Pointer to the starting bit position in the bit
//                          stream byte pointed to by *ppBitStream.
//                          The parameter is valid within the
//                          range of 0 to 7, where 0 corresponds to the most
//                          significant bit, and 7 corresponds to the least
//                          significant bit. The function updates this
//                          parameter.
//    pDstIs                Pointer to the vector of decoded Huffman symbols
//                          used to compute the quantized values of the 576
//                          spectral coefficients associated with the
//                          current granule and channel.
//    pDstNonZeroBound      Pointer to the spectral region above which all
//                          coefficients are set to zero.
//    pSideInfo             Pointer to MP3 structure containing side
//                          information associated with the current granule
//                          and channel.
//    pFrameHeader          Pointer to MP3 structure containing the header
//                          associated with the current frame.
//    hufSize               The number of Huffman code bits associated with
//                          the current granule and channel.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pOffset, pDstIs,
//                          pDstNonZeroBound, pSideInfo, pFrameHeader, or
//                          ppBitStream is NULL or when pOffset < 0 or
//                          pOffset > 7.
//    ippStsMP3FrameHeaderErr Indicates an error when some elements in the
//                          MP3 frame header structure are invalid.
//    ippStsMP3SideInfoErr  Indicates an error when some elements in the MP3
//                          side information structure are invalid or when
//                          hufSize < 0 or hufSize > pSideInfo->part23Len.
//    ippStsErr             Indicates unknown error.
*/
IPPAPI(IppStatus, ippsHuffmanDecode_MP3_1u32s, (
  Ipp8u**            ppBitStream,
  int*               pOffset,
  Ipp32s*            pDstIs,
  int*               pDstNonZeroBound,
  IppMP3SideInfo*    pSideInfo,
  IppMP3FrameHeader* pFrameHeader,
  int                hufSize))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsHuffmanDecodeSfb_MP3_1u32s
//
//  Purpose:
//    Decodes Huffman data.
//
//  Parameters:
//    ppBitStream           Pointer to the pointer to the first bit stream
//                          byte that contains the Huffman code words
//                          associated with the current granule and channel.
//                          The function updates this parameter.
//    pOffset               Pointer to the starting bit position in the bit
//                          stream byte pointed to by *ppBitStream.
//                          The parameter is valid within the
//                          range of 0 to 7, where 0 corresponds to the most
//                          significant bit, and 7 corresponds to the least
//                          significant bit. The function updates this
//                          parameter.
//    pDstIs                Pointer to the vector of decoded Huffman symbols
//                          used to compute the quantized values of the 576
//                          spectral coefficients associated with the
//                          current granule and channel.
//    pDstNonZeroBound      Pointer to the spectral region above which all
//                          coefficients are set to zero.
//    pSideInfo             Pointer to MP3 structure containing side
//                          information associated with the current granule
//                          and channel.
//    pFrameHeader          Pointer to MP3 structure containing the header
//                          associated with the current frame.
//    hufSize               The number of Huffman code bits associated with
//                          the current granule and channel.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pOffset, pDstIs,
//                          pDstNonZeroBound, pSideInfo, pFrameHeader, or
//                          ppBitStream is NULL or when pOffset < 0 or
//                          pOffset > 7.
//    ippStsMP3FrameHeaderErr Indicates an error when some elements in the
//                          MP3 frame header structure are invalid.
//    ippStsMP3SideInfoErr  Indicates an error when some elements in the MP3
//                          side information structure are invalid or when
//                          hufSize < 0 or hufSize > pSideInfo->part23Len.
//    ippStsErr             Indicates unknown error.
*/
IPPAPI(IppStatus, ippsHuffmanDecodeSfb_MP3_1u32s, (
  Ipp8u**            ppBitStream,
  int*               pOffset,
  Ipp32s*            pDstIs,
  int*               pDstNonZeroBound,
  IppMP3SideInfo*    pSideInfo,
  IppMP3FrameHeader* pFrameHeader,
  int                hufSize,
  IppMP3ScaleFactorBandTableLong pSfbTableLong))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsHuffmanDecodeSfbMbp_MP3_1u32s
//
//  Purpose:
//    Decodes Huffman data.
//
//  Parameters:
//    ppBitStream           Pointer to the pointer to the first bit stream
//                          byte that contains the Huffman code words
//                          associated with the current granule and channel.
//                          The function updates this parameter.
//    pOffset               Pointer to the starting bit position in the bit
//                          stream byte pointed to by *ppBitStream.
//                          The parameter is valid within the
//                          range of 0 to 7, where 0 corresponds to the most
//                          significant bit, and 7 corresponds to the least
//                          significant bit. The function updates this
//                          parameter.
//    pDstIs                Pointer to the vector of decoded Huffman symbols
//                          used to compute the quantized values of the 576
//                          spectral coefficients associated with the
//                          current granule and channel.
//    pDstNonZeroBound      Pointer to the spectral region above which all
//                          coefficients are set to zero.
//    pSideInfo             Pointer to MP3 structure containing side
//                          information associated with the current granule
//                          and channel.
//    pFrameHeader          Pointer to MP3 structure containing the header
//                          associated with the current frame.
//    hufSize               The number of Huffman code bits associated with
//                          the current granule and channel.
//    pSfbTableLong         Pointer to the scale factor bands table for a
//                          long block.
//    pSfbTableShort        Pointer to scale factor band table for short
//                          block. You can use the default table from
//                          MPEG-1, MPEG-2 standards.
//    pMbpTable             Pointer to the mixed block partition table.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pOffset, pDstIs,
//                          pDstNonZeroBound, pSideInfo, pFrameHeader, or
//                          ppBitStream is NULL or when pOffset < 0 or
//                          pOffset > 7.
//    ippStsMP3FrameHeaderErr Indicates an error when some elements in the
//                          MP3 frame header structure are invalid.
//    ippStsMP3SideInfoErr  Indicates an error when some elements in the MP3
//                          side information structure are invalid or when
//                          hufSize < 0 or hufSize > pSideInfo->part23Len.
//    ippStsErr             Indicates unknown error.
*/
IPPAPI(IppStatus, ippsHuffmanDecodeSfbMbp_MP3_1u32s, (
  Ipp8u**            ppBitStream,
  int*               pOffset,
  Ipp32s*            pDstIs,
  int*               pDstNonZeroBound,
  IppMP3SideInfo*    pSideInfo,
  IppMP3FrameHeader* pFrameHeader,
  int                hufSize,
  IppMP3ScaleFactorBandTableLong    pSfbTableLong,
  IppMP3ScaleFactorBandTableShort   pSfbTableShort,
  IppMP3MixedBlockPartitionTable    pMbpTable))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsReQuantize_MP3_32s_I
//
//  Purpose:
//    Requantizes the decoded Huffman symbols.
//
//  Parameters:
//    pSrcDstIsXr           Pointer to the vector of the decoded Huffman
//                          symbols. For stereo and dual channel modes,
//                          right channel data begins at the address &(
//                          pSrcDstIsXr[576]). The function updates this
//                          vector.
//    pNonZeroBound         Pointer to the spectral bound above which all
//                          coefficients are set to zero. For stereo and
//                          dual channel modes, the left channel bound is
//    pNonZeroBound         [0], and the right channel bound is pNonZeroBound
//    [,\.
//    pScaleFactor          Pointer to the scalefactor buffer. For stereo
//                          and dual channel modes, the right channel
//                          scalefactors begin at & (pScaleFactor [
//                          IPP_MP3_SF_BUF_LEN]).
//    pSideInfo             Pointer to the side information for the current
//                          granule.
//    pFrameHeader          Pointer to the frame header for the current frame.
//    pBuffer               Pointer to the workspace buffer. The buffer
//                          length must be 576 samples.
//    pSfbTableLong         Pointer to the scale factor bands table for a
//                          long block.
//    pSfbTableShort        Pointer to the scale factor bands table for a
//                          short block.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSrcDstIsXr,
//                          pNonZeroBound, pScaleFactor, pSideInfo,
//                          pFrameHeader, or pBuffer is NULL.
//    ippStsBadArgErr       Indicates an error when pNonZeroBound exceeds [0
//                          , 576].
//    ippStsMP3FrameHeaderErr Indicates an error when
//                          pFrameHeader->id != IPP_MP3_ID_MPEG1
//                          pFrameHeader->id != IPP_MP3_ID_MPEG2
//                          pFrameHeader->samplingFreq exceeds [0, 2],
//                          pFrameHeader->mode exceeds [0, 3],
//                          pFrameHeader->modeExt exceeds [0, 3].
//    ippStsMP3SideInfoErr  Indicates an error when the bitstream is in the
//                          stereo mode, but the block type of left is
//                          different from that of right, pSideInfo[ch].
//                          blockType exceeds [0, 3], pSideInfo[ch].
//                          mixedBlock exceeds [0, 1], pSideInfo[ch].
//                          globGain exceeds [0, 255], pSideInfo[ch].sfScale
//                          exceeds [0, 1], pSideInfo[ch].preFlag exceeds [0
//                          , 1], pSideInfo[ch].pSubBlkGain[w] exceeds [0, 7
//                          ], where ch is within the range of 0 to 1, and w
//                          is within the range of 0 to 2.
//    ippStsErr             Indicates an unknown error.
*/
IPPAPI(IppStatus, ippsReQuantize_MP3_32s_I, (
  Ipp32s*            pSrcDstIsXr,
  int*               pNonZeroBound,
  Ipp8s*             pScaleFactor,
  IppMP3SideInfo*    pSideInfo,
  IppMP3FrameHeader* pFrameHeader,
  Ipp32s*            pBuffer))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsReQuantizeSfb_MP3_32s_I
//
//  Purpose:
//    Requantizes the decoded Huffman symbols.
//
//  Parameters:
//    pSrcDstIsXr           Pointer to the vector of the decoded Huffman
//                          symbols. For stereo and dual channel modes,
//                          right channel data begins at the address &(
//                          pSrcDstIsXr[576]). The function updates this
//                          vector.
//    pNonZeroBound         Pointer to the spectral bound above which all
//                          coefficients are set to zero. For stereo and
//                          dual channel modes, the left channel bound is
//                          pNonZeroBound[0], and the right channel bound
//                          is pNonZeroBound[1].
//    pScaleFactor          Pointer to the scalefactor buffer. For stereo
//                          and dual channel modes, the right channel
//                          scalefactors begin at & (pScaleFactor [
//                          IPP_MP3_SF_BUF_LEN]).
//    pSideInfo             Pointer to the side information for the current
//                          granule.
//    pFrameHeader          Pointer to the frame header for the current frame.
//    pBuffer               Pointer to the workspace buffer. The buffer
//                          length must be 576 samples.
//    pSfbTableLong         Pointer to the scale factor bands table for a
//                          long block.
//    pSfbTableShort        Pointer to the scale factor bands table for a
//                          short block.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSrcDstIsXr,
//                          pNonZeroBound, pScaleFactor, pSideInfo,
//                          pFrameHeader, or pBuffer is NULL.
//    ippStsBadArgErr       Indicates an error when pNonZeroBound exceeds
//                          [0, 576].
//    ippStsMP3FrameHeaderErr Indicates an error when
//                          pFrameHeader->id != IPP_MP3_ID_MPEG1
//                          pFrameHeader->id != IPP_MP3_ID_MPEG2
//                          pFrameHeader->samplingFreq exceeds [0, 2],
//                          pFrameHeader->mode exceeds [0, 3],
//                          pFrameHeader->modeExt exceeds [0, 3].
//    ippStsMP3SideInfoErr  Indicates an error when the bitstream is in the
//                          stereo mode, but the block type of left is
//                          different from that of right, pSideInfo[ch].
//                          blockType exceeds [0, 3], pSideInfo[ch].
//                          mixedBlock exceeds [0, 1], pSideInfo[ch].
//                          globGain exceeds [0, 255], pSideInfo[ch].sfScale
//                          exceeds [0, 1], pSideInfo[ch].preFlag exceeds [0
//                          , 1], pSideInfo[ch].pSubBlkGain[w] exceeds [0, 7
//                          ], where ch is within the range of 0 to 1, and w
//                          is within the range of 0 to 2.
//    ippStsErr             Indicates an unknown error.
*/
IPPAPI(IppStatus, ippsReQuantizeSfb_MP3_32s_I, (
  Ipp32s*            pSrcDstIsXr,
  int*               pNonZeroBound,
  Ipp8s*             pScaleFactor,
  IppMP3SideInfo*    pSideInfo,
  IppMP3FrameHeader* pFrameHeader,
  Ipp32s*            pBuffer,
  IppMP3ScaleFactorBandTableLong  pSfbTableLong,
  IppMP3ScaleFactorBandTableShort pSfbTableShort))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInv_MP3_32s
//
//  Purpose:
//    Performs the first stage of hybrid synthesis filter bank.
//
//  Parameters:
//    pSrcXr                Pointer to the vector of requantized spectral
//                          samples for the current channel and granule,
//                          represented in Q5.26 format.
//    pDstY                 Pointer to the vector of IMDCT outputs in Q7.24
//                          format for input to PQMF bank.
//    pSrcDstOverlapAdd     Pointer to the overlap-add buffer. Contains the
//                          overlapped portion of the previous granule IMDCT
//                          output in Q7.24 format. The function updates
//                          this buffer.
//    nonZeroBound          Limiting bound for spectral coefficients. All
//                          spectral coefficients exceeding this boundary
//                          become zero for the current granule and channel.
//    pPrevNumOfImdct       Pointer to the number of IMDCTs computed for the
//                          current channel of the previous granule. The
//                          function updates this parameter so that it
//                          references the number of IMDCTs for the current
//                          granule.
//    blockType             Block type indicator.
//    mixedBlock            Mixed block indicator.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsBadArgErr       Indicates an error condition if at least one of
//                          the specified pointers is NULL.
//    ippStsErr             Indicates an error when one or more of the
//                          following input data errors are detected: either
//                          blockType exceeds [0,3], or mixedBlock exceeds [0
//                          ,1], or nonZeroBound exceeds [0,576], or *
//                          pPrevNumOfImdct exceeds [0,32].
*/
IPPAPI(IppStatus, ippsMDCTInv_MP3_32s, (
  Ipp32s* pSrcXr,
  Ipp32s* pDstY,
  Ipp32s* pSrcDstOverlapAdd,
  int     nonZeroBound,
  int*    pPrevNumOfImdct,
  int     blockType,
  int     mixedBlock))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthPQMF_MP3_32s16s
//
//  Purpose:
//    Performs the second stage of hybrid synthesis filter bank.
//
//  Parameters:
//    pSrcY                 Pointer to the block of 32 IMDCT sub-band input
//                          samples in Q7.24 format.
//    pDstAudioOut          Pointer to the block of 32 reconstructed PCM
//                          output samples in 16-bit signed little-endian
//                          format. Left and right channels are interleaved
//                          according to the mode flag.
//    pVBuffer              Pointer to the input workspace buffer containing
//                          Q7.24 data. The function updates this parameter.
//                          NOTE. Note that the pointers pSrcXr and pDstY
//                          must reference different buffers. 10-104 10
//                          Intel Integrated Performance Primitives
//                          Reference Manual: Volume 1
//    pVPosition            Pointer to the internal workspace index. The
//                          function updates this parameter.
//    mode                  Flag that indicates whether or not PCM audio
//                          output channels should be interleaved. 1
//                          indicates not interleaved, 2 indicates
//                          interleaved.
//
//  Returns:
//    ippStsNoErr           Indicates no error. ippNullPtrErr Indicates an
//                          error when least one of the pointers pSrcY,
//                          pDstAudioOut, pVBuffer, or pVPosition is NULL.
//                          Audio Coding Functions 10 10-105
//    ippStsBadArgErr       Indicates an error when at least one of the
//                          specified pointers is NULL, or the value of mode
//                          is less than 1 or more than 2, or when the value
//                          of pVPosition exceeds [0, 15].
//    ippStsErr             Indicates an unknown error.
//
//  Notes:
//    Because the PQMF bank contains memory, the MP3 application must maintain
//    two state variables in between calls to the primitive.
//    First, the application must preallocate for the PQMF computation
//    a workspace buffer of size 512 x Number of Channels. This buffer is
//    referenced by the pointer pVBuffer, and its elements should be
//    initialized to zero prior to the first call. During subsequent calls,
//    the pVBuffer input for the current call should contain the pVbuffer
//    output generated by the previous call. In addition to pVBuffer, the MP3
//    application must also initialize to zero and thereafter preserve
//    the value of the state variable pVPosition. The MP3 application should
//    modify the values contained in pVBuffer or pVPosition only during
//    decoder reset, and the reset values should always be zero.
*/
IPPAPI(IppStatus, ippsSynthPQMF_MP3_32s16s, (
  Ipp32s* pSrcY,
  Ipp16s* pDstAudioOut,
  Ipp32s* pVBuffer,
  int*    pVPosition,
  int     mode))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsUnpackADIFHeader_AAC
//
//  Purpose:
//    Gets the AAC ADIF format header.
//
//  Parameters:
//    ppBitStream           Double pointer to the current byte before the
//                          ADIF header.
//    pADIFHeader           Pointer to the IppACCADIFHeader structure.
//    pPrgCfgElt            Pointer to the IppAACPrgCfgElt structure. There
//                          must be prgCfgEltMax elements in the buffer.
//    prgCfgEltMax          Maximum program configure element number. Must
//                          be within the range of [1, 16].
//
//  Returns:
//    ippStsNoErr           Indicates no error. Audio Coding Functions 10 10-
//                          121
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pADIFHeader, pPrgCfgElt,
//                          or *ppBitStream is NULL.
//    ippStsAacPrgNumErr    Indicates an error when the decoded pADIFHeader->
//                          numPrgCfgElt > prgCfgEltMax, or prgCfgEltMax is
//                          outside the range of [1,IPP_AAC_MAX_ELT_NUM].
//
// Notes:
//    1. pADIFHeader->numPrgCfgElt is the number directly unpacked
//       from bitstream plus 1.
//    2. It is assumed that it is known how many IppAACPrgCfgElt
//       must be supported.
*/
IPPAPI(IppStatus, ippsUnpackADIFHeader_AAC,(Ipp8u **ppBitStream,
                    IppAACADIFHeader *pADIFHeader, IppAACPrgCfgElt *pPrgCfgElt,
                                                            int prgCfgEltMax ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodePrgCfgElt_AAC
//
//  Purpose:
//    Gets program configuration element from the input bitstream.
//
//  Parameters:
//    ppBitStream           Double pointer to the current byte after
//                          decoding the program configuration element.
//    pOffset               Pointer to the bit position in the byte pointed
//                          to by *ppBitStream. Valid within 0 to 7: 0
//                          stands for the most significant bit of the byte;
//                          7 stands for the least significant bit of the
//                          byte.
//    pPrgCfgElt            Pointer to IppAACPrgCfgElt structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pOffset, pPrgCfgElt, or *
//                          ppBitStream is NULL.
//    ippStsAacBitOffsetErr Indicates an error when pOffset is out of the
//                          range of [0,7].
*/
IPPAPI(IppStatus, ippsDecodePrgCfgElt_AAC,(Ipp8u **ppBitStream, int *pOffset,
                                                 IppAACPrgCfgElt *pPrgCfgElt ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsUnpackADTSFrameHeader_AAC
//
//  Purpose:
//    Gets ADTS frame header from the input bitstream.
//
//  Parameters:
//    ppBitStream           Double pointer to the current byte after
//                          unpacking the ADTS frame header.
//    pADTSFrameHeader      Pointer to the IppAACADTSFrameHeader structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pADTSFrameHeader, or
//                          *ppBitStream is NULL.
*/
IPPAPI(IppStatus, ippsUnpackADTSFrameHeader_AAC, (Ipp8u **ppBitStream,
                                                  IppAACADTSFrameHeader *pADTSFrameHeader))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeDatStrElt_AAC
//
//  Purpose:
//    Gets data stream element from the input bitstream.
//
//  Parameters:
//    ppBitStream           Double pointer to the current byte.
//    pOffset               Pointer to the bit position in the byte pointed
//                          to by *ppBitStream. Valid within 0 to 7. 0
//                          stands for the most significant bit of the byte.
//                          7 stands for the least significant bit of the
//                          byte.
//    ppBitStream           Double pointer to the current byte after the
//                          decode data stream element.
//    pOffset               Pointer to the bit position in the byte pointed
//                          to by *ppBitStream. Valid within 0 to 7. 0
//                          stands for the most significant bit of the byte.
//                          7 stands for the least significant bit of the
//                          byte.
//    pDataTag              Pointer to element_instance_tag. See Table 6.20
//                          of ISO/IEC 13818-7:1997. pDataCn Pointer to the
//                          value of data length in bytes.
//    pDstDataElt           Pointer to the data stream buffer that contains
//                          the data stream extracted from the input
//                          bitstream. There are 512 elements in the buffer
//                          pointed to by pDstDataElt.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pOffset, *ppBitStream,
//                          pDataTag, pDataCnt, or pDstDataElt is NULL.
//    ippStsAacBitOffsetErr Indicates an error when *pOffset is out of range
//                          [0,7].
*/
IPPAPI(IppStatus, ippsDecodeDatStrElt_AAC,(Ipp8u **ppBitStream, int *pOffset,
                            int *pDataTag, int *pDataCnt, Ipp8u *pDstDataElt ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeFillElt_AAC
//
//  Purpose:
//    Gets the fill element from the input bitstream.
//
//  Parameters:
//    ppBitStream           Pointer to the pointer to the current byte after
//                          the decode fill element.
//    pOffset               Pointer to the bit position in the byte pointed
//                          to by *ppBitStream. Valid within 0 to 7. 0
//                          stands for the most significant bit of the byte.
//                          7 stands for the least significant bit of the
//                          byte.
//    pFillCnt              Pointer to the value of the length of total fill
//                          data in bytes.
//    pDstFillElt           Pointer to the fill data buffer whose length
//                          must be equal to or greater than 270.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pOffset, pFillCnt, or
//                          pDstFillElt is NULL.
//    ippStsAacBitOffsetErr Indicates an error when *pOffset is out of the
//                          range of [0,7].
*/
IPPAPI(IppStatus, ippsDecodeFillElt_AAC,(Ipp8u **ppBitStream, int *pOffset,
                                           int *pFillCnt, Ipp8u *pDstFillElt ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeChanPairElt_AAC
//
//  Purpose:
//    Gets channel_pair_element from the input bitstream.
//
//  Parameters:
//    ppBitStream           Double pointer to the current byte after
//                          decoding the channel pair element.
//    pOffset               Pointer to the bit position in the byte pointed
//                          to by *ppBitStream. Valid within 0 to 7: 0
//                          stands for the most significant bit of the byte;
//                          7 stands for the least significant bit of the
//                          byte.
//    pIcsInfo              Pointer to IppAACIcsInfo structure. If pIcsInfo->
//                          predDataPres = 0, set pIcsInfo->predReset = 0.
//                          Only the first pIcsInfo->numWinGrp elements in
//                          pIcsInfo-> pWinGrpLen are meaningful. You should
//                          not change some members of the structure, as
//                          shown in Table 10-5.
//    pChanPairElt          Pointer to IppAACChanPairElt structure. You
//                          should not change some members of the structure,
//                          as shown in Table 10-5.
//    predSfbMax            Maximum prediction scale factor bands. For LC
//                          profile, set predSfbMax = 0.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pOffset, *ppBitStream,
//                          pIcsInfo, or pChanPairElt is NULL.
//    ippStsAacBitOffsetErr Indicates an error when pOffset is out of the
//                          range of [0,7].
//    ippStsAacMaxSfbErr    Indicates an error when pIcsInfo->maxSfb decoded
//                          from bitstream is greater than IPP_AAC_MAX_SFB,
//                          the maximum scale factor band for all sampling
//                          frequencies.
//    ippStsAacPredSfbErr   Indicates an error when predSfbMax is out of
//                          the range of [0, IPP_AAC_PRED_SFB_MAX].
*/
IPPAPI(IppStatus, ippsDecodeChanPairElt_AAC, (Ipp8u             **ppBitStream,
                                              int               *pOffset,
                                              IppAACIcsInfo     *pIcsInfo,
                                              IppAACChanPairElt *pChanPairElt,
                                              int predSfbMax))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsNoiselessDecoder_LC_AAC
//
//  Purpose:
//    Decodes all data for one channel.
//
//  Parameters:
//    ppBitStream           Double pointers to bitstream buffer.
//    pOffset               Pointer to the offset in one byte.
//    pChanInfo             Pointer to the channel information.
//                          IppAACChanInfo structure. Denotes pIcsInfo as
//                          pChanInfo->pIcsInfo as shown in Table 10-6.
//    commonWin             Common window indicator.
//    pDstScalefactor       Pointer to the scalefactor or intensity position
//                          buffer. Buffer length is more than or equal to
//                          120. Only maxSfb elements are stored for each
//                          group. There is no space between sequence groups.
//    pDstQuantizedSpectralCoef Pointer to the quantized spectral
//                          coefficients data. For short block, the
//                          coefficients are interleaved by scale factor
//                          window bands in each group. Buffer length is
//                          more than or equal to 1024.
//    pDstSfbCb             Pointer to the scale factor band codebook.
//                          Buffer length must be more than or equal to 120.
//                          Store maxSfb elements for each group. There is
//                          no space between the sequence groups.
//    pDstTnsFiltCoef       Pointer to TNS coefficients. Buffer length must
//                          be more than or equal to 60. The store sequence
//                          is TNS order elements for each filter for each
//                          window. The elements are not changed if the
//                          corresponding TNS order is zero.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers ppBitStream, pOffset, pChanInfo,
//                          pDstScalefactor, pDstQuantizedSpectralCoef,
//                          pDstSfbCb, pDstTnsFiltCoef, pChanInfo->pIcsInfo,
//                          or *ppBitStream is NULL.
//    ippStsAacBitOffsetErr Indicates an error when pOffset is out of range
//                          [0,7].
//    ippStsAacComWinErr    Indicates an error when commonWin exceeds [0,1].
//    ippStsAacSmplRateIdxErr Indicates an error when pChanInfo->
//                          samplingRateIndex exceeds [0,11].
//    ippStsAacPredSfbErr   Indicates an error when pChanInfo->predSfbMax is
//                          not equal to 0.
//    ippStsAacMaxSfbErr    Indicates an error when pChanInfo->pIcsInfo->
//                          maxSfb > pChanInfo->numSwb.
//    ippStsAacSectCbErr    Indicates an error when the codebook pointed to
//                          by pChanInfo->pSectCb is illegal or when (
//                          pChanInfo->pSectCb)==12, 13. pTnsRegionLen As
//                          output. Pointer to the length of the region in
//                          units of scale factor bands to which one filter
//                          is applied in each window. pTnsFiltOrder As
//                          output. Pointer to the order of the temporal
//                          noise shaping filter applied to each window.
//                          pTnsDirection As output. Pointer to the token
//                          that indicates whether the filter is applied in
//                          the upward or downward direction. 0 stands for
//                          upward and 1 for downward. pIcsInfo As input if
//                          commonWin == 1. As output if commonWin == 0. If
//                          pIcsInfo->predDataPres == 0, set pIcsInfo->
//                          predReset = 0. Only the first pIcsInfo->
//                          numWinGrp elements in pIcsInfo-> pWinGrpLen are
//                          meaningful. Under specific conditions, some
//                          members of the structure must remain unchanged.
//    ippStsAacPlsDataErr   Indicates an error when the pChanInfo->pIcsInfo->
//                          winSequence indicates a short sequence and
//                          pChanInfo->pulsePres indicates pulse data present
//                          . The start scale factor band for pulse data >=
//                          pChanInfo->numSwb, or pulse data position offset
//                          >= winLen.
//    ippStsAacGainCtrErr   Indicates an error when pChanInfo->
//                          gainControlPres is decoded as 1, which means
//                          that gain control data is present. Gain control
//                          data is not currently supported.
//    ippStsAacCoefValErr   Indicates an error when the quantized
//                          coefficients value pointed to by pDstCoef
//                          exceeds range [-8191, 8191].
//
//  Notes:
//    User must set pChanInfo->samplingRateIndex, predSfbMax, pIcsInfo,
//    before calling this function.
*/
IPPAPI(IppStatus, ippsNoiselessDecoder_LC_AAC,(Ipp8u **ppBitStream,
                        int *pOffset, int commonWin, IppAACChanInfo *pChanInfo,
                    Ipp16s *pDstScalefactor, Ipp32s *pDstQuantizedSpectralCoef,
                                    Ipp8u *pDstSfbCb, Ipp8s *pDstTnsFiltCoef ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsQuantInv_AAC_32s_I
//
//  Purpose:
//    Performs inverse quantization of Huffman symbols for current channel
//    inplace.
//
//  Parameters:
//    pSrcDstSpectralCoef   On input, pointer to the input quantized
//                          coefficients. For short block the coefficients
//                          are interleaved by scale factor window bands in
//                          each group. Buffer length must be more than or
//                          equal to 1024. On output, pointer to the
//                          destination inverse quantized coefficient in Q13.
//                          18 format. For short block, the coefficients are
//                          interleaved by scale factor window bands in each
//                          group. Buffer length must be more than or equal
//                          to 1024. The maximum error of output.
//                          pSrcDstSpectralCoef[i] is listed in Table 10-7.
//    pScalefactor          Pointer to the scalefactor buffer. Buffer length
//                          must be more than or equal to 120.
//    numWinGrp             Group number.
//    pWinGrpLen            Pointer to the number of windows in each group.
//                          Buffer length must be more than or equal to 8.
//    maxSfb                Maximal scale factor bands number for the
//                          current block.
//    pSfbCb                Pointer to the scale factor band codebook,
//                          buffer length must be more than or equal to 120.
//                          Only maxSfb elements for each group are
//                          meaningful. There are no spaces between the
//                          sequence groups.
//    samplingRateIndex     Sampling rate index. Valid within [0, 11]. See
//                          Table 6.5 of ISO/IEC 13818-7:1997.
//    winLen                Data number in one window.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacSmplRateIdxErr Indicates an error when pChanInfo->
//                          samplingRateIndex exceeds [0, 11].
//    ippStsAacMaxSfbErr    Indicates an error when maxSfb exceeds [0,
//                          IPP_AAC_MAX_SFB].
//    ippStsAacWinGrpErr    Indicates an error when numWinGrp exceeds [0, 8]
//                          for long window or is not equal to 1 for short
//                          window.
//    ippStsAacWinLenErr    Indicates an error when winLen is not equal to
//                          128 or 1024;
//    ippStsAacCoefValErr   Indicates an error when the quantized
//                          coefficients value pointed to by
//                          pSrcDstSpectralCoef exceeds [-8191,8191].
*/
IPPAPI(IppStatus, ippsQuantInv_AAC_32s_I,(Ipp32s *pSrcDstSpectralCoef,
                                     const Ipp16s *pScalefactor, int numWinGrp,
                        const int *pWinGrpLen, int maxSfb, const Ipp8u *pSfbCb,
                                           int samplingRateIndex, int winLen ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeMsStereo_AAC_32s_I
//
//  Purpose:
//    Processes mid-side (MS) stereo for pair channels in-place.
//
//  Parameters:
//    pSrcDstL              On input, pointer to left channel data in Q13.18
//                          format. For short block, the coefficients are
//                          interleaved by scale factor window bands in each
//                          group. Buffer length must be more than or equal
//                          to 1024. On output, pointer to left channel data
//                          in Q13.18 format. For short blocks, the
//                          coefficients are interleaved by scale factor
//                          window bands in each group. Buffer length must
//                          be more than or equal to 1024.
//    pSrcDstR              On input, pointer to right channel data in Q13.
//                          18 format. For short block, the coefficients are
//                          interleaved by scale factor window bands in each
//                          group. Buffer length must be more than or equal
//                          to 1024. On output, pointer to right channel
//                          data in Q13.18 format. For short blocks, the
//                          coefficients are interleaved by scale factor
//                          window bands in each group. Buffer length must
//                          be more than or equal to 1024.
//    msMaskPres            MS stereo mask flag: - 0: MS off; - 1: MS on; - 2:
//                          MS all bands on.
//    pMsUsed               Pointer to the MS Stereo flag buffer. Buffer
//                          length must be more than or equal to 120.
//                          pSfbCbPointer Pointer to the scale factor band
//                          codebook, buffer length must be more than or
//                          equal to 120. Store maxSfb elements for each
//                          group.There is no space between the sequence
//                          groups.
//    numWinGrp             Group number.
//    pWinGrpLen            Pointer to the number of windows in each group.
//                          Buffer length must be more than or equal to 8.
//    maxSfb                Maximal scale factor bands number for the
//                          current block.
//    samplingRateIndex     Sampling rate index. Valid within [0, 11]. See
//                          Table 6.5 of ISO/IEC 13818-7:1997.
//    winLen                Data number in one window.
//    pSrcDstR              Pointer to right channel data in Q13.18 format.
//                          For short blocks, the coefficients are
//                          interleaved by scale factor window bands in each
//                          group. Buffer length must be more than or equal
//                          to 1024.
//    pSfbCb                Pointer to the scale factor band codebook. If
//                          invert_intensity (group, sfb) = -1, and if
//                          *pSfbCb = INTERITY_HCB, let
//                          *pSfbCb = INTERITY_HCB2.
//                          If *pSfbCb = INTERITY_HCB2, let
//                          *pSfbCb = INTERITY_HCB. Buffer length must be more
//                          than or equal to 120. Store maxSfb elements for
//                          each group. There is no space between the
//                          sequence groups.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacMaxSfbErr    Indicates an error when the coefficient index
//                          calculated from samplingFreqIndex and maxSfb
//                          exceeds winLen in each window.
//    ippStsAacSamplRateIdxErr Indicates an error when pChanInfor->
//                          samplingRateIndex exceeds [0,11].
//    ippStsAacWinGrpErr    Indicates an error when numWinGrp exceeds [0,8]
//                          for long window or is not equal to 1 for short
//                          window.
//    ippStsAacWinLenErr    Indicates an error when winLen is not equal 128
//                          or 1024.
//    ippStsStereoMaskErr   Indicates an error when the stereo mask flag is
//                          not equal 1 or 2.
//
// Notes:
//    Input and output data are in interleaving mode, only for CPE.
*/
IPPAPI(IppStatus, ippsDecodeMsStereo_AAC_32s_I,(Ipp32s *pSrcDstL,
                        Ipp32s *pSrcDstR, int msMaskPres, const Ipp8u *pMsUsed,
                           Ipp8u *pSfbCb, int numWinGrp, const int *pWinGrpLen,
                               int maxSfb, int samplingRateIndex, int winLen ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeIsStereo_AAC_32s
//
//  Purpose:
//    Processes intensity stereo for pair channels.
//
//  Parameters:
//    pSrcL                 Pointer to left channel data in Q13.18 format.
//                          For short block, the coefficients are
//                          interleaved by scale factor window bands in each
//                          group. Buffer length must be more than or equal
//                          to 1024.
//    pDstR                 Pointer to right channel data in Q13.18 format.
//                          For short block, the coefficients are
//                          interleaved by scale factor window bands in each
//                          group. Buffer length must be more than or equal
//                          to 1024.
//    pScalefactor          Pointer to the scalefactor buffer. Buffer length
//                          must be more than or equal to 120.
//    pSfbCb                Pointer to the scale factor band codebook,
//                          buffer length must be more than or equal to 120.
//                          Store maxSfb elements for each group.There is no
//                          space between the sequence groups. Respective
//                          values of pSfbCb[sfb] equal to 1, -1, or 0
//                          indicate the intensity stereo mode, that is,
//                          direct, inverse, or none.
//    numWinGrp             Group number.
//    pWinGrpLen            Pointer to the number of windows in each group.
//                          Buffer length must be more than or equal to 8.
//    maxSfbMax             Maximal scale factor bands number for
//                          the current block.
//    samplingRateIndex     Sampling rate index. Valid within [0, 11]. See
//                          Table 6.5 of ISO/IEC 13818-7:1997.
//    winLen                Data number in one window.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacMaxSfbErr    Indicates an error when the coefficient index
//                          calculated from samplingFreqIndex and maxSfb
//                          exceeds winLen in each window.
//    ippStsAacSamplRateIdxErr Indicates an error when pChanInfor->
//                          samplingRateIndex exceeds [0,11].
*/
IPPAPI(IppStatus, ippsDecodeIsStereo_AAC_32s,(const Ipp32s *pSrcL,
                                     Ipp32s *pDstR, const Ipp16s *pScalefactor,
                     const Ipp8u *pSfbCb, int numWinGrp, const int *pWinGrpLen,
                               int maxSfb, int samplingRateIndex, int winLen ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDeinterleaveSpectrum_AAC_32s
//
//  Purpose:
//    Deinterleaves the coefficients for short block.
//
//  Parameters:
//    pSrc                  Pointer to source coefficients buffer. The
//                          coefficients are interleaved by scale factor
//                          window bands in each group. Buffer length must
//                          be more than or equal to 1024.
//    pDst                  Pointer to the output of coefficients. Data
//                          sequence is ordered in pDst[w*128+sfb*sfbWidth[
//                          sfb]+i], where w is window index, sfb is scale
//                          factor band index, sfbWidth is the scale factor
//                          band width table, i is the index within scale
//                          factor band. Buffer length must be more than or
//                          equal to 1024.
//    numWinGrp             Group number.
//    pWinGrpLen            Pointer to the number of windows in each group.
//                          Buffer length must be more than or equal to 8.
//    maxSfb                Maximal scale factor bands number for
//                          the current block.
//    samplingRateIndex     Sampling rate index. Valid in [0, 11]. See Table
//                          6.5 of ISO/IEC 13818-7:1997.
//    winLen                Data number in one window.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacMaxSfbErr    Indicates an error when the coefficient index
//                          calculated from samplingFreqIndex and maxSfb
//                          exceeds winLen in each window.
//    ippStsAacSamplRateIdxErr Indicates an error when pChanInfor->
//                          samplingRateIndex exceeds [0,11].
//    ippStsAacWinGrpErr    Indicates an error when numWinGrp exceeds [0,8].
//    ippStsAacWinLenErr    Indicates an error when winLen is not equal to
//                          128.
*/
IPPAPI(IppStatus, ippsDeinterleaveSpectrum_AAC_32s,(const Ipp32s *pSrc,
                            Ipp32s *pDst, int numWinGrp, const int *pWinGrpLen,
                               int maxSfb, int samplingRateIndex, int winLen ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeTNS_AAC_32s_I
//
//  Purpose:
//    Decodes for Temporal Noise Shaping in-place.
//
//  Parameters:
//    pSrcDstSpectralCoefs  On input, pointer to the input spectral
//                          coefficients to be filtered by the all-pole
//                          filters in Q13.18 format. There are 1024
//                          elements in the buffer pointed to by
//                          pSrcDstSpectralCoefs. On output, pointer to the
//                          output spectral coefficients after filtering by
//                          the all-pole filters in Q13.18 format.
//    pTnsNumFilt           Pointer to the number of noise shaping filters
//                          used for each window of the current frame. There
//                          are 8 elements in the buffer pointed to by
//    pTnsNumFilt           which are arranged as follows:pTnsNumFilt[w]:
//                          the number of noise shaping filters used for
//                          window w, w = 0 to numWin-1.
//    pTnsRegionLen         Pointer to the length of the region in units of
//                          scale factor bands to which one filter is
//                          applied in each window of the current frame.
//                          There are 8 elements in the buffer pointed to by
//                          pTnsRegionLen, which are arranged as follows:
//                          pTnsRegionLen[i]: the length of the region to
//                          which filter filt is applied in window w w = 0
//                          to numWin-1, filt=0 to pTnsNumFilt[w]-1.
//    pTnsFiltOrder         Pointer to the order of one noise shaping filter
//                          applied to each window of the current frame.
//                          There are 8 elements in the buffer pointed to by
//                          pTnsFiltOrder, which are arranged as follows:
//                          pTnsFiltOrder[i]: the order of one noise shaping
//                          filter filt, which is applied to window w w = 0
//                          to numWin-1, filt=0 to pTnsNumFilt[w]-1.
//    pTnsFiltCoefRes       Pointer to the resolution of 3 bits or 4 bits of
//                          the transmitted filter coefficients for each
//                          window of the current frame. There are 8
//                          elements in the buffer pointed to by
//                          pTnsFiltCoefRes, which are arranged as follows:
//                          pTnsFiltCoefRes[w]: the resolution of the
//                          transmitted filter coefficients for window w, w
//                          = 0 to numWin-1.
//    pTnsFiltCoef          Pointer to the coefficients of one noise shaping
//                          filter applied to each window of the current
//                          frame. There are 60 elements in the buffer
//                          pointed to by pTnsFiltCoef, which are arranged
//                          as follows: pTnsFiltCoef[i], pTnsFiltCoef[i+1],
//                          ..., pTnsFiltCoef[i+order-1]: the coefficients of
//                          one noise shaping filter filt, which is applied
//                          to window w. The order is the same as that of
//                          the noise shaping filter filt as applied to
//                          window w, w = 0 to numWin-1, filt=0 to
//                          pTnsNumFilt[w]-1. For example, pTnsFiltCoef[0],
//                          pTnsFiltCoef[1], ..., pTnsFiltCoef[order0-1] are
//                          the coefficients of the noise shaping filter 0,
//                          which is applied to window 0, if present. If so,
//                          pTnsFiltCoef[order0], pTnsFiltCoef[order0+1], ...,
//                          pTnsFiltCoef[order0+order1-1] are the
//                          coefficients of the noise shaping filter 1
//                          applied to window 0, if present, and so on.
//                          order0 is the same as that of the noise shaping
//                          filter 0 applied to window 0, and order1 is the
//                          order of the noise shaping filter 1 applied to
//                          window 0. After window 0 is processed, process
//                          window 1, then window 2 until all numWin windows
//                          are processed.
//    pTnsDirection         Pointer to the token that indicates whether the
//                          filter is applied in the upward or downward
//                          direction. 0 stands for upward and 1 for downward
//                          . There are 8 elements in the buffer pointed to
//                          by pTnsDirection which are arranged as follows:
//                          pTnsDirection[i]: the token indicating whether
//                          the filter filt is applied in upward or downward
//                          direction to window w w = 0 to numWin-1, filt=0
//                          to pTnsNumFilt[w]-1.
//    maxSfb                Number of scale factor bands transmitted per
//                          window group of the current frame.
//    profile               Profile index from Table 7.1 in ISO/IEC 13818-7:
//                          1997.
//    samplingRateIndex     Index indicating the sampling rate of the
//                          current frame.
//    winLen                Data number in one window.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL. IppStsTnsProfielErr Indicates
//                          an error when profile != 1.
//    ippStsAacTnsNumFiltErr Indicates an error when a data error occurs:
//                          for a short window sequence pTnsNumFilt[w]
//                          exceeds [0,1]; Table 10-8 Computation Error List
//                          for pSrcDstSpectralCoefs MAX(error(
//                          pSrcDstSpectralCoefs[i])) Condition 4095 8 ==
//                          numWin 32767 1 == numWin NOTE. This function
//                          supports LC profile only. 10-142 10 Intel
//                          Integrated Performance Primitives Reference
//                          Manual: Volume 1 for a long window sequence,
//                          pTnsNumFilt[w] exceeds [0,3].
//    ippStsAacTnsLenErr    Indicates an error when pTnsRegionLen exceeds
//                          [0, numSwb].
//    ippStsAacTnsOrderErr  Indicates an error when a data error occurs: for
//                          a short window sequence, pTnsFiltOrder exceeds
//                          [0,7]; for a long window sequence, pTnsFiltOrder
//                          exceeds [0,12].
//    ippStsAacTnsCoefResErr Indicates an error when pTnsFiltCoefRes[w]
//                          exceeds [3, 4].
//    ippStsAacTnsCoefErr   Indicates an error when *pTnsFiltCoef exceeds
//                          [-8, 7].
//    ippStsAacTnsDirectErr Indicates an error when *pTnsDirection exceeds
//                          [0, 1].
//
// Notes:
//    Input/Output data(pSrcDstSpectralCoefs[]) are in Q3.
*/
IPPAPI(IppStatus, ippsDecodeTNS_AAC_32s_I,(Ipp32s *pSrcDstSpectralCoefs,
                              const int *pTnsNumFilt, const int *pTnsRegionLen,
                          const int *pTnsFiltOrder, const int *pTnsFiltCoefRes,
                           const Ipp8s *pTnsFiltCoef, const int *pTnsDirection,
                  int maxSfb, int profile, int samplingRateIndex, int winLen ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInv_AAC_32s16s
//
//  Purpose:
//    Maps time-frequency domain signal into time domain and generates 1024
//    reconstructed 16-bit signed little-endian PCM samples.
//
//  Parameters:
//    pSrcSpectralCoefs     Pointer to the input time-frequency domain
//                          samples in Q13.18 format. There are 1024
//                          elements in the buffer pointed to by
//                          pSrcSpectralCoefs.
//    pSrcDstOverlapAddBuf  Pointer to the overlap-add buffer that contains
//                          the second half of the previous block windowed
//                          sequence in Q13.18. There are 1024 elements in
//                          this buffer.
//    winSequence           Flag that indicates which window sequence is
//                          used for current block.
//    winShape              Flag that indicates which window function is
//                          selected for current block.
//    prevWinShape          Flag that indicates which window function is
//                          selected for previous block.
//    pcmMode               Flag that indicates whether the PCM audio output
//                          is interleaved, that is has the pattern LRLRLR...
//                          or not. 1 stands for not interleaved. 2 stands
//                          for interleaved
//    pDstPcmAudioOut       Pointer to the output 1024 reconstructed 16-bit
//                          signed little-endian PCM samples in Q15,
//                          interleaved, if needed. The maximum computation
//                          error for pDstPcmAudioOut is less than 1 for
//                          each vector element. The total quadratic error
//                          for the vector is less than 96.
//    pSrcDstOverlapAddBuf  Pointer to the overlap-add buffer which contains
//                          the second half of the current block windowed
//                          sequence in Q13.18. The maximum computation
//                          error for pDstPcmAudioOut is less than 4 for
//                          each vector element. The total quadratic error
//                          for the vector is less than 1536.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsAacWinSeqErr    Indicates an error when winSequence exceeds [0,3].
//    ippStsAacWinShapeErr  Indicates an error when winShape or prevWinShape
//                          exceeds [0,1].
//    ippStsAacPcmModeErr   Indicates an error when pcmMode exceeds [1,2].
//
// Notes:
//    Input data (pSrcSpectralCoefs[]) is in Q = _IPP_AAC_FB_Q;
//    data (pSrcDstOverlapAdd[]) is in Q = _IPP_AAC_FB_Q.
*/
IPPAPI(IppStatus, ippsMDCTInv_AAC_32s16s,(Ipp32s *pSrcSpectralCoefs,
                         Ipp16s *pDstPcmAudioOut, Ipp32s *pSrcDstOverlapAddBuf,
                int winSequence, int winShape, int prevWinShape, int pcmMode ))

IPPAPI(IppStatus, ippsMDCTInv_AAC_32s_I,(Ipp32s *tst_SrcDst, Ipp32s *tst_OverlapBuf,
       int winSeq, int winShape, int prewinShape, int len))

IPPAPI(IppStatus, ippsMDCTFwd_AAC_32s_I,(Ipp32s *tst_SrcDst, int winSeq, int winShape,
       int prewinShape, int len))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeMainHeader_AAC
//
//  Purpose:
//    Gets main header information and main layer information from bit stream.
//
//  Parameters:
//    ppBitStream           Double pointer to bitstream buffer. *ppBitStream
//                          will be updated after decoding.
//    pOffset               Pointer to the offset in one byte. *pOffset will
//                          be updated after decoding.
//    channelNum            Number of channels.
//    monoStereoFlag        Current frame has mono and stereo layers.
//    pAACMainHeader        Pointer to the main element header.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacBitOffsetErr Indicates an error when pOffset exceeds [0,7].
//    ippStsAacChanErr      Indicates an error when channelNum exceeds [1,2].
//    ippStsAacMonoStereoErr Indicates an error when monoStereoFlag exceeds
//                          [0,1].
*/
IPPAPI(IppStatus, ippsDecodeMainHeader_AAC,(Ipp8u **ppBitStream,
                int *pOffset, IppAACMainHeader *pAACMainHeader, int channelNum,
                                                          int monoStereoFlag ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeExtensionHeader_AAC
//
//  Purpose:
//    Gets extension header information and extension layer information from
//    bit stream.
//
//  Parameters:
//    ppBitStream           Double pointer to bitstream buffer. *ppBitStream
//                          will be updated after decoding.
//    pOffset               Pointer to the offset in one byte. *pOffset will
//                          be updated after decoding.
//    monoStereoFlag        Flag indicating that the current frame has mono
//                          and stereo layers.
//    thisLayerStereo       Flag indicating that the current layer is stereo.
//    monoLayerFlag         Flag indicating that the current frame has a
//                          mono layer.
//    preStereoMaxSfb       Previous stereo layer maxSfb.
//    hightstMonoMaxSfb     Last mono layer maxSfb.
//    winSequence           Window type, short or long.
//    pAACExtHeader         Pointer to the extension element header.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacBitOffsetErr Indicates an error when pOffset is out of the
//                          range of [0,7].
//    ippStsAacStereoLayerErr Indicates an error when thisLayerStereo
//                          exceeds [0,1].
//    ippStsAacMonoLayerErr Indicates an error when monoLayerFlag exceeds
//                          [0,1].
//    ippStsAacMaxSfbErr    Indicates an error when at least one of
//                          preStereoMaxSfb, hightstMonoMaxSfb or number of
//                          scale factor bands used in this layer exceeds
//                          [0,IPP_AAC_MAX_SFB].
//    ippStsAacMonoStereoErr Indicates an error when monoStereoFlag exceeds
//                          [0,1].
//    ippStsAacWinSeqErr    Indicates an error when winSequence exceeds
//                          [0,3].
//
*/
IPPAPI(IppStatus, ippsDecodeExtensionHeader_AAC,(Ipp8u **ppBitStream,
              int *pOffset, IppAACExtHeader *pAACExtHeader, int monoStereoFlag,
                   int thisLayerStereo, int monoLayerFlag, int preStereoMaxSfb,
                                      int hightstMonoMaxSfb, int winSequence ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodePNS_AAC_32s
//
//  Purpose:
//    Implements perceptual noise substitution (PNS) coding within
//    individual channel stream (ICS).
//
//  Parameters:
//    pSrcDstSpec           Pointer to spectrum coefficients for perceptual
//                          noise substitution (PNS).
//    pSrcDstLtpFlag        Pointer to long term predict (LTP) flag.
//    pSfbCb                Pointer to the scale factor codebook.
//    pScaleFactor          Pointer to the scalefactor value.
//    maxSfb                Number of scale factor bands used in this layer.
//    numWinGrp             Number of window groups.
//    pWinGrpLen            Pointer to the length of every window group.
//    samplingFreqIndex     Sampling frequency index.
//    winLen                Window length. 1024 for long windows, 128 for
//                          short windows.
//    pRandomSeed           Random seed for PNS.
//    pSrcDstSpec           Pointer to the output spectrum substituted by
//                          perceptual noise.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacMaxSfbErr    Indicates an error when maxSfb exceeds [0,
//                          IPP_AAC_MAX_SFB].
//    ippStsAacSmplRateIdxErr Indicates an error when pChanInfo->
//                          samplingRateIndex exceeds [0,12].
//    ippStsAacWinLenErr    Indicates an error when winLen is not equal to
//                          128 or 1024.
*/
IPPAPI(IppStatus, ippsDecodePNS_AAC_32s,(Ipp32s *pSrcDstSpec,
                      int *pSrcDstLtpFlag, Ipp8u *pSfbCb, Ipp16s *pScaleFactor,
             int maxSfb, int numWinGrp, int *pWinGrpLen, int samplingFreqIndex,
                                                int winLen, int *pRandomSeed ))

/* ///////////////////////////////////////////////////////////////////////////
// Name:
//    ippsDecodeMsPNS_AAC_32s
//
// Purpose:
//    Implements perceptual noise substitution coding within an ICS.
//
// Parameters:
//    pSrcDstSpec           Pointer to spectrum coefficients before PNS.
//    pSrcDstLtpFlag        Pointer to the LTP flag.
//    pSfbCb                Pointer to the scalefactor code book.
//    pScaleFactor          Pointer to the scalefactor coefficients.
//    maxSfb                Number of max scalefactor band.
//    numWinGrp             Number groups of windows.
//    pWinGrpLen            Pointer to the group length.
//    samplingFreqIndex     Sampling frequency index.
//    winLen                Length of windows.
//    pRandomSeed           Random seed.
//    channel               Index of current channel:
//                            0:  left;
//                            1:  right.
//    pMsUsed               Pointer to MS used buffer in CPE structure.
//    pNoiseState           Pointer to noise state buffer, which stores
//                          the left channel's noise random seed for
//                          every scalefactor band. When pMsUsed[sfb]==1,
//                          the content in this buffer will be used for
//                          right channel.
//
// Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacMaxSfbErr    Indicates an error when maxSfb exceeds [0,
//                          IPP_AAC_MAX_SFB].
//    ippStsAacWinGrpErr    Indicates an error when numWinGrp exceeds [0,8].
//    ippStsAacSmplRateIdxErr Indicates an error when samplingFreqIndex
//                          exceeds [0,16].
//    ippStsAacWinLenErr    Indicates an error when winLen is not equal to
//                          128 or 1024.
*/
IPPAPI(IppStatus, ippsDecodeMsPNS_AAC_32s,(Ipp32s *pSrcDstSpec,
                      int *pSrcDstLtpFlag, Ipp8u *pSfbCb, Ipp16s *pScaleFactor,
             int maxSfb, int numWinGrp, int *pWinGrpLen, int samplingFreqIndex,
             int winLen, int *pRandomSeed, int channel, Ipp8u *pMsUsed, int *pNoiseState ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsLongTermReconstruct_AAC_32s
//
//  Purpose:
//    Uses Long Term Reconstruct (LTR) to reduce signal redundancy between
//    successive coding frames.
//
//  Parameters:
//    pSrcDstSpec           Pointer to spectral coefficients for LTP.
//    pSrcEstSpec           Pointer to the frequency domain vector.
//    winSequence           Window type, long or short.
//    samplingFreqIndex     Sampling frequency index.
//    pLtpFlag              Pointer to the LTP flag.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacSmplRateIdxErr Indicates an error when pChanInfo->
//                          samplingRateIndex exceeds [0,12].
//    ippStsAacWinSeqErr    Indicates an error when winSequence exceeds
//                          [0,3].
*/
IPPAPI(IppStatus, ippsLongTermReconstruct_AAC_32s,(Ipp32s *pSrcEstSpec,
                           Ipp32s *pSrcDstSpec, int *pLtpFlag, int winSequence,
                                                       int samplingFreqIndex ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsEncodeTNS_AAC_32s_I
//
//  Purpose:
//    Performs reversion of TNS in the Long Term Reconstruct loop in-place.
//
//  Parameters:
//    pSrcDst               On input, pointer to the spectral coefficients
//                          for the TNS encoding operation. On output,
//                          pointer to the spectral coefficients after the
//                          TNS encoding operation.
//    pTnsNumFilt           Pointer to the number of TNS filters.
//    pTnsRegionLen         Pointer to the length of TNS filter.
//    pTnsFiltOrder         Pointer to the TNS filter order.
//    pTnsFiltCoefRes       Pointer to the TNS coefficient resolution flag.
//    pTnsFiltCoef          Pointer to the TNS filter coefficients.
//    pTnsDirection         Pointer to the TNS direction flag.
//    maxSfb                Maximum scale factor number.
//    profile               Audio profile.
//    samplingFreqIndex     Sampling frequency index.
//    winLen                Window length.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsTnsProfileErr   Indicates an error when profile != 1.
//    ippStsAacTnsNumFiltErr Indicates an error when *pTnsNumFilt exceeds
//                          [0, 1] for the short window sequence or [0, 3]
//                          for the long window sequence.
//    ippStsAacTnsLenErr    Indicates an error when *pTnsRegionLen exceeds
//                          [0, numSwb].
//    ippStsAacTnsOrderErr  Indicates an error when *pTnsFiltOrder exceeds
//                          [0, 7] for the short window sequence or [0, 12]
//                          for the long window sequence.
//    ippStsAacTnsCoefResErr Indicates an error when *pTnsFiltCoefRes
//                          exceeds [3, 4].
//    ippStsAacTnsCoefErr   Indicates an error when *pTnsFiltCoef exceeds
//                          [-8, 7].
//    ippStsAacTnsDirectErr Indicates an error when *pTnsDirection exceeds
//                          [0, 1].
//    ippStsAacSmplRateIdxErr Indicates an error when samplingRateIndex
//                          exceeds [0, 12].
//    ippStsAacWinLenErr    Indicates an error when winLen is not equal to
//                          128 or 1024.
//
// Notes:
//    Input/Output data(pSrcDstSpectralCoefs[]) are in Q3.
*/
IPPAPI(IppStatus, ippsEncodeTNS_AAC_32s_I,(Ipp32s *pSrcDstSpectralCoefs,
                              const int *pTnsNumFilt, const int *pTnsRegionLen,
                          const int *pTnsFiltOrder, const int *pTnsFiltCoefRes,
                           const Ipp8s *pTnsFiltCoef, const int *pTnsDirection,
                  int maxSfb, int profile, int samplingRateIndex, int winLen ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsLongTermPredict_AAC_32s
//
//  Purpose:
//    Gets the predicted time domain signals in the Long Term Reconstruct
//    (LTP) loop.
//
//  Parameters:
//    pSrcTimeSignal        Pointer to the temporal signals to be predicted
//                          in the temporary domain.
//    pDstEstTimeSignal     Pointer to the output of samples after LTP.
//    pAACLtpInfo           Pointer to the LTP information.
//    winSequence           Window type, short or long.
//    pDstEstTimeSignal     Pointer to the prediction output in time domain.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers is NULL.
//    ippStsAacWinSeqErr    Indicates an error when winSequence exceeds
//                          [0,3].
*/
IPPAPI(IppStatus, ippsLongTermPredict_AAC_32s,(Ipp32s *pSrcTimeSignal,
                         Ipp32s *pDstEstTimeSignal, IppAACLtpInfo *pAACLtpInfo,
                                                             int winSequence ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsNoiseLessDecode_AAC
//
//  Purpose:
//    Performs noiseless decoding.
//
//  Parameters:
//    ppBitStream           Double pointer to the bitstream to be parsed.
//                          *ppBitStream will be updated after decoding.
//    pOffset               Pointer to the offset in one byte. *pOffset will
//                          be updated after decoding.
//    pAACMainHeader        Pointer to main header information. Not used for
//                          scalable objects. When commonWin == 0 &&
//                          scaleFlag==0, you need to decode LTP information
//                          and save it in pAACMainHeader->pLtpInfo[].
//    pChanInfo             Pointer to channel information structure.
//                          windowSequence Window type, short or long.
//    maxSfb                Number of scale factor bands.
//    commonWin             Indicates if the channel pair uses the same ICS
//                          information.
//    scaleFlag             Flag indicating whether the scalable type is used.
//    audioObjectType       Audio object type indicator: 1 indicates the
//                          main type 2 indicates the LC type 6 indicates
//                          the scalable mode.
//    pDstScaleFactor       Pointer to the parsed scalefactor.
//    pDstQuantizedSpectralCoef Pointer to the quantized spectral
//                          coefficients after Huffman decoding.
//    pDstSfbCb             Pointer to the scale factor codebook index.
//    pDstTnsFiltCoef       Pointer to TNS filter coefficients. Not used for
//                          scalable objects.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacBitOffsetErr Indicates an error when pOffset is out of the
//                          range [0,7].
//    ippStsAacComWinErr    Indicates an error when commonWin exceeds [0,1].
//    ippStsAacMaxSfbErr    Indicates an error when maxSfb exceeds [0,
//                          IPP_AAC_MAX_SFB].
//    ippStsAacSmplRateIdxErr Indicates an error when pChanInfo->
//                          samplingRateIndex exceeds [0,11].
//    ippStsAacCoefValErr   Indicates an error when the quantized
//                          coefficients value pointed to by pDstCoef
//                          exceeds the range of [-8191,8191].
//
// Notes:
//    User must set pChanInfo,winSequence, maxSfb, commonWin and
//    scaleflag before calling this function; commonWin and scaleFlag
//    are 1 in condition of scaleable;
//    In comparison with AAC LC, only decode section data has little
//    changes when get dpcm_noise_nrg or hcod_sf[dpcm_noise_nrg]
//    information (all are stored in pDstScalefactor).
*/
IPPAPI(IppStatus, ippsNoiselessDecode_AAC,(Ipp8u **ppBitStream, int *pOffset,
                     IppAACMainHeader *pAACMainHeader, Ipp16s *pDstScalefactor,
                           Ipp32s *pDstQuantizedSpectralCoef, Ipp8u *pDstSfbCb,
                             Ipp8s *pDstTnsFiltCoef, IppAACChanInfo *pChanInfo,
                                    int winSequence, int maxSfb, int commonWin,
                                          int scaleFlag, int audioObjectType ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsLtpUpdate_AAC_32s
//
//  Purpose:
//    Performs required buffer update in the Long Term Reconstruct (LTP) loop.
//
//  Parameters:
//    pSpecVal              Pointer to spectral value after TNS decoder in
//                          LTP loop.
//    pLtpSaveBuf           Pointer to save buffer for LTP. Buffer length
//                          should be 3*frameLength. The value is saved for
//                          next frame.
//    winSequence           Window type:
//                            - 0 stands for long
//                            - 1 stands for long start
//                            - 2 stands for short
//                            - 3 stands for long stop.
//    winShape              KBD or SIN window shape.
//    preWinShape           Previous window shape.
//    pWorkBuf              Work buffer for LTP update, length of pWorkBuf
//                          should be at least 2048*3 = 6144 words.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacWinSeqErr    Indicates an error when winSequence exceeds [0,3].
//    ippStsAacWinShapeErr  Indicates an error when winShape or preWinShape
//                          exceeds [0,1].
*/
IPPAPI(IppStatus, ippsLtpUpdate_AAC_32s,(Ipp32s *pSpecVal,
                            Ipp32s *pLtpSaveBuf, int winSequence, int winShape,
                                           int preWinShape, Ipp32s *pWorkBuf ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwd_AAC_32s
//
//  Purpose:
//    Generates spectrum coefficient of PCM samples.
//
//  Parameters:
//    pSrc                  Pointer to temporal signals to do MDCT.
//    pDst                  Output of MDCT, the spectral coefficients of PCM
//                          samples.
//    pOverlapAdd           Pointer to overlap buffer. Not used for MPEG-4
//                          AAC decoding.
//    winSequence           Window sequence indicating if the block is long
//                          or short.
//    winShape              Current window shape.
//    preWinShape           Previous window shape.
//    pWindowedBuf          Work buffer for MDCT. Should be at least 2048
//                          words.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacWinSeqErr    Indicates an error when winSequence exceeds [0,3].
//    ippStsAacWinShapeErr  Indicates an error when preWinShape exceeds
//                          [0,1].
*/
IPPAPI(IppStatus, ippsMDCTFwd_AAC_32s,(Ipp32s *pSrc, Ipp32s *pDst,
                            Ipp32s *pOverlapAdd, int winSequence, int winShape,
                                       int preWinShape, Ipp32s *pWindowedBuf ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsDecodeChanPairElt_MP4_AAC
//
//  Purpose:
//    Gets channel_pair_element from the input bitstream.
//
//  Parameters:
//    ppBitStream           Double pointer to the current byte.*ppBitStream
//                          will be updated after decoding.
//    pOffset               Pointer to the bit position in the byte pointed
//                          to by *ppBitStream. Valid within 0 to 7: 0
//                          stands for the most significant bit of the byte;
//                          7 stands for the least significant bit of the
//                          byte.
//    pIcsInfo              Pointer to IppAACIcsInfo structure.
//    pChanPairElt          Pointer to IppAACChanPairElt structure.
//    pAACMainHeader        Pointer to the main element header.
//    predSfbMax            Maximum prediction scale factor bands.
//    audioObjectType       Audio object type indicator: 1 indicates the
//                          main type 2 indicates the LC type 6 indicates
//                          the scalable mode.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          specified pointers is NULL.
//    ippStsAacBitOffsetErr Indicates an error when pOffset is out of the
//                          range of [0,7].
//    ippStsAacMaxSfbErr    Indicates an error when pIcsInfo->maxSfb decoded
//                          from bitstream is greater than IPP_AAC_MAX_SFB,
//                          the maximum scale factor band for all sampling
//                          frequencies.
*/
IPPAPI(IppStatus, ippsDecodeChanPairElt_MP4_AAC,(Ipp8u **ppBitStream, int *pOffset,
                            IppAACIcsInfo *pIcsInfo, IppAACChanPairElt *pChanPairElt,
                            IppAACMainHeader *pAACMainHeader, int predSfbMax,
                            int audioObjectType))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVLCDecodeEscBlock_MP3_1u16s
//
//  Purpose:
//    Parses the bitstream and decodes variable length code for MP3.
//
//  Parameters:
//    ppBitStream           Pointer to pointer to the current byte in the
//                          bitstream buffer. *ppBitStream is updated by the
//                          function.
//    pBitOffset            Pointer to the bit position in the byte pointed
//                          by *ppBitStream.
//                          *pBitOffset is updated by the function.
//    linbits               Length of escape sequence.
//    pData                 Pointer to the array of decoded values.
//    len                   Number of values to decode into array pData.
//    pVLCSpec              Pointer to VLCDecoder specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one or more pointers
//                          passed to the function is NULL.
//    ippStsVLCInputDataErr Indicates an error when incorrect input is used.
//                          For decode functions it can indicate that
//                          bitstream contains code that is not specified
//                          inside the used table.
//    ippStsBitOffsetErr    Indicates an error when *pBitOffset is less
//                          than 0 or more than 7.
//    ippStsContextMatchErr Indicate an error when pVLCSpec struct was not
//                          created by ippsVLCDecodeInit_32s or
//                          ippsVLCDecodeInitAlloc_32s functions.
*/
IPPAPI(IppStatus, ippsVLCDecodeEscBlock_MP3_1u16s,(Ipp8u **ppBitStream,
                                                   int *pBitOffset,
                                                   int linbits,
                                                   Ipp16s *pData,
                                                   int len,
                                                   const IppsVLCDecodeSpec_32s *pVLCSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVLCDecodeEscBlock_AAC_1u16s
//
//  Purpose:
//    Parses the bitstream and decodes variable length code for AAC.
//
//  Parameters:
//    ppBitStream           Pointer to pointer to the current byte in the
//                          bitstream buffer. *ppBitStream is updated by
/                           the function.
//    pBitOffset            Pointer to the bit position in the byte pointed
//                          by *ppBitStream.
//                          *pBitOffset is updated by the function.
//    pData                 Pointer to the array of decoded values.
//    len                   Number of values to decode into array pData.
//    pVLCSpec              Pointer to VLCDecoder specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one or more pointers
//                          passed to the function is NULL.
//    ippStsVLCInputDataErr Indicates an error when incorrect input is used.
//                          For decode functions it can indicate that
//                          bitstream contains code that is not specified
//                          inside the used table.
//    ippStsBitOffsetErr    Indicates an error when *pBitOffset is less
//                          than 0 or more than 7.
//    ippStsVLCAACEscCodeLengthErr Indicates an error when bitstream
//                          contains AAC-Esc code with the length more than
//                          21.
//    ippStsContextMatchErr Indicate an error when pVLCSpec struct was not
//                          created by ippsVLCDecodeInit_32s or
//                          ippsVLCDecodeInitAlloc_32s functions.
*/
IPPAPI(IppStatus, ippsVLCDecodeEscBlock_AAC_1u16s,(Ipp8u **ppBitStream,
                                                   int *pBitOffset,
                                                   Ipp16s *pData,
                                                   int len,
                                                   const IppsVLCDecodeSpec_32s *pVLCSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVLCEncodeEscBlock_MP3_16s1u
//
//  Purpose:
//    Encodes an array of values into destination bitstream in MP3 format
//    and advances bitstream pointer.
//
//  Parameters:
//    pInputData            Pointer to the array of source values.
//    len                   Size of values array pInputData.
//    linbits               Length of escape sequence.
//    ppBitStream           Pointer to pointer to the current byte in the
//                          bitstream buffer. *ppBitStream is updated by
//                          the function.
//    pBitOffset            Ppointer to the bit position in the byte pointed
//                          by *ppBitStream.
//                          *pBitOffset is updated by the function.
//    pVLCSpec              Pointer to VLCEncoder specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pointers to data
//                          arrays are NULL.
//    ippStsBitOffsetErr    Indicates an error when *pBitOffset is less
//                          than 0 or more than 7.
//    ippStsContextMatchErr Indicate an error when pVLCSpec struct was not
//                          created by ippsVLCEncodeInit_32s or
//                          ippsVLCEncodeInitAlloc_32s functions.
*/
IPPAPI(IppStatus, ippsVLCEncodeEscBlock_MP3_16s1u,(const Ipp16s *pInputData,
                                                   int len,
                                                   int linbits,
                                                   Ipp8u **ppBitStream,
                                                   int *pBitOffset,
                                                   const IppsVLCEncodeSpec_32s *pVLCSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVLCEncodeEscBlock_AAC_16s1u
//
//  Purpose:
//    Encodes an array of values into destination bitstream in AAC format
//    and advances bitstream pointer.
//
//  Parameters:
//    pInputData            Pointer to the array of source values.
//    len                   Size of values array pInputData.
//    ppBitStream           Pointer to pointer to the current byte in the
//                          bitstream buffer.
//                          *ppBitStream is updated by the function.
//    pBitOffset            Pointer to the bit position in the byte pointed
//                          by *ppBitStream.
//                          *pBitOffset is updated by the function.
//    pVLCSpec              Pointer to VLCEncoder specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pointers to data
//                          arrays are NULL.
//    ippStsBitOffsetErr    Indicates an error when *pBitOffset is less
//                          than 0 or more than 7.
//    ippStsContextMatchErr Indicate an error when pVLCSpec struct was not
//                          created by ippsVLCEncodeInit_32s or
//                          ippsVLCEncodeInitAlloc_32s functions.
*/
IPPAPI(IppStatus, ippsVLCEncodeEscBlock_AAC_16s1u,(const Ipp16s *pInputData,
                                                   int len,
                                                   Ipp8u **ppBitStream,
                                                   int *pBitOffset,
                                                   const IppsVLCEncodeSpec_32s *pVLCSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVLCCountEscBits_MP3_16s32s
//
//  Purpose:
//    Calculates the number of bits necessary for encoding in MP3 format.
//
//  Parameters:
//    pInputData            Pointer to the array of source values.
//    len                   Size of values array pInputData.
//    linbits               Length of escape sequence. pCounBits Pointer to
//                          calculated length in bits to encode pInputData.
//    pVLCSpec              Pointer to VLCEncoder specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pointers to data
//                          arrays are NULL.
//    ippStsContextMatchErr Indicate an error when pVLCSpec struct was not
//                          created by ippsVLCEncodeInit_32s or
//                          ippsVLCEncodeInitAlloc_32s functions.
*/
IPPAPI(IppStatus, ippsVLCCountEscBits_MP3_16s32s,(const Ipp16s *pInputData,
                                                  int len,
                                                  int linbits,
                                                  Ipp32s *pCountBits,
                                                  const IppsVLCEncodeSpec_32s *pVLCSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsVLCCountEscBits_AAC_16s32s
//
//  Purpose:
//    Calculates the number of bits necessary for encoding in AAC format.
//
//  Parameters:
//    pInputData            Pointer to the array of source values.
//    len                   Size of values array pInputData.
//    pCountBits            Pointer to calculated length in bits to encode
//                          pInputData.
//    pVLCSpec              Pointer to VLCEncoder specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when the pointers to data
//                          arrays are NULL.
//    ippStsContextMatchErr Indicate an error when pVLCSpec struct was not
//                          created by ippsVLCEncodeInit_32s or
//                          ippsVLCEncodeInitAlloc_32s functions.
*/
IPPAPI(IppStatus, ippsVLCCountEscBits_AAC_16s32s,(const Ipp16s *pInputData,
                                                  int len,
                                                  Ipp32s *pCountBits,
                                                  const IppsVLCEncodeSpec_32s *pVLCSpec))

/*******************************************************************/

/*
//  Name:       ippsVLCDecodeUTupleEscBlock_MP3_1u16s
//  Purpose:    decodes a len element encoded by VLC code from the source data
//              *ppBitStream with *pBitOffset bits offset and stores the result in
//              the destination *pData
//
//  Arguments:
//     ppBitStream                 Pointer to pointer to the current byte in the
//                                 bitstream buffer. *ppBitStream is updated by the function.
//     pBitOffset                  Pointer to the bit position in the byte pointed by
//                                 *ppBitStream. Valid within the range 0 to 7.
//                                 *pBitOffset is updated by the function.
//     linbits                     Length of escape sequence.
//     pData                       Pointer to the array of decoded values.
//     len                         Number of values to decode into array pData.
//     pVLCSpec                    pointer to VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr                  Indicates no error.
//     ippStsNullPtrErr             Indicates an error when one or more pointers
//                                  passed to the function is NULL.
//     ippStsVLCInputDataErr        Indicates an error when incorrect input is used.
//                                  For decode functions it can indicate that bitstream
//                                  contains code that is not specified inside the used table.
//     ippStsBitOffsetErr           Indicate an error when offset less than 0 or more then 7.
//     ippStsContextMatchErr        Indicate an error when pVLCSpec struct was not created by
//                                  ippsVLCDecodeInit_32s or ippsVLCDecodeInitAlloc_32s functions.
//
*/

IPPAPI(IppStatus, ippsVLCDecodeUTupleEscBlock_MP3_1u16s,(Ipp8u **ppBitStream,
                                                   int *pBitOffset,
                                                   int linbits,
                                                   Ipp16s *pData,
                                                   int len,
                                                   const IppsVLCDecodeUTupleSpec_32s *pVLCSpec))

/*******************************************************************/

/*
//  Name:       ippsVLCDecodeUTupleEscBlock_AAC_1u16s
//  Purpose:    decodes a len element encoded by VLC code from the source data
//              *ppBitStream with *pBitOffset bits offset and stores the result in
//              the destination *pData
//
//  Arguments:
//     ppBitStream                 Pointer to pointer to the current byte in the
//                                 bitstream buffer. *ppBitStream is updated by the function.
//     pBitOffset                  Pointer to the bit position in the byte pointed by
//                                 *ppBitStream. Valid within the range 0 to 7.
//                                 *pBitOffset is updated by the function.
//     pData                       Pointer to the array of decoded values.
//     len                         Number of values to decode into array pData.
//     pVLCSpec                    pointer to VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr                  Indicates no error.
//     ippStsNullPtrErr             Indicates an error when one or more pointers
//                                  passed to the function is NULL.
//     ippStsVLCInputDataErr        Indicates an error when incorrect input is used.
//                                  For decode functions it can indicate that bitstream
//                                  contains code that is not specified inside the used table.
//     ippStsVLCAACEscCodeLengthErr Indicates an error when bitstream contains AAC-Esc code
//                                  with the length more than 21.
//     ippStsBitOffsetErr           Indicate an error when offset less than 0 or more then 7.
//     ippStsContextMatchErr        Indicate an error when pVLCSpec struct was not created by
//                                  ippsVLCDecodeInit_32s or ippsVLCDecodeInitAlloc_32s functions.
//
*/

IPPAPI(IppStatus, ippsVLCDecodeUTupleEscBlock_AAC_1u16s,(Ipp8u **ppBitStream,
                                                   int *pBitOffset,
                                                   Ipp16s *pData,
                                                   int len,
                                                   const IppsVLCDecodeUTupleSpec_32s *pVLCSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsPackFrameHeader_MP3
//
//  Purpose:
//    Packs the content of the frame header into the bitstream.
//
//  Parameters:
//    pSrcFrameHeader       Pointer to the IppMP3FrameHeader structure. This
//                          structure contains all the header information
//                          associated with the current frame. All structure
//                          fields must contain valid data upon function
//                          entry.
//    ppBitStream           Pointer to the encoded bitstream buffer - a
//                          double pointer to the first byte in the
//                          bitstream buffer intended to receive the packed
//                          frame header bits generated by this function.
//                          The frame header bits are sequentially written
//                          into the stream buffer starting from the bit
//                          indexed by the combination of byte pointer
//                          ppBitStream. The frame header bits
//                          are formatted according to the bitstream syntax
//                          given in ISO/IEC 11172-3.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSrcFrameHeader or
//                          ppBitStream is NULL.
*/
IPPAPI(IppStatus, ippsPackFrameHeader_MP3, (
    IppMP3FrameHeader*  pSrcFrameHeader,
    Ipp8u**             ppBitStream))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsPackSideInfo_MP3
//
//  Purpose:
//    Packs the side information into the bitstream buffer.
//
//  Parameters:
//    pSrcSideInfo          Pointer to the IppMP3SideInfo structures. This
//                          should contain twice the channel number of
//                          elements. The order is the following: - granule 1
//                          , channel 1; - granule 1, channel 2; - granule 2,
//                          channel 1; - granule 2, channel 2. All fields of
//                          all set elements should contain valid data upon
//                          the function entry.
//    mainDataBegin         Negative bitstream offset, in bytes. The
//                          parameter value is typically the number of bytes
//                          remaining in the bit reservoir before the start
//                          of quantization for the current frame. When
//                          computing mainDataBegin, you should exclude the
//                          header and side information bytes. The side
//                          information formatter packs the 9-bit value of
//                          mainDataBegin into the main_data_begin field of
//                          the output bitstream.
//    privateBits           Depending on the number of channels, the
//                          function extracts the appropriate number of
//                          least significant bits from the parameter
//                          privateBits and packs them into the private_bits
//                          field of the output bitstream. The ISO/IEC 11172-3
//                          bitstream syntax reserves a channel-dependent
//                          number of application-specific (private) bits in
//                          the layer III bitstream audio data section
//                          immediately following the parameter
//                          main_data_begin. See ISO/IEC 11172-3:1993. For
//                          dualand single-channel streams, respectively,
//                          three and five bits are reserved.
//    pSrcScfsi             Pointer to the scale factor selection
//                          information table. This vector contains a set of
//                          binary flags that indicate whether scalefactors
//                          are shared across granules of a frame within
//                          predefined scale factor selection groups. For
//                          example, bands 0,1,2,3,4,5 form one group and
//                          bands 6,7,8,9,10 form the second group, as
//                          defined in ISO/IEC 11172-3.
//                          The vector is indexed as
//                          pDstScfsi[ch][scfsi_band], where:
//                          - ch is the channel index, 0 stands for
//                          channel 1, 1 stands for channel 2;
//                          - scfsi_band is the scale factor selection group
//                          number. Group 0 includes SFBs 0-5, group 1
//                          includes SFBs 6-10, group 2 includes SFBs 11-15,
//                          and group 3 includes SFBs 16-20.
//    pFrameHeader          Pointer to the IppMP3FrameHeader structure. Only
//                          MPEG-1 (id = 1) is supported. Upon the function
//                          entry, the  structure fields id, mode, and layer
//                          should contain, respectively, the algorithm id
//                          (MPEG-1 or MPEG-2), the mono or stereo mode, and
//                          the MPEG layer specifier. All other pFrameHeader
//                          fields are ignored.
//    ppBitStream           Pointer to the encoded bitstream buffer. The
//                          parameter is a double pointer to the first byte
//                          in the bitstream buffer intended to receive the
//                          packed side information bits generated by this
//                          function. The side information bits are
//                          sequentially written into the stream buffer
//                          starting from the byte-aligned location
//                          referenced by *ppBitStream.
//    ppBitStream           Updated bitstream byte pointer. The parameter *
//                          ppBitStream points to the first available
//                          bitstream buffer byte immediately following the
//                          packed side information bits. The frame header
//                          bits are formatted according to the bitstream
//                          syntax given in ISO/IEC 11172-3:1993. Description
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSrcSideInfo,
//                          ppBitStream, pSrcScfsi, or pFrameHeader is NULL.
//    ippStsMP3FrameHeaderErr Indicates an error when
//                          pFrameHeader->id != IPP_MP3_ID_MPEG1
//                          pFrameHeader->id != IPP_MP3_ID_MPEG2
//                          pFrameHeader->mode exceeds [0, 3].
*/
IPPAPI(IppStatus, ippsPackSideInfo_MP3, (
    IppMP3SideInfo*     pSrcSideInfo,
    Ipp8u**             ppBitStream,
    int                 mainDataBegin,
    int                 privateBits,
    int*                pSrcScfsi,
    IppMP3FrameHeader*  pFrameHeader))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsPackScalefactors_MP3_8s1u
//
//  Purpose:
//    Applies noiseless coding to scalefactors and packs output into
//    bitstream buffer.
//
//  Parameters:
//    pSrcScaleFactor       Pointer to a vector of scalefactors generated
//                          during the quantization process for one channel
//                          of one granule. Scale factor vector lengths
//                          depend on the block mode. Short block granule
//                          scale factor vectors contain 36 elements, or
//                          12 elements for each subblock. Long block
//                          granule scale factor vectors contain 21 elements.
//                          Thus short block scale factor vectors are indexed
//                          as follows: pSrcScaleFactor[sb*12+sfb], where
//                          - sb is the subblock index. 0 stands for subblock
//                          1, 1 stands for subblock 2, 2 stands for subblock
//                          3.
//                          - sfb is the scale factor band index (0-11).
//                          Long block scale factor vectors are indexed as
//                          follows:
//                          pSrcScaleFactor[sfb], where sfb is the scale
//                          factor band index (0-20).
//                          The associated side information for an individual
//                          granule/channel can be used to select the
//                          appropriate indexing scheme.
//    ppBitStream           Updated bitstream byte pointer. This parameter
//                          points to the first available bitstream buffer
//                          byte immediately following the bits generated by
//                          the scale factor Huffman encoder and
//                          sequentially written into the stream buffer. The
//                          scale factor bits are formatted according to the
//                          bitstream syntax given in ISO/IEC 11172-3.
//    pOffset               Updated bitstream bit pointer. The pOffset
//                          parameter indexes the next available bit in the
//                          next available byte referenced by the updated
//                          bitstream buffer byte pointer ppBitStream. This
//                          parameter is valid within the range of 0 to 7,
//                          where 0 corresponds to the most significant bit
//                          and 7 corresponds to the least significant bit.
//    pFrameHeader          Pointer to the IppMP3FrameHeader structure for
//                          this frame. Upon the function entry, the
//                          structure fields id and modeExt should contain,
//                          respectively, the algorithm id (MPEG-1 or MPEG-2
//                          ) and the joint stereo coding commands generated
//                          by the psychoacoustic model. All other
//                          pFrameHeader fields are ignored. Only MPEG-1 (id
//                          = 1) is supported.
//    pSideInfo             Pointer to the IppMP3SideInfo structure for the
//                          current granule and channel. Upon function entry
//                          , the structure fields blockType, mixedBlock,
//                          and sfCompress should contain, respectively, the
//                          block type indicator start, short, or stop,
//                          filter bank mixed block analysis mode specifier,
//                          and scale factor bit allocation. All other *
//                          pSideInfo fields are ignored by the scale factor
//                          encoder.
//    pScfsi                Pointer to the scale factor selection
//                          information table that contains the set of
//                          binary flags that indicate whether scalefactors
//                          are shared across granules of a frame within the
//                          predefined scale factor selection groups. For
//                          example, bands 0,1,2,3,4,5 form one group and
//                          bands 6,7,8,9,10 form a second group (as defined
//                          in ISO/IEC 11172-3). The vector is indexed as
//                          follows: pScfsi[ch][scfsi_band], where - ch is
//                          the channel index. 0 stands for channel 1, 1
//                          stands for channel 2. - scfsi_band is the scale
//                          factor selection group number. Group 0 includes
//                          SFBs 0-5, group 1 includes SFBs 6-10, group 2
//                          includes SFBs 11-15, and group 3 includes SFBs
//                          16-20.
//    granule               Index of the current granule. 0 stands for
//                          granule 1, 1 stands for granule 2.
//    channel               Index of the current channel. 0 stands for
//                          channel 1, 1 stands for channel 2.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsMP3SideInfoErr  Indicates an error when
//                          pFrameHeader->id == IPP_MP3_ID_MPEG1 and
//                          pSideInfo->sfCompress exceeds [0..15];
//                          pFrameHeader->id == IPP_MP3_ID_MPEG2 and
//                          pSideInfo->sfCompress exceeds [0..511].
//    ippStsMP3FrameHeaderErr Indicates an error when pFrameHeader->id ==
//                          IPP_MP3_ID_MPEG2 and pFrameHeader->modeExt
//                          exceeds [0..3].
*/
IPPAPI(IppStatus, ippsPackScaleFactors_MP3_8s1u, (
    const Ipp8s*        pSrcScaleFactor,
    Ipp8u**             ppBitStream,
    int*                pOffset,
    IppMP3FrameHeader*  pFrameHeader,
    IppMP3SideInfo*     pSideInfo,
    int*                pScfsi,
    int                 granule,
    int                 channel))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsHuffmanEncode_MP3_32s1u
//
//  Purpose:
//    Applies lossless Huffman encoding to quantized samples and packs
//    output into bitstream buffer.
//
//  Parameters:
//    pSrcIx                Pointer to the quantized samples of a granule.
//                          The buffer length is 576. Depending on which
//                          type of joint coding has been applied, if any,
//                          the coefficient vector might be associated with
//                          either the L, R, M, S, and/or intensity channel
//                          of the quantized spectral data.
//    ppBitStream           Updated bitstream byte pointer. The parameter *
//                          ppBitStream points to the first available
//                          bitstream buffer byte immediately following the
//                          bits generated by the spectral coefficient
//                          Huffman encoder and sequentially written into
//                          the stream buffer. The Huffman symbol bits are
//                          formatted according to the bitstream syntax
//                          given in ISO/IEC 11172-3.
//    pOffset               Updated bitstream bit pointer. The pOffset
//                          parameter indexes the next available bit in the
//                          next available byte referenced by the updated
//                          bitstream buffer byte pointer ppBitStream. This
//                          parameter is valid within the range of 0 to 7,
//                          where 0 corresponds to the most significant bit
//                          and 7 corresponds to the least significant bit.
//    pFrameHeader          Pointer to the IppMP3FrameHeader structure for
//                          this frame. The Huffman encoder uses the frame
//                          header id field in connection with the side
//                          information (as described below) to compute the
//                          Huffman table region boundaries for the big
//                          value spectral region. The Huffman encoder
//                          ignores all other frame header fields.Only
//                          MPEG-1 (id = 1) is supported.
//    pSideInfo             Pointer to the IppMP3SideInfo structure for the
//                          current granule and channel. The structure
//                          elements bigVals, pTableSelect[0]-[2], reg0Cnt,
//                          and reg1Cnt are used to control coding of
//                          spectral coefficients in the big value region.
//                          The structure element cnt1TabSel is used to
//                          select the appropriate Huffman table for the (-1,
//                          0,+1)-valued 4-tuples in the count1 region. For
//                          detailed descriptions of all side information
//                          elements, see the structure definition header
//                          file.
//    count1Len             The count1 region length specifier. Indicates
//                          the number of spectral samples for the current
//                          granule/channel above the big value region that
//                          can be combined into 4-tuples in which all
//                          elements are of magnitude less than or equal to 1.
//    hufSize               Huffman coding bit allocation specifier.
//                          Indicates the total number of bits that are
//                          required to represent the Huffman-encoded
//                          quantized spectral coefficients for the current
//                          granule/channel in both the bigvals and count1
//                          regions. Whenever necessary, this bit count
//                          should be augmented to include the number of
//                          bits required to manage the bit reservoir. For
//                          frames in which the reservoir has reached
//                          maximum capacity, the surplus bits are expended
//                          by padding with additional bits the Huffman
//                          representation of the spectral samples. The
//                          HufSize result returned by the function
//                          Quantize_MP3_32s_I reflects these padding
//                          requirements. That is, HufSize[i] is equal to
//                          the total of the number of bits required for
//                          Huffman symbols and the number of padding bits.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the specified
//                          pointers is NULL.
//    ippStsBadArgErr       Indicates an error when pOffset exceeds [0,7].
//    ippStsMP3SideInfoErr  Indicates an error when pSideInfo->bigVals*2>
//                          IPP_MP3_GRANULE_LEN (pSideInfo->reg0Cnt +
//                          pSideInfo->reg1Cnt + 2) >= 23, pSideInfo->
//                          cnt1TabSel exceeds [0,1], pSideInfo->pTableSelect
//                          [i] exceeds [0..31].
//    ippStsMP3FrameHeader  Indicates an error when
//                          pFrameHeader->id != IPP_MP3_ID_MPEG1
//                          pFrameHeader->layer != 1
//                          pFrameHeader->samplingFreq exceeds [0..2].
*/
IPPAPI(IppStatus, ippsHuffmanEncode_MP3_32s1u, (
    Ipp32s *pSrcIx,
    Ipp8u **ppBitStream,
    int *pOffset,
    IppMP3FrameHeader *pFrameHeader,
    IppMP3SideInfo *pSideInfo,
    int count1Len,
    int hufSize))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsBitReservoirInit_MP3
//
//  Purpose:
//    Initializes all elements of the bit reservoir state structure.
//
//  Parameters:
//    pFrameHeader          Pointer to the IppMP3FrameHeader structure that
//                          contains the header information associated with
//                          the current frame. The frame header fields
//                          bitRate and id, bit rate index and algorithm
//                          identification, respectively, must contain valid
//                          data prior to calling the function
//                          BitReservoirInit_MP3 since both are used to
//                          generate the bit reservoir initialization
//                          parameters. All other frame header parameters
//                          are ignored by the bit reservoir initialization
//                          function. Only MPEG-1 (id = 1) is supported.
//    pDstBitResv           Pointer to the initialized IppMP3BitReservoir
//                          state structure. The structure element
//                          BitsRemaining is initialized as 0. The structure
//                          element MaxBits is initialized to reflect the
//                          maximum number of bits that can be contained in
//                          the reservoir at the start of any given frame.
//                          The appropriate value of MaxBits is directly
//                          determined by the selected algorithm (MPEG-1 or
//                          MPEG-2) and the stream bit rate indicated by the
//                          rate index parameter pFrameHeader.bitRate.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pDstBitResv or
//                          pFrameHeader is NULL.
//    ippStsMP3FrameHeaderErr Indicates an error when
//                          pFrameHeader->id != IPP_MP3_ID_MPEG1.
*/
IPPAPI(IppStatus, ippsBitReservoirInit_MP3, (
    IppMP3BitReservoir* pDstBitResv,
    IppMP3FrameHeader*  pFrameHeader))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTFwd_MP3_32s
//
//  Purpose:
//    Implements stage 2 of the MP3 hybrid analysis filterbank.
//
//  Parameters:
//    pSrc                  Pointer to the start of the 576-element block
//                          PQMF analysis output vector containing 18
//                          consecutive blocks of 32 subband samples that
//                          are indexed as follows:
//                          pDstS[32*i+sb], where i = 0,1,...,17 is time series
//                          index, sb = 0,1,...,31 is the subband index. All
//                          coefficients are represented using the Q7.24
//                          format.
//    pDst                  Pointer to the 576-element spectral coefficient
//                          output vector generated by the analysis
//                          filterbank.
//    blockType             Block type indicator: 0 stands for normal block;
//                          1 stands for start block; 2 stands for short
//                          block; 3 stands for stop block.
//    mixedBlock            Mixed block indicator: 0 stands for not mixed; 1
//                          stands for mixed.
//    pFrameHeader          Pointer to the IppMP3FrameHeader structure that
//                          contains the header associated with the current
//                          frame. Only MPEG-1 (id = 1) is supported.
//    pOverlapBuf           Pointer to the MDCT overlap buffer that contains
//                          a copy of the most recent 576-element block of
//                          PQMF bank outputs. Prior to processing a new
//                          audio stream with the analysis filterbank, all
//                          elements of the buffer pOverlapBuf should be
//                          initialized to the constant value 0.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the pointers
//                          pSrcXs, pDstXr, pFrameHeader, or pOverlapBuf is
//                          NULL.
*/
IPPAPI(IppStatus, ippsMDCTFwd_MP3_32s, (
    const Ipp32s*       pSrc,
    Ipp32s*             pDst,
    int                 blockType,
    int                 mixedBlock,
    IppMP3FrameHeader*  pFrameHeader,
    Ipp32s*             pOverlapBuf))



/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsMDCTInvWindow_MP3_32s
//
//  Purpose:
//    Performs 36 or 18 points IMDCT and windowing depending on blockType.
//  Parameters:
//    pSrc                  pointer to the input buffer which is the
//                          output of alias reduction.
//
//    pDst                  pointer to the output buffer of IMDCT for
//                          long block (36 points) and short block (18 points).
//
//    blockType             Block type indicator: 0 stands for normal window,
//                                                1 stands for start block,
//                                                2 stands for short block,
//                                                3 stands for stop block.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when one of the pointers
//                          pSrc or pDst is NULL.
*/
IPPAPI(IppStatus, ippsMDCTInvWindow_MP3_32s,(
    Ipp32s* pSrc,
    Ipp32s* pDst,
    int     blockType))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisPQMF_MP3_16s32s
//
//  Purpose:
//    Implements stage 1 of MP3 hybrid analysis filterbank.
//
//  Parameters:
//    pSrcPcm               Pointer to the start of the buffer containing
//                          the input PCM audio vector. The samples conform
//                          to the following guidelines: - must be in 16-bit,
//                          signed, little-endian, Q15 format; - most
//                          recent 480 (512-32) samples should be contained
//                          in the vector pSrcPcm[pcmMode*i], where i = 0,1
//                          ,..,479; - samples associated with the current
//                          granule should be contained in the vector pSrcPcm
//                          [pcmMode*j], where j = 480,481,..,1055.
//    pcmMode               PCM mode flag. Communicates to PQMF filterbank
//                          the type of input PCM vector organization to
//                          expect: - pcmMode = 1 denotes non-interleaved
//                          PCM input samples; - pcmMode = 2 denotes
//                          interleaved PCM input samples.
//    pDstS                 Pointer to the start of the 576-element block
//                          PQMF analysis output vector containing 18
//                          consecutive blocks of 32 subband samples under
//                          the following index: pDstXs[32*i + sb], where i
//                          = 0,1,...,17 is time series index sb = 0,1,...,31 is
//                          the subband index.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSrcPcm or pDstXs is NULL.
//    ippStsErr             Indicates an error when pcmMode exceeds [1,2].
*/
IPPAPI(IppStatus, ippsAnalysisPQMF_MP3_16s32s, (
    const Ipp16s*       pSrcPcm,
    Ipp32s*             pDstS,
    int                 mode))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsJointStereoEncode_MP3_32s_I
//
//  Purpose:
//    Transforms independent left and right channel spectral coefficient
//    vectors into combined mid/side and/or intensity mode coefficient
//    vectors suitable for quantization.
//
//  Parameters:
//    pSrcDstXrL            Pointer to the 576-element joint stereo spectral
//                          coefficient output vector associated with the M
//                          channel, as well as the intensity coded
//                          coefficients above the intensity lower SFB bound.
//                          All elements are represented using the Q5.26
//                          format.
//    pSrcDstXrR            Pointer to the 576-element joint stereo spectral
//                          coefficient output vector associated with the S
//                          channel. All elements are represented using the
//                          Q5.26 format.
//    pDstScaleFactorR      Pointer to the vector of scalefactors associated
//                          with one granule of the right/S channel. If
//                          intensity coding has been enabled by the
//                          psychoacoustic model above a certain SFB lower
//                          bound, as indicated by the frame header and the
//                          vector pointed to by pIsSfbBound, the function
//                          StereoEncode_MP3_32s_I updates with the
//                          appropriate scalefactors those elements of
//                          pDstScaleFactorR[] that are associated with
//                          intensity coded scale factor bands. Other SFB
//                          entries in the scale factor vector remain
//                          unmodified. The length of the vector referenced
//                          by pDstScaleFactorR varies as a function of
//                          block size. The vector contains 21 elements for
//                          long block granules, or 36 elements for short
//                          block granules.
//    pFrameHeader          Pointer to the IppMP3FrameHeader structure that
//                          contains the header information associated with
//                          the current frame. Upon function entry, the
//                          structure fields samplingFreq, id, mode, and
//                          modeExt should contain, respectively, the sample
//                          rate associated with the current input audio,
//                          the algorithm id (MPEG-1 or MPEG-2), and the
//                          joint stereo coding commands generated by the
//                          psychoacoustic model. All other pFrameHeader
//                          fields are ignored. Only MPEG-1 (id = 1) is
//                          supported.
//    pSideInfo             Pointer to the pair of IppMP3SideInfo structures
//                          associated with the channel pair to be jointly
//                          encoded. The number of elements in the set is 2,
//                          and ordering of the set elements is as follows:
//                          pSideInfo[0] describes channel 1, and pSideInfo[1
//                          ] describes channel 2. Upon the function entry,
//                          the blockType side information fields for both
//                          channels should reflect the analysis modes (
//                          short or long block) selected by the
//                          psychoacoustic model on each channel. All other
//                          fields in the pSideInfo[0] and pSideInfo[1]
//                          structures are ignored.
//    pIsSfbBound           Pointer to the list of intensity coding SFB
//                          lower bounds for both channels of the current
//                          granule above which all L/R channel spectral
//                          coefficients are combined into an intensity-
//                          coded representation. The number of elements
//                          depends on the block type associated with the
//                          current granule. For short blocks, the SFB
//                          bounds are represented in the following order:
//                          pIsSfbBound[0] describes block 1, pIsSfbBound[1]
//                          describes block 2, and pIsSfbBound[2] describes
//                          block 3. For long blocks, only a single SFB
//                          lower bound decision is required and is
//                          represented in pIsSfbBound[0]. If both MS and
//                          intensity coding are enabled, the SFB intensity
//                          coding lower bound simultaneously represents the
//                          upper bound SFB for MS coding. If only MS coding
//                          is enabled, the SFB bound represents the lowest
//                          non-MS SFB.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers pSrcDstXrL, pSrcDstXrR, pDstScaleFactorR,
//                          pFrameHeader, pSideInfo, or pIsSfbBound is NULL.
//    ippStsMP3SideInfoErr  Indicates an error if pSideInfo[0].blockType!=
//                          pSideInfo[1].blockType when IS or MS is used.
*/
IPPAPI(IppStatus, ippsJointStereoEncode_MP3_32s_I, (
    Ipp32s*             pSrcDstXrL,
    Ipp32s*             pSrcDstXrR,
    Ipp8s*              pDstScaleFactorR,
    IppMP3FrameHeader*  pFrameHeader,
    IppMP3SideInfo*     pSideInfo,
    int*                pIsSfbBound))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsQuantize_MP3_32s_I
//
//  Purpose:
//    Quantizes spectral coefficients generated by analysis filterbank.
//
//  Parameters:
//    pSrcDstXrIx           Pointer to the output set of quantized spectral
//                          coefficient vectors. These are suitable for
//                          input to the Huffman encoder. The coefficients
//                          are indexed as follows: pSrcDstXrIx[gr*1152 + ch*
//                          576 + i] for stereophonic and dual-mono input
//                          sources, and pSrcDstXrIx[gr*576 + i] for single
//                          channel input sources, where: - i=0,1,...,575 is
//                          the spectral coefficient index; - gr is the
//                          granule index. 0 stands for granule 1, 1 stands
//                          for granule 2; - ch is the channel index. 0
//                          stands for channel 1, 1 stands for channel 2.
//    pDstScaleFactor       Pointer to the output set of
//                          scalefactors generated during the quantization
//                          process. These scalefactors determine the
//                          quantizer granularity. Scale factor vector
//                          lengths depend on the block mode associated with
//                          each granule. The order of the elements is: 1. (
//                          granule 1, channel 1); 2. (granule 1, channel 2);
//                          3. (granule 2, channel 1); 4. (granule 2,
//                          channel 2). Given this general organization, the
//                          side information for each granule/channel in
//                          conjunction with the flags contained in the vector
//                          pDstScfsi can be used to determine the precise
//                          scale factor vector indices and lengths.
//    pDstScfsi             Pointer to the output vector of scale factor
//                          selection information. This vector contains a
//                          set of binary flags that indicate whether or not
//                          scalefactors are shared across granules of a
//                          frame within predefined scale factor selection
//                          groups. For example, bands 0,1,2,3,4,5 form one
//                          group; bands 6,7,8,9,10 form a second group, as
//                          defined in ISO/IEC 11172-3. The vector is
//                          indexed as follows: pDstScfsi[ch][scfsi_band],
//                          where: - ch is the channel index. 0 stands for
//                          channel 1, 1 stands for channel 2; - scfsi_band
//                          is the scale factor selection group number.
//                          Group 0 includes SFBs 0-5, group 1 includes SFBs
//                          6-10, group 2 includes SFBs 11-15, and group 3
//                          includes SFBs 16-20.
//    pDstCount1Len         Pointer to an output vector of count1 region
//                          length specifiers. For the purposes of Huffman
//                          coding spectral coefficients of a higher
//                          frequency than the bigvals region, the count1
//                          parameter indicates the size of the region in
//                          which spectral samples can be combined into
//                          quadruples for which all elements are of
//                          magnitude less than or equal to 1. The vector
//                          contains 2*nchan, elements and is indexed as
//                          follows: pDstCount1Len[gr*nchan + ch], where: -
//                          gr is the granule index. 0 stands for granule 1,
//                          1 stands for granule 2; - nchan is the number of
//                          channels; - ch is the channel index. 0 stands
//                          for channel 1, 1 stands for channel 2.
//    pDstHufSize           Pointer to an output vector of Huffman coding
//                          bit allocation specifiers. For each granule/
//                          channel, the specifiers indicate the total
//                          number of Huffman bits required to represent the
//                          quantized spectral coefficients in the bigvals
//                          and count1 regions. Whenever necessary, each
//                          HufSize bit count is augmented to include the
//                          number of bits required to manage the bit
//                          reservoir. For frames in which the reservoir has
//                          reached the maximum capacity, the quantizer
//                          expends the surplus bits by padding with
//                          additional bits the Huffman representation of
//                          the spectral samples. The HufSize result
//                          returned by the quantizer reflects these padding
//                          requirements. That is, HufSize[i] is equal to
//                          the sum of the number of bits required for
//                          Huffman symbols and the number of padding bits.
//                          The vector contains 2*nchan, elements and is
//                          indexed as follows: pDstHufSize[gr*nchan+ch],
//                          where: - gr is the granule index. 0 stands for
//                          granule 1, 1 stands for granule 2; - nchan is
//                          the number of channels; - ch is the channel index.
//                          0 stands for channel 1, 1 stands for channel 2.
//    pFrameHeader          Pointer to the IppMP3FrameHeader structure that
//                          contains the header information associated with
//                          the current frame. Upon the function entry, the
//                          structure fields samplingFreq, id, mode, and
//                          modeExt should contain, respectively, the sample
//                          rate associated with the current input audio,
//                          the algorithm id, that is, MPEG-1 or MPEG-2, and
//                          the joint stereo coding commands generated by
//                          the psychoacoustic model. All other *
//                          pFrameHeader fields are ignored. Only MPEG-1 (id
//                          = 1) is supported.
//    pSideInfo             Pointer to the set of IppMP3SideInfo structures
//                          associated with all granules and channels. The
//                          set should contain 2*nchan, elements and should
//                          be indexed as follows: pSideInfo[gr*nchan + ch],
//                          where: - gr is the granule index. 0 stands for
//                          granule 1, 1 stands for granule 2; - nchan is
//                          the number of channels; - ch is the channel index.
//                          0 stands for channel 1, 1 stands for channel 2.
//                          Upon the function entry, in all set elements
//                          the structure fields blockType, mixedBlock, and
//                          winSwitch should contain, respectively, the
//                          block type indicator (start, short, or stop),
//                          filter bank mixed block analysis mode specifier,
//                          and window switching flags (normal
//                          or blockType) associated with the current
//                          input audio. All other *pSideInfo fields are
//                          ignored upon the function entry and updated upon
//                          function exit, as described below under the
//                          description of output arguments.
//    pPsychoInfo           Pointer to the first element in a set of
//                          PsychoacousticModelTwoAnalysis structures
//                          associated with the current frame. Each set
//                          member contains the MSR and PE estimates for one
//                          channel of one granule. The set should contain 2*
//                          nchan, elements and is indexed as: pPsychoaInfo[
//                          gr*nchan+ ch], where: - gr is the granule index.
//                          0 stands for granule 1, 1 stands for granule 2;
//                          - nchan is the number of channels; - ch is the
//                          channel index. 0 stands for channel 1, 1 stands
//                          for channel 2.
//    pFramePsyState        Pointer to the first element in a set of
//                          IppMP3PsychoacousticModelTwoState structures
//                          that contains the psychoacoustic model state
//                          information associated with both the current
//                          frame and next frame. The number of elements in
//                          the set is equal to the number of channels
//                          contained in the input audio. That is, a
//                          separate analysis is carried for each channel.
//                          The quantizer uses the frame type look ahead
//                          information nextBlockType to manage the bit
//                          reservoir. All other structure elements are
//                          ignored by the quantizer.
//    pResv                 Pointer to the updated IppMP3BitReservoir
//                          structure. The quantizer updates the
//                          BitsRemaining field to add or remove bits as
//                          necessary. All other fields are unmodified by
//                          the quantizer.
//    meanBits              The number of bits allocated on an average basis
//                          for each frame of spectral coefficients and
//                          scalefactors given the target bit rate (kilobits
//                          per second) specified in the frame header. This
//                          number excludes the bits allocated for the frame
//                          header and side information. The quantizer uses
//                          meanBits as a target allocation for the current
//                          frame. Given perceptual bit allocation
//                          requirements greater than this target, the
//                          quantizer makes use of the surplus bits held in
//                          the bit reservoir to satisfy frame-instantaneous
//                          demands. Similarly, given perceptual bit
//                          allocation requirements below this target, the
//                          quantizer will store surplus bits in the bit
//                          reservoir for use by future frames.
//    pIsSfbBound           Pointer to the list of SFB lower bounds above
//                          which all L/R channel spectral coefficients have
//                          been combined into an intensity-coded
//                          representation. The number of valid elements
//                          pointed to by pIsSfbBound
//                          depends upon the block types associated with the
//                          granules of the current frame. In particular,
//                          the list of SFB bounds pointed to by pIsSfbBound
//                          is indexed as follows: pIsSfbBound[3*gr] for
//                          long block granules, and pIsSfbBound[3*gr + w]
//                          for short block granules, where: - gr is the
//                          granule index. 0 stands for granule 1, 1 stands
//                          for granule 2; - w is the block index. 0 stands
//                          for block 1, 1 stands for block 2, 2 stands for
//                          block 3. For example, given short-block analysis
//                          in granule 1 followed by long block analysis in
//                          granule 2, the list of SFB bounds would be
//                          expected in the following order: pIsSfbBound[] =
//                          {granule 1/block 1, granule 1/block 2, granule 1/
//                          block 2, granule 2/long block}. If a granule is
//                          configured for long block analysis, then only a
//                          single SFB lower bound decision is expected,
//                          whereas three are expected for short block
//                          granules. If both MS and intensity coding have
//                          been enabled, then the SFB intensity coding
//                          lower bound simultaneously represents the upper
//                          bound SFB for MS coding. If only MS coding has
//                          been enabled, then the SFB bound represents the
//                          lowest non-MS SFB.
//    pWorkBuffer           Pointer to the workspace buffer internally used
//                          by the quantizer for storage of intermediate
//                          results and other temporary data. The buffer
//                          length should be at least 2880 bytes, that is,
//                          720 words of 32-bits each.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSrcDstXrIx,
//                          pDstScaleFactor, pDstScfsi, pDstCount1Len,
//                          pDstHufSize, pFrameHeader, pSideInfo, pPsychInfo,
//                          pFramePsyState, pResv, pIsSfbBound, or
//                          pWorkBuffer is NULL.
//    ippStsMP3SideInfo     Indicates an error when pSideInfo->winSwitch and
//                          pSideInfo->mixedBlock are both defined.
//    ippStsMP3FrameHeaderErr  Indicates an error when pFrameHeader->samplingFreq or
//                          pFrameHeader->id is not correct.
*/
IPPAPI(IppStatus, ippsQuantize_MP3_32s_I, (
    Ipp32s*             pSrcDstXrIx,
    Ipp8s*              pDstScaleFactor,
    int*                pDstScfsi,
    int*                pDstCount1Len,
    int*                pDstHufSize,
    IppMP3FrameHeader*  pFrameHeader,
    IppMP3SideInfo*     pSideInfo,
    IppMP3PsychoacousticModelTwoAnalysis*   pPsychInfo,
    IppMP3PsychoacousticModelTwoState*      pFramePsyState,
    IppMP3BitReservoir* pResv,
    int                 meanBits,
    int*                pIsSfbBound,
    Ipp32s*             pWorkBuffer))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsPsychoacousticModelTwo_MP3_16s
//
//  Purpose:
//    Implements ISO/IEC 11172-3 psychoacoustic model recommendation 2 to
//    estimate masked threshold and perceptual entropy associated with a
//    block of PCM audio input.
//
//  Parameters:
//    pSrcPcm               Pointer to the start of the buffer containing
//                          the input PCM audio vector, the samples of which
//                          should conform to the following format
//                          specification: - 16-bits per sample, signed,
//                          little-endian, Q15; - pSrcPcm buffer should
//                          contain 1152 samples, that is, two granules of
//                          576 samples each, if the parameter pFrameHeader
//                          -> mode has the value 1 (mono), or 2304 samples,
//                          that is, two granules of 576 samples each, if
//                          the parameter pFrameHeader -> mode has the value
//                          of 2 (stereo, dual mono); - In the stereophonic
//                          case, the PCM samples associated with the left
//                          and right channels should be organized according
//                          to the pcmMode flag. Failure to satisfy any of
//                          the above PCM format and/or buffer requirements
//                          results in undefined model outputs.
//    pDstPsyInfo           Pointer to the first element in a
//                          set of PsychoacousticModelTwoAnalysis structures.
//                          Each set member contains the MSR and PE
//                          estimates for one granule. The number of
//                          elements in the set is equal to the number of
//                          channels, with the outputs arranged as follows: (
//                          Analysis[0] = granule 1, channel 1), (...Analysis[1]
//                          = granule 1, channel 2), (...Analysis[2] =
//                          granule 2, channel 1), (...Analysis[3] = granule 2,
//                          channel 2).
//    pDstIsSfbBound        If intensity coding has been enabled,
//                          pDstIsSfbBound points to the list of SFB lower
//                          bounds above which all spectral coefficients
//                          should be processed by the joint stereo
//                          intensity coding module. Since the intensity
//                          coding SFB lower bound is block-specific, the
//                          number of valid elements pointed to by
//                          pDstIsSfbBound varies depending upon the
//                          individual block types associated with each
//                          granule. In particular, the list of SFB bounds
//                          is indexed as follows: - pIsSfbBound[3*gr] for
//                          long block granules; - pIsSfbBound[3*gr + w] for
//                          short block granules, where gr is the granule
//                          index (0 indicates granule 1 and 1 indicates
//                          granule 2), and w is the block index (0
//                          indicates block 1, 1 indicates block 2, 2
//                          indicates block 3). For example, given short-
//                          block analysis in granule 1 followed by long
//                          block analysis in granule 2, the list of SFB:
//                          bounds would be generated in the following order
//                          pIsSfbBound[] = {granule 1/block 1, granule 1/
//                          block 2, granule 1/block 2, granule 2/long
//                          block}. Only one SFB lower bound decision is
//                          generated for long block granules, whereas three
//                          are generated for short block granules. If both
//                          MS and intensity coding are enabled, then the
//                          SFB intensity coding lower bound simultaneously
//                          represents the upper bound SFB for MS coding. If
//                          only MS coding has been enabled, then the SFB
//                          bound represents the lowest non-MS SFB.
//    pDstSideInfo          Pointer to the updated set of IppMP3SideInfo
//                          structures associated with all granules and
//                          channels. The model updates the following fields
//                          in all set elements: blockType, winSwitch, and
//                          mixedBlock. The number of elements in the set is
//                          equal to 2 times the number of channels.
//                          Ordering of the set elements is the same as
//                          pDstPsyInfo.
//    pFrameHeader          Pointer to the IppMP3FrameHeader structure that
//                          contains the header associated with the current
//                          frame. The samplingFreq, id, and mode fields of
//                          the structure pFrameHeader control the behavior
//                          of the psychoacoustic model. All three fields
//                          must be appropriately initialized prior to
//                          calling this function. All other frame header
//                          fields are ignored. Only MPEG-1 (id = 1) is
//                          supported.
//    pFramePsyState        Pointer to the first element in a set of
//                          IppMP3PsychoacousticModelTwoState structures
//                          that contains the updated psychoacoustic model
//                          state information associated with both the
//                          current frame and next frame. The number of
//                          elements in the set is equal to the number of
//                          channels contained in the input audio. That is,
//                          a separate analysis is carried for each channel.
//                          Prior to encoding a new audio stream, all
//                          elements of the psychoacoustic model state
//                          structure pPsychoacousticModelState should be
//                          initialized to contain the value 0. In the
//                          signal processing domain, this could be
//                          accomplished using the function ippsZero_16s as
//                          follows: ippsZero_16s ((Ipp16s *)
//                          pPsychoacousticModelState,sizeof(
//                          IppMP3PsychoacousticModelTwoState)
//                          /sizeof(Ipp16s)).
//    pcmMode               PCM mode flag. Communicates the psychoacoustic
//                          model which type of PCM vector organization to
//                          expect: - pcmMode = 1 denotes non-interleaved
//                          PCM input samples, that is, pSrcPcm[0..1151]
//                          contains the input samples associated with the
//                          left channel, and pSrcPcm[1152..2303] contains
//                          the input samples associated with the right
//                          channel; - pcmMode = 2 denotes interleaved PCM
//                          input samples, that is, pSrcPcm[2*i] and pSrcPcm[
//                          2*i+1] contain the samples associated with the
//                          left and right channels, respectively, where i =
//                          0,1,...,1151. You can also use appropriately
//                          typecast elements ippMP3NonInterleavedPCM and
//                          ippMP3InterleavedPCM of the enumerated type
//                          IppMP3PcmMode as an alternative to the constants
//                          1 and 2 for pcmMode.
//    pWorkBuffer           Pointer to the workspace buffer internally used
//                          by the psychoacoustic model for storage of
//                          intermediate results and other temporary data.
//                          The buffer length must be at least 25,200 bytes,
//                          that is, 6300 elements of type Ipp32s.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when at least one of the
//                          pointers pSrcPcm, pDstPsyInfo, pDstSideInfo,
//                          pDstIsSfbBound, pFrameHeader, pDstPsyState, or
//                          pWorkBuffer is NULL.
*/
IPPAPI(IppStatus, ippsPsychoacousticModelTwo_MP3_16s, (
    const Ipp16s*       pSrcPcm,
    IppMP3PsychoacousticModelTwoAnalysis*   pDstPsyInfo,
    int*                pDstIsSfbBound,
    IppMP3SideInfo*     pDstSideInfo,
    IppMP3FrameHeader*  pFrameHeader,
    IppMP3PsychoacousticModelTwoState*      pFramePsyState,
    int                 pcmMode,
    Ipp32s*             pWorkBuffer))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilter_PQMF_MP3_32f
//
//  Purpose:
//    Transforms PQMF MP3-processed subband signals into time domain samples.
//
//  Parameters:
//    pSrc                  Array of pointers, holds PQMF MP3-processed subband
//                          signals.
//    pDst                  Pointer to the output vector, holds time domain
//                          output samples.
//    pFilterSpec           Pointer to the IppsFilterSpec_PQMF_MP3 specification
//                          structure.
//    mode                  Flag that indicates whether or not MP3 audio
//                          output channels should be interleaved.
//                            1 - indicates not interleaved,
//                            2 - indicates interleaved.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
//    ippStsBadArgErr       Indicates an error when mode exceeds [1, 2].
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI(IppStatus, ippsSynthesisFilter_PQMF_MP3_32f, (const  Ipp32f  *pSrc,
                                                            Ipp32f  *pDst,
                                                     const  IppsFilterSpec_PQMF_MP3 *pFilterSpec,
                                                            int     mode))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterGetSize_PQMF_MP3_32f
//
//  Purpose:
//    Returns size of IppsFilterSpec_PQMF_MP3 specification structure.
//
//  Parameters:
//    pSizeSpec             Address of the IppsFilterSpec_PQMF_MP3 specification
//                          structure size value in bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSizeSpec is NULL.
*/
IPPAPI(IppStatus, ippsSynthesisFilterGetSize_PQMF_MP3_32f, (int *pSizeSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterInit_PQMF_MP3_32f
//
//  Purpose:
//    Initializes synthesis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the IppsFilterSpec_PQMF_MP3
//                          specification structure is written. Only the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the IppsFilterSpec_PQMF_MP3
//                          specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
*/
IPPAPI(IppStatus, ippsSynthesisFilterInit_PQMF_MP3_32f, (IppsFilterSpec_PQMF_MP3 **ppFilterSpec,
                                                         Ipp8u                   *pMemSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterInitAlloc_PQMF_MP3_32f
//
//  Purpose:
//    Allocate memory for synthesis specification structure and initializes it.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the IppsFilterSpec_PQMF_MP3
//                          specification structure is written.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when ppFilterSpec is NULL.
*/
IPPAPI(IppStatus, ippsSynthesisFilterInitAlloc_PQMF_MP3_32f, (IppsFilterSpec_PQMF_MP3 **ppFilterSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterFree_PQMF_MP3_32f
//
//  Purpose:
//    Frees memory allocated for internal IppsFilterSpec_PQMF_MP3 structure.
//
//  Parameters:
//    pFilterSpec           Pointer to the IppsFilterSpec_PQMF_MP3 specification
//                          structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pFilterSpec is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI(IppStatus, ippsSynthesisFilterFree_PQMF_MP3_32f, (IppsFilterSpec_PQMF_MP3 *pFilterSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilter_PQMF_MP3_32f
//
//  Purpose:
//    Transforms time domain samples into PQMF MP3-processed subband signals .
//
//  Parameters:
//    pSrc                  Array of pointers, holds PQMF MP3-processed subband
//                          signals.
//    pDst                  Pointer to the output vector, holds time domain
//                          output samples.
//    pFilterSpec           Pointer to the IppsFilterSpec_PQMF_MP3 specification
//                          structure.
//    mode                  Flag that indicates whether or not MP3 audio
//                          output channels should be interleaved.
//                            1 - indicates not interleaved,
//                            2 - indicates interleaved.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
//    ippStsBadArgErr       Indicates an error when mode exceeds [1, 2].
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI(IppStatus, ippsAnalysisFilter_PQMF_MP3_32f, (const  Ipp32f  *pSrc,
                                                            Ipp32f  *pDst,
                                                     const  IppsFilterSpec_PQMF_MP3 *pFilterSpec,
                                                            int     mode))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterGetSize_PQMF_MP3_32f
//
//  Purpose:
//    Returns size of IppsFilterSpec_PQMF_MP3 specification structure.
//
//  Parameters:
//    pSizeSpec             Address of the IppsFilterSpec_PQMF_MP3 specification
//                          structure size value in bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSizeSpec is NULL.
*/
IPPAPI(IppStatus, ippsAnalysisFilterGetSize_PQMF_MP3_32f, (int *pSizeSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterInit_PQMF_MP3_32f
//
//  Purpose:
//    Initializes analysis specification structure.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the IppsFilterSpec_PQMF_MP3
//                          specification structure is written. Only the
//                          main function uses this structure.
//    pMemSpec              Pointer to the area for the IppsFilterSpec_PQMF_MP3
//                          specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
*/
IPPAPI(IppStatus, ippsAnalysisFilterInit_PQMF_MP3_32f, (IppsFilterSpec_PQMF_MP3 **ppFilterSpec,
                                                         Ipp8u                   *pMemSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterInitAlloc_PQMF_MP3_32f
//
//  Purpose:
//    Allocate memory for analysis specification structure and initializes it.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the IppsFilterSpec_PQMF_MP3
//                          specification structure is written.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when ppFilterSpec is NULL.
*/
IPPAPI(IppStatus, ippsAnalysisFilterInitAlloc_PQMF_MP3_32f, (IppsFilterSpec_PQMF_MP3 **ppFilterSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsAnalysisFilterFree_PQMF_MP3_32f
//
//  Purpose:
//    Frees memory allocated for internal IppsFilterSpec_PQMF_MP3 structure.
//
//  Parameters:
//    pFilterSpec           Pointer to the IppsFilterSpec_PQMF_MP3 specification
//                          structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pFilterSpec is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI(IppStatus, ippsAnalysisFilterFree_PQMF_MP3_32f, (IppsFilterSpec_PQMF_MP3 *pFilterSpec))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilter_DTS_32f
//
//  Purpose:
//    Transforms QMF DTS processed subband signals into time domain samples.
//
//  Parameters:
//    pSrc                  Array of pointers, holds QMF DTS processed subband
//                          signals.
//    pDst                  Pointer to the output vector, holds time domain
//                          output samples.
//    pFilterSpec           Pointer to the IppsFilterSpec_DTS_32f specification
//                          structure.
//    filtType              Flag, indicates filter bank.
//                            IPPAC_DTS_PERFECT - perfect reconstruction
//                            IPPAC_DTS_NONPERFECT - nonperfect reconstruction.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
//    ippStsBadArgErr       Indicates an error when filtType is neither
//                          IPPAC_DTS_PERFECT nor IPPAC_DTS_NONPERFECT.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI(IppStatus, ippsSynthesisFilter_DTS_32f, (const  Ipp32f  *pSrc,
                                                Ipp32f  *pDst,
                                                Ipp32s filt_type,
                                                const  IppsFilterSpec_DTS_32f *pFilterSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterGetSize_DTS_32f
//
//  Purpose:
//    Returns size of IppsFilterSpec_DTS_32f specification structure.
//
//  Parameters:
//    pSizeSpec             Address of the IppsFilterSpec_DTS_32f specification
//                          structure size value in bytes.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pSizeSpec is NULL.
*/
IPPAPI(IppStatus, ippsSynthesisFilterGetSize_DTS_32f, (int *pSizeSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterInit_DTS_32f
//
//  Purpose:
//    Initializes DTS synthesis specification structure.
//
//  Parameters:
//    pFilterSpec          Pointer to the IppsFilterSpec_DTS_32f
//                         specification structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when any of the specified
//                          pointers is NULL.
*/
IPPAPI(IppStatus, ippsSynthesisFilterInit_DTS_32f, (IppsFilterSpec_DTS_32f *pFilterSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterInitAlloc_DTS_32f
//
//  Purpose:
//    Allocate memory for synthesis specification structure and initializes it.
//
//  Parameters:
//    ppFilterSpec          Pointer to where pointer to the IppsFilterSpec_DTS_32f
//                          specification structure is written.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when ppFilterSpec is NULL.
*/
IPPAPI(IppStatus, ippsSynthesisFilterInitAlloc_DTS_32f, (IppsFilterSpec_DTS_32f **ppFilterSpec))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippsSynthesisFilterFree_DTS_32f
//
//  Purpose:
//    Frees memory allocated for internal IppsFilterSpec_DTS_32f structure.
//
//  Parameters:
//    pFilterSpec           Pointer to the IppsFilterSpec_DTS_32f specification
//                          structure.
//
//  Returns:
//    ippStsNoErr           Indicates no error.
//    ippStsNullPtrErr      Indicates an error when pFilterSpec is NULL.
//    ippStsContextMatchErr Indicates an error when the identifier
//                          pFilterSpec is incorrect.
*/
IPPAPI(IppStatus, ippsSynthesisFilterFree_DTS_32f, (IppsFilterSpec_DTS_32f *pFilterSpec))

#ifdef __cplusplus
}
#endif

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#endif  /* #if !defined(__IPPAC_H__ ) || defined(_OWN_BLDPCS ) */
