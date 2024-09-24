#include "main.h"
#include <ks.h>
#include <ksmedia.h>
#include <malloc.h>

static void make_wfx(WAVEFORMATEX * wfx,int srate,int nch,int bps)
{
	wfx->wFormatTag=WAVE_FORMAT_PCM;
	wfx->nChannels=nch;
	wfx->nSamplesPerSec=srate;
	wfx->nAvgBytesPerSec=srate*nch*(bps>>3);
	wfx->nBlockAlign=nch * (bps>>3);
	wfx->wBitsPerSample=bps;
	wfx->cbSize=0;
}

static void make_wfxe(WAVEFORMATEXTENSIBLE * wfx,int srate,int nch,int bps)
{
	make_wfx(&wfx->Format,srate,nch,bps);
	wfx->Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
	wfx->Format.cbSize=22;
	wfx->Samples.wReserved=0;
	wfx->dwChannelMask=0;
	wfx->SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
}

#ifndef IN_MIDI_NO_WAVEIN_SOURCE

extern cfg_int cfg_samp_revert;

#define MMBOOL MIXERCONTROLDETAILS_BOOLEAN

static MMBOOL *do_mixer_shit(DWORD param,DWORD type,BOOL store,UINT input,MMBOOL *tab)
{
	UINT id=0;
	mixerGetID((HMIXEROBJ)param,&id,type);

	MIXERCAPS caps;
	mixerGetDevCaps(id,&caps,sizeof(caps));
	MIXERLINE ml;
	ZeroMemory(&ml,sizeof(ml));
	ml.cbStruct=sizeof(ml);
	ml.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

	mixerGetLineInfo((HMIXEROBJ)id,&ml,MIXER_GETLINEINFOF_COMPONENTTYPE|MIXER_OBJECTF_MIXER);

	MIXERLINECONTROLS cs;
	MIXERCONTROL c;
	ZeroMemory(&cs,sizeof(cs));
	cs.cbStruct=sizeof(cs);
	cs.cControls=1;
	cs.dwLineID=ml.dwLineID;
	cs.dwControlType=MIXERCONTROL_CONTROLTYPE_MUX;
	cs.cbmxctrl=sizeof(c);
	cs.pamxctrl=&c;
	ZeroMemory(&c,sizeof(c));
	c.cbStruct=sizeof(c);

	if (!mixerGetLineControls((HMIXEROBJ)id,&cs,MIXER_OBJECTF_MIXER|MIXER_GETLINECONTROLSF_ONEBYTYPE))
	{
		if (store)
		{
			if (!tab)
			{
				tab=(MMBOOL*)alloca(sizeof(MMBOOL)*c.cMultipleItems);
				memset(tab,0,sizeof(MMBOOL)*c.cMultipleItems);
				tab[input].fValue=1;
			}
		}
		else 
		{
			if (!tab) tab=new MMBOOL[c.cMultipleItems];
		}

		if (tab)
		{
			MIXERCONTROLDETAILS d;
			d.cbStruct=sizeof(d);
			d.dwControlID=c.dwControlID;
			d.cbDetails=sizeof(MMBOOL);
			d.cChannels=ml.cChannels;
			d.cMultipleItems=c.cMultipleItems;
			d.paDetails=tab;
			
			if (store) mixerSetControlDetails((HMIXEROBJ)id,&d,MIXER_SETCONTROLDETAILSF_VALUE |MIXER_OBJECTF_MIXER);
			else mixerGetControlDetails((HMIXEROBJ)id,&d,MIXER_GETCONTROLDETAILSF_VALUE |MIXER_OBJECTF_MIXER);
		}
	}
	return tab;
}
#endif

class CVis : public CStream
{
private:
#ifndef IN_MIDI_NO_WAVEIN_SOURCE
	MMBOOL * old_settings;
	UINT wavein_id;
	void src_init()
	{
		wavein_id=(UINT)cfg_wavein_dev;
		if (cfg_wavein_src)
		{
			if (cfg_samp_revert) old_settings = do_mixer_shit(wavein_id,MIXER_OBJECTF_WAVEIN,0,0,0);
			do_mixer_shit(wavein_id,MIXER_OBJECTF_WAVEIN,1,cfg_wavein_src-1,0);
		}
	}
	void src_deinit()
	{
		if (old_settings)
		{
			do_mixer_shit(wavein_id,MIXER_OBJECTF_WAVEIN,1,0,old_settings);
			delete[] old_settings;
			old_settings=0;
		}
	}
#endif
	bool eof;
public:
	bool init(int p_srate,int p_nch,int p_bps);	
	void Eof() {eof=1;}


	virtual void Pause(int);
	virtual UINT ReadData(void*,UINT,bool*);
	virtual void Flush();
	virtual ~CVis();
	CVis()
	{
#ifndef IN_MIDI_NO_WAVEIN_SOURCE
		old_settings=0;
#endif
		eof=0;buffer=0;blox=0;hWi=0;}

private:
	BYTE * buffer;
	UINT bufsize;
	UINT read_pos;
	UINT data;
	UINT blocksize;
	HWAVEIN hWi;
	WAVEHDR *blox;
	UINT numblocks;
	UINT cur_block,cur_done;
	int paused;
	UINT in_mm;
	int srate,nch,bps;
//	void on_done(WAVEBUFFER*);
};


