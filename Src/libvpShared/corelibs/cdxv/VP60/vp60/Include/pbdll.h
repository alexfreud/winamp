/****************************************************************************
*
*   Module Title :     pbdll.h
*
*   Description  :     Decoder definition header file.
*
****************************************************************************/
#ifndef __INC_PBDLL_H
#define __INC_PBDLL_H

/****************************************************************************
*  Module statics.
****************************************************************************/
#define VAL_RANGE   256     // Must come before header files--REMOVE THIS DEPENDENCY!!

/****************************************************************************
*  Header Files
****************************************************************************/
#include "codec_common.h"
#include "huffman.h"
#include "tokenentropy.h"
#include "vfw_pb_interface.h"
#include "postproc_if.h"
#include "vputil_if.h"
#include "quantize.h"
#include "boolhuff.h"
#include "rawbuffer.h"

/****************************************************************************
*  MACROS
****************************************************************************/


// Enumeration of how block is coded
// VP6.2 version is >= 8
#define CURRENT_ENCODE_VERSION  8
#define CURRENT_DECODE_VERSION  8

#define SIMPLE_PROFILE			0
#define PROFILE_1				1
#define PROFILE_2				2
#define ADVANCED_PROFILE		3

// Loop filter options
#define NO_LOOP_FILTER			0
#define LOOP_FILTER_BASIC		2
#define LOOP_FILTER_DERING		3

#define UMV_BORDER              48
#define STRIDE_EXTRA            (UMV_BORDER * 2)
#define BORDER_MBS				(UMV_BORDER>>4)

#define MAX_MV_EXTENT           63      //  Max search distance in half pixel increments
#define MV_ENTROPY_TOKENS       511     
#define LONG_MV_BITS            8

#define PPROC_QTHRESH           64

#define MAX_MODES               10

#define MAX_NEAREST_ADJ_INDEX	2 

#define Y_MVSHIFT       0x2 
#define UV_MVSHIFT      0x3
#define Y_MVMODMASK     0x3
#define UV_MVMODMASK    0x7

//    INT32  MvShift;                 // motion vector shift value
//    INT32  MvModMask; 

// Prediction filter modes:
// Note: when trying to use an enum here we ran into an odd compiler bug in
// the WriteFrameHeader() code. Also an enum type is implicitly an int which 
// is a bit big for something that can only have 3 values
#define BILINEAR_ONLY_PM	    0
#define BICUBIC_ONLY_PM		    1
#define AUTO_SELECT_PM		    2

#define DCProbOffset(A,B) \
	( (A) * (MAX_ENTROPY_TOKENS-1) \
    + (B) )

#define ACProbOffset(A,B,C,D) \
	( (A) * PREC_CASES * VP6_AC_BANDS * (MAX_ENTROPY_TOKENS-1) \
	+ (B) * VP6_AC_BANDS * (MAX_ENTROPY_TOKENS-1) \
	+ (C) * (MAX_ENTROPY_TOKENS-1) \
	+ (D) ) 

#define DcNodeOffset(A,B,C) \
	( (A) * DC_TOKEN_CONTEXTS * CONTEXT_NODES \
	+ (B) * CONTEXT_NODES \
	+ (C) ) 


#define MBOffset(row,col) ( (row) * pbi->MBCols + (col) )

/****************************************************************************
*  Types
****************************************************************************/
typedef enum
{
    CODE_INTER_NO_MV        = 0x0,      // INTER prediction, (0,0) motion vector implied.
    CODE_INTRA              = 0x1,      // INTRA i.e. no prediction.
    CODE_INTER_PLUS_MV      = 0x2,      // INTER prediction, non zero motion vector.
    CODE_INTER_NEAREST_MV   = 0x3,      // Use Last Motion vector
    CODE_INTER_NEAR_MV      = 0x4,      // Prior last motion vector
    CODE_USING_GOLDEN       = 0x5,      // 'Golden frame' prediction (no MV).
    CODE_GOLDEN_MV          = 0x6,      // 'Golden frame' prediction plus MV.
    CODE_INTER_FOURMV       = 0x7,      // Inter prediction 4MV per macro block.
    CODE_GOLD_NEAREST_MV    = 0x8,      // Use Last Motion vector
    CODE_GOLD_NEAR_MV       = 0x9,      // Prior last motion vector
    DO_NOT_CODE             = 0x10       // Fake Mode
} CODING_MODE;

