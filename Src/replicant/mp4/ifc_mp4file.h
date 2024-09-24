#pragma once
#include "foundation/dispatch.h"

static const uint32_t mp4file_invalid_track_id=0;

class ifc_mp4file : public Wasabi2::Dispatchable
{
	protected:
	ifc_mp4file() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_mp4file() {}
public:
	typedef uint32_t TrackID;
	typedef uint32_t SampleID;
	typedef uint64_t Timestamp;
	typedef uint64_t Duration;
	typedef struct metadata_itunes_atom_s {} *metadata_itunes_atom_t;

	int Free(void *buffer) { return MP4File_Free(buffer); }

	int Track_GetESConfiguration(TrackID track_number, uint8_t **buffer, uint32_t *buffer_size) { return MP4File_Track_GetESConfiguration(track_number, buffer, buffer_size); }
	int Track_GetMaxSampleSize(TrackID track_number, uint32_t *max_sample_size) { return MP4File_Track_GetMaxSampleSize(track_number, max_sample_size); }
	int Track_ConvertFromTimestamp(TrackID track_number, Timestamp timestamp, double *seconds) { return MP4File_Track_ConvertFromTimestamp(track_number, timestamp, seconds); }
	int Track_ConvertToDuration(TrackID track_number, double seconds, Duration *duration) { return MP4File_Track_ConvertToDuration(track_number, seconds, duration); }
	int Track_GetMediaDataName(TrackID track_number, const char **name) { return MP4File_Track_GetMediaDataName(track_number, name); }
	int Track_GetESDSObjectTypeID(TrackID track_number, uint8_t *type) { return MP4File_Track_GetESDSObjectTypeID(track_number, type); }
	int Track_GetAudioMPEG4Type(TrackID track_number, uint8_t *type) { return MP4File_Track_GetAudioMPEG4Type(track_number, type); }
	int Track_GetBytesProperty(TrackID track_number, const char *property_name, uint8_t **buffer, uint32_t *buffer_size) { return MP4File_Track_GetBytesProperty(track_number, property_name, buffer, buffer_size); }

	int Metadata_iTunes_FindFreeform(const char *name, const char *mean, metadata_itunes_atom_t *atom) { return MP4File_Metadata_iTunes_FindFreeform(name, mean, atom); }
	int Metadata_iTunes_GetBinary(metadata_itunes_atom_t atom, const uint8_t **value, size_t *value_length) { return MP4File_Metadata_iTunes_GetBinary(atom, value, value_length); }

	int Sample_Read(TrackID track_number, SampleID sample_number, uint8_t **bytes, uint32_t *bytes_length, Timestamp *start_time=0, Duration *duration=0, Duration *offset=0, int *is_sync=0) { return MP4File_Sample_Read(track_number, sample_number, bytes, bytes_length, start_time, duration, offset, is_sync); }
	int Sample_GetFromDuration(TrackID track_number, Duration duration, SampleID *sample_id) { return MP4File_Sample_GetFromDuration(track_number, duration, sample_id); }
	

	enum
	{
		esds_object_type_mpeg4_audio = 0x40,
		esds_object_type_mpeg2_aac_lc_audio = 0x67,
		esds_object_type_mpeg2_audio = 0x69,
		esds_object_type_mpeg1_audio	= 0x6B,
		
		mpeg4_audio_type_aac_lc = 2,
		mpeg4_audio_type_he_aac = 5,
		mpeg4_audio_type_layer1 = 32,
		mpeg4_audio_type_layer2 = 33,
		mpeg4_audio_type_layer3 = 34,
	};

	enum
	{
		DISPATCHABLE_VERSION,
	};
private:
	virtual int WASABICALL MP4File_Free(void *buffer)=0;

	virtual int WASABICALL MP4File_Track_GetESConfiguration(TrackID track_number, uint8_t **buffer, uint32_t *buffer_size)=0;
	virtual int WASABICALL MP4File_Track_GetMaxSampleSize(TrackID track_number, uint32_t *max_sample_size)=0;
	virtual int WASABICALL MP4File_Track_ConvertFromTimestamp(TrackID track_number, Timestamp timestamp, double *seconds)=0;
	virtual int WASABICALL MP4File_Track_ConvertToDuration(TrackID track_number, double seconds, Duration *duration)=0;
	virtual int WASABICALL MP4File_Track_GetMediaDataName(TrackID track_number, const char **name)=0;
	virtual int WASABICALL MP4File_Track_GetESDSObjectTypeID(TrackID track_number, uint8_t *type)=0;
	virtual int WASABICALL MP4File_Track_GetAudioMPEG4Type(TrackID track_number, uint8_t *type)=0;
	virtual int WASABICALL MP4File_Track_GetBytesProperty(TrackID track_number, const char *property_name, uint8_t **buffer, uint32_t *buffer_size)=0;

	virtual int WASABICALL MP4File_Metadata_iTunes_FindFreeform(const char *name, const char *mean, metadata_itunes_atom_t *atom)=0;
	virtual int WASABICALL MP4File_Metadata_iTunes_GetBinary(metadata_itunes_atom_t atom, const uint8_t **value, size_t *value_length)=0;

	virtual int WASABICALL MP4File_Sample_Read(TrackID track_number, SampleID sample_number, uint8_t **bytes, uint32_t *bytes_length, Timestamp *start_time, Duration *duration, Duration *offset, int *is_sync)=0;
	virtual int WASABICALL MP4File_Sample_GetFromDuration(TrackID track_number, Duration duration, SampleID *sample_id)=0;	
	
};
