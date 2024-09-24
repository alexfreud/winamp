#ifndef _CNV_DS2_H
#define _CNV_DS2_H

#include "../studio/wac.h"
#include "../common/rootcomp.h"
#include "../attribs/cfgitemi.h"
#include "../attribs/attrint.h"
#include "../attribs/attrbool.h"
#include "../attribs/attrstr.h"


#include "../studio/services/svc_mediaconverter.h"
#include "../studio/services/servicei.h"
#include "../studio/corecb.h"
#include "../studio/wac.h"


#define WACNAME WACcnv_waveout

class WACNAME : public WAComponentClient {
public:
  WACNAME();
  virtual ~WACNAME();

  virtual const char *getName() { return "WaveOut Output"; };
  virtual GUID getGUID();

  virtual void onDestroy();
  virtual void onCreate();
  
  virtual int getDisplayComponent() { return FALSE; };

  virtual CfgItem *getCfgInterface(int n) { return this; }
};


#endif
