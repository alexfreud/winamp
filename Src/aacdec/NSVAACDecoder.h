#pragma once
#include "../nsv/dec_if.h"
#include "incs/mp4AudioDecIfc.h"
#include <bfc/platform/types.h>
#include "ADTSHeader.h"
#include "../nsv/svc_nsvFactory.h"

// {55632E28-8171-4670-AE5D-CF714900C62E}
static const GUID NSV_AAC_GUID = 
{ 0x55632e28, 0x8171, 0x4670, { 0xae, 0x5d, 0xcf, 0x71, 0x49, 0x0, 0xc6, 0x2e } };

class NSVDecoder : public svc_nsvFactory
{
public:
	static const char *getServiceName() { return "AAC NSV Decoder"; }
	static GUID getServiceGuid() { return NSV_AAC_GUID; }
	IAudioDecoder *CreateAudioDecoder(FOURCC format, IAudioOutput **output);

protected:
	RECVS_DISPATCH;
};

class NSVAACDecoder : public IAudioDecoder
{
public:
	static NSVAACDecoder *CreateDecoder();
	NSVAACDecoder();
	~NSVAACDecoder();
	void Initialize(CAccessUnitPtr access_unit);
	int decode(void *in, int in_len, void *out, int *out_len, unsigned int out_fmt[8]);
	void flush();
	bool OK();

private:
	void FillOutputFormat(unsigned int out_fmt[8]);
	void CopyToOutput(void *out, int *out_len);

private:
	/* data */
	mp4AudioDecoderHandle decoder;
	CCompositionUnitPtr composition_unit; /* output */
	CAccessUnitPtr access_unit; /* input */
	
	int in_position;
	int out_left;
	size_t source_position;
//	unsigned char pcm_buf[65536*2];
//	int pcm_buf_used;
//	int readpos;
//	unsigned int cbvalid;
};

