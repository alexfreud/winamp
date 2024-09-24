#include "ArtworkManager.h"
#include "api__wasabi2_metadata.h"
#include "nswasabi/ReferenceCounted.h"
#include "metadata/MetadataKeys.h"
#include "metadata/svc_metadata.h"
#ifdef __linux__
#include <alloca.h>
#include <unistd.h>
#include <dirent.h>
#elif defined (__APPLE__)
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(_WIN32)
#include <malloc.h>
#include <strsafe.h>
#endif

static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}

ArtworkManager::ArtworkManager()
{
	for (size_t i=0;i<sizeof(album_file_specs)/sizeof(*album_file_specs);i++)
	{
		album_file_specs[i]=0;
	}

	for (size_t i=0;i<sizeof(supported_image_types)/sizeof(*supported_image_types);i++)
	{
		supported_image_types[i].extension=0;
		supported_image_types[i].mime=0;
	}
}

int ArtworkManager::FillImageType(supported_image_types_t &image_type, const char *extension, const char *mime)
{
	int ret = NXStringCreateWithUTF8(&image_type.extension, extension);
	if (ret != NErr_Success)
		return ret;

	return NXStringCreateWithUTF8(&image_type.mime, mime);
}

int ArtworkManager::Initialize()
{
	int ret;

	ret = FillImageType(supported_image_types[0], "jpg", "image/jpeg");
	if (ret != NErr_Success)
		return ret;

	ret = FillImageType(supported_image_types[1], "jpeg", "image/jpeg");
	if (ret != NErr_Success)
		return ret;

	ret = FillImageType(supported_image_types[2], "png", "image/png");
	if (ret != NErr_Success)
		return ret;

	ret = FillImageType(supported_image_types[3], "gif", "image/gif");
	if (ret != NErr_Success)
		return ret;


	/* make the list of files to check for MetadataKeys::ALBUM */
	ret = NXStringCreateWithUTF8(&album_file_specs[0], "cover.");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&album_file_specs[1], "front.");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&album_file_specs[2], "album.");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&album_file_specs[3], "albumart.");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&album_file_specs[4], "Cover Art (front).");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&album_file_specs[5], "folder.");
	if (ret != NErr_Success)
		return ret;

	return NErr_Success;
}

void ArtworkManager::Shutdown()
{
	for (size_t i=0;i<sizeof(album_file_specs)/sizeof(*album_file_specs);i++)
	{
		NXStringRelease(album_file_specs[i]);
		album_file_specs[i]=0;
	}

	for (size_t i=0;i<sizeof(supported_image_types)/sizeof(*supported_image_types);i++)
	{
		NXStringRelease(supported_image_types[i].extension);
		supported_image_types[i].extension=0;
		NXStringRelease(supported_image_types[i].mime);
		supported_image_types[i].mime=0;
	}
}

nx_string_t ArtworkManager::GetFileSpecForField(unsigned int field, size_t index)
{
	switch(field)
	{
	case MetadataKeys::ALBUM:
		if (index < sizeof(album_file_specs)/sizeof(*album_file_specs))
			return album_file_specs[index];
		break;
	}
	return 0;
}

nx_string_t ArtworkManager::EnumerateExtension(size_t index)
{
	if (index < sizeof(supported_image_types)/sizeof(*supported_image_types))
		return supported_image_types[index].extension;
	return 0;
}

nx_string_t ArtworkManager::EnumerateMIME(size_t index)
{
	if (index < sizeof(supported_image_types)/sizeof(*supported_image_types))
		return supported_image_types[index].mime;
	return 0;
}


#ifdef _WIN32


static uint64_t FileTimeToUnixTime(FILETIME *ft)
{
	ULARGE_INTEGER end;
	memcpy(&end,ft,sizeof(end));
	end.QuadPart -= 116444736000000000;
	end.QuadPart /= 10000000; // 100ns -> seconds
	return end.QuadPart;
}

