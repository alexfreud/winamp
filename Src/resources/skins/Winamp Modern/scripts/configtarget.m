#include <lib/std.mi>

// ------------------------------------------------------------------------------------
Global GuiObject target;
Global ComponentBucket buck;
// ------------------------------------------------------------------------------------
Function turnAllOffExcept(GuiObject except);
Function turnOn(GuiObject obj);
Function turnOff(GuiObject obj);
// ------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------
// init
// ------------------------------------------------------------------------------------
System.onScriptLoaded() {
  target = getScriptGroup().findObject("skin.config.target");
  buck = getScriptGroup().findObject("my.bucket");

  // turn off all
  GuiObject o = NULL;
  turnAllOffExcept(o);
}

// ------------------------------------------------------------------------------------
// save scroller position
// ------------------------------------------------------------------------------------
System.onScriptUnloading() {
  if (buck) {
    setPrivateInt("configmenu", "last_scroll", buck.getScroll());
  }
}

// ------------------------------------------------------------------------------------
// turn on last open
// ------------------------------------------------------------------------------------
buck.onStartup() {
  setScroll(getPrivateInt("configmenu", "last_scroll", 0));
  Group g = buck.enumChildren(getPrivateInt("configmenu", "last_page", 0));
  if (!g) g = buck.enumChildren(0);
  if (!g) return;
  ToggleButton btn = g.getObject("btn");
  if (btn) btn.leftClick();
}

// ------------------------------------------------------------------------------------
// this is called by the bucket button to switch to a new group
// ------------------------------------------------------------------------------------
target.onAction(String action, String param, int x, int y, int p1, int p2, GuiObject source) {
  if (getToken(action,";",0) == "switchto") {
    String grp = getToken(action, ";", 1);
    String is_subpage = getToken(action, ";", 2);
    target.setXmlParam("groupid", grp);

    if (is_subpage!="subpage") turnAllOffExcept(source.getParent()); // getParent because the source is the button itself, the parent is the whole group item in the bucket
  }
}

// ------------------------------------------------------------------------------------
// turn off all buttons except for the parameter, also save last_page param based on param item
// ------------------------------------------------------------------------------------
turnAllOffExcept(GuiObject except) {
  if (!buck) return;
  int i=0;
  // enumerate all inserted groups, turn them off if they're not our exception
  while (i<buck.getNumChildren()) {
    GuiObject obj = buck.enumChildren(i);
    if (obj == except) { // otherwise record last page
      setPrivateInt("configmenu", "last_page", i);
      i++;
      continue;
    }
    if (obj == NULL) { break; } // shoundnt happen
    turnOff(obj);
    i++;
  }
  // turn on the clicked item
  if (except) turnOn(except);
}

// ------------------------------------------------------------------------------------
turnOn(GuiObject obj) {
  Group gobj = obj;

  // otherwise we just need this :
  ToggleButton tg = gobj.getObject("btn");
  tg.setActivated(1);
}

// ------------------------------------------------------------------------------------
turnOff(GuiObject obj) {
  Group gobj = obj;

  // otherwise we just need this :
  ToggleButton tg = gobj.getObject("btn");
  tg.setActivated(0);
}
