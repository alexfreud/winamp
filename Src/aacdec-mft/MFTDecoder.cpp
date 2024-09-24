#include "MFTDecoder.h"
#include "util.h"
#include "../nsutil/pcm.h"
#include <Mfapi.h>
#include <Mferror.h>

static // Release the events that an MFT might allocate in IMFTransform::ProcessOutput().
void ReleaseEventCollection(MFT_OUTPUT_DATA_BUFFER* pBuffers)
{

	if (pBuffers->pEvents)
	{
		pBuffers->pEvents->Release();
		pBuffers->pEvents = NULL;
	}

}

MFTDecoder::MFTDecoder()
{
	decoder=0;
	output_buffer=0;
	output_sample=0;
}

MFTDecoder::~MFTDecoder()
{
	if (decoder) {
		decoder->Release();
	}
	decoder = 0;

	if (output_buffer) {
		output_buffer->Release();
	}
	output_buffer = 0;

	if (output_sample) {
		output_sample->Release();
	}
	output_sample = 0;
}

HRESULT MFTDecoder::Open(const void *asc, size_t asc_bytes)
{
	HRESULT hr = CreateAACDecoder(&decoder, asc, asc_bytes);
	if (SUCCEEDED(hr)) {
		decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	}
	return hr;
}

HRESULT MFTDecoder::Open()
{
	HRESULT hr = CreateADTSDecoder(&decoder);
	if (SUCCEEDED(hr)) {
		decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
		decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
	}
	return hr;
}

void MFTDecoder::Flush()
{
	if (decoder) {
		decoder->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
	}
}

HRESULT MFTDecoder::GetOutputProperties(uint32_t *sampleRate, uint32_t *channels)
{
	HRESULT hr;
	IMFMediaType *media_type;
	UINT32 local_sample_rate, local_channels;

	if (!decoder) {
		return E_FAIL;
	}

	hr = decoder->GetOutputCurrentType(0, &media_type);

	if (FAILED(hr)) {
		return hr;
	}

	if (FAILED(hr=media_type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &local_sample_rate))
		|| FAILED(hr=media_type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &local_channels))) {
			media_type->Release();
			return hr;
	}

	*sampleRate = local_sample_rate;
	*channels = local_channels;

	return hr;
}

HRESULT MFTDecoder::Feed(const void *inputBuffer, size_t inputBufferBytes)
{
	HRESULT hr;
	if (inputBuffer && inputBufferBytes) {
		IMFMediaBuffer *media_buffer=0;
		IMFSample *media_sample=0;
		MFCreateMemoryBuffer((DWORD)inputBufferBytes, &media_buffer);
		MFCreateSample(&media_sample);
		media_sample->AddBuffer(media_buffer);

		BYTE *buffer;
		DWORD max_length, current_length;
		media_buffer->Lock(&buffer, &max_length, &current_length);
		memcpy(buffer, inputBuffer, inputBufferBytes);

		media_buffer->Unlock();
		media_buffer->SetCurrentLength((DWORD)inputBufferBytes);

		hr = decoder->ProcessInput(0, media_sample, 0);
		media_sample->Release();
		media_buffer->Release();
		media_sample=0;
		media_buffer=0;
	} else {
		decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
		decoder->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, 0);
	}
	return S_OK;
}

HRESULT MFTDecoder::Decode(void *outputBuffer, size_t *outputBufferBytes, unsigned int bitsPerSample, bool isFloat, double gain)
{
	HRESULT hr;

	if (!output_sample) {
		MFT_OUTPUT_STREAM_INFO output_stream_info;
		hr = decoder->GetOutputStreamInfo(0, &output_stream_info);
		if (FAILED(hr)) {
			return hr;
		}

		MFCreateMemoryBuffer(output_stream_info.cbSize, &output_buffer);
		MFCreateSample(&output_sample);
		output_sample->AddBuffer(output_buffer);
	}

	output_buffer->SetCurrentLength(0);
	MFT_OUTPUT_DATA_BUFFER output_data_buffer = {0, output_sample, 0, 0};
	DWORD status=0;
	hr = decoder->ProcessOutput(0, 1, &output_data_buffer, &status);
	if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
		*outputBufferBytes = 0;
		return S_OK;
	} else if (hr == MF_E_TRANSFORM_STREAM_CHANGE) {
		AssociateFloat(decoder);
		*outputBufferBytes = 0;
		return hr;
	}
	IMFMediaBuffer *decimation_buffer;
	hr = output_data_buffer.pSample->ConvertToContiguousBuffer(&decimation_buffer);
	float *pcm;
	DWORD max_length, current_length;
	decimation_buffer->Lock((BYTE **)&pcm, &max_length, &current_length);
	size_t num_samples = current_length / 4;
	*outputBufferBytes = num_samples*bitsPerSample/8;

	if (!isFloat)
	{
		nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, pcm, bitsPerSample, num_samples, (float)gain);
	}
	else
	{
		for (size_t i = 0;i != num_samples;i++)
			((float *)outputBuffer)[i] = pcm[i] * (float)gain;
	}

	decimation_buffer->Unlock();
	decimation_buffer->Release();

	ReleaseEventCollection(&output_data_buffer);

	return S_OK;
}

HRESULT MFTDecoder::OutputBlockSizeSamples(size_t *frameSize)
{
	HRESULT hr;
	MFT_OUTPUT_STREAM_INFO output_stream_info;

	if (!decoder) {
		return E_FAIL;
	}

	hr = decoder->GetOutputStreamInfo(0, &output_stream_info);
	if (FAILED(hr)) {
		return hr;
	}

	*frameSize = output_stream_info.cbSize;
	return hr;
}

bool MFTDecoder::AcceptingInput()
{
	DWORD flags;
	if (decoder && SUCCEEDED(decoder->GetInputStatus(0, &flags))) {
		if (flags & MFT_INPUT_STATUS_ACCEPT_DATA) {
			return true;
		}
	}
	return false;
}