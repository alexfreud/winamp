#include"mkv_flac_decoder.h"
#include "main.h"

static FLAC__StreamDecoderReadStatus Packet_Read(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	packet_client_data_t packet = (packet_client_data_t)client_data;
	size_t to_copy = *bytes;
	if (to_copy > packet->buffer_length) {
		to_copy = packet->buffer_length;
	}
	memcpy(buffer, packet->buffer, to_copy);
	*bytes = to_copy;
	packet->buffer += to_copy;
	packet->buffer_length -= to_copy;
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static FLAC__StreamDecoderSeekStatus Packet_Seek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	return FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED;
}

static FLAC__StreamDecoderTellStatus Packet_Tell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
}

static FLAC__StreamDecoderLengthStatus Packet_Length(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
}

static FLAC__bool Packet_EOF(const FLAC__StreamDecoder *decoder, void *client_data)
{
	return 0;
}

static void OnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//client_data=client_data; // dummy line so i can set a breakpoint
}

static void OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	packet_client_data_t packet = (packet_client_data_t)client_data;
	switch(metadata->type)
	{
	case FLAC__METADATA_TYPE_STREAMINFO:
		{
			packet->frame_size = metadata->data.stream_info.max_blocksize;
			packet->bps=metadata->data.stream_info.bits_per_sample;
			packet->bytes_per_sample = (packet->bps + 7) / 8;
			packet->channels=metadata->data.stream_info.channels;
			packet->sample_rate=metadata->data.stream_info.sample_rate;
			packet->samples=metadata->data.stream_info.total_samples;
		}
		break;
	}
}

static FLAC__StreamDecoderWriteStatus OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	packet_client_data_t packet = (packet_client_data_t)client_data;

	size_t byteLength = packet->bytes_per_sample * packet->channels * frame->header.blocksize;
	if (byteLength > packet->outputBufferBytes[0]) {
		FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	InterleaveAndTruncate(buffer, packet->outputBuffer, packet->bytes_per_sample * 8, packet->channels, frame->header.blocksize);
	packet->outputBufferBytes[0] = byteLength;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


MKVFLACDecoder *MKVFLACDecoder::Create(const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels)
{
	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if (!decoder) {
		return 0;
	}
	packet_client_data_t packet = new packet_client_data_s;
	packet->buffer = 0;
	packet->buffer_length = 0;

	if(FLAC__stream_decoder_init_stream(
		decoder,
		Packet_Read,
		Packet_Seek,
		Packet_Tell,
		Packet_Length,
		Packet_EOF,  
		OnAudio,
		OnMetadata,  
		OnError,
		packet
		) != FLAC__STREAM_DECODER_INIT_STATUS_OK) 
	{
		delete packet;
		FLAC__stream_decoder_delete(decoder);
		return 0;	
	}

	packet->buffer = (const uint8_t *)track_entry_data->codec_private;
	packet->buffer_length = track_entry_data->codec_private_len;
	if (!FLAC__stream_decoder_process_until_end_of_metadata(decoder)) {
		delete packet;
		FLAC__stream_decoder_delete(decoder);
		return 0;	
	}

	MKVFLACDecoder *mkv_decoder = new MKVFLACDecoder(decoder, packet, preferred_bits);
	if (!mkv_decoder) {
		delete packet;
		FLAC__stream_decoder_delete(decoder);
		return 0;	
	}
	return mkv_decoder;
}

MKVFLACDecoder::MKVFLACDecoder(FLAC__StreamDecoder *decoder, packet_client_data_t packet, unsigned int bps)
: decoder(decoder), packet(packet), bps(bps)
{	
}

MKVFLACDecoder::~MKVFLACDecoder()
{
	delete packet;
	FLAC__stream_decoder_delete(decoder);
}

int MKVFLACDecoder::OutputFrameSize(size_t *frame_size)
{
	*frame_size = packet->frame_size * packet->bytes_per_sample * packet->channels;
	return MKV_SUCCESS;
}

int MKVFLACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	*sampleRate = packet->sample_rate;
	*channels = packet->channels;
	*bitsPerSample = packet->bps;
	*isFloat = false;

	return MKV_SUCCESS;
}

int MKVFLACDecoder::DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	packet->buffer = (const uint8_t *)inputBuffer;
	packet->buffer_length = inputBufferBytes;

	packet->outputBuffer = outputBuffer;
	packet->outputBufferBytes = outputBufferBytes;

	if (FLAC__stream_decoder_process_single(decoder) == 0) {
		return MKV_FAILURE;
	}

	return MKV_SUCCESS;
}

void MKVFLACDecoder::Flush()
{
	FLAC__stream_decoder_flush(decoder);
}

void MKVFLACDecoder::Close()
{
	delete this;
}

#define CBCLASS MKVFLACDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS


int MKVDecoder::CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels,bool floating_point, ifc_mkvaudiodecoder **decoder)
{
	if (!strcmp(codec_id, "A_FLAC"))
	{
		MKVFLACDecoder *flac_decoder = MKVFLACDecoder::Create(track_entry_data, audio_data, preferred_bits, max_channels);
		if (flac_decoder)
		{
			*decoder = flac_decoder;
			return CREATEDECODER_SUCCESS;
		}
		return CREATEDECODER_FAILURE;
	}

	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS MKVDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS