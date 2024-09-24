#include "VLBDecoder.h"
VLBDecoder::VLBDecoder()
{
	decoder = 0;
}

VLBDecoder::~VLBDecoder()
{
	delete decoder;
}

int VLBDecoder::Open(DataIOControl *paacInput)
{
	if (decoder)
		return 1;

	decoder = new CAacDecoderApi(paacInput);
	return 0;
}

void VLBDecoder::Close()
{
	delete decoder;
	decoder=0;
}

long VLBDecoder::Synchronize(AACStreamParameters *paacStreamParameters)
{
	if (decoder)
	{
		long status = decoder->Synchronize(paacStreamParameters);
		if (status == 0)
		{
			info.ucNChannels = (unsigned char) paacStreamParameters->num_channels;
			info.uiSampleRate = paacStreamParameters->sampling_frequency;

			dataout.SetFormatInfo(&info);
		}
		return status;
	}
	else
		return ERR_SYNC_ERROR;
}

long VLBDecoder::DecodeFrame(AACStreamParameters *paacStreamParameters)
{
	if (decoder)
		return decoder->DecodeFrame(&dataout, paacStreamParameters);
	else
		return ERR_INVALID_BITSTREAM;
}

void VLBDecoder::Flush()
{
	dataout.Empty();
}

size_t VLBDecoder::Read(void *buffer, size_t bufferlen)
{
	if (dataout.BytesAvail() > 0)
	{
		size_t l = dataout.BytesAvail();
		if (l > bufferlen)
			l = bufferlen;
		dataout.PullBytes((unsigned char *)buffer, (int)l);
		return l;
	}
	return 0;
}

#define CBCLASS VLBDecoder
START_DISPATCH;
CB(OBJ_VLBDECODER_OPEN, Open)
VCB(OBJ_VLBDECODER_CLOSE, Close)
CB(OBJ_VLBDECODER_SYNCHRONIZE, Synchronize)
CB(OBJ_VLBDECODER_DECODEFRAME, DecodeFrame)
VCB(OBJ_VLBDECODER_FLUSH, Flush)
CB(OBJ_VLBDECODER_READ, Read)
END_DISPATCH;
