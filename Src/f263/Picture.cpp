#include "Decoder.h"
#define sign(a)         ((a) < 0 ? -1 : 1)
/* private prototypes*/
static int motion_decode(int vec,int pmv);
static void make_edge_image(const unsigned char *src, unsigned char *dst, int width, int height, int edge);

/* decode one frame or field picture */

void Decoder::getpicture(Frame decodedFrame)
{
	int i;
	unsigned char *tmp;

	for (i=0; i<3; i++)
	{
		tmp = oldrefframe[i];
		oldrefframe[i] = refframe[i];
		refframe[i] = tmp;
		newframe[i] = refframe[i];
	}

	if (!firstFrame)
	{
		make_edge_image(oldrefframe[0],edgeframe[0],coded_picture_width,
			coded_picture_height,32);
		make_edge_image(oldrefframe[1],edgeframe[1],chrom_width, chrom_height,16);
		make_edge_image(oldrefframe[2],edgeframe[2],chrom_width, chrom_height,16);
	}

	//getMBs();
	get_I_P_MBs();

	if (deblock)
		edge_filter(newframe[0], newframe[1], newframe[2],
		coded_picture_width, coded_picture_height);

	/*
	PostFilter(newframe[0], newframe[1], newframe[2],
	coded_picture_width, coded_picture_height);
	*/

	decodedFrame[0] = newframe[0];
	decodedFrame[1] = newframe[1];
	decodedFrame[2] = newframe[2];

	firstFrame=false;
}


/* decode all macroblocks of the current picture */


void Decoder::clearblock(int comp)
{
	int *bp;
	int i;

	bp = (int *)block[comp];

	for (i=0; i<8; i++)
	{
		bp[0] = bp[1] = bp[2] = bp[3] = 0;
		bp += 4;
	}
}


/* move/add 8x8-Block from block[comp] to refframe or bframe */

void Decoder::addblock(int comp, int bx, int by, int addflag)
{
	int cc,i, iincr;
	unsigned char *rfp;
	short *bp;

	bp = block[comp];

	/* TODO: benski>
ippiCopy8x8_8u_C1R (addflag = 0)
ippiAdd8x8_16s8u_C1IRS (addflag = 1)
	*/
	cc = (comp<4) ? 0 : (comp&1)+1; /* color component index */

	if (cc==0)
	{
		/* luminance */

		/* frame DCT coding */
		rfp = newframe[0]
		+ coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);

		iincr = coded_picture_width;
	}
	else
	{
		/* chrominance */

		/* scale coordinates */
		bx >>= 1;
		by >>= 1;
		/* frame DCT coding */
		rfp = newframe[cc] + chrom_width*by + bx;
		iincr = chrom_width;
	}


	if (addflag)
	{
		for (i=0; i<8; i++)
		{
			rfp[0] = clp[bp[0]+rfp[0]];
			rfp[1] = clp[bp[1]+rfp[1]];
			rfp[2] = clp[bp[2]+rfp[2]];
			rfp[3] = clp[bp[3]+rfp[3]];
			rfp[4] = clp[bp[4]+rfp[4]];
			rfp[5] = clp[bp[5]+rfp[5]];
			rfp[6] = clp[bp[6]+rfp[6]];
			rfp[7] = clp[bp[7]+rfp[7]];
			bp += 8;
			rfp+= iincr;
		}
	}
	else
	{
		for (i=0; i<8; i++)
		{
			rfp[0] = clp[bp[0]];
			rfp[1] = clp[bp[1]];
			rfp[2] = clp[bp[2]];
			rfp[3] = clp[bp[3]];
			rfp[4] = clp[bp[4]];
			rfp[5] = clp[bp[5]];
			rfp[6] = clp[bp[6]];
			rfp[7] = clp[bp[7]];
			bp += 8;
			rfp += iincr;
		}
	}
}


int motion_decode(int vec, int pmv)
{
	if (vec > 31) vec -= 64;
	vec += pmv;

	if (vec > 31)
		vec -= 64;
	if (vec < -32)
		vec += 64;

	return vec;
}


int Decoder::find_pmv(int x, int y, int block, int comp)

