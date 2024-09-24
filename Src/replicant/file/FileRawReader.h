#pragma once
#include "decode/svc_raw_media_reader.h"
#include "decode/ifc_raw_media_reader.h"
#include "nswasabi/ServiceName.h"
#include "svc_filerawreader.h"

// {3877B6CF-937B-4E83-9A88-02E16B3A9654}
static const GUID file_raw_reader_guid  = 
{ 0x3877b6cf, 0x937b, 0x4e83, { 0x9a, 0x88, 0x2, 0xe1, 0x6b, 0x3a, 0x96, 0x54 } };

class FileRawReaderService : public svc_raw_media_reader
{
public:
	WASABI_SERVICE_NAME("File Raw Reader");
	WASABI_SERVICE_GUID(file_raw_reader_guid);
	
	int WASABICALL RawMediaReaderService_CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, unsigned int pass);
};
