/*
** Copyright (C) 2007-2012 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/
#include <windows.h>
#include <FLAC/all.h>
#include <assert.h>
#include "StreamFileWin32.h"

FLAC__StreamDecoderReadStatus Win32_Read(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	assert(*bytes <= 4294967295U);

	HANDLE file = ((Win32_State *)client_data)->handle;
	if(*bytes > 0) 
	{
		assert(sizeof(FLAC__byte) == 1);
		DWORD bytesRead=0, bytesToRead=*bytes;
		BOOL result = ReadFile(file, buffer, bytesToRead, &bytesRead, NULL);
		*bytes = bytesRead;

		if (!result)
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
		else if(bytesRead == 0)
		{
			((Win32_State *)client_data)->endOfFile = true;
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		}
		else
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	}
	else
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}

__int64 Seek64(HANDLE hf, __int64 distance, DWORD MoveMethod)
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

FLAC__StreamDecoderSeekStatus Win32_Seek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	HANDLE file = ((Win32_State *)client_data)->handle;

	__int64 result = Seek64(file, absolute_byte_offset, FILE_BEGIN);

	if (result == INVALID_SET_FILE_POINTER)
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	else
	{
		((Win32_State *)client_data)->endOfFile = false;
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	}
}

FLAC__StreamDecoderTellStatus Win32_Tell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	HANDLE file = ((Win32_State *)client_data)->handle;

	__int64 position = Seek64(file, 0, FILE_CURRENT);

	if (position == INVALID_SET_FILE_POINTER)
		return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
	else
	{
		*absolute_byte_offset=position;
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}
}
__int64 FileSize64(HANDLE file)
{
	LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(file, (LPDWORD)&position.HighPart); 	
	
	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return INVALID_FILE_SIZE;
	else
		return position.QuadPart;
}

FLAC__StreamDecoderLengthStatus Win32_Length(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	HANDLE file = ((Win32_State *)client_data)->handle;

	LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(file, (LPDWORD)&position.HighPart); 	
	
	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	else
	{
		*stream_length = position.QuadPart;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}
}

FLAC__bool Win32_EOF(const FLAC__StreamDecoder *decoder, void *client_data)
{
	return  ((Win32_State *)client_data)->endOfFile;
}