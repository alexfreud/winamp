#include <windows.h>
#include "header_mpg.h"
#include <bfc/platform/types.h>

static int frameratecode2framerate[16] = {
  0,
  // Official mpeg1/2 framerates:
  24000*10000/1001, 24*10000,25*10000, 30000*10000/1001, 30*10000,50*10000,60000*10000/1001, 60*10000,
  // libmpeg3's "Unofficial economy rates":
  1*10000,5*10000,10*10000,12*10000,15*10000,0,0
}; 

static const int gaSampleRate[3][4] = 
{
  {44100, 48000, 32000, 0}, // MPEG-1
  {22050, 24000, 16000, 0}, // MPEG-2
  {11025, 12000,  8000, 0}, // MPEG-2.5
};

static const int gaBitrate[2][3][15] =
{
  {
  // MPEG-1
  {  0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448}, // Layer 1
  {  0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384}, // Layer 2
  {  0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320}, // Layer 3
  },

  {
  // MPEG-2, MPEG-2.5
  {  0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256}, // Layer 1
  {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160}, // Layer 2
  {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160}, // Layer 3
  },
};

#define MPEG_SYNCWORD 0x7ff
#define MPEG1          0
#define MPEG2          1
#define MPEG25         2
#define MODE_MONO      3

HeaderMpg::HeaderMpg() : fh(INVALID_HANDLE_VALUE)
{
  memset(buf, 0, sizeof(buf));
  pbuf = end = 0;
  m_BitrateNdx = m_SampleRateNdx = m_Layer = m_Id = m_Mode =
  m_Idex = video_bitrate = audio_bitrate = 0;
}

int HeaderMpg::mp3headerFromInt(unsigned long dwHdrBits)
{
  // header fields
  int m_Syncword;
  /*int m_CrcCheck;
  int m_Padding;
  int m_Private;
  int m_ModeExt;
  int m_Copyright;
  int m_Original;
  int m_Emphasis;*/

  // calculated data
  int   m_HeaderValid;

  // read header fields
  m_Syncword      =     (dwHdrBits >> 21) & 0x000007ff;
  m_Idex          =     (dwHdrBits >> 20) & 0x00000001;
  m_Id            =     (dwHdrBits >> 19) & 0x00000001;
  m_Layer         = 4 -((dwHdrBits >> 17) & 0x00000003);
  //m_CrcCheck      =   !((dwHdrBits >> 16) & 0x00000001);
  m_BitrateNdx    =     (dwHdrBits >> 12) & 0x0000000f;
  m_SampleRateNdx =     (dwHdrBits >> 10) & 0x00000003;
  /*m_Padding       =     (dwHdrBits >>  9) & 0x00000001;
  m_Private       =     (dwHdrBits >>  8) & 0x00000001;*/
  m_Mode          =     (dwHdrBits >>  6) & 0x00000003;
  /*m_ModeExt       =     (dwHdrBits >>  4) & 0x00000003;
  m_Copyright     =     (dwHdrBits >>  3) & 0x00000001;
  m_Original      =     (dwHdrBits >>  2) & 0x00000001;
  m_Emphasis      =     (dwHdrBits      ) & 0x00000003;*/

  // check if header is valid
/*  if ( 
       (m_Syncword      != MPEG_SYNCWORD) ||

#ifndef SYNC_ALL_LAYERS
       (m_Layer         !=  3           ) ||
#else
       (m_Layer         ==  4           ) ||
#endif

       (m_BitrateNdx    ==  15          ) ||
       (m_BitrateNdx    ==  0           ) ||
       (m_SampleRateNdx == 3            ) ||
       (m_Idex == 0 && m_Layer != 3     ) ||
       (m_Idex == 0 && m_Id == 1 && m_Layer == 3)
     )*/

    if ( 
       (m_Syncword      != MPEG_SYNCWORD) ||
       (m_Layer         ==  4           ) ||
       (m_BitrateNdx    ==  15          ) ||
       (m_BitrateNdx    ==  0           ) ||
       (m_SampleRateNdx == 3            ) ||
       (m_Idex == 0 && m_Layer != 3     ) ||
       (m_Idex == 0 && m_Id == 1        ) ||
       (m_Idex == 0 && m_BitrateNdx>8   )
     )
    {
    m_HeaderValid = 0;
//    ResetMembers();
    }
  else
    {
    m_HeaderValid = 1;
//    SetMembers();
    }

  return m_HeaderValid;
}

