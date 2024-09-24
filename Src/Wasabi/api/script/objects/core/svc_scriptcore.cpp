#include <precomp.h>
#include "svc_scriptcore.h"
#include <api/script/objects/core/coreobj.h>
#include <api/script/objects/core/coreadminobj.h>

#ifndef _WASABIRUNTIME

BEGIN_SERVICES(ScriptCore_Svc);
DECLARE_SERVICETSINGLE(svc_scriptObject, CoreScriptObjectSvc);
END_SERVICES(ScriptCore_Svc, _ScriptCore_Svc);

#ifdef _X86_
extern "C" { int _link_ScriptCore_Svc; }
#else
extern "C" { int __link_ScriptCore_Svc; }
#endif

#endif

// -----------------------------------------------------------------------------------------------------
// Service

ScriptObjectController *CoreScriptObjectSvc::getController(int n) {
  switch (n) {
    case 0:
      return coreController;
  }
  return NULL;
}

ScriptObjectController *CoreAdminScriptObjectSvc::getController(int n) {
  switch (n) {
    case 0:
      return coreAdminController;
  }
  return NULL;
}

