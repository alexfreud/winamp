#include "Decoder.h"
#include "indices.h"
#include "vlc_table.h"

static unsigned char zig_zag_scan[64]={
  0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
  12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
  35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
  58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
};

#define ESCAPE                          7167
// mode 1 = Inter (or Intra in advanced intra coding mode)
// mode 0 = Intra
void Decoder::getblock(int comp, int mode)
{
	int val, i, j, sign;
	unsigned int code;
	VLCtab *tab;
	short *bp;
	int run, last, level, QP;
	short *qval;


	/* TODO: benski>
	i think this whole function can get replaced with
	ippiDecodeCoeffsIntra_H263_1u16s (mode==0)
	or 
	ippiDecodeCoeffsInter_H263_1u16s (mode == 1)
	with pCoef = bp
	modQuantFlag = escapemode>=1
	scan = IPPVC_SCAN_ZIGZAG

	*/
	bp = block[comp];

	/* decode AC coefficients (or all coefficients in advanced intra coding
	* mode) */

	for (i = (mode == 0);; i++)
	{

		code = buffer.showbits(12);
		

			if (code >= 512)
				tab = &DCT3Dtab0[(code >> 5) - 16];
			else if (code >= 128)
				tab = &DCT3Dtab1[(code >> 2) - 32];
			else if (code >= 8)
				tab = &DCT3Dtab2[(code >> 0) - 8];
			else
			{
				fault = 1;
				return;
			}

			run = (tab->val >> 4) & 255;
			level = tab->val & 15;
			last = (tab->val >> 12) & 1;
		
		buffer.flushbits(tab->len);
		if (tab->val == ESCAPE)
		{
			/* escape */
			if (escapemode >= 1)
			{
				int is11 = buffer.getbits1();
				sign=0;
				last = buffer.getbits1();
				i+=run = buffer.getbits(6);
				if (is11)
				{
					level = buffer.getbits(11);
					if ((sign = (level>=1024)))
						val = 2048 - level;
					else
						val = level;
				}
				else
				{
					level = buffer.getbits(7);
					if ((sign = (level>=64)))
						val = 128 - level;
					else
						val = level;
				}
			}
			else
			{
				last = buffer.getbits1();
				i += run = buffer.getbits(6);
				level = buffer.getbits(8);

				if ((sign = (level >= 128)))
					val = 256 - level;
				else
					val = level;
			}
		}
		else
		{
			i += run;
			val = level;
			sign = buffer.getbits(1);
		}


		if (i >= 64)
		{
			fault = 1;
			return;
		}

		/* Descan in the proper order in advanced intra coding mode */


		j = zig_zag_scan[i];
		qval = &bp[j];
			QP = quant;



			/* TODO: benski>
			ippiQuantInvIntra_H263_16s_C1I
			or
			ippiQuantInvInter_H263_16s_C1I (mode == 1)
			but outside the loop
			pSrcDst = bp
			QP = quant
			modQuantFlag = escapemode >= 1
			*/
		/* TMN3 dequantization */
		if ((QP % 2) == 1)
			*qval = (sign ? -(QP * (2 * val + 1)) : QP * (2 * val + 1));
		else
			*qval = (sign ? -(QP * (2 * val + 1) - 1) : QP * (2 * val + 1) - 1);

		if (last)
		{
			/* That's it */
			return;
		}
	}}
