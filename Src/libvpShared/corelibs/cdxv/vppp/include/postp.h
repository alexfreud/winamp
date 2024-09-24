/****************************************************************************
*
*   Module Title :     postp.h
*
*   Description  :     Post processor interface
*
****************************************************************************/
#ifndef POSTP_H
#define POSTP_H

#include "codec_common.h"

// YUV buffer configuration structure
typedef struct
{
    int     YWidth;
    int     YHeight;
    int     YStride;

    int     UVWidth;
    int     UVHeight;
    int     UVStride;

    char *  YBuffer;
    char *  UBuffer;
    char *  VBuffer;

} YUV_BUFFER_CONFIG;

typedef enum
{
    MAINTAIN_ASPECT_RATIO   = 0x0,
    SCALE_TO_FIT            = 0x1,
    CENTER                  = 0x2,
    OTHER                   = 0x3
} SCALE_MODE;

// macro defined so that I can get the information  from fraginfo ( I suspect this is going to change !) 
// and I wanted to be ready for the change!
#define blockCoded(i) (ppi->FragInfo[(i)*ppi->FragInfoElementSize]&ppi->FragInfoCodedMask)


typedef struct 
{

	// per frame information passed in
	INT32        Vp3VersionNo;			// version of frame
	INT32		 FrameType;				// key or non key
	INT32		 PostProcessingLevel;	// level of post processing to perform 
	INT32		 FrameQIndex;			// q index value used on passed in frame
	UINT8		*LastFrameRecon;		// reconstruction buffer : passed in
	UINT8		*PostProcessBuffer;		// postprocessing buffer : passed in

	// per block information passed in 
	UINT8		*FragInfo;				// blocks coded : passed in
	UINT32       FragInfoElementSize;	// size of each element
	UINT32		 FragInfoCodedMask;		// mask to get at whether fragment is coded

	// per block info maintained by postprocessor
	INT32		*FragQIndex;			// block's q index : allocated and filled
	INT32		*FragmentVariances;		// block's pseudo variance : allocated and filled
	UINT8		*FragDeblockingFlag;	// whether to deblock block : allocated and filled

	// filter specific vars
    INT32		*BoundingValuePtr;		// pointer to a filter     
	INT32		*FiltBoundingValue;		// allocated (512 big)

	// deblocker specific vars
    INT32		*DeblockValuePtr;		// pointer to a filter     
	INT32		*DeblockBoundingValue;	// allocated (512 big)

	
	// frame configuration 
	CONFIG_TYPE  Configuration;			
	UINT32		 ReconYDataOffset;		// position within buffer of first y fragment
	UINT32		 ReconUDataOffset;		// position within buffer of first u fragment
	UINT32		 ReconVDataOffset;		// position within buffer of first v fragment
	UINT32		 YPlaneFragments;		// number of y fragments
	UINT32		 UVPlaneFragments;		// number of u and v fragments
	UINT32		 UnitFragments;			// number of total fragments y+u+v 
	UINT32		 HFragments;			// number of horizontal fragments in y
	UINT32		 VFragments;			// number of vertical fragments in y
	INT32        YStride;				// pitch of y in bytes
	INT32        UVStride;				// pitch of uv in bytes

	// allocs so we can align our ptrs
	INT32		*FiltBoundingValueAlloc;
	INT32		*DeblockBoundingValueAlloc;
	INT32		*FragQIndexAlloc;		
	INT32		*FragmentVariancesAlloc;
	UINT8		*FragDeblockingFlagAlloc;
	UINT32      MVBorder;
    UINT8       *IntermediateBufferAlloc;
    UINT8       *IntermediateBuffer; 
    UINT32      DeInterlaceMode;
    UINT32      AddNoiseMode;

} POSTPROC_INSTANCE;

#define VAL_RANGE   256
extern UINT8 LimitVal_VP31[VAL_RANGE * 3];
typedef POSTPROC_INSTANCE * xPB_INST ;

extern void  (*FilteringVert_12)(UINT32 QValue,UINT8 * Src, INT32 Pitch); 
extern void  (*FilteringHoriz_12)(UINT32 QValue,UINT8 * Src, INT32 Pitch); 
extern void  (*FilteringVert_8)(UINT32 QValue,UINT8 * Src, INT32 Pitch); 
extern void  (*FilteringHoriz_8)(UINT32 QValue,UINT8 * Src, INT32 Pitch); 

extern void  (*CopyBlock) (unsigned char *src, unsigned char *dest, unsigned int srcstride);
extern void  (*VerticalBand_4_5_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
extern void  (*LastVerticalBand_4_5_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
extern void  (*VerticalBand_3_5_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
extern void  (*LastVerticalBand_3_5_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
extern void  (*HorizontalLine_1_2_Scale)(const unsigned char * source,unsigned int sourceWidth,unsigned char * dest,unsigned int destWidth);
extern void  (*HorizontalLine_3_5_Scale)(const unsigned char * source,unsigned int sourceWidth,unsigned char * dest,unsigned int destWidth);
extern void  (*HorizontalLine_4_5_Scale)(const unsigned char * source,unsigned int sourceWidth,unsigned char * dest,unsigned int destWidth);
extern void  (*VerticalBand_1_2_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
extern void  (*LastVerticalBand_1_2_Scale)(unsigned char * dest,unsigned int destPitch,unsigned int destWidth);
extern void  (*FilterHoriz_Simple)(xPB_INST pbi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
extern void  (*FilterVert_Simple)(xPB_INST pbi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
extern void  (*DeringBlockWeak)(xPB_INST, const UINT8 *, UINT8 *, INT32, UINT32, UINT32 *);
extern void  (*DeringBlockStrong)(xPB_INST, const UINT8 *, UINT8 *, INT32, UINT32, UINT32 *);
extern void  (*DeblockLoopFilteredBand)(xPB_INST, UINT8 *, UINT8 *, UINT32, UINT32, UINT32, UINT32 *);
extern void  (*DeblockNonFilteredBand)(xPB_INST, UINT8 *, UINT8 *, UINT32, UINT32, UINT32, UINT32 *);
extern void  (*DeblockNonFilteredBandNewFilter)(xPB_INST, UINT8 *, UINT8 *, UINT32, UINT32, UINT32, UINT32 *);
extern INT32*(*SetupBoundingValueArray)(xPB_INST pbi, INT32 FLimit);
extern INT32*(*SetupDeblockValueArray)(xPB_INST pbi, INT32 FLimit);
extern void  (*FilterHoriz)(xPB_INST pbi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
extern void  (*FilterVert)(xPB_INST pbi, UINT8 * PixelPtr, INT32 LineLength, INT32*);
extern void  (*ClampLevels)( POSTPROC_INSTANCE *pbi,INT32 BlackClamp,	INT32 WhiteClamp, UINT8	*Src, UINT8	*Dst);
extern void  (*FastDeInterlace)(UINT8 *SrcPtr, UINT8 *DstPtr, INT32 Width, INT32 Height, INT32 Stride);  
extern void  (*PlaneAddNoise)( UINT8 *Start, UINT32 Width, UINT32 Height, INT32 Pitch, int q);

extern void  DMachineSpecificConfig(INT32 MmxEnabled, INT32 XmmEnabled, INT32 WmtEnabled);

#endif