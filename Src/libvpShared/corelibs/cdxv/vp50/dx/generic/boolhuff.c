
/****************************************************************************
*
*   Module Title :     boolhuff.c
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

#define STRICT              /* Strict type checking. */
#include "boolhuff.h"
#ifdef MAPCA 
#include "eti/mm.h"
#endif
/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
 
/****************************************************************************
*  Forward references.
*****************************************************************************
*/       
 
                      
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/
// in the bool coder defined herein a probability of 4 means 4/256 chance its a 0 252/256 chance its a 1 
// so shannon cost of 0 given prob x = 8 - log2(x) | shannon cost of 1 given prob x =  8-log2(256-x)  
#ifndef MAPCA
double shannonCost0[256]=
{
8.000000000,8.000000000,7.000000000,6.415037499,6.000000000,5.678071905,5.415037499,5.192645078,5.000000000,4.830074999,4.678071905,4.540568381,4.415037499,4.299560282,4.192645078,4.093109404,
4.000000000,3.912537159,3.830074999,3.752072487,3.678071905,3.607682577,3.540568381,3.476438044,3.415037499,3.356143810,3.299560282,3.245112498,3.192645078,3.142019005,3.093109404,3.045803690,
3.000000000,2.955605881,2.912537159,2.870716983,2.830074999,2.790546634,2.752072487,2.714597781,2.678071905,2.642447995,2.607682577,2.573735245,2.540568381,2.508146904,2.476438044,2.445411148,
2.415037499,2.385290156,2.356143810,2.327574658,2.299560282,2.272079545,2.245112498,2.218640286,2.192645078,2.167109986,2.142019005,2.117356951,2.093109404,2.069262662,2.045803690,2.022720077,
2.000000000,1.977632187,1.955605881,1.933910810,1.912537159,1.891475543,1.870716983,1.850252880,1.830074999,1.810175441,1.790546634,1.771181310,1.752072487,1.733213459,1.714597781,1.696219252,
1.678071905,1.660149997,1.642447995,1.624960569,1.607682577,1.590609064,1.573735245,1.557056504,1.540568381,1.524266569,1.508146904,1.492205360,1.476438044,1.460841189,1.445411148,1.430144392,
1.415037499,1.400087158,1.385290156,1.370643380,1.356143810,1.341788517,1.327574658,1.313499473,1.299560282,1.285754482,1.272079545,1.258533014,1.245112498,1.231815675,1.218640286,1.205584134,
1.192645078,1.179821038,1.167109986,1.154509949,1.142019005,1.129635280,1.117356951,1.105182237,1.093109404,1.081136763,1.069262662,1.057485495,1.045803690,1.034215715,1.022720077,1.011315313,
1.000000000,0.988772745,0.977632187,0.966576998,0.955605881,0.944717564,0.933910810,0.923184403,0.912537159,0.901967917,0.891475543,0.881058927,0.870716983,0.860448648,0.850252880,0.840128663,
0.830074999,0.820090910,0.810175441,0.800327655,0.790546634,0.780831480,0.771181310,0.761595261,0.752072487,0.742612157,0.733213459,0.723875595,0.714597781,0.705379251,0.696219252,0.687117045,
0.678071905,0.669083122,0.660149997,0.651271846,0.642447995,0.633677786,0.624960569,0.616295708,0.607682577,0.599120564,0.590609064,0.582147485,0.573735245,0.565371772,0.557056504,0.548788888,
0.540568381,0.532394450,0.524266569,0.516184223,0.508146904,0.500154113,0.492205360,0.484300162,0.476438044,0.468618539,0.460841189,0.453105540,0.445411148,0.437757576,0.430144392,0.422571172,
0.415037499,0.407542963,0.400087158,0.392669686,0.385290156,0.377948181,0.370643380,0.363375379,0.356143810,0.348948309,0.341788517,0.334664083,0.327574658,0.320519900,0.313499473,0.306513043,
0.299560282,0.292640868,0.285754482,0.278900811,0.272079545,0.265290380,0.258533014,0.251807150,0.245112498,0.238448768,0.231815675,0.225212940,0.218640286,0.212097441,0.205584134,0.199100100,
0.192645078,0.186218809,0.179821038,0.173451513,0.167109986,0.160796212,0.154509949,0.148250959,0.142019005,0.135813855,0.129635280,0.123483053,0.117356951,0.111256751,0.105182237,0.099133192,
0.093109404,0.087110664,0.081136763,0.075187496,0.069262662,0.063362061,0.057485495,0.051632768,0.045803690,0.039998068,0.034215715,0.028456446,0.022720077,0.017006425,0.011315313,0.005646563
};
double shannonCost1[256]=
{
0.000000000,0.005646563,0.011315313,0.017006425,0.022720077,0.028456446,0.034215715,0.039998068,0.045803690,0.051632768,0.057485495,0.063362061,0.069262662,0.075187496,0.081136763,0.087110664,
0.093109404,0.099133192,0.105182237,0.111256751,0.117356951,0.123483053,0.129635280,0.135813855,0.142019005,0.148250959,0.154509949,0.160796212,0.167109986,0.173451513,0.179821038,0.186218809,
0.192645078,0.199100100,0.205584134,0.212097441,0.218640286,0.225212940,0.231815675,0.238448768,0.245112498,0.251807150,0.258533014,0.265290380,0.272079545,0.278900811,0.285754482,0.292640868,
0.299560282,0.306513043,0.313499473,0.320519900,0.327574658,0.334664083,0.341788517,0.348948309,0.356143810,0.363375379,0.370643380,0.377948181,0.385290156,0.392669686,0.400087158,0.407542963,
0.415037499,0.422571172,0.430144392,0.437757576,0.445411148,0.453105540,0.460841189,0.468618539,0.476438044,0.484300162,0.492205360,0.500154113,0.508146904,0.516184223,0.524266569,0.532394450,
0.540568381,0.548788888,0.557056504,0.565371772,0.573735245,0.582147485,0.590609064,0.599120564,0.607682577,0.616295708,0.624960569,0.633677786,0.642447995,0.651271846,0.660149997,0.669083122,
0.678071905,0.687117045,0.696219252,0.705379251,0.714597781,0.723875595,0.733213459,0.742612157,0.752072487,0.761595261,0.771181310,0.780831480,0.790546634,0.800327655,0.810175441,0.820090910,
0.830074999,0.840128663,0.850252880,0.860448648,0.870716983,0.881058927,0.891475543,0.901967917,0.912537159,0.923184403,0.933910810,0.944717564,0.955605881,0.966576998,0.977632187,0.988772745,
1.000000000,1.011315313,1.022720077,1.034215715,1.045803690,1.057485495,1.069262662,1.081136763,1.093109404,1.105182237,1.117356951,1.129635280,1.142019005,1.154509949,1.167109986,1.179821038,
1.192645078,1.205584134,1.218640286,1.231815675,1.245112498,1.258533014,1.272079545,1.285754482,1.299560282,1.313499473,1.327574658,1.341788517,1.356143810,1.370643380,1.385290156,1.400087158,
1.415037499,1.430144392,1.445411148,1.460841189,1.476438044,1.492205360,1.508146904,1.524266569,1.540568381,1.557056504,1.573735245,1.590609064,1.607682577,1.624960569,1.642447995,1.660149997,
1.678071905,1.696219252,1.714597781,1.733213459,1.752072487,1.771181310,1.790546634,1.810175441,1.830074999,1.850252880,1.870716983,1.891475543,1.912537159,1.933910810,1.955605881,1.977632187,
2.000000000,2.022720077,2.045803690,2.069262662,2.093109404,2.117356951,2.142019005,2.167109986,2.192645078,2.218640286,2.245112498,2.272079545,2.299560282,2.327574658,2.356143810,2.385290156,
2.415037499,2.445411148,2.476438044,2.508146904,2.540568381,2.573735245,2.607682577,2.642447995,2.678071905,2.714597781,2.752072487,2.790546634,2.830074999,2.870716983,2.912537159,2.955605881,
3.000000000,3.045803690,3.093109404,3.142019005,3.192645078,3.245112498,3.299560282,3.356143810,3.415037499,3.476438044,3.540568381,3.607682577,3.678071905,3.752072487,3.830074999,3.912537159,
4.000000000,4.093109404,4.192645078,4.299560282,4.415037499,4.540568381,4.678071905,4.830074999,5.000000000,5.192645078,5.415037499,5.678071905,6.000000000,6.415037499,7.000000000,8.000000000
};

