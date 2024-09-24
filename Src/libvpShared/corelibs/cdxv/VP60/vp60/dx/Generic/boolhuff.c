/****************************************************************************
*
*   Module Title :     boolhuff.c
*
*   Description  :     Boolean Encoder/Decoder
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "boolhuff.h"
#include "TokenEntropy.h"
#include <stdio.h>

// STATS Variables for measuring section costs
#if defined MEASURE_SECTION_COSTS
UINT32 Sectionbits[10] = {0,0,0,0,0,0,0,0,0,0};
UINT32 ActiveSection = 0;
#endif

#ifdef NOTNORMALIZED
/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StartDecode
 *
 *  INPUTS        :     BOOL_CODER *bc		  : pointer to instance of a boolean decoder.
 *						unsigned char *buffer : pointer to buffer of data to be decoded.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Initializes the boolean decoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_StartDecode ( BOOL_CODER *bc, unsigned char *buffer )
{
    bc->pos    = 0;
    bc->value  = 0;
    bc->range  = 0;
    bc->buffer = buffer;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DecodeBool
 *
 *  INPUTS        :     BOOL_CODER *bc  : pointer to instance of a boolean decoder.
 *						int probability : probability next symbol is a 0 (0-255)
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		Next decoded bit: 0 or 1 
 *
 *  FUNCTION      :     Determines the next value stored in the boolean decoder 
 *                      based upon the probability passed in. It uses a simple 
 *                      probability model to approximate an arithmetic coder.
 *
 *  SPECIAL NOTES :     The accuracy of this decoder gets worse as the range 
 *						approaches 0. This can be avoided with more complex 
 *						normalization functions (as in a standard arithmetic)
 *						coder. Chosen to avoid this for speed reasons.
 *
 ****************************************************************************/
