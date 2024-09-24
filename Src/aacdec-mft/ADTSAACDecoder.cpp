#include "ADTSAACDecoder.h"
#include <nsaac/ADTSHeader.h>
#include "../nsutil/pcm.h"
#include <bfc/error.h>
#include <Mferror.h>

ADTSAACDecoder::ADTSAACDecoder()
{
	predelay = 0;
	useFloat = false; /* we'll fix during Initialize */
	gain=1.0f; /* we'll fix during Initialize */
	// get bps
	bitsPerSample = 16; /* we'll fix during Initialize */
	allowRG = false; /* we'll fix during Initialize */
}

int ADTSAACDecoder::Initialize(bool forceMono, bool reverse_stereo, bool allowSurround, int maxBits, bool _allowRG, bool _useFloat, bool _useCRC)
{
	allowRG = _allowRG;
	useFloat = _useFloat;

	if (_useFloat) {
		bitsPerSample = 32;
	} else if (maxBits >= 24) {
		bitsPerSample = 24;
	} else {
		bitsPerSample = 16;
	}

	decoder.Open();
	return adts::SUCCESS;
}

bool ADTSAACDecoder::Open(ifc_mpeg_stream_reader *file)
{
	if (allowRG)
		gain = file->MPEGStream_Gain();

	return true;
}

void ADTSAACDecoder::Close()
{
}

void ADTSAACDecoder::GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate)
{
	uint32_t local_sample_rate, local_channels;
	decoder.GetOutputProperties(&local_sample_rate, &local_channels);

	*sampleRate = local_sample_rate;
	*numChannels = local_channels;
	*numBits = bitsPerSample;
}

void ADTSAACDecoder::CalculateFrameSize(int *frameSize)
{
	size_t local_frame_size;
	if (SUCCEEDED(decoder.OutputBlockSizeSamples(&local_frame_size))) {
		*frameSize = (int)local_frame_size;
	} else {
		*frameSize = 0;
	}
}

void ADTSAACDecoder::Flush(ifc_mpeg_stream_reader *file)
{
	decoder.Flush();
}

size_t ADTSAACDecoder::GetCurrentBitrate()
{
	return 0;  // TODO?
}

size_t ADTSAACDecoder::GetDecoderDelay()
{
	return predelay;
}

static int ADTSSync(const uint8_t *buffer, size_t bytes_in_buffer, size_t *header_position)
{
	for (size_t position=0;position<bytes_in_buffer;position++)
	{
		// find POTENTIAL sync
		if (buffer[position] == 0xFF && bytes_in_buffer - position >= 7)
		{
			ADTSHeader header;
			if (nsaac_adts_parse(&header, &buffer[position]) == NErr_Success)
			{
				int frame_length = (int)header.frame_length;
				if (frame_length && bytes_in_buffer - position - frame_length >= 7)
				{
					ADTSHeader header2;
					if (nsaac_adts_parse(&header2, &buffer[position+frame_length]) == NErr_Success)
					{
						// verify that parameters match
						if (nsaac_adts_match(&header, &header2) != NErr_True)
							return NErr_Changed;

						// do a dummy read to advance the stream
						*header_position = position;
						return NErr_Success;
					}
				}
				else
				{
					/* not enough in the buffer to verify the next header */
					*header_position = position;
					return NErr_NeedMoreData;
				}
			}
		}
	}
	return NErr_False;	
}

static int ReturnIsEOF(ifc_mpeg_stream_reader *file)
{
	if (file->MPEGStream_EOF())
		return adts::ENDOFFILE;
	else
		return adts::NEEDMOREDATA;
}

int ADTSAACDecoder::Internal_Decode(ifc_mpeg_stream_reader *file, 
									const void *input, size_t input_length,
									unsigned __int8 *output, size_t outputSize, size_t *outputWritten)
{
	*outputWritten = outputSize;
	if (SUCCEEDED(decoder.Feed(input, input_length))) {
		HRESULT hr = decoder.Decode(output, outputWritten, bitsPerSample, useFloat, gain);
		if (SUCCEEDED(hr)) {
			if (*outputWritten == 0) {
				return adts::NEEDMOREDATA;
			}
			return adts::SUCCESS;
		} else if (hr == MF_E_TRANSFORM_STREAM_CHANGE) {
			*outputWritten = 0;
			return adts::SUCCESS;
		}
	}
	return adts::FAILURE;
}

