/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// Note! 
// *Video points to memory in 32bit video memory, ie, the following can be used
// for your utility and to help your understanding of the pixel format used.

// In Visual C, you should start a DLL project, under code generation set it to
// use the DLL libraries, and most likely for release you will want to turn
// optimisation for speed on. Include this .h file in your project, then create
// or add your main C/C++ file and #include "vis.h"

// To start off, fill in the VisInfo structure. Render should draw a whole frame
// into the 32 bit ABGR buffer (for blur, smoke and zoom to work it should, rather
// than replacing the video data, add to it using bitwise or, saturated add, or
// alpha blend). Make sure not to exceed the boundaries given. 'Pitch' specifies
// the distance, in pixels, between the start of each line. If you have a pointer
// at the start of a line, Pointer+Pitch is the start of the next line and
// Pointer-Pitch is the start of the previous.
// Make sure that QueryModule is defined exactly as at the end of this file,
// including the extern "C" keyword if you're using C++. It should return a
// pointer to this structure. All routines that are pointed to by the structure
// must be filled in even if they don't do anything. Render is the only function
// that is required to have a body.



#define ALPHA_MASK	0xFF000000
#define RED_MASK	0x00FF0000
#define GREEN_MASK	0x0000FF00
#define BLUE_MASK	0x000000FF

#define ALPHA_SHIFT	24
#define RED_SHIFT	16
#define GREEN_SHIFT 8
#define BLUE_SHIFT	0

#define AlphaByte(x)	(((x))>>ALPHA_SHIFT)
#define RedByte(x)		(((x) & RED_MASK)>>RED_SHIFT)
#define GreenByte(x)	(((x) & GREEN_MASK)>>GREEN_SHIFT)
#define BlueByte(x)		(((x) & BLUE_MASK)>>BLUE_SHIFT)



// files should be renamed from .DLL to .SVP

#ifndef DLLEXPORT
#define DLLEXPORT    __declspec( dllexport )
#endif


#define VI_WAVEFORM			0x0001		// set if you need the waveform
#define VI_SPECTRUM			0x0002		// set if you need the FFT values 
#define SONIQUEVISPROC		0x0004		// set if you want to allow Soniques user pref vis to affect your vis
										//   for example - blur, smoke and zoom

#pragma pack (push, 8)

typedef struct 
{
	unsigned long	MillSec;			// Sonique sets this to the time stamp of end this block of data
	unsigned char	Waveform[2][512];	// Sonique sets this to the PCM data being outputted at this time
	unsigned char	Spectrum[2][256];	// Sonique sets this to a lowfidely version of the spectrum data
										//   being outputted at this time
} VisData;

typedef struct _VisInfo
{
	unsigned long Reserved;				// Reserved

	char	*PluginName;				// Set to the name of the plugin
	long	lRequired;					// Which vis data this plugin requires (set to a combination of
										//   the VI_WAVEFORM, VI_SPECTRUM and SONIQEUVISPROC flags)

	void	(*Initialize)(void);		// Called some time before your plugin is asked to render for
										// the first time
	BOOL	(*Render)( unsigned long *Video, int width, int height, int pitch, VisData* pVD);
										// Called for each frame. Pitch is in pixels and can be negative.
										// Render like this:
										// for (y = 0; y < height; y++)
										// {
										//    for (x = 0; x < width; x++)
										//       Video[x] = <pixel value>;
										//	  Video += pitch;
										// }
										//				OR
										// void PutPixel(int x, int y, unsigned long Pixel)
										// {
										//    _ASSERT( x >= 0 && x < width && y >= 0 && y < height );
										//	  Video[y*pitch+x] = Pixel;
										// }
	BOOL	(*SaveSettings)( char* FileName );
										// Use WritePrivateProfileString to save settings when this is called
										// Example:
										// WritePrivateProfileString("my plugin", "brightness", "3", FileName);
	BOOL	(*OpenSettings)( char* FileName );
										// Use GetPrivateProfileString similarly:
										// char BrightnessBuffer[256];
										// GetPrivateProfileString("my plugin", "brightness", "3", BrightnessBuffer, sizeof(BrightnessBuffer), FileName);
} VisInfo;

#pragma pack (pop, 8)


// DLL exports this function - it should return a pointer to a static structure
// as above.
extern "C"
DLLEXPORT VisInfo* QueryModule(void);

