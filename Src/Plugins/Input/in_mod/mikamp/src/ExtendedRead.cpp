extern "C" 
{
#include "main.h"
}
#include "drv_buffer.h"
#include <bfc/platform/types.h>

typedef struct
{
	const char *cmd;
	const char *file;
	const char *title;
	int   titleLength;
	int   start;
	int   startUnit;
	int   loops;
	int   flags;
} PlayParams;
extern "C" int GetSampleSizeFlag();
extern "C" MMSTREAM *_mm_fopen_rf(const CHAR *fname);	//rf_wrapper.c
BOOL GetPlayParams(const char *fileName, BOOL open, PlayParams *params);
BOOL InitPlayer(UNIMOD *mf, MPLAYER **ps, const PlayParams *params, BOOL quick);
// TODO; is there a way to get floating point out of this stuff?
extern "C" 
{
	__declspec(dllexport) intptr_t winampGetExtendedRead_open(const char *fn, int *size, int *bps, int *nch, int *srate)
	{
		PlayParams params;
		if (!GetPlayParams(fn, FALSE, &params))
			return 0;

		int requested_channels = *nch;
		int requested_bits = *bps;
		int requested_srate = *srate;

		uint md_mode = 0;
		if (config_interp & 1) md_mode |= DMODE_INTERP;
		if (config_interp & 2) md_mode |= DMODE_NOCLICK;
		if (config_interp & 4) md_mode |= DMODE_FIR;
		switch(requested_bits)
		{
		case 0:
			md_mode |= GetSampleSizeFlag();
			break;
		case 16:
			md_mode |= DMODE_16BITS;
			break;
		case 24:
			md_mode |= DMODE_24BITS;
			break;
		}


		if (requested_channels != 1 && requested_channels != 2) md_mode |= DMODE_SURROUND;
		if (config_panrev)     md_mode |= DMODE_REVERSE;
		if (config_resonance)  md_mode |= DMODE_RESONANCE;

		MDRIVER *md = Mikmod_Init(requested_srate?requested_srate:config_srate, 0, 0, MD_STEREO, config_cpu, md_mode, &drv_buffer);
		MPLAYER *mp;

		MMSTREAM  *fp;
		fp = _mm_fopen_rf(params.file);
		if (!fp)
		{
			Mikmod_Exit(md);
			//CleanupTemp();
			return 0;
		}
		UNIMOD *mf=Unimod_Load_FP(md, params.file,fp);
		_mm_fclose(fp);
		if (mf==NULL)
		{				
			Mikmod_Exit(md);
			//                CleanupTemp();
			return 0;
		}

		if (!InitPlayer(mf, &mp, &params, FALSE))
		{
			//CleanupTemp();
			Unimod_Free(mf);
			Mikmod_Exit(md);
			return 0;
		}

		Player_Start(mp);
		DecodeInfo  *hwdata = (DecodeInfo *)md->device.local;
		*bps = hwdata->bits;
		*srate = hwdata->mixspeed;
		*nch = hwdata->channels;
		if (mf->songlen)
			*size = MulDiv(mf->songlen, hwdata->mixspeed * hwdata->channels *hwdata->bits, 8*1000);
		else
			*size = -1; 
		return (intptr_t)mp;
	}

	__declspec(dllexport) size_t winampGetExtendedRead_getData(intptr_t handle, char *dest, size_t len, int *killswitch)
	{
		MPLAYER *mp = (MPLAYER *)handle;
		DecodeInfo  *hwdata = (DecodeInfo *)mp->mf->md->device.local;

		if (!Player_Active(mp)) // check if we're done
			return 0;

		hwdata->buffer = dest;
		hwdata->buffersize = len;
		hwdata->bytesWritten = 0;
		Mikmod_Update(mp->mf->md);
		return hwdata->bytesWritten;
	}


	__declspec(dllexport) void winampGetExtendedRead_close(intptr_t handle)
	{
		MPLAYER *mp = (MPLAYER *)handle;
		MDRIVER *md = mp->mf->md;
		UNIMOD *mf = (UNIMOD *)mp->mf;		
		Player_Free(mp);
		Unimod_Free(mf);
		Mikmod_Exit(md);
	}
}