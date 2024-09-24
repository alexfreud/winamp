#pragma once
#include "metadata/metadata.h"
#include "nsid3v2/nsid3v2.h"

/* this class mimics ifc_metadata and ifc_metadata_editor, but doesn't inherit (because it's not given out directly) */
class ID3v2Metadata
{
public:
	ID3v2Metadata();
	~ID3v2Metadata();

	static int Initialize(api_metadata *metadata_api);
	int Initialize(nsid3v2_tag_t tag);

	/* ifc_metadata implementation */
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);

	/* ifc_metadata_editor implementation */
	int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value);
	int WASABICALL MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *data, data_flags_t flags);
private:
	nsid3v2_tag_t id3v2_tag;

	int GetGenre(int index, nx_string_t *value);

	static api_metadata *metadata_api;

#ifdef __APPLE__
	CFNumberFormatterRef number_formatter;
#endif
};