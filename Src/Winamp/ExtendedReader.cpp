/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "ExtendedReader.h"
#include "../nu/AutoCharFn.h"

ExtendedReader::ExtendedReader(OpenFunc _open, OpenWFunc _openW, OpenFunc _openFloat, OpenWFunc _openWFloat, GetDataFunc _getData, CloseFunc _close, SetTimeFunc _setTime)
	: open(_open), openFloat(_openFloat), openW(_openW), openWFloat(_openWFloat), getData(_getData), close(_close), setTime(_setTime), handle(0)
{}

bool ExtendedReader::Open(const wchar_t *filename, AudioParameters *parameters)
{
	if (parameters->flags & AUDIOPARAMETERS_FLOAT)
	{
		if (this->openWFloat)
			handle = this->openWFloat(filename, (int *) & parameters->sizeBytes, (int *) & parameters->bitsPerSample, (int *) & parameters->channels, (int *) & parameters->sampleRate);
		else if (this->openFloat)
			handle = this->openFloat(AutoCharFn(filename), (int *) & parameters->sizeBytes, (int *) & parameters->bitsPerSample, (int *) & parameters->channels, (int *) & parameters->sampleRate);
	}
	else
	{
		if (this->openW)
			handle = this->openW(filename, (int *) & parameters->sizeBytes, (int *) & parameters->bitsPerSample, (int *) & parameters->channels, (int *) & parameters->sampleRate);
		else
			handle = this->open(AutoCharFn(filename), (int *) & parameters->sizeBytes, (int *) & parameters->bitsPerSample, (int *) & parameters->channels, (int *) & parameters->sampleRate);
	}
	if (handle > 0)
		return true;
	else
	{
		if (handle == -1)
			parameters->errorCode = API_DECODEFILE_FAIL_NO_WARN;
		else
			parameters->errorCode = API_DECODEFILE_FAILURE;
		return false;
	}
}

size_t ExtendedReader::ReadAudio(void *buffer, size_t sizeBytes)
{
	int killswitch = 0;
	return this->getData(handle, buffer, sizeBytes, &killswitch);
}

size_t ExtendedReader::ReadAudio_kill(void *buffer, size_t sizeBytes, int *killswitch, int *error)
{
	*error=0;
	intptr_t ret = this->getData(handle, buffer, sizeBytes, killswitch);
	if (ret < 0)
	{
		*error=1;
		return 0;
	}
	else
		return static_cast<size_t>(ret);
}

BOOL ExtendedReader::SeekToTimeMs(int millisecs)
{
	if (!setTime)
		return FALSE;
	return this->setTime(handle, millisecs);
}

int ExtendedReader::CanSeek()
{
	return setTime?1:0;
}

ExtendedReader::~ExtendedReader()
{
	if (handle)
		this->close(handle);
}

#define CBCLASS ExtendedReader
START_DISPATCH;
CB(IFC_AUDIOSTREAM_READAUDIO, ReadAudio)
CB(IFC_AUDIOSTREAM_READAUDIO2, ReadAudio_kill)
CB(IFC_AUDIOSTREAM_SEEKTOTIMEMS, SeekToTimeMs)
CB(IFC_AUDIOSTREAM_CANSEEK, CanSeek)
END_DISPATCH;