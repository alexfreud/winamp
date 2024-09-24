#ifndef _SVC_COREADMIN_H              
#define _SVC_COREADMIN_H

#include <bfc/dispatch.h>
#include <api/core/corehandle.h>
#include <api/core/sequence.h>
#include <api/service/services.h>

// There is only ONE INSTANCE of the coreadmin running in the application
class NOVTABLE svc_coreAdmin : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::COREADMIN; }

  // create a new playback core
  CoreToken createCore(const char *name=NULL) { return _call(CREATECORE,0,name); }

  // (CoreToken)0 is the maincore 
  // "main" is the maincore
  CoreToken nameToToken(const char *name) { return _call(NAMETOTOKEN,0,name); }

  int freeCoreByToken(CoreToken core) { return _call(FREECOREBYTOKEN,0,core); }
  int freeCoreByName(const char *name) { return _call(FREECOREBYNAME,0,name); }

  // returns 1 if present, 0 if non existent
  int verifyToken(CoreToken token) { return _call(VERIFYTOKEN,0,token); }

  //just the *.mp3 or whatever
  const char *getSupportedExtensions() { return _call(GETSUPPORTEDEXTENSIONS,(const char *)0); } 
  // including names
  const char *getExtSupportedExtensions() { return _call(GETEXTSUPPORTEDEXTENSIONS,(const char *)0); } 
  void registerExtension(const char *extensions, const char *extension_name, const char *family=NULL) { _voidcall(REGISTEREXTENSION,extensions,extension_name,family); } 
  const char *getExtensionFamily(const char *extension) {
    return _call(GETEXTENSIONFAMILY, (const char *)0, extension);
  }
  void unregisterExtension(const char *extensions) { _voidcall(UNREGISTEREXTENSION,extensions); } 

  int setNextFile(CoreToken core, const char *playstring, const char *destination=NULL) { return _call(SETNEXTFILE,0,core,playstring,destination); } 
  // returns -1 if paused, 0 if stopped and 1 if playing
  int getStatus(CoreToken core) { return _call(GETSTATUS,0,core); } 
  const char *getCurrent(CoreToken core) { return _call(GETCURRENT,(const char *)0,core); } 
  int getCurPlaybackNumber(CoreToken core) { return _call(GETCURPLAYBACKNUMBER,-1,core); } 
  int getNumTracks(CoreToken core) { return _call(GETNUMTRACKS, -1, core); }
  int getPosition(CoreToken core) { return _call(GETPOSITION,0,core); } 
  int getWritePosition(CoreToken core) { return _call(GETWRITEPOSITION,0,core); } 
  int setPosition(CoreToken core, int ms) { return _call(SETPOSITION,0,core,ms); } 
  int getLength(CoreToken core) { return _call(GETLENGTH,-1,core); } 
  // this method queries the core plugins directly, bypassing the db
  // returns size of data
  int getPluginData(const char *playstring, const char *name,
    char *data, int data_len, int data_type=0) { return _call(GETPLUGINDATA,0,playstring,name,data,data_len,data_type); } 
  unsigned int getVolume(CoreToken core) { return _call(GETVOLUME,0,core); } 
  // 0..255
  void setVolume(CoreToken core, unsigned int vol) { _voidcall(SETVOLUME,core,vol); } 
  // -127..127
  int getPan(CoreToken core) { return _call(GETPAN,0,core); } 
  // -127..127
  void setPan(CoreToken core, int bal) { _voidcall(SETPAN,core,bal); } 
  
  void setMute(CoreToken core, int mute) { _voidcall(SETMUTE,core,mute); }
  int getMute(CoreToken core) { return _call(GETMUTE,0,core); }
  
  // register here for general callbacks in core status.
  void addCallback(CoreToken core, CoreCallback *cb) { _voidcall(ADDCALLBACK,core,cb); } 
  void delCallback(CoreToken core, CoreCallback *cb) { _voidcall(DELCALLBACK,core,cb); } 
  // get visualization data, returns 0 if you should blank out
  int getVisData(CoreToken core, void *dataptr, int sizedataptr) { return _call(GETVISDATA,0,core,dataptr,sizedataptr); } 
  int getLeftVuMeter(CoreToken core) { return _call(GETLEFTVUMETER,0,core); } 
  int getRightVuMeter(CoreToken core) { return _call(GETRIGHTVUMETER,0,core); } 
  
  int registerSequencer(CoreToken core, ItemSequencer *seq) { return _call(REGISTERSEQUENCER,0,core,seq); } 
  int deregisterSequencer(CoreToken core, ItemSequencer *seq) { return _call(DEREGISTERSEQUENCER,0,core,seq); } 
  ItemSequencer *getSequencer(CoreToken core) { return _call(GETSEQUENCER, (ItemSequencer*)NULL,core); } 
  // see buttons.h
  void userButton(CoreToken core, int button) { _voidcall(USERBUTTON,core,button); } 

  // returns 1 if on, 0 if off
  int getEqStatus(CoreToken core) { return _call(GETEQSTATUS,0,core); } 
  void setEqStatus(CoreToken core, int enable) { _voidcall(SETEQSTATUS,core,enable); } 
  // -127 to 127 (-20db to +20db)
  int getEqPreamp(CoreToken core) { return _call(GETEQPREAMP,0,core); } 
  void setEqPreamp(CoreToken core, int pre) { _voidcall(SETEQPREAMP,core,pre); } 
  // band=0-9
  int getEqBand(CoreToken core, int band) { return _call(GETEQBAND,0,core,band); } 
  void setEqBand(CoreToken core, int band, int val) { _voidcall(SETEQBAND,core,band,val); } 
  // returns 1 if on, 0 if off
  int getEqAuto(CoreToken core) { return _call(GETEQAUTO,0,core); } 
  void setEqAuto(CoreToken core, int enable) { _voidcall(SETEQAUTO,core,enable); } 

  // for status msgs
  void setCustomMsg(CoreToken core, const char *text) { _voidcall(SETCUSTOMMSG,core,text); }
  
  void setPriority(CoreToken core, int priority) { _voidcall(SETPRIORITY,core,priority); }
  int getPriority(CoreToken core) { return _call(GETPRIORITY,0,core); }

  void rebuildConvertersChain(CoreToken core) { _voidcall(REBUILDCONVERTERSCHAIN,core); }

  int sendConvertersMsg(CoreToken core, const char *msg, const char *value) { return _call(SENDCONVERTERSMSG,0,core,msg,value); }
  const char *getTitle(CoreToken core) { return _call(GETTITLE,(const char *)NULL, core); }

  enum {
    CREATECORE=10,
    NAMETOTOKEN=20,
    FREECOREBYTOKEN=30,
    FREECOREBYNAME=40,
    VERIFYTOKEN=50,

    GETSUPPORTEDEXTENSIONS=100,
    GETEXTSUPPORTEDEXTENSIONS=110,
    REGISTEREXTENSION=121,	//120 retired
    GETEXTENSIONFAMILY=122,
    UNREGISTEREXTENSION=130,

    SETNEXTFILEOLD=200,
    SETNEXTFILE=210,

    GETSTATUS=300,
    GETCURRENT=310,
    GETCURPLAYBACKNUMBER=315,
    GETNUMTRACKS=317,
    GETPOSITION=320,
    GETWRITEPOSITION=330,
    GETLENGTH=340,
    GETPLUGINDATA=350,
    GETVOLUME=360,
    GETPAN=370,
    GETVISDATA=380,
    GETLEFTVUMETER=390,
    GETRIGHTVUMETER=400,
    GETEQSTATUS=410,
    GETEQPREAMP=420,
    GETEQBAND=430,
    GETEQAUTO=440,
    GETMUTE=450,
    
    SETPOSITION=500,
    SETVOLUME=510,
    SETPAN=520,
    SETEQSTATUS=530,
    SETEQPREAMP=540,
    SETEQBAND=550,
    SETEQAUTO=560,
    SETMUTE=570,

    ADDCALLBACK=700,
    DELCALLBACK=710,

    REGISTERSEQUENCER=800,
    DEREGISTERSEQUENCER=810,
    GETSEQUENCER=812,
    
    USERBUTTON=820,

    SETCUSTOMMSG=900,

    SETPRIORITY=1000,
    GETPRIORITY=1100,

    REBUILDCONVERTERSCHAIN=1200,

    SENDCONVERTERSMSG=1300,
    GETTITLE=1400,
  };
};

