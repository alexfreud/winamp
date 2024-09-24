#include "PlaybackBase.h"
#include <stdlib.h>
#include <assert.h>
#ifdef __ANDROID__
#include <android/log.h> // TODO: replace with generic logging API

#else
#define ANDROID_LOG_INFO 0
#define ANDROID_LOG_ERROR 1
static void __android_log_print(int, const char *, const char *, ...)
{
}
#endif
PlaybackBase::PlaybackBase()
{	
	wake_flags=0;
	last_wake_flags=0;
	playback_thread=0;
	secondary_parameters=0;
	filename=0;
	player=0;
	queued_seek=0;
	output_service=0;
}

int PlaybackBase::Initialize(nx_uri_t filename, ifc_player *player)
{
	this->player = player;
	this->filename = NXURIRetain(filename);
	return NErr_Success;
}

PlaybackBase::~PlaybackBase()
{
	if (secondary_parameters)
		secondary_parameters->Release();
	if (filename)
		NXURIRelease(filename);
	if (queued_seek)
		free(queued_seek);
	if (playback_thread)
		NXThreadJoin(playback_thread, 0);
}

int PlaybackBase::Playback_Play(svc_output *output, ifc_playback_parameters *secondary_parameters)
{
	if (!playback_thread)
		return 1;

	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] Play");

	output_service = output;
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		this->secondary_parameters = secondary_parameters;
		if (secondary_parameters)
			secondary_parameters->Retain();

		apc->func = APC_Play;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

int PlaybackBase::Playback_SeekSeconds(double seconds)
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] Seek (%f seconds)", seconds);
	Agave_Seek *seek = (Agave_Seek *)malloc(sizeof(Agave_Seek));
	if (seek)
	{
		seek->position_type = AGAVE_PLAYPOSITION_SECONDS;
		seek->position.seconds = seconds;
		threadloop_node_t *apc = thread_loop.GetAPC();
		if (apc)
		{
			apc->func = APC_Seek;
			apc->param1 = this;
			apc->param2 = seek;
			thread_loop.Schedule(apc);
			return NErr_Success;
		}
	}
	free(seek);
	return NErr_OutOfMemory;

}

int PlaybackBase::Playback_Pause()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] Pause");
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Pause;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

int PlaybackBase::Playback_Unpause()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] Unpause");
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Unpause;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

int PlaybackBase::Playback_Stop()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] Stop");
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Stop;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

int PlaybackBase::Playback_Close()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] Close");
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Close;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}


int PlaybackBase::FileLockCallback_Interrupt()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] Interrupt");
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_Interrupt;
		apc->param1 = this;
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

void PlaybackBase::ToggleFlags(int wake_reason)
{
	switch(wake_reason)
	{
	case WAKE_KILL:
		last_wake_flags ^= WAKE_KILL; /* toggle kill flag */
		break;
	case WAKE_STOP:
		last_wake_flags ^= WAKE_STOP; /* toggle stop flag */
		break;
	case WAKE_PLAY:
		last_wake_flags ^= WAKE_PLAY; /* toggle play flag */
		break;
	case WAKE_PAUSE:
	case WAKE_UNPAUSE:
		last_wake_flags ^= WAKE_PAUSE; /* toggle pause flag */
		break;
	case WAKE_INTERRUPT:
		last_wake_flags ^= WAKE_INTERRUPT; /* toggle interrupt flag */
		break;
	}
}

int PlaybackBase::WakeReason(int mask) const
{
	int reason_awoken = last_wake_flags ^ wake_flags;

	reason_awoken = reason_awoken & mask;

	if (reason_awoken & WAKE_INTERRUPT)
	{
		if (wake_flags & WAKE_INTERRUPT)
			return WAKE_INTERRUPT;
		else
			return WAKE_RESUME;
	}

	if (reason_awoken & WAKE_STOP)
	{
		return WAKE_STOP;
	}

	if (reason_awoken & WAKE_KILL)
	{
		return WAKE_KILL;
	}

	if (reason_awoken & WAKE_PLAY)
	{
		if (wake_flags & WAKE_PLAY)
			return WAKE_PLAY;
		else /* if someone cleared the play flag for whatever reason, just treat it as a 0 */
			return 0; 	
	}

	if (reason_awoken & WAKE_PAUSE)
	{
		if (wake_flags & WAKE_PAUSE)
			return WAKE_PAUSE;
		else
			return WAKE_UNPAUSE;
	}

	return 0; 
}

