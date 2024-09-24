#include <precomp.h>
#include "checkbox.h"
#include <api/service/svcs/svc_action.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/script/objects/c_script/c_button.h>
#include <api/script/objects/guiobject.h>
#include <api/script/scriptguid.h>
#include <api/script/api_maki.h>

// -----------------------------------------------------------------------
CheckBox::CheckBox(const wchar_t *_text, const wchar_t *_radioid) :
    textclicks(NULL), toggleclicks(NULL), text(_text), radioid(_radioid), 
    buttonGuiObj(NULL), use_radioval(0), 
    CHECKBOX_PARENT() { }

CheckBox::~CheckBox() { 
  delete textclicks; 
  delete toggleclicks; 
}

int CheckBox::onInit() {
  int r = CHECKBOX_PARENT::onInit();
  if (radioid.len()) {
    abstract_setContent(L"wasabi.radiobutton.group");
    // After we set the content for radio, we should register ourselves
    // as a radio button under the radio group we are assigned.
    GuiObject *radioGuiObj = abstract_findObject(radioid); // this will actually go down a level
    if (radioGuiObj != NULL) {
      ifc_window *radioRootWnd = *radioGuiObj; // explicit cast operator :)
      // Now, once we have the rootwnd, we can send our radio group object a message, it will turn off other items as we turn on specific ones
      if (radioRootWnd != NULL) {
        sendAction(radioRootWnd, L"REGISTER", NULL, -1, -1, 0, 0, (void *)getGuiObject(), 4);
      }
    }
  } else {
    abstract_setContent(L"wasabi.checkbox.group");
  }
  return r;
}

void CheckBox::onNewContent() {
  // remove all previous hooks because our content changed
  delete toggleclicks;
  toggleclicks = NULL;

  delete textclicks;
  textclicks = NULL;

  // Capture the clicks on the button
  buttonGuiObj = abstract_findObject(L"checkbox.toggle");
  if (buttonGuiObj != NULL) {
    toggleclicks = new ToggleClicks(*buttonGuiObj, this);  
  }

  // Capture the clicks on the text
  GuiObject *textGuiObj = abstract_findObject(L"checkbox.text");
  if (textGuiObj != NULL) {
    textclicks = new TextClicks(*textGuiObj, this);  
  }

  // Set the text object to display the requested text.
  updateText();
}  

int CheckBox::getPreferences(int what) {
  ifc_window *w = abstract_getContentRootWnd();
  if (w != NULL)
    return w->getPreferences(what);
  return CHECKBOX_PARENT::getPreferences(what);
}

void CheckBox::setActivated(int activated, int writetocfg) {
  // this is usually called as a response to onReloadConfig, but could also be called
  // directly by a 3rd party, so if we can, we read the current value and do nothing if
  // it hasn't changed. 
  GuiObject *toggleGuiObj = abstract_findObject(L"checkbox.toggle");
  if (toggleGuiObj != NULL) {
    C_Button buttonObj(*toggleGuiObj);
    int act = buttonObj.getActivated();
    if (!!act == !!activated) return;
    buttonObj.setActivatedNoCallback(!!activated);
  }
  if (writetocfg) {
    if (use_radioval) {
      if (activated) {
#ifdef WASABI_COMPILE_CONFIG
        getGuiObject()->guiobject_setCfgString(radioval);
#endif
        if (radioval != 0) doAction();
      }
    } else {
#ifdef WASABI_COMPILE_CONFIG
      getGuiObject()->guiobject_setCfgInt(activated);
#endif
      if (activated) doAction();
    }
  }
}

// This is called by the click catchers
void CheckBox::toggle(int self_switch) {

  if (!buttonGuiObj) return;

  int activated = !!isActivated();
  if (self_switch) activated = !activated;

/*  int no_setactive = self_switch;

  if (radioid.len() > 0 && activated) { // but if we're a radiobox, do not toggle if we're already activated
    no_setactive = 1;
  }

  if (self_switch) activated = !!!activated;
*/

  if (!(activated && !radioid.isempty()))
    activated = !activated;

  if (!use_radioval || activated) {
    C_Button b(*buttonGuiObj);
    b.setActivatedNoCallback(activated);
  }

  if (activated) {
    if (use_radioval) {
#ifdef WASABI_COMPILE_CONFIG
      getGuiObject()->guiobject_setCfgString(radioval);
#endif
      if (radioval != 0) doAction();
    } else {
#ifdef WASABI_COMPILE_CONFIG
      getGuiObject()->guiobject_setCfgInt(activated);
#endif
      if (activated) doAction();
    }
   } else 
#ifdef WASABI_COMPILE_CONFIG
   if (radioid.len() == 0) getGuiObject()->guiobject_setCfgInt(0); // if we're a radioid being turned off, we shouldn't write our value, as we'll prolly be sharing the same cfgattr with other radioboxes, todo: make that optional
#endif

  if (radioid.len() > 0 && activated) {
    GuiObject *radioGuiObj = abstract_findObject(radioid);
    ifc_window *radioRootWnd = *radioGuiObj;
    if (radioRootWnd != NULL) 
      sendAction(radioRootWnd, L"TOGGLE", NULL, -1, -1, 0, 0, (void *)getGuiObject(), 4);
  }

  onToggle();
}

void CheckBox::onToggle() {
}

void CheckBox::setText(const wchar_t *_text) 
{
  text = _text;
  setName(text);
  if (isInited()) {
    updateText();
  }
}

const wchar_t *CheckBox::getText() {
  return text;
}

void CheckBox::setRadioid(const wchar_t *_radioid) 
{
  radioid = _radioid;
}

void CheckBox::setRadioVal(const wchar_t *val, int _use_radioval) 
{
  radioval = val;
  use_radioval = _use_radioval;
}

#ifdef WASABI_COMPILE_CONFIG
int CheckBox::onReloadConfig() 
{
  StringW newVal = getGuiObject()->guiobject_getCfgString();
  int checkit = use_radioval ? (newVal == radioval) : WTOI(newVal);
  setActivated(checkit, 0);
  return CHECKBOX_PARENT::onReloadConfig();
}
#endif

void CheckBox::updateText() {
  GuiObject *textGuiObj = abstract_findObject(L"checkbox.text");
  if (textGuiObj != NULL) {
    textGuiObj->guiobject_setXmlParam(L"text", text);
  }
}
int CheckBox::isActivated() {
  if (!buttonGuiObj) return 0;
  C_Button b(*buttonGuiObj);
  return b.getActivated();
}

int CheckBox::onChar(unsigned int c) {
  switch (c) {
#ifdef _WIN32
    case VK_SPACE:
      toggle(0);
      break;
#else
#warning port me
#endif
    default:
      return CHECKBOX_PARENT::onChar(c);
  }
  return 1;
}

void CheckBox::setAction(const wchar_t *str) 
{
  action = str;
}

void CheckBox::setActionTarget(const wchar_t *target) {
  action_target = target;
}

void CheckBox::setActionParam(const wchar_t *param) {
  action_param = param;
}

const wchar_t *CheckBox::getActionParam() 
{
  return action_param;
}

void CheckBox::doAction() 
{
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
        RECT cr;
        getClientRect(&cr);
        int _x = cr.left;
        int _y = cr.top;
        clientToScreen(&_x, &_y);
        sendAction(w, action, action_target, _x, _y);
      }
    }
  } else {
    svc_action *act = ActionEnum(action).getNext();
    if (act) {
      act->onAction(action, getActionParam());
      SvcEnum::release(act);
    }
  }
}
