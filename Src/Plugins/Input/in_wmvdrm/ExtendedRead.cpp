#include "main.h"
#include "ExtendedRead.h"

extern WM_MEDIA_TYPE *NewMediaType(IWMOutputMediaProps *props);

ExtendedReadStruct::ExtendedReadStruct() : buffer(0), reader(0), streamNum(0), bufferUsed(0), endOfFile(false), length(0)
{}

ExtendedReadStruct::ExtendedReadStruct(IWMSyncReader *_reader) : buffer(0), reader(0), streamNum(0), bufferUsed(0), endOfFile(false), length(0)
{
	reader = _reader;
	reader->AddRef();
}

#define CBCLASS ExtendedReadStruct
START_DISPATCH;
CB(IFC_AUDIOSTREAM_READAUDIO, ReadAudio)
END_DISPATCH;

ExtendedReadStruct::~ExtendedReadStruct()
{
	if (reader)
	{
		reader->Close();
		reader->Release();
	}
	if (buffer)
		buffer->Release();
}

size_t ExtendedReadStruct::ReadAudio(void *_buffer, size_t len)
{
	__int8 *dest = reinterpret_cast<__int8 *>(_buffer);
	int bytesCopied = 0;
	/*
	   while we still have bytes left
		{
			read a buffer
			copy buffer to user passed buffer
			if we have stuff left in the buffer, save it and return
			if we hit EOF, return
		}
	*/
	size_t frameSize = (BitSize()/8)*Channels();
	len -= (len % frameSize); // only do whole frames
	while (len)
	{
		if (buffer)
		{
			BYTE *bufferBytes;
			DWORD bufferTotal;
			buffer->GetBufferAndLength(&bufferBytes, &bufferTotal);

			if (bufferUsed < bufferTotal)
			{
				size_t toCopy = min(bufferTotal - bufferUsed, len);
				memcpy(dest, bufferBytes + bufferUsed, toCopy);
				bufferUsed += toCopy;
				len -= toCopy;
				dest += toCopy;
				bytesCopied += toCopy;

				if (bufferUsed == bufferTotal)
				{
					bufferUsed = 0;
					buffer->Release();
					buffer = 0;
				}
			}
		}
		else if (!endOfFile)
		{
			QWORD sampleTime, duration;
			DWORD flags;
			DWORD outputNum;
			WORD streamNum2;
			HRESULT hr;
			hr = reader->GetNextSample(streamNum, &buffer, &sampleTime, &duration, &flags, & outputNum, &streamNum2);
			if (hr == NS_E_NO_MORE_SAMPLES)
				endOfFile = true;

		}
		else
			return bytesCopied;
	}

	return bytesCopied;
}

bool ExtendedReadStruct::Open(const wchar_t *filename)
{
	//mod.InitWM();
	if (!reader)
	{
		if (!SUCCEEDED(WMCreateSyncReader(NULL, 0, &reader)))
		{
			reader = 0;
			return false;
		}
	}

	if (SUCCEEDED(reader->Open(filename)))
	{
		return true;
	}
	else
	{
		reader->Release();
		reader = 0;
		return false;
	}
}

