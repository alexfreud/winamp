/****************************************************************************
*
*   Module Title :     Huffman.h
*
*   Description  :     Huffman Coding header file.
*
****************************************************************************/
#ifndef __INC_HUFFMAN_H
#define __INC_HUFFMAN_H

/****************************************************************************
*  Header Files
****************************************************************************/
#include "type_aliases.h"
#include "boolhuff.h"

/****************************************************************************
*  Module Statics
****************************************************************************/
#define HUFF_LUT_LEVELS 6

/****************************************************************************
*  Types
****************************************************************************/  
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
****************************************************************************/
typedef struct _HUFF_TALBE_NODE
{
    unsigned short flag     :1;      // bit 0: 1-Token, 0-Index
    unsigned short value    :5;      // value: the value of the Token or the Index to the huffman tree
    unsigned short unused   :6;      // not used for now
    unsigned short length   :4;      // Huffman code length of the token
} HUFF_TABLE_NODE;

/****************************************************************************
*  Functions
****************************************************************************/
extern void VP6_BuildHuffLookupTable ( HUFF_NODE * HuffTreeRoot, UINT16 * HuffTable );
extern void VP6_BuildHuffTree ( HUFF_NODE *hn, unsigned int *counts, int values );
extern void VP6_CreateCodeArray( HUFF_NODE *hn,
                      int node,
                      unsigned int *codearray,
                      unsigned char *lengtharray,
					  int codevalue, 
                      int codelength );
extern void VP6_EncodeValue ( BOOL_CODER *bc, HUFF_NODE *hn, int value, int length );

#endif
