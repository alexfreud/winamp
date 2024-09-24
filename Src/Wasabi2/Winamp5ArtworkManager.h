#pragma once
#include "metadata/api_artwork.h"

#include "nswasabi/ServiceName.h"
#include "nx/nx.h"

class Winamp5ArtworkManager : public api_artwork
{
public:
		WASABI_SERVICE_NAME("Winamp5 Artwork Manager API");
		
private:
	/* returns the data for the first piece of artwork found 
	pass NULL for any of the values that you don't care about */
	int WASABICALL Artwork_GetArtwork(nx_uri_t filename, unsigned int field, artwork_t *artwork, data_flags_t flags, nx_time_unix_64_t *filename_modified);

};
