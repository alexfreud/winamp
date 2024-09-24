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
// video delay
// copyright tom holden, 2002
// mail: cfp@myrealbox.com

#include <windows.h>
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"

#define MOD_NAME "Trans / Multi Delay"
#define C_DELAY C_MultiDelayClass

// R_MultiDelay / #define UNIQUEIDSTRING "Holden05: Multi Delay"

// saved
bool usebeats[6];
int delay[6];

// unsaved
LPVOID buffer[6];
LPVOID inpos[6];
LPVOID outpos[6];
unsigned long buffersize[6];
unsigned long virtualbuffersize[6];
unsigned long oldvirtualbuffersize[6];
unsigned long framedelay[6];
unsigned int numinstances = 0;
unsigned long framessincebeat;
unsigned long framesperbeat;
unsigned long framemem;
unsigned long oldframemem;
unsigned int renderid;

class C_DELAY : public C_RBASE 
{
	protected:
	public:
		// standard ape members
		C_DELAY();
		virtual ~C_DELAY();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual char *get_desc();
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		// saved members
		int mode;
		int activebuffer;

		// unsaved members
		unsigned int creationid;
};

// global configuration dialog pointer 
static C_DELAY *g_Delay; 
// global DLL instance pointer
static HINSTANCE g_hDllInstance; 

// configuration screen
static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	char value[16];
	unsigned int objectcode, objectmessage;
	HWND hwndEdit;
	switch (uMsg)
	{
		case WM_INITDIALOG: //init
      {
        int i;
			  for (i=0;i<3;i++) CheckDlgButton(hwndDlg,1100+i,g_Delay->mode==i);
			  for (i=0;i<6;i++)
			  {
				  CheckDlgButton(hwndDlg,1000+i,g_Delay->activebuffer==i);
				  CheckDlgButton(hwndDlg,1020+i,usebeats[i]);
				  CheckDlgButton(hwndDlg,1030+i,!usebeats[i]);
				  hwndEdit = GetDlgItem(hwndDlg,1010+i);
				  _itoa(delay[i],value,10);
				  SetWindowText(hwndEdit,value);
			  }
      }
			return 1;
		case WM_COMMAND:
			objectcode = LOWORD(wParam);
			objectmessage = HIWORD(wParam);
			// mode stuff
			if (objectcode >= 1100)
			{
				if(IsDlgButtonChecked(hwndDlg,objectcode)==1)
				{
					g_Delay->mode = objectcode - 1100;
					for (int i=1100;i<1103;i++)	if (objectcode != i) CheckDlgButton(hwndDlg,i,BST_UNCHECKED);
				}
			    return 0;
			}
			// frames stuff
			if (objectcode >= 1030)
			{
				if(IsDlgButtonChecked(hwndDlg,objectcode)==1)
				{
					usebeats[objectcode-1030] = false;
					CheckDlgButton(hwndDlg,objectcode-10,BST_UNCHECKED);
					framedelay[objectcode-1030] = delay[objectcode-1030]+1;
				}
				else usebeats[objectcode-1030] = true;
			    return 0;
			}
			// beats stuff
			if (objectcode >= 1020)
			{
				if(IsDlgButtonChecked(hwndDlg,objectcode)==1)
				{
					usebeats[objectcode-1020] = true;
					CheckDlgButton(hwndDlg,objectcode+10,BST_UNCHECKED);
					framedelay[objectcode-1020] = framesperbeat+1;
				}
				else usebeats[objectcode-1020] = false;
			    return 0;
			}
			// edit box stuff
			if (objectcode >= 1010)
			{
				hwndEdit = GetDlgItem(hwndDlg,objectcode);
				if (objectmessage == EN_CHANGE)
				{
					GetWindowText(hwndEdit,value,16);
					delay[objectcode-1010] = max(atoi(value),0);
					framedelay[objectcode-1010] = (usebeats[objectcode-1010]?framesperbeat:delay[objectcode-1010])+1;
				}
				else if (objectmessage == EN_KILLFOCUS)
				{
					_itoa(delay[objectcode-1010],value,10);
					SetWindowText(hwndEdit,value);
				}
				return 0;
			}
			// active buffer stuff
			if (objectcode >= 1000)
			{
				if(IsDlgButtonChecked(hwndDlg,objectcode)==1)
				{
					g_Delay->activebuffer = objectcode - 1000;
					for (int i=1000;i<1006;i++)	if (objectcode != i) CheckDlgButton(hwndDlg,i,BST_UNCHECKED);
				}
			    return 0;
			}
	}
	return 0;
}

