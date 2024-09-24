#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "nx/nxfile.h"
#include "foundation/error.h"
#include "ifc_fileplayback.h"
#include "metadata/ifc_metadata.h"
// {AAB6F26D-FF99-4CE8-BC7F-81BEA9F35CA9}
static const GUID svc_fileplayback_type_guid = 
{ 0xaab6f26d, 0xff99, 0x4ce8, { 0xbc, 0x7f, 0x81, 0xbe, 0xa9, 0xf3, 0x5c, 0xa9 } };

class NOVTABLE svc_fileplayback : public Wasabi2::Dispatchable
{
protected:
	svc_fileplayback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_fileplayback() {}

public:
	static GUID GetServiceType() { return svc_fileplayback_type_guid; }
	ns_error_t CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent) { return FilePlaybackService_CreatePlayback(out_playback_object, filename, file, parent_metadata, parent); }
	
enum
	{
		DISPATCHABLE_VERSION=0,
	};
protected:
	/* do _not_ retain/release the ifc_fileplayback_parent object! */
	virtual ns_error_t WASABICALL FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)=0;
};

