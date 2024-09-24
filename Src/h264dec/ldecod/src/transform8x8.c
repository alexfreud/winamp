
/*!
***************************************************************************
* \file transform8x8.c
*
* \brief
*    8x8 transform functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Yuri Vatis
*    - Jan Muenster
*
* \date
*    12. October 2003
**************************************************************************
*/

#include "global.h"

#include "image.h"
#include "mb_access.h"
#include "elements.h"
#include "transform8x8.h"
#include "transform.h"
#include "quant.h"
#include <emmintrin.h>

static void inverse8x8_sse2(h264_short_8x8block_row_t *block)
{
		__m128i a0, a1, a2, a3;
		__m128i p0, p1, p2, p3, p4, p5 ,p6, p7;  
		__m128i b0, b1, b2, b3, b4, b5, b6, b7;
		__m128i r0, r1, r2, r3, r4, r5, r6, r7;

		// Horizontal  
			b0 = _mm_load_si128((__m128i *)(block[0])); 
			b1 = _mm_load_si128((__m128i *)(block[1])); 
			b2 = _mm_load_si128((__m128i *)(block[2])); 
			b3 = _mm_load_si128((__m128i *)(block[3])); 
			b4 = _mm_load_si128((__m128i *)(block[4])); 
			b5 = _mm_load_si128((__m128i *)(block[5])); 
			b6 = _mm_load_si128((__m128i *)(block[6])); 
			b7 = _mm_load_si128((__m128i *)(block[7])); 

			/* rotate 8x8 (ugh) */
			r0 = _mm_unpacklo_epi16(b0, b2); 
			r1 = _mm_unpacklo_epi16(b1, b3); 
			r2 = _mm_unpackhi_epi16(b0, b2); 
			r3 = _mm_unpackhi_epi16(b1, b3); 
			r4 = _mm_unpacklo_epi16(b4, b6); 
			r5 = _mm_unpacklo_epi16(b5, b7); 
			r6 = _mm_unpackhi_epi16(b4, b6); 
			r7 = _mm_unpackhi_epi16(b5, b7); 

			b0 = _mm_unpacklo_epi16(r0, r1); 
			b1 = _mm_unpackhi_epi16(r0, r1); 
			b2 = _mm_unpacklo_epi16(r2, r3); 
			b3 = _mm_unpackhi_epi16(r2, r3); 
			b4 = _mm_unpacklo_epi16(r4, r5); 
			b5 = _mm_unpackhi_epi16(r4, r5); 
			b6 = _mm_unpacklo_epi16(r6, r7); 
			b7 = _mm_unpackhi_epi16(r6, r7); 

			p0 = _mm_unpacklo_epi64(b0, b4);
			p1 = _mm_unpackhi_epi64(b0, b4);
			p2 = _mm_unpacklo_epi64(b1, b5);
			p3 = _mm_unpackhi_epi64(b1, b5);
			p4 = _mm_unpacklo_epi64(b2, b6);
			p5 = _mm_unpackhi_epi64(b2, b6);
			p6 = _mm_unpacklo_epi64(b3, b7);
			p7 = _mm_unpackhi_epi64(b3, b7);

			/* perform approx DCT */
						a0 = _mm_add_epi16(p0, p4); // p0 + p4
			a1 = _mm_sub_epi16(p0, p4); // p0 - p4
			r0 = _mm_srai_epi16(p2, 1); // p2 >> 1
			a2 = _mm_sub_epi16(p6, r0); // p6 - (p2 >> 1)
			r0 = _mm_srai_epi16(p6, 1); // p6 >> 1
			a3 = _mm_add_epi16(p2, r0); //p2 + (p6 >> 1)

			b0 =  _mm_add_epi16(a0, a3); // a0 + a3;
			b2 =  _mm_sub_epi16(a1, a2);  // a1 - a2;
			b4 =  _mm_add_epi16(a1, a2);    // a1 + a2;
			b6 =  _mm_sub_epi16(a0, a3);  // a0 - a3;

			//-p3 + p5 - p7 - (p7 >> 1);    
			r0 = _mm_srai_epi16(p7, 1); // p7 >> 1
			a0 = _mm_sub_epi16(p5, p3); // p5 - p3
			a0 = _mm_sub_epi16(a0, p7); // (-p3 + p5) - p7
			a0 = _mm_sub_epi16(a0, r0); // (-p3 + p5 - p7) - (p7 >> 1)

			//p1 + p7 - p3 - (p3 >> 1);    
			r0 =  _mm_srai_epi16(p3, 1); // (p3 >> 1)
			a1 = _mm_add_epi16(p1, p7); // p1 + p7
			a1 = _mm_sub_epi16(a1, p3); // (p1 + p7) - p3
			a1 = _mm_sub_epi16(a1, r0); // (p1 + p7 - p3) - (p3>>1)

			// -p1 + p7 + p5 + (p5 >> 1);   
			r0 =  _mm_srai_epi16(p5, 1); // (p5 >> 1)
			a2 = _mm_sub_epi16(p7, p1); // p7 - p1
			a2 = _mm_add_epi16(a2, p5); // -p1 + p7 + p5
			a2 = _mm_add_epi16(a2, r0); // (-p1 + p7 + p5) + (p5 >> 1)

			// p3 + p5 + p1 + (p1 >> 1);
			a3 = _mm_add_epi16(p3, p5); // p3+p5
			a3 = _mm_add_epi16(a3, p1); // p3 + p5 + p1
			p1 = _mm_srai_epi16(p1, 1); // p1 >> 1
			a3 = _mm_add_epi16(a3, p1); //p3 + p5 + p1 + (p1 >> 1)

			r0 = _mm_srai_epi16(a3, 2); // a3>>2
			b1 = _mm_add_epi16(a0, r0); //a0 + (a3>>2);    
			r0 = _mm_srai_epi16(a2, 2); // a2>>2
			b3 = _mm_add_epi16(a1, r0); // a1 + (a2>>2);    
			a1 = _mm_srai_epi16(a1, 2); // all done with a1, so this is safe
			b5 = _mm_sub_epi16(a2, a1); //a2 - (a1>>2);
			a0 = _mm_srai_epi16(a0, 2); // all done with a0, so this is safe
			b7 = _mm_sub_epi16(a3, a0); //a3 - (a0>>2);                

			p0 = _mm_add_epi16(b0, b7); // b0 + b7;
			p1 = _mm_sub_epi16(b2, b5); // b2 - b5;
			p2 = _mm_add_epi16(b4, b3); // b4 + b3;
			p3 = _mm_add_epi16(b6, b1); // b6 + b1;
			p4 = _mm_sub_epi16(b6, b1); // b6 - b1;
			p5 = _mm_sub_epi16(b4, b3); // b4 - b3;
			p6 = _mm_add_epi16(b2, b5); // b2 + b5;
			p7 = _mm_sub_epi16(b0, b7); // b0 - b7;

						/* rotate 8x8 (ugh) */
			r0 = _mm_unpacklo_epi16(p0, p2); 
			r1 = _mm_unpacklo_epi16(p1, p3); 
			r2 = _mm_unpackhi_epi16(p0, p2); 
			r3 = _mm_unpackhi_epi16(p1, p3); 
			r4 = _mm_unpacklo_epi16(p4, p6); 
			r5 = _mm_unpacklo_epi16(p5, p7); 
			r6 = _mm_unpackhi_epi16(p4, p6); 
			r7 = _mm_unpackhi_epi16(p5, p7); 

			b0 = _mm_unpacklo_epi16(r0, r1); 
			b1 = _mm_unpackhi_epi16(r0, r1); 
			b2 = _mm_unpacklo_epi16(r2, r3); 
			b3 = _mm_unpackhi_epi16(r2, r3); 
			b4 = _mm_unpacklo_epi16(r4, r5); 
			b5 = _mm_unpackhi_epi16(r4, r5); 
			b6 = _mm_unpacklo_epi16(r6, r7); 
			b7 = _mm_unpackhi_epi16(r6, r7); 

			p0 = _mm_unpacklo_epi64(b0, b4);
			p1 = _mm_unpackhi_epi64(b0, b4);
			p2 = _mm_unpacklo_epi64(b1, b5);
			p3 = _mm_unpackhi_epi64(b1, b5);
			p4 = _mm_unpacklo_epi64(b2, b6);
			p5 = _mm_unpackhi_epi64(b2, b6);
			p6 = _mm_unpacklo_epi64(b3, b7);
			p7 = _mm_unpackhi_epi64(b3, b7);


		/*  Vertical  */

			a0 = _mm_add_epi16(p0, p4); // p0 + p4
			a1 = _mm_sub_epi16(p0, p4); // p0 - p4
			r0 = _mm_srai_epi16(p2, 1); // p2 >> 1
			a2 = _mm_sub_epi16(p6, r0); // p6 - (p2 >> 1)
			r0 = _mm_srai_epi16(p6, 1); // p6 >> 1
			a3 = _mm_add_epi16(p2, r0); //p2 + (p6 >> 1)

			b0 =  _mm_add_epi16(a0, a3); // a0 + a3;
			b2 =  _mm_sub_epi16(a1, a2);  // a1 - a2;
			b4 =  _mm_add_epi16(a1, a2);    // a1 + a2;
			b6 =  _mm_sub_epi16(a0, a3);  // a0 - a3;

			//-p3 + p5 - p7 - (p7 >> 1);    
			r0 = _mm_srai_epi16(p7, 1); // p7 >> 1
			a0 = _mm_sub_epi16(p5, p3); // p5 - p3
			a0 = _mm_sub_epi16(a0, p7); // (-p3 + p5) - p7
			a0 = _mm_sub_epi16(a0, r0); // (-p3 + p5 - p7) - (p7 >> 1)

			//p1 + p7 - p3 - (p3 >> 1);    
			r0 =  _mm_srai_epi16(p3, 1); // (p3 >> 1)
			a1 = _mm_add_epi16(p1, p7); // p1 + p7
			a1 = _mm_sub_epi16(a1, p3); // (p1 + p7) - p3
			a1 = _mm_sub_epi16(a1, r0); // (p1 + p7 - p3) - (p3>>1)

			// -p1 + p7 + p5 + (p5 >> 1);   
			r0 =  _mm_srai_epi16(p5, 1); // (p5 >> 1)
			a2 = _mm_sub_epi16(p7, p1); // p7 - p1
			a2 = _mm_add_epi16(a2, p5); // -p1 + p7 + p5
			a2 = _mm_add_epi16(a2, r0); // (-p1 + p7 + p5) + (p5 >> 1)

			// p3 + p5 + p1 + (p1 >> 1);
			r0 = _mm_srai_epi16(p1, 1); // p1 >> 1
			a3 = _mm_add_epi16(p3, p5); // p3+p5
			a3 = _mm_add_epi16(a3, p1); // p3 + p5 + p1
			a3 = _mm_add_epi16(a3, r0); //p3 + p5 + p1 + (p1 >> 1)

			r0 = _mm_srai_epi16(a3, 2); // a3>>2
			b1 = _mm_add_epi16(a0, r0); //a0 + (a3>>2);    
			r0 = _mm_srai_epi16(a2, 2); // a2>>2
			b3 = _mm_add_epi16(a1, r0); // a1 + (a2>>2);    
			a1 = _mm_srai_epi16(a1, 2); // all done with a1, so this is safe
			b5 = _mm_sub_epi16(a2, a1); //a2 - (a1>>2);
			a0 = _mm_srai_epi16(a0, 2); // all done with a0, so this is safe
			b7 = _mm_sub_epi16(a3, a0); //a3 - (a0>>2);                

			r0 = _mm_add_epi16(b0, b7); // b0 + b7;
			_mm_store_si128((__m128i *)(block[0]), r0);
			r1 = _mm_sub_epi16(b2, b5); // b2 - b5;
			_mm_store_si128((__m128i *)(block[1]), r1);
			r2 = _mm_add_epi16(b4, b3); // b4 + b3;
			_mm_store_si128((__m128i *)(block[2]), r2);
			r3 = _mm_add_epi16(b6, b1); // b6 + b1;
			_mm_store_si128((__m128i *)(block[3]), r3);
			r4 = _mm_sub_epi16(b6, b1); // b6 - b1;
			_mm_store_si128((__m128i *)(block[4]), r4);
			r5 = _mm_sub_epi16(b4, b3); // b4 - b3;
			_mm_store_si128((__m128i *)(block[5]), r5);
			r6 = _mm_add_epi16(b2, b5); // b2 + b5;
			_mm_store_si128((__m128i *)(block[6]), r6);
			r7 = _mm_sub_epi16(b0, b7); // b0 - b7;
			_mm_store_si128((__m128i *)(block[7]), r7);
}

