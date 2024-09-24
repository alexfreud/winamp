/*
 * This program is  free software; you can redistribute it  and modify it
 * under the terms of the GNU  General Public License as published by the
 * Free Software Foundation; either version 2  of the license or (at your
 * option) any later version.
 *
 * Authors: Markus Fick <webmaster@mark-f.de> fir-resampler, technical information
 *          Chris Moeller <chris@kode54.net> Table/struct file generator
 *          
*/

#include <math.h>
#include <stdio.h>

/* 
  ------------------------------------------------------------------------------------------------
   fir interpolation doc,
	(derived from "an engineer's guide to fir digital filters", n.j. loy)

	calculate coefficients for ideal lowpass filter (with cutoff = fc in 0..1 (mapped to 0..nyquist))
	  c[-N..N] = (i==0) ? fc : sin(fc*pi*i)/(pi*i)

	then apply selected window to coefficients
	  c[-N..N] *= w(0..N)
	with n in 2*N and w(n) being a window function (see loy)

	then calculate gain and scale filter coefs to have unity gain.
  ------------------------------------------------------------------------------------------------
*/
// quantizer scale of window coefs
#define WFIR_QUANTBITS		14
#define WFIR_QUANTSCALE		(1L<<WFIR_QUANTBITS)
#define WFIR_8SHIFT			(WFIR_QUANTBITS-8)
#define WFIR_16BITSHIFT		(WFIR_QUANTBITS)
// log2(number)-1 of precalculated taps range is [4..12]
#define WFIR_FRACBITS		10
#define WFIR_LUTLEN			((1L<<(WFIR_FRACBITS+1))+1)
// number of samples in window
#define WFIR_LOG2WIDTH		3
#define WFIR_WIDTH			(1L<<WFIR_LOG2WIDTH)
#define WFIR_SMPSPERWING	((WFIR_WIDTH-1)>>1)
// cutoff (1.0 == pi/2)
#define WFIR_CUTOFF			0.90f
#define WFIR_CUTOFFBITS         9
#define WFIR_CUTOFFLEN		((1L<<(WFIR_CUTOFFBITS))+1)
// wfir type
#define WFIR_HANN			0
#define WFIR_HAMMING		1
#define WFIR_BLACKMANEXACT	2
#define WFIR_BLACKMAN3T61	3
#define WFIR_BLACKMAN3T67	4
#define WFIR_BLACKMAN4T92	5
#define WFIR_BLACKMAN4T74	6
#define WFIR_KAISER4T		7
#define WFIR_TYPE			WFIR_BLACKMANEXACT
// wfir help
#ifndef M_zPI
#define M_zPI			3.1415926535897932384626433832795
#endif
#define M_zEPS			1e-8
#define M_zBESSELEPS	1e-21

