#include "MP3Header.h"
#include "global.h"
#include "nmrCommon/stl/stringUtils.h"
#include "nmrCommon/services/stdServiceImpl.h"
#include "nmrCommon/unicode/uniString.h"

const __uint32 make28BitValue(const __uint8 buf[4])
{
	return ((((__uint32)buf[0]) << 21) | 
			(((__uint32)buf[1]) << 14) | 
			(((__uint32)buf[2]) <<  7) | 
			(((__uint32)buf[3])));
}

// Bitrates - use [version][layer][bitrate]
const __uint16 mpeg_bitrates[4][4][16] = {
  { // Version 2.5
    { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Reserved
    { 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 3
    { 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 2
    { 0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }  // Layer 1
  },
  { // Reserved
    { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Invalid
    { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Invalid
    { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Invalid
    { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }  // Invalid
  },
  { // Version 2
    { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Reserved
    { 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 3
    { 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 2
    { 0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }  // Layer 1
  },
  { // Version 1
    { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Reserved
    { 0,  32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0 }, // Layer 3
    { 0,  32,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0 }, // Layer 2
    { 0,  32,  64,  96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 }, // Layer 1
  }
};

// Sample rates - use [version][srate]
const __uint16 mpeg_srates[4][4] = {
    { 11025, 12000,  8000, 0 }, // MPEG 2.5
    {     0,     0,     0, 0 }, // Reserved
    { 22050, 24000, 16000, 0 }, // MPEG 2
    { 44100, 48000, 32000, 0 }  // MPEG 1
};

// Samples per frame - use [version][layer]
const __uint16 mpeg_frame_samples[4][4] = {
//    Rsvd     3     2     1  < Layer  v Version
    {    0,  576, 1152,  384 }, //       2.5
    {    0,    0,    0,    0 }, //       Reserved
    {    0,  576, 1152,  384 }, //       2
    {    0, 1152, 1152,  384 }  //       1
};

// Slot size (MPEG unit of measurement) - use [layer]
const __uint8 mpeg_slot_size[4] = { 0, 1, 1, 4 }; // Rsvd, 3, 2, 1

const char *MP3_FrameInfo::getLayerName() const
{
    switch (m_layer)
    {
        case 1: return "3";
        case 2: return "2";
        case 3: return "1";
    }
    return "unknown layer";
}

const char *MP3_FrameInfo::getVersionName() const
{
    switch (m_version)
    {
        case 0: return "v2.5";
        case 2: return "v2";
        case 3: return "v1";
    }
    return "unknown version";
}

int getMP3FrameSize (MP3_FrameInfo &info, const unsigned char *hdr, unsigned int len)
{
    if (len < 4)
        return 0;
    int samples = mpeg_frame_samples[info.m_version][info.m_layer];
    if (samples == 0)
        return -1;
    int bitrate = mpeg_bitrates [info.m_version] [info.m_layer] [((hdr[2] & 0xf0) >> 4)];
    if (bitrate == 0)
        return -1;
    info.m_bitrate = bitrate;
    info.m_samples = samples;

    return (int)(((float)(samples / 8.0) * (float)bitrate * 1000) /
            (float)info.m_samplerate) + (((hdr[2] & 0x02) >> 1) ? mpeg_slot_size[info.m_layer] : 0);
}

const int getMP3FrameInfo(const char *hdr, unsigned int *samplerate, int *bitrate, bool *mono)
{
    // Quick validity check
    if ( ( ((unsigned char)hdr[0] & 0xFF) != 0xFF)
      || ( ((unsigned char)hdr[1] & 0xE0) != 0xE0)   // 3 sync bits
      || ( ((unsigned char)hdr[1] & 0x18) == 0x08)   // Version rsvd
      || ( ((unsigned char)hdr[1] & 0x06) == 0x00)   // Layer rsvd
      || ( ((unsigned char)hdr[2] & 0xF0) == 0xF0)   // Bitrate rsvd
    ) return 0;

    // Data to be extracted from the header
    __uint8 ver = (hdr[1] & 0x18) >> 3;   // Version index
    __uint8 lyr = (hdr[1] & 0x06) >> 1;   // Layer index
    //__uint8 pad = (hdr[2] & 0x02) >> 1;   // Padding? 0/1
    //__uint8 brx = (hdr[2] & 0xf0) >> 4;   // Bitrate index
    __uint8 srx = (hdr[2] & 0x0c) >> 2;   // SampRate index

	if (mono)
	{
		*mono = (((hdr[3] >> 6) & 3) == 0x3);	// Channel mode
	}

    // Lookup real values of these fields
	*samplerate = mpeg_srates[ver][srx];
    *bitrate = mpeg_bitrates[ver][lyr][((hdr[2] & 0xf0) >> 4)];
    //__uint16 samples = mpeg_frame_samples[ver][lyr];
    //__uint8 slot_size = mpeg_slot_size[lyr];

    // Frame sizes are truncated integers
    return (__uint16)(((float)(mpeg_frame_samples[ver][lyr] / 8.0) *
			(float)*bitrate * 1000) / (float)mpeg_srates[ver][srx]) +
			(((hdr[2] & 0x02) >> 1) ? mpeg_slot_size[lyr] : 0);
}


const int getMP3FrameInfo (const unsigned char *hdr, unsigned int len, MP3_FrameInfo &info)
{
    // Quick validity check
    if ( len < 4
      || ( ((unsigned char)hdr[0] & 0xFF) != 0xFF)
      || ( ((unsigned char)hdr[1] & 0xE0) != 0xE0)   // 3 sync bits
      || ( ((unsigned char)hdr[1] & 0x18) == 0x08)   // Version rsvd
      || ( ((unsigned char)hdr[1] & 0x06) == 0x00)   // Layer rsvd
      || ( ((unsigned char)hdr[2] & 0xF0) == 0xF0)   // Bitrate rsvd
    ) return -1;

    // Data to be extracted from the header
    __uint8 ver = (hdr[1] & 0x18) >> 3;   // Version index
    __uint8 lyr = (hdr[1] & 0x06) >> 1;   // Layer index
    __uint8 srx = (hdr[2] & 0x0c) >> 2;   // SampRate index

    do
    {
        // Lookup real values of these fields
        unsigned int samplerate = mpeg_srates [ver][srx];
        if (samplerate == 0)
            break;

        int bitrate = mpeg_bitrates [ver][lyr][((hdr[2] & 0xf0) >> 4)];
        if (bitrate == 0)
            break;
        info.m_bitrate = bitrate;
        // the following are not supposed to change
        info.m_samplerate = samplerate;
        info.m_mono = (((hdr[3] >> 6) & 3) == 0x3);         // Channel mode
        info.m_layer = lyr;                                 // Layer index
        info.m_version = ver;
        if (info.m_pattern == 0)
            info.m_pattern = (unsigned long)(hdr[0]<<24 | (hdr[1]<<16) | (hdr[2]<<8) | hdr[0]) & info.m_mask;

        // DLOG (uniString::utf8("MPEG ") + info.getVersionName() + " layer " + info.getLayerName() + (info.m_mono ? " mono (" : " stereo (") + stringUtil::tos(bitrate) + "k)");

        return getMP3FrameSize (info, hdr, 4);
    } while (0);
    return -1;
}

int MP3_FrameInfo::verifyFrame (const unsigned char *buf, unsigned int len)
{
     if (len > 4)
     {
         unsigned long v = (unsigned long)(buf[0])<<24 | (buf[1]<<16) | (buf[2]<<8) | buf[0];

         if ((v & m_mask) == m_pattern)
         {
#if 0
             if (len > 40)
             {
                unsigned char str[6] = "LAME";
                int i;
                for (i=0; i < 5 && buf[36+i] == str[i]; i++)
                    ;
                if (str[i] == '\0') DLOG ("LAME header found");
             }
#endif
             return getMP3FrameSize (*this, buf, len);
         }
         // DLOG ("MPG: mask is " + stringUtil::tos(v&m_mask) + ", patt " + stringUtil::tos(m_pattern));
         return -1;
     }
     return 0;
}

MP3_FrameInfo::MP3_FrameInfo (unsigned long value) : parserInfo (0xFFFE0000, value)
{
    m_description = "MPEG ";
}

MP3_FrameInfo::MP3_FrameInfo(const unsigned char *p, unsigned int len) : parserInfo()
{
    m_mask = 0xFFFE0000;
    m_description = "MPEG ";
    getMP3FrameInfo (p, len, *this);
}

