#include <precomp.h>

#include "svc_coreadmin.h"

#define CBCLASS svc_coreAdminI
START_DISPATCH;
  CB(CREATECORE,createCore)
  CB(NAMETOTOKEN,nameToToken)
  CB(FREECOREBYTOKEN,freeCoreByToken)
  CB(FREECOREBYNAME,freeCoreByName)
  CB(VERIFYTOKEN,verifyToken)
  
  CB(GETSUPPORTEDEXTENSIONS,getSupportedExtensions)
  CB(GETEXTSUPPORTEDEXTENSIONS,getExtSupportedExtensions)
  VCB(REGISTEREXTENSION,registerExtension)
  CB(GETEXTENSIONFAMILY, getExtensionFamily);
  VCB(UNREGISTEREXTENSION,unregisterExtension)
  
  CB(SETNEXTFILE,setNextFile)
  CB(SETNEXTFILEOLD,setNextFile)

  CB(GETSTATUS,getStatus)
  CB(GETCURRENT,getCurrent)
  CB(GETCURPLAYBACKNUMBER,getCurPlaybackNumber)
  CB(GETNUMTRACKS, getNumTracks);
  CB(GETPOSITION,getPosition)
  CB(GETWRITEPOSITION,getWritePosition)
  CB(GETLENGTH,getLength)
  CB(GETPLUGINDATA,getPluginData)
  CB(GETVOLUME,getVolume)
  CB(GETPAN,getPan)
  CB(GETVISDATA,getVisData)
  CB(GETLEFTVUMETER,getLeftVuMeter)
  CB(GETRIGHTVUMETER,getRightVuMeter)
  CB(GETEQSTATUS,getEqStatus)
  CB(GETEQPREAMP,getEqPreamp)
  CB(GETEQBAND,getEqBand)
  CB(GETEQAUTO,getEqAuto)
  CB(GETMUTE,getMute)
    
  CB(SETPOSITION,setPosition)
  VCB(SETVOLUME,setVolume)
  VCB(SETPAN,setPan)
  VCB(SETEQSTATUS,setEqStatus)
  VCB(SETEQPREAMP,setEqPreamp)
  VCB(SETEQBAND,setEqBand)
  VCB(SETEQAUTO,setEqAuto)
  VCB(SETMUTE,setMute)
  CB(GETTITLE,getTitle);

  VCB(ADDCALLBACK,addCallback)
  VCB(DELCALLBACK,delCallback)

  CB(REGISTERSEQUENCER,registerSequencer)
  CB(DEREGISTERSEQUENCER,deregisterSequencer)
  CB(GETSEQUENCER,getSequencer)
    
  VCB(USERBUTTON,userButton)
  
  VCB(SETCUSTOMMSG, setCustomMsg)
  
  VCB(SETPRIORITY, setPriority)
  CB(GETPRIORITY, getPriority)

  VCB(REBUILDCONVERTERSCHAIN, rebuildConvertersChain)
  CB(SENDCONVERTERSMSG, sendConvertersMsg)

END_DISPATCH;
#undef CBCLASS

