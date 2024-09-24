/****************************************************************************
*
*   Module Title :     quantize.h
*
*   Description  :     Quantizer header file.
*
****************************************************************************/
#ifndef __INC_QUANTIZE_H
#define __INC_QUANTIZE_H


/****************************************************************************
*  Header Files
****************************************************************************/
#include "codec_common.h"
#include "codec_common_interface.h"


/****************************************************************************
*  Macros
****************************************************************************/



/****************************************************************************
*  Structures
****************************************************************************/
typedef struct 
{
	UINT32 FrameQIndex;					   // Quality specified as a table index 
	UINT32 LastFrameQIndex;	
	short round[8];
	short mult[8];
	short zbin[8];
	UINT32 QThreshTable[Q_TABLE_SIZE];	   // ac quantizer scale values

    UINT32 *transIndex;					   // array to reorder zig zag to idct's ordering
	UINT8   quant_index[64];			   // array to reorder from raster to zig zag

	// used by the dequantizer 
	Q_LIST_ENTRY * dequant_coeffs[2];	   // pointer to current dequantization tables
	Q_LIST_ENTRY * dequant_coeffsAlloc[2]; // alloc so we can keep alligned

	INT32 QuantCoeffs[2][64];			   // Quantizer values table
	INT32 QuantRound[2][64];			   // Quantizer rounding table
	INT32 ZeroBinSize[2][64];			   // Quantizer zero bin table
	INT32 ZlrZbinCorrections[2][64];	   // Zbin corrections based upon zero run length.

} QUANTIZER;

/****************************************************************************
*  Exports
****************************************************************************/
extern const UINT8 VP6_QTableSelect[6];
extern const Q_LIST_ENTRY VP6_DcQuant[Q_TABLE_SIZE];

extern void (*VP6_quantize) ( QUANTIZER *pbi, INT16 * DCT_block, Q_LIST_ENTRY * quantized_list, UINT8 bp );
extern void (*VP6_BuildQuantIndex)( QUANTIZER * pbi);
extern void VP6_InitQTables ( QUANTIZER *pbi, UINT8 Vp3VersionNo );
extern void VP6_UpdateQ ( QUANTIZER *pbi, UINT8 Vp3VersionNo );
extern void VP6_UpdateQC ( QUANTIZER *pbi, UINT8 Vp3VersionNo );
extern QUANTIZER * VP6_CreateQuantizer ( void );
extern void VP6_DeleteQuantizer ( QUANTIZER **pbi );          

#endif
