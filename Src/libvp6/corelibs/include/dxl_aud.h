#ifndef _dxl_aud_h
#define _dxl_aud_h

#include "duck_hfb.h"
#include "duck_dxl.h"


enum spkrst8 { SPEAKEROFF = 0, SPEAKERON = 1, FEEDSPEAKER = 2, MONOSPEAKER = 3};
enum syncst8 { NOSYNC = 0, SYNCSPEAKER = 1, SYNCSYSTEM = 2};


extern enum spkrst8 speakerstate;
extern enum syncst8 syncstate;


void Announcement(char *msg);

extern "C" { 

	int FillAudio( HFB_BUFFER_HANDLE hfb, MFP_STREAM_HANDLE as, int *index,
			   void **blk, long *Len, int buffPreload, int MultiBlock);
			   
	int SetupAudio( HFB_BUFFER_HANDLE hfb, MFP_STREAM_HANDLE as, int Setstate, 
				int freq, int width16, int Stereo);
				
	void StartPlaying(void) ;
	
	void StopPlaying(void);
	
	
	void ResyncAudio(
	HFB_BUFFER_HANDLE hfb, 
	HFB_STREAM_HANDLE astr,
	int *index, void **blk, long *Len, int frame, int frame_rate);
}
			   
			   
#endif
