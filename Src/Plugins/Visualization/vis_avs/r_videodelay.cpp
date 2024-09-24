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

#ifndef LASER

#define MOD_NAME "Trans / Video Delay"
#define C_DELAY C_VideoDelayClass

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
		bool enabled;
		bool usebeats;
		unsigned int delay;

		// unsaved members
		LPVOID buffer;
		LPVOID inoutpos;
		unsigned long buffersize;
		unsigned long virtualbuffersize;
		unsigned long oldvirtualbuffersize;
		unsigned long framessincebeat;
		unsigned long framedelay;
		unsigned long framemem;
		unsigned long oldframemem;
};

// global configuration dialog pointer 
static C_DELAY *g_Delay; 
// global DLL instance pointer
static HINSTANCE g_hDllInstance; 

// configuration screen
static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	char value[16];
	int val;
	unsigned int objectcode, objectmessage;
	HWND hwndEdit;
	switch (uMsg)
	{
		case WM_INITDIALOG: //init
			CheckDlgButton(hwndDlg,IDC_CHECK1,g_Delay->enabled);
			CheckDlgButton(hwndDlg,IDC_RADIO1,g_Delay->usebeats);
			CheckDlgButton(hwndDlg,IDC_RADIO2,!g_Delay->usebeats);
			hwndEdit = GetDlgItem(hwndDlg,IDC_EDIT1);
			_itoa(g_Delay->delay,value,10);
			SetWindowText(hwndEdit,value);
			return 1;
		case WM_COMMAND:
			objectcode = LOWORD(wParam);
			objectmessage = HIWORD(wParam);
			// see if enable checkbox is checked
			if (objectcode == IDC_CHECK1)
			{
				g_Delay->enabled = IsDlgButtonChecked(hwndDlg,IDC_CHECK1)==1;
			    return 0;
			}
			// see if beats radiobox is checked
			if (objectcode == IDC_RADIO1)
			{
				if(IsDlgButtonChecked(hwndDlg,IDC_RADIO1)==1)
				{
					g_Delay->usebeats = true;
					CheckDlgButton(hwndDlg,IDC_RADIO2,BST_UNCHECKED);
					g_Delay->framedelay = 0;
					g_Delay->framessincebeat = 0;
					hwndEdit = GetDlgItem(hwndDlg,IDC_EDIT1);	//new
					if (g_Delay->delay>16) {	//new
						g_Delay->delay = 16;	//new
						SetWindowText(hwndEdit,"16");	//new
					}	//new
				}
				else g_Delay->usebeats = false;
			    return 0;
			}
			// see if frames radiobox is checked
			if (objectcode == IDC_RADIO2)
			{
				if(IsDlgButtonChecked(hwndDlg,IDC_RADIO2)==1)
				{
					g_Delay->usebeats = false;
					CheckDlgButton(hwndDlg,IDC_RADIO1,BST_UNCHECKED);
					g_Delay->framedelay = g_Delay->delay;
				}
				else g_Delay->usebeats = true;
			    return 0;
			}
			//get and put data from the delay box
			if (objectcode == IDC_EDIT1)
			{
				hwndEdit = GetDlgItem(hwndDlg,IDC_EDIT1);
				if (objectmessage == EN_CHANGE)
				{
					GetWindowText(hwndEdit,value,16);
					val = atoi(value);
					if (g_Delay->usebeats) {if (val > 16) val = 16;}	//new
					else {if (val > 200) val = 200;}	//new
					g_Delay->delay = val;
					g_Delay->framedelay = g_Delay->usebeats?0:g_Delay->delay;
				}
				else if (objectmessage == EN_KILLFOCUS)
				{
					_itoa(g_Delay->delay,value,10);
					SetWindowText(hwndEdit,value);
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
	enabled = true;
	usebeats = false;
	delay = 10;
	framedelay = 10;
	framessincebeat = 0;
	buffersize = 1;
	virtualbuffersize = 1;
	oldvirtualbuffersize = 1;
	buffer = VirtualAlloc(NULL,buffersize,MEM_COMMIT,PAGE_READWRITE);
	inoutpos = buffer;
}

// virtual destructor
C_DELAY::~C_DELAY() 
{
	VirtualFree(buffer,buffersize,MEM_DECOMMIT);
}

// RENDER FUNCTION:
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout
// w and h are the-width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].

int C_DELAY::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (isBeat&0x80000000) return 0;

	framemem = w*h*4;
	if (usebeats)
	{
		if (isBeat)
		{
			framedelay = framessincebeat*delay;	//changed
			if (framedelay > 400) framedelay = 400;	//new
			framessincebeat = 0;
		}
		framessincebeat++;
	}
	if (enabled && framedelay!=0)
	{
		virtualbuffersize = framedelay*framemem;
		if (framemem == oldframemem)
		{
			if (virtualbuffersize != oldvirtualbuffersize)
			{
				if (virtualbuffersize > oldvirtualbuffersize)
				{
					if (virtualbuffersize > buffersize)
					{
						// allocate new memory
						if (!VirtualFree(buffer,buffersize,MEM_DECOMMIT)) return 0;
						if (usebeats)
						{
							buffersize = 2*virtualbuffersize;
							if (buffersize > framemem*400) buffersize = framemem*400;	//new
							buffer = VirtualAlloc(NULL,buffersize,MEM_COMMIT,PAGE_READWRITE);
							if (buffer == NULL)
							{
								buffersize = virtualbuffersize;
								buffer = VirtualAlloc(NULL,buffersize,MEM_COMMIT,PAGE_READWRITE);
							}
						}
						else
						{
							buffersize = virtualbuffersize;
							buffer = VirtualAlloc(NULL,buffersize,MEM_COMMIT,PAGE_READWRITE);
						}
						inoutpos = buffer;
						if (buffer == NULL)
						{
							framedelay = 0;
							framessincebeat = 0;
							return 0;
						}
					}
					else
					{
						unsigned long size = (((unsigned long)buffer)+oldvirtualbuffersize) - ((unsigned long)inoutpos);
						unsigned long l = ((unsigned long)buffer)+virtualbuffersize;
						unsigned long d =  l - size;
						MoveMemory((LPVOID)d, inoutpos, size);
						for (l = (unsigned long)inoutpos; l < d; l += framemem) CopyMemory((LPVOID)l,(LPVOID)d,framemem);
					}
				}
				else
				{	// virtualbuffersize < oldvirtualbuffersize
					unsigned long presegsize = ((unsigned long)inoutpos)-((unsigned long)buffer)+framemem;
					if (presegsize > virtualbuffersize)
					{
						MoveMemory(buffer,(LPVOID)(((unsigned long)buffer)+presegsize-virtualbuffersize),virtualbuffersize);
						inoutpos = (LPVOID)(((unsigned long)buffer)+virtualbuffersize-framemem);
					}
					else if (presegsize < virtualbuffersize) MoveMemory((LPVOID)(((unsigned long)inoutpos)+framemem),(LPVOID)(((unsigned long)buffer)+oldvirtualbuffersize+presegsize-virtualbuffersize),virtualbuffersize-presegsize);
				}
				oldvirtualbuffersize = virtualbuffersize;
			}
		}
		else
		{
			// allocate new memory
			if (!VirtualFree(buffer,buffersize,MEM_DECOMMIT)) return 0;
			if (usebeats)
			{
				buffersize = 2*virtualbuffersize;
				buffer = VirtualAlloc(NULL,buffersize,MEM_COMMIT,PAGE_READWRITE);
				if (buffer == NULL)
				{
					buffersize = virtualbuffersize;
					buffer = VirtualAlloc(NULL,buffersize,MEM_COMMIT,PAGE_READWRITE);
				}
			}
			else
			{
				buffersize = virtualbuffersize;
				buffer = VirtualAlloc(NULL,buffersize,MEM_COMMIT,PAGE_READWRITE);
			}
			inoutpos = buffer;
			if (buffer == NULL)
			{
				framedelay = 0;
				framessincebeat = 0;
				return 0;
			}
			oldvirtualbuffersize = virtualbuffersize;
		}
		oldframemem = framemem;
		CopyMemory(fbout,inoutpos,framemem);
		CopyMemory(inoutpos,framebuffer,framemem);
		inoutpos = (LPVOID)(((unsigned long)inoutpos)+framemem);
		if ((unsigned long)inoutpos>=((unsigned long)buffer)+virtualbuffersize) inoutpos = buffer;
		return 1;
	}
	else return 0;
}

HWND C_DELAY::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_Delay = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_VIDEODELAY,hwndParent,g_DlgProc);
}

