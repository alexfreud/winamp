#include "main.h"
#include "SABuffer.h"
#include <math.h>
#include "WinampAttributes.h"
#include "fft.h"
extern int _srate;
#ifdef _M_IX86
__inline static int lrint(float flt)
{
	int intgr;

	_asm
	{
		fld flt
		fistp intgr
	}

	return intgr;
}
#else
__inline static int lrint(float flt)
{
	return (int)flt;
}
#endif


//  quantizes to 23 bits - use appropriately
inline static float fastmin(float x, const float b)
{
	x = b - x;
	x += (float)fabs(x);
	x *= 0.5f;
	x = b - x;
	return x;
}
#define FASTMIN(x,b) { x = b - x;   x += (float)fabs(x);   x *= 0.5f;   x = b - x; }
inline static float fastclip(float x, const float a, const float b)
{
	float x1 = (float)fabs(x-a);
	float x2 = (float)fabs(x-b);
	x = x1 + (a+b);
	x -= x2;
	x *= 0.5f;
	return (x);
}


void makeOscData(char *tempdata, char *data_buf, int little_block, int channels, int bits)
{
	float dd = little_block/75.0f;
	int x,c;
	int stride=bits/8; // number of bytes between samples

	// we're calculating using only the most significant byte,
	// because we only end up with 6 bit data anyway
	// if you want full resolution, check out CVS tag BETA_2005_1122_182830, file: vis.c
	char *ptr, *sbuf = data_buf;
	for (x = 0; x < 75; x ++)
	{
		float val=0;
		int index =(int)((float)x * dd);  // calculate the nearest sample for this point, interpolation is too expensive for this use
		ptr=&sbuf[index*stride*channels+stride-1]; // find first sample, and offset for little endian
		for (c=0;c<channels;c++)
		{
			val += (float)*ptr / 8.0f; // we want our final value to be -32 to 32
			ptr+=stride; // jump to the next sample (channels are interleaved)
		}
		tempdata[x] = (char)lrint(val / (float)channels);  // average the channels
	}
}



inline double fast_exp2(const double val)
{
	int    e;
	double ret;

	if (val >= 0)
	{
		e = int (val);
		ret = val - (e - 1);
		((*(1 + (int *) &ret)) &= ~(2047 << 20)) += (e + 1023) << 20;
	}
	else
	{
		e = int (val + 1023);
		ret = val - (e - 1024);
		((*(1 + (int *) &ret)) &= ~(2047 << 20)) += e << 20;
	}
	return (ret);
}

// ~6 clocks on Pentium M vs. ~24 for single precision sqrtf
#if !defined(_WIN64)
static inline float squareroot_sse_11bits(float x)
{
	float z;
	_asm
	{
		rsqrtss xmm0, x
		rcpss    xmm0, xmm0
		movss    z, xmm0            // z ~= sqrt(x) to 0.038%
	}
	return z;
}

static inline int floor_int(double x)
{
	int      i;
	static const float round_toward_m_i = -0.5f;
	__asm
	{
		fld      x
		fadd     st, st(0)
		fadd     round_toward_m_i
		fistp    i
		sar      i, 1
	}

	return (i);
}
#endif
/*
static inline float hermite(float x, float y0, float y1, float y2, float y3)
{
	// 4-point, 3rd-order Hermite (x-form)
	float c0 = y1;
	float c1 = 0.5f * (y2 - y0);
	float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
	float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);

	return ((c3 * x + c2) * x + c1) * x + c0;
}
*/

/*
static const float c_half = 0.5f;
__declspec(naked) static float hermite(float frac_pos, const float* pntr)
{
	__asm
	{
		push    ecx;
		mov     ecx, dword ptr[esp + 12]; //////////////////////////////////////////////////////////////////////////////////////////////////
		add     ecx, 0x04;            //    ST(0)        ST(1)        ST(2)        ST(3)        ST(4)        ST(5)        ST(6)        ST(7)
		fld     dword ptr [ecx+4];    //    x1
		fsub    dword ptr [ecx-4];    //    x1-xm1
		fld     dword ptr [ecx];      //    x0           x1-xm1
		fsub    dword ptr [ecx+4];    //    v            x1-xm1
		fld     dword ptr [ecx+8];    //    x2           v            x1-xm1
		fsub    dword ptr [ecx];      //    x2-x0        v            x1-xm1
		fxch    st(2);                //    x1-m1        v            x2-x0
		fmul    c_half;               //    c            v            x2-x0
		fxch    st(2);                //    x2-x0        v            c
		fmul    c_half;               //    0.5*(x2-x0)  v            c
		fxch    st(2);                //    c            v            0.5*(x2-x0)
		fst     st(3);                //    c            v            0.5*(x2-x0)    c
		fadd    st(0), st(1);         //    w            v            0.5*(x2-x0)    c
		fxch    st(2);                //    0.5*(x2-x0)  v            w              c
		faddp   st(1), st(0);         //    v+.5(x2-x0)  w            c
		fadd    st(0), st(1);         //    a            w            c
		fadd    st(1), st(0);         //    a            b_neg        c
		fmul    dword ptr [esp+8];    //    a*frac       b_neg        c
		fsubrp  st(1), st(0);         //    a*f-b        c
		fmul    dword ptr [esp+8];    //    (a*f-b)*f    c
		faddp   st(1), st(0);         //    res-x0/f
		fmul    dword ptr [esp+8];    //    res-x0
		fadd    dword ptr [ecx];      //    res
		pop     ecx;
		ret;
	}
}
*/
inline float hermite(float x, float y0, float y1, float y2, float y3)
{
    // 4-point, 3rd-order Hermite (x-form)
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);
    float c2 = y0 - y1 + c1 - c3;

    return ((c3 * x + c2) * x + c1) * x + c0;
}

