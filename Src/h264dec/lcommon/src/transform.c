/*!
***************************************************************************
* \file transform.c
*
* \brief
*    Transform functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Alexis Michael Tourapis
* \date
*    01. July 2007
**************************************************************************
*/
#include "global.h"
#include "transform.h"
#include <emmintrin.h>

void forward4x4(int **block, int **tblock, int pos_y, int pos_x)
{
	int i, ii;  
	int tmp[16];
	int *pTmp = tmp, *pblock;
	int p0,p1,p2,p3;
	int t0,t1,t2,t3;

	// Horizontal
	for (i=pos_y; i < pos_y + BLOCK_SIZE; i++)
	{
		pblock = &block[i][pos_x];
		p0 = *(pblock++);
		p1 = *(pblock++);
		p2 = *(pblock++);
		p3 = *(pblock  );

		t0 = p0 + p3;
		t1 = p1 + p2;
		t2 = p1 - p2;
		t3 = p0 - p3;

		*(pTmp++) =  t0 + t1;
		*(pTmp++) = (t3 << 1) + t2;
		*(pTmp++) =  t0 - t1;    
		*(pTmp++) =  t3 - (t2 << 1);
	}

	// Vertical 
	for (i=0; i < BLOCK_SIZE; i++)
	{
		pTmp = tmp + i;
		p0 = *pTmp;
		p1 = *(pTmp += BLOCK_SIZE);
		p2 = *(pTmp += BLOCK_SIZE);
		p3 = *(pTmp += BLOCK_SIZE);

		t0 = p0 + p3;
		t1 = p1 + p2;
		t2 = p1 - p2;
		t3 = p0 - p3;

		ii = pos_x + i;
		tblock[pos_y    ][ii] = t0 +  t1;
		tblock[pos_y + 1][ii] = t2 + (t3 << 1);
		tblock[pos_y + 2][ii] = t0 -  t1;
		tblock[pos_y + 3][ii] = t3 - (t2 << 1);
	}
}

static void inverse4x4(const h264_short_block_t tblock, h264_short_block_t block, int pos_y, int pos_x)
{
		int i;  
		short tmp[16];
		short *pTmp = tmp;
		int p0,p1,p2,p3;
		int t0,t1,t2,t3;

		// Horizontal
		for (i = 0; i < BLOCK_SIZE; i++)
		{
			t0 = tblock[i][0];
			t1 = tblock[i][1];
			t2 = tblock[i][2];
			t3 = tblock[i][3];

			p0 =  t0 + t2;
			p1 =  t0 - t2;
			p2 = (t1 >> 1) - t3;
			p3 =  t1 + (t3 >> 1);

			*(pTmp++) = p0 + p3;
			*(pTmp++) = p1 + p2;
			*(pTmp++) = p1 - p2;
			*(pTmp++) = p0 - p3;
		}

		//  Vertical 
		for (i = 0; i < BLOCK_SIZE; i++)
		{
			pTmp = tmp + i;
			t0 = *pTmp;
			t1 = *(pTmp += BLOCK_SIZE);
			t2 = *(pTmp += BLOCK_SIZE);
			t3 = *(pTmp += BLOCK_SIZE);

			p0 = t0 + t2;
			p1 = t0 - t2;
			p2 =(t1 >> 1) - t3;
			p3 = t1 + (t3 >> 1);

			block[0][i] = p0 + p3;
			block[1][i] = p1 + p2;
			block[2][i] = p1 - p2;
			block[3][i] = p0 - p3;
		}
}

