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
// notes:
// any window that remains in foreground should optimally pass 
// keystrokes to the parent (winamp's) window, so that the user
// can still control it. unless escape is hit, or some option 
// key specific to the vis is hit. As for storing configuration,
// Configuration data should be stored in <dll directory>\plugin.ini
// Look at the example plugin for a framework.

// ints are 32 bits, and structure members are aligned on the default 8 byte boundaries
// tested with VC++ 4.2 and 5.0


typedef struct winampVisModule {
  char *description; // description of module
  HWND hwndParent;   // parent window (filled in by calling app)
  HINSTANCE hDllInstance; // instance handle to this DLL (filled in by calling app)
  int sRate;		 // sample rate (filled in by calling app)
  int nCh;			 // number of channels (filled in...)
  int latencyMs;     // latency from call of RenderFrame to actual drawing
                     // (calling app looks at this value when getting data)
  int delayMs;       // delay between calls in ms

  // the data is filled in according to the respective Nch entry
  int spectrumNch;
  int waveformNch;
  unsigned char spectrumData[2][576];
  unsigned char waveformData[2][576];

  void (*Config)(struct winampVisModule *this_mod);  // configuration dialog
  int (*Init)(struct winampVisModule *this_mod);     // 0 on success, creates window, etc
  int (*Render)(struct winampVisModule *this_mod);   // returns 0 if successful, 1 if vis should end
  void (*Quit)(struct winampVisModule *this_mod);    // call when done

  void *userData; // user data, optional
} winampVisModule;

typedef struct {
  int version;       // VID_HDRVER
  char *description; // description of library
  winampVisModule* (*getModule)(int);
} winampVisHeader;

// exported symbols
typedef winampVisHeader* (*winampVisGetHeaderType)();

// version of current module (0x101 == 1.01)
#define VIS_HDRVER 0x101