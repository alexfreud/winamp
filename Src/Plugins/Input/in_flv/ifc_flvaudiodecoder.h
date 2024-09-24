#pragma once

enum
{
	FLV_AUDIO_SUCCESS = 0,
	FLV_AUDIO_FAILURE = 1,
	FLV_AUDIO_NEEDS_MORE_INPUT = 2,
};

class ifc_flvaudiodecoder : public Dispatchable
{
protected:
	ifc_flvaudiodecoder() {}
	~ifc_flvaudiodecoder() {}
public:
	int GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *bits);
	int DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate);
	void Flush();
	void Close();
	int Ready(); // returns 1 for ready [default], 0 for not ready.  Some codecs in FLV use the first packet for decoder config data.  return 1 from this once you've gotten it
	void SetPreferences(unsigned int max_channels, unsigned int preferred_bits);
	DISPATCH_CODES
	{
		FLV_AUDIO_GETOUTPUTFORMAT = 0,
			FLV_AUDIO_DECODE = 1,
			FLV_AUDIO_FLUSH = 2,
			FLV_AUDIO_CLOSE = 3,
			FLV_AUDIO_READY = 4,
			FLV_AUDIO_SETPREFERENCES=5,
	};
};

inline int ifc_flvaudiodecoder::GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *bits)
{
	return _call(FLV_AUDIO_GETOUTPUTFORMAT, (int)FLV_AUDIO_FAILURE, sample_rate, channels, bits);
}

inline int ifc_flvaudiodecoder::DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate)
{
	return _call(FLV_AUDIO_DECODE, (int)FLV_AUDIO_FAILURE, input_buffer, input_buffer_bytes, samples, samples_size_bytes, bitrate);
}

inline void ifc_flvaudiodecoder::Flush()
{
	_voidcall(FLV_AUDIO_FLUSH);
}

inline void ifc_flvaudiodecoder::Close()
{
	_voidcall(FLV_AUDIO_CLOSE);
}

inline int ifc_flvaudiodecoder::Ready()
{
	return _call(FLV_AUDIO_READY, (int)1); // default to true so that decoders that don't implement won't block in_flv from seeking
}

inline void ifc_flvaudiodecoder::SetPreferences(unsigned int max_channels, unsigned int preferred_bits)
{
	_voidcall(FLV_AUDIO_SETPREFERENCES, max_channels, preferred_bits);
}

