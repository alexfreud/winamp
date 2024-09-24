/****************************************************************************
*
*   Module Title :     Huffman.h
*
*   Description  :     Video CODEC
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*  
*   1.04 YWX 06-Nov-01 Changed for compatibility with Equator C compiler
*   1.03 JBB 26 Jan 01 New Huffman Code
*	1.02 PGW 11 Oct 00 Deleted reference to FrequencyCounts[].
*   1.01 PGW 15/03/00  Changes re. updated entropy tables.
*   1.00 PGW 12/10/99  Configuration baseline
*
*****************************************************************************
*/

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "type_aliases.h"
#include "boolhuff.h"

/****************************************************************************
*  Constants
*****************************************************************************
*/

/****************************************************************************/

/****************************************************************************
*  Types
*****************************************************************************
*/  

typedef struct _tokenorptr
{
    unsigned int selector : 1;   // 1 bit selector 0->ptr, 1->token
    unsigned int value : 7;
} tokenorptr;


typedef struct _huffnode
{
	union
	{
		char l;
		tokenorptr left;
    } leftunion; 
	union
	{
		char r;
		tokenorptr right;
    } rightunion; 
	unsigned char freq;

} HUFF_NODE;

/****************************************************************************
*   Data structures
*****************************************************************************
*/


/****************************************************************************
*  Functions
*****************************************************************************
*/
extern void VP5_BuildHuffTree(
    HUFF_NODE *hn, 
    unsigned int *counts, 
    int values );

extern void VP5_CreateCodeArray( HUFF_NODE *hn,
                      int node,
                      unsigned int *codearray,
                      unsigned char *lengtharray,
					  int codevalue, 
                      int codelength );

extern void VP5_EncodeValue(
    BOOL_CODER *bc,
    HUFF_NODE *hn,
    int value,
    int length);



#endif

