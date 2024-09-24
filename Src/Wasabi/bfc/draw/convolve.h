#ifndef _CONVOLVE_H
#define _CONVOLVE_H

#include "platform/types.h"
// world's slowest crappiest convolve :P think it sucks? write a better one
// and send it to me

class Convolve3x3 {
public:
  Convolve3x3(ARGB32 *bits, int w, int h);

  void setPos(int x, int y, float v);
  void setMultiplier(int m);

  void convolve();

private:
  ARGB32 *bits;
  int w, h;
  float vals[3][3];
  float multiplier;
};

#endif
