#pragma once

#include "foundation/dispatch.h"
#include "ifc_raw_media_reader.h"
#include "nx/nxuri.h"

// {BE616DD5-5F42-4E42-88CF-CB7DCB47A3CD}
static const GUID svc_raw_media_reader_guid = 
{ 0xbe616dd5, 0x5f42, 0x4e42, { 0x88, 0xcf, 0xcb, 0x7d, 0xcb, 0x47, 0xa3, 0xcd } };


class svc_raw_media_reader : public Wasabi2::Dispatchable
{
protected:
	svc_raw_media_reader() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_raw_media_reader() {}
public:
	static GUID GetServiceType() { return svc_raw_media_reader_guid; }
	int CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, unsigned int pass) { return RawMediaReaderService_CreateRawMediaReader(reader, filename, pass); }

	enum
	{
		DISPATCHABLE_VERSION = 0,
	};
protected:
	virtual int WASABICALL RawMediaReaderService_CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, unsigned int pass)=0;
};
