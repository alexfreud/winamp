#pragma once
#include "../Agave/DecodeFile/svc_raw_media_reader.h"
#include "../Agave/DecodeFile/ifc_raw_media_reader.h"
#include "main.h"

// {4FB808DC-C327-4999-9822-BDDDE20F44B0}
static const GUID sndfile_raw_reader_guid = 
{ 0x4fb808dc, 0xc327, 0x4999, { 0x98, 0x22, 0xbd, 0xdd, 0xe2, 0xf, 0x44, 0xb0 } };


class RawMediaReaderService : public svc_raw_media_reader
{
public:
	static const char *getServiceName()                               { return "SndFile Raw Reader"; }
	static GUID getServiceGuid()                                      { return sndfile_raw_reader_guid; }

	int CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **reader);

protected:
	RECVS_DISPATCH;
};

class RawMediaReader : public ifc_raw_media_reader
{
public:
	RawMediaReader()                                                  {}
	~RawMediaReader();

	int Initialize(const wchar_t *filename);
	int Read(void *buffer, size_t buffer_size, size_t *bytes_read);
	size_t Release();

protected:
	RECVS_DISPATCH;

private:
	SNDFILE *soundFile = NULL;
	SF_INFO info;
};