int ArtworkManager::FindValidFile(nx_uri_t filepath, nx_string_t file_spec, artwork_t *artwork, data_flags_t flags)
{
	size_t search_spec_length;

	search_spec_length = (filepath?filepath->len:0) + file_spec->len + 1 /* for * */ + 1 /* for null terminator */;
	wchar_t search_spec[MAX_PATH] = {0};

	if (filepath && filepath->len)
	{
		wchar_t path_end = filepath->string[filepath->len-1];
		StringCchPrintf(search_spec, search_spec_length, L"%s%s%s*", filepath->string, path_end?L"":L"\\", file_spec->string);

	}
	else
	{
		StringCchPrintf(search_spec, search_spec_length, L"%s*", file_spec->string);
	}

	WIN32_FIND_DATA find_data;
	HANDLE hFind = FindFirstFile(search_spec, &find_data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			ReferenceCountedNXURI filename;
			NXURICreateFromPath(&filename, find_data.cFileName, filepath);
			nx_string_t extension;
			size_t i;
			for (i=0, extension = EnumerateExtension(i);extension;extension = EnumerateExtension(++i))
			{
				if (NXPathMatchExtension(filename, extension) == NErr_True)
				{
					FindClose(hFind);
					if (artwork)
					{
						nx_data_t data=0;

						if (flags != DATA_FLAG_NONE)
						{
							int ret;
							if (TestFlag(flags, DATA_FLAG_DATA))
							{
								ret = NXDataCreateFromURI(&data, filename);
								if (ret != NErr_Success)
									return ret;
							}
							else
							{
								ret = NXDataCreateEmpty(&data);
								if (ret != NErr_Success)
									return ret;

								if (TestFlag(flags, DATA_FLAG_SOURCE_INFORMATION))
								{
									ret = NXDataSetSourceURI(data, filename);
									if (ret != NErr_Success)
									{
										NXDataRelease(data);
										return ret;
									}

									nx_file_stat_s stat_buffer;
									ret = NXFile_stat(filename, &stat_buffer);
									if (ret != NErr_Success)
									{
										NXDataRelease(data);
										return ret;
									}

									ret = NXDataSetSourceStat(data, &stat_buffer);
									if (ret != NErr_Success)
									{
										NXDataRelease(data);
										return ret;
									}
								}
							}

							if (TestFlag(flags, DATA_FLAG_MIME))
							{
								nx_string_t mime_type = EnumerateMIME(i);
								ret = NXDataSetMIME(data, mime_type);
								if (ret != NErr_Success)
								{
									NXDataRelease(data);
									return ret;
								}
							}


						}
						artwork->data = data;
						artwork->width = 0;
						artwork->height = 0;
					}

					return NErr_Success;
				}
			}
		} while (FindNextFileW(hFind, &find_data));
		FindClose(hFind);
	}

	return NErr_False;
}
#elif defined(__linux__)
static int match_extension(const char *filename, const char *extension)
{
	size_t a, b;

	if (!filename || !extension)
		return 0;

	a = strlen(filename);
	b = strlen(extension);

	if (!a)
		return 0;

	if (b > a)
		return 0;

	if (filename[a-b-1] != '.')
		return 0;

	do
	{
		if ((filename[--a] & ~0x20) != (extension[--b] & ~0x20))
			return 0;
	} while (b);

	return 1;
}

