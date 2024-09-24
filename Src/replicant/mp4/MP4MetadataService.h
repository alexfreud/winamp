#pragma once
#include "metadata/svc_metadata.h"
#include "file/svc_filemetadata.h"
#include "nswasabi/ServiceName.h"
#include "MP4MetadataFile.h"
#include "nswasabi/MetadataChain.h"
#include "nx/nxonce.h"

// {71102FF2-D9CF-4a0e-9C83-858E80B65DD4}
static const GUID mp4_metadata_guid = 
{ 0x71102ff2, 0xd9cf, 0x4a0e, { 0x9c, 0x83, 0x85, 0x8e, 0x80, 0xb6, 0x5d, 0xd4 } };


class MP4MetadataService : public svc_metadata
{
public:
	WASABI_SERVICE_NAME("MP4 Metadata");
	WASABI_SERVICE_GUID(mp4_metadata_guid);
	MP4MetadataService();
	static nx_once_value_t mime_once;
private:
	int WASABICALL MetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension);
	int WASABICALL MetadataService_CreateMetadata(unsigned int pass, nx_uri_t filename, ifc_metadata **metadata);
	int WASABICALL MetadataService_CreateMetadataEditor(unsigned int pass, nx_uri_t filename, ifc_metadata_editor **metadata);
};

// {F3AD1E12-345C-4E00-83C1-4DAB2D88BF4F}
static const GUID mp4_file_metadata_guid = 
{ 0xf3ad1e12, 0x345c, 0x4e00, { 0x83, 0xc1, 0x4d, 0xab, 0x2d, 0x88, 0xbf, 0x4f } };

class MP4FileMetadataService : public svc_filemetadata
{
public:
	WASABI_SERVICE_NAME("MP4 File Metadata");
	WASABI_SERVICE_GUID(mp4_file_metadata_guid);
private:
	int WASABICALL FileMetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension);
	int WASABICALL FileMetadataService_CreateFileMetadata(ifc_metadata **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);	
};

class MP4FileMetadata : public MetadataChain<MP4MetadataFile>
{
public:
	MP4FileMetadata();
	~MP4FileMetadata();
	ns_error_t Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);

};