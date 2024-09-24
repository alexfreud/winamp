#ifndef __MAINMINIBROWSER_H
#define __MAINMINIBROWSER_H

#include <wasabicfg.h>
#include <api/script/objects/c_script/c_browser.h>
#include <api/script/objects/c_script/h_browser.h>

class MainMiniBrowser {

  public:
  
    static ScriptObject *getScriptObject();
    static void back();
    static void forward();
    static void refresh();
    static void stop();
    static void home();
    static void navigateUrl(const wchar_t *url);
#ifdef WASABI_COMPILE_WNDMGR
    static void popMb();
#endif  
};

#endif
