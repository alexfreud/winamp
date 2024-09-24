/****************************************************************************
*
*   Module Title :     COMPDLL.H
*
*   Description  :     Video CODEC demo compression DLL main header
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.34 YWX 09-Dec-02 Added Function pointers for frame/field varainces calculation
*   1.33 YWX 30-Oct-02 Added EncoderLoopFilterOff flag
*   1.32 YWX 28-Oct-02 Added function pointer for 5 region diamond search 
*   1.31 YWX 28-Oct-02 Added above and left token context and 5 region
*                      diamond motion search sites
*   1.30 YWX 02-Jul-02 Added new funcion pointers for motion search
*   1.31 JBB 04 JUL-02 Added preprocessor code
*   1.29 AWG 20-Jun-01 Removed QuadCodeComponent function prototype & HExtra/VExtra
*   1.29 AWG 22-May-01 Added support for DCT16
*   1.28 JBB 05-May-01 Changes for VP5 (new entropytablebits and tokenextra chgs
*   1.27 JBB 23-Mar-01 Changed QuickCompress datatype from BOOL to INT32
*   1.26 JBB 11 Feb 01 Merged in: added vars for map ca move ac choice to right after dc
*   1.25 PGW 31 Jan 01 Added some stats variables and VP5 Mv entropy tables.
*   1.24 JBB 30 Nov 00 Version number changes
*   1.23 JBB 15 Nov 00 Cleaned out ifdefs
*   1.22 JBB 15 Oct 00 Added First Pass Function
*   1.21 JBB 11 Sep 00 new function pointers for subtract removed transxquant
*   1.20 JBB 07 Sep 00 Changed error metrics to Unsigned int
*   1.19 JBB 24 Aug 00 Ansi C compatible
*   1.18 JBB 27Jul00   added checks on Mallocs
*   1.17 JBB 24Jul00   Changed error functions to return INT32 instead of double
*   1.16 PGW 12 Jul 00 Removed CompAutoKeyFrameThreshold.
*   1.15 PGW 29 Jun 00 Removed instnace varibale CarryOverAdaptionEnabled.
*   1.14 PGW 27 Jun 00 Added QTargetModifier[]. Changes to CONFIG_TYPE2.
*   1.13 JBB 30/05/00  Removed hard coded size limits
*   1.12 JBB 22/05/00  Added OriginalDC support to remove max_fragments depends
*   1.11 YX  13/04/00  Add function pointers for new optimizations
*   1.10 YX  06/04/00  More buffers alligned MMX Fdct
*   1.09 YX  20/03/00  32 Byte alligned buffers, Back to Integer Forward DCT
*                      Additional Function pointers for optimized code
*   1.08 PGW 17/03/00  Changes to support seperate Y and UV entropy tables.
*                      Added PreProcFilterLevel to allow control of preprecessor
*                      filter level.
*   1.07 YX  09/03/00  Change to use floating point forward DCT
*   1.06 PGW 17/12/99  Draw dib functionality removed.
*   1.05 PGW 05/10/99  Remove some Windows dependancies for VFW compressor.
*   1.04 PGW 20/07/99  Rate targeting corrections for VFW version of codec
*   1.03 PGW 15/07/99  Added QuickCompress flag.
*   1.02 PGW 05/07/99  Added GetFOURMVExhaustiveSearch() function
*   1.01 PGW 29/06/99  Added GetMBMVExhaustiveSearch() function.
*   1.00 PGW 14/06/99  Configuration baseline
*
*****************************************************************************
*/

#ifndef __INC_COMPDLL_H
#define __INC_COMPDLL_H

#define MIN_BPB_FACTOR          0.1
#define MAX_BPB_FACTOR          10.0

#define KEY_FRAME_CONTEXT       5

#include "codec_common.h"
#include "preprocif.h"
#include "preproc.h"
#include "pbdll.h"
#include "vp50_comp_interface.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/

// Debug/stats code
//#define PSNR_ON


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
    UINT32 MaxQ;				// Absolute Max Q allowed.
    UINT32 ActiveWorstQuality;	// Reflects worst quality Currently allowed (specified as an index where 0 is worst quality)
    UINT32 ActiveBestQuality;	// Reflects best quality currently allowed (specified as an index where 0 is worst quality)

} CONFIG_TYPE2;


/* Defines the largest positive integer expressable with a standard int type */
/****************************************************************************
* *     Type declarations
****************************************************************************
*/

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
} TOKENEXTRA;


typedef struct LineEq2
{
    double  M;
    double  C;

} LINE_EQ2;