class NOVTABLE svc_coreAdminI : public svc_coreAdmin {
public:
  virtual CoreToken createCore(const char *name=NULL)=0;
  virtual CoreToken nameToToken(const char *name)=0;
  virtual int freeCoreByToken(CoreToken core)=0;
  virtual int freeCoreByName(const char *name)=0;

  virtual int verifyToken(CoreToken token)=0;

  virtual const char *getSupportedExtensions()=0;
  virtual const char *getExtSupportedExtensions()=0;
  virtual void registerExtension(const char *extensions, const char *extension_name, const char *family=NULL)=0;
  virtual const char *getExtensionFamily(const char *extension)=0;
  virtual void unregisterExtension(const char *extensions)=0;

  virtual int setNextFile(CoreToken core, const char *playstring, const char *destination)=0;
  virtual int getStatus(CoreToken core)=0;
  virtual const char *getCurrent(CoreToken core)=0;
  virtual int getCurPlaybackNumber(CoreToken core)=0;
  virtual int getNumTracks(CoreToken core)=0;
  virtual int getPosition(CoreToken core)=0;
  virtual int getWritePosition(CoreToken core)=0;
  virtual int setPosition(CoreToken core, int ms)=0;
  virtual int getLength(CoreToken core)=0;
  virtual int getPluginData(const char *playstring, const char *name, char *data, int data_len, int data_type=0)=0;
  virtual unsigned int getVolume(CoreToken core)=0;
  virtual void setVolume(CoreToken core, unsigned int vol)=0;
  virtual int getPan(CoreToken core)=0;
  virtual void setPan(CoreToken core, int bal)=0;
  virtual void addCallback(CoreToken core, CoreCallback *cb)=0;
  virtual void delCallback(CoreToken core, CoreCallback *cb)=0;
  virtual int getVisData(CoreToken core, void *dataptr, int sizedataptr)=0;
  virtual int getLeftVuMeter(CoreToken core)=0;
  virtual int getRightVuMeter(CoreToken core)=0;
  virtual void setMute(CoreToken core, int mute)=0;
  virtual int getMute(CoreToken core)=0;

