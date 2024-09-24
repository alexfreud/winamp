#pragma once
#include "foundation/dispatch.h"
#include "nx/nxstring.h"
#include "nx/nxdata.h"
#include "foundation/error.h"
#include "metadata/MetadataKeys.h"
#include "metadata/types.h"

class NOVTABLE ifc_metadata : public Wasabi2::Dispatchable
{
protected:
	ifc_metadata() : Dispatchable(DISPATCHABLE_VERSION) {}
public:
	virtual ~ifc_metadata() {}

	enum
	{
		INDEX_DEFAULT = -1,
	};

	/* Fields */
	ns_error_t GetField(int field, unsigned int index, nx_string_t *value) { return Metadata_GetField(field, index, value); }
	ns_error_t GetInteger(int field, unsigned int index, int64_t *value) { return Metadata_GetInteger(field, index, value); }
	ns_error_t GetReal(int field, unsigned int index, double *value) { return Metadata_GetReal(field, index, value); }

	ns_error_t SetField(int field, unsigned int index, nx_string_t value) { return Metadata_SetField(field, index, value); }
	ns_error_t SetInteger(int field, unsigned int index, int64_t value) { return Metadata_SetInteger(field, index, value); }
	ns_error_t SetReal(int field, unsigned int index, double value) { return Metadata_SetReal(field, index, value); }

	/* Art */
	ns_error_t GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags=DATA_FLAG_ALL) { return Metadata_GetArtwork(field, index, artwork, flags); }

	/* Binary Data */
	ns_error_t GetBinary(int field, unsigned int index, nx_data_t *data) { return Metadata_GetBinary(field, index, data); }

	/* sub-tracks */
	ns_error_t GetMetadata(int field, unsigned int index, ifc_metadata **metadata) { return Metadata_GetMetadata(field, index, metadata); }

	ns_error_t Serialize(nx_data_t *data) { return Metadata_Serialize(data); }

	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value)=0;
	virtual ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value)=0;
	virtual ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value)=0;

	virtual ns_error_t WASABICALL Metadata_SetField(int field, unsigned int index, nx_string_t value){ return NErr_NotImplemented; };
	virtual ns_error_t WASABICALL Metadata_SetInteger(int field, unsigned int index, int64_t value){ return NErr_NotImplemented; };
	virtual ns_error_t WASABICALL Metadata_SetReal(int field, unsigned int index, double value){ return NErr_NotImplemented; };

	/* notes:
	   passing NULL for any of the pointers (most notably, data) indicates that it's not needed
		 width and height aren't guaranteed to be actually accurate.  they will be set only if it was marked in the file, otherwise they will be set to 0
		 mime_type might also come back as 0 if it wasn't stored in the metadata.  and again, mime_type isn't guaranteed to be accurate
		 for type, MetadataKeys::ALBUM is for album art.  use Metadata::UNKNOWN for unknown types, such as id3v2 tags with Picture types marked as "other" ($00)
		 index should be as stored in the file, do not re-arrange.  */
	virtual ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)=0;
	virtual ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data)=0;
	virtual ns_error_t WASABICALL Metadata_GetMetadata(int field, unsigned int index, ifc_metadata **metadata) { return NErr_NotImplemented; }

	virtual ns_error_t WASABICALL Metadata_Serialize(nx_data_t *data) { return NErr_NotImplemented; }
};
