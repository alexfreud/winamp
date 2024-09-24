#include "mkv_vorbis_decoder.h"
#include "../nsmkv/Lacing.h"
#include "../nsmkv/Cluster.h"
#include <math.h>


int MKVDecoderCreator::CreateAudioDecoder(const char *codec_id, 
																					const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data,
																					unsigned int preferred_bits, unsigned int max_channels, bool floating_point,
																					ifc_mkvaudiodecoder **decoder)
{
	if (!strcmp(codec_id, "A_VORBIS"))
	{
		MKVVorbis *vorbis = new MKVVorbis;
		vorbis_info_init(&vorbis->info);
		vorbis_comment_init(&vorbis->comment);
		nsmkv::LacingState lacing_state;
		if (nsmkv::Lacing::GetState(nsmkv::BlockBinary::XIPH_LACING, (const uint8_t *)track_entry_data->codec_private, track_entry_data->codec_private_len, &lacing_state))
		{
			const uint8_t *frame;
			size_t frame_len;
			uint16_t frame_number=0;
			while (nsmkv::Lacing::GetFrame(frame_number, (const uint8_t *)track_entry_data->codec_private, track_entry_data->codec_private_len, &frame, &frame_len, &lacing_state))
			{
				ogg_packet packet = {const_cast<uint8_t *>(frame), (long)frame_len, (frame_number==0), 0, 0 /*-1?*/, vorbis->packet_number++};
				int ret = vorbis_synthesis_headerin(&vorbis->info, &vorbis->comment, &packet);
				if (ret != 0)
					goto bail;
				frame_number++;
			}
			if (vorbis_synthesis_init(&vorbis->dsp, &vorbis->info) == 0
				&& vorbis_block_init(&vorbis->dsp, &vorbis->block) == 0)
			{
				vorbis->bps = preferred_bits?preferred_bits:16;
				*decoder = vorbis;
				return CREATEDECODER_SUCCESS;
			}
		}

bail:
		delete vorbis;
		return CREATEDECODER_FAILURE;
	}

	return CREATEDECODER_NOT_MINE;
}


#define CBCLASS MKVDecoderCreator
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

MKVVorbis::MKVVorbis()
{
	bps=16;
	packet_number=0;
}

#define PA_CLIP_( val, min, max )\
	{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

#if defined(_M_IX86)
static __inline long float_to_long(double t)
{
	long r;
	__asm fld t
	__asm fistp r
	return r;
}
#else
#define float_to_long(x) ((long)( x ))
#endif

inline static void clip(double &x, double a, double b)
{
	double x1 = fabs (x - a);
	double x2 = fabs (x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
}

static void Float32_To_Int24_Clip(void *destinationBuffer, void *sourceBuffer, size_t count, size_t channels, double gain)
{
	float *src = (float*)sourceBuffer;
	unsigned char *dest = (unsigned char*)destinationBuffer;
	gain*=65536.*32768.;
	while ( count-- )
	{
		/* convert to 32 bit and drop the low 8 bits */
		double scaled = *src * gain;
		clip( scaled, -2147483648., 2147483647.);
		signed long temp = (signed long) scaled;

		dest[0] = (unsigned char)(temp >> 8);
		dest[1] = (unsigned char)(temp >> 16);
		dest[2] = (unsigned char)(temp >> 24);

		src++;
		dest += 3*channels;
	}
}

static void Float32_To_Int16_Clip(void *destinationBuffer, void *sourceBuffer, size_t count, size_t channels, double gain)
{
	float *src = (float*)sourceBuffer;
	signed short *dest = (signed short*)destinationBuffer;

	gain*=32768.0;
	while ( count-- )
	{
		long samp = float_to_long((*src) * gain/* - 0.5*/);

		PA_CLIP_( samp, -0x8000, 0x7FFF );
		*dest = (signed short) samp;

		src ++;
		dest += channels;
	}
}

int MKVVorbis::DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	uint8_t *out = (uint8_t *)outputBuffer;
	ogg_packet packet = {(uint8_t *)inputBuffer, (long)inputBufferBytes, 0, 0, 0 -1, packet_number++};
	int ret = vorbis_synthesis(&block, &packet);
	if (ret == 0)
	{
		vorbis_synthesis_blockin(&dsp,&block);
		long channels = info.channels;
		float **pcm;
		int samples = vorbis_synthesis_pcmout(&dsp, &pcm);
		if (samples)
		{
			switch(bps)
			{
			case 16:
				for(int i=0;i<channels;i++)
				{
					Float32_To_Int16_Clip(out, pcm[i], samples, channels, 1.0 /*gain*/);
					out+=2;
				}
				break;
			case 24:
			for(int i=0;i<channels;i++)
				{
					Float32_To_Int24_Clip(out, pcm[i], samples, channels, 1.0 /*gain*/);
					out+=3;
				}
				break;
			}
		}
		*outputBufferBytes = samples*channels*bps/8;
		// let the decoder know we're processed them
		vorbis_synthesis_read(&dsp,samples);
		return MKV_SUCCESS;
	}
	return MKV_FAILURE;
}

int MKVVorbis::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	*sampleRate = info.rate;
	*channels = info.channels;
	*bitsPerSample = bps;
	*isFloat = false; // TODO
	return MKV_SUCCESS;
}

void MKVVorbis::Flush()
{
	vorbis_synthesis_restart(&dsp);
}

void MKVVorbis::Close()
{
	// TODO: benski> verify
	vorbis_info_clear(&info);
  vorbis_comment_clear(&comment);
	vorbis_dsp_clear(&dsp);
  vorbis_block_clear(&block);
	delete this;
}
#define CBCLASS MKVVorbis
START_DISPATCH;
//CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS