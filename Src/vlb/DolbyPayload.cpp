/* $Header: /cvs/root/winamp/vlb/DolbyPayload.cpp,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: DolbyPayload.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: SE bitstream parsing routines
 *
\***************************************************************************/

#include <assert.h>
#include "DolbyPayload.h"

/* Table of warnings returned by SE bitstream reading procedure, spectralExtInfo().

	142		Invalid Dolby SE version number. This decoder only decodes Dolby SE v1.0.
	143     Invalid Dolby SE version number. This decoder only decodes Dolby SE v1.0- v1.3.
	144		Invalid Dolby SE number of bands to ignore.
	146		Invalid # of Chans for Dolby SEv1.0 (only mono or stereo allowed. Check to see if adts_header.channel_config is valid.
	147		Invalid Dolby SE spectral flatness measure.
	148		Invalid Dolby SE noise shaping index.
	149		Dolby SEv1.0: number of bytes read from bitstream != number of bytes in extension payload!
	242		Attempt to read past end of fill element containing Dolby SE payload

 */

/* fDamp */
float baseband_dynamic_noise_shaping_index_lookup_table [] = {
	1.00f, 
	0.98f,
	0.96f,
	0.90f,
	0.84f,
	0.60f,
	0.30f,
	0.00f
};

/* DNS (zlevel) values */
float q_2_unq_power_ratio_lookup_table [] = {
	0.000000f,
	0.030000f,
	0.035755f,
	0.042613f,
	0.050787f,
	0.060529f,
	0.072140f,
	0.085977f,
	0.102470f,
	0.122125f,
	0.145551f,
	0.173471f,
	0.206746f,
	0.246404f,
	0.293669f,
	0.350000f
};


/* Huffman codebooks for LONG,START, and STOP blocks */

Huffman se_hcb_long_lores_sorted_by_length[] = {
	/* value, length, binary codeword */
	{	15,	2,	2}, 
	{	14,	2,	3}, 
	{	13,	3,	2}, 
	{	16,	3,	3}, 
	{	17,	4,	3}, 
	{	12,	5,	4}, 
	{	18,	5,	5}, 
	{	10,	6,	5}, 
	{	11,	6,	6}, 
	{	19,	6,	7}, 
	{	20,	7,	5}, 
	{	21,	7,	6}, 
	{	 7,	7,	7}, 
	{	 8,	7,	8}, 
	{	 9,	7,	9}, 
	{	22,	8,	4}, 
	{	23,	8,	5}, 
	{	24,	8,	6}, 
	{    5,	8,	7}, 
	{	 6,	8,	8}, 
	{    0,	8,	9}, 
	{    3,	9,	1}, 
	{    4,	9,	2}, 
	{   25,	9,	3}, 
	{   26,	9,	4}, 
	{   27,	9,	5}, 
	{    2,	9,	6}, 
	{    1,	9,	7}, 
	{   28,	10,	1}, 
	{   29,	11,	0}, 
	{   30,	11,	1}  
};

/* this table will be used to determine codewords and codeword lengths once 
   the index of the codeword is known. N.B. that two tables are really not needed;
   there are more efficient ways of looking up the codeword and its length
   simultaneously, without two table lookups. However, we will adopt the Huffman decoding 
   style given in this codec which in general uses two tables to determine codeword and length. */


Huffman_lookup se_hcb_long_lores_sorted_by_index[] = {
	/* index, length, codeword */
	{	 0,	8,	-15}, 
	{	 1,	9,	-14}, 
	{	 2,	9,	-13}, 
	{	 3,	9,	-12},
	{	 4,	9,	-11}, 
	{	 5,	8,	-10},
	{	 6,	8,	 -9}, 
	{	 7,	7,	 -8}, 
	{	 8,	7,	 -7}, 
	{	 9,	7,	 -6}, 
	{	10,	6,	 -5}, 
	{	11,	6,	 -4}, 
	{	12,	5,	 -3}, 
	{	13,	3,	 -2},
	{	14,	2,	 -1},
	{	15,	2,	  0}, 
	{	16,	3,	  1}, 
	{	17,	4,	  2},
	{   18,	5,	  3}, 
	{	19,	6,	  4}, 
	{   20,	7,	  5}, 
	{   21,	7,	  6}, 
	{   22,	8,	  7}, 
	{   23,	8,	  8}, 
	{   24,	8,	  9}, 
	{   25,	9,	 10}, 
	{   26,	9,	 11}, 
	{   27,	9,	 12},
	{   28,	10,	 13},
	{   29,	11,	 14}, 
	{   30,	11,	 15}  
};



Huffman se_hcb_long_hires_sorted_by_length[] = {
	/* index,     	length, codeword*/
	{     33,     	3,  	3},
	{     34,     	3,  	7},
	{     35,     	3,  	4},
	{     36,     	3,  	6},
	{     37,     	3,  	1},
	{     32,     	4,  	4},
	{     38,     	4,  	0},
	{     30,     	5,  	11},
	{     31,     	5,  	23},
	{     39,     	5,  	21},
	{     40,     	5,  	3},
	{     29,     	6,  	5},
	{     41,     	6,  	44},
	{     42,     	6,  	21},
	{     27,     	7,  	8},
	{     28,     	7,  	82},
	{     43,     	7,  	91},
	{     24,     	8,  	81},
	{     25,     	8,  	181},
	{     26,     	8,  	163},
	{     44,     	8,  	160},
	{     45,     	8,  	180},
	{     46,     	8,  	19},
	{     47,     	8,  	83},
	{     20,     	9,  	164},
	{     21,     	9,  	36},
	{     22,     	9,  	333},
	{     23,     	9,  	324},
	{     48,     	9,  	325},
	{     49,     	9,  	160},
	{     50,     	9,  	161},
	{     17,     	10,  	75},
	{     18,     	10,  	670},
	{     19,     	10,  	646},
	{     51,     	10,  	664},
	{     52,     	10,  	665},
	{     53,     	10,  	668},
	{     13,     	11,  	660},
	{     14,     	11,  	1342},
	{     15,     	11,  	1291},
	{     16,     	11,  	1290},
	{     54,     	11,  	1289},
	{     55,     	11,  	1288},
	{     56,     	11,  	1338},
	{     57,     	11,  	661},
	{     59,     	11,  	663},
	{     9,     	12,  	299},
	{     10,     	12,  	297},
	{     11,     	12,  	2591},
	{     12,     	12,  	2589},
	{     58,     	12,  	2588},
	{     60,     	12,  	2686},
	{     61,     	12,  	2590},
	{     62,     	12,  	2687},
	{     63,     	12,  	2679},
	{     64,     	12,  	1324},
	{     7,     	13,  	592},
	{     8,     	13,  	5356},
	{     65,     	13,  	596},
	{     66,     	13,  	593},
	{     4,     	14,  	5300},
	{     5,     	14,  	1194},
	{     6,     	14,  	10714},
	{     67,     	14,  	10715},
	{     68,     	14,  	5301},
	{     69,     	14,  	5302},
	{     0,     	15,  	10606},
	{     1,     	15,  	2390},
	{     3,     	15,  	2391},
	{     2,     	16,  	21214},
	{     70,     	16,  	21215}
};

/* this table will be used to determine codewords and codeword lengths once 
   the index of the codeword is known. N.B. that two tables are really not needed;
   there are more efficient ways of looking up the codeword and its length
   simultaneously, without two table lookups. However, we will adopt the Huffman decoding
   style given in this codec which in general uses two tables to determine codeword and length. */


Huffman_lookup se_hcb_long_hires_sorted_by_index[] = {
	/* index,     	length, value*/
	{     0,     	15,  	-36},
	{     1,     	15,  	-35},
	{     2,     	16,  	-34},
	{     3,     	15,  	-33},
	{     4,     	14,  	-32},
	{     5,     	14,  	-31},
	{     6,     	14,  	-30},
	{     7,     	13,  	-29},
	{     8,     	13,  	-28},
	{     9,     	12,  	-27},
	{     10,     	12,  	-26},
	{     11,     	12,  	-25},
	{     12,     	12,  	-24},
	{     13,     	11,  	-23},
	{     14,     	11,  	-22},
	{     15,     	11,  	-21},
	{     16,     	11,  	-20},
	{     17,     	10,  	-19},
	{     18,     	10,  	-18},
	{     19,     	10,  	-17},
	{     20,     	9,  	-16},
	{     21,     	9,  	-15},
	{     22,     	9,  	-14},
	{     23,     	9,  	-13},
	{     24,     	8,  	-12},
	{     25,     	8,  	-11},
	{     26,     	8,  	-10},
	{     27,     	7,  	-9},
	{     28,     	7,  	-8},
	{     29,     	6,  	-7},
	{     30,     	5,  	-6},
	{     31,     	5,  	-5},
	{     32,     	4,  	-4},
	{     33,     	3,  	-3},
	{     34,     	3,  	-2},
	{     35,     	3,  	-1},
	{     36,     	3,  	0},
	{     37,     	3,  	1},
	{     38,     	4,  	2},
	{     39,     	5,  	3},
	{     40,     	5,  	4},
	{     41,     	6,  	5},
	{     42,     	6,  	6},
	{     43,     	7,  	7},
	{     44,     	8,  	8},
	{     45,     	8,  	9},
	{     46,     	8,  	10},
	{     47,     	8,  	11},
	{     48,     	9,  	12},
	{     49,     	9,  	13},
	{     50,     	9,  	14},
	{     51,     	10,  	15},
	{     52,     	10,  	16},
	{     53,     	10,  	17},
	{     54,     	11,  	18},
	{     55,     	11,  	19},
	{     56,     	11,  	20},
	{     57,     	11,  	21},
	{     58,     	12,  	22},
	{     59,     	11,  	23},
	{     60,     	12,  	24},
	{     61,     	12,  	25},
	{     62,     	12,  	26},
	{     63,     	12,  	27},
	{     64,     	12,  	28},
	{     65,     	13,  	29},
	{     66,     	13,  	30},
	{     67,     	14,  	31},
	{     68,     	14,  	32},
	{     69,     	14,  	33},
	{     70,     	16,  	34}
};


/* Huffman codebooks for SHORT blocks */

/* Shortblock Tables derived from shortblock statistics only */

