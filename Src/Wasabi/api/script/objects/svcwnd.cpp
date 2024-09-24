#include "precomp.h"
#include "../../bfc/std.h"
#include "script.h"
#include "scriptmgr.h"
#include "../../bfc/notifmsg.h"
#include "../../common/script/scriptobj.h"
#include "compoobj.h"
#include "../api.h"
#include "vcpu.h"
#include "../smap.h"
#include "../skinparse.h"
#include "svcwnd.h"
#include "../services/services.h"
#include "../services/servicei.h"
#include "../svcmgr.h"
#include "../services/svc_wndcreate.h"

char svcWndXuiObjectStr[] = "SvcWnd"; // This is the xml tag
char svcWndXuiSvcName[] = "SvcWnd xui object"; // this is the name of the xuiservice


SvcWndScriptController _svcWndController;
SvcWndScriptController *svcWndController = &_svcWndController;

// -- Functions table -------------------------------------
function_descriptor_struct SvcWndScriptController::exportedFunction[] = {
  {"getGUID", 1, (void*)SvcWnd::script_vcpu_getGUID },
  {"getWac", 0, (void*)SvcWnd::script_vcpu_getWac },
};
// --------------------------------------------------------

const wchar_t *SvcWndScriptController::getClassName() {
  return L"SvcGuiObject";
}

const wchar_t *SvcWndScriptController::getAncestorClassName() {
  return "GuiObject";
}

int SvcWndScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *SvcWndScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID SvcWndScriptController::getClassGuid() {
  return svcWndGuid;
}

ScriptObject *SvcWndScriptController::instantiate() {
  SvcWnd *sv = new SvcWnd();
  ASSERT(sv != NULL);
  return sv->getScriptObject();
}

void SvcWndScriptController::destroy(ScriptObject *o) {
  SvcWnd *obj = static_cast<SvcWnd*>(o->vcpu_getInterface(svcWndGuid));
  ASSERT(obj != NULL);
  delete obj;
}

void *SvcWndScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for svcwnd yet
}

void SvcWndScriptController::deencapsulate(void *o) {
}

char SvcWndParams[][] =
{
	"DBLCLICKACTION", //SVCWND_DBLCLKACTION
		"GUID" // SVCWND_GUID
};
SvcWnd::SvcWnd() {
  getScriptObject()->vcpu_setInterface(svcWndGuid, (void *)static_cast<SvcWnd*>(this));
  getScriptObject()->vcpu_setClassName("SvcGuiObject");
  getScriptObject()->vcpu_setController(svcWndController);
  myGUID = INVALID_GUID;
  svcwnd = NULL;
  svc = NULL;
  forwarded = 0;
  xuihandle = newXuiHandle();

	addParam(xuihandle, SvcWndParams[0], SVCWND_DBLCLKACTION, XUI_ATTRIBUTE_IMPLIED);
  addParam(xuihandle, SvcWndParams[1], SVCWND_GUID, XUI_ATTRIBUTE_IMPLIED);
  
}

// servicewnd to svcwnd

int SvcWnd::childNotify(api_window *child, int msg, intptr_t param1, intptr_t param2) {
  if (child == svcwnd) {
    switch (msg) {
      case ChildNotify::SVCWND_LBUTTONDOWN:
        onLeftButtonDown(param1, param2);
        break;
      case ChildNotify::SVCWND_RBUTTONDOWN:
        onRightButtonDown(param1, param2);
        break;
      case ChildNotify::SVCWND_LBUTTONUP:
        onLeftButtonUp(param1, param2);
        break;
      case ChildNotify::SVCWND_RBUTTONUP:
        onRightButtonUp(param1, param2);
        break;
      case ChildNotify::SVCWND_LBUTTONDBLCLK:
        onLeftButtonDblClk(param1, param2);
        break;
      case ChildNotify::SVCWND_RBUTTONDBLCLK:
        onRightButtonDblClk(param1, param2);
        break;
      case ChildNotify::SVCWND_MOUSEMOVE:
        onMouseMove(param1, param2);
        break;
    }
    forwarded = 0;
  } else 
    return SVCWND_PARENT::childNotify(child, msg, param1, param2);
  return 1;
}

