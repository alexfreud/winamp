#ifndef __ROOTOBJECTI_IMPL_H
#define __ROOTOBJECTI_IMPL_H

/*<?<autoheader/>*/
#include "rootobject.h"
#include "rootobjectx.h"

class RootObjectCallback;
class ScriptObject;
/*?>*/

/*[interface.header.h]
#include "common/script/rootobjcontroller.h"
*/

extern RootScriptObjectController *rootScriptObjectController;

class RootObjectI : public RootObjectX {
public:
  RootObjectI(ScriptObject *o);
  virtual ~RootObjectI();
  DISPATCH(10) virtual const wchar_t *rootobject_getClassName();
  DISPATCH(20) virtual void rootobject_notify(const wchar_t *s, const wchar_t *t, int u, int v);
  DISPATCH(30) virtual ScriptObject *rootobject_getScriptObject();
  DISPATCH(40) virtual void rootobject_setScriptObject(ScriptObject *obj);
  DISPATCH(50) virtual void rootobject_addCB(RootObjectCallback *cb);
  PtrList < RootObjectCallback > cbs;
  ScriptObject * my_script_object;
};


#endif // __ROOTOBJECTI_IMPL_H