typedef struct
{
    unsigned int FragCodingMode   :  4;
    int          MVectorX         :  8;
    int          MVectorY         :  8;
} FRAG_INFO;

typedef struct _DCINFO
{
    Q_LIST_ENTRY dc;
    short frame;
} DCINFO;

// defined so i don't have to remember which block goes where
typedef enum
{
    TOP_LEFT_Y_BLOCK        = 0,
    TOP_RIGHT_Y_BLOCK       = 1,
    BOTTOM_LEFT_Y_BLOCK     = 2,
    BOTTOM_RIGHT_Y_BLOCK    = 3,
    U_BLOCK                 = 4,
    V_BLOCK                 = 5
} BLOCK_POSITION;

// all the information gathered from a block to be used as context in the next block
typedef struct
{
    UINT8        Token;
    CODING_MODE  Mode;
    UINT16       Frame;
    Q_LIST_ENTRY Dc;
    UINT8        unused[3];
}  BLOCK_CONTEXT;

// all the contexts maintained for a frame
typedef struct
{
    BLOCK_CONTEXT    LeftY[2];   // 1 for each block row in a macroblock
    BLOCK_CONTEXT    LeftU;
    BLOCK_CONTEXT    LeftV;

    BLOCK_CONTEXT   *AboveY;
    BLOCK_CONTEXT   *AboveU;
    BLOCK_CONTEXT   *AboveV;

//    BLOCK_CONTEXT   *AboveYAlloc;
//    BLOCK_CONTEXT   *AboveUAlloc;
//    BLOCK_CONTEXT   *AboveVAlloc;

    Q_LIST_ENTRY     LastDcY[4]; // 1 for each frame 
    Q_LIST_ENTRY     LastDcU[4];
    Q_LIST_ENTRY     LastDcV[4];

} FRAME_CONTEXT;

// Structure to hold last token values at each position in block
typedef UINT8 TOKENBUFFER[256];



typedef struct
{
    INT16 *dequantPtr;
    INT16 *coeffsPtr;
    INT8 *reconPtr;

    INT32  MvShift;                 // motion vector shift value
    INT32  MvModMask;               // motion vector mod mask

    INT32  FrameReconStride;        // Stride of the frame
    INT32  CurrentReconStride;      // pitch of reconstruction

    INT32  CurrentSourceStride;     // pitch of source (compressor only)
	INT32  FrameSourceStride;		// Stride of the frame (compressor only)
    UINT32 Plane;                   // plane block is from (compressor only)

    BLOCK_CONTEXT  *Above;          // above block context
    BLOCK_CONTEXT  *Left;           // left block context
    Q_LIST_ENTRY   *LastDc;         // last dc value seen

    UINT32 thisRecon;               // index for recon
    UINT32 Source;                  // index for source (compressor only)

    UINT32 EobPos;

	UINT8	*BaselineProbsPtr;
	UINT8	*ContextProbsPtr;

	UINT8	*AcProbsBasePtr; 
	UINT8	*DcProbsBasePtr; 
	UINT8	*DcNodeContextsBasePtr; 
    UINT8	*ZeroRunProbsBasePtr;

//    BOOL_CODER *br; 
//    INT32	token;
//    UINT8 *MergedScanOrder;
//    UINT8 *MergedScanOrderPtr;

}BLOCK_DX_INFO;


typedef struct
{
    BOOL_CODER *br;

    BLOCK_DX_INFO blockDxInfo[6];

    CODING_MODE   Mode;             // mode macroblock coded as

//note: these should be moved into blockDxInfo
    CODING_MODE   BlockMode[6];     // mode macroblock coded as
    MOTION_VECTOR Mv[6];            // one motion vector per block u and v calculated from rest


    MOTION_VECTOR NearestInterMVect;// nearest mv in last frame
    MOTION_VECTOR NearInterMVect;   // near mv in last frame
	INT32         NearestMvIndex;   // Indicates how neare nearest is.
    MOTION_VECTOR NearestGoldMVect; // nearest mv in gold frame
    MOTION_VECTOR NearGoldMVect;    // near mv in gold frame
	INT32         NearestGMvIndex;  // Indicates how neare nearest is.

	INT32  Interlaced;				// is the macroblock interlaced?

//    Q_LIST_ENTRY  *CoeffsAlloc;     // coefficients 64 per frag 4 y in raster order, u then v
} MACROBLOCK_INFO;

