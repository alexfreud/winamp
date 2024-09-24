#include <lib/std.mi>
#include "attribs.m"

Global Layout pl_normal;
Global Layout pl_shade;

System.onScriptLoaded() {
  initAttribs();
  menubar_pe_attrib.onDataChanged();
  pl_normal = getScriptGroup().getParentLayout();
}

menubar_pe_attrib.onDataChanged() {
  Group Player = getscriptgroup().findobject("player.content.pl.dummy.group");
  Group MenuBar = getscriptgroup().findobject("wasabi.menubar.pl");
  Layout main = getscriptgroup().getParentLayout();

  main.beforeRedock();
  if (getData() == "1") {
    Player.setXmlParam("y","16");
    MenuBar.show();
    main.snapAdjust(0,0,0,0);
  }
  else {
    Player.setXmlParam("y","0");
    MenuBar.hide();
    main.snapAdjust(0,0,0,16);
  }
  main.Redock();
}

System.onKeyDown(String k) {
  if (pl_shade == NULL)
    pl_shade = pl_normal.getContainer().getLayout("shade");

  if (StrLeft(k,4) == "ctrl" && StrSearch(k, "+w") != -1 && (pl_normal.isActive() || pl_shade.isActive())) {
    if (pl_normal.isVisible())
      pl_normal.getContainer().switchToLayout("shade");
    else
      pl_normal.getContainer().switchToLayout("normal");
    complete;
  }
}

System.onAccelerator(String action, String section, String key) {
  if (menubar_main_attrib.getData() == "0") return;
  Layout l = getScriptGroup().getParentLayout();
  if (!l.isActive()) return;

  // we use the general accelerators otherwise use specific ones
  // will allow the skin to cope with variations in localisations
  if (action == "MENUHOTKEY_FILE" || action == "PL_MENUHOTKEY_FILE")
  {
    getScriptGroup().findObject("PE_File.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "MENUHOTKEY_PLAY" || action == "PL_MENUHOTKEY_PLAYLIST")
  {
    getScriptGroup().findObject("PE_Playlist.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "PL_MENUHOTKEY_SORT")
  {
    getScriptGroup().findObject("PE_Sort.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
  if (action == "MENUHOTKEY_HELP" || action == "PL_MENUHOTKEY_HELP")
  {
    getScriptGroup().findObject("PE_Help.menu").sendAction("open", "", 0, 0, 0, 0);
    complete;
  }
}