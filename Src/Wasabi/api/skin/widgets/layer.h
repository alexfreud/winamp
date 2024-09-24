//PORTABLE
#ifndef _LAYER_H
#define _LAYER_H

#ifndef _NOSTUDIO

#include <api/script/objects/smap.h>
#ifdef WASABI_WIDGETS_GUIOBJECT
#include <api/script/objects/guiobj.h>
#endif
#include <api/wnd/basewnd.h>
#include <tataki/bitmap/bitmap.h>
#include <api/wnd/virtualwnd.h>
#include <tataki/region/region.h>
#include <tataki/bitmap/autobitmap.h>
#include <api/wnd/wndclass/qpaintwnd.h>

#endif

#include <api/script/script.h>
#ifdef WASABI_WIDGETS_GUIOBJECT
#include <api/script/objects/guiobj.h>
#endif

// {5AB9FA15-9A7D-4557-ABC8-6557A6C67CA9}
static const GUID layerGuid = 
{ 0x5ab9fa15, 0x9a7d, 0x4557, { 0xab, 0xc8, 0x65, 0x57, 0xa6, 0xc6, 0x7c, 0xa9 } };

#define LAYER_PARENT GuiObjectWnd
#ifdef WASABI_WIDGETS_GUIOBJECT
class LayerScriptController : public GuiObjectScriptController {
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

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern LayerScriptController *layerController;
#endif
#ifndef _NOSTUDIO

#define RESIZE_NONE   0
#define RESIZE_TOP    1
#define RESIZE_BOTTOM 2
#define RESIZE_LEFT   4
#define RESIZE_RIGHT  8
#define RESIZE_TOPLEFT RESIZE_TOP | RESIZE_LEFT
#define RESIZE_TOPRIGHT RESIZE_TOP | RESIZE_RIGHT
#define RESIZE_BOTTOMLEFT RESIZE_BOTTOM | RESIZE_LEFT
#define RESIZE_BOTTOMRIGHT RESIZE_BOTTOM | RESIZE_RIGHT

class FxDynamicMove;

class Layer : public LAYER_PARENT, public SkinCallbackI
{
public:
  Layer();
	virtual ~Layer();

 virtual int onInit();
  virtual int onPaint(Canvas *canvas);
  virtual int onLeftButtonDown(int x, int y);
  virtual int onLeftButtonUp(int x, int y);
  virtual int onMouseMove(int x, int y);
  virtual int onLeftButtonDblClk(int x, int y);
  virtual int getCursorType(int x, int y);
  virtual int onResize();
  virtual int onActivate();
  virtual int onDeactivate();
  virtual int getPreferences(int what);

  virtual void setDblClickParam(const wchar_t *p);
  virtual const wchar_t *getDblClickParam();

  virtual void timerCallback(int id);

  virtual void setRegionFromBitmap(const wchar_t *bmpid);

  virtual void setRegionFromMap(SMap *map, int byte, int inversed);
  virtual void setRegion(SRegion *reg);
  virtual int onSiblingInvalidateRgn(api_region *r, ifc_window *who, int who_idx, int my_idx);

  virtual int wantSiblingInvalidations();

  virtual int getSourceOffsetX();
  virtual int getSourceOffsetY();

	virtual void layer_adjustDest(RECT *r) {}

  virtual void onBeginResize(RECT r);
  virtual void onEndResize(RECT r);
  virtual SkinBitmap *getBitmap();

  virtual void onSetVisible(int show);

  void setResize(int r);
  void setScaler(int m);
  virtual void invalidateRegionCache();

  virtual api_region *getRegion(); // stretched and tiled as needed
  virtual api_region *getBitmapRegion(); // not stretched or tiled
  virtual void makeRegion();
  virtual void deleteRegion();

  virtual void setTiling(int t);
  virtual int getTiling();

	virtual void setBitmap(const wchar_t *name);
	virtual int getWidth();
	virtual int getHeight();

  virtual int setXuiParam(int _xuihandle, int id, const wchar_t *paramname, const wchar_t *strvalue);

  virtual void setRegionOp(int i);

  virtual void setInactiveBitmap(const wchar_t *name);
  virtual void onCancelCapture();

  virtual int applyResizeRestrictions(int way, int *side=NULL);
  
