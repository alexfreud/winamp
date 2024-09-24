#include <precomp.h>
#include "combobox.h"
#include <api/script/objects/c_script/c_edit.h>
#include <api/skin/xmlobject.h>

ComboBox::ComboBox() {
  keys_edit = NULL;
  lastlist = NULL;
  disable_getselection = 0;
  savedidle = 0;
  savedautoenter = 0;
}

ComboBox::~ComboBox() {
  delete keys_edit;
}

void ComboBox::abstract_onNewContent() {
  COMBOBOX_PARENT::abstract_onNewContent();
  trapControls();
}

void ComboBox::trapControls() {
  delete keys_edit;

  keys_edit = NULL;

  if (wantTrapEdit()) {
    GuiObject *editGuiObj = getGuiObject()->guiobject_findObject(combobox_getEditId());
    if (editGuiObj) keys_edit = new HEBKeysCallback(*editGuiObj, this);
  }
}

void ComboBox::updateTextInControl(const wchar_t *txt) 
{
  if (txt == NULL) return;
  if (WCSCASEEQLSAFE(getText(), txt)) return;
  GuiObject *content = getContent();
  if (content != NULL) {
    if (wantTrapEdit()) {
      GuiObject *text = content->guiobject_findObject(combobox_getEditId());
      if (text != NULL) {
        C_Edit t(*text);
        t.setText(txt);
        curtxt = txt;
      }
    }
  }
}

void ComboBox::dropdownlist_onCloseList() {
  COMBOBOX_PARENT::dropdownlist_onCloseList();
  if (wantTrapEdit()) {
    GuiObject *o = embeddedxui_getEmbeddedObject();
    if (o != NULL) {
      o->guiobject_getRootWnd()->setFocus();
      GuiObject *edit = o->guiobject_findObjectByInterface(editGuid);
      if (edit != NULL) {
        C_Edit e(*edit);
        e.setAutoEnter(savedautoenter);
        e.setIdleEnabled(savedidle);
      }
    }
  }
  if (wantEnterOnSelect())
    enter();
  disable_getselection = 0;
}

void ComboBox::dropdownlist_onOpenList() {
  COMBOBOX_PARENT::dropdownlist_onOpenList();
  if (wantTrapEdit()) {
    GuiObject *o = embeddedxui_getEmbeddedObject();
    if (o != NULL) {
      o->guiobject_getRootWnd()->setFocus();
      GuiObject *edit = o->guiobject_findObjectByInterface(editGuid);
      if (edit != NULL) {
        C_Edit e(*edit);
        savedidle = e.getIdleEnabled();
        savedautoenter = e.getAutoEnter();
        e.setIdleEnabled(0);
        e.setAutoEnter(0);
      }
    }
  }
}

void ComboBox::setText(const wchar_t *text, int hover) {
  updateTextInControl(text);
  selectItem(-1, hover);
  selectEditor();
}

const wchar_t *ComboBox::getText(int fromcontrol)
{
	
  if (!fromcontrol) 
		return curtxt;

  const wchar_t *c = NULL;
  GuiObject *content = getContent();
  if (content != NULL) {
    if (wantTrapEdit()) {
      GuiObject *text = content->guiobject_findObject(combobox_getEditId());
      if (text != NULL) {
        C_Edit t(*text);
        c = t.getText();
      }
    }
  }
  curtxt = c;
  return c;
}

void ComboBox::dropdownlist_onConfigureList(GuiObject *o) {
  COMBOBOX_PARENT::dropdownlist_onConfigureList(o);
  ifc_window *w = o->guiobject_getRootWnd()->findWindowByInterface(listGuid);
  sendAction(w, L"register_tempselectnotify");
  //w->getGuiObject()->guiobject_setXmlParam("select", getCustomText());
  lastlist = w->getGuiObject();
}

void ComboBox::onSelect(int id, int hover) {
  COMBOBOX_PARENT::onSelect(id, hover);
  if (!hover) {
    selectEditor();
    if (wantEnterOnSelect())
      enter();
  }
}

void ComboBox::enter() {
  GuiObject *content = getContent();
  if (content != NULL) {
    if (wantTrapEdit()) {
      GuiObject *text = content->guiobject_findObject(combobox_getEditId());
      if (text != NULL) {
        C_Edit t(*text);
        t.enter();
      }
    }
  }
}

void ComboBox::selectEditor() {
  GuiObject *content = getContent();
  if (content != NULL) {
    if (wantTrapEdit()) {
      GuiObject *text = content->guiobject_findObject(combobox_getEditId());
      if (text != NULL) {
        C_Edit t(*text);
        t.selectAll();
      }
    }
  }
}

int ComboBox::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
  int r = COMBOBOX_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
  if (WCSCASEEQLSAFE(action, L"tempselectnotify")) {
    if (!disable_getselection)
      setText(param, 1);
  }
  return r;
}

void ComboBox::onEditKeyDown(int vk) {
  if (Std::keyDown(VK_CONTROL)) return;
  if (vk == VK_DOWN) {
    if (wantDownOpenList()) {
      if (!isListOpen())
        openList();
      else {
        if (wantTransferDownToList())
          listDown();
      }
    }
  } else if (vk == VK_UP) {
    if (wantTransferUpToList())
      listUp();
  } else if (vk == VK_HOME) {
    if (wantTransferHomeToList())
      listHome();
  } else if (vk == VK_END) {
    if (wantTransferEndToList())
      listEnd();
  } else if (vk == VK_PRIOR) {
    if (wantTransferPgUpToList())
      listPageUp();
  } else if (vk == VK_NEXT) {
    if (wantTransferPgDnToList())
      listPageDown();
  } else if (vk == VK_ESCAPE) {
    if (isListOpen())
      closeList();
  } else if (vk == VK_BACK || vk == VK_DELETE || vk == VK_LEFT || vk == VK_RIGHT) {
    if (wantCloseListOnChar()) {
      if (isListOpen())
        closeList();
    }
  }
}

void ComboBox::onEditKeyUp(int vk) 
{
  curtxt = getText(1);
}

void ComboBox::onEditEnter(const wchar_t *txt)
{
  if (isListOpen()) {
    if (wantTransferEnterToList())
      listSelect();
  }
}

void ComboBox::onEditChar(int c) {
  if (wantCloseListOnChar()) {
    if (isListOpen())
      closeList();
  }
}


void ComboBox::listUp() {
  if (lastlist != NULL && isListOpen()) {
    sendAction(lastlist->guiobject_getRootWnd(), L"up");
  }
}

void ComboBox::listDown() {
  if (lastlist != NULL && isListOpen()) {
    sendAction(lastlist->guiobject_getRootWnd(), L"down");
  }
}

void ComboBox::listHome() {
  if (lastlist != NULL && isListOpen()) {
    sendAction(lastlist->guiobject_getRootWnd(), L"home");
  }
}

void ComboBox::listEnd() {
  if (lastlist != NULL && isListOpen()) {
    sendAction(lastlist->guiobject_getRootWnd(), L"end");
  }
}

void ComboBox::listPageUp() {
  if (lastlist != NULL && isListOpen()) {
    sendAction(lastlist->guiobject_getRootWnd(), L"pageup");
  }
}

void ComboBox::listPageDown() {
  if (lastlist != NULL && isListOpen()) {
    sendAction(lastlist->guiobject_getRootWnd(), L"pagedown");
  }
}

void ComboBox::listSelect() {
  if (lastlist != NULL && isListOpen()) {
    sendAction(lastlist->guiobject_getRootWnd(), L"select_current");
  }
}

void ComboBox::onPreCloseList() {
  disable_getselection = 1;
}

