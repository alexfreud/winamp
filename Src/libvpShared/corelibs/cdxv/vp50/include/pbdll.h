/****************************************************************************
*
*   Module Title :     PBDLL
*
*   Description  :     Video CODEC DEMO playback dll header
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.40 YWX 17-Dec-02 Added DeInteralceMode
*   1.39 YWX 06-Nov-01 Changed to align the MB coeffs buffer memory
*   1.38 AWG 22-MAY-01 Added support for DCT16
*   1.37 JBB 01-MAY-01 Added features to support vp5
*   1.36 JBB 06-Apr-01 Added cpufree variable
*   1.35 JBB 23-Mar-01 New data structure defined for DC prediction
*   1.34 JBX 22-Mar-01 Merged with vp4-mapca bitstream
*   1.33.PGW 08 Feb 01 Added LastFrameQIndex.
*   1.32 PGW 25 Jan 01 Changes to support new motion vector entropy coding in VP5.
*   1.31 JBB 26-JAN-01 Fixes for New Huffman Strategy
*   1.30 YWX 27-Nov-00 Added function Pointers for simple deblocker, i.e.
*                      Deblocking filter for low end machines
*   1.29 YWX 02-Nov-00 Added function pointers for new loopfilter
*   1.28 PGW 16 Nov 00 Deleted redundant data structures.
*                      Added BlockPatternPredictor.
*   1.27 YWX 02-Nov-00 Added function pointers for new loopfilter
*   1.26 YWX 19_Oct-00 Added function pointers for 1-2 scaling
*   1.25 JBB 17-oct-00 Ifdefs around version information
*   1.24 YWX 17-Oct-00 Added *FragCoordinates for new loop filter strategy
*   1.23 PGW 15 Oct 00 Added select_InterUV_quantiser() and related data structures.
*   1.22 PGW 11 Oct 00 Added CreateHuffmanTrees() and DestroyHuffmanTrees()
*                      Added void SelectHuffmanSet() and Huffman selector variables.
*   1.23 YWX 11-Oct-00 Added LastFrameNoMvRecon and LastFrameNoMvReconAlloc
*   1.22 YWX 04 Oct 00 Merged scaling and new loop filtering code
*   1.21 YWX 06 Sep 00 Added new deringing functions pointers
*   1.21 PGW 18 Sep 00 QThreshTable[] made instance specific.
*                      Added InitQTables().
*   1.20 JBB 25 Aug 00 Versioning differences
*   1.19 JBB 21 Aug 00 New More Blurry in high variance area deringer
*   1.18 YWX 2  Aug 00 Added function pointers for Postproc
*   1.17 JBB 28 Jul 00 Added Fragment Variance Value for eliminating deringer
*                      in some cases...
*   1.16 JBB 27 Jul 00 Moved kernel modifiers to pbi, malloc checks
*   1.15 SJL 24Jul00   Changes for Mac
*   1.14 YWX 15/05/00  More variable and function pointersf for postprocessor
*   1.13 YWX 08/05/00  Added #ifdef s and function pointers for postprocessor
*   1.12 JYX 05/05/00  Added PostProcessing (PostProcessBuffer + PostProcessLevel)
*   1.11 SJL 20/04/00  Added ability to enable new dequant code for the dxer.
*   1.10 JYX 06/04/00  Alligned Small Buffers & Live Codec Reordering
*   1.09 SJL 22/03/00  Added func ptr for the loop filter.
*   1.08 JBB 20/03/00  32 Byte alligned buffers, Back to Integer Forward DCT
*                      Additional function pointers for optimized code
*   1.07 PGW 20/03/00  Removed InterIntra.
*   1.06 PGW 17/03/00  Changes to support seperate Y and UV entropy tables.
*   1.05 JBB 29/01/00  Removed Globals added Playback only function externs !
*   1.04 PGW 17/12/99  Draw dib functionality removed.
*   1.03 PGW 22/11/99  Changes relating to restructuring of block map stuff.
*   1.02 PGW 15/07/99  Added bit extraction variables.
*   1.01 PGW 09/07/99  Added code to support profile timing
*   1.00 PGW 28/06/99  New Configuration baseline.
*
*****************************************************************************
*/