#ifdef _M_IX86
// benski> this exists just for conformance testing. not used in production code
static void inverse4x4_sse2_x86(const h264_short_macroblock_t tblock, h264_short_macroblock_t block, int pos_y, int pos_x)
{
		__asm
		{
			mov edx, pos_y
			shl edx, 4 // 16 step stride
			add edx, pos_x
			shl edx, 1 // * sizeof(short)

			// eax: pointer to the start of tblock (offset by passed pos_y, pos_x)
			mov eax, edx
			add eax, tblock

			// esi: results
			mov esi, edx
			add esi, block

			// load 4x4 matrix
			movq mm0, MMWORD PTR 0[eax]
			movq mm1, MMWORD PTR 32[eax]
			movq mm2, MMWORD PTR 64[eax]
			movq mm3, MMWORD PTR 96[eax]

			// rotate 4x4 matrix
			movq mm4, mm0 // p0 = mm4 (copy)
			punpcklwd mm0, mm2 // r0 = mm0
			punpckhwd mm4, mm2 // r2 = mm4
			movq mm5, mm1 // p1 = mm5 (copy)
			punpcklwd mm1, mm3 // r1 = mm1
			punpckhwd mm5, mm3 // r3 = mm5
			movq mm6, mm0 // r0 = mm6 (copy)
			punpcklwd mm0, mm1 // t0 = mm0
			punpckhwd mm6, mm1 // t1 = mm6
			movq mm1, mm4 // r2 = mm1 (copy)
			punpcklwd mm1, mm5 // t2 = mm1
			punpckhwd mm4, mm5 // t3 = mm4

			/* register state:
			mm0: t0
			mm1: t2
			mm2: 
			mm3: 
			mm4: t3
			mm5: 
			mm6: t1
			mm7: 
			*/

			/*
			p0 =  t0 + t2;
			p1 =  t0 - t2;
			p2 = (t1 >> 1) - t3;
			p3 =  t1 + (t3 >> 1);
			*/
			movq mm2, mm0 // mm2 = t0 (copy)
			paddw mm0, mm1 // mm0 = p0
			psubw mm2, mm1 // mm2 = p1, mm1 available
			movq mm5, mm6 // mm5 = t1 (copy)
			psraw mm5, 1 // mm5 = (t1 >> 1)
			psubw mm5, mm4 // mm5 = p2
			psraw mm4, 1 // mm4 = (t3 >> 1)
			paddw mm6, mm4 // mm6 = p3

			/* register state:
			mm0: p0
			mm1: 
			mm2: p1
			mm3: 
			mm4: 
			mm5: p2
			mm6: p3
			mm7: 
			*/

			/*
			*(pTmp++) = p0 + p3;
			*(pTmp++) = p1 + p2;
			*(pTmp++) = p1 - p2;
			*(pTmp++) = p0 - p3;
			*/

			movq mm3, mm0 // mm3 = p0 (copy)
			paddw mm0, mm6 // mm0 = r0
			movq mm1, mm2 // mm1 = p1 (copy)
			paddw mm1, mm5 // mm1 = r1
			psubw mm2, mm5 // mm2 = r2, mm5 available
			psubw mm3, mm6 // mm3 = r3

			/* register state:
			mm0: r0
			mm1: r1
			mm2: r2
			mm3: r3
			mm4: 
			mm5: 
			mm6: 
			mm7: 
			*/

			// rotate 4x4 matrix to set up for vertical
			movq mm4, mm0 // r0 = mm4 (copy)
			punpcklwd mm0, mm2 // p0 = mm0
			punpckhwd mm4, mm2 // p2 = mm4
			movq mm5, mm1 // r1 = mm5 (copy)
			punpcklwd mm1, mm3 // p1 = mm1
			punpckhwd mm5, mm3 // p3 = mm5
			movq mm6, mm0 // p0 = mm6 (copy)
			punpcklwd mm0, mm1 // t0 = mm0
			punpckhwd mm6, mm1 // t1 = mm6
			movq mm1, mm4 // p2 = mm1 (copy)
			punpcklwd mm1, mm5 // t2 = mm1
			punpckhwd mm4, mm5 // t3 = mm4

			/* register state:
			mm0: t0
			mm1: t2
			mm2: 
			mm3: 
			mm4: t3
			mm5: 
			mm6: t1
			mm7: 
			*/
					/*
			p0 =  t0 + t2;
			p1 =  t0 - t2;
			p2 = (t1 >> 1) - t3;
			p3 =  t1 + (t3 >> 1);
			*/
			movq mm2, mm0 // mm2 = t0 (copy)
			paddw mm0, mm1 // mm0 = p0
			psubw mm2, mm1 // mm2 = p1, mm1 available
			movq mm5, mm6 // mm5 = t1 (copy)
			psraw mm5, 1 // mm5 = (t1 >> 1)
			psubw mm5, mm4 // mm5 = p2
			psraw mm4, 1 // mm4 = (t3 >> 1)
			paddw mm6, mm4 // mm6 = p3

			/* register state:
			mm0: p0
			mm1: 
			mm2: p1
			mm3: 
			mm4: 
			mm5: p2
			mm6: p3
			mm7: 
			*/

			/*
			*(pTmp++) = p0 + p3;
			*(pTmp++) = p1 + p2;
			*(pTmp++) = p1 - p2;
			*(pTmp++) = p0 - p3;
			*/

			movq mm3, mm0 // mm3 = p0 (copy)
			paddw mm0, mm6 // mm0 = r0
			movq mm1, mm2 // mm1 = p1 (copy)
			paddw mm1, mm5 // mm1 = r1
			psubw mm2, mm5 // mm2 = r2, mm5 available
			psubw mm3, mm6 // mm3 = r3

			/* register state:
			mm0: r0
			mm1: r1
			mm2: r2
			mm3: r3
			mm4: 
			mm5: 
			mm6: 
			mm7: 
			*/
			movq XMMWORD PTR 0[esi], mm0
			movq XMMWORD PTR 32[esi], mm1
			movq XMMWORD PTR 64[esi], mm2
			movq XMMWORD PTR 96[esi], mm3
		}
}
#endif

