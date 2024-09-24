#ifndef __SREGION_H
#define __SREGION_H

class SRegion;

#ifndef _NOSTUDIO

#include <tataki/region/region.h>
#include <api/script/objects/smap.h>

#endif

#include <api/script/script.h>
#include <api/script/scriptobj.h>

// {3A370C02-3CBF-439f-84F1-86885BCF1E36}
static const GUID regionGuid = 
{ 0x3a370c02, 0x3cbf, 0x439f, { 0x84, 0xf1, 0x86, 0x88, 0x5b, 0xcf, 0x1e, 0x36 } };

#define SREGION_SCRIPTPARENT RootObjectInstance

class RegionScriptController : public ScriptObjectControllerI {
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

extern RegionScriptController *regionController;


#ifndef _NOSTUDIO

class SRegion : public SREGION_SCRIPTPARENT {
public:
  SRegion();
  virtual ~SRegion();

  int inRegion(int x, int y);
  void loadFromMap(SMap *map, int byte, int inv);
  void loadFromBitmap(const wchar_t *p);
  int getBoundX();
  int getBoundY();
  int getBoundW();
  int getBoundH();
  api_region *getRegion();
  void addRegion(SRegion *s);
  void subRegion(SRegion *s);
  void offset(int x, int y);
  void stretch(double s);
  void copy(SRegion *s);


private:
	RegionI *reg;

#else	
class SRegion : SREGION_SCRIPTPARENT {
#endif

// FG>
// -- SCRIPT -----------------------------------------------------
public:
  static scriptVar script_vcpu_loadFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv);
  static scriptVar script_vcpu_loadFromBitmap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map);
  static scriptVar script_vcpu_inRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_add(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r);
  static scriptVar script_vcpu_sub(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r);
  static scriptVar script_vcpu_offset(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_stretch(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s);
  static scriptVar script_vcpu_copy(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r);
  static scriptVar script_vcpu_getBoundX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getBoundY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getBoundW(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getBoundH(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

};

#endif
