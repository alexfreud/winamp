#include "MP3Coder.h"

AudioCoderMP3::~AudioCoderMP3()
{
	if (hbeStream) beCloseStream(hbeStream); hbeStream = 0;
	if (bs) free(bs); bs = 0;
}

int AudioCoderMP3::GetLastError() { return m_err; };

void AudioCoderMP3::setVbrFilename(char *filename)
{
	if (hbeStream) beCloseStream(hbeStream);
	hbeStream = 0;
	if (filename)
	{
		beWriteVBRHeader(filename);
	}
}

int AudioCoderMP3::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
		if (m_err) return -1;
	if (!hbeStream)
	{
		if (beInitStream(&beConfig, &ibuf_size_spls, &obuf_size, (PHBE_STREAM) &hbeStream) != BE_ERR_SUCCESSFUL)
		{
			m_err++;
			return -1;
		}
		ibuf_size = ibuf_size_spls * bytesPerSample;

		if (is_downmix) ibuf_size *= 2;

		bs = (char*)malloc(ibuf_size);
		bs_size = 0;
	}

	
	*in_used = 0;

	int needbytes = ibuf_size - bs_size;
	if (needbytes > 0 && in_avail > 0)
	{
		if (needbytes > in_avail)
			needbytes = in_avail;
		memcpy(bs + bs_size, in, needbytes);
		bs_size += needbytes;
		*in_used = needbytes;
	}
	if (!done)
	{
		if (bs_size < (int)ibuf_size) return 0;

	}

	if (out_avail < (int)obuf_size) return 0;

	int feedBytes = min(bs_size, (int)ibuf_size);
	int feedSamples = feedBytes / bytesPerSample;
	bs_size -= feedBytes;

	if (is_downmix)
	{
		int x;
		int l = feedBytes / 4;
		short *b = (short*)bs;
		for (x = 0; x < l; x ++)
		{
			b[x] = b[x * 2] / 2 + b[x * 2 + 1] / 2;
		}
		feedSamples/=2;
	}
	DWORD dwo = 0;

	if (feedSamples)
	{
		if (beEncodeChunk(hbeStream, feedSamples, (short*)bs, (unsigned char*)out, &dwo) != BE_ERR_SUCCESSFUL)
		{
			m_err++;
			return -1;
		}
	} 
	if (!dwo && done==1)
	{
		if (beDeinitStream(hbeStream, (unsigned char *)out, &dwo) != BE_ERR_SUCCESSFUL)
		{
			m_err++;
			return -1;
		}
		done++;
	}

	return dwo;
}

void AudioCoderMP3::PrepareToFinish()
{
	done = 1;
}

AudioCoderMP3::AudioCoderMP3(int nch, int srate, int bps, configtype *cfg)
		: obuf_size(0), ibuf_size(0), ibuf_size_spls(0), done(false), bs_size(0)
{
	m_err = 0;
	hbeStream = 0;
	bs = 0;
	is_downmix = 0;
	mono_input=0;

	memset(&beConfig, 0, sizeof(beConfig));					// clear all fields

	if (srate != 32000 
		&& srate != 44100 
			&& srate != 48000 
			&& srate != 16000  /* MPEG 2 sample rates */
			&& srate != 22050 
			&& srate != 24000
			&& srate != 11025 /* MPEG 2.5 sample rates */
			&& srate != 12000
			&& srate != 8000)
	{
		//MessageBox(NULL, "The only valid audio sampling rates for the LAME mp3 encoder are:\r\t16000\r\t22050\r\t24000\r\t32000\r\t44100\r\t48000\r\rPlease modify the encoding profile to use one of these sampling rates, and then try again.", "Invalid sampling rate", MB_OK);
		m_err++;
		return ;
	}

	// use the LAME config structure
	beConfig.dwConfig = BE_CONFIG_LAME;

	// this are the default settings for testcase.wav
	beConfig.format.LHV1.dwStructVersion	= 1;
	beConfig.format.LHV1.dwStructSize	= sizeof(beConfig);
	beConfig.format.LHV1.dwSampleRate	= srate;							// INPUT FREQUENCY

	beConfig.format.LHV1.nMode = cfg->stereo_mode;
	if (nch < 2) 
	{
		beConfig.format.LHV1.nMode = BE_MP3_MODE_MONO;
		mono_input=1;
	}
	else
	{
		if (beConfig.format.LHV1.nMode == BE_MP3_MODE_MONO)
			is_downmix = 1;
		//   beConfig.format.LHV1.nMode = BE_MP3_MODE_STEREO;
	}

	beConfig.format.LHV1.dwBitrate = cfg->bitrate;
	beConfig.format.LHV1.dwMaxBitrate = (cfg->vbr_method != -1 ? cfg->vbr_max_bitrate : cfg->bitrate);

	beConfig.format.LHV1.dwReSampleRate	= 0; // DOWNSAMPLERATE, 0=ENCODER DECIDES

	if (beConfig.format.LHV1.dwMaxBitrate < 32) // less than 32, let's force mono
	{
		if (nch > 1)
		{
			beConfig.format.LHV1.nMode = BE_MP3_MODE_MONO;
			is_downmix = 1;
		}
	}

	/*int effective_nch = (beConfig.format.LHV1.nMode == BE_MP3_MODE_MONO) ? 1 : 2;

	if (beConfig.format.LHV1.dwReSampleRate >= 32000 &&
	    beConfig.format.LHV1.dwMaxBitrate / effective_nch <= 32)
	{
		beConfig.format.LHV1.dwReSampleRate /= 2;
	}*/
/*
	if (beConfig.format.LHV1.dwReSampleRate < 32000)
		beConfig.format.LHV1.dwMpegVersion	= MPEG2;							// MPEG VERSION (I or II)
	else
		beConfig.format.LHV1.dwMpegVersion	= MPEG1;							// MPEG VERSION (I or II)
*/

	beConfig.format.LHV1.nPreset = cfg->quality;

	beConfig.format.LHV1.dwPsyModel	= 0;									// USE DEFAULT PSYCHOACOUSTIC MODEL
	beConfig.format.LHV1.dwEmphasis	= 0;									// NO EMPHASIS TURNED ON

	beConfig.format.LHV1.bOriginal	= 0;		// SET ORIGINAL FLAG
	beConfig.format.LHV1.bCRC	= 0;			// INSERT CRC
	beConfig.format.LHV1.bCopyright	= 0;		// SET COPYRIGHT FLAG
	beConfig.format.LHV1.bPrivate	= 0;		// SET PRIVATE FLAG
	beConfig.format.LHV1.bNoRes	= 0;	
	beConfig.format.LHV1.bWriteVBRHeader = 1;

	beConfig.format.LHV1.bEnableVBR = cfg->vbr_method != VBR_METHOD_NONE;
	beConfig.format.LHV1.nVBRQuality = cfg->vbr;
	//beConfig.format.LHV1.dwVbrAbr_bps = (cfg->vbr_method != VBR_METHOD_NONE ? 1000 * cfg->abr_bitrate : 0);
	beConfig.format.LHV1.dwVbrAbr_bps = (cfg->vbr_method ==  VBR_METHOD_ABR ? 1000 * cfg->abr_bitrate : 0);
	beConfig.format.LHV1.nVbrMethod = (VBRMETHOD)cfg->vbr_method;

	bytesPerSample = bps / 8;
}