static void sample_reconstruct(h264_imgpel_macroblock_t curImg, const h264_imgpel_macroblock_t mpr, const h264_short_block_t tblock, int joff, int mb_x, int max_imgpel_value)
{
	#ifdef _M_IX86
		__asm
		{
			// mm0 : constant value 32
			mov edx, 0x00200020
			movd mm0, edx
			punpckldq	mm0, mm0

			// ecx: y offset
			mov ecx, joff
			shl ecx, 4 // imgpel stuff is going to be 16 byte stride
			add ecx, mb_x

			// eax: curImg
			mov eax, curImg
			add eax, ecx

			// edx: mpr
			mov edx, mpr
			add edx, ecx

			// ecx: tblock (which is short, not byte)
			mov ecx, tblock
			
			// mm7: zero
			pxor mm7, mm7

			// load coefficients
			movq	mm1, MMWORD PTR 0[ecx]
			movq	mm2, MMWORD PTR 8[ecx]
			movq	mm3, MMWORD PTR 16[ecx]
			movq	mm4, MMWORD PTR 24[ecx]
			paddw mm1, mm0 // rres + 32
			paddw mm2, mm0 // rres + 32
			paddw mm3, mm0 // rres + 32
			paddw mm0, mm4 // rres + 32
			psraw mm1, 6 // (rres + 32) >> 6
			psraw mm2, 6 // (rres + 32) >> 6
			psraw mm3, 6 // (rres + 32) >> 6
			psraw mm0, 6 // (rres + 32) >> 6
			// mm1-mm3: tblock[0] - tblock[2], mm0: tblock[3]

			// convert mpr from unsigned char to short
			movd mm4, DWORD PTR 0[edx]
			movd mm5, DWORD PTR 16[edx]
			movd mm6, DWORD PTR 32[edx]
			punpcklbw mm4, mm7
			punpcklbw mm5, mm7
			punpcklbw mm6, mm7
			paddsw mm4, mm1 // pred_row + rres_row
			movd mm1, DWORD PTR 48[edx] // reuse mm1 for mpr[3]
			paddsw mm5, mm2 // pred_row + rres_row
			punpcklbw mm1, mm7
			paddsw mm6, mm3 // pred_row + rres_row			
			paddsw mm1, mm0 // pred_row + rres_row
			// results in mm4, mm5, mm6, mm1
			
			// move back to 8 bit
			packuswb mm4, mm7
			packuswb mm5, mm7
			packuswb mm6, mm7
			packuswb mm1, mm7
			movd DWORD PTR 0[eax], mm4
			movd DWORD PTR 16[eax], mm5
			movd DWORD PTR 32[eax], mm6
			movd DWORD PTR 48[eax], mm1
		}
#else
  int i, j;

  for (j = 0; j < BLOCK_SIZE; j++)
  {
    for (i=0;i<BLOCK_SIZE;i++)
      curImg[j+joff][mb_x+i] = (imgpel) iClip1( max_imgpel_value, rshift_rnd_sf(tblock[j][i], DQ_BITS) + mpr[j+joff][mb_x+i]);
  }
#endif
}

