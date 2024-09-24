#pragma once
#include "mp4.h"
#include "metadata/ifc_metadata_editor.h"
#include "nx/nxuri.h"

class MP4MetadataEditor : public ifc_metadata_editor
{
public:
	MP4MetadataEditor();
	~MP4MetadataEditor();
	int Initialize(nx_uri_t filename);

private:
	MP4FileHandle mp4_file;

	int WASABICALL MetadataEditor_Save();

	int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value);
	int WASABICALL MetadataEditor_SetInteger(int field, unsigned int index, int64_t value);
	int WASABICALL MetadataEditor_SetReal(int field, unsigned int index, double value);

	int WASABICALL MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	nx_uri_t filename;
};