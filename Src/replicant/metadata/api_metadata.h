#pragma once
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "service/types.h"

#include "nx/nxuri.h"
#include "metadata/ifc_metadata.h"
#include "metadata/ifc_metadata_editor.h"
#include "metadata/MetadataKeys.h"

// {63149C84-08DC-4EA0-9351-2B0CB263FE55}
static const GUID metadataApiServiceGuid = 
{ 0x63149c84, 0x8dc, 0x4ea0, { 0x93, 0x51, 0x2b, 0xc, 0xb2, 0x63, 0xfe, 0x55 } };


// ----------------------------------------------------------------------------
class NOVTABLE api_metadata: public Wasabi2::Dispatchable
{
protected:
	api_metadata()	: Dispatchable(DISPATCHABLE_VERSION) {}
	~api_metadata()	{}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return metadataApiServiceGuid; }
	
	ns_error_t RegisterField(nx_string_t field_name, int *field) { return Metadata_RegisterField(field_name, field); }	// Register for a non standard metadata field name
	ns_error_t GetFieldKey(nx_string_t field_name, int *field) { return Metadata_GetFieldKey(field_name, field); }		// Return the integer key of the current field
	ns_error_t GetFieldName(int field_key, nx_string_t *name) { return Metadata_GetFieldName(field_key, name); }		// Return the field name when provided with the key
	ns_error_t GetGenre(uint8_t genre_id, nx_string_t *genre) { return Metadata_GetGenre(genre_id, genre); }
	ns_error_t GetGenreID(nx_string_t genre, uint8_t *genre_id) { return Metadata_GetGenreID(genre, genre_id); }
	ns_error_t SupportedFilename(nx_uri_t filename)  { return Metadata_SupportedFilename(filename); }
	ns_error_t CreateMetadata(ifc_metadata **out_metadata, nx_uri_t filename) { return Metadata_CreateMetadata(out_metadata, filename); }
	ns_error_t CreateMetadataEditor(ifc_metadata_editor **out_metadata, nx_uri_t filename) { return Metadata_CreateMetadataEditor(out_metadata, filename); }
	enum
	{
		DISPATCHABLE_VERSION,			// ToDo: Does this need to be set here?
	};
protected:
	virtual ns_error_t WASABICALL Metadata_RegisterField(nx_string_t field_name, int *field)=0;
	virtual ns_error_t WASABICALL Metadata_GetFieldKey(nx_string_t field_name, int *field)=0;
	virtual ns_error_t WASABICALL Metadata_GetFieldName(int field_key, nx_string_t *name)=0;
	virtual ns_error_t WASABICALL Metadata_GetGenre(uint8_t genre_id, nx_string_t *genre)=0;
	virtual ns_error_t WASABICALL Metadata_GetGenreID(nx_string_t genre, uint8_t *genre_id)=0;
	virtual ns_error_t WASABICALL Metadata_SupportedFilename(nx_uri_t filename)=0;
	virtual ns_error_t WASABICALL Metadata_CreateMetadata(ifc_metadata **out_metadata, nx_uri_t filename)=0;
	virtual ns_error_t WASABICALL Metadata_CreateMetadataEditor(ifc_metadata_editor **out_metadata, nx_uri_t filename)=0;
};