unsigned int shannon64Cost0[256]={
512,512,448,411,384,363,347,332,320,309,299,291,283,275,268,262,
256,250,245,240,235,231,227,222,219,215,211,208,204,201,198,195,
192,189,186,184,181,179,176,174,171,169,167,165,163,161,158,157,
155,153,151,149,147,145,144,142,140,139,137,136,134,132,131,129,
128,127,125,124,122,121,120,118,117,116,115,113,112,111,110,109,
107,106,105,104,103,102,101,100,99,98,97,96,94,93,93,92,
91,90,89,88,87,86,85,84,83,82,81,81,80,79,78,77,
76,76,75,74,73,72,72,71,70,69,68,68,67,66,65,65,
64,63,63,62,61,60,60,59,58,58,57,56,56,55,54,54,
53,52,52,51,51,50,49,49,48,48,47,46,46,45,45,44,
43,43,42,42,41,41,40,39,39,38,38,37,37,36,36,35,
35,34,34,33,33,32,32,31,30,30,29,29,29,28,28,27,
27,26,26,25,25,24,24,23,23,22,22,21,21,21,20,20,
19,19,18,18,17,17,17,16,16,15,15,14,14,14,13,13,
12,12,12,11,11,10,10,9,9,9,8,8,8,7,7,6,
6,6,5,5,4,4,4,3,3,3,2,2,1,1,1,0,
};
unsigned int shannon64Cost1[256]={
0,0,1,1,1,2,2,3,3,3,4,4,4,5,5,6,
6,6,7,7,8,8,8,9,9,9,10,10,11,11,12,12,
12,13,13,14,14,14,15,15,16,16,17,17,17,18,18,19,
19,20,20,21,21,21,22,22,23,23,24,24,25,25,26,26,
27,27,28,28,29,29,29,30,30,31,32,32,33,33,34,34,
35,35,36,36,37,37,38,38,39,39,40,41,41,42,42,43,
43,44,45,45,46,46,47,48,48,49,49,50,51,51,52,52,
53,54,54,55,56,56,57,58,58,59,60,60,61,62,63,63,
64,65,65,66,67,68,68,69,70,71,72,72,73,74,75,76,
76,77,78,79,80,81,81,82,83,84,85,86,87,88,89,90,
91,92,93,93,94,96,97,98,99,100,101,102,103,104,105,106,
107,109,110,111,112,113,115,116,117,118,120,121,122,124,125,127,
128,129,131,132,134,136,137,139,140,142,144,145,147,149,151,153,
155,157,158,161,163,165,167,169,171,174,176,179,181,184,186,189,
192,195,198,201,204,208,211,215,219,222,227,231,235,240,245,250,
256,262,268,275,283,291,299,309,320,332,347,363,384,411,448,512,
};
#endif
// TEMP STATS VARIABLES 

