#include <lib/std.mi>
#include "attribs.m"

System.onScriptLoaded() {
  initAttribs();
  menubar_main_attrib.onDataChanged();
}

menubar_main_attrib.onDataChanged() {
  Group Player = getscriptgroup().findobject("player.content.dummy.group");
  Group MenuBar = getscriptgroup().findobject("wasabi.menubar");
  if (getData() == "1") {
    Player.setXmlParam("y","0");
    MenuBar.show();
  } else {
    Player.setXmlParam("y","-17");
    MenuBar.hide();
  }
}

System.onAccelerator(String action, String section, String key) {
  if (menubar_main_attrib.getData() == "0") return;
  Layout l = getScriptGroup().getParentLayout();
  if (!l.isActive()) return;
  if (action == "MENUHOTKEY_FILE")
  {
    getScriptGroup().findObject("File.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "MENUHOTKEY_PLAY")
  {
    getScriptGroup().findObject("Play.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "MENUHOTKEY_OPTIONS")
  {
    getScriptGroup().findObject("Options.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "MENUHOTKEY_VIEW")
  {
    getScriptGroup().findObject("View.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "MENUHOTKEY_HELP")
  {
    getScriptGroup().findObject("Help.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
}