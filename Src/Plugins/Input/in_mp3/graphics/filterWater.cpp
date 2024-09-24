#include ".\filterwater.h"
#include <math.h>

#define random( min, max ) (( rand() % (int)((( max ) + 1 ) - ( min ))) + ( min ))

MLImageFilterWater::MLImageFilterWater(void)
{
	hField1 = NULL;
	hField2 = NULL;
	hHandle = NULL;

	width = 0;
	height = 0;

	drawWithLight = TRUE;
	lightModifier = 1;
	hPage = 0;
	density = 5;

}

MLImageFilterWater::~MLImageFilterWater(void)
{
	ClearData();
}

void MLImageFilterWater::ClearData(void)
{
	if (hHandle)
	{
		if (hField1) HeapFree(hHandle, NULL, hField1);
		if (hField2) HeapFree(hHandle, NULL, hField2);
		HeapDestroy(hHandle);
		hField1 = NULL;
		hField2 = NULL;
		hHandle = NULL;
	}
	
}

BOOL MLImageFilterWater::CreateFor(const MLImage *image)
{
	ClearData();
	width = image->GetWidth();
	height = image->GetHeight();
	hPage = 0;
	int len = height * width * sizeof(int);
	hHandle = HeapCreate(NULL, 3*len, 3*len);
	if (!hHandle) 
	{
		width = 0;
		height = 0;
		return FALSE;
	}

	hField1 = (int*)HeapAlloc(hHandle, HEAP_ZERO_MEMORY, len);
	hField2 = (int*)HeapAlloc(hHandle, HEAP_ZERO_MEMORY, len);
	return hField1 && hField2;
}

void MLImageFilterWater::Render(MLImage* destination, const MLImage* source)
{
	if(!drawWithLight) DrawWaterNoLight(hPage, destination, source);
	else DrawWaterWithLight(destination, source);
	CalculateWater(hPage, density);
//	CalcWaterBigFilter(hPage, density);
	hPage ^= 1;
}

void MLImageFilterWater::CalculateWater(int page, int density)
{
	int newh;
	int count = width + 1;
	int *newptr;
	int *oldptr;

	if(page == 0)
	{
		newptr = hField1;
		oldptr = hField2;
	}
	else
	{
		newptr = hField2;
		oldptr = hField1;
	}

	int x, y;
	for (y = (height - 1) * width; count < y; count += 2)
	{
		for (x = count + width - 2; count < x; count++)
		{
			// This does the eight-pixel method.

			newh		= ((oldptr[count + width]
					+ oldptr[count - width]
					+ oldptr[count + 1]
					+ oldptr[count - 1]
					+ oldptr[count - width - 1]
					+ oldptr[count - width + 1]
					+ oldptr[count + width - 1]
					+ oldptr[count + width + 1]
					) >> 2 )
					- newptr[count];
			newptr[count] =  newh - (newh >> density);
		/*
		// This is the "sludge" method...
			newh = (oldptr[count]<<2)
				+  oldptr[count-1-m_iWidth]
				+  oldptr[count+1-m_iWidth]
				+  oldptr[count-1+m_iWidth]
				+  oldptr[count+1+m_iWidth]
				+ ((oldptr[count-1]
				+   oldptr[count+1]
				+   oldptr[count-m_iWidth]
				+   oldptr[count+m_iWidth])<<1);

			newptr[count] = (newh-(newh>>6)) >> density;
		*/
		}
	}
}

