#ifndef NULLSOFT_IN_MP3_ADTS_H
#define NULLSOFT_IN_MP3_ADTS_H

#include "ifc_mpeg_stream_reader.h"
#include <bfc/std_mkncc.h> // for MKnCC()
class adts
{
protected:
	adts() {}
	~adts() {}
public:
	static FOURCC getServiceType() { return MK4CC('a','d','t','s'); } 
	virtual int Initialize(bool forceMono, bool reverseStereo, bool allowSurround, int maxBits, bool allowRG, bool _useFloat = false, bool _useCRC = false)=0;
	virtual bool Open(ifc_mpeg_stream_reader *file)=0;
	virtual int Sync(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate)=0;
	virtual void CalculateFrameSize(int *frameSize)=0;
	virtual void GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate)=0;
	virtual void Flush(ifc_mpeg_stream_reader *file)=0;
	virtual void Close() = 0;

	enum
	{
		SUCCESS = 0,
		FAILURE=1,
		ENDOFFILE = 2,
		NEEDMOREDATA = 3,
		NEEDSYNC = 4,
	};
	virtual int Decode(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut)=0;
	virtual size_t GetCurrentBitrate()=0;
	virtual size_t GetDecoderDelay()=0;
	virtual int GetLayer()=0;
	virtual void Release()=0;
	virtual void SetDecoderHooks(void *layer3_vis, void *layer2_eq, void *layer3_eq) {}
};

#endif