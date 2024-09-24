#include <precomp.h>
#include <api/wndmgr/layout.h>
#include "sstatus.h"
#include <api/core/api_core.h>

const wchar_t statusXuiStr[] = L"Status"; // This is the xml tag
char statusXuiSvcName[] = "Status xui object"; // this is the name of the xuiservice
XMLParamPair SStatus::params[] = {
  {SSTATUS_SETPLAYBITMAP, L"PLAYBITMAP"},
  {SSTATUS_SETSTOPBITMAP, L"STOPBITMAP"},
  {SSTATUS_SETPAUSEBITMAP, L"PAUSEBITMAP"},
};

SStatus::SStatus() {
  getScriptObject()->vcpu_setInterface(statusGuid, (void *)static_cast<SStatus *>(this));
  getScriptObject()->vcpu_setClassName(L"Status");
  getScriptObject()->vcpu_setController(statusController);
	currentStatus = -666;
  xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
	
}

void SStatus::CreateXMLParameters(int master_handle)
{
	//SSTATUS_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

SStatus::~SStatus() {
  WASABI_API_MEDIACORE->core_delCallback(0, this);
}

int SStatus::setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval) {
  if (_xuihandle != xuihandle) return SSTATUS_PARENT::setXuiParam(_xuihandle, attrid, name, strval);
  switch (attrid) {
    case SSTATUS_SETPLAYBITMAP:
      setPlayBitmap(strval);
      break;
    case SSTATUS_SETSTOPBITMAP:
      setStopBitmap(strval);
      break;
    case SSTATUS_SETPAUSEBITMAP:
      setPauseBitmap(strval);
      break;
    default:
      return 0;
  }
  return 1;
}

int SStatus::getPreferences(int what) {
  if (what == SUGGESTED_W) return getWidth();
  if (what == SUGGESTED_H) return getHeight();
  return SSTATUS_PARENT::getPreferences(what);
}

int SStatus::getWidth() {
  if (!playBitmap.getBitmap()) return 16;
  return playBitmap.getWidth();
}

int SStatus::getHeight() {
  if (!playBitmap.getBitmap()) return 16;
  return playBitmap.getHeight();
}

int SStatus::onInit() {
  SSTATUS_PARENT::onInit();
  WASABI_API_MEDIACORE->core_addCallback(0, this);
  currentStatus=WASABI_API_MEDIACORE->core_getStatus(0);
	return 1;
}

int SStatus::onPaint(Canvas *canvas) {
  PaintBltCanvas paintcanvas;
  if (canvas == NULL) {
    if (!paintcanvas.beginPaint(this)) return 0;
    canvas = &paintcanvas;
  }

  SSTATUS_PARENT::onPaint(canvas);

  RECT r;
  getClientRect(&r);

  AutoSkinBitmap *rb = NULL;
  switch(currentStatus) {
    case -1: rb = &pauseBitmap; break;
    case 0: rb = &stopBitmap; break;
    case 1: rb = &playBitmap; break;
  }
  if (rb != NULL && rb->getBitmap() != NULL) rb->stretchToRect(canvas, &r);

  return 1;
}

int SStatus::corecb_onStarted() { currentStatus=1; invalidate(); return 0; }
int SStatus::corecb_onStopped() { currentStatus=0; invalidate(); return 0; }
int SStatus::corecb_onPaused() { currentStatus=-1; invalidate(); return 0; }
int SStatus::corecb_onUnpaused() { currentStatus=1; invalidate(); return 0; }

void SStatus::setPlayBitmap(const wchar_t *name) 
{
  playBitmap=name;
}

void SStatus::setPauseBitmap(const wchar_t *name) {
	pauseBitmap=name;
}

void SStatus::setStopBitmap(const wchar_t *name) {
	stopBitmap=name;
}

StatusScriptController _statusController;
StatusScriptController  *statusController=&_statusController;

// -- Functions table -------------------------------------
function_descriptor_struct StatusScriptController::exportedFunction[] = {
  {L"fake", 0, (void*)SStatus::script_vcpu_fake },
};

// --------------------------------------------------------

const wchar_t *StatusScriptController::getClassName() {
  return L"Status";
}

const wchar_t *StatusScriptController::getAncestorClassName() {
  return L"GuiObject";
}

ScriptObject *StatusScriptController::instantiate() {
  SStatus *st = new SStatus;
  ASSERT(st != NULL);
  return st->getScriptObject();
}

void StatusScriptController::destroy(ScriptObject *o) {
  SStatus *st = static_cast<SStatus*>(o->vcpu_getInterface(statusGuid));
  ASSERT(st != NULL);
  delete st;
}

void *StatusScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for status yet
}

void StatusScriptController::deencapsulate(void *o) {
}

int StatusScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *StatusScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID StatusScriptController::getClassGuid() {
  return statusGuid;
}

scriptVar SStatus::script_vcpu_fake(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  RETURN_SCRIPT_VOID;
}

