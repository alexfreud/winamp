#pragma once
#include "service/types.h"
#include "nx/nx.h"
#include "foundation/foundation.h"
#include "metadata/types.h"

// {60DB7F78-0238-4DA7-9260-87E60D30FAC4}
static const GUID artwork_api_guid = 
{ 0x60db7f78, 0x238, 0x4da7, { 0x92, 0x60, 0x87, 0xe6, 0xd, 0x30, 0xfa, 0xc4 } };

// ----------------------------------------------------------------------------
class NOVTABLE api_artwork : public Wasabi2::Dispatchable
{
protected:
	api_artwork()	: Dispatchable(DISPATCHABLE_VERSION) {}
	~api_artwork()	{}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return artwork_api_guid; }
	
	/* returns the data for the first piece of artwork found 
	pass NULL for any of the values that you don't care about */
	int GetArtwork(nx_uri_t filename, unsigned int field, artwork_t *artwork, data_flags_t flags=DATA_FLAG_ALL, nx_time_unix_64_t *file_modified=0)
	{
		return Artwork_GetArtwork(filename, field, artwork, flags, file_modified);
	}
	enum
	{
		DISPATCHABLE_VERSION,
	};

protected:
	virtual int WASABICALL Artwork_GetArtwork(nx_uri_t filename, unsigned int field, artwork_t *artwork, data_flags_t flags, nx_time_unix_64_t *file_modified)=0;
};

