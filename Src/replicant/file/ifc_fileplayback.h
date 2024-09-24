#pragma once
#include "audio/ifc_audioout.h"
#include "foundation/error.h"
#include "foundation/dispatch.h"
#include "metadata/ifc_metadata.h"
#include "player/types.h"
#include "nx/nxfile.h"

class ifc_fileplayback_parent : public Wasabi2::Dispatchable
{
protected:
	ifc_fileplayback_parent() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_fileplayback_parent() {}
public:
	// only call these functions during DecodeStep! 

	// if any of these return an error, return it from DecodeStep().
	// these return NErr_Aborted if there's a seek pending, and NErr_Interrupted if there is an interrupt pending
	ns_error_t OpenOutput(const ifc_audioout::Parameters *parameters) { return FilePlaybackParent_OpenOutput(parameters); }
	ns_error_t Output(const void *audio_data, size_t audio_data_length, size_t *frames_consumed, double begin_position_seconds) { return FilePlaybackParent_Output(audio_data, audio_data_length, frames_consumed, begin_position_seconds); }
	ns_error_t OutputNonInterleaved(const void *audio_data, size_t audio_data_length, size_t *frames_consumed, double begin_position_seconds) { return FilePlaybackParent_OutputNonInterleaved(audio_data, audio_data_length, frames_consumed, begin_position_seconds); }
	// call this if you have mid-stream metadata updates.  
	ns_error_t OnMetadata(ifc_metadata *new_metadata) { return FilePlaybackParent_OnMetadata(new_metadata); }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
protected:
	virtual ns_error_t WASABICALL FilePlaybackParent_OpenOutput(const ifc_audioout::Parameters *parameters)=0;
	virtual ns_error_t WASABICALL FilePlaybackParent_Output(const void *audio_data, size_t audio_data_length, size_t *frames_consumed, double begin_position_seconds)=0;
	virtual ns_error_t WASABICALL FilePlaybackParent_OutputNonInterleaved(const void *audio_data, size_t audio_data_length, size_t *frames_consumed, double begin_position_seconds)=0;
	virtual ns_error_t WASABICALL FilePlaybackParent_OnMetadata(ifc_metadata *new_metadata)=0;
};

class ifc_fileplayback : public Wasabi2::Dispatchable
{
protected:
	ifc_fileplayback() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_fileplayback() {}
public:
	void Close() { FilePlayback_Close(); }
	ns_error_t Seekable() { return FilePlayback_Seekable(); }
	ns_error_t GetMetadata(ifc_metadata **metadata) { return FilePlayback_GetMetadata(metadata); }
	ns_error_t GetLength(double *length, ns_error_t *exact) { return FilePlayback_GetLength(length, exact); }
	ns_error_t GetBitrate(double *bitrate, ns_error_t *exact) { return FilePlayback_GetBitrate(bitrate, exact); }
	ns_error_t Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position) { return FilePlayback_Seek(seek, seek_error, new_position); }
	ns_error_t DecodeStep() { return FilePlayback_DecodeStep(); }
	ns_error_t Interrupt(Agave_Seek *resume_information) { return FilePlayback_Interrupt(resume_information); }
	ns_error_t Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata) { return FilePlayback_Resume(resume_information, file, parent_metadata); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
protected:
	/* you need to handle the possibility that Close gets called more than one time */
	virtual void WASABICALL FilePlayback_Close()=0;
	virtual ns_error_t WASABICALL FilePlayback_Seekable()=0;
	/* implementation note:  add a reference (Retain) before assigning the value */
	virtual ns_error_t WASABICALL FilePlayback_GetMetadata(ifc_metadata **metadata)=0;
	/* if you set *exact=NErr_False, GetLength will get called after the next DecodeStep */
	virtual ns_error_t WASABICALL FilePlayback_GetLength(double *length, ns_error_t *exact)=0;
	virtual ns_error_t WASABICALL FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact)=0;
	/* only return an error if you're in a state you can't recover from.  
	if you can't seek, then just don't seek and return NErr_Success */
	virtual ns_error_t WASABICALL FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position)=0;
	/* return NErr_Success to continue
	NErr_EndOfFile to indicate a natural end of file
	otherwise return an error 
	do _not_ return NErr_Stopped, NErr_Aborted, or NErr_Interrupted, unless they were returned from an ifc_fileplayback_parent function, as these have special meaning	*/
	virtual ns_error_t WASABICALL FilePlayback_DecodeStep()=0;
	/* Save information and close the OS file handle.
	fill resume_information with whatever information you'll need to resume */
	virtual ns_error_t WASABICALL FilePlayback_Interrupt(Agave_Seek *resume_information)=0;
	/* During resume, be sure to call player->SetMetadata again */
	virtual ns_error_t WASABICALL FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata)=0;
};
