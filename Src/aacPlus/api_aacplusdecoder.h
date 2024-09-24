#ifndef __WASABI_API_AACPLUSDECODER_H
#define __WASABI_API_AACPLUSDECODER_H

#include <bfc/dispatch.h>
#include "aacplusdectypes.h"
#include <bfc/platform/types.h>
class NOVTABLE api_aacplusdecoder : public Dispatchable
{
protected:
	api_aacplusdecoder() {}
	~api_aacplusdecoder() {}
public:
	DISPATCH_CODES
	{
		AACPLUSDECODER_EASY_OPEN = 10,
			AACPLUSDECODER_CLOSE = 20,
			AACPLUSDECODER_RESTART = 30,

			AACPLUSDECODER_READ_CONFIG_STREAM = 40,
			AACPLUSDECODER_GET_DECODER_SETTINGS_HANDLE = 50,
			AACPLUSDECODER_SET_DECODER_SETTINGS = 60,

			AACPLUSDECODER_GET_STREAM_PROPERTIES_HANDLE = 70,

			AACPLUSDECODER_STREAM_FEED =80,
			AACPLUSDECODER_STREAM_DECODE=90,
	
			AACPLUSDECODER_FRAME_DECODE=100,
	
	};

	AACPLUSDEC_ERROR EasyOpen(AACPLUSDEC_OUTPUTFORMAT outputFormat, int nMaxAudioChannels);
	void Close();
	AACPLUSDEC_ERROR Restart();

	AACPLUSDEC_ERROR ReadConfigStream(unsigned char *pucConfigStreamBufferIn, AACPLUSDEC_BITSTREAMBUFFERINFO *hConfigStreamBufferInfoInOut, AACPLUSDEC_CONFIGTYPE nConfigTypeIn, int bConfigStreamInBand, AACPLUSDEC_BITSTREAMFORMAT bitstreamFormatIn);
	AACPLUSDEC_EXPERTSETTINGS *GetDecoderSettingsHandle();
	AACPLUSDEC_ERROR SetDecoderSettings();

	AACPLUSDEC_STREAMPROPERTIES *GetStreamPropertiesHandle();

	AACPLUSDEC_ERROR StreamFeed(unsigned char *pucBitstrmBufIn, AACPLUSDEC_BITSTREAMBUFFERINFO *hBitstrmBufInfoInOut);
	AACPLUSDEC_ERROR StreamDecode(void  *pPcmAudioBufOut, AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut, unsigned char *pucDataStreamBufOut, AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut);
	AACPLUSDEC_ERROR FrameDecode(                    void          *pPcmAudioBufOut,                     
                    AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut,  
                    unsigned char *pucFrameBufferIn,                    
                    AACPLUSDEC_BITSTREAMBUFFERINFO *hFrameBufferInfoInOut,
                    int            bFrameCorrupt,                         
                    unsigned char *pucDataStreamBufOut,                   
                    AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut);
};

inline AACPLUSDEC_ERROR api_aacplusdecoder::EasyOpen(AACPLUSDEC_OUTPUTFORMAT outputFormat, int nMaxAudioChannels)
{
	return _call(AACPLUSDECODER_EASY_OPEN, (AACPLUSDEC_ERROR)AACPLUSDEC_ERROR_GENERALERROR, outputFormat, nMaxAudioChannels);
}

inline void api_aacplusdecoder::Close()
{
	_voidcall(AACPLUSDECODER_CLOSE); 
}

inline AACPLUSDEC_ERROR api_aacplusdecoder::Restart()
{ 
	return _call(AACPLUSDECODER_RESTART, (AACPLUSDEC_ERROR)AACPLUSDEC_ERROR_GENERALERROR); 
}

inline AACPLUSDEC_EXPERTSETTINGS *api_aacplusdecoder::GetDecoderSettingsHandle()
{
	return _call(AACPLUSDECODER_GET_DECODER_SETTINGS_HANDLE, (AACPLUSDEC_EXPERTSETTINGS *)0);
}

