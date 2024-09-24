#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "metadata/ifc_metadata.h"

/* this is the class you actually use */
class ifc_audio_decoder_callback : public Wasabi2::Dispatchable
{
protected:
	ifc_audio_decoder_callback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_audio_decoder_callback() {}
public:
	/* you must implement this class to use the decoder */
	class callback : public Wasabi2::Dispatchable
	{
	protected:
		callback() : Dispatchable(DISPATCHABLE_VERSION) {}
		~callback() {}
	public:
		/* frames is defined as all channels, e.g. 16bit stereo is 4 bytes per frame (2 bytes per sample)
		return NErr_Success to continue receiving callbacks
		*/
		int OnAudio(const void *buffer, size_t buffer_frames) { return AudioDecoderCallback_OnAudio(buffer, buffer_frames); }

		enum
		{
			DISPATCHABLE_VERSION=0,
		};
	private:
		virtual int WASABICALL AudioDecoderCallback_OnAudio(const void *buffer, size_t buffer_frames)=0;
	};

	/* if possible, returns an upper bound on the number of frames used internally.  this would be the maximum buffer_frames value you receive in a callback */
	int GetFrameSize(size_t *frame_size) { return AudioDecoderCallback_GetFrameSize(frame_size); }
	int GetMetadata(ifc_metadata **metadata) { return AudioDecoderCallback_GetMetadata(metadata); }

	/* returns 
	* NErr_Success on a successfully completed decode
	* NErr_Interrupted if the callback function aborted decoding
	* anything else indicates a decoding error
	*/
	int Decode(ifc_audio_decoder_callback::callback *callback) { return AudioDecoderCallback_Decode(callback); }

	/* Like decode, but only processes one frame.  
	returns NErr_EndOfFile on the last frame */
	int DecodeStep(ifc_audio_decoder_callback::callback *callback) { return AudioDecoderCallback_DecodeStep(callback); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback)=0;
	virtual int WASABICALL AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback)=0;
	virtual int WASABICALL AudioDecoderCallback_GetFrameSize(size_t *frame_size)=0;
	virtual int WASABICALL AudioDecoderCallback_GetMetadata(ifc_metadata **metadata)=0;
};
