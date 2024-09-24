#include "flv_vp6_decoder.h"
#include "../nsv/nsvlib.h"
#include "../libvp6/include/vp6.h"
int FLVDecoderCreator::CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder)
{
	if (format_type == FLV::VIDEO_FORMAT_VP6 || format_type == FLV::VIDEO_FORMAT_VP62)
	{
		//DXL_XIMAGE_HANDLE xim = DXL_CreateXImageOfType((unsigned char *)"" , format_type == FLV::VIDEO_FORMAT_VP6?NSV_MAKETYPE('V','P','6','0'):NSV_MAKETYPE('V','P','6','2'));
		DXL_XIMAGE_HANDLE xim = DXL_AlterXImage( NULL, (unsigned char *)"" ,NSV_MAKETYPE('V','P','6','0'), DXRGBNULL, 0, 0);
		if (!xim)
			return CREATEDECODER_FAILURE;
		*decoder = new FLVVP6(xim);
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoderCreator::HandlesVideo(int format_type)
{
	if (format_type == FLV::VIDEO_FORMAT_VP6 || format_type == FLV::VIDEO_FORMAT_VP62)
	{
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS FLVDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
CB(HANDLES_VIDEO, HandlesVideo)
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

FLVVP6::FLVVP6(DXL_XIMAGE_HANDLE xim) : xim(xim)
{
	decoded=0;

	if(vp6_cpuFree)
		vp60_SetParameter(xim, PBC_SET_CPUFREE, vp6_cpuFree);
	else
		vp60_SetParameter(xim, PBC_SET_POSTPROC, vp6_postProcess);

	vp60_SetParameter(xim, PBC_SET_DEINTERLACEMODE, vp6_deInterlace );
	vp60_SetParameter(xim, PBC_SET_ADDNOISE, vp6_addNoise);
  vp60_SetParameter(xim, PBC_SET_BLACKCLAMP,0);
	vp60_SetParameter(xim, PBC_SET_WHITECLAMP,0);
}

int FLVVP6::GetOutputFormat(int *x, int *y, int *color_format)
{
	if (xim)
	{
		if (vp60_getWH(xim, x, y) == DXL_OK)
		{
			*color_format = NSV_MAKETYPE('Y','V','1','2');
			return FLV_VIDEO_SUCCESS;
		}
	}
	return FLV_VIDEO_FAILURE;
}

int FLVVP6::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp)
{
	uint8_t *vp6_data = (uint8_t *)inputBuffer;
	if (inputBufferBytes)
	{
		// skip first byte
		vp6_data++;
		inputBufferBytes--;
	
		DXL_AlterXImageData(xim, (unsigned char *)vp6_data);
		DXL_SetXImageCSize(xim, inputBufferBytes);
		if (!vp60_decompress(xim))
		{
			decoded=1;
			return FLV_VIDEO_SUCCESS;
		}
	}

	return FLV_VIDEO_FAILURE;

}

void FLVVP6::Close()
{
	if (xim)
		DXL_DestroyXImage(xim);

	delete this;
}

int FLVVP6::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (decoded)
	{
		GetImageBufs(xim,&vidbufdec);
		*data=&vidbufdec;
		*decoder_data = 0;
		decoded = 0;
		return FLV_VIDEO_SUCCESS;
	}

	return FLV_VIDEO_FAILURE;
}


#define CBCLASS FLVVP6
START_DISPATCH;
CB(FLV_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_VIDEO_DECODE, DecodeSample)
VCB(FLV_VIDEO_CLOSE, Close)
CB(FLV_VIDEO_GET_PICTURE, GetPicture)
END_DISPATCH;
#undef CBCLASS

