//PORTABLE
#ifndef _STIMER_H
#define _STIMER_H

#include <api/wnd/basewnd.h>
#include <api/script/objects/rootobject.h>
#include <api/script/objects/rootobj.h>

#include <api/timer/timerclient.h>

#define STIMER_PARENT RootObjectInstance

// {5D0C5BB6-7DE1-4b1f-A70F-8D1659941941}
static const GUID timerGuid = 
{ 0x5d0c5bb6, 0x7de1, 0x4b1f, { 0xa7, 0xf, 0x8d, 0x16, 0x59, 0x94, 0x19, 0x41 } };

class TimerScriptController : public ScriptObjectControllerI {
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

extern TimerScriptController *timerController;


#ifndef _NOSTUDIO

#define STIMER_ID 2481

class STimer : public STIMER_PARENT, public TimerClientDI {
public:
  STimer();
	virtual ~STimer();

  void start(void);
  void stop(void);
  int getDelay(void);
  void setDelay(int d);
  void onTimer(void);
  int isRunning();

  void timerclient_timerCallback(int id);

private:
  int delay;
  int started;

#else
class STimer : public STIMER_SCRIPTPARENT {
#endif

public:

  static scriptVar script_onTimer(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_setDelay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar d);
  static scriptVar script_getDelay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_start(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_isRunning(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_getSkipped(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};

#endif