static __int64 FileSize64(HANDLE file)
{
	LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(file, (LPDWORD)&position.HighPart); 	
	
	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return INVALID_FILE_SIZE;
	else
		return position.QuadPart;
}

int HeaderMpg::getInfos(const wchar_t *filename, bool metadata) 
{
	fh = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if(fh==INVALID_HANDLE_VALUE) return 0;
  int ret=decodeHeader();
  if(ret) {
    int l=MulDiv(video_bitrate+audio_bitrate,2048,2018);
    l/=8;
		int64_t flen = FileSize64(fh);
    if(l)
			length=(int)((flen*1000ull) / (int64_t)l);
  }
CloseHandle(fh);
  return ret;
}

int HeaderMpg::decodeHeader() 
{
  DWORD bytesRead = 0;
  memset(buf,0,sizeof(buf));
  end=buf+sizeof(buf);
  ReadFile(fh, &buf, sizeof(buf), &bytesRead, NULL);
  
  pbuf=buf;
  while(1) {
    int code;
    code=sync_packet();
    if(!code) return 1;
    switch(code) {
    case 0x1b3: {
      if(has_video) break;

      pbuf+=4;
      if ((pbuf[6] & 0x20) != 0x20){
	      //printf("missing marker bit!\n");
	      //return 1;	/* missing marker_bit */
        break;
      }

      int height = (pbuf[0] << 16) | (pbuf[1] << 8) | pbuf[2];

      video_w = (height >> 12);
      video_h = (height & 0xfff);

      int width = ((height >> 12) + 15) & ~15;
      height = ((height & 0xfff) + 15) & ~15;

      if ((width > 768) || (height > 576)){
	      //printf("size restrictions for MP@ML or MPEG1 exceeded! (%dx%d)\n",width,height);
        //	return 1;	/* size restrictions for MP@ML or MPEG1 */
        break;
      }
    
      has_video=true;

      int bitrate=(pbuf[4]<<10)|(pbuf[5]<<2)|(pbuf[6]>>6);
      if(bitrate==262143) {
        //variable bitrate
        //has_video=false;
        break;
      }
      bitrate=bitrate*2/5;
      video_bitrate=bitrate*1000;
      break;
    }
    case 0x1be: {
      // padding stream
      pbuf+=4;
      int s=pbuf[0]<<8|pbuf[1];
      pbuf+=2+s;
      break;
    }
    }
    if(code>=0x1c0 && code<=0x1df) {
     
      pbuf+=4;
      int len=pbuf[0]<<8|pbuf[1];
      pbuf+=2;
      pbuf+=5;
			if (len > (end-pbuf))
				return 0;
      // decode mpeg audio header
      while(--len) {
        if(mp3headerFromInt((pbuf[0]<<24)|(pbuf[1]<<16)|(pbuf[2]<<8)|pbuf[3])) {
          has_audio=true;
          audio_bps=16;
          int m_MpegVersion = m_Id==1 ? MPEG1 : (m_Idex==1?MPEG2 : MPEG25);
          audio_nch = m_Mode == MODE_MONO ? 1:2;
          audio_srate  = gaSampleRate[m_MpegVersion][m_SampleRateNdx];
          audio_bitrate = gaBitrate[m_MpegVersion==MPEG1?0:1][m_Layer-1][m_BitrateNdx] * 1000;
          break;
        }
        pbuf++;
      }
    }
    if(has_audio && has_video) return 1;
    pbuf++;
  }

  return 1;
}
