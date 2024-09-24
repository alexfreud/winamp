#ifndef __WINBMP_H
#define __WINBMP_H

typedef struct tagWINRGBQUAD
{
  BYTE    rgbBlue;
  BYTE    rgbGreen;
  BYTE    rgbRed;
  BYTE    rgbReserved;
} WINRGBQUAD;

typedef struct tagWINBITMAPFILEHEADER
{
  WORD    bfType;
  LONG    bfSize;
  WORD    bfReserved1;
  WORD    bfReserved2;
  LONG    bfOffBits;
} WINBITMAPFILEHEADER;

typedef struct tagWINBITMAPINFOHEADER
{
  LONG   biSize;
  LONG   biWidth;
  LONG   biHeight;
  WORD   biPlanes;
  WORD   biBitCount;
  LONG   biCompression;
  LONG   biSizeImage;
  LONG   biXPelsPerMeter;
  LONG   biYPelsPerMeter;
  LONG   biClrUsed;
  LONG   biClrImportant;
} WINBITMAPINFOHEADER;

typedef struct tagWINBITMAPINFO
{
  WINBITMAPINFOHEADER bmiHeader;
  WINRGBQUAD          bmiColors[1];
} WINBITMAPINFO;

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

#endif
