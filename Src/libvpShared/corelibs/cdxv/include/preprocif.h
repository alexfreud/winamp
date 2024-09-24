/****************************************************************************
*
*   Module Title :     preproc_if.h
*
*   Description  :     Pre-processor interface header file.
*
****************************************************************************/						

#ifndef __PREPROC_IF_H
#define __PREPROC_IF_H

/****************************************************************************
*  Header Files
****************************************************************************/
#include "type_aliases.h"

/****************************************************************************
*  Types
****************************************************************************/

typedef struct
{
	UINT8 * Yuv0ptr;
	UINT8 * Yuv1ptr;

	UINT8	*FragInfo;				// blocks coded : passed in
	UINT32   FragInfoElementSize;	// size of each element
	UINT32	 FragInfoCodedMask;		// mask to get at whether fragment is coded

    UINT32 * RegionIndex;           // Gives pixel index for top left of each block 
	UINT32 VideoFrameHeight;
	UINT32 VideoFrameWidth;
	UINT8 HFragPixels;
	UINT8 VFragPixels;

} SCAN_CONFIG_DATA;

typedef enum
{	SCP_FILTER_ON_OFF,
    SCP_SET_SRF_OFFSET,
    SCP_SET_EBO_ON_OFF,
    SCP_SET_VCAP_LEVEL_OFFSET,
	SCP_SET_SHOW_LOCAL

} SCP_SETTINGS;

typedef struct PP_INSTANCE * xPP_INST;

/****************************************************************************
*  Module statics
****************************************************************************/
/* Controls whether Early break out is on or off in default case */
#define EARLY_BREAKOUT_DEFAULT  TRUE           

/****************************************************************************
*  Functions
****************************************************************************/
extern  void SetScanParam ( xPP_INST ppi, UINT32 ParamId, INT32 ParamValue );
extern  UINT32 YUVAnalyseFrame ( xPP_INST ppi, UINT32 * KFIndicator );
extern  xPP_INST CreatePPInstance ( void );
extern  void DeletePPInstance ( xPP_INST * );
extern  BOOL ScanYUVInit ( xPP_INST,  SCAN_CONFIG_DATA *ScanConfigPtr );

#endif
