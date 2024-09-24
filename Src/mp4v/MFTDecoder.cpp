#include "MFTDecoder.h"
#include <Mfapi.h>
#include <wmcodecdsp.h>
#include <Mferror.h>


//-----------------------------------------------------------------------------
// GetDefaultStride
//
// Gets the default stride for a video frame, assuming no extra padding bytes.
//
//-----------------------------------------------------------------------------

static HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride)
{
	LONG lStride = 0;

	// Try to get the default stride from the media type.
	HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
	if (FAILED(hr))
	{
		// Attribute not set. Try to calculate the default stride.
		GUID subtype = GUID_NULL;

		UINT32 width = 0;
		UINT32 height = 0;

		// Get the subtype and the image size.
		hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
		if (SUCCEEDED(hr))
		{
			hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
		}
		if (SUCCEEDED(hr))
		{
			hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &lStride);
		}

		// Set the attribute for later reference.
		if (SUCCEEDED(hr))
		{
			(void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
		}
	}

	if (SUCCEEDED(hr))
	{
		*plStride = lStride;
	}
	return hr;
}


static HRESULT ConfigureOutput(IMFTransform *decoder, LONG *stride)
{
	HRESULT hr = S_OK;
	IMFMediaType *media_type = 0;
	AM_MEDIA_TYPE *format = NULL;
	int index=0;
	while(SUCCEEDED(hr)) {
		hr = decoder->GetOutputAvailableType(0, index++, &media_type);
		if (FAILED(hr)) {
			break;
		}
		media_type->GetRepresentation(FORMAT_MFVideoFormat, (LPVOID*)&format);
		MFVIDEOFORMAT* z = (MFVIDEOFORMAT*)format->pbFormat;
		unsigned int surface_format = z->surfaceInfo.Format;
		media_type->FreeRepresentation(FORMAT_MFVideoFormat, (LPVOID)format);

		if (surface_format == '21VY') { // MFVideoFormat_YV12
			hr = GetDefaultStride(media_type, stride);
			hr = decoder->SetOutputType(0, media_type, 0);
			break;
		}

	}
	if(media_type) {
		media_type->Release();
	}
	return hr;
}


MFTDecoder::MFTDecoder()
{
	decoder = 0;
	stride = 0;
	width = 0;
	height = 0;
}

MFTDecoder::~MFTDecoder()
{
	if (decoder) {
		decoder->Release();	
	}
}

static HRESULT CreateInputMediaType(IMFMediaType **_media_type, VIDEOINFOHEADER *header)
{
	HRESULT hr=E_FAIL;
	IMFMediaType *media_type=0;
	do {
		hr = MFCreateMediaType(&media_type);
		if (FAILED(hr)) {
			break;
		}

		hr = MFInitMediaTypeFromVideoInfoHeader(media_type, header, 88);
		if (FAILED(hr)) {
			break;
		}

		if (FAILED(hr)) {
			break;
		}

		*_media_type = media_type;
		return S_OK;
	} while(0);

	if (media_type) {
		media_type->Release();
	}
	return hr;
}

HRESULT MFTDecoder::Open(VIDEOINFOHEADER *header)
{
	HRESULT hr=E_FAIL;
	hr = CoCreateInstance(CLSID_CMpeg4sDecMFT, NULL, CLSCTX_INPROC_SERVER, __uuidof(IMFTransform), (void**)&decoder);

	if (FAILED(hr)) {
		return hr;
	}

	/* set input */
	IMFMediaType *media_type=0;
	hr = CreateInputMediaType(&media_type, header);
	if (FAILED(hr)) {
		return hr;
	}

	hr = decoder->SetInputType(0, media_type, 0);
	media_type->Release();
	if (FAILED(hr)) {
		return hr;
	}

	/* set output */

	ConfigureOutput(decoder, &stride);
	width=0;
	height=0;

	decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);

	return S_OK;
}

