#include <precomp.h>
#include <wasabicfg.h>
#include "button.h"
#include <api/skin/skinparse.h>

#include <api/script/scriptmgr.h>

#ifdef WASABI_WIDGETS_COMPBUCK
#include <api/skin/widgets/compbuck2.h>
#endif

#ifdef WASABI_COMPILE_MEDIACORE
#include <api/core/api_core.h>
#endif

#include <api/service/svcs/svc_action.h>

const wchar_t buttonXuiObjectStr[] = L"Button"; // This is the xml tag
char buttonXuiSvcName[] = "Button xui object"; // this is the name of the xuiservice


ButtonScriptController _buttonController;
ButtonScriptController *buttonController = &_buttonController;

// -- Functions table -------------------------------------
function_descriptor_struct ButtonScriptController::exportedFunction[] = {
{L"onActivate",    1, (void*)Wasabi::Button::script_vcpu_onActivate },
  {L"setActivated",  1, (void*)Wasabi::Button::script_vcpu_setActivated },
  {L"getActivated",  0, (void*)Wasabi::Button::script_vcpu_getActivated },
  {L"onLeftClick",   0, (void*)Wasabi::Button::script_vcpu_onLeftClick },
  {L"onRightClick",  0, (void*)Wasabi::Button::script_vcpu_onRightClick },
  {L"leftClick",     0, (void*)Wasabi::Button::script_vcpu_leftClick },
  {L"rightClick",    0, (void*)Wasabi::Button::script_vcpu_rightClick },
  {L"setActivatedNoCallback",  1, (void*)Wasabi::Button::script_vcpu_setActivatedNoCallback },

};
// --------------------------------------------------------

const wchar_t *ButtonScriptController::getClassName() {
  return L"Button";
}

const wchar_t *ButtonScriptController::getAncestorClassName() {
  return L"GuiObject";
}

ScriptObject *ButtonScriptController::instantiate() {
  Wasabi::Button *b = new Wasabi::Button;
  ASSERT(b != NULL);
  return b->getScriptObject();
}

void ButtonScriptController::destroy(ScriptObject *o) {
  Wasabi::Button *b = static_cast<Wasabi::Button *>(o->vcpu_getInterface(buttonGuid));
  ASSERT(b != NULL);
  delete b;
}

void *ButtonScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for buttojn yet
}

void ButtonScriptController::deencapsulate(void *o) {
}

int ButtonScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *ButtonScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID ButtonScriptController::getClassGuid() {
  return buttonGuid;
}

XMLParamPair Wasabi::Button::params[] =
{
	 {BUTTON_ACTION, L"ACTION"}, 
   {BUTTON_ACTIONTARGET, L"ACTION_TARGET"}, 
	 {BUTTON_ACTIVEIMAGE, L"ACTIVEIMAGE"}, 
	 {BUTTON_BORDERS, L"BORDERS"}, 
	#ifdef WASABI_WIDGETS_COMPBUCK
   {BUTTON_CBTARGET, L"CBTARGET"}, 
#endif
	 {BUTTON_CENTER_IMAGE, L"CENTER_IMAGE"}, 
   {BUTTON_DOWNIMAGE, L"DOWNIMAGE"}, 
	 {BUTTON_HOVERIMAGE, L"HOVERIMAGE"}, 
	 {BUTTON_IMAGE, L"IMAGE"}, 
   {BUTTON_PARAM, L"PARAM"}, 
  // {BUTTON_RECTRGN, "RECTRGN"}, 
   {BUTTON_RETCODE, L"RETCODE"}, 
   {BUTTON_BORDERSTYLE, L"STYLE"}, 
	 {BUTTON_TEXT, L"TEXT"}, 
   {BUTTON_TEXTCOLOR, L"TEXTCOLOR"}, 
	 {BUTTON_TEXTHOVERCOLOR, L"TEXTDIMMEDCOLOR"}, 
   {BUTTON_TEXTHOVERCOLOR, L"TEXTHOVERCOLOR"},    
};
// -----------------------------------------------------------------------

Wasabi::Button::Button() {
  disablenextcontextmenu = 0;
#ifdef WASABI_COMPILE_MEDIACORE
  WASABI_API_MEDIACORE->core_addCallback(0, this);
#endif
  getScriptObject()->vcpu_setInterface(buttonGuid, (void *)static_cast<Button*>(this));
  getScriptObject()->vcpu_setClassName(L"Button");
  getScriptObject()->vcpu_setController(buttonController);
  cbtarget = NULL;
  setBorders(FALSE);
  xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);	
}

void Wasabi::Button::CreateXMLParameters(int master_handle)
{
	//BUTTON_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

Wasabi::Button::~Button() {
  //int c=getNotifyId();
#ifdef WASABI_COMPILE_MEDIACORE
  WASABI_API_MEDIACORE->core_delCallback(0, this);
#endif
  WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<WndCallbackI*>(this));
}

