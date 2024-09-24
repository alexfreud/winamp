/****************************************************************************
*
*   Module Title :     Huffman.c
*
*   Description  :     Huffman coding routines.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "huffman.h"
#include "pbdll.h"

/****************************************************************************
*  Typedefs
****************************************************************************/              
typedef struct _SORT_NODE
{
    int next;
    int freq;
    unsigned char value;
} SORT_NODE;

typedef struct _sortnode
{
    int next;
    int freq;
    tokenorptr value;
} sortnode;

/****************************************************************************
 * 
 *  ROUTINE       :     InsertSorted
 *
 *  INPUTS        :     sortnode *sn   : Array of sort nodes.
 *                      int node       : Index of node to be inserted.
 *                      int *startnode : Pointer to _head of linked-list.
 *
 *  OUTPUTS       :     int *startnode : Pointer to _head of linked-list.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Inserts a node into a sorted linklist.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
static void InsertSorted ( sortnode *sn, int node, int *startnode )
{
    int which = *startnode;
    int prior = *startnode;

    // find the position at which to insert the node
    while( which != -1 && sn[node].freq > sn[which].freq )
    {
        prior = which;
        which = sn[which].next;
    }

    if(which == *startnode)
    {
        *startnode = node;
        sn[node].next = which;
    }
    else
    {
        sn[prior].next = node;
        sn[node].next = which;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_BuildHuffTree
 *
 *  INPUTS        :     int values           : Number of values in the tree.
 *                      unsigned int *counts : Histogram of token frequencies.
 *
 *  OUTPUTS       :     HUFF_NODE *hn        : Array of nodes (containing token frequency) 
 *                                             from which to create tree.
 *                      unsigned int *counts : Histogram of token frequencies (0 freq clipped to 1).
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Creates a Huffman tree data structure from list
 *                      of token frequencies.
 *
 *  SPECIAL NOTES :     Maximum of 256 nodes can be handled. 
 *
 ****************************************************************************/
