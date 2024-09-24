#include "idctref.h"
#include <math.h>
#ifndef PI
# ifdef M_PI
#  define PI M_PI
# else
#  define PI 3.14159265358979323846
# endif
#endif

	bool IDCTRef::initted = false;
	double IDCTRef::c[8][8]; /* cosine transform matrix for 8x1 IDCT */


	/* initialize DCT coefficient matrix */

	void IDCTRef::init()
	{
		if (!initted)
		{
			int freq, time;

			for (freq=0; freq < 8; freq++)
			{
				double scale = (freq == 0) ? sqrt(0.125) : 0.5;
				for (time=0; time<8; time++)
					c[freq][time] = scale*cos((PI/8.0)*freq*(time + 0.5));
			}
			initted=true;
		}
	}

	/* perform IDCT matrix multiply for 8x8 coefficient block */

	void IDCTRef::idct(short *block)
	{
		int i, j, k, v;
		double partial_product;
		double tmp[64];

		for (i=0; i<8; i++)
			for (j=0; j<8; j++)
			{
				partial_product = 0.0;

				for (k=0; k<8; k++)
					partial_product+= c[k][j]*block[8*i+k];

				tmp[8*i+j] = partial_product;
			}

		/* Transpose operation is integrated into address mapping by switching
		   loop order of i and j */

		for (j=0; j<8; j++)
			for (i=0; i<8; i++)
			{
				partial_product = 0.0;

				for (k=0; k<8; k++)
					partial_product+= c[k][i]*tmp[8*k+j];

				v = (int)floor(partial_product+0.5);
				block[8*i+j] = (v<-256) ? -256 : ((v>255) ? 255 : v);
			}
	}
