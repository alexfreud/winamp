/****************************************************************************
*
*   Module Title :     boolhuff.h
*
*   Description  :     Bool Coder header file.
*
****************************************************************************/
#ifndef __INC_BOOLHUFF_H 
#define __INC_BOOLHUFF_H

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

// Section cost measaurement stats
//#define MEASURE_SECTION_COSTS 1
#if defined MEASURE_SECTION_COSTS

extern unsigned int Sectionbits[10];
extern unsigned int ActiveSection;

#define HEADER_SECTION 0
#define MODE_SECTION   1
#define MV_SECTION     2
#define CONTEXT_OVERHEADS_SECTION 3
#define DC_SECTION     4
#define AC_SECTION     5

#endif
extern void VP6_StartDecode ( BOOL_CODER *bc, unsigned char *buffer );
extern int  VP6_DecodeBool ( BOOL_CODER *bc, int context );
extern int  VP6_DecodeBool128 ( BOOL_CODER *bc );
extern void VP6_StopDecode ( BOOL_CODER *bc );
extern void VP6_StartEncode ( BOOL_CODER *bc, unsigned char *buffer );
extern void VP6_EncodeBool ( BOOL_CODER *bc, int x, int context );
extern void VP6_EncodeBool2 ( BOOL_CODER *bc, int x, int context );
extern void VP6_StopEncode ( BOOL_CODER *bc );

#endif
