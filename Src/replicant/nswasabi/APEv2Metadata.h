#pragma once
#include "metadata/metadata.h"
#include "nsapev2/nsapev2.h"

/* this class mimics ifc_metadata and ifc_metadata_editor, but doesn't inherit (because it's not given out directly) */
class APEv2Metadata
{
public:
	APEv2Metadata();
	~APEv2Metadata();

	static int Initialize(api_metadata *metadata_api);
	int Initialize(nsapev2_tag_t tag);
	
	/* ifc_metadata implementation */
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);

	/* ifc_metadata_editor implementation */
	int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value);
	int WASABICALL MetadataEditor_SetInteger(int field, unsigned int index, int64_t value);
	int WASABICALL MetadataEditor_SetReal(int field, unsigned int index, double value);
	int WASABICALL MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *data, data_flags_t flags);
private:
	nsapev2_tag_t apev2_tag;
	static api_metadata *metadata_api;
};
