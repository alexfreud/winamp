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
#ifdef LASER
#include <windows.h>
extern "C" {
#include "ld32.h"
};

int active_state;

int g_laser_nomessage,g_laser_zones;
int init=0;
int ld32_framebase=1;

static void onActivate()
{
  if (active_state) return;
  active_state=1;

  ResetLD();
  SetWorkingTracks(-1);
  SetWorkingScanners(-1);
  DisplayFlags(0);
}
static void onDeactivate()
{
  if (!active_state) return;
  active_state=0;

  ResetLD();
  SetWorkingTracks(-1);
  SetWorkingScanners(-1);
  DisplayFlags(0);
  DisplayFrame(0);
  DisplayUpdate();
}


void laser_connect(void)
{  
  LONG v;
  InitialQMCheck(&v);
  if (v != LDERR_OK)
  {
    if (!g_laser_nomessage) MessageBox(NULL,"No QM2000 found.","AVS/Laser Error",MB_OK);
    return;
  }
  {
    LONG mf,mp,mb,uf,ver;
    BeginSessionEx(&ver,&mf, &mp, &mb, &uf, &v);
    if (v != LDERR_OK)
    {
      return;
    }
    GetLDDLLModuleUsage(&v);
    if (v != 1)
    {
      if (!g_laser_nomessage) MessageBox(NULL,"QM2000 is being shared with other software.","AVS/Laser Warning",MB_OK);
    }
    else
    {
      onActivate();
    }
    ld32_framebase=mf;
  }

  init=1;
}
extern "C" void LaserQuit();


void laser_disconnect(void)
{
  if (!init) return;
  init=0;
  onDeactivate();
  LaserQuit();
}

void laser_sendframe(void *data, int datalen)
{
  struct t
  {
  FRAMESTRUCTEX frame;
  PTSTRUCT points[4096];
  } *framedata=(struct t *)data;

  if (!init) return;

  extern int g_laser_nomessage;
  if (g_laser_nomessage&4) 
  {
    onDeactivate();
    return;
  }
  if (g_laser_nomessage&2)
  {
    DWORD dw;
    GetWindowThreadProcessId(GetForegroundWindow(),&dw);
    if (dw != GetCurrentProcessId()) 
    {
      onDeactivate();
      return;
    }
  }
  onActivate();
 

  SetWorkingFrame(ld32_framebase);
  WriteFrameEx(&framedata->frame,framedata->points);

  SetWorkingTracks(1);
  DisplayFlags(0);
  DisplayFrame(ld32_framebase);
  DisplayProjectionZones(g_laser_zones);

  LONG bif=0, cop;
  if (!(g_laser_nomessage&8)) DisplayBufferStatus(&bif,&cop);
  if (!bif) 
    DisplayUpdate();
}
#endif