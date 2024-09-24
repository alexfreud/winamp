#ifndef __GUIVIDEOSVC_H
#define __GUIVIDEOSVC_H

#include <api/service/svcs/svc_scriptobji.h>

class CoreScriptObjectSvc : public svc_scriptObjectI {

public:
  CoreScriptObjectSvc() {};
  virtual ~CoreScriptObjectSvc() {};

  static const char *getServiceName() { return "MediaCore maki object"; }
  virtual ScriptObjectController *getController(int n);
}; 
      
class CoreAdminScriptObjectSvc : public svc_scriptObjectI {

public:
  CoreAdminScriptObjectSvc() {};
  virtual ~CoreAdminScriptObjectSvc() {};

  static const char *getServiceName() { return "MediaCoreAdmin maki object"; }
  virtual ScriptObjectController *getController(int n);
};       

#endif

