#ifndef NULLSOFT_IN_MP3_ADTS_VLB_H
#define NULLSOFT_IN_MP3_ADTS_VLB_H
#include "adts.h"

#include "../vlb/obj_vlbDecoder.h"
#include "api__in_mp3.h"
#include <api/service/waServiceFactory.h>

class ADTS_VLB : public adts
{
public:
	ADTS_VLB();
	int Initialize( bool forceMono, bool reverseStereo, bool allowSurround, int maxBits, bool allowRG, bool _useFloat, bool _useCRC );

	bool Open( ifc_mpeg_stream_reader *file );
	void Close();

	void GetOutputParameters( size_t *numBits, int *numChannels, int *sampleRate );
	void CalculateFrameSize( int *frameSize );

	void Flush( ifc_mpeg_stream_reader *file );

	size_t GetCurrentBitrate();
	size_t GetDecoderDelay();

	int Sync( ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate );
	int Decode( ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut );

	int GetLayer();
	void Release();

private:
	obj_vlbDecoder *decoder;
	int needsync;
	AACStreamParameters params;
};
#endif