typedef struct
{
	BLOCK_CONTEXTA *  AbovePtr;
	BLOCK_CONTEXTA    Above;
	BLOCK_CONTEXT *  LeftPtr;
	BLOCK_CONTEXT    Left;
	Q_LIST_ENTRY  *  LastDcPtr;
	Q_LIST_ENTRY     LastDc;

} MB_DC_CONTEXT;

/****************************************************************************
*  MACROS
*****************************************************************************
*/

/****************************************************************************
*  Global Variables
*****************************************************************************
*/

//****************************************************************
// Function Pointers now library globals!
extern UINT32 (*GetSAD16)(UINT8 *, INT32, UINT8 *, INT32, UINT32, UINT32);
extern UINT32 (*GetSadHalfPixel16)(UINT8 *, INT32, UINT8 *, UINT8 *, INT32, UINT32, UINT32);
extern void   (*fdct_short) ( INT16 * InputData, INT16 * OutputData );
extern void   (*idctc[65])( INT16 *InputData, INT16 *QuantMatrix, INT16 * OutputData );
extern UINT32 (*GetSAD)(UINT8 *, INT32, UINT8 *, INT32, UINT32, UINT32) ;
//extern UINT32 (*GetNextSAD)(UINT8 *, INT32, UINT8 *, UINT32, UINT32 );
extern UINT32 (*GetSadHalfPixel)(UINT8 *, INT32, UINT8 *, UINT8 *, INT32, UINT32, UINT32  );
extern UINT32 (*GetInterError)( UINT8 *, INT32, UINT8 *,  UINT8 *, INT32 );
extern UINT32 (*GetIntraError)( UINT8 *, INT32);
extern void   (*Sub8)( UINT8 *FiltPtr, UINT8 *ReconPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconStride );
extern void   (*Sub8_128)( UINT8 *FiltPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride );
extern void   (*Sub8Av2)( UINT8 *FiltPtr, UINT8 *ReconPtr1, UINT8 *ReconPtr2, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, INT32 SourceStride, INT32 ReconStride );

//****************************************************************




#define HUGE_ERROR              (1<<28)  //  Out of range test value

#define MAX_SEARCH_SITES        33      //  Number of search sites for a 4-step search (at pixel accuracy)

