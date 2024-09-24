#pragma once

#include "Biquad.h"

class EqBand
{
public:
	EqBand();
	void set_num_channels(int num_channels);
	void SetSampleRate(double sample_freq);
	void set_parameters(double freq, double gain, double q);
	void process(float ** const out, float ** in, long nbr_spl, int nbr_chn);

private:
	double sampleRate, centerFrequency, gain;
	double _q;
	int nch;
	Biquad *channels;
	bool bypass;

	void clear_buffers();
}; 

