#ifndef NULLSOFT_IN_FLAC_STREAMFILEWIN32_H
#define NULLSOFT_IN_FLAC_STREAMFILEWIN32_H

#include <FLAC/all.h>
#include <windows.h>

struct Win32_State
{
	void *userData;
	HANDLE handle;
	bool endOfFile;	
};

FLAC__StreamDecoderReadStatus Win32_Read(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data);
FLAC__StreamDecoderSeekStatus Win32_Seek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
FLAC__StreamDecoderTellStatus Win32_Tell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
FLAC__StreamDecoderLengthStatus Win32_Length(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data);
FLAC__bool Win32_EOF(const FLAC__StreamDecoder *decoder, void *client_data);

// helper function extern'd here because DecodeThread needs it
__int64 FileSize64(HANDLE file);

#endif