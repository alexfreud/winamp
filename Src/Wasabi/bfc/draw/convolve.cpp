#include <precomp.h>

#include "convolve.h"

#define RED(a) (((a)>>16)&0xff)
#define GRN(a) (((a)>>8)&0xff)
#define BLU(a) (((a)&0xff))
#define ALP(a) (((a)>>24))

Convolve3x3::Convolve3x3(ARGB32 *_bits, int _w, int _h) : bits(_bits), w(_w), h(_h) {
  ZERO(vals);
  multiplier = 0;
}

void Convolve3x3::setPos(int x, int y, float v) {
  ASSERT(x >= -1 && x <= 1 && y >= -1 && y <= 1);
  vals[y+1][x+1] = v;
}

void Convolve3x3::setMultiplier(int m) {
  multiplier = m;
}

void Convolve3x3::convolve() {
  if (bits == NULL || w <= 0 || h <= 0) return;	// nothin'
  MemMatrix<ARGB32> temp(w, h, bits);	// make a copy
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      if (ALP(temp(x, y))<=1) continue;
      float ra=0, rg=0, rb=0;
      for (int a = -1; a <= 1; a++) {
        for (int b = -1; b <= 1; b++) {
          int px = x + a, py = y + b;
          if (px < 0 || px >= w || py < 0 || py >= h) continue;
          ARGB32 c = temp(px, py);
          if (ALP(c) <= 1) continue;
          ra += (float)RED(c) * vals[b][a] * multiplier;
          rg += (float)GRN(c) * vals[b][a] * multiplier;
          rb += (float)BLU(c) * vals[b][a] * multiplier;
        }
      } 
      unsigned int r = MINMAX((int)ra, 0, 255);
      unsigned int g = MINMAX((int)rg, 0, 255);
      unsigned int b = MINMAX((int)rb, 0, 255);
      unsigned int lum = MAX(MAX(r, g), b);
if (lum < 64) lum = 0;
else if (lum > 192) lum = 255;
      //bits[x+y*w] = (ALP(*temp.m(x, y))<<24)|(r<<16)|(g<<8)|(b);
      bits[x+y*w] &= 0xffffff;
      bits[x+y*w] |= (255-lum) << 24;
//if (lum < 64) {
//      bits[x+y*w] &= 0xffff00ff;
//      bits[x+y*w] |= lum << 8;
//} else {
//      bits[x+y*w] &= 0xff00ffff;
//      bits[x+y*w] |= lum << 16;
//}
    }
  }
}
