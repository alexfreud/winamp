/****************************************************************************
*
*   Module Title :     Reconstruct.h
*
*   Description  :     Block Reconstruction module header
*
*   AUTHOR       :     Paul Wilkins
*
*****************************************************************************
*   Revision History
* 
*   1.00 PGW 14/10/99  Created
*
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#ifndef RECONSTRUCT_H
#define RECONSTRUCT_H

#include "type_aliases.h"

/****************************************************************************
*  Constants
*****************************************************************************
*/

/****************************************************************************
*  Types
*****************************************************************************
*/        

/****************************************************************************
*   Data structures
*****************************************************************************
*/

/****************************************************************************
*  Functions
*****************************************************************************
*/

// Scalar (no mmx) reconstruction functions
extern void ScalarReconIntra( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void ScalarReconInter( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void ScalarReconInterHalfPixel2( INT16 *TmpDataBuffer, UINT8 * ReconPtr,UINT8 * RefPtr1, UINT8 * RefPtr2, INT16 * ChangePtr, UINT32 LineStep );

// MMx versions
extern void MMXReconIntra( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void MmxReconInter( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void MmxReconInterHalfPixel2( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr1, UINT8 * RefPtr2, INT16 * ChangePtr, UINT32 LineStep );

// WMT versions
extern void WmtReconIntra( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT16 * ChangePtr, UINT32 LineStep );
extern void WmtReconInter( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep );
extern void WmtReconInterHalfPixel2( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr1, UINT8 * RefPtr2, INT16 * ChangePtr, UINT32 LineStep );


#endif
