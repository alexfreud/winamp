#include "mp3stub.h"
#include "../../mp3dec/mpgadecoder.h"

int mp3_quality;
int mp3_downmix;

class MP3_Decoder : public IAudioDecoder
{
  public:
    MP3_Decoder() : mp3_dec(mp3_quality,0,mp3_downmix) { fused=0; pcm_buf_used=0; }
    ~MP3_Decoder() { };
    int decode(void *in, int in_len, 
                       void *out, int *out_len,
                       unsigned int out_fmt[8]);
    void flush() { fused=0; pcm_buf_used=0; mp3_dec.Reset(); }
  private:
    CMpgaDecoder mp3_dec;
    char pcm_buf[1152*4*2];
    int pcm_buf_used;
    int pcm_offs;
    int fused;
};

int MP3_Decoder::decode(void *in, int in_len, 
                   void *out, int *out_len, 
                   unsigned int out_fmt[8]) 
{
  int rval=1;
  if (fused < in_len)
  {
    int l=mp3_dec.GetInputFree();
    if (l > in_len-fused) l=in_len-fused;
    if (l) mp3_dec.Fill((unsigned char *)in + fused,l);
    fused+=l;
  }

  if (!pcm_buf_used)
  {
    SSC s=mp3_dec.DecodeFrame((unsigned char *)pcm_buf,sizeof(pcm_buf),&pcm_buf_used);
    pcm_offs=0;
  }

  if (pcm_buf_used)
  {
    int l=*out_len;
    if (l > pcm_buf_used) l=pcm_buf_used;
    memcpy(out,pcm_buf+pcm_offs,l);
    pcm_buf_used-=l;
    pcm_offs+=l;
    *out_len=l;
  }
  else 
  {
    if (fused >= in_len) rval=fused=0;
    *out_len=0;
  }

  int nch=mp3_dec.m_Info.GetEffectiveChannels();
  int srate=mp3_dec.m_Info.GetEffectiveSFreq();
  out_fmt[0]=(nch && srate)?NSV_MAKETYPE('P','C','M',' '):0;
  out_fmt[1]=srate;
  out_fmt[2]=nch;
  out_fmt[3]=(nch && srate)?16:0;

  return rval;
}


IAudioDecoder *MP3_CREATE(unsigned int fmt)
{
  if (fmt == NSV_MAKETYPE('M','P','3',' ')) return new MP3_Decoder;
  return NULL;
}