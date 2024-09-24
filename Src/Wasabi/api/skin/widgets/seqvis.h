#ifndef _SEQVIS_H
#define _SEQVIS_H

#include <api/wnd/basewnd.h>
#include <api/wnd/virtualwnd.h>
#include <api/syscb/callbacks/corecbi.h>
#include <api/service/svc_enum.h>
#include <api/service/svcs/svc_skinfilter.h>
#include <api/script/objects/guiobj.h>

// {8D1EBA38-489E-483e-B960-8D1F43C5C405}
static const GUID eqvisGuid = 
{ 0x8d1eba38, 0x489e, 0x483e, { 0xb9, 0x60, 0x8d, 0x1f, 0x43, 0xc5, 0xc4, 0x5 } };

#define SEQVIS_PARENT GuiObjectWnd

class EqVisScriptController : public GuiObjectScriptController {
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

extern EqVisScriptController *eqvisController;

class SEQVis : public SEQVIS_PARENT, public CoreCallbackI {
public:
	SEQVis();
	virtual ~SEQVis();

  virtual int setXuiParam(int xiuhandle, int attrid, const wchar_t *paramname, const wchar_t *strvalue);

	int onInit();
	void DrawEQVis();
	int onPaint(Canvas *canvas);

  virtual int onResize();

protected:
/*static */void CreateXMLParameters(int master_handle);
  typedef struct {
    float *keys;              /* Key data, keyWidth*numKeys */
    signed int keyWidth;            /* Number of floats per key */
    signed int numKeys;             /* Number of keys */
    float cont;               /* Continuity. Should be -1.0 -> 1.0 */
    float bias;               /* Bias. -1.0 -> 1.0 */
    float tens;               /* Tension. -1.0 -> 1.0 */
  } spline_struct;

  void splineGetPoint(spline_struct *s, float frame, float *out);

  virtual int corecb_onEQPreampChange(int newval);
  virtual int corecb_onEQBandChange(int band, int newval);

  virtual void reloadResources();

  enum {
    SEQVIS_SETALPHA=0,
    SEQVIS_SETCOLORTOP,
    SEQVIS_SETCOLORMIDDLE,
    SEQVIS_SETCOLORBOTTOM,
    SEQVIS_SETCOLORPREAMP,
  };
	

private:
	static XMLParamPair params[];
	BltCanvas *bc;
	int *specData;
  int cur_w, cur_h;
  int colortop, colormid, colorbottom;
  int colorpreamp;
  int *shadedColors;
  int invalidated;
  PtrList<svc_skinFilter>filters;
  SkinFilterEnum *sfe;
  int xuihandle;

public:

  static scriptVar script_vcpu_fake(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static void instantiate(SEQVis *s);

};

extern const wchar_t eqVisXuiStr[];
extern char eqVisXuiSvcName[];
class EqVisXuiSvc : public XuiObjectSvc<SEQVis, eqVisXuiStr, eqVisXuiSvcName> {};


#endif