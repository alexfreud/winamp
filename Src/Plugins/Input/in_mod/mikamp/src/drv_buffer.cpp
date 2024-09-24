#include <windows.h>
#include <malloc.h>
#include "mikmod.h"
#include "virtch.h"
#include "main.h"
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "drv_buffer.h"

#define BUFSIZE 28


// =====================================================================================
static BOOL Decode_IsThere(void)
// =====================================================================================
{
	return 1;
}


// =====================================================================================
static BOOL Decode_Init(MDRIVER *md, uint latency, void *optstr)
// =====================================================================================
{
	DecodeInfo  *hwdata;

	hwdata = (DecodeInfo *)MikMod_calloc(md->allochandle, 1, sizeof(DecodeInfo));

	md->device.vc = VC_Init();
	if(!md->device.vc)
	{   
		hwdata->error = 1;
		return 1;
	}

	hwdata->mode     = DMODE_16BITS | DMODE_INTERP | DMODE_NOCLICK;
	hwdata->mixspeed = 48000;
	hwdata->channels = 2;

	md->device.local = hwdata;

	return 0;
}


// =====================================================================================
static void Decode_Exit(MDRIVER *md)
// =====================================================================================
{
	DecodeInfo  *hwdata = (DecodeInfo *)md->device.local;
	VC_Exit(md->device.vc);
}


// =====================================================================================
static void Decode_Update(MDRIVER *md)
// =====================================================================================
{
	DecodeInfo  *hwdata = (DecodeInfo *)md->device.local;

	hwdata->bytesWritten = VC_WriteBytes(md, (SBYTE *)hwdata->buffer, hwdata->buffersize);
}


// =====================================================================================
static BOOL Decode_SetMode(MDRIVER *md, uint mixspeed, uint mode, uint channels, uint cpumode)
// =====================================================================================
{
	DecodeInfo  *hwdata = (DecodeInfo *)md->device.local;

	// Check capabilities...
	// [...]

	// Set the new mode of play

	if (mixspeed) hwdata->mixspeed = mixspeed;

	if(!(mode & DMODE_DEFAULT)) hwdata->mode = mode;

	switch(channels)
	{
	case MD_MONO:
	hwdata->channels = 1;
	break;

	default:
		hwdata->channels = 2;
		channels  = MD_STEREO;
		break;
	}

	VC_SetMode(md->device.vc, hwdata->mixspeed, hwdata->mode, channels, cpumode);

		int     bits = (hwdata->mode & DMODE_16BITS) ? 16: ((hwdata->mode & DMODE_24BITS) ? 24 : 8);

		hwdata->bits = bits;
		hwdata->frame_size = MulDiv(hwdata->channels, bits, 8);


	return 0;
}


// =====================================================================================
static BOOL Decode_SetSoftVoices(MDRIVER *md, uint voices)
// =====================================================================================
{
	return VC_SetSoftVoices(md->device.vc, voices);
}


// =====================================================================================
static void Decode_GetMode(MDRIVER *md, uint *mixspeed, uint *mode, uint *channels, uint *cpumode)
// =====================================================================================
{
	VC_GetMode(md->device.vc, mixspeed, mode, channels, cpumode);
}


// =====================================================================================
extern "C" MD_DEVICE drv_buffer =
// =====================================================================================
{
	"ExtendedRead",
		BLAH("Nullsoft Extended Read decode driver v0.1"),
		0, VC_MAXVOICES, 

		NULL,
		NULL,
		NULL,

		// Sample Loading
		VC_SampleAlloc,
		VC_SampleGetPtr,
		VC_SampleLoad,
		VC_SampleUnload,
		VC_SampleSpace,
		VC_SampleLength,

		// Detection and Initialization
		Decode_IsThere,
		Decode_Init,
		Decode_Exit,
		Decode_Update,
		VC_Preempt,

		NULL,
		Decode_SetSoftVoices,

		Decode_SetMode,
		Decode_GetMode,

		VC_SetVolume,
		VC_GetVolume,

		// Voice control and Voie information
		VC_GetActiveVoices,

		VC_VoiceSetVolume,
		VC_VoiceGetVolume,
		VC_VoiceSetFrequency,
		VC_VoiceGetFrequency,
		VC_VoiceSetPosition,
		VC_VoiceGetPosition,
		VC_VoiceSetSurround,
		VC_VoiceSetResonance,

		VC_VoicePlay,
		VC_VoiceResume,
		VC_VoiceStop,
		VC_VoiceStopped,
		VC_VoiceReleaseSustain,
		VC_VoiceRealVolume,

};