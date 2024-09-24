#include <windows.h>

static void __inline DrawPixel(int color, unsigned char *fb)
{
  int a=*fb;
  a+=color;
  if (a>255)a=255;
  *fb=(unsigned char)a;
}

// based on abrash's wu antialiasing routine
void line(unsigned char *fb, int X0, int Y0, int X1, int Y1, int w, int h)
{
	unsigned int ErrorAdj=0, ErrorAcc;
	int DeltaX, DeltaY, XDir=1;

	if (Y0 > Y1) 
	{
		int t = Y0; Y0 = Y1; Y1 = t;
		t = X0; X0 = X1; X1 = t;
	}

	DeltaY = Y1 - Y0;
	DeltaX = X1 - X0;
	if (DeltaX < 0) 
	{
		XDir = -1;
		DeltaX = -DeltaX;
	}

	if (DeltaY > DeltaX) {

		ErrorAcc = (X0&0xffff);
		Y0>>=16;
		X0>>=16;
		DeltaY += 65535;   
		DeltaY >>= 16;
		if (DeltaY) ErrorAdj = DeltaX/DeltaY;
		fb+=Y0*w+X0;
		while (DeltaY-- >= 0) {
			if (X0 > 0 && Y0 >= 0 && X0 < w-1 && Y0 < h)
			{
				int Weighting = ErrorAcc >> 10;
				DrawPixel(Weighting^63,fb);
				DrawPixel(Weighting,fb+XDir);
			}
	        
			ErrorAcc += ErrorAdj;
			if (ErrorAcc >= 65536) {
				X0 += XDir;
				fb += XDir;
				ErrorAcc -= 65536;
			}
			fb+=w;
			Y0++;
		}
	}
	else
	{
		ErrorAcc = (Y0&0xffff);
		X0>>=16;
		Y0>>=16;
		DeltaX += 65535;   
		DeltaX >>= 16;
		if (DeltaX) ErrorAdj = DeltaY/DeltaX;

		fb+=Y0*w+X0;
		while (DeltaX-- >= 0) {
			if (Y0 >= 0 && Y0 < h-1 && X0 >= 0 && X0 < w)
			{
				int Weighting = ErrorAcc >> 10;
				DrawPixel(Weighting^63,fb);
				DrawPixel(Weighting,fb+w);
			}

			ErrorAcc += ErrorAdj;
			if (ErrorAcc >= 65536) {
				Y0++;
				fb+=w;
				ErrorAcc-=65536;
			}
			fb+=XDir;
			X0+=XDir;
		}
	}
}