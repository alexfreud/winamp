/******************************************************************************
Plush Version 1.1
text.c
Text code and data (8xX bitmapped)
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"
#include <stdarg.h>

static pl_uChar font_height = 16;

static pl_uChar *current_font = plText_DefaultFont;

void plTextSetFont(pl_uChar *font, pl_uChar height) {
  current_font = font;
  font_height = height;
}

void plTextPutChar(pl_Cam *cam, pl_sInt x, pl_sInt y, pl_Float z,
                   pl_uChar color, pl_uChar c) {
  pl_uChar *font = current_font + (c*font_height);
  pl_sInt offset = x+(y*cam->ScreenWidth);
  pl_ZBuffer zz = (pl_ZBuffer) (1.0/z);
  pl_sInt xx = x, a;
  pl_uChar len = font_height;
  pl_uChar ch;
  pl_uChar *outmem;
  pl_ZBuffer *zbuffer;
  if (y+font_height < cam->ClipTop || y >= cam->ClipBottom) return;
  if (y < cam->ClipTop) {
    font += (cam->ClipTop-y);
    offset += (cam->ClipTop-y)*cam->ScreenWidth;
    len -= (cam->ClipTop-y);
    y = cam->ClipTop;
  }
  if (y+font_height >= cam->ClipBottom) {
    len = cam->ClipBottom-y;
  }
  if (len > 0) {
    if (cam->zBuffer && z != 0.0) do {
      outmem = cam->frameBuffer + offset;
      zbuffer = cam->zBuffer + offset;
      offset += cam->ScreenWidth;
      xx = x;
      ch = *font++;
      a = 128;
      while (a) {
        if (xx >= cam->ClipRight) break;
        if (xx++ >= cam->ClipLeft)
          if (ch & a)
            if (zz > *zbuffer) {
              *zbuffer = zz;
              *outmem = color;
            }
        zbuffer++;
        outmem++;
        a >>= 1;
      }
      if (a) break;
    } while (--len);
    else do {
      outmem = cam->frameBuffer + offset;
      offset += cam->ScreenWidth;
      xx = x;
      ch = *font++;
      a = 128;
      while (a) {
        if (xx >= cam->ClipRight) break;
        if (xx++ >= cam->ClipLeft) if (ch & a) *outmem = color;
        outmem++;
        a >>= 1;
      }
      if (a) break;
    } while (--len);
  }
}

void plTextPutStr(pl_Cam *cam, pl_sInt x, pl_sInt y, pl_Float z,
                  pl_uChar color, pl_sChar *string) {
  pl_sInt xx = x;
  while (*string) {
    switch (*string) {
      case '\n': y += font_height; xx = x; break;
      case ' ': xx += 8; break;
      case '\r': break;
      case '\t': xx += 8*5; break;
      default:
        plTextPutChar(cam,xx,y,z,color,(pl_uChar) *string);
        xx += 8;
      break;
    }
    string++;
  }
}

void plTextPutStrW(pl_Cam* cam, pl_sInt x, pl_sInt y, pl_Float z,
    pl_uChar color, const wchar_t* string) {
    pl_sInt xx = x;
    while (*string) {
        switch (*string) {
        case L'\n': y += font_height; xx = x; break;
        case L' ': xx += 8; break;
        case L'\r': break;
        case L'\t': xx += 8 * 5; break;
        default:
            plTextPutChar(cam, xx, y, z, color, (pl_uChar)*string);
            xx += 8;
            break;
        }
        string++;
    }
}

void plTextPrintf(pl_Cam *cam, pl_sInt x, pl_sInt y, pl_Float z,
                  pl_uChar color, pl_sChar *format, ...) {
  va_list arglist;
  pl_sChar str[256];
  va_start(arglist, format);
  vsprintf((char *)str, (char *) format,arglist);
  va_end(arglist);
  plTextPutStr(cam,x,y,z,color,str);
}
