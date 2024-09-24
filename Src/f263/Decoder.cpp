#include "Decoder.h"


bool Decoder::initted;
unsigned char *Decoder::clp;
unsigned char Decoder::clp_table[1024];

int Decoder::DecodeFrame(YV12_PLANES *yv12, int *width, int *height, int *keyframe)
{
	if (getheader())
	{
		if (firstFrame)
		{
			if (init())
				return 1;
		}

		Frame frame;
		getpicture(frame);
		yv12->y.baseAddr = frame[0];
		yv12->y.rowBytes = coded_picture_width;
		yv12->u.baseAddr = frame[1];
		yv12->u.rowBytes = chrom_width;
		yv12->v.baseAddr = frame[2];
		yv12->v.rowBytes = chrom_width;

		*width = horizontal_size;
		*height = vertical_size;
		*keyframe = (pict_type == PCT_INTRA)?1:0;

		return 0;
	}
	return 1;
}

Decoder::Decoder()
{
	for (int cc=0; cc<3; cc++)
	{
		refframe[cc]=0;
		newframe[cc]=0;
		oldrefframe[cc]=0;
		edgeframe[cc]=0;
		edgeframeorig[cc]=0;
	}

	horizontal_size=0;
	vertical_size=0;
	mb_width=0;
	mb_height=0;
	coded_picture_width=0;
	coded_picture_height=0;
	chrom_width=0;
	chrom_height=0;
	pict_type=0;
	fault=0;
	refidct=0;
	quant=0;
	escapemode=0;

	firstFrame=true;
	if (!initted)
	{
		/* clip table */
		clp=&clp_table[384];

		for (int i=-384; i<640; i++)
			clp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

		initted=true;
	}
	idct.init();
}

Decoder::~Decoder()
{
	for (int cc=0; cc<3; cc++)
	{
		free(refframe[cc]);
		free(oldrefframe[cc]);
		free(edgeframeorig[cc]);
	}
}

#define ROUNDUP16(size) (((size)+15) & ~15)

int Decoder::init()
{
	int cc;
	unsigned int size;

	mb_width = (horizontal_size+15)/16;
	mb_height = (vertical_size +15)/16;
	coded_picture_width = ROUNDUP16(horizontal_size);
	coded_picture_height = ROUNDUP16(vertical_size);
	chrom_width =  coded_picture_width>>1;
	chrom_height = coded_picture_height>>1;

	if (coded_picture_width >= (65536 - 64)
		|| coded_picture_height >= (65536 - 64))
		return 1;

	for (cc=0; cc<3; cc++)
	{
		if (cc==0)
			size = coded_picture_width*coded_picture_height;
		else
			size = chrom_width*chrom_height;

		refframe[cc] = (unsigned char *)malloc(size);
		oldrefframe[cc] = (unsigned char *)malloc(size);
	}

	for (cc=0; cc<3; cc++)
	{
		if (cc==0)
		{
			size = (coded_picture_width+64)*(coded_picture_height+64);
			edgeframeorig[cc] = (unsigned char *)malloc(size);

			edgeframe[cc] = edgeframeorig[cc] + (coded_picture_width+64) * 32 + 32;
		}
		else
		{
			size = (chrom_width+32)*(chrom_height+32);
			edgeframeorig[cc] = (unsigned char *)malloc(size);

			edgeframe[cc] = edgeframeorig[cc] + (chrom_width+32) * 16 + 16;
		}
	}

	return 0;
}
