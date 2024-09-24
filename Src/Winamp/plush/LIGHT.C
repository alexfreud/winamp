/******************************************************************************
Plush Version 1.2
light.c
Light Control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

pl_Light *plLightSet(pl_Light *light, pl_uChar mode, pl_Float x, pl_Float y,
                     pl_Float z, pl_Float intensity, pl_Float halfDist) {
  pl_Float m[16], m2[16];
  light->Type = mode;
  light->Intensity = intensity;
  light->HalfDistSquared = halfDist*halfDist;
  switch (mode) {
    case PL_LIGHT_VECTOR:
      plMatrixRotate(m,1,x);
      plMatrixRotate(m2,2,y);
      plMatrixMultiply(m,m2);
      plMatrixRotate(m2,3,z);
      plMatrixMultiply(m,m2);
      plMatrixApply(m,0.0,0.0,-1.0,&light->Xp, &light->Yp, &light->Zp);
    break;
    case PL_LIGHT_POINT_ANGLE:
    case PL_LIGHT_POINT_DISTANCE:
    case PL_LIGHT_POINT:
      light->Xp = x;
      light->Yp = y;
      light->Zp = z;
    break;
  }
  return light;
}

pl_Light *plLightCreate() {
  pl_Light *l;
  l = malloc(sizeof(pl_Light));
  if (!l) return 0;
  memset(l,0,sizeof(pl_Light));
  return (l);
}

void plLightDelete(pl_Light *l) {
  if (l) free(l);
}
