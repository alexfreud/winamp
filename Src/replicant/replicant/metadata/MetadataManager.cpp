#include "MetadataManager.h"
#include "metadata/genres.h"
#include "metadata/svc_metadata.h"
#include "nx/nxpath.h"

#include "api__wasabi2_metadata.h"

MetadataManager::MetadataManager()
{
	NXOnceInit(&fields_once);
	NXOnceInit(&genres_once);
 
	memset(&lookup, 0, sizeof(lookup));
	memset(&genres, 0, sizeof(genres));
}

MetadataManager::~MetadataManager()
{
	Shutdown(); // this should have already been called
}

void MetadataManager::Shutdown()
{
	// Release all the lookup entries
	for (size_t i = 0; i < MetadataKeys::NUM_OF_METADATA_KEYS; i++)
	{
		NXStringRelease(lookup[i]);
	}

	// Release all the extended lookup entries
	for ( nx_string_t l_lookup_extended : lookup_extended )
		NXStringRelease( l_lookup_extended );

	// Release all the extended lookup entries
	for (size_t i = 0; i < metadata_genre_list_size; i++)
	{
		NXStringRelease(genres[i]);
	}
	
	lookup_extended.clear();	
}

using namespace MetadataKeys;
#define METADATA_LOOKUP(x) { nx_string_t temp=0;  NXStringCreateWithUTF8(&temp, #x); me->lookup[x]=temp; }
int MetadataManager::InitializeMetadataFieldLookup(nx_once_t once, void *_me, void **unused)
{
	MetadataManager *me = (MetadataManager *)_me;
	
	METADATA_LOOKUP(ARTIST);
	METADATA_LOOKUP(ALBUM_ARTIST);
	METADATA_LOOKUP(ALBUM);
	METADATA_LOOKUP(TITLE);
	METADATA_LOOKUP(URI);
	METADATA_LOOKUP(GENRE);
	METADATA_LOOKUP(YEAR);
	METADATA_LOOKUP(TRACK);
	METADATA_LOOKUP(DISC);
	METADATA_LOOKUP(BITRATE);
	METADATA_LOOKUP(COMPOSER);
	METADATA_LOOKUP(PUBLISHER);
	METADATA_LOOKUP(BPM);
	METADATA_LOOKUP(COMMENT);
	METADATA_LOOKUP(DISCS);
	METADATA_LOOKUP(FILE_SIZE);
	METADATA_LOOKUP(FILE_TIME);
	METADATA_LOOKUP(LENGTH);
	METADATA_LOOKUP(PLAY_COUNT);
	METADATA_LOOKUP(RATING);
	METADATA_LOOKUP(SERVER);
	METADATA_LOOKUP(MIME_TYPE);
	METADATA_LOOKUP(TRACK_GAIN);
	METADATA_LOOKUP(TRACK_PEAK);
	METADATA_LOOKUP(ALBUM_GAIN);
	METADATA_LOOKUP(ALBUM_PEAK);
	METADATA_LOOKUP(TRACKS);
	METADATA_LOOKUP(PREGAP);
	METADATA_LOOKUP(POSTGAP);
	METADATA_LOOKUP(STAT);
	METADATA_LOOKUP(CATEGORY);
	METADATA_LOOKUP(DIRECTOR);
	METADATA_LOOKUP(PRODUCER);
	METADATA_LOOKUP(LAST_PLAY);
	METADATA_LOOKUP(LAST_UPDATE);
	return 1;
}
#undef METADATA_LOOKUP


int MetadataManager::InitializeGenres(nx_once_t once, void *_me, void **)
{
	MetadataManager *me = (MetadataManager *)_me;
	size_t num_genres = sizeof(metadata_genre_list) / sizeof(*metadata_genre_list);
	
	for (size_t i=0;i<num_genres;i++)
	{
		nx_string_t genre_name=0;  
		if (NXStringCreateWithUTF8(&genre_name, metadata_genre_list[i]) == NErr_Success)
			me->genres[i]=genre_name;					
	}
	return 1;
}


