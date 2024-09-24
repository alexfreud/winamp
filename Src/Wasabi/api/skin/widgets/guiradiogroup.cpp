#include <precomp.h>
#include "guiradiogroup.h"
#include <api/script/objects/c_script/c_button.h>
#include <api/script/objects/guiobject.h>
#include <api/script/scriptguid.h>

void GuiRadioGroup::toggleChild(GuiObject *who) {
  int i, num = children.getNumItems();
  for (i = 0; i < num; i++) {
    GuiObject *childGuiObj = children.enumItem(i);
    if (childGuiObj != NULL) {
      GuiObject *toggleGuiObj = childGuiObj->guiobject_findObject(L"checkbox.toggle");
      if (toggleGuiObj != NULL) {
        C_Button buttonObj(*toggleGuiObj);
        if (childGuiObj != who) {
          buttonObj.setActivated(0);
        }
      }
    }
  }  
}

void GuiRadioGroup::registerChild(GuiObject *who) {
  children.addItem(who);
}

int GuiRadioGroup::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
  if (!_wcsicmp(action, L"REGISTER") && source != NULL) {
    registerChild(static_cast<GuiObject *>(source->getInterface(guiObjectGuid)));
  } else if (!_wcsicmp(action, L"TOGGLE")) {
    toggleChild(static_cast<GuiObject *>(source->getInterface(guiObjectGuid)));
    return 1;
  }
  return GUIRADIOGROUP_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
}

