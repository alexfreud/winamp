#include <precomp.h>
#include "tgbutton.h"
#include <api/script/scriptmgr.h>
#include <bfc/parse/paramparser.h>

const wchar_t toggleButtonXuiObjectStr[] = L"ToggleButton"; // This is the xml tag
char toggleButtonXuiSvcName[] = "ToggleButton xui object"; // this is the name of the xuiservice

XMLParamPair ToggleButton::params[] = {
  {TOGGLEBUTTON_AUTOTOGGLE, L"AUTOTOGGLE"},
#ifdef WASABI_COMPILE_CONFIG
  {TOGGLEBUTTON_CFGVAL, L"CFGVAL"},
#endif
	};

ToggleButton::ToggleButton() {
	param=0;
getScriptObject()->vcpu_setInterface(toggleButtonGuid, (void *)static_cast<ToggleButton *>(this));
  getScriptObject()->vcpu_setClassName(L"ToggleButton");
  getScriptObject()->vcpu_setController(tgbuttonController);
  autotoggle = 1;
#ifdef WASABI_COMPILE_CONFIG
  cfgVal = 1;
#endif
  xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
	
}

void ToggleButton::CreateXMLParameters(int master_handle)
{
	//TOGGLEBUTTON_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

ToggleButton::~ToggleButton() {
}

void ToggleButton::onLeftPush(int x, int y) {
  autoToggle();
  TOGGLEBUTTON_PARENT::onLeftPush(x, y);
  onToggle(getActivatedButton());
}

void ToggleButton::autoToggle() {
  if (autotoggle) {
    if (!getActivatedButton())
      setActivatedButton(1);
    else
      setActivatedButton(0);
  }
}

void ToggleButton::onToggle(int i) {
  scriptVar _y = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_y, i ? 1 : 0);
  script_onToggle(SCRIPT_CALL, getScriptObject(), _y);
#ifdef WASABI_COMPILE_CONFIG
  getGuiObject()->guiobject_setCfgInt(i ? cfgVal : 0);
#endif
}

int ToggleButton::setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *name, const wchar_t *value) {
  if (xuihandle == _xuihandle) {
    switch (xmlattributeid) {
      case TOGGLEBUTTON_AUTOTOGGLE: autotoggle = WTOI(value); return 1;
#ifdef WASABI_COMPILE_CONFIG
      case TOGGLEBUTTON_CFGVAL: 
        cfgVal = WTOI(value); return 1;
#endif
    }
  }
  return TOGGLEBUTTON_PARENT::setXuiParam(_xuihandle, xmlattributeid, name, value);
}

#ifdef WASABI_COMPILE_CONFIG
int ToggleButton::onReloadConfig() {
  TOGGLEBUTTON_PARENT::onReloadConfig();
  setActivatedButton(getGuiObject()->guiobject_getCfgInt());
  return 1;
}
#endif

int ToggleButton::getCurCfgVal() {
  return cfgVal;
}

TgButtonScriptController _tgbuttonController;
TgButtonScriptController *tgbuttonController=&_tgbuttonController;


// -- Functions table -------------------------------------
function_descriptor_struct TgButtonScriptController::exportedFunction[] = {
  {L"onToggle", 1, (void*)ToggleButton::script_onToggle },
  {L"getCurCfgVal", 0, (void*)ToggleButton::script_getCurCfgVal},
};
// --------------------------------------------------------

const wchar_t *TgButtonScriptController::getClassName() {
  return L"ToggleButton";
}

const wchar_t *TgButtonScriptController::getAncestorClassName() {
  return L"Button";
}

ScriptObject *TgButtonScriptController::instantiate() {
  ToggleButton *tb = new ToggleButton;
  ASSERT(tb != NULL);
  return tb->getScriptObject();
}

void TgButtonScriptController::destroy(ScriptObject *o) {
  ToggleButton *tb = static_cast<ToggleButton *>(o->vcpu_getInterface(toggleButtonGuid));
  ASSERT(tb != NULL);
  delete tb;
}

void *TgButtonScriptController::encapsulate(ScriptObject *o) {
  return NULL;
}

void TgButtonScriptController::deencapsulate(void *o) {
}

int TgButtonScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *TgButtonScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID TgButtonScriptController::getClassGuid() {
  return toggleButtonGuid;
}

const wchar_t *ToggleButton::vcpu_getClassName() {
  return L"ToggleButton";
}

scriptVar ToggleButton::script_onToggle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar is) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, tgbuttonController, is);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, is);
}

scriptVar ToggleButton::script_getCurCfgVal(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  ToggleButton *tg = static_cast<ToggleButton*>(o->vcpu_getInterface(toggleButtonGuid));
  return MAKE_SCRIPT_INT(tg->getCurCfgVal());
}

//---

