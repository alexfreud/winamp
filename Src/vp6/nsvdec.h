#pragma once
#include "duck_dxl.h"
#include "../nsv/dec_if.h"
class VP6_Decoder : public IVideoDecoder {
  public:
    VP6_Decoder(int w, int h);
    ~VP6_Decoder();
    int decode(int need_kf, 
            void *in, int in_len, 
            void **out, // out is set to a pointer to data
            unsigned int *out_type, // 'Y','V','1','2' is currently defined
            int *is_kf);
    void flush() { }

    void initMmx();

  private:
    int l_tcpu, l_pp;
    DXL_XIMAGE_HANDLE xim;
    YV12_PLANES vidbufdec;
};