int ArtworkManager::FindValidFile(nx_uri_t filepath, nx_string_t file_spec[], size_t spec_count, artwork_t *artwork, data_flags_t flags)
{
	int ret;
	DIR *dir = opendir(filepath->string);
	if (!dir)
		return NErr_Empty;

	dirent entryp;

	struct dirent *entry;
	while(readdir_r(dir, &entryp, &entry) == 0 && entry)
	{
		for (size_t spec=0;spec<spec_count;spec++)
		{
		if (!strncasecmp(entry->d_name, file_spec[spec]->string, file_spec[spec]->len))
		{
			nx_string_t ext;
			size_t i;
			for (i=0, ext = EnumerateExtension(i);ext;ext = EnumerateExtension(++i))
			{
				if (match_extension(entry->d_name, ext->string))
				{
					closedir(dir);
					if (artwork)
					{
						ReferenceCountedNXURI uri_filename;
						ReferenceCountedNXURI filename;
						NXURICreateWithUTF8(&uri_filename, entry->d_name);
						NXURICreateWithPath(&filename, uri_filename, filepath);

						nx_data_t data=0;

						if (flags != DATA_FLAG_NONE)
						{
							if (TestFlag(flags, DATA_FLAG_DATA))
							{
								ret = NXDataCreateFromURI(&data, filename);
								if (ret != NErr_Success)
									return ret;
							}
							else
							{
								ret = NXDataCreateEmpty(&data);
								if (ret != NErr_Success)
									return ret;

								if (TestFlag(flags, DATA_FLAG_SOURCE_INFORMATION))
								{
									ret = NXDataSetSourceURI(data, filename);
									if (ret != NErr_Success)
									{
										NXDataRelease(data);
										return ret;
									}

									nx_file_stat_s stat_buffer;
									ret = NXFile_stat(filename, &stat_buffer);
									if (ret != NErr_Success)
									{
										NXDataRelease(data);
										return ret;
									}

									ret = NXDataSetSourceStat(data, &stat_buffer);
									if (ret != NErr_Success)
									{
										NXDataRelease(data);
										return ret;
									}
								}
							}

							if (TestFlag(flags, DATA_FLAG_MIME))
							{
								nx_string_t mime_type = EnumerateMIME(i);
								ret = NXDataSetMIME(data, mime_type);
								if (ret != NErr_Success)
								{
									NXDataRelease(data);
									return ret;
								}
							}


						}
						artwork->data = data;
						artwork->width = 0;
						artwork->height = 0;
					}
					return NErr_Success;
				}
			}
		}
		}
	}

	closedir(dir);		

	return NErr_False;
}
#elif defined(__APPLE__)


int ArtworkManager::IsMatchingFile(nx_uri_t file, nx_string_t file_spec, size_t *extension_idx)
{
	CFStringRef lastComponent = CFURLCopyLastPathComponent(file);
	if (NULL == lastComponent)
		return NErr_False;

	int err = NErr_False;
	CFIndex file_spec_len = CFStringGetLength(file_spec);

	if (kCFCompareEqualTo == CFStringCompareWithOptionsAndLocale(lastComponent, file_spec, 
		CFRangeMake(0, file_spec_len), 
		kCFCompareCaseInsensitive, NULL))
	{
		nx_string_t extension;
		size_t i;

		for (i=0, extension = EnumerateExtension(i);NULL != extension;extension = EnumerateExtension(++i))
		{
			CFIndex extension_len = CFStringGetLength(extension);
			if (kCFCompareEqualTo == CFStringCompareWithOptionsAndLocale(lastComponent, extension, 
				CFRangeMake(file_spec_len, extension_len), 
				kCFCompareCaseInsensitive, NULL))
			{
				if (NULL != extension_idx)
					*extension_idx = i;

				err = NErr_True;
			}

			if (NErr_False != err)
				break;
		}
	}

	CFRelease(lastComponent);
	return err;
}

int ArtworkManager::FillFileData(nx_uri_t file, size_t extension_idx,
								 artwork_t *artwork, data_flags_t flags)
{
	if (NULL == artwork)
		return NErr_BadParameter;
	
	nx_data_t data = NULL;
	ns_error_t ret;
	
	if (flags != DATA_FLAG_NONE)
	{
		if (TestFlag(flags, DATA_FLAG_DATA))
		{
			ret = NXDataCreateFromURI(&data, file);
			if (ret != NErr_Success)
				return ret;
		}
		else
		{
			ret = NXDataCreateEmpty(&data);
			if (ret != NErr_Success)
				return ret;
			
			if (TestFlag(flags, DATA_FLAG_SOURCE_INFORMATION))
			{
				ret = NXDataSetSourceURI(data, file);
				if (ret != NErr_Success)
				{
					NXDataRelease(data);
					return ret;
				}
				
				nx_file_stat_s stat_buffer;
				ret = NXFile_stat(file, &stat_buffer);
				if (ret != NErr_Success)
				{
					NXDataRelease(data);
					return ret;
				}
				
				ret = NXDataSetSourceStat(data, &stat_buffer);
				if (ret != NErr_Success)
				{
					NXDataRelease(data);
					return ret;
				}
			}
		}
		
		if (TestFlag(flags, DATA_FLAG_MIME))
		{
			nx_string_t mime_type = EnumerateMIME(extension_idx);
			ret = NXDataSetMIME(data, mime_type);
			if (ret != NErr_Success)
			{
				NXDataRelease(data);
				return ret;
			}
		}
		
		
	}
	artwork->data = data;
	artwork->width = 0;
	artwork->height = 0;
	
	return ret;
}

