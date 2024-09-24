#include <precomp.h>
#include "api_core.h"

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS api_coreI
START_DISPATCH;
  CB(API_CORE_GETSUPPORTEDEXTENSIONS, core_getSupportedExtensions);
  CB(API_CORE_GETEXTSUPPORTEDEXTENSIONS, core_getExtSupportedExtensions);
  CB(API_CORE_CREATE, core_create);
  CB(API_CORE_FREE, core_free);
  CB(API_CORE_SETNEXTFILE, core_setNextFile);
  CB(API_CORE_GETSTATUS, core_getStatus);
  CB(API_CORE_GETCURRENT, core_getCurrent);
  CB(API_CORE_GETCURPLAYBACKNUMBER, core_getCurPlaybackNumber);
  CB(API_CORE_GETPOSITION, core_getPosition);
  CB(API_CORE_GETWRITEPOSITION, core_getWritePosition);
  CB(API_CORE_SETPOSITION, core_setPosition);
  CB(API_CORE_GETLENGTH, core_getLength);
  CB(API_CORE_GETPLUGINDATA, core_getPluginData);
  CB(API_CORE_GETVOLUME, core_getVolume);
  VCB(API_CORE_SETVOLUME, core_setVolume);
  CB(API_CORE_GETPAN, core_getPan);
  VCB(API_CORE_SETPAN, core_setPan);
  VCB(API_CORE_ADDCALLBACK, core_addCallback);
  VCB(API_CORE_DELCALLBACK, core_delCallback);
  CB(API_CORE_GETVISDATA, core_getVisData);
  CB(API_CORE_GETLEFTVUMETER, core_getLeftVuMeter);
  CB(API_CORE_GETRIGHTVUMETER, core_getRightVuMeter);
  CB(API_CORE_REGISTERSEQUENCER, core_registerSequencer);
  CB(API_CORE_DEREGISTERSEQUENCER, core_deregisterSequencer);
  VCB(API_CORE_USERBUTTON, core_userButton);
  CB(API_CORE_GETEQSTATUS, core_getEqStatus);
  VCB(API_CORE_SETEQSTATUS, core_setEqStatus);
  CB(API_CORE_GETEQPREAMP, core_getEqPreamp);
  VCB(API_CORE_SETEQPREAMP, core_setEqPreamp);
  CB(API_CORE_GETEQBAND, core_getEqBand);
  VCB(API_CORE_SETEQBAND, core_setEqBand);
  CB(API_CORE_GETEQAUTO, core_getEqAuto);
  VCB(API_CORE_SETEQAUTO, core_setEqAuto);
  VCB(API_CORE_SETCUSTOMMSG, core_setCustomMsg);
  VCB(API_CORE_REGISTEREXTENSION, core_registerExtension);
  CB(API_CORE_GETEXTENSIONFAMILY, core_getExtensionFamily);
  VCB(API_CORE_UNREGISTEREXTENSION, core_unregisterExtension);
  CB(API_CORE_GETTITLE, core_getTitle);
  CB(API_CORE_GETRATING, core_getRating);
  VCB(API_CORE_SETRATING, core_setRating);
  CB(API_CORE_GETDECODERNAME, core_getDecoderName);
  VCB(API_CORE_SETTITLE, core_setTitle);
END_DISPATCH;