#include "main.h"
#include "decoder.h"
#include <math.h>
#include <locale.h>
#pragma warning(disable:4244)
#include "shaper.h"
#include "api__in_vorbis.h"

Decoder::~Decoder() {if (shaper) delete shaper;}

extern CfgInt
	cfg_mc6_dm, cfg_mc6_map;
/*
if (vorbis_cfg.use_hq_preamp)
{
   sample *= pow(10., preamp_db/20);
					
   //hard 6dB limiting 
  if (sample < -0.5)
               sample = tanh((sample + 0.5) / (1-0.5)) * (1-0.5) - 0.5;
   else if (sample > 0.5)
              sample = tanh((sample - 0.5) / (1-0.5)) * (1-0.5) + 0.5;
}	*/

#if 0
static float q_tanh(float x)
{
	double foo1, foo2;
	foo1 = pow(2.71828182845904523536028747135266, x);
	foo2 = 1.0 / foo1;
	return (foo1 -foo2) / (foo1 + foo2);
}
#else
#define q_tanh tanh
#endif

float VorbisFile::GetGain()
{
	float peak;

	vorbis_comment * c;
	float scale = 1.0f;
	c = ov_comment(&vf, -1);
	peak = 0.99f;
	if (c)
	{
		if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain", false))
		{
			char * _peak = 0, *_gain = 0;
			float gain = 0;
			bool have_rg = 0;
			float lwing_gain = 0;
			char *gain1 = 0, *gain2 = 0, *peak1 = 0, *peak2 = 0;
			gain1 = vorbis_comment_query(c, "replaygain_album_gain", 0);
			if (!gain1) gain1 = vorbis_comment_query(c, "rg_audiophile", 0);
			gain2 = vorbis_comment_query(c, "replaygain_track_gain", 0);
			if (!gain2) gain2 = vorbis_comment_query(c, "rg_radio", 0);

			peak1 = vorbis_comment_query(c, "replaygain_album_peak", 0);
			peak2 = vorbis_comment_query(c, "replaygain_track_peak", 0);
			switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_source", 0))
			{
			case 0:   // track
				_gain = gain2;
				if (!_gain && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					_gain = gain1;
				_peak = peak2;
				if (!_peak	&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					_peak = peak1;
				break;
			case 1:   // album
				_gain = gain1;
				if (!_gain	&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					_gain = gain2;
				_peak = peak1;
				if (!_peak	&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					_peak = peak2;
				break;
			}

			if (!_peak)
			{
				_peak = vorbis_comment_query(c, "rg_peak", 0);
			}

			_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();

			if (_peak) peak = _atof_l(_peak, C_locale);
			if (_gain) gain = _atof_l(_gain, C_locale);

			if (!_peak && !_gain)
			{
				char * l = vorbis_comment_query(c, "lwing_gain", 0);
				if (l)
				{
					lwing_gain = _atof_l(l, C_locale);
					have_rg = 1;
				}
			}
			else have_rg = 1;

			if (!have_rg)
			{
				gain = AGAVE_API_CONFIG->GetFloat(playbackConfigGroupGUID, L"non_replaygain", -6.0f);
			}

			scale = powf(10, (gain) / 20.0f);
			if (lwing_gain)
				scale *= lwing_gain;
			else if (have_rg)
				switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_mode", 1))
				{
				case 1:   // apply gain, but don't clip
					if (scale*peak > 1.0) scale = 1.0 / peak;
					break;
				case 2:   // normalize
					scale = 1.0 / peak;
					break;
				case 3:   // no clipping
					if (peak > 1.0f)
						scale = 1.0 / peak;
					break;
				}
		}
	}


	return scale;
}

void Decoder::process_rg()
{
	scale = file->GetGain();
}

void Decoder::setup_mc()
{
	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false))
		nch = 1;
	else if (src_nch == 6)
	{
		switch (cfg_mc6_dm)
		{
		case 2:
			nch = 4;
			break;
		case 3:
		case 4:
			nch = 2;
			break;
		case 5:
			nch = 1;
			break;
		}

		if (nch > 2 && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"surround", true))
			nch = 2;
	}
}

void Decoder::Flush()
{
	bptr = 0;
	pcmbuf = 0;
	data = 0;
	pos = 0;
	if (shaper) {delete shaper;shaper = 0;}
}

void Decoder::Init(VorbisFile * f, UINT _bits, UINT _nch, bool _useFloat, bool allowRG)
{
	useFloat = _useFloat;

	file = f;
	vorbis_info * i = ov_info(&file->vf, -1);

	if (allowRG)
		process_rg();
	else
		scale = 1.0f;

	if (useFloat)
	{
		dither = false;
		bps = 32;
	}
	else
	{
		dither = AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"dither", true);

		if (_bits)
			bps = _bits;
		else
			bps = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
	}

	if (useFloat)
	{
		clipmin = -10000; // some arbitrarily large number
		clipmax = 10000; // some arbitrarily large number
	}
	else
	{
		clipmin = - (1 << (bps - 1));
		clipmax = (1 << (bps - 1)) - 1;
	}
	sr = i->rate;
	nch = src_nch = i->channels;
	Flush();
	cur_link = file->vf.current_link;

	if (_nch)
		nch = _nch;
	else
		setup_mc();
}

