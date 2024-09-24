/*!
************************************************************************
* \file vlc.c
*
* \brief
*    VLC support functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Inge Lille-Langøy               <inge.lille-langoy@telenor.com>
*    - Detlev Marpe                    <marpe@hhi.de>
*    - Gabi Blaettermann
************************************************************************
*/
#include "contributors.h"

#include "global.h"
#include "vlc.h"
#include "elements.h"
#include "optim.h"
#include <emmintrin.h>

// A little trick to avoid those horrible #if TRACE all over the source code
#if TRACE
#define SYMTRACESTRING(s) strncpy(symbol.tracestring,s,TRACESTRING_SIZE)
#else
#define SYMTRACESTRING(s) // do nothing
#endif

static int ShowBits (const uint8_t buffer[],int totbitoffset,int bitcount, int numbits);

// Note that all NA values are filled with 0

/*!
*************************************************************************************
* \brief
*    ue_v, reads an ue(v) syntax element, the length in bits is stored in
*    the global p_Dec->UsedBits variable
*
* \param tracestring
*    the string for the trace file
*
* \param bitstream
*    the stream to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
int ue_v (const char *tracestring, Bitstream *bitstream)
{
	SyntaxElement symbol;

	//assert (bitstream->streamBuffer != NULL);
	symbol.mapping = linfo_ue;   // Mapping rule
	SYMTRACESTRING(tracestring);
	readSyntaxElement_VLC (&symbol, bitstream);
	return symbol.value1;
}


/*!
*************************************************************************************
* \brief
*    ue_v, reads an se(v) syntax element, the length in bits is stored in
*    the global p_Dec->UsedBits variable
*
* \param tracestring
*    the string for the trace file
*
* \param bitstream
*    the stream to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
int se_v (const char *tracestring, Bitstream *bitstream)
{
	SyntaxElement symbol;

	//assert (bitstream->streamBuffer != NULL);
	symbol.mapping = linfo_se;   // Mapping rule: signed integer
	SYMTRACESTRING(tracestring);
	readSyntaxElement_VLC (&symbol, bitstream);
	return symbol.value1;
}


/*!
*************************************************************************************
* \brief
*    ue_v, reads an u(v) syntax element, the length in bits is stored in
*    the global p_Dec->UsedBits variable
*
* \param LenInBits
*    length of the syntax element
*
* \param tracestring
*    the string for the trace file
*
* \param bitstream
*    the stream to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
int u_v (int LenInBits, const char*tracestring, Bitstream *bitstream)
{
	return readSyntaxElement_FLC(bitstream, LenInBits);
}

/*!
*************************************************************************************
* \brief
*    i_v, reads an i(v) syntax element, the length in bits is stored in
*    the global p_Dec->UsedBits variable
*
* \param LenInBits
*    length of the syntax element
*
* \param tracestring
*    the string for the trace file
*
* \param bitstream
*    the stream to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
int i_v (int LenInBits, const char*tracestring, Bitstream *bitstream)
{
	int val;
	val = readSyntaxElement_FLC (bitstream, LenInBits);

	// can be negative
	val = -( val & (1 << (LenInBits - 1)) ) | val;

	return val;
}


/*!
*************************************************************************************
* \brief
*    ue_v, reads an u(1) syntax element, the length in bits is stored in
*    the global p_Dec->UsedBits variable
*
* \param tracestring
*    the string for the trace file
*
* \param bitstream
*    the stream to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
Boolean u_1 (const char *tracestring, Bitstream *bitstream)
{
	return (Boolean) u_v (1, tracestring, bitstream);
}



/*!
************************************************************************
* \brief
*    mapping rule for ue(v) syntax elements
* \par Input:
*    lenght and info
* \par Output:
*    number in the code table
************************************************************************
*/
void linfo_ue(int len, int info, int *value1, int *dummy)
{
	//assert ((len >> 1) < 32);
	*value1 = (int) (((unsigned int) 1 << (len >> 1)) + (unsigned int) (info) - 1);
}

/*!
************************************************************************
* \brief
*    mapping rule for se(v) syntax elements
* \par Input:
*    lenght and info
* \par Output:
*    signed mvd
************************************************************************
*/
void linfo_se(int len,  int info, int *value1, int *dummy)
{
	//assert ((len >> 1) < 32);
	unsigned int n = ((unsigned int) 1 << (len >> 1)) + (unsigned int) info - 1;
	*value1 = (n + 1) >> 1;
	if((n & 0x01) == 0)                           // lsb is signed bit
		*value1 = -*value1;
}


/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    cbp (intra)
************************************************************************
*/
void linfo_cbp_intra_normal(int len,int info,int *cbp, int *dummy)
{
	int cbp_idx;

	linfo_ue(len, info, &cbp_idx, dummy);
	*cbp=NCBP[1][cbp_idx][0];
}


/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    cbp (intra)
************************************************************************
*/
void linfo_cbp_intra_other(int len,int info,int *cbp, int *dummy)
{
	int cbp_idx;

	linfo_ue(len, info, &cbp_idx, dummy);
	*cbp=NCBP[0][cbp_idx][0];
}

/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    cbp (inter)
************************************************************************
*/
void linfo_cbp_inter_normal(int len,int info,int *cbp, int *dummy)
{
	int cbp_idx;

	linfo_ue(len, info, &cbp_idx, dummy);
	*cbp=NCBP[1][cbp_idx][1];
}

/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    cbp (inter)
************************************************************************
*/
void linfo_cbp_inter_other(int len,int info,int *cbp, int *dummy)
{
	int cbp_idx;

	linfo_ue(len, info, &cbp_idx, dummy);
	*cbp=NCBP[0][cbp_idx][1];
}

/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    level, run
************************************************************************
*/
void linfo_levrun_inter(int len, int info, int *level, int *irun)
{
	//assert (((len >> 1) - 5) < 32);

	if (len <= 9)
	{
		int l2     = imax(0,(len >> 1)-1);
		int inf    = info >> 1;

		*level = NTAB1[l2][inf][0];
		*irun  = NTAB1[l2][inf][1];
		if ((info & 0x01) == 1)
			*level = -*level;                   // make sign
	}
	else                                  // if len > 9, skip using the array
	{
		*irun  = (info & 0x1e) >> 1;
		*level = LEVRUN1[*irun] + (info >> 5) + ( 1 << ((len >> 1) - 5));
		if ((info & 0x01) == 1)
			*level = -*level;
	}

	if (len == 1) // EOB
		*level = 0;
}


/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    level, run
************************************************************************
*/
void linfo_levrun_c2x2(int len, int info, int *level, int *irun)
{
	if (len<=5)
	{
		int l2     = imax(0, (len >> 1) - 1);
		int inf    = info >> 1;
		*level = NTAB3[l2][inf][0];
		*irun  = NTAB3[l2][inf][1];
		if ((info & 0x01) == 1)
			*level = -*level;                 // make sign
	}
	else                                  // if len > 5, skip using the array
	{
		*irun  = (info & 0x06) >> 1;
		*level = LEVRUN3[*irun] + (info >> 3) + (1 << ((len >> 1) - 3));
		if ((info & 0x01) == 1)
			*level = -*level;
	}

	if (len == 1) // EOB
		*level = 0;
}

/*!
************************************************************************
* \brief
*    read next UVLC codeword from UVLC-partition and
*    map it to the corresponding syntax element
************************************************************************
*/
int readSyntaxElement_VLC(SyntaxElement *sym, Bitstream *currStream)
{

	int info;
	sym->len =  GetVLCSymbol (currStream->streamBuffer, currStream->frame_bitoffset, &info, currStream->bitstream_length);
	if (sym->len == -1)
		return -1;

	currStream->frame_bitoffset += sym->len;
	sym->mapping(sym->len, info, &(sym->value1), &(sym->value2));

	return 1;
}


