/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <windows.h>
#include <commctrl.h>
#include <math.h>
#include <stdio.h>
#include "draw.h"
#include "wnd.h"
#include "r_defs.h"
#include "render.h"
#include "vis.h"
#include "cfgwnd.h"
#include "resource.h"
#include "bpm.h"
#include "../Agave/Language/api_language.h"

int refineBeat(int isBeat);
BOOL TCHistStep(BeatType *t, DWORD _Avg, int *_halfDiscriminated, int *_hdPos, DWORD *_lastTC, DWORD TC, int Type);
void InsertHistStep(BeatType *t, DWORD TC, int Type, int i);
void CalcBPM(void);
BOOL ReadyToLearn(void);
BOOL ReadyToGuess(void);
void doubleBeat(void);
void halfBeat(void);
void ResetAdapt(void);
void SliderStep(int Ctl, int *slide);
void initBpm(void);
extern int g_fakeinit;

int cfg_smartbeat=0;
int cfg_smartbeatsticky=1;
int cfg_smartbeatresetnewsong=1;
int cfg_smartbeatonlysticky=0;
int sticked=0;
int arbVal, skipVal;				// Values of arbitrary beat and beat skip
int Bpm, Confidence, Confidence1, Confidence2; // Calculated BPM (realtime), Confidence computation
DWORD lastTC;						// Last beat Tick count
DWORD lastTC2;					// Last beat tick count 2
BeatType TCHist[8];					// History of last 8 beats
BeatType TCHist2[8];					// History of last 8 beats
int Smoother[8];					// History of last 8 Bpm values, used to smooth changes
int halfDiscriminated[8];			// Discriminated beats table
int halfDiscriminated2[8];			// Discriminated beats table
int hdPos;							// Position in discrimination table
int hdPos2;							// Position in discrimination table
int smPtr, smSize;					// Smoother pointer and size
int TCHistPtr;						// Tick count history pointer
int TCHistSize;						// Tick count history size
int offIMax;						// Max divisor/multiplier used to guess/discriminate beats
int lastBPM;						// Last calculated BPM, used by the smoother to detect new entry
int insertionCount;					// Remembers how many beats were guessed
DWORD predictionLastTC;				// Last tick count of guessed/accepted beat
DWORD Avg;							// Average tick count interval between beats
DWORD Avg2;							// Average tick count interval between beats
int skipCount;						// Beat counter used by beat skipper
int inInc, outInc;					// +1/-1, Used by the nifty beatsynced sliders
int inSlide, outSlide;				// Positions of sliders
int oldInSlide, oldOutSlide;		// Used by timer to detect changes in sliders
int oldsticked;						// Used by timer to detect changes in sticked state
char txt[256];						// Contains txt about current BPM and confidence
int halfCount, doubleCount;			// Counter used to autodetect if double/half beat needed
int TCUsed;							// Remembers how many beats in the history were actually used for computation
int predictionBpm;					// Contains BPM actually used to prediction (eliminates Bpm driftings)
int oldDisplayBpm, oldDisplayConfidence; // Detects stuff to redraw
int bestConfidence;					// Best confidence we had so far
char lastSongName[256];				// Last song name, used to detect song changes in winamp
HWND winampWnd;						// Winamp window
int forceNewBeat;					// Force new bpm adoption
int betterConfidenceCount;			// Used to decide when to adpot new beat
int topConfidenceCount;				// Used to decide when to adpot new beat
int stickyConfidenceCount;			// Used to decided when to go sticky
BOOL doResyncBpm=FALSE;