Huffman se_hcb_short_hires_sorted_by_length[] = {
	/* index,     	length, codeword*/
	{     31,     	3,  	3},
	{     32,     	3,  	1},
	{     33,     	3,  	2},
	{     27,     	4,  	15},
	{     28,     	4,  	13},
	{     29,     	4,  	11},
	{     30,     	4,  	9},
	{     34,     	4,  	10},
	{     35,     	4,  	0},
	{     24,     	5,  	2},
	{     25,     	5,  	24},
	{     26,     	5,  	16},
	{     36,     	5,  	28},
	{     22,     	6,  	59},
	{     23,     	6,  	50},
	{     37,     	6,  	34},
	{     38,     	6,  	6},
	{     19,     	7,  	14},
	{     20,     	7,  	102},
	{     21,     	7,  	70},
	{     39,     	7,  	116},
	{     18,     	8,  	207},
	{     40,     	8,  	142},
	{     41,     	8,  	31},
	{     15,     	9,  	60},
	{     16,     	9,  	412},
	{     17,     	9,  	287},
	{     42,     	9,  	413},
	{     12,     	10,  	122},
	{     13,     	10,  	939},
	{     14,     	10,  	938},
	{     43,     	10,  	936},
	{     44,     	10,  	942},
	{     45,     	10,  	941},
	{     11,     	11,  	246},
	{     46,     	11,  	1886},
	{     52,     	11,  	1881},
	{     53,     	11,  	1880},
	{     7,     	12,  	494},
	{     8,     	12,  	2292},
	{     9,     	12,  	2290},
	{     10,     	12,  	2289},
	{     47,     	12,  	2291},
	{     48,     	12,  	2295},
	{     49,     	12,  	3748},
	{     50,     	12,  	2294},
	{     51,     	12,  	2288},
	{     54,     	12,  	3751},
	{     55,     	12,  	3775},
	{     58,     	12,  	3749},
	{     0,     	13,  	7501},
	{     5,     	13,  	7500},
	{     57,     	13,  	4586},
	{     60,     	13,  	991},
	{     2,     	14,  	1981},
	{     3,     	14,  	1980},
	{     4,     	14,  	15096},
	{     6,     	14,  	9175},
	{     59,     	14,  	9174},
	{     61,     	14,  	15097},
	{     1,     	15,  	30197},
	{     56,     	15,  	30196},
	{     62,     	15,  	30199},
	{     63,     	15,  	30198}
};

/* this table will be used to determine codewords and codeword lengths once 
   the index of the codeword is known. N.B. that two tables are really not needed;
   there are more efficient ways of looking up the codeword and its length
   simultaneously, without two table lookups. However, we will adopt the Huffman decoding
   style given in this codec which in general uses two tables to determine codeword and length. */

Huffman_lookup se_hcb_short_hires_sorted_by_index[] = {
	/* index,     	length, value*/
	{     0,     	13,  	-34},
	{     1,     	15,  	-33},
	{     2,     	14,  	-32},
	{     3,     	14,  	-31},
	{     4,     	14,  	-30},
	{     5,     	13,  	-29},
	{     6,     	14,  	-28},
	{     7,     	12,  	-27},
	{     8,     	12,  	-26},
	{     9,     	12,  	-25},
	{     10,     	12,  	-24},
	{     11,     	11,  	-23},
	{     12,     	10,  	-22},
	{     13,     	10,  	-21},
	{     14,     	10,  	-20},
	{     15,     	9,  	-19},
	{     16,     	9,  	-18},
	{     17,     	9,  	-17},
	{     18,     	8,  	-16},
	{     19,     	7,  	-15},
	{     20,     	7,  	-14},
	{     21,     	7,  	-13},
	{     22,     	6,  	-12},
	{     23,     	6,  	-11},
	{     24,     	5,  	-10},
	{     25,     	5,  	-9},
	{     26,     	5,  	-8},
	{     27,     	4,  	-7},
	{     28,     	4,  	-6},
	{     29,     	4,  	-5},
	{     30,     	4,  	-4},
	{     31,     	3,  	-3},
	{     32,     	3,  	-2},
	{     33,     	3,  	-1},
	{     34,     	4,  	0},
	{     35,     	4,  	1},
	{     36,     	5,  	2},
	{     37,     	6,  	3},
	{     38,     	6,  	4},
	{     39,     	7,  	5},
	{     40,     	8,  	6},
	{     41,     	8,  	7},
	{     42,     	9,  	8},
	{     43,     	10,  	9},
	{     44,     	10,  	10},
	{     45,     	10,  	11},
	{     46,     	11,  	12},
	{     47,     	12,  	13},
	{     48,     	12,  	14},
	{     49,     	12,  	15},
	{     50,     	12,  	16},
	{     51,     	12,  	17},
	{     52,     	11,  	18},
	{     53,     	11,  	19},
	{     54,     	12,  	20},
	{     55,     	12,  	21},
	{     56,     	15,  	22},
	{     57,     	13,  	23},
	{     58,     	12,  	24},
	{     59,     	14,  	25},
	{     60,     	13,  	26},
	{     61,     	14,  	27},
	{     62,     	15,  	28},
	{     63,     	15,  	29}
};

Huffman se_hcb_short_lores_sorted_by_length[] = {
	/* index,     	length, codeword*/
	{     14,     	2,  	1},
	{     11,     	3,  	1},
	{     12,     	3,  	7},
	{     13,     	3,  	4},
	{     15,     	3,  	5},
	{     10,     	4,  	0},
	{     16,     	4,  	12},
	{     9,     	5,  	27},
	{     17,     	5,  	2},
	{     8,     	6,  	53},
	{     18,     	6,  	7},
	{     7,     	7,  	104},
	{     5,     	8,  	27},
	{     6,     	8,  	24},
	{     19,     	8,  	210},
	{     4,     	9,  	51},
	{     20,     	9,  	422},
	{     3,     	10,  	106},
	{     21,     	10,  	847},
	{     22,     	10,  	101},
	{     24,     	10,  	100},
	{     25,     	10,  	105},
	{     0,     	11,  	208},
	{     1,     	11,  	209},
	{     2,     	11,  	1693},
	{     23,     	11,  	1692},
	{     27,     	11,  	215},
	{     26,     	12,  	428},
	{     28,     	12,  	429}
};

/* this table will be used to determine codewords and codeword lengths once 
   the index of the codeword is known. N.B. that two tables are really not needed;
   there are more efficient ways of looking up the codeword and its length
   simultaneously, without two table lookups. However, we will adopt the Huffman decoding
   style given in this codec which in general uses two tables to determine codeword and length. */

Huffman_lookup se_hcb_short_lores_sorted_by_index[] = {
	/* index,     	length, value*/
	{     0,     	11,  	-15},
	{     1,     	11,  	-14},
	{     2,     	11,  	-13},
	{     3,     	10,  	-12},
	{     4,     	9,  	-11},
	{     5,     	8,  	-10},
	{     6,     	8,  	-9},
	{     7,     	7,  	-8},
	{     8,     	6,  	-7},
	{     9,     	5,  	-6},
	{     10,     	4,  	-5},
	{     11,     	3,  	-4},
	{     12,     	3,  	-3},
	{     13,     	3,  	-2},
	{     14,     	2,  	-1},
	{     15,     	3,  	0},
	{     16,     	4,  	1},
	{     17,     	5,  	2},
	{     18,     	6,  	3},
	{     19,     	8,  	4},
	{     20,     	9,  	5},
	{     21,     	10,  	6},
	{     22,     	10,  	7},
	{     23,     	11,  	8},
	{     24,     	10,  	9},
	{     25,     	10,  	10},
	{     26,     	12,  	11},
	{     27,     	11,  	12},
	{     28,     	12,  	13}
};

/* Control table for Huffman Codebooks for all blocktypes and resolutions */

SE_power_resolution_table_struct SE_power_resolution_table[NUM_BLOCKTYPES][NUM_BANDRES_OPTIONS] = 
{
		/* powerResolution,		cwLengthLookupTable,					cwValueLookupTable,					num_codewords */

	/* SE_power_resolution_table[0][0,1]. Huffman codebooks for blocktype == 0 (LONG_WINDOW) */
	{
		{SE_BANDRES_2_67_DB,	se_hcb_long_lores_sorted_by_length,		se_hcb_long_lores_sorted_by_index,	31},
		{SE_BANDRES_1_33_DB,	se_hcb_long_hires_sorted_by_length,		se_hcb_long_hires_sorted_by_index,	71}
	},

	/* SE_power_resolution_table[1][0,1]. Huffman codebooks for blocktype == 1 (START_WINDOW) */
	{
		{SE_BANDRES_2_67_DB,	se_hcb_long_lores_sorted_by_length,		se_hcb_long_lores_sorted_by_index,	31},
		{SE_BANDRES_1_33_DB,	se_hcb_long_hires_sorted_by_length,		se_hcb_long_hires_sorted_by_index,	71}
	},

	/* SE_power_resolution_table[2][0,1]. Huffman codebooks for blocktype == 2 (SHORT_WINDOW) */
	{
		{SE_BANDRES_2_67_DB,	se_hcb_short_lores_sorted_by_length,	se_hcb_short_lores_sorted_by_index,	29},
		{SE_BANDRES_1_33_DB,	se_hcb_short_hires_sorted_by_length,	se_hcb_short_hires_sorted_by_index,	64}
	},

	/* SE_power_resolution_table[3][0,1]. Huffman codebooks for blocktype == 3 (STOP_WINDOW) */
	{
		{SE_BANDRES_2_67_DB,	se_hcb_long_lores_sorted_by_length,		se_hcb_long_lores_sorted_by_index,	31},
		{SE_BANDRES_1_33_DB,	se_hcb_long_hires_sorted_by_length,		se_hcb_long_hires_sorted_by_index,	71}
	},

};




// tables for computeSeBandInfo
// LONG,START,STOP blocks only

unsigned int SE_bands_bw_44_48_1024[] = {
	1,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,
	2,3,3,3,3,3,3,3,3,4,
	4,4,4,4,5,5,5,6,6,6,
	7,7,8,8,8,9,9,10,10,11,
	11,12,13,13,14,14,15,16,17,19,
	20,23,25,28,33,39,46,58,74,100,
	147
};