// Frame Header type
typedef struct FRAME_HEADER
{
    UINT8 *buffer;
    UINT32 value;
    INT32  bits_available;
    UINT32 pos;
} FRAME_HEADER;

typedef struct _BITREADER
{
	int bitsinremainder;				// # of bits still used in remainder
	UINT32 remainder;					// remaining bits from original long
	const unsigned char * position;		// character pointer position within data
} BITREADER;

// Playback Instance Definition
typedef struct PB_INSTANCE
{
	MACROBLOCK_INFO  mbi;		// all the information needed for one macroblock
	FRAME_CONTEXT    fc;		// all of the context information needed for a frame
	QUANTIZER	    *quantizer;

    // Should be able to delete these entries when VP5 complete
	INT32      CodedBlockIndex;		   
	UINT8	  *DataOutputInPtr;		  
    FRAG_INFO *FragInfo;
//    FRAG_INFO *FragInfoAlloc;

    /* Current access points fopr input and output buffers */
    BOOL_CODER br;
	BOOL_CODER br2;
    BITREADER  br3;

	// Decoder and Frame Type Information
	UINT8   Vp3VersionNo;
	UINT8	VpProfile;

	UINT32  PostProcessingLevel;	   /* Perform post processing */
	UINT32  ProcessorFrequency;	   /* CPU frequency	*/
	UINT32  CPUFree;
	UINT8   FrameType;       

	CONFIG_TYPE Configuration;	// frame configuration
	UINT32  CurrentFrameSize;

	UINT32  YPlaneSize;  
	UINT32  UVPlaneSize;  
	UINT32  VFragments;
	UINT32  HFragments;
	UINT32  UnitFragments;
	UINT32  YPlaneFragments;
	UINT32  UVPlaneFragments;
	
	UINT32  ReconYPlaneSize;
	UINT32  ReconUVPlaneSize;
	
	UINT32  YDataOffset;
	UINT32  UDataOffset;
	UINT32  VDataOffset;
	UINT32  ReconYDataOffset;
	UINT32  ReconUDataOffset;
	UINT32  ReconVDataOffset;

	UINT32  MacroBlocks;	// Number of Macro-Blocks in Y component
	UINT32  MBRows;			// Number of rows of MacroBlocks in a Y frame
	UINT32  MBCols;			// Number of cols of MacroBlocks in a Y frame
    UINT32	ScaleWidth;
    UINT32	ScaleHeight;
    UINT32	OutputWidth;
    UINT32	OutputHeight;
	
	// Frame Buffers 
	YUV_BUFFER_ENTRY *ThisFrameRecon;
//	YUV_BUFFER_ENTRY *ThisFrameReconAlloc;
	YUV_BUFFER_ENTRY *GoldenFrame; 
//	YUV_BUFFER_ENTRY *GoldenFrameAlloc; 
	YUV_BUFFER_ENTRY *LastFrameRecon;
//	YUV_BUFFER_ENTRY *LastFrameReconAlloc;
	YUV_BUFFER_ENTRY *PostProcessBuffer;
//	YUV_BUFFER_ENTRY *PostProcessBufferAlloc;
	YUV_BUFFER_ENTRY *ScaleBuffer;     /* new buffer for testing new loop filtering scheme */
//	YUV_BUFFER_ENTRY *ScaleBufferAlloc; 	

    Q_LIST_ENTRY *quantized_list;  
//    INT16		 *ReconDataBuffer;
    INT16		 *ReconDataBuffer[6];
//	INT16		 *ReconDataBufferAlloc;
//	UINT8         FragCoefEOB;	   // Position of last non 0 coef within QFragData
	INT16		 *TmpReconBuffer;
//	INT16		 *TmpReconBufferAlloc;
	INT16		 *TmpDataBuffer;
//	INT16		 *TmpDataBufferAlloc;
    
//	UINT8		 *LoopFilteredBlockAlloc;
	UINT8		 *LoopFilteredBlock;

    void (**idct)(INT16 *InputData, INT16 *QuantMatrix, INT16 * OutputData );

	POSTPROC_INST    postproc;

	TOKENBUFFER LastToken;			// LTIndex of tokens at each position in block

    CODING_MODE      LastMode;      // Last Mode decoded;

	UINT8 DcProbs[2*(MAX_ENTROPY_TOKENS-1)];
	UINT8 AcProbs[2*PREC_CASES*VP6_AC_BANDS*(MAX_ENTROPY_TOKENS-1)];

                               //3             MAX_ENTROPY_TOKENS-7                     
//	UINT8 DcNodeContexts[2][DC_TOKEN_CONTEXTS][CONTEXT_NODES];								// Plane, Contexts, Node
	UINT8 DcNodeContexts[2 * DC_TOKEN_CONTEXTS * CONTEXT_NODES];								// Plane, Contexts, Node
	
	UINT8 ZeroRunProbs[ZRL_BANDS][ZERO_RUN_PROB_CASES];

    UINT8 MergedScanOrder[BLOCK_SIZE + 65];
	UINT8 ModifiedScanOrder[BLOCK_SIZE];
	UINT8 EobOffsetTable[BLOCK_SIZE];
	UINT8 ScanBands[BLOCK_SIZE];

    UINT8 MBModeProb[11];
    UINT8 BModeProb[11];

	UINT8  PredictionFilterMode;
	UINT8  PredictionFilterMvSizeThresh;
	UINT32 PredictionFilterVarThresh;
	UINT8  PredictionFilterAlpha;
	
	BOOL   RefreshGoldenFrame;

	UINT8 Inter00Prob;
	UINT32 AvgFrameQIndex;

	BOOL testMode;

    UINT32 mvNearOffset[16];
	
	int probInterlaced;
	char *MBInterlaced;
	char *predictionMode;
	MOTION_VECTOR *MBMotionVector;
//	char *MBInterlacedAlloc;
//	char *predictionModeAlloc;
//	MOTION_VECTOR *MBMotionVectorAlloc;

	UINT8  MvSignProbs[2];
	UINT8  IsMvShortProb[2];
	UINT8  MvShortProbs[2][7];
	UINT8  MvQPelProbs[2];
	UINT8  MvHalfPixelProbs[2];
	UINT8  MvLowBitProbs[2];
	UINT8  MvSizeProbs[2][LONG_MV_BITS];

	UINT8 probXmitted[4][2][MAX_MODES];
	UINT8 probModeSame[4][MAX_MODES];
	UINT8 probMode[4][MAX_MODES][MAX_MODES-1]; // nearest+near,nearest only, nonearest+nonear, 10 preceding modes, 9 nodes

	UINT32 maxTimePerFrame;
	UINT32 thisDecodeTime;
	UINT32 avgDecodeTime;
	UINT32 avgPPTime[10];
	UINT32 avgBlitTime;

	// Does this frame use multiple data streams
	// Multistream is implicit for SIMPLE_PROFILE
	BOOL   MultiStream;

    // Huffman code tables for DC, AC & Zero Run Length
    UINT32 DcHuffCode[2][MAX_ENTROPY_TOKENS];
    UINT8  DcHuffLength[2][MAX_ENTROPY_TOKENS];
    UINT32 DcHuffProbs[2][MAX_ENTROPY_TOKENS];
    HUFF_NODE DcHuffTree[2][MAX_ENTROPY_TOKENS];

    UINT32 AcHuffCode[PREC_CASES][2][VP6_AC_BANDS][MAX_ENTROPY_TOKENS];
    UINT8  AcHuffLength[PREC_CASES][2][VP6_AC_BANDS][MAX_ENTROPY_TOKENS];
    UINT32 AcHuffProbs[PREC_CASES][2][VP6_AC_BANDS][MAX_ENTROPY_TOKENS];
    HUFF_NODE AcHuffTree[PREC_CASES][2][VP6_AC_BANDS][MAX_ENTROPY_TOKENS];

    UINT32 ZeroHuffCode[ZRL_BANDS][ZERO_RUN_PROB_CASES];
    UINT8  ZeroHuffLength[ZRL_BANDS][ZERO_RUN_PROB_CASES];
    UINT32 ZeroHuffProbs[ZRL_BANDS][ZERO_RUN_PROB_CASES];
    HUFF_NODE ZeroHuffTree[ZRL_BANDS][ZERO_RUN_PROB_CASES];

    /* FAST look-up-table for huffman Trees */
    UINT16 DcHuffLUT[2][1<<HUFF_LUT_LEVELS];
    UINT16 AcHuffLUT[PREC_CASES][2][VP6_AC_BANDS][1<<HUFF_LUT_LEVELS];
    UINT16 ZeroHuffLUT[ZRL_BANDS][1<<HUFF_LUT_LEVELS];

    RAW_BUFFER  HuffBuffer;

    // Second partition buffer details
    FRAME_HEADER Header;
    UINT32 Buff2Offset;

	// Note: Use of huffman codes for DCT data is only allowed 
	// when using multiple data streams / partitions
	BOOL   UseHuffman;	

    // Counters for runs of zeros at DC & EOB at first AC position in Huffman mode
    INT32  CurrentDcRunLen[2];
    INT32  CurrentAc1RunLen[2];

    // Should we do loop filtering.
	// In simple profile this is ignored and there is no loop filtering	
	UINT8  UseLoopFilter;

    // Control of dering loop/prediction filter
	UINT32 DrCutOff;
	UINT32 DrThresh[256];

    UINT32 BlackClamp;
    UINT32 WhiteClamp;

    UINT32 DeInterlaceMode;
	
	UINT32 AddNoiseMode;

} PB_INSTANCE;