int PlaybackBase::Wake(int mask)
{
	assert(mask != 0); /* make sure they didn't specify a 0 mask (which would make this function potentially never return */
	assert((mask & WAKE_ALL_MASK) != 0); /* make sure it's a valid mask */

	for (;;)
	{
		int reason_awoken = last_wake_flags ^ wake_flags;
		reason_awoken = reason_awoken & mask;

		if (reason_awoken)
		{
			int ret = WakeReason(mask);
			ToggleFlags(ret); // mark the last-known-state of the wake flags 

			return ret;

		}

		if (((mask & WAKE_PLAY) && !(wake_flags & WAKE_PLAY))/* if we're stopped and they asked to be woken up for play */
			|| ((mask & WAKE_PAUSE) && (wake_flags & WAKE_PAUSE)) /* or waiting to be woken up for unpause */
			|| ((mask & WAKE_INTERRUPT) && (wake_flags & WAKE_INTERRUPT))) /* or waiting to be woken up for resume */
		{
			thread_loop.Step();

			int ret = WakeReason(mask);
			if (ret) /* if ret is !0, it means we gotten woken up for a reason we care about */
			{
				ToggleFlags(ret); // mark the last-known-state of the wake flags 
				return ret;
			}
		}
		else /* no reason to sleep, so just return 0 (no change) */
		{
			return 0;
		}
	}
}

int PlaybackBase::Check(int mask)
{
	assert(mask != 0); /* make sure they didn't specify a 0 mask (which would make this function potentially never return */
	assert((mask & WAKE_ALL_MASK) != 0); /* make sure it's a valid mask */

	int reason_awoken = last_wake_flags ^ wake_flags;
	reason_awoken = reason_awoken & mask;

	int ret = 0;
	if (reason_awoken)
	{
		ret = WakeReason(mask);
		ToggleFlags(ret); // mark the last-known-state of the wake flags 
	}
	return ret;
}


int PlaybackBase::Wait(unsigned int milliseconds, int mask)
{
	int reason_awoken = last_wake_flags ^ wake_flags;
	reason_awoken = reason_awoken & mask;

	if (reason_awoken)
	{
		int ret = WakeReason(mask);
		ToggleFlags(ret); // mark the last-known-state of the wake flags 

		return ret;
	}

	thread_loop.Step(milliseconds);

	int ret = WakeReason(mask);
	ToggleFlags(ret); // mark the last-known-state of the wake flags 
	return ret;
}

int PlaybackBase::Sleep(unsigned int milliseconds, int mask)
{
	int reason_awoken = last_wake_flags ^ wake_flags;
	reason_awoken = reason_awoken & mask;

	if (reason_awoken)
	{
		int ret = WakeReason(mask);
		assert(ret != 0);

		return ret;

	}

	thread_loop.Step(milliseconds);

	int ret = WakeReason(mask);	
	return ret;

}

void PlaybackBase::OnStopPlaying()
{
	// turn off the play flag (also adjust old wake flags so we don't trigger a WAKE_STOP)
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] OnStopPlaying");

	wake_flags &= ~WAKE_PLAY;
	last_wake_flags &= ~WAKE_PLAY;	
}

void PlaybackBase::OnInterrupted()
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[PlaybackBase] OnInterrupted");

	wake_flags &= ~WAKE_INTERRUPT;
	last_wake_flags &= ~WAKE_INTERRUPT;
}

Agave_Seek *PlaybackBase::GetSeek()
{
	Agave_Seek *seek = queued_seek;
	queued_seek=0;
	return seek;
}

void PlaybackBase::FreeSeek(Agave_Seek *seek)
{
	free(seek);
}

bool PlaybackBase::PendingSeek()
{
	return !!queued_seek;
}

void PlaybackBase::APC_Play(void *_playback_base, void *param2, double real_value)
{
	PlaybackBase *playback_base = (PlaybackBase *)_playback_base;
	playback_base->wake_flags |= WAKE_PLAY;
}

void PlaybackBase::APC_Seek(void *_playback_base, void *_seek, double real_value)
{
	PlaybackBase *playback_base = (PlaybackBase *)_playback_base;
	Agave_Seek *seek = (Agave_Seek *)_seek;
	free(playback_base->queued_seek);
	playback_base->queued_seek = seek;

}

void PlaybackBase::APC_Pause(void *_playback_base, void *param2, double real_value)
{
	PlaybackBase *playback_base = (PlaybackBase *)_playback_base;
	playback_base->wake_flags |= WAKE_PAUSE;
}

void PlaybackBase::APC_Unpause(void *_playback_base, void *param2, double real_value)
{
	PlaybackBase *playback_base = (PlaybackBase *)_playback_base;
	playback_base->wake_flags &= ~WAKE_PAUSE;
}

void PlaybackBase::APC_Stop(void *_playback_base, void *param2, double real_value)
{
	PlaybackBase *playback_base = (PlaybackBase *)_playback_base;
	playback_base->wake_flags |= WAKE_STOP;
}

void PlaybackBase::APC_Close(void *_playback_base, void *param2, double real_value)
{
	PlaybackBase *playback_base = (PlaybackBase *)_playback_base;
	playback_base->wake_flags |= WAKE_KILL;
}

void PlaybackBase::APC_Interrupt(void *_playback_base, void *param2, double real_value)
{
	PlaybackBase *playback_base = (PlaybackBase *)_playback_base;
	playback_base->wake_flags |= WAKE_INTERRUPT;
}
