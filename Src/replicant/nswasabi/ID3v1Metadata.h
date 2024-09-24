#pragma once
#include "metadata/metadata.h"
#include "nsid3v1/nsid3v1.h"

/* this class mimics ifc_metadata and ifc_metadata_editor, but doesn't inherit (because it's not given out directly) */
class ID3v1Metadata
{
public:
	ID3v1Metadata();
	~ID3v1Metadata();

	static int Initialize(api_metadata *metadata_api);
	int Initialize(nsid3v1_tag_t tag);
	
	/* ifc_metadata implementation */
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	
	/* ifc_metadata_editor implementation */
	int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value);
	int WASABICALL MetadataEditor_SetInteger(int field, unsigned int index, int64_t value);
private:
	nsid3v1_tag_t id3v1_tag;
	static api_metadata *metadata_api;
};
