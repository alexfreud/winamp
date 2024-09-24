#ifndef __C_SCRIPTOBJ_H
#define __C_SCRIPTOBJ_H

#include <api/script/scriptobj.h>

class C_RootObject{

  public:

    C_RootObject(ScriptObject *o);
    C_RootObject();
    virtual ~C_RootObject();

    virtual void C_hook(ScriptObject *o);

    virtual const wchar_t *getClassName();
    virtual void notify(const wchar_t *a, const wchar_t *b, int c, int d);
    virtual ScriptObject *getScriptObject();

    operator ScriptObject *() { return getScriptObject(); }

  private:

    ScriptObject *obj;  
    static int getclassname_id;
    static int notify_id;
    static int inited;
    static int count;
};



#endif