typedef struct CP_INSTANCE * xCP_INST;
typedef struct CP_INSTANCE
{
    PB_INSTANCE pb; // playback
    INT32  DropCounter;

    //****************************************************************************************************
    // Compressor Configuration
    CONFIG_TYPE2 Configuration;

    YUV_BUFFER_CONFIG InputConfig;
    YUV_BUFFER_CONFIG YuvInputData;
    INT32  SizeStep;

    INT32  QuickCompress;
    BOOL   GoldenFrameEnabled;
    BOOL   InterPrediction;
    BOOL   MotionCompensation;
    BOOL   AutoKeyFrameEnabled ;
    INT32  ForceKeyFrameEvery ;
    INT32  AutoKeyFrameThreshold ;
    UINT32 LastKeyFrame ;
    UINT32 MinimumDistanceToKeyFrame ;
    INT32  KeyFrameDataTargetOrig ;        // Data rate target for key frames
    INT32  KeyFrameDataTarget ;        // Data rate target for key frames
    UINT32 KeyFrameFrequency ;
    BOOL   DropFramesAllowed ;
	BOOL   DropFrame;
    INT32  DropCount ;
    UINT32 QualitySetting;
    UINT32 PreProcFilterLevel;
    BOOL   AllowSpatialResampling;
	UINT8  RdOpt;		// 0 - off, 1 - basic rd on, 2 - all rd options on


    // Compressor Statistics
    double TotErrScore;
    UINT32 InterError;
    UINT32 MVErrorPerBit; 
    UINT32 ErrorPerBit; 
    UINT32 IntraError;
    INT64  KeyFrameCount ;                          // Count of key frames.
    INT64  TotKeyFrameBytes ;
    UINT32 LastKeyFrameSize ;
    UINT32 PriorKeyFrameSize[KEY_FRAME_CONTEXT];
    UINT32 PriorKeyFrameDistance[KEY_FRAME_CONTEXT];
    INT32  FrameQuality[6];
    int    DecoderErrorCode;        // Decoder error flag.
    INT32  ThreshMapThreshold;
    INT32  TotalMotionScore;
    INT64  TotalByteCount;
    INT32  FixedQ;

    // Frame Statistics
    INT64  CurrentFrame;
    UINT32 LastFrameSize;
    UINT32 ThisFrameSize;
    BOOL   ThisIsFirstFrame;
    BOOL   ThisIsKeyFrame;
	BOOL   GfRecoveryFrame;

    INT32  MotionScore;
	UINT32  FirstSixthBoundary;		// Macro block index marking the first sixth of the image
	UINT32  LastSixthBoundary;		// Macro block index marking the last sixth of the image

    /* Rate Targeting variables PGW 08/05/96). */
    double BpbCorrectionFactor;
	double KeyFrameBpbCorrectionFactor;

    // Controlling Block Selection
    UINT32 MVChangeFactor;
    UINT32 FourMvChangeFactor;
    UINT32 ExhaustiveSearchThresh;
    UINT32 MinImprovementForFourMV;
    UINT32 FourMVThreshold;
    UINT32 IntraThresh;

	UINT32 MinErrorForMacroBlockMVSearch;
	UINT32 MinErrorForBlockMVSearch;
	UINT32 MinErrorForGoldenMVSearch;


    //****************************************************************************************************


    //****************************************************************************************************
    // Frames
    // Used in the selecetive convolution filtering of the Y plane. */
    YUV_BUFFER_ENTRY *yuv1ptr;
    YUV_BUFFER_ENTRY *yuv1ptrAlloc;
    //****************************************************************************************************

    //****************************************************************************************************
    // Token Buffers
    TOKENEXTRA *CoeffTokens;
    TOKENEXTRA *CoeffTokensAlloc;
    TOKENEXTRA *CoeffTokenPtr;

    INT16  LastDC[3];

    BOOL_CODER bc;

    //****************************************************************************************************

    //****************************************************************************************************
    // SuperBlock, MacroBLock and Fragment Information
    // Coded flag arrays and counters for them

    //****************************************************************************************************
    // Live Codec Variables

    UINT8  *DataOutputBuffer;
    //****************************************************************************************************

    //****************************************************************************************
    // STATICS COPIED FROM C FILES (USED IN MULTIPLE FUNCTIONS BUT ARE NOT REALLY INSTANCE GLOBALS )
    // copied from cencode.c
    UINT8  MBCodingMode;        // Coding mode flags

    // copied from mcomp.c
    INT32  MVPixelOffsetY[MAX_SEARCH_SITES];
    UINT32 InterTripOutThresh;
    INT32  MVSearchSteps;
    INT32  MVOffsetX[MAX_SEARCH_SITES];
    INT32  MVOffsetY[MAX_SEARCH_SITES];
    INT32  HalfPixelRef2Offset[9];    // Offsets for half pixel compensation
    INT8   HalfPixelXOffset[9];       // Half pixel MV offsets for X
    INT8   HalfPixelYOffset[9];       // Half pixel MV offsets for Y


    Q_LIST_ENTRY    *quantized_list;
    Q_LIST_ENTRY    *quantized_listAlloc;

    MOTION_VECTOR   MVector;
    INT16  *DCT_codes;          //Buffer that stores the result of Forward DCT
    INT16  *DCTDataBuffer;      //Input data buffer for Forward DCT
    INT16  *DCT_codesAlloc;
    INT16  *DCTDataBufferAlloc;


    // Motion compensation related variables
    UINT32  MvMaxExtent;

    INT32  byte_bit_offset;

    // copied from cbitman.c
    UINT32 NearestError[4];
    UINT32 NearError[4];
    UINT32 ZeroError[4];
    UINT32 BestError[4];

	UINT32 ErrorBins[128];

    //****************************************************************
    // instances (used for reconstructing buffers and to hold tokens etc.)
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
#endif

    // Structures for entropy contexts
    UINT32 FrameDcTokenDist[2][MAX_ENTROPY_TOKENS];
    UINT32 FrameAcTokenDist[PREC_CASES][2][VP5_AC_BANDS][MAX_ENTROPY_TOKENS];

	// Storage for the first frame entropy probabilities.
	// These are re-used for all subsequent key frames when we are operating in
	// error (drop frame) ressiliant mode.
	UINT8 FirstFrameDcProbs[2*(MAX_ENTROPY_TOKENS-1)];
	UINT8 FirstFrameAcProbs[2*PREC_CASES*VP5_AC_BANDS*(MAX_ENTROPY_TOKENS-1)];

    // The Plane Y or UV to which the current block belongs (0 = Y 1 = UV)
    UINT8  EncoderPlane;

    // Last token coded this block.
    UINT8  ThisBlockLastToken;
    UINT8  ZeroCount;
    //UINT32 MBModeCount[MAX_MODES+1];
    UINT32 MBModeCount[4][MAX_MODES+1];
    UINT32 BModeCount[MAX_MODES+1];
	UINT32 CountModeSameAsLast[4][MAX_MODES+1];
	UINT32 CountModeDiffFrLast[4][MAX_MODES+1];

    UINT32 ModeCodeArray[4][MAX_MODES+1][MAX_MODES+1];
    UINT8  ModeLengthArray[4][MAX_MODES+1][MAX_MODES+1];

    // TEMP 
    UINT32 ModeBitCount[2];
    INT64  ModeComplexity[2];
    UINT32 ModeBlocks[2];

	UINT32 MBModeCostBoth[11];
	UINT32 MBModeCostNoNear[11];
	UINT32 MBModeCostNoNearest[11];
	UINT32 BModeCost[11];
	UINT32 MvBaselineDist[2][MV_ENTROPY_TOKENS];
	UINT32 FrameMvCount;
	UINT32 EstMVCost[2][MV_ENTROPY_TOKENS];
	UINT32 EstModeCost[2][MAX_MODES];
	
    UINT32 nExperimentals;
    INT32 Experimental[C_SET_EXPERIMENTAL_MAX - C_SET_EXPERIMENTAL_MIN + 1];

	// Bandwidth and buffer control variables
	INT32  PerFrameBandwidth;				// Target for average bandwidth per frame.
    INT32  InterFrameTarget;				// Average "inter" frame bit target corrected for key frame costs
    INT32  ThisFrameTarget;					// Modified rate target for this frame

	BOOL   BufferedMode;					// FALSE = Tight buffering (Video Conferencing mode); TRUE = normal buffered/streaming mode.
	BOOL   ErrorResilliantMode;				// A mode used for VC etc. to make the codec more resilliant to dropped frames.
	INT32  StartingBufferLevel;             // The initial encoder buffer level
	INT32  CurrentBufferLevel;				// Current decoder buffer fullness state 
	INT32  OptimalBufferLevel;				// The buffer level target we strive to reach / maintain.
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
	UINT32 * MbBestErrAlloc;

    UINT32 EstDcTokenCosts[2][MAX_ENTROPY_TOKENS];
    UINT32 EstAcTokenCosts[PREC_CASES][2][VP5_AC_BANDS][MAX_ENTROPY_TOKENS];

	// Data structures used to save and restor MB and DC contexts during rate distortion
	MACROBLOCK_INFO CopyMbi;
	BLOCK_CONTEXTA AboveCopyY[2];		
	BLOCK_CONTEXTA AboveCopyU;		
	BLOCK_CONTEXTA AboveCopyV;		
	BLOCK_CONTEXT LeftYCopy[2];
	BLOCK_CONTEXT LeftUCopy;
	BLOCK_CONTEXT LeftVCopy;
	Q_LIST_ENTRY  LastDcYCopy[3];
	Q_LIST_ENTRY  LastDcUCopy[3];
	Q_LIST_ENTRY  LastDcVCopy[3];

    // Above and left context for encoding
    UINT8  *aboveDcTokensAlloc[3];       // 0 for y, 1 for u and 2 for v
    UINT8  *aboveDcTokens[3];            // 0 for y, 1 for u and 2 for v
    UINT8  leftTokens[4][64];            // 0 1 for y 2 for u and 3 for v


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
    INT32  FrameRateInput;
    INT32  FrameRateDropFrames;
    INT32  FrameRateDropCount;

    
    //
    UINT32 EncoderLoopFilterOff;
    // variables for 5 region diamond MV search
    INT32  DSMVSearchSteps;
    INT32  DSMVPixelOffsetY[MAX_SEARCH_SITES];
    INT32  DSMVOffsetX[MAX_SEARCH_SITES];
    INT32  DSMVOffsetY[MAX_SEARCH_SITES];


    UINT32 (*FindMvViaSearch)(  xCP_INST cpi,
                                UINT8 *SrcPtr,
                                INT32  SourceStride,    
                                UINT8 *RefPtr,
                                INT32 ReconStride,
                                MOTION_VECTOR *MV,
                                UINT8 **BestBlockPtr,
                                UINT32 BlockSize);


    void (*FindBestHalfPixelMv)(xCP_INST cpi,
                                UINT8 *SrcPtr,
                                INT32 SourceStride,
                                UINT8 *RefPtr,
                                INT32 ReconStride,
                                MOTION_VECTOR *MV,
                                UINT8 **BestBlockPtr,
                                UINT32 BlockSize,
                                UINT32 MinError);



} CP_INSTANCE;


