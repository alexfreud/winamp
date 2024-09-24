#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "metadata/ifc_metadata.h"

class ifc_audio_decoder_pull : public Wasabi2::Dispatchable
{
protected:
	ifc_audio_decoder_pull() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_audio_decoder_pull() {}
public:
    
	/* if possible, returns an upper bound on the number of frames used internally.  pull decoders are most optimal if you use this to malloc your buffer */
	int GetFrameSize(size_t *frame_size) { return AudioDecoderPull_GetFrameSize(frame_size); }
    int GetMetadata(ifc_metadata **metadata) { return AudioDecoderPull_GetMetadata(metadata); }
	 
	 /* returns
	 * NErr_EndOfFile when decode is done (frames_written will be valid, but probably 0)
	 * NErr_Success on successful decode, but not end-of-file (frames_written will be valid)
	 * anything else indicates a decode error */
    int Decode(void *buffer, size_t buffer_frames, size_t *frames_written) { return AudioDecoderPull_Decode(buffer, buffer_frames, frames_written); }
    /* You need to call Close() when you are done (even if you Release) because some implementations might have ifc_metadata being the same object */
    void Close() { AudioDecoderPull_Close(); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL AudioDecoderPull_GetFrameSize(size_t *frame_size)=0;
    virtual int WASABICALL AudioDecoderPull_GetMetadata(ifc_metadata **metadata)=0;
    virtual int WASABICALL AudioDecoderPull_Decode(void *buffer, size_t buffer_frames, size_t *frames_written)=0;
    virtual void WASABICALL AudioDecoderPull_Close()=0;
};
