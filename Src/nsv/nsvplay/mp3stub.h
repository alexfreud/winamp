#ifndef _MP3STUB_H_
#define _MP3STUB_H_

#include "main.h"

extern int mp3_quality;
extern int mp3_downmix;
extern int mp3_downshit;

IAudioDecoder *MP3_CREATE(unsigned int fmt);

#endif