/****************************************************************************
*  Module Static Variables
*****************************************************************************
*/              

#ifdef NOTNORMALIZED
/****************************************************************************
 * 
 *  ROUTINE       :     StartDecode
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *						buffer	ptr to data to start decoding
 *
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function fills initializes the boolean coder
 *                           
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void StartDecode(BOOL_CODER *bc, unsigned char *buffer)
{
    bc->pos = 0;
    bc->value = 0;
    bc->range = 0;
    bc->buffer = buffer;

}

/****************************************************************************
 * 
 *  ROUTINE       :     DecodeBool
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *						prob	probability of getting a 0 normalized to 8 bits 
 *
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		0 or 1 
 *
 *  FUNCTION      :     This function determines the next value stored in the 
 *						boolean coder based upon the probability passed in.
 *						It uses a simple probability model to approximate 
 *						an arithmetic coder.
 *                           
 *
 *  SPECIAL NOTES :     The accuracy of this encoder gets worse as the range 
 *						approaches 0.  This can be avoided with more complex 
 *						normalization functions (as in a standard arithmetic)
 *						coder.  I chose to avoid this for speed reasons.
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
int DecodeBool(
	BOOL_CODER *bc, 
	int probability)
{
	unsigned int split;

	// we don't have enough in our range to tell between a 0 and 1 so get 
	// a new 3 bytes. 
    if( bc->range < 2)
    {
		unsigned char *spot = bc->buffer+bc->pos;
		bc->v[0] = spot[0];
		bc->v[1] = spot[1];
		bc->v[2] = spot[2];

		// range is set to 0x01000001 to avoid having the range * probability 
		// calculation outrange ( this can be handled differently at the cost 
		// of an extra if.
        bc->range = 0x01000000;
        bc->pos+=3;
    }

	// calculate the decision point 
	// black magic: This code works better than if I calculate probability *
	// range and then truncating to 1 ( I can't explain why)
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
 *  ROUTINE       :     StopDecode
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function does clean up for boolean decoder
 *                           
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void StopDecode(BOOL_CODER *bc)
{
    return;
}
/****************************************************************************
 * 
 *  ROUTINE       :     StartEncode
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *						buffer	ptr to hold encoded data
 *
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function fills initializes the boolean coder
 *                           
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void StartEncode(BOOL_CODER *bc, unsigned char *buffer)
{
    bc->pos = 0;
    bc->value = 0;
    bc->range = 0x01000000;
    bc->buffer = buffer;
}

/****************************************************************************
 * 
 *  ROUTINE       :     EncodeBool
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *						x		value to encode
 *						prob	probability of getting a 0 normalized to 8 bits 
 *
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		
 *
 *  FUNCTION      :     This function encodes a boolean value using the 
 *						boolean coder.
 *                           
 *
 *  SPECIAL NOTES :     The accuracy of this encoder gets worse as the range 
 *						approaches 0.  This can be avoided with more complex 
 *						normalization functions (as in a standard arithmetic)
 *						coder.  I chose to avoid this for speed reasons.
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void EncodeBool(BOOL_CODER *bc, int x, int probability)
{
	unsigned int split;

	// we don't have enough in our range to tell between a 0 and 1 so get 
	// a new 3 bytes. 
    if( bc->range < 2 )
    {
		bc->buffer[bc->pos] = bc->v[0];
		bc->buffer[bc->pos+1] = bc->v[1];
		bc->buffer[bc->pos+2] = bc->v[2];
        bc->pos+=3;

		// range is set to 0x01000001 to avoid having the range * probability 
		// calculation outrange ( this can be handled differently at the cost 
		// of an extra if.
        bc->range = 0x01000000;
        bc->value = 0;
    }
	
	// calculate the decision point 
	// black magic: This code works better than if I calculate probability *
	// range and then truncating to 1 ( I can't explain why)
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
 *  ROUTINE       :     StopEncode
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function does clean up for boolean encoder
 *                           
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void StopEncode(BOOL_CODER *bc)
{
	int i;
	for(i=0;i<3;i++)
	{ 
		bc->buffer[bc->pos + i] = 
			*((unsigned char *) &bc->value + i);
	}
    bc->pos+=3;
}

#else 

#ifndef MAPCA
/****************************************************************************
 * 
 *  ROUTINE       :     StartEncode
 *
 *  INPUTS        :     br		ptr to instance of our boolean coder
 *						source	ptr to data to start decoding
 *
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function initializes the boolean coder
 *                           
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void StartEncode
(
	BOOL_CODER *br, 
	unsigned char *source
)
{
	br->lowvalue = 0;
	br->range = 255;
	br->value = 0;
	br->count = -24; 
	br->buffer=source;
	br->pos=0;
}
/****************************************************************************
 * 
 *  ROUTINE       :     StopEncode
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function does clean up for boolean encoder
 *                           
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void StopEncode
(
	BOOL_CODER *br
)
{	
	if(br->count<-16)
		br->lowvalue <<= (24-(br->count&7));
	else if(br->count<-8)
		br->lowvalue <<= (16-(br->count&7));
	else 
		br->lowvalue <<= (8-(br->count&7));

	br->buffer[br->pos++]=(br->lowvalue>>24);
	br->buffer[br->pos++]=(br->lowvalue>>16)& 0xff;
	br->buffer[br->pos++]=(br->lowvalue>>8)& 0xff;
	br->buffer[br->pos++]=(br->lowvalue)& 0xff;
}
	
/****************************************************************************
 * 
 *  ROUTINE       :     EncodeBool
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *						x		value to encode
 *						prob	probability of getting a 0 normalized to 8 bits 
 *
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		
 *
 *  FUNCTION      :     This function encodes a boolean value using the 
 *						boolean coder.
 *                           
 *
 *  SPECIAL NOTES :     This encoder uses normalizations, and is fairly accurate,
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void EncodeBool
(
	BOOL_CODER 	* br,
	int bit,
	int probability
)
{
	unsigned int split;
    split = 1 +  (((br->range-1) * probability) >> 8);
	if(bit)
	{
		br->lowvalue += split;
		br->range -= split;
	}
	else
	{	
		br->range = split;
	}
	while(br->range < 0x80)
	{
		br->range <<= 1;


		if((br->lowvalue & 0x80000000 ))
        {
            int x = br->pos-1;
            while(x>=0 && br->buffer[x] == 0xff)
            {
                br->buffer[x] =(unsigned char)0;
                x--;
            }
            br->buffer[x]+=1;
            
        }
        br->lowvalue  <<= 1;
		if (!++br->count) 
		{
			br->count = -8;
			br->buffer[br->pos++]=(br->lowvalue >> 24);
			br->lowvalue &= 0xffffff;
		}
	}
}




// TEMP

extern const unsigned long ProbCost[256];
extern const unsigned long ProbCost[256];
void EncodeBool2
(
	BOOL_CODER 	* br,
	int bit,
	int probability
)
{
	if (bit)
		br->BitCounter += ProbCost[255-probability];
	else
		br->BitCounter += ProbCost[probability];
}

#endif

/****************************************************************************
 * 
 *  ROUTINE       :     DecodeBool
 *
 *  INPUTS        :     br		ptr to instance of our boolean coder
 *						prob	probability of getting a 0 normalized to 8 bits 
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :		0 or 1 
 *
 *  FUNCTION      :     This function determines the next value stored in the 
 *						boolean coder based upon the probability passed in.
 *						It uses a simple probability model to approximate 
 *						an arithmetic coder.
 *                           
 *
 *  ERRORS        :     None.
 *
 *  SPECIAL NOTES :     The DecodeBool128() is a special case for this
 *                      function that assums the input probability is 128
 *
 ****************************************************************************/