// configuration dialog stuff
BOOL CALLBACK DlgProc_Bpm(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
switch (uMsg)
	{
	case WM_INITDIALOG:
		inInc = 1;
		outInc = 1;
		inSlide = 0;
		outSlide = 0;
		oldDisplayBpm=-1;
		oldDisplayConfidence=-1;
		oldInSlide=-1;
		oldOutSlide=-1;
    if (cfg_smartbeat) CheckDlgButton(hwndDlg,IDC_BPMADV,BST_CHECKED); else CheckDlgButton(hwndDlg,IDC_BPMSTD,BST_CHECKED);
    if (cfg_smartbeatsticky) CheckDlgButton(hwndDlg,IDC_STICKY,BST_CHECKED); 
    if (cfg_smartbeatresetnewsong) CheckDlgButton(hwndDlg,IDC_NEWRESET,BST_CHECKED); else CheckDlgButton(hwndDlg,IDC_NEWADAPT,BST_CHECKED);
    if (cfg_smartbeatonlysticky) CheckDlgButton(hwndDlg,IDC_ONLYSTICKY,BST_CHECKED);
		SendDlgItemMessage(hwndDlg, IDC_IN, TBM_SETTICFREQ, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_IN, TBM_SETRANGE, TRUE, MAKELONG(0, 8));
		SendDlgItemMessage(hwndDlg, IDC_OUT, TBM_SETTICFREQ, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_OUT, TBM_SETRANGE, TRUE, MAKELONG(0, 8));
		if (predictionBpm)
			{
			ShowWindow(GetDlgItem(hwndDlg, IDC_STICK), sticked ? SW_HIDE : SW_NORMAL);
			ShowWindow(GetDlgItem(hwndDlg, IDC_UNSTICK), sticked ? SW_NORMAL : SW_HIDE);
			}
		else
			{
			ShowWindow(GetDlgItem(hwndDlg, IDC_STICK), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_UNSTICK), SW_HIDE);
			}
/*		ShowWindow(GetDlgItem(hwndDlg, IDC_CURBPM), cfg_smartbeat ? SW_NORMAL : SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CURCONF), cfg_smartbeat ? SW_NORMAL : SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_BPM), cfg_smartbeat ? SW_NORMAL : SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CONFIDENCE), cfg_smartbeat ? SW_NORMAL : SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_RESET), cfg_smartbeat ? SW_NORMAL : SW_HIDE);*/
		SetTimer(hwndDlg, 0, 50, NULL);
		return 1;
	case WM_TIMER:
		{
		if (oldInSlide != inSlide) {
			SendDlgItemMessage(hwndDlg, IDC_IN, TBM_SETPOS, TRUE, inSlide); oldInSlide=inSlide; }
		if (oldOutSlide != outSlide) {
			SendDlgItemMessage(hwndDlg, IDC_OUT, TBM_SETPOS, TRUE, outSlide); oldOutSlide=outSlide; }
		if (oldDisplayBpm != predictionBpm || oldsticked != sticked) {
			char lBuf[16];
			wsprintf(txt, predictionBpm ? "%d%s"/*/%d"*/ : WASABI_API_LNGSTRING_BUF(IDS_LEARNING,lBuf,16), predictionBpm, cfg_smartbeatsticky && sticked ? WASABI_API_LNGSTRING(IDS_GOT_IT) : ""/*, Bpm*/);
			SetDlgItemText(hwndDlg, IDC_BPM, txt);
			oldDisplayBpm=predictionBpm;
			oldsticked=sticked;
			if (predictionBpm)
				{
				ShowWindow(GetDlgItem(hwndDlg, IDC_STICK), sticked ? SW_HIDE : SW_NORMAL);
				ShowWindow(GetDlgItem(hwndDlg, IDC_UNSTICK), sticked ? SW_NORMAL : SW_HIDE);
				}
			else
				{
				ShowWindow(GetDlgItem(hwndDlg, IDC_STICK), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_UNSTICK), SW_HIDE);
				}
			}
		if (oldDisplayConfidence != Confidence) {
			wsprintf(txt, "%d%%"/* (%d%%/%d%% - %d)"*/, Confidence/*, Confidence1, Confidence2, TCUsed*/);
			SetDlgItemText(hwndDlg, IDC_CONFIDENCE, txt);
			oldDisplayConfidence=Confidence;
			}
		}
		return 0;
	case WM_COMMAND:
      if ((LOWORD(wParam) == IDC_BPMSTD) ||
		  (LOWORD(wParam) == IDC_BPMADV) ||
		  (LOWORD(wParam) == IDC_NEWRESET) ||
		  (LOWORD(wParam) == IDC_NEWADAPT) ||
		  (LOWORD(wParam) == IDC_ONLYSTICKY) ||
		  (LOWORD(wParam) == IDC_STICKY))
		  {
			cfg_smartbeat=IsDlgButtonChecked(hwndDlg,IDC_BPMADV)?1:0;
			cfg_smartbeatsticky=IsDlgButtonChecked(hwndDlg,IDC_STICKY)?1:0;
			cfg_smartbeatresetnewsong=IsDlgButtonChecked(hwndDlg,IDC_NEWRESET)?1:0;
			cfg_smartbeatonlysticky=IsDlgButtonChecked(hwndDlg,IDC_ONLYSTICKY)?1:0;
			oldsticked=-1;
/*			ShowWindow(GetDlgItem(hwndDlg, IDC_CURBPM), cfg_smartbeat ? SW_NORMAL : SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CURCONF), cfg_smartbeat ? SW_NORMAL : SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BPM), cfg_smartbeat ? SW_NORMAL : SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CONFIDENCE), cfg_smartbeat ? SW_NORMAL : SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_RESET), cfg_smartbeat ? SW_NORMAL : SW_HIDE);*/
		  }
      if (LOWORD(wParam) == IDC_2X)
        doubleBeat();
      if (LOWORD(wParam) == IDC_DIV2)
        halfBeat();
      if (LOWORD(wParam) == IDC_RESET)
        ResetAdapt();
      if (LOWORD(wParam) == IDC_STICK)
		{
        sticked=1;
		stickyConfidenceCount=0;
		}
      if (LOWORD(wParam) == IDC_UNSTICK)
		{
        sticked=0;
		stickyConfidenceCount=0;
		}
	  return 0;
	case WM_DESTROY:
		KillTimer(hwndDlg, 0);
		return 0;
	}