UINT32 (*GetMBFrameVertVar)(CP_INSTANCE *cpi);
UINT32 (*GetMBFieldVertVar)(CP_INSTANCE *cpi);

/****************************************************************************
*  Functions.
*****************************************************************************
*/


extern void UpdateFrame(CP_INSTANCE *cpi);

extern UINT32 QuadCodeDisplayFragments (CP_INSTANCE *cpi);

extern UINT32 QuadCodeComponent ( CP_INSTANCE *cpi, UINT32 FirstSB, UINT32 SBRows, UINT32 SBCols, UINT32 HExtra, UINT32 VExtra, INT32 SourceStride );

extern void AcquireSingleFrame( CP_INSTANCE *cpi, UINT32  CurrFrame );
extern void AcquireFirstFrame(CP_INSTANCE *cpi);
extern void AcquireNextFrame( CP_INSTANCE *cpi, UINT32 CurrFrame );

extern void InitFrameTimer( CP_INSTANCE *cpi);

extern UINT32 EncodeData(CP_INSTANCE *cpi);

// Loop optimizations
extern void InitMapArrays();

// Codec
extern UINT32 DPCMTokenizeBlock  ( CP_INSTANCE *cpi, INT32 FragIndex, INT32 SourceStride );
extern void SUB8( UINT8 *FiltPtr, UINT8 *ReconPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1,
               INT32 SourceStride, INT32 ReconStride );
