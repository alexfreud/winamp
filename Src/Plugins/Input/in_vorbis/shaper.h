//from SSRC
#ifndef NULLSOFT_VORBIS_SHAPER_H
#define NULLSOFT_VORBIS_SHAPER_H
#include "main.h"

typedef float REAL;
enum
{
	DITHER_RECTANGLE=0,
	DITHER_TRIANGLE=1,
	DITHER_GAUSSIAN=2,
};
class Shaper
{
	double **shapebuf;
	int shaper_type,shaper_len,shaper_clipmin,shaper_clipmax;
	REAL *randbuf;
	int randptr;
	int dtype;
	int nch;

  public:
	Shaper(int freq,int _nch,int min,int max,int _dtype,int pdf,double noiseamp);

	int do_shaping(double s,/*double *peak,*/int ch);

	~Shaper();
};

#endif