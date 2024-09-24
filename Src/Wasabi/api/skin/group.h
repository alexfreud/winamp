#include "widgets/group.h"
#if 0 // not in use
#ifndef __GROUP_H
#define __GROUP_H

#ifndef _NOSTUDIO

class Group;
class Container;
class Layout;
class CfgItem;
class CfgGroup;
class SRegion;

#include <bfc/tlist.h>
#include <bfc/depview.h>
#include <api/wnd/wndclass/embeddedxui.h>
#include <api/wnd/wndclass/clickwnd.h>
#include <api/wnd/wndclass/buttwnd.h>
#include <tataki/bitmap/bitmap.h>
#include <tataki/region/region.h>
#ifdef WASABI_COMPILE_CONFIG
#include <api/config/items/cfgitem.h>
#endif // wasabi_compile_config

#include <api/wndmgr/container.h>

#endif // _nostudio

#include <api/script/script.h>
#include <api/script/scriptobj.h>
#ifdef WASABI_WIDGETS_GUIOBJECT
#include <api/script/objects/guiobj.h>
#endif
// {80F0F8BD-1BA5-42a6-A093-3236A00C8D4A}
static const GUID cfgGroupGuid = 
{ 0x80f0f8bd, 0x1ba5, 0x42a6, { 0xa0, 0x93, 0x32, 0x36, 0xa0, 0xc, 0x8d, 0x4a } };

#define RESIZE_MINW 96
#define RESIZE_MINH 24

class XmlObject;

class GroupScriptController : public GuiObjectScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return guiController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);
    virtual int getInstantiable();
    virtual ScriptObject *cast(ScriptObject *, GUID g);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern GroupScriptController *groupController;

class XuiParam {
  public:
    XuiParam(const wchar_t *_param, const wchar_t *_value) : param(_param), value(_value) {}
    virtual ~XuiParam() {}
    StringW param;
    StringW value;
};

#define GROUP_PARENT EmbeddedXuiObject

class Group : public GROUP_PARENT {

public:
	Group();
	virtual ~Group();

	int onPaint(Canvas *canvas);

  virtual int onResize();
  virtual int onPostedMove();
  virtual int onInit();
  virtual Container *getParentContainer();
  virtual int childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2);

  virtual void setBaseTexture(const wchar_t *b, int regis=1);
  virtual SkinBitmap *getBaseTexture();
  virtual ifc_window *getBaseTextureWindow();

  virtual int setXmlParam(const wchar_t *paramname, const wchar_t *strvalue);
  virtual int setXuiParam(int _xuihandle, int xuiid, const wchar_t *paramname, const wchar_t *strvalue);
  virtual api_region *getRegion();
  virtual void setRegion(api_region *r);
  void reloadDefaults();
  virtual int onGroupChange(const wchar_t *id);
  virtual void autoResize();
  virtual void startScripts();

  void onCreateObject(GuiObject *o);
  GuiObject *getObject(const wchar_t *id);

  void sendNotifyToAllChildren(int notifymsg, int param1, int param2);

  int isDeleting() { return deleting; }

  void updatePos(GuiObject *o, RECT *r=NULL);

	AutoSkinBitmap *background;
	int x, y;

  LPARAM wndHolder_getParentParam(int i=0);

  virtual void setDesignWidth(int w);
  virtual void setDesignHeight(int h);
  virtual int getDesignWidth();
  virtual int getDesignHeight();

  virtual void invalidateWindowRegion();
  virtual void setRegionOp(int o);
  virtual void setGroupContent(const wchar_t *id, SkinItem *specific_item, int scripts_enabled);
  virtual const wchar_t *getGroupContentId();
  virtual SkinItem *getGroupContentSkinItem();
  virtual void setAutoWidthSource(const wchar_t *obj);
  virtual void setAutoHeightSource(const wchar_t *obj);

  virtual void cancelCapture() {};
  virtual int getPreferences(int what);

  virtual const wchar_t *vcpu_getClassName();
  virtual ScriptObjectController *vcpu_getController() { return groupController; }

  int getNumObjects();
  ifc_window *enumObjects(int i);

  void addChild(GuiObject *g);
  void removeChild(GuiObject *g);

#ifdef WASABI_COMPILE_WNDMGR
  virtual void mouseResize(int x, int y, int resizeway);// screen coords!
  virtual void beginMove();
  virtual void beginScale();
  virtual void beginResize();
  virtual void endMove();
  virtual void endScale();
  virtual void endResize();
#endif

  virtual int getAutoWidth(void);
  virtual int getAutoHeight(void);

  virtual int isLayout();

  void setDrawBackground(int t);
  int getDrawBackground(void);

#ifdef WASABI_COMPILE_CONFIG
  static int isCfgGroup(Group *ptr);
#endif

  void addScript(int scriptid);
  void deleteScripts();
  int enumScript(int n);
  int getNumScripts();
  virtual int isDesktopAlphaSafe();
  virtual int isTransparencySafe(int excludeme=0);

  static int isGroup(Group *o);
  const wchar_t *getBackgroundStr();
  int getWidthBasedOn(GuiObject *o=NULL);
  int getHeightBasedOn(GuiObject *o=NULL);

  void fixPosition();

  const wchar_t *embeddedxui_getEmbeddedObjectId() { return xui_embedded_id; }
  virtual void onFillGroup();
  virtual int onUnknownXuiParam(const wchar_t *xmlattributename, const wchar_t *value);

  virtual ScriptObject *script_cast(GUID g);

  virtual void onMinMaxEnforcerChanged();
  virtual int isTransparencyForcedOff() { return 0; }