{
	int p1,p2,p3;
	int xin1,xin2,xin3;
	int yin1,yin2,yin3;
	int vec1,vec2,vec3;
	int l8,o8,or8;

	x++;y++;

	l8 = (modemap[y][x-1] == MODE_INTER4V ? 1 : 0);
	o8 = (modemap[y-1][x] == MODE_INTER4V ? 1 : 0);
	or8 = (modemap[y-1][x+1] == MODE_INTER4V ? 1 : 0);

	switch (block)
	{
	case 0:
		vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
		vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
		vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
		break;
	case 1:
		vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
		vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
		vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
		break;
	case 2:
		vec1 = 1            ; yin1 = y  ; xin1 = x;
		vec2 = (o8 ? 4 : 0) ; yin2 = y-1; xin2 = x;
		vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
		break;
	case 3:
		vec1 = (l8 ? 4 : 0) ; yin1 = y  ; xin1 = x-1;
		vec2 = 1            ; yin2 = y  ; xin2 = x;
		vec3 = 2            ; yin3 = y  ; xin3 = x;
		break;
	case 4:
		vec1 = 3            ; yin1 = y  ; xin1 = x;
		vec2 = 1            ; yin2 = y  ; xin2 = x;
		vec3 = 2            ; yin3 = y  ; xin3 = x;
		break;
	default:
		exit(1);
		break;
	}
	p1 = MV[comp][vec1][yin1][xin1];
	p2 = MV[comp][vec2][yin2][xin2];
	p3 = MV[comp][vec3][yin3][xin3];

	if (p2 == NO_VEC)
	{
		p2 = p3 = p1;
	}

	return p1+p2+p3 - max(p1,max(p2,p3)) - min(p1,min(p2,p3));
}


void make_edge_image(const unsigned char *src,unsigned char *dst,int width,int height,int edge)
{
	int i,j;
	unsigned char *p1,*p2,*p3,*p4;
	const unsigned char *o1,*o2,*o3,*o4;

	/* center image */
	p1 = dst;
	o1 = src;
	for (j = 0; j < height;j++)
	{
		for (i = 0; i < width; i++)
		{
			*(p1 + i) = *(o1 + i);
		}
		p1 += width + (edge<<1);
		o1 += width;
	}

	/* left and right edges */
	p1 = dst-1;
	o1 = src;
	for (j = 0; j < height;j++)
	{
		for (i = 0; i < edge; i++)
		{
			*(p1 - i) = *o1;
			*(p1 + width + i + 1) = *(o1 + width - 1);
		}
		p1 += width + (edge<<1);
		o1 += width;
	}

	/* top and bottom edges */
	p1 = dst;
	p2 = dst + (width + (edge<<1))*(height-1);
	o1 = src;
	o2 = src + width*(height-1);
	for (j = 0; j < edge;j++)
	{
		p1 = p1 - (width + (edge<<1));
		p2 = p2 + (width + (edge<<1));
		for (i = 0; i < width; i++)
		{
			*(p1 + i) = *(o1 + i);
			*(p2 + i) = *(o2 + i);
		}
	}

	/* corners */
	p1 = dst - (width+(edge<<1)) - 1;
	p2 = p1 + width + 1;
	p3 = dst + (width+(edge<<1))*(height)-1;
	p4 = p3 + width + 1;

	o1 = src;
	o2 = o1 + width - 1;
	o3 = src + width*(height-1);
	o4 = o3 + width - 1;
	for (j = 0; j < edge; j++)
	{
		for (i = 0; i < edge; i++)
		{
			*(p1 - i) = *o1;
			*(p2 + i) = *o2;
			*(p3 - i) = *o3;
			*(p4 + i) = *o4;
		}
		p1 = p1 - (width + (edge<<1));
		p2 = p2 - (width + (edge<<1));
		p3 = p3 + width + (edge<<1);
		p4 = p4 + width + (edge<<1);
	}

}

static bool Mode_IsInter(int Mode)
{
return (Mode == MODE_INTER || Mode == MODE_INTER_Q ||
				Mode == MODE_INTER4V || Mode == MODE_INTER4V_Q);
}


static bool Mode_IsIntra(int Mode)
{
				return (Mode == MODE_INTRA || Mode == MODE_INTRA_Q);
}

