#include "main.h"
#include "fakedsound.h"

//DirectMusic output capture hack.

class FakeDirectSoundBuffer : public IDirectSoundBuffer
{
private:
	ULONG ref;
	CPipe* out;
	UINT freq;
	BYTE * buf;
	UINT buf_size;

	bool playing;
	DWORD pos_play;

	DWORD samples_played;
	DWORD start;

	void do_update();

public:
	~FakeDirectSoundBuffer()
	{
		if (buf) free(buf);
	};

    HRESULT _stdcall QueryInterface(REFIID iid, void** i)
	{
		if (IsEqualIID(iid,IID_IUnknown) || IsEqualIID(iid,IID_IDirectSoundBuffer))
		{
			ref++;
			*i = this;
			return S_OK;
		}
		else return E_NOINTERFACE;
	}

    ULONG _stdcall AddRef() {return ++ref;};
    ULONG _stdcall Release()
	{
		UINT r=--ref;
		if (!r)
		{
			delete this;
		}
		return r;
	}
    HRESULT _stdcall GetCaps(LPDSBCAPS _caps)
	{
		DSBCAPS caps=
		{
			sizeof(DSBCAPS),
			DSBCAPS_GLOBALFOCUS|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_LOCSOFTWARE,
			buf_size,
			0,0	//CPU crap
		};
		*_caps = caps;
		return S_OK;
	}
	HRESULT _stdcall Initialize(LPDIRECTSOUND, LPCDSBUFFERDESC) {return DSERR_ALREADYINITIALIZED;}
	HRESULT _stdcall SetFormat(LPCWAVEFORMATEX) {return DSERR_INVALIDCALL;}
	HRESULT _stdcall GetFormat(LPWAVEFORMATEX wfx,DWORD,LPDWORD w)
	{
		wfx->wFormatTag=WAVE_FORMAT_PCM;
		wfx->nChannels=2;
		wfx->nSamplesPerSec=freq;
		wfx->nAvgBytesPerSec=4*freq;
		wfx->nBlockAlign=4;
		wfx->wBitsPerSample=16;
		wfx->cbSize=0;
		if (w) *w=sizeof(WAVEFORMATEX);
		return S_OK;
	}

	HRESULT _stdcall GetVolume(long* v) {return S_OK;}
	HRESULT _stdcall SetVolume(long v) {return S_OK;}
	HRESULT _stdcall GetPan(long *p) {return S_OK;}
	HRESULT _stdcall SetPan(long p) {return S_OK;}
	HRESULT _stdcall GetFrequency(DWORD* f) {*f=freq;return S_OK;}
	HRESULT _stdcall SetFrequency(DWORD f) {return S_OK;}
	HRESULT _stdcall GetStatus(DWORD* s)
	{
		*s = DSBSTATUS_PLAYING|DSBSTATUS_LOOPING;
		return S_OK;
	}
	HRESULT _stdcall SetCurrentPosition(DWORD) {return S_OK;}
    HRESULT _stdcall Restore() {return S_OK;}

	HRESULT _stdcall Lock(DWORD wr_cur, DWORD wr_b, void** p1, DWORD* s1, void** p2, DWORD* s2, DWORD flagz)
	{
		if (wr_b>buf_size)
		{
			return DSERR_INVALIDPARAM;
		}
		*p1 = buf + wr_cur;
		if (wr_cur + wr_b > buf_size)
		{
			*s1 = buf_size - wr_cur;
			*p2 = buf;
			*s2 = wr_cur+wr_b - buf_size;
		}
		else
		{
			*s1 = wr_b;
			*p2 = 0;
			*s2 = 0;
		}
		return S_OK;
	}


	HRESULT _stdcall GetCurrentPosition(LPDWORD p, LPDWORD w)
	{
		do_update();
		if (p) *p=pos_play;
		if (w) *w=pos_play;
		return S_OK;
	}

	HRESULT _stdcall Play(DWORD, DWORD, DWORD)
	{
		playing=1;
		pos_play=0;
		samples_played=0;
		start=timeGetTime();
		return S_OK;
	}

    HRESULT _stdcall Stop() {do_update();playing=0;return S_OK;}
    HRESULT _stdcall Unlock(LPVOID, DWORD, LPVOID, DWORD)
	{
		do_update();
		return S_OK;
	}

	FakeDirectSoundBuffer(UINT _freq,UINT size)

	{
		ref=1;
		buf_size=size;
		buf=(BYTE*)malloc(size);
		memset(buf,0,size);
		freq=_freq;
		out=new CPipe(4,freq);
		MIDI_core::player_setSource(out);
		playing=0;
		pos_play=0;
		samples_played=0;
	}
};





void FakeDirectSoundBuffer::do_update()
{
	if (playing)
	{
		int ds=MulDiv(timeGetTime()-start,freq,1000)-samples_played;

		if (ds>0)
		{
			UINT todo=ds*4;
			while(pos_play+todo>buf_size)
			{
				out->WriteData(buf+pos_play,buf_size-pos_play);
				todo-=buf_size-pos_play;
				pos_play=0;
			}
			if (todo)
			{
				out->WriteData(buf+pos_play,todo);
				pos_play+=todo;
				//todo=0;
			}
			samples_played+=ds;
		}
	}
}

IDirectSoundBuffer* dhb_create(DWORD s,DWORD f)
{
	return new FakeDirectSoundBuffer(f,s);
}


//fake IDirectSound crap. one static instance

static DSCAPS h_caps=
	{
		sizeof(DSCAPS),
		DSCAPS_SECONDARY16BIT|DSCAPS_SECONDARYSTEREO,
		1000,
		100000,
		1,
		1000,
		1000,
		1000,//streaming buffers
		1000,
		1000,
		1000,
		0,0,0,0,0,0,//3d crap
		1024*1024,
		1024*1024,
		1024*1024,
		0,0, //CPU speed crap
		0,0 //reserved crap
	};

class FakeDsound : public IDirectSound
{
	ULONG ref:1;
	HRESULT _stdcall QueryInterface(REFIID iid,void** i)
	{
		if (IsEqualIID(iid,IID_IUnknown) || IsEqualIID(iid,IID_IDirectSound))
		{
			ref++;
			*i = this;
			return S_OK;
		}
		else return E_NOINTERFACE;
	}
	ULONG _stdcall AddRef() {return ++ref;}
	ULONG _stdcall Release() {return --ref;}
	HRESULT _stdcall CreateSoundBuffer(LPCDSBUFFERDESC, LPDIRECTSOUNDBUFFER *, LPUNKNOWN) {return DSERR_INVALIDCALL;}
	HRESULT _stdcall GetCaps(LPDSCAPS _caps)
	{
		*_caps = h_caps;
		return S_OK;
	}
	HRESULT _stdcall DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER, LPDIRECTSOUNDBUFFER *) {return DSERR_INVALIDCALL;}
	HRESULT _stdcall SetCooperativeLevel(HWND, DWORD) {return S_OK;}
	HRESULT _stdcall Compact() {return S_OK;}
	HRESULT _stdcall GetSpeakerConfig(LPDWORD moo) {*moo=0;return S_OK;}
	HRESULT _stdcall SetSpeakerConfig(DWORD) {return S_OK;}
	HRESULT _stdcall Initialize(LPCGUID) {return DSERR_ALREADYINITIALIZED;}
};

static FakeDsound HACK;

IDirectSound * get_ds() {return &HACK;}