	virtual bool layer_isInvalid();
  //FG> fx

#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
  virtual void fx_setEnabled(int i);
  virtual int fx_getEnabled(void);
  virtual void fx_setWrap(int i);
  virtual int fx_getWrap(void);
  virtual void fx_setRect(int i);
  virtual int fx_getRect(void);
  virtual void fx_setBilinear(int i);
  virtual int fx_getBilinear(void);
  virtual void fx_setAlphaMode(int i);
  virtual int fx_getAlphaMode(void);
  virtual void fx_setBgFx(int i);
  virtual int fx_getBgFx(void);
  virtual void fx_setClear(int i);
  virtual int fx_getClear(void);
  virtual void fx_setLocalized(int i);
  virtual int fx_getLocalized(void);
  virtual void fx_setGridSize(int x, int y);
  virtual void fx_update(void);
  virtual void fx_restart(void);
  virtual void fx_onInit(void);
  virtual void fx_onFrame(void);
  virtual void fx_setSpeed(int d);
  virtual int fx_getSpeed(void);
  virtual void fx_setRealtime(int r);
  virtual int fx_getRealtime(void);
  virtual double fx_onGetPixelA(double r, double d, double x, double y);
  virtual double fx_onGetPixelX(double r, double d, double x, double y);
  virtual double fx_onGetPixelY(double r, double d, double x, double y);
  virtual double fx_onGetPixelR(double r, double d, double x, double y);
  virtual double fx_onGetPixelD(double r, double d, double x, double y);
#endif

	virtual int skincb_onColorThemeChanged(const wchar_t *newcolortheme); 

  enum {
    LAYER_SETIMAGE=0,
    LAYER_SETRESIZE,
    LAYER_SETSCALE,
    LAYER_SETREGION,
    LAYER_SETTILE,
    LAYER_SETDBLCLICKACTION,
	LAYER_DBLCLICKPARAM,
    LAYER_SETINACTIVEIMAGE,
	LAYER_SETMYCURSOR,
	LAYER_SETQUALITY,
	//	LAYER_NUMPARAMS, // martin> there is no reference for this elsewhere in gen_ff, so CUT
		
  };

  int l_customcursor;

protected:
/*static */void CreateXMLParameters(int master_handle);
  const wchar_t *layer_getBitmapName();
  SkinBitmap *layer_getBitmap();

private:
static XMLParamPair params[];
	AutoSkinBitmap bitmap;
	int resizer, resizeway, resizing, resizerect;
	int cap, scaler, scalerway, scaling;
	POINT anchor;
	int clickthrough;
  RegionI *rgn, *secrgn, *rgnclone;
  int tiling;
  StringW bitmapname;
  int hasInactiveImage;
  StringW inactiveImageName;
  AutoSkinBitmap inactiveBitmap;
  int xuihandle;
  StringW dblclickparam;

  int fx_on;
  int fx_wrap;
  int fx_rect;
  int fx_grid_x;
  int fx_grid_y;
  int fx_bilinear;
  int fx_alphamode;
  int fx_alphaonce;
  int fx_bgfx;
  int fx_clear;
  int fx_delay;
  int fx_timeron;
  int fx_local;
  int fx_realtime;
  int last_w, last_h;
  FxDynamicMove *fx_dmove;

  StringW dblClickAction;
  StringW statustext;

// FG>
// -- SCRIPT -----------------------------------------------------

public:
  static scriptVar script_vcpu_onBeginResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l, scriptVar t, scriptVar w, scriptVar h);
  static scriptVar script_vcpu_onEndResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l, scriptVar t, scriptVar w, scriptVar h);
  static scriptVar script_vcpu_setRegionFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inversed);
  static scriptVar script_vcpu_setRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar reg);

	static scriptVar script_vcpu_isInvalid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  static scriptVar script_vcpu_fx_setEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setWrap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getWrap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setRect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getRect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setBgFx(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getBgFx(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setClear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getClear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setLocalized(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getLocalized(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setBilinear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getBilinear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setAlphaMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_fx_getAlphaMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setGridSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_fx_update(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_restart(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_onInit(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_onFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_setSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s);
  static scriptVar script_vcpu_fx_getSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_fx_onGetPixelA(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_fx_onGetPixelX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_fx_onGetPixelY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_fx_onGetPixelR(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_fx_onGetPixelD(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y);

#else
class Layer : public LAYER_SCRIPTPARENT {

#endif

public:

//  INSERT_SCRIPT_OBJECT_CONTROL

};
#ifdef WASABI_WIDGETS_GUIOBJECT
extern const wchar_t layerXuiObjectStr[];
extern char layerXuiSvcName[];
class LayerXuiSvc : public XuiObjectSvc<Layer, layerXuiObjectStr, layerXuiSvcName> {};
#endif
#endif
