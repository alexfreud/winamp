#pragma once
#include <FLAC/all.h>
#include "nx/nxfile.h"


FLAC__StreamDecoderReadStatus FLAC_NXFile_Read(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data);
FLAC__StreamDecoderSeekStatus FLAC_NXFile_Seek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
FLAC__StreamDecoderTellStatus FLAC_NXFile_Tell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
FLAC__StreamDecoderLengthStatus FLAC_NXFile_Length(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data);
FLAC__bool FLAC_NXFile_EOF(const FLAC__StreamDecoder *decoder, void *client_data);


class FLACClientData
{
public:
	FLACClientData() : object(0) {}
	void SetFile(nx_file_t file) { this->file = file; }
	void SetObject(void *object) { this->object = object; }
	nx_file_t GetFile() { return file; }
	void *GetObject() { return object; }
private:
	nx_file_t file;
	void *object;
};

template <typename _t>
static _t *FLAC_GetObject(void *client_data)
{
	return (_t *)((FLACClientData *)client_data)->GetObject();
}

static nx_file_t FLAC_GetFile(void *client_data)
{
	return ((FLACClientData *)client_data)->GetFile();
}