#include "StreamFileWin32.h"
#include <assert.h>

static __int64 Seek64(HANDLE hf, __int64 distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;

	li.QuadPart = distance;

	li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}


FLAC__StreamEncoderWriteStatus Win32_Write(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
{
	assert(bytes <= 4294967295U);

	HANDLE file = ((Win32_State *)client_data)->handle;
	if(bytes > 0) 
	{
		assert(sizeof(FLAC__byte) == 1);
		DWORD bytesWritten = 0, bytesToWrite = (DWORD)bytes;
		BOOL result = WriteFile(file, buffer, bytesToWrite, &bytesWritten, NULL);
		((Win32_State *)client_data)->bytesWritten += bytesWritten;

		if (!result)
			return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
		return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
	}
	else
		return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
}

FLAC__StreamEncoderSeekStatus Win32_Seek(const FLAC__StreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	HANDLE file = ((Win32_State *)client_data)->handle;

	__int64 result = Seek64(file, absolute_byte_offset, FILE_BEGIN);

	if (result == INVALID_SET_FILE_POINTER)
		return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
	else
	{
		return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
	}
}

FLAC__StreamEncoderTellStatus Win32_Tell(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	HANDLE file = ((Win32_State *)client_data)->handle;

	__int64 position = Seek64(file, 0, FILE_CURRENT);

	if (position == INVALID_SET_FILE_POINTER)
		return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
	else
	{
		*absolute_byte_offset=position;
		return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
	}
}

