#include <lib/std.mi>
#include "attribs.m"

System.onScriptLoaded() {
  initAttribs();
  menubar_ml_attrib.onDataChanged();
}

menubar_ml_attrib.onDataChanged() {
  Group Player = getscriptgroup().findobject("player.content.ml.dummy.group");
  Group MenuBar = getscriptgroup().findobject("wasabi.menubar.ml");
  Layout main = getscriptgroup().getParentLayout();

  main.beforeRedock();
  if (getData() == "1") {
    Player.setXmlParam("y","17");
    MenuBar.show();
    main.snapAdjust(0,0,0,0);
  }
  else {
    Player.setXmlParam("y","0");
    MenuBar.hide();
    main.snapAdjust(0,0,0,17);
  }
  main.Redock();
}

System.onAccelerator(String action, String section, String key) {
  if (menubar_main_attrib.getData() == "0") return;
  Layout l = getScriptGroup().getParentLayout();
  if (!l.isActive()) return;

  // we use the general accelerators otherwise use specific ones
  // will allow the skin to cope with variations in localisations
  if (action == "MENUHOTKEY_FILE" || action == "ML_MENUHOTKEY_FILE")
  {
    getScriptGroup().findObject("ML_File.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "MENUHOTKEY_VIEW" || action == "ML_MENUHOTKEY_VIEW")
  {
    getScriptGroup().findObject("ML_View.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "MENUHOTKEY_HELP" || action == "ML_MENUHOTKEY_HELP")
  {
    getScriptGroup().findObject("ML_Help.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
}