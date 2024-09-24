#include "Shaper.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795028842
#endif

#define RANDBUFLEN 65536

#define RINT(x) ((x) >= 0 ? ((int)((x) + 0.5)) : ((int)((x) - 0.5)))

const int scoeffreq[] =
  {
    0, 48000, 44100, 37800, 32000, 22050, 48000, 44100
  };


const int scoeflen[] =
  {
    1, 16, 20, 16, 16, 15, 16, 15
  };

const int samp[] =
  {
    8, 18, 27, 8, 8, 8, 10, 9
  };

const double shapercoefs[8][21] =
  {
    {
      -1
    }
    , /* triangular dither */

    { -2.8720729351043701172,   5.0413231849670410156,  -6.2442994117736816406,   5.8483986854553222656,
      -3.7067542076110839844,   1.0495119094848632812,   1.1830236911773681641,  -2.1126792430877685547,
      1.9094531536102294922,  -0.99913084506988525391,  0.17090806365013122559,  0.32615602016448974609,
      -0.39127644896507263184,  0.26876461505889892578, -0.097676105797290802002, 0.023473845794796943665,
    }, /* 48k, N=16, amp=18 */

    { -2.6773197650909423828,   4.8308925628662109375,  -6.570110321044921875,    7.4572014808654785156,
      -6.7263274192810058594,   4.8481650352478027344,  -2.0412089824676513672,  -0.7006359100341796875,
      2.9537565708160400391,  -4.0800385475158691406,   4.1845216751098632812,  -3.3311812877655029297,
      2.1179926395416259766,  -0.879302978515625,       0.031759146600961685181, 0.42382788658142089844,
      -0.47882103919982910156,  0.35490813851356506348, -0.17496839165687561035,  0.060908168554306030273,
    }, /* 44.1k, N=20, amp=27 */

    { -1.6335992813110351562,   2.2615492343902587891,  -2.4077029228210449219,   2.6341717243194580078,
      -2.1440362930297851562,   1.8153258562088012695,  -1.0816224813461303711,   0.70302653312683105469,
      -0.15991993248462677002, -0.041549518704414367676, 0.29416576027870178223, -0.2518316805362701416,
      0.27766478061676025391, -0.15785403549671173096,  0.10165894031524658203, -0.016833892092108726501,
    }, /* 37.8k, N=16 */

    { -0.82901298999786376953,  0.98922657966613769531, -0.59825712442398071289,  1.0028809309005737305,
      -0.59938216209411621094,  0.79502451419830322266, -0.42723315954208374023,  0.54492527246475219727,
      -0.30792605876922607422,  0.36871799826622009277, -0.18792048096656799316,  0.2261127084493637085,
      -0.10573341697454452515,  0.11435490846633911133, -0.038800679147243499756, 0.040842197835445404053,
    }, /* 32k, N=16 */

    { -0.065229974687099456787, 0.54981261491775512695,  0.40278548002243041992,  0.31783768534660339355,
      0.28201797604560852051,  0.16985194385051727295,  0.15433363616466522217,  0.12507140636444091797,
      0.08903945237398147583,  0.064410120248794555664, 0.047146003693342208862, 0.032805237919092178345,
      0.028495194390416145325, 0.011695005930960178375, 0.011831838637590408325,
    }, /* 22.05k, N=15 */

    { -2.3925774097442626953,   3.4350297451019287109,  -3.1853709220886230469,   1.8117271661758422852,
      0.20124770700931549072, -1.4759907722473144531,   1.7210904359817504883,  -0.97746700048446655273,
      0.13790138065814971924,  0.38185903429985046387, -0.27421241998672485352, -0.066584214568138122559,
      0.35223302245140075684, -0.37672343850135803223,  0.23964276909828186035, -0.068674825131893157959,
    }, /* 48k, N=16, amp=10 */

    { -2.0833916664123535156,   3.0418450832366943359,  -3.2047898769378662109,   2.7571926116943359375,
      -1.4978630542755126953,   0.3427594602108001709,   0.71733748912811279297, -1.0737057924270629883,
      1.0225815773010253906,  -0.56649994850158691406,  0.20968692004680633545,  0.065378531813621520996,
      -0.10322438180446624756,  0.067442022264003753662, 0.00495197344571352005,
    }, /* 44.1k, N=15, amp=9 */

#if 0
    { -3.0259189605712890625,  6.0268716812133789062,  -9.195003509521484375,   11.824929237365722656,
      -12.767142295837402344,  11.917946815490722656,   -9.1739168167114257812,   5.3712320327758789062,
      -1.1393624544143676758, -2.4484779834747314453,   4.9719839096069335938,  -6.0392003059387207031,
      5.9359521865844726562, -4.903278350830078125,    3.5527443885803222656,  -2.1909697055816650391,
      1.1672389507293701172, -0.4903914332389831543,   0.16519790887832641602, -0.023217858746647834778,
    }, /* 44.1k, N=20 */
#endif
  };

