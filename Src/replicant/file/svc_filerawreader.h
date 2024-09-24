#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "nx/nxfile.h"
#include "foundation/error.h"
#include "decode/ifc_raw_media_reader.h"
#include "metadata/ifc_metadata.h"

// {FF84B47B-5ED4-45E8-B822-0F8FE20F80A8}
static const GUID filerawreader_service_type_guid = 
{ 0xff84b47b, 0x5ed4, 0x45e8, { 0xb8, 0x22, 0xf, 0x8f, 0xe2, 0xf, 0x80, 0xa8 } };

class NOVTABLE svc_filerawreader : public Wasabi2::Dispatchable
{
protected:
	svc_filerawreader() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_filerawreader() {}

public:
	static GUID GetServiceType() { return filerawreader_service_type_guid; }
	int CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata) { return FileRawReaderService_CreateRawMediaReader(reader, filename, file, parent_metadata); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
protected:
	virtual ns_error_t WASABICALL FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)=0;
};