// set up default configuration 
C_DELAY::C_DELAY() 
{	
	// enable
	mode = 0;
	activebuffer = 0;
	numinstances++;
	creationid = numinstances;
	if (creationid == 1)
	{
		for (int i=0; i<6; i++)
		{
			renderid = 0;
			framessincebeat = 0;
			framesperbeat = 0;
			framemem = 1;
			oldframemem = 1;
			usebeats[i] = false;
			delay[i] = 0;
			framedelay[i] = 0;
			buffersize[i] = 1;
			virtualbuffersize[i] = 1;
			oldvirtualbuffersize[i] = 1;
			buffer[i] = VirtualAlloc(NULL,buffersize[i],MEM_COMMIT,PAGE_READWRITE);
			inpos[i] = buffer[i];
			outpos[i] = buffer[i];
		}
	}
}

// virtual destructor
C_DELAY::~C_DELAY() 
{
	numinstances--;
	if (numinstances == 0) for (int i=0; i<6; i++) VirtualFree(buffer[i],buffersize[i],MEM_DECOMMIT);
}

// RENDER FUNCTION:
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout
// w and h are the-width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].

int C_DELAY::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (isBeat&0x80000000) return 0;

	if (renderid == numinstances) renderid = 0;
	renderid++;
	if (renderid == 1)
	{
		framemem = w*h*4;
		if (isBeat)
		{
			framesperbeat = framessincebeat;
			for (int i=0;i<6;i++) if (usebeats[i]) framedelay[i] = framesperbeat+1;
			framessincebeat = 0;
		}
		framessincebeat++;
		for (int i=0;i<6;i++)
		{
			if (framedelay[i]>1)
			{
				virtualbuffersize[i] = framedelay[i]*framemem;
				if (framemem == oldframemem)
				{
					if (virtualbuffersize[i] != oldvirtualbuffersize[i])
					{
						if (virtualbuffersize[i] > oldvirtualbuffersize[i])
						{
							if (virtualbuffersize[i] > buffersize[i])
							{
								// allocate new memory
								if (!VirtualFree(buffer[i],buffersize[i],MEM_DECOMMIT)) return 0;
								if (usebeats[i])
								{
									buffersize[i] = 2*virtualbuffersize[i];
									buffer[i] = VirtualAlloc(NULL,buffersize[i],MEM_COMMIT,PAGE_READWRITE);
									if (buffer[i] == NULL)
									{
										buffersize[i] = virtualbuffersize[i];
										buffer[i] = VirtualAlloc(NULL,buffersize[i],MEM_COMMIT,PAGE_READWRITE);
									}
								}
								else
								{
									buffersize[i] = virtualbuffersize[i];
									buffer[i] = VirtualAlloc(NULL,buffersize[i],MEM_COMMIT,PAGE_READWRITE);
								}
								outpos[i] = buffer[i];
								inpos[i] = (LPVOID)(((unsigned long)buffer[i])+virtualbuffersize[i]-framemem);
								if (buffer[i] == NULL)
								{
									framedelay[i] = 0;
									if (usebeats[i])
									{
										framesperbeat = 0;
										framessincebeat = 0;
										framedelay[i] = 0;
										delay[i] = 0;
									}
								}
							}
							else
							{
								unsigned long size = (((unsigned long)buffer[i])+oldvirtualbuffersize[i]) - ((unsigned long)outpos[i]);
								unsigned long l = ((unsigned long)buffer[i])+virtualbuffersize[i];
								unsigned long d =  l - size;
								MoveMemory((LPVOID)d, outpos[i], size);
								for (l = (unsigned long)outpos[i]; l < d; l += framemem) CopyMemory((LPVOID)l,(LPVOID)d,framemem);
							}
						}
						else
						{	// virtualbuffersize < oldvirtualbuffersize
							unsigned long presegsize = ((unsigned long)outpos[i])-((unsigned long)buffer[i]);
							if (presegsize > virtualbuffersize[i])
							{
								MoveMemory(buffer[i],(LPVOID)(((unsigned long)buffer[i])+presegsize-virtualbuffersize[i]),virtualbuffersize[i]);
								inpos[i] = (LPVOID)(((unsigned long)buffer[i])+virtualbuffersize[i]-framemem);
								outpos[i] = buffer[i];
							}
							else if (presegsize < virtualbuffersize[i]) MoveMemory(outpos[i],(LPVOID)(((unsigned long)buffer[i])+oldvirtualbuffersize[i]+presegsize-virtualbuffersize[i]),virtualbuffersize[i]-presegsize);
						}
						oldvirtualbuffersize[i] = virtualbuffersize[i];
					}
				}
				else
				{
					// allocate new memory
					if (!VirtualFree(buffer[i],buffersize[i],MEM_DECOMMIT)) return 0;
					if (usebeats[i])
					{
						buffersize[i] = 2*virtualbuffersize[i];
						buffer[i] = VirtualAlloc(NULL,buffersize[i],MEM_COMMIT,PAGE_READWRITE);
						if (buffer[i] == NULL)
						{
							buffersize[i] = virtualbuffersize[i];
							buffer[i] = VirtualAlloc(NULL,buffersize[i],MEM_COMMIT,PAGE_READWRITE);
						}
					}
					else
					{
						buffersize[i] = virtualbuffersize[i];
						buffer[i] = VirtualAlloc(NULL,buffersize[i],MEM_COMMIT,PAGE_READWRITE);
					}
					outpos[i] = buffer[i];
					inpos[i] = (LPVOID)(((unsigned long)buffer[i])+virtualbuffersize[i]-framemem);
					if (buffer[i] == NULL)
					{
						framedelay[i] = 0;
						if (usebeats[i])
						{
							framesperbeat = 0;
							framessincebeat = 0;
							framedelay[i] = 0;
							delay[i] = 0;
						}
					}
					oldvirtualbuffersize[i] = virtualbuffersize[i];
				}
				oldframemem = framemem;
			}
		}
	}
	if (mode != 0 && framedelay[activebuffer]>1)
	{
		if (mode == 2) CopyMemory(framebuffer,outpos[activebuffer],framemem);
		else CopyMemory(inpos[activebuffer],framebuffer,framemem);
	}
	if (renderid == numinstances) for (int i=0;i<6;i++)
	{
		inpos[i] = (LPVOID)(((unsigned long)inpos[i])+framemem);
		outpos[i] = (LPVOID)(((unsigned long)outpos[i])+framemem);
		if ((unsigned long)inpos[i]>=((unsigned long)buffer[i])+virtualbuffersize[i]) inpos[i] = buffer[i];
		if ((unsigned long)outpos[i]>=((unsigned long)buffer[i])+virtualbuffersize[i]) outpos[i] = buffer[i];
	}
	return 0;
}