void Decoder::get_I_P_MBs()
{
	int comp;
	int MBA, MBAmax;

	int COD = 0, MCBPC, CBPY, CBP = 0, CBPB = 0, MODB = 0, Mode = 0, DQUANT;
	int mvx = 0, mvy = 0,  pmv0, pmv1, xpos, ypos, k;
	int startmv, stopmv,   last_done = 0, pCBP = 0, pCBPB = 0, pCOD = 0, pMODB = 0;
	int DQ_tab[4] = {-1, -2, 1, 2};
	unsigned int i;
	short *bp;

	/* number of macroblocks per picture */
	MBAmax = mb_width * mb_height;

	MBA = 0;                      /* macroblock address */
	xpos = ypos = 0;

	/* mark MV's above the picture */
	for (i = 1; i < mb_width + 1; i++)
	{
		for (k = 0; k < 5; k++)
		{
			MV[0][k][0][i] = NO_VEC;
			MV[1][k][0][i] = NO_VEC;
		}
		modemap[0][i] = MODE_INTRA;
	}
	/* zero MV's on the sides of the picture */
	for (i = 0; i < mb_height + 1; i++)
	{
		for (k = 0; k < 5; k++)
		{
			MV[0][k][i][0] = 0;
			MV[1][k][i][0] = 0;
			MV[0][k][i][mb_width + 1] = 0;
			MV[1][k][i][mb_width + 1] = 0;
		}
		modemap[i][0] = MODE_INTRA;
		modemap[i][mb_width + 1] = MODE_INTRA;
	}
	/* initialize the qcoeff used in advanced intra coding */

	fault = 0;

	for (;;)
	{


resync:
		/* This version of the decoder does not resync on every possible
		 * error, and it does not do all possible error checks. It is not
		 * difficult to make it much more error robust, but I do not think it
		 * is necessary to include this in the freely available version. */

		if (fault)
		{
			startcode();              /* sync on new startcode */
			fault = 0;
		}


		xpos = MBA % mb_width;
		ypos = MBA / mb_width;

		if (MBA >= MBAmax)
		{
			/* all macroblocks decoded */
			return;
		}
read_cod:


		if (PCT_INTER == pict_type || PCT_DISPOSABLE_INTER == pict_type)
		{
			COD = buffer.showbits(1);
		}
		else
		{
			COD = 0;                /* Intra picture -> not skipped */
			coded_map[ypos + 1][xpos + 1] = 1;
		}


		if (!COD)
		{
			/* COD == 0 --> not skipped */

			if (PCT_INTER == pict_type || PCT_DISPOSABLE_INTER == pict_type)
			{
				/* flush COD bit */
				buffer.flushbits(1);
			}
			if (PCT_INTRA == pict_type)
			{
				MCBPC = getMCBPCintra();
			}
			else
			{
				MCBPC = getMCBPC();
			}


			if (fault)
				goto resync;

			if (MCBPC == 255)
			{
				/* stuffing - read next COD without advancing MB count. */
				goto read_cod;
			}
			else
			{
				/* normal MB data */
				Mode = MCBPC & 7;


				/* MODB and CBPB */


				CBPY = getCBPY();

			}

			/* Decode Mode and CBP */
			if ((Mode == MODE_INTRA || Mode == MODE_INTRA_Q))
			{
				/* Intra */
				coded_map[ypos + 1][xpos + 1] = 1;

				CBPY = CBPY ^ 15;   /* needed in huffman coding only */
			}

			CBP = (CBPY << 2) | (MCBPC >> 4);

			if (Mode == MODE_INTER_Q || Mode == MODE_INTRA_Q || Mode == MODE_INTER4V_Q)
			{
				/* Read DQUANT if necessary */

				DQUANT = buffer.getbits(2);
				quant += DQ_tab[DQUANT];


				if (quant > 31 || quant < 1)
				{
					quant = max(1, (31, quant));
					/* could set fault-flag and resync here */
					fault = 1;
				}
			}

			/* motion vectors */
			if (Mode == MODE_INTER || Mode == MODE_INTER_Q ||
			    Mode == MODE_INTER4V || Mode == MODE_INTER4V_Q)
			{
				if (Mode == MODE_INTER4V || Mode == MODE_INTER4V_Q)
				{
					startmv = 1;
					stopmv = 4;
				}
				else
				{
					startmv = 0;
					stopmv = 0;
				}

				for (k = startmv; k <= stopmv; k++)
				{
					mvx = getTMNMV();
					mvy = getTMNMV();

					pmv0 = find_pmv(xpos, ypos, k, 0);
					pmv1 = find_pmv(xpos, ypos, k, 1);

					mvx = motion_decode(mvx, pmv0);
					mvy = motion_decode(mvy, pmv1);

					/* store coded or not-coded */
					coded_map[ypos + 1][xpos + 1] = 1;
					MV[0][k][ypos+1][xpos+1] = mvx;
					MV[1][k][ypos+1][xpos+1] = mvy;
				}

			}
			/* Intra. */
			else
			{
			}
			if (fault)
				goto resync;
		}
		else
		{
			/* COD == 1 --> skipped MB */
			if (MBA >= MBAmax)
			{
				/* all macroblocks decoded */
				return;
			}

			if (PCT_INTER == pict_type || PCT_DISPOSABLE_INTER == pict_type)
				buffer.flushbits(1);

			Mode = MODE_INTER;
			/* Reset CBP */
			CBP = CBPB = 0;
			coded_map[ypos + 1][xpos + 1] = 0;

			/* reset motion vectors */
			MV[0][0][ypos + 1][xpos + 1] = 0;
			MV[1][0][ypos + 1][xpos + 1] = 0;
		}

		/* Store mode and prediction type */
		modemap[ypos + 1][xpos + 1] = Mode;

		/* store defaults for advanced intra coding mode */

		if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
		{
			MV[0][0][ypos + 1][xpos + 1] = MV[1][0][ypos + 1][xpos + 1] = 0;
		}



		
		if (!COD)
		{
			Mode = modemap[ypos + 1][xpos + 1];

			/* decode blocks */
			for (comp = 0; comp < 6; comp++)
			{
				clearblock(comp);
				if ((Mode == MODE_INTRA || Mode == MODE_INTRA_Q))
				{
					/* Intra (except in advanced intra coding mode) */
					bp = block[comp];


					bp[0] = buffer.getbits(8);

					if (bp[0] == 255)   /* Spec. in H.26P, not in TMN4 */
						bp[0] = 128;
					bp[0] *= 8;         /* Iquant */
					if ((CBP & (1 << (6 - 1 - comp))))
					{
						getblock(comp, 0);
					}
				}
				else
				{
					/* Inter (or Intra in advanced intra coding mode) */
					if ((CBP & (1 << (6 - 1 - comp))))
					{
						getblock(comp, 1);
					}
				}

				if (fault)
					goto resync;
			}

		}

		/* decode the last MB if data is missing */

		/* advance to next macroblock */
		MBA++;
		pCBP = CBP;
		pCBPB = CBPB;
		pCOD = COD;
		pMODB = MODB;
		quant_map[ypos + 1][xpos + 1] = quant;

					int bx = 16 * xpos;
			int by = 16 * ypos;

			Mode = modemap[by / 16 + 1][bx / 16 + 1];

			/* motion compensation for P-frame */
			if (Mode == MODE_INTER || Mode == MODE_INTER_Q ||
			    Mode == MODE_INTER4V || Mode == MODE_INTER4V_Q)
			{
				reconstruct(bx, by, Mode);
			}

			/* copy or add block data into P-picture */
			for (comp = 0; comp < 6; comp++)
			{
				/* inverse DCT */
				if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
				{
					idct.idct(block[comp]);
					addblock(comp, bx, by, 0);
				}
				else if ((pCBP & (1 << (6 - 1 - comp))))
				{
					/* No need to to do this for blocks with no coeffs */
					idct.idct(block[comp]);
					addblock(comp, bx, by, 1);
				}
			}
	}
}

