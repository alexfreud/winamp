#ifndef NULLSOFT_OUT_DS_VOLCTRL_H
#define NULLSOFT_OUT_DS_VOLCTRL_H

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

class DsVolCtrl
{
public:
	DsVolCtrl(int VolMode, double LogVolMin, bool logfades);
	void SetFade(__int64 duration, double destvol, double destpan);
	inline void SetFadeVol(__int64 duration, double destvol) {SetFade(duration, destvol, DestPanHack);}
	inline void SetFadePan(__int64 duration, double destpan) {SetFade(duration, DestVolHack, destpan);}
	__int64 RelFade(__int64 max, double destvol);
	void SetTime(__int64 time);
	void SetVolume(double vol);
	void SetPan(double pan);
	void Apply(IDirectSoundBuffer * pDSB);
	//		inline double GetCurVol() {return CurVol;}
	inline double GetDestVol() { return DestVolHack;}
	inline void Reset() {CurTime = 0;FadeDstTime = -1;}
	double Stat_GetVolLeft();
	double Stat_GetVolRight();

	bool Fading();

private:
	bool IsFading;
	int VolMode;
	double LogVolMin;

	double FadeSrcVol, FadeDstVol, FadeSrcPan, FadeDstPan;
	__int64 FadeSrcTime, FadeDstTime;

	__int64 CurTime;

	double CurVol, CurPan, LastVol, LastPan;
	double DestVolHack, DestPanHack;
	bool LogFades;
	void MapVol(double Vol, double Pan, double &NewVol, double &NewPan);
};

#endif
