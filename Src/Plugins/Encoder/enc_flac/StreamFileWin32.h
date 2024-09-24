#pragma once

#include <FLAC/all.h>
#include <windows.h>

struct Win32_State
{
public:
	HANDLE handle;
	FLAC__uint64 bytesWritten;
};

FLAC__StreamEncoderWriteStatus Win32_Write(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data);
FLAC__StreamEncoderSeekStatus Win32_Seek(const FLAC__StreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data);
FLAC__StreamEncoderTellStatus Win32_Tell(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
void Progress(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);


