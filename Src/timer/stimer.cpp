#include "api.h"
#include "stimer.h"
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include "Factory.h"
extern Factory factory;

STimer::STimer() {
  getScriptObject()->vcpu_setInterface(timerGuid, (void *)static_cast<STimer *>(this));
  getScriptObject()->vcpu_setClassName(L"Timer");
  getScriptObject()->vcpu_setController(timerController);
  delay = 1000;
  started = 0;
}

STimer::~STimer() {
}

void STimer::setDelay(int d) {
  delay = d;
  if (started)
    timerclient_setTimer(STIMER_ID, getDelay());
}

int STimer::getDelay(void) {
  return delay;
}

void STimer::start(void) {
  if (started) { stop(); } 
  timerclient_setTimer(STIMER_ID, getDelay());
  started = 1;
}

void STimer::stop(void) {
  if (!started) return;
  timerclient_killTimer(STIMER_ID);
  started = 0;
}

void STimer::onTimer(void) 
{
	if (started)
	{
		script_onTimer(SCRIPT_CALL, getScriptObject());
	}
}

void STimer::timerclient_timerCallback(int id)
{
  if (id == STIMER_ID) 
		onTimer();
}

int STimer::isRunning() {
  return started;
}

extern ScriptObjectController *script_root;
TimerScriptController _timerController;
TimerScriptController *timerController=&_timerController;

// -- Functions table -------------------------------------
function_descriptor_struct TimerScriptController::exportedFunction[] = {
  {L"setDelay", 1, (void*)STimer::script_setDelay },
  {L"getDelay", 0, (void*)STimer::script_getDelay },
  {L"start",    0, (void*)STimer::script_start },
  {L"stop",     0, (void*)STimer::script_stop },
  {L"onTimer",  0, (void*)STimer::script_onTimer },
  {L"isRunning",0, (void*)STimer::script_isRunning },
  {L"getSkipped",0, (void*)STimer::script_getSkipped },
};
// --------------------------------------------------------

const wchar_t *TimerScriptController::getClassName() {
  return L"Timer";
}

const wchar_t *TimerScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *TimerScriptController::getAncestorController() {
  return script_root;
}

ScriptObject *TimerScriptController::instantiate() {
	if (!WASABI_API_TIMER)
	{
		WASABI_API_TIMER = (TimerApi *)factory.GetInterface(0);
	}
  STimer *s = new STimer;
  ASSERT(s != NULL);
  return s->getScriptObject();
}

void TimerScriptController::destroy(ScriptObject *o) {
  STimer *s = static_cast<STimer *>(o->vcpu_getInterface(timerGuid));
  ASSERT(s != NULL);
  delete s;
}

void *TimerScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for timer yet
}

void TimerScriptController::deencapsulate(void *o) {
}

int TimerScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *TimerScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID TimerScriptController::getClassGuid() {
  return timerGuid;
}

scriptVar STimer::script_onTimer(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, timerController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

static bool isNumeric(int t)
{
	return (t == SCRIPT_INT || t == SCRIPT_BOOLEAN || t == SCRIPT_FLOAT || t == SCRIPT_DOUBLE);
}

scriptVar STimer::script_setDelay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar d) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(isNumeric(d.type));
  STimer *t = static_cast<STimer *>(o->vcpu_getInterface(timerGuid));
  if (t) t->setDelay(d.data.idata);
  RETURN_SCRIPT_VOID;
}

scriptVar STimer::script_getDelay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  STimer *t = static_cast<STimer *>(o->vcpu_getInterface(timerGuid));
  if (t) return MAKE_SCRIPT_INT(t->getDelay());
  RETURN_SCRIPT_ZERO;
}

scriptVar STimer::script_start(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  STimer *t = static_cast<STimer *>(o->vcpu_getInterface(timerGuid));
  if (t) t->start();
  RETURN_SCRIPT_VOID;
}

scriptVar STimer::script_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  STimer *t = static_cast<STimer *>(o->vcpu_getInterface(timerGuid));
  if (t) t->stop();
  RETURN_SCRIPT_VOID;
}

scriptVar STimer::script_isRunning(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  STimer *t = static_cast<STimer *>(o->vcpu_getInterface(timerGuid));
  if (t) return MAKE_SCRIPT_BOOLEAN(t->isRunning());
  RETURN_SCRIPT_ZERO;
}

scriptVar STimer::script_getSkipped(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  STimer *t = static_cast<STimer *>(o->vcpu_getInterface(timerGuid));
  if (t) return MAKE_SCRIPT_INT(t->timerclient_getSkipped());
  RETURN_SCRIPT_ZERO;
}
