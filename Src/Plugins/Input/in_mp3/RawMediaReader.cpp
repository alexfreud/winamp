#include "RawMediaReader.h"
#include <limits.h>

bool IsMyExtension(const wchar_t *filename);

int RawMediaReaderService::CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **out_reader)
{
	if (IsMyExtension(filename))
	{
		CGioFile *file = new CGioFile();
		if (!file)
			return NErr_OutOfMemory;

		if (file->Open(filename, 0) != NErr_Success)
		{
			delete file;
			return NErr_FileNotFound;
		}

		RawMediaReader *reader = new RawMediaReader(file);
		if (!reader)
		{
			file->Close();
			delete file;
			return NErr_OutOfMemory;
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

RawMediaReader::RawMediaReader(CGioFile *file) : file(file)
{}

int RawMediaReader::Read( void *buffer, size_t buffer_size, size_t *bytes_read )
{
	if ( buffer_size > INT_MAX )
		return NErr_BadParameter;

	int file_bytes_read = 0;
	int ret = file->Read( buffer, (int)buffer_size, &file_bytes_read );

	if ( ret == NErr_Success )
	{
		*bytes_read = (size_t)file_bytes_read;
		if ( !file_bytes_read && file->IsEof() )
			return NErr_EndOfFile;

		return NErr_Success;
	}
	else
		return NErr_Error;
}

size_t RawMediaReader::Release()
{
	file->Close();

	delete file;
	file = NULL;

	delete this;

	return 0;
}


#define CBCLASS RawMediaReader
START_DISPATCH;
CB( RELEASE,  Release );
CB( RAW_READ, Read );
END_DISPATCH;
#undef CBCLASS