#define POOLSIZE 97

Shaper::Shaper(int freq, int _nch, int min, int max, int _dtype, int pdf, double noiseamp)
{
	int i;
	float pool[POOLSIZE] = {0};

	nch = _nch;
	dtype = _dtype;

	for (i = 1;i < 6;i++) if (freq == scoeffreq[i]) break;
	/*	  if ((dtype == 3 || dtype == 4) && i == 6) {
			fprintf(stderr,"Warning: ATH based noise shaping for destination frequency %dHz is not available, using triangular dither\n",freq);
		  }*/
	if (dtype == 2 || i == 6) i = 0;
	if (dtype == 4 && (i == 1 || i == 2)) i += 5;

	shaper_type = i;

	shapebuf = (double**)malloc(sizeof(double *) * nch);
	shaper_len = scoeflen[shaper_type];

	for (i = 0;i < nch;i++)
		shapebuf[i] = (double*)calloc(shaper_len, sizeof(double));

	shaper_clipmin = min;
	shaper_clipmax = max;

	randbuf = (REAL*)malloc(sizeof(REAL) * RANDBUFLEN);

	for (i = 0;i < POOLSIZE;i++) pool[i] = warandf();

	switch (pdf)
	{
		case DITHER_RECTANGLE: // rectangular
			for (i = 0;i < RANDBUFLEN;i++)
			{
				float r;
				int p;

				p = warand() % POOLSIZE;
				r = pool[p]; pool[p] = warandf();
				randbuf[i] = (REAL)(noiseamp * (((double)r) - 0.5));
			}
			break;

		case DITHER_TRIANGLE:
			for (i = 0;i < RANDBUFLEN;i++)
			{
				float r1, r2;
				int p;

				p = warand() % POOLSIZE;
				r1 = pool[p]; pool[p] = warandf();
				p = warand() % POOLSIZE;
				r2 = pool[p]; pool[p] = warandf();
				randbuf[i] = (REAL)(noiseamp * ((((double)r1)) - (((double)r2))));
			}
			break;
#if 0
		case DITHER_GAUSSIAN: // gaussian
			for (i = 0;i < RANDBUFLEN;i++)
			{
				int sw = 0;
				double t, u;
				double r;
				int p;

				if (sw == 0)
				{
					sw = 1;

					p = warand() % POOLSIZE;
					r = ((double)pool[p]); pool[p] = warandf();

					t = sqrt(-2 * log(1 - r));

					p = warand() % POOLSIZE;
					r = ((double)pool[p]); pool[p] = warandf();

					u = 2 * M_PI * r;

					randbuf[i] = noiseamp * t * cos(u);
				}
				else
				{
					sw = 0;

					randbuf[i] = noiseamp * t * sin(u);
				}
			}
			break;
#endif
	}

	randptr = 0;

//	  if (dtype == 0 || dtype == 1) return 1;
	//return samp[shaper_type];
}

Shaper::~Shaper()
{
	int i;

	for (i = 0;i < nch;i++) free(shapebuf[i]);
	free(shapebuf);
	free(randbuf);
}

int Shaper::do_shaping(double s,/*double *peak,*/int ch)
{
	double u, h;
	int i;

	if (dtype == 1)
	{
		s += randbuf[randptr++ & (RANDBUFLEN-1)];

		if (s < shaper_clipmin)
		{
			//double d = (double)s / shaper_clipmin;
			//*peak = *peak < d ? d : *peak;
			s = shaper_clipmin;
		}
		if (s > shaper_clipmax)
		{
			//double d = (double)s / shaper_clipmax;
			//*peak = *peak < d ? d : *peak;
			s = shaper_clipmax;
		}

		return RINT(s);
	}

	h = 0;
	for (i = 0;i < shaper_len;i++)
		h += shapercoefs[shaper_type][i] * shapebuf[ch][i];
	s += h;
	u = s;
	s += randbuf[randptr++ & (RANDBUFLEN-1)];
	if (s < shaper_clipmin)
	{
		//double d = (double)s / shaper_clipmin;
		//*peak = *peak < d ? d : *peak;
		s = shaper_clipmin;
	}
	if (s > shaper_clipmax)
	{
		//double d = (double)s / shaper_clipmax;
		//*peak = *peak < d ? d : *peak;
		s = shaper_clipmax;
	}
	s = RINT(s);
	for (i = shaper_len - 2;i >= 0;i--) shapebuf[ch][i+1] = shapebuf[ch][i];
	shapebuf[ch][0] = s - u;

	return (int)s;
}