int ADTSAACDecoder::Sync(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate)
{
	/* ok this will be interesting.  we'll peek from the input buffer and try to synchronize on an ADTS header */
	uint8_t peek_buffer[16384];
	size_t bytes_read = 0;
	if (file->MPEGStream_Peek(peek_buffer, sizeof(peek_buffer), &bytes_read) != 0)
	{
		return adts::FAILURE;
	}

	size_t header_position=0;
	int ret = ADTSSync(peek_buffer, bytes_read, &header_position);
	if (ret == NErr_NeedMoreData)
	{ 
		// this one means we found one sync but not enough to verify the next frame

		// if the header was at the start of the block, then unfortunately this might be the LAST adts frame in the file, so let's just pass it the decoder and hope for the best
		if (header_position != 0) 
		{
			if (file->MPEGStream_EOF())
				return adts::ENDOFFILE;
	
			/* dummy read to advance the stream */
			file->MPEGStream_Read(peek_buffer, header_position, &header_position);
			return adts::NEEDMOREDATA;
		}
	}
	else if (ret == NErr_False)
	{
		if (file->MPEGStream_EOF())
			return adts::ENDOFFILE;

		// not even a potential sync found
		/* dummy read to advance the stream */
		file->MPEGStream_Read(peek_buffer, bytes_read, &bytes_read);
		return adts::NEEDMOREDATA;
	}
	else if (ret != NErr_Success)
	{
		if (file->MPEGStream_EOF())
			return adts::ENDOFFILE;

		return adts::FAILURE;
	}

	ADTSHeader header;
	if (nsaac_adts_parse(&header, &peek_buffer[header_position]) == NErr_Success)
	{
		/* all this error checking might be uncessary, since in theory we did a successful peek above.  but you never know ... */
		if (file->MPEGStream_Read(peek_buffer, header_position, &bytes_read))	/* dummy read to advance the stream */
			return adts::FAILURE;

		if (bytes_read != header_position)
			return ReturnIsEOF(file);

		if (file->MPEGStream_Read(peek_buffer, header.frame_length, &bytes_read)) /* read ADTS frame */
			return adts::FAILURE;

		if (bytes_read != header.frame_length) 
			return ReturnIsEOF(file);

		if (bytes_read < 7) /* bad header data? */
			return adts::FAILURE;

		/* ok, we've created the decoder, but we should really decode the frame to see if there's VBR, PS or MPEGS in it */
		size_t header_size = nsaac_adts_get_header_size(&header);

		int ret = Internal_Decode(file, peek_buffer, bytes_read, output, outputSize, outputWritten);
		if (ret == adts::SUCCESS && *outputWritten == 0) {
			return adts::NEEDMOREDATA;
		}
		return ret;
	}
	return adts::FAILURE;
}

int ADTSAACDecoder::Decode(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut)
{
	HRESULT hr = decoder.Decode(output, outputWritten, bitsPerSample, useFloat, gain);
	if (SUCCEEDED(hr) && *outputWritten) {
		return adts::SUCCESS;
	}

	uint8_t peek_buffer[8192];
	size_t bytes_read = 0;

	file->MPEGStream_Peek(peek_buffer, 7, &bytes_read);
	if (bytes_read != 7)
		return ReturnIsEOF(file);

	ADTSHeader header;
	if (nsaac_adts_parse(&header, peek_buffer) == NErr_Success)
	{
		if (header.frame_length < 7)
			return adts::FAILURE;

		file->MPEGStream_Peek(peek_buffer, header.frame_length, &bytes_read);
		if (bytes_read != header.frame_length)
			return ReturnIsEOF(file);
		file->MPEGStream_Read(peek_buffer, header.frame_length, &bytes_read);

		size_t header_size = nsaac_adts_get_header_size(&header);
		*bitrate = nsaac_adts_get_frame_bitrate(&header)/1000;
		return Internal_Decode(file, peek_buffer, bytes_read, output, outputSize, outputWritten);
	}
	else
	{
		/* Resynchronize */
		return Sync(file, output, outputSize, outputWritten, bitrate);
	}
}

int ADTSAACDecoder::GetLayer()
{
	return 4;
}

void ADTSAACDecoder::Release()
{
	delete this;
}