#ifdef MAPCA

int DecodeBool
(
	BOOL_CODER	* br,
	int probability
) 
{

    unsigned int bit;
	unsigned int split;
	unsigned int bigsplit;
    unsigned int lmbdoffset;
    int count = br->count;
    unsigned int range = br->range;
    unsigned int value = br->value;

	split = 1 +  (((range-1) * probability) >> 8);	
    bigsplit = (split<<24);

	if(value >= bigsplit)
	{
		range = range-split;
		value = value-bigsplit;
		bit = 1;
	}
	else
	{	
		range = split;
		bit = 0;
	}
    
    
    if(range>=0x80)
    {
        br->value = value;
        br->range = range;
        return bit;
            
    }

    lmbdoffset = 7 - hmpv_lmo_32(range);
	value 	 <<= lmbdoffset;
	range 	 <<= lmbdoffset;
	count 	  -= lmbdoffset;	

    if(count<=0)
	{
		count +=8;
		value |= ((unsigned int)br->buffer[br->pos]<<(8-count));				
		br->pos++;
		
	}

    br->count = count;
    br->value = value;
    br->range = range;
	return bit;
} 


#else
int DecodeBool
(
	BOOL_CODER	* br,
	int probability
) 
{

    unsigned int bit=0;
	unsigned int split;
	unsigned int bigsplit;
    unsigned int count = br->count;
    unsigned int range = br->range;
    unsigned int value = br->value;

	split = 1 +  (((range-1) * probability) >> 8);	
    bigsplit = (split<<24);

	if(value >= bigsplit)
	{
		range -= split;
		value -= bigsplit;
		bit = 1;
	}
	else
	{	
		range = split;		
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
        value <<=1;
            
        	if (!--count) 
        	{
    	        count = 8;
	            value |= br->buffer[br->pos];
        	    br->pos++;
	    	}
    	}while(range < 0x80 );
    }
    br->count = count;
    br->value = value;
    br->range = range;
	return bit;
} 
#endif
/****************************************************************************
 * 
 *  ROUTINE       :     DecodeBool128
 *
 *  INPUTS        :     br		ptr to instance of our boolean coder
 *
 *  RETURNS       :		0 or 1 
 *
 *  FUNCTION      :     This function determines the next value stored in the 
 *						boolean coder based upon the probability passed in.
 *						It uses a simple probability model to approximate 
 *						an arithmetic coder.
 *
 *  ERRORS        :     None.
 *
 *  SPECIAL NOTES :     The DecodeBool128() is a special case for DecodeBool()
 *                      functionf and assums the input probability is 128
 *
 ****************************************************************************/
int DecodeBool128
(
	BOOL_CODER	* br
) 
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
 *  ROUTINE       :     StartDecode
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *						buffer	ptr to data to start decoding
 *
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function fills initializes the boolean coder
 *                           
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void StartDecode
(
	BOOL_CODER *br,
	unsigned char *source
)
{
	br->lowvalue = 0;
	br->range = 255;
	br->count = 8;
	br->buffer=source;
	br->pos =0;
	br->value = (br->buffer[0]<<24)+(br->buffer[1]<<16)+(br->buffer[2]<<8)+(br->buffer[3]);
	br->pos+=4;
}

/****************************************************************************
 * 
 *  ROUTINE       :     StopDecode
 *
 *  INPUTS        :     bc		ptr to instance of our boolean coder
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function does clean up for boolean decoder
 *                           
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void StopDecode(BOOL_CODER *bc)
{
}
#endif

