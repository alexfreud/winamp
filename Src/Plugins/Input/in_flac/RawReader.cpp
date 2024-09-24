#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "RawReader.h"
#include "main.h"
#include <limits.h>
#include <shlwapi.h>
#include "../nu/AutoWide.h"
#include <new>
#include <strsafe.h>
#include "nswasabi/ReferenceCounted.h"

static bool IsMyExtension(const wchar_t *filename)
{
	const wchar_t *extension = PathFindExtensionW(filename);
	if (extension && *extension)
	{
		wchar_t exts[128] = {0};
		GetPrivateProfileStringW(L"in_flac", L"extensions", DEFAULT_EXTENSIONSW, exts, 128, winampINI);

		extension++;
		wchar_t *b = exts;
		wchar_t *c;
		do
		{
			wchar_t d[20] = {0};
			StringCchCopyW(d, 15, b);
			if ((c = wcschr(b, L';')))
			{
				if ((c-b)<15)
					d[c - b] = 0;
			}

			if (!lstrcmpiW(extension, d))
				return true;

			b = c + 1;
		}
		while (c);
	}
	return false;
}

int RawMediaReaderService::CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **out_reader)
{
	if (IsMyExtension(filename))
	{
		nx_file_t file;
		ReferenceCountedNXString filename_nx;
		ReferenceCountedNXURI filename_uri;
		NXStringCreateWithUTF16(&filename_nx, filename);
		NXURICreateWithNXString(&filename_uri, filename_nx);

		int ret = NXFileOpenFile(&file, filename_uri, nx_file_FILE_read_binary);
		if (ret != NErr_Success)
			return ret;

		RawMediaReader *reader = new (std::nothrow) RawMediaReader();
		if (!reader)
		{
			NXFileRelease(file);
			return NErr_OutOfMemory;
		}

		ret = reader->Initialize(file);
		NXFileRelease(file);
		if (ret != NErr_Success)
		{
			delete reader;
			return ret;
		}

		*out_reader = reader;
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

static void OnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//client_data=client_data; // dummy line so i can set a breakpoint
}

static void OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{

}

static FLAC__StreamDecoderWriteStatus OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

RawMediaReader::RawMediaReader()
{
	decoder=0;
	file=0;
}

RawMediaReader::~RawMediaReader()
{
	if (decoder)
		FLAC__stream_decoder_delete(decoder);
	NXFileRelease(file);
}

int RawMediaReader::Initialize(nx_file_t file)
{
	this->file = NXFileRetain(file);
	decoder = FLAC__stream_decoder_new();
	if (!decoder)
		return NErr_FailedCreate;

	FLAC__stream_decoder_set_metadata_ignore(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	state.SetFile(file);
	state.SetObject(this);
	FLAC__stream_decoder_set_md5_checking(decoder, true);
	if(FLAC__stream_decoder_init_stream(
		decoder,
		FLAC_NXFile_Read,
		FLAC_NXFile_Seek,
		FLAC_NXFile_Tell,
		FLAC_NXFile_Length,
		FLAC_NXFile_EOF,
		OnAudio,
		OnMetadata,
		OnError,
		&state 
		) != FLAC__STREAM_DECODER_INIT_STATUS_OK) 
	{
		FLAC__stream_decoder_delete(decoder);
		decoder=0;
		return NErr_Error;	
	}

	FLAC__stream_decoder_process_until_end_of_metadata(decoder);
	uint64_t position;
	FLAC__stream_decoder_get_decode_position(decoder, &position);
	FLAC__stream_decoder_delete(decoder);
	decoder=0;
	NXFileSeek(file, position);
	
	return NErr_Success;
}

int RawMediaReader::Read(void *buffer, size_t buffer_size, size_t *bytes_read)
{
	return NXFileRead(file, buffer, buffer_size, bytes_read);
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