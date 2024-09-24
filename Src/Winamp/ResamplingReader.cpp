/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "ResamplingReader.h"

ResamplingReader::ResamplingReader(Resampler *_resampler, CommonReader *_reader, size_t inputFrameSize)
		: resampler(_resampler), reader(_reader),
		bufferValid(0),
		readState(READING)
{
	bufferAlloc = inputFrameSize * 1024; // enough room for 1024 samples
	buffer = (__int8 *)calloc(bufferAlloc, sizeof(__int8));
}

ResamplingReader::~ResamplingReader()
{
	free(buffer);
	delete resampler;
	delete reader;
}

size_t ResamplingReader::ReadAudio(void *outputBuffer, size_t sizeBytes)
{
	size_t origSize = sizeBytes;
	__int8 *origBuffer = (__int8 *)outputBuffer;
	size_t bytesResampled = 0;
read_again:
	// First, read from the file decoder
	switch (readState)
	{
	case READING:
		{
			size_t bytesToRead = bufferAlloc - bufferValid;
			if (bytesToRead)
			{
				int decode_killswitch=0, decode_error;
				size_t bytesRead = reader->ReadAudio(buffer + (bufferAlloc - bytesToRead), bytesToRead, &decode_killswitch, &decode_error);
				bufferValid += bytesRead;
				if (bytesRead == 0)
				{
					readState = ENDOFFILE;
				}
			}
		}
		break;
	case ENDOFFILE:
		resampler->Flush();
		readState = FLUSHING;

	}

	// now, resample
	size_t inputBytes = bufferValid;
	size_t bytesDone;

	bytesDone = resampler->Convert(buffer, &inputBytes, outputBuffer, sizeBytes);
	bytesResampled += bytesDone;

	// if we didn't use all of our input buffer, then we'll copy what's left to the beginning
	if (inputBytes)
		memmove(buffer, buffer + (bufferValid - inputBytes), inputBytes);

	// mark the number of bytes of data still valid in the input buffer
	bufferValid = inputBytes;

	if (!bytesDone && readState == FLUSHING)
		readState = DONE;

	if (bytesResampled != origSize && readState != DONE) // if we didn't provide enough data to fill the output buffer
	{
		sizeBytes = origSize - bytesResampled;
		outputBuffer = origBuffer + bytesResampled;
		goto read_again;
	}



	return bytesResampled;

}
#define CBCLASS ResamplingReader
START_DISPATCH;
CB(IFC_AUDIOSTREAM_READAUDIO, ReadAudio)
END_DISPATCH;