#if defined(_M_IX86) && defined(_DEBUG)
void itrans4x4_sse2(const h264_short_macroblock_t tblock, const h264_imgpel_macroblock_t mb_pred, h264_imgpel_macroblock_t mb_rec, int pos_x, int pos_y)
{
	__declspec(align(32)) static const short const32[4] = {32, 32, 32, 32};
		__asm
		{
			mov edx, pos_y
			shl edx, 4 // imgpel stuff is going to be 16 byte stride
			add edx, pos_x

			// eax: tblock
			lea eax, [edx*2]
			add eax, tblock

			// ecx: mpr
			mov ecx, mb_pred
			add ecx, edx

			// edx: results
			add edx, mb_rec

			// load 4x4 matrix
			movq mm0, MMWORD PTR 0[eax]
			movq mm1, MMWORD PTR 32[eax]
			movq mm2, MMWORD PTR 64[eax]
			movq mm3, MMWORD PTR 96[eax]

			// rotate 4x4 matrix
			movq mm4, mm0 // p0 = mm4 (copy)
			punpcklwd mm0, mm2 // r0 = mm0
			punpckhwd mm4, mm2 // r2 = mm4
			movq mm5, mm1 // p1 = mm5 (copy)
			punpcklwd mm1, mm3 // r1 = mm1
			punpckhwd mm5, mm3 // r3 = mm5
			movq mm6, mm0 // r0 = mm6 (copy)
			punpcklwd mm0, mm1 // t0 = mm0
			punpckhwd mm6, mm1 // t1 = mm6
			movq mm1, mm4 // r2 = mm1 (copy)
			punpcklwd mm1, mm5 // t2 = mm1
			punpckhwd mm4, mm5 // t3 = mm4

			/* register state:
			mm0: t0
			mm1: t2
			mm2: 
			mm3: 
			mm4: t3
			mm5: 
			mm6: t1
			mm7: 
			*/

			/*
			p0 =  t0 + t2;
			p1 =  t0 - t2;
			p2 = (t1 >> 1) - t3;
			p3 =  t1 + (t3 >> 1);
			*/
			movq mm2, mm0 // mm2 = t0 (copy)
			paddw mm0, mm1 // mm0 = p0
			psubw mm2, mm1 // mm2 = p1, mm1 available
			movq mm5, mm6 // mm5 = t1 (copy)
			psraw mm5, 1 // mm5 = (t1 >> 1)
			psubw mm5, mm4 // mm5 = p2
			psraw mm4, 1 // mm4 = (t3 >> 1)
			paddw mm6, mm4 // mm6 = p3

			/* register state:
			mm0: p0
			mm1: 
			mm2: p1
			mm3: 
			mm4: 
			mm5: p2
			mm6: p3
			mm7: 
			*/

			/*
			*(pTmp++) = p0 + p3;
			*(pTmp++) = p1 + p2;
			*(pTmp++) = p1 - p2;
			*(pTmp++) = p0 - p3;
			*/

			movq mm3, mm0 // mm3 = p0 (copy)
			paddw mm0, mm6 // mm0 = r0
			movq mm1, mm2 // mm1 = p1 (copy)
			paddw mm1, mm5 // mm1 = r1
			psubw mm2, mm5 // mm2 = r2, mm5 available
			psubw mm3, mm6 // mm3 = r3

			/* register state:
			mm0: r0
			mm1: r1
			mm2: r2
			mm3: r3
			mm4: 
			mm5: 
			mm6: 
			mm7: 
			*/

			// rotate 4x4 matrix to set up for vertical
			movq mm4, mm0 // r0 = mm4 (copy)
			punpcklwd mm0, mm2 // p0 = mm0
			punpckhwd mm4, mm2 // p2 = mm4
			movq mm5, mm1 // r1 = mm5 (copy)
			punpcklwd mm1, mm3 // p1 = mm1
			punpckhwd mm5, mm3 // p3 = mm5
			movq mm6, mm0 // p0 = mm6 (copy)
			punpcklwd mm0, mm1 // t0 = mm0
			punpckhwd mm6, mm1 // t1 = mm6
			movq mm1, mm4 // p2 = mm1 (copy)
			punpcklwd mm1, mm5 // t2 = mm1
			punpckhwd mm4, mm5 // t3 = mm4

			/* register state:
			mm0: t0
			mm1: t2
			mm2: 
			mm3: 
			mm4: t3
			mm5: 
			mm6: t1
			mm7: 
			*/
					/*
			p0 =  t0 + t2;
			p1 =  t0 - t2;
			p2 = (t1 >> 1) - t3;
			p3 =  t1 + (t3 >> 1);
			*/
			movq mm2, mm0 // mm2 = t0 (copy)
			paddw mm0, mm1 // mm0 = p0
			psubw mm2, mm1 // mm2 = p1, mm1 available
			movq mm5, mm6 // mm5 = t1 (copy)
			psraw mm5, 1 // mm5 = (t1 >> 1)
			psubw mm5, mm4 // mm5 = p2
			psraw mm4, 1 // mm4 = (t3 >> 1)
			paddw mm6, mm4 // mm6 = p3

			/* register state:
			mm0: p0
			mm1: 
			mm2: p1
			mm3: 
			mm4: 
			mm5: p2
			mm6: p3
			mm7: 
			*/

			/*
			*(pTmp++) = p0 + p3;
			*(pTmp++) = p1 + p2;
			*(pTmp++) = p1 - p2;
			*(pTmp++) = p0 - p3;
			*/

			movq mm3, mm0 // mm3 = p0 (copy)
			paddw mm0, mm6 // mm0 = r0
			movq mm1, mm2 // mm1 = p1 (copy)
			paddw mm1, mm5 // mm1 = r1
			psubw mm2, mm5 // mm2 = r2, mm5 available
			psubw mm3, mm6 // mm3 = r3

			/* register state:
			mm0: r0
			mm1: r1
			mm2: r2
			mm3: r3
			mm4: 
			mm5: 
			mm6: 
			mm7: 
			*/
/* --- 4x4 iDCT done, now time to combine with mpr --- */
			// mm0 : constant value 32
			movq	mm7, const32

			paddw mm0, mm7 // rres + 32
			psraw mm0, 6 // (rres + 32) >> 6
			paddw mm1, mm7 // rres + 32
			psraw mm1, 6 // (rres + 32) >> 6
			paddw mm2, mm7 // rres + 32
			psraw mm2, 6 // (rres + 32) >> 6
			paddw mm3, mm7 // rres + 32
			psraw mm3, 6 // (rres + 32) >> 6

			pxor mm7, mm7

			// convert mpr from unsigned char to short
			movd mm4, DWORD PTR 0[ecx]
			movd mm5, DWORD PTR 16[ecx]
			movd mm6, DWORD PTR 32[ecx]
			punpcklbw mm4, mm7
			punpcklbw mm5, mm7
			punpcklbw mm6, mm7
			paddsw mm4, mm0 // pred_row + rres_row
			movd mm0, DWORD PTR 48[ecx] // reuse mm0 for mpr[3]
			paddsw mm5, mm1 // pred_row + rres_row
			punpcklbw mm0, mm7
			paddsw mm6, mm2 // pred_row + rres_row			
			paddsw mm0, mm3 // pred_row + rres_row
			// results in mm4, mm5, mm6, mm0
			
			// move back to 8 bit
			packuswb mm4, mm7
			packuswb mm5, mm7
			packuswb mm6, mm7
			packuswb mm0, mm7
			movd DWORD PTR 0[edx], mm4
			movd DWORD PTR 16[edx], mm5
			movd DWORD PTR 32[edx], mm6
			movd DWORD PTR 48[edx], mm0
		}
}
#elif defined(_M_X64)
static void itrans4x4_sse2(const h264_int_macroblock_t tblock, const h264_imgpel_macroblock_t mb_pred, h264_imgpel_macroblock_t mb_rec, int pos_x, int pos_y)
{
	__declspec(align(32)) static const int const32[4] = {32, 32, 32, 32};
			__m128i  p0,p1,p2,p3;
		__m128i t0,t1,t2,t3;
		__m128i r0,r1,r2,r3;
		__m128i c32, zero;

		// horizontal 
		// load registers in vertical mode, we'll rotate them next
		p0 = _mm_loadu_si128((__m128i *)&tblock[pos_y][pos_x]); // 00 01 02 03
		p1 = _mm_loadu_si128((__m128i *)&tblock[pos_y+1][pos_x]); // 10 11 12 13
		p2 = _mm_loadu_si128((__m128i *)&tblock[pos_y+2][pos_x]); // 20 21 22 23
		p3 = _mm_loadu_si128((__m128i *)&tblock[pos_y+3][pos_x]); // 30 31 32 33
		
		// rotate 4x4 matrix
		r0 = _mm_unpacklo_epi32(p0, p2); // 00 20 01 21
		r1 = _mm_unpacklo_epi32(p1, p3); // 10 30 11 31
		r2 = _mm_unpackhi_epi32(p0, p2); // 02 22 03 23
		r3 = _mm_unpackhi_epi32(p1, p3); // 12 32 13 33
		t0 = _mm_unpacklo_epi32(r0, r1); // 00 10 20 30
		t1 = _mm_unpackhi_epi32(r0, r1); // 01 11 21 31
		t2 = _mm_unpacklo_epi32(r2, r3); // 02 12 22 32
		t3 = _mm_unpackhi_epi32(r2, r3); // 03 13 23 33

		p0 = _mm_add_epi32(t0, t2); //t0 + t2;
		p1 = _mm_sub_epi32(t0, t2); // t0 - t2;
		p2 = _mm_srai_epi32(t1, 1); // t1 >> 1
		p2 = _mm_sub_epi32(p2, t3); // (t1 >> 1) - t3;
		p3 = _mm_srai_epi32(t3, 1); // (t3 >> 1)
		p3 = _mm_add_epi32(p3, t1); // t1 + (t3 >> 1);

		t0 = _mm_add_epi32(p0, p3); //p0 + p3;
		t1 = _mm_add_epi32(p1, p2);//p1 + p2;
		t2 = _mm_sub_epi32(p1, p2); //p1 - p2;
		t3 = _mm_sub_epi32(p0, p3); //p0 - p3;

		// rotate 4x4 matrix to set up for vertical
		r0 = _mm_unpacklo_epi32(t0, t2); 
		r1 = _mm_unpacklo_epi32(t1, t3); 
		r2 = _mm_unpackhi_epi32(t0, t2); 
		r3 = _mm_unpackhi_epi32(t1, t3); 
		t0 = _mm_unpacklo_epi32(r0, r1); 
		t1 = _mm_unpackhi_epi32(r0, r1); 
		t2 = _mm_unpacklo_epi32(r2, r3); 
		t3 = _mm_unpackhi_epi32(r2, r3); 

		// vertical
		p0 = _mm_add_epi32(t0, t2); //t0 + t2;
		p3 = _mm_srai_epi32(t3, 1); // (t3 >> 1)
		p3 = _mm_add_epi32(p3, t1); // t1 + (t3 >> 1);
		r0 = _mm_add_epi32(p0, p3); //p0 + p3;
		r3 = _mm_sub_epi32(p0, p3); //p0 - p3;
		p1 = _mm_sub_epi32(t0, t2); // t0 - t2;
		p2 = _mm_srai_epi32(t1, 1); // t1 >> 1
		p2 = _mm_sub_epi32(p2, t3); // (t1 >> 1) - t3;
		r1 = _mm_add_epi32(p1, p2);//p1 + p2;
		r2 = _mm_sub_epi32(p1, p2); //p1 - p2;

		c32 = _mm_load_si128((const __m128i *)const32);
		zero = _mm_setzero_si128();

		// (x + 32) >> 6
		r0 = _mm_add_epi32(r0, c32);
		r0 = _mm_srai_epi32(r0, 6);
		r1 = _mm_add_epi32(r1, c32);
		r1 = _mm_srai_epi32(r1, 6);
		r2 = _mm_add_epi32(r2, c32);
		r2 = _mm_srai_epi32(r2, 6);
		r3 = _mm_add_epi32(r3, c32);
		r3 = _mm_srai_epi32(r3, 6);

		// convert to 16bit values
		r0 = _mm_packs_epi32(r0, r1);
		r2 = _mm_packs_epi32(r2, r3);

		// convert mpr from unsigned char to short
		p0 = _mm_cvtsi32_si128(*(int32_t *)&mb_pred[pos_y][pos_x]);
		p1 = _mm_cvtsi32_si128(*(int32_t *)&mb_pred[pos_y+1][pos_x]);
		p0 = _mm_unpacklo_epi32(p0, p1);
		p0 = _mm_unpacklo_epi8(p0, zero); // convert to short
		r0 = _mm_add_epi16(r0, p0);

		p0 = _mm_cvtsi32_si128(*(int32_t *)&mb_pred[pos_y+2][pos_x]);
		p1 = _mm_cvtsi32_si128(*(int32_t *)&mb_pred[pos_y+3][pos_x]);
		p0 = _mm_unpacklo_epi32(p0, p1);
		p0 = _mm_unpacklo_epi8(p0, zero); // convert to short
		r2 = _mm_add_epi16(r2, p0);

		r0 = _mm_packus_epi16(r0, r2); // convert to unsigned char
		*(int32_t *)&mb_rec[pos_y][pos_x] = _mm_cvtsi128_si32(r0);
		r0 = _mm_srli_si128(r0, 4);
		*(int32_t *)&mb_rec[pos_y+1][pos_x] = _mm_cvtsi128_si32(r0);
		r0 = _mm_srli_si128(r0, 4);
		*(int32_t *)&mb_rec[pos_y+2][pos_x] = _mm_cvtsi128_si32(r0);
		r0 = _mm_srli_si128(r0, 4);
		*(int32_t *)&mb_rec[pos_y+3][pos_x] = _mm_cvtsi128_si32(r0);
}
#endif