/*!
************************************************************************
* \brief
*    read next UVLC codeword from UVLC-partition and
*    map it to the corresponding syntax element
************************************************************************
*/
int readSyntaxElement_UVLC(SyntaxElement *sym, struct datapartition *dp)
{
	return (readSyntaxElement_VLC(sym, dp->bitstream));
}

/*!
************************************************************************
* \brief
*    read next VLC codeword for 4x4 Intra Prediction Mode and
*    map it to the corresponding Intra Prediction Direction
************************************************************************
*/
int readSyntaxElement_Intra4x4PredictionMode(SyntaxElement *sym, Bitstream *currStream)
{
	int info;
	sym->len = GetVLCSymbol_IntraMode (currStream->streamBuffer, currStream->frame_bitoffset, &info, currStream->bitstream_length);

	if (sym->len == -1)
		return -1;

	currStream->frame_bitoffset += sym->len;
	sym->value1       = (sym->len == 1) ? -1 : info;

#if TRACE
	tracebits2(sym->tracestring, sym->len, sym->value1);
#endif

	return 1;
}

int GetVLCSymbol_IntraMode (const uint8_t buffer[],int totbitoffset,int *info, int bytecount)
{
	int byteoffset = (totbitoffset >> 3);        // byte from start of buffer
	int bitoffset   = (7 - (totbitoffset & 0x07)); // bit from start of byte
	const uint8_t *cur_byte  = &(buffer[byteoffset]);
	int ctr_bit     = (*cur_byte & (0x01 << bitoffset));      // control bit for current bit posision

	//First bit
	if (ctr_bit)
	{
		*info = 0;
		return 1;
	}

	if (byteoffset >= bytecount) 
	{
		return -1;
	}
	else
	{
		int inf = (*(cur_byte) << 8) + *(cur_byte + 1);
		inf <<= (sizeof(uint8_t) * 8) - bitoffset;
		inf = inf & 0xFFFF;
		inf >>= (sizeof(uint8_t) * 8) * 2 - 3;

		*info = inf;
		return 4;           // return absolute offset in bit from start of frame
	} 
}


/*!
************************************************************************
* \brief
*    test if bit buffer contains only stop bit
*
* \param buffer
*    buffer containing VLC-coded data bits
* \param totbitoffset
*    bit offset from start of partition
* \param bytecount
*    buffer length
* \return
*    true if more bits available
************************************************************************
*/
int more_rbsp_data (const uint8_t buffer[],int totbitoffset,int bytecount)
{
	long byteoffset = (totbitoffset >> 3);      // byte from start of buffer
	// there is more until we're in the last byte
	if (byteoffset < (bytecount - 1)) 
		return TRUE;
	else
	{
		int bitoffset   = (7 - (totbitoffset & 0x07));      // bit from start of byte
		const uint8_t *cur_byte  = &(buffer[byteoffset]);
		// read one bit
		int ctr_bit     = ctr_bit = ((*cur_byte)>> (bitoffset--)) & 0x01;      // control bit for current bit posision

		//assert (byteoffset<bytecount);       

		// a stop bit has to be one
		if (ctr_bit==0) 
			return TRUE;  
		else
		{
			int cnt = 0;

			while (bitoffset>=0 && !cnt)
			{
				cnt |= ((*cur_byte)>> (bitoffset--)) & 0x01;   // set up control bit
			}

			return (cnt);
		}
	}
}


/*!
************************************************************************
* \brief
*    Check if there are symbols for the next MB
************************************************************************
*/
int uvlc_startcode_follows(Slice *currSlice, int dummy)
{
	byte            dp_Nr = assignSE2partition[currSlice->dp_mode][SE_MBTYPE];
	DataPartition     *dP = &(currSlice->partArr[dp_Nr]);
	Bitstream *currStream = dP->bitstream;
	const uint8_t *buf = currStream->streamBuffer;

	return (!(more_rbsp_data(buf, currStream->frame_bitoffset,currStream->bitstream_length)));
}



/*!
************************************************************************
* \brief
*  read one exp-golomb VLC symbol
*
* \param buffer
*    containing VLC-coded data bits
* \param totbitoffset
*    bit offset from start of partition
* \param  info
*    returns the value of the symbol
* \param bytecount
*    buffer length
* \return
*    bits read
************************************************************************
*/
int GetVLCSymbol (const uint8_t buffer[],int totbitoffset,int *info, int bytecount)
{
	long byteoffset = (totbitoffset >> 3);         // byte from start of buffer
	int  bitoffset  = (7 - (totbitoffset & 0x07)); // bit from start of byte
	int  bitcounter = 1;
	int  len        = 0;
	const uint8_t *cur_byte  = &(buffer[byteoffset]);
	int  ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;  // control bit for current bit posision

	while (ctr_bit == 0)
	{                 // find leading 1 bit
		len++;
		bitcounter++;
		bitoffset--;
		bitoffset &= 0x07;
		cur_byte  += (bitoffset == 7);
		byteoffset+= (bitoffset == 7);      
		ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;
	}

	if (byteoffset + ((len + 7) >> 3) > bytecount)
		return -1;
	else
	{
		// make infoword
		int inf = 0;                          // shortest possible code is 1, then info is always 0    

		while (len--)
		{
			bitoffset --;    
			bitoffset &= 0x07;
			cur_byte  += (bitoffset == 7);
			bitcounter++;
			inf <<= 1;    
			inf |= ((*cur_byte) >> (bitoffset)) & 0x01;
		}

		*info = inf;
		return bitcounter;           // return absolute offset in bit from start of frame
	}
}


/*!
************************************************************************
* \brief
*  Reads bits from the bitstream buffer (Threshold based)
*
* \param inf
*    bytes to extract numbits from with bitoffset already applied
* \param numbits
*    number of bits to read
*
************************************************************************
*/

static inline int ShowBitsThres16(int inf, int numbits)
{
	return ((inf) >> ((sizeof(uint8_t) * 16) - (numbits)));
}

//static inline int ShowBitsThres (int inf, int bitcount, int numbits)
static inline int ShowBitsThres(int inf, int numbits)
{
	return ((inf) >> ((sizeof(uint8_t) * 24) - (numbits)));
	/*
	if ((numbits + 7) > bitcount) 
	{
	return -1;
	}
	else 
	{
	//Worst case scenario is that we will need to traverse 3 bytes
	inf >>= (sizeof(byte)*8)*3 - numbits;
	}

	return inf; //Will be a small unsigned integer so will not need any conversion when returning as int
	*/
}


/*!
************************************************************************
* \brief
*    code from bitstream (2d tables)
************************************************************************
*/

static int code_from_bitstream_2d(SyntaxElement *sym,
																	Bitstream *currStream,
																	const uint8_t *lentab,
																	const uint8_t *codtab,
																	int tabwidth,
																	int tabheight,
																	int *code)
{
	int i, j;
	const uint8_t *len = &lentab[0], *cod = &codtab[0];

	int *frame_bitoffset = &currStream->frame_bitoffset;
	const uint8_t *buf = &currStream->streamBuffer[*frame_bitoffset >> 3];

	//Apply bitoffset to three bytes (maximum that may be traversed by ShowBitsThres)
	unsigned int inf = ((*buf) << 16) + (*(buf + 1) << 8) + *(buf + 2); //Even at the end of a stream we will still be pulling out of allocated memory as alloc is done by MAX_CODED_FRAME_SIZE
	inf <<= (*frame_bitoffset & 0x07);                                  //Offset is constant so apply before extracting different numbers of bits
	inf  &= 0xFFFFFF;                                                   //Arithmetic shift so wipe any sign which may be extended inside ShowBitsThres

	// this VLC decoding method is not optimized for speed
	for (j = 0; j < tabheight; j++) 
	{
		for (i = 0; i < tabwidth; i++)
		{
			if ((*len == 0) || (ShowBitsThres(inf, *len) != *cod))
			{
				len++;
				cod++;
			}
			else
			{
				sym->len = *len;
				*frame_bitoffset += *len; // move bitstream pointer
				*code = *cod;             
				sym->value1 = i;
				sym->value2 = j;        
				return 0;                 // found code and return 
			}
		}
	}
	return -1;  // failed to find code
}