static void inverse8x8(h264_short_8x8block_row_t *block)
{
	int i;

		//int tmp[64];
		//int *pTmp = tmp;
		int a0, a1, a2, a3;
		int p0, p1, p2, p3, p4, p5 ,p6, p7;  
		int b0, b1, b2, b3, b4, b5, b6, b7;

		// Horizontal  
		for (i=0; i < BLOCK_SIZE_8x8; i++)
		{
			p0 = block[i][0];
			p1 = block[i][1];
			p2 = block[i][2];
			p3 = block[i][3];
			p4 = block[i][4];
			p5 = block[i][5];
			p6 = block[i][6];
			p7 = block[i][7];

			a0 = p0 + p4;
			a1 = p0 - p4;
			a2 = p6 - (p2 >> 1);
			a3 = p2 + (p6 >> 1);

			b0 =  a0 + a3;
			b2 =  a1 - a2;
			b4 =  a1 + a2;
			b6 =  a0 - a3;

			a0 =  p5 - p3 - p7 - (p7 >> 1);    
			a1 =  p1 + p7 - p3 - (p3 >> 1);    
			a2 =  p7 - p1 + p5 + (p5 >> 1);    
			a3 =  p3 + p5 + p1 + (p1 >> 1);


			b1 =  a0 + (a3>>2);    
			b3 =  a1 + (a2>>2);    
			b5 =  a2 - (a1>>2);
			b7 =  a3 - (a0>>2);                

			block[i][0] = b0 + b7;
			block[i][1] = b2 - b5;
			block[i][2] = b4 + b3;
			block[i][3] = b6 + b1;
			block[i][4] = b6 - b1;
			block[i][5] = b4 - b3;
			block[i][6] = b2 + b5;
			block[i][7] = b0 - b7;
		}

		//  Vertical 
		for (i=0; i < BLOCK_SIZE_8x8; i++)
		{
			//    pTmp = tmp + i;
			p0 = block[0][i];
			p1 = block[1][i];
			p2 = block[2][i];
			p3 = block[3][i];
			p4 = block[4][i];
			p5 = block[5][i];
			p6 = block[6][i];
			p7 = block[7][i];

			a0 =  p0 + p4;
			a1 =  p0 - p4;
			a2 =  p6 - (p2>>1);
			a3 =  p2 + (p6>>1);

			b0 = a0 + a3;
			b2 = a1 - a2;
			b4 = a1 + a2;
			b6 = a0 - a3;

			a0 = -p3 + p5 - p7 - (p7 >> 1);
			a1 =  p1 + p7 - p3 - (p3 >> 1);
			a2 = -p1 + p7 + p5 + (p5 >> 1);
			a3 =  p3 + p5 + p1 + (p1 >> 1);


			b1 =  a0 + (a3 >> 2);
			b7 =  a3 - (a0 >> 2);
			b3 =  a1 + (a2 >> 2);
			b5 =  a2 - (a1 >> 2);

			block[0][i] = b0 + b7;
			block[1][i] = b2 - b5;
			block[2][i] = b4 + b3;
			block[3][i] = b6 + b1;
			block[4][i] = b6 - b1;
			block[5][i] = b4 - b3;
			block[6][i] = b2 + b5;
			block[7][i] = b0 - b7;
		}
	
}
#if defined(_DEBUG) || defined(_M_IX64)
void itrans8x8_sse2(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x)
{
		__m128i a0, a1, a2, a3;
		__m128i p0, p1, p2, p3, p4, p5 ,p6, p7;  
		__m128i b0, b1, b2, b3, b4, b5, b6, b7;
		__m128i r0, r1, r2, r3, r4, r5, r6, r7;
		__m128i const32, zero;
		__declspec(align(32)) static const int16_t c32[8] = {32, 32, 32, 32, 32, 32, 32, 32};
		__m128i pred0, pred1;

		const32 = _mm_load_si128((const __m128i *)c32);
		zero = _mm_setzero_si128();

				// Horizontal  
			b0 = _mm_load_si128((__m128i *)(block[0])); 
			b1 = _mm_load_si128((__m128i *)(block[1])); 
			b2 = _mm_load_si128((__m128i *)(block[2])); 
			b3 = _mm_load_si128((__m128i *)(block[3])); 
			b4 = _mm_load_si128((__m128i *)(block[4])); 
			b5 = _mm_load_si128((__m128i *)(block[5])); 
			b6 = _mm_load_si128((__m128i *)(block[6])); 
			b7 = _mm_load_si128((__m128i *)(block[7])); 

			/* rotate 8x8 (ugh) */
			r0 = _mm_unpacklo_epi16(b0, b2); 
			r1 = _mm_unpacklo_epi16(b1, b3); 
			r2 = _mm_unpackhi_epi16(b0, b2); 
			r3 = _mm_unpackhi_epi16(b1, b3); 
			r4 = _mm_unpacklo_epi16(b4, b6); 
			r5 = _mm_unpacklo_epi16(b5, b7); 
			r6 = _mm_unpackhi_epi16(b4, b6); 
			r7 = _mm_unpackhi_epi16(b5, b7); 

			b0 = _mm_unpacklo_epi16(r0, r1); 
			b1 = _mm_unpackhi_epi16(r0, r1); 
			b2 = _mm_unpacklo_epi16(r2, r3); 
			b3 = _mm_unpackhi_epi16(r2, r3); 
			b4 = _mm_unpacklo_epi16(r4, r5); 
			b5 = _mm_unpackhi_epi16(r4, r5); 
			b6 = _mm_unpacklo_epi16(r6, r7); 
			b7 = _mm_unpackhi_epi16(r6, r7); 

			p0 = _mm_unpacklo_epi64(b0, b4);
			p1 = _mm_unpackhi_epi64(b0, b4);
			p2 = _mm_unpacklo_epi64(b1, b5);
			p3 = _mm_unpackhi_epi64(b1, b5);
			p4 = _mm_unpacklo_epi64(b2, b6);
			p5 = _mm_unpackhi_epi64(b2, b6);
			p6 = _mm_unpacklo_epi64(b3, b7);
			p7 = _mm_unpackhi_epi64(b3, b7);

			/* perform approx DCT */
						a0 = _mm_add_epi16(p0, p4); // p0 + p4
			a1 = _mm_sub_epi16(p0, p4); // p0 - p4
			r0 = _mm_srai_epi16(p2, 1); // p2 >> 1
			a2 = _mm_sub_epi16(p6, r0); // p6 - (p2 >> 1)
			r0 = _mm_srai_epi16(p6, 1); // p6 >> 1
			a3 = _mm_add_epi16(p2, r0); //p2 + (p6 >> 1)

			b0 =  _mm_add_epi16(a0, a3); // a0 + a3;
			b2 =  _mm_sub_epi16(a1, a2);  // a1 - a2;
			b4 =  _mm_add_epi16(a1, a2);    // a1 + a2;
			b6 =  _mm_sub_epi16(a0, a3);  // a0 - a3;

			//-p3 + p5 - p7 - (p7 >> 1);    
			r0 = _mm_srai_epi16(p7, 1); // p7 >> 1
			a0 = _mm_sub_epi16(p5, p3); // p5 - p3
			a0 = _mm_sub_epi16(a0, p7); // (-p3 + p5) - p7
			a0 = _mm_sub_epi16(a0, r0); // (-p3 + p5 - p7) - (p7 >> 1)

			//p1 + p7 - p3 - (p3 >> 1);    
			r0 =  _mm_srai_epi16(p3, 1); // (p3 >> 1)
			a1 = _mm_add_epi16(p1, p7); // p1 + p7
			a1 = _mm_sub_epi16(a1, p3); // (p1 + p7) - p3
			a1 = _mm_sub_epi16(a1, r0); // (p1 + p7 - p3) - (p3>>1)

			// -p1 + p7 + p5 + (p5 >> 1);   
			r0 =  _mm_srai_epi16(p5, 1); // (p5 >> 1)
			a2 = _mm_sub_epi16(p7, p1); // p7 - p1
			a2 = _mm_add_epi16(a2, p5); // -p1 + p7 + p5
			a2 = _mm_add_epi16(a2, r0); // (-p1 + p7 + p5) + (p5 >> 1)

			// p3 + p5 + p1 + (p1 >> 1);
			a3 = _mm_add_epi16(p3, p5); // p3+p5
			a3 = _mm_add_epi16(a3, p1); // p3 + p5 + p1
			p1 = _mm_srai_epi16(p1, 1); // p1 >> 1
			a3 = _mm_add_epi16(a3, p1); //p3 + p5 + p1 + (p1 >> 1)

			r0 = _mm_srai_epi16(a3, 2); // a3>>2
			b1 = _mm_add_epi16(a0, r0); //a0 + (a3>>2);    
			r0 = _mm_srai_epi16(a2, 2); // a2>>2
			b3 = _mm_add_epi16(a1, r0); // a1 + (a2>>2);    
			a1 = _mm_srai_epi16(a1, 2); // all done with a1, so this is safe
			b5 = _mm_sub_epi16(a2, a1); //a2 - (a1>>2);
			a0 = _mm_srai_epi16(a0, 2); // all done with a0, so this is safe
			b7 = _mm_sub_epi16(a3, a0); //a3 - (a0>>2);                

			p0 = _mm_add_epi16(b0, b7); // b0 + b7;
			p1 = _mm_sub_epi16(b2, b5); // b2 - b5;
			p2 = _mm_add_epi16(b4, b3); // b4 + b3;
			p3 = _mm_add_epi16(b6, b1); // b6 + b1;
			p4 = _mm_sub_epi16(b6, b1); // b6 - b1;
			p5 = _mm_sub_epi16(b4, b3); // b4 - b3;
			p6 = _mm_add_epi16(b2, b5); // b2 + b5;
			p7 = _mm_sub_epi16(b0, b7); // b0 - b7;

						/* rotate 8x8 (ugh) */
			r0 = _mm_unpacklo_epi16(p0, p2); 
			r1 = _mm_unpacklo_epi16(p1, p3); 
			r2 = _mm_unpackhi_epi16(p0, p2); 
			r3 = _mm_unpackhi_epi16(p1, p3); 
			r4 = _mm_unpacklo_epi16(p4, p6); 
			r5 = _mm_unpacklo_epi16(p5, p7); 
			r6 = _mm_unpackhi_epi16(p4, p6); 
			r7 = _mm_unpackhi_epi16(p5, p7); 

			b0 = _mm_unpacklo_epi16(r0, r1); 
			b1 = _mm_unpackhi_epi16(r0, r1); 
			b2 = _mm_unpacklo_epi16(r2, r3); 
			b3 = _mm_unpackhi_epi16(r2, r3); 
			b4 = _mm_unpacklo_epi16(r4, r5); 
			b5 = _mm_unpackhi_epi16(r4, r5); 
			b6 = _mm_unpacklo_epi16(r6, r7); 
			b7 = _mm_unpackhi_epi16(r6, r7); 

			p0 = _mm_unpacklo_epi64(b0, b4);
			p1 = _mm_unpackhi_epi64(b0, b4);
			p2 = _mm_unpacklo_epi64(b1, b5);
			p3 = _mm_unpackhi_epi64(b1, b5);
			p4 = _mm_unpacklo_epi64(b2, b6);
			p5 = _mm_unpackhi_epi64(b2, b6);
			p6 = _mm_unpacklo_epi64(b3, b7);
			p7 = _mm_unpackhi_epi64(b3, b7);


		/*  Vertical  */

			a0 = _mm_add_epi16(p0, p4); // p0 + p4
			a1 = _mm_sub_epi16(p0, p4); // p0 - p4
			r0 = _mm_srai_epi16(p2, 1); // p2 >> 1
			a2 = _mm_sub_epi16(p6, r0); // p6 - (p2 >> 1)
			r0 = _mm_srai_epi16(p6, 1); // p6 >> 1
			a3 = _mm_add_epi16(p2, r0); //p2 + (p6 >> 1)

			b0 =  _mm_add_epi16(a0, a3); // a0 + a3;
			b2 =  _mm_sub_epi16(a1, a2);  // a1 - a2;
			b4 =  _mm_add_epi16(a1, a2);    // a1 + a2;
			b6 =  _mm_sub_epi16(a0, a3);  // a0 - a3;

			//-p3 + p5 - p7 - (p7 >> 1);    
			r0 = _mm_srai_epi16(p7, 1); // p7 >> 1
			a0 = _mm_sub_epi16(p5, p3); // p5 - p3
			a0 = _mm_sub_epi16(a0, p7); // (-p3 + p5) - p7
			a0 = _mm_sub_epi16(a0, r0); // (-p3 + p5 - p7) - (p7 >> 1)

			//p1 + p7 - p3 - (p3 >> 1);    
			r0 =  _mm_srai_epi16(p3, 1); // (p3 >> 1)
			a1 = _mm_add_epi16(p1, p7); // p1 + p7
			a1 = _mm_sub_epi16(a1, p3); // (p1 + p7) - p3
			a1 = _mm_sub_epi16(a1, r0); // (p1 + p7 - p3) - (p3>>1)

			// -p1 + p7 + p5 + (p5 >> 1);   
			r0 =  _mm_srai_epi16(p5, 1); // (p5 >> 1)
			a2 = _mm_sub_epi16(p7, p1); // p7 - p1
			a2 = _mm_add_epi16(a2, p5); // -p1 + p7 + p5
			a2 = _mm_add_epi16(a2, r0); // (-p1 + p7 + p5) + (p5 >> 1)

			// p3 + p5 + p1 + (p1 >> 1);
			r0 = _mm_srai_epi16(p1, 1); // p1 >> 1
			a3 = _mm_add_epi16(p3, p5); // p3+p5
			a3 = _mm_add_epi16(a3, p1); // p3 + p5 + p1
			a3 = _mm_add_epi16(a3, r0); //p3 + p5 + p1 + (p1 >> 1)

			r0 = _mm_srai_epi16(a3, 2); // a3>>2
			b1 = _mm_add_epi16(a0, r0); //a0 + (a3>>2);    
			r0 = _mm_srai_epi16(a2, 2); // a2>>2
			b3 = _mm_add_epi16(a1, r0); // a1 + (a2>>2);    
			a1 = _mm_srai_epi16(a1, 2); // all done with a1, so this is safe
			b5 = _mm_sub_epi16(a2, a1); //a2 - (a1>>2);
			a0 = _mm_srai_epi16(a0, 2); // all done with a0, so this is safe
			b7 = _mm_sub_epi16(a3, a0); //a3 - (a0>>2);                

			r0 = _mm_add_epi16(b0, b7); // b0 + b7;
			r1 = _mm_sub_epi16(b2, b5); // b2 - b5;
			r2 = _mm_add_epi16(b4, b3); // b4 + b3;
			r3 = _mm_add_epi16(b6, b1); // b6 + b1;
			r4 = _mm_sub_epi16(b6, b1); // b6 - b1;
			r5 = _mm_sub_epi16(b4, b3); // b4 - b3;
			r6 = _mm_add_epi16(b2, b5); // b2 + b5;
			r7 = _mm_sub_epi16(b0, b7); // b0 - b7;


			// add in prediction values
			pred0 = _mm_loadl_epi64((__m128i *)(&mb_pred[0][pos_x]));
			pred1 = _mm_loadl_epi64((__m128i *)(&mb_pred[1][pos_x]));
			// (x + 32) >> 6
			r0 = _mm_adds_epi16(r0, const32);
			r0 = _mm_srai_epi16(r0, 6);
			r1 = _mm_adds_epi16(r1, const32);
			r1 = _mm_srai_epi16(r1, 6);
			pred0 = _mm_unpacklo_epi8(pred0, zero); // convert to short
			pred1 = _mm_unpacklo_epi8(pred1, zero); // convert to short
			pred0 = _mm_adds_epi16(pred0, r0);
			pred1 = _mm_adds_epi16(pred1, r1);

			pred0 = _mm_packus_epi16(pred0, pred1); // convert to unsigned char

			// store
			_mm_storel_epi64((__m128i *)(&mb_rec[0][pos_x]), pred0);
			// TODO: if mb_pred was converted to 4 8x8 blocks, we could store more easily.
			pred0 = _mm_srli_si128(pred0, 8);
			_mm_storel_epi64((__m128i *)(&mb_rec[1][pos_x]), pred0);

			/* --- */

			pred0 = _mm_loadl_epi64((__m128i *)(&mb_pred[2][pos_x]));
			pred1 = _mm_loadl_epi64((__m128i *)(&mb_pred[3][pos_x]));
			// (x + 32) >> 6
			r2 = _mm_adds_epi16(r2, const32);
			r2 = _mm_srai_epi16(r2, 6);
			r3 = _mm_adds_epi16(r3, const32);
			r3 = _mm_srai_epi16(r3, 6);
			pred0 = _mm_unpacklo_epi8(pred0, zero); // convert to short
			pred1 = _mm_unpacklo_epi8(pred1, zero); // convert to short
			pred0 = _mm_adds_epi16(pred0, r2);
			pred1 = _mm_adds_epi16(pred1, r3);

			pred0 = _mm_packus_epi16(pred0, pred1); // convert to unsigned char

			// store
			_mm_storel_epi64((__m128i *)(&mb_rec[2][pos_x]), pred0);
			// TODO: if mb_pred was converted to 4 8x8 blocks, we could store more easily.
			pred0 = _mm_srli_si128(pred0, 8);
			_mm_storel_epi64((__m128i *)(&mb_rec[3][pos_x]), pred0);

			/* --- */

			pred0 = _mm_loadl_epi64((__m128i *)(&mb_pred[4][pos_x]));
			pred1 = _mm_loadl_epi64((__m128i *)(&mb_pred[5][pos_x]));
			// (x + 32) >> 6
			r4 = _mm_adds_epi16(r4, const32);
			r4 = _mm_srai_epi16(r4, 6);
			r5 = _mm_adds_epi16(r5, const32);
			r5 = _mm_srai_epi16(r5, 6);
			pred0 = _mm_unpacklo_epi8(pred0, zero); // convert to short
			pred1 = _mm_unpacklo_epi8(pred1, zero); // convert to short
			pred0 = _mm_adds_epi16(pred0, r4);
			pred1 = _mm_adds_epi16(pred1, r5);

			pred0 = _mm_packus_epi16(pred0, pred1); // convert to unsigned char

			// store
			_mm_storel_epi64((__m128i *)(&mb_rec[4][pos_x]), pred0);
			// TODO: if mb_pred was converted to 4 8x8 blocks, we could store more easily.
			pred0 = _mm_srli_si128(pred0, 8);
			_mm_storel_epi64((__m128i *)(&mb_rec[5][pos_x]), pred0);

			/* --- */

			pred0 = _mm_loadl_epi64((__m128i *)(&mb_pred[6][pos_x]));
			pred1 = _mm_loadl_epi64((__m128i *)(&mb_pred[7][pos_x]));
			// (x + 32) >> 6
			r6 = _mm_adds_epi16(r6, const32);
			r6 = _mm_srai_epi16(r6, 6);
			r7 = _mm_adds_epi16(r7, const32);
			r7 = _mm_srai_epi16(r7, 6);
			pred0 = _mm_unpacklo_epi8(pred0, zero); // convert to short
			pred1 = _mm_unpacklo_epi8(pred1, zero); // convert to short
			pred0 = _mm_adds_epi16(pred0, r6);
			pred1 = _mm_adds_epi16(pred1, r7);

			pred0 = _mm_packus_epi16(pred0, pred1); // convert to unsigned char

			// store
			_mm_storel_epi64((__m128i *)&mb_rec[6][pos_x], pred0);
			// TODO: if mb_pred was converted to 4 8x8 blocks, we could store more easily.
			pred0 = _mm_srli_si128(pred0, 8);
			_mm_storel_epi64((__m128i *)&mb_rec[7][pos_x], pred0);
}

