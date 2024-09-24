#pragma once
#include "nx/nx.h"
#include "nu/LockFreeItem.h"
#include "player/ifc_playback.h"
#include "player/ifc_player.h"
#include "player/svc_output.h"
#include "nu/ThreadLoop.h"
#include "filelock/api_filelock.h"

/* TODO: we can probably redo this without the mutex, possibly using semaphores */
class PlaybackBase : public ifc_playback, public cb_filelock
{
public:
	using ifc_playback::Retain;
	using ifc_playback::Release;


	/* ifc_playback implementation */
	int WASABICALL Playback_Play(svc_output *output, ifc_playback_parameters *secondary_parameters);
	int WASABICALL Playback_SeekSeconds(double seconds);
	int WASABICALL Playback_Pause();
	int WASABICALL Playback_Unpause();
	int WASABICALL Playback_Stop();
	int WASABICALL Playback_Close();

	/* cb_filelock implementation */
	int WASABICALL FileLockCallback_Interrupt();
protected:
	nx_thread_t playback_thread;
	svc_output *output_service;
	ifc_player *player;
	ifc_playback_parameters *secondary_parameters;
	nx_uri_t filename;
	
	enum
	{
		WAKE_KILL=(1<<0),
		WAKE_PLAY=(1<<1),
		WAKE_PAUSE=(1<<2), 
		WAKE_STOP=(1<<3),
		WAKE_INTERRUPT=(1<<4),
		WAKE_UNPAUSE=(1<<5), // this is actually unused in wake_flags, just used as a return value from Wake/WakeReason		
		WAKE_RESUME=(1<<6), // this is actually unused in wake_flags, just used as a return value from Wake/WakeReason		
		WAKE_START_MASK = WAKE_PLAY|WAKE_STOP, 
		WAKE_KILL_MASK = WAKE_KILL|WAKE_STOP,
		WAKE_ALL_MASK = WAKE_KILL|WAKE_PLAY|WAKE_PAUSE|WAKE_STOP|WAKE_INTERRUPT,
	};
	

protected:
	PlaybackBase();
	~PlaybackBase();

	/* === API for derived classes to use === */
	int Initialize(nx_uri_t filename, ifc_player *player);
	int Init();

	/* this checks if one of the flags in mask has toggled.  
	   if playback is stopped and WAKE_PLAY is in the mask, this will sleep until a flag changes
	   if playback is paused and WAKE_PAUSE is in the mask, this will sleep until a flag changes
		 if both WAKE_PLAY and WAKE_PAUSE are in the mask, this will sleep until either happens 
	   returns 0 if no flag changed */
	int Wake(int mask); /* Sleeps indefinitely until a flag changes */
	int Check(int mask); /* like Wake() but never actually goes to sleep for any reason */
	int Wait(unsigned int milliseconds, int mask); /* Sleeps for a limited amount of time for a flag to change */
	int Sleep(unsigned int milliseconds, int mask); /* unlike Wait, this one does *not* update last flags */
	int WakeReason(int mask) const;

	void OnInterrupted(); /* turn off the WAKE_INTERRUPT flag */
	void OnStopPlaying(); /* turn off the WAKE_PLAY flag */
	bool PendingSeek();
	Agave_Seek *GetSeek();
	void FreeSeek(Agave_Seek *seek);
  /* === End of API for derived classes to use === */
private:
	void ToggleFlags(int wake_reason);

	int wake_flags; /* marked volatile so the compiler doesn't cache */
	int last_wake_flags;
	Agave_Seek *queued_seek;

	ThreadLoop thread_loop;
	static void APC_Play(void *_playback_base, void *param2, double real_value);
	static void APC_Seek(void *_playback_base, void *param2, double real_value);
	static void APC_Pause(void *_playback_base, void *param2, double real_value);
	static void APC_Unpause(void *_playback_base, void *param2, double real_value);
	static void APC_Stop(void *_playback_base, void *param2, double real_value);
	static void APC_Close(void *_playback_base, void *param2, double real_value);
	static void APC_Interrupt(void *_playback_base, void *param2, double real_value);
	
};