static int code_from_bitstream_2d_16_1(Bitstream *currStream,
																			 const uint8_t *lentab,
																			 const uint8_t *codtab)
{
	int i;
	const uint8_t *len = &lentab[0], *cod = &codtab[0];

	int *frame_bitoffset = &currStream->frame_bitoffset;
	const uint8_t *buf = &currStream->streamBuffer[*frame_bitoffset >> 3];

	//Apply bitoffset to three bytes (maximum that may be traversed by ShowBitsThres)
	unsigned int inf = ((*buf) << 16) + (*(buf + 1) << 8) + *(buf + 2); //Even at the end of a stream we will still be pulling out of allocated memory as alloc is done by MAX_CODED_FRAME_SIZE
	inf <<= (*frame_bitoffset & 0x07);                                  //Offset is constant so apply before extracting different numbers of bits
	inf  &= 0xFFFFFF;                                                   //Arithmetic shift so wipe any sign which may be extended inside ShowBitsThres

	// this VLC decoding method is not optimized for speed
	for (i = 0; i < 16 && len[i]; i++)
	{
		if (ShowBitsThres(inf, len[i]) == cod[i])
		{
			*frame_bitoffset += len[i]; // move bitstream pointer
			return i;                 // found code and return 
		}
	}

	return -1;  // failed to find code
}

int code_from_bitstream_2d_16_1_sse2(Bitstream *currStream, const uint16_t *lentab,	const uint16_t *codtab, const uint16_t *masktab)
{
	unsigned long result;

	int frame_bitoffset = currStream->frame_bitoffset;
	const uint8_t *buf = &currStream->streamBuffer[frame_bitoffset >> 3];
	uint16_t inf;

	__m128i xmm_inf, xmm_mask, xmm_cod;
	int match;
	unsigned int _inf = _byteswap_ulong(*(unsigned long *)buf);
	_inf >>= 16-(frame_bitoffset & 0x07);
	_inf  &= 0xFFFF;
	inf = (uint16_t)_inf;

	xmm_inf = _mm_set1_epi16(inf);

	xmm_cod  = _mm_load_si128((__m128i *)codtab);	
	xmm_mask = _mm_load_si128((__m128i *)masktab);	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += lentab[result]; // move bitstream pointer
		return result;                 // found code and return 
	}

	xmm_cod  = _mm_load_si128((__m128i *)(codtab+8));	
	xmm_mask = _mm_load_si128((__m128i *)(masktab+8));	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += lentab[result+8]; // move bitstream pointer
		return result+8;
	}


	return -1;
}

int code_from_bitstream_2d_16_1_c(Bitstream *currStream, const uint16_t *lentab,	const uint16_t *codtab, const uint16_t *masktab)
{
	int i;

	int frame_bitoffset = currStream->frame_bitoffset;
	const uint8_t *buf = &currStream->streamBuffer[frame_bitoffset >> 3];
	uint16_t inf;

	unsigned int _inf = _byteswap_ulong(*(unsigned long *)buf);
	_inf >>= 16-(frame_bitoffset & 0x07);
	_inf  &= 0xFFFF;
	inf = (uint16_t)_inf;

	// this VLC decoding method is not optimized for speed
	for (i=0; i < 16; i++)
	{
		if ((inf & masktab[i]) == codtab[i])//ShowBitsThres(inf, len[i]) == cod[i])
		{
			currStream->frame_bitoffset += lentab[i]; // move bitstream pointer
			return i;                 // found code and return 
		}
	}

	return -1;  // failed to find code
}

int code_from_bitstream_2d_17_4_sse2(SyntaxElement *sym, Bitstream *currStream, const uint16_t *lentab, const uint16_t *codtab, const uint16_t *masktab)
{
	unsigned long result;
	const uint16_t *len = lentab, *cod = codtab, *mask = masktab;

	int frame_bitoffset = currStream->frame_bitoffset;
	const uint8_t *buf = &currStream->streamBuffer[frame_bitoffset >> 3];
	uint16_t inf;

	__m128i xmm_inf, xmm_mask, xmm_cod;
	int match;
	unsigned int _inf = _byteswap_ulong(*(unsigned long *)buf);
	_inf >>= 16-(frame_bitoffset & 0x07);
	_inf  &= 0xFFFF;
	inf = (uint16_t)_inf;

	xmm_inf = _mm_set1_epi16(inf);

	xmm_cod  = _mm_loadu_si128((__m128i *)cod);	
	xmm_mask = _mm_loadu_si128((__m128i *)mask);	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += len[result]; // move bitstream pointer
		sym->value1 = result;
		sym->value2 = 0;        
		return 0;                 // found code and return 
	}

	/* second table - rows 1-8 */
	xmm_cod  = _mm_loadu_si128((__m128i *)(cod+17));	
	xmm_mask = _mm_loadu_si128((__m128i *)(mask+17));	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += len[result+17]; // move bitstream pointer
		sym->value1 = 1+result;
		sym->value2 = 1;        
		return 0;                 // found code and return 
	}

	/*  first table, rows 9-16 */
	xmm_cod  = _mm_loadu_si128((__m128i *)(cod+8));	
	xmm_mask = _mm_loadu_si128((__m128i *)(mask+8));	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += len[result+8]; // move bitstream pointer
		sym->value1 = 8+result;
		sym->value2 = 0;        
		return 0;                 // found code and return 
	}

	/* extra one just for first table */
	if ((inf & mask[16]) == cod[16])//ShowBitsThres(inf, len[i]) == cod[i])
	{
		currStream->frame_bitoffset += len[16]; // move bitstream pointer
		sym->value1 = 16;
		sym->value2 = 0;        
		return 0;                 // found code and return 
	}



	/* second table - rows 9-16 */
	xmm_cod  = _mm_loadu_si128((__m128i *)(cod+25));	
	xmm_mask = _mm_loadu_si128((__m128i *)(mask+25));	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += len[result+25]; // move bitstream pointer
		sym->value1 = 9+result;
		sym->value2 = 1;        
		return 0;                 // found code and return 
	}


	/* third table - rows 1-8 */
	xmm_cod  = _mm_loadu_si128((__m128i *)(cod+34));	
	xmm_mask = _mm_loadu_si128((__m128i *)(mask+34));	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += len[result+34]; // move bitstream pointer
		sym->value1 = 2+result;
		sym->value2 = 2;        
		return 0;                 // found code and return 
	}


	/* third table - rows 9-16 */
	xmm_cod  = _mm_loadu_si128((__m128i *)(cod+42));	
	xmm_mask = _mm_loadu_si128((__m128i *)(mask+42));	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += len[result+42]; // move bitstream pointer
		sym->value1 = 10+result;
		sym->value2 = 2;        
		return 0;                 // found code and return 
	}

	/* fourth table - rows 1-8 */
	xmm_cod  = _mm_loadu_si128((__m128i *)(cod+51));	
	xmm_mask = _mm_loadu_si128((__m128i *)(mask+51));	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += len[result+51]; // move bitstream pointer
		sym->value1 = 3+result;
		sym->value2 = 3;        
		return 0;                 // found code and return 
	}

	/* fourth table - rows 9-16 */
	xmm_cod  = _mm_loadu_si128((__m128i *)(cod+59));	
	xmm_mask = _mm_loadu_si128((__m128i *)(mask+59));	
	xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
	xmm_mask = _mm_cmpeq_epi16(xmm_mask, xmm_cod); // mask == cod
	match = _mm_movemask_epi8(xmm_mask);
	if (match)
	{
		_BitScanForward(&result, match);
		result >>= 1;

		currStream->frame_bitoffset += len[result+59]; // move bitstream pointer
		sym->value1 = 11+result;
		sym->value2 = 3;        
		return 0;                 // found code and return 
	}

	return -1;  // failed to find code
}