static int STRENGTH[] = {1, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 12};
void Decoder::horiz_edge_filter(unsigned char *rec, int width, int height, int chr)
{
	int i, j;
	int delta, d1, d2;
	int mbc, mbr, do_filter;
	int QP;
	int mbr_above;


	/* horizontal edges */
	for (j = 8; j < height; j += 8)
	{
		if (!chr)
		{
			mbr = j >> 4;
			mbr_above = (j - 8) >> 4;
		}
		else
		{
			mbr = j >> 3;
			mbr_above = mbr - 1;
		}
		const int * const cur_coded_map = coded_map[mbr + 1];
		for (i = 0; i < width; i++)
		{
			// TODO: replace all below with FilterDeblocking8x8HorEdge_H263(rec+i+(j+1)*width, width, QP) and i+=8  ?
			if (!chr)
			{
				mbc = i >> 4;
			}
			else
			{
				mbc = i >> 3;
			}

			do_filter = cur_coded_map[mbc + 1] || coded_map[mbr_above + 1][mbc + 1];
			if (do_filter)
			{
				QP = cur_coded_map[mbc + 1] ? quant_map[mbr + 1][mbc + 1] : quant_map[mbr_above + 1][mbc + 1];


				delta = (int)(((int)(*(rec + i + (j - 2) * width)) +
					(int)(*(rec + i + (j - 1) * width) * (-4)) +
					(int)(*(rec + i + (j) * width) * (4)) +
					(int)(*(rec + i + (j + 1) * width) * (-1))) / 8.0);

				d1 = sign(delta) * max(0, abs(delta) - max(0, 2 * (abs(delta) - STRENGTH[QP - 1])));

				d2 = min(abs(d1 / 2), max(-abs(d1 / 2), (int)(((*(rec + i + (j - 2) * width) -
					*(rec + i + (j + 1) * width))) / 4)));

				*(rec + i + (j + 1) * width) += d2; /* D */
				*(rec + i + (j) * width) = min(255, max(0, (int)(*(rec + i + (j) * width)) - d1));    /* C */
				*(rec + i + (j - 1) * width) = min(255, max(0, (int)(*(rec + i + (j - 1) * width)) + d1));    /* B */
				*(rec + i + (j - 2) * width) -= d2; /* A */
			}
		}
	}
	return;
}