void CVis::Flush()
{
	if (paused) return;
	waveInReset(hWi);
	UINT n;
	for(n=0;n<numblocks;n++)
	{
		blox[n].dwUser=0;
		waveInAddBuffer(hWi,&blox[n],sizeof(WAVEHDR));	
	}

	cur_block=0;
	cur_done=0;
	in_mm=numblocks;//added all blocks already

	read_pos=0;
	data=0;
	waveInStart(hWi);
}

void CALLBACK waveInProc(HWAVEIN hWi,UINT msg,DWORD dwIns,DWORD p1,DWORD p2)
{
	if (msg==WIM_DATA && p1)
	{
		((WAVEHDR*)p1)->dwUser=1;
	}
}

bool CVis::init(int p_srate,int p_nch,int p_bps)
{
	srate=p_srate;
	nch=p_nch;
	bps=p_bps;
	blocksize=576 * (bps/8) * (nch);
	if (cfg_sampout) blocksize<<=3;
	numblocks=(2 * srate * nch * (bps>>3))/blocksize;
	bufsize=numblocks*blocksize;
	blox=new WAVEHDR[numblocks];
	memset(blox,0,sizeof(WAVEHDR)*numblocks);
	buffer=(BYTE*)malloc(bufsize);

	try
	{
		WAVEFORMATEX wfx;
		make_wfx(&wfx,srate,nch,bps);
		if (waveInOpen(&hWi,cfg_wavein_dev,&wfx,(DWORD)waveInProc,0,CALLBACK_FUNCTION))
		{
			WAVEFORMATEXTENSIBLE wfxe = {0};
			make_wfxe(&wfxe,srate,nch,bps);
			if (waveInOpen(&hWi,cfg_wavein_dev,&wfxe.Format,(DWORD)waveInProc,0,CALLBACK_FUNCTION))
			{
				return 0;
			}
		}
	} catch(...)//gay drivers etc
	{
		return 0;
	}
#ifndef IN_MIDI_NO_WAVEIN_SOURCE
	src_init();
#endif

	UINT n;
	for(n=0;n<numblocks;n++)
	{
		blox[n].lpData=(char*)(buffer+blocksize*n);
		blox[n].dwBufferLength=blocksize;
		waveInPrepareHeader(hWi,&blox[n],sizeof(WAVEHDR));
	}

	paused=0;
	Flush();
#ifdef USE_LOG	
	log_write("sampling started OK");
#endif
	return 1;
}

CVis::~CVis()
{
#ifdef USE_LOG
	log_write("shutting down sampling");
#endif
	if (hWi)
	{
		waveInReset(hWi);
		UINT n;
		for(n=0;n<numblocks;n++) 
		{
			waveInUnprepareHeader(hWi,&blox[n],sizeof(WAVEHDR));
		}

#ifndef IN_MIDI_NO_WAVEIN_SOURCE
		src_deinit();
#endif
		waveInClose(hWi);
		hWi=0;
	}
#ifdef USE_LOG
	log_write("sampling shut down OK");
#endif
	if (blox) delete[] blox;
	if (buffer) free(buffer);
}

UINT CVis::ReadData(void * _dst,UINT bytes,bool * ks)
{
	if (eof) return 0;
	BYTE * dst=(BYTE*)_dst;
	if (paused) return 0;
	while(!*ks)
	{
		while(blox[cur_done].dwUser)
		{
			blox[cur_done].dwUser=0;
			cur_done=(cur_done+1)%numblocks;
			in_mm--;
			data+=blocksize;
		}

		{
			UINT d=data;
			if (d)
			{
				if (d>bytes) d=bytes;
				if (read_pos+d>bufsize)
				{
					UINT foo=bufsize-read_pos;
					memcpy(dst,buffer+read_pos,foo);
					memcpy(dst+foo,buffer,read_pos=d-foo);
				}
				else
				{
					memcpy(dst,buffer+read_pos,d);
					read_pos+=d;
				}
				dst+=d;
				data-=d;
				bytes-=d;
			}
		}


		{
			UINT max=numblocks-(data+blocksize-1)/blocksize;
			while(in_mm < max)
			{
				waveInAddBuffer(hWi,&blox[cur_block],sizeof(WAVEHDR));
				cur_block=(cur_block+1)%numblocks;
				in_mm++;
			}
		}
		if (!bytes) break;
		MIDI_callback::Idle();
	}

	return dst-(BYTE*)_dst;
}

void CVis::Pause(int b)
{
	paused=b;
	if (b)
	{
		waveInStop(hWi);
	}
	else
	{
		Flush();
	}
}

CStream * sampling_create(int srate,int nch,int bps)
{
	CVis * ptr = new CVis;
	if (!ptr->init(srate,nch,bps))
	{
		delete ptr;
		ptr=0;
	}
	return ptr;
}