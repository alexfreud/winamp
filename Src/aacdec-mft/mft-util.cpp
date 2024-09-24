#include "util.h"
#include <bfc/platform/types.h>
#include <mferror.h>
#include <mftransform.h>
#include <wmcodecdsp.h>
#include <Mfidl.h>
#include <Mfapi.h>

struct AACUserData {
  WORD         wPayloadType;
  WORD         wAudioProfileLevelIndication;
  WORD         wStructType;
  WORD         wReserved1;
  DWORD        dwReserved2;
  uint8_t buffer[16];
};

HRESULT GetAACCodec(IMFTransform **pDecoder)
{
	HRESULT hr = S_OK;
	UINT32 count = 0;

	IMFActivate **ppActivate = NULL;    // Array of activation objects.

	MFT_REGISTER_TYPE_INFO info = { MFMediaType_Audio, MFAudioFormat_AAC };

	UINT32 unFlags = MFT_ENUM_FLAG_SYNCMFT  | 
		MFT_ENUM_FLAG_LOCALMFT | 
		MFT_ENUM_FLAG_SORTANDFILTER;

	hr = MFTEnumEx(
		MFT_CATEGORY_AUDIO_DECODER,
		unFlags,
		&info,      // Input type
		NULL,       // Output type
		&ppActivate,
		&count
		);


	if (SUCCEEDED(hr) && count == 0)
	{
		hr = MF_E_TOPO_CODEC_NOT_FOUND;
	}

	// Create the first decoder in the list.

	if (SUCCEEDED(hr))
	{
		hr = ppActivate[0]->ActivateObject(IID_PPV_ARGS(pDecoder));
	}

	for (UINT32 i = 0; i < count; i++)
	{
		ppActivate[i]->Release();
	}
	CoTaskMemFree(ppActivate);
	return hr;
}

bool AssociateFloat(IMFTransform *decoder)
{
	IMFMediaType *media_type;
	DWORD index=0;
	for (;;)
	{
		GUID subtype;
		UINT32 bps;
		HRESULT hr = decoder->GetOutputAvailableType(0, index++, &media_type);
		if (hr != S_OK)
			return false;

		if (media_type->GetGUID(MF_MT_SUBTYPE, &subtype) != S_OK
			|| media_type->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bps) != S_OK)
		{
			media_type->Release();
			return false;
		}

		if (subtype == MFAudioFormat_Float && bps == 32)
		{
			hr = decoder->SetOutputType(0, media_type, 0);
			media_type->Release();

			return true;
		}
		media_type->Release();
	}
}

static uint32_t SampleRateForASC(const void *asc, size_t aacConfigLength)
{
	const uint8_t *pAacConfig = (const uint8_t *)asc;
	const uint32_t samplingRates[]={96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,0};
	int index;
	if (aacConfigLength >= 5 && (pAacConfig[4] >> 7) == 1)
	{
		index = (pAacConfig[4] >> 3) & 0x7;

	} else if (aacConfigLength >= 2) {
		index = ((pAacConfig[0] & 0x7) << 1) | (pAacConfig[1] >> 7); // bits 5-8
	} else {
		index = 12;
	}
	return samplingRates[index];
}

HRESULT CreateAACDecoder(IMFTransform **pDecoder, const void *asc, size_t asc_bytes)
{
	HRESULT hr;
	IMFMediaType *media_type;

	hr = GetAACCodec(pDecoder);
	if (FAILED(hr)) {
		return hr;
	}
	
	hr = MFCreateMediaType(&media_type);
	if (FAILED(hr)) {
		return hr;
	}
	hr = media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	hr = media_type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
	//
	hr = media_type->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, 0); // The stream contains raw_data_block elements only.
	uint8_t profile_level =0xFE;// MP4GetAudioProfileLevel(mp4_file);
	hr = media_type->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, profile_level); 
	media_type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, SampleRateForASC(asc, asc_bytes));
	
	// this is such a pain in the ass, it's part of an HEAACWAVEINFO struct packed together with the ASC data
	AACUserData aacinfo;
	aacinfo.wPayloadType = 0;
	aacinfo.wAudioProfileLevelIndication = profile_level;
	aacinfo.wStructType = 0;
		
	if (asc && asc_bytes) {
		if (asc_bytes > 16) { // don't overrun the buffer
			return E_FAIL;
		}
		memcpy(aacinfo.buffer, asc, asc_bytes);
		
		hr = media_type->SetBlob(MF_MT_USER_DATA, (const UINT8 *)&aacinfo, 12+(UINT)asc_bytes);
	}

	hr = (*pDecoder)->SetInputType(0, media_type, 0);
	if (FAILED(hr)) {
		return hr;
	}
	media_type->Release();
	media_type=0;

	AssociateFloat(*pDecoder);

	return S_OK;
}

HRESULT CreateADTSDecoder(IMFTransform **pDecoder)
{
	HRESULT hr;
	IMFMediaType *media_type;

	hr = GetAACCodec(pDecoder);
	if (FAILED(hr)) {
		return hr;
	}
	
	hr = MFCreateMediaType(&media_type);
	if (FAILED(hr)) {
		return hr;
	}
	hr = media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	hr = media_type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
	hr = media_type->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, 1); // ADTS
	//hr = media_type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16); // 16 bits per sample
	uint8_t profile_level =0xFE;// MP4GetAudioProfileLevel(mp4_file);
	hr = media_type->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, profile_level); 
	
	const BYTE user_data[] = {0x01, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	hr = media_type->SetBlob(MF_MT_USER_DATA, user_data, sizeof(user_data));

	hr = (*pDecoder)->SetInputType(0, media_type, 0);
	if (FAILED(hr)) {
		return hr;
	}
	media_type->Release();
	media_type=0;

	AssociateFloat(*pDecoder);

	return S_OK;
}