#ifndef __INC_PBDLL_H
#define __INC_PBDLL_H


#define VAL_RANGE   256


#include "codec_common.h"
#include "huffman.h"
#include "tokenentropy.h"
#include "vfw_pb_interface.h"
#include "postproc_if.h"
#include "vputil_if.h"
#include "quantize.h"
#include "boolhuff.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/

#ifdef MAPCA
// switch to turn on the Data streamer
#define DMAREADREFERENCE
#define DMAWRITERECON
#define RECONSTRUCTMBATONCE

#define __inline
#endif
/****************************************************************************
*  Types
*****************************************************************************
*/

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
    unsigned int DisplayFragment  :  1;
    unsigned int FragCodingMode   :  4;
    int          MVectorX         :  6;
    int          MVectorY         :  6;
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
    UINT8        Tokens[64];
    CODING_MODE  Mode;
    UINT16       Frame;
    Q_LIST_ENTRY Dc;
    UINT32       EOBPos;
    UINT32       unused;
}  BLOCK_CONTEXT;

typedef struct
{
    UINT32       EOBPos;
    CODING_MODE  Mode;
    UINT16       Frame;
    Q_LIST_ENTRY Dc;
    UINT8        Tokens[1];
    UINT8        unused[3];
}  BLOCK_CONTEXTA;

typedef struct
{
	INT16		x;
	INT16		y;

} MOTION_VECTORA;

// all the contexts maintained for a frame
typedef struct
{
    BLOCK_CONTEXT    LeftY[2];   // 1 for each block row in a macroblock
    BLOCK_CONTEXT    LeftU;
    BLOCK_CONTEXT    LeftV;

    BLOCK_CONTEXTA   *AboveY;
    BLOCK_CONTEXTA   *AboveU;
    BLOCK_CONTEXTA   *AboveV;

    BLOCK_CONTEXTA   *AboveYAlloc;
    BLOCK_CONTEXTA   *AboveUAlloc;
    BLOCK_CONTEXTA   *AboveVAlloc;

    Q_LIST_ENTRY     LastDcY[3]; // 1 for each frame 
    Q_LIST_ENTRY     LastDcU[3];
    Q_LIST_ENTRY     LastDcV[3];

} FRAME_CONTEXT;

// Structure to hold last token values at each position in block
typedef UINT8 TOKENBUFFER[256];

//#define BIT_STATS				1
#ifdef BIT_STATS
#define BIT_STAT_CATEGORIES		8

extern	UINT32 BitStats[BIT_STAT_CATEGORIES];
extern 	UINT8  BitStatCategory;
#endif

typedef struct
{
    Q_LIST_ENTRY  (*CoeffsAlloc)[72];   // coefficients 64 per frag 4 y in raster order, u then v
    Q_LIST_ENTRY  (*Coeffs)[72];    // coefficients 64 per frag 4 y in raster order, u then v
    CODING_MODE   Mode;             // mode macroblock coded as
    CODING_MODE   BlockMode[6];     // mode macroblock coded as
    MOTION_VECTOR Mv[6];            // one motion vector per block u and v calculated from rest

    MOTION_VECTOR NearestInterMVect;// nearest mv in last frame
    MOTION_VECTOR NearInterMVect;   // near mv in last frame
    MOTION_VECTOR NearestGoldMVect; // nearest mv in gold frame
    MOTION_VECTOR NearGoldMVect;    // near mv in gold frame
    UINT32 MBrow;                   // mb row
    UINT32 MBcol;                   // mb col

    BLOCK_POSITION bp;              // block number 0 - 5
    UINT32 Source;                  // address for source (compressor only)
    UINT32 SourceY;                 // starting row
    UINT32 SourceX;                 // starting column
    INT32  CurrentSourceStride;     // pitch of source (compressor only)
    UINT32 Recon;                   // address in reconstruction buffer of block
    INT32  CurrentReconStride;      // pitch of reconstruction
    UINT32 Plane;                   // plane block is from
    INT32  MvShift;                 // motion vector shift value
    INT32  MvModMask;               // motion vector mod mask
	INT32  FrameSourceStride;		// Stride of the frame
	INT32  FrameReconStride;		// Stride of the frame

#ifdef RECONSTRUCTMBATONCE
    UINT32 ReconIndex[6];           // ReconIndex for each block
#endif

	UINT32 SourcePtr[6];			// address for source (compressor only)
	UINT32 ReconPtr[6];				// address for source (compressor only)
	UINT32 StripPtr[6];
#ifdef DMAREADREFERENCE
    INT32 Offset[6];
    UINT32 BoundaryX[6];
    UINT32 BoundaryY[6];
#endif
    BLOCK_CONTEXTA  *Above;          // above block context
    BLOCK_CONTEXT  *Left;           // left block context
    Q_LIST_ENTRY   *LastDc;         // last dc value seen

	INT32  Interlaced;				// is the macroblock interlaced?

} MACROBLOCK_INFO;

