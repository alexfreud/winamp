#ifndef __C_WAVEIN_H__
#define __C_WAVEIN_H__

#include <windows.h>
#include <mmsystem.h>
#define EXIT_ON_ERROR(hr)  \
	if (FAILED(hr)) { goto Exit; }
#define SAFE_RELEASE(what)  \
	if ((what) != NULL)  \
{ (what)->Release(); (what) = NULL; }

template<int numbuffers, int buffersize> class C_WAVEIN {
private:
	short Samples[numbuffers][buffersize];
	WAVEFORMATEX wfx;
	WAVEHDR wvhdr[numbuffers];
	HWAVEIN hwi;
	WAVEINCAPS wic;
	unsigned long iNumDevs, iy;
	HRESULT hr;
	IMMDeviceEnumerator *pEnumerate;
	IMMDevice *pDevice;
	IMMDeviceCollection *ppDevices;
	IPropertyStore *pProps;
	BOOL useXpSound;
	PROPVARIANT varName;
	char buf[1024];
public:
	C_WAVEIN() {
		hwi = NULL;
		memset(Samples, 0, sizeof(Samples));
		memset(wvhdr, 0, sizeof(wvhdr));
		iNumDevs = iy = 0;
		hr = S_OK;
		pEnumerate = NULL;
		pDevice = NULL;
		ppDevices = NULL;
		pProps = NULL;
		useXpSound = false;
		memset(buf, 0, sizeof(buf));
	}

	virtual ~C_WAVEIN() {
		Close();
	}

	char * getDeviceName(unsigned int devid=-1) {
		hr = S_OK;
		pEnumerate = NULL;
		pDevice = NULL;
		ppDevices = NULL;
		pProps = NULL;
		useXpSound = false;
		PROPVARIANT varName;
		PropVariantInit(&varName);
		// Get enumerator for audio endpoint devices.
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
							  NULL, CLSCTX_INPROC_SERVER,
							  __uuidof(IMMDeviceEnumerator),
							  (void**)&pEnumerate);
		EXIT_ON_ERROR(hr)

		hr = pEnumerate->GetDefaultAudioEndpoint(eCapture,eConsole,&pDevice);
		EXIT_ON_ERROR(hr)
Exit:
		if (FAILED(hr)) {
			useXpSound = true; 
		} else
			useXpSound = false;

		memset(buf, 0, sizeof(buf));
		if (useXpSound) {
			if (!waveInGetDevCaps(devid, &wic, sizeof(WAVEINCAPS))) {
				lstrcpyn(buf, wic.szPname, ARRAYSIZE(buf));
				goto Fin;
			}
		} else {
			pDevice->OpenPropertyStore(STGM_READ, &pProps);
			pProps->GetValue(PKEY_Device_FriendlyName, &varName);
			WideCharToMultiByte(CP_ACP, 0, (LPWSTR)varName.pwszVal, -1, buf, ARRAYSIZE(buf), NULL, NULL);
            goto Fin;
		}
Fin:
		PropVariantClear(&varName);
		SAFE_RELEASE(pProps)
		SAFE_RELEASE(pEnumerate)
		SAFE_RELEASE(pDevice)
		SAFE_RELEASE(ppDevices)
		CoUninitialize();
		return buf;
	}

	void Create(int sRate, int nCh,int devid=-1) {
		if (hwi == NULL) {
			wfx.wFormatTag = WAVE_FORMAT_PCM;
			wfx.wBitsPerSample = 16;
			wfx.nSamplesPerSec = sRate;
			wfx.nChannels = (WORD)nCh;
			wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
			wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
			wfx.cbSize = 0;
			waveInOpen(&hwi,devid,&wfx,0,0,CALLBACK_NULL);
			waveInStop(hwi);
			waveInReset(hwi);
			for(int i = 0; i < numbuffers; i++) {
				memset(&wvhdr[i],0,sizeof(wvhdr[i]));
				wvhdr[i].lpData = (char *)&Samples[i];
				wvhdr[i].dwBufferLength = buffersize * sizeof(short);
				waveInPrepareHeader(hwi,&wvhdr[i],sizeof(WAVEHDR));
				waveInAddBuffer(hwi,&wvhdr[i],sizeof(WAVEHDR));
			}
			waveInStart(hwi);
		}
	}

	void Close() {
		if (hwi != NULL) {
			waveInStop(hwi);
			waveInReset(hwi);
			for(int i = 0; i < numbuffers; i++) {
				if (wvhdr[i].dwFlags & WHDR_PREPARED) {
					waveInUnprepareHeader(hwi,&wvhdr[i],sizeof(WAVEHDR));
				}
			}
			waveInClose(hwi);
			hwi = NULL;
		}
	}

	short *operator[](int buffernum) {
		return (short *)&Samples[buffernum];
	}

	int getNumSamples(int buffernum) {
		return wvhdr[buffernum].dwBytesRecorded / (wfx.nChannels * sizeof(short));
	}

	int isOpen() {
		return hwi != NULL;
	}

	int isFilled(int buffernum) {
		return wvhdr[buffernum].dwFlags & WHDR_DONE && wvhdr[buffernum].dwBytesRecorded <= buffersize * sizeof(short);
	}

	void cycleBuffer(int buffernum) {
		if (hwi != NULL) {
			wvhdr[buffernum].dwFlags = WHDR_PREPARED;
			wvhdr[buffernum].dwBytesRecorded = 0;
			waveInAddBuffer(hwi,&wvhdr[buffernum],sizeof(WAVEHDR));
		}
	}
};

#endif // !__C_WAVEIN_H__