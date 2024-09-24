#pragma once
#include <api/service/svcs/svc_scriptobj.h>

class ScriptObjectService : public svc_scriptObject
{
public:
	ScriptObjectController *getController(int n);
  void onRegisterClasses(ScriptObjectController *rootController);
protected:
	RECVS_DISPATCH;
};
