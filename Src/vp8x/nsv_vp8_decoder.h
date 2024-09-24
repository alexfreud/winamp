#pragma once
#include "../nsv/svc_nsvFactory.h"
#include "../nsv/dec_if.h"
#define VPX_CODEC_DISABLE_COMPAT 1
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

// {9CF1837B-4A88-433d-B54B-9C783D39974F}
static const GUID vp8_nsv_guid = 
{ 0x9cf1837b, 0x4a88, 0x433d, { 0xb5, 0x4b, 0x9c, 0x78, 0x3d, 0x39, 0x97, 0x4f } };


class NSVFactory : public svc_nsvFactory
{
public:
	static const char *getServiceName() { return "VP8 NSV Decoder"; }
	static GUID getServiceGuid() { return vp8_nsv_guid; } 
	IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip);

protected:
	RECVS_DISPATCH;
};


class VP8_Decoder : public IVideoDecoder 
{
  public:
    VP8_Decoder(int w, int h);
    ~VP8_Decoder();
    int decode(int need_kf, 
            void *in, int in_len, 
            void **out, // out is set to a pointer to data
            unsigned int *out_type, // 'Y','V','1','2' is currently defined
            int *is_kf);
    void flush() { }
  private:
   	vpx_codec_ctx_t decoder;
    YV12_PLANES planes;
};
