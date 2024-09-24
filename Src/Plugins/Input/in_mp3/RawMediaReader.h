#pragma once
#include "../Agave/DecodeFile/svc_raw_media_reader.h"
#include "../Agave/DecodeFile/ifc_raw_media_reader.h"
#include "giofile.h"

// {5EC19CF3-E1ED-4AA5-AFD3-8E93149692BD}
static const GUID mpeg_audio_raw_reader_guid = 
{ 0x5ec19cf3, 0xe1ed, 0x4aa5, { 0xaf, 0xd3, 0x8e, 0x93, 0x14, 0x96, 0x92, 0xbd } };

class RawMediaReaderService : public svc_raw_media_reader
{
public:
	static const char *getServiceName() { return "MPEG-1/2 Audio Raw Reader"; }
	static GUID getServiceGuid() { return mpeg_audio_raw_reader_guid; } 
	int CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **reader);
protected:
	RECVS_DISPATCH;
};

class RawMediaReader : public ifc_raw_media_reader
{
public:
	RawMediaReader(CGioFile *file);
	int Read(void *buffer, size_t buffer_size, size_t *bytes_read);
	size_t Release();
protected:
	RECVS_DISPATCH;
private:
	CGioFile *file;
};