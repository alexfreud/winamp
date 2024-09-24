#pragma once

#include "metadata/ifc_metadata.h"
#include "mp4.h"

class MP4MetadataBase : public ifc_metadata
{
public:
	MP4MetadataBase();
	~MP4MetadataBase();
	int Initialize(nx_uri_t filename, MP4FileHandle mp4_file);

	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	int WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }
private:
	MP4FileHandle mp4_file;
	nx_uri_t mp4_metadata_filename;
	nx_file_stat_s file_stats;
	static struct mime_types_t
	{
		nx_string_t jpeg;
		nx_string_t png;
		nx_string_t bmp;
		nx_string_t gif;
	} mime_types;
	static int NX_ONCE_API InitMIME(nx_once_t, void *, void **);
	static nx_string_t GetMIMEFromType(uint32_t type);
};