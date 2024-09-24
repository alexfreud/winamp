#pragma once
#ifndef _MP3_HEADER_H
#define _MP3_HEADER_H

#include "nmrCommon/intTypes.h"
#include <stddef.h>
#include "global.h"
#include "uvox2Common.h"


struct MP3_FrameInfo : public parserInfo
{
    bool    m_mono;
    int     m_layer;
    int     m_samples;

    int verifyFrame (const unsigned char *buf, unsigned int len);

    MP3_FrameInfo (unsigned long value = 0);
    MP3_FrameInfo (const unsigned char *p, unsigned int len);

    const char *getLayerName() const;
    const char *getVersionName() const;
    int getUvoxType() { return MP3_DATA; }
};


const __uint32 make28BitValue(const __uint8 buf[4]);
const int getMP3FrameInfo(const char *hdr, unsigned int *samplerate, int *bitrate, bool *mono = 0);
const int getMP3FrameInfo (const unsigned char *hdr, unsigned int len, MP3_FrameInfo &info);
int getMP3FrameSize (MP3_FrameInfo &info, const unsigned char *hdr, unsigned int len);


#endif
