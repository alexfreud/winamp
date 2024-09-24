/****************************************************************************
*
*   Module Title :     COMPDLL.H
*
*   Description  :     Encoder definitions.
*
*****************************************************************************
*/
#ifndef __INC_COMPDLL_H
#define __INC_COMPDLL_H

#include "codec_common.h"
#include "preprocif.h"
#include "preproc.h"
#include "pbdll.h"
#include "vp60_comp_interface.h"
#include "RawBuffer.h"
#include <stdio.h>
/****************************************************************************
*  Module constants.
*****************************************************************************
*/
// Debug/stats code
//#define PSNR_ON
//#define FILE_PSNR 
#define MIN_BPB_FACTOR          0.1
#define MAX_BPB_FACTOR          10.0
#define KEY_FRAME_CONTEXT       5

// GF update constants
#define DEFAULT_GF_UPDATE_INTERVAL	8
#define DEFAULT_2PASS_GF_UPDATE_INTERVAL 4
#define MIN_GF_UPDATE_INTERVAL		4
#define MAX_GF_UPDATE_INTERVAL		8
#define GF_UPDATE_MOTION_INTERVAL	48
#define MAX_GF_UPDATE_MOTION		16
#define GF_DEFAULT_MOTION_CMPLX		12
#define GF_MODE_DIST_THRESH1		50 
#define GF_MODE_DIST_THRESH2		25
#define GF_MAX_VAR_THRESH			36
#define FIRSTPASS_Q                 32
//#define FULLFRAMEFDCT
/****************************************************************************
*  Types
*****************************************************************************
*/

typedef struct CONFIG_TYPE2
{
    UINT32 TargetBandwidth;
    UINT32 OutputFrameRate;

    UINT32 FirstFrameQ;
    UINT32 BaseQ;
    UINT32 WorstQuality;		// Worst Quality allowed.
    UINT32 ActiveWorstQuality;	// Reflects worst quality Currently allowed (specified as an index where 0 is worst quality)
    UINT32 ActiveBestQuality;	// Reflects best quality currently allowed (specified as an index where 0 is worst quality)

} CONFIG_TYPE2;

typedef enum
{
    DCT_COEF_TOKEN,
    MODE_TOKEN,
    BLOCKMAP_TOKEN,
    MV_TOKEN
} TOKENTYPE;

typedef struct _TOKENEXTRA
{
    INT32  Token;
    UINT32 Extra;

    INT32  LastTokenL;      // Last token in block LEFT
    INT32  LastTokenA;      // Last token in block ABOVE

} TOKENEXTRA;

typedef struct LineEq2
{
    double  M;
    double  C;

} LINE_EQ2;

typedef struct
{
	BLOCK_CONTEXT *  AbovePtr;
	BLOCK_CONTEXT    Above;
	BLOCK_CONTEXT *  LeftPtr;
	BLOCK_CONTEXT    Left;
	Q_LIST_ENTRY  *  LastDcPtr;
	Q_LIST_ENTRY     LastDc;

} MB_DC_CONTEXT;

typedef struct MOTION_STATS
{
	UINT32	NumMvs;
	UINT32  SumAbsX;
	UINT32  SumAbsY;
	INT32   SumX;
	INT32   SumY;
	UINT32  SumXSq;
	UINT32  SumYSq;

} MOTION_STATS;

typedef struct
{
    double           MotionSpeed;
    double           VarianceX;
    double           VarianceY;
    double           PercentGolden;
    double           PercentMotionY;
    double           PercentMotion;
    double           PercentNewMotion;
    unsigned int     QValue;
    double           MeanInterError;
    double           MeanIntraError;
    double           BitsPerMacroblock;
    double           SqBitsPerMacroblock;
    double           PSNR;
    int              isGolden;
    int              isKey;
    int              count;
    int              frame;
} FIRSTPASS_STATS;