void Decoder::vert_edge_filter(unsigned char *rec, int width, int height, int chr)
{
	int i, j;
	int delta, d1, d2;
	int mbc, mbr;
	int do_filter;
	int QP;
	int mbc_left;


	/* vertical edges */
	for (i = 8; i < width; i += 8)
	{
		if (!chr)
		{
			mbc = i >> 4;
			mbc_left = (i - 8) >> 4;
		}
		else
		{
			mbc = i >> 3;
			mbc_left = mbc - 1;
		}
		// TODO: replace all below with  FilterDeblocking8x8VerEdge_H263(rec+i +j*width, width, QP) and i+=8  ?
		for (j = 0; j < height; j++)
		{
			if (!chr)
			{
				mbr = j >> 4;
			}
			else
			{
				mbr = j >> 3;

			}
			do_filter = coded_map[mbr + 1][mbc + 1] || coded_map[mbr + 1][mbc_left + 1];

			if (do_filter)
			{

				QP = coded_map[mbr + 1][mbc + 1] ?
					quant_map[mbr + 1][mbc + 1] : quant_map[mbr + 1][mbc_left + 1];

				delta = (int)(((int)(*(rec + i - 2 + j * width)) +
					(int)(*(rec + i - 1 + j * width) * (-4)) +
					(int)(*(rec + i + j * width) * (4)) +
					(int)(*(rec + i + 1 + j * width) * (-1))) / 8.0);

				d1 = sign(delta) * max(0, abs(delta) -
					max(0, 2 * (abs(delta) - STRENGTH[QP - 1])));

				d2 = min(abs(d1 / 2), max(-abs(d1 / 2),
					(int)((*(rec + i - 2 + j * width) -
					*(rec + i + 1 + j * width)) / 4)));

				*(rec + i + 1 + j * width) += d2; /* D */
				*(rec + i + j * width) = min(255, max(0, (int)(*(rec + i + j * width)) - d1));    /* C */
				*(rec + i - 1 + j * width) = min(255, max(0, (int)(*(rec + i - 1 + j * width)) + d1));    /* B */
				*(rec + i - 2 + j * width) -= d2; /* A */
			}
		}
	}
	return;
}

