#include "avi_vp6_decoder.h"
#include "../nsv/nsvlib.h"
#include "../nsavi/nsavi.h"
#include "../libvp6/include/vp6.h"

int AVIDecoderCreator::CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	if (format)
	{
		if (format->compression == '26PV' || format->compression == '16PV' || format->compression == '06PV') 
		{
		DXL_XIMAGE_HANDLE xim = DXL_AlterXImage( NULL, (unsigned char *)"" ,NSV_MAKETYPE('V','P','6','0'), DXRGBNULL, 0, 0);
		if (!xim)
			return CREATEDECODER_FAILURE;
		*decoder = new AVIVP6(xim);
			return CREATEDECODER_SUCCESS;
		}
	}

	return CREATEDECODER_NOT_MINE;
}


#define CBCLASS AVIDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS

static const int vp6_postProcess=6;
static const int vp6_cpuFree=70;
static const int vp6_deInterlace=0;
static const int vp6_addNoise=1;

 enum
{
	PBC_SET_POSTPROC,
	PBC_SET_CPUFREE,
    PBC_MAX_PARAM,
	PBC_SET_TESTMODE,
	PBC_SET_PBSTRUCT,
	PBC_SET_BLACKCLAMP,
	PBC_SET_WHITECLAMP,
	PBC_SET_REFERENCEFRAME,
    PBC_SET_DEINTERLACEMODE,
    PBC_SET_ADDNOISE

} ;

 extern "C" 
 {
  void GetImageBufs(DXL_XIMAGE_HANDLE x, YV12_PLANES *p);
  void vp60_SetParameter(DXL_XIMAGE_HANDLE src, int Command, uintptr_t Parameter );
};


AVIVP6::AVIVP6(DXL_XIMAGE_HANDLE xim) : xim(xim)
{
	decoded=0;

	if(vp6_cpuFree)
		DXL_SetParameter(xim, PBC_SET_CPUFREE, vp6_cpuFree);
	else
		DXL_SetParameter(xim, PBC_SET_POSTPROC, vp6_postProcess);

	DXL_SetParameter(xim, PBC_SET_DEINTERLACEMODE, vp6_deInterlace );
	DXL_SetParameter(xim, PBC_SET_ADDNOISE, vp6_addNoise);
  DXL_SetParameter(xim, PBC_SET_BLACKCLAMP,0);
	DXL_SetParameter(xim, PBC_SET_WHITECLAMP,0);
}

int AVIVP6::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	if (xim)
	{
		if (vp60_getWH(xim, x, y) == DXL_OK)
		{
			*color_format = nsaviFOURCC('Y','V','1','2');
			*flip = 1;
			return AVI_SUCCESS;
		}
	}
	return AVI_FAILURE;
}

int AVIVP6::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
		uint8_t *vp6_data = (uint8_t *)inputBuffer;
	if (inputBufferBytes)
	{
		// skip first byte
		//vp6_data++;
		//inputBufferBytes--;
	
		DXL_AlterXImageData(xim, (unsigned char *)vp6_data);
		DXL_SetXImageCSize(xim, inputBufferBytes);
		if (!vp60_decompress(xim))
		{
			decoded=1;
			return AVI_SUCCESS;
		}
	}

	return AVI_FAILURE;
}

void AVIVP6::Flush()
{
	//if (decoder)
//		MPEG4Video_Flush(decoder);
}

int AVIVP6::GetPicture(void **data, void **decoder_data)
{
	if (decoded)
	{
		GetImageBufs(xim,&vidbufdec);
		*data=&vidbufdec;
		*decoder_data = 0;
		decoded = 0;
		return AVI_SUCCESS;
	}


	return AVI_FAILURE;
}

void AVIVP6::FreePicture(void *data, void *decoder_data)
{
	
}

void AVIVP6::HurryUp(int state)
{

}

#define CBCLASS AVIVP6
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(HURRY_UP, HurryUp)
END_DISPATCH;
#undef CBCLASS