void itrans4x4_c(const h264_short_block_t tblock, const h264_imgpel_macroblock_t mb_pred, h264_imgpel_macroblock_t mb_rec, int pos_x, int pos_y)
{
  inverse4x4(tblock, (h264_short_block_row_t *)tblock,pos_y,pos_x);
	sample_reconstruct(mb_rec, mb_pred, tblock, pos_y, pos_x, 255);
}

void ihadamard4x4(int block[4][4])
{
	int i;  
	int tmp[16];
	int *pTmp = tmp;
	int p0,p1,p2,p3;
	int t0,t1,t2,t3;

	// Horizontal
	for (i = 0; i < BLOCK_SIZE; i++)
	{
		t0 = block[i][0];
		t1 = block[i][1];
		t2 = block[i][2];
		t3 = block[i][3];

		p0 = t0 + t2;
		p1 = t0 - t2;
		p2 = t1 - t3;
		p3 = t1 + t3;

		*(pTmp++) = p0 + p3;
		*(pTmp++) = p1 + p2;
		*(pTmp++) = p1 - p2;
		*(pTmp++) = p0 - p3;
	}

	//  Vertical 
	for (i = 0; i < BLOCK_SIZE; i++)
	{
		pTmp = tmp + i;
		t0 = *pTmp;
		t1 = *(pTmp += BLOCK_SIZE);
		t2 = *(pTmp += BLOCK_SIZE);
		t3 = *(pTmp += BLOCK_SIZE);

		p0 = t0 + t2;
		p1 = t0 - t2;
		p2 = t1 - t3;
		p3 = t1 + t3;

		block[0][i] = p0 + p3;
		block[1][i] = p1 + p2;
		block[2][i] = p1 - p2;
		block[3][i] = p0 - p3;
	}
}

