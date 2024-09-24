/******************************************************************************
Plush Version 1.2
pf_ptex.c
Perspective Correct Texture Mapping Rasterizers
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"
#include "putface.h"

void plPF_PTexF(pl_Cam *cam, pl_Face *TriFace) {
  pl_uChar i0, i1, i2;
  pl_uChar *gmem = cam->frameBuffer;
  pl_uChar *remap = TriFace->Material->_ReMapTable;
  pl_ZBuffer *zbuf = cam->zBuffer;
  pl_Float MappingU1, MappingU2, MappingU3;
  pl_Float MappingV1, MappingV2, MappingV3;
  pl_sInt32 MappingU_AND, MappingV_AND;
  pl_uChar *texture;
  pl_uChar vshift;
  pl_uInt16 bc;
  pl_Texture *Texture;
  pl_sInt32 iShade;

  pl_uChar nm, nmb;
  pl_sInt n;
  pl_Float U1,V1,U2,V2,dU1=0,dU2=0,dV1=0,dV2=0,dUL=0,dVL=0,UL,VL;
  pl_sInt32 iUL, iVL, idUL=0, idVL=0, iULnext, iVLnext;

  pl_sInt32 scrwidth = cam->ScreenWidth;
  pl_sInt32 X1, X2, dX1=0, dX2=0, XL1, Xlen;
  pl_ZBuffer Z1, dZ1=0, dZ2=0, Z2, dZL=0, ZL, pZL, pdZL;
  pl_sInt32 Y1, Y2, Y0, dY;
  pl_uChar stat;

  pl_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;

  if (TriFace->Material->Environment) Texture = TriFace->Material->Environment;
  else Texture = TriFace->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  iShade = (pl_sInt32)(TriFace->fShade*256.0);
  if (iShade < 0) iShade=0;
  if (iShade > 255) iShade=255;

  if (!TriFace->Material->_AddTable) bc=0;
  else bc = TriFace->Material->_AddTable[iShade];
  nm = TriFace->Material->PerspectiveCorrect;
  nmb = 0; while (nm) { nmb++; nm >>= 1; }
  nmb = plMin(6,nmb);
  nm = 1<<nmb;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
  vshift = 16 - Texture->Width;

  if (TriFace->Material->Environment) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  MappingU1 *= TriFace->Scrz[i0]/65536.0f;
  MappingV1 *= TriFace->Scrz[i0]/65536.0f;
  MappingU2 *= TriFace->Scrz[i1]/65536.0f;
  MappingV2 *= TriFace->Scrz[i1]/65536.0f;
  MappingU3 *= TriFace->Scrz[i2]/65536.0f;
  MappingV3 *= TriFace->Scrz[i2]/65536.0f;

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2-Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      XL1 = dX2; dX2 = dX1; dX1 = XL1;
      dUL = dU1; dU1 = dU2; dU2 = dUL;
      dVL = dV1; dV1 = dV2; dV2 = dVL;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = ((dX1-dX2)*dY+(1<<19))>>20;
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
  } else {
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
      dZL = (Z2-Z1)/XL1;
    }
  }

  pdZL = dZL * nm;
  dUL *= nm;
  dVL *= nm;

  while (Y0 < Y2) {
    if (Y0 == Y1) {
      dY = Y2-((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]-Z1)/dY;
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    Xlen = ((X2+(1<<19))>>20) - XL1;
    if (Xlen > 0) {
      register pl_Float t;
      pZL = ZL = Z1;
      UL = U1;
      VL = V1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += Xlen-scrwidth;
      t = 65536.0f/ZL;
      iUL = iULnext = ((pl_sInt32) (UL*t));
      iVL = iVLnext = ((pl_sInt32) (VL*t));
      do {
        UL += dUL;
        VL += dVL;
        iUL = iULnext;
        iVL = iVLnext;
        pZL += pdZL;
        t = 65536.0f/pZL;
        iULnext = ((pl_sInt32) (UL*t));
        iVLnext = ((pl_sInt32) (VL*t));
        idUL = (iULnext - iUL)>>nmb;
        idVL = (iVLnext - iVL)>>nmb;
        n = nm;
        Xlen -= n;
        if (Xlen < 0) n += Xlen;
        if (zb) do {
            if (*zbuf < ZL) {
              *zbuf = ZL;
              *gmem = remap[bc + texture[((iUL>>16)&MappingU_AND) +
                                   ((iVL>>vshift)&MappingV_AND)]];
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        else do {
            *gmem++ = remap[bc + texture[((iUL>>16)&MappingU_AND) +
                                   ((iVL>>vshift)&MappingV_AND)]];
            iUL += idUL;
            iVL += idVL;
          } while (--n);
      } while (Xlen > 0);
      gmem -= XL1;
      zbuf -= XL1;
    } else {
      zbuf += cam->ScreenWidth;
      gmem += cam->ScreenWidth;
    }
    Z1 += dZ1;
    U1 += dU1;
    V1 += dV1;
    X1 += dX1;
    X2 += dX2;
    Y0++;
  }
}

void plPF_PTexG(pl_Cam *cam, pl_Face *TriFace) {
  pl_uChar i0, i1, i2;
  pl_Float MappingU1, MappingU2, MappingU3;
  pl_Float MappingV1, MappingV2, MappingV3;

  pl_Texture *Texture;
  pl_Bool zb = (cam->zBuffer&&TriFace->Material->zBufferable) ? 1 : 0;

  pl_uChar nm, nmb;
  pl_uInt n;
  pl_sInt32 MappingU_AND, MappingV_AND;
  pl_uChar vshift;
  pl_uChar *texture;
  pl_uInt16 *addtable;
  pl_uChar *remap = TriFace->Material->_ReMapTable;
  pl_sInt32 iUL, iVL, idUL, idVL, iULnext, iVLnext;
  pl_Float U2,V2,dU2=0,dV2=0,dUL=0,dVL=0,UL,VL;
  pl_sInt32 XL1, Xlen;
  pl_sInt32 C2, dC2=0, CL, dCL=0;
  pl_Float ZL, Z2, dZ2=0, dZL=0, pdZL, pZL;

  pl_sInt32 Y2, dY;
  pl_uChar stat;

  /* Cache line */
  pl_sInt32 Y0,Y1;
  pl_sInt32 C1, dC1=0, X2, dX2=0, X1, dX1=0;

  /* Cache line */
  pl_Float dU1=0, U1, dZ1=0, Z1, V1, dV1=0;
  pl_sInt32 scrwidth = cam->ScreenWidth;
  pl_uChar *gmem = cam->frameBuffer;
  pl_ZBuffer *zbuf = cam->zBuffer;

  if (TriFace->Material->Environment) Texture = TriFace->Material->Environment;
  else Texture = TriFace->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  addtable = TriFace->Material->_AddTable;
  if (!addtable) return;

  nm = TriFace->Material->PerspectiveCorrect;
  nmb = 0; while (nm) { nmb++; nm >>= 1; }
  nmb = plMin(6,nmb);
  nm = 1<<nmb;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
  vshift = 16 - Texture->Width;

  if (TriFace->Material->Environment) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  MappingU1 *= TriFace->Scrz[i0]/65536.0f;
  MappingV1 *= TriFace->Scrz[i0]/65536.0f;
  MappingU2 *= TriFace->Scrz[i1]/65536.0f;
  MappingV2 *= TriFace->Scrz[i1]/65536.0f;
  MappingU3 *= TriFace->Scrz[i2]/65536.0f;
  MappingV3 *= TriFace->Scrz[i2]/65536.0f;
  TriFace->Shades[0] *= 65536.0f;
  TriFace->Shades[1] *= 65536.0f;
  TriFace->Shades[2] *= 65536.0f;

  C1 = C2 = (pl_sInt32) TriFace->Shades[i0];
  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2-Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
    dC2 = (pl_sInt32) ((TriFace->Shades[i2] - C1) / dY);
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    dC1 = (pl_sInt32) ((TriFace->Shades[i1] - C1) / dY);
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dUL = dU1; dU1 = dU2; dU2 = dUL;
      dVL = dV1; dV1 = dV2; dV2 = dVL;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      dCL = dC1; dC1 = dC2; dC2 = dCL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      C2 = (pl_sInt32)TriFace->Shades[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      C1 = (pl_sInt32)TriFace->Shades[i1];
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  }

  gmem += (Y0 * scrwidth);
  zbuf += (Y0 * scrwidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
    dCL = ((dC1-dC2)*dY)/XL1;
  } else {
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
      dZL = (Z2-Z1)/XL1;
      dCL = (C2-C1)/XL1;
    }
  }

  pdZL = dZL * nm;
  dUL *= nm;
  dVL *= nm;
  Y1 -= Y0;
  Y0 = Y2-Y0;
  while (Y0--) {
    if (!Y1--) {
      dY = Y2-((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]-Z1)/dY;
        dC1 = (pl_sInt32)((TriFace->Shades[i2]-C1)/dY);
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    Xlen = ((X2+(1<<19))>>20) - XL1;
    if (Xlen > 0) {
      register pl_Float t;
      CL = C1;
      pZL = ZL = Z1;
      UL = U1;
      VL = V1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += Xlen-scrwidth;
      t = 65536.0f / ZL;
      iUL = iULnext = ((pl_sInt32) (UL*t));
      iVL = iVLnext = ((pl_sInt32) (VL*t));
      do {
        UL += dUL;
        VL += dVL;
        iUL = iULnext;
        iVL = iVLnext;
        pZL += pdZL;
        t = 65536.0f/pZL;
        iULnext = ((pl_sInt32) (UL*t));
        iVLnext = ((pl_sInt32) (VL*t));
        idUL = (iULnext - iUL)>>nmb;
        idVL = (iVLnext - iVL)>>nmb;
        n = nm;
        Xlen -= n;
        if (Xlen < 0) n += Xlen;
        if (zb) do {
            if (*zbuf < ZL) {
              int av;
              if (CL < 0) av=addtable[0];
              else if (CL > (255<<8)) av=addtable[255];
              else av=addtable[CL>>8];
              *zbuf = ZL;
              *gmem = remap[av +
                      texture[((iUL>>16)&MappingU_AND) +
                              ((iVL>>vshift)&MappingV_AND)]];
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            CL += dCL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        else do {
            int av;
            if (CL < 0) av=addtable[0];
            else if (CL > (255<<8)) av=addtable[255];
            else av=addtable[CL>>8];
            *gmem++ = remap[av +
                      texture[((iUL>>16)&MappingU_AND) +
                              ((iVL>>vshift)&MappingV_AND)]];
            CL += dCL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
      } while (Xlen > 0);
      gmem -= XL1;
      zbuf -= XL1;
    } else {
      zbuf += scrwidth;
      gmem += scrwidth;
    }
    Z1 += dZ1;
    U1 += dU1;
    V1 += dV1;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
  }
}