int code_from_bitstream_2d_17_4_c(SyntaxElement *sym, Bitstream *currStream, const uint16_t *lentab, const uint16_t *codtab, const uint16_t *masktab)
{
	int i, j;
	const uint16_t *len, *cod, *mask;

	int frame_bitoffset = currStream->frame_bitoffset;
	const uint8_t *buf = &currStream->streamBuffer[frame_bitoffset >> 3];
	uint16_t inf;

	unsigned int _inf = _byteswap_ulong(*(unsigned long *)buf);
	_inf >>= 16-(frame_bitoffset & 0x07);
	_inf  &= 0xFFFF;
	inf = (uint16_t)_inf;

	for (j=0;j<4;j++)
	{
		len = &lentab[j*17];
		cod = &codtab[j*17];
		mask = &masktab[j*17];
		// this VLC decoding method is not optimized for speed
		for (i=0; i < 17; i++)
		{
			if ((inf & mask[i]) == cod[i])//ShowBitsThres(inf, len[i]) == cod[i])
			{
				currStream->frame_bitoffset += len[i]; // move bitstream pointer
				sym->value1 = j+i;
				sym->value2 = j;        
				return 0;                 // found code and return 
			}
		}
	}

	return -1;  // failed to find code
}

static int code_from_bitstream_2d_9_4(SyntaxElement *sym,
																			Bitstream *currStream,
																			 const uint16_t *lentab,
																			 const uint16_t *codtab,
																			 const uint16_t *masktab)
{
	int i, j;
	const uint16_t *len, *cod, *mask;

	int frame_bitoffset = currStream->frame_bitoffset;
	const uint8_t *buf = &currStream->streamBuffer[frame_bitoffset >> 3];

	uint16_t inf;
		unsigned int _inf = _byteswap_ulong(*(unsigned long *)buf);
		_inf >>= 16-(frame_bitoffset & 0x07);
		_inf  &= 0xFFFF;
		inf = (uint16_t)_inf;

	// this VLC decoding method is not optimized for speed
	for (j = 0; j < 4; j++) 
	{
		len = &lentab[j*9];
		cod = &codtab[j*9];
		mask = &masktab[j*9];

		for (i=0; i < 9; i++)
		{
			if ((inf & mask[i]) == cod[i])
			{
				sym->len = len[i];
				currStream->frame_bitoffset += len[i]; // move bitstream pointer
				sym->value1 = j+i;
				sym->value2 = j;        
				return 0;                 // found code and return 
			}
		}
	}
	return -1;  // failed to find code
}

int code_from_bitstream_2d_5_4_c(SyntaxElement *sym, Bitstream *currStream, const uint8_t *lentab, const uint8_t *codtab, const uint8_t *masktab)
{

		int i;
		int frame_bitoffset = currStream->frame_bitoffset;
		const uint8_t *buf = &currStream->streamBuffer[frame_bitoffset >> 3];
		uint8_t inf;

		unsigned int _inf = _byteswap_ushort(*(unsigned short *)buf);
		_inf >>= 8-(frame_bitoffset & 0x07);
		_inf  &= 0xFF;
		inf = (uint8_t)_inf;

		for (i = 0; i<16;i++)
		{
			if ((inf & masktab[i]) == codtab[i])
			{
				currStream->frame_bitoffset += lentab[i]; // move bitstream pointer
				sym->value2 = (i<<1)/9;        
				sym->value1 = sym->value2 + (((i<<1)%9)>>1);

				return 0;                 // found code and return 
			}
		}

		return -1;  // failed to find code

}


int code_from_bitstream_2d_5_4_sse2(SyntaxElement *sym, Bitstream *currStream, const uint8_t *lentab, const uint8_t *codtab, const uint8_t *masktab)
{
		int frame_bitoffset = currStream->frame_bitoffset;
		const uint8_t *buf = &currStream->streamBuffer[frame_bitoffset >> 3];
		uint8_t inf;
		__m128i xmm_inf, xmm_mask, xmm_cod;
		int match;
		unsigned int _inf = _byteswap_ushort(*(unsigned short *)buf);
		_inf >>= 8-(frame_bitoffset & 0x07);
		_inf  &= 0xFF;
		inf = (uint8_t)_inf;

		xmm_inf = _mm_set1_epi8(_inf);

		xmm_cod  = _mm_load_si128((__m128i *)codtab);	
		xmm_mask = _mm_load_si128((__m128i *)masktab);	
		xmm_mask = _mm_and_si128(xmm_mask, xmm_inf); // mask = mask & inf
		xmm_mask = _mm_cmpeq_epi8(xmm_mask, xmm_cod); // mask == cod
		match = _mm_movemask_epi8(xmm_mask);
		if (match)
		{
			unsigned long result;
			_BitScanForward(&result, match);

			currStream->frame_bitoffset += lentab[result]; // move bitstream pointer
			sym->value2 = (result<<1)/9;        
			sym->value1 = sym->value2 + (((result<<1)%9)>>1);     
			return 0;                 // found code and return 
		}
		return -1;
}

/*!
************************************************************************
* \brief
*    read FLC codeword from UVLC-partition
************************************************************************
*/
int readSyntaxElement_FLC(Bitstream *currStream, int numbits)
{
	int totbitoffset = currStream->frame_bitoffset;
	int bitoffset  = /*7 - */(totbitoffset & 0x07); // bit from start of byte
	int byteoffset = (totbitoffset >> 3); // byte from start of buffer
	const uint8_t *ptr  = &(currStream->streamBuffer[byteoffset]);

	uint32_t tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8);// | (ptr[3]);
	tmp <<= bitoffset;
	tmp >>= 32 - numbits;
	currStream->frame_bitoffset += numbits;
	return tmp;
}



/*!
************************************************************************
* \brief
*    read NumCoeff/TrailingOnes codeword from UVLC-partition
************************************************************************
*/

