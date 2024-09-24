#pragma once

class Biquad
{
public:
	Biquad();
	void copy_filter(const Biquad &other);
	void SetSampleRate(double fs);
	void set_freq(double f0);
	void set_s_eq(const double b[3], const double a[3]);
	void transform_s_to_z();

	void process_block(float *dest_ptr, const float *src_ptr, long nbr_spl);
	void clear_buffers();

private:
	double _s_eq_b[3]; // Coefs for numerator (zeros)
	double _s_eq_a[3]; // Coefs for denominator (poles)
	double _z_eq_b[3]; // Direct coefficients, order z^(-n)
	double _z_eq_a[3]; // Recursive coefficients, order z^(-n)
	double sampleRate; // Hz, > 0
	double _f0; // Hz, > 0, _f0 % (_sample_freq/2) != 0
	double xn[2]; // Input memory, order z^(-n)
	double yn[2]; // Output memory, order z^(-n)
	int _mem_pos; // 0 or 1

	inline double process_sample(double x);
};