HWND C_DELAY::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_Delay = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_MULTIDELAY,hwndParent,g_DlgProc);
}

char *C_DELAY::get_desc(void)
{ 
	static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_MULTI_DELAY,desc,128):desc); 
}

// load_/save_config are called when saving and loading presets (.avs files)
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_DELAY::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	// always ensure there is data to be loaded
	if (len-pos >= 4) 
	{
		// load mode
		mode=GET_INT();
		pos+=4; 
	}
	if (len-pos >= 4) 
	{
		// load active buffer
		activebuffer=GET_INT();
		pos+=4; 
	}
	if (len-pos >= 4) 
	{
		for (int i=0;i<6;i++)
		{
			if (len-pos >= 4) 
			{
				// load usebeats
				usebeats[i]=(GET_INT()==1);
				pos+=4;
			}
			if (len-pos >= 4) 
			{
				// load delay
				delay[i]=GET_INT();
				framedelay[i] = (usebeats[i]?framesperbeat:delay[i])+1;
				pos+=4;
			}
		}
	}
}

// write configuration to data, return length. config data should not exceed 64k.
#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_DELAY::save_config(unsigned char *data) 
{
	int pos=0;
	PUT_INT(mode);
	pos+=4;
	PUT_INT(activebuffer);
	pos+=4;
	if (creationid == 1)
	{
		for (int i=0;i<6;i++)
		{
			PUT_INT((int)usebeats[i]);
			pos+=4;
			PUT_INT(delay[i]);
			pos+=4;
		}
	}
	return pos;
}

// export stuff
C_RBASE *R_MultiDelay(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) 
	{ 
		strcpy(desc,MOD_NAME); 
		return NULL; 
	}
	return (C_RBASE *) new C_DELAY();
}