unsigned int SE_bands_bw_32_1024[] = {
	2,2,2,2,2,2,2,2,3,3,
	3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,4,4,4,4,4,4,
	5,5,5,5,6,6,6,6,7,7,
	8,8,9,9,10,10,11,12,13,13,
	14,15,15,16,17,18,19,20,21,22,
	23,24,26,28,31,34,38,43,50,59,
	71,89
};

unsigned int SE_bands_bw_24_22_1024[] = {
	2,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,4,4,4,4,
	4,4,4,4,4,5,5,5,5,5,
	5,6,6,6,7,7,7,8,8,8,
	9,10,10,11,11,12,13,14,15,16,
	16,17,18,19,20,21,22,23,24,25,
	27,28,30,32,34,37,40,44,49,55,
	64
};

unsigned int SE_bands_bw_16_12_11_1024[] = {
	1,4,4,4,4,4,4,4,5,5,
	5,5,5,5,5,5,5,5,5,5,
	5,6,6,6,6,6,7,7,7,7,
	8,8,8,9,9,9,10,10,11,11,
	12,13,14,14,15,16,17,18,20,21,
	22,23,25,26,28,29,31,32,34,35,
	37,38,40,43,45,49
};

unsigned int SE_bands_bw_8_1024[] = {
	8,8,8,8,8,8,8,8,9,9,
	9,9,9,9,9,9,10,10,10,10,
	10,11,11,11,12,12,12,13,13,14,
	14,15,15,16,17,17,18,19,20,21,
	22,24,25,26,28,30,32,34,36,38,
	41,43,46,50
};

SE_Band_Info_struct SE_Band_Info_1024[] = {
	/*	sr	SE_bw_table		max_num_se_bands	num_ignored_upper_bands */
	{48000,	SE_bands_bw_44_48_1024,		71,	1	},
	{44100,	SE_bands_bw_44_48_1024,		71,	1	},
	{32000,	SE_bands_bw_32_1024,		72,	0	},
	{24000,	SE_bands_bw_24_22_1024,		71,	0	},
	{22050,	SE_bands_bw_24_22_1024,		71,	0	},
	{16000,	SE_bands_bw_16_12_11_1024,	66,	0	},
	{12000,	SE_bands_bw_16_12_11_1024,	66,	0	},
	{11025,	SE_bands_bw_16_12_11_1024,	66,	0	},
	{8000,	SE_bands_bw_8_1024,			54,	0	},
	{0,		0,							0,	0	}
};

unsigned int SE_bands_bw_44_48_2048[] = {
	2,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,4,4,4,4,
	4,4,4,4,4,5,5,5,5,5,
	5,6,6,6,7,7,7,8,8,8,
	9,10,10,11,12,12,13,14,15,16,
	17,18,19,20,20,21,22,24,25,26,
	27,29,30,32,35,37,41,45,50,57,
	65,77,92,115,148,201,293
};

unsigned int SE_bands_bw_32_2048[] = {
	2,4,4,4,4,4,4,4,5,5,
	5,5,5,5,5,5,5,5,5,5,
	5,6,6,6,6,6,7,7,7,7,
	8,8,8,9,9,9,10,10,11,12,
	12,13,14,15,15,16,17,19,20,21,
	22,24,25,26,28,29,31,32,34,35,
	37,39,41,43,46,49,52,56,62,68,
	76,86,99,117,141,177
};

unsigned int SE_bands_bw_24_22_2048[] = {
	4,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,7,7,7,7,
	7,7,8,8,8,8,9,9,9,10,
	10,10,11,11,12,12,13,14,14,15,
	16,17,18,19,20,21,23,24,26,27,
	29,31,33,35,37,38,40,42,44,46,
	49,51,53,56,59,63,68,73,79,87,
	97,109,127
};

unsigned int SE_bands_bw_16_12_11_2048[] = {
	3,8,8,8,8,8,8,8,9,9,
	9,9,9,9,9,9,9,10,10,10,
	10,10,11,11,11,12,12,12,13,13,
	14,14,15,15,16,17,18,19,19,20,
	22,23,24,25,27,29,30,32,34,37,
	39,41,44,47,49,52,55,58,61,64,
	67,70,73,77,80,85,90,97
};

unsigned int SE_bands_bw_8_2048[] = {
	14,16,16,16,16,16,16,16,17,17,
	17,17,17,18,18,18,18,19,19,20,
	20,21,21,22,22,23,24,24,25,26,
	27,28,29,30,32,33,35,36,38,40,
	42,45,47,50,53,56,60,63,67,72,
	76,81,86,91,98
};

SE_Band_Info_struct SE_Band_Info_2048[] = {
	/*	sr	SE_bw_table		max_num_se_bands	num_ignored_upper_bands */
	{48000,	SE_bands_bw_44_48_2048,		77,	1	},
	{44100,	SE_bands_bw_44_48_2048,		77,	1	},
	{32000,	SE_bands_bw_32_2048,		76,	0	},
	{24000,	SE_bands_bw_24_22_2048,		73,	0	},
	{22050,	SE_bands_bw_24_22_2048,		73,	0	},
	{16000,	SE_bands_bw_16_12_11_2048,	68,	0	},
	{12000,	SE_bands_bw_16_12_11_2048,	68,	0	},
	{11025,	SE_bands_bw_16_12_11_2048,	68,	0	},
	{8000,	SE_bands_bw_8_2048,			55,	0	},
	{0,		0,							0,	0	}
};

/* Short block tables for which SE band resolution is 1.0 critical bands */

// tables for computeSeBandInfo
// SHORT block only, for blocklength = 2048 samples (normal AAC blocklength: USE_AAC_XFORM is #defined)
// created from compute_se_band_info.m with parameters: N=256; Res = 1.0

unsigned int SE_bands_bw_44_48_128[] = {
	1,1,1,1,1,1,1,2,2,3,
	3,4,5,5,6,8,11,18,43
};

unsigned int SE_bands_bw_32_128[] = {
	1,1,1,1,2,2,2,2,3,3,
	4,5,6,7,8,10,12,17,30
};

unsigned int SE_bands_bw_24_22_128[] = {
	2,2,2,2,2,2,3,3,4,4,
	5,6,8,9,11,13,16,23
};

unsigned int SE_bands_bw_16_12_11_128[] = {
	1,2,2,2,2,2,3,3,3,4,
	4,5,6,8,10,11,13,16,20
};

unsigned int SE_bands_bw_8_128[] = {
	1,4,4,4,4,4,5,5,6,6,
	7,9,10,13,15,20
};

SE_Band_Info_struct SE_Band_Info_128[] = {
	/*	sr	SE_bw_table	max_num_se_bands	num_ignored_upper_bands */
	{48000,	SE_bands_bw_44_48_128,	19,	0	},
	{44100,	SE_bands_bw_44_48_128,	19,	0	},
	{32000,	SE_bands_bw_32_128,	19,	0	},
	{24000,	SE_bands_bw_24_22_128,	18,	0	},
	{22050,	SE_bands_bw_24_22_128,	18,	0	},
	{16000,	SE_bands_bw_16_12_11_128,	19,	0	},
	{12000,	SE_bands_bw_16_12_11_128,	19,	0	},
	{11025,	SE_bands_bw_16_12_11_128,	19,	0	},
	{8000,	SE_bands_bw_8_128,	16,	0	},
	{0,	0,	0,	0	}
};


// tables for computeSeBandInfo
// SHORT block only, for blocklength = 2048 samples (normal AAC blocklength: USE_AAC_XFORM is #defined)
// created from compute_se_band_info.m with parameters: N=512; Res = 1.0

unsigned int SE_bands_bw_44_48_256[] = {
	2,2,2,2,2,2,3,3,3,4,
	5,6,8,9,11,12,16,21,36,86
};

unsigned int SE_bands_bw_32_256[] = {
	1,2,2,2,2,2,3,3,3,4,
	4,5,6,8,10,12,14,16,19,24,
	34,59
};

unsigned int SE_bands_bw_24_22_256[] = {
	1,3,3,3,3,3,3,4,4,5,
	6,7,9,11,13,15,18,21,25,32,
	46
};

unsigned int SE_bands_bw_16_12_11_256[] = {
	1,4,4,4,4,4,5,5,6,6,
	7,9,10,13,15,19,23,27,31,38
};

unsigned int SE_bands_bw_8_256[] = {
	7,7,7,7,8,8,9,10,11,13,
	15,17,21,25,31,39
};


SE_Band_Info_struct SE_Band_Info_256[] = {
	/*	sr	SE_bw_table	max_num_se_bands	num_ignored_upper_bands */
	{48000,	SE_bands_bw_44_48_256,	20,	0	},
	{44100,	SE_bands_bw_44_48_256,	20,	0	},
	{32000,	SE_bands_bw_32_256,	22,	0	},
	{24000,	SE_bands_bw_24_22_256,	21,	0	},
	{22050,	SE_bands_bw_24_22_256,	21,	0	},
	{16000,	SE_bands_bw_16_12_11_256,	20,	0	},
	{12000,	SE_bands_bw_16_12_11_256,	20,	0	},
	{11025,	SE_bands_bw_16_12_11_256,	20,	0	},
	{8000,	SE_bands_bw_8_256,	16,	0	},
	{0,	0,	0,	0	}
};


float random(long *seed)
{
  *seed = (1664525L * *seed) + 1013904223L;
  return((float)(*seed) * (1.0f / 2.14749e+9f));
}