int readSyntaxElement_NumCoeffTrailingOnes(SyntaxElement *sym,  
																					 Bitstream *currStream,
																					 int vlcnum)
{
	int frame_bitoffset        = currStream->frame_bitoffset;
	int BitstreamLengthInBytes = currStream->bitstream_length;
	int BitstreamLengthInBits  = (BitstreamLengthInBytes << 3) + 7;
	const uint8_t *buf = currStream->streamBuffer;

	static const uint16_t lentab[3][4][17] =
	{
		{   // 0702
			{ 1, 6, 8, 9,10,11,13,13,13,14,14,15,15,16,16,16,16},
			{ 2, 6, 8, 9,10,11,13,13,14,14,15,15,15,16,16,16, 0},
			{ 3, 7, 8, 9,10,11,13,13,14,14,15,15,16,16,16, 0, 0},
			{ 5, 6, 7, 8, 9,10,11,13,14,14,15,15,16,16, 0, 0, 0},
		},
		{
			{ 2, 6, 6, 7, 8, 8, 9,11,11,12,12,12,13,13,13,14,14},
			{ 2, 5, 6, 6, 7, 8, 9,11,11,12,12,13,13,14,14,14, 0},
			{ 3, 6, 6, 7, 8, 9,11,11,12,12,13,13,13,14,14, 0, 0},
			{ 4, 4, 5, 6, 6, 7, 9,11,11,12,13,13,13,14, 0, 0, 0},
			},
			{
				{ 4, 6, 6, 6, 7, 7, 7, 7, 8, 8, 9, 9, 9,10,10,10,10},
				{ 4, 5, 5, 5, 5, 6, 6, 7, 8, 8, 9, 9, 9,10,10,10, 0},
				{ 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,10, 0, 0},
				{ 4, 4, 4, 4, 4, 5, 6, 7, 8, 8, 9,10,10,10, 0, 0, 0},
			},
	};
#if 0 // save for reference
	static const uint32_t codtab[3][4][17] =
	{
		{
			{ 1, 5, 7, 7, 7, 7,15,11, 8,15,11,15,11,15,11, 7,4},
			{ 0, 1, 4, 6, 6, 6, 6,14,10,14,10,14,10, 1,14,10,6},
			{ 0, 0, 1, 5, 5, 5, 5, 5,13, 9,13, 9,13, 9,13, 9,5},
			{ 0, 0, 0, 3, 3, 4, 4, 4, 4, 4,12,12, 8,12, 8,12,8},
		},
		{
			{ 3,11, 7, 7, 7, 4, 7,15,11,15,11, 8,15,11, 7, 9,7},
			{ 0, 2, 7,10, 6, 6, 6, 6,14,10,14,10,14,10,11, 8,6},
			{ 0, 0, 3, 9, 5, 5, 5, 5,13, 9,13, 9,13, 9, 6,10,5},
			{ 0, 0, 0, 5, 4, 6, 8, 4, 4, 4,12, 8,12,12, 8, 1,4},
			},
			{
				{15,15,11, 8,15,11, 9, 8,15,11,15,11, 8,13, 9, 5,1},
				{ 0,14,15,12,10, 8,14,10,14,14,10,14,10, 7,12, 8,4},
				{ 0, 0,13,14,11, 9,13, 9,13,10,13, 9,13, 9,11, 7,3},
				{ 0, 0, 0,12,11,10, 9, 8,13,12,12,12, 8,12,10, 6,2},
			},
	};
#endif
	static const uint16_t codtab[3][4][17] =
	{
		{
			{ 0x8000, 0x1400, 0x0700, 0x0380, 0x01C0, 0x00E0, 0x0078, 0x0058, 0x0040, 0x003C, 0x002C, 0x001E, 0x0016, 0x000F, 0x000B, 0x0007, 0x0004 },
			{ 0x4000, 0x1000, 0x0600, 0x0300, 0x0180, 0x00C0, 0x0070, 0x0050, 0x0038, 0x0028, 0x001C, 0x0014, 0x0002, 0x000E, 0x000A, 0x0006, 0xFFFF },
			{ 0x2000, 0x0A00, 0x0500, 0x0280, 0x0140, 0x00A0, 0x0068, 0x0048, 0x0034, 0x0024, 0x001A, 0x0012, 0x000D, 0x0009, 0x0005, 0xFFFF, 0xFFFF },
			{ 0x1800, 0x0C00, 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0060, 0x0030, 0x0020, 0x0018, 0x0010, 0x000C, 0x0008, 0xFFFF, 0xFFFF, 0xFFFF }
		},
		{
			{ 0xC000, 0x2C00, 0x1C00, 0x0E00, 0x0700, 0x0400, 0x0380, 0x01E0, 0x0160, 0x00F0, 0x00B0, 0x0080, 0x0078, 0x0058, 0x0038, 0x0024, 0x001C },
			{ 0x8000, 0x3800, 0x2800, 0x1800, 0x0C00, 0x0600, 0x0300, 0x01C0, 0x0140, 0x00E0, 0x00A0, 0x0070, 0x0050, 0x002C, 0x0020, 0x0018, 0xFFFF },
			{ 0x6000, 0x2400, 0x1400, 0x0A00, 0x0500, 0x0280, 0x01A0, 0x0120, 0x00D0, 0x0090, 0x0068, 0x0048, 0x0030, 0x0028, 0x0014, 0xFFFF, 0xFFFF },
			{ 0x5000, 0x4000, 0x3000, 0x2000, 0x1000, 0x0800, 0x0200, 0x0180, 0x0100, 0x00C0, 0x0060, 0x0040, 0x0008, 0x0010, 0xFFFF, 0xFFFF, 0xFFFF }
		},
		{
			{ 0xF000, 0x3C00, 0x2C00, 0x2000, 0x1E00, 0x1600, 0x1200, 0x1000, 0x0F00, 0x0B00, 0x0780, 0x0580, 0x0400, 0x0340, 0x0240, 0x0140, 0x0040 },
			{ 0xE000, 0x7800, 0x6000, 0x5000, 0x4000, 0x3800, 0x2800, 0x1C00, 0x0E00, 0x0A00, 0x0700, 0x0500, 0x0380, 0x0300, 0x0200, 0x0100, 0xFFFF },
			{ 0xD000, 0x7000, 0x5800, 0x4800, 0x3400, 0x2400, 0x1A00, 0x1400, 0x0D00, 0x0900, 0x0680, 0x0480, 0x02C0, 0x01C0, 0x00C0, 0xFFFF, 0xFFFF },
			{ 0xC000, 0xB000, 0xA000, 0x9000, 0x8000, 0x6800, 0x3000, 0x1800, 0x0C00, 0x0800, 0x0600, 0x0280, 0x0180, 0x0080, 0xFFFF, 0xFFFF, 0xFFFF }
		}
	};

	static const uint16_t masktab[3][4][17] =
	{
		{
			{ 0x8000, 0xFC00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF8, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF },
			{ 0xC000, 0xFC00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000 },
			{ 0xE000, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000 },
			{ 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000 }
		},
		{
			{ 0xC000, 0xFC00, 0xFC00, 0xFE00, 0xFF00, 0xFF00, 0xFF80, 0xFFE0, 0xFFE0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF8, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC },
			{ 0xC000, 0xF800, 0xFC00, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFE0, 0xFFE0, 0xFFF0, 0xFFF0, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFC, 0x0000 },
			{ 0xE000, 0xFC00, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFE0, 0xFFE0, 0xFFF0, 0xFFF0, 0xFFF8, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0x0000, 0x0000 },
			{ 0xF000, 0xF000, 0xF800, 0xFC00, 0xFC00, 0xFE00, 0xFF80, 0xFFE0, 0xFFE0, 0xFFF0, 0xFFF8, 0xFFF8, 0xFFF8, 0xFFFC, 0x0000, 0x0000, 0x0000 }
		},
		{
			{ 0xF000, 0xFC00, 0xFC00, 0xFC00, 0xFE00, 0xFE00, 0xFE00, 0xFE00, 0xFF00, 0xFF00, 0xFF80, 0xFF80, 0xFF80, 0xFFC0, 0xFFC0, 0xFFC0, 0xFFC0 },
			{ 0xF000, 0xF800, 0xF800, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFE00, 0xFF00, 0xFF00, 0xFF80, 0xFF80, 0xFF80, 0xFFC0, 0xFFC0, 0xFFC0, 0x0000 },
			{ 0xF000, 0xF800, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFE00, 0xFE00, 0xFF00, 0xFF00, 0xFF80, 0xFF80, 0xFFC0, 0xFFC0, 0xFFC0, 0x0000, 0x0000 },
			{ 0xF000, 0xF000, 0xF000, 0xF000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF00, 0xFF80, 0xFFC0, 0xFFC0, 0xFFC0, 0x0000, 0x0000, 0x0000 }
		}
	};

	int code;
	// vlcnum is the index of Table used to code coeff_token
	// vlcnum==3 means (8<=nC) which uses 6bit FLC

	if (vlcnum == 3)
	{
		// read 6 bit FLC
		//code = ShowBits(buf, frame_bitoffset, BitstreamLengthInBytes, 6);
		code = ShowBits(buf, frame_bitoffset, BitstreamLengthInBits, 6);
		currStream->frame_bitoffset += 6;
		sym->value2 = (code & 3);
		sym->value1 = (code >> 2);

		if (!sym->value1 && sym->value2 == 3)
		{
			// #c = 0, #t1 = 3 =>  #c = 0
			sym->value2 = 0;
		}
		else
			sym->value1++;
	}
	else
	{
		//retval = code_from_bitstream_2d(sym, currStream, &lentab[vlcnum][0][0], &codtab[vlcnum][0][0], 17, 4, &code);    
		code = opt_code_from_bitstream_2d_17_4(sym, currStream, lentab[vlcnum][0], codtab[vlcnum][0], masktab[vlcnum][0]);
	}

	return 0;
}


