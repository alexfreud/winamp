#ifndef NULLSOFT_WINAMP_RESAMPLINGREADER_H
#define NULLSOFT_WINAMP_RESAMPLINGREADER_H

#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "Resampler.h"
#include "CommonReader.h"
// TODO: should probably pass in a sample frame size (nch*bps/8) so that we can have an integral number of samples in the buffer
#define RESAMPLE_BUFFERSIZE 1024
class ResamplingReader :public CommonReader
{
public:
	ResamplingReader(Resampler *_resampler, CommonReader *_reader, size_t inputFrameSize);
	~ResamplingReader();
	size_t ReadAudio(void *outputBuffer, size_t sizeBytes);
	

protected:
	RECVS_DISPATCH;

private:
	Resampler *resampler;
	CommonReader *reader;
	
	__int8 *buffer;
	size_t bufferAlloc;
	size_t bufferValid;

		enum ReadState
		{
			READING=0,
			ENDOFFILE=1,
			FLUSHING=2,
			DONE=3,
		};
		ReadState readState;
};

#endif