void noiseGen(float noise[], int nsamp)
{
	int bin;
	float sigma = 0., mean = 0.;
	static int count = 0, initialized = 0;
	static long curNoiseState = 101L;
	static float noiseBuf[NOISE_LENGTH];

#ifdef TRI_NOISE
	static float noiseBuf2[NOISE_LENGTH];
#endif

	if(initialized == 0)
	{
		for (bin = 0; bin < NOISE_LENGTH; bin++)
		{
			noiseBuf[bin] = (float) random(&curNoiseState);
			mean += noiseBuf[bin];
		}
		mean /= (float) NOISE_LENGTH;

		for (bin = 0; bin < NOISE_LENGTH; bin++)
		{
			noiseBuf[bin] -= mean;
			sigma += noiseBuf[bin] * noiseBuf[bin];
		}
		sigma = (float) sqrt(sigma / (double) NOISE_LENGTH);

		for (bin = 0; bin < NOISE_LENGTH; bin++)
		{
			noiseBuf[bin] /= sigma;
		}

#ifdef TRI_NOISE
		sigma = 0.;
		mean  = 0.;

		for (bin = 0; bin < NOISE_LENGTH; bin++)
		{
			noiseBuf2[bin] = (float) random(&curNoiseState);
			mean += noiseBuf2[bin];
		}
		mean /= (float) NOISE_LENGTH;

		for (bin = 0; bin < NOISE_LENGTH; bin++)
		{
			noiseBuf2[bin] -= mean;
			sigma += noiseBuf2[bin] * noiseBuf2[bin];
		}
		sigma = (float) sqrt(sigma / (double) NOISE_LENGTH);

		for (bin = 0; bin < NOISE_LENGTH; bin++)
		{
			noiseBuf2[bin] /= sigma;
		}

		for (bin = 0; bin < NOISE_LENGTH; bin++)
		{
			noiseBuf[bin] = (noiseBuf[bin] + noiseBuf2[bin]) / 2;
		}
#endif

		initialized = 1;
	}

	for (bin = 0; bin < nsamp; bin++)
	{
		count++;
		count %= NOISE_LENGTH;
		noise[bin] = noiseBuf[count];
	}
}


/*	This function computes the average energy in each SE band for all bands */
void	computeAvgCopyEnergies(float* pfData,
					int iCopyStart,
					int iCopyStop,
					int num_se_bands,
					int* piBands,
					int halfWindowLength,
					int iWindowSequence,
					float* avgCopyEnergies)
{

	int iCopyPtr;
	int iDestPtr;
	int iBinCounter,iBandCounter;

#ifdef DEBUG_PRINT_BUFFER_MODEL
	// print out buffer model
	static int already_printed_out_buffer_model = 0;
#endif

	/* initialize avgCopyEnergies to zero */
	for (iBandCounter=0;iBandCounter<num_se_bands;iBandCounter++)
	{
		avgCopyEnergies[iBandCounter]=0.f;
	}

	/* initialize the buffer model */

#ifdef NEW_BUFFER_MODEL
	iCopyPtr = iCopyStart + (iCopyStop - iCopyStart) % ((iWindowSequence == 2) ? 2 : 16);
#else
	iCopyPtr = iCopyStart;
#endif
	iDestPtr = iCopyStop;



	for (iBandCounter=0;iBandCounter<num_se_bands;iBandCounter++)
	{


#ifdef NEW_BUFFER_MODEL
		// new buffer model for longs
		// if we don't have enough continuous region in the baseband
		// copy region, then we need to reset the copy pointer.
		// 1) start the pointer at iCopyStart, which is 96 for
		//    long blocks and 12 for short blocks.
		//    N.B. that this setting must be done by the caller of spectralExtend()
		// 2) we need to add to iCopyPtr the number of bins which will bring:
		//    a) for short blocks: the distance between iCopyPtr and iDestPtr
		//       to an EVEN number. By doing this, we can avoid having to deal with
		//       phase flipping issues. This additional "offset" is equal to (iDestPtr-iCopyStart) % 2
		//    b) for non short blocks: the distance between iCopyPtr and iDestPtr
		//       to an exact multiple of 16. 16 is chosen because it is divisible by 8 - in this manner,
		//       we can exactly match tones found in long block bins to short block bins. We also
		//       incorporate another factor of 2 here (2*8=16) so that the copying is done on boundaries
		//       of 2 short block bins. By doing this, we can avoid having to copy short block information
		//       an odd number of short block bins, and thus avoid a phase flip.

		// new buffer model for longs
		if (iCopyPtr + piBands[iBandCounter] > iCopyStop)
		{
			iCopyPtr = iCopyStart + (iDestPtr-iCopyStart) % ((iWindowSequence == 2) ? 2 : 16);
		}
#else		
		// old buffer model for longs
		if (iCopyPtr + piBands[iBandCounter] > iCopyStop)
		{
			iCopyPtr = iCopyStart;
			if ((iDestPtr-iCopyPtr)%2)
			{
				iCopyPtr++;
			}
		}
#endif			

		// simulate band loop in spectral extend
		for(iBinCounter=0;iBinCounter<piBands[iBandCounter];iBinCounter++)
		{
			float fTemp;
			fTemp=pfData[iCopyPtr];
			avgCopyEnergies[iBandCounter]+=(fTemp*fTemp);

#ifdef DEBUG_PRINT_BUFFER_MODEL
			//print out buffer model
			if (iWindowSequence == 2 &&
				((already_printed_out_buffer_model == 0) || (already_printed_out_buffer_model == 1))   )
			{
				// extra zero is to distinguish this buffer model printout from other printouts
				printf("%d %d 0\n",iDestPtr,iCopyPtr);
				already_printed_out_buffer_model = 1;
			}
#endif


			iCopyPtr++;
			iDestPtr++;
			if (iCopyPtr >= iCopyStop)
			{
				iCopyPtr = iCopyStart + (iDestPtr-iCopyStart) % ((iWindowSequence == 2) ? 2 : 16);
			}

		} /* iBinCounter */

		avgCopyEnergies[iBandCounter] /= (float) piBands[iBandCounter];


	} /* iBandCounter */

#ifdef DEBUG_PRINT_BUFFER_MODEL
	if (already_printed_out_buffer_model == 1)
	{
		already_printed_out_buffer_model = 2;
	}
#endif


}