void VP6_BuildHuffTree ( HUFF_NODE *hn, unsigned int *counts, int values )
{
    int i;
    sortnode sn[256];
    int sncount=0;
    int startnode=0;

    // NOTE:
    // Create huffman tree in reverse order so that the root will always be 0
    int huffptr=values-1;

    // Set up sorted linked list of values/pointers into the huffman tree
    for ( i=0; i<values; i++ )
    {
        sn[i].value.selector = 1;
        sn[i].value.value = i;
        if ( counts[i] == 0 )
            counts[i] = 1;
        sn[i].freq = counts[i];
        sn[i].next = -1;
    }

    sncount = values;

    // Connect above list into a linked list
    for ( i=1; i<values; i++ )
        InsertSorted ( sn, i, &startnode );

    // while there is more than one node in our linked list
    while ( sn[startnode].next != -1 )
    {
        int first = startnode;
        int second = sn[startnode].next;
        int sumfreq = sn[first].freq + sn[second].freq;

        // set-up new merged huffman node
        --huffptr;
        hn[huffptr].leftunion.left = sn[first].value;
        hn[huffptr].rightunion.right = sn[second].value;
        hn[huffptr].freq = 256 * sn[first].freq / sumfreq;

        // set up new merged sort node pointing to our huffnode
        sn[sncount].value.selector = 0;
        sn[sncount].value.value = huffptr;
        sn[sncount].freq = sumfreq;
        sn[sncount].next = -1;

        // remove the two nodes we just merged from the linked list
        startnode = sn[second].next;

        // insert the new sort node into the proper location
        InsertSorted(sn, sncount, &startnode);

        // account for new nodes
        sncount++;
    }

    return;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_BuildHuffLookupTable
 *
 *  INPUTS        :     HUFF_NODE *HuffTreeRoot : Pointer to root of Huffman tree. 
 *
 *  OUTPUTS       :     UINT16 *HuffTable       : Array (LUT) of Huffman codes.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Traverse Huffman tree to create LUT of Huffman codes.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_BuildHuffLookupTable ( HUFF_NODE *HuffTreeRoot, UINT16 *HuffTable )
{
    int i, j;
    int bits;
    tokenorptr torp;

    for ( i=0; i<(1<<HUFF_LUT_LEVELS); i++ )
    {
        bits = i;        
        j=0;

        torp.value    = 0;
        torp.selector = 0;

        do
        {
            j++;
            if ( (bits>>(HUFF_LUT_LEVELS - j)) & 1 )
                torp = HuffTreeRoot[torp.value].rightunion.right;
            else
                torp = HuffTreeRoot[torp.value].leftunion.left;
        }
        while ( !(torp.selector) && (j < HUFF_LUT_LEVELS) );
        
//        HuffTable[i] = torp.value<<1 | torp.selector | (j << 12);
        ((HUFF_TABLE_NODE *)HuffTable)[i].value = torp.value;
        ((HUFF_TABLE_NODE *)HuffTable)[i].flag = torp.selector;
        ((HUFF_TABLE_NODE *)HuffTable)[i].length = j;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_BuildHuffLookupTable
 *
 *  INPUTS        :     HUFF_NODE *hn  : List of Huffman tree nodes.
 *                      int node       : Current position within list of Huffman tree nodes.
 *                      int codevalue  : Huffman code as found so far. 
 *                      int codelength : Length of Huffman code so far (in bits).
 *
 *  OUTPUTS       :     unsigned int *codearray    : Array to hold Huffman codes.
 *                      unsigned char *lengtharray : Array to hold lengths of Huffman codes.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Recursively traverse Huffman tree to create LUT of Huffman codes.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_CreateCodeArray
( 
    HUFF_NODE *hn,
    int node,
    unsigned int *codearray,
    unsigned char *lengtharray,
    int codevalue, 
    int codelength 
)
{    
    /* If we are at a leaf then fill in a code array entry */
    /* Use recursive calls to scan down the tree */
    if( hn[node].leftunion.left.selector )
    {
        codearray[hn[node].leftunion.left.value] = (codevalue<<1)+0;
        lengtharray[hn[node].leftunion.left.value] = codelength+1;
    }
    else
    {
        VP6_CreateCodeArray ( 
            hn, 
            hn[node].leftunion.left.value,
            codearray,
            lengtharray, 
            ((codevalue << 1) + 0), 
            (codelength + 1) );
    }
    
    if( hn[node].rightunion.right.selector )
    {
        codearray[hn[node].rightunion.right.value] = (codevalue<<1)+1;
        lengtharray[hn[node].rightunion.right.value] = codelength+1;
    }
    else
    {
        VP6_CreateCodeArray ( 
            hn, 
            hn[node].rightunion.right.value,
            codearray,
            lengtharray, 
            ((codevalue << 1) + 1), 
            (codelength + 1) );
    }    
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DecodeValue
 *
 *  INPUTS        :     BOOL_CODER *bc : Pointer to a Bool Coder instance.
 *                      HUFF_NODE *hn  : List of Huffman tree nodes.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     int: Decoded token value.
 *
 *  FUNCTION      :     Traverse the Huffman tree by reading node decisions
 *                      from the bitstream until a leaf node is reached. Returns
 *                      the value associated with this leaf node.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
int VP6_DecodeValue ( BOOL_CODER *bc, HUFF_NODE *hn )
{
    tokenorptr torp;
	
    torp.value    = 0;
	torp.selector = 0;
    
    // Loop searches down through tree based upon bits read from the bitstream 
    // until it hits a leaf at which point we have decoded a token.
    do
    {
		if ( VP6_DecodeBool(bc, hn[torp.value].freq) )
	        torp = hn[torp.value].rightunion.right;
		else
	        torp = hn[torp.value].leftunion.left;
    }
	while ( !(torp.selector) );

	return torp.value;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_EncodeValue
 *
 *  INPUTS        :     BOOL_CODER *bc : Pointer to a Bool Coder instance.
 *                      HUFF_NODE *hn  : List of Huffman tree nodes.
 *                      int value      : Value to be encoded.
 *                      int length     : Length of value to be encoded (in bits).
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Given a Huffman code either output its bits to the encoded
 *                      stream or measure the cost of doing so, depending on the 
 *                      flag bc->MeasureCost. Use VP6_EncodeBool2 if only measuring
 *                      approximate number of bits required to encode the Huffman code
 *                      or VP6_EncodeBool if actually producing the coded bits using
 *                      the specified Bool Coder.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void VP6_EncodeValue
(
	BOOL_CODER *bc,
    HUFF_NODE *hn,
    int value,
    int length
)
{
    int i;
    int node = 0;

    for ( i=length-1; i>=0; i-- )
    {
        int v = (value>>i) & 1;

		if ( bc->MeasureCost )
			VP6_EncodeBool2 ( bc, (BOOL)v, hn[node].freq );
		else
			VP6_EncodeBool ( bc, (BOOL)v, hn[node].freq );

        if ( v )
            node = hn[node].rightunion.right.value;
        else
            node = hn[node].leftunion.left.value;
    }
}
