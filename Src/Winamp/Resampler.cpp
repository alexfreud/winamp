/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "Resampler.h"
#include <ks.h>
#include <ksmedia.h>

static int GetChannelMask(size_t channels)
{
	switch(channels)
	{
	case 1: return KSAUDIO_SPEAKER_MONO;
	case 2: return KSAUDIO_SPEAKER_STEREO;
	case 4: return KSAUDIO_SPEAKER_SURROUND;
	case 6: return KSAUDIO_SPEAKER_5POINT1;
	case 8: return KSAUDIO_SPEAKER_7POINT1;
	default: return 0;
	}
}
static void FillWFX_float(WAVEFORMATIEEEFLOATEX *format, size_t channels, size_t bits, size_t sampleRate)
{
	ZeroMemory(format, sizeof(WAVEFORMATIEEEFLOATEX));
	size_t padded_bits = (bits + 7) & (~7);
	format->Format.nChannels = (WORD)channels;
	format->Format.nSamplesPerSec = (DWORD)sampleRate;
	format->Format.nAvgBytesPerSec = (DWORD)(sampleRate * channels * (padded_bits >> 3));
	format->Format.nBlockAlign = (WORD)(channels * (padded_bits >> 3));
	format->Format.wBitsPerSample = (WORD)padded_bits;

	format->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	format->Format.cbSize = sizeof(WAVEFORMATIEEEFLOATEX) - sizeof(WAVEFORMATEX);
	format->Samples.wValidBitsPerSample = (WORD)bits;
	format->dwChannelMask = GetChannelMask(channels);
	format->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
}

static void FillWFX(WAVEFORMATEXTENSIBLE *format, size_t channels, size_t bits, size_t sampleRate)
{
	ZeroMemory(format, sizeof(WAVEFORMATEXTENSIBLE));
	size_t padded_bits = (bits + 7) & (~7);
	format->Format.nChannels = (WORD)channels;
	format->Format.nSamplesPerSec = (DWORD)sampleRate;
	format->Format.nAvgBytesPerSec = (DWORD)(sampleRate * channels * (padded_bits >> 3));
	format->Format.nBlockAlign = (WORD)(channels * (padded_bits >> 3));
	format->Format.wBitsPerSample = (WORD)padded_bits;

	if (channels > 2 || padded_bits != bits)
	{
		format->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		format->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		format->Samples.wValidBitsPerSample = (WORD)bits;
		format->dwChannelMask = GetChannelMask(channels);
		format->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	}
	else
	{
		format->Format.wFormatTag = WAVE_FORMAT_PCM;
		format->Format.cbSize = 0;
	}
}

Resampler::Resampler(size_t inputBits, size_t inputChannels, size_t inputSampleRate,
                     size_t outputBits, size_t outputChannels, size_t outputSampleRate, bool floatingPoint)
		: sizeFactor(1.), hStream(0), buffer(0), eof(false)
{
	if (floatingPoint)
	{
		WAVEFORMATIEEEFLOATEX inputFormat;
		FillWFX_float(&inputFormat, inputChannels, inputBits, inputSampleRate);

		WAVEFORMATIEEEFLOATEX outputFormat;
		FillWFX_float(&outputFormat, outputChannels, outputBits, outputSampleRate);

		acmStreamOpen(&hStream, 0, &inputFormat.Format, &outputFormat.Format, 0, 0, 0, ACM_STREAMOPENF_NONREALTIME);
	}
	else
	{
		WAVEFORMATEXTENSIBLE inputFormat = {0};
		FillWFX(&inputFormat, inputChannels, inputBits, inputSampleRate);

		WAVEFORMATEXTENSIBLE outputFormat = {0};
		FillWFX(&outputFormat, outputChannels, outputBits, outputSampleRate);

		acmStreamOpen(&hStream, 0, &inputFormat.Format, &outputFormat.Format, 0, 0, 0, ACM_STREAMOPENF_NONREALTIME);
	}

	if (!hStream)
	{
		// error - what should we do?
		return ;
	}

	bufferAlloc = MulDiv(1024, (int) (outputBits * outputChannels), 8); // TODO: use acmStreamSize
	buffer = (__int8 *)calloc(bufferAlloc, sizeof(__int8));
	bufferValid = 0;

	sizeFactor = (outputChannels * outputBits * outputSampleRate) * (inputChannels * inputBits * inputSampleRate);
}

Resampler::~Resampler()
{
	if (hStream)
	{
		acmStreamClose(hStream, 0);
	}
	free(buffer);
}

size_t Resampler::UseInternalBuffer(void *output, size_t outputBytes)
{
	if (bufferValid)
	{
		size_t writeSize = min(outputBytes, bufferValid);
		memcpy(output, buffer, writeSize);

		if (writeSize)
			memmove(buffer, buffer + writeSize, bufferValid - writeSize);
		bufferValid -= writeSize;

		return writeSize;
	}
	else
		return 0;
}

void Resampler::Flush()
{
	eof = true;
}

size_t Resampler::Convert(void *input, size_t *inputBytes, void *output, size_t outputBytes)
{
	size_t x = UseInternalBuffer(output, outputBytes);
	if (x)
		return x;

	ACMSTREAMHEADER streamHeader;
	ZeroMemory(&streamHeader, sizeof(streamHeader));

	streamHeader.cbStruct = sizeof(streamHeader);
	streamHeader.pbSrc = reinterpret_cast<LPBYTE>(input);
	streamHeader.cbSrcLength = (DWORD)*inputBytes;
	streamHeader.pbDst = reinterpret_cast<LPBYTE>(buffer);
	streamHeader.cbDstLength = (DWORD)bufferAlloc;

	if (acmStreamPrepareHeader(hStream, &streamHeader, 0))
		return 0; //			m_error = 1;

	if (input && *inputBytes && !eof)
		acmStreamConvert(hStream, &streamHeader, ACM_STREAMCONVERTF_BLOCKALIGN);
	else
		acmStreamConvert(hStream, &streamHeader, ACM_STREAMCONVERTF_END);

	*inputBytes -= streamHeader.cbSrcLengthUsed;
	bufferValid = streamHeader.cbDstLengthUsed;

	acmStreamUnprepareHeader(hStream, &streamHeader, 0);
	return UseInternalBuffer(output, outputBytes);
}

bool Resampler::OK()
{
	if (hStream)
		return true;
	else
		return false;
}
