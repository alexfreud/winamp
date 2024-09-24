/* $Header: /cvs/root/winamp/vlb/DolbyPayload.h,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: DolbyPayload.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: Dolby SE bitstream parser include file
 *
\***************************************************************************/

#ifndef _DolbyPayload_h_
#define _DolbyPayload_h_

//This is the quickest way to get all the Dolby Payload into the AAC Decoder but not the best!
#include<math.h>
#include<stdlib.h>
#include<stdio.h>
#include"bitstream.h"

#define DSPmax(a,b) (((a) > (b)) ? (a) : (b));
#define DSPmin(a,b) (((a) < (b)) ? (a) : (b));

#define DNS
#define SIGNAL_SUB
#define DOLBY_MAX_NUM_SFB		64
#define DOLBY_BLOCK_LEN_LONG	1024
#define SS_BUF_LEN_LONG			150
#define SS_BUF_LEN_SHORT		18
#define SS_COPYSTART			50
#define SS_STEP_LONG			10	/* note that SS_STEP_LONG must divide into SS_BUF_LEN_LONG without remainder,
									   to assure that all values in normCopy array are power normalized.*/
#define SS_STEP_SHORT			3	/* note that SS_STEP_SHORT must divide into SS_BUF_LEN_SHORT without remainder,
									   to assure that all values in normCopy array are power normalized.*/

#define GLOBAL_ATTEN			0.08f
#define NS_ATTEN1_LONG			0.975f
#define NS_ATTEN2_LONG			0.90f
#define NS_ATTEN1_SHORT			0.90f /* 0.816 would be attenuation factor to apply same per octave attenuation as 
										  short block, but it is desireable to allow gentler rolloff for short blocks.
										  Thus set to 0.90. */	
#define NS_ATTEN2_SHORT			0.7f   /* To apply the same per octave attenuation as long block, the slope would
										  be too sharp (greater than 6dB from bin to bin), thus allow a gentler slope 
										  for short blocks due to flat nature of short block spectra. */
#define NSFILT_COEF				0.3f
#define BACKWARD_MASK_COMP		0.95f
#define START_BIN_LONG			50
#define START_BIN_SHORT			(START_BIN_LONG / 8)
#define SE_MAX_NUM_BANDS		80
#define MAX_NUM_POWER_VALUES	SE_MAX_NUM_BANDS



#define SE_BAND_IGNORE_BITS		3
#define SE_NSHAPE_ORDER			2
#define NOISE_LENGTH			10000
#define NUM_BANDRES_OPTIONS		2
#define SE_BANDRES_2_67_DB		0
#define SE_BANDRES_1_33_DB		1
#define NUM_BLOCKTYPES			4
#define MAX_NUM_WINDOW_GROUPS	8
#define SE_DNS_PWR_BITS_SHORT	3
#define SE_DNS_PWR_BITS_LONG	4
#define SE_SPECT_FLAT_BITS_LONG  8
#define SE_SPECT_FLAT_BITS_SHORT 4
#define SE_REF_ENERG_BITS_LONG	8
#define SE_REF_ENERG_BITS_SHORT	7

// buffer model control variables
#define NEW_BUFFER_MODEL				/* if #defined, uses the new buffer model which accounts for */
                                        /*    frequency alignment between non-short and short frames */
//#define DEBUG_PRINT_BUFFER_MODEL		/* if #defined, prints out buffer model. Check the exact if statement */
										/*    in the code to see whether or not the long or short buffer model is being printed out */ 

#define USE_XFORM_HYSTERESIS
#define XFORM_HYSTERESIS_LENGTH	2	/* Hysteresis for switching SE transform lengths. Relevant
									 * only if USE_XFORM_HYSTERESIS is #define'd.
									 * NOTE: Should be between 2 and (2 * NRDB per SE frame) inclusive or
									 * even a clean switch (from single to double or vice-versa) will 
									 * elicit an unnecessary sound card close/re-open/glitch (twice).
									 */