#ifdef DNS
void applyDNS(	float *pfSpectralData,
				DNS_INFORMATION_STRUCT*psDNSInfoStruct,
				int iPrevWnd)
{
	int bin, bnd, grp, sec, copyStop, qCount;
	const int *sfBand;
#ifdef SIGNAL_SUB
	int m;
	int copyStart;
	int step;
	int bufLen;
	int segment;
	float ns_atten_hi, ns_atten_lo;
#endif
	int zeroup, zerodown;
	float normCopy[MAX_NUM_WINDOW_GROUPS][DOLBY_BLOCK_LEN_LONG], noiseEnvelope[MAX_NUM_WINDOW_GROUPS][DOLBY_BLOCK_LEN_LONG];
	float power, ratio, qPower;
	float startScale, prevScale, *coef;
	float slope, intercept, scale, dither;
	int winInGrp, windowBinOffset, windowNumber, numNoiseRecords;
	static long curNoiseState = 0;
	int attenSlopeThreshold;
	int startBin;
	int moduloCopyOffset;

	// compute some preliminaries. These values do not change
	// for all windows in the current frame.
	copyStop = psDNSInfoStruct->iLastBin;
	coef = pfSpectralData;
	sfBand = psDNSInfoStruct->piBandOffsets;
	numNoiseRecords = (psDNSInfoStruct->iWindowSequence == 2 ? 8 : 1);

	// initialize noiseEnvelope, the noise mask for zero-valued mdct
	// coefficients belonging to sections with non-zero huffman codebooks.
  
	for (windowNumber=0;windowNumber<numNoiseRecords;windowNumber++)
	{
		for (bin = 0; bin < DOLBY_BLOCK_LEN_LONG; bin++)
		{
			noiseEnvelope[windowNumber][bin] = 0.;
		}
	}

#ifdef SIGNAL_SUB
	/* Generate a unit-variance signal substitution vector */
	// we'll need to generate substitution vectors for all windows in the group.
	// these noise vectors will be used in two places in this function: 
	// dns and non-zero huffman codebook dithering.
	windowBinOffset = 0;
	windowNumber = 0;

	if (psDNSInfoStruct->iWindowSequence != 2)  /* Long, or transition block */
	{
		copyStart = SS_COPYSTART;
		step = SS_STEP_LONG;
		bufLen = SS_BUF_LEN_LONG;
		attenSlopeThreshold = 19;				/* slope change at bin 120 (long block). This is 2.58kHz for fs = 44.1kHz. */
		startBin = START_BIN_LONG;
		moduloCopyOffset = 14;					/* offset to assure that any holes in the translation are not coincident with 
												   holes in the target spectra.  Only a problem with the first pass of the normCopy 
												   modulo buffer */
		ns_atten_hi = NS_ATTEN1_LONG;
		ns_atten_lo = NS_ATTEN2_LONG;
	}
	else										/* Short block */
	{
		copyStart = SS_COPYSTART / 8;
		step = SS_STEP_SHORT;
		bufLen = SS_BUF_LEN_SHORT;
		attenSlopeThreshold = 4;				/* slope change at bin 16 (short block).  This is 2.75kHz for fs = 44.1kHz */
		startBin = START_BIN_SHORT;
		moduloCopyOffset = 7;					/* see definition of moduloCopyOffset above for long blocks */
		ns_atten_hi = NS_ATTEN1_SHORT;
		ns_atten_lo = NS_ATTEN2_SHORT;
	}

	for(windowNumber = 0; windowNumber < numNoiseRecords; windowNumber++, windowBinOffset += 256)
	{
		for (bin = 0; bin < bufLen; bin++)
		{
			normCopy[windowNumber][bin] = coef[windowBinOffset + copyStart + bin];
		}

		for (segment = 0; segment < bufLen / step; segment++)
		{
			power = 0.;
			qCount = 0;
			for (m = 0; m < step; m++)
			{
				if (normCopy[windowNumber][segment * step + m] != 0.)
				{
					power += normCopy[windowNumber][segment * step + m] * normCopy[windowNumber][segment * step + m];
					qCount++;
				}
			}
			if (qCount)
			{
				power /= (float) qCount;
				power = 1.0f / (float)(sqrt(power));
			}
			for (m = 0; m < step; m++)
			{
				if (normCopy[windowNumber][segment * step + m] == 0.)
				{
					normCopy[windowNumber][segment * step + m] = 1.5f * random(&curNoiseState);
				}
				else
				{
					normCopy[windowNumber][segment * step + m] *= power;
				}
			}
		}
	} // windowNumber

#else
	/* Generate unit-variance random noise vector */

	windowNumber = 0;
	numNoiseRecords = (psDNSInfoStruct->iWindowSequence == 2 ? 8 : 1);

	for(windowNumber=0;windowNumber<numNoiseRecords;windowNumber++)
	{
		power = 0.0f;
		for (bin = 0; bin < DOLBY_BLOCK_LEN_LONG; bin++)
		{
			normCopy[windowNumber][bin] = random(&curNoiseState);
			power += normCopy[windowNumber][bin] * normCopy[windowNumber][bin];
		}
		power /= (float) DOLBY_BLOCK_LEN_LONG;
		power = 1.0f / (float) sqrt(power);
		for (bin = 0; bin < DOLBY_BLOCK_LEN_LONG; bin++)
		{
			normCopy[windowNumber][bin] *= power;
		}
	}
#endif /* SIGNAL_SUB */

	/* Compute DNS */
	windowBinOffset = 0;
	windowNumber = 0;
	for (grp = 0; grp < psDNSInfoStruct->iGroupCount; grp++)
	{
		// this look increments the windowBinOffset index to reflect the 0th bin of the current
		// window in the group. If there is more than one window in the current frame, then
		// we are dealing with 8 short windows, and the data for each of these windows will
		// be in the coef[] array spaced out at intervals of 256 samples. The interval will be
		// 256 samples whether we're dealing with double or single length transforms.

		for (winInGrp = 0; winInGrp < psDNSInfoStruct->iGroupLength[grp]; winInGrp++, windowNumber++, windowBinOffset += 256)
		{
			for (sec = 0; sec < psDNSInfoStruct->psSectionInfoStruct->aiSectionCount[grp]; sec++)
			{
				for (bnd = psDNSInfoStruct->psSectionInfoStruct->aaiSectionStart[grp][sec];
					bnd < psDNSInfoStruct->psSectionInfoStruct->aaiSectionEnd[grp][sec]; bnd++)
				{
					qPower = 0.;
					qCount = 0;

					for (bin = sfBand[bnd]; bin < sfBand[bnd+1]; bin++)
					{
						if (coef[windowBinOffset+bin] != 0.0f)
						{
							qPower += coef[windowBinOffset+bin] * coef[windowBinOffset+bin];
							qCount++;
						}
					} /* bin */

					if (qCount)
					{
						ratio = psDNSInfoStruct->aafDNSRatio[grp][bnd] * (float) sqrt(qPower / (double) qCount);

						for (bin = sfBand[bnd]; bin < sfBand[bnd+1]; bin++)
						{
							if (psDNSInfoStruct->iWindowSequence != 2)					/* long/transition block */
							{
								if ((coef[windowBinOffset+bin] == 0.) && (bin >= startBin))	
								{
									slope = (0.6f - 0.3f) / (float) (copyStop - startBin);
									intercept = 0.3f- (50.0f * slope);
									scale = slope * (float) bin + intercept;
#ifdef SIGNAL_SUB
									coef[windowBinOffset+bin] = normCopy[windowNumber][bin % bufLen] * scale * ratio;
#else
									coef[windowBinOffset+bin] = normCopy[windowNumber][bin] * scale * ratio;
#endif
								}
							}
							else														/* short block */
							{
								if ((coef[windowBinOffset+bin] == 0.) && (bin >= startBin))
								{
#ifdef SIGNAL_SUB
									coef[windowBinOffset+bin] = normCopy[windowNumber][bin % bufLen] * 0.8f * ratio;
#else
									coef[windowBinOffset+bin] = normCopy[windowNumber][bin] * 0.3f * ratio;
#endif
								}
							}
						} /* bin */
					} /* if (qCount) */
				} /* bnd */
			} /* sec */
		} /* WinInGrp */
	} /* grp */


	/* Compute dither for non-zero huffman codebooks, starting from DC and progressing upwards */
	windowBinOffset = 0;
	windowNumber = 0;

	for (grp = 0; grp < psDNSInfoStruct->iGroupCount; grp++)
	{
		// this look increments the windowBinOffset index to reflect the 0th bin of the current
		// window in the group. If there is more than one window in the current frame, then
		// we are dealing with 8 short windows, and the data for each of these windows will
		// be in the coef[] array spaced out at intervals of 256 samples. The interval will be
		// 256 samples whether we're dealing with double or single length transforms.

		for (winInGrp=0;winInGrp<psDNSInfoStruct->iGroupLength[grp];winInGrp++,windowNumber++,windowBinOffset+=256)
		{
			// initialize state variables for the filter
			startScale = 0.;
			prevScale = 0.;

			for (bnd = 0; bnd < psDNSInfoStruct->iMaxSFB; bnd++)
			{
				for (bin = sfBand[bnd]; bin < sfBand[bnd+1]; bin++)
				{
					if (coef[windowBinOffset + bin] == 0.0f)
					{
						noiseEnvelope[windowNumber][bin] = startScale * GLOBAL_ATTEN;
						startScale *= ((bin >= sfBand[attenSlopeThreshold]) ? ns_atten_hi : ns_atten_lo);
						prevScale = 0.0f;
					}
					else
					{
						if (prevScale == 0.0f)		/* This starts off the dither envelope */
						{
							startScale = (float)(fabs(coef[windowBinOffset+bin])) * NSFILT_COEF;
							prevScale = startScale;
						}
						else						/* dither is averaged as low-pass process */
						{
							startScale = (float)(fabs(coef[windowBinOffset+bin])) * NSFILT_COEF + prevScale * (1.0f - NSFILT_COEF);
							prevScale = startScale;
						}
					}
				} /* bin */
			} /* bnd */
		} /* winInGrp */
	} /* grp */


	/* Compute dither for non-zero huffman codebooks, starting from Fs/2 and progressing downwards */
	windowBinOffset = 0;
	windowNumber = 0;

	for (grp = 0; grp < psDNSInfoStruct->iGroupCount; grp++)
	{
		for (winInGrp=0;winInGrp<psDNSInfoStruct->iGroupLength[grp];winInGrp++,windowNumber++,windowBinOffset+=256)
		{
			startScale = 0.;
			prevScale = 0.;
			
			for (bnd = psDNSInfoStruct->iMaxSFB - 1; bnd >= 0; bnd--)
			{
				for (bin = sfBand[bnd+1] - 1; bin >= sfBand[bnd]; bin--)
				{
					if (coef[windowBinOffset+bin] == 0.0f)
					{
						noiseEnvelope[windowNumber][bin] = DSPmax(noiseEnvelope[windowNumber][bin], (startScale * GLOBAL_ATTEN));
						startScale *= ((bin >= sfBand[attenSlopeThreshold]) ? ns_atten_hi : ns_atten_lo);
						startScale *= BACKWARD_MASK_COMP;
						prevScale = 0.0f;
					}
					else
					{
						if (prevScale == 0.0f)		/* This starts off the dither envelope */
						{
							startScale = (float)(fabs(coef[windowBinOffset+bin])) * NSFILT_COEF;
							prevScale = startScale;
						}
						else						/* dither is averaged as low-pass process */
						{
							startScale = (float)(fabs(coef[windowBinOffset+bin])) * NSFILT_COEF + prevScale * (1.0f - NSFILT_COEF);
							prevScale = startScale;
						}
					}
				} /* bin */
			} /* bnd */
		} /* winInGrp */
	} /* grp */

	// merge noise records and coefficient records
	windowBinOffset = 0;
	for(windowNumber=0;windowNumber<numNoiseRecords;windowNumber++,windowBinOffset+=256)
	{
		for (bin = startBin; bin < psDNSInfoStruct->iLastBin; bin++)
		{
#ifdef SIGNAL_SUB
			dither = normCopy[windowNumber][(bin + moduloCopyOffset) % bufLen];
#else
			dither = normCopy[windowNumber][bin];
#endif
			dither *= noiseEnvelope[windowNumber][bin];

			zeroup = ((noiseEnvelope[windowNumber][bin+1] != 0.0f) ? 1 : 0);
			zerodown = ((noiseEnvelope[windowNumber][bin-1] != 0.0f) ? 1 : 0);
			if ((coef[windowBinOffset+bin] == 0.0) && (zeroup || zerodown))
			{
				coef[windowBinOffset+bin] = dither;
				if ((psDNSInfoStruct->iWindowSequence == 3) && (iPrevWnd == 1))		/* done to support legacy bitstreams */
				{
					coef[windowBinOffset+bin] *= 0.5f;
				}
			}
		} /* bin */
	} /* windowNumber */
}

#endif /* DNS */


unsigned char computeSeBandInfo (unsigned int SRate,				/* input: Sample Rate */
								 unsigned int first_SE_bin,			/* input: bin number of the first bin of the extension range */
								 SE_Band_Info_struct* pSE_Band_Info_struct, /* input: SE Band info struct ptr */
								 int* SE_bands_bw,				/* output: SE bandwidths, in bins (array; memory assumed to be already allocated) */
								 int* num_SE_bands)
{
	int i;
	unsigned int sum_bandwidths = 0;
	int band_index = 0;
	int distance_low, distance_high;
	unsigned int* puiSE_bands_bw;
	int band_containing_first_SE_bin;

	/* find the right entry of SE_Band_Info to use based on the input sample rate */

	while(pSE_Band_Info_struct->sr != SRate)
	{
		pSE_Band_Info_struct++;

		if(pSE_Band_Info_struct->sr == 0)
		{
			/* FAILSAFE...rather than just getting stuck in this while() loop until we receive
			 * an access violation, assume we've received MAX_INCONSISTENT_BLOCKS number of blocks 
			 * indicating double length x-form, when the bitstream is really still single length.
			 *
			 * If we've searched the SE_Band_Info_struct and not found a matching SR, assume the
			 * useAACTransform bit is incorrectly indicating double length transforms (0). So,
			 * divide SR by 2 and force the pSE_Band_Info_struct pointer to point to the 
			 * 1024 point version.
			 */
			SRate >>= 1;

			pSE_Band_Info_struct = SE_Band_Info_1024;


			//printf("SE does not support the given sample rate.\n");
			//exit(342);
		}
	}

	/* find the closest entry in SE_bands_bw to first_SE_bin.
	   count the number of se bands.*/
	puiSE_bands_bw = pSE_Band_Info_struct->SE_bands_bw;
	
	sum_bandwidths = 0;
	band_containing_first_SE_bin = 0;

	while(sum_bandwidths < first_SE_bin)
	{
		distance_low = first_SE_bin - sum_bandwidths;
		sum_bandwidths += *puiSE_bands_bw++;
		band_containing_first_SE_bin++;
	}
	
	distance_high = first_SE_bin - sum_bandwidths;
	if (distance_low < -distance_high)
	{
		/* Lower band edge was closer to first_SE_bin */
		band_containing_first_SE_bin--;
		puiSE_bands_bw--;
	}
	
	*num_SE_bands = pSE_Band_Info_struct->SE_max_num_se_bands - band_containing_first_SE_bin;


	/* copy the bandwidth information into the Lines array */
	for(i=0;i<*num_SE_bands; i++)
		SE_bands_bw[i] = *puiSE_bands_bw++;

	/* perform SE band shrinkage / expansion to make first_SE_bin be the first bin of the first
	   extension band */

	i = *num_SE_bands - 1;

	if (distance_low <= -distance_high) 
	{
		/* round-robin decrease band sizes */

		while(distance_low > 0)
		{
			SE_bands_bw[i]--;
			i--;
			if(i<0) i = *num_SE_bands - 1;
			distance_low--;
		}

	} else {

		/* round-robin decrease band sizes */

		while(-distance_high > 0)
		{
			SE_bands_bw[i]++;
			i--;
			if(i<0) i = *num_SE_bands - 1;
			distance_high++;
		}

	}

	assert(pSE_Band_Info_struct->SE_num_ignored_upper_bands >= 0);
	assert(pSE_Band_Info_struct->SE_num_ignored_upper_bands <= ((1<<SE_BAND_IGNORE_BITS)-1));
	
	/* REMOVED IN FAVOR OF ABOVE ASSERTIONS */
	/*
	if (pSE_Band_Info_struct->SE_num_ignored_upper_bands < 0 ||
		pSE_Band_Info_struct->SE_num_ignored_upper_bands > ((1<<SE_BAND_IGNORE_BITS)-1) )
	{
		//printf("SE error: number of ignored bands must be between 0 and 7, inclusive.\n");
		//exit(343);
	}*/

	/* return the number of bands to ignore */
	return (pSE_Band_Info_struct->SE_num_ignored_upper_bands);
}