void MLImageFilterWater::SmoothWater(int page)
{
	int newh;
	int count = width + 1;

	int *newptr;
	int *oldptr;

	if(page == 0)
	{
		newptr = hField1;
		oldptr = hField2;
	}
	else
	{
		newptr = hField2;
		oldptr = hField1;
	}

	int x, y;

	for(y = 1; y < height; y++, count += 2)
	{
		for( x = 1; x < width; x++, count++)
		{
			// This does the eight-pixel method.  

			newh = ((oldptr[count + width]
					+ oldptr[count - width]
					+ oldptr[count + 1]
					+ oldptr[count - 1]
					+ oldptr[count - width - 1]
					+ oldptr[count - width + 1]
					+ oldptr[count + width - 1]
					+ oldptr[count + width + 1]
					) >> 3 )
					+ newptr[count];
			newptr[count] =  newh>>1;
		}
    }
}
void MLImageFilterWater::FlattenWater(void)
{
	int len = width * height * sizeof(int);
	SecureZeroMemory(hField1, len);
	SecureZeroMemory(hField2, len);
}
void MLImageFilterWater::SineBlob(int x, int y, int radius, int height, int page)
{
	int cx, cy;
	int left,top,right,bottom;
	double square, dist;
	double radsquare = radius * radius;
	double length = double((1024.0/(double)radius)*(1024.0/(double)radius));
	int *newptr;

	if(page == 0)
	{
		newptr = hField1;
	}
	else
	{
		newptr = hField2;
	}

	int t = (this->width - 2*radius - 1);
	if (t == 0) t = 1;
	if(x<0) x = 1 + radius + rand() % t;
	t = (this->height - 2*radius - 1); 
	if (t == 0) t = 1;
	if(y<0) y = 1 + radius + rand() % t;

	radsquare = (radius*radius);

	left = -radius; right = radius;
	top = -radius; bottom = radius;

	// Perform edge clipping...
	if(x - radius < 1) left -= (x-radius-1);
	if(y - radius < 1) top  -= (y-radius-1);
	if(x + radius > this->width - 1) right -= (x + radius - this->width + 1);
	if(y + radius > this->height - 1) bottom -= (y + radius - this->height + 1);

	for(cy = top; cy < bottom; cy++)
	{
		for(cx = left; cx < right; cx++)
		{
			square = cy*cy + cx*cx;
			if(square < radsquare)
			{
				dist = sqrt(square*length);
				newptr[this->width*(cy+y) + cx+x] += (int)((cos(dist)+0xffff)*(height)) >> 19;
			}
		}
	}
}

void MLImageFilterWater::WarpBlob(int x, int y, int radius, int height, int page)
{
	int cx, cy;
	int left,top,right,bottom;
	int square;
	int radsquare = radius * radius;
	int *newptr;

	if(page == 0)
	{
		newptr = hField1;
	}
	else
	{
		newptr = hField2;
	}

	radsquare = (radius*radius);

	height /= 64;

	left=-radius; right = radius;
	top=-radius; bottom = radius;

	// Perform edge clipping...
	if(x - radius < 1) left -= (x-radius-1);
	if(y - radius < 1) top  -= (y-radius-1);
	if(x + radius > this->width-1) right -= (x+ radius - this->width + 1);
	if(y + radius > this->height-1) bottom-= (y + radius - this->height + 1);

	for(cy = top; cy < bottom; cy++)
	{
		for(cx = left; cx < right; cx++)
		{
			square = cy*cy + cx*cx;
			if(square < radsquare)
			{
				newptr[this->width*(cy+y) + cx+x] += int((radius-sqrt((float)square))*(float)(height));
			}
		}
	}
}
void MLImageFilterWater::HeightBox (int x, int y, int radius, int height, int page)
{
	int cx, cy;
	int left,top,right,bottom;
	int *newptr;

	if(page == 0)
	{
		newptr = hField1;
	}
	else
	{
		newptr = hField2;
	}

	int t = (this->width - 2*radius - 1);
	if (t == 0) t = 1;
	if(x<0) x = 1 + radius + rand() % t;
	t = (this->height - 2*radius - 1); 
	if (t == 0) t = 1;
	if(y<0) y = 1 + radius + rand() % t;

	left=-radius; right = radius;
	top=-radius; bottom = radius;

	// Perform edge clipping...
	if(x - radius < 1) left -= (x-radius-1);
	if(y - radius < 1) top  -= (y-radius-1);
	if(x + radius > this->width-1) right -= (x+ radius - this->width + 1);
	if(y + radius > this->height-1) bottom-= (y + radius - this->height + 1);

	for(cy = top; cy < bottom; cy++)
	{
		for(cx = left; cx < right; cx++)
		{
			newptr[this->width*(cy+y) + cx+x] = height;
		}
	}
}
void MLImageFilterWater::HeightBlob(int x, int y, int radius, int height, int page)
{
	int rquad;
	int cx, cy;
	int left, top, right, bottom;

	rquad = radius * radius;

	// Make a randomly-placed blob...
	int t = (this->width - 2*radius - 1);
	if (t == 0) t = 1;
	if(x<0) x = 1 + radius + rand() % t;
	t = (this->height - 2*radius - 1); 
	if (t == 0) t = 1;
	if(y<0) y = 1 + radius + rand() % t;

	left = -radius; right = radius;
	top = -radius; bottom = radius;

	// Perform edge clipping...
	if(x - radius < 1) left -= (x-radius-1);
	if(y - radius < 1) top  -= (y-radius-1);
	if(x + radius > this->width-1) right -= (x+ radius - this->width + 1);
	if(y + radius > this->height-1) bottom-= (y + radius - this->height + 1);

	for(cy = top; cy < bottom; cy++)
	{
		int cyq = cy*cy;
		for(cx = left; cx < right; cx++)
		{
			if(cx*cx + cyq < rquad) newptr[this->width * (cy+y) + (cx+x)] += height;
		}
	}
}

