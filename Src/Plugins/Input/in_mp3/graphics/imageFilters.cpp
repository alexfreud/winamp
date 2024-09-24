#include ".\imagefilters.h"

void MLImageFilter_GrayScale(MLImage *image)
{
	DWORD *line = (DWORD*)(image->GetData());
	DWORD *end = line + image->GetHeight() * image->GetWidth();
	for(;line != end; line++)
	{
		BYTE y = (BYTE)(0.3f * GetBValue(*line) +  0.59f *GetGValue(*line)  +  0.11f *GetRValue(*line));
		*line = RGB(y,y,y);
	}
}

void MLImageFilter_Invert(MLImage *image)
{
	DWORD *line = (DWORD*)(image->GetData());
	DWORD *end = line + image->GetHeight() * image->GetWidth();
	for(;line != end; line++) 	*line = ((~*line) & 0x00FFFFFF) |  (*line & 0xFF000000);
}

void MLImageFilter_SetToColor(MLImage *image, COLORREF color)
{
	COLORREF rColor = FIXCOLORREF(color);
	DWORD *line = (DWORD*)(image->GetData());
	DWORD *end = line + image->GetHeight() * image->GetWidth();
	for(;line != end; line++)	*line = rColor;
}

void MLImageFilter_Fader1(MLImage *dest,  const MLImage* source, COLORREF color)
{
	int len = dest->GetHeight() * dest->GetWidth();
	BYTE r = GetRValue(color), g  = GetGValue(color), b  = GetBValue(color);

	DWORD *dataS = (DWORD*)(source->GetData());
	DWORD *dataD = (DWORD*)(dest->GetData());
	DWORD *end = dataD + len;
	for(;dataD != end; dataD++, dataS++)
	{
		*dataD = RGB( max(b, GetRValue(*dataS)), max(g, GetGValue(*dataS)), max(r, GetBValue(*dataS))) ;
	}
}

void MLImageFilter_Fader2(MLImage *dest,  const MLImage* source, COLORREF color)
{
	int len = dest->GetHeight() * dest->GetWidth();
	BYTE r = GetRValue(color), g  = GetGValue(color), b  = GetBValue(color);

	DWORD *dataS = (DWORD*)(source->GetData());
	DWORD *dataD = (DWORD*)(dest->GetData());
	DWORD *end = dataD + len;
	for(;dataD != end; dataD++, dataS++)
	{
		*dataD = RGB( min(b, GetRValue(*dataS)), min(g, GetGValue(*dataS)), min(r, GetBValue(*dataS))) ;
	}
}

void MLImageFilter_Fader3(MLImage *dest,  const MLImage* source, int koeff)
{
	int len = dest->GetHeight() * dest->GetWidth();

	DWORD *dataS = (DWORD*)(source->GetData());
	DWORD *dataD = (DWORD*)(dest->GetData());
	DWORD *end = dataD + len;
	for(;dataD != end; dataD++, dataS++)
	{
		*dataD = RGB(min(255,GetRValue(*dataS)  + koeff), min(255,GetGValue(*dataS) + koeff), min(255, GetBValue(*dataS) + koeff));
	}
}

void MLImageFilter_Blend1(MLImage *dest, MLImage *src1, int destX, int destY, int width, int height, const MLImage* src2, int srcX, int srcY, COLORREF color)
{
	int widthS1 = src1->GetWidth();
	int widthS2 = src2->GetWidth();

	DWORD *dataD = (DWORD*)(dest->GetData()) + destY * widthS1 + destX;
	DWORD *dataS1 = (DWORD*)(src1->GetData()) + destY * widthS1 + destX;
	DWORD *dataS2 = (DWORD*)(src2->GetData()) + srcY * widthS2 + srcX;

	DWORD *curS1;

	for (int y = 0; y < height; y++)
	{
		DWORD *curD = dataD + y * widthS1;
		curS1 = dataS1 + y * widthS1;
		DWORD *curS2 = dataS2 + y * widthS2;
		for (DWORD *end = curS1 + width; end != curS1; curD++, curS1++, curS2++)
		{
			*curD = (*curS1 == color) ? *curS2 : *curS1;
		}
	}
}