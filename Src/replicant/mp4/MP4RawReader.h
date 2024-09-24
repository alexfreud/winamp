#pragma once
#include "file/svc_filerawreader.h"
#include "decode/ifc_raw_media_reader.h"
#include "mp4.h"
#include "nswasabi/ServiceName.h"

// {5CBD1F27-5A63-4D8C-9297-D74518E1EF3A}
static const GUID mp4_raw_reader_guid = 
{ 0x5cbd1f27, 0x5a63, 0x4d8c, { 0x92, 0x97, 0xd7, 0x45, 0x18, 0xe1, 0xef, 0x3a } };

class MP4RawReaderService : public svc_filerawreader
{
public:
	WASABI_SERVICE_NAME("MP4 Raw Reader");	
	static GUID GetServiceGUID() { return mp4_raw_reader_guid; } 
	ns_error_t WASABICALL FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);
};

class MP4RawReader : public ifc_raw_media_reader
{
public:
	MP4RawReader();
	~MP4RawReader();
	int Initialize(MP4FileHandle file);
	int WASABICALL RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read);
private:
	uint16_t track_num;
	uint32_t number_of_tracks;
	MP4TrackId current_track;
	MP4FileHandle file;
	MP4ChunkId chunk_id;
	MP4ChunkId number_of_chunks;
	uint32_t chunk_position, chunk_size;
	uint8_t *chunk_buffer;
	int ReadNextChunk();
};