int ArtworkManager::FindValidFile(nx_uri_t directory, nx_string_t file_spec, 
								  artwork_t *artwork, data_flags_t flags)
{
	if (NULL == directory)
		return NErr_BadParameter;

	FSRef directory_ref;
	if (false == CFURLGetFSRef(directory, &directory_ref))
		return NErr_FileNotFound;

	int err;
	OSErr os_err;
	FSIterator iterator;

	os_err = FSOpenIterator(&directory_ref, kFSIterateFlat, &iterator);
	if (noErr != os_err)
		return NErr_Error;


	FSCatalogInfoBitmap catalog_info_bitmap = kFSCatInfoNodeFlags;

	const size_t maximum_count = ((4096 * 4) / (sizeof(FSCatalogInfo) + sizeof(FSRef)));

	FSCatalogInfo *catalog_info = (FSCatalogInfo *)malloc(maximum_count * sizeof(FSCatalogInfo));
	FSRef *item_ref = (FSRef *)malloc(maximum_count * sizeof(FSRef));
	if (NULL == item_ref 
		|| NULL == catalog_info)
	{
		err = NErr_OutOfMemory;
	}
	else 
	{
		err = NErr_False;
		while(noErr == os_err)
		{
			ItemCount actual_count;

			os_err = FSGetCatalogInfoBulk(iterator, maximum_count, &actual_count, NULL, 
				catalog_info_bitmap, catalog_info, 
				item_ref, NULL, NULL);

			// Process all items received
			if(noErr == os_err 
				|| errFSNoMoreItems == os_err)
			{
				UInt32  index;
				for(index = 0; index < actual_count; index += 1)
				{
					if(0 == (catalog_info[index].nodeFlags & kFSNodeIsDirectoryMask))
					{
						CFURLRef file_uri = CFURLCreateFromFSRef(NULL, &item_ref[index]);
						if (NULL != file_uri)
						{
							size_t extension_idx;
							if (NErr_True == IsMatchingFile(file_uri, file_spec, &extension_idx))
							{
								if (NULL != artwork
									&& DATA_FLAG_NONE != flags)
								{
									err = FillFileData(file_uri, extension_idx, artwork, flags);
								}

							}
							CFRelease(file_uri);

							if (NErr_False != err)
								break;
						}
					}
				}
			}

			if (NErr_False != err)
				break;
		}

	}

	FSCloseIterator(iterator);

	if (NULL != catalog_info)
		free(catalog_info);
	if (NULL != item_ref)
		free(item_ref);

	return err;
}

#else
// TODO: other ports
#endif

