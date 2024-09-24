#pragma once
#include "metadata/svc_metadata.h"
#include "metadata/ifc_metadata.h"
#include "nswasabi/ServiceName.h"
#include "nx/nxuri.h"
#include "nx/nxfile.h"
#include "nsid3v2/nsid3v2.h"
#include "nsid3v1/nsid3v1.h"
#include "nsapev2/nsapev2.h"
#include "nswasabi/ID3v2Metadata.h"
#include "nswasabi/APEv2Metadata.h"
#include "nswasabi/ID3v1Metadata.h"
#include "ifc_filemetadata_editor.h"

// {6FCF1A5A-79D0-4FB8-9911-222638CA32A1}
static const GUID file_metadata_guid = 
{ 0x6fcf1a5a, 0x79d0, 0x4fb8, { 0x99, 0x11, 0x22, 0x26, 0x38, 0xca, 0x32, 0xa1 } };


class FileMetadataService : public svc_metadata
{
public:
	WASABI_SERVICE_NAME("File Metadata");
	WASABI_SERVICE_GUID(file_metadata_guid);
	
private:
	int WASABICALL MetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension);
	int WASABICALL MetadataService_CreateMetadata(unsigned int pass, nx_uri_t filename, ifc_metadata **metadata);
	int WASABICALL MetadataService_CreateMetadataEditor(unsigned int pass, nx_uri_t filename, ifc_metadata_editor **metadata);
};

class FileMetadata
{
public:
	ns_error_t SetFileInformation(nx_uri_t filename, nx_file_stat_t file_stat);
	ns_error_t FindMetadata(nx_file_t file);
	bool HasMetadata() const; // returns whether or not there was any metadata found
	
protected:
	FileMetadata();
	~FileMetadata();

	ns_error_t OwnID3v2(nsid3v2_tag_t id3v2, uint64_t position, uint64_t length);
	ns_error_t OwnID3v1(nsid3v1_tag_t id3v1, uint64_t position, uint64_t length);
	ns_error_t OwnAPEv2(nsapev2_tag_t apev2, uint64_t position, uint64_t length);
	ns_error_t OwnLyrics3(void *lyrics, uint64_t position, uint64_t length);

	nx_uri_t filename;
	nx_file_stat_s file_stat;

	template <class tag_t>
	class MetaTag
	{
	public:
		MetaTag() : tag(0), position(0), length(0) {}
		tag_t tag;
		uint64_t position;
		uint64_t length;
	};

	/* ID3v2 */
	MetaTag<nsid3v2_tag_t> id3v2;
	ID3v2Metadata id3v2_metadata;

	/* ID3v1 */
	MetaTag<nsid3v1_tag_t> id3v1;
	ID3v1Metadata id3v1_metadata;

	/* APEv2 */
	MetaTag<nsapev2_tag_t> apev2;
	APEv2Metadata apev2_metadata;

	MetaTag<void *> lyrics3;

	uint64_t start_position;
	uint64_t content_length;
	uint64_t end_position;
};

class FileMetadataRead : public FileMetadata, public ifc_metadata
{
private:
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	int WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data);
	int WASABICALL Metadata_GetMetadata(int field, unsigned int index, ifc_metadata **metadata);

	int WASABICALL Metadata_Serialize(nx_data_t *data);
};

class FileMetadataWrite : public FileMetadata, public ifc_metadata_editor
{
public:
	FileMetadataWrite();
	~FileMetadataWrite();
	int Initialize(ifc_filemetadata_editor *editor);
private:
		/* ifc_metadata_editor implementation */
	int WASABICALL MetadataEditor_Save();
	int WASABICALL MetadataEditor_SaveAs(nx_uri_t destination);

	int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value);
	int WASABICALL MetadataEditor_SetInteger(int field, unsigned int index, int64_t value);
	int WASABICALL MetadataEditor_SetReal(int field, unsigned int index, double value);
	int WASABICALL MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);

	int MakeID3v2();
	ifc_filemetadata_editor *editor;
};