return 0;
}

void initBpm(void)
{
	if (g_fakeinit) return;
  TCUsed=0;
  *txt=0;
  betterConfidenceCount=0;
  topConfidenceCount=0;
  forceNewBeat=0;
  hdPos=0;
  hdPos2=0;
  inSlide=0;
  outSlide=0;
  oldDisplayBpm=-1;
  oldDisplayConfidence=-1;
  oldInSlide=0;
  oldOutSlide=0;
  Bpm = 0;
  Avg = 0;
	Avg2 = 0;
  smPtr = 0;
  smSize = 8;
  offIMax = 8;
  insertionCount = 0;
  predictionLastTC = 0;
  Confidence = 0;
  Confidence1 = 0;
  Confidence2 = 0;
  halfCount=0;
  doubleCount=0;
  TCHistSize = 8;
  predictionBpm=0;
  lastTC=GetTickCount();
  stickyConfidenceCount=0;
  memset(TCHist, 0, TCHistSize*sizeof(BeatType));
  memset(Smoother, 0, smSize*sizeof(int));
  memset(halfDiscriminated, 0, TCHistSize*sizeof(int));
  memset(halfDiscriminated2, 0, TCHistSize*sizeof(int));
  winampWnd = FindWindow("Winamp v1.x", NULL);
  *lastSongName=0;
  sticked=0;
  oldsticked=-1;
}

BOOL songChanged(DWORD TCNow)
{
static DWORD lastCheck=0;
if (TCNow > lastCheck+1000)
	{
	char songName[256];
	lastCheck=TCNow;
	GetWindowText(winampWnd, songName, 255);
	if (strcmp(songName, lastSongName))
		{
		strcpy(lastSongName, songName);
		return TRUE;
		}
	}
return FALSE;
}

void ResetAdapt(void)
{
// Reset adaptive learning
*txt=0;
TCUsed=0;
hdPos=0;
Avg = 0;
Confidence=0;
Confidence1=0;
Confidence2=0;
betterConfidenceCount=0;
topConfidenceCount=0;
Bpm = 0;
smPtr = 0;
smSize = 8;
offIMax = 8;
insertionCount = 0;
predictionLastTC = 0;
halfCount=0;
doubleCount=0;
TCHistSize = 8;
predictionBpm=0;
bestConfidence=0;
lastTC=GetTickCount();
sticked=0;
oldsticked=-1;
stickyConfidenceCount=0;
memset(TCHist, 0, TCHistSize*sizeof(BeatType));
memset(TCHist2, 0, TCHistSize*sizeof(BeatType));
memset(Smoother, 0, smSize*sizeof(int));
memset(halfDiscriminated, 0, TCHistSize*sizeof(int));
}

// Insert a beat in history table. May be either real beat or guessed 
void InsertHistStep(BeatType *t, DWORD TC, int Type, int i)
{
if (i >= TCHistSize) return;
if (t == TCHist && insertionCount < TCHistSize*2) insertionCount++;
memmove(t+i+1, t+i, sizeof(BeatType)*(TCHistSize-(i+1)));
t[0].TC = TC;
t[0].Type = Type;
}

