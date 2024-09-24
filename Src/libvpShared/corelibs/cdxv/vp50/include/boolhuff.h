/****************************************************************************
*
*   Module Title :     boolhuff.H
*
*   Description  :     Video CODEC
*
*    AUTHOR      :     James Bankoski
*
*****************************************************************************
*   Revision History
*  
*   1.00 JBB 01JUN01  Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/
#ifndef boolhuff_h 

#define boolhuff_h

#ifdef NOTNORMALIZED
typedef struct _boolcoder
{
    unsigned char *buffer;
    unsigned int pos;
	union
	{
		unsigned int value;
		unsigned char v[4];
	};
    unsigned int range;
} BOOL_CODER;
#else 
typedef struct 
{ 
	unsigned int  bits;
	unsigned int  bitpos;
	unsigned int *source;
	unsigned int  pos;
} bitpump;
typedef struct 
{
	unsigned int lowvalue;
	unsigned int range;
	unsigned int value;
	         int count;
	unsigned int pos;
    unsigned char *buffer;

	// Variables used to track bit costs without outputing to the bitstream
	unsigned int  MeasureCost;
	unsigned long BitCounter;
} BOOL_CODER;
#endif 

extern void StartDecode(BOOL_CODER *bc, unsigned char *buffer);

extern int DecodeBool(BOOL_CODER *bc, int context);
extern int DecodeBool128(BOOL_CODER *bc);

extern void StopDecode(BOOL_CODER *bc);

extern void StartEncode(BOOL_CODER *bc, unsigned char *buffer);

extern void EncodeBool(BOOL_CODER *bc, int x, int context);
extern void EncodeBool2(BOOL_CODER *bc, int x, int context);
extern void StopEncode(BOOL_CODER *bc);

extern double shannonCost0[256];
extern double shannonCost1[256];
extern unsigned int shannon64Cost0[256];
extern unsigned int shannon64Cost1[256];

#endif