struct SECTION_INFORMATION_STRUCT{
	int aiSectionCount[MAX_NUM_WINDOW_GROUPS];
	int aaiSectionCodebooks[MAX_NUM_WINDOW_GROUPS][DOLBY_MAX_NUM_SFB];
	int aaiSectionStart[MAX_NUM_WINDOW_GROUPS][DOLBY_MAX_NUM_SFB];
	int aaiSectionEnd[MAX_NUM_WINDOW_GROUPS][DOLBY_MAX_NUM_SFB];
};

struct DNS_INFORMATION_STRUCT{
	int									iWindowSequence;
	int									iGroupCount;
	int									iGroupLength[MAX_NUM_WINDOW_GROUPS];
	int									iMaxSFB;
	int									iLastBin;
	const int							*piBandOffsets;
	const SECTION_INFORMATION_STRUCT	*psSectionInfoStruct;
	float								aafDNSRatio[MAX_NUM_WINDOW_GROUPS][DOLBY_MAX_NUM_SFB];
};

struct DOLBY_PAYLOAD_STRUCT{
	int		iDolbyBitStreamWarning;
	int		iUsesDoubleLengthXForm;
	int		iSEPowerResolution;
	int		iChannels;
	int		iSampleRateIndex;
	int		iSampleRate;
	int		num_se_bands[2];
	int		seBands[2][MAX_NUM_WINDOW_GROUPS][SE_MAX_NUM_BANDS];
	int		sfm[2][MAX_NUM_WINDOW_GROUPS];
	int		delta_power_values[2][MAX_NUM_WINDOW_GROUPS][MAX_NUM_POWER_VALUES];
	float	fdamp[2][MAX_NUM_WINDOW_GROUPS];
	int		SE_num_ignored_upper_bands[2];
	int		aiMaxSFB[2];
	int		aiTotalSFB[2];
    int		aiCopyStop[2];
	int		iGroupLength[2][MAX_NUM_WINDOW_GROUPS];
	int		iGroupCount[2];

	DNS_INFORMATION_STRUCT asDNSInfoStruct[2];
};

struct Huffman
{
    int		index;
    int		len;
    unsigned long	cw;
} ;

struct Huffman_lookup
{
    int		index;
    int		len;
    int      cw;
};

typedef struct {
	unsigned int sr;
	unsigned int* SE_bands_bw;
	unsigned char SE_max_num_se_bands;
	unsigned char SE_num_ignored_upper_bands;
} SE_Band_Info_struct;

typedef struct {
	int	powerResolution;
	Huffman* cwLengthLookupTable;
	Huffman_lookup* cwValueLookupTable;
	int num_codewords;
} SE_power_resolution_table_struct;


void applyDNS(	float *pfSpectralData,
				DNS_INFORMATION_STRUCT*psDNSInfoStruct,
				int iPrevWnd);


unsigned char computeSeBandInfo (unsigned int SRate,				/* input: Sample Rate */
								 unsigned int first_SE_bin,			/* input: bin number of the first bin of the extension range */
								 SE_Band_Info_struct* pSE_Band_Info_struct, /* input: SE band info struct ptr */
								 int* SE_bands_bw,					/* output: SE bandwidths, in bins (array; memory assumed to be already allocated) */
								 int* num_SE_bands);

int spectralExtInfo(	int cnt,
						DOLBY_PAYLOAD_STRUCT *seData,
						CDolbyBitStream*poBitStream);
void spectralExtend(float* pfData,
					 int iCopyStart,
					 int iCopyStop,
					 int iSfm,
					 int iBandCount,
					 int* piBands,
					 int* delta_power_values,
					 float fDamp,
					 int iHBlockSize, 
					 int wnd,
					 int SEPowerResolution,
					 int iPreviousWnd,
					 float* avgCopyEnergies);

void	computeAvgCopyEnergies(float* pfData,
					int iCopyStart,
					int iCopyStop,
					int num_se_bands,
					int* piBands,
					int halfWindowLength,
					int iWindowSequence,
					float* avgCopyEnergies);


#endif