// virtualwnd to guiobject bridging

int SvcWnd::onLeftButtonDblClk(int x, int y) {
  if(!dblClickAction.isempty()) {
    const char *toCheck="SWITCH;";
    if(!STRNINCMP(dblClickAction,toCheck)) {
      onLeftButtonUp(x,y);
      getGuiObject()->guiobject_getParentGroup()->getParentContainer()->switchToLayout(dblClickAction.getValue()+STRLEN(toCheck));
    }
  }
  return SVCWND_PARENT::onLeftButtonDblClk(x, y);
}

int SvcWnd::onResize() {
  SVCWND_PARENT::onResize();
  RECT r = clientRect();
  if (svcwnd) 
    svcwnd->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
  return 1;
}

void SvcWnd::onSetVisible(int v) {
  if (svcwnd) svcwnd->setVisible(v);
  SVCWND_PARENT::onSetVisible(v);
}

int SvcWnd::setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (_xuihandle == xuihandle) {
    switch (xmlattributeid) {
      case SVCWND_GUID: {
        GUID *g;
        g = SkinParser::getComponentGuid(value);
        if (g)
          setGUID(*g);
        return 1;
      }
      case SVCWND_DBLCLKACTION:
        dblClickAction = value;
        return 1;
    }
  }
  return SVCWND_PARENT::setXuiParam(_xuihandle,xmlattributeid,xmlattributename,value);
}

int SvcWnd::onUnknownXuiParam(const wchar_t *param, const wchar_t *value) {
  params.addItem(new String(param));
  params.addItem(new String(value));
  return 0;
}

int SvcWnd::onInit() {
  int r = SVCWND_PARENT::onInit();
  WindowCreateByGuidEnum wce(getGUID());
  for (;;) {
    if (!svc)
      svc = wce.getNext();
    if (!svc) return 0;
    svcwnd = svc->createWindowByGuid(getGUID(), this);
    if (svcwnd != NULL) break;
    SvcEnum::release(svc); svc = NULL;
  }
  if (svcwnd != NULL) 
  {
    svcwnd->setStartHidden(1);
    if (!svcwnd->isInited())
      r &= svcwnd->init(this);
    if (params.getNumItems() > 0) {
      for (int i=0;i<params.getNumItems();i+=2) {
        svcwnd->getGuiObject()->guiobject_setXmlParam(params[i]->getValue(), params[i+1]->getValue());
      }
    }
  }
  params.deleteAll();
  return r;
}

SvcWnd::~SvcWnd() {
  if (svc) {
    svc->destroyWindow(svcwnd);
    ServiceManager::release(svc);
  }  
}

int SvcWnd::handleRatio() {
  return 1; // todo: ask window
}

void SvcWnd::setGUID(GUID g) {
  myGUID = g;
}

GUID SvcWnd::getGUID(void) {
  return myGUID;
}

int SvcWnd::getPreferences(int what) {
  if (svcwnd) return svcwnd->getPreferences(what);
  return SVCWND_PARENT::getPreferences(what);
}

scriptVar SvcWnd::script_vcpu_getGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SvcWnd *s = static_cast<SvcWnd *>(o->vcpu_getInterface(svcWndGuid));
  if (s)
    return MAKE_SCRIPT_STRING(StringPrintf(s->myGUID));
  else
    return MAKE_SCRIPT_STRING("");
}

scriptVar SvcWnd::script_vcpu_getWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  SvcWnd *s = static_cast<SvcWnd *>(o->vcpu_getInterface(svcWndGuid));
  if (s) return MAKE_SCRIPT_OBJECT(SOM::getWACObject(s->getGUID())->getScriptObject());
  RETURN_SCRIPT_VOID;
}

