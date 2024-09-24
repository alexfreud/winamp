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

#include "ld32.h"

static HINSTANCE hDll;

LONG (WINAPI *_LoadPalette)(LPSTR ColorFilename, LPLONG RETURN_LDStatus);
LONG (WINAPI *_WritePoint)(LONG PointNumber, PTSTRUCT *SUPPLY_PtStruct);
LONG (WINAPI *_InitialQMCheck)(LPLONG RETURN_LDStatus);
LONG (WINAPI *_DisplayUpdate)(void);
LONG (WINAPI *_DisplayBufferStatus)(LONG *RETURN_BufferIsFree, LONG *RETURN_CurrentOutputPoints);
LONG (WINAPI *_DisplayFrame)(LONG Frame);
LONG (WINAPI *_WriteFrameEx)(FRAMESTRUCTEX *SUPPLY_FrameStruct, PTSTRUCT *SUPPLY_PointArray);
LONG (WINAPI *_SetWorkingScanners)(LONG Scanner);
LONG (WINAPI *_SetWorkingTracks)(LONG Track);
LONG (WINAPI *_SetWorkingFrame)(LONG FrameNumber);
LONG (WINAPI *_BeginSessionEx)(LPLONG RETURN_Version, LPLONG RETURN_MaxFrames, LPLONG RETURN_MaxPoints, LPLONG RETURN_MaxBuffer, LPLONG RETURN_UndoFrames, LPLONG RETURN_LDStatus);
LONG (WINAPI *_DisplayFlags)(LONG Flags);
LONG (WINAPI *_EndSession)(void);
LONG (WINAPI *_GetLDDLLModuleUsage)(LPLONG ModuleUsage);
LONG (WINAPI *_ResetLD)(void);			//Updates all track variables 
LONG (WINAPI *_ReadProjectionZone)(LONG ZoneNumber, PROJECTIONZONE *RETURN_PZ);
LONG (WINAPI *_OpenLDCommWindow)(void);
LONG (WINAPI *_DisplayProjectionZones)(long ProjectionZoneCode);

LONG WINAPI InitialQMCheck(LPLONG RETURN_LDStatus)
{
  hDll=LoadLibrary("ld2000.dll");
  if (!hDll)
  {
//    printf("LD2000: error loading DLL\n");
    *RETURN_LDStatus=1;
    return 1;
  }
#define RETR(x) *((void**)&_##x)=(void*)GetProcAddress(hDll,#x); if (!_##x) { FreeLibrary(hDll); hDll=0; *RETURN_LDStatus=1; return 1; }
  
 // \
   //             if (!_##x) printf("LD2000: error loading DLL: " #x "\n");

  RETR(ReadProjectionZone);
  RETR(InitialQMCheck);
  RETR(DisplayUpdate);
  RETR(DisplayBufferStatus);
  RETR(DisplayFrame);
  RETR(WriteFrameEx);
  RETR(WritePoint);
  RETR(SetWorkingScanners);
  RETR(LoadPalette);
  RETR(SetWorkingTracks);
  RETR(SetWorkingFrame);
  RETR(EndSession);
  RETR(BeginSessionEx);
  RETR(DisplayFlags);
  RETR(ResetLD);
  RETR(OpenLDCommWindow);
  RETR(GetLDDLLModuleUsage);
  RETR(DisplayProjectionZones);
  return _InitialQMCheck(RETURN_LDStatus);
}

LONG WINAPI DisplayProjectionZones(long ProjectionZoneCode)
{
  return _DisplayProjectionZones(ProjectionZoneCode);
}



LONG WINAPI ReadProjectionZone(LONG ZoneNumber, PROJECTIONZONE *RETURN_PZ)
{
  if (!hDll||!_ReadProjectionZone) return 1;
  return _ReadProjectionZone(ZoneNumber,RETURN_PZ);
}

LONG WINAPI DisplayBufferStatus(LONG *RETURN_BufferIsFree, LONG *RETURN_CurrentOutputPoints)
{
  return _DisplayBufferStatus(RETURN_BufferIsFree,RETURN_CurrentOutputPoints);
}

LONG WINAPI DisplayUpdate(void)
{
  return _DisplayUpdate();
}
LONG WINAPI DisplayFrame(LONG Frame)
{
  return _DisplayFrame(Frame);
}

LONG WINAPI WriteFrameEx(FRAMESTRUCTEX *SUPPLY_FrameStruct, PTSTRUCT *SUPPLY_PointArray)
{
  return _WriteFrameEx(SUPPLY_FrameStruct,SUPPLY_PointArray);
}

LONG WINAPI SetWorkingScanners(LONG Scanner)
{
  return _SetWorkingScanners(Scanner);
}

LONG WINAPI SetWorkingTracks(LONG Track)
{
  return _SetWorkingTracks(Track);
}

LONG WINAPI SetWorkingFrame(LONG FrameNumber)
{
  return _SetWorkingFrame(FrameNumber);
}

LONG WINAPI OpenLDCommWindow(void)
{
  return _OpenLDCommWindow();
}

LONG WINAPI EndSession(void)
{
  return _EndSession();
}

LONG WINAPI LoadPalette(LPSTR ColorFilename, LPLONG RETURN_LDStatus)
{
  return _LoadPalette(ColorFilename,RETURN_LDStatus);
}

LONG WINAPI WritePoint(LONG PointNumber, PTSTRUCT *SUPPLY_PtStruct)
{
  return _WritePoint(PointNumber,SUPPLY_PtStruct);
}

LONG WINAPI GetLDDLLModuleUsage(LPLONG ModuleUsage)
{
  return _GetLDDLLModuleUsage(ModuleUsage);
}

LONG WINAPI ResetLD()
{
  return _ResetLD();
}

LONG WINAPI BeginSessionEx(LPLONG RETURN_Version, LPLONG RETURN_MaxFrames, LPLONG RETURN_MaxPoints, LPLONG RETURN_MaxBuffer, LPLONG RETURN_UndoFrames, LPLONG RETURN_LDStatus)
{
  return _BeginSessionEx(
    RETURN_Version, RETURN_MaxFrames, 
    RETURN_MaxPoints, RETURN_MaxBuffer, 
    RETURN_UndoFrames,RETURN_LDStatus);
}
LONG WINAPI DisplayFlags(LONG Flags)
{
  return _DisplayFlags(Flags);
}

void LaserQuit()
{
  if (hDll) FreeLibrary(hDll);
  hDll=0;
  _InitialQMCheck=0;
}

#endif