#pragma once
#include "PlaybackBase.h"
#include "foundation/error.h"
#include "service/api_service.h"
/* this one does most of the work for you.  
It assumes blocking I/O and, generally, a simple implementation
You provide an implementation of PlaybackImpl */

class PlaybackBase2;

class PlaybackImpl
{
public:
	void Connect(PlaybackBase2 *playback, ifc_playback_parameters *playback_parameters);

	/* stuff you need to implement */

	/* destructor.  be diligent about closing stuff, can't guarantee that Close() got called beforehand */
	virtual ~PlaybackImpl();
	
	virtual ns_error_t Open(nx_uri_t filename)=0;
	/* you need to handle the possibility that Close gets called more than one time */
	virtual void Close()=0;
	virtual bool IsSeekable()=0;
	/* implementation note:  add a reference (Retain) before assigning the value */
	virtual ns_error_t GetMetadata(ifc_metadata **metadata)=0;
	/* if you set *exact=false, GetLength will get called after the next DecodeStep */
	virtual ns_error_t GetLength(double *length, bool *exact)=0;
	/* only return an error if you're in a state you can't recover from.  
	if you can't seek, then just don't seek and return NErr_Success */
	virtual ns_error_t Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position)=0;
	/* return NErr_Success to continue
	NErr_EndOfFile to indicate a natural end of file
	otherwise return an error */
	virtual ns_error_t DecodeStep()=0;
	/* Save information and close the OS file handle.
	fill resume_information with whatever information you'll need to resume */
	virtual ns_error_t Interrupt(Agave_Seek *resume_information)=0;
	/* During resume, be sure to call player->SetMetadata again */
	virtual ns_error_t Resume(Agave_Seek *resume_information)=0;
protected:
	
	PlaybackBase2 *playback;
	ifc_playback_parameters *playback_parameters;
	PlaybackImpl();
	
};

class PlaybackBase2 : public PlaybackBase
{
public:
	PlaybackBase2();
	~PlaybackBase2();

	ns_error_t Initialize(api_service *service_manager, PlaybackImpl *implementation, nx_uri_t filename, ifc_player *player);

	/* this two should ONLY be called from within your DecodeStep function! */
	ns_error_t OpenOutput(const ifc_audioout::Parameters *parameters); // if this returns an error, return it from DecodeStep()
	ns_error_t Output(const void *audio_data, size_t audio_data_length, double begin_position_seconds); // if this returns an error, return it from DecodeStep()
	ns_error_t OutputNonInterleaved(const void *audio_data, size_t audio_data_length, double begin_position_seconds); // if this returns an error, return it from DecodeStep()

private:
	static nx_thread_return_t NXTHREADCALL PlayerThreadFunction(nx_thread_parameter_t param);
	nx_thread_return_t NXTHREADCALL DecodeLoop();
	ns_error_t Init();
	ns_error_t WaitForClose();
	ns_error_t OutputWait();
	ns_error_t Internal_Interrupt();
	
	PlaybackImpl *implementation;
	api_filelock *filelocker;
	ifc_audioout *out;
	bool paused;
	double last_position;
	bool exact_length;
	ifc_audioout::Parameters parameters;
	const uint8_t **output_pointers;
};