/****************************************************************************
*
*   Module Title :     preproc.h
*
*   Description  :     Content analysis module header
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.08 PGW 27 Apr 01  Removed code to set Y from UV
*   1.07 PGW 28 Feb 01  Removal of history buffer functionality.
*   1.06 PGW 04 Oct 00  Added CANDIDATE_BLOCK_LOW.
*   1.05 PGW 24 Jul 00  Added Column SAD functions. Deleted BeThreshold.
*   1.04 PGW 13 Jul 00	Added BLOCK_CODED_LOW. Deleted BLOCK_CODED_EXTRA.
*   1.03 PGW 10 Jul 00  Added lookup tables to reduce number of conditionals 
*						in RowDiffScan(). Removed old "ifdef 0"ed code.
*						Added KFIndicator.
*   1.02 JBB 30/05/00   Removed hard coded size limits
*   1.01 YX  06/04/00   Added XMMEnabled for optimizations
*   1.00 PGW 16/06/96   Configuration baseline.
*
*****************************************************************************
*/						

#include "preprocconf.h"
#include "type_aliases.h"
#include "preprocif.h"

/* Constants. */
#define OUTPUT_BLOCK_HEIGHT		8
#define OUTPUT_BLOCK_WIDTH		8

#define INTERNAL_BLOCK_HEIGHT   8
#define INTERNAL_BLOCK_WIDTH	8

#define FILTER_BLOCK_SIZE		(INTERNAL_BLOCK_WIDTH * INTERNAL_BLOCK_HEIGHT)

/* NEW Line search values. */ 
#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3

/* Low Pass Filter levels. */
#define NO_LOW_PASS         0
#define VERY_LOW_LOW_PASS   1               
#define LOW_LOW_PASS        2               
#define MODERATE_LOW_PASS   5               
#define HIGH_LOW_PASS       7               
#define VERY_HIGH_LOW_PASS  9    

#define FIRST_ROW           0
#define NOT_EDGE_ROW        1
#define LAST_ROW            2      

#define YDIFF_CB_ROWS			(INTERNAL_BLOCK_HEIGHT * 3)
#define CHLOCALS_CB_ROWS		(INTERNAL_BLOCK_HEIGHT * 3)
#define PMAP_CB_ROWS			(INTERNAL_BLOCK_HEIGHT * 3)
#define FRAG_PIXEL_DIFF_ROWS	(INTERNAL_BLOCK_HEIGHT * 3)
#define PSCORE_CB_ROWS			(INTERNAL_BLOCK_HEIGHT * 4)

#define PIXEL_SCORES_BUFFER_SIZE	SCAN_MAX_LINE_LENGTH * PSCORE_CB_ROWS

#define YUV_DIFFS_CIRC_BUFFER_SIZE	(SCAN_MAX_LINE_LENGTH * YDIFF_CB_ROWS)
#define CH_LOCALS_CIRC_BUFFER_SIZE	(SCAN_MAX_LINE_LENGTH * CHLOCALS_CB_ROWS)
#define PIXEL_MAP_CIRC_BUFFER_SIZE  (SCAN_MAX_LINE_LENGTH * PMAP_CB_ROWS)

// Status values in block coding map
#define CANDIDATE_BLOCK_LOW					-2
#define CANDIDATE_BLOCK						-1
#define BLOCK_NOT_CODED						0
#define BLOCK_CODED_BAR 					3		
#define BLOCK_ALREADY_MARKED_FOR_CODING		4
#define BLOCK_CODED_SGC						4	
#define BLOCK_CODED_LOW						4	
#define BLOCK_CODED 						5	

#define MAX_PREV_FRAMES             16
#define MAX_SEARCH_LINE_LEN 7   