bool ExtendedReadStruct::FindOutput(int bits, int channels)
{
	DWORD numOutputs, output, format, numFormats;
	IWMOutputMediaProps *formatProperties;
	GUID mediaType;
	DWORD audioOutputNum;

	if (FAILED((reader->GetOutputCount(&numOutputs))))
		return false;

	for (output = 0;output < numOutputs;output++)
	{
		HRESULT hr;
		DWORD speakerConfig;
		switch (channels)
		{
			case 1:
				speakerConfig = DSSPEAKER_MONO;
				break;
			case 2:
				speakerConfig = DSSPEAKER_STEREO;
				break;
			case 0:
			default:
				speakerConfig = config_audio_num_channels; // TODO: force max channels?
		}

		hr = reader->SetOutputSetting(output, g_wszSpeakerConfig, WMT_TYPE_DWORD, (BYTE *) & speakerConfig, sizeof(speakerConfig));
		assert(hr == S_OK);

		BOOL discreteChannels = TRUE;
		hr = reader->SetOutputSetting(output, g_wszEnableDiscreteOutput, WMT_TYPE_BOOL, (BYTE *) & discreteChannels , sizeof(discreteChannels));
		assert(hr == S_OK);

		if (FAILED(reader->GetOutputFormatCount(output, &numFormats)))
			continue;

		for (format = 0;format < numFormats;format++)
		{
			reader->GetOutputFormat(output, format, &formatProperties);
			formatProperties->GetType(&mediaType);
			if (mediaType != WMMEDIATYPE_Audio)
				continue;

			WM_MEDIA_TYPE *mediaType = NewMediaType(formatProperties);
			if (mediaType->subtype == WMMEDIASUBTYPE_PCM)
			{
				if (bits)
				{
					WAVEFORMATEXTENSIBLE *waveFormat = (WAVEFORMATEXTENSIBLE *) mediaType->pbFormat;
					if (waveFormat->Format.cbSize >= 22)
						waveFormat->Samples.wValidBitsPerSample = bits;
					waveFormat->Format.wBitsPerSample = bits;
					waveFormat->Format.nBlockAlign = (waveFormat->Format.wBitsPerSample / 8) * waveFormat->Format.nChannels;
					waveFormat->Format.nAvgBytesPerSec = waveFormat->Format.nSamplesPerSec * waveFormat->Format.nBlockAlign;
					if (FAILED(formatProperties->SetMediaType(mediaType)))
					{
						// blah, just use the default settings then
						delete[] mediaType;
						mediaType = NewMediaType(formatProperties);
					}
				}

				AudioFormat::Open(mediaType);
				delete[] mediaType;
				audioOutputNum = output;
				reader->SetOutputProps(audioOutputNum, formatProperties);
				reader->GetStreamNumberForOutput(audioOutputNum, &streamNum);
				formatProperties->Release();
				reader->SetReadStreamSamples(streamNum, FALSE);

				IWMHeaderInfo *headerInfo = 0;
				reader->QueryInterface(&headerInfo);

				WMT_ATTR_DATATYPE type = WMT_TYPE_QWORD;
				WORD byteLength = sizeof(QWORD);
				WORD blah = 0;
				headerInfo->GetAttributeByName(&blah, g_wszWMDuration, &type, (BYTE *)&length, &byteLength);

				headerInfo->Release();

				return true;
			}

			delete[] mediaType;
			formatProperties->Release();
		}
	}
	return false;
}

extern "C"
	__declspec(dllexport) intptr_t winampGetExtendedRead_openW(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
{
	ExtendedReadStruct *read = new ExtendedReadStruct;

	if (read->Open(fn)
	    && read->FindOutput(*bps, *nch))
	{
		*nch = read->Channels();
		*bps = read->BitSize();
		*srate = read->SampleRate();
		__int64 bytespersec = ((*nch) * (*bps / 8) * (*srate));
		__int64 s = (bytespersec * ((__int64)read->length)) / (__int64)(1000 * 10000);
		*size = (int)s;
		return reinterpret_cast<intptr_t>(read);
	}
	else
	{
		delete read;
		return 0;
	}
}

extern "C"
	__declspec(dllexport) intptr_t winampGetExtendedRead_getData(intptr_t handle, char *dest, int len, int *killswitch)
{
	ExtendedReadStruct *read = reinterpret_cast<ExtendedReadStruct *>(handle);
	return read->ReadAudio(dest, len);
}

extern "C"
	__declspec(dllexport) void winampGetExtendedRead_close(intptr_t handle)
{
	ExtendedReadStruct *read = reinterpret_cast<ExtendedReadStruct *>(handle);
	delete read;
}