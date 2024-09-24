/****************************************************************************
*
*   Module Title :     Huffman.c
*
*   Description  :     Video CODEC
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*  
*   1.13 YWX 06-Nov-01 Changed for compatibility with Equator C compiler
*   1.12 JBB 13-Jun-01 VP4 Code Clean Out
*   1.11 SJL 22-Mar-01 Fixed MAC compile errors
*   1.10 JBX 22-Mar-01 Changed size of SORT_NODE array to 1024;
*   1.09 JBB 26 Jan 00 Reworked Huffman to remove dynamic allocation and 
*                      to condense tree storage.
*   1.08 PGW 11 Oct 00 Changes to support different entropy tables for 
*					   different encoder versions.
*   1.07 PGW 17/03/00  Further Entropy changes.
*   1.06 PGW 15/03/00  Updated entropy tables.
*	1.05 JBB 27/01/99  Globals Removed, use of PB_INSTANCE, Bit Management
*   1.04 PGW 05/11/99  Changes to support AC range entropy tables.
*   1.03 PGW 12/10/99  Changes to reduce uneccessary dependancies.
*   1.02 PGW 19/07/99  Deleted the funtion DecodeHuffToken().
*   1.01 PGW 15/07/99  Added inline bit extraction to DecodeHuffToken().
*   1.00 PGW 07/07/99  Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "systemdependant.h"
#include "huffman.h"
#include "pbdll.h"
#include "boolhuff.h"
/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

/****************************************************************************
*  Forward references.
*****************************************************************************
*/       
 
void VP5_BuildHuffTree(
    HUFF_NODE *hn, 
    unsigned int *counts, 
    int values );

void VP5_CreateCodeArray( HUFF_NODE *hn,
                      int node,
                      unsigned int *codearray,
                      unsigned char *lengtharray,
					  int codevalue, 
                      int codelength );
                      
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/  
typedef struct _SORT_NODE
{
    int next;
    int freq;
    unsigned char value;
} SORT_NODE;

/****************************************************************************
*  Module Static Variables
*****************************************************************************
*/              

//***********************************************************
// Jim's version of Eric's condensed huffman trees!


typedef struct _sortnode
{
    int next;
    int freq;
    tokenorptr value;
} sortnode;


// inserts a node into a sorted linklist
static void InsertSorted( 
    sortnode *sn, 
    int node, 
    int *startnode  )
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

// returns a pointer to the condensed huffman root node
void VP5_BuildHuffTree(
    HUFF_NODE *hn, 
    unsigned int *counts, 
    int values )
{
    int i;
    sortnode sn[256];
    int sncount=0;
    int startnode=0;

    // note we are creating the huffman tree in 
    // reverse order so that the root will always be 0
    int huffptr=values-1;

    // set up our sorted linked list of values 
    // or pointers into the huffman tree
    for(i=0;i<values;i++)
    {
        sn[i].value.selector = 1;
        sn[i].value.value = i;
        if(counts[i] == 0)
            counts[i] = 1;
        sn[i].freq = counts[i];
        sn[i].next = -1;
    }
    sncount=values;

    // connected the above list into a linked list
    for(i=1;i<values;i++)
    {
        InsertSorted(sn,i,&startnode);
    }

    // while there is more than one node in our linked list
    while(sn[startnode].next!=-1)
    {
        int first = startnode;
        int second = sn[startnode].next;
        int sumfreq = sn[first].freq + sn[second].freq;

        // setup new merged huffman node
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
    return ;
}

void VP5_CreateCodeArray( HUFF_NODE *hn,
                      int node,
                      unsigned int *codearray,
                      unsigned char *lengtharray,
					  int codevalue, 
                      int codelength )
{    
    
    /* If we are at a leaf then fill in a code array entry. */
    /* Recursive calls to scan down the tree. */
    if( hn[node].leftunion.left.selector )
    {
        codearray[hn[node].leftunion.left.value] = (codevalue<<1)+0;
        lengtharray[hn[node].leftunion.left.value] = codelength+1;
    }
    else
    {
        VP5_CreateCodeArray( 
            hn, 
            hn[node].leftunion.left.value,
            codearray,
            lengtharray, 
            ((codevalue << 1) + 0), 
            (codelength + 1)
            );
    }
    
    if( hn[node].rightunion.right.selector )
    {
        codearray[hn[node].rightunion.right.value] = (codevalue<<1)+1;
        lengtharray[hn[node].rightunion.right.value] = codelength+1;
    }
    else
    {
        VP5_CreateCodeArray( 
            hn, 
            hn[node].rightunion.right.value,
            codearray,
            lengtharray, 
            ((codevalue << 1) + 1), 
            (codelength + 1)
            );
    }    
}

int VP5_DecodeValue(
    BOOL_CODER *bc, 
    HUFF_NODE *hn
    )
{
    tokenorptr torp;
	torp.value=0;
	torp.selector=0;
    // Loop searches down through tree based upon bits read from the bitstream 
    // until it hits a leaf at which point we have decoded a token

    do
    {
		if(DecodeBool(bc, hn[torp.value].freq))
		{
	        torp = hn[torp.value].rightunion.right;
		}
		else
		{
	        torp = hn[torp.value].leftunion.left;
		}
    }
	while ( !(torp.selector));

	return torp.value;
}

void VP5_EncodeValue(
	BOOL_CODER *bc,
    HUFF_NODE *hn,
    int value,
    int length)
{
    int i;
    int node = 0;
    for(i=length-1;i>=0;i--)
    {
        int v= (value>>i) & 1;

		if ( bc->MeasureCost )
			EncodeBool2(bc,(BOOL) v , hn[node].freq);
		else
			EncodeBool(bc,(BOOL) v , hn[node].freq);

        if(v)
        {
            node=hn[node].rightunion.right.value;
        }
        else
        {
            node=hn[node].leftunion.left.value;
        }
    }
}
