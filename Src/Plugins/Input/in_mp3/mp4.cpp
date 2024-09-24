#include "vlb_sub/aacdecoder.h"
#include "vlbout.h"

void CStreamInfo::setSampleRate()
{
  SetSamplingRate(CChannelInfo::SamplingRateFromIndex(GetSamplingRateIndex ()));
}

#ifndef ACTIVEX_CONTROL
//methods used by in_mp4
extern "C" 
{
	__declspec( dllexport ) int aacGetBitBuffer()
	{
    return (int) new CBitBuffer;
	}

	__declspec( dllexport ) int aacGetDecoderInterfaces(CAacDecoder **decoder, CBitBuffer *buf, CStreamInfo **info, VLBOut **dataout)
	{
    *decoder=new CAacDecoder(*buf);
    *info=new CStreamInfo;
    *dataout=new VLBOut();
    return 1;
	}
	
	__declspec( dllexport ) void aacDeleteBitBuffer(CBitBuffer *bitBuffer)
		{
		if (bitBuffer)
			delete bitBuffer;
		}

	__declspec( dllexport ) void aacDeleteDecoderInterfaces(CAacDecoder *decoder, CStreamInfo *info, VLBOut *dataout)
		{
		if (decoder)
			delete decoder;
		if (info) 
			delete info;
		if (dataout)
			delete dataout;
		}

	
}
#endif