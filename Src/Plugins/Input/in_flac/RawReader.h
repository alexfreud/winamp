#pragma once
#include "../Agave/DecodeFile/svc_raw_media_reader.h"
#include "../Agave/DecodeFile/ifc_raw_media_reader.h"
#include "FLACFileCallbacks.h"
#include <FLAC/all.h>

// {E906F4DC-3080-4B9B-951F-85950193ACBF}
static const GUID flac_raw_reader_guid = 
{ 0xe906f4dc, 0x3080, 0x4b9b, { 0x95, 0x1f, 0x85, 0x95, 0x1, 0x93, 0xac, 0xbf } };


class RawMediaReaderService : public svc_raw_media_reader
{
public:
	static const char *getServiceName() { return "FLAC Raw Reader"; }
	static GUID getServiceGuid() { return flac_raw_reader_guid; } 
	int CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **reader);
protected:
	RECVS_DISPATCH;
};

class RawMediaReader : public ifc_raw_media_reader
{
public:
	RawMediaReader();
	~RawMediaReader();
	int Initialize(nx_file_t );
	int Read(void *buffer, size_t buffer_size, size_t *bytes_read);
	size_t Release();
protected:
	RECVS_DISPATCH;
private:
	FLAC__StreamDecoder *decoder;
	FLACClientData state;
	nx_file_t file;
};