int Wasabi::Button::onInit() 
{
  BUTTON_PARENT::onInit();
  if (!action.isempty()) 
		setAction(action);
  //FG> this is retarded, and yes, i'm the one who coded it in 
  //if (s_normal.isempty() && s_down.isempty() && s_hover.isempty() && s_active.isempty())
  //  setRectRgn(1);
  setupBitmaps();
  //int c=getNotifyId();
	setHandleRightClick(TRUE);
#ifdef WASABI_COMPILE_MEDIACORE
  if (WCSCASEEQLSAFE(actionstr, L"eq_toggle")
		|| WCSCASEEQLSAFE(actionstr, L"eq_auto"))
	{
    if (WCSCASEEQLSAFE(actionstr, L"eq_toggle"))
			corecb_onEQStatusChange(WASABI_API_MEDIACORE->core_getEqStatus(0));
    else corecb_onEQAutoChange(WASABI_API_MEDIACORE->core_getEqAuto(0));
  }
#endif
  WASABI_API_SYSCB->syscb_registerCallback(static_cast<WndCallbackI*>(this));
  return 1;
}

void Wasabi::Button::setupBitmaps() 
{
  setBitmaps(s_normal, s_down, s_hover, s_active);
}

void Wasabi::Button::setParam(const wchar_t *p) 
{
	param=p;
}

void Wasabi::Button::setAction(const wchar_t *_action) 
{
  actionstr = _action;
  const wchar_t *name;
  int id = SkinParser::getAction(_action, &name);
  actionname = name;
  if (id == ACTION_NONE || !action_target.isempty())
    action = _action;
  else {
    setNotifyId(id);  
    action = L"";
  }
}

int Wasabi::Button::setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *paramname, const wchar_t *strvalue) {
  if (xuihandle == _xuihandle) {
    switch (xmlattributeid) {
      case BUTTON_TEXT: setButtonText(strvalue); return 1;
      case BUTTON_ACTION: action = strvalue; if (isInited()) { setAction(action); } return 1;
	    case BUTTON_IMAGE: s_normal = strvalue; if (isInited()) { setBitmaps(s_normal, s_down, s_hover, s_active); } return 1;
	    case BUTTON_DOWNIMAGE: s_down = strvalue; if (isInited()) { setBitmaps(s_normal, s_down, s_hover, s_active); } return 1;
	    case BUTTON_HOVERIMAGE: s_hover = strvalue; if (isInited()) { setBitmaps(s_normal, s_down, s_hover, s_active); } return 1;
	    case BUTTON_ACTIVEIMAGE: s_active = strvalue; if (isInited()) { setBitmaps(s_normal, s_down, s_hover, s_active); }return 1;
	    case BUTTON_PARAM: setParam(strvalue); return 1;
	    //case BUTTON_RECTRGN: setRectRgn(WTOI(strvalue)); return 1;
#ifdef WASABI_WIDGETS_COMPBUCK
      case BUTTON_CBTARGET: setCBTarget(strvalue); return 1;
#endif
      case BUTTON_BORDERS: setBorders(WTOI(strvalue)); return 1;
      case BUTTON_BORDERSTYLE: setBorderStyle(strvalue); return 1;
      case BUTTON_RETCODE: setModalRetCode(WTOI(strvalue)); return 1;
      case BUTTON_ACTIONTARGET: {
        action_target = strvalue; 
        if (!actionstr.isempty()) {
          action = actionstr;
          setAction(action);
        }
        return 1;
      }
      case BUTTON_CENTER_IMAGE: setBitmapCenter(WTOI(strvalue)); return 1;
      case BUTTON_TEXTCOLOR: setTextColor(strvalue); return 1;
      case BUTTON_TEXTHOVERCOLOR: setTextHoverColor(strvalue); return 1;
      case BUTTON_TEXTDIMMEDCOLOR: setTextDimmedColor(strvalue); return 1;
    }
  }
  return BUTTON_PARENT::setXuiParam(_xuihandle, xmlattributeid, paramname, strvalue);
}

#ifdef WASABI_WIDGETS_COMPBUCK
void Wasabi::Button::setCBTarget(const wchar_t *t) {
  cbtarget = ComponentBucket2::getComponentBucket(t);
}
#endif

const wchar_t *Wasabi::Button::getParam() 
{
	return param;
}

