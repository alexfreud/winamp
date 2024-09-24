#ifndef __HOOK_SCRIPTOBJECT_H
#define __HOOK_SCRIPTOBJECT_H

#include <api/script/objects/c_script/scripthook.h>

class H_RootObject : public ScriptHookI {

  public:
    
    H_RootObject(ScriptObject *o);
    H_RootObject();
    virtual ~H_RootObject();

    virtual void H_hook(ScriptObject *o);

    virtual int eventCallback(ScriptObject *object, int dlfid, scriptVar **params, int nparams);

    virtual void hook_onNotify(const wchar_t *s, const wchar_t *t, int u, int v) {}

    virtual ScriptObject *getHookedObject() { return me; }

  private:

    ScriptObject *me;
    static int onnotify_id;
    static int inited;
    static int count;
};

#endif