// Doubles current beat
void doubleBeat(void)
{
int i;
int iv[8];

if (sticked && Bpm > MIN_BPM) return;

for (i=0;i<TCHistSize-1;i++)
	iv[i] = TCHist[i].TC - TCHist[i+1].TC;

for (i=1;i<TCHistSize;i++)
	TCHist[i].TC = TCHist[i-1].TC-iv[i-1]/2;

Avg /= 2;
Bpm *= 2;
doubleCount=0;
memset(Smoother, 0, smSize*sizeof(int));
memset(halfDiscriminated, 0, TCHistSize*sizeof(int));
//forceNewBeat=1;
}

// Halfs current beat
void halfBeat(void)
{
int i;
int iv[8];

if (sticked && Bpm < MIN_BPM) return;

for (i=0;i<TCHistSize-1;i++)
	iv[i] = TCHist[i].TC - TCHist[i+1].TC;

for (i=1;i<TCHistSize;i++)
	TCHist[i].TC = TCHist[i-1].TC-iv[i-1]*2;

Avg *= 2;
Bpm /= 2;
halfCount=0;
memset(Smoother, 0, smSize*sizeof(int));
memset(halfDiscriminated, 0, TCHistSize*sizeof(int));
}

// Called whenever isBeat was true in render
BOOL TCHistStep(BeatType *t, DWORD _Avg, int *_halfDiscriminated, int *_hdPos, DWORD *_lastTC, DWORD TC, int Type)
{
int i=0;
int offI;
DWORD thisLen;
BOOL learning = ReadyToLearn();
thisLen = TC-lastTC;

// If this beat is sooner than half the average - 20%, throw it away
if (thisLen < Avg/2 - Avg*0.2)
	{
	if (learning)
		{
		if (labs(Avg - (TC - t[1].TC)) < labs(Avg - (t[0].TC - t[1].TC)))
			{
/*			if (predictionLastTC && t[0].Type == BEAT_GUESSED && Type == BEAT_REAL)
				predictionLastTC += (TC - t[0].TC)/2;*/
			t[0].TC = TC;
			t[0].Type = Type;
			return TRUE;
			}
		}
	return FALSE;
	}

if (learning) 
  for (offI=2;offI<offIMax;offI++) // Try to see if this beat is in the middle of our current Bpm, or maybe 1/3, 1/4 etc... to offIMax
	if ((float)labs((Avg/offI)-thisLen) < (float)(Avg/offI)*0.2)
		{
		_halfDiscriminated[(*_hdPos)++]=1; // Should test if offI==2 before doing that, but seems to have better results ? I'll have to investigate this
		(*_hdPos)%=8;
		return FALSE;
		}

// This beat is accepted, so set this discrimination entry to false
_halfDiscriminated[hdPos++]=0;
(*_hdPos)%=8;

// Check if we missed some beats
/*if (learning && thisLen > 1000 || (float)abs(Avg-thisLen) > (float)Avg*0.3)
  for (offI=2;offI<offIMax;offI++)
	{
	if ((float)abs((Avg*offI)-thisLen) < (float)Avg*0.2)
		{
		for (j=1;j<offI;j++) // Oh yeah we definitly did, add one!
			InsertHistStep(TC - (Avg*j), BEAT_GUESSED, offI-1); // beat has been guessed so report it so confidence can be calculated
		break;
		}
	}*/


// Remember this tick count
*_lastTC = TC;
// Insert this beat.
InsertHistStep(t, TC, Type, 0); 
return TRUE;
}

// Am i ready to learn ?
BOOL ReadyToLearn(void)
{
int i;
for (i=0;i<TCHistSize;i++)
	if (TCHist[i].TC==0) return FALSE;
return TRUE;
}

// Am i ready to guess ?
BOOL ReadyToGuess(void)
{
return insertionCount == TCHistSize*2;
}

void newBpm(int thisBpm)
{
Smoother[smPtr++] = thisBpm; 
smPtr %= smSize;
}

int GetBpm(void)
{
int i;
int smN=0;
int smSum=0;
// Calculate smoothed Bpm
for (i=0;i<smSize;i++) 
	if (Smoother[i] > 0) {
		smSum += Smoother[i];
		smN++;
		}
if (smN) return smSum / smN;
return 0;
}

