#include "Stopper.h"
#include "main.h"
#include "../Winamp/wa_ipc.h"

int m_force_seek=-1;

Stopper::Stopper() : isplaying(0), timems(0)
{
}

void Stopper::ChangeTracking(bool mode)
{
	SendMessage(mod.hMainWindow, WM_USER, mode, IPC_ALLOW_PLAYTRACKING); // enable / disable stats updating
}

void Stopper::Stop()
{
	isplaying = SendMessage(mod.hMainWindow, WM_USER, 0, IPC_ISPLAYING);
	if (isplaying)
	{
		ChangeTracking(0); // disable stats updating
		timems = SendMessage(mod.hMainWindow, WM_USER, 0, IPC_GETOUTPUTTIME);
		SendMessage(mod.hMainWindow, WM_COMMAND, 40047, 0); // Stop
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
		else m_force_seek = -1;
		SendMessage(mod.hMainWindow, WM_COMMAND, 40045, 0); // Play
		m_force_seek = -1;
		if (isplaying & 2)
		{
			SendMessage(mod.hMainWindow, WM_COMMAND, 40046, 0); // Pause
		}
		ChangeTracking(1); // enable stats updating
	}
	isplaying = 0;
}