  virtual int registerSequencer(CoreToken core, ItemSequencer *seq)=0;
  virtual int deregisterSequencer(CoreToken core, ItemSequencer *seq)=0;
  virtual ItemSequencer *getSequencer(CoreToken core)=0;
  virtual void userButton(CoreToken core, int button)=0;

  virtual int getEqStatus(CoreToken core)=0;
  virtual void setEqStatus(CoreToken core, int enable)=0;
  virtual int getEqPreamp(CoreToken core)=0;
  virtual void setEqPreamp(CoreToken core, int pre)=0;
  virtual int getEqBand(CoreToken core, int band)=0;
  virtual void setEqBand(CoreToken core, int band, int val)=0;
  virtual int getEqAuto(CoreToken core)=0;
  virtual void setEqAuto(CoreToken core, int enable)=0;

  virtual void setCustomMsg(CoreToken core, const char *text)=0;

  virtual void setPriority(CoreToken core, int priority)=0;
  virtual int getPriority(CoreToken core)=0;

  virtual void rebuildConvertersChain(CoreToken core)=0;

  virtual int sendConvertersMsg(CoreToken core, const char *msg, const char *value)=0;
  virtual const char *getTitle(CoreToken core)=0;

protected:
  RECVS_DISPATCH;
};

#endif
