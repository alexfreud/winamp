/****************************************************************************
*
*   Module Title :     postproc_if.h
*
*   Description  :     Post-processor interface header file.
*
****************************************************************************/
#ifndef __INC_POSTPROC_IF_H
#define __INC_POSTPROC_IF_H

/****************************************************************************
*  Header Files
****************************************************************************/
#include "codec_common.h"
#include "codec_common_interface.h"

/****************************************************************************
*  Typedefs
****************************************************************************/
typedef struct POSTPROC_INSTANCE * POSTPROC_INST;

/****************************************************************************
*  Imported Functions.
****************************************************************************/
extern void InitPostProcessing
( 
	UINT32 *DCQuantScaleV2p,
	UINT32 *DCQuantScaleUVp,
	UINT32 *DCQuantScaleV1p,
	UINT32 Version
);

extern void DeInitPostProcessing ();

extern POSTPROC_INST CreatePostProcInstance
(
 CONFIG_TYPE *ConfigurationInit		// configuration to setup
);

extern void DeletePostProcInstance
(
 POSTPROC_INST	 *pbi				// postprocessor instance to delete
);

extern void SetPPInterlacedMode(POSTPROC_INST ppi, int Interlaced);
extern void SetDeInterlaceMode(POSTPROC_INST ppi, int DeInterlaceMode);
extern void SetAddNoiseMode(POSTPROC_INST ppi, int AddNoiseMode);

extern void ChangePostProcConfiguration
(
 POSTPROC_INST	pbi,				// postprocessor instance to use	
 CONFIG_TYPE *Configuration			// configuration to change to
);

extern void PostProcess
(
 POSTPROC_INST	 pbi,				// postprocessor instance to use
 INT32       Vp3VersionNo,			// version of frame
 INT32		 FrameType,				// key or non key
 INT32		 PostProcessingLevel,	// level of post processing to perform 
 INT32		 FrameQIndex,			// q index value used on passed in frame
 UINT8		*LastFrameRecon,		// reconstruction buffer : passed in
 UINT8		*PostProcessBuffer,		// postprocessing buffer : passed in
 UINT8		*FragInfo,				// blocks coded : passed in
 UINT32      FragInfoElementSize,	// size of each element
 UINT32		 FragInfoCodedMask		// mask to get at whether fragment is coded
);

extern void (*ClampLevels)
( 
	POSTPROC_INST pbi,
	INT32        BlackClamp,			// number of values to clamp from 0 
	INT32        WhiteClamp,			// number of values to clamp from 255
	UINT8		*Src,					// reconstruction buffer : passed in
	UINT8		*Dst					// postprocessing buffer : passed in
);

extern void LoopFilter
(
 POSTPROC_INST	 pbi,				// postprocessor instance to use
 INT32		 FrameQIndex,			// q index value used on passed in frame
 UINT8		*LastFrameRecon,		// reconstruction buffer : passed in
 UINT8		*PostProcessBuffer,		// postprocessing buffer : passed in
 UINT8		*FragInfo,				// blocks coded : passed in
 UINT32      FragInfoElementSize,	// size of each element
 UINT32		 FragInfoCodedMask		// mask to get at whether fragment is coded
);

extern void ApplyReconLoopFilter
(
 POSTPROC_INST	 pbi,				// postprocessor instance to use
 INT32		 FrameQIndex,			// q index value used on passed in frame
 UINT8		*LastFrameRecon,		// reconstruction buffer : passed in
 UINT8		*PostProcessBuffer,		// postprocessing buffer : passed in
 UINT8		*FragInfo,				// blocks coded : passed in
 UINT32      FragInfoElementSize,	// size of each element
 UINT32		 FragInfoCodedMask		// mask to get at whether fragment is coded
);

extern void ScaleOrCenter
( 
 POSTPROC_INST	 pbi,				// postprocessor instance to use
 UINT8		       *FrameBuffer,	// buffer to use passed in
 YUV_BUFFER_CONFIG * YuvConfig		// size you want to output buffer to
);

/****************************************************************************
*  Exported Functions.
****************************************************************************/
extern void UpdateUMVBorder
( 
 POSTPROC_INST    pbi, 
 UINT8 * DestReconPtr 
);

extern void  (*FilteringVert_12) 
(
 UINT32 QValue,
 UINT8 * Src, 
 INT32 Pitch
); 

extern void  (*FilteringHoriz_12)
(
 UINT32 QValue,
 UINT8 * Src, 
 INT32 Pitch
); 

extern void  (*FilteringVert_8)  
(
 UINT32 QValue,
 UINT8 * Src, 
 INT32 Pitch
); 

extern void  (*FilteringHoriz_8) 
(
 UINT32 QValue,
 UINT8 * Src, 
 INT32 Pitch
); 

extern void CopyFrame( POSTPROC_INST pbi, YUV_BUFFER_CONFIG *b, UINT8 *DestReconPtr);

/****************************************************************************
*  Exported Data.
****************************************************************************/
extern UINT8 LimitVal_VP31[VAL_RANGE * 3];

#endif