int VP6_DecodeBool ( BOOL_CODER *bc, int probability )
{
	unsigned int split;

	// Don't have enough in our range to tell between a 0 and 1 so get 
	// 3 new bytes. 
    if( bc->range < 2)
    {
		unsigned char *spot = bc->buffer+bc->pos;
		bc->v[0] = spot[0];
		bc->v[1] = spot[1];
		bc->v[2] = spot[2];

		// range is set to 0x01000001 to avoid having the range * probability 
		// calculation outrange (this can be handled differently at the cost 
		// of an extra if).
        bc->range = 0x01000000;
        bc->pos += 3;
    }

	// calculate the decision point 
	// black magic: This code works better than if I calculate probability *
	// range and then truncating to 1 (can't explain why)
	split = bc->range;
	split --;				// we always have to maintain
	split *= probability;
	split >>= 8;
	split ++;

	if( bc->value < split )
	{
		bc->range = split;
		return 0;
	}
	else
	{
		bc->range-=split;
		bc->value-=split;
		return 1;
	}
} 

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StopDecode
 *
 *  INPUTS        :     BOOL_CODER *bc  : pointer to instance of a boolean decoder.
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Performs clean-up for boolean decoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_StopDecode ( BOOL_CODER *bc )
{
    return;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StartEncode
 *
 *  INPUTS        :     BOOL_CODER *bc        : pointer to instance of a boolean encoder.
 *						unsigned char *buffer : pointer to buffer to hold encoded data.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Initializes the boolean encoder
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_StartEncode ( BOOL_CODER *bc, unsigned char *buffer )
{
    bc->pos    = 0;
    bc->value  = 0;
    bc->range  = 0x01000000;
    bc->buffer = buffer;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_EncodeBool
 *
 *  INPUTS        :     BOOL_CODER *bc  : pointer to instance of a boolean encoder.
 *						int x		    : value to be encoded (0 or 1).
 *						int probability : probability of getting a 0 (0-255) 
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		void.
 *
 *  FUNCTION      :     Encodes a boolean value (0 or 1) using the specified 
 *                      boolean encoder.
 *
 *  SPECIAL NOTES :     The accuracy of this encoder gets worse as the range 
 *						approaches 0. This can be avoided with more complex 
 *						normalization functions (as in a standard arithmetic
 *						coder). Chose to avoid this for speed reasons.
 *
 ****************************************************************************/
void VP6_EncodeBool ( BOOL_CODER *bc, int x, int probability )
{
	unsigned int split;

	// we don't have enough in our range to tell between a 0 and 1,
	// so get 3 new bytes. 
    if( bc->range < 2 )
    {
		bc->buffer[bc->pos] = bc->v[0];
		bc->buffer[bc->pos+1] = bc->v[1];
		bc->buffer[bc->pos+2] = bc->v[2];
        bc->pos+=3;

		// range is set to 0x01000001 to avoid having the range * probability 
		// calculation outrange ( this can be handled differently at the cost 
		// of an extra if).
        bc->range = 0x01000000;
        bc->value = 0;
    }
	
	// calculate the decision point 
	// black magic: This code works better than if I calculate probability *
	// range and then truncating to 1 (can't explain why)
	split = bc->range;
	split --;
	split *= probability;
	split >>= 8;
	split ++;
	
	if( x )
	{
		bc->range-=split;
		bc->value+=split;
	}
	else
	{
		bc->range = split;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StopEncode
 *
 *  INPUTS        :     BOOL_CODER *bc  : pointer to instance of a boolean encoder.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Performs clean-up for boolean encoder
 *                           
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_StopEncode( BOOL_CODER *bc )
{
	int i;

	for ( i=0; i<3; i++ )
	{ 
		bc->buffer[bc->pos + i] = *((unsigned char *) &bc->value + i);
	}
    bc->pos += 3;
}

#else 

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StartEncode
 *
 *  INPUTS        :     BOOL_CODER *br        : pointer to instance of a boolean encoder.
 *						unsigned char *source :	pointer to buffer to hold encoded data.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Perform initialization of the boolean encoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_StartEncode ( BOOL_CODER *br, unsigned char *source )
{
	br->lowvalue = 0;
	br->range    = 255;
	br->value    = 0;
	br->count    = -24; 
	br->buffer   = source;
	br->pos      = 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StopEncode
 *
 *  INPUTS        :     BOOL_CODER *br : pointer to instance of a boolean encoder.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Performs clean-up for a boolean encoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_StopEncode ( BOOL_CODER *br )
{	
	if(br->count<-16)
		br->lowvalue <<= (24-(br->count&7));
	else if(br->count<-8)
		br->lowvalue <<= (16-(br->count&7));
	else 
		br->lowvalue <<= (8-(br->count&7));

	br->buffer[br->pos++] = (br->lowvalue>>24);
	br->buffer[br->pos++] = (br->lowvalue>>16) & 0xff;
	br->buffer[br->pos++] = (br->lowvalue>> 8) & 0xff;
	br->buffer[br->pos++] = (br->lowvalue    ) & 0xff;
	br->buffer[br->pos++] = 0;
}
	
/****************************************************************************
 * 
 *  ROUTINE       :     VP6_EncodeBool
 *
 *  INPUTS        :     BOOL_CODER *br  : pointer to instance of a boolean encoder.
 *						int bit         : value to be encoded (0 or 1).
 *						int probability : probability of getting a 0 (0-255) 
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		void
 *
 *  FUNCTION      :     Encodes a boolean value (0 or 1) using the 
 *                      specified boolean encoder.
 *
 *  SPECIAL NOTES :     This encoder uses normalizations, and is fairly accurate,
 *
 ****************************************************************************/
void VP6_EncodeBool ( BOOL_CODER *br, int bit, int probability )
{
	unsigned int split;
    unsigned int count = br->count;
    unsigned int range = br->range;
    unsigned int lowvalue = br->lowvalue;

#if defined MEASURE_SECTION_COSTS
	if (bit)
		Sectionbits[ActiveSection] += VP6_ProbCost[255-probability];
	else
		Sectionbits[ActiveSection] += VP6_ProbCost[probability];
#endif

    split = 1 +  (((range-1) * probability) >> 8);
	
    range = split;
    if(bit)
	{
		lowvalue += split;
		range = br->range-split;
	}

    while(range < 0x80)
	{
		range <<= 1;

		if((lowvalue & 0x80000000 ))
        {
            int x = br->pos-1;
            while(x>=0 && br->buffer[x] == 0xff)
            {
                br->buffer[x] =(unsigned char)0;
                x--;
            }
            br->buffer[x]+=1;
            
        }
        lowvalue  <<= 1;
		if (!++count) 
		{
			count = -8;
			br->buffer[br->pos++]=(lowvalue >> 24);
			lowvalue &= 0xffffff;
		}
	}
    br->count = count;
    br->lowvalue = lowvalue;
    br->range = range;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_EncodeBoolOne
 *
 *  INPUTS        :     BOOL_CODER *br  : pointer to instance of a boolean encoder.
 *						int bit         : value to be encoded (UNUSED).
 *						int probability : probability of getting a 0 (0-255) 
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		void
 *
 *  FUNCTION      :     Encodes the boolean value 1 using the specified boolean encoder.
 *
 *  SPECIAL NOTES :     This encoder uses normalizations, and is fairly accurate,
 *
 ****************************************************************************/
void VP6_EncodeBoolOne ( BOOL_CODER *br, int bit, int probability )
{
	unsigned int split;
    unsigned int count    = br->count;
    unsigned int range    = br->range;
    unsigned int lowvalue = br->lowvalue;

#if defined MEASURE_SECTION_COSTS
	if (bit)
		Sectionbits[ActiveSection] += VP6_ProbCost[255-probability];
	else
		Sectionbits[ActiveSection] += VP6_ProbCost[probability];
#endif

    split = 1 +  (((range-1) * probability) >> 8);
	    
    lowvalue += split;
	range = range-split;

    while(range < 0x80)
	{
		range <<= 1;

		if((lowvalue & 0x80000000 ))
        {
            int x = br->pos-1;
            while(x>=0 && br->buffer[x] == 0xff)
            {
                br->buffer[x] =(unsigned char)0;
                x--;
            }
            br->buffer[x]+=1;
            
        }
        lowvalue  <<= 1;
		if (!++count) 
		{
			count = -8;
			br->buffer[br->pos++]=(lowvalue >> 24);
			lowvalue &= 0xffffff;
		}
	}
    br->count = count;
    br->lowvalue = lowvalue;
    br->range = range;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_EncodeBoolZero
 *
 *  INPUTS        :     BOOL_CODER *br  : pointer to instance of a boolean encoder.
 *						int bit         : value to be encoded (UNUSED).
 *						int probability : probability of getting a 0 (0-255) 
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		void
 *
 *  FUNCTION      :     Encodes the boolean value 0 using the specified boolean encoder.
 *
 *  SPECIAL NOTES :     This encoder uses normalizations, and is fairly accurate,
 *
 ****************************************************************************/
void VP6_EncodeBoolZero ( BOOL_CODER *br, int bit, int probability )
{
    unsigned int count = br->count;
    unsigned int range = br->range;
    unsigned int lowvalue = br->lowvalue;

#if defined MEASURE_SECTION_COSTS
	if (bit)
		Sectionbits[ActiveSection] += VP6_ProbCost[255-probability];
	else
		Sectionbits[ActiveSection] += VP6_ProbCost[probability];
#endif

    range = 1 +  (((range-1) * probability) >> 8);

    while(range < 0x80)
	{
		range <<= 1;

		if((lowvalue & 0x80000000 ))
        {
            int x = br->pos-1;
            while(x>=0 && br->buffer[x] == 0xff)
            {
                br->buffer[x] =(unsigned char)0;
                x--;
            }
            br->buffer[x]+=1;
            
        }
        lowvalue  <<= 1;
		if (!++count) 
		{
			count = -8;
			br->buffer[br->pos++]=(lowvalue >> 24);
			lowvalue &= 0xffffff;
		}
	}
    br->count = count;
    br->lowvalue = lowvalue;
    br->range = range;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_EncodeBool2
 *
 *  INPUTS        :     BOOL_CODER *br  : pointer to instance of a boolean encoder.
 *						int bit         : value to be encoded (0 or 1).
 *						int probability : probability of getting a 0 (0-255) 
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		void
 *
 *  FUNCTION      :     Updates br->BitCounter with approximate cost of encoding
 *                      bit.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void VP6_EncodeBool2 ( BOOL_CODER *br, int bit, int probability )
{
	if (bit)
		br->BitCounter += VP6_ProbCost[255-probability];
	else
		br->BitCounter += VP6_ProbCost[probability];
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DecodeBool
 *
 *  INPUTS        :     BOOL_CODER *br  : pointer to instance of a boolean decoder.
 *						int probability : probability that next symbol is a 0 (0-255) 
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		Next decoded symbol (0 or 1)
 *
 *  FUNCTION      :     Decodes the next symbol (0 or 1) using the specified
 *                      boolean decoder.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
int VP6_DecodeBool ( BOOL_CODER *br, int probability ) 
{
    unsigned int bit=0;
	unsigned int split;
	unsigned int bigsplit;
    unsigned int count = br->count;
    unsigned int range = br->range;
    unsigned int value = br->value;

	split = 1 +  (((range-1) * probability) >> 8);	
    bigsplit = (split<<24);
    
    range = split;
	if(value >= bigsplit)
	{
		range = br->range-split;
		value = value-bigsplit;
		bit = 1;
	}

	if(range>=0x80)
    {
        br->value = value;
        br->range = range;
        return bit;
    }
    else
	{
		do
		{
       	    range +=range;
            value +=value;
            
        	if (!--count) 
        	{
    	        count = 8;
	            value |= br->buffer[br->pos];
        	    br->pos++;
	    	}
        } 
        while(range < 0x80 );
    }
    br->count = count;
    br->value = value;
    br->range = range;
    return bit;
} 

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DecodeBool128
 *
 *  INPUTS        :     BOOL_CODER *br : pointer to instance of a boolean decoder.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		int: Next decoded symbol (0 or 1)
 *
 *  FUNCTION      :     This function determines the next value stored in the 
 *						boolean coder based upon a fixed probability of 0.5 
 *                      (128 in normalized units).
 *
 *  SPECIAL NOTES :     VP6_DecodeBool128() is a special case of VP6_DecodeBool()
 *                      where the input probability is fixed at 128.
 *
 ****************************************************************************/
int VP6_DecodeBool128 (	BOOL_CODER	*br ) 
{
    unsigned int bit;
	unsigned int split;
	unsigned int bigsplit;
    unsigned int count = br->count;
    unsigned int range = br->range;
    unsigned int value = br->value;

    split = ( range + 1) >> 1;
    bigsplit = (split<<24);
    
	if(value >= bigsplit)
	{
		range = (range-split)<<1;
		value = (value-bigsplit)<<1;
		bit = 1;
	}
	else
	{	
		range = split<<1;
		value = value<<1;
		bit = 0;
	}

    if(!--count)
    {
        count=8;
        value |= br->buffer[br->pos];
        br->pos++;        
    }
    br->count = count;
    br->value = value;
    br->range = range;
    return bit;
        
}    

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StartDecode
 *
 *  INPUTS        :     BOOL_CODER *bc		  : pointer to instance of a boolean decoder.
 *						unsigned char *source : pointer to buffer of data to be decoded.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Performs initialization of the boolean decoder.
 *                           
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_StartDecode ( BOOL_CODER *br, unsigned char *source )
{
	br->lowvalue = 0;
	br->range    = 255;
	br->count    = 8;
	br->buffer   = source;
	br->pos      = 0;
	br->value    = (br->buffer[0]<<24)+(br->buffer[1]<<16)+(br->buffer[2]<<8)+(br->buffer[3]);
	br->pos     += 4;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StopDecode
 *
 *  INPUTS        :     BOOL_CODER *bc : pointer to instance of a boolean decoder (UNUSED).
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Performs clean-up of the specified boolean decoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_StopDecode ( BOOL_CODER *bc )
{
}

#endif