int ArtworkManager::Artwork_GetArtwork(nx_uri_t filename, unsigned int field, artwork_t *artwork, data_flags_t flags, nx_time_unix_64_t *out_file_modified)
{
	ifc_metadata *metadata = 0;
	int ret = REPLICANT_API_METADATA->CreateMetadata(&metadata, filename);

	// Step 1) try embedded
	if (ret == NErr_Success)
	{
		if (out_file_modified)
		{
			int64_t modified;
			if (metadata->GetInteger(MetadataKeys::FILE_TIME, 0, &modified) == NErr_Success)
				*out_file_modified = (nx_time_unix_64_t)modified;
			else
			{
				nx_file_stat_s stat_buffer;
				if (NXFile_stat(filename, &stat_buffer) == NErr_Success)
					*out_file_modified=stat_buffer.modified_time;
				else
					*out_file_modified=0;
			}
		}

		if (metadata->GetArtwork(field, 0, artwork, flags) == NErr_Success)
		{
			metadata->Release();
			return NErr_Success;
		}
	}
	else
	{
		if (out_file_modified)
		{
			nx_file_stat_s stat_buffer;
			if (NXFile_stat(filename, &stat_buffer) == NErr_Success)
				*out_file_modified=stat_buffer.modified_time;
			else
				*out_file_modified=0;
		}
	}

	ReferenceCountedNXURI filepath;
	NXURICreateRemovingFilename(&filepath, filename);

	// Step 2) try svc_art_provider


	// Step 3) album name.jpg
	if (metadata)
	{
		ReferenceCountedNXString field_name;
		if (metadata->GetField(field, 0, &field_name) == NErr_Success)
		{
			//	nx_uri_t filepart, search_spec;
			size_t byte_count=0;
			int ret = NXStringGetBytesSize(&byte_count, field_name, nx_charset_utf8, 0);
			if (ret == NErr_DirectPointer || ret == NErr_Success)
			{
				char *ptr = (char *)malloc(byte_count+3); // +1 for prefixed ".", +1 for "." and +1 for terminator
				if (ptr)
				{
					ptr[0]='.';
					size_t length;
					NXStringGetBytes(&length, field_name, ptr+1, byte_count, nx_charset_utf8, 0);
					length++; // adjust length because of the prefix
					for (size_t i=1;i<length;i++)
					{
						switch(ptr[i])
						{
						case '?':
						case '*':
						case  '|':
							ptr[i] = '_';
							break;
						case '/':
						case '\\':
						case ':':
							ptr[i] =  '-';
							break;
						case '\"': 
							ptr[i]  = '\'';
							break;
						case '<':
							ptr[i]  = '(';
							break;
						case '>':
							ptr[i] = ')';
							break;
						}
					}
					ptr[length++]='.';
					ptr[length]=0;

					// try with prefixed '.' first
					ReferenceCountedNXString file_spec;
					ret = NXStringCreateWithUTF8(&file_spec, ptr);
					if (ret == NErr_Success)
					{
#if defined(__linux__)
						nx_string_t temp = file_spec;
						ret = FindValidFile(filepath, &temp, 1, artwork, flags);
#else
						ret = FindValidFile(filepath, file_spec, artwork, flags);
#endif
						if (ret == NErr_Success)
						{
							metadata->Release();
							free(ptr);
							return NErr_Success;
						}
					}

					// now try with no prefixed '.'
					ret = NXStringCreateWithUTF8(&file_spec, ptr+1);
					free(ptr);
					if (ret == NErr_Success)
					{
#if defined(__linux__)
						nx_string_t temp = file_spec;
						ret = FindValidFile(filepath, &temp, 1, artwork, flags);
#else
						ret = FindValidFile(filepath, file_spec, artwork, flags);
#endif

						if (ret == NErr_Success)
						{
							metadata->Release();						
							return NErr_Success;
						}
					}
				}
			}
		}
	}

	if (metadata)
		metadata->Release();

	// Step 4) .nfo 

	// Step 5) <field>.jpg
#if defined(__linux__)
	if (field == MetadataKeys::ALBUM)
	{
		int ret = FindValidFile(filepath, album_file_specs, sizeof(album_file_specs)/sizeof(*album_file_specs), artwork, flags);
		if (ret == NErr_Success)
			return NErr_Success;
	}
#else
	size_t index=0;
	nx_string_t field_name;
	for (field_name = GetFileSpecForField(field, index);field_name;field_name = GetFileSpecForField(field, ++index))
	{

		int ret = FindValidFile(filepath, field_name, artwork, flags);

		if (ret == NErr_Success)
			return NErr_Success;		
	}
#endif
	return NErr_Empty;
}
