#include "Biquad.h"
#include <assert.h>
#include <math.h>

Biquad::Biquad()
		: sampleRate(44100)
		, _f0(1000)
{
	_z_eq_b[0] = 1;
	_z_eq_b[1] = 0;
	_z_eq_b[2] = 0;
	_z_eq_a[0] = 1;
	_z_eq_a[1] = 0;
	_z_eq_a[2] = 0;

	clear_buffers();

	_s_eq_a[0] = 1;
	_s_eq_a[1] = 2;
	_s_eq_a[2] = 1;

	_s_eq_b[0] = _s_eq_a[0];
	_s_eq_b[1] = _s_eq_a[1];
	_s_eq_b[2] = _s_eq_a[2];
}


#define M_PI 3.14159265358979323846
// warp to the z-plane
void Biquad::transform_s_to_z()
{
	// s to z bilinear transform
	const double inv_k = tan(_f0 * M_PI / sampleRate);
	const double k = 1 / inv_k;
	const double kk = k*k;

	const double b1k = _s_eq_b[1] * k;
	const double b2kk = _s_eq_b[2] * kk;
	const double b2kk_plus_b0 = b2kk + _s_eq_b[0];
	const double b0z = b2kk_plus_b0 + b1k;
	const double b2z = b2kk_plus_b0 - b1k;
	const double b1z = 2 * (_s_eq_b[0] - b2kk);

	const double a1k = _s_eq_a[1] * k;
	const double a2kk = _s_eq_a[2] * kk;
	const double a2kk_plus_a0 = a2kk + _s_eq_a[0];
	const double a0z = a2kk_plus_a0 + a1k;
	const double a2z = a2kk_plus_a0 - a1k;
	const double a1z = 2 * (_s_eq_a[0] - a2kk);

	// IIR coefficients
	const double mult = 1 / a0z;

	_z_eq_b[0] = float(b0z * mult);
	_z_eq_b[1] = float(b1z * mult);
	_z_eq_b[2] = float(b2z * mult);

	_z_eq_a[0] = 1;
	_z_eq_a[1] = float(a1z * mult);
	_z_eq_a[2] = float(a2z * mult);
}

void Biquad::process_block(float *dest_ptr, const float *src_ptr, long nbr_spl)
{
	assert(nbr_spl >= 0);

	if (nbr_spl == 0)
	{
		return;
	}

// If we're not on a pair boudary, we process a single sample.
	if (_mem_pos != 0)
	{
		*dest_ptr++ = (float)process_sample(*src_ptr++);
		nbr_spl--;
	}

	if (nbr_spl == 0)
	{
		return;
	}

	long    half_nbr_spl = nbr_spl >> 1;
	long    index = 0;
	if (half_nbr_spl > 0)
	{
		double    mem_x[2];
		double    mem_y[2];
		mem_x[0] = xn[0];
		mem_x[1] = xn[1];
		mem_y[0] = yn[0];
		mem_y[1] = yn[1];

		do
		{

			float    x = src_ptr[index];
			mem_y[1] = _z_eq_b[0] * x
			           + (_z_eq_b[1] * mem_x[0]
			              + _z_eq_b[2] * mem_x[1])
			           - (_z_eq_a[1] * mem_y[0]
			              + _z_eq_a[2] * mem_y[1]);

			mem_x[1] = x;
			dest_ptr[index] = (float)mem_y[1];

			x = src_ptr[index + 1];
			mem_y[0] = _z_eq_b[0] * x
			           + (_z_eq_b[1] * mem_x[1]
			              + _z_eq_b[2] * mem_x[0])
			           - (_z_eq_a[1] * mem_y[1]
			              + _z_eq_a[2] * mem_y[0]);

			mem_x[0] = x;
			dest_ptr[index + 1] = (float)mem_y[0];
			index += 2;

			-- half_nbr_spl;
		}
		while (half_nbr_spl > 0);

		xn[0] = mem_x[0];
		xn[1] = mem_x[1];
		yn[0] = mem_y[0];
		yn[1] = mem_y[1];
	}

// If number of samples was odd, there is one more to process.
	if ((nbr_spl & 1) > 0)
	{
		dest_ptr[index] = (float)process_sample(src_ptr[index]);
	}
}

void Biquad::clear_buffers()
{
	xn[0] = 0;
	xn[1] = 0;
	yn[0] = 0;
	yn[1] = 0;
	_mem_pos = 0;
}

double Biquad::process_sample(double x)
{
	const int  alt_pos = 1 - _mem_pos;
	const double  y = _z_eq_b[0] * x
	                  + (_z_eq_b[1] * xn[_mem_pos]
	                     + _z_eq_b[2] * xn[alt_pos])
	                  - (_z_eq_a[1] * yn[_mem_pos]
	                     + _z_eq_a[2] * yn[alt_pos]);

	xn[alt_pos] = x;
	yn[alt_pos] = y;
	_mem_pos = alt_pos;

	return (y);
}

void Biquad::copy_filter(const Biquad &other)
{
	_z_eq_b[0] = other._z_eq_b[0];
	_z_eq_b[1] = other._z_eq_b[1];
	_z_eq_b[2] = other._z_eq_b[2];
	_z_eq_a[1] = other._z_eq_a[1];
	_z_eq_a[2] = other._z_eq_a[2];

	sampleRate = other.sampleRate;
	_f0 = other._f0;
	set_s_eq(other._s_eq_b, other._s_eq_a);
}

void Biquad::SetSampleRate(double fs)
{
	assert(fs > 0);

	sampleRate = fs;
	transform_s_to_z();
}

void Biquad::set_freq(double f0)
{
	assert(f0 > 0);

	_f0 = f0;
}

void Biquad::set_s_eq(const double b[3], const double a[3])
{
	assert(a != 0);
	assert(a[2] != 0);
	assert(b != 0);

	_s_eq_b[0] = float(b[0]);
	_s_eq_b[1] = float(b[1]);
	_s_eq_b[2] = float(b[2]);

	_s_eq_a[0] = float(a[0]);
	_s_eq_a[1] = float(a[1]);
	_s_eq_a[2] = float(a[2]);
}