protected:
  static PtrList<CfgGroup> cfggrouplist;

private:
	StringW basetextureStr;
  StringW xui_embedded_id;

  void invalidateScaledReg();
  void ensureScaledRegValid();

	int resizing;
	int size_w,size_h;
	int cX,cY;
  int captured;
  POINT mousepos;
  int propagatesize;

  PtrList<XuiParam> xuiparams;

  int moving;
  int mover;
  int drawbackground;
  RECT oldRect;
  int groupmaxheight;
  int groupmaxwidth;
  int groupminheight;
  int groupminwidth;
  int lockminmax;
//  int regionop;
  TList<int> scripts;
  RegionI *subtractedreg;
  static PtrList<Group> groups;
  StringW backgroundstr;
  StringW instanceid;
  RegionI *reg;
  RegionI *scaledreg;
  int scaledregionvalid;
  int autoregionop;
  StringW content_id;
  SkinItem *content_item;
  int no_init_on_addchild;
  StringW autoheightsource;
  StringW autowidthsource;
  GuiObject *lastheightsource;
  GuiObject *lastwidthsource;
  int lastgetwidthbasedon, lastgetheightbasedon;
  
  int default_w, default_h;
  int design_w, design_h;
  int scripts_enabled;
	int xuihandle;
	static XMLParamPair groupParams[];
protected:
  enum {
    XUIGROUP_INSTANCEID=0,
    XUIGROUP_BACKGROUND,
    XUIGROUP_DRAWBACKGROUND,
    XUIGROUP_DEFAULT_W,
    XUIGROUP_DEFAULT_H,
    XUIGROUP_MAXIMUM_H,
    XUIGROUP_MAXIMUM_W,
    XUIGROUP_MINIMUM_H,
    XUIGROUP_MINIMUM_W,
    XUIGROUP_PROPAGATESIZE,
    XUIGROUP_LOCKMINMAX,
    XUIGROUP_NAME,
    XUIGROUP_AUTOWIDTHSOURCE,
    XUIGROUP_AUTOHEIGHTSOURCE,
    XUIGROUP_EMBED_XUI,
    XUIGROUP_XUITAG,
    XUIGROUP_INHERIT_GROUP,
    XUIGROUP_INHERIT_CONTENT,
    XUIGROUP_DESIGN_W,
    XUIGROUP_DESIGN_H,
		XUIGROUP_NUMPARAMS,
  };

// FG>
// -- SCRIPT -----------------------------------------------------
private:
  PtrList<ScriptObject> script_objects;
  PtrList<GuiObject> gui_objects;
  int deleting;
  int skinpart;
  int alpha;
  int disable_update_pos;

public:
  void addObject(GuiObject *o);
  void removeObject(GuiObject *o);
  void setSkinPartId(int i) { skinpart = i; }
  int getSkinPartId() { return skinpart; }

  static scriptVar script_vcpu_getObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj);
  static scriptVar script_vcpu_getNumObjects(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_enumObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i);
  static scriptVar script_vcpu_onCreateObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar ob);
  static scriptVar script_vcpu_getMousePosX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getMousePosY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_subtractRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar reg);
  static scriptVar script_vcpu_isLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_autoResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  static void instantiate(Layout *l);

};

extern const wchar_t groupXuiObjectStr[];
extern char groupXuiSvcName[];
class GroupXuiSvc : public XuiObjectSvc<Group, groupXuiObjectStr, groupXuiSvcName> {};


#ifdef WASABI_COMPILE_CONFIG

class CfgGroupScriptController : public GroupScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return groupController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual int getInstantiable();

  private:

    static function_descriptor_struct exportedFunction[];
};

extern CfgGroupScriptController *cfgGroupController;

class CfgGroup : public Group, public DependentViewerTPtr<CfgItem> {
 public:

  CfgGroup();
  virtual ~CfgGroup();

  void setAttr(CfgItem *item, const wchar_t *name);
  const wchar_t *vcpu_getClassName();
  virtual ScriptObjectController *vcpu_getController() { return cfgGroupController; }
  
  virtual int viewer_onEvent(CfgItem *item, int event, intptr_t param, void *ptr, size_t ptrlen);
  virtual void dataChanged();

  CfgItem *getCfgItem();
  const wchar_t *getAttributeName();
  const wchar_t *getCfgGuid() { return cfgguid; }
  virtual int onInit();

  static scriptVar script_vcpu_cfgGetInt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_cfgSetInt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
  static scriptVar script_vcpu_cfgGetFloat(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_cfgSetFloat(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
  static scriptVar script_vcpu_cfgGetString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_cfgSetString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
  static scriptVar script_vcpu_cfgGetName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_cfgGetGuid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onCfgChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  
 private:

  CfgItem *cfgitem;
  StringW attrname;
  StringW cfgguid;
  static wchar_t txt[512];
};

extern const wchar_t cfgGroupXuiObjectStr[];
extern char cfgGroupXuiSvcName[];
class CfgGroupXuiSvc : public XuiObjectSvc<CfgGroup, cfgGroupXuiObjectStr, cfgGroupXuiSvcName> {};

#endif // wasabi_compile_config


#endif // group.h
#endif