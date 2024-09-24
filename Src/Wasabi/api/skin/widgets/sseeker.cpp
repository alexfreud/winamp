#include <precomp.h>

#include "sseeker.h"
#include <api/script/scriptmgr.h>
#include <api/core/api_core.h>

#define SSeeker_TIMER_POS 1
#define SSEEKER_INTERVAL 500

const wchar_t seekBarXuiStr[] = L"SeekBar"; // This is the xml tag
char seekBarXuiSvcName[] = "SeekBar xui object"; // this is the name of the xuiservice

XMLParamPair SSeeker::params[] = {
	{SSEEKER_SETINTERVAL, L"INTERVAL"},
};
SSeeker::SSeeker() {
	setDrawOnBorders(TRUE);
  status = STOP;
  update_interval = SSEEKER_INTERVAL;
  locked = 0;
  xuihandle = newXuiHandle();
  CreateXMLParameters(xuihandle);

  setLimits(0, 65535);
}

void SSeeker::CreateXMLParameters(int master_handle)
{
	//SSEEKER_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

SSeeker::~SSeeker() {
  killTimer(SSeeker_TIMER_POS);
  WASABI_API_MEDIACORE->core_delCallback(0, this);
}

int SSeeker::setXuiParam(int _xuihandle, int attribid, const wchar_t *name, const wchar_t *strval) {
  if (xuihandle != _xuihandle) return SSEEKER_PARENT::setXuiParam(_xuihandle, attribid, name, strval);
  switch (attribid) {
    case SSEEKER_SETINTERVAL:
      if ((update_interval = WTOI(strval)) <= 20)
        update_interval = SSEEKER_INTERVAL;
      break;
    default:
      return 0;
  }
  return 1;
}

int SSeeker::onInit() {
  SSEEKER_PARENT::onInit();

  timerCallback(SSeeker_TIMER_POS);
  setTimer(SSeeker_TIMER_POS, update_interval);

  WASABI_API_MEDIACORE->core_addCallback(0, this);

  return 1;
}

int SSeeker::onSetFinalPosition() {
  SSEEKER_PARENT::onSetFinalPosition();
  if (WASABI_API_MEDIACORE->core_getPosition(0) == -1) return 1;
  int pos = getSliderPosition();	// get slider pos
  int len = WASABI_API_MEDIACORE->core_getLength(0);
  int corepos = (int)(((double)pos * (double)len) / 65535.f);
	WASABI_API_MEDIACORE->core_setPosition(0,corepos);
  return 1;
}

void SSeeker::timerCallback(int id) {
  switch (id) {
    case SSeeker_TIMER_POS: {
      if (getSeekStatus()) return;

	    int playpos = WASABI_API_MEDIACORE->core_getPosition(0);
      int len = WASABI_API_MEDIACORE->core_getLength(0);
      if (playpos < 0 || len <= 0) {
        setVisible(0);
        status=STOP;
        return;
      } 
      int newpos = (int)(((double)playpos / (double)len) * 65535.f);
      if (getSliderPosition() != newpos) {
        setPosition(newpos, 0);
        onPostedPosition(newpos / scriptDivisor());
      }
      if (len > 0 && !isVisible()) {
        status=PLAY;
        setVisible(1);
      }
    }
    break;
  default:
    SSEEKER_PARENT::timerCallback(id);
  }
}

int SSeeker::corecb_onStarted() {
  timerCallback(SSeeker_TIMER_POS);
  return 0;
}

int SSeeker::corecb_onStopped() {
  timerCallback(SSeeker_TIMER_POS);
  return 0;
}

int SSeeker::corecb_onSeeked(int newpos) {
  timerCallback(SSeeker_TIMER_POS);
  return 0;
}

void SSeeker::lock() {
  if (locked) return;
  locked = 1;
  WASABI_API_MEDIACORE->core_delCallback(0, this);
}

void SSeeker::unlock() {
  if (!locked) return;
  locked = 0;
  WASABI_API_MEDIACORE->core_addCallback(0, this);
}