inline AACPLUSDEC_ERROR api_aacplusdecoder::SetDecoderSettings()
{
	return _call(AACPLUSDECODER_SET_DECODER_SETTINGS, (AACPLUSDEC_ERROR)AACPLUSDEC_ERROR_GENERALERROR);
}

inline AACPLUSDEC_STREAMPROPERTIES *api_aacplusdecoder::GetStreamPropertiesHandle()
{
	return _call(AACPLUSDECODER_GET_STREAM_PROPERTIES_HANDLE, (AACPLUSDEC_STREAMPROPERTIES *)0); }

inline AACPLUSDEC_ERROR api_aacplusdecoder::StreamFeed(unsigned char *pucBitstrmBufIn, AACPLUSDEC_BITSTREAMBUFFERINFO *hBitstrmBufInfoInOut)
{
	return _call(AACPLUSDECODER_STREAM_FEED, (AACPLUSDEC_ERROR)AACPLUSDEC_ERROR_GENERALERROR, pucBitstrmBufIn, hBitstrmBufInfoInOut);
}

inline AACPLUSDEC_ERROR api_aacplusdecoder::StreamDecode(void  *pPcmAudioBufOut, AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut, unsigned char *pucDataStreamBufOut, AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut)
{
	return _call(AACPLUSDECODER_STREAM_DECODE, (AACPLUSDEC_ERROR)AACPLUSDEC_ERROR_GENERALERROR, pPcmAudioBufOut, hPcmAudioBufInfoInOut, pucDataStreamBufOut, hDataStreamBufInfoInOut);
}

inline AACPLUSDEC_ERROR api_aacplusdecoder::ReadConfigStream(unsigned char *pucConfigStreamBufferIn, AACPLUSDEC_BITSTREAMBUFFERINFO *hConfigStreamBufferInfoInOut, AACPLUSDEC_CONFIGTYPE nConfigTypeIn, int bConfigStreamInBand, AACPLUSDEC_BITSTREAMFORMAT bitstreamFormatIn)
{
	return _call(AACPLUSDECODER_READ_CONFIG_STREAM, (AACPLUSDEC_ERROR)AACPLUSDEC_ERROR_GENERALERROR,  pucConfigStreamBufferIn, hConfigStreamBufferInfoInOut,  nConfigTypeIn, bConfigStreamInBand, bitstreamFormatIn);
}

inline AACPLUSDEC_ERROR api_aacplusdecoder::FrameDecode(void   *pPcmAudioBufOut,                     
AACPLUSDEC_AUDIOBUFFERINFO *hPcmAudioBufInfoInOut,  
                    unsigned char *pucFrameBufferIn,                    
                    AACPLUSDEC_BITSTREAMBUFFERINFO *hFrameBufferInfoInOut,
                    int            bFrameCorrupt,                         
                    unsigned char *pucDataStreamBufOut,                   
                    AACPLUSDEC_DATASTREAMBUFFERINFO *hDataStreamBufInfoInOut )
{
	return _call(AACPLUSDECODER_FRAME_DECODE, (AACPLUSDEC_ERROR)AACPLUSDEC_ERROR_GENERALERROR,  pPcmAudioBufOut,          
                    hPcmAudioBufInfoInOut,    
                    pucFrameBufferIn,         
                    hFrameBufferInfoInOut,    
                    bFrameCorrupt,                                pucDataStreamBufOut,      
                    hDataStreamBufInfoInOut  );
}

// {257C56BD-90CD-4f1f-9A27-3C73BCCCDE4E}
static const GUID aacPlusDecoderGUID= 
{ 0x257c56bd, 0x90cd, 0x4f1f, { 0x9a, 0x27, 0x3c, 0x73, 0xbc, 0xcc, 0xde, 0x4e } };

#endif