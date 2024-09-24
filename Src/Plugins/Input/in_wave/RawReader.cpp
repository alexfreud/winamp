#include "main.h"
#include "RawReader.h"
#include <shlwapi.h>
	 /*
bool IsMyExtension(const wchar_t *filename)
{
	const wchar_t *ext = PathFindExtension(filename);
	if (ext && *ext)
	{
		ext++;
		return fileTypes.GetAVType(ext) != -1;
	}
	return false;
}*/

int RawMediaReaderService::CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **out_reader)
{
	// TODO:if (IsMyExtension(filename))
	if (!_wcsicmp(L".WAV", PathFindExtensionW(filename)))
	{
		RawMediaReader *raw_reader = new RawMediaReader();
		if (!raw_reader)
		{
			return NErr_OutOfMemory;
		}

		int ret = raw_reader->Initialize(filename);
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
CB( CREATERAWMEDIAREADER, CreateRawMediaReader )
END_DISPATCH;
#undef CBCLASS


RawMediaReader::~RawMediaReader()
{
	if (soundFile)
		sf_close(soundFile);
}

int RawMediaReader::Initialize(const wchar_t *filename)
{
	info.format = 0;
	soundFile = sf_wchar_open(filename, SFM_READ, &info);
	if (!soundFile)
		return NErr_FileNotFound;
	
	return NErr_Success;
}

int RawMediaReader::Read(void *out_buffer, size_t buffer_size, size_t *bytes_read)
{
	sf_count_t sf_read = sf_read_raw(soundFile, out_buffer, buffer_size);
	if (sf_read == 0)
		return NErr_EndOfFile;
	*bytes_read = (size_t)sf_read;
	return NErr_Success;
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