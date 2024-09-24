#ifndef _VP3STUB_H_
#define _VP3STUB_H_

#include "main.h"

extern int vp3_postprocess;
extern int vp3_targetcpu;

IVideoDecoder *VP3_CREATE(int w, int h, double framerate, unsigned int fmt, int *flip);

#endif