/******************************************************************/
/* Type definitions. */
/******************************************************************/
#define blockCoded(i) (ppi->ScanConfig.FragInfo[(i)*ppi->ScanConfig.FragInfoElementSize]&ppi->ScanConfig.FragInfoCodedMask)
#define setBlockCoded(i) ppi->ScanConfig.FragInfo[(i)*ppi->ScanConfig.FragInfoElementSize]|=ppi->ScanConfig.FragInfoCodedMask;
#define setBlockUncoded(i) ppi->ScanConfig.FragInfo[(i)*ppi->ScanConfig.FragInfoElementSize]&=(~ppi->ScanConfig.FragInfoCodedMask);

typedef struct PP_INSTANCE
{
     UINT32 *ScanPixelIndexTableAlloc;		
     INT8   *ScanDisplayFragmentsAlloc;

     UINT32 *FragScoresAlloc;               // The individual frame difference ratings.    
     INT8   *SameGreyDirPixelsAlloc;
     INT8   *BarBlockMapAlloc;

     // Number of pixels changed by diff threshold in row of a fragment. 
     UINT8  *FragDiffPixelsAlloc;  

     UINT8  *PixelScoresAlloc;  
     UINT8  *PixelChangedMapAlloc;
     UINT8  *ChLocalsAlloc;
     INT16  *yuv_differencesAlloc;  
     INT32  *RowChangedPixelsAlloc;
	 INT8   *TmpCodedMapAlloc;

     UINT32 *ScanPixelIndexTable;		
     INT8   *ScanDisplayFragments;

     UINT32 *FragScores;               // The individual frame difference ratings.    
     INT8   *SameGreyDirPixels;
     INT8   *BarBlockMap;

     // Number of pixels changed by diff threshold in row of a fragment. 
     UINT8  *FragDiffPixels;  

     UINT8  *PixelScores;  
     UINT8  *PixelChangedMap;
     UINT8  *ChLocals;
     INT16  *yuv_differences;  
     INT32  *RowChangedPixels;
	 INT8   *TmpCodedMap;

     // Plane pointers and dimension variables
     UINT8 * YPlanePtr0;
     UINT8 * YPlanePtr1;
     UINT8 * UPlanePtr0;
     UINT8 * UPlanePtr1;
     UINT8 * VPlanePtr0;
     UINT8 * VPlanePtr1;

     UINT32  VideoYPlaneWidth;
     UINT32  VideoYPlaneHeight;
     UINT32  VideoUVPlaneWidth;
     UINT32  VideoUVPlaneHeight;

     UINT32  VideoYPlaneStride;
     UINT32  VideoUPlaneStride;
     UINT32  VideoVPlaneStride;

/* Scan control variables. */
     UINT8   HFragPixels;
     UINT8   VFragPixels;

     UINT32  ScanFrameFragments;
     UINT32  ScanYPlaneFragments;
     UINT32  ScanUVPlaneFragments;
     UINT32  ScanHFragments;
     UINT32  ScanVFragments;

     UINT32  YFramePixels; 
     UINT32  UVFramePixels; 
     UINT32  TotFramePixels;

     BOOL	   SgcOnOff;

     UINT32  SgcThresh;

     UINT32  OutputBlocksUpdated;
	 UINT32  KFIndicator;

     BOOL	   ScanSRF_Enabled;

/* The VCAP scan configuration. */
     SCAN_CONFIG_DATA ScanConfig;

     BOOL    VcapOn;

     INT32 SRFGreyThresh;
     INT32 SRFColThresh;
     INT32 SgcLevelThresh;
     INT32 SuvcLevelThresh;

     INT32 SRFGreyThreshOffset;
     INT32 SRFColThreshOffset;
     INT32 SgcLevelThreshOffset;
     INT32 SuvcLevelThreshOffset;

     UINT32 NoiseSupLevel;

	/* Block Thresholds. */
     UINT32 PrimaryBlockThreshold;

     INT32  SRFLevel;
     INT32  SRFLevelOffset;

     BOOL   PAKEnabled;

     BOOL   EBO_Enabled;
     BOOL   CategorisationEnabled;

     int    LevelThresh; 
     int    NegLevelThresh; 
     int    SrfThresh;
     int    NegSrfThresh;
     int    HighChange;
     int    NegHighChange;     

     // Threshold lookup tables
	 UINT8 SrfPakThreshTable[512];
	 UINT8 * SrfPakThreshTablePtr;
	 UINT8 SrfThreshTable[512];
	 UINT8 * SrfThreshTablePtr;
	 UINT8 SgcThreshTable[512];
	 UINT8 * SgcThreshTablePtr;

     // Variables controlling S.A.D. break outs.
     UINT32 GrpLowSadThresh;
     UINT32 GrpHighSadThresh;
     UINT32 ModifiedGrpLowSadThresh;
     UINT32 ModifiedGrpHighSadThresh;

     INT32  PlaneHFragments;
     INT32  PlaneVFragments;
     INT32  PlaneHeight;
     INT32  PlaneWidth;
     INT32  PlaneStride;

     UINT32 BlockThreshold;
     UINT32 BlockSgcThresh;
     double UVBlockThreshCorrection;
     double UVSgcCorrection;

     UINT32 SpeedCritical;

// Live test harness specific.

// PC specific variables
	BOOL  MmxEnabled;
	BOOL  XmmEnabled;

	double YUVPlaneCorrectionFactor;	
	double AbsDiff_ScoreMultiplierTable[256];
	UINT8  NoiseScoreBoostTable[256];
	UINT8  MaxLineSearchLen;

	INT32 YuvDiffsCircularBufferSize;
	INT32 ChLocalsCircularBufferSize;
	INT32 PixelMapCircularBufferSize;

	// Temp stats variable
	UINT32 TotBlocksUpdated;

	// Function pointers for mmx switches
	UINT32 (*RowSAD)(UINT8 *, UINT8 * );            
	UINT32 (*ColSAD)(xPP_INST ppi, UINT8 *, UINT8 * );            

} PP_INSTANCE;

