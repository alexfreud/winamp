#include <precomp.h>
#include "mainminibrowser.h"
#include <api/script/scriptguid.h>
#include <api/script/objects/guiobject.h>

ScriptObject *MainMiniBrowser::getScriptObject() 
{
  return WASABI_API_MAKI->maki_getObjectAtom(L"browser.main.object");
}

void MainMiniBrowser::back() {
  ScriptObject *so = getScriptObject();
  if (so) {
    C_Browser browser(so);
    browser.back();
  }
}

void MainMiniBrowser::forward(){
  ScriptObject *so = getScriptObject();
  if (so) {
    C_Browser browser(so);
    browser.forward();
  }
}

void MainMiniBrowser::refresh(){
  ScriptObject *so = getScriptObject();
  if (so) {
    C_Browser browser(so);
    browser.refresh();
  }
}

void MainMiniBrowser::stop(){
  ScriptObject *so = getScriptObject();
  if (so) {
    C_Browser browser(so);
    browser.stop();
  }
}

void MainMiniBrowser::home(){
  ScriptObject *so = getScriptObject();
  if (so) {
    C_Browser browser(so);
    browser.home();
  }
}

void MainMiniBrowser::navigateUrl(const wchar_t *url){
  ScriptObject *so = getScriptObject();
  if (so) {
    C_Browser browser(so);
    browser.navigateUrl(url);
  }
}

#ifdef WASABI_COMPILE_WNDMGR
void MainMiniBrowser::popMb(){
  ScriptObject *so = getScriptObject();
  if (so) {
    GuiObject *go = static_cast<GuiObject*>(so->vcpu_getInterface(guiObjectGuid));
    if (go) {
      go->guiobject_popParentLayout();
    }
  }
}
#endif

