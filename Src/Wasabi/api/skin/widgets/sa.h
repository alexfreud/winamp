#ifndef _SA_H
#define _SA_H

#include <api/wnd/wndclass/qpaintwnd.h>
#include <api/service/svc_enum.h>
#include <api/service/svcs/svc_skinfilter.h>

#define SA_TIMER_UPDATE 1
#define SA_PARENT QuickPaintWnd

// {CE4F97BE-77B0-4e19-9956-D49833C96C27}
static const GUID visGuid = 
{ 0xce4f97be, 0x77b0, 0x4e19, { 0x99, 0x56, 0xd4, 0x98, 0x33, 0xc9, 0x6c, 0x27 } };

#include <api/script/objects/guiobj.h>

class VisScriptController : public GuiObjectScriptController {
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

extern VisScriptController *visController;

class SAWnd : public SA_PARENT, public DependentViewerI {
public:
	SAWnd();
	virtual ~SAWnd();

	int onInit();
	int onRightButtonUp(int x, int y);
  int onLeftButtonDown(int x, int y);
  virtual int setXuiParam(int _xuihandle, int attrid, const wchar_t *p, const wchar_t *s);
  virtual int getPreferences(int what);

  void setChannel(int c);
  void setFlipH(int v);
  void setFlipV(int v);
  int wantAutoContextMenu() { return 0; }

  virtual int onQuickPaint(BltCanvas *c, int w, int h, int newone);

  virtual void setBandColor(int band, ARGB32 col);
  virtual void setOscColor(int n, ARGB32 col);
  virtual void setPeakColor(ARGB32 col);

  virtual void setMode(int mode);
  virtual int getMode();
  virtual void nextMode();
  virtual void getQuickPaintSize(int *w, int *h); 
  virtual void getQuickPaintSource(RECT *r); 
  virtual int wantNegativeHeight() { return 1; }
  virtual int wantFilters() { return 1; }
  virtual const wchar_t *getFiltersGroup() { return filtergroup; }
protected:
	/*static */void CreateXMLParameters(int master_handle);
private:
	int *specData;
  int  palette[256];
  int realtime;
  int xuihandle;

  enum {
    SA_SETCOLORALLBANDS=0,
    SA_SETCOLORBAND1,
    SA_SETCOLORBAND2,
    SA_SETCOLORBAND3,
    SA_SETCOLORBAND4,
    SA_SETCOLORBAND5,
    SA_SETCOLORBAND6,
    SA_SETCOLORBAND7,
    SA_SETCOLORBAND8,
    SA_SETCOLORBAND9,
    SA_SETCOLORBAND10,
    SA_SETCOLORBAND11,
    SA_SETCOLORBAND12,
    SA_SETCOLORBAND13,
    SA_SETCOLORBAND14,
    SA_SETCOLORBAND15,
    SA_SETCOLORBAND16,
    SA_SETCOLORBANDPEAK,
    SA_SETCOLORALLOSC,
    SA_SETCOLOROSC1,
    SA_SETCOLOROSC2,
    SA_SETCOLOROSC3,
    SA_SETCOLOROSC4,
    SA_SETCOLOROSC5,
    SA_SETCHANNEL,
    SA_SETFLIPH,
    SA_SETFLIPV,
    SA_SETMODE,
    SA_SETGAMMA,
		SA_SETFALLOFF,
		SA_SETPEAKFALLOFF,
		SA_SETBANDWIDTH,
		SA_FPS,
		SA_COLORING,
		SA_PEAKS,
		SA_OSCDRAWSTYLE,
  };
	static XMLParamPair params[];

  int config_safalloff;
  int config_sa_peak_falloff;
  int config_sa;
  int config_safire;
  int config_sa_peaks;
  int flip_h, flip_v;
  int channel;
  StringW filtergroup;

  int bx[75];
int t_bx[75];
float t_vx[75];

  PtrList<svc_skinFilter>filters;
  SkinFilterEnum *sfe;

#ifdef WASABI_COMPILE_CONFIG
  int saveconfsa;
  StringW confsaname;
#endif
  int off;

  virtual int viewer_onEvent(api_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen);

public:

  static scriptVar script_onFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_setRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r);
  static scriptVar script_getRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_setMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_getMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_nextMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

};

extern const wchar_t visXuiStr[];
extern char visXuiSvcName[];
class VisXuiSvc : public XuiObjectSvc<SAWnd, visXuiStr, visXuiSvcName> {};


#endif