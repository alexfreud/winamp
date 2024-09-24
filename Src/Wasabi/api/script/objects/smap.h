#ifndef __SMAP_H
#define __SMAP_H

class SMap;
class SRegion;

#include <tataki/bitmap/bitmap.h>
#include <api/script/script.h>
#ifdef WASABI_WIDGETS_GUIOBJECT
#include <api/script/objects/guiobj.h>
#endif
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>

// {38603665-461B-42a7-AA75-D83F6667BF73}
static const GUID mapGuid = 
{ 0x38603665, 0x461b, 0x42a7, { 0xaa, 0x75, 0xd8, 0x3f, 0x66, 0x67, 0xbf, 0x73 } };

#define SMAP_SCRIPTPARENT RootObjectInstance

class MapScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController();
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern MapScriptController *mapController;

class SMap : public SMAP_SCRIPTPARENT {
public:
  SMap();
  virtual ~SMap();

  int getValue(int x, int y);
  int getARGBValue(int x, int y, int whichCol);
  int inRegion(int x, int y);
  int getWidth();
  int getHeight();
  void loadMap(const wchar_t *b);
  static void instantiate(SMap *s);
  virtual SkinBitmap *getBitmap() { return bmp; };
  SRegion *getSRegion();

private:
	SkinBitmap *bmp;
	SRegion *reg;
  ScriptObject *region_so;

public:
  static scriptVar script_vcpu_loadMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar bitmap);
  static scriptVar script_vcpu_getValue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_getARGBValue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y, scriptVar whichCol);
  static scriptVar script_vcpu_getWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_inRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_getRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

};

#endif
