#include "EqBand.h"
#include <assert.h>
#include <math.h>

EqBand::EqBand() : sampleRate(44100), centerFrequency(1000), gain(1), _q(0.5), nch(0), channels(0), bypass(true)
{
}

void EqBand::set_num_channels(int num_channels)
{
	if (nch < num_channels)
	{
		nch = num_channels;
		delete[]channels;
		channels = new Biquad[nch];
		clear_buffers();
		set_parameters(centerFrequency, gain, _q);
	}
}

void EqBand::SetSampleRate(double sample_freq)
{
	if (sample_freq != sampleRate)
	{
		sampleRate = sample_freq;
		for (int chn = 0; chn < nch; ++chn)
		{
			channels[chn].SetSampleRate(sampleRate);
		}
		clear_buffers();
		set_parameters(centerFrequency, gain, _q);
	}
}

void EqBand::set_parameters(double freq, double newGain, double q)
{
	centerFrequency = freq;
	gain = newGain;
	_q = q;

	if (nch > 0)
	{
		Biquad & ref_filter = channels[0];

		ref_filter.set_freq(centerFrequency);

		double a[3] = { 1, 1/_q, 1 };
		double b[3] = {1, gain / _q, 1};
		ref_filter.set_s_eq(b, a);

		ref_filter.transform_s_to_z();

		for (int chn = 1; chn < nch; ++chn)
			channels[chn].copy_filter(ref_filter);
	}

	bypass = (fabs(gain - 1.0) < 0.02); // About 1/4 dB
}

void EqBand::process(float ** const out, float ** in, long nbr_spl, int nbr_chn)
{
	assert(nbr_chn >= 0);
	assert(nbr_chn <= nch);

	if (!bypass)
	{
		for (int chn = 0; chn < nbr_chn; ++chn)
		{
			channels[chn].process_block(out[chn], in[chn], nbr_spl);
		}
	}
}

void EqBand::clear_buffers()
{
	for (int chn = 0; chn < nch; ++chn)
	{
		channels[chn].clear_buffers();
	}
}

