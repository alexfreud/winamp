/****************************************************************************
*
*   Module Title :     HuffTables.h
*
*   Description  :     Video CODEC
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*  
*   1.01 JBB 26 Jan 01 New Huffman Code
*   1.00 PGW 12/10/00  Configuration baseline
*
*****************************************************************************
*/

#ifndef HUFFTAB_H
#define HUFFTAB_H

#include "type_aliases.h"
#include "huffman.h"

/****************************************************************************
*  Hufman tables
*****************************************************************************
*/

// For details of tokens and extra bit breakdown see token definitions in huffman.h
UINT8  ExtraBitLengths_VP5[MAX_ENTROPY_TOKENS] = { 0, 1, 1, 1, 1, 2, 3, 5, 6, 12, 0 };
UINT32 DctRangeMinVals[MAX_ENTROPY_TOKENS] = { 0, 1, 2, 3, 4, 5, 7, 11, 27, 59, 0 };

#endif