/****************************************************************************
*  Exports
****************************************************************************/
extern UINT8        LimitVal_VP31[VAL_RANGE * 3];
extern BOOL         VP6_ModeUsesMC[MAX_MODES]; // table to indicate if the given mode uses motion estimation
extern const int    VP6_Mode2Frame[DO_NOT_CODE];
extern const INT32  VP6_CoeffToBand[65];
extern const UINT8  DefaultNonInterlacedScanBands[BLOCK_SIZE]; 
extern const UINT8  DefaultInterlacedScanBands[BLOCK_SIZE];

extern PB_INSTANCE *VP6_CreatePBInstance ( void );
extern void		    VP6_DeletePBInstance ( PB_INSTANCE** );
extern BOOL	        VP6_LoadFrame ( PB_INSTANCE *pbi );
extern void	        VP6_SetFrameType ( PB_INSTANCE *pbi, UINT8 FrType );
extern UINT8        VP6_GetFrameType ( PB_INSTANCE *pbi );
extern BOOL	        VP6_InitFrameDetails ( PB_INSTANCE *pbi );
extern void	        VP6_ErrorTrap ( PB_INSTANCE *pbi, int ErrorCode );
extern BOOL	        VP6_AllocateFragmentInfo ( PB_INSTANCE *pbi );
extern BOOL	        VP6_AllocateFrameInfo ( PB_INSTANCE *pbi, unsigned int FrameSize );
extern void	        VP6_DeleteFragmentInfo ( PB_INSTANCE *pbi );
extern void	        VP6_DeleteFrameInfo ( PB_INSTANCE *pbi );
extern void	        VP6_DMachineSpecificConfig ( void );
extern UINT32	    VP6_bitread1 ( BOOL_CODER *br ) ;
extern UINT32	    VP6_bitread ( BOOL_CODER *br, int bits );
extern void         vp6_appendframe ( PB_INSTANCE *pbi );
extern void		    VP6_readTSC ( unsigned long *tsc );
extern void         VP6_ConfigureContexts ( PB_INSTANCE *pbi );
extern void         VP6_ResetAboveContext ( PB_INSTANCE *pbi );
extern void         VP6_ResetLeftContext ( PB_INSTANCE *pbi );
extern void         VP6_UpdateContext ( PB_INSTANCE *pbi, BLOCK_CONTEXT *c, BLOCK_POSITION bp );
extern void         VP6_UpdateContextA ( PB_INSTANCE *pbi, BLOCK_CONTEXT *c, BLOCK_POSITION bp );

extern void         VP6_PredictDC ( PB_INSTANCE *pbi, BLOCK_POSITION bp );
extern void         VP6_PredictDC_MB ( PB_INSTANCE *pbi );

extern void         VP6_ReconstructBlock ( PB_INSTANCE *pbi, BLOCK_POSITION bp );
//extern void         VP6_ReconstructMacroBlock ( PB_INSTANCE *pbi);
extern void			VP6_PredictFilteredBlock(PB_INSTANCE* pbi, INT16* OutputPtr, UINT32 bp);                    
#endif              