#endif

#ifdef _M_IX86
// TODO!! fix for 16bit coefficients instead of 32
static void sample_reconstruct8x8_mmx(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *mb_rres8, int pos_x)
{
			__asm
		{
			mov esi, 8 // loop 8 times

			mov eax, mb_rec
			add eax, pos_x

			mov ebx, mb_pred
			add ebx, pos_x

			mov ecx, mb_rres8

			// mm0 : constant value 32
			mov edx, 0x00200020
			movd mm0, edx
			punpckldq	mm0, mm0
			// mm5:  zero
			pxor mm7, mm7

loop8:

			movq	mm1, MMWORD PTR 0[ecx]
			paddw mm1, mm0 // rres + 32
			psraw mm1, 6 // (rres + 32) >> 6
			movq mm2, MMWORD PTR 0[ebx]
			punpcklbw mm2, mm7			// convert pred_row from unsigned char to short
			paddsw mm2, mm1 // pred_row + rres_row
			packuswb mm2, mm7
			movq MMWORD PTR 0[eax], mm2


			add eax, 16
			add ebx, 16
			add ecx, 16

			sub	esi, 1
			jne	loop8
			emms
		}
}
#endif

// benski> unused, left in place for unit testing and if we ever need to port the decoder to non-intel
static void sample_reconstruct8x8(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *mb_rres8, int pos_x, int max_imgpel_value)
{
	int i,j;
	for( j = 0; j <  8; j++)
	{
		imgpel *rec_row = mb_rec[j] + pos_x;
		const short *rres_row = mb_rres8[j];
		const imgpel *pred_row = mb_pred[j] + pos_x;

		for( i = 0; i < 8; i++)
			rec_row[i] = (imgpel) iClip1(max_imgpel_value, pred_row[i] + rshift_rnd_sf(rres_row[i], DQ_BITS_8)); 
	}
}
/*!
***********************************************************************
* \brief
*    Inverse 8x8 transformation
***********************************************************************
*/ 
#ifdef _M_IX86
void itrans8x8_mmx(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x)
{
	inverse8x8((h264_short_8x8block_row_t *)block);
	sample_reconstruct8x8_mmx(mb_rec, mb_pred, block, pos_x);
}
#endif

void itrans8x8_c(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x)
{
	inverse8x8((h264_short_8x8block_row_t *)block);
	sample_reconstruct8x8(mb_rec, mb_pred, block, pos_x, 255);
}

void itrans8x8_lossless(h264_imgpel_macroblock_row_t *mb_rec, const h264_imgpel_macroblock_row_t *mb_pred, const h264_short_8x8block_row_t *block, int pos_x)
{
	int i,j;

	for( j = 0; j <  8; j++)
	{
		imgpel *rec_row = mb_rec[j] + pos_x;
		const short *rres_row = block[j];
		const imgpel *pred_row = mb_pred[j] + pos_x;
		for( i = 0; i <  8; i++)
			rec_row[i] = (imgpel) iClip1(255, (rres_row[i] + (long)pred_row[i])); 
	}
}