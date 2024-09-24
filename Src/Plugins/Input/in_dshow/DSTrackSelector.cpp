#include "DSTrackSelector.h"
#include "main.h"
#include "CWAAudioRenderer.h"
#include "../Agave/Language/api_language.h"
#include "resource.h"

int DSTrackSelector::getNumAudioTracks()
{
	if (nullfilter)
		return nullfilter->GetConnectedInputsCount();
	return 1;
}

void DSTrackSelector::enumAudioTrackName(int n, char *buf, int size)
{
	char t[256] = {0};
	StringCchPrintfA(t, 256, WASABI_API_LNGSTRING(IDS_TRACK_X), n);
	int l = min(size, 255);
	lstrcpynA((char *)buf, t, l);
}

int DSTrackSelector::getCurAudioTrack()
{
	if (nullfilter)
		return nullfilter->GetSelectedInput();
	return 0;
}

int DSTrackSelector::getNumVideoTracks()
{
	return 1;
}

void DSTrackSelector::enumVideoTrackName(int n, char *buf, int size)
{
	WASABI_API_LNGSTRING_BUF(IDS_TRACK_1,buf,min(7, size));
}

int DSTrackSelector::getCurVideoTrack()
{
	return 0;
}

void DSTrackSelector::setAudioTrack(int n)
{
	if (nullfilter)
	{
		CComPtr<IMediaPosition> pMediaPosition = NULL;
		pGraphBuilder->QueryInterface(IID_IMediaPosition, (void **)&pMediaPosition);
		pMediaControl->Pause();
		REFTIME pos;
		pMediaPosition->get_CurrentPosition(&pos);
		nullfilter->SetSelectedInput(n);
		{
			CMediaType *mt = nullfilter->GetAcceptedType();
			if (mt->subtype != MEDIASUBTYPE_PCM)
				has_audio = NULL;
			else
			{
				WAVEFORMATEX *pHeader = (WAVEFORMATEX*)mt->pbFormat;
				// reget this cause this is the real UNCOMPRESSED format
				audio_bps = pHeader->wBitsPerSample;
				audio_srate = pHeader->nSamplesPerSec;
				audio_nch = pHeader->nChannels;
				if (audio_bps == 32 || audio_bps == 64)
				{
					m_float = 1;
					m_src_bps = audio_bps;
					audio_bps = 16; //TODO: read bits from AGAVE_API_CONFIG :)
				}
			}
			mod.outMod->Close();
			int maxlat = mod.outMod->Open(audio_srate, audio_nch, audio_bps, -1, -1);
			if (maxlat < 0)
			{
				releaseObjects();
				return ;
			}
			mod.SetInfo(m_bitrate, audio_srate / 1000, audio_nch, 1);
			mod.SAVSAInit(maxlat, audio_srate);
			mod.VSASetInfo(audio_srate, audio_nch);
			mod.outMod->SetVolume( -666);
		}
		pMediaPosition->put_CurrentPosition(pos);
		pMediaControl->Run();
	}
}

void DSTrackSelector::setVideoTrack(int n){}