ns_error_t MetadataManager::Metadata_RegisterField(nx_string_t field_name, int *field)
{
	// see if we have already registered this field
	int ret = Metadata_GetFieldKey(field_name, field);
	if (ret == NErr_Success)
		return ret;

	//If not then register the field assign an id and pass it back
	nx_string_t added_field = NXStringRetain(field_name);
	lookup_extended.push_back(added_field);
	//ret = lookup_extended.push_back(added_field);
	//if (ret != NErr_Success)
	//{
	//	NXStringRelease(added_field);
	//	return ret;
	//}

	// Return the ID which is derived from the OFFSET (1000) zero based, so first id assigned will be 1000
	*field = (int)lookup_extended.size() + MetadataKeys::EXTENDED_KEYS_OFFSET - 1;
	return NErr_Success;
}

ns_error_t MetadataManager::Metadata_GetFieldKey(nx_string_t field_name, int *field)
{
	// Initialize our lookup table if this is the first time we are being called.
	NXOnce(&fields_once, InitializeMetadataFieldLookup, this);

	for (size_t i = 0; i < MetadataKeys::NUM_OF_METADATA_KEYS; i++)
	{
		if (lookup[i] && NXStringKeywordCompare(field_name, lookup[i]) == NErr_True)
		{
			*field = (int)i;
			return NErr_Success;
		}
	}

	for (size_t i = 0; i < lookup_extended.size(); i++)
	{
		if (NXStringKeywordCompare(field_name, lookup_extended[i]) == NErr_True)
		{
			*field = (int)i +  MetadataKeys::EXTENDED_KEYS_OFFSET;
			return NErr_Success;
		}
	}

	return NErr_Unknown;
}

ns_error_t MetadataManager::Metadata_GetFieldName(int field_key, nx_string_t *name)
{
	// Initialize our lookup table if this is the first time we are being called.
	NXOnce(&fields_once, InitializeMetadataFieldLookup, this);

	if (field_key >= MetadataKeys::EXTENDED_KEYS_OFFSET)
	{
		field_key -= MetadataKeys::EXTENDED_KEYS_OFFSET;
		if (field_key < (int)lookup_extended.size())
		{
			*name = NXStringRetain(lookup_extended.at(field_key));
			return NErr_Success;
		}
		else
			return NErr_Unknown;
	}

	if (field_key < MetadataKeys::NUM_OF_METADATA_KEYS)
	{
		*name = NXStringRetain(lookup[field_key]);
		return NErr_Success;
	}

	return NErr_Unknown;
}

ns_error_t MetadataManager::Metadata_GetGenre(uint8_t genre_id, nx_string_t *genre)
{
	NXOnce(&genres_once, InitializeGenres, this);
	if (genre_id >= metadata_genre_list_size)
		return NErr_Unknown;

	nx_string_t name = genres[genre_id];
	if (!name) /* shouldn't happen unless we were out of memory during load */
		return NErr_Unknown; 

	*genre = NXStringRetain(name);
	return NErr_Success;
}

ns_error_t MetadataManager::Metadata_GetGenreID(nx_string_t genre, uint8_t *genre_id)
{
	NXOnce(&genres_once, InitializeGenres, this);
	for (size_t i = 0; i < metadata_genre_list_size; i++)
	{
		nx_string_t name = genres[i];
		if (name && NXStringKeywordCompare(genre, name) == NErr_True)
		{
			*genre_id = (uint8_t)i;
			return NErr_Success;
		}
	}

	return NErr_Unknown;
}

//#include "nswasabi/AutoCharNX.h"

