#define INITGUID
#include <mmdeviceapi.h>
#undef INITGUID
#include <mmreg.h>

static WAVEFORMATEXTENSIBLE format_24_stereo =
{
	{
	WAVE_FORMAT_EXTENSIBLE,
	2,
	88200,
	705600,
	8,
	32,
	22,
	},
	24,
	SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT,
	KSDATAFORMAT_SUBTYPE_PCM
};

static WORD BitsPerSampleContainer(int bitspersamp) 
{
	if (bitspersamp <= 8) {
		return 8;
	} else if (bitspersamp <= 16) {
		return 16;
	} else if (bitspersamp <= 24) {
		return 24;
	} else {
		return 32;
	}
}

static WORD ChannelMask(int numchannels)
{
	switch(numchannels) {
		case 1:
			return SPEAKER_FRONT_CENTER;
		case 2:
			return SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT;
		case 4: // quadraphonic
			return SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
		case 6: // 5.1
			return SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY;
		default:
			return 0;
	}
}

WAVEFORMATEXTENSIBLE WaveFormatForParameters(int samplerate, int numchannels, int bitspersamp)
{
	WAVEFORMATEXTENSIBLE wave_format;
	wave_format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE; /* format type */
	wave_format.Format.nChannels = numchannels; /* number of channels (i.e. mono, stereo...) */
	wave_format.Format.nSamplesPerSec = samplerate; /* sample rate */
	wave_format.Format.nAvgBytesPerSec = BitsPerSampleContainer(bitspersamp) * numchannels * samplerate / 8; /* for buffer estimation */
	wave_format.Format.nBlockAlign = BitsPerSampleContainer(bitspersamp) * numchannels / 8; /* block size of data */
	wave_format.Format.wBitsPerSample = BitsPerSampleContainer(bitspersamp);
	wave_format.Format.cbSize = 22;  /* the count in bytes of the size of */
                                    /* extra information (after cbSize) */
    
	wave_format.Samples.wValidBitsPerSample = bitspersamp;

	wave_format.dwChannelMask = ChannelMask(numchannels);
 	wave_format.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
 
	return wave_format;
}