UINT Decoder::DataAvailable()
{
	return data * (bps >> 3);
}

int Decoder::DoFrame()
{
	need_reopen = 0;
	while (1)
	{
		data = ov_read_float(&file->vf, &pcmbuf, 576, 0);
		if ((int)data <= 0)
		{
			if (data == OV_HOLE) {continue;}
			data = 0;
			return 0;
		}
		break;
	}

	pos = 0;
	if (cur_link != file->vf.current_link)
	{
		vorbis_info* i = ov_info(&file->vf, -1);
		if (sr != (UINT)i->rate || src_nch != (UINT)i->channels)
		{
			UINT old_nch = nch, old_sr = sr;
			if (shaper) {delete shaper;shaper = 0;}
			sr = i->rate;
			src_nch = nch = i->channels;
			setup_mc();
			if (nch != old_nch || sr != old_sr)
			{
				need_reopen = 1;
			}
		}
		process_rg();
		cur_link = file->vf.current_link;
	}
	data *= nch;
	return 1;
}

int Decoder::Read(UINT bytes, void * buf)
{
	UINT wr = 0;
	if (buf && bytes && data > 0)
	{
		char* out = (char*)buf;
		UINT d;
		double mul;
		int ofs;
		float * img;

		d = bytes / (bps >> 3);
		if (d > data) d = data;
		if (!d) return 0;
		data -= d;
		if (useFloat)
		{
			mul = 1.0;
			ofs = 0;
		}
		else
		{
			mul = (double)( (1 << ((bps) - 1)) - 1 );
			ofs = (bps == 8) ? 0x80 : 0;
		}
		wr += d * (bps >> 3);

		img = (float*)alloca(sizeof(float) * nch);
		do
		{
			UINT cur_ch;
			if (nch == 1 && src_nch > 0)
			{
				UINT c;
				img[0] = 0;
				for (c = 0;c < src_nch;c++)
				{
					img[0] += pcmbuf[c][pos];
				}
				img[0] /= (float)src_nch;
			}
			else if (nch == src_nch && !(nch == 6 && cfg_mc6_dm == 1))
			{
				UINT c;
				for (c = 0;c < nch;c++)
				{
					img[c] = pcmbuf[c][pos];
				}
			}
			else if (src_nch == 6)
			{
				UINT FL, FR, C;
				if (cfg_mc6_map == 1)
				{
					FL = 0;
					FR = 1;
					C = 2;
				}
				else
				{
					FL = 0;
					C = 1;
					FR = 2;
				}

				if (nch == 6)
				{ //remap order for correct 5.1 output
					img[0] = pcmbuf[FL][pos];
					img[1] = pcmbuf[FR][pos];
					img[2] = pcmbuf[C][pos];
					img[3] = pcmbuf[5][pos];
					img[4] = pcmbuf[3][pos];
					img[5] = pcmbuf[4][pos];
				}
				else if (nch == 2)
				{
					/*
					FL FR C  BL BR LFE
					0  1  2  3  4  5
					 
					L,C,R,SL,SR,LFE
					0 1 2 3  4  5
					 
					 
					output:
					FL FR C LFE BL BR
					 
					 
					stereo:
					Lt=L+0.707*(V-SL-SR+LFE)
					Rt=R+0.707*(C+SL+SR+LFE)
					 
					 
					Lt=L+0.707*(C+LFE)
					Rt=R+0.707*(C+LFE)
					SLt=SL 
					SRt=SR
					 
					*/
					if (cfg_mc6_dm == 4) //ds2
					{
						const double a = pow(10., 1.5 / 20.), b = 1 / a;
						img[0] = pcmbuf[FL][pos] + 0.707 * (pcmbuf[C][pos] - a * pcmbuf[3][pos] - b * pcmbuf[4][pos] + pcmbuf[5][pos]);
						img[1] = pcmbuf[FR][pos] + 0.707 * (pcmbuf[C][pos] + b * pcmbuf[3][pos] + a * pcmbuf[4][pos] + pcmbuf[5][pos]);
					}
					else
					{
						img[0] = pcmbuf[FL][pos] + 0.707 * (pcmbuf[C][pos] - pcmbuf[3][pos] - pcmbuf[4][pos] + pcmbuf[5][pos]);
						img[1] = pcmbuf[FR][pos] + 0.707 * (pcmbuf[C][pos] + pcmbuf[3][pos] + pcmbuf[4][pos] + pcmbuf[5][pos]);
					}
				}
				else if (nch == 4)
				{
					img[0] = pcmbuf[FL][pos] + 0.707 * (pcmbuf[C][pos] + pcmbuf[5][pos]);
					img[1] = pcmbuf[FR][pos] + 0.707 * (pcmbuf[C][pos] + pcmbuf[5][pos]);
					img[2] = pcmbuf[3][pos];
					img[3] = pcmbuf[4][pos];
				}
			}

			for (cur_ch = 0;cur_ch < nch;cur_ch++)
			{
				float v = img[cur_ch];
				int val;
				v *= scale;
				v *= mul;
				if (dither)
				{
					if (!shaper)
					{
						//Shaper(int freq,int _nch,int min,int max,int _dtype,int pdf,double noiseamp);
						shaper = new Shaper(sr, nch, clipmin, clipmax, 2, DITHER_TRIANGLE, 0);
					}
					//					double peak=0;
					val = shaper->do_shaping(v /*,&peak*/, cur_ch);
					//shaper clips for us
				}
				else
				{
					val = (int)v;
					if (val < clipmin) val = clipmin;
					else if (val > clipmax) val = clipmax;
					//1<<16 = 0x10000

				}
				val += ofs;

				switch (bps)
				{
				case 8:
					*(BYTE*)out = (UINT)val;
					break;
				case 16:
					*(short*)out = val;
					break;
				case 24:
					{
						((BYTE*)out)[0] = (UINT)val;
						((BYTE*)out)[1] = (UINT)val >> 8;
						((BYTE*)out)[2] = (UINT)val >> 16;
					}
					break;
				case 32:
					if (useFloat)
					{
						*(float *)out = v;
					}
					else
					{
						//*(long*)out=val;
						//break;
						*(long*)out = 0;
					}
					break;
				};
				out += (bps >> 3);
				d--;
			}
			pos++;
		}
		while (d);

	}
	return wr;
}

int VorbisFile::Seek(double p) { return ov_time_seek(&vf, p);}

int Decoder::Seek(double p)
{
	Flush();
	return file->Seek(p);
}

//char *vorbis_comment_query(vorbis_comment *vc, char *tag, int count)
const char* VorbisFile::get_meta(const char* tag, UINT c)
{
	return vorbis_comment_query(vf.seekable ? vf.vc + vf.current_link : vf.vc, (char*)tag, c);
}