ns_error_t MetadataManager::Metadata_SupportedFilename(nx_uri_t filename)
{

	GUID metadata_guid = svc_metadata::GetServiceType();
	size_t n = WASABI2_API_SVC->GetServiceCount(metadata_guid);
	//printf("Looking through %d services for extensions\n", n );
	for (size_t i=0; i<n; i++)
	{
		ifc_serviceFactory *sf = WASABI2_API_SVC->EnumService(metadata_guid,i);
		if (sf)
		{	
			nx_string_t service_name = sf->GetServiceName();

			svc_metadata * l = (svc_metadata*)sf->GetInterface();
			if (l)
			{
				//printf("Looking through '%s' for extensions\n", AutoCharPrintfUTF8(service_name)  );
				for (unsigned int index=0;;index++)
				{
					nx_string_t nx_extension;
					if (l->EnumerateExtensions(index, &nx_extension) == NErr_Success)
					{
						//printf("Looking at extension, '%s'\n", AutoCharPrintfUTF8(nx_extension) );
						if (NXPathMatchExtension(filename, nx_extension) == NErr_True)
						{
							l->Release();
							NXStringRelease(nx_extension);
							return NErr_True;
						}
						NXStringRelease(nx_extension);
					}
					else
					{
						break;
					}
				}
				l->Release();
			}
		}
	}
	return NErr_False;
}

static ifc_metadata *FindMetadataTryAgain(size_t i, size_t n, nx_uri_t filename, svc_metadata *fallback)
{
	ifc_metadata *metadata;

	GUID metadata_guid = svc_metadata::GetServiceType();
	for (;i<n; i++)
	{
		ifc_serviceFactory *sf = WASABI2_API_SVC->EnumService(metadata_guid, i);
		if (sf)
		{	
			svc_metadata * l = (svc_metadata *)sf->GetInterface();
			if (l)
			{
				//printf("searching through, '%x'\n", l);
				ns_error_t ret = l->CreateMetadata(0, filename, &metadata);
				//printf("metadata creation returned, '%d'\n", ret);
				if (ret == NErr_Success)
				{
					l->Release();
					return metadata;
				}
				else if (ret == NErr_TryAgain)
				{
					metadata = FindMetadataTryAgain(i+1, n, filename, l);
					//printf("trying again got metadata, '%x'\n", metadata);
					l->Release();
					return metadata;
				}
				l->Release();
			}
		}
	}

	if (fallback && fallback->CreateMetadata(1, filename, &metadata) == NErr_Success)
	{
		return metadata;
	}

	return 0;
}

ns_error_t MetadataManager::Metadata_CreateMetadata(ifc_metadata **out_metadata, nx_uri_t filename)
{
	GUID metadata_guid = svc_metadata::GetServiceType();
	size_t n = WASABI2_API_SVC->GetServiceCount(metadata_guid);
	ifc_metadata *metadata = FindMetadataTryAgain(0, n, filename, 0);
	if (metadata)
	{
		*out_metadata = metadata;
		return NErr_Success;
	}
	else
		return NErr_NoMatchingImplementation;
}

static ifc_metadata_editor *FindMetadataEditorTryAgain(size_t i, size_t n, nx_uri_t filename, svc_metadata *fallback)
{
	ifc_metadata_editor *metadata;

	GUID metadata_guid = svc_metadata::GetServiceType();
	for (;i<n; i++)
	{
		ifc_serviceFactory *sf = WASABI2_API_SVC->EnumService(metadata_guid, i);
		if (sf)
		{	
			svc_metadata * l = (svc_metadata *)sf->GetInterface();
			if (l)
			{
				ns_error_t ret = l->CreateMetadataEditor(0, filename, &metadata);
				if (ret == NErr_Success)
				{
					l->Release();
					return metadata;
				}
				else if (ret == NErr_TryAgain)
				{
					metadata = FindMetadataEditorTryAgain(i+1, n, filename, l);
					l->Release();
					return metadata;
				}
				l->Release();
			}
		}
	}

	if (fallback && fallback->CreateMetadataEditor(1, filename, &metadata) == NErr_Success)
	{
		return metadata;
	}

	return 0;
}

ns_error_t MetadataManager::Metadata_CreateMetadataEditor(ifc_metadata_editor **out_metadata, nx_uri_t filename)
{
	GUID metadata_guid = svc_metadata::GetServiceType();
	size_t n = WASABI2_API_SVC->GetServiceCount(metadata_guid);
	ifc_metadata_editor *metadata = FindMetadataEditorTryAgain(0, n, filename, 0);
	if (metadata)
	{
		*out_metadata = metadata;
		return NErr_Success;
	}
	else
		return NErr_NoMatchingImplementation;
}
