/******************************************************************************
Plush Version 1.2
cam.c
Camera Control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

void plCamDelete(pl_Cam *c) {
  if (c) free(c);
}

void plCamSetTarget(pl_Cam *c, pl_Float x, pl_Float y, pl_Float z) {
  double dx, dy, dz;
  dx = x - c->X;
  dy = y - c->Y;
  dz = z - c->Z;
  c->Roll = 0;
  if (dz > 0.0001f) {
    c->Pan = (pl_Float) (-atan(dx/dz)*(180.0/PL_PI));
    dz /= cos(c->Pan*(PL_PI/180.0));
    c->Pitch = (pl_Float) (atan(dy/dz)*(180.0/PL_PI));
  } else if (dz < -0.0001f) {
    c->Pan = (pl_Float) (180.0-atan(dx/dz)*(180.0/PL_PI));
    dz /= cos((c->Pan-180.0f)*(PL_PI/180.0));
    c->Pitch = (pl_Float) (-atan(dy/dz)*(180.0/PL_PI));
  } else {
    c->Pan = 0.0f;
    c->Pitch = -90.0f;
  }
}

pl_Cam *plCamCreate(pl_uInt sw, pl_uInt sh, pl_Float ar, pl_Float fov,
                    pl_uChar *fb, pl_ZBuffer *zb) {
  pl_Cam *c;
  c = malloc(sizeof(pl_Cam));
  if (!c) return 0;
  memset(c,0,sizeof(pl_Cam));
  c->Fov = fov;
  c->AspectRatio = ar;
  c->ClipRight = c->ScreenWidth = sw;
  c->ClipBottom = c->ScreenHeight = sh;
  c->CenterX = sw>>1;
  c->CenterY = sh>>1;
  c->ClipBack = 8.0e30f;
  c->frameBuffer = fb;
  c->zBuffer = zb;
  c->Sort = 1;
  if (zb) c->Sort = 0;
  return (c);
}
