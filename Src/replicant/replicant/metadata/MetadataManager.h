#pragma once
#include "metadata/api_metadata.h"
#include "metadata/MetadataKeys.h"

#include "nswasabi/ServiceName.h"

#include "nx/nxstring.h"
#include "nx/nxonce.h"
#include <vector>

#include "metadata/genres.h"



class MetadataManager : public api_metadata
{
public:
	WASABI_SERVICE_NAME("Metadata Manager");
	MetadataManager();
	~MetadataManager();

	void Shutdown();
	
	/* api_metadata implementation */
	ns_error_t WASABICALL Metadata_RegisterField(nx_string_t field_name, int *field);
	ns_error_t WASABICALL Metadata_GetFieldKey(nx_string_t field_name, int *field);
	ns_error_t WASABICALL Metadata_GetFieldName(int field_key, nx_string_t *name);
	ns_error_t WASABICALL Metadata_GetGenre(uint8_t genre_id, nx_string_t *genre);
	ns_error_t WASABICALL Metadata_GetGenreID(nx_string_t genre, uint8_t *genre_id);
	ns_error_t WASABICALL Metadata_SupportedFilename(nx_uri_t filename);
	ns_error_t WASABICALL Metadata_CreateMetadata(ifc_metadata **out_metadata, nx_uri_t filename);
	ns_error_t WASABICALL Metadata_CreateMetadataEditor(ifc_metadata_editor **out_metadata, nx_uri_t filename);
private:
	nx_once_value_t fields_once, genres_once;
	nx_string_t lookup[MetadataKeys::NUM_OF_METADATA_KEYS]; // Lookup list for all the standard "hard coded" fields
	
	std::vector<nx_string_t> lookup_extended;		// Lookup list for all the registered extended lookup entries
	nx_string_t genres[metadata_genre_list_size];
	

	static int NX_ONCE_API InitializeGenres(nx_once_t once, void *me, void **);
	static int NX_ONCE_API InitializeMetadataFieldLookup(nx_once_t once, void *me, void **);
};

