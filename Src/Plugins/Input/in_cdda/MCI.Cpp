#include "main.h"

MCIERROR MCISendCommand(MCIDEVICEID IDDevice, UINT uMsg, DWORD fdwCommand, DWORD_PTR dwParam)
{
	MCIERROR nErr = mciSendCommand(IDDevice, uMsg, fdwCommand, dwParam);
#if 0
	if (nErr) 
	{
		wchar_t szError[256] = {0};
		mciGetErrorString(nErr, szError, 256);
		MessageBox(NULL,szError,L"MCI CD Error",MB_OK);
	}
#endif
	return nErr;
}

int isMediaPresent(MCIDEVICEID wDeviceID)
{
	MCI_STATUS_PARMS p;
	p.dwItem=MCI_STATUS_MEDIA_PRESENT;
	if (MCISendCommand(wDeviceID,MCI_STATUS,MCI_STATUS_ITEM,(DWORD_PTR)&p)) return 1;
	return p.dwReturn?1:0;
}

BOOL CDOpen(MCIDEVICEID* lpDeviceID, int device, const wchar_t *alias)
{
	//OutputDebugString("Calling cdopen()\n");
	MCI_OPEN_PARMS sMCIOpen;
	MCI_SET_PARMS sMCISet;
	wchar_t zDevice[3]={(wchar_t)device,':',0};
	DWORD nErr;

	sMCIOpen.dwCallback = 0;
	sMCIOpen.wDeviceID  = 0;
	sMCIOpen.lpstrAlias  = alias;
	sMCIOpen.lpstrDeviceType = (LPCTSTR) MCI_DEVTYPE_CD_AUDIO;
	sMCIOpen.lpstrElementName = zDevice;
	nErr = MCISendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE |
						  MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT |
						  (alias ? MCI_OPEN_ALIAS : 0), (DWORD_PTR) &sMCIOpen);
	if (nErr) 
	{
		nErr = MCISendCommand(0, MCI_OPEN, MCI_OPEN_TYPE |
							  MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT |
							  (alias ? MCI_OPEN_ALIAS : 0), (DWORD_PTR) &sMCIOpen);
		if (nErr) 
		{
			return FALSE;
		}
	}
    
	sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
	MCISendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR) &sMCISet);
	
	*lpDeviceID = sMCIOpen.wDeviceID;

	return TRUE;
}

void CDClose(MCIDEVICEID* lpDeviceID)
{
	MCI_GENERIC_PARMS sMCIGeneric;

	sMCIGeneric.dwCallback = (DWORD_PTR) line.hMainWindow;
	MCISendCommand(*lpDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD_PTR) &sMCIGeneric);

	*lpDeviceID = 0;
}

void CDEject(MCIDEVICEID wDeviceID)
{
	MCI_SET_PARMS mciset;
	MCISendCommand (wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN,(DWORD_PTR) &mciset);
}

unsigned int CDGetTracks(MCIDEVICEID wDeviceID)
{
	MCI_STATUS_PARMS sMCIStatus;
	sMCIStatus.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	if (MCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT,(DWORD_PTR) &sMCIStatus)) return -1;
	return sMCIStatus.dwReturn;
}

unsigned int CDGetCurrTrack(MCIDEVICEID wDeviceID)
{
	MCI_STATUS_PARMS sMCIStatus;

	sMCIStatus.dwItem = MCI_STATUS_POSITION;
	MCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT,(DWORD_PTR) &sMCIStatus);

	return ((int) MCI_TMSF_TRACK (sMCIStatus.dwReturn));
}

int CDPlay(MCIDEVICEID wDeviceID, unsigned int nTrack, BOOL bResume, unsigned int nMin, unsigned int nSec, unsigned int endms)
{
	MCI_PLAY_PARMS sMCIPlay;
	unsigned int nActualTrack = nTrack;

	sMCIPlay.dwFrom = MCI_MAKE_TMSF (nActualTrack, nMin, nSec, 0);
	sMCIPlay.dwTo = MCI_MAKE_TMSF (nActualTrack, endms/60000, (endms/1000)%60,0);
	if (!bResume) 
	{
		return MCISendCommand (wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO ,(DWORD_PTR) &sMCIPlay);
	}
	else 
	{
		return MCISendCommand (wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD_PTR) (LPVOID) &sMCIPlay);
	}
}

void CDStop(MCIDEVICEID wDeviceID)
{
	MCISendCommand(wDeviceID, MCI_STOP, 0, 0);
}

void CDPause(MCIDEVICEID wDeviceID)
{
	MCISendCommand(wDeviceID, MCI_PAUSE, 0,0);
}

unsigned int CDGetTrackLength(MCIDEVICEID wDeviceID, unsigned int nTrack)
{
	MCI_STATUS_PARMS sMCIStatus;
	int r;

	sMCIStatus.dwItem = MCI_STATUS_POSITION ;
	sMCIStatus.dwTrack = nTrack+1;
	if (MCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT,(DWORD_PTR) &sMCIStatus)) // if error than last track
	{
		sMCIStatus.dwItem = MCI_STATUS_LENGTH;
		sMCIStatus.dwTrack = nTrack;
		if (MCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT,(DWORD_PTR) &sMCIStatus)) return -1000;
		return sMCIStatus.dwReturn;
	}

	r=sMCIStatus.dwReturn;
	sMCIStatus.dwItem = MCI_STATUS_POSITION ;
	sMCIStatus.dwTrack = nTrack;
	if (MCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT,(DWORD_PTR) &sMCIStatus)) return -1000;
	return r-sMCIStatus.dwReturn;
}

BOOL CDID(MCIDEVICEID wDeviceID, wchar_t *id, size_t len)
{
	MCI_INFO_PARMS sMCIInfo;
	sMCIInfo.dwCallback=0;
	sMCIInfo.dwRetSize=len;
	sMCIInfo.lpstrReturn=id;
	DWORD nErr= MCISendCommand(wDeviceID, MCI_INFO, MCI_INFO_MEDIA_IDENTITY, (DWORD_PTR)&sMCIInfo);

	return !nErr;
}