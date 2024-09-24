#ifndef _CORECB_H
#define _CORECB_H

#include <bfc/dispatch.h>

// don't derive from this
class NOVTABLE CoreCallback : public Dispatchable 
{
protected:
  CoreCallback() {}

public:
  int ccb_notify(int msg, intptr_t param1=0, intptr_t param2=0) {
    return _call(CCB_NOTIFY, 0, msg, param1, param2);
  }

  // class Dispatchable codes
  enum {
    CCB_NOTIFY = 100,
  };

  // various ccb_notify notifications. these are *not* the Dispatchable codes
  enum {
    REGISTER        = 100,
    DEREGISTER      = 200,
    NEXTFILE        = 300,

    STARTED         = 500,
    STOPPED         = 600,
    PAUSED          = 700,
    UNPAUSED        = 800,
    SEEKED          = 900,

    VOLCHANGE       = 2000,
    EQSTATUSCHANGE  = 2100,
    EQPREAMPCHANGE  = 2200,
    EQBANDCHANGE    = 2300,
		EQFREQCHANGE    = 2310,
    EQAUTOCHANGE    = 2400,
    PANCHANGE       = 2500,
    
    STATUSMSG       = 3000,
    WARNINGMSG      = 3100,
    ERRORMSG        = 3200,
    ERROROCCURED    = 3300,

    TITLECHANGE     = 4000,
    TITLE2CHANGE    = 4100,
    INFOCHANGE      = 4200,
    SAMPLERATECHANGE = 4210,
    BITRATECHANGE   = 4220,
    CHANNELSCHANGE  = 4230,
    URLCHANGE       = 4300, 
    LENGTHCHANGE    = 4400,

    NEEDNEXTFILE    = 5100,
    SETNEXTFILE     = 5200,

    ABORTCURRENTSONG= 6000,
    
    ENDOFDECODE     = 7000,

    ONFILECOMPLETE     = 8000,

    CONVERTERSCHAINREBUILT = 9000,

    MEDIAFAMILYCHANGE = 10000,
  };
};

#endif