// Calculate BPM according to beat history
void CalcBPM(void)
{
int i;
int hdCount=0;
int r=0;
int totalTC=0, totalN=0;
float rC, etC;
int v;
double sc=0;
int mx=0;
float et;
int smSum=0, smN=0;

if (!ReadyToLearn())
	return;

// First calculate average beat
for (i=0;i<TCHistSize-1;i++)
	totalTC += TCHist[i].TC - TCHist[i+1].TC;

Avg = totalTC/(TCHistSize-1);

// Count how many of then are real as opposed to guessed
for (i=0;i<TCHistSize;i++)
	if (TCHist[i].Type == BEAT_REAL)
		r++;

// Calculate part 1 of confidence
rC = (float)min((float)((float)r / (float)TCHistSize) * 2, 1);

// Calculate typical drift
for (i=0;i<TCHistSize-1;i++)
	{
	v = TCHist[i].TC - TCHist[i+1].TC;
	mx = max(mx, v);
	sc += v*v;
	}
et = (float)sqrt(sc / (TCHistSize-1) - Avg*Avg);
// Calculate confidence based on typical drift and max derivation
etC = 1 - ((float)et / (float)mx);

// Calculate confidence
Confidence = max(0, (int)(((rC * etC) * 100.0) - 50) * 2);
Confidence1 = (int)(rC * 100);
Confidence2 = (int)(etC * 100);

// Now apply second layer, recalculate average using only beats within range of typical drift
// Also, count how many of them we are keeping
totalTC=0;
for (i=0;i<TCHistSize-1;i++)
	{
	v += TCHist[i].TC - TCHist[i+1].TC;
	if (labs(Avg-v) < et)
		{
		totalTC += v;
		totalN++;
		v = 0;
		}
	else
		if ((float)v > Avg)
			v = 0;
	}
TCUsed = totalN;
// If no beat was within typical drift (how would it be possible? well lets cover our ass) then keep the simple
// average calculated earlier, else recalculate average of beats within range
if (totalN)
	Avg = totalTC/totalN;

if (ReadyToGuess())
	{
	if (Avg) // Avg = 0 ? Ahem..
		Bpm = 60000 / Avg;


	if (Bpm != lastBPM)
		{
		newBpm(Bpm); // If realtime Bpm has changed since last time, then insert it in the smoothing tab;e
		lastBPM = Bpm;

		if (cfg_smartbeatsticky && predictionBpm && Confidence >= ((predictionBpm < 90) ? STICKY_THRESHOLD_LOW : STICKY_THRESHOLD))
			{
			stickyConfidenceCount++;
			if (stickyConfidenceCount >= MIN_STICKY)
				sticked=1;
			}
		else
			stickyConfidenceCount=0;
		}

	Bpm = GetBpm();

	// Count how many beats we discriminated
	for (i=0;i<TCHistSize;i++)
		if (halfDiscriminated[i]) hdCount++;

	if (hdCount >= TCHistSize/2) // If we removed at least half of our beats, then we are off course. We should double our bpm
		{
		if (Bpm * 2 < MAX_BPM) // Lets do so only if the doubled bpm is < MAX_BPM
			{
			doubleBeat();
			memset(halfDiscriminated, 0, TCHistSize*sizeof(int)); // Reset discrimination table
			}
		}
  if (Bpm > 500 || Bpm < 0)
    {
      ResetAdapt();
    }
	if (Bpm < MIN_BPM)
		{ 
		if (++doubleCount > 4) // We're going too slow, lets double our bpm
			doubleBeat();
		}
	else
		doubleCount=0;
	if (Bpm > MAX_BPM) // We're going too fast, lets slow our bpm by a factor of 2
		{
		if (++halfCount > 4)
			halfBeat();
		}
	else
		halfCount=0;
	}
}

void SliderStep(int Ctl, int *slide)
{
*slide += Ctl == IDC_IN ? inInc : outInc;
if (!*slide || *slide == 8) (Ctl == IDC_IN ? inInc : outInc) *= -1;
}

// render function
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].

