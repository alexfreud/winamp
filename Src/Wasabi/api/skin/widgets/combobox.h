#ifndef __COMBOBOX_H
#define __COMBOBOX_H

#include <api/skin/widgets/dropdownlist.h>
#include <api/script/objects/c_script/h_edit.h>
#include <api/script/objects/c_script/c_edit.h>

#define COMBOBOX_PARENT DropDownList

class XmlObject;
class HEBKeysCallback;

class ComboBox : public COMBOBOX_PARENT {

  public:

    ComboBox();
    virtual ~ComboBox();

    virtual int wantTrapButton() { return 1; }
    virtual int wantTrapText() { return 0; }
    virtual int wantTrapEdit() { return 1; }

    virtual void abstract_onNewContent();
    virtual void trapControls();

    virtual const wchar_t *dropdownlist_getMainGroupId() { return L"wasabi.combobox.main.group"; }
    virtual const wchar_t *dropdownlist_getListGroupId() { return L"wasabi.combobox.list.group"; }
    virtual const wchar_t *dropdownlist_getButtonId() { return L"combobox.button"; }
    virtual const wchar_t *dropdownlist_getListId() { return L"combobox.list"; }

    virtual const wchar_t *combobox_getEditId() { return L"combobox.edit"; }
                        
    virtual const wchar_t *embeddedxui_getEmbeddedObjectId() { return combobox_getEditId(); }

    virtual void dropdownlist_onCloseList();
    virtual void dropdownlist_onOpenList();

    virtual void setText(const wchar_t *text, int hover=0); // use this to set the content of the edit box
    virtual const wchar_t *getText(int fromcontrol=0); // use this one to ask for the currently displayed entry

    virtual const wchar_t *getCustomText() { return NULL; }

    virtual void dropdownlist_onConfigureList(GuiObject *o);
    virtual void onSelect(int id, int hover);
    virtual void enter();

    void selectEditor();
    virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);

    virtual void onEditKeyDown(int vk);
    virtual void onEditKeyUp(int vk);
    virtual void onEditEnter(const wchar_t *txt);
    virtual void onEditChar(int c);

    virtual int wantTransferDownToList() { return 1; }
    virtual int wantTransferUpToList() { return 1; }
    virtual int wantTransferHomeToList() { return 1; }
    virtual int wantTransferEndToList() { return 1; }
    virtual int wantTransferPgUpToList() { return 1; }
    virtual int wantTransferPgDnToList() { return 1; }
    virtual int wantTransferEnterToList() { return 1; }
    virtual int wantDownOpenList() { return 1; }
    virtual int wantCloseListOnChar() { return 1; }
    virtual int wantEnterOnSelect() { return 1; }

    virtual void listDown();
    virtual void listUp();
    virtual void listHome();
    virtual void listEnd();
    virtual void listPageDown();
    virtual void listPageUp();
    virtual void listSelect();

    virtual void onPreCloseList();

  private:
    
    virtual void updateTextInControl(const wchar_t *text);

    HEBKeysCallback *keys_edit;
    GuiObject *lastlist;
    StringW curtxt;

    int savedidle, savedautoenter;
    int disable_getselection;
};

class HEBKeysCallback : public H_Edit {
  public:
    
    HEBKeysCallback(ScriptObject *trap, ComboBox *_callback) :
        callback(_callback), H_Edit(trap), o(trap) {
    }

  virtual void hook_onKeyDown(int vk) {
    callback->onEditKeyDown(vk);
  }

  virtual void hook_onKeyUp(int vk) {
    callback->onEditKeyUp(vk);
  }

  virtual void hook_onEnter() 
	{
    C_Edit e(o);
    callback->onEditEnter(e.getText());
  }


  virtual void hook_onChar(wchar_t c) {
    callback->onEditChar(c);
  } 

  private:
    ComboBox *callback;
    ScriptObject *o;
};


#endif
