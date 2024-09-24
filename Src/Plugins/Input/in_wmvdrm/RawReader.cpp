#include "main.h"
#include "RawReader.h"
#include "FileTypes.h"
#include <shlwapi.h>

#define NS_E_FILE_IS_CORRUPTED	_HRESULT_TYPEDEF_(0xC00D080DL)

bool IsMyExtension(const wchar_t *filename)
{
	const wchar_t *ext = PathFindExtension(filename);
	if (ext && *ext)
	{
		ext++;
		return fileTypes.GetAVType(ext) != -1;
	}
	return false;
}

int RawMediaReaderService::CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **out_reader)
{
	if (IsMyExtension(filename))
	{
		IWMSyncReader *reader = 0;
		if (!SUCCEEDED(WMCreateSyncReader(NULL, 0, &reader)))
		{
			return NErr_FailedCreate;
		}

		if (FAILED(reader->Open(filename)))
		{
			reader->Release();
			reader = 0;
			return NErr_FileNotFound; // TODO: check HRESULT
		}

		RawMediaReader *raw_reader = new RawMediaReader();
		if (!raw_reader)
		{
			reader->Close();
			reader->Release();
			return NErr_OutOfMemory;
		}

		int ret = raw_reader->Initialize(reader);
		if (ret != NErr_Success)
		{
			delete raw_reader;
			return ret;
		}

		*out_reader = raw_reader;
		return NErr_Success;
	}
	else
	{
		return NErr_False;
	}
}

#define CBCLASS RawMediaReaderService
START_DISPATCH;
CB(CREATERAWMEDIAREADER, CreateRawMediaReader);
END_DISPATCH;
#undef CBCLASS

RawMediaReader::RawMediaReader()
{
	reader=0;
	stream_num=0;
	buffer_used=0;
	end_of_file=false;
	length=0;
	buffer=0;
	next_output=0;
}

RawMediaReader::~RawMediaReader()
{
	if (reader)
	{
		reader->Close();
		reader->Release();
	}

	if (buffer)
	{
		buffer->Release();
	}
}

int RawMediaReader::Initialize(IWMSyncReader *reader)
{
	this->reader=reader;
	return NErr_Success;
}

int RawMediaReader::Read(void *out_buffer, size_t buffer_size, size_t *bytes_read)
{
	/* we don't care about these, but the API does not allows NULL */
	QWORD sample_time = 0, duration = 0;
	size_t bytesCopied = 0;
	uint8_t *dest = (uint8_t *)out_buffer;
	for (;;)
	{
		if (buffer)
		{
			BYTE *bufferBytes = 0;
			DWORD bufferTotal = 0;
			buffer->GetBufferAndLength(&bufferBytes, &bufferTotal);

			if (buffer_used < bufferTotal)
			{
				size_t toCopy = min(bufferTotal - buffer_used, buffer_size);
				memcpy(dest, bufferBytes + buffer_used, toCopy);
				buffer_used += toCopy;
				buffer_size -= toCopy;
				dest += toCopy;
				bytesCopied += toCopy;

				if (buffer_used == bufferTotal)
				{
					buffer_used = 0;
					buffer->Release();
					buffer = 0;
				}
			}
			if (buffer_size == 0)
			{
				*bytes_read = bytesCopied;
				return NErr_Success;
			}
		}

		if (stream_num == 0)
		{
			DWORD outputs = 0;
			HRESULT hr=reader->GetOutputCount(&outputs);
			if (FAILED(hr))
				return NErr_Error;
			if (next_output >= outputs)
				return NErr_EndOfFile;
			hr=reader->GetStreamNumberForOutput(next_output, &stream_num);
			if (FAILED(hr))
				return NErr_Error;
			hr=reader->SetReadStreamSamples(stream_num, TRUE);
			if (FAILED(hr))
				return NErr_Error;
			next_output++;
		}

		DWORD flags = 0;
		HRESULT r = reader->GetNextSample(stream_num, &buffer, &sample_time, &duration, &flags, 0, 0);
		if (r == NS_E_NO_MORE_SAMPLES || r == NS_E_FILE_IS_CORRUPTED)
		{
			stream_num=0;
		}
	}

	return NErr_Error;
}

size_t RawMediaReader::Release()
{
	delete this;
	return 0;
}

#define CBCLASS RawMediaReader
START_DISPATCH;
CB(RELEASE, Release);
CB(RAW_READ, Read);
END_DISPATCH;
#undef CBCLASS