#pragma once
#include "foundation/foundation.h"
#include "ifc_metadata.h"
#include "ifc_metadata_editor.h"
#include "nx/nxuri.h"

// {7DBF780E-78B6-436C-A188-864AF6859D87}
static const GUID metadata_service_type_guid =
{ 0x7dbf780e, 0x78b6, 0x436c, { 0xa1, 0x88, 0x86, 0x4a, 0xf6, 0x85, 0x9d, 0x87 } };

class svc_metadata : public Wasabi2::Dispatchable
{
protected:
	svc_metadata() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_metadata() {}
public:
	static GUID GetServiceType() { return metadata_service_type_guid; }
	// to make the implementation more flexible, you need to NXStringRelease on the extension you get (i.e. this function follows Apple's "Create" rule)
	int EnumerateExtensions(unsigned int index, nx_string_t *extension) { return MetadataService_EnumerateExtensions(index, extension); }

	int CreateMetadata(nx_uri_t filename, ifc_metadata **metadata) {return MetadataService_CreateMetadata(filename, metadata); }
	int CreateMetadataEditor(nx_uri_t filename, ifc_metadata_editor **metadata) {	return MetadataService_CreateMetadataEditor(filename, metadata); 	}

	int DeserializeMetadata(nx_data_t data, ifc_metadata **metadata) { return MetadataService_DeserializeMetadata(data, metadata); }

	int CreateMetadata(unsigned int pass, nx_uri_t filename, ifc_metadata **metadata) 
	{
		if (dispatchable_version == 0)
		{
			if (pass == 0)
				return MetadataService_CreateMetadata(filename, metadata); 
			else
				return NErr_False;
		}
		else 
			return MetadataService_CreateMetadata(pass, filename, metadata); 
	}
	int CreateMetadataEditor(unsigned int pass, nx_uri_t filename, ifc_metadata_editor **metadata) 
	{
		if (dispatchable_version == 0)
		{
			if (pass == 0)
				return MetadataService_CreateMetadataEditor(filename, metadata); 
			else
				return NErr_False;
		}
		else
			return MetadataService_CreateMetadataEditor(pass, filename, metadata); 
	}
	enum
	{
		DISPATCHABLE_VERSION=1,
	};
private:
	// implementation note: to make the implementation more flexible, you need to NXStringRetain on the extension you pass back (i.e. follow Apple's "Create" rule)
	virtual int WASABICALL MetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension)=0;

	/* these two no longer have to be implemented */
	virtual int WASABICALL MetadataService_CreateMetadata(nx_uri_t filename, ifc_metadata **metadata) { return MetadataService_CreateMetadata(0, filename, metadata); } 
	virtual int WASABICALL MetadataService_CreateMetadataEditor(nx_uri_t filename, ifc_metadata_editor **metadata) { return MetadataService_CreateMetadataEditor(0, filename, metadata); }

	virtual int WASABICALL MetadataService_DeserializeMetadata(nx_data_t data, ifc_metadata **metadata) { return NErr_NotImplemented; }
	virtual int WASABICALL MetadataService_CreateMetadata(unsigned int pass, nx_uri_t filename, ifc_metadata **metadata)=0;
	virtual int WASABICALL MetadataService_CreateMetadataEditor(unsigned int pass, nx_uri_t filename, ifc_metadata_editor **metadata)=0;
};