static inline float fpow2(const float y)
{
    union
    {
        float f;
        int i;
    } c;

    int integer = lrint(floor(y));
		/* cut: because we guarantee y>=0
    if(y < 0)
        integer = integer-1;
		*/

    float frac = y - (float)integer;

    c.i = (integer+127) << 23;
    c.f *= 0.33977f*frac*frac + (1.0f-0.33977f)*frac + 1.0f;

    return c.f;
}

//#define SAPOW(x) (powf(2.f, (float)(x)/12.f))
#define SAPOW(x) (fpow2((float)(x)/12.f))
//#define WARP(x) ((powf(1.1f, (float)(x)/12.f) - 1.) * bla)
#define WARP(x) ((SAPOW(x) - 1.f) * bla)
void makeSpecData(unsigned char *tempdata, float *wavetrum)
{
	//WARP(75);
	float bla = (255.f/SAPOW(75.f));
	fft_9(wavetrum);

	float spec_scale=0.5;
	if (config_replaygain)
	{ // benski> i'm sure there's some math identity we can use to optimize this.
		spec_scale/=pow(10.0f, config_replaygain_non_rg_gain.GetFloat() / 20.0f);
	}

	for (int i=0;i<256;i++)
	{
		//int lookup=2*i;
		float sinT = wavetrum[2*i];
		float cosT = wavetrum[2*i+1];
		wavetrum[i] = sqrt(sinT*sinT+cosT*cosT)*spec_scale;
	}

	float next = WARP(0)+1 ;
	for (int x = 0; x < 75; x ++)
	{
		//float prev = 1.+(pow(2.,(float)x/12.) -1.) * bla;
		float binF = next;
		next = WARP(x+1) +1;

		float thisValue = 0;
		int bin = lrint(floor(binF));
		int end = lrint(floor(next));
		end = min(end, 255);
		float mult = ((float)(bin+1))-binF;
		bool herm=true;
		do
		{
			if (bin == end)
			{
				mult = (next-binF);
				herm=true;
			}

			if (herm)
			{				
				float C=0, D=0;
				if (bin<255)
				{
					C=wavetrum[bin+1];
					if (bin<254)
						D=wavetrum[bin+2];
				}

				//float samples[4] = { wavetrum[lookupA], wavetrum[lookupB], wavetrum[lookupC], wavetrum[lookupD] };
				//thisValue += hermite(binF-bin, samples) * mult;
				thisValue += hermite(binF-bin, wavetrum[bin-1], wavetrum[bin], C, D) * mult;
			}
			else
			{
				thisValue += wavetrum[bin];
			}

			herm=false;
			bin++;
			binF=(float)bin;
		}
		while (bin <= end);

		tempdata[x]=lrint(fastmin(thisValue, 255.f));
	}

}

////////////////////////////////

SABuffer saBuffer;

void sa_addpcmdata(void *_data_buf, int numChannels, int numBits, int ts)
{
	char *data_buf = reinterpret_cast<char *>(_data_buf);
	char tempdata[75*2] = {0};
	__declspec(align(16)) float wavetrum[512];
	//extern int sa_curmode;
	int vis_Csa=sa_override ? 3 : sa_curmode;

	switch (vis_Csa)
	{
	case 4:
		tempdata[0] = 0;
		tempdata[1] = 0;
		sa_add(tempdata,ts,4);
		return;
	case 2:
		makeOscData(tempdata,data_buf,576,numChannels, numBits);
		sa_add(tempdata,ts,2);
		return ;
	case 3:
		makeOscData(tempdata+75,data_buf,576,numChannels, numBits);
		// fall through!
	case 1:
		calcVuData((unsigned char*)tempdata, data_buf, numChannels, numBits);
		vu_add(tempdata, ts);
		break;
	}
	bool done=false;
	size_t samples=576;
	while (samples)
	{
		unsigned int copied = saBuffer.AddToBuffer(data_buf, numChannels, numBits, ts, (unsigned int) samples);
		samples-=copied;
		data_buf+=(copied*(numBits/8)*numChannels);
		if (saBuffer.Full())
		{
			saBuffer.WindowToFFTBuffer(wavetrum);
			if (!done)
			{
				if (vis_Csa == 3)
				{
					makeSpecData((unsigned char*)tempdata, wavetrum);
					sa_add(tempdata, ts, 0x80000003);
				}
				else if (vis_Csa == 1)
				{
					makeSpecData((unsigned char*)tempdata, wavetrum);
					sa_add(tempdata, ts, 1);
				}
			}
			//done=true;
			saBuffer.CopyHalf();
			ts+=MulDiv(SABUFFER_WINDOW_INCREMENT,1000,_srate);
		}
	}
}
