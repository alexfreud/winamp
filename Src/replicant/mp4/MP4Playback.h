#pragma once
#include "mp4.h"
#include "file/ifc_fileplayback.h"
#include "mp4/ifc_mp4audiodecoder.h"
#include "MP4MetadataBase.h"
#include "MP4FileObject.h"
#include "nswasabi/MetadataChain.h"

class MP4Playback : public ifc_fileplayback
{
public:
	MP4Playback();
	~MP4Playback();

	int Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);
private:
	ifc_mp4audiodecoder *audio_decoder;
	MP4FileHandle mp4_file;
	ifc_audioout::Parameters audio_parameters;
	MetadataChain<MP4FileObject> *mp4_file_object;
	bool output_opened;
	double samples_per_second;
	nx_file_t file;
	ifc_fileplayback_parent *parent;
	nx_uri_t filename;
private:
	int Init(nx_file_t file, ifc_metadata *parent_metadata);
	int Configure();
	
	void WASABICALL FilePlayback_Close();
	ns_error_t WASABICALL FilePlayback_Seekable();
	ns_error_t WASABICALL FilePlayback_GetMetadata(ifc_metadata **metadata);
	ns_error_t WASABICALL FilePlayback_GetLength(double *length, ns_error_t *exact);
	ns_error_t WASABICALL FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact);
	ns_error_t WASABICALL FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position);
	ns_error_t WASABICALL FilePlayback_DecodeStep();
	ns_error_t WASABICALL FilePlayback_Interrupt(Agave_Seek *resume_information);
	ns_error_t WASABICALL FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata);
};