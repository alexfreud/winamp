#pragma once
#include "../nsv/dec_if.h"
#include "MFTDecoder.h"
#include <bfc/platform/types.h>
//#include "ADTSHeader.h"
#include "../nsv/svc_nsvFactory.h"

// {E890E15C-A6D8-4ed9-9204-85C77AB80C53}
static const GUID NSV_AAC_GUID = 
{ 0xe890e15c, 0xa6d8, 0x4ed9, { 0x92, 0x4, 0x85, 0xc7, 0x7a, 0xb8, 0xc, 0x53 } };

class NSVDecoder : public svc_nsvFactory
{
public:
	static const char *getServiceName() { return "MFT AAC NSV Decoder"; }
	static GUID getServiceGuid() { return NSV_AAC_GUID; }
	IAudioDecoder *CreateAudioDecoder(FOURCC format, IAudioOutput **output) override;

protected:
	RECVS_DISPATCH;
};

class NSVAACDecoder : public IAudioDecoder
{
public:
	static NSVAACDecoder *CreateDecoder();
	NSVAACDecoder();
	~NSVAACDecoder();
	void Initialize();
	int decode(void *in, int in_len, void *out, int *out_len, unsigned int out_fmt[8]);
	void flush();
	bool OK();

private:
	void FillOutputFormat(unsigned int out_fmt[8]);
	void CopyToOutput(void *out, int *out_len);

private:
	/* data */
	MFTDecoder decoder;	
	int in_position;
	int out_left;
	size_t source_position;
};