/****************************************************************************
*  Imports
****************************************************************************/
extern UINT32 (*FiltBlockBilGetSad)(UINT8 *SrcPtr,INT32 SrcStride,UINT8 *ReconPtr1,UINT8 *ReconPtr2,INT32 PixelsPerLine,INT32 ModX, INT32 ModY,UINT32 BestSoFar);
extern UINT32 (*GetSAD16)(UINT8 *, INT32, UINT8 *, INT32, UINT32, UINT32);
extern UINT32 (*GetSadHalfPixel16)(UINT8 *, INT32, UINT8 *, UINT8 *, INT32, UINT32, UINT32);
extern void   (*fdct_short) ( INT16 * InputData, INT16 * OutputData );
extern void   (*idctc[65])( INT16 *InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern UINT32 (*GetSAD)(UINT8 *, INT32, UINT8 *, INT32, UINT32, UINT32);
extern UINT32 (*GetSadHalfPixel)(UINT8 *, INT32, UINT8 *, UINT8 *, INT32, UINT32, UINT32  );
extern UINT32 (*GetInterError)( UINT8 *, INT32, UINT8 *,  UINT8 *, INT32 );
extern UINT32 (*GetIntraError)( UINT8 *, INT32);
extern void   (*Sub8)( UINT8 *FiltPtr, UINT8 *ReconPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconStride );
extern void   (*Sub8_128)( UINT8 *FiltPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride );
extern void   (*Sub8Av2)( UINT8 *FiltPtr, UINT8 *ReconPtr1, UINT8 *ReconPtr2, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconStride );

#define HUGE_ERROR              (1<<28)  //  Out of range test value

// Number of search sites for heirachical search (8*steps)+1 
// so for  (+- 32 pixels) = 5 step = 41 (previously 4 step = 33)
#define MAX_SEARCH_SITES        41       

typedef struct CP_INSTANCE * xCP_INST;

typedef struct CP_INSTANCE
{
    PB_INSTANCE pb; // playback

    CONFIG_TYPE2 Configuration;

    YUV_BUFFER_CONFIG InputConfig;
    YUV_BUFFER_CONFIG YuvInputData;
    INT32  SizeStep;
    INT32  LastSizeStep;

    INT32  QuickCompress;
    BOOL   GoldenFrameEnabled;
    BOOL   InterPrediction;
    BOOL   MotionCompensation;
    BOOL   AutoKeyFrameEnabled;
    INT32  ForceKeyFrameEvery;
    INT32  AutoKeyFrameThreshold;
    INT32  LastKeyFrame;
    INT32  MinimumDistanceToKeyFrame;
    INT32  KeyFrameDataTargetOrig;        // Data rate target for key frames
    INT32  KeyFrameDataTarget;            // Data rate target for key frames
    UINT32 KeyFrameFrequency;
    BOOL   DropFramesAllowed;
	BOOL   DropFrame;
    INT32  DropCount;
	INT32  MaxDropCount;
	INT32  MaxConsecDroppedFrames;
    UINT32 QualitySetting;
    UINT32 PreProcFilterLevel;
    BOOL   AllowSpatialResampling;
	UINT8  RdOpt;		// 0 - off, 1 - basic rd on, 2 - all rd options on

    // Compressor Statistics
    double TotErrScore;
    UINT32 InterError;

    UINT32 LastInterError;
    UINT32 LastIntraError;
    UINT32 MVErrorPerBit; 
    UINT32 ErrorPerBit; 
    UINT32 IntraError;
    INT64  KeyFrameCount;                          // Count of key frames.
    INT64  TotKeyFrameBytes;
    UINT32 LastKeyFrameSize;
    UINT32 PriorKeyFrameSize[KEY_FRAME_CONTEXT];
    UINT32 PriorKeyFrameDistance[KEY_FRAME_CONTEXT];
    INT32  FrameQuality[6];
    int    DecoderErrorCode;        // Decoder error flag.
    INT32  ThreshMapThreshold;
    INT32  TotalMotionScore;
    INT64  TotalByteCount;
    INT32  FixedQ;

	// Used for prediction filter selection
	UINT32 MotionInterErr;
	UINT32 MotionIntraErr;
	UINT8  BaselineAlpha;
	UINT8  BaselineBicThresh;

    // Frame Statistics
    INT64  CurrentFrame;
    UINT32 LastFrameSize;
    UINT32 ThisFrameSize;
    BOOL   ThisIsFirstFrame;
    BOOL   ThisIsKeyFrame;
	BOOL   GfRecoveryFrame;
    UINT32 FrameError ;

	// Stats for normal inter frames (excludes GFU frames and key frames)
	UINT32 NiFrames;
	UINT32 NiTotQi;					
	UINT32 NiAvQi;

    INT32  MotionScore;
	UINT32  FirstSixthBoundary;		// Macro block index marking the first sixth of the image
	UINT32  LastSixthBoundary;		// Macro block index marking the last sixth of the image

    /* Rate Targeting variables */
    double BpbCorrectionFactor;
	double KeyFrameBpbCorrectionFactor;
	double GfuBpbCorrectionFactor;

    // Controlling Block Selection
    UINT32 MVChangeFactor;
    UINT32 FourMvChangeFactor;
    UINT32 ExhaustiveSearchThresh;
    UINT32 BlockExhaustiveSearchThresh;
    UINT32 MinImprovementForFourMV;
    UINT32 FourMVThreshold;
    UINT32 IntraThresh;

	UINT32 MinErrorForMacroBlockMVSearch;
	UINT32 MinErrorForBlockMVSearch;
	UINT32 MinErrorForGoldenMVSearch;

    UINT16 *FrameZeroCountsAlloc;
    UINT16 *FrameZeroCounts;
	UINT32 FrameNzCount[BLOCK_SIZE][2];
	UINT8  NewScanOrderBands[BLOCK_SIZE];

    // Frames
    YUV_BUFFER_ENTRY *yuv0ptr;			// Un-pre-processed raw input (but scaled if appropriate)
    YUV_BUFFER_ENTRY *yuv1ptr;

    // Token Buffers
    TOKENEXTRA *CoeffTokens;
    TOKENEXTRA *CoeffTokenPtr;

    INT16  LastDC[3];

    BOOL_CODER bc;
    BOOL_CODER bc2;

    UINT8  *DataOutputBuffer;
    UINT8  MBCodingMode;        // Coding mode flags

    INT32  MVPixelOffsetY[MAX_SEARCH_SITES];
    UINT32 InterTripOutThresh;
    INT32  MVSearchSteps;
    INT32  MVOffsetX[MAX_SEARCH_SITES];
    INT32  MVOffsetY[MAX_SEARCH_SITES];
    INT8   SubPixelXOffset[9];       // Half pixel MV offsets for X
    INT8   SubPixelYOffset[9];       // Half pixel MV offsets for Y

    Q_LIST_ENTRY    *quantized_list;

    MOTION_VECTOR   MVector;
    INT16  *DCT_codes;          //Buffer that stores the result of Forward DCT
    INT16  *DCTDataBuffer;      //Input data buffer for Forward DCT

    // Motion compensation related variables
    UINT32  MvMaxExtent;

    INT32  byte_bit_offset;

    UINT32 NearestError[4];
    UINT32 NearError[4];
    UINT32 ZeroError[4];
    UINT32 BestError[4];
	UINT32 ErrorBins[128];

    xPP_INST pp;    // preprocessor

#if defined PSNR_ON
    double TotPsnr;
    double MinPsnr;
    double MaxPsnr;
    double TotYPsnr;
    double MinYPsnr;
    double MaxYPsnr;
    double TotUPsnr;
    double MinUPsnr;
    double MaxUPsnr;
    double TotVPsnr;
    double MinVPsnr;
    double MaxVPsnr;
    double TotalSqError;
#endif
#if defined FULLFRAMEFDCT
    Q_LIST_ENTRY  (*FDCTCoeffs)[64];
#endif

    // Structures for entropy contexts
    UINT32 FrameDcTokenDist[2][MAX_ENTROPY_TOKENS];
    //UINT32 FrameAcTokenDist[PREC_CASES][2][VP6_AC_BANDS][MAX_ENTROPY_TOKENS];
    UINT32 FrameAcTokenDist[PREC_CASES][2][8][16];
    // Extra structures needed to decide if we choose huffman and DC / EOB runs
    UINT32 FrameDcTokenDist2[2][MAX_ENTROPY_TOKENS];
    //UINT32 FrameAcTokenDist2[PREC_CASES][2][VP6_AC_BANDS][MAX_ENTROPY_TOKENS];
    UINT32 FrameAcTokenDist2[PREC_CASES][2][8][16];

    // AWG Debug Accumulate token count for entire run
    UINT32 CumulativeFrameDcTokenDist[2][MAX_ENTROPY_TOKENS];
    UINT32 CumulativeFrameAcTokenDist[PREC_CASES][2][VP6_AC_BANDS][MAX_ENTROPY_TOKENS];

	// Storage for the first frame entropy probabilities.
	// These are re-used for all subsequent key frames when we are operating in
	// error (drop frame) ressiliant mode.
	UINT8 FirstFrameDcProbs[2*(MAX_ENTROPY_TOKENS-1)];
	UINT8 FirstFrameAcProbs[2*PREC_CASES*VP6_AC_BANDS*(MAX_ENTROPY_TOKENS-1)];

	UINT32 FrameZrlDist[ZRL_BANDS][64];
	UINT32 FrameZeroCount[ZRL_BANDS];
	UINT8  FrameZrlProbs[ZRL_BANDS][ZERO_RUN_PROB_CASES];
	UINT32 FrameZrlBranchHits[ZRL_BANDS][ZERO_RUN_PROB_CASES][2];

    // Last token coded this block.
    UINT32 MBModeCount[4][MAX_MODES+1];
    UINT32 BModeCount[MAX_MODES+1];
	UINT32 CountModeSameAsLast[4][MAX_MODES+1];
	UINT32 CountModeDiffFrLast[4][MAX_MODES+1];

    UINT32 ModeCodeArray[4][MAX_MODES+1][MAX_MODES+1];
    UINT8  ModeLengthArray[4][MAX_MODES+1][MAX_MODES+1];

	UINT32 MBModeCostBoth[11];
	UINT32 MBModeCostNoNear[11];
	UINT32 MBModeCostNoNearest[11];
	UINT32 BModeCost[11];
	UINT32 MvBaselineDist[2][MV_ENTROPY_TOKENS];
	UINT32 FrameMvCount;
	UINT32 EstModeCost[2][MAX_MODES];
	UINT32 EstMVCost[2][MV_ENTROPY_TOKENS];
	UINT32 * EstMvCostPtrX;
	UINT32 * EstMvCostPtrY;

	// Data structure used in re-calculating MV probability nodes	
	UINT8  NewMvSignProbs[2];
	UINT8  NewIsMvShortProb[2];
	UINT8  NewMvShortProbs[2][7];
	UINT8  NewMvSizeProbs[2][LONG_MV_BITS];

	UINT32 NewMvSignHits[2][2];
	UINT32 NewIsMvShortHits[2][2];
	UINT32 NewMvShortHits[2][7][2];
	UINT32 NewMvSizeHits[2][LONG_MV_BITS][2];


    UINT32 nExperimentals;
    INT32  Experimental[C_SET_EXPERIMENTAL_MAX - C_SET_EXPERIMENTAL_MIN + 1];

	// Bandwidth and buffer control variables
	INT32  PerFrameBandwidth;				// Target for average bandwidth per frame.
    INT32  InterFrameTarget;				// Average "inter" frame bit target corrected for key frame costs
    INT32  ThisFrameTarget;					// Modified rate target for this frame

	BOOL   BufferedMode;					// FALSE = Tight buffering (Video Conferencing mode); TRUE = normal buffered/streaming mode.
	BOOL   ErrorResilliantMode;				// A mode used for VC etc. to make the codec more resilliant to dropped frames.
	INT32  StartingBufferLevel;             // The initial encoder buffer level
	INT32  BytesOffTarget;				    // How far off target are we in repect of target bytes for clip 
	INT32  OptimalBufferLevel;				// The buffer level target we strive to reach / maintain.
	INT32  BufferLevel;                     // Buffer level based upon the max sustainable rate used for rate targeting
	INT32  MaxBufferLevel;			        // The maximum permited value for the buffer level.
	INT32  DropFramesWaterMark;				// Buffer fullness watermark for forced drop frames.
	INT32  ResampleDownWaterMark;			// Buffer fullness watermark for downwards spacial re-sampling
	INT32  ResampleUpWaterMark;				// Buffer fullness watermark where returning to larger image size is consdered
	INT32  LastKeyFrameBufferLevel;			// Used to monitor changes in buffer level when considering re-sampling.

	INT32  Speed;
	INT32  CPUUsed;

	UINT32 ModeMvCostEstimate;				// Running total of cost estimates for modes and MVs in this frame.

	// Variables used in regulating cost of new motion vectors based upon an estimate of new MV frequency.
	UINT32 FrameNewMvCounter;
	UINT32 FrameModeCounter;
	UINT32 MvEpbCorrection;
	UINT32 LastFrameNewMvUsage;				// 0 = Low 9 = High

	UINT32 * MbBestErr;

    UINT32 EstDcTokenCosts[2][MAX_ENTROPY_TOKENS];
    UINT32 EstAcTokenCosts[PREC_CASES][2][VP6_AC_BANDS][MAX_ENTROPY_TOKENS];
    UINT32 EstZrlCosts[ZRL_BANDS][64];

	// Data structures used to save and restor MB and DC contexts during rate distortion
	MACROBLOCK_INFO CopyMbi;
	BLOCK_CONTEXT AboveCopyY[2];		
	BLOCK_CONTEXT AboveCopyU;		
	BLOCK_CONTEXT AboveCopyV;		
	BLOCK_CONTEXT LeftYCopy[2];
	BLOCK_CONTEXT LeftUCopy;
	BLOCK_CONTEXT LeftVCopy;
	Q_LIST_ENTRY  LastDcYCopy[3];
	Q_LIST_ENTRY  LastDcUCopy[3];
	Q_LIST_ENTRY  LastDcVCopy[3];

	MB_DC_CONTEXT MbDcContexts[MAX_MODES][6];		// Per mode, per block position data structure for and MB

	UINT32 avgPickModeTime;
	UINT32 avgEncodeTime;
	UINT32 avgPackVideoTime;

	UINT32 ForceHScale;
	UINT32 ForceHRatio;
	UINT32 ForceVScale;
	UINT32 ForceVRatio;
	BOOL   ForceInternalSize;

	PreProcInstance preproc;

    // Buffers for output bitstream partitions
    UINT8 *OutputBuffer2;
    RAW_BUFFER RawBuffer;

    // In Huffman mode runs of zeros at DC position & runs
    // of EOB at first AC position are used
    INT32 CurrentDcZeroRun[2];
    TOKENEXTRA *DcZeroRunStartPtr[2];
    INT32 CurrentAc1EobRun[2];
    TOKENEXTRA *Ac1EobRunStartPtr[2];
    
    // DEBUG
    UINT32 HuffCost;
    UINT32 CostShannon;

	BOOL   AllowScanOrderUpdates;
    INT32  FrameRateInput;
    INT32  FrameRateDropFrames;
    INT32  FrameRateDropCount;

	// Stats for monitoring frame mode and MV data
	UINT32 ModeDist[MAX_MODES];

	// Stats collected about the use of motion vectors in the curent frame
	MOTION_STATS FrameMvStats;			
									
	// Variables used in control of GF update
	UINT32 FramesTillGfUpdateDue;
	INT32 GfUpdateInterval;
	UINT32 GfuMotionSpeed;
	UINT32 GfuMotionComplexity;
	UINT32 GfuBoost;
	UINT32 GfUsage;					// GF usage metric 
	UINT32 LastGfOrKFrameQ;
    
    // variables for 5 region diamond MV search
    INT32  DSMVSearchSteps;
    INT32  DSMVPixelOffsetY[MAX_SEARCH_SITES];
    INT32  DSMVOffsetX[MAX_SEARCH_SITES];
    INT32  DSMVOffsetY[MAX_SEARCH_SITES];

    // 2 pass stats
    INT32  pass;
    FIRSTPASS_STATS fps;
    FIRSTPASS_STATS fpmss;
    FILE *fs;
    FILE *ss;
    INT32  GoldenFrameBoost;
    INT32  MbsSinceGolden;
    INT32  OneGoldenFrame;
    INT32  KFBoost;
    INT32  InterBoostFreq;
    INT32  InterBoost;
    INT32  GoldenMbsSinceGolden;
    INT32  GoldenMbsThisFrame;
    INT32  InterErrorb;
    INT32  FramesToKey;
    double FirstPassPSNR;
    INT32  ActualTargetBitRate;
    INT32  KFForced;
    INT32  NextKFForced;
    INT32  CalculatedWorstQ;
    INT32  PassedInWorstQ;


    // new parameters

    BOOL   DisableGolden;                   // disable golden frame updates
    BOOL   VBMode;                          // run in variable bandwidth 1 pass mode
    BOOL   EndUsage;						// Local file playback mode / vs streamed
	BOOL   AutoWorstQ;						// Auto adjust worst quality.... 1 pass vbr within buffering constraints
    UINT32 BestAllowedQ;                    // best allowed quality ( save bits by disallowings frames that are too high quality ) 
    INT32  UnderShootPct;                   // target a percentage of the actual frame to allow for sections that go over

    INT32  MaxAllowedDatarate;              // maximum the datarate is allowed to go.
    INT32  MaximumBufferSize;               // maximum buffer size.

    BOOL   TwoPassVBREnabled;               // two pass variable bandwidth enabled
    INT32  TwoPassVBRBias;                  // how variable do we want to target?
    INT32  TwoPassVBRMaxSection;            // maximum 
    INT32  TwoPassVBRMinSection;            // minimum 
    INT32  Pass;                            // which pass of the compression are we running.
    double TotalBitsLeftInClip;
    double FramesYetToEncode;
    double TotalBitsPerMB;

	// Prediction mode parameters for VP6.2
	UINT8  LastPredictionFilterMode;
	UINT8  LastPredictionFilterMvSizeThresh;
	UINT32 LastPredictionFilterVarThresh;
	UINT8  LastPredictionFilterAlpha;


    UINT32 (*FindMvViaSearch)
        (xCP_INST cpi,
        CODING_MODE	Mode,
        UINT8 *SrcPtr,
        UINT8 *RefPtr,
        MOTION_VECTOR *MV,
        UINT8 **BestBlockPtr,
        UINT32 BlockSize);

    void (*FindBestHalfPixelMv)
        (xCP_INST cpi,
        CODING_MODE	Mode,
        UINT8 *SrcPtr,
        UINT8 *RefPtr,
        MOTION_VECTOR *MV,
        UINT32 BlockSize,
        UINT32 *MinError,
        UINT8  BitShift);

    void (*FindBestQuarterPixelMv)
        (xCP_INST cpi,
        CODING_MODE	Mode,
        UINT8 *SrcPtr,
        UINT8 *RefPtr,
        MOTION_VECTOR *MV,
        UINT32 BlockSize,
        UINT32 *MinError,
        UINT8  BitShift);

} CP_INSTANCE;


/****************************************************************************
*  Exports
****************************************************************************/
UINT32 (*GetMBFrameVertVar)(CP_INSTANCE *cpi);
UINT32 (*GetMBFieldVertVar)(CP_INSTANCE *cpi);
UINT32 (*GetBlockReconErr)(CP_INSTANCE *cpi, UINT32 bp);


/****************************************************************************
*  Imports
****************************************************************************/
extern void UpdateFrame(CP_INSTANCE *cpi);
extern UINT32 EncodeData(CP_INSTANCE *cpi);

// Loop optimizations
extern void InitMapArrays();

// Codec
extern void SUB8( UINT8 *FiltPtr, UINT8 *ReconPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1,
               INT32 SourceStride, INT32 ReconStride );
extern void SUB8_128( UINT8 *FiltPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1,
               INT32 SourceStride );
extern void SUB8AV2( UINT8 *FiltPtr, UINT8 *ReconPtr1, UINT8 *ReconPtr2, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1,
              INT32 SourceStride, INT32 ReconStride );

extern CP_INSTANCE * CreateCPInstance(void);
extern void DeleteCPInstance(CP_INSTANCE **cpi);
extern void CMachineSpecificConfig(void);
extern void fdct_short_C ( INT16 * InputData, INT16 * OutputData );

extern BOOL EAllocateFragmentInfo(CP_INSTANCE *cpi);
extern BOOL EAllocateFrameInfo(CP_INSTANCE *cpi);
extern void EDeleteFragmentInfo(CP_INSTANCE *cpi);
extern void EDeleteFrameInfo(CP_INSTANCE *cpi);
extern UINT32 PickIntra( CP_INSTANCE *cpi );
extern UINT32 PickModes( CP_INSTANCE *cpi, UINT32 *InterError, UINT32 *IntraError);

extern void ClampAndUpdateQ ( CP_INSTANCE *cpi, UINT32 QIndex);
extern void EncodeFrameMbs(CP_INSTANCE *cpi);
extern void CCONV ChangeEncoderSize(CP_INSTANCE* cpi, UINT32 Width, UINT32 Height);
extern void CopyOrResize(CP_INSTANCE* cpi, BOOL ResetPreproc );
extern UINT32 TokenizeFrag(CP_INSTANCE* cpi, INT16*  RawData, UINT32 Plane, BLOCK_CONTEXT* Above, BLOCK_CONTEXT* Left);
extern void PredictScanOrder( CP_INSTANCE *cpi );
extern void BuildScanOrder( PB_INSTANCE *pbi, UINT8 * );

#endif
