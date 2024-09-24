#ifndef NULLSOFT_ML_IMAGE_FILTER_HEADER
#define NULLSOFT_ML_IMAGE_FILTER_HEADER

#include <windows.h>
#include ".\image.h"
#include ".\filterwater.h"

void MLImageFilter_GrayScale(MLImage *image);
void MLImageFilter_Invert(MLImage *image);
void MLImageFilter_SetToColor(MLImage *image, COLORREF color);
void MLImageFilter_Fader1(MLImage *dest,  const MLImage* source, COLORREF color);
void MLImageFilter_Fader2(MLImage *dest,  const MLImage* source, COLORREF color);
void MLImageFilter_Fader3(MLImage *dest,  const MLImage* source, int koeff);
void MLImageFilter_Blend1(MLImage *dest, MLImage *src1, int destX, int destY, int width, int height, const MLImage* src2, int srcX, int srcY, COLORREF color);


#endif //NULLSOFT_ML_IMAGE_FILTER_HEADER
