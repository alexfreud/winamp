#include <precomp.h>
#include "corecbi.h"

#define CBCLASS CoreCallbackI
START_DISPATCH;
  CB(CCB_NOTIFY, ccb_notify);
END_DISPATCH;

int CoreCallbackI::ccb_notify(int msg, intptr_t param1, intptr_t param2) {
  ReentryFilter f(&filter, msg);
  if (f.mustLeave()) return 0;
  switch (msg) {
    case STARTED:
      return corecb_onStarted();
    case STOPPED:
      return corecb_onStopped();
    case PAUSED:
      return corecb_onPaused();
    case UNPAUSED:
      return corecb_onUnpaused();
    case SEEKED:
      return corecb_onSeeked(param1);
    case VOLCHANGE:
      return corecb_onVolumeChange(param1);
    case PANCHANGE:
      return corecb_onPanChange(param1);
    case EQSTATUSCHANGE:
      return corecb_onEQStatusChange(param1);
    case EQPREAMPCHANGE:
      return corecb_onEQPreampChange(param1);
    case EQBANDCHANGE:
      return corecb_onEQBandChange(param1, param2);
	case EQFREQCHANGE:
	  return corecb_onEQFreqChange(param1);
    case EQAUTOCHANGE:
      return corecb_onEQAutoChange(param1);
    case STATUSMSG:
      return corecb_onStatusMsg((const wchar_t *)param1);
    case WARNINGMSG:
      return corecb_onWarningMsg((const wchar_t *)param1);
    case ERRORMSG:
      return corecb_onErrorMsg((const wchar_t *)param1);
    case TITLECHANGE:
      return corecb_onTitleChange((const wchar_t *)param1);
    case TITLE2CHANGE:
      return corecb_onTitle2Change((const wchar_t *)param1);
    case INFOCHANGE:
      return corecb_onInfoChange((const wchar_t *)param1);
    case URLCHANGE:
      return corecb_onUrlChange((const wchar_t *)param1);
    case LENGTHCHANGE:
      return corecb_onLengthChange(param1);
    case NEXTFILE:
      return corecb_onNextFile();
    case NEEDNEXTFILE:
      return corecb_onNeedNextFile(param1);
    case SETNEXTFILE:
      return corecb_onSetNextFile((const wchar_t *)param1);
    case ERROROCCURED:
      return corecb_onErrorOccured(param1, (const wchar_t *)param2);
    case ABORTCURRENTSONG:
      return corecb_onAbortCurrentSong();
    case ENDOFDECODE:
      return corecb_onEndOfDecode();
    case ONFILECOMPLETE:
      return corecb_onFileComplete((const wchar_t *)param1);
    case CONVERTERSCHAINREBUILT:
      return corecb_onConvertersChainRebuilt();
    case MEDIAFAMILYCHANGE:
      return corecb_onMediaFamilyChange((const wchar_t *)param1);
    case BITRATECHANGE:
      return corecb_onBitrateChange(param1);
    case SAMPLERATECHANGE:
      return corecb_onSampleRateChange(param1);
    case CHANNELSCHANGE:
      return corecb_onChannelsChange(param1);
  }
  return 0;
}