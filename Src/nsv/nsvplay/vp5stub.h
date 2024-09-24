#ifndef _VP5STUB_H_
#define _VP5STUB_H_

#include "main.h"

IVideoDecoder *VP5_CREATE(int w, int h, double framerate, unsigned int fmt, int *flip);

#endif