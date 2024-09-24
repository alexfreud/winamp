/******************************************************************************
Plush Version 1.2
spline.c
n-th Dimensional Spline Interpolator
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

void plSplineGetPoint(pl_Spline *s, pl_Float frame, pl_Float *out) {
  pl_sInt32 i, i_1, i0, i1, i2;
  pl_Float time1,time2,time3;
  pl_Float t1,t2,t3,t4,u1,u2,u3,u4,v1,v2,v3;
  pl_Float a,b,c,d;

  pl_Float *keys = s->keys;

  a = (1-s->tens)*(1+s->cont)*(1+s->bias);
  b = (1-s->tens)*(1-s->cont)*(1-s->bias);
  c = (1-s->tens)*(1-s->cont)*(1+s->bias);
  d = (1-s->tens)*(1+s->cont)*(1-s->bias);
  v1 = t1 = -a / 2.0; u1 = a;
  u2 = (-6-2*a+2*b+c)/2.0; v2 = (a-b)/2.0; t2 = (4+a-b-c) / 2.0;
  t3 = (-4+b+c-d) / 2.0;
  u3 = (6-2*b-c+d)/2.0;
  v3 = b/2.0;
  t4 = d/2.0; u4 = -t4;

  i0 = (pl_uInt) frame;
  i_1 = i0 - 1;
  while (i_1 < 0) i_1 += s->numKeys;
  i1 = i0 + 1;
  while (i1 >= s->numKeys) i1 -= s->numKeys;
  i2 = i0 + 2;
  while (i2 >= s->numKeys) i2 -= s->numKeys;
  time1 = frame - (pl_Float) ((pl_uInt) frame);
  time2 = time1*time1;
  time3 = time2*time1;
  i0 *= s->keyWidth;
  i1 *= s->keyWidth;
  i2 *= s->keyWidth;
  i_1 *= s->keyWidth;
  for (i = 0; i < s->keyWidth; i ++) {
    a = t1*keys[i+i_1]+t2*keys[i+i0]+t3*keys[i+i1]+t4*keys[i+i2];
    b = u1*keys[i+i_1]+u2*keys[i+i0]+u3*keys[i+i1]+u4*keys[i+i2];
    c = v1*keys[i+i_1]+v2*keys[i+i0]+v3*keys[i+i1];
    *out++ = a*time3 + b*time2 + c*time1 + keys[i+i0];
  }
}
