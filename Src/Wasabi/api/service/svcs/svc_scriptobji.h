#pragma once
#include "svc_scriptobji.h"
// derive from this one
class svc_scriptObjectI : public svc_scriptObject {
public:
  virtual ScriptObjectController *getController(int n)=0;
  virtual void onRegisterClasses(ScriptObjectController *rootController) {};

protected:
  RECVS_DISPATCH;
};

template <class T>
class ScriptObjectControllerCreator : public svc_scriptObjectI {
public:
  static const char *getServiceName() { return "ScriptObjectControllerCreator"; }

  ScriptObjectControllerCreator() 
	{
  }

  virtual ~ScriptObjectControllerCreator() 
	{
  }

  virtual ScriptObjectController *getController(int n) 
	{
    if (n == 0) return &single_controller;
    return NULL;
  }

private:
  T single_controller;
};

#include <api/service/servicei.h>
template <class T>
class ScriptObjectCreator : public waServiceFactoryTSingle<svc_scriptObject, T> {};

#include <api/service/svc_enum.h>

class ExternalScriptObjectEnum : public SvcEnumT<svc_scriptObject> {
public:
  ExternalScriptObjectEnum() { }

protected:
  virtual int testService(svc_scriptObject*svc) {
    return (svc->getController(0) != NULL);
  }
};

