#pragma once
#include "player/svc_playback.h"
#include "nx/nxstring.h"
#include "nx/nxfile.h"
#include "nswasabi/ServiceName.h"
#include "nswasabi/PlaybackBase.h"
#include "ifc_fileplayback.h"
#include "replaygain/ifc_replaygain_settings.h"
#include "FileMetadata.h"

// {60DB6A52-1A82-4C0D-A947-203549260758}
static const GUID file_playback_guid = 
{ 0x60db6a52, 0x1a82, 0x4c0d, { 0xa9, 0x47, 0x20, 0x35, 0x49, 0x26, 0x7, 0x58 } };

class FilePlaybackService : public svc_playback
{
public:
	WASABI_SERVICE_NAME("File Playback");
	static GUID GetServiceGUID() { return file_playback_guid; }
	FilePlaybackService();
	int WASABICALL PlaybackService_CreatePlayback(unsigned int pass, nx_uri_t filename, ifc_player *player, ifc_playback **out_playback_object);
};

class FilePlayback : public PlaybackBase, public ifc_fileplayback_parent
{
public:
	FilePlayback();
	~FilePlayback();
	ns_error_t Initialize(nx_uri_t filename, ifc_player *player);
private:
	ns_error_t WASABICALL FilePlaybackParent_OpenOutput(const ifc_audioout::Parameters *parameters);
	ns_error_t WASABICALL FilePlaybackParent_Output(const void *audio_data, size_t audio_data_length, size_t *frames_consumed, double begin_position_seconds);
	ns_error_t WASABICALL FilePlaybackParent_OutputNonInterleaved(const void *audio_data, size_t audio_data_length, size_t *frames_consumed, double begin_position_seconds);
	ns_error_t WASABICALL FilePlaybackParent_OnMetadata(ifc_metadata *new_metadata);

	ifc_fileplayback *implementation;
	nx_file_t file;
	ifc_audioout *out;
	bool paused;
	double last_position;
	ns_error_t exact_length, exact_bitrate;
	ifc_audioout::Parameters parameters;
	const uint8_t **output_pointers;
	ifc_metadata *implementation_metadata;
	FileMetadataRead *metadata;

	ns_error_t Internal_Interrupt();
	ns_error_t OutputWait();
	ns_error_t WaitForClose();
	
	/* Thread function */
	static nx_thread_return_t NXTHREADCALL FilePlayerThreadFunction(nx_thread_parameter_t param);
	nx_thread_return_t NXTHREADCALL DecodeLoop();
};