HRESULT MFTDecoder::GetOutputFormat(UINT *width, UINT *height, bool *flip, double *aspect)
{
	HRESULT hr=E_FAIL;
	IMFMediaType *media_type = 0;

	do {
		hr = decoder->GetOutputCurrentType(0, &media_type);
		if (FAILED(hr)) {
			break;
		}

		if (width && height) {
			hr = MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, width, height);
			if (FAILED(hr)) {
				break;
			}
		}

		if (flip) {
			LONG stride;
			hr = GetDefaultStride(media_type, &stride);
			if (FAILED(hr)) {
				break;
			}
			*flip = stride<0;
		}

		if (aspect) {
			MFRatio PAR = {0};
			hr = MFGetAttributeRatio(media_type, MF_MT_PIXEL_ASPECT_RATIO,
				(UINT32*)&PAR.Numerator,
				(UINT32*)&PAR.Denominator);

			if (FAILED(hr)) {
				*aspect = 1.0;
			} else {
				*aspect = (double)PAR.Numerator / (double)PAR.Denominator;
			}

		}
	} while(0);
	if (media_type) {
		media_type->Release();
	}
	return hr;
}

HRESULT MFTDecoder::Feed(const void *data, size_t data_size, uint64_t timestamp_hundred_nanos)
{
	HRESULT hr=E_FAIL;
	IMFMediaBuffer *buffer = 0;
	BYTE *buffer_pointer = 0;
	IMFSample *sample = 0;

	do {
		hr = MFCreateMemoryBuffer((DWORD)data_size, &buffer);
		if (FAILED(hr)) {
			break;
		}
		hr = buffer->Lock(&buffer_pointer, NULL, NULL);
		if (FAILED(hr)) {
			break;
		}

		memcpy(buffer_pointer, data, data_size);

		hr = buffer->Unlock();
		if (FAILED(hr)) {
			break;
		}

		hr = buffer->SetCurrentLength((DWORD)data_size);
		if (FAILED(hr)) {
			break;
		}

		hr = MFCreateSample(&sample);
		if (FAILED(hr)) {
			break;
		}

		hr = sample->AddBuffer(buffer);
		if (FAILED(hr)) {
			break;
		}

		hr = sample->SetSampleTime(timestamp_hundred_nanos);
		if (FAILED(hr)) {
			break;
		}

		hr = decoder->ProcessInput(0, sample, 0);
		if (FAILED(hr)) {
			break;
		}
	} while(0);
	if (buffer) {
		buffer->Release();
	}

	if (sample) {
		sample->Release();
	}
	return hr;
}

static HRESULT CreateOutputSample(IMFTransform *decoder, IMFSample **_output_sample)
{
	HRESULT hr=E_FAIL;
	MFT_OUTPUT_STREAM_INFO stream_info;
	IMFMediaBuffer *media_buffer = 0;
	IMFSample *sample = 0;

	do {
		hr = MFCreateSample(&sample);
		if (FAILED(hr)) {
			break;
		}

		hr = decoder->GetOutputStreamInfo(0, &stream_info);
		if (FAILED(hr)) {
			break;
		}

		hr = MFCreateAlignedMemoryBuffer(stream_info.cbSize, MF_16_BYTE_ALIGNMENT, &media_buffer);
		if (FAILED(hr)) {
			break;
		}

		hr = sample->AddBuffer(media_buffer);
		if (FAILED(hr)) {
			break;
		}

		if (media_buffer) {
			media_buffer->Release();
		}
		*_output_sample = sample;
		return S_OK;
	} while(0);
	if (sample) {
		sample->Release();
	}

	if (media_buffer) {
		media_buffer->Release();
	}
	return hr;
}

