#pragma once
#include "metadata/api_artwork.h"
#include "nswasabi/ServiceName.h"
#include "nx/nx.h"


#if 0 // TODO
class ArtworkEnumerator
{
public:
};
#endif

class ArtworkManager : public api_artwork
{
public:
		WASABI_SERVICE_NAME("Artwork Manager API");
		ArtworkManager();
		
		int Initialize();
		void Shutdown();
private:
	/* returns the data for the first piece of artwork found 
	pass NULL for any of the values that you don't care about */
	int WASABICALL Artwork_GetArtwork(nx_uri_t filename, unsigned int field, artwork_t *artwork, data_flags_t flags, nx_time_unix_64_t *filename_modified);

#if defined(__linux__)
	int FindValidFile(nx_uri_t filepath, nx_string_t file_spec[], size_t spec_count, artwork_t *artwork, data_flags_t flags);
#else
	int FindValidFile(nx_uri_t filepath, nx_string_t file_spec, artwork_t *artwork, data_flags_t flags);
#endif
	
	nx_string_t GetFileSpecForField(unsigned int field, size_t index);
	nx_string_t EnumerateExtension(size_t index);
	nx_string_t EnumerateMIME(size_t index);

	// TODO: this will go away at some point, when we have image loaders ready
	struct supported_image_types_t
	{
		nx_string_t extension;
		nx_string_t mime;
	} supported_image_types[4];
	static int FillImageType(supported_image_types_t &image_type, const char *extension, const char *mime);
	nx_string_t album_file_specs[6];
	

#ifdef __APPLE__
private:
	int IsMatchingFile(nx_uri_t file, nx_string_t file_spec, size_t *extension_idx);
	int FillFileData(nx_uri_t file, size_t extension_idx, artwork_t *artwork, data_flags_t flags);
#endif	
};
