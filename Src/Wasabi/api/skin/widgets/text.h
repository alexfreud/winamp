//PORTABLE
#ifndef _TEXT_H
#define _TEXT_H

#include <api/script/script.h>
#include <api/script/objects/guiobj.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <bfc/string/StringW.h>
#include <bfc/depend.h>
#include "textbase.h"
#include <api/syscb/callbacks/svccbi.h>
#include <api/syscb/callbacks/skincb.h>

#define TEXT_PARENT TextBase

class svc_textFeed;

class TextScriptController : public GuiObjectScriptController {
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

extern TextScriptController *textController;

#include <api/wnd/wndclass/clickwnd.h>

#ifdef WASABI_COMPILE_MEDIACORE
#include <api/syscb/callbacks/corecbi.h>
class Text : public TEXT_PARENT, public CoreCallbackI, public DependentViewerI, public SvcCallbackI{
#else
class Text : public TEXT_PARENT, public DependentViewerI, public SvcCallbackI {
#endif
public:
  Text();
  virtual ~Text();

  virtual int onInit();
  virtual int onBufferPaint(BltCanvas *canvas, int w, int h);

	virtual int onLeftButtonDown(int x, int y);
  virtual int onMouseMove(int x, int y);
  virtual int onLeftButtonUp(int x, int y);

  virtual int getPreferences(int what);

  virtual int setXuiParam(int xuihandle, int attribid, const wchar_t *name, const wchar_t *strval);
  virtual const wchar_t *vcpu_getClassName();
  virtual ScriptObjectController *vcpu_getController() { return textController; }

  virtual int getTextWidth();
  virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

  
  virtual void getBufferPaintSize(int *w, int *h); 
  virtual void getBufferPaintSource(RECT *r);
  virtual void onNewBuffer(int w, int h) { BufferPaintWnd::onNewBuffer(w, h); invalidateTextBuffer(); }

  int setTextSize(int newsize, int alt=0);
  void setTickering(int enable);
  void setDisplay(int disp);
    
  
  
  void setShadowColor(ARGB32 c, int alt=0);
  void setShadowX(int x, int alt=0);	// relative offsets
  void setShadowY(int y, int alt=0);
  void setTimeTTS(int tts);
  void resetTicker();
  void setTimeColonWidth(int w);
  int getTimeColonWidth(int def);
  void setTimerOffStyle(int o);
  void setTimerHours(int o);
  void setTimerHoursRollover(int o);
  const wchar_t *getLastText() { return lasttxt; }

  void setAlternateName(const wchar_t *s);
  const wchar_t *getAlternateName(void);
  void setText(const wchar_t *t);

  void addCBSource(const wchar_t *cbsource);

  virtual void onTextChanged(const wchar_t *txt);
  virtual void onSetName();
  virtual void advanceTicker(int *upd);

  virtual void setTimeDisplayMode(int remaining); // will only do so if text is displaying time in the first place

  ARGB32 getShadowColor(int alt=0);

#ifdef WASABI_COMPILE_MEDIACORE
  // core callbacks
  virtual int corecb_onStatusMsg(const wchar_t *text);
  virtual int corecb_onInfoChange(const wchar_t *text);
  virtual int corecb_onStarted();
  virtual int corecb_onStopped();
  virtual int corecb_onSeeked(int newpos);
	virtual int corecb_onBitrateChange(int kbps);
	virtual int corecb_onSampleRateChange(int hz);
#endif

	static void textOut(Canvas *canvas, int x, int y, const wchar_t *txt, wchar_t widthchar, const Wasabi::FontInfo *fontInfo);

  virtual int viewer_onEvent(api_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen);
  virtual int triggerEvent(int event, intptr_t p1, intptr_t p2);

  const wchar_t *getPrintedText();

  virtual void svccb_onSvcRegister(FOURCC type, waServiceFactory *svc);
  

  void initDisplay();
  void invalidateTextBuffer();

  enum {
    TEXT_SETDISPLAY=0,
    TEXT_SETTICKER,
    TEXT_SETTEXT,
    TEXT_SETSHADOWCOLOR,
    TEXT_SETALTSHADOWCOLOR,
    TEXT_SETSHADOWX,
    TEXT_SETSHADOWY,
    TEXT_SETALTSHADOWX,
    TEXT_SETALTSHADOWY,
    TEXT_SETTIMEROFFSTYLE,
    TEXT_SETTIMERHOURS,
    TEXT_SETTIMECOLONWIDTH,
    TEXT_SETNOGRAB,
    TEXT_SETSHOWLEN,
    TEXT_SETFORCEFIXED,
    TEXT_SETFORCEUPCASE,
    TEXT_SETFORCELOCASE,
    TEXT_SETCBSOURCE,
    TEXT_SETWRAPPED,
    TEXT_SETVALIGN,
    TEXT_SETALTVALIGN,
    TEXT_SETDBLCLKACTION,
    TEXT_SETRCLKACTION,
    TEXT_SETOFFSETX,
    TEXT_SETOFFSETY,
    TEXT_SETTICKERSTEP,
	TEXT_SETTIMERHOURSROLLOVER,
	TEXT_NUMPARAMS,
  };
	

protected:
	/*static */void CreateXMLParameters(int master_handle);
  virtual void timerCallback(int id);

private:
	static XMLParamPair params[];
  
  const wchar_t *parseText(const wchar_t *s);
  void registerToTextFeedService();
  int size[2];
	int textpos,tts,sens;
  int time_tts;
	int grab_x;
	int cur_len;
	int ticker;
	int display;
	int elapsed;
	int fixedTimerStyle;
	
  int nograb;
  int showlen;
  int forcefixed;
  int timeroffstyle;
  
  StringW displaystr;
	StringW alternatename;
  StringW lastText;
	
	FilteredColor shadowcolor[2];
  
  SkinColor sshadowcolor[2];
  int shadowcolor_mode[2];
  int shadowx[2], shadowy[2];
	int timecolonw;
	StringW deftext;
	
	
	
  PtrList<StringW> mycbid;
  StringW cbsource;
  int forceupcase, forcelocase;
  StringW lasttxt;
  
  
  int lastautowidth;

  svc_textFeed *textfeed;
  StringW feed_id;
  int registered_syscb;

  int wrapped;
  int valign[2];
  int xuihandle;
  int offsetx, offsety;

  StringW printedtxt;
  int tickerstep;
  int skipn;
  int skip;
  int skipcfgcount;
  
  int timerhours;
  int timerhoursRollover;
  int bufferinvalid;
  int cachedsizew;

public:
  static scriptVar script_vcpu_setText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t);
  static scriptVar script_vcpu_setAlternateText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t);
  static scriptVar script_vcpu_getText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getTextWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onTextChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar text);
  static wchar_t s_txt[WA_MAX_PATH];
};

extern const wchar_t textXuiObjectStr[];
extern char textXuiSvcName[];
class TextXuiSvc : public XuiObjectSvc<Text, textXuiObjectStr, textXuiSvcName> {};

#endif
