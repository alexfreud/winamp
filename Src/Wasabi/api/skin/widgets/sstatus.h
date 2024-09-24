//PORTABLE
#ifndef _SSTATUS_H
#define _SSTATUS_H

#include <api/wnd/basewnd.h>
#include <tataki/bitmap/autobitmap.h>
#include <api/wnd/virtualwnd.h>
#include <api/syscb/callbacks/corecbi.h>
#include <api/wnd/wndclass/guiobjwnd.h>

// {0F08C940-AF39-4b23-80F3-B8C48F7EBB59}
static const GUID statusGuid = 
{ 0xf08c940, 0xaf39, 0x4b23, { 0x80, 0xf3, 0xb8, 0xc4, 0x8f, 0x7e, 0xbb, 0x59 } };

#define SSTATUS_PARENT GuiObjectWnd

class StatusScriptController : public GuiObjectScriptController {
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

extern StatusScriptController *statusController;


#ifndef _NOSTUDIO
class SStatus : public SSTATUS_PARENT, public CoreCallbackI {
public:
  SStatus();
	virtual ~SStatus();

  virtual int onInit();
  virtual int onPaint(Canvas *canvas);

	void setPlayBitmap(const wchar_t *name);
	void setPauseBitmap(const wchar_t *name);
	void setStopBitmap(const wchar_t *name);

	virtual int getWidth();
	virtual int getHeight();

  virtual int setXuiParam(int xuihandle, int attrid, const wchar_t *name, const wchar_t *strval);
  virtual int getPreferences(int what);

  // core callbacks
  virtual int corecb_onStarted();
  virtual int corecb_onStopped();
  virtual int corecb_onPaused();
  virtual int corecb_onUnpaused();

  enum {
    SSTATUS_SETPLAYBITMAP=0,
    SSTATUS_SETSTOPBITMAP,
    SSTATUS_SETPAUSEBITMAP,
  };
	
protected:
	/*static */void CreateXMLParameters(int master_handle);
/*protected:
  virtual void timerCallback(int id);*/

private:
	AutoSkinBitmap playBitmap,pauseBitmap,stopBitmap;

	int currentStatus;
  int xuihandle;
	static XMLParamPair params[];

#else

class SStatus : public SSTATUS_SCRIPTPARENT {

#endif

public:

  static scriptVar script_vcpu_fake(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

};

extern const wchar_t statusXuiStr[];
extern char statusXuiSvcName[];
class StatusXuiSvc : public XuiObjectSvc<SStatus, statusXuiStr, statusXuiSvcName> {};


#endif
