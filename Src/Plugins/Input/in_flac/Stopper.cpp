/*
** Copyright (C) 2008 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: January 31, 2008
**
*/
#include "Stopper.h"
#include "main.h"
#include "../Winamp/wa_ipc.h"

Stopper::Stopper() : isplaying(0), timems(0)
{
}

void Stopper::ChangeTracking(bool mode)
{
	SendMessage(plugin.hMainWindow, WM_USER, mode, IPC_ALLOW_PLAYTRACKING); // enable / disable stats updating
}

void Stopper::Stop()
{
	isplaying = SendMessage(plugin.hMainWindow, WM_USER, 0, IPC_ISPLAYING);
	if (isplaying)
	{
		ChangeTracking(0); // disable stats updating
		timems = SendMessage(plugin.hMainWindow, WM_USER, 0, IPC_GETOUTPUTTIME);
		SendMessage(plugin.hMainWindow, WM_COMMAND, 40047, 0); // Stop
	}
}

void Stopper::Play()
{
	if (isplaying) // this works _most_ of the time, not sure why a small portion of the time it doesnt hrmph :/
		// ideally we should replace it with a system that pauses the decode thread, closes its file,
		// does the shit, and reopens and reseeks to the new offset. for gaplessness
	{
		if (timems)
		{
			m_force_seek = timems; //  SendMessage(mod.hMainWindow,WM_USER,timems,106);
		}
		else 
			m_force_seek = -1;
		SendMessage(plugin.hMainWindow, WM_COMMAND, 40045, 0); // Play
		//m_force_seek = -1;
		if (isplaying & 2)
		{
			SendMessage(plugin.hMainWindow, WM_COMMAND, 40046, 0); // Pause
		}
		ChangeTracking(1); // enable stats updating
	}
}