void Decoder::edge_filter(unsigned char *lum, unsigned char *Cb, unsigned char *Cr, int width, int height)
{

	/* Luma */
	horiz_edge_filter(lum, width, height, 0);
	vert_edge_filter(lum, width, height, 0);

	/* Chroma */
	horiz_edge_filter(Cb, width / 2, height / 2, 1);
	vert_edge_filter(Cb, width / 2, height / 2, 1);
	horiz_edge_filter(Cr, width / 2, height / 2, 1);
	vert_edge_filter(Cr, width / 2, height / 2, 1);

	/* that's it */
	return;
}



void Decoder::PostFilter(unsigned char *lum, unsigned char *Cb, unsigned char *Cr,
												 int width, int height)
{

	/* Luma */
	horiz_post_filter(lum, width, height, 0);
	vert_post_filter(lum, width, height, 0);

	/* Chroma */
	horiz_post_filter(Cb, width / 2, height / 2, 1);
	vert_post_filter(Cb, width / 2, height / 2, 1);
	horiz_post_filter(Cr, width / 2, height / 2, 1);
	vert_post_filter(Cr, width / 2, height / 2, 1);

	/* that's it */
	return;
}


/***********************************************************************/

static int STRENGTH1[] = {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4};
void Decoder::horiz_post_filter(unsigned char *rec, int width, int height, int chr)
{
	int i, j;
	int delta, d1;
	int mbc, mbr;
	int QP;
	int mbr_above;


	/* horizontal edges */
	for (j = 8; j < height; j += 8)
	{
		for (i = 0; i < width; i++)
		{
			if (!chr)
			{
				mbr = j >> 4;
				mbc = i >> 4;
				mbr_above = (j - 8) >> 4;
			}
			else
			{
				mbr = j >> 3;
				mbc = i >> 3;
				mbr_above = mbr - 1;
			}

			QP = coded_map[mbr + 1][mbc + 1] ?
				quant_map[mbr + 1][mbc + 1] : quant_map[mbr_above + 1][mbc + 1];

			delta = (int)(((int)(*(rec + i + (j - 3) * width)) +
				(int)(*(rec + i + (j - 2) * width)) +
				(int)(*(rec + i + (j - 1) * width)) +
				(int)(*(rec + i + (j) * width) * (-6)) +
				(int)(*(rec + i + (j + 1) * width)) +
				(int)(*(rec + i + (j + 2) * width)) +
				(int)(*(rec + i + (j + 3) * width))) / 8.0);

			d1 = sign(delta) * max(0, abs(delta) - max(0, 2 * (abs(delta) - STRENGTH1[QP - 1])));

			/* Filter D */
			*(rec + i + (j) * width) += d1;
		}
	}
	return;
}

static int STRENGTH2[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
void Decoder::vert_post_filter(unsigned char *rec, int width, int height, int chr)
{
	int i, j;
	int delta, d1;
	int mbc, mbr;
	int QP;
	int mbc_left;


	/* vertical edges */
	for (i = 8; i < width; i += 8)
	{
		for (j = 0; j < height; j++)
		{
			if (!chr)
			{
				mbr = j >> 4;
				mbc = i >> 4;
				mbc_left = (i - 8) >> 4;
			}
			else
			{
				mbr = j >> 3;
				mbc = i >> 3;
				mbc_left = mbc - 1;
			}

			QP = coded_map[mbr + 1][mbc + 1] ?
				quant_map[mbr + 1][mbc + 1] : quant_map[mbr + 1][mbc_left + 1];

			delta = (int)(((int)(*(rec + i - 3 + j * width)) +
				(int)(*(rec + i - 2 + j * width)) +
				(int)(*(rec + i - 1 + j * width)) +
				(int)(*(rec + i + j * width) * (-6)) +
				(int)(*(rec + i + 1 + j * width)) +
				(int)(*(rec + i + 2 + j * width)) +
				(int)(*(rec + i + 3 + j * width))) / 8.0);

			d1 = sign(delta) * max(0, abs(delta) - max(0, 2 * (abs(delta) - STRENGTH2[QP - 1])));

			/* Post Filter D */
			*(rec + i + j * width) += d1;
		}
	}
	return;
}
