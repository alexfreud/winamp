#ifndef __WASABI_CORECBI_H
#define __WASABI_CORECBI_H

#include <bfc/wasabi_std.h>
#include <api/syscb/callbacks/corecb.h>
#include <bfc/reentryfilter.h>

// this class does NOT automatically deregister itself when you destruct it
// that is up to you, and obviously, if you fail to do so, you'll crash the
// whole app. and they'll all laugh at you. see class CoreHandle
class NOVTABLE CoreCallbackI : public CoreCallback {
protected:
  CoreCallbackI() { } // no instantiating this on its own

public:
  virtual int corecb_onStarted() { return 0; }
  virtual int corecb_onStopped() { return 0; }
  virtual int corecb_onPaused() { return 0; }
  virtual int corecb_onUnpaused() { return 0; }
  virtual int corecb_onSeeked(int newpos) { return 0; }

  virtual int corecb_onVolumeChange(int newvol) { return 0; }
  virtual int corecb_onPanChange(int newpan) { return 0; }

  virtual int corecb_onEQStatusChange(int newval) { return 0; }
  virtual int corecb_onEQPreampChange(int newval) { return 0; }
  virtual int corecb_onEQBandChange(int band, int newval) { return 0; }
	virtual int corecb_onEQFreqChange(int newval) { return 0; }
  virtual int corecb_onEQAutoChange(int newval) { return 0; }

  virtual int corecb_onStatusMsg(const wchar_t *text) { return 0; }
  virtual int corecb_onWarningMsg(const wchar_t *text) { return 0; }
  virtual int corecb_onErrorMsg(const wchar_t *text) { return 0; }

  virtual int corecb_onTitleChange(const wchar_t *title) { return 0; }
  virtual int corecb_onTitle2Change(const wchar_t *title2) { return 0; }
  virtual int corecb_onInfoChange(const wchar_t *info) { return 0; }
  virtual int corecb_onBitrateChange(int kbps) { return 0; }
  virtual int corecb_onSampleRateChange(int khz) { return 0; }
  virtual int corecb_onChannelsChange(int nch) { return 0; }
  virtual int corecb_onUrlChange(const wchar_t *url) { return 0; }
  virtual int corecb_onLengthChange(int newlength) { return 0; }

  virtual int corecb_onNextFile() { return 0; }
  virtual int corecb_onNeedNextFile(int fileid) { return 0; }
  virtual int corecb_onSetNextFile(const wchar_t *playstring) { return 0; }

  virtual int corecb_onErrorOccured(int severity, const wchar_t *text) { return 0; }

  // return 1 in this callback to make the current callback chain to abort
  virtual int corecb_onAbortCurrentSong() { return 0; }

  virtual int corecb_onEndOfDecode() { return 0; }

  virtual int corecb_onFileComplete(const wchar_t *playstring) { return 0; }

  virtual int corecb_onConvertersChainRebuilt() { return 0; }

  virtual int corecb_onMediaFamilyChange(const wchar_t *newfamily) { return 0; }

  virtual int ccb_notify(int msg, intptr_t param1=0, intptr_t param2=0);
protected:
  RECVS_DISPATCH;
  ReentryFilterObject filter;
};

#endif