const wchar_t nStatesTgButtonXuiObjectStr[] = L"NStatesButton"; // This is the xml tag
char nStatesTgButtonXuiSvcName[] = "NStatesButton xui object"; // this is the name of the xuiservice
XMLParamPair NStatesTgButton::params[] = {
  {NSTATESTGBUTTON_NSTATES, L"NSTATES"},
  {NSTATESTGBUTTON_ONEVSTATE, L"AUTOELEMENTS"},
#ifdef WASABI_COMPILE_CONFIG
  {NSTATESTGBUTTON_CFGVALS, L"CFGVALS"},
#endif
};
NStatesTgButton::NStatesTgButton() {
  getScriptObject()->vcpu_setInterface(NStatesTgButtonGuid, (void *)static_cast<NStatesTgButton*>(this));
  xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
  onevstate = 0;	
}

void NStatesTgButton::CreateXMLParameters(int master_handle)
{
	//NSTATESTGBUTTON_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

NStatesTgButton::~NStatesTgButton() {
}

int NStatesTgButton::setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *paramname, const wchar_t *strvalue) {
  if (_xuihandle == btn_getXuiHandle()) {
    switch (xmlattributeid) {
      case BUTTON_IMAGE: image = strvalue; break;
      case BUTTON_HOVERIMAGE: hover = strvalue; break;
      case BUTTON_DOWNIMAGE: down = strvalue; break;
      case BUTTON_ACTIVEIMAGE: active = strvalue; break;
    }
  }
  if (xuihandle == _xuihandle) {
    switch (xmlattributeid) {
      case NSTATESTGBUTTON_NSTATES:
        setNStates(WTOI(strvalue));
        return 1;
      case NSTATESTGBUTTON_ONEVSTATE:
        setOneVisualState(!WTOI(strvalue));
        return 1;
#ifdef WASABI_COMPILE_CONFIG
      case NSTATESTGBUTTON_CFGVALS:
        cfgvals = strvalue;
        return 1;
#endif
    }
  }
  return NSTATESTGBUTTON_PARENT::setXuiParam(_xuihandle, xmlattributeid, paramname, strvalue);
}

int NStatesTgButton::onInit() {
  setState(0);
  return NSTATESTGBUTTON_PARENT::onInit();
}

void NStatesTgButton::setOneVisualState(int v) { 
  if (!!onevstate == !!v) return;
  onevstate = v; 
  if (isPostOnInit()) {
    setupBitmaps();
    invalidate();
  }
}

void NStatesTgButton::setState(int n) {
  if (nstates <= 1) return;
  state = n;
  setupBitmaps();
#ifdef WASABI_COMPILE_CONFIG
  if (!cfgvals.isempty()) {
    ParamParser pp(cfgvals);
    const wchar_t *p = pp.enumItem(state);
    if (p != NULL)
      setXmlParam(L"cfgval", p);
  } else {
    // if the skinner doesn't ask for custom config values,
    // simply use the current state number as the cfgval.
    setXmlParam(L"cfgval", StringPrintfW(L"%d", state));
  }
#endif
}

int NStatesTgButton::getActivatedButton() {
  if (nstates <= 1) return NSTATESTGBUTTON_PARENT::getActivatedButton();
  return (getState() != 0);
}

void NStatesTgButton::autoToggle() {
  if (nstates <= 1) {
    NSTATESTGBUTTON_PARENT::autoToggle();
    return;
  } else {
    int s = (state+1) % nstates;
    setState(s);
  }
}

void NStatesTgButton::setupBitmaps() {
  if (nstates <= 1 || onevstate) 
    setBitmaps(image, down, hover, active);
  else
    setBitmaps(StringPrintfW(L"%s%d", image.v(), state), 
		StringPrintfW(L"%s%d", down.v(), state), 
		StringPrintfW(L"%s%d", hover.v(), state) /*, StringPrintf("%s%d", image.v(), (state+1) % nstates)*/);
}

void NStatesTgButton::setActivatedButton(int a) {
  if (nstates <= 1) { 
    NSTATESTGBUTTON_PARENT::setActivatedButton(a);
    return;
  }
  
#ifdef WASABI_COMPILE_CONFIG
  if (!cfgvals.isempty()) {
    ParamParser pp(cfgvals);
    wchar_t t[64] = {0};
    wcsncpy(t, StringPrintfW(L"%d", a), 64);
    for (int i=0;i<pp.getNumItems();i++) {
      const wchar_t *p = pp.enumItem(i);
      if (WCSCASEEQLSAFE(p, t)) {
        setState(i);
        return;
      }
    }
  } else {
    if (!a)
      setState(0);
    else
      setState(a);
  }
#endif
}

int NStatesTgButton::getCurCfgVal() {
#ifdef WASABI_COMPILE_CONFIG
  if (!cfgvals.isempty()) {
    ParamParser pp(cfgvals);
    const wchar_t *p = pp.enumItem(state);
    if (p) return WTOI(p);
    return 0;
  } else {
    // if the skinner doesn't ask for custom config values,
    // simply use the current state number as the cfgval.
    return state;
  }
#else
  return ToggleButton::getCurCfgVal();
#endif
}