int Wasabi::Button::childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2) {
#ifdef WASABI_COMPILE_WNDMGR
  switch(msg) 
  {
#ifdef _WIN32
    case WM_WA_CONTAINER_TOGGLED:
      if(!param) break;
      if(!WCSICMP((const wchar_t *)param1,param)) setActivatedButton(param2);
    break;
#else
#warning port me
#endif
#ifdef WASABI_COMPILE_COMPONENTS
    case WM_WA_COMPONENT_TOGGLED:
      if(!param) break;
      GUID *g;
      if(g=SkinParser::getComponentGuid(param)) {
        if(!MEMCMP((GUID *)param1,g,sizeof(GUID))) setActivatedButton(param2);
      }
    break;
#endif
  }
#endif
  return BUTTON_PARENT::childNotify(child, msg, param1, param2);
}

#ifdef WASABI_COMPILE_MEDIACORE
int Wasabi::Button::corecb_onEQStatusChange(int newval) {
  if(WCSCASEEQLSAFE(actionstr, L"eq_toggle")) setActivatedButton(newval);
  return 0;
}

int Wasabi::Button::corecb_onEQAutoChange(int newval) {
  if(WCSCASEEQLSAFE(actionstr,L"eq_auto")) setActivatedButton(newval);
  return 0;
}
#endif

int Wasabi::Button::onLeftButtonUp(int x, int y) {
  BUTTON_PARENT::onLeftButtonUp(x, y);
  if (!WASABI_API_MAKI->vcpu_getComplete()) {
#ifdef WASABI_WIDGETS_COMPBUCK
    if (getNotifyId() == ACTION_CB_NEXT) {
      if (cbtarget)
        cbtarget->ComponentBucket2::next_up();
      else
        ComponentBucket2::next_up(getGuiObject()->guiobject_getParentGroup());
    } else if (getNotifyId() == ACTION_CB_PREV) {
      if (cbtarget)
        cbtarget->ComponentBucket2::prev_up();
      else
        ComponentBucket2::prev_up(getGuiObject()->guiobject_getParentGroup());
    } else if (getNotifyId() == ACTION_CB_NEXTPAGE) {
      if (cbtarget)
        cbtarget->ComponentBucket2::next_page();
      else
        ComponentBucket2::next_page(getGuiObject()->guiobject_getParentGroup());
    } else if (getNotifyId() == ACTION_CB_PREVPAGE) {
      if (cbtarget)
        cbtarget->ComponentBucket2::prev_page();
      else
        ComponentBucket2::prev_page(getGuiObject()->guiobject_getParentGroup());
    }
#endif
  }
  return 1;
}

int Wasabi::Button::onLeftButtonDown(int x, int y) {
  BUTTON_PARENT::onLeftButtonDown(x, y);
  if (!WASABI_API_MAKI->vcpu_getComplete()) {
#ifdef WASABI_WIDGETS_COMPBUCK
    if (getNotifyId() == ACTION_CB_NEXT) {
      if (cbtarget)
        cbtarget->ComponentBucket2::next_down();
      else
        ComponentBucket2::next_down(getGuiObject()->guiobject_getParentGroup());
    } else if (getNotifyId() == ACTION_CB_PREV) {
      if (cbtarget)
        cbtarget->ComponentBucket2::prev_down();
      else
        ComponentBucket2::prev_down(getGuiObject()->guiobject_getParentGroup());
    }
#endif
  }
  return 1;
}

void Wasabi::Button::onLeftPush(int x, int y) {
  BUTTON_PARENT::onLeftPush(x, y);
  script_vcpu_onLeftClick(SCRIPT_CALL, getScriptObject());
  if (!WASABI_API_MAKI->vcpu_getComplete()) {
    if (!action.isempty()) {
      if (!action_target.isempty()) 
			{
        GuiObject *go = getGuiObject()->guiobject_findObject(action_target);
        if (!go) 
				{
          ScriptObject *so = WASABI_API_MAKI->maki_findObject(action_target);
          if (so != NULL)
            go = static_cast<GuiObject *>(so->vcpu_getInterface(guiObjectGuid));
        }
        if (go) {
          ifc_window *w = go->guiobject_getRootWnd();
          if (w) {
            int _x = x;
            int _y = y;
            clientToScreen(&_x, &_y);
            sendAction(w, actionstr, getParam(), _x, _y);
          }
        }
      } else {
        svc_action *act = ActionEnum(actionstr).getNext();
        if (act) {
          int _x = x;
          int _y = y;
          clientToScreen(&_x, &_y);
          act->onAction(actionstr, getParam(), _x, _y, NULL, 0, this);
          SvcEnum::release(act);
        }
      }
    }
  }
}

int Wasabi::Button::wantAutoContextMenu() { 
  int a = disablenextcontextmenu; 
  disablenextcontextmenu = 0; 
  return !a; 
}

