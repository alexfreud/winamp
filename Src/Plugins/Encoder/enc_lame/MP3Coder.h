#ifndef NULLSOFT_MP3_CODER_H
#define NULLSOFT_MP3_CODER_H

#include <windows.h>
#include "../nsv/enc_if.h"
#include "BladeMP3EncDLL.h"
#ifndef _BLADEDLL
extern BEINITSTREAM beInitStream;
extern BECLOSESTREAM beCloseStream;
extern BEENCODECHUNK beEncodeChunk;
extern BEDEINITSTREAM beDeinitStream;
extern BEWRITEVBRHEADER beWriteVBRHeader;
extern BEVERSION beVersion;
extern BEENCODECHUNKFLOATS16NI beEncodeChunkFloatS16NI;
#endif // !_BLADEDLL



typedef struct
{
	int bitrate;
	int vbr_max_bitrate;
	int abr_bitrate;
	int stereo_mode; //0=stereo,1=jstereo,2=mchannel,3=mono
	int quality; //0=normal,1=low,2=high,3=voice,4=r3mix,5=vh

	int vbr; // 0=high-9=low
	int vbr_method; // -1=none, 0=default, 1=old, 2=new, 3=mtrh, 4=abr

}
configtype;
class AudioCoderMP3 : public AudioCoder
{
public:
	AudioCoderMP3(int nch, int srate, int bps, configtype *cfg);
	int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail);
	virtual ~AudioCoderMP3();
	int GetLastError();
	void setVbrFilename(char *filename);
	void PrepareToFinish();
protected:
	int m_err;
	DWORD obuf_size;
	DWORD ibuf_size, ibuf_size_spls;
	HBE_STREAM hbeStream;
	BE_CONFIG beConfig;
	int bytesPerSample;
	int done;
	char *bs;
	int bs_size;
	int is_downmix;
	int mono_input;
};

class AudioCoderMP3_24 : public AudioCoderMP3
{
public:
	AudioCoderMP3_24(int nch, int srate, int bps, configtype *cfg) : AudioCoderMP3(nch, srate, bps, cfg) {}
int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail);
};

#endif