#ifndef _SSEEKER_H
#define _SSEEKER_H

#include "pslider.h"
#include <api/syscb/callbacks/corecbi.h>

#define SSEEKER_PARENT PSliderWnd
#define SSEEKER_XMLPARENT PSliderWnd

#define STOP 0
#define PLAY 1

class SSeeker : public SSEEKER_PARENT, public CoreCallbackI {
public:
  SSeeker();
  virtual ~SSeeker();

  virtual int onInit();

  virtual int setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *val);
  virtual void lock();
  virtual void unlock();

  virtual int scriptDivisor() { return 256; }

  enum {
    SSEEKER_SETINTERVAL=0,
  };

protected:
	/*static */void CreateXMLParameters(int master_handle);
  virtual int onSetFinalPosition();

  // from BaseWnd
  virtual void timerCallback(int id);

  // from CoreCallbackI
  virtual int corecb_onSeeked(int newpos);
  virtual int corecb_onStarted();
  virtual int corecb_onStopped();

  int status;

private:
	static XMLParamPair params[];
  int update_interval;
  int locked;
  int xuihandle;
};

extern const wchar_t seekBarXuiStr[];
extern char seekBarXuiSvcName[];
class SeekBarXuiSvc : public XuiObjectSvc<SSeeker, seekBarXuiStr, seekBarXuiSvcName> {};


#endif
