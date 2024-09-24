#pragma once
#include "../Agave/DecodeFile/svc_raw_media_reader.h"
#include "../Agave/DecodeFile/ifc_raw_media_reader.h"
#include <mmreg.h>
#include <wmsdk.h>

// {9AF5FD89-DC41-4F2A-A156-8D1399FDE57B}
static const GUID wm_raw_reader_guid = 
{ 0x9af5fd89, 0xdc41, 0x4f2a, { 0xa1, 0x56, 0x8d, 0x13, 0x99, 0xfd, 0xe5, 0x7b } };

class RawMediaReaderService : public svc_raw_media_reader
{
public:
	static const char *getServiceName() { return "Windows Media Audio Raw Reader"; }
	static GUID getServiceGuid() { return wm_raw_reader_guid; } 
	int CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **reader);
protected:
	RECVS_DISPATCH;
};

class RawMediaReader : public ifc_raw_media_reader
{
public:
	RawMediaReader();
	~RawMediaReader();
	int Initialize(IWMSyncReader *reader);
	int Read(void *buffer, size_t buffer_size, size_t *bytes_read);
	size_t Release();
protected:
	RECVS_DISPATCH;
private:
	IWMSyncReader *reader;
	WORD stream_num;

	INSSBuffer *buffer;
	size_t buffer_used;
	bool end_of_file;
	QWORD length;
	DWORD next_output;
};
