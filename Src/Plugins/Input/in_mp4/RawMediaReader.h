#pragma once
#include "../Agave/DecodeFile/svc_raw_media_reader.h"
#include "../Agave/DecodeFile/ifc_raw_media_reader.h"
#include <mp4.h>

// {5CBD1F27-5A63-4D8C-9297-D74518E1EF3A}
static const GUID mpeg4_raw_reader_guid = 
{ 0x5cbd1f27, 0x5a63, 0x4d8c, { 0x92, 0x97, 0xd7, 0x45, 0x18, 0xe1, 0xef, 0x3a } };

class RawMediaReaderService : public svc_raw_media_reader
{
public:
	static const char *getServiceName() { return "MPEG-4 Raw Reader"; }
	static GUID getServiceGuid() { return mpeg4_raw_reader_guid; } 
	int CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **reader);
protected:
	RECVS_DISPATCH;
};

class RawMediaReader : public ifc_raw_media_reader
{
public:
	RawMediaReader(MP4FileHandle file, void *reader);
	~RawMediaReader();
	int Read(void *buffer, size_t buffer_size, size_t *bytes_read);
	size_t Release();
protected:
	RECVS_DISPATCH;
private:
	uint16_t track_num;
	uint32_t number_of_tracks;
	MP4TrackId current_track;
	MP4FileHandle file;
	void *reader;
	MP4ChunkId chunk_id;
	MP4ChunkId number_of_chunks;
	uint32_t chunk_position, chunk_size;
	uint8_t *chunk_buffer;
	int ReadNextChunk();
};