extern void SUB8_128( UINT8 *FiltPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1,
               INT32 SourceStride );
extern void SUB8AV2( UINT8 *FiltPtr, UINT8 *ReconPtr1, UINT8 *ReconPtr2, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1,
              INT32 SourceStride, INT32 ReconStride );



extern void  PackEOBRun(CP_INSTANCE *cpi);
extern void ConvertBmpToYUV( PB_INSTANCE *pbi, UINT8 * BmpDataPtr, UINT8 * YuvBufferPtr );
extern CP_INSTANCE * CreateCPInstance(void);
extern void DeleteCPInstance(CP_INSTANCE **cpi);
extern void CMachineSpecificConfig(void);
// extern void fdct_slow16 ( INT16 * InputData, INT16 * OutputData );
extern void fdct_slowf ( INT16 * InputData, INT16 * OutputData );
extern void fdct_short_C ( INT16 * InputData, INT16 * OutputData );
extern void fdct_short_C ( INT16 * InputData, INT16 * OutputData );

extern BOOL EAllocateFragmentInfo(CP_INSTANCE *cpi);
extern BOOL EAllocateFrameInfo(CP_INSTANCE *cpi);
extern void EDeleteFragmentInfo(CP_INSTANCE *cpi);
extern void EDeleteFrameInfo(CP_INSTANCE *cpi);
extern UINT32 PickIntra( CP_INSTANCE *cpi );
extern UINT32 PickModes( CP_INSTANCE *cpi, UINT32 *InterError, UINT32 *IntraError);

extern INT32  GetSpeckSumAbsDiffs( UINT8 * NewDataPtr, UINT8 * RefDataPtr,
                              INT32 SourceStride, INT32 ErrorSoFar, INT32 BestSoFar );
extern INT32  GetNextSpeckSumAbsDiffs( UINT8 * NewDataPtr, UINT8 * RefDataPtr,
                              INT32 SourceStride, INT32 ErrorSoFar, INT32 BestSoFar );

extern INT32  GetHalfPixelSpeckSumAbsDiffs( UINT8 * SrcData, UINT8 * RefDataPtr1, UINT8 * RefDataPtr2,
                              INT32 SourceStride, INT32 ErrorSoFar, INT32 BestSoFar );
extern void ClampAndUpdateQ ( CP_INSTANCE *cpi, UINT32 QIndex) ;

//  cx\generic\encodembs.c
extern void EncodeFrameMbs(CP_INSTANCE *cpi);


//  cx\generic\vfw_comp_if.c
extern void CCONV ChangeEncoderSize(CP_INSTANCE* cpi, UINT32 Width, UINT32 Height);
extern void CopyOrResize(CP_INSTANCE* cpi);

//  cx\generic\tokenize.c
extern UINT16 TokenizeFrag(CP_INSTANCE* cpi, INT16*  RawData, UINT16 BlockSize, UINT32 Plane, BLOCK_CONTEXTA* Above, BLOCK_CONTEXT* Left);

#endif