/****************************************************************************
*  MACROS
*****************************************************************************
*/

// Enumeration of how block is coded
#define CURRENT_ENCODE_VERSION  5
#define CURRENT_DECODE_VERSION  5

#define UMV_BORDER              32
#define STRIDE_EXTRA            (UMV_BORDER * 2)


#define MAX_MV_EXTENT           31      //  Max search distance in half pixel increments
#define MV_ENTROPY_TABLES       16
#define MV_ENTROPY_TOKENS       ((MAX_MV_EXTENT * 2) + 1)

#define PPROC_QTHRESH           64

#define MAX_MODES               10

#define DCT_KEY_FRAME           0

#define DEFAULT_HALF_PIXEL_PROB 85


#define DCProbOffset(A,B) \
	( (A) * (MAX_ENTROPY_TOKENS-1) \
    + (B) )

#define DCContextOffset(A,B,C,D) \
	( (A) * TOKEN_CONTEXTS * TOKEN_CONTEXTS * CONTEXT_NODES \
	+ (B) * TOKEN_CONTEXTS * CONTEXT_NODES \
	+ (C) * CONTEXT_NODES \
	+ (D) )

#define ACProbOffset(A,B,C,D) \
	( (A) * PREC_CASES * VP5_AC_BANDS * (MAX_ENTROPY_TOKENS-1) \
	+ (B) * VP5_AC_BANDS * (MAX_ENTROPY_TOKENS-1) \
	+ (C) * (MAX_ENTROPY_TOKENS-1) \
	+ (D) ) 


#define ACContextOffset(A,B,C,D,E) \
	( (A) * PREC_CASES * (VP5_AC_BANDS-3) * TOKEN_CONTEXTS * CONTEXT_NODES \
	+ (B) * (VP5_AC_BANDS-3) * TOKEN_CONTEXTS * CONTEXT_NODES \
	+ (C) * TOKEN_CONTEXTS * CONTEXT_NODES \
	+ (D) * CONTEXT_NODES \
	+ (E) )

#define MBOffset(row,col) ( (row) * pbi->MBCols + (col) )

/****************************************************************************
*  Global Variables
*****************************************************************************
*/
extern UINT8 LimitVal_VP31[VAL_RANGE * 3];

extern BOOL VP5_ModeUsesMC[MAX_MODES]; // table to indicate if the given mode uses motion estimation

extern const int VP5_Mode2Frame[DO_NOT_CODE];

extern const INT32  CoeffToBand[65];

//****************************************************************
// Function Pointers some probably could be library globals!
// all the information we ever need about a macroblock