void Wasabi::Button::onRightPush(int x, int y) {
  BUTTON_PARENT::onRightPush(x, y);
  script_vcpu_onRightClick(SCRIPT_CALL, getScriptObject());  
  if (!WASABI_API_MAKI->vcpu_getComplete()) {
    if (!action.isempty()) {
      if (!action_target.isempty()) 
			{
        GuiObject *go = getGuiObject()->guiobject_findObject(action_target);
        if (!go) {
          ScriptObject *so = WASABI_API_MAKI->maki_findObject(action_target);
          if (so != NULL)
            go = static_cast<GuiObject *>(so->vcpu_getInterface(guiObjectGuid));
        }
        if (go) {
          ifc_window *w = go->guiobject_getRootWnd();
          if (w) {
            int _x = x;
            int _y = y;
            clientToScreen(&_x, &_y);
            sendAction(w, actionstr, getParam(), _x, _y);
            disablenextcontextmenu = 1;
          }
        }
      } else {
        svc_action *act = ActionEnum(actionstr).getNext();
        if (act) {
          const wchar_t *par = getParam();
          if (par == NULL || *par == 0) par = L"1";
          int _x = x;
          int _y = y;
          clientToScreen(&_x, &_y);
          disablenextcontextmenu = act->onAction(actionstr, par, _x, _y, NULL, 0, this);
          SvcEnum::release(act);
        }
      }
    }
  }
}

int Wasabi::Button::onActivateButton(int is) {
  BUTTON_PARENT::onActivateButton(is);
  scriptVar _is = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_is, is);
  script_vcpu_onActivate(SCRIPT_CALL, getScriptObject(), _is);
  return 1;
}

int Wasabi::Button::getPreferences(int what) {
  if (what == SUGGESTED_W) return getWidth();
  if (what == SUGGESTED_H) return getHeight();
  return BUTTON_PARENT::getPreferences(what);
}

int Wasabi::Button::onShowWindow(Container *c, GUID guid, const wchar_t *groupid) 
{
  if(!param) return 1;
  if (groupid != NULL && !WCSICMP(groupid, param)) 
	{
    setActivatedButton(1);
    return 1;
  }
#ifdef WASABI_COMPILE_WNDMGR
  GUID *g;
  if (g=SkinParser::getComponentGuid(param)) 
	{
    if(*g != INVALID_GUID && guid == *g) setActivatedButton(1);
  }
#endif
  return 1;
}

int Wasabi::Button::onHideWindow(Container *c, GUID guid, const wchar_t *groupid) {
  if(!param) return 1;
  if (groupid != NULL && !WCSICMP(groupid, param)) {
    setActivatedButton(0);
    return 1;
  }
#ifdef WASABI_COMPILE_WNDMGR
  GUID *g;
  if (g=SkinParser::getComponentGuid(param)) {
    if(guid == *g) setActivatedButton(0);
  }
#endif
  return 1;
}

scriptVar Wasabi::Button::script_vcpu_setActivatedNoCallback(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT
  ASSERT(SOM::isNumeric(&v));
  Button *b = static_cast<Button *>(o->vcpu_getInterface(buttonGuid));
  if (b) b->setActivatedNoCallback(SOM::makeBoolean(&v));
  RETURN_SCRIPT_VOID;
}

scriptVar Wasabi::Button::script_vcpu_onLeftClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, buttonController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar Wasabi::Button::script_vcpu_onRightClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, buttonController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar Wasabi::Button::script_vcpu_leftClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  RECT r;
  Wasabi::Button *b = static_cast<Button *>(o->vcpu_getInterface(buttonGuid));
  if (b)  {
    b->getClientRect(&r);
    b->onLeftPush(r.left, r.top);
  }
  RETURN_SCRIPT_VOID;
}

scriptVar Wasabi::Button::script_vcpu_rightClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  RECT r;
  Wasabi::Button *b = static_cast<Button *>(o->vcpu_getInterface(buttonGuid));
  if (b)  {
    b->getClientRect(&r);
    b->onRightPush(r.left, r.top);
  }
  RETURN_SCRIPT_VOID;
}

scriptVar Wasabi::Button::script_vcpu_setActivated(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT
  ASSERT(SOM::isNumeric(&v));
  Wasabi::Button *b = static_cast<Button *>(o->vcpu_getInterface(buttonGuid));
  if (b) b->setActivatedButton(SOM::makeBoolean(&v));
  RETURN_SCRIPT_VOID;
}

scriptVar Wasabi::Button::script_vcpu_getActivated(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  Wasabi::Button *b = static_cast<Button *>(o->vcpu_getInterface(buttonGuid));
  if (b) return MAKE_SCRIPT_BOOLEAN(b->getActivatedButton());
  RETURN_SCRIPT_ZERO;
}

scriptVar Wasabi::Button::script_vcpu_onActivate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, buttonController, v);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, v);
}