float decompressSEWeights(int logWeight, int resolution)
{
	static int init = 0;
	static float invLogTable[2][256];
	int n;
	float weight;

	if (!init)
	{
		for (n = 0; n < 256; n++)
		{
			invLogTable[SE_BANDRES_2_67_DB][n] = (float) pow(10.0, ((double) n - 32.0) / 3.75);
			invLogTable[SE_BANDRES_1_33_DB][n] = (float) pow(10.0, ((double) n - 32.0) / 7.50);
		}
		init++;
	}

	if (logWeight < -32)
	{
		logWeight = -32;
	}

	/* some error checking
	   return a very small power if there is a problem with the resolution value */
	if (resolution ==  SE_BANDRES_2_67_DB || resolution == SE_BANDRES_1_33_DB) {
		weight = invLogTable[resolution][logWeight + 32];
	} else {
		weight = invLogTable[0][0];
	}

	return(weight);
}

void computeLPC(float*pfData, int order, int iStart, int iStop, float *alpha)
{
	int n, lag;
	float rxx[10];

	/* Compute autocorrelation */
	for (lag = 0; lag < order + 1; lag++)
	{
		rxx[lag] = 0.f;
		for (n = iStart; n < iStop - lag; n++)
		{
			rxx[lag] += pfData[n] * pfData[n+lag];
		}
	}

	if (rxx[0] != 0.f)
	{
		for (lag = 1; lag < order + 1; lag++)
		{
			rxx[lag] /= rxx[0];
		}
		rxx[0] /= rxx[0];
	}
	else
	{
		rxx[0] = 1.0f;
		for (lag = 1; lag < order + 1; lag++)
		{
			rxx[lag] = 0.f;
		}
	}

	/* Compute 2nd order predictor coefficients */
	alpha[0] = rxx[1] * (1.0f - rxx[2]) / (1.0f - rxx[1] * rxx[1]);
	alpha[1] = (rxx[2] - rxx[1] * rxx[1]) / (1.0f - rxx[1] * rxx[1]);
}

void synthesisFilter(float *Data, float *alpha, float fDamp, int length)
{
	int k, n;
	float power, state[SE_NSHAPE_ORDER] = {0.};

	power = 0;
	alpha[0] *= fDamp;
	alpha[1] *= (fDamp * fDamp);

	for (n = 0; n < length; n++)
	{
		for (k = 0; k < SE_NSHAPE_ORDER; k++)
		{
			Data[n] += alpha[k] * state[k];
		}

		for (k = SE_NSHAPE_ORDER - 1; k > 0; k--)
		{
			state[k] = state[k-1];
		}
		state[0] = Data[n];

		power += Data[n] * Data[n];
	}

	/* Re-normalize to unit variance */
	if (power != 0.)
	{
		power /= (float) length;
		power = 1.0f / (float) sqrt(power);
		for (n = 0; n < length; n++)
		{
			Data[n] *= power;
		}
	}
}

inline int readPastEndOfFillElement(int cnt, int numBitsRead, int numBitsToRead)
{
	if (numBitsRead + numBitsToRead > cnt * 8)
	{
		return (1);
	}
	else
	{
		return (0);
	}
}

// This function returns -(num_bits_used) if it attempts to read beyond the end of the fill element.
// Otherwise, it returns a positive integer corresponding to the index of the decoded Huffman codeword.
int decodeHuffCw(Huffman *h,
				   CDolbyBitStream*poBitStream,
				   int cnt,
				   int numBitsUsed)
{
    int i, j;
    unsigned long cw;
    
    i = h->len;

	// test for read past end of fill element
	if (readPastEndOfFillElement(cnt,numBitsUsed,i))
	{
		return (-numBitsUsed);
	}
    cw = poBitStream->Get(i);
	numBitsUsed += i;


    while (cw != h->cw) 
	{
		h++;
		j = h->len-i;
		i += j;
		cw <<= j;

		// test for read past end of fill element
		if (readPastEndOfFillElement(cnt,numBitsUsed,j))
		{
			return (-numBitsUsed);
		}

		cw |= poBitStream->Get(j);
		numBitsUsed += j;

    }
    return(h->index);
}