typedef struct PB_INSTANCE
{
	// Should be able to delete these entries when VP5 complete
	INT32   CodedBlockIndex;		   
	UINT8	  *DataOutputInPtr;		  
    FRAG_INFO *FragInfo;
    FRAG_INFO *FragInfoAlloc;


    /* Current access points fopr input and output buffers */
    BOOL_CODER br;

	//****************************************************************************************
	// Decoder and Frame Type Information
	UINT8   Vp3VersionNo;
    UINT32  DeInterlaceMode;
	UINT32  PostProcessingLevel;	   /* Perform post processing */
	UINT32  ProcessorFrequency;	   /* CPU frequency	*/
	UINT32  CPUFree;
	UINT8   FrameType;       
	UINT8   KeyFrameType;
	//****************************************************************************************

	//****************************************************************************************
	// Frame Size & Index Information

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
	UINT32  OutputStride;
	
	//****************************************************************************************

	//****************************************************************************************
	// Frames 
	YUV_BUFFER_ENTRY *ThisFrameRecon;
	YUV_BUFFER_ENTRY *ThisFrameReconAlloc;
	YUV_BUFFER_ENTRY *GoldenFrame; 
	YUV_BUFFER_ENTRY *GoldenFrameAlloc; 
	YUV_BUFFER_ENTRY *LastFrameRecon;
	YUV_BUFFER_ENTRY *LastFrameReconAlloc;
	YUV_BUFFER_ENTRY *PostProcessBuffer;
	YUV_BUFFER_ENTRY *PostProcessBufferAlloc;
	YUV_BUFFER_ENTRY *ScaleBuffer;     /* new buffer for testing new loop filtering scheme */
	YUV_BUFFER_ENTRY *ScaleBufferAlloc; 	
	//****************************************************************************************

	//****************************************************************************************
	Q_LIST_ENTRY *quantized_list;  
#ifdef RECONSTRUCTMBATONCE
    INT16		 (*ReconDataBuffer)[64];
	INT16		 (*ReconDataBufferAlloc)[64];
#else
    INT16		 *ReconDataBuffer;
	INT16		 *ReconDataBufferAlloc;
#endif
	UINT8         FragCoefEOB;	   // Position of last non 0 coef within QFragData
	INT16		 *TmpReconBuffer;
	INT16		 *TmpReconBufferAlloc;
	INT16		 *TmpDataBuffer;
	INT16		 *TmpDataBufferAlloc;
    
	UINT8		 *LoopFilteredBlockAlloc;
	UINT8		 *LoopFilteredBlock;

#ifdef DMAREADREFERENCE
    UINT8  (*ReferenceBlocksAlloc)[192];
    UINT8  (*ReferenceBlocks)[192]; // Six Reference Blocks
    UINT32  mvX[6], mvY[6];    
#endif

#ifdef DMAWRITERECON    
    #ifdef RECONSTRUCTMBATONCE
        UINT8		 *ThisBandReconPtr[6];			//Current Band to write to
    #else
        UINT8		 *ThisBandReconPtr;				//Current Band to write to
    #endif
	UINT8 		 *ReconstructedMBs;			    //bandbuffer for DMA reconstructed MB row.
    UINT8        *ReconstructedMBsAlloc;       
	UINT8        *FillMem;
#endif    
    //****************************************************************

	void (**idct)(INT16 *InputData, INT16 *QuantMatrix, INT16 * OutputData );

	POSTPROC_INST    postproc;
	QUANTIZER	    *quantizer;
	MACROBLOCK_INFO  mbi;		// all the information needed for one macroblock
	FRAME_CONTEXT    fc;		// all of the context information needed for a frame

	TOKENBUFFER LastToken;			// LTIndex of tokens at each position in block

    CODING_MODE      LastMode;      // Last Mode decoded;

	UINT8 DcProbs[2*(MAX_ENTROPY_TOKENS-1)];
	UINT8 AcProbs[2*PREC_CASES*VP5_AC_BANDS*(MAX_ENTROPY_TOKENS-1)];
	UINT8 DcNodeContexts[2*TOKEN_CONTEXTS*TOKEN_CONTEXTS*CONTEXT_NODES];									// Plane, Node, Contexts, Contexts
	UINT8 AcNodeContexts[2*PREC_CASES*(VP5_AC_BANDS-3)*TOKEN_CONTEXTS*CONTEXT_NODES];						// Prec, Plane, AcBand, Context, Node
	
	UINT8 ZeroCount;
    UINT8 MBModeProb[11];
    UINT8 BModeProb[11];

	UINT8 Inter00Prob;
	UINT32 AvgFrameQIndex;

	BOOL testMode;

    UINT32 mvNearOffset[16];
	
	int probInterlaced;
	char *MBInterlaced;
	char *predictionMode;
	MOTION_VECTORA *MBMotionVector;
	char *MBInterlacedAlloc;
	char *predictionModeAlloc;
	MOTION_VECTORA *MBMotionVectorAlloc;

	UINT8  MvSignProbs[2];
	UINT8  MvZeroProbs[2];
	UINT8  MvHalfPixelProbs[2];
	UINT8  MvLowBitProbs[2];
	UINT8  MvSizeProbs[2][((MAX_MV_EXTENT+1) >> 2) - 1];

	UINT8 probXmitted[4][2][MAX_MODES];
	UINT8 probModeSame[4][MAX_MODES];
	UINT8 probMode[4][MAX_MODES][MAX_MODES-1]; // nearest+near,nearest only, nonearest+nonear, 10 preceding modes, 9 nodes

	UINT32 maxTimePerFrame;
	UINT32 thisDecodeTime;
	UINT32 avgDecodeTime;
	UINT32 avgPPTime[10];
	UINT32 avgBlitTime;
    UINT32 BlackClamp;
    UINT32 WhiteClamp;

} PB_INSTANCE;

