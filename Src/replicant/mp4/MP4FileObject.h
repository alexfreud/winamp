#pragma once
#include "ifc_mp4file.h"
#include "MP4MetadataBase.h"
#include "mp4.h"

class MP4FileObject : public ifc_mp4file, public MP4MetadataBase
{
public:
	MP4FileObject();
	~MP4FileObject();
	void Initialize(nx_uri_t filename, MP4FileHandle file_handle);

private:
	MP4FileHandle file_handle;

private:
	int WASABICALL MP4File_Free(void *buffer);

	int WASABICALL MP4File_Track_GetESConfiguration(TrackID track_number, uint8_t **buffer, uint32_t *buffer_size);
	int WASABICALL MP4File_Track_GetMaxSampleSize(TrackID track_number, uint32_t *max_sample_size);
	int WASABICALL MP4File_Track_ConvertFromTimestamp(TrackID track_number, Timestamp timestamp, double *seconds);
	int WASABICALL MP4File_Track_ConvertToDuration(TrackID track_number, double seconds, Duration *duration);
	int WASABICALL MP4File_Track_GetMediaDataName(TrackID track_number, const char **name);
	int WASABICALL MP4File_Track_GetESDSObjectTypeID(TrackID track_number, uint8_t *type);
	int WASABICALL MP4File_Track_GetAudioMPEG4Type(TrackID track_number, uint8_t *type);
	int WASABICALL MP4File_Track_GetBytesProperty(TrackID track_number, const char *property_name, uint8_t **buffer, uint32_t *buffer_size);

	int WASABICALL MP4File_Metadata_iTunes_FindFreeform(const char *name, const char *mean, metadata_itunes_atom_t *atom);
	int WASABICALL MP4File_Metadata_iTunes_GetBinary(metadata_itunes_atom_t atom, const uint8_t **value, size_t *value_length);

	int WASABICALL MP4File_Sample_Read(TrackID track_number, SampleID sample_number, uint8_t **bytes, uint32_t *bytes_length, Timestamp *start_time, Duration *duration, Duration *offset, int *is_sync);
	int WASABICALL MP4File_Sample_GetFromDuration(TrackID track_number, Duration duration, SampleID *sample_id);	
};