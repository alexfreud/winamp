#include <precomp.h>
#include "pathpicker.h"
#include <api/script/objects/guiobject.h>
#include <api/script/scriptguid.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/util/selectfile.h>

#define PATHPICKER_MAIN_GROUP L"wasabi.pathpicker.main.group"
#define PATHPICKER_BUTTON L"pathpicker.button"
#define PATHPICKER_TEXT L"pathpicker.text"

// -----------------------------------------------------------------------
PathPicker::PathPicker()
{
  abstract_setAllowDeferredContent(1);
  abstract_setContent(PATHPICKER_MAIN_GROUP);
  clicks_button = NULL;
  disable_cfg_event = 0;
}

PathPicker::~PathPicker() { 
}

int PathPicker::onInit() {
  int rt = PATHPICKER_PARENT::onInit();
  return rt;
}

void PathPicker::abstract_onNewContent() {
  PATHPICKER_PARENT::abstract_onNewContent();
  trapControls();
  updatePathInControl();
}  

#ifdef WASABI_COMPILE_CONFIG
int PathPicker::onReloadConfig() {
  int r = PATHPICKER_PARENT::onReloadConfig();
  disable_cfg_event = 1;
  updatePathFromConfig(); // triggers onSelect
  disable_cfg_event = 0;
  return r;
}

void PathPicker::updatePathFromConfig() {

  const wchar_t *val = getGuiObject()->guiobject_getCfgString();
  const wchar_t *old = getPath();
  if (old && val && !_wcsicmp(val, old)) return;

  setPath(val);
}
#endif

void PathPicker::trapControls() {
  delete clicks_button;
  clicks_button = NULL;

  GuiObject *butGuiObj = getGuiObject()->guiobject_findObject(PATHPICKER_BUTTON);
  if (butGuiObj) clicks_button = new PPClicksCallback(*butGuiObj, this);
}

void PathPicker::clickCallback() {
	SelectFile sf(this,0,0);
	sf.setDefaultDir(getPath());
	if (sf.runSelector(L"directory",0,0)) {
    StringW p = sf.getDirectory();
    if (p[wcslen(p-1)] != '/' && p[wcslen(p-1)] != '\\')
      p += L"\\";
    setPath(p);
	}
}

void PathPicker::setDefault() {
#ifdef WASABI_COMPILE_CONFIG
  onReloadConfig();
#endif
}

void PathPicker::setPath(const wchar_t *path)
{
  if (WCSCASEEQLSAFE(curpath, path)) return;
  curpath = path;
  onPathChanged(path);
}

void PathPicker::onPathChanged(const wchar_t *newpath) {
  updatePathInControl();
  if (!disable_cfg_event) {
#ifdef WASABI_COMPILE_CONFIG
    if (newpath == NULL)
      getGuiObject()->guiobject_setCfgString(L"");
    else
      getGuiObject()->guiobject_setCfgString(newpath);
#endif
  }
}

void PathPicker::updatePathInControl() {
  GuiObject *content = getContent();
  if (content != NULL) {
    GuiObject *text = content->guiobject_findObject(PATHPICKER_TEXT);
    if (text != NULL) {
      C_Text t(*text);
      if (curpath.isempty())
        t.setText(L"");
      else
        t.setText(curpath);
    }
  }
}