/****************************************************************************
*  Functions.
*****************************************************************************
*/
//****************************************************************
// Function Pointers now library globals!
//extern void (*ReadTokens)( xPB_INST pbi, UINT32 BlockSize, UINT32 Hpos );

//****************************************************************
extern PB_INSTANCE * VP5_CreatePBInstance(void);
extern void			 VP5_DeletePBInstance(PB_INSTANCE **);
extern BOOL			 VP5_LoadFrame(PB_INSTANCE *pbi);
extern void			 VP5_SetFrameType(PB_INSTANCE *pbi, UINT8 FrType );
extern UINT8		 VP5_GetFrameType(PB_INSTANCE *pbi);
extern BOOL			 VP5_InitFrameDetails(PB_INSTANCE *pbi);
extern void			 VP5_ErrorTrap( PB_INSTANCE *pbi, int ErrorCode );
extern BOOL			 VP5_AllocateFragmentInfo(PB_INSTANCE * pbi);
extern BOOL			 VP5_AllocateFrameInfo(PB_INSTANCE * pbi, unsigned int FrameSize);
extern void			 VP5_DeleteFragmentInfo(PB_INSTANCE * pbi);
extern void			 VP5_DeleteFrameInfo(PB_INSTANCE * pbi);
extern void			 VP5_DMachineSpecificConfig(void);

INLINE UINT32 VP5_bitread1(BOOL_CODER *br) 
{
	return (DecodeBool128(br));
}
INLINE UINT32 VP5_bitread(BOOL_CODER *br, int bits)
{
	UINT32 z = 0;
	int bit;
	for(bit=bits-1;bit>=0;bit--)
	{
		z|=(DecodeBool128(br)<<bit);
	}
	return z;
}
extern void          vp5_appendframe(PB_INSTANCE *pbi);
extern void			 VP5_readTSC(unsigned long *tsc);
extern void ConfigureContexts(PB_INSTANCE *pbi);

//  dx\generic\decodembs.c
extern void ResetAboveContext(PB_INSTANCE* pbi);
extern void ResetLeftContext(PB_INSTANCE* pbi);
extern void UpdateContext(PB_INSTANCE* pbi, BLOCK_CONTEXT* c, BLOCK_POSITION bp);
extern void UpdateContextA(PB_INSTANCE* pbi, BLOCK_CONTEXTA* c, BLOCK_POSITION bp);
extern void PredictDC(PB_INSTANCE* pbi, BLOCK_POSITION bp, Q_LIST_ENTRY* LastDC, BLOCK_CONTEXTA* Above, BLOCK_CONTEXT* Left);

//  dx\generic\recon.c
extern void ReconstructBlock(PB_INSTANCE* pbi, BLOCK_POSITION bp);

//  dx\generic\decodemode.c
extern CODING_MODE DecodeBlockMode(PB_INSTANCE *pbi);
extern CODING_MODE DecodeMode(PB_INSTANCE *pbi, CODING_MODE lastmode, UINT32 type);

#endif