/******************************************************************/
/* Function prototypes. */
/******************************************************************/


INLINE UINT32 ScanGetFragIndex( PP_INSTANCE *ppi, UINT32 FragmentNo )
{   
    return ppi->ScanPixelIndexTable[ FragmentNo ];
}


extern void InitScanMapArrays
(
 PP_INSTANCE *ppi
);

extern void AnalysePlane
(
 PP_INSTANCE *ppi, UINT8 * PlanePtr0, UINT8 * PlanePtr1, UINT32 FragArrayOffset, UINT32 PWidth, UINT32 PHeight, UINT32 PStride 
);

extern void ScanCalcPixelIndexTable
(
 PP_INSTANCE *ppi
);

extern void CreateOutputDisplayMap
(
 PP_INSTANCE *ppi, 
 INT8		 *InternalFragmentsPtr
);

extern void SetVcapLevelOffset
(
 PP_INSTANCE *ppi, INT32 LevelOffset 
);

//  Analysis functions
extern void RowBarEnhBlockMap
(
 PP_INSTANCE *ppi, 
 UINT32 * FragScorePtr, 
 INT8   * FragSgcPtr,
 INT8   * UpdatedBlockMapPtr,
 INT8   * BarBlockMapPtr,
 UINT32 RowNumber 
);

extern void BarCopyBack
(
 PP_INSTANCE *ppi, 
 INT8  * UpdatedBlockMapPtr,
 INT8  * BarBlockMapPtr 
);

// Secondary filter functions
extern UINT8 ApplyLowPass
(
 PP_INSTANCE *ppi, UINT8 * SrcPtr, UINT32 PlaneLineLength, INT32 Level 
);

// PC specific functions
extern void MachineSpecificConfig
(
 
);
extern void ClearMmx
(
 PP_INSTANCE *ppi
);

extern UINT32 ScalarRowSAD
(
 UINT8 * Src1, UINT8 * Src2 
);
extern UINT32 ScalarColSAD
(
 PP_INSTANCE *ppi, UINT8 * Src1, UINT8 * Src2 
);

extern PP_INSTANCE * CreatePPInstance
(
 void
);
extern void DeletePPInstance
(
 PP_INSTANCE **ppi
);