void ihadamard4x2(int **tblock, int **block)
{
	int i;  
	int tmp[8];
	int *pTmp = tmp;
	int p0,p1,p2,p3;
	int t0,t1,t2,t3;

	// Horizontal
	*(pTmp++) = tblock[0][0] + tblock[1][0];
	*(pTmp++) = tblock[0][1] + tblock[1][1];
	*(pTmp++) = tblock[0][2] + tblock[1][2];
	*(pTmp++) = tblock[0][3] + tblock[1][3];

	*(pTmp++) = tblock[0][0] - tblock[1][0];
	*(pTmp++) = tblock[0][1] - tblock[1][1];
	*(pTmp++) = tblock[0][2] - tblock[1][2];
	*(pTmp  ) = tblock[0][3] - tblock[1][3];

	// Vertical
	pTmp = tmp;
	for (i = 0; i < 2; i++)
	{
		p0 = *(pTmp++);
		p1 = *(pTmp++);
		p2 = *(pTmp++);
		p3 = *(pTmp++);

		t0 = p0 + p2;
		t1 = p0 - p2;
		t2 = p1 - p3;
		t3 = p1 + p3;

		// coefficients (transposed)
		block[0][i] = t0 + t3;
		block[1][i] = t1 + t2;
		block[2][i] = t1 - t2;
		block[3][i] = t0 - t3;
	}
}

//following functions perform 8 additions, 8 assignments. Should be a bit faster
void ihadamard2x2(int tblock[4], int block[4])
{
	int t0,t1,t2,t3;

	t0 = tblock[0] + tblock[1];
	t1 = tblock[0] - tblock[1];
	t2 = tblock[2] + tblock[3];
	t3 = tblock[2] - tblock[3];

	block[0] = (t0 + t2);
	block[1] = (t1 + t3);
	block[2] = (t0 - t2);
	block[3] = (t1 - t3);
}

