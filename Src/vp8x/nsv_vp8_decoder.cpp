#include "api.h"
#include "nsv_vp8_decoder.h"
#include "../nsv/nsvlib.h"
#include <new>

IVideoDecoder *NSVFactory::CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip)
{
	if (fmt == NSV_MAKETYPE('V','P','8','0')) 
	{
		*flip=1;
		void *mem = WASABI_API_MEMMGR->sysMalloc(sizeof(VP8_Decoder));
		VP8_Decoder *dec = new (mem) VP8_Decoder(w,h);
		return dec;
	}
	return NULL;
}

#define CBCLASS NSVFactory
START_DISPATCH;
CB(SVC_NSVFACTORY_CREATEVIDEODECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS


VP8_Decoder::VP8_Decoder(int w, int h)
{
	vpx_codec_dec_init(&decoder, &vpx_codec_vp8_dx_algo, NULL, 0);
}


VP8_Decoder::~VP8_Decoder()
{
  vpx_codec_destroy(&decoder);
}

int VP8_Decoder::decode(int need_kf, 
        void *in, int in_len, 
        void **out, // out is set to a pointer to data
        unsigned int *out_type, // 'Y','V','1','2' is currently defined
        int *is_kf)
{
  unsigned char *data=(unsigned char *)in;

	if (in_len)
	{
		vpx_codec_decode(&decoder, (const uint8_t *)in, in_len, 0, 0);

		vpx_codec_stream_info_t stream_info;
		stream_info.sz = sizeof(stream_info);
		if (vpx_codec_get_stream_info(&decoder, &stream_info) == VPX_CODEC_OK)
		{
			*is_kf = stream_info.is_kf;
			if (need_kf && !stream_info.is_kf)
				return 0;
		}
	}

  *out_type=NSV_MAKETYPE('Y','V','1','2');
  
	vpx_codec_iter_t frame_iterator = 0;
	vpx_image_t *image = vpx_codec_get_frame(&decoder, &frame_iterator);
	if (image)
	{
		planes.y.baseAddr = image->planes[0];
		planes.y.rowBytes = image->stride[0];
		planes.u.baseAddr = image->planes[1];
		planes.u.rowBytes = image->stride[1];
		planes.v.baseAddr = image->planes[2];
		planes.v.rowBytes = image->stride[2];
		*out = &planes;
		return 0;
	}
  return 0;
}