int refineBeat(int isBeat)
{
  BOOL accepted=FALSE;
  BOOL predicted=FALSE;
  BOOL resyncin=FALSE;
  BOOL resyncout=FALSE;

  if (isBeat) // Show the beat received from AVS
	SliderStep(IDC_IN, &inSlide);

  DWORD TCNow = GetTickCount();

   if (songChanged(TCNow))
	{
	bestConfidence=(int)((float)bestConfidence*0.5);
	sticked=0;
	stickyConfidenceCount=0;
	if (cfg_smartbeatresetnewsong)
		ResetAdapt();
	}

   // Try to predict if this frame should be a beat
   if (Bpm && TCNow > predictionLastTC + (60000 / Bpm))
 	predicted = TRUE;


   if (isBeat) // If it is a real beat, do discrimination/guessing and computations, then see if it is accepted
 		accepted = TCHistStep(TCHist, Avg, halfDiscriminated, &hdPos, &lastTC, TCNow, BEAT_REAL);

   // Calculate current Bpm
   CalcBPM();

   // If prediction Bpm has not yet been set
   // or if prediction bpm is too high or too low
   // or if 3/4 of our history buffer contains beats within the range of typical drift
   // the accept the calculated Bpm as the new prediction Bpm
   // This allows keeping the beat going on when the music fades out, and readapt to the new beat as soon as
   // the music fades in again
   if ((accepted || predicted) && !sticked && (!predictionBpm || predictionBpm > MAX_BPM || predictionBpm < MIN_BPM))
 	{
	if (Confidence >= bestConfidence)
		{
/*		betterConfidenceCount++;
		if (!predictionBpm || betterConfidenceCount == BETTER_CONF_ADOPT)
			{*/
			forceNewBeat=1;
/*			betterConfidenceCount=0;
			}*/
		}
	if (Confidence >= 50)
		{
		topConfidenceCount++;
		if (topConfidenceCount == TOP_CONF_ADOPT)
			{
			forceNewBeat=1;
			topConfidenceCount=0;
			}
		}
	if (forceNewBeat)
		{
		forceNewBeat=0;
		bestConfidence = Confidence;
		predictionBpm=Bpm;
		}
	}

   if (!sticked) predictionBpm = Bpm;
   Bpm=predictionBpm;


/*   resync = (predictionBpm && 
					  (predictionLastTC < TCNow - (30000/predictionBpm) - (60000/predictionBpm)*0.2) ||
					  (predictionLastTC < TCNow - (30000/predictionBpm) - (60000/predictionBpm)*0.2));*/
	if (predictionBpm && accepted && !predicted)
	{
		int b=0;
		if (TCNow > predictionLastTC + (60000 / predictionBpm)*0.7)
		{
			resyncin = TRUE;
			b = (int)((float)predictionBpm * 1.01);
		}
		if (TCNow < predictionLastTC + (60000 / predictionBpm)*0.3)
		{
			resyncout = TRUE;
			b = (int)((float)predictionBpm * 0.98);
		}
		if (!sticked && doResyncBpm && (resyncin || resyncout))
		{
			newBpm(b);
			predictionBpm = GetBpm();
		}
	}

	if (resyncin)
		{
		predictionLastTC = TCNow;
		SliderStep(IDC_OUT, &outSlide);
		doResyncBpm=TRUE;
 		return ((cfg_smartbeat && !cfg_smartbeatonlysticky) || (cfg_smartbeat && cfg_smartbeatonlysticky && sticked)) ? 1 : isBeat;
		}
	if (predicted)
		{
		predictionLastTC = TCNow;
		if (Confidence > 25) TCHistStep(TCHist, Avg, halfDiscriminated, &hdPos, &lastTC, TCNow, BEAT_GUESSED);
		SliderStep(IDC_OUT, &outSlide);
		doResyncBpm=FALSE;
 		return ((cfg_smartbeat && !cfg_smartbeatonlysticky) || (cfg_smartbeat && cfg_smartbeatonlysticky && sticked)) ? 1 : isBeat;
		}
	if (resyncout)
		{
		predictionLastTC = TCNow;
		doResyncBpm=TRUE;
 		return ((cfg_smartbeat && !cfg_smartbeatonlysticky) || (cfg_smartbeat && cfg_smartbeatonlysticky && sticked)) ? 0 : isBeat;
		}

   return ((cfg_smartbeat && !cfg_smartbeatonlysticky) || (cfg_smartbeat && cfg_smartbeatonlysticky && sticked)) ? (predictionBpm ? 0 : isBeat) : isBeat;
}