class CzWINDOWEDFIR
{	public:
		CzWINDOWEDFIR( );
		~CzWINDOWEDFIR( );
		float coef( int _PCnr, float _POfs, float _PCut, int _PWidth, int _PType ) //float _PPos, float _PFc, int _PLen )
		{	double	_LWidthM1		= _PWidth-1;
			double	_LWidthM1Half	= 0.5*_LWidthM1;
			double	_LPosU			= ((double)_PCnr - _POfs);
			double	_LPos			= _LPosU-_LWidthM1Half;
			double	_LPIdl			= 2.0*M_zPI/_LWidthM1;
			double	_LWc,_LSi;
			if( fabs(_LPos)<M_zEPS )
			{	_LWc	= 1.0;
				_LSi	= _PCut;
			}
			else
			{	switch( _PType )
				{	case WFIR_HANN:
						_LWc = 0.50 - 0.50 * cos(_LPIdl*_LPosU);
						break;
					case WFIR_HAMMING:
						_LWc = 0.54 - 0.46 * cos(_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMANEXACT:
						_LWc = 0.42 - 0.50 * cos(_LPIdl*_LPosU) + 0.08 * cos(2.0*_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMAN3T61:
						_LWc = 0.44959 - 0.49364 * cos(_LPIdl*_LPosU) + 0.05677 * cos(2.0*_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMAN3T67:
						_LWc = 0.42323 - 0.49755 * cos(_LPIdl*_LPosU) + 0.07922 * cos(2.0*_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMAN4T92:
						_LWc = 0.35875 - 0.48829 * cos(_LPIdl*_LPosU) + 0.14128 * cos(2.0*_LPIdl*_LPosU) - 0.01168 * cos(3.0*_LPIdl*_LPosU);
						break;
					case WFIR_BLACKMAN4T74:
						_LWc = 0.40217 - 0.49703 * cos(_LPIdl*_LPosU) + 0.09392 * cos(2.0*_LPIdl*_LPosU) - 0.00183 * cos(3.0*_LPIdl*_LPosU);
						break;
					case WFIR_KAISER4T:
						_LWc = 0.40243 - 0.49804 * cos(_LPIdl*_LPosU) + 0.09831 * cos(2.0*_LPIdl*_LPosU) - 0.00122 * cos(3.0*_LPIdl*_LPosU);
						break;
					default:
						_LWc = 1.0;
						break;
				}
				_LPos	 *= M_zPI;
				_LSi	 = sin(_PCut*_LPos)/_LPos;
			}
			return (float)(_LWc*_LSi);
		}
		static signed short lut[WFIR_LUTLEN*WFIR_WIDTH];
};

signed short CzWINDOWEDFIR::lut[WFIR_LUTLEN*WFIR_WIDTH];

CzWINDOWEDFIR::CzWINDOWEDFIR()
{	int _LPcl;
	float _LPcllen	= (float)(1L<<WFIR_FRACBITS);	// number of precalculated lines for 0..1 (-1..0)
	float _LNorm	= 1.0f / (float)(2.0f * _LPcllen);
	float _LCut		= WFIR_CUTOFF;
	float _LScale	= (float)WFIR_QUANTSCALE;
	float _LGain,_LCoefs[WFIR_WIDTH];
	for( _LPcl=0;_LPcl<WFIR_LUTLEN;_LPcl++ )
	{
		float _LOfs		= ((float)_LPcl-_LPcllen)*_LNorm;
		int _LCc,_LIdx	= _LPcl<<WFIR_LOG2WIDTH;
		for( _LCc=0,_LGain=0.0f;_LCc<WFIR_WIDTH;_LCc++ )
		{	_LGain	+= (_LCoefs[_LCc] = coef( _LCc, _LOfs, _LCut, WFIR_WIDTH, WFIR_TYPE ));
		}
		_LGain = 1.0f/_LGain;
		for( _LCc=0;_LCc<WFIR_WIDTH;_LCc++ )
		{	float _LCoef = (float)floor( 0.5 + _LScale*_LCoefs[_LCc]*_LGain );
			lut[_LIdx+_LCc] = (signed short)( (_LCoef<-_LScale)?-_LScale:((_LCoef>_LScale)?_LScale:_LCoef) );
		}
	}
}

CzWINDOWEDFIR::~CzWINDOWEDFIR()
{	// nothing todo
}

CzWINDOWEDFIR sfir;

// extern "C" signed short *fir_lut = &CzWINDOWEDFIR::lut[0];

#define lut(a) CzWINDOWEDFIR::lut[a]

int main()
{

	FILE *f;
	int i;

    f = fopen("fir_table.h","w");

    fprintf(f,"static __int64 fir_lut[%d] = {\n",WFIR_LUTLEN * 2);

	for (i=0;i<(WFIR_LUTLEN*WFIR_WIDTH);i+=WFIR_WIDTH)
	{
        fprintf(f,"\t0x%.4hx%.4hx%.4hx%.4hx, 0x%.4hx%.4hx%.4hx%.4hx%s",
            lut(i+3), lut(i+2), lut(i+1), lut(i),
            lut(i+7), lut(i+6), lut(i+5), lut(i+4),
            (i<((WFIR_LUTLEN-1)*WFIR_WIDTH)) ? ",\n" : "\n};\n");
	}

	fclose(f);

	return(0);
}

