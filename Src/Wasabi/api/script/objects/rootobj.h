#ifndef __ROOTOBJ_H
#define __ROOTOBJ_H

#include <bfc/dispatch.h>
#include <api/script/objects/rootobjcbi.h>
#include <api/script/scriptobji.h>

class ScriptObject;
class ScriptObjectI;
class RootObject;

// RootObjectInstance::this == RootObjectInstance::getScriptObject()->vcpu_getInterface(rootObjectInstanceGuid);
// {F6D49468-4036-41a1-9683-C372416AD31B}
static const GUID rootObjectInstanceGuid = 
{ 0xf6d49468, 0x4036, 0x41a1, { 0x96, 0x83, 0xc3, 0x72, 0x41, 0x6a, 0xd3, 0x1b } };

// Instantiate this class to create an object from which you can trap notify events, or inherit from
// it if you want to implement your own descendant of script class 'Object' (see GuiObjectWnd)
class RootObjectInstance : public RootObjectCallbackI 
{
public:

  RootObjectInstance();
  virtual ~RootObjectInstance();

  virtual RootObject *getRootObject();
  virtual ScriptObject *getScriptObject();

  virtual void rootobjectcb_onNotify(const wchar_t *a, const wchar_t *b, int c, int d) {};
  virtual const wchar_t *getClassName();
  virtual void notify(const wchar_t *s, const wchar_t *t, int u, int v);

 private:
	void rootobject_init();
  ScriptObjectI my_script_object;
  RootObject * my_root_object;
};


#endif
