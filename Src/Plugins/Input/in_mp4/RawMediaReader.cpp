#include "RawMediaReader.h"
#include "virtualIO.h"
#include <limits.h>

bool IsMyExtension(const wchar_t *filename);

int RawMediaReaderService::CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **out_reader)
{
	if (IsMyExtension(filename))
	{
		void *unicode_reader = CreateUnicodeReader(filename);
		if (!unicode_reader )
			return NErr_FileNotFound;

		MP4FileHandle mp4 = MP4ReadEx(filename, unicode_reader, &UnicodeIO);
		if (!mp4) 
		{
			DestroyUnicodeReader(unicode_reader);
			return NErr_Malformed;
		}


		RawMediaReader *reader = new RawMediaReader(mp4, unicode_reader);
		if (!reader)
		{
			MP4Close(mp4); 
			DestroyUnicodeReader(unicode_reader); 
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

RawMediaReader::RawMediaReader(MP4FileHandle file, void *reader) : file(file), reader(reader)
{
	track_num=0;
	number_of_tracks=MP4GetNumberOfTracks(file);
	current_track = MP4_INVALID_TRACK_ID;
	chunk_position=0;
	chunk_size=0;
	chunk_buffer=0;
}

RawMediaReader::~RawMediaReader()
{
	if (chunk_buffer)
		MP4Free(chunk_buffer);
	MP4Close(file); 
	DestroyUnicodeReader(reader);
}

int RawMediaReader::ReadNextChunk()
{
again:
	/* see if it's time to cycle to the next track */
	if (current_track == MP4_INVALID_TRACK_ID)
	{
		if (track_num == number_of_tracks)
			return NErr_EndOfFile;

		current_track = MP4FindTrackId(file, track_num);
		if (current_track == MP4_INVALID_TRACK_ID)
			return NErr_EndOfFile;

		track_num++;

		const char* trackType = MP4GetTrackType(file, current_track);
		if (!MP4_IS_AUDIO_TRACK_TYPE(trackType) && !MP4_IS_VIDEO_TRACK_TYPE(trackType))
		{
			current_track = MP4_INVALID_TRACK_ID;
			goto again;
		}

		chunk_id = 1;
		number_of_chunks= MP4GetTrackNumberOfChunks(file, current_track);		
	}

	/* see if we've read all of our samples */
	if (chunk_id > number_of_chunks)
	{
		current_track = MP4_INVALID_TRACK_ID;
		goto again;
	}

	bool readSuccess = MP4ReadChunk(file, current_track, chunk_id, &chunk_buffer, &chunk_size);
	if (!readSuccess)
		return NErr_Error;

	chunk_position=0;
	chunk_id++;
	return NErr_Success;
}

int RawMediaReader::Read(void *buffer, size_t buffer_size, size_t *bytes_read)
{
	if (buffer_size > INT_MAX)
		return NErr_BadParameter;

	if (chunk_position==chunk_size)
	{
		MP4Free(chunk_buffer);
		chunk_buffer=0;
	}

	if (!chunk_buffer)
	{
		int ret = ReadNextChunk();
		if (ret != NErr_Success)
			return ret;
	}

	size_t to_read = chunk_size-chunk_position;
	if (to_read > buffer_size)
		to_read = buffer_size;

	memcpy(buffer, &chunk_buffer[chunk_position], to_read);
	chunk_position += to_read;
	*bytes_read = to_read;
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