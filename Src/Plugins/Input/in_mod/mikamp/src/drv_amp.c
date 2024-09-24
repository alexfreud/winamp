#include <windows.h>
#include <malloc.h>
#include "mikmod.h"
#include "virtch.h"
#include "main.h"
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 28

typedef struct AMP_LOCALINFO
{
    uint    mode;
    uint    mixspeed;
    uint    channels;

    SBYTE   RAW_DMABUF[(BUFSIZE*1024*2) + 64];  // added 64 for mmx mixer (it's not exact :)

    int     block_len, bits,ismono, bytes_per_sec;

} AMP_LOCALINFO;

extern int decode_pos;  // from main.c

// =====================================================================================
    static BOOL RAW_IsThere(void)
// =====================================================================================
{
    return 1;
}


// =====================================================================================
    static BOOL RAW_Init(MDRIVER *md, uint latency, void *optstr)
// =====================================================================================
{
    AMP_LOCALINFO  *hwdata;
    
    hwdata = (AMP_LOCALINFO *)MikMod_calloc(md->allochandle, 1, sizeof(AMP_LOCALINFO));
    
    md->device.vc = VC_Init();
    if(!md->device.vc)
    {   mikmod.outMod->Close();
        return 1;
    }

    hwdata->mode     = DMODE_16BITS | DMODE_INTERP | DMODE_NOCLICK;
    hwdata->mixspeed = 48000;
    hwdata->channels = 2;

    md->device.local = hwdata;

    return 0;
}


// =====================================================================================
    static void RAW_Exit(MDRIVER *md)
// =====================================================================================
{
    VC_Exit(md->device.vc);
    mikmod.outMod->Close();
}


// =====================================================================================
    static void RAW_Update(MDRIVER *md)
// =====================================================================================
{
    AMP_LOCALINFO  *hwdata = md->device.local;
	int             l;
	char * vis;
	int visbits;

    if ((l=mikmod.outMod->CanWrite()) > hwdata->block_len*16) l = hwdata->block_len*16;
    if (mikmod.dsp_isactive()) l>>=1;

    if (l > hwdata->block_len)
    {	int o=0;

        l -= l % hwdata->block_len;
        VC_WriteBytes(md, hwdata->RAW_DMABUF, l);

        while (o < l)
        {
            int a = min(hwdata->block_len,l-o);
			
            if (mikmod.dsp_isactive())
            {	int t;
                int k = (hwdata->bits>>3)*(hwdata->ismono?1:2);

                t = mikmod.dsp_dosamples((short *)(hwdata->RAW_DMABUF+o),a / k,hwdata->bits,(hwdata->ismono?1:2),hwdata->mixspeed) * k;
                mikmod.outMod->Write(hwdata->RAW_DMABUF+o,t);
			} else
                mikmod.outMod->Write(hwdata->RAW_DMABUF+o,a);

			vis=hwdata->RAW_DMABUF+o;
			visbits=hwdata->bits;

            if (visbits > 16)
			{
				uint n = 576 * 2>>hwdata->ismono;
				const uint d = visbits >> 3;
	            WORD *const visbuf = (WORD*)alloca(n * sizeof(WORD));
				char *ptr = vis + d - 2;
				WORD *vp = visbuf;

				for (;n;n--)
				{
					*vp++ = *(WORD*)ptr;
					ptr += d;
				}
				vis=(char*)visbuf;
				visbits=16;
			}
			
			mikmod.SAAddPCMData(vis,hwdata->ismono ? 1 : 2, visbits, decode_pos/64);
			mikmod.VSAAddPCMData(vis,hwdata->ismono ? 1 : 2, visbits, decode_pos/64);

			decode_pos += (a*1000*64) / hwdata->bytes_per_sec;
			o+=a;
		}
    } else Sleep(6);
}


// =====================================================================================
    static BOOL RAW_SetMode(MDRIVER *md, uint mixspeed, uint mode, uint channels, uint cpumode)
// =====================================================================================
{
    AMP_LOCALINFO  *hwdata = md->device.local;
        
    // Check capabilities...
    // [...]
    
    // Set the new mode of play

    if (mixspeed) hwdata->mixspeed = mixspeed;

    if(!(mode & DMODE_DEFAULT)) hwdata->mode = mode;

    switch(channels)
    {   case MD_MONO:
            hwdata->channels = 1;
        break;

        default:
            hwdata->channels = 2;
            channels  = MD_STEREO;
        break;
    }

    VC_SetMode(md->device.vc, hwdata->mixspeed, hwdata->mode, channels, cpumode);

    {
	int     bits = (hwdata->mode & DMODE_16BITS) ? 16: ((hwdata->mode & DMODE_24BITS) ? 24 : 8);
    int     z;
	int     a = 576*2*(bits>>3);

    hwdata->bits = bits;
    hwdata->ismono = (hwdata->channels == 1) ? 1 : 0;

	if (hwdata->ismono) a/=2;
	
    hwdata->block_len = a;

	hwdata->bytes_per_sec = hwdata->mixspeed * (hwdata->bits>>3) * (hwdata->ismono ? 1 : 2);

    z = mikmod.outMod->Open(hwdata->mixspeed,hwdata->channels,bits,-1,-1);
    if (z < 0) return 1;

	mikmod.SAVSAInit(z,hwdata->mixspeed);
	mikmod.VSASetInfo(hwdata->mixspeed,hwdata->channels);

    mikmod.outMod->SetVolume(-666);
    }

    return 0;
}


// =====================================================================================
    static BOOL AMP_SetSoftVoices(MDRIVER *md, uint voices)
// =====================================================================================
{
    return VC_SetSoftVoices(md->device.vc, voices);
}


// =====================================================================================
    static void AMP_GetMode(MDRIVER *md, uint *mixspeed, uint *mode, uint *channels, uint *cpumode)
// =====================================================================================
{
    VC_GetMode(md->device.vc, mixspeed, mode, channels, cpumode);
}


// =====================================================================================
    MD_DEVICE drv_amp =
// =====================================================================================
{
    "win32au",
    BLAH("Nullsoft win32 output driver v0.700"),
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
    RAW_IsThere,
    RAW_Init,
    RAW_Exit,
    RAW_Update,
    VC_Preempt,

    NULL,
    AMP_SetSoftVoices,

    RAW_SetMode,
    AMP_GetMode,

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