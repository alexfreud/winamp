#include <precomp.h>

#include "svc_mediacore.h"

#define CBCLASS svc_mediaCoreI
START_DISPATCH;
  VCB(SETCALLBACK,setCallback);
  VCB(SETNEXTFILE,setNextFile);
  VCB(START,start);
  VCB(PAUSE,pause);
  VCB(SETPOSITION,setPosition);
  VCB(SETVOLUME,setVolume);
  VCB(SETPAN,setPan);
  VCB(ABORTCURRENTSONG,abortCurrentSong);
  VCB(STOP,stop);
  CB(GETPLAYING,getPlaying);
  CB(GETPOSITION,getPosition);
  CB(GETWRITEPOSITION,getWritePosition);
  CB(GETTITLE,getTitle);
  VCB(GETINFO,getInfo);
  CB(GETLENGTH,getLength);
  CB(GETVOLUME,getVolume);
  CB(GETPAN,getPan);
  VCB(SETEQSTATUS,setEQStatus);
  CB(GETEQSTATUS,getEQStatus);
  VCB(SETEQPREAMP,setEQPreamp);
  CB(GETEQPREAMP,getEQPreamp);
  VCB(SETEQBAND,setEQBand);
  CB(GETEQBAND,getEQBand);
  VCB(SETEQBANDS,setEQBands);
  VCB(GETEQBANDS,getEQBands);
  VCB(SETEQ,setEQ);
  VCB(GETEQ,getEQ);
  CB(GETMETADATA,getMetaData);
  CB(GETVISDATA,getVisData);
  VCB(MUTE,mute);
  CB(ISMUTED,isMuted);
  VCB(SETCORETOKEN,setCoreToken);
  VCB(SETPRIORITY,setPriority);
  CB(GETPRIORITY,getPriority);
  VCB(REBUILDCONVERTERSCHAIN, rebuildConvertersChain)
  CB(SENDCONVERTERSMSG, sendConvertersMsg)
END_DISPATCH;
#undef CBCLASS