int spectralExtInfo(int cnt, DOLBY_PAYLOAD_STRUCT *seData,CDolbyBitStream*poBitStream)
{
	int max_sfb[2];
	int n=0;
	int num_bits_used;
	int num_leftover_bits = 0;
	int last_valid_q_2_unq_power_ratio = 0;
	long get_bits_return_buffer;
	int powerval_index;
	int iNumDNSBits;
	int iNumSpectFlatBits;
	int ch_index,grp_index;
	int i;
	int num_chans_present;
	int num_ref_energy_bits_to_read;

	int start_sfb, end_sfb, sfb_index;
	float q_2_unq_power_ratio;

	max_sfb[0]=seData->aiMaxSFB[0];
	max_sfb[1]=seData->aiMaxSFB[1];

	/* set error flag to zero as a default */
	seData->iDolbyBitStreamWarning = 0;

	/* Account for the bits used to describe the extension type EXT_SE_DATA.
	   This data has already been read from the datastream */
	num_bits_used = 4;

	/* get version_id */

	// test for read past end of fill element
	if (readPastEndOfFillElement(cnt,num_bits_used,3))
	{
		seData->iDolbyBitStreamWarning = 242;
		return ((cnt*8) - num_bits_used);
	}

	get_bits_return_buffer = poBitStream->Get(3);
	num_bits_used += 3;
	if (get_bits_return_buffer >= 4) 
	{
		// Invalid Dolby SE version number. This decoder only decodes Dolby SE v1.0- v1.3.
		seData->iDolbyBitStreamWarning = 142;
		return ((cnt*8) - num_bits_used);
	}

	/* get reserved bits */
	// test for read past end of fill element
	if (readPastEndOfFillElement(cnt,num_bits_used,2))
	{
		seData->iDolbyBitStreamWarning = 242;
		return ((cnt*8) - num_bits_used);
	}

	get_bits_return_buffer = poBitStream->Get(2);
	num_bits_used += 2;

	/* N.B. the reserve bits are not checked against any specific value,
	   since the reserve bits might depend on the revision number. Since we
	   are allowing multiple SE versions to be decoded by this decoder, the
	   reserve bits cannot be known for sure. */


	/* Get iSEPowerResolution parameter from bitstream */

	// test for read past end of fill element
	if (readPastEndOfFillElement(cnt,num_bits_used,1))
	{
		seData->iDolbyBitStreamWarning = 242;
		return ((cnt*8) - num_bits_used);
	}

	get_bits_return_buffer = poBitStream->Get(1);
	num_bits_used += 1;
	seData->iSEPowerResolution = get_bits_return_buffer;


	/* get single length x-form bit (1 = single length, 0 = double length) */
	// test for read past end of fill element
	if (readPastEndOfFillElement(cnt,num_bits_used,1))
	{
		seData->iDolbyBitStreamWarning = 242;
		return ((cnt*8) - num_bits_used);
	}

	get_bits_return_buffer = poBitStream->Get(1);
	num_bits_used += 1;

#ifdef USE_XFORM_HYSTERESIS
	static int iConsistentBlockCount		  = 0;
	static int iPreviousDoubleLengthXFormFlag = 2; //Init to invalid value so that first block is considered "consistent" */
#endif

	if (get_bits_return_buffer)
	{
		/* Bit set indicates single length x-form */
#ifdef USE_XFORM_HYSTERESIS
		if (iPreviousDoubleLengthXFormFlag == 1)
		{
			/* Last block indicated double length x-form, current block indicates single. */
			if (++iConsistentBlockCount == XFORM_HYSTERESIS_LENGTH)
			{
				seData->iUsesDoubleLengthXForm = 0;
				
				iConsistentBlockCount = 0;
			}
			else
			{
				seData->iUsesDoubleLengthXForm = 1;
			}
		}
		else
		{
			seData->iUsesDoubleLengthXForm = 0;
			iConsistentBlockCount = 0;
		}
#else 
		seData->iUsesDoubleLengthXForm = 0;
#endif
	}
	else
	{
		/* Bit clear indicates double length */
		
#ifdef USE_XFORM_HYSTERESIS
		if (iPreviousDoubleLengthXFormFlag == 0)
		{
			/* Last block indicated single length x-form, current block indicates double. */
			if (++iConsistentBlockCount == XFORM_HYSTERESIS_LENGTH)
			{
				seData->iUsesDoubleLengthXForm = 1;

				iConsistentBlockCount = 0;
			}
			else
			{
				seData->iUsesDoubleLengthXForm = 0;
			}
		}
		else
		{	
			seData->iUsesDoubleLengthXForm = 1;
			iConsistentBlockCount = 0;
		}
#else
		seData->iUsesDoubleLengthXForm = 1;
#endif
	}

#ifdef USE_XFORM_HYSTERESIS
	iPreviousDoubleLengthXFormFlag = seData->iUsesDoubleLengthXForm;
#endif
	/* verify that the source bitstream is either mono or stereo */	
	num_chans_present = seData->iChannels;
	if(num_chans_present < 1 || num_chans_present > 2)
	{
		// Invalid # of Chans for Dolby SEv1.0 (only mono or stereo allowed. Check to see if adts_header.channel_config is valid.
		seData->iDolbyBitStreamWarning = 146;
		return ((cnt*8) - num_bits_used);
	}

	for(ch_index=0;ch_index<num_chans_present;ch_index++)
	{
		if (seData->asDNSInfoStruct[ch_index].iWindowSequence == 2)
		{
			iNumDNSBits = SE_DNS_PWR_BITS_SHORT;
			iNumSpectFlatBits = SE_SPECT_FLAT_BITS_SHORT;
		}
		else
		{
			iNumDNSBits = SE_DNS_PWR_BITS_LONG;
			iNumSpectFlatBits = SE_SPECT_FLAT_BITS_LONG;
		}

		/* get number of bands to ignore */
		// test for read past end of fill element
		if (readPastEndOfFillElement(cnt,num_bits_used,3))
		{
			seData->iDolbyBitStreamWarning = 242;
			return ((cnt*8) - num_bits_used);
		}

		get_bits_return_buffer = poBitStream->Get(3);
		num_bits_used += 3;
		if ( (get_bits_return_buffer < 0) || (get_bits_return_buffer > 7) ) 
		{
			// Invalid Dolby SE number of bands to ignore.
			seData->iDolbyBitStreamWarning = 144;
			return ((cnt*8) - num_bits_used);
		}
		seData->SE_num_ignored_upper_bands[ch_index] = get_bits_return_buffer;

		for(grp_index=0;grp_index<seData->iGroupCount[ch_index];grp_index++)
		{
			/* get spectral flatness measure */

			// test for read past end of fill element
			if (readPastEndOfFillElement(cnt,num_bits_used,8))
			{
				seData->iDolbyBitStreamWarning = 242;
				return ((cnt*8) - num_bits_used);
			}

			get_bits_return_buffer = poBitStream->Get(iNumSpectFlatBits);
			num_bits_used += iNumSpectFlatBits;
			if ( (get_bits_return_buffer < 0) || (get_bits_return_buffer > (1 << iNumSpectFlatBits) - 1) ) 
			{
				// Invalid Dolby SE spectral flatness measure.
				seData->iDolbyBitStreamWarning = 147;
				return ((cnt*8) - num_bits_used);
			}
			seData->sfm[ch_index][grp_index] = get_bits_return_buffer;

			/* get baseband_dynamic_noise_shaping_index */

			// test for read past end of fill element
			if (readPastEndOfFillElement(cnt,num_bits_used,3))
			{
				seData->iDolbyBitStreamWarning = 242;
				return ((cnt*8) - num_bits_used);
			}

			if (seData->asDNSInfoStruct[ch_index].iWindowSequence != 2)
			{
				get_bits_return_buffer = poBitStream->Get(3);
				num_bits_used += 3;
				if ( (get_bits_return_buffer < 0) || (get_bits_return_buffer > 7) ) 
				{
					// Invalid Dolby SE noise shaping index.
					seData->iDolbyBitStreamWarning = 148;
					return ((cnt*8) - num_bits_used);
				}
				seData->fdamp[ch_index][grp_index] = baseband_dynamic_noise_shaping_index_lookup_table[get_bits_return_buffer];
			}
			else
			{
				seData->fdamp[ch_index][grp_index] = baseband_dynamic_noise_shaping_index_lookup_table[0];
			}
				
			/* get dolby DNS information */

			/* nsect_export, and sect_export taken directly from huffdec2.c: getics().
			   even values for the second index = the hcb values for that section;
			   odd values for the second index = the last valid sfb index for that section + 1 */

			for(i=0;i<seData->asDNSInfoStruct[ch_index].psSectionInfoStruct->aiSectionCount[grp_index];i++)
			{					
				/* compute starting and ending sfb numbers for this hcb section */
				start_sfb = seData->asDNSInfoStruct[ch_index].psSectionInfoStruct->aaiSectionStart[grp_index][i];
				end_sfb = seData->asDNSInfoStruct[ch_index].psSectionInfoStruct->aaiSectionEnd[grp_index][i];

				if ((seData->asDNSInfoStruct[ch_index].psSectionInfoStruct->aaiSectionCodebooks[grp_index][i] > 0) &&
					(seData->asDNSInfoStruct[ch_index].psSectionInfoStruct->aaiSectionCodebooks[grp_index][i] < 12))
				{
					// test for read past end of fill element
					if (readPastEndOfFillElement(cnt,num_bits_used, iNumDNSBits))
					{
						seData->iDolbyBitStreamWarning = 242;
						return ((cnt*8) - num_bits_used);
					}

					get_bits_return_buffer = poBitStream->Get(iNumDNSBits);
					num_bits_used += iNumDNSBits;
					if (seData->asDNSInfoStruct[ch_index].iWindowSequence == 2)
					{
						q_2_unq_power_ratio = q_2_unq_power_ratio_lookup_table[2 * get_bits_return_buffer + 1];
					}
					else
					{
						q_2_unq_power_ratio = q_2_unq_power_ratio_lookup_table[get_bits_return_buffer];
					}
				}
				else
				{
					/* For HCB's >0 and <12, q_2_unq_power_ratio for the section should never be used.
					   However, we'll set the value to 0.0f to be safe, effectively turning DNS off for
					   this HCB section. */
					q_2_unq_power_ratio = 0.0f;
				}

				/* assign the zpower value just read from the bitstream to the sfb's to
				   which the current section corresponds. */

				for (sfb_index = start_sfb; sfb_index<end_sfb; sfb_index++) 
				{
					seData->asDNSInfoStruct[ch_index].aafDNSRatio[grp_index][sfb_index] = q_2_unq_power_ratio;
				}
			}

			/* get power normalization values */

			/* first, determine the number of se bands in the datastream. 
			   to do this, we'll use computeSeBandInfo. */

			if (max_sfb[ch_index] == 0)
			{
				seData->num_se_bands[ch_index] = 0;
			}
			else
			{
				// decide which lookup table to use when determining
				// the se banding structure for the current frame.
				// this is a function of transform size and window type
				SE_Band_Info_struct* pSE_Band_Info_table_to_use; 
				if (seData->asDNSInfoStruct[ch_index].iWindowSequence == 2)
				{
					// SHORT window
					if (seData->iUsesDoubleLengthXForm)
					{
						pSE_Band_Info_table_to_use = SE_Band_Info_256;
					}
					else
					{
						pSE_Band_Info_table_to_use = SE_Band_Info_128;
					}
				}
				else
				{
					// LONG,START,or STOP window
					if (seData->iUsesDoubleLengthXForm)
					{
						pSE_Band_Info_table_to_use = SE_Band_Info_2048;
					}
					else
					{
						pSE_Band_Info_table_to_use = SE_Band_Info_1024;
					}
				}

				computeSeBandInfo ((seData->iUsesDoubleLengthXForm) ? (2 * seData->iSampleRate) : seData->iSampleRate,
									seData->aiCopyStop[ch_index],	
									pSE_Band_Info_table_to_use,
									seData->seBands[ch_index][grp_index],	
									&seData->num_se_bands[ch_index]);
				seData->num_se_bands[ch_index] -= seData->SE_num_ignored_upper_bands[ch_index];
			}

			/* read the reference energy */
			// test for read past end of fill element

			if (seData->asDNSInfoStruct[ch_index].iWindowSequence == 2)
			{
				num_ref_energy_bits_to_read = SE_REF_ENERG_BITS_SHORT;
			}
			else
			{
				num_ref_energy_bits_to_read = SE_REF_ENERG_BITS_LONG;
			}

			if (readPastEndOfFillElement(cnt,num_bits_used,num_ref_energy_bits_to_read))
			{
				seData->iDolbyBitStreamWarning = 242;
				return ((cnt*8) - num_bits_used);
			}

			get_bits_return_buffer = poBitStream->Get(num_ref_energy_bits_to_read);
			num_bits_used += num_ref_energy_bits_to_read;
			seData->delta_power_values[ch_index][grp_index][0] = get_bits_return_buffer;

			/* read (num_se_bands[ch_index] - 1) Huffman encoded power values from the bitstream
			   (we've already read the first, absolute value from the file. */

			for(i=0;i<seData->num_se_bands[ch_index]-1;i++)
			{
				powerval_index = decodeHuffCw(
					SE_power_resolution_table[seData->asDNSInfoStruct[ch_index].iWindowSequence]
					                         [seData->iSEPowerResolution].cwLengthLookupTable,
					poBitStream,
					cnt,
					num_bits_used);

				// check to see if decodeHuffCw wanted to read beyond the end of the Fill element
				if (powerval_index < 0) 
				{
					seData->iDolbyBitStreamWarning = 242;
					return ((cnt*8) - (-powerval_index)); // -powerval_index is the number of bits already read by decodeHuffCw
				}

				seData->delta_power_values[ch_index][grp_index][i+1] = 
					(SE_power_resolution_table[seData->asDNSInfoStruct[ch_index].iWindowSequence]
					                          [seData->iSEPowerResolution].cwValueLookupTable)
					                                                                              [powerval_index].cw;

				num_bits_used += 
					(SE_power_resolution_table[seData->asDNSInfoStruct[ch_index].iWindowSequence]
					                          [seData->iSEPowerResolution].cwValueLookupTable)
											                                                      [powerval_index].len;		
			}
		} /* for (grp...) */
	} /* for ch_index */

	/* calculate number of bytes used and read fill bits to end on a multiple of 8  */
	num_leftover_bits = num_bits_used % 8;
	if (num_leftover_bits != 0)
	{
		/* debug: at this point, byte_align() could be called
		   in lieu of the next two lines of code. If byte_align is called,
		   it should return the number 8 - leftover_bits */

		// test for read past end of fill element
		if (readPastEndOfFillElement(cnt,num_bits_used,8 - num_leftover_bits))
		{
			seData->iDolbyBitStreamWarning = 242;
			return ((cnt*8) - num_bits_used);
		}

		get_bits_return_buffer = poBitStream->Get(8 - num_leftover_bits);
		num_bits_used += (8 - num_leftover_bits);

		if (get_bits_return_buffer != 0)
		{
			/* 243     SE Bitstream synchronization error.*/
			seData->iDolbyBitStreamWarning = 243;
			return ((cnt*8) - num_bits_used);
		}
	}

	/* at this point, n should be an exact integer, since we're at an even byte boundary */
	n = (num_bits_used / 8);

	/* At this point, there might be extra "pad bytes" in the SE data stream. This might
	   occur if the encoder found that it needed to "purge" some bits from the bit resevoir by including
	   pad (all zero) bytes in the current FILL element after the SE data. The encoder might have chosen to
	   do this if it found that the number of bits it wanted to purge was greater than the number of bits needed
	   to transmit the SE bitstream. 

       In this case, just read out the pad bytes until n = cnt. */
	while(n < cnt)
	{
		// test for read past end of fill element
		if (readPastEndOfFillElement(cnt,num_bits_used,8))
		{
			seData->iDolbyBitStreamWarning = 242;
			return ((cnt*8) - num_bits_used);
		}

		get_bits_return_buffer = poBitStream->Get(8);
		num_bits_used += 8;

		if (get_bits_return_buffer != 0)
		{
			/* 243     SE Bitstream synchronization error.*/
			seData->iDolbyBitStreamWarning = 243;
			return ((cnt*8) - num_bits_used);
		}
		n += 1;
	}

	/* now, a sanity check: make sure we've read the correct number of bytes from the bitstream! */
	if (n != cnt)
	{
		// Dolby SEv1.0: number of bytes read from bitstream != number of bytes in extension payload!
		seData->iDolbyBitStreamWarning = 149;
		return ((cnt*8) - num_bits_used);
	}

	// return the number of *bits* left in the payload, after reading all of the SE data.
	// if there are no more payload bits, then the return value should be zero.

	return (8*(cnt-n));
}

