#ifndef NULLSOFT_MCIPLAYH
#define NULLSOFT_MCIPLAYH

#include "CDPlay.h"
#include "main.h"

class MciPlay : public C_CDPlay {
public:
	MciPlay()
	{
		posinms=0;
	}

	int play(wchar_t drive, int track)
	{
		g_drive = drive;

		{
			MCI_SET_PARMS sMCISet;
			g_playtrack = track;

			if (!CDOpen(&playdev, drive, L"mci"))
			{
				playdev=0;
				g_drive = 0;
				return 1;
			}
    		sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
			MCISendCommand (playdev, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR) (LPVOID) &sMCISet);
			g_playlength=CDGetTrackLength(playdev,g_playtrack); 
			sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
  			MCISendCommand (playdev, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR) (LPVOID) &sMCISet);
			if (CDPlay(playdev,g_playtrack,FALSE,0,0,g_playlength))
			{
				CDClose(&playdev);
				playdev=0;
				g_drive = 0;
				return 1;
			}
  			line.is_seekable=1;
		}
		line.SetInfo(1411,44,2,1);
		line.SAVSAInit(0,44100);
		line.VSASetInfo(2,44100);
		setvolume(a_v, a_p);
		{
			short dta[576*2] = {0};
			line.VSAAddPCMData(dta,2,16,0);
			line.SAAddPCMData(dta,2,16,0);	
		}
		if (audioInit(/*config_sample*/))
		{
		}
		return 0;
	}

	void stop()
	{
		//CDStop(playdev);
		if (!done) CDPause(playdev);
		CDClose(&playdev);
  		audioQuit();
	}

	void pause()
	{
		posinms=audioGetPos();
		CDPause(playdev);
		audioPause(1);
	}

	void unpause()
	{
	    int pos=audioGetPos();
		CDPlay(playdev, g_playtrack, TRUE,pos/60000,(pos/1000)%60,g_playlength);
		audioPause(0); 
	}

	int getlength()
	{
		return g_playlength;
	}

	int getoutputtime()
	{
		int p;
		if (paused)
		{
			return posinms;
		}
		p=audioGetPos();
		if (GetCurrentThreadId()==MainThreadId)
		{
			if ((p > g_playlength || (p-g_lastpos > 2000 && !isMediaPresent(playdev))) && !done)
			{
	  			done=1;
	  			PostMessage(line.hMainWindow,WM_WA_MPEG_EOF,0,0);
			}
			if (g_lastpos > p+2000)
				g_lastpos=p;
		}
		return p;
	}

	void setoutputtime(int time_in_ms)
	{
		CDPlay(playdev,g_playtrack,FALSE,time_in_ms/60000,(time_in_ms/1000)%60,g_playlength);
		if (paused)
		{
			audioPause(0);
		}
		audioSetPos(time_in_ms);
		g_lastpos=time_in_ms;
		if (paused)
		{
			audioPause(1);
			CDPause(playdev);
		}
		done=0;
	}

	void setvolume(int _a_v, int _a_p) { 
		HMIXER hmix;
		int vol1, vol2;
		if (_a_v < 0) _a_v = 0;
		if (_a_v > 255) _a_v = 255;
		if (_a_p < -127) _a_p = -127;
		if (_a_p > 127) _a_p = 127;
		vol1 = vol2 = (_a_v*2048) / 255;
		if (_a_p > 0)
		{
			vol1 *= (127-_a_p);
			vol1 /= 127;
		}
		else if (_a_p < 0)
		{
			vol2 *= (_a_p+127);
			vol2 /= 127;
		}

		if (mixerOpen(&hmix,0,0,0,0) == MMSYSERR_NOERROR)
		{
			MIXERLINE ml={sizeof(ml),0};
			ml.dwComponentType=MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;
			if (mixerGetLineInfo((HMIXEROBJ)hmix,&ml,MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
			{
				MIXERLINECONTROLS mlc = {sizeof(mlc),ml.dwLineID,};
				MIXERCONTROL mc={sizeof(mc),};
				mlc.cControls=1;
				mlc.cbmxctrl=sizeof(mc);
				mlc.pamxctrl=&mc;
				mlc.dwControlType=MIXERCONTROL_CONTROLTYPE_VOLUME;
				if (mixerGetLineControls((HMIXEROBJ)hmix,&mlc,MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR)
				{
					MIXERCONTROLDETAILS mcd = {sizeof(mcd),mc.dwControlID,ml.cChannels,};
					MIXERCONTROLDETAILS_UNSIGNED v[2];
					mcd.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
					mcd.paDetails=v;

					v[0].dwValue=mc.Bounds.dwMinimum + (vol1*(mc.Bounds.dwMaximum-mc.Bounds.dwMinimum))/2048;
					v[1].dwValue=mc.Bounds.dwMinimum + (vol2*(mc.Bounds.dwMaximum-mc.Bounds.dwMinimum))/2048;

					if (mixerSetControlDetails((HMIXEROBJ)hmix,&mcd,0) == MMSYSERR_NOERROR)
					{
						// yay we're done!	 
					}
					//  else MessageBox(NULL,"mixerSetLineControlDetails()","Error",MB_OK);
				}
				//else MessageBox(NULL,"mixerGetLineControls()","Error",MB_OK);
			}
//			else MessageBox(NULL,"mixerGetLineInfo()","Error",MB_OK);
			mixerClose(hmix);
		}
		//else MessageBox(NULL,"MixerOpen()","Error",MB_OK);
	}

private:
	int posinms;
	MCIDEVICEID playdev;
};

#endif