char *C_DELAY::get_desc(void)
{ 
	static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_VIDEO_DELAY,desc,128):desc);
}

// load_/save_config are called when saving and loading presets (.avs files)
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_DELAY::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	// always ensure there is data to be loaded
	if (len-pos >= 4) 
	{
		// load activation toggle
		enabled=(GET_INT()==1);
		pos+=4; 
	}
	if (len-pos >= 4) 
	{
		// load beats toggle
		usebeats=(GET_INT()==1);
		pos+=4; 
	}
	if (len-pos >= 4) 
	{
		// load delay
		delay=GET_INT();

    if (usebeats) {if (delay > 16) delay = 16;}	//new
		else {if (delay > 200) delay = 200;}	//new

		pos+=4; 
	}
}

// write configuration to data, return length. config data should not exceed 64k.
#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_DELAY::save_config(unsigned char *data) 
{
	int pos=0;
	PUT_INT((int)enabled);
	pos+=4;
	PUT_INT((int)usebeats);
	pos+=4;
	PUT_INT((unsigned int)delay);
	pos+=4;
	return pos;
}

// export stuff
C_RBASE *R_VideoDelay(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) 
	{ 
		strcpy(desc,MOD_NAME); 
		return NULL; 
	}
	return (C_RBASE *) new C_DELAY();
}

#endif