/*!
************************************************************************
* \brief
*    read NumCoeff/TrailingOnes codeword from UVLC-partition ChromaDC
************************************************************************
*/
int readSyntaxElement_NumCoeffTrailingOnesChromaDC(VideoParameters *p_Vid, SyntaxElement *sym,  Bitstream *currStream)
{
#if 0
	static const uint8_t lentab[3][4][17] =
	{
		//YUV420
		{{ 2, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 1, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 3, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
		//YUV422
		{{ 1, 7, 7, 9, 9,10,11,12,13, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 2, 7, 7, 9,10,11,12,12, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 3, 7, 7, 9,10,11,12, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 5, 6, 7, 7,10,11, 0, 0, 0, 0, 0, 0, 0, 0}},
		//YUV444
		{{ 1, 6, 8, 9,10,11,13,13,13,14,14,15,15,16,16,16,16},
		{ 0, 2, 6, 8, 9,10,11,13,13,14,14,15,15,15,16,16,16},
		{ 0, 0, 3, 7, 8, 9,10,11,13,13,14,14,15,15,16,16,16},
		{ 0, 0, 0, 5, 6, 7, 8, 9,10,11,13,14,14,15,15,16,16}}
	};
#endif

	//YUV420

	__declspec(align(32)) static const uint8_t lentab420[16] =    
	{ 2, 6, 6, 6, 6, 1, 6, 7, 8, 3, 7, 8, 0, 0, 6, 7 };
	__declspec(align(32)) static const uint8_t codtab420[16] =	
	{ 0x40, 0x1C, 0x10, 0x0C, 0x08,	0x80, 0x18, 0x06, 0x03, 0x20, 0x04, 0x02, 0xFF, 0xFF,	0x14, 0x00 };
	__declspec(align(32)) static const uint8_t masktab420[16] =
	{ 0xC0, 0xFC, 0xFC, 0xFC, 0xFC,	0x80, 0xFC, 0xFE, 0xFF, 0xE0, 0xFE, 0xFF, 0x00, 0x00,	0xFC, 0xFE };


	// YUV422
	__declspec(align(32)) static const uint16_t lentab422[4][9] = 
	{
		{ 1, 7, 7, 9, 9,10,11,12,13 },
		{ 2, 7, 7, 9,10,11,12,12, 0 },
		{ 3, 7, 7, 9,10,11,12, 0, 0 },
		{ 5, 6, 7, 7,10,11, 0, 0, 0 }
	};
	__declspec(align(32)) static const uint16_t codtab422[4][9] = 
	{
		{ 0x8000, 0x1E00, 0x1C00, 0x0380, 0x0300, 0x01C0, 0x00E0, 0x0070, 0x0038 },
		{ 0x4000, 0x1A00, 0x1800, 0x0280, 0x0180, 0x00C0, 0x0060, 0x0050, 0xFFFF },
		{ 0x2000, 0x1600, 0x1400, 0x0200, 0x0140, 0x00A0, 0x0040, 0xFFFF, 0xFFFF },
		{ 0x0800, 0x0400, 0x1200, 0x1000, 0x0100, 0x0080, 0xFFFF, 0xFFFF, 0xFFFF }
	};
	__declspec(align(32)) static const uint16_t masktab422[4][9] = 
	{
		{ 0x8000, 0xFE00, 0xFE00, 0xFF80, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF0, 0xFFF8 },
		{ 0xC000, 0xFE00, 0xFE00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF0, 0xFFF0, 0x0000 },
		{ 0xE000, 0xFE00, 0xFE00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF0, 0x0000, 0x0000 },
		{ 0xF800, 0xFC00, 0xFE00, 0xFE00, 0xFFC0, 0xFFE0, 0x0000, 0x0000, 0x0000 }
	};

	// YUV444
	__declspec(align(32)) static const uint16_t lentab444[4][17] = 
	{
		{ 1, 6, 8, 9,10,11,13,13,13,14,14,15,15,16,16,16,16},
		{ 2, 6, 8, 9,10,11,13,13,14,14,15,15,15,16,16,16, 0},
		{ 3, 7, 8, 9,10,11,13,13,14,14,15,15,16,16,16, 0, 0},
		{ 5, 6, 7, 8, 9,10,11,13,14,14,15,15,16,16, 0, 0, 0}
	};
	__declspec(align(32)) static const uint16_t codtab444[4][17] = 
	{
		{ 0x8000, 0x1400, 0x0700, 0x0380, 0x01C0, 0x00E0, 0x0078, 0x0058, 0x0040, 0x003C, 0x002C, 0x001E, 0x0016, 0x000F, 0x000B, 0x0007, 0x0004 },
		{ 0x4000, 0x1000, 0x0600, 0x0300, 0x0180, 0x00C0, 0x0070, 0x0050, 0x0038, 0x0028, 0x001C, 0x0014, 0x0002, 0x000E, 0x000A, 0x0006, 0xFFFF },
		{ 0x2000, 0x0A00, 0x0500, 0x0280, 0x0140, 0x00A0, 0x0068, 0x0048, 0x0034, 0x0024, 0x001A, 0x0012, 0x000D, 0x0009, 0x0005, 0xFFFF, 0xFFFF },
		{ 0x1800, 0x0C00, 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0060, 0x0030, 0x0020, 0x0018, 0x0010, 0x000C, 0x0008, 0xFFFF, 0xFFFF, 0xFFFF }
	};
	__declspec(align(32)) static const uint16_t masktab444[4][17] = 
	{
		{ 0x8000, 0xFC00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF8, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF },
		{ 0xC000, 0xFC00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000 },
		{ 0xE000, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000 },
		{ 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000 }
	};

#if 0
	static const uint8_t codtab[3][4][17] =
	{
		//YUV420
		{{ 1, 7, 4, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 1, 6, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
		//YUV422
		{{ 1,15,14, 7, 6, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 1,13,12, 5, 6, 6, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 1,11,10, 4, 5, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 1, 1, 9, 8, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0}},
		//YUV444
		{{ 1, 5, 7, 7, 7, 7,15,11, 8,15,11,15,11,15,11, 7, 4},
		{ 0, 1, 4, 6, 6, 6, 6,14,10,14,10,14,10, 1,14,10, 6},
		{ 0, 0, 1, 5, 5, 5, 5, 5,13, 9,13, 9,13, 9,13, 9, 5},
		{ 0, 0, 0, 3, 3, 4, 4, 4, 4, 4,12,12, 8,12, 8,12, 8}}
	};
#endif

	int code;
	int yuv = p_Vid->active_sps->chroma_format_idc - 1;
	switch(yuv)
	{
	case 0:
		code = opt_code_from_bitstream_2d_5_4(sym, currStream, lentab420, codtab420, masktab420);
		break;
	case 1:
		code = code_from_bitstream_2d_9_4(sym, currStream,  lentab422[0], codtab422[0], masktab422[0]);
		break;
	case 2:
		code = opt_code_from_bitstream_2d_17_4(sym, currStream,  lentab444[0], codtab444[0], masktab444[0]);
		break;
	default:
		__assume(0);
		return -1;
	}

	return 0;
}

/*!
************************************************************************
* \brief
*    read Level VLC0 codeword from UVLC-partition
************************************************************************
*/
int readSyntaxElement_Level_VLC0(Bitstream *currStream)
{
	int frame_bitoffset        = currStream->frame_bitoffset;
	int BitstreamLengthInBytes = currStream->bitstream_length;
	int BitstreamLengthInBits  = (BitstreamLengthInBytes << 3) + 7;
	byte *buf                  = currStream->streamBuffer;
	int len = 1, sign = 0, level = 0, code = 1;

	while (!ShowBits(buf, frame_bitoffset++, BitstreamLengthInBits, 1))
		len++;

	if (len < 15)
	{
		sign  = (len - 1) & 1;
		level = ((len - 1) >> 1) + 1;
	}
	else if (len == 15)
	{
		// escape code
		code <<= 4;
		code |= ShowBits(buf, frame_bitoffset, BitstreamLengthInBits, 4);
		len  += 4;
		frame_bitoffset += 4;
		sign = (code & 0x01);
		level = ((code >> 1) & 0x07) + 8;
	}
	else if (len >= 16)
	{
		// escape code
		int addbit = (len - 16);
		int offset = (2048 << addbit) - 2032;
		len   -= 4;
		code   = ShowBits(buf, frame_bitoffset, BitstreamLengthInBits, len);
		sign   = (code & 0x01);
		frame_bitoffset += len;    
		level = (code >> 1) + offset;

		code |= (1 << (len)); // for display purpose only
		len += addbit + 16;
	}
	currStream->frame_bitoffset = frame_bitoffset;
	return (sign) ? -level : level ;
	//sym->len = len;

#if TRACE
	tracebits2(sym->tracestring, sym->len, code);
#endif

	
	return 0;
}

/*!
************************************************************************
* \brief
*    read Level VLC codeword from UVLC-partition
************************************************************************
*/
int readSyntaxElement_Level_VLCN(int vlc, Bitstream *currStream)
{
	int frame_bitoffset        = currStream->frame_bitoffset;
	int BitstreamLengthInBytes = currStream->bitstream_length;
	int BitstreamLengthInBits  = (BitstreamLengthInBytes << 3) + 7;
	byte *buf                  = currStream->streamBuffer;

	int levabs, sign;
	int len = 1;
	int code = 1, sb;

	int shift = vlc - 1;

	// read pre zeros
	while (!ShowBits(buf, frame_bitoffset ++, BitstreamLengthInBits, 1))
		len++;

	frame_bitoffset -= len;

	if (len < 16)
	{
		levabs = ((len - 1) << shift) + 1;

		// read (vlc-1) bits -> suffix
		if (shift)
		{
			sb =  ShowBits(buf, frame_bitoffset + len, BitstreamLengthInBits, shift);
			code = (code << (shift) )| sb;
			levabs += sb;
			len += (shift);
		}

		// read 1 bit -> sign
		sign = ShowBits(buf, frame_bitoffset + len, BitstreamLengthInBits, 1);
		code = (code << 1)| sign;
		len ++;
	}
	else // escape
	{
		int addbit = len - 5;
		int offset = (1 << addbit) + (15 << shift) - 2047;

		sb = ShowBits(buf, frame_bitoffset + len, BitstreamLengthInBits, addbit);
		code = (code << addbit ) | sb;
		len   += addbit;

		levabs = sb + offset;

		// read 1 bit -> sign
		sign = ShowBits(buf, frame_bitoffset + len, BitstreamLengthInBits, 1);

		code = (code << 1)| sign;

		len++;
	}

	currStream->frame_bitoffset = frame_bitoffset + len;
	return (sign)? -levabs : levabs;
}

/*!
************************************************************************
* \brief
*    read Total Zeros codeword from UVLC-partition
************************************************************************
*/
int readSyntaxElement_TotalZeros(Bitstream *currStream, int vlcnum)
{
	__declspec(align(32)) static const uint16_t lentab[TOTRUN_NUM][16] =
	{

		{ 1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},
		{ 3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},
		{ 4,3,3,3,4,4,3,3,4,5,5,6,5,6},
		{ 5,3,4,4,3,3,3,4,3,4,5,5,5},
		{ 4,4,4,3,3,3,3,3,4,5,4,5},
		{ 6,5,3,3,3,3,3,3,4,3,6},
		{ 6,5,3,3,3,2,3,4,3,6},
		{ 6,4,5,3,2,2,3,3,6},
		{ 6,6,4,2,2,3,2,5},
		{ 5,5,3,2,2,2,4},
		{ 4,4,3,3,1,3},
		{ 4,4,2,1,3},
		{ 3,3,1,2},
		{ 2,2,1},
		{ 1,1},
	};
/*
	static const byte codtab[TOTRUN_NUM][16] =
	{
		{1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
		{7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
		{5,7,6,5,4,3,4,3,2,3,2,1,1,0},
		{3,7,5,4,6,5,4,3,3,2,2,1,0},
		{5,4,3,7,6,5,4,3,2,1,1,0},
		{1,1,7,6,5,4,3,2,1,1,0},
		{1,1,5,4,3,3,2,1,1,0},
		{1,1,1,3,3,2,2,1,0},
		{1,0,1,3,2,1,1,1,},
		{1,0,1,3,2,1,1,},
		{0,1,1,2,1,3},
		{0,1,1,1,1},
		{0,1,1,1},
		{0,1,1},
		{0,1},
	};*/

	__declspec(align(32)) static const uint16_t codtab[TOTRUN_NUM][16] =
		{
{ 0x8000, 0x6000, 0x4000, 0x3000, 0x2000, 0x1800, 0x1000, 0x0C00, 0x0800, 0x0600, 0x0400, 0x0300, 0x0200, 0x0180, 0x0100, 0x0080,  },
{ 0xE000, 0xC000, 0xA000, 0x8000, 0x6000, 0x5000, 0x4000, 0x3000, 0x2000, 0x1800, 0x1000, 0x0C00, 0x0800, 0x0400, 0x0000, 0xFFFF,  },
{ 0x5000, 0xE000, 0xC000, 0xA000, 0x4000, 0x3000, 0x8000, 0x6000, 0x2000, 0x1800, 0x1000, 0x0400, 0x0800, 0x0000, 0xFFFF, 0xFFFF,  },
{ 0x1800, 0xE000, 0x5000, 0x4000, 0xC000, 0xA000, 0x8000, 0x3000, 0x6000, 0x2000, 0x1000, 0x0800, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x5000, 0x4000, 0x3000, 0xE000, 0xC000, 0xA000, 0x8000, 0x6000, 0x2000, 0x0800, 0x1000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0400, 0x0800, 0xE000, 0xC000, 0xA000, 0x8000, 0x6000, 0x4000, 0x1000, 0x2000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0400, 0x0800, 0xA000, 0x8000, 0x6000, 0xC000, 0x4000, 0x1000, 0x2000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0400, 0x1000, 0x0800, 0x6000, 0xC000, 0x8000, 0x4000, 0x2000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0400, 0x0000, 0x1000, 0xC000, 0x8000, 0x2000, 0x4000, 0x0800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0800, 0x0000, 0x2000, 0xC000, 0x8000, 0x4000, 0x1000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0000, 0x1000, 0x2000, 0x4000, 0x8000, 0x6000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0000, 0x1000, 0x4000, 0x8000, 0x2000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0000, 0x2000, 0x8000, 0x4000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0000, 0x4000, 0x8000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x0000, 0x8000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  }
	};
	__declspec(align(32)) static const uint16_t masktab[TOTRUN_NUM][16] = 
		{
{ 0x8000, 0xE000, 0xE000, 0xF000, 0xF000, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFE00, 0xFE00, 0xFF00, 0xFF00, 0xFF80, 0xFF80, 0xFF80,  },
{ 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0xF000, 0xF000, 0xF000, 0xF000, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0x0000,  },
{ 0xF000, 0xE000, 0xE000, 0xE000, 0xF000, 0xF000, 0xE000, 0xE000, 0xF000, 0xF800, 0xF800, 0xFC00, 0xF800, 0xFC00, 0x0000, 0x0000,  },
{ 0xF800, 0xE000, 0xF000, 0xF000, 0xE000, 0xE000, 0xE000, 0xF000, 0xE000, 0xF000, 0xF800, 0xF800, 0xF800, 0x0000, 0x0000, 0x0000,  },
{ 0xF000, 0xF000, 0xF000, 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0xF000, 0xF800, 0xF000, 0xF800, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xFC00, 0xF800, 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0xF000, 0xE000, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xFC00, 0xF800, 0xE000, 0xE000, 0xE000, 0xC000, 0xE000, 0xF000, 0xE000, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xFC00, 0xF000, 0xF800, 0xE000, 0xC000, 0xC000, 0xE000, 0xE000, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xFC00, 0xFC00, 0xF000, 0xC000, 0xC000, 0xE000, 0xC000, 0xF800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xF800, 0xF800, 0xE000, 0xC000, 0xC000, 0xC000, 0xF000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xF000, 0xF000, 0xE000, 0xE000, 0x8000, 0xE000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xF000, 0xF000, 0xC000, 0x8000, 0xE000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xE000, 0xE000, 0x8000, 0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0xC000, 0xC000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
{ 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  },
};
	

	return opt_code_from_bitstream_2d_16_1(currStream, lentab[vlcnum], codtab[vlcnum], masktab[vlcnum]);
}

/*!
************************************************************************
* \brief
*    read Total Zeros Chroma DC codeword from UVLC-partition
************************************************************************
*/
int readSyntaxElement_TotalZerosChromaDC(VideoParameters *p_Vid, Bitstream *currStream, int vlcnum)
{
	static const byte lentab[3][TOTRUN_NUM][16] =
	{
		//YUV420
		{{ 1,2,3,3},
		{ 1,2,2},
		{ 1,1}},
		//YUV422
		{{ 1,3,3,4,4,4,5,5},
		{ 3,2,3,3,3,3,3},
		{ 3,3,2,2,3,3},
		{ 3,2,2,2,3},
		{ 2,2,2,2},
		{ 2,2,1},
		{ 1,1}},
		//YUV444
		{{ 1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},
		{ 3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},
		{ 4,3,3,3,4,4,3,3,4,5,5,6,5,6},
		{ 5,3,4,4,3,3,3,4,3,4,5,5,5},
		{ 4,4,4,3,3,3,3,3,4,5,4,5},
		{ 6,5,3,3,3,3,3,3,4,3,6},
		{ 6,5,3,3,3,2,3,4,3,6},
		{ 6,4,5,3,2,2,3,3,6},
		{ 6,6,4,2,2,3,2,5},
		{ 5,5,3,2,2,2,4},
		{ 4,4,3,3,1,3},
		{ 4,4,2,1,3},
		{ 3,3,1,2},
		{ 2,2,1},
		{ 1,1}}
	};

	static const byte codtab[3][TOTRUN_NUM][16] =
	{
		//YUV420
		{{ 1,1,1,0},
		{ 1,1,0},
		{ 1,0}},
		//YUV422
		{{ 1,2,3,2,3,1,1,0},
		{ 0,1,1,4,5,6,7},
		{ 0,1,1,2,6,7},
		{ 6,0,1,2,7},
		{ 0,1,2,3},
		{ 0,1,1},
		{ 0,1}},
		//YUV444
		{{1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
		{7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
		{5,7,6,5,4,3,4,3,2,3,2,1,1,0},
		{3,7,5,4,6,5,4,3,3,2,2,1,0},
		{5,4,3,7,6,5,4,3,2,1,1,0},
		{1,1,7,6,5,4,3,2,1,1,0},
		{1,1,5,4,3,3,2,1,1,0},
		{1,1,1,3,3,2,2,1,0},
		{1,0,1,3,2,1,1,1,},
		{1,0,1,3,2,1,1,},
		{0,1,1,2,1,3},
		{0,1,1,1,1},
		{0,1,1,1},
		{0,1,1},
		{0,1}}
	};

	int yuv = p_Vid->active_sps->chroma_format_idc - 1;
	return code_from_bitstream_2d_16_1(currStream, &lentab[yuv][vlcnum][0], &codtab[yuv][vlcnum][0]);
}


/*!
************************************************************************
* \brief
*    read  Run codeword from UVLC-partition
************************************************************************
*/
int readSyntaxElement_Run(Bitstream *currStream, int vlcnum)
{
	__declspec(align(32)) static const uint16_t lentab[TOTRUN_NUM][16] =
	{
		{1,1},
		{1,2,2},
		{2,2,2,2},
		{2,2,2,3,3},
		{2,2,3,3,3,3},
		{2,3,3,3,3,3,3},
		{3,3,3,3,3,3,3,4,5,6,7,8,9,10,11},
	};
/*
	static const byte codtab[TOTRUN_NUM][16] =
	{
		{1,0},
		{1,1,0},
		{3,2,1,0},
		{3,2,1,1,0},
		{3,2,3,2,1,0},
		{3,0,1,3,2,5,4},
		{7,6,5,4,3,2,1,1,1,1,1,1,1,1,1},
	};*/

	__declspec(align(32)) static const uint16_t codtab[TOTRUN_NUM][16] = 
		{
{ 0x8000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0x8000, 0x4000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0xC000, 0x8000, 0x4000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0xC000, 0x8000, 0x4000, 0x2000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0xC000, 0x8000, 0x6000, 0x4000, 0x2000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0xC000, 0x0000, 0x2000, 0x6000, 0x4000, 0xA000, 0x8000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,  },
{ 0xE000, 0xC000, 0xA000, 0x8000, 0x6000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0xFFFF,  }
		};
	__declspec(align(32)) static const uint16_t masktab[TOTRUN_NUM][16] = 
{
{ 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, },
{ 0x8000, 0xC000, 0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, },
{ 0xC000, 0xC000, 0xC000, 0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, },
{ 0xC000, 0xC000, 0xC000, 0xE000, 0xE000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, },
{ 0xC000, 0xC000, 0xE000, 0xE000, 0xE000, 0xE000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, },
{ 0xC000, 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, },
{ 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0x0000, }
	};
	return opt_code_from_bitstream_2d_16_1(currStream, lentab[vlcnum], codtab[vlcnum], masktab[vlcnum]);
}


/*!
************************************************************************
* \brief
*  Reads bits from the bitstream buffer
*
* \param buffer
*    containing VLC-coded data bits
* \param totbitoffset
*    bit offset from start of partition
* \param info
*    returns value of the read bits
* \param bitcount
*    total bytes in bitstream
* \param numbits
*    number of bits to read
*
************************************************************************
*/

int GetBits (const uint8_t buffer[],int totbitoffset,int *info, int bitcount,
						 int numbits)
{
	int bitoffset  = /*7 - */(totbitoffset & 0x07); // bit from start of byte
	int byteoffset = (totbitoffset >> 3); // byte from start of buffer
	const uint8_t *ptr  = &(buffer[byteoffset]);

	uint32_t tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8) | (ptr[3]);
	tmp <<= bitoffset;
	tmp >>= 32 - numbits;
	*info = tmp;
	return numbits;
}

/*!
************************************************************************
* \brief
*  Reads bits from the bitstream buffer
*
* \param buffer
*    buffer containing VLC-coded data bits
* \param totbitoffset
*    bit offset from start of partition
* \param bitcount
*    total bytes in bitstream
* \param numbits
*    number of bits to read
*
************************************************************************
*/

static int ShowBits (const uint8_t buffer[],int totbitoffset,int bitcount, int numbits)
{
	int bitoffset  = /*7 - */(totbitoffset & 0x07); // bit from start of byte
	int byteoffset = (totbitoffset >> 3); // byte from start of buffer
	const uint8_t *ptr  = &(buffer[byteoffset]);

	uint32_t tmp = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] <<  8) | (ptr[3]);
	tmp <<= bitoffset;
	tmp >>= 32 - numbits;
	return tmp;

#if 0
	if ((totbitoffset + numbits )  > bitcount) 
	{
		return -1;
	}
	else
	{
		int bitoffset  = 7 - (totbitoffset & 0x07); // bit from start of byte
		int byteoffset = (totbitoffset >> 3); // byte from start of buffer
		const uint8_t *curbyte  = &(buffer[byteoffset]);
		int inf        = 0;

		while (numbits--)
		{
			inf <<=1;    
			inf |= ((*curbyte)>> (bitoffset--)) & 0x01;

			if (bitoffset == -1 ) 
			{ //Move onto next byte to get all of numbits
				curbyte++;
				bitoffset = 7;
			}
		}
		return inf;           // return absolute offset in bit from start of frame
	}
#endif
}

