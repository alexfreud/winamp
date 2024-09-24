#ifndef QUANTIZE_H
#define QUANTIZE_H
#include "codec_common.h"
#include "codec_common_interface.h"

/****************************************************************************
*  Structures
*****************************************************************************
*/
typedef struct 
{
	UINT32 FrameQIndex;							// Quality specified as a table index 
	UINT32 ThisFrameQuantizerValue;				// Quality value for this frame  
	short round[8];
	short mult[8];
	short zbin[8];
	UINT32 LastQuantizerValue;					// Quality value for this frame  
	UINT32 QThreshTable[Q_TABLE_SIZE];			// ac quantizer scale values

    UINT32 *transIndex;							// array to reorder zig zag to idct's ordering
	UINT8   quant_index[64];					// array to reorder from raster to zig zag

	// used by the dequantizer 
	Q_LIST_ENTRY * dequant_coeffs[2];			// pointer to current dequantization tables
	Q_LIST_ENTRY * dequant_coeffsAlloc[2];		// alloc so we can keep alligned

	INT32 QuantCoeffs[2][64];					// Quantizer values table
	INT32 QuantRound[2][64];					// Quantizer rounding table
	INT32 ZeroBinSize[2][64];					// Quantizer zero bin table


} QUANTIZER;

/****************************************************************************
*  Functions
*****************************************************************************
*/

extern void VP5_InitQTables
(
 QUANTIZER *pbi,
 UINT8 Vp3VersionNo 
);

extern void VP5_UpdateQ
(
 QUANTIZER *pbi,
 UINT8 Vp3VersionNo  
);

extern void VP5_UpdateQC
(
 QUANTIZER *pbi,
 UINT8 Vp3VersionNo  
);

extern void VP5_init_quantizer 
(
 QUANTIZER *pbi,
 UINT8 Vp3VersionNo 
);

extern void (*VP5_quantize)
(
 QUANTIZER *pbi,
 INT16 * DCT_block,
 Q_LIST_ENTRY * quantized_list,
 UINT8 bp
);

extern void VP5_init_dequantizer 
(
 QUANTIZER *pbi,
 UINT8 Vp3VersionNo 
);

extern QUANTIZER * VP5_CreateQuantizer
(
 void
);

extern void VP5_DeleteQuantizer
(
 QUANTIZER **pbi
);          

extern UINT8 QTableSelect[6];

#endif
