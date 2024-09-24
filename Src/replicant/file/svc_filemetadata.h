#pragma once
#include "foundation/foundation.h"
#include "nx/nxstring.h"
#include "ifc_filemetadata_editor.h"

// {92269164-30E1-469A-96C3-96879EF6C39E}
static const GUID file_metadata_service_type_guid = 
{ 0x92269164, 0x30e1, 0x469a, { 0x96, 0xc3, 0x96, 0x87, 0x9e, 0xf6, 0xc3, 0x9e } };

class svc_filemetadata : public Wasabi2::Dispatchable
{
protected:
	svc_filemetadata() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_filemetadata() {}
public:
	static GUID GetServiceType() { return file_metadata_service_type_guid; }
	// to make the implementation more flexible, you need to NXStringRelease on the extension you get (i.e. this function follows Apple's "Create" rule)
	int EnumerateExtensions(unsigned int index, nx_string_t *extension) { return FileMetadataService_EnumerateExtensions(index, extension); }
	int CreateFileMetadata(ifc_metadata **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata) { return FileMetadataService_CreateFileMetadata(file_metadata, filename, file, parent_metadata); }
	int CreateFileMetadataEditor(ifc_filemetadata_editor **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata) { return FileMetadataService_CreateFileMetadataEditor(file_metadata, filename, file, parent_metadata); }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	// implementation note: to make the implementation more flexible, you need to NXStringRetain on the extension you pass back (i.e. follow Apple's "Create" rule)
	virtual int WASABICALL FileMetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension)=0;
	virtual int WASABICALL FileMetadataService_CreateFileMetadata(ifc_metadata **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)=0;
	virtual int WASABICALL FileMetadataService_CreateFileMetadataEditor(ifc_filemetadata_editor **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata) { return NErr_NotImplemented; }
};
