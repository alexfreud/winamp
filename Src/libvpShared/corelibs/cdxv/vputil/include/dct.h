/****************************************************************************
*
*   Module Title :     dct.h
*
*   Description  :     DCT header file.
*
****************************************************************************/						

#ifndef __INC_DCT_H
#define __INC_DCT_H

/****************************************************************************
*  Header files
****************************************************************************/
#include "type_aliases.h"

/****************************************************************************
*  Macros
****************************************************************************/
#define COEFF_MAX   32768   // Max magnitude of DCT coefficient
// Extra bits of precision added to the fdct that have to be stripped off during the quantize
#define FDCT_PRECISION_BITS			1
#define FDCT_PRECISION_NEG_ADJ      ((INT16) (1<<FDCT_PRECISION_BITS)-1)




#if 0   // AWG not required any more!!!
/*	Cos and Sin constant multipliers used during DCT and IDCT */
extern const double C1S7;
extern const double C2S6;
extern const double C3S5;
extern const double C4S4;
extern const double C5S3;
extern const double C6S2;
extern const double C7S1;

// DCT lookup tables and pointers
extern INT32 * C4S4_TablePtr;
extern INT32 C4S4_Table[(COEFF_MAX * 4) + 1];

extern INT32 * C6S2_TablePtr;
extern INT32 C6S2_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C2S6_TablePtr;
extern INT32 C2S6_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C1S7_TablePtr;
extern INT32 C1S7_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C7S1_TablePtr;
extern INT32 C7S1_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C3S5_TablePtr;
extern INT32 C3S5_Table[(COEFF_MAX * 2) + 1];

extern INT32 * C5S3_TablePtr;
extern INT32 C5S3_Table[(COEFF_MAX * 2) + 1];
#endif

/****************************************************************************
*  Exports
****************************************************************************/
#ifdef COMPDLL
// Forward Transform
extern void fdct_slow ( INT32 *InputData, double *OutputData );
#endif

// Reverse Transform
extern void IDctSlow(  INT16 *InputData, INT16 *QuantMatrix, INT16 *OutputData );
extern void IDct10  (  INT16 *InputData, INT16 *QuantMatrix, INT16 *OutputData );
extern void IDct1   (  INT16 *InputData, INT16 *QuantMatrix, INT16 *OutputData );

#endif