// Release the events that an MFT might allocate in IMFTransform::ProcessOutput().
static void ReleaseEventCollection(MFT_OUTPUT_DATA_BUFFER &pBuffers)
{
	if (pBuffers.pEvents) {
		pBuffers.pEvents->Release();
		pBuffers.pEvents = NULL;
	}

}
HRESULT MFTDecoder::GetFrame(IMFMediaBuffer **out_buffer, uint64_t *hundrednanos)
{
	HRESULT hr=E_FAIL;
	IMFSample *output_sample=0;
	DWORD mftStatus;
	do {
		hr = CreateOutputSample(decoder, &output_sample);
		if (FAILED(hr)) {
			break;
		}

		MFT_OUTPUT_DATA_BUFFER mftDataBuffer = {0, };
		mftDataBuffer.pSample = output_sample;
		mftStatus = 0;

		hr = decoder->ProcessOutput(0, 1, &mftDataBuffer, &mftStatus);
		if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
			break;
		}
		if (hr == MF_E_TRANSFORM_STREAM_CHANGE)	{
			ConfigureOutput(decoder, &stride);
			width=0;
			height=0;
		} else if (FAILED(hr)) {
			break;
		} else {
			if (mftDataBuffer.pSample) {
				IMFMediaBuffer *mediaBuffer;
				hr = mftDataBuffer.pSample->GetBufferByIndex(0, &mediaBuffer);
				if (FAILED(hr)) {
					break;
				}

				LONGLONG sample_time;
				output_sample->GetSampleTime(&sample_time);
				if (hundrednanos) {
					*hundrednanos = sample_time;
				}
				*out_buffer = mediaBuffer;
			}
			ReleaseEventCollection(mftDataBuffer);
		}
	} while (0);

	if (output_sample) {
		output_sample->Release();
	}

	return hr;
}

HRESULT MFTDecoder::Flush()
{
	return decoder->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
}

HRESULT MFTDecoder::Drain()
{
	return decoder->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, 0);
}

HRESULT MFTDecoder::GetFrame(YV12_PLANES **data, void **decoder_data, uint64_t *mft_timestamp)
{
	HRESULT hr=E_FAIL;
	IMFMediaBuffer *media_buffer = 0;
	IMFMediaType *media_type = 0;
	do {
		if (!height || !stride) {
			hr = decoder->GetOutputCurrentType(0, &media_type);
			if (FAILED(hr)) {
				break;
			}

			hr = MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, &width, &height);
			if (FAILED(hr)) {
				break;
			}

			hr = GetDefaultStride(media_type, &stride);
			if (FAILED(hr)) {
				break;
			}

		}
		hr = this->GetFrame(&media_buffer, mft_timestamp);
		if (FAILED(hr)) {
			break;
		}

		YV12_PLANES *planes = (YV12_PLANES *)malloc(sizeof(YV12_PLANES));
		IMF2DBuffer *buffer2d=0;
		if (SUCCEEDED(media_buffer->QueryInterface(&buffer2d))) {
			BYTE *pbScanline0;
			LONG pitch;
			buffer2d->Lock2D(&pbScanline0, &pitch);
			planes->y.baseAddr = pbScanline0;
			planes->y.rowBytes = pitch;
			pbScanline0 += pitch * height;
			planes->v.baseAddr = pbScanline0;
			planes->v.rowBytes = pitch/2;
			pbScanline0 += pitch * height/4;
			planes->u.baseAddr = pbScanline0;
			planes->u.rowBytes = pitch/2;
			buffer2d->Release();
		} else {
			DWORD length, max_length;
			BYTE *video_data;
			media_buffer->Lock(&video_data, &length, &max_length);

			planes->y.baseAddr = video_data;
			planes->y.rowBytes = stride;
			video_data += stride * height;
			planes->v.baseAddr = video_data;
			planes->v.rowBytes = stride/2;
			video_data += (stride/2) * (height/2);
			planes->u.baseAddr = video_data;
			planes->u.rowBytes = stride/2;
		}
		*data = planes;
		*decoder_data = media_buffer;
	} while(0);

	if (media_type) {
		media_type->Release();
	}

	return hr;
}

HRESULT MFTDecoder::FreeFrame(YV12_PLANES *data, void *decoder_data)
{
	IMFMediaBuffer *buffer= (IMFMediaBuffer *)decoder_data;
	if (buffer) {
		IMF2DBuffer *buffer2d=0;
		if (SUCCEEDED(buffer->QueryInterface(&buffer2d))) {
			buffer2d->Unlock2D();
			buffer2d->Release();
		} else {
			buffer->Unlock();
		}
		buffer->Release();
	}
	free(data);
	return S_OK;
}