void MLImageFilterWater::CalcWaterBigFilter(int page, int density)
{
	int newh;
	int count = (2 * width) + 2;

	int *newptr;
	int *oldptr;

	// Set up the pointers
  	if(page == 0)
	{
		newptr = hField1;
		oldptr = hField2;
	}
	else
	{
		newptr = hField2;
		oldptr = hField1;
	}

	int x, y;

	for(y=2; y < height-2; y++, count += 4)
	{
		for(x=2; x < width-2; x++, count++)
		{
			// This does the 25-pixel method.  It looks much okay.

			newh = (
					(
                    (
                    (oldptr[count + width] 
                     + oldptr[count - width]
                     + oldptr[count + 1]
                     + oldptr[count - 1]
                     )<<1)
                     + ((oldptr[count - width - 1]
                     + oldptr[count - width + 1]
                     + oldptr[count + width - 1]
                     + oldptr[count + width + 1]))
                     + ( (
                          oldptr[count - (width*2)]
                        + oldptr[count + (width*2)]
                        + oldptr[count - 2]
                        + oldptr[count + 2]
                        ) >> 1 )
                      + ( (
                          oldptr[count - (width*2) - 1]
                        + oldptr[count - (width*2) + 1]
                        + oldptr[count + (width*2) - 1]
                        + oldptr[count + (width*2) + 1]
                        + oldptr[count - 2 - width]
                        + oldptr[count - 2 + width]
                        + oldptr[count + 2 - width]
                        + oldptr[count + 2 + width]
                        ) >> 2 )
                     )
                    >> 3)
                    - (newptr[count]);


			newptr[count] =  newh - (newh >> density);
		}
	}
}

void MLImageFilterWater::DrawWaterNoLight(int page, MLImage* destination, const MLImage* source)
{
	unsigned int brk = width * height;

	int *ptr = hField1;

	DWORD *dataS = (DWORD*)source->GetData();
	DWORD *dataD = (DWORD*)destination->GetData();

	for (unsigned int offset = 0; offset < brk; offset++)
	{
		int dx = ptr[offset] - ptr[offset + 1];
		int dy = ptr[offset] - ptr[offset + width];
		unsigned int index = offset + width * (dy>>3) + (dx>>3);
		dataD[offset] = (index < brk ) ? dataS[offset + width*(dy>>3) + (dx>>3)] : dataS[offset];
	}
}

void MLImageFilterWater::DrawWaterWithLight(MLImage* destination, const MLImage* source)
{
	unsigned int brk = width * height;

	int *ptr = hField1;

	DWORD *dataS = (DWORD*)source->GetData();
	DWORD *dataD = (DWORD*)destination->GetData();

	for (unsigned int offset = 0; offset < brk; offset++)
	{
		int dx = ptr[offset] - ptr[offset + 1];
		int dy = ptr[offset] - ptr[offset + width];
		unsigned int index = offset + width * (dy>>3) + (dx>>3);
		dataD[offset] = (index < brk ) ?  GetShiftedColor(dataS[index], dx) : dataS[offset];
	}
}
COLORREF MLImageFilterWater::GetShiftedColor(COLORREF color,int shift)
{
	int R,G, B;
	int r, g, b;

	R = GetRValue(color)-shift;
	G = GetGValue(color)-shift;
	B = GetBValue(color)-shift;

	r = (R < 0) ? 0 : (R > 255) ? 255 : R;
	g = (G < 0) ? 0 : (G > 255) ? 255 : G;
	b = (B < 0) ? 0 : (B > 255) ? 255 : B;

	return RGB(r,g,b);
}