void spectralExtend(float* pfData,
					 int iCopyStart,
					 int iCopyStop,
					 int iSfm,
					 int iBandCount,
					 int* piBands,
					 int* piDeltaPowerValues,
					 float fDamp,
					 int iHBlockSize,
					 int iWnd,
					 int iSEPowerResolution,
					 int iPreviousWnd,
					 float* avgCopyEnergies) 
{
	/* SE declarations */
	int iBandCounter;
	int iCounter;
	int iCopyPtr;
	int iDestPtr;
	int iDestPtrThisBand;

	float fOneOverHBlockSize = 1.0f / (float) iHBlockSize;
	float fCopyEnergy, fDestEnergy;
	float fSFM;
	float fNoiseRatio;
	float fPhase;
	float fScale0, fScale1, fNoise[2048], fAlpha[SE_NSHAPE_ORDER];
	int iNoiseOffset;

	/* additional declarations */
	int iCopyWidth;
	static int piWeights[MAX_NUM_POWER_VALUES];

	static unsigned char uchIsOddFrame;
	static int iFunctionInitialized = 0;

#ifdef DEBUG_PRINT_BUFFER_MODEL
	// print out buffer model
	static int already_printed_out_buffer_model = 0;
#endif

	if (!iFunctionInitialized)
	{
		uchIsOddFrame = 0;
		iFunctionInitialized = 1;
	}

	/* Compute some preliminaries */

	iCopyWidth = iCopyStop - iCopyStart;
	noiseGen(fNoise, ((iWnd == 2) ? 256 : 2048));

	/* Time-Shape the noise prior to injection into extension */

	computeLPC(pfData, SE_NSHAPE_ORDER, iCopyStart, iCopyStop, fAlpha);

	synthesisFilter(fNoise, fAlpha, fDamp, ((iWnd == 2) ? 256 : 2048));

	piWeights[0] = piDeltaPowerValues[0] - 32;

	for (iBandCounter=1;iBandCounter<iBandCount;iBandCounter++) 
	{
		piWeights[iBandCounter] = piDeltaPowerValues[iBandCounter] + piWeights[iBandCounter-1];
	}

	/* Compute how much noise to use */

	fSFM = (float) iSfm;
	fSFM /= ((iWnd == 2) ? ((1 << SE_SPECT_FLAT_BITS_SHORT) - 1) : ((1 << SE_SPECT_FLAT_BITS_LONG) - 1));

	iNoiseOffset = (int) ((fSFM-0.15)*(float)iHBlockSize);


	/* Create Noise records for use later */

#ifdef NEW_BUFFER_MODEL
	iCopyPtr = iCopyStart + (iCopyStop - iCopyStart) % ((iWnd == 2) ? 2 : 16);
#else
	iCopyPtr = iCopyStart;
#endif
	iDestPtr = iCopyStop;

	/* Do Spectral Replication: */

	for(iBandCounter=0;iBandCounter<iBandCount;iBandCounter++)
	{
		fCopyEnergy=0.f;
		fDestEnergy=decompressSEWeights(piWeights[iBandCounter],iSEPowerResolution);

#ifdef NEW_BUFFER_MODEL
		// new buffer model for longs
		// if we don't have enough continuous region in the baseband
		// copy region, then we need to reset the copy pointer.
		// 1) start the pointer at iCopyStart, which is 96 for
		//    long blocks and 12 for short blocks.
		//    N.B. that this setting must be done by the caller of spectralExtend()
		// 2) we need to add to iCopyPtr the number of bins which will bring:
		//    a) for short blocks: the distance between iCopyPtr and iDestPtr
		//       to an EVEN number. By doing this, we can avoid having to deal with
		//       phase flipping issues. This additional "offset" is equal to (iDestPtr-iCopyStart) % 2
		//    b) for non short blocks: the distance between iCopyPtr and iDestPtr
		//       to an exact multiple of 16. 16 is chosen because it is divisible by 8 - in this manner,
		//       we can exactly match tones found in long block bins to short block bins. We also
		//       incorporate another factor of 2 here (2*8=16) so that the copying is done on boundaries
		//       of 2 short block bins. By doing this, we can avoid having to copy short block information
		//       an odd number of short block bins, and thus avoid a phase flip.

		// new buffer model for longs
		if (iCopyPtr + piBands[iBandCounter] > iCopyStop)
		{
			iCopyPtr = iCopyStart + (iDestPtr-iCopyStart) % ((iWnd == 2) ? 2 : 16);
		}
#else		
		// old buffer model for longs
		if (iCopyPtr + piBands[iBandCounter] > iCopyStop)
		{
			iCopyPtr = iCopyStart;
			if ((iDestPtr-iCopyPtr)%2)
			{
				iCopyPtr++;
			}
		}
#endif
		iDestPtrThisBand = iDestPtr;		/* Save initial value for next piBand loop */

		/* First Copy up the data and compute the copied energy: */

		// replicate spectrum
		for(iCounter=0;iCounter<piBands[iBandCounter];iCounter++)
		{
			float fTemp;
			fTemp=pfData[iCopyPtr];
//			fCopyEnergy+=(fTemp*fTemp);

			/* Work out phase issues: */
			if(uchIsOddFrame){
				if((iDestPtr-iCopyPtr)%2) fPhase=-1.0;
				else fPhase=1.0;
			}
			else fPhase=1.0;

			pfData[iDestPtr]=fPhase*fTemp;

#ifdef DEBUG_PRINT_BUFFER_MODEL
			//print out buffer model
			if (iWnd == 2 &&
				((already_printed_out_buffer_model == 0) || (already_printed_out_buffer_model == 1))   )
			{
				printf("%d %d\n",iDestPtr,iCopyPtr);
				already_printed_out_buffer_model = 1;
			}
#endif
			iCopyPtr++;
			iDestPtr++;
			if (iCopyPtr >= iCopyStop)
			{
				iCopyPtr = iCopyStart + (iDestPtr-iCopyStart) % ((iWnd == 2) ? 2 : 16);
			}
		}

//		/* Compute average energy for the band. */
//		fCopyEnergy/=(float)piBands[iBandCounter];

		// Even though average energies have already been computed, don't divide by zero
		fCopyEnergy = avgCopyEnergies[iBandCounter];

		if(fCopyEnergy==0.0f) fCopyEnergy=1.0f;
		fScale0 = (float) sqrt(fDestEnergy / fCopyEnergy);
		fScale1 = (float) sqrt(fDestEnergy);
		iDestPtr = iDestPtrThisBand;

		for(iCounter=0;iCounter<piBands[iBandCounter];iCounter++)
		{
			float fTemp;
			if (iDestPtr-iNoiseOffset <= iHBlockSize)
			{
				fNoiseRatio=((float)(iDestPtr-iNoiseOffset))*fOneOverHBlockSize;
			}
			else
			{
				fNoiseRatio = 1.;
			}

			// legacy kludge: originally, this if statement read
			// if (iWnd == 3)... We'd like to remove this if statement, and control noise
			// inejction for shortblocks is handled directly by setting encoder parameters. However, we
			// have content out there where shortblocks are disabled swhich is correctly decoded 
			// when the following statement reads (iWnd == 3). Thus, we have decided to fix the problem
			// by adding a state variable which contains the previous window.

			if (iWnd == 3 && iPreviousWnd != 2)
			{
				fNoiseRatio = (fNoiseRatio<0.08f)?fNoiseRatio:0.08f;
			}
			fNoiseRatio=(fNoiseRatio>0.0f)?fNoiseRatio:0.0f;

			pfData[iDestPtr] *= (fScale0 * (1.0f - fNoiseRatio));
			pfData[iDestPtr] += (fScale1 * fNoise[iDestPtr] * fNoiseRatio);
			fTemp = 2.0f * fNoiseRatio;
			pfData[iDestPtr] /= (float) sqrt(1.0f - fTemp + fTemp * fNoiseRatio);

			iDestPtr++;
		}
	}

	for(;iDestPtr<iHBlockSize;iDestPtr++)
	{
		pfData[iDestPtr] = 0.0f;
	}

	uchIsOddFrame^=1;

#ifdef DEBUG_PRINT_BUFFER_MODEL
	if (already_printed_out_buffer_model == 1)
	{
		already